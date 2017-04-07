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
// net_mpath.h

int  MPATH_Init (void);
void MPATH_Shutdown (void);
void MPATH_Listen (qboolean state);
int  MPATH_OpenSocket (int port);
int  MPATH_CloseSocket (int socket);
int  MPATH_Connect (int socket, struct qsockaddr *addr);
int  MPATH_CheckNewConnections (void);
int  MPATH_Read (int socket, byte *buf, int len, struct qsockaddr *addr);
int  MPATH_Write (int socket, byte *buf, int len, struct qsockaddr *addr);
int  MPATH_Broadcast (int socket, byte *buf, int len);
char *MPATH_AddrToString (struct qsockaddr *addr);
int  MPATH_StringToAddr (char *string, struct qsockaddr *addr);
int  MPATH_GetSocketAddr (int socket, struct qsockaddr *addr);
int  MPATH_GetNameFromAddr (struct qsockaddr *addr, char *name);
int  MPATH_GetAddrFromName (char *name, struct qsockaddr *addr);
int  MPATH_AddrCompare (struct qsockaddr *addr1, struct qsockaddr *addr2);
int  MPATH_GetSocketPort (struct qsockaddr *addr);
int  MPATH_SetSocketPort (struct qsockaddr *addr, int port);
