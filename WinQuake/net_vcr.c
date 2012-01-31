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
// net_vcr.c

#include "quakedef.h"
#include "net_vcr.h"

extern int vcrFile;

// This is the playback portion of the VCR.  It reads the file produced
// by the recorder and plays it back to the host.  The recording contains
// everything necessary (events, timestamps, and data) to duplicate the game
// from the viewpoint of everything above the network layer.

static struct
{
	double	time;
	int		op;
	long	session;
}	next;

int VCR_Init (void)
{
	net_drivers[0].Init = VCR_Init;

	net_drivers[0].SearchForHosts = VCR_SearchForHosts;
	net_drivers[0].Connect = VCR_Connect;
	net_drivers[0].CheckNewConnections = VCR_CheckNewConnections;
	net_drivers[0].QGetMessage = VCR_GetMessage;
	net_drivers[0].QSendMessage = VCR_SendMessage;
	net_drivers[0].CanSendMessage = VCR_CanSendMessage;
	net_drivers[0].Close = VCR_Close;
	net_drivers[0].Shutdown = VCR_Shutdown;

	Sys_FileRead(vcrFile, &next, sizeof(next));
	return 0;
}

void VCR_ReadNext (void)
{
	if (Sys_FileRead(vcrFile, &next, sizeof(next)) == 0)
	{
		next.op = 255;
		Sys_Error ("=== END OF PLAYBACK===\n");
	}
	if (next.op < 1 || next.op > VCR_MAX_MESSAGE)
		Sys_Error ("VCR_ReadNext: bad op");
}


void VCR_Listen (qboolean state)
{
}


void VCR_Shutdown (void)
{
}


int VCR_GetMessage (qsocket_t *sock)
{
	int	ret;
	
	if (host_time != next.time || next.op != VCR_OP_GETMESSAGE || next.session != *(long *)(&sock->driverdata))
		Sys_Error ("VCR missmatch");

	Sys_FileRead(vcrFile, &ret, sizeof(int));
	if (ret != 1)
	{
		VCR_ReadNext ();
		return ret;
	}

	Sys_FileRead(vcrFile, &net_message.cursize, sizeof(int));
	Sys_FileRead(vcrFile, net_message.data, net_message.cursize);

	VCR_ReadNext ();

	return 1;
}


int VCR_SendMessage (qsocket_t *sock, sizebuf_t *data)
{
	int	ret;

	if (host_time != next.time || next.op != VCR_OP_SENDMESSAGE || next.session != *(long *)(&sock->driverdata))
		Sys_Error ("VCR missmatch");

	Sys_FileRead(vcrFile, &ret, sizeof(int));

	VCR_ReadNext ();

	return ret;
}


qboolean VCR_CanSendMessage (qsocket_t *sock)
{
	qboolean	ret;

	if (host_time != next.time || next.op != VCR_OP_CANSENDMESSAGE || next.session != *(long *)(&sock->driverdata))
		Sys_Error ("VCR missmatch");

	Sys_FileRead(vcrFile, &ret, sizeof(int));

	VCR_ReadNext ();

	return ret;
}


void VCR_Close (qsocket_t *sock)
{
}


void VCR_SearchForHosts (qboolean xmit)
{
}


qsocket_t *VCR_Connect (char *host)
{
	return NULL;
}


qsocket_t *VCR_CheckNewConnections (void)
{
	qsocket_t	*sock;

	if (host_time != next.time || next.op != VCR_OP_CONNECT)
		Sys_Error ("VCR missmatch");

	if (!next.session)
	{
		VCR_ReadNext ();
		return NULL;
	}

	sock = NET_NewQSocket ();
	*(long *)(&sock->driverdata) = next.session;

	Sys_FileRead (vcrFile, sock->address, NET_NAMELEN);
	VCR_ReadNext ();

	return sock;
}
