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
	,
	{
	"Serial",
	false,
	Serial_Init,
	Serial_Listen,
	Serial_SearchForHosts,
	Serial_Connect,
	Serial_CheckNewConnections,
	Serial_GetMessage,
	Serial_SendMessage,
	Serial_SendUnreliableMessage,
	Serial_CanSendMessage,
	Serial_CanSendUnreliableMessage,
	Serial_Close,
	Serial_Shutdown
	}
};

int net_numdrivers = 3;


#include "net_bw.h"
#include "net_ipx.h"
#include "net_mp.h"

net_landriver_t	net_landrivers[MAX_NET_DRIVERS] =
{
	{
	"Beame & Whiteside TCP/IP",
	false,
	0,
	BW_Init,
	BW_Shutdown,
	BW_Listen,
	BW_OpenSocket,
	BW_CloseSocket,
	BW_Connect,
	BW_CheckNewConnections,
	BW_Read,
	BW_Write,
	BW_Broadcast,
	BW_AddrToString,
	BW_StringToAddr,
	BW_GetSocketAddr,
	BW_GetNameFromAddr,
	BW_GetAddrFromName,
	BW_AddrCompare,
	BW_GetSocketPort,
	BW_SetSocketPort
	}
,
	{
	"IPX",
	false,
	0,
	IPX_Init,
	IPX_Shutdown,
	IPX_Listen,
	IPX_OpenSocket,
	IPX_CloseSocket,
	IPX_Connect,
	IPX_CheckNewConnections,
	IPX_Read,
	IPX_Write,
	IPX_Broadcast,
	IPX_AddrToString,
	IPX_StringToAddr,
	IPX_GetSocketAddr,
	IPX_GetNameFromAddr,
	IPX_GetAddrFromName,
	IPX_AddrCompare,
	IPX_GetSocketPort,
	IPX_SetSocketPort
	}
,
	{
	"Win95 TCP/IP",
	false,
	0,
	MPATH_Init,
	MPATH_Shutdown,
	MPATH_Listen,
	MPATH_OpenSocket,
	MPATH_CloseSocket,
	MPATH_Connect,
	MPATH_CheckNewConnections,
	MPATH_Read,
	MPATH_Write,
	MPATH_Broadcast,
	MPATH_AddrToString,
	MPATH_StringToAddr,
	MPATH_GetSocketAddr,
	MPATH_GetNameFromAddr,
	MPATH_GetAddrFromName,
	MPATH_AddrCompare,
	MPATH_GetSocketPort,
	MPATH_SetSocketPort
	}
};

int net_numlandrivers = 3;
