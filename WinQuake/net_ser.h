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
// net_ser.h

int			Serial_Init (void);
void		Serial_Listen (qboolean state);
void		Serial_SearchForHosts (qboolean xmit);
qsocket_t	*Serial_Connect (char *host);
qsocket_t 	*Serial_CheckNewConnections (void);
int			Serial_GetMessage (qsocket_t *sock);
int			Serial_SendMessage (qsocket_t *sock, sizebuf_t *data);
int			Serial_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data);
qboolean	Serial_CanSendMessage (qsocket_t *sock);
qboolean	Serial_CanSendUnreliableMessage (qsocket_t *sock);
void		Serial_Close (qsocket_t *sock);
void		Serial_Shutdown (void);
