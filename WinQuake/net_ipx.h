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
// net_ipx.h

int  IPX_Init (void);
void IPX_Shutdown (void);
void IPX_Listen (qboolean state);
int  IPX_OpenSocket (int port);
int  IPX_CloseSocket (int socket);
int  IPX_Connect (int socket, struct qsockaddr *addr);
int  IPX_CheckNewConnections (void);
int  IPX_Read (int socket, byte *buf, int len, struct qsockaddr *addr);
int  IPX_Write (int socket, byte *buf, int len, struct qsockaddr *addr);
int  IPX_Broadcast (int socket, byte *buf, int len);
char *IPX_AddrToString (struct qsockaddr *addr);
int  IPX_StringToAddr (char *string, struct qsockaddr *addr);
int  IPX_GetSocketAddr (int socket, struct qsockaddr *addr);
int  IPX_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  IPX_GetAddrFromName (char *name, struct qsockaddr *addr);
int  IPX_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  IPX_GetSocketPort (struct qsockaddr *addr);
int  IPX_SetSocketPort (struct qsockaddr *addr, int port);
