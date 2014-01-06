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
// net_ipx.c

#include <stdio.h>
#include <stdlib.h>
#include <dpmi.h>

#include "quakedef.h"
#include "dosisms.h"
#include "net_ipx.h"

#define	EIO		 		5	/* I/O error */

#define AF_NETWARE 		64

#define IPX_OPEN					0
#define IPX_CLOSE					1
#define IPX_GETROUTE				2
#define IPX_SEND					3
#define IPX_LISTEN					4
#define IPX_SCHEDULEEVENT			5
#define IPX_CANCEL					6
#define IPX_SCHEDULESPECIALEVENT	7
#define IPX_GETINTERVALMARKER		8
#define IPX_GETADDRESS				9
#define IPX_RELINQUISH				10

#define PTYPE_UNKNOWN				0
#define PTYPE_RIP					1
#define PTYPE_ECHO					2
#define PTYPE_ERROR					3
#define PTYPE_IPX					4
#define PTYPE_SPX					5

#pragma pack(1)

typedef struct
{
	byte	network[4];
	byte	node[6];
	short	socket;
} IPXaddr;

struct sockaddr_ipx
{
    short			sipx_family;
	IPXaddr			sipx_addr;
    char			sipx_zero[2];
};
#define sipx_port sipx_addr.socket

typedef struct
{
	short			checkSum;
	short			length;
	byte			transportControl;
	byte			type;
	IPXaddr			destination;
	IPXaddr			source;
} IPXheader;

typedef struct ECBStructure
{
	struct ECBStructure *link;
	unsigned short	ESR_off;
	unsigned short	ESR_seg;
	byte	inUse;
	byte	completionCode;
	short	socket;
	byte	IPXWorkspace[4];
	byte	driverWorkspace[12];
	byte	immediateAddress[6];
	short	fragCount;
	short	fragOff;
	short	fragSeg;
	short	fragSize;
} ECB;

#pragma pack()

typedef struct
{
	ECB			ecb;
	IPXheader	header;
	int			sequence;
	char		data[NET_DATAGRAMSIZE];
} ipx_lowmem_buffer_t;

#define LOWMEMSIZE		(100 * 1024)
#define LOWMEMSAVE		256
#define IPXBUFFERS		((LOWMEMSIZE - LOWMEMSAVE)/ sizeof(ipx_lowmem_buffer_t))
#define IPXSOCKBUFFERS	5
#define IPXSOCKETS		(IPXBUFFERS / IPXSOCKBUFFERS)

// each socket's socketbuffer 0 is used for sending, the others for listening

typedef struct
{
	char				reserved[LOWMEMSAVE];
	ipx_lowmem_buffer_t	socketbuffer[IPXSOCKETS][IPXSOCKBUFFERS];
} ipx_lowmem_area_t;


static int ipxsocket[IPXSOCKETS];
static ECB *readlist[IPXSOCKETS];
static int sequence[IPXSOCKETS];
static int handlesInUse;
static ipx_lowmem_area_t *lma;
static char *lowmem_buffer;
static int lowmem_bufseg;
static int lowmem_bufoff;
static unsigned short ipx_cs;
static unsigned short ipx_ip;
static int net_acceptsocket = -1;
static int net_controlsocket;

static void IPX_PollProcedure(void);
static PollProcedure pollProcedure = {NULL, 0.0, IPX_PollProcedure};

//=============================================================================

static void IPX_GetLocalAddress(IPXaddr *addr)
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_GETADDRESS;
	regs.x.es = lowmem_bufseg;
	regs.x.si = lowmem_bufoff;
	__dpmi_simulate_real_mode_procedure_retf((__dpmi_regs *)&regs);
	Q_memcpy(addr, lowmem_buffer, 10);
}

//=============================================================================

static int IPX_GetLocalTarget(IPXaddr *addr, byte *localTarget)
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_GETROUTE;
	regs.x.es = lowmem_bufseg;
	regs.x.si = lowmem_bufoff;
	regs.x.di = lowmem_bufoff + sizeof(IPXaddr);
	Q_memcpy(lowmem_buffer, addr, sizeof(IPXaddr));
	__dpmi_simulate_real_mode_procedure_retf((__dpmi_regs *)&regs);
	if (regs.h.al)
		return -1;
	Q_memcpy(localTarget, lowmem_buffer + sizeof(IPXaddr), 6);
	return 0;
}

//=============================================================================

static void IPX_ListenForPacket(ECB *ecb)
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_LISTEN;
	regs.x.es = ptr2real(ecb) >> 4;
	regs.x.si = ptr2real(ecb) & 0xf;
	__dpmi_simulate_real_mode_procedure_retf((__dpmi_regs *)&regs);
}

//=============================================================================

static void IPX_RelinquishControl(void)
{
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_RELINQUISH;
	__dpmi_simulate_real_mode_procedure_retf((__dpmi_regs *)&regs);
}


void IPX_PollProcedure(void)
{
	IPX_RelinquishControl();
	SchedulePollProcedure(&pollProcedure, 0.01);
}

//=============================================================================

static void ProcessReadyList(int s)
{
	int n;
	ECB *ecb;
	ECB *prev;

	for (n = 1; n < IPXSOCKBUFFERS; n++)
	{
		if (lma->socketbuffer[s][n].ecb.inUse == 0)
		{
			for (ecb = readlist[s], prev = NULL; ecb; ecb = ecb->link)
			{
				if (lma->socketbuffer[s][n].sequence < ((ipx_lowmem_buffer_t *) ecb)->sequence)
					break;
				prev = ecb;
			}
			if (ecb)
				lma->socketbuffer[s][n].ecb.link = ecb;
			else
				lma->socketbuffer[s][n].ecb.link = NULL;
			if (prev)
				prev->link = &lma->socketbuffer[s][n].ecb;
			else
				readlist[s] = &lma->socketbuffer[s][n].ecb;
			lma->socketbuffer[s][n].ecb.inUse = 0xff;
		}
	}
}

//=============================================================================

int IPX_Init(void)
{
	int s;
	int n;
	struct qsockaddr addr;
	char *colon;

	if (COM_CheckParm ("-noipx"))
		return -1;

	// find the IPX far call entry point
	regs.x.ax = 0x7a00;
	__dpmi_simulate_real_mode_interrupt (0x2f, (__dpmi_regs *)&regs);
	if (regs.h.al != 0xff)
	{
		Con_Printf("IPX not detected\n");
		return -1;
	}
	ipx_cs = regs.x.es;
	ipx_ip = regs.x.di;

	// grab a chunk of memory down in DOS land
	lowmem_buffer = dos_getmemory(LOWMEMSIZE);
	if (!lowmem_buffer)
	{
		Con_Printf("IPX_Init: Not enough low memory\n");
		return -1;
	}
	lowmem_bufoff = ptr2real(lowmem_buffer) & 0xf;
	lowmem_bufseg = ptr2real(lowmem_buffer) >> 4;

	// init socket handles & buffers
	handlesInUse = 0;
	lma = (ipx_lowmem_area_t *)lowmem_buffer;
	for (s = 0; s < IPXSOCKETS; s++)
	{
		ipxsocket[s] = 0;
		for (n = 0; n < IPXSOCKBUFFERS; n++)
		{
			lma->socketbuffer[s][n].ecb.link = NULL;
			lma->socketbuffer[s][n].ecb.ESR_off = 0;
			lma->socketbuffer[s][n].ecb.ESR_seg = 0;
			lma->socketbuffer[s][n].ecb.socket = 0;
			lma->socketbuffer[s][n].ecb.inUse = 0xff;
			lma->socketbuffer[s][n].ecb.completionCode = 0;
			lma->socketbuffer[s][n].ecb.fragCount = 1;
			lma->socketbuffer[s][n].ecb.fragOff = ptr2real(&lma->socketbuffer[s][n].header) & 0xf;
			lma->socketbuffer[s][n].ecb.fragSeg = ptr2real(&lma->socketbuffer[s][n].header) >> 4;
			lma->socketbuffer[s][n].ecb.fragSize = sizeof(IPXheader) + sizeof(int) + NET_DATAGRAMSIZE;
		}
	}

	if ((net_controlsocket = IPX_OpenSocket (0)) == -1)
	{
		dos_freememory(lowmem_buffer);
		Con_DPrintf ("IPX_Init: Unable to open control socket\n");
		return -1;
	}

	SchedulePollProcedure(&pollProcedure, 0.01);

	IPX_GetSocketAddr (net_controlsocket, &addr);
	Q_strcpy(my_ipx_address,  IPX_AddrToString (&addr));
	colon = Q_strrchr (my_ipx_address, ':');
	if (colon)
		*colon = 0;

	Con_Printf("IPX initialized\n");
	ipxAvailable = true;
	return net_controlsocket;
}

//=============================================================================

void IPX_Shutdown(void)
{
	IPX_Listen (false);
	IPX_CloseSocket (net_controlsocket);
	dos_freememory(lowmem_buffer);
}

//=============================================================================

void IPX_Listen (qboolean state)
{
	// enable listening
	if (state)
	{
		if (net_acceptsocket != -1)
			return;
		if ((net_acceptsocket = IPX_OpenSocket (net_hostport)) == -1)
			Sys_Error ("IPX_Listen: Unable to open accept socket\n");
		return;
	}

	// disable listening
	if (net_acceptsocket == -1)
		return;
	IPX_CloseSocket (net_acceptsocket);
	net_acceptsocket = -1;
}

//=============================================================================

int IPX_OpenSocket(int port)
{
	int handle;
	int n;
	unsigned short socket;

	if (handlesInUse == IPXSOCKETS)
		return -1;

	// open the IPX socket
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_OPEN;
	regs.h.al = 0;
	regs.x.dx = htons(port);
	__dpmi_simulate_real_mode_procedure_retf((__dpmi_regs *)&regs);
	if (regs.h.al == 0xfe)
	{
		Con_DPrintf("IPX_OpenSocket: all sockets in use\n");
		return -1;
	}
	if (regs.h.al == 0xff)
	{
		Con_DPrintf("IPX_OpenSocket: socket already open\n");
		return -1;
	}
	if (regs.h.al != 0)
	{
		Con_DPrintf("IPX_OpenSocket: error %02x\n", regs.h.al);
		return -1;
	}
	socket = regs.x.dx;

// grab a handle; fill in the ECBs, and get them listening
	for (handle = 0; handle < IPXSOCKETS; handle++)
	{
		if (ipxsocket[handle] == 0)
		{
			ipxsocket[handle] = socket;
			readlist[handle] = NULL;
			sequence[handle] = 0;
			for (n = 0; n < IPXSOCKBUFFERS; n ++)
			{
				lma->socketbuffer[handle][n].ecb.socket = socket;
				lma->socketbuffer[handle][n].ecb.inUse = 0;
				if (n)
					IPX_ListenForPacket(&lma->socketbuffer[handle][n].ecb);
			}
			handlesInUse++;
			return handle;
		}
	}

	// "this will NEVER happen"
	Sys_Error("IPX_OpenSocket: handle allocation failed\n");
	return -1;
}

//=============================================================================

int IPX_CloseSocket(int handle)
{
	// if there's a send in progress, give it one last chance
	if (lma->socketbuffer[handle][0].ecb.inUse != 0)
		IPX_RelinquishControl();

	// close the socket (all pending sends/received are cancelled)
	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_CLOSE;
	regs.x.dx = ipxsocket[handle];
	__dpmi_simulate_real_mode_procedure_retf((__dpmi_regs *)&regs);

	ipxsocket[handle] = 0;
	handlesInUse--;

	return 0;
}

//=============================================================================

int IPX_Connect (int handle, struct qsockaddr *addr)
{
	IPXaddr ipxaddr;

	Q_memcpy(&ipxaddr, &((struct sockaddr_ipx *)addr)->sipx_addr, sizeof(IPXaddr));
	if (IPX_GetLocalTarget(&ipxaddr, lma->socketbuffer[handle][0].ecb.immediateAddress) != 0)
	{
		Con_Printf("Get Local Target failed\n");
		return -1;
	}

	return 0;
}

//=============================================================================

int IPX_CheckNewConnections (void)
{
	int n;

	if (net_acceptsocket == -1)
		return -1;

	for (n = 1; n < IPXSOCKBUFFERS; n ++)
		if (lma->socketbuffer[net_acceptsocket][n].ecb.inUse == 0)
			return net_acceptsocket;
	return -1;
}

//=============================================================================

int IPX_Read (int handle, byte *buf, int len, struct qsockaddr *addr)
{
	ECB		*ecb;
	ipx_lowmem_buffer_t *rcvbuf;
	int		copylen;

	ProcessReadyList(handle);
tryagain:
	if (readlist[handle] == NULL)
		return 0;
	ecb = readlist[handle];
	readlist[handle] = ecb->link;

	if (ecb->completionCode != 0)
	{
		Con_Printf("Warning: IPX_Read error %02x\n", ecb->completionCode);	
		ecb->fragSize = sizeof(IPXheader) + sizeof(int) + NET_DATAGRAMSIZE;
		IPX_ListenForPacket(ecb);
		goto tryagain;
	}

	rcvbuf = (ipx_lowmem_buffer_t *)ecb;

	// copy the data up to the buffer
	copylen = ntohs(rcvbuf->header.length) - (sizeof(int) + sizeof(IPXheader));
	if (len < copylen)
		Sys_Error("IPX_Read: buffer too small (%d vs %d)\n", len, copylen);
	Q_memcpy(buf, rcvbuf->data, copylen);

	// fill in the addr if they want it
	if (addr)
	{
		((struct sockaddr_ipx *)addr)->sipx_family = AF_NETWARE;
		Q_memcpy(&((struct sockaddr_ipx *)addr)->sipx_addr, rcvbuf->header.source.network, sizeof(IPXaddr));
		((struct sockaddr_ipx *)addr)->sipx_zero[0] = 0;
		((struct sockaddr_ipx *)addr)->sipx_zero[1] = 0;
	}

	// update the send ecb's immediate address
	Q_memcpy(lma->socketbuffer[handle][0].ecb.immediateAddress, rcvbuf->ecb.immediateAddress, 6);

	// get this ecb listening again
	rcvbuf->ecb.fragSize = sizeof(IPXheader) + sizeof(int) + NET_DATAGRAMSIZE;
	IPX_ListenForPacket(&rcvbuf->ecb);
	return copylen;
}

//=============================================================================

int IPX_Broadcast (int handle, byte *buf, int len)
{
	struct sockaddr_ipx addr;
	int ret;

	Q_memset(addr.sipx_addr.network, 0x00, 4);
	Q_memset(addr.sipx_addr.node, 0xff, 6);
	addr.sipx_port = htons(net_hostport);
	Q_memset(lma->socketbuffer[handle][0].ecb.immediateAddress, 0xff, 6);
	ret = IPX_Write (handle, buf, len, (struct qsockaddr *)&addr);
	return ret;
}

//=============================================================================

int IPX_Write (int handle, byte *buf, int len, struct qsockaddr *addr)
{
	// has the previous send completed?
	while (lma->socketbuffer[handle][0].ecb.inUse != 0)
		IPX_RelinquishControl();

	switch (lma->socketbuffer[handle][0].ecb.completionCode)
	{
		case 0x00: // success
		case 0xfc: // request cancelled
			break;

		case 0xfd: // malformed packet
		default:
			Con_Printf("IPX driver send failure: %02x\n", lma->socketbuffer[handle][0].ecb.completionCode);
			break;

		case 0xfe: // packet undeliverable
		case 0xff: // unable to send packet
			Con_Printf("IPX lost route, trying to re-establish\n");

			// look for a new route
			if (IPX_GetLocalTarget (&lma->socketbuffer[handle][0].header.destination, lma->socketbuffer[handle][0].ecb.immediateAddress) != 0)
				return -1;

			// re-send the one that failed
			regs.x.cs = ipx_cs;
			regs.x.ip = ipx_ip;
			regs.x.bx = IPX_SEND;
			regs.x.es = ptr2real(&lma->socketbuffer[handle][0].ecb) >> 4;
			regs.x.si = ptr2real(&lma->socketbuffer[handle][0].ecb) & 0xf;
			__dpmi_simulate_real_mode_procedure_retf((__dpmi_regs *)&regs);

			// report that we did not send the current one
			return 0;
	}

	// ecb : length
	lma->socketbuffer[handle][0].ecb.fragSize = sizeof(IPXheader) + sizeof(int) + len;

	// ipx header : type
	lma->socketbuffer[handle][0].header.type = PTYPE_IPX;

	// ipx header : destination
	Q_memcpy(&lma->socketbuffer[handle][0].header.destination, &((struct sockaddr_ipx *)addr)->sipx_addr, sizeof(IPXaddr));

	// sequence number
	lma->socketbuffer[handle][0].sequence = sequence[handle];
	sequence[handle]++;

	// copy down the data
	Q_memcpy(lma->socketbuffer[handle][0].data, buf, len);

	regs.x.cs = ipx_cs;
	regs.x.ip = ipx_ip;
	regs.x.bx = IPX_SEND;
	regs.x.es = ptr2real(&lma->socketbuffer[handle][0].ecb) >> 4;
	regs.x.si = ptr2real(&lma->socketbuffer[handle][0].ecb) & 0xf;
	__dpmi_simulate_real_mode_procedure_retf((__dpmi_regs *)&regs);

	return len;
}

//=============================================================================

char *IPX_AddrToString (struct qsockaddr *addr)
{
	static char buf[28];

	sprintf(buf, "%02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%u",
		((struct sockaddr_ipx *)addr)->sipx_addr.network[0],
		((struct sockaddr_ipx *)addr)->sipx_addr.network[1],
		((struct sockaddr_ipx *)addr)->sipx_addr.network[2],
		((struct sockaddr_ipx *)addr)->sipx_addr.network[3],
		((struct sockaddr_ipx *)addr)->sipx_addr.node[0],
		((struct sockaddr_ipx *)addr)->sipx_addr.node[1],
		((struct sockaddr_ipx *)addr)->sipx_addr.node[2],
		((struct sockaddr_ipx *)addr)->sipx_addr.node[3],
		((struct sockaddr_ipx *)addr)->sipx_addr.node[4],
		((struct sockaddr_ipx *)addr)->sipx_addr.node[5],
		ntohs(((struct sockaddr_ipx *)addr)->sipx_port)
		);
	return buf;
}

//=============================================================================

int IPX_StringToAddr (char *string, struct qsockaddr *addr)
{
	int  val;
	char buf[3];

	buf[2] = 0;
	Q_memset(addr, 0, sizeof(struct qsockaddr));
	addr->sa_family = AF_NETWARE;

#define DO(src,dest)	\
	buf[0] = string[src];	\
	buf[1] = string[src + 1];	\
	if (sscanf (buf, "%x", &val) != 1)	\
		return -1;	\
	((struct sockaddr_ipx *)addr)->sipx_addr.dest = val

	DO(0, network[0]);
	DO(2, network[1]);
	DO(4, network[2]);
	DO(6, network[3]);
	DO(9, node[0]);
	DO(11, node[1]);
	DO(13, node[2]);
	DO(15, node[3]);
	DO(17, node[4]);
	DO(19, node[5]);
#undef DO

	sscanf (&string[22], "%u", &val);
	((struct sockaddr_ipx *)addr)->sipx_port = htons(val);

	return 0;
}

//=============================================================================

int IPX_GetSocketAddr (int handle, struct qsockaddr *addr)
{
	Q_memset(addr, 0, sizeof(struct qsockaddr));
	addr->sa_family = AF_NETWARE;
	IPX_GetLocalAddress(&((struct sockaddr_ipx *)addr)->sipx_addr);
	((struct sockaddr_ipx *)addr)->sipx_port = ipxsocket[handle];
	return 0;
}

//=============================================================================

int IPX_GetNameFromAddr (struct qsockaddr *addr, char *name)
{
	Q_strcpy(name, IPX_AddrToString(addr));
	return 0;
}

//=============================================================================

int IPX_GetAddrFromName (char *name, struct qsockaddr *addr)
{
	int n;
	char buf[32];

	n = Q_strlen(name);

	if (n == 12)
	{
		sprintf(buf, "00000000:%s:%u", name, net_hostport);
		return IPX_StringToAddr (buf, addr);
	}
	if (n == 21)
	{
		sprintf(buf, "%s:%u", name, net_hostport);
		return IPX_StringToAddr (buf, addr);
	}
	if (n > 21 && n <= 27)
		return IPX_StringToAddr (name, addr);

	return -1;
}

//=============================================================================

int IPX_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2)
{
	if (addr1->sa_family != addr2->sa_family)
		return -1;

	if(Q_memcmp(&((struct sockaddr_ipx *)addr1)->sipx_addr, &((struct sockaddr_ipx *)addr2)->sipx_addr, 10))
		return -1;

	if (((struct sockaddr_ipx *)addr1)->sipx_port != ((struct sockaddr_ipx *)addr2)->sipx_port)
		return 1;

	return 0;
}

//=============================================================================

int IPX_GetSocketPort (struct qsockaddr *addr)
{
	return ntohs(((struct sockaddr_ipx *)addr)->sipx_port);
}


int IPX_SetSocketPort (struct qsockaddr *addr, int port)
{
	((struct sockaddr_ipx *)addr)->sipx_port = htons(port);
	return 0;
}

//=============================================================================
