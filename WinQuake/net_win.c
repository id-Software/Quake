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
#include "quakedef.h"

#include "net_loop.h"
#include "net_dgrm.h"
#include "net_ser.h"

net_driver_t net_drivers[MAX_NET_DRIVERS] =
{
	{
	"Loopback",
	false,
	Loop_Init,
	Loop_Listen,
	Loop_SearchForHosts,
	Loop_Connect,
	Loop_CheckNewConnections,
	Loop_GetMessage,
	Loop_SendMessage,
	Loop_SendUnreliableMessage,
	Loop_CanSendMessage,
	Loop_CanSendUnreliableMessage,
	Loop_Close,
	Loop_Shutdown
	}
	,
	{
	"Datagram",
	false,
	Datagram_Init,
	Datagram_Listen,
	Datagram_SearchForHosts,
	Datagram_Connect,
	Datagram_CheckNewConnections,
	Datagram_GetMessage,
	Datagram_SendMessage,
	Datagram_SendUnreliableMessage,
	Datagram_CanSendMessage,
	Datagram_CanSendUnreliableMessage,
	Datagram_Close,
	Datagram_Shutdown
	}
};

int net_numdrivers = 2;


#include "net_wins.h"
#include "net_wipx.h"

net_landriver_t	net_landrivers[MAX_NET_DRIVERS] =
{
	{
	"Winsock TCPIP",
	false,
	0,
	WINS_Init,
	WINS_Shutdown,
	WINS_Listen,
	WINS_OpenSocket,
	WINS_CloseSocket,
	WINS_Connect,
	WINS_CheckNewConnections,
	WINS_Read,
	WINS_Write,
	WINS_Broadcast,
	WINS_AddrToString,
	WINS_StringToAddr,
	WINS_GetSocketAddr,
	WINS_GetNameFromAddr,
	WINS_GetAddrFromName,
	WINS_AddrCompare,
	WINS_GetSocketPort,
	WINS_SetSocketPort
	},
	{
	"Winsock IPX",
	false,
	0,
	WIPX_Init,
	WIPX_Shutdown,
	WIPX_Listen,
	WIPX_OpenSocket,
	WIPX_CloseSocket,
	WIPX_Connect,
	WIPX_CheckNewConnections,
	WIPX_Read,
	WIPX_Write,
	WIPX_Broadcast,
	WIPX_AddrToString,
	WIPX_StringToAddr,
	WIPX_GetSocketAddr,
	WIPX_GetNameFromAddr,
	WIPX_GetAddrFromName,
	WIPX_AddrCompare,
	WIPX_GetSocketPort,
	WIPX_SetSocketPort
	}

};

int net_numlandrivers = 2;
