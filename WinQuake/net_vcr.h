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
// net_vcr.h

#define VCR_OP_CONNECT					1
#define VCR_OP_GETMESSAGE				2
#define VCR_OP_SENDMESSAGE				3
#define VCR_OP_CANSENDMESSAGE			4
#define VCR_MAX_MESSAGE					4

int			VCR_Init (void);
void		VCR_Listen (qboolean state);
void		VCR_SearchForHosts (qboolean xmit);
qsocket_t 	*VCR_Connect (char *host);
qsocket_t 	*VCR_CheckNewConnections (void);
int			VCR_GetMessage (qsocket_t *sock);
int			VCR_SendMessage (qsocket_t *sock, sizebuf_t *data);
qboolean	VCR_CanSendMessage (qsocket_t *sock);
void		VCR_Close (qsocket_t *sock);
void		VCR_Shutdown (void);
