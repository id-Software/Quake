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
// net_bw.c

#include <stdio.h>
#include <stdlib.h>
#include <dpmi.h>

#include "quakedef.h"
#include "dosisms.h"


// this section is general Unix stuff that we need

#define	EIO		 		5	/* I/O error */
#define EBADS			9
#define EWOULDBLOCK		35	/* function would block */
#define EMSGSIZE		40	/* message to big for buffers */
#define	EPROTONOSUPPORT	43	/* Protocol not supported */
#define	ESOCKTNOSUPPORT	44	/* Socket type not supported */
#define	EPFNOSUPPORT	46	/* Protocol family not supported */
#define EAFNOSUPPORT	47	/* Address family not supported */
#define ECONNABORTED	53	/* User requested hangup */
#define ENOBUFS 		55	/* No buffers available */
#define EISCONN 		56	/* Socket has closed */
#define ENOTCONN		57	/* Socket is not connected */
#define ESHUTDOWN		58	/* Socket is closed */
#define ETOOMANYREFS	59	/* Too many sockets open */
#define ETIMEDOUT		60	/* Connection timed out */
#define ECONNREFUSED	61	/* Connection refused */

#define AF_INET 		2	/* internet */

#define PF_INET			AF_INET

#define SOCK_STREAM		1		/* stream */
#define SOCK_DGRAM		2		/* datagram */

#define IPPROTO_TCP		6
#define IPPROTO_UDP		17

#define INADDR_ANY		0

#define SIOCDONE		0x7300
#define FIONREAD		0x667f
#define FIONBIO 		0x667e
#define FIONWIN 		0x1000
#define FIONTIN 		0x2000

#define BRDINIT 		0
#define BRDADDR 		10

#define MAXHOSTNAMELEN	256

#define SOL_SOCKET		0xffff		/* options for socket level */

/*
 * Option flags per-socket.
 */
#define SO_DEBUG		0x0001		/* turn on debugging info recording */
#define SO_ACCEPTCONN	0x0002		/* socket has had listen() */
#define SO_REUSEADDR	0x0004		/* allow local address reuse */
#define SO_KEEPALIVE	0x0008		/* keep connections alive */
#define SO_DONTROUTE	0x0010		/* just use interface addresses */
#define SO_BROADCAST	0x0020		/* permit sending of broadcast msgs */
#define SO_USELOOPBACK	0x0040		/* bypass hardware when possible */
#define SO_LINGER		0x0080		/* linger on close if data present */
#define SO_OOBINLINE	0x0100		/* leave received OOB data in line */
#define SO_USEPRIV		0x4000		/* allocate from privileged port area */
#define SO_CANTSIG		0x8000		/* prevent SIGPIPE on SS_CANTSENDMORE */

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF		0x1001		/* send buffer size */
#define SO_RCVBUF		0x1002		/* receive buffer size */
#define SO_SNDLOWAT		0x1003		/* send low-water mark */
#define SO_RCVLOWAT		0x1004		/* receive low-water mark */
#define SO_SNDTIMEO		0x1005		/* send timeout */
#define SO_RCVTIMEO		0x1006		/* receive timeout */
#define SO_ERROR		0x1007		/* get error status and clear */
#define SO_TYPE			0x1008		/* get socket type */


struct in_addr
{
	union
	{
		struct { unsigned char s_b1,s_b2,s_b3,s_b4; } S_un_b;
		struct { unsigned short s_w1,s_w2; } S_un_w;
		unsigned long S_addr;
	} S_un;
};
#define	s_addr	S_un.S_addr	/* can be used for most tcp & ip code */
#define	s_host	S_un.S_un_b.s_b2	/* host on imp */
#define	s_net	S_un.S_un_b.s_b1	/* network */
#define	s_imp	S_un.S_un_w.s_w2	/* imp */
#define	s_impno	S_un.S_un_b.s_b4	/* imp # */
#define	s_lh	S_un.S_un_b.s_b3	/* logical host */

struct sockaddr_in
{
    short			sin_family;
    unsigned short	sin_port;
	struct in_addr	sin_addr;
    char			sin_zero[8];
};

struct	hostent {
	char	*h_name;	/* official name of host */
	char	**h_aliases;	/* alias list */
	int	h_addrtype;	/* host address type */
	int	h_length;	/* length of address */
	char	**h_addr_list;	/* list of addresses from name server */
#define	h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

char *inet_ntoa(struct in_addr in);


// this section is B&W specific constants & structures

#define BW_IOCTL_BIND			0
#define BW_IOCTL_CLEAROPTIONS	5
#define BW_IOCTL_SETOPTIONS		6
#define BW_IOCTL_PEEK			7
#define BW_IOCTL_SETWINMASK		8

#define BW_OPTION_BLOCKING		0x01
#define BW_OPTION_REUSEBUFFERS	0x80

#define BW_ERR_USR_HANGUP		50
#define BW_ERR_HANGUP			51
#define BW_ERR_NET_ERR			52
#define BW_ERR_IS_CLOSED		53
#define BW_ERR_TIME_OUT			54
#define BW_ERR_RESET			55
#define BW_ERR_FULL				56
#define BW_ERR_BLOCK			57
#define BW_ERR_SHUTDOWN			58

#pragma pack(1)

typedef struct
{
	char			state;			// always 1
	short			localPort;
	struct in_addr	localAddr;
	char			reason;			// always 0
	char			options;
	short			dataAvailable;
} BW_UDPinfo_t;

typedef struct
{
	char			reserved1 [6];
	unsigned short	info2Offset;
	char			reserved2 [18];
	struct in_addr	remoteAddr;
} BW_UDPreadInfo1_t;

typedef struct
{
	short			remotePort;
	char			reserved1 [2];
	unsigned short	dataLenPlus8;
	char			reserved2 [2];
	char			data[1];			// actual size is <dataLenPlus8> - 8		
} BW_UDPreadInfo2_t;

typedef struct
{
	char			reserved1 [2];
	short			remotePort;
	unsigned short	dataLen;
	struct in_addr	remoteAddr;
	char			reserved2 [42];
	char			data[1];			// actual size is <datalen>
} BW_writeInfo_t;

typedef struct
{
	short	ioport;
	byte	dma;
	byte	vector;
	byte	irq;
	short	bufferSize;
	short	maxWindow;
	short	timeZone;
	byte	myType;
	int		inetAddr;
	short	value;
	byte	subnetMask;
	short	etherPointer;
	short	logserverPointer;
	short	nameserverPointer;
	short	printserverPointer;
	short	timeserverPointer;
	short	gatewayPointer;
	short	driverSegment;
	byte	transferSize;
	char	cardName [9];
} BW_ethdevinfo_t;

#pragma pack()

#define LOWMEM_SIZE	4096

static unsigned char *lowmem_buffer;
static int lowmem_bufseg;
static int lowmem_bufoff;
static BW_ethdevinfo_t ethdevinfo;
static int netmask;
static struct in_addr bcastaddr;

extern regs_t regs;

static int net_acceptsocket = -1;		// socket for fielding new connections
static int net_controlsocket = 0;

#include "net_bw.h"

//=============================================================================

static int BW_ioctl(int s, char *msg, int msglen)
{
	Q_memcpy(lowmem_buffer, msg, msglen);

	regs.x.ax = 0x4403;
	regs.x.bx = s;
	regs.x.cx = msglen;
	regs.x.dx = lowmem_bufoff;
	regs.x.ds = lowmem_bufseg;
	if (dos_int86(0x21))
		return regs.x.ax;
	return 0;
}

//=============================================================================

static int BW_TranslateError(int error)
{
	switch(error)
	{
		case BW_ERR_USR_HANGUP:	return ECONNABORTED;
		case BW_ERR_HANGUP:		return EISCONN;
		case BW_ERR_NET_ERR:	return ENOTCONN;
		case BW_ERR_IS_CLOSED:	return ENOTCONN;
		case BW_ERR_TIME_OUT:	return ETIMEDOUT;
		case BW_ERR_RESET:		return ECONNREFUSED;
		case BW_ERR_FULL:		return ETOOMANYREFS;
		case BW_ERR_BLOCK:		return EWOULDBLOCK;
		case BW_ERR_SHUTDOWN:	return ESHUTDOWN;
	}
	return EIO;
}

//=============================================================================

static int GetEthdevinfo(void)
{
	int fd;

	Q_strcpy((char *)lowmem_buffer, "ETHDEV27");
	regs.x.ax = 0x3d42;
	regs.x.ds = lowmem_bufseg;
	regs.x.dx = lowmem_bufoff;
	if (dos_int86(0x21))
		return -1;
	fd = regs.x.ax;

	regs.x.ax = 0x4401;
	regs.x.bx = fd;
	regs.x.dx = 0x60;
	dos_int86(0x21);

	regs.h.ah = 0x3f;
	regs.x.cx = sizeof(ethdevinfo);
	regs.x.es = regs.x.ds = lowmem_bufseg;
	regs.x.dx = lowmem_bufoff;
	regs.x.bx = fd;
	if (dos_int86(0x21))
		return -1;
	Q_memcpy(&ethdevinfo, lowmem_buffer, regs.x.ax);

	regs.h.ah = 0x3e;
	regs.x.bx = fd;
	dos_int86(0x21);

	return 0;
}

//=============================================================================

int BW_Init(void)
{
	struct qsockaddr addr;
	char *colon;

	if (COM_CheckParm ("-noudp"))
		return -1;

	lowmem_buffer = dos_getmemory(LOWMEM_SIZE);
	if (!lowmem_buffer)
		Sys_Error("not enough low memory\n");
	lowmem_bufoff = ptr2real(lowmem_buffer) & 0xf;
	lowmem_bufseg = ptr2real(lowmem_buffer) >> 4;

	if (GetEthdevinfo())
	{
		Con_DPrintf("Beame & Whiteside TCP/IP not detected\n");
		dos_freememory(lowmem_buffer);
		return -1;
	}
	netmask = 0xffffffff >> (32 - ethdevinfo.subnetMask);
	bcastaddr.s_addr = (ethdevinfo.inetAddr & netmask) | (~netmask);

	if ((net_controlsocket = BW_OpenSocket (0)) == -1)
	{
		dos_freememory(lowmem_buffer);
		Con_DPrintf ("BW_Init unable to open control socket; disabled\n");
		return -1;
	}

	BW_GetSocketAddr (net_controlsocket, &addr);
	Q_strcpy(my_tcpip_address,  BW_AddrToString (&addr));
	colon = Q_strrchr (my_tcpip_address, ':');
	if (colon)
		*colon = 0;

	Con_Printf("BW_Init: UDP initialized\n");
	tcpipAvailable = true;

	return net_controlsocket;
}

//=============================================================================

void BW_Shutdown(void)
{
	BW_Listen (false);
	BW_CloseSocket (net_controlsocket);
	dos_freememory(lowmem_buffer);
}

//=============================================================================

void BW_Listen (qboolean state)
{
	// enable listening
	if (state)
	{
		if (net_acceptsocket != -1)
			return;
		if ((net_acceptsocket = BW_OpenSocket (net_hostport)) == -1)
			Sys_Error ("BW_Listen: Unable to open accept socket\n");
		return;
	}

	// disable listening
	if (net_acceptsocket == -1)
		return;
	BW_CloseSocket (net_acceptsocket);
	net_acceptsocket = -1;
}


//=============================================================================

/*
OpenSocket returns a handle to a network socket that has been opened,
set to nonblocking, and bound to <port>.  Additional socket options
should be set here if they are needed.  -1 is returned on failure.
*/

int BW_OpenSocket(int port)
{
	int s;
	int ret;
	int deadman = 3 * 1024;
	static int dynamic = 1024;
	static char reuse_msg[2] = {BW_IOCTL_SETOPTIONS, BW_OPTION_REUSEBUFFERS};
	static char bind_msg[3] = {BW_IOCTL_BIND, 0, 0};
	static char nonblock_msg[2] = {BW_IOCTL_CLEAROPTIONS, BW_OPTION_BLOCKING};

	// allocate a UDP socket
	Q_strcpy((char *)lowmem_buffer, "UDP-IP10");
	regs.x.ax = 0x3d42;
	regs.x.ds = lowmem_bufseg;
	regs.x.dx = lowmem_bufoff;
	if (dos_int86(0x21))
	{
		Con_Printf("BW_OpenSocket failed: %u\n", BW_TranslateError(regs.x.ax));
		return -1;
	}
	s = regs.x.ax;

	// set file descriptor to raw mode
	regs.x.ax = 0x4401;
	regs.x.bx = s;
	regs.x.dx = 0x60;
	dos_int86(0x21);

	if (BW_ioctl(s, reuse_msg, 2))
	{
		Con_Printf("BW_OpenSocket ioctl(reuse) failed\n");
		return -1;
	}

	if (BW_ioctl(s, nonblock_msg, 2))
	{
		Con_Printf("BW_OpenSocket ioctl(nonblocking) failed\n");
		return -1;
	}

	// if a socket was specified, bind to it and return
	if (port)
	{
		*(short *)&bind_msg[1] = port;
		if (BW_ioctl(s, bind_msg, 3))
		{
			BW_CloseSocket(s);
			return -1;
		}
		return s;
	}

	// B&W does NOT do dynamic allocation, so if port == 0 we must fake it
	do
	{
		port = dynamic++;
		if (dynamic == 4096)
			dynamic = 1024;
		deadman--;
		*(short *)&bind_msg[1] = port;
		ret = BW_ioctl(s, bind_msg, 3);
	}
	while (ret && deadman);
	if (ret)
		return -1;
	return s;
}

//=============================================================================

int BW_CloseSocket(int socket)
{
	regs.h.ah = 0x3e;
	regs.x.bx = socket;
	if(dos_int86(0x21))
		{
			Con_Printf("BW_CloseSocket %u failed: %u\n", socket, BW_TranslateError(regs.x.ax));
			return -1;
		}
	return 0;
}

//=============================================================================

int BW_Connect (int socket, struct qsockaddr *hostaddr)
{
	return 0;
}

//=============================================================================

int BW_CheckNewConnections(void)
{
	if (net_acceptsocket == 0)
		return -1;

	// see if there's anything waiting
	regs.x.ax = 0x4406;
	regs.x.bx = net_acceptsocket;
	dos_int86(0x21);
	if (regs.x.ax == 0)
		return -1;
	return net_acceptsocket;
}

//=============================================================================

int BW_Read(int s, byte *buf, int len, struct qsockaddr *from)
{
	BW_UDPreadInfo1_t *info1;
	BW_UDPreadInfo2_t *info2;

	// ask if there's anything waiting
	regs.x.ax = 0x4406;
	regs.x.bx = s;
	dos_int86(0x21);
	if (regs.x.ax == 0)
		return 0;

	// there was, so let's get it
	regs.h.ah = 0x3f;
	regs.x.cx = /* len + 53 */ LOWMEM_SIZE;
	regs.x.es = regs.x.ds = lowmem_bufseg;
	regs.x.dx = lowmem_bufoff;
	regs.x.bx = s;
	if (dos_int86(0x21))
	{
		Con_Printf("BW UDP read error: %u\n", BW_TranslateError(regs.x.ax));
		return -1;
	}

	info1 = (BW_UDPreadInfo1_t *)lowmem_buffer;
	info2 = (BW_UDPreadInfo2_t *)(lowmem_buffer + info1->info2Offset);

	if (from)
	{
		from->sa_family = AF_INET;
		((struct sockaddr_in *)from)->sin_addr = info1->remoteAddr;
		((struct sockaddr_in *)from)->sin_port = htons(info2->remotePort);
	}

	len = info2->dataLenPlus8 - 8;
	if (len > NET_DATAGRAMSIZE)
	{
		Con_Printf("BW UDP read packet too large: %u\n", len);
		return -1;
	}
	Q_memcpy(buf, info2->data, len);

	return len;
}

//=============================================================================

int BW_Broadcast(int s, byte *msg, int len)
{
	BW_writeInfo_t *writeInfo;

	// ask if we're clear to send
	regs.x.ax = 0x4407;
	regs.x.bx = s;
	dos_int86(0x21);
	if (regs.x.ax == 0)
		return 0;

	// yes, let's do it
	writeInfo = (BW_writeInfo_t *)lowmem_buffer;
	writeInfo->remoteAddr = bcastaddr;
	writeInfo->remotePort = net_hostport;
	writeInfo->dataLen = len;
	if (len > NET_DATAGRAMSIZE)
		Sys_Error("BW UDP write packet too large: %u\n", len);
	Q_memcpy(writeInfo->data, msg, len);
	writeInfo->data[len] = 0;
	regs.h.ah = 0x40;
	regs.x.bx = s;
	regs.x.cx = len + sizeof(BW_writeInfo_t);
	regs.x.es = regs.x.ds = lowmem_bufseg;
	regs.x.dx = lowmem_bufoff;
	if (dos_int86(0x21))
	{
		Con_Printf("BW_Broadcast failed: %u\n", BW_TranslateError(regs.x.ax));
		return -1;
	}

	return len;
}

//=============================================================================

int BW_Write(int s, byte *msg, int len, struct qsockaddr *to)
{
	BW_writeInfo_t *writeInfo;

	// ask if we're clear to send
	regs.x.ax = 0x4407;
	regs.x.bx = s;
	dos_int86(0x21);
	if (regs.x.ax == 0)
		return 0;

	// yes, let's do it
	writeInfo = (BW_writeInfo_t *)lowmem_buffer;
	writeInfo->remoteAddr = ((struct sockaddr_in *)to)->sin_addr;
	writeInfo->remotePort = ntohs(((struct sockaddr_in *)to)->sin_port);
	writeInfo->dataLen = len;
	if (len > NET_DATAGRAMSIZE)
		Sys_Error("BW UDP write packet too large: %u\n", len);
	Q_memcpy(writeInfo->data, msg, len);
	writeInfo->data[len] = 0;
	regs.h.ah = 0x40;
	regs.x.bx = s;
	regs.x.cx = len + sizeof(BW_writeInfo_t);
	regs.x.es = regs.x.ds = lowmem_bufseg;
	regs.x.dx = lowmem_bufoff;
	if (dos_int86(0x21))
	{
		Con_Printf("BW_Write failed: %u\n", BW_TranslateError(regs.x.ax));
		return -1;
	}

	return len;
}

//=============================================================================


char *BW_AddrToString (struct qsockaddr *addr)
{
	static char buffer[22];

	sprintf(buffer, "%d.%d.%d.%d:%d",
		((struct sockaddr_in *)addr)->sin_addr.s_net,
		((struct sockaddr_in *)addr)->sin_addr.s_host,
		((struct sockaddr_in *)addr)->sin_addr.s_lh,
		((struct sockaddr_in *)addr)->sin_addr.s_impno,
		ntohs(((struct sockaddr_in *)addr)->sin_port)
		);
	return buffer;
}

//=============================================================================

int BW_StringToAddr (char *string, struct qsockaddr *addr)
{
	int ha1, ha2, ha3, ha4, hp;
	int ipaddr;

	sscanf(string, "%d.%d.%d.%d:%d", &ha1, &ha2, &ha3, &ha4, &hp);
	ipaddr = (ha1 << 24) | (ha2 << 16) | (ha3 << 8) | ha4;

	addr->sa_family = AF_INET;
	((struct sockaddr_in *)addr)->sin_addr.s_addr = htonl(ipaddr);
	((struct sockaddr_in *)addr)->sin_port = htons((short)hp);
	return 0;
}

//=============================================================================

int BW_GetSocketAddr (int socket, struct qsockaddr *addr)
{
	regs.x.ax = 0x4402;
	regs.x.bx = socket;
	regs.x.cx = sizeof(BW_UDPinfo_t);
	regs.x.dx = lowmem_bufoff;
	regs.x.ds = lowmem_bufseg;
	dos_int86(0x21);

	addr->sa_family = AF_INET;
	((struct sockaddr_in *)addr)->sin_addr.s_addr = ((BW_UDPinfo_t *)lowmem_buffer)->localAddr.s_addr;
	((struct sockaddr_in *)addr)->sin_port = htons(((BW_UDPinfo_t *)lowmem_buffer)->localPort);

	return 0;
}

//=============================================================================

int BW_GetNameFromAddr (struct qsockaddr *addr, char *name)
{
	Q_strcpy(name, BW_AddrToString(addr));
	return 0;
}

///=============================================================================

int BW_GetAddrFromName (char *name, struct qsockaddr *hostaddr)
{
	char buff[MAXHOSTNAMELEN];
	char *b;
	int addr;
	int num;
	int mask;
	int run;
	int port;

	if (name[0] < '0' || name[0] > '9')
		return -1;

	buff[0] = '.';
	b = buff;
	Q_strcpy(buff+1, name);
	if (buff[1] == '.')
		b++;

	addr = 0;
	mask = -1;
	while (*b == '.')
	{
		b++;
		num = 0;
		run = 0;
		while (!( *b < '0' || *b > '9'))
		{
		  num = num*10 + *b++ - '0';
		  if (++run > 3)
		  	return -1;
		}
		if ((*b < '0' || *b > '9') && *b != '.' && *b != ':' && *b != 0)
			return -1;
		if (num < 0 || num > 255)
			return -1;
		mask<<=8;
		addr = (addr<<8) + num;
	}
	addr = htonl(addr);
	mask = htonl(mask);

	if (*b++ == ':')
		port = Q_atoi(b);
	else
		port = net_hostport;

	hostaddr->sa_family = AF_INET;
	((struct sockaddr_in *)hostaddr)->sin_port = htons((short)port);	
	((struct sockaddr_in *)hostaddr)->sin_addr.s_addr =
	((ethdevinfo.inetAddr & mask) | addr);

	return 0;
}

//=============================================================================

int BW_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2)
{
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if (((struct sockaddr_in *)addr1)->sin_addr.s_addr != ((struct sockaddr_in *)addr2)->sin_addr.s_addr)
		return -1;

	if (((struct sockaddr_in *)addr1)->sin_port != ((struct sockaddr_in *)addr2)->sin_port)
		return 1;

	return 0;
}

//=============================================================================

int BW_GetSocketPort (struct qsockaddr *addr)
{
	return ntohs(((struct sockaddr_in *)addr)->sin_port);
}


int BW_SetSocketPort (struct qsockaddr *addr, int port)
{
	((struct sockaddr_in *)addr)->sin_port = htons(port);
	return 0;
}

//=============================================================================
