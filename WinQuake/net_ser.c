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
// net_ser.c

#include "quakedef.h"
#include "net_ser.h"
#include "dosisms.h"
#include "crc.h"

#include "net_comx.c"

// serial protocol

#define SERIAL_PROTOCOL_VERSION 3

// The serial protocol is message oriented.  The high level message format is
// a one byte message type (MTYPE_xxx), data, and a 16-bit checksum.  All
// multi-byte fields are sent in network byte order.  There are currently 4
// MTYPEs defined.  Their formats are as follows:
//
// MTYPE_RELIABLE     sequence      data_length   data       checksum   eom
// MTYPE_UNRELIABLE   sequence      data_length   data       checksum   eom
// MTYPE_ACK          sequence      checksum      eom
// MTYPE_CONTROL      data_length   data          checksum   eom
//
// sequence is an 8-bit unsigned value starting from 0
// data_length is a 16-bit unsigned value; it is the length of the data only
// the checksum is a 16-bit value.  the CRC formula used is defined in crc.h.
//              the checksum covers the entire messages, excluding itself
// eom is a special 2 byte sequence used to mark the End Of Message.  This is
//              needed for error recovery.
//
// A lot of behavior is based on knowledge of the upper level Quake network
// layer.  For example, only one reliable message can be outstanding (pending
// reception of an MTYPE_ACK) at a time.
//
// The low level routines used to communicate with the modem are not part of
// this protocol.
//
// The CONTROL messages are only used for session establishment.  They are
// not reliable or sequenced.

#define MTYPE_RELIABLE			0x01
#define MTYPE_UNRELIABLE		0x02
#define MTYPE_CONTROL			0x03
#define MTYPE_ACK				0x04
#define MTYPE_CLIENT			0x80

#define ESCAPE_COMMAND			0xe0
#define ESCAPE_EOM				0x19

static qboolean listening = false;


typedef struct SerialLine_s
{
	struct SerialLine_s	*next;
	qsocket_t			*sock;
	int					lengthStated;
	int					lengthFound;
	int					tty;
	qboolean			connected;
	qboolean			connecting;
	qboolean			client;
	double				connect_time;
	unsigned short		crcStated;
	unsigned short		crcValue;
	byte				currState;
	byte				prevState;
	byte				mtype;
	byte				sequence;
} SerialLine;

#define STATE_READY		0
#define STATE_SEQUENCE	1
#define STATE_LENGTH1	2
#define STATE_LENGTH2	3
#define STATE_DATA		4
#define STATE_CRC1		5
#define STATE_CRC2		6
#define STATE_EOM		7
#define STATE_ESCAPE	8
#define STATE_ABORT		9

SerialLine serialLine[NUM_COM_PORTS];

int myDriverLevel;

static void Serial_SendACK (SerialLine *p, byte sequence);


static void ResetSerialLineProtocol (SerialLine *p)
{
	p->connected = false;
	p->connecting = false;
	p->currState = STATE_READY;
	p->prevState = STATE_READY;
	p->lengthFound = 0;
}


static int ProcessInQueue(SerialLine *p)
{
	int	b;

	while (1)
	{
		b = TTY_ReadByte(p->tty);
		if (b == ERR_TTY_NODATA)
			break;

		if (b == ERR_TTY_LINE_STATUS)
		{
			p->currState = STATE_ABORT;
			continue;
		}
		if (b == ERR_TTY_MODEM_STATUS)
		{
			p->currState = STATE_ABORT;
			return -1;
		}

		if (b == ESCAPE_COMMAND)
			if (p->currState != STATE_ESCAPE)
			{
				p->prevState = p->currState;
				p->currState = STATE_ESCAPE;
				continue;
			}

		if (p->currState == STATE_ESCAPE)
		{
			if (b == ESCAPE_EOM)
			{
				if (p->prevState == STATE_ABORT)
				{
					p->currState = STATE_READY;
					p->lengthFound = 0;
					continue;
				}

				if (p->prevState != STATE_EOM)
				{
					p->currState = STATE_READY;
					p->lengthFound = 0;
					Con_DPrintf("Serial: premature EOM\n");
					continue;
				}

				switch (p->mtype)
				{
					case MTYPE_RELIABLE:
						Con_DPrintf("Serial: sending ack %u\n", p->sequence);
						Serial_SendACK (p, p->sequence);
						if (p->sequence == p->sock->receiveSequence)
						{
							p->sock->receiveSequence = (p->sequence + 1) & 0xff;
							p->sock->receiveMessageLength += p->lengthFound;
						}
						else
							Con_DPrintf("Serial: reliable out of order; got %u wanted %u\n", p->sequence, p->sock->receiveSequence);
						break;

					case MTYPE_UNRELIABLE:
						p->sock->unreliableReceiveSequence = (p->sequence + 1) & 0xff;
						p->sock->receiveMessageLength += p->lengthFound;
						break;

					case MTYPE_ACK:
						Con_DPrintf("Serial: got ack %u\n", p->sequence);
						if (p->sequence == p->sock->sendSequence)
						{
							p->sock->sendSequence = (p->sock->sendSequence + 1) & 0xff;
							p->sock->canSend = true;
						}
						else
							Con_DPrintf("Serial: ack out of order; got %u wanted %u\n",p->sequence, p->sock->sendSequence);
						break;

					case MTYPE_CONTROL:
						p->sock->receiveMessageLength += p->lengthFound;
						break;
					}

				p->currState = STATE_READY;
				p->lengthFound = 0;
				continue;
			}


			if (b != ESCAPE_COMMAND)
			{
				p->currState = STATE_ABORT;
				Con_DPrintf("Serial: Bad escape sequence\n");
				continue;
			}

			// b == ESCAPE_COMMAND
			p->currState = p->prevState;
		}

		p->prevState = p->currState;

//DEBUG
		if (p->sock->receiveMessageLength + p->lengthFound > NET_MAXMESSAGE)
		{
			Con_DPrintf("Serial blew out receive buffer: %u\n", p->sock->receiveMessageLength + p->lengthFound);
			p->currState = STATE_ABORT;
		}
		if (p->sock->receiveMessageLength + p->lengthFound == NET_MAXMESSAGE)
		{
			Con_DPrintf("Serial hit receive buffer limit: %u\n", p->sock->receiveMessageLength + p->lengthFound);
			p->currState = STATE_ABORT;
		}
//end DEBUG

		switch (p->currState)
		{
			case STATE_READY:
				CRC_Init(&p->crcValue);
				CRC_ProcessByte(&p->crcValue, b);
				if (p->client)
				{
					if ((b & MTYPE_CLIENT) != 0)
					{
						p->currState = STATE_ABORT;
						Con_DPrintf("Serial: client got own message\n");
						break;
					}
				}
				else
				{
					if ((b & MTYPE_CLIENT) == 0)
					{
						p->currState = STATE_ABORT;
						Con_DPrintf("Serial: server got own message\n");
						break;
					}
					b &= 0x7f;
				}
				p->mtype = b;
				if (b != MTYPE_CONTROL)
					p->currState = STATE_SEQUENCE;
				else
					p->currState = STATE_LENGTH1;
				if (p->mtype < MTYPE_ACK)
				{
					p->sock->receiveMessage[p->sock->receiveMessageLength] = b;
					p->lengthFound++;
				}
				break;

			case STATE_SEQUENCE:
				p->sequence = b;
				CRC_ProcessByte(&p->crcValue, b);
				if (p->mtype != MTYPE_ACK)
					p->currState = STATE_LENGTH1;
				else
					p->currState = STATE_CRC1;
				break;

			case STATE_LENGTH1:
				p->lengthStated = b * 256;
				CRC_ProcessByte(&p->crcValue, b);
				p->currState = STATE_LENGTH2;
				break;

			case STATE_LENGTH2:
				p->lengthStated += b;
				CRC_ProcessByte(&p->crcValue, b);
				if (p->mtype == MTYPE_RELIABLE && p->lengthStated > MAX_MSGLEN)
				{
					p->currState = STATE_ABORT;
					Con_DPrintf("Serial: bad reliable message length %u\n", p->lengthStated);
				}
				else if (p->mtype == MTYPE_UNRELIABLE && p->lengthStated > MAX_DATAGRAM)
				{
					p->currState = STATE_ABORT;
					Con_DPrintf("Serial: bad unreliable message length %u\n", p->lengthStated);
				}
				else
				{
					p->currState = STATE_DATA;
					if (p->mtype < MTYPE_ACK)
					{
						*(short *)&p->sock->receiveMessage [p->sock->receiveMessageLength + 1] = p->lengthStated;
						p->lengthFound += 2;
					}
				}
				break;

			case STATE_DATA:
				p->sock->receiveMessage[p->sock->receiveMessageLength + p->lengthFound] = b;
				p->lengthFound++;
				CRC_ProcessByte(&p->crcValue, b);
				if (p->lengthFound == p->lengthStated + 3)
					p->currState = STATE_CRC1;
				break;

			case STATE_CRC1:
				p->crcStated = b * 256;
				p->currState = STATE_CRC2;
				break;

			case STATE_CRC2:
				p->crcStated += b;
				if (p->crcStated == CRC_Value(p->crcValue))
				{
					p->currState = STATE_EOM;
				}
				else
				{
					p->currState = STATE_ABORT;
					Con_DPrintf("Serial: Bad crc\n");
				}
				break;

			case STATE_EOM:
				p->currState = STATE_ABORT;
				Con_DPrintf("Serial: Bad message format\n");
				break;

			case STATE_ABORT:
				break;
		}
	}
	return 0;
}


int Serial_Init (void)
{
	int     n;

// LATER do Win32 serial support
#ifdef	_WIN32
	return -1;
#endif

	if (COM_CheckParm("-nolan"))
		return -1;
	if (COM_CheckParm ("-noserial"))
		return -1;

	myDriverLevel = net_driverlevel;

	if (TTY_Init())
		return -1;

	for (n = 0; n < NUM_COM_PORTS; n++)
	{
		serialLine[n].tty = TTY_Open(n);
		ResetSerialLineProtocol (&serialLine[n]);
	}

	Con_Printf("Serial driver initialized\n");
	serialAvailable = true;

	return 0;
}


void Serial_Shutdown (void)
{
	int     n;

	for (n = 0; n < NUM_COM_PORTS; n++)
	{
		if (serialLine[n].connected)
			Serial_Close(serialLine[n].sock);
	}

	TTY_Shutdown();
}


void Serial_Listen (qboolean state)
{
	listening = state;
}


qboolean Serial_CanSendMessage (qsocket_t *sock)
{
	return sock->canSend;
}


qboolean Serial_CanSendUnreliableMessage (qsocket_t *sock)
{
	return TTY_OutputQueueIsEmpty(((SerialLine *)sock->driverdata)->tty);
}


int Serial_SendMessage (qsocket_t *sock, sizebuf_t *message)
{
	SerialLine *p;
	int n;
	unsigned short crc;
	byte b;

	p = (SerialLine *)sock->driverdata;
	CRC_Init (&crc);

	// message type
	b = MTYPE_RELIABLE;
	if (p->client)
		b |= MTYPE_CLIENT;
	TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);

	// sequence
	b = p->sock->sendSequence;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);

	// data length
	b = message->cursize >> 8;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);
	b = message->cursize & 0xff;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);

	// data
	for (n = 0; n < message->cursize; n++)
	{
		b = message->data[n];
		TTY_WriteByte(p->tty, b);
		if (b == ESCAPE_COMMAND)
			TTY_WriteByte(p->tty, b);
		CRC_ProcessByte (&crc, b);
	}

	// checksum
	b = CRC_Value (crc) >> 8;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	b = CRC_Value (crc) & 0xff;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);

	// end of message
	TTY_WriteByte(p->tty, ESCAPE_COMMAND);
	TTY_WriteByte(p->tty, ESCAPE_EOM);

	TTY_Flush(p->tty);

	// mark sock as busy and save the message for possible retransmit
	sock->canSend = false;
	Q_memcpy(sock->sendMessage, message->data, message->cursize);
	sock->sendMessageLength = message->cursize;
	sock->lastSendTime = net_time;

	return 1;
}


static void ReSendMessage (qsocket_t *sock)
{
	sizebuf_t       temp;

	Con_DPrintf("Serial: re-sending reliable\n");
	temp.data = sock->sendMessage;
	temp.maxsize = sock->sendMessageLength;
	temp.cursize = sock->sendMessageLength;
	Serial_SendMessage (sock, &temp);
}


int Serial_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *message)
{
	SerialLine *p;
	int n;
	unsigned short crc;
	byte b;

	p = (SerialLine *)sock->driverdata;

	if (!TTY_OutputQueueIsEmpty(p->tty))
	{
		TTY_Flush(p->tty);
		return 1;
	}

	CRC_Init (&crc);

	// message type
	b = MTYPE_UNRELIABLE;
	if (p->client)
		b |= MTYPE_CLIENT;
	TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);

	// sequence
	b = p->sock->unreliableSendSequence;
	p->sock->unreliableSendSequence = (b + 1) & 0xff;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);

	// data length
	b = message->cursize >> 8;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);
	b = message->cursize & 0xff;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);

	// data
	for (n = 0; n < message->cursize; n++)
	{
		b = message->data[n];
		TTY_WriteByte(p->tty, b);
		if (b == ESCAPE_COMMAND)
			TTY_WriteByte(p->tty, b);
		CRC_ProcessByte (&crc, b);
	}

	// checksum
	b = CRC_Value (crc) >> 8;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	b = CRC_Value (crc) & 0xff;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);

	// end of message
	TTY_WriteByte(p->tty, ESCAPE_COMMAND);
	TTY_WriteByte(p->tty, ESCAPE_EOM);

	TTY_Flush(p->tty);

	return 1;
}


static void Serial_SendACK (SerialLine *p, byte sequence)
{
	unsigned short crc;
	byte b;

	CRC_Init (&crc);

	// message type
	b = MTYPE_ACK;
	if (p->client)
		b |= MTYPE_CLIENT;
	TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);

	// sequence
	b = sequence;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);

	// checksum
	b = CRC_Value (crc) >> 8;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	b = CRC_Value (crc) & 0xff;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);

	// end of message
	TTY_WriteByte(p->tty, ESCAPE_COMMAND);
	TTY_WriteByte(p->tty, ESCAPE_EOM);

	TTY_Flush(p->tty);
}


static void Serial_SendControlMessage (SerialLine *p, sizebuf_t *message)
{
	unsigned short crc;
	int n;
	byte b;

	CRC_Init (&crc);

	// message type
	b = MTYPE_CONTROL;
	if (p->client)
		b |= MTYPE_CLIENT;
	TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);

	// data length
	b = message->cursize >> 8;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);
	b = message->cursize & 0xff;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	CRC_ProcessByte (&crc, b);

	// data
	for (n = 0; n < message->cursize; n++)
	{
		b = message->data[n];
		TTY_WriteByte(p->tty, b);
		if (b == ESCAPE_COMMAND)
			TTY_WriteByte(p->tty, b);
		CRC_ProcessByte (&crc, b);
	}

	// checksum
	b = CRC_Value (crc) >> 8;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);
	b = CRC_Value (crc) & 0xff;
	TTY_WriteByte(p->tty, b);
	if (b == ESCAPE_COMMAND)
		TTY_WriteByte(p->tty, b);

	// end of message
	TTY_WriteByte(p->tty, ESCAPE_COMMAND);
	TTY_WriteByte(p->tty, ESCAPE_EOM);

	TTY_Flush(p->tty);
}


static int _Serial_GetMessage (SerialLine *p)
{
	byte	ret;
	short	length;

	if (ProcessInQueue(p))
		return -1;

	if (p->sock->receiveMessageLength == 0)
		return 0;

	ret = p->sock->receiveMessage[0];
	length = *(short *)&p->sock->receiveMessage[1];
	if (ret == MTYPE_CONTROL)
		ret = 1;

	SZ_Clear (&net_message);
	SZ_Write (&net_message, &p->sock->receiveMessage[3], length);

	length += 3;
	p->sock->receiveMessageLength -= length;

	if (p->sock->receiveMessageLength + p->lengthFound)
		Q_memcpy(p->sock->receiveMessage, &p->sock->receiveMessage[length], p->sock->receiveMessageLength + p->lengthFound);

	return ret;
}

int Serial_GetMessage (qsocket_t *sock)
{
	SerialLine *p;
	int		ret;

	p = (SerialLine *)sock->driverdata;

	ret = _Serial_GetMessage (p);

	if (ret == 1)
		messagesReceived++;

	if (!sock->canSend)
		if ((net_time - sock->lastSendTime) > 1.0)
		{
			ReSendMessage (sock);
			sock->lastSendTime = net_time;
		}

	return ret;
}


void Serial_Close (qsocket_t *sock)
{
	SerialLine *p = (SerialLine *)sock->driverdata;
	TTY_Close(p->tty);
	ResetSerialLineProtocol (p);
}


char *com_types[] = {"direct", "modem"};
unsigned com_bauds[] = {9600, 14400, 19200, 28800, 57600};

void Serial_SearchForHosts (qboolean xmit)
{
	int		n;
	SerialLine *p;

	if (sv.active)
		return;

	if (hostCacheCount == HOSTCACHESIZE)
		return;

	// see if we've already answered
	for (n = 0; n < hostCacheCount; n++)
		if (Q_strcmp (hostcache[n].cname, "#") == 0)
			return;

	for (n = 0; n < NUM_COM_PORTS; n++)
		if (TTY_IsEnabled(n))
			break;
	if (n == NUM_COM_PORTS)
		return;
	p = &serialLine[n];

	if (TTY_IsModem(p->tty))
		return;

	sprintf(hostcache[hostCacheCount].name, "COM%u", n+1);
	Q_strcpy(hostcache[hostCacheCount].map, "");
	hostcache[hostCacheCount].users = 0;
	hostcache[hostCacheCount].maxusers = 0;
	hostcache[hostCacheCount].driver = net_driverlevel;
	Q_strcpy(hostcache[hostCacheCount].cname, "#");
	hostCacheCount++;

	return;
}


static qsocket_t *_Serial_Connect (char *host, SerialLine *p)
{
	int		ret;
	double	start_time;
	double	last_time;

	p->client = true;
	if (TTY_Connect(p->tty, host))
		return NULL;

	p->sock = NET_NewQSocket ();
	p->sock->driver = myDriverLevel;
	if (p->sock == NULL)
	{
		Con_Printf("No sockets available\n");
		return NULL;
	}
	p->sock->driverdata = p;

	// send the connection request
	start_time = SetNetTime();
	last_time = 0.0;

	SZ_Clear(&net_message);
	MSG_WriteByte(&net_message, CCREQ_CONNECT);
	MSG_WriteString(&net_message, "QUAKE");
	do
	{
		SetNetTime();
		if ((net_time - last_time) >= 1.0)
		{
			Serial_SendControlMessage (p, &net_message);
			last_time = net_time;
			Con_Printf("trying...\n"); SCR_UpdateScreen ();
		}
		ret = _Serial_GetMessage (p);
	}
	while (ret == 0 && (net_time - start_time) < 5.0);

	if (ret == 0)
	{
		Con_Printf("Unable to connect, no response\n");
		goto ErrorReturn;
	}

	if (ret == -1)
	{
		Con_Printf("Connection request error\n");
		goto ErrorReturn;
	}

	MSG_BeginReading ();
	ret = MSG_ReadByte();
	if (ret == CCREP_REJECT)
	{
		Con_Printf(MSG_ReadString());
		goto ErrorReturn;
	}
	if (ret != CCREP_ACCEPT)
	{
		Con_Printf("Unknown connection response\n");
		goto ErrorReturn;
	}

	p->connected = true;
	p->sock->lastMessageTime = net_time;

	Con_Printf ("Connection accepted\n");

	return p->sock;

ErrorReturn:
	TTY_Disconnect(p->tty);
	return NULL;
}

qsocket_t *Serial_Connect (char *host)
{
	int			n;
	qsocket_t	*ret = NULL;

	// see if this looks like a phone number
	if (*host == '#')
		host++;
	for (n = 0; n < Q_strlen(host); n++)
		if (host[n] == '.' || host[n] == ':')
			return NULL;

	for (n = 0; n < NUM_COM_PORTS; n++)
		if (TTY_IsEnabled(n) && !serialLine[n].connected)
			if ((ret = _Serial_Connect (host, &serialLine[n])))
				break;
	return ret;
}


static qsocket_t *_Serial_CheckNewConnections (SerialLine *p)
{
	int	command;

	p->client = false;
	if (!TTY_CheckForConnection(p->tty))
		return NULL;

	if (TTY_IsModem(p->tty))
	{
		if (!p->connecting)
		{
			p->connecting = true;
			p->connect_time = net_time;
		}
		else if ((net_time - p->connect_time) > 15.0)
		{
			p->connecting = false;
			TTY_Disconnect(p->tty);
			return NULL;
		}
	}

	p->sock = NET_NewQSocket ();
	p->sock->driver = myDriverLevel;
	if (p->sock == NULL)
	{
		Con_Printf("No sockets available\n");
		return NULL;
	}
	p->sock->driverdata = p;

	SZ_Clear(&net_message);
	if (_Serial_GetMessage(p) != 1)
	{
		NET_FreeQSocket(p->sock);
		return NULL;
	}

	MSG_BeginReading ();
	command = MSG_ReadByte();

	if (command == CCREQ_SERVER_INFO)
	{
		if (Q_strcmp(MSG_ReadString(), "QUAKE") != 0)
			return NULL;

		if (MSG_ReadByte() != SERIAL_PROTOCOL_VERSION)
			return NULL;

		SZ_Clear(&net_message);
		MSG_WriteByte(&net_message, CCREP_SERVER_INFO);
		MSG_WriteString(&net_message, hostname.string);
		MSG_WriteString(&net_message, sv.name);
		MSG_WriteByte(&net_message, net_activeconnections);
		MSG_WriteByte(&net_message, svs.maxclients);
		Serial_SendControlMessage (p, &net_message);
		SZ_Clear(&net_message);
		return NULL;
	}

	if (command != CCREQ_CONNECT)
		return NULL;

	if (Q_strcmp(MSG_ReadString(), "QUAKE") != 0)
		return NULL;

	// send him back the info about the server connection he has been allocated
	SZ_Clear(&net_message);
	MSG_WriteByte(&net_message, CCREP_ACCEPT);
	Serial_SendControlMessage (p, &net_message);
	SZ_Clear(&net_message);

	p->connected = true;
	p->connecting = false;
	p->sock->lastMessageTime = net_time;
	sprintf(p->sock->address, "COM%u", (int)((p - serialLine) + 1));

	return p->sock;
}

qsocket_t *Serial_CheckNewConnections (void)
{
	int			n;
	qsocket_t	*ret = NULL;

	for (n = 0; n < NUM_COM_PORTS; n++)
		if (TTY_IsEnabled(n) && !serialLine[n].connected)
			if ((ret = _Serial_CheckNewConnections (&serialLine[n])))
				break;
	return ret;
}
