/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// net_wipx.c

#include "quakedef.h"
#include "winquake.h"
#include <wsipx.h>
#include "net_wipx.h"

extern cvar_t hostname;

#define MAXHOSTNAMELEN		256

static int net_acceptsocket = -1;		// socket for fielding new connections
static int net_controlsocket;
static struct qsockaddr broadcastaddr;

extern qboolean winsock_initialized;
extern WSADATA		winsockdata;

#define IPXSOCKETS 18
static int ipxsocket[IPXSOCKETS];
static int sequence[IPXSOCKETS];

//=============================================================================

int WIPX_Init (void)
{
	int		i;
	char	buff[MAXHOSTNAMELEN];
	struct qsockaddr addr;
	char	*p;
	int		r;
	WORD	wVersionRequested; 

	if (COM_CheckParm ("-noipx"))
		return -1;

// make sure LoadLibrary has happened successfully
	if (!winsock_lib_initialized)
		return -1;

	if (winsock_initialized == 0)
	{
		wVersionRequested = MAKEWORD(1, 1); 

		r = pWSAStartup (MAKEWORD(1, 1), &winsockdata);

		if (r)
		{
			Con_Printf ("Winsock initialization failed.\n");
			return -1;
		}
	}
	winsock_initialized++;

	for (i = 0; i < IPXSOCKETS; i++)
		ipxsocket[i] = 0;

	// determine my name & address
	if (pgethostname(buff, MAXHOSTNAMELEN) == 0)
	{
		// if the quake hostname isn't set, set it to the machine name
		if (Q_strcmp(hostname.string, "UNNAMED") == 0)
		{
			// see if it's a text IP address (well, close enough)
			for (p = buff; *p; p++)
				if ((*p < '0' || *p > '9') && *p != '.')
					break;

			// if it is a real name, strip off the domain; we only want the host
			if (*p)
			{
				for (i = 0; i < 15; i++)
					if (buff[i] == '.')
						break;
				buff[i] = 0;
			}
			Cvar_Set ("hostname", buff);
		}
	}

	if ((net_controlsocket = WIPX_OpenSocket (0)) == -1)
	{
		Con_Printf("WIPX_Init: Unable to open control socket\n");
		if (--winsock_initialized == 0)
			pWSACleanup ();
		return -1;
	}

	((struct sockaddr_ipx *)&broadcastaddr)->sa_family = AF_IPX;
	memset(((struct sockaddr_ipx *)&broadcastaddr)->sa_netnum, 0, 4);
	memset(((struct sockaddr_ipx *)&broadcastaddr)->sa_nodenum, 0xff, 6);
	((struct sockaddr_ipx *)&broadcastaddr)->sa_socket = htons((unsigned short)net_hostport);

	WIPX_GetSocketAddr (net_controlsocket, &addr);
	Q_strcpy(my_ipx_address,  WIPX_AddrToString (&addr));
	p = Q_strrchr (my_ipx_address, ':');
	if (p)
		*p = 0;

	Con_Printf("Winsock IPX Initialized\n");
	ipxAvailable = true;

	return net_controlsocket;
}

//=============================================================================

void WIPX_Shutdown (void)
{
	WIPX_Listen (false);
	WIPX_CloseSocket (net_controlsocket);
	if (--winsock_initialized == 0)
		pWSACleanup ();
}

//=============================================================================

void WIPX_Listen (qboolean state)
{
	// enable listening
	if (state)
	{
		if (net_acceptsocket != -1)
			return;
		if ((net_acceptsocket = WIPX_OpenSocket (net_hostport)) == -1)
			Sys_Error ("WIPX_Listen: Unable to open accept socket\n");
		return;
	}

	// disable listening
	if (net_acceptsocket == -1)
		return;
	WIPX_CloseSocket (net_acceptsocket);
	net_acceptsocket = -1;
}

//=============================================================================

int WIPX_OpenSocket (int port)
{
	int handle;
	int newsocket;
	struct sockaddr_ipx address;
	u_long _true = 1;

	for (handle = 0; handle < IPXSOCKETS; handle++)
		if (ipxsocket[handle] == 0)
			break;
	if (handle == IPXSOCKETS)
		return -1;

	if ((newsocket = psocket (AF_IPX, SOCK_DGRAM, NSPROTO_IPX)) == INVALID_SOCKET)
		return -1;

	if (pioctlsocket (newsocket, FIONBIO, &_true) == -1)
		goto ErrorReturn;

	if (psetsockopt(newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&_true, sizeof(_true)) < 0)
		goto ErrorReturn;

	address.sa_family = AF_IPX;
	memset(address.sa_netnum, 0, 4);
	memset(address.sa_nodenum, 0, 6);;
	address.sa_socket = htons((unsigned short)port);
	if( bind (newsocket, (void *)&address, sizeof(address)) == 0)
	{
		ipxsocket[handle] = newsocket;
		sequence[handle] = 0;
		return handle;
	}

	Sys_Error ("Winsock IPX bind failed\n");
ErrorReturn:
	pclosesocket (newsocket);
	return -1;
}

//=============================================================================

int WIPX_CloseSocket (int handle)
{
	int socket = ipxsocket[handle];
	int ret;

	ret =  pclosesocket (socket);
	ipxsocket[handle] = 0;
	return ret;
}


//=============================================================================

int WIPX_Connect (int handle, struct qsockaddr *addr)
{
	return 0;
}

//=============================================================================

int WIPX_CheckNewConnections (void)
{
	unsigned long	available;

	if (net_acceptsocket == -1)
		return -1;

	if (pioctlsocket (ipxsocket[net_acceptsocket], FIONREAD, &available) == -1)
		Sys_Error ("WIPX: ioctlsocket (FIONREAD) failed\n");
	if (available)
		return net_acceptsocket;
	return -1;
}

//=============================================================================

static byte packetBuffer[NET_DATAGRAMSIZE + 4];

int WIPX_Read (int handle, byte *buf, int len, struct qsockaddr *addr)
{
	int addrlen = sizeof (struct qsockaddr);
	int socket = ipxsocket[handle];
	int ret;

	ret = precvfrom (socket, packetBuffer, len+4, 0, (struct sockaddr *)addr, &addrlen);
	if (ret == -1)
	{
		int errno = pWSAGetLastError();

		if (errno == WSAEWOULDBLOCK || errno == WSAECONNREFUSED)
			return 0;

	}

	if (ret < 4)
		return 0;
	
	// remove sequence number, it's only needed for DOS IPX
	ret -= 4;
	memcpy(buf, packetBuffer+4, ret);

	return ret;
}

//=============================================================================

int WIPX_Broadcast (int handle, byte *buf, int len)
{
	return WIPX_Write (handle, buf, len, &broadcastaddr);
}

//=============================================================================

int WIPX_Write (int handle, byte *buf, int len, struct qsockaddr *addr)
{
	int socket = ipxsocket[handle];
	int ret;

	// build packet with sequence number
	*(int *)(&packetBuffer[0]) = sequence[handle];
	sequence[handle]++;
	memcpy(&packetBuffer[4], buf, len);
	len += 4;

	ret = psendto (socket, packetBuffer, len, 0, (struct sockaddr *)addr, sizeof(struct qsockaddr));
	if (ret == -1)
		if (pWSAGetLastError() == WSAEWOULDBLOCK)
			return 0;

	return ret;
}

//=============================================================================

char *WIPX_AddrToString (struct qsockaddr *addr)
{
	static char buf[28];

	sprintf(buf, "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%u",
		((struct sockaddr_ipx *)addr)->sa_netnum[0] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_netnum[1] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_netnum[2] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_netnum[3] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[0] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[1] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[2] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[3] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[4] & 0xff,
		((struct sockaddr_ipx *)addr)->sa_nodenum[5] & 0xff,
		ntohs(((struct sockaddr_ipx *)addr)->sa_socket)
		);
	return buf;
}

//=============================================================================

int WIPX_StringToAddr (char *string, struct qsockaddr *addr)
{
	int  val;
	char buf[3];

	buf[2] = 0;
	Q_memset(addr, 0, sizeof(struct qsockaddr));
	addr->sa_family = AF_IPX;

#define DO(src,dest)	\
	buf[0] = string[src];	\
	buf[1] = string[src + 1];	\
	if (sscanf (buf, "%x", &val) != 1)	\
		return -1;	\
	((struct sockaddr_ipx *)addr)->dest = val

	DO(0, sa_netnum[0]);
	DO(2, sa_netnum[1]);
	DO(4, sa_netnum[2]);
	DO(6, sa_netnum[3]);
	DO(9, sa_nodenum[0]);
	DO(11, sa_nodenum[1]);
	DO(13, sa_nodenum[2]);
	DO(15, sa_nodenum[3]);
	DO(17, sa_nodenum[4]);
	DO(19, sa_nodenum[5]);
#undef DO

	sscanf (&string[22], "%u", &val);
	((struct sockaddr_ipx *)addr)->sa_socket = htons((unsigned short)val);

	return 0;
}

//=============================================================================

int WIPX_GetSocketAddr (int handle, struct qsockaddr *addr)
{
	int socket = ipxsocket[handle];
	int addrlen = sizeof(struct qsockaddr);
	unsigned int a;

	Q_memset(addr, 0, sizeof(struct qsockaddr));
	if(pgetsockname(socket, (struct sockaddr *)addr, &addrlen) != 0)
	{
		int errno = pWSAGetLastError();
	}

	return 0;
}

//=============================================================================

int WIPX_GetNameFromAddr (struct qsockaddr *addr, char *name)
{
	Q_strcpy(name, WIPX_AddrToString(addr));
	return 0;
}

//=============================================================================

int WIPX_GetAddrFromName(char *name, struct qsockaddr *addr)
{
	int n;
	char buf[32];

	n = Q_strlen(name);

	if (n == 12)
	{
		sprintf(buf, "00000000:%s:%u", name, net_hostport);
		return WIPX_StringToAddr (buf, addr);
	}
	if (n == 21)
	{
		sprintf(buf, "%s:%u", name, net_hostport);
		return WIPX_StringToAddr (buf, addr);
	}
	if (n > 21 && n <= 27)
		return WIPX_StringToAddr (name, addr);

	return -1;
}

//=============================================================================

int WIPX_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2)
{
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (*((struct sockaddr_ipx *)addr1)->sa_netnum && *((struct sockaddr_ipx *)addr2)->sa_netnum)
		if (memcmp(((struct sockaddr_ipx *)addr1)->sa_netnum, ((struct sockaddr_ipx *)addr2)->sa_netnum, 4) != 0)
			return -1;
	if (memcmp(((struct sockaddr_ipx *)addr1)->sa_nodenum, ((struct sockaddr_ipx *)addr2)->sa_nodenum, 6) != 0)
		return -1;

	if (((struct sockaddr_ipx *)addr1)->sa_socket != ((struct sockaddr_ipx *)addr2)->sa_socket)
		return 1;

	return 0;
}

//=============================================================================

int WIPX_GetSocketPort (struct qsockaddr *addr)
{
	return ntohs(((struct sockaddr_ipx *)addr)->sa_socket);
}


int WIPX_SetSocketPort (struct qsockaddr *addr, int port)
{
	((struct sockaddr_ipx *)addr)->sa_socket = htons((unsigned short)port);
	return 0;
}

//=============================================================================
