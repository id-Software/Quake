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
// net_comx.c

#include <dos.h>
#include <dpmi.h>

#define NUM_COM_PORTS	2

#define ERR_TTY_LINE_STATUS		-1
#define ERR_TTY_MODEM_STATUS	-2
#define ERR_TTY_NODATA			-3

#define QUEUESIZE	8192
#define QUEUEMASK	(QUEUESIZE - 1)

typedef struct
{
	volatile int  head;
	volatile int  tail;
	volatile byte data[QUEUESIZE];
} queue;

#define FULL(q)			(q.head == ((q.tail-1) & QUEUEMASK))
#define EMPTY(q)		(q.tail == q.head)
#define ENQUEUE(q,b)	(q.data[q.head] = b, q.head = (q.head + 1) & QUEUEMASK)
#define DEQUEUE(q,b)	(b = q.data[q.tail], q.tail = (q.tail + 1) & QUEUEMASK)

extern cvar_t	config_com_port;
extern cvar_t	config_com_irq;
extern cvar_t	config_com_baud;
extern cvar_t	config_com_modem;
extern cvar_t	config_modem_dialtype;
extern cvar_t	config_modem_clear;
extern cvar_t	config_modem_init;
extern cvar_t	config_modem_hangup;

extern int m_return_state;
extern int m_state;
extern qboolean m_return_onerror;
extern char m_return_reason[32];

// 8250, 16550 definitions
#define TRANSMIT_HOLDING_REGISTER            0x00
#define RECEIVE_BUFFER_REGISTER              0x00
#define INTERRUPT_ENABLE_REGISTER            0x01
#define   IER_RX_DATA_READY                  0x01
#define   IER_TX_HOLDING_REGISTER_EMPTY      0x02
#define   IER_LINE_STATUS                    0x04
#define   IER_MODEM_STATUS                   0x08
#define INTERRUPT_ID_REGISTER                0x02
#define   IIR_MODEM_STATUS_INTERRUPT         0x00
#define   IIR_TX_HOLDING_REGISTER_INTERRUPT  0x02
#define   IIR_RX_DATA_READY_INTERRUPT        0x04
#define   IIR_LINE_STATUS_INTERRUPT          0x06
#define   IIR_FIFO_TIMEOUT                   0x0c
#define   IIR_FIFO_ENABLED                   0xc0
#define FIFO_CONTROL_REGISTER                0x02
#define   FCR_FIFO_ENABLE                    0x01
#define   FCR_RCVR_FIFO_RESET                0x02
#define   FCR_XMIT_FIFO_RESET                0x04
#define   FCR_TRIGGER_01                     0x00
#define   FCR_TRIGGER_04                     0x40
#define   FCR_TRIGGER_08                     0x80
#define   FCR_TRIGGER_16                     0xc0
#define LINE_CONTROL_REGISTER                0x03
#define   LCR_DATA_BITS_5                    0x00
#define   LCR_DATA_BITS_6                    0x01
#define   LCR_DATA_BITS_7                    0x02
#define   LCR_DATA_BITS_8                    0x03
#define   LCR_STOP_BITS_1                    0x00
#define   LCR_STOP_BITS_2                    0x04
#define   LCR_PARITY_NONE                    0x00
#define   LCR_PARITY_ODD                     0x08
#define   LCR_PARITY_EVEN                    0x18
#define   LCR_PARITY_MARK                    0x28
#define   LCR_PARITY_SPACE                   0x38
#define   LCR_SET_BREAK                      0x40
#define   LCR_DLAB                           0x80
#define MODEM_CONTROL_REGISTER               0x04
#define   MCR_DTR                            0x01
#define   MCR_RTS                            0x02
#define   MCR_OUT1                           0x04
#define   MCR_OUT2                           0x08
#define   MCR_LOOPBACK                       0x10
#define LINE_STATUS_REGISTER                 0x05
#define   LSR_DATA_READY                     0x01
#define   LSR_OVERRUN_ERROR                  0x02
#define   LSR_PARITY_ERROR                   0x04
#define   LSR_FRAMING_ERROR                  0x08
#define   LSR_BREAK_DETECT                   0x10
#define   LSR_TRANSMITTER_BUFFER_EMPTY       0x20
#define   LSR_TRANSMITTER_EMPTY              0x40
#define   LSR_FIFO_DIRTY                     0x80
#define MODEM_STATUS_REGISTER                0x06
#define   MSR_DELTA_CTS                      0x01
#define   MSR_DELTA_DSR                      0x02
#define   MSR_DELTA_RI                       0x04
#define   MSR_DELTA_CD                       0x08
#define   MSR_CTS                            0x10
#define   MSR_DSR                            0x20
#define   MSR_RI                             0x40
#define   MSR_CD                             0x80
#define DIVISOR_LATCH_LOW                    0x00
#define DIVISOR_LATCH_HIGH                   0x01

#define MODEM_STATUS_MASK	(MSR_CTS | MSR_DSR | MSR_CD)

#define UART_AUTO	0
#define UART_8250	1
#define UART_16550	2

static int ISA_uarts[]	= {0x3f8,0x2f8,0x3e8,0x2e8};
static int ISA_IRQs[]	= {4,3,4,3};

typedef struct ComPort_s
{
	struct ComPort_s		*next;
	_go32_dpmi_seginfo		protectedModeInfo;
	_go32_dpmi_seginfo		protectedModeSaveInfo;
	int						uart;
	volatile byte			modemStatus;
	byte					modemStatusIgnore;
	byte					lineStatus;
	byte					bufferUsed;
	qboolean				enabled;
	volatile qboolean		statusUpdated;
	qboolean				useModem;
	qboolean				modemInitialized;
	qboolean				modemRang;
	qboolean				modemConnected;
	queue					inputQueue;
	queue					outputQueue;
	char					clear[16];
	char					startup[32];
	char					shutdown[16];
	char					buffer[128];
	PollProcedure			poll;
	double					timestamp;
	byte					uartType;
	byte					irq;
	byte					baudBits;
	byte					lineControl;
	byte					portNumber;
	char					dialType;
	char					name[4];
} ComPort;

ComPort *portList = NULL;
ComPort *handleToPort [NUM_COM_PORTS];

static int Modem_Command(ComPort *p, char *commandString);
static char *Modem_Response(ComPort *p);
static void Modem_Hangup(ComPort *p);

int TTY_Init(void);
void TTY_Shutdown(void);
int TTY_Open(int serialPortNumber);
void TTY_Close(int handle);
int TTY_ReadByte(int handle);
int TTY_WriteByte(int handle, byte data);
void TTY_Flush(int handle);
int TTY_Connect(int handle, char *host);
void TTY_Disconnect(int handle);
qboolean TTY_CheckForConnection(int handle);
qboolean TTY_IsEnabled(int serialPortNumber);
qboolean TTY_IsModem(int serialPortNumber);
qboolean TTY_OutputQueueIsEmpty(int handle);

static void ISR_8250 (ComPort *p)
{
	byte	source = 0;
	byte	b;

	disable();

	while((source = inportb (p->uart + INTERRUPT_ID_REGISTER) & 0x07) != 1)
	{
		switch (source)
		{
			case IIR_RX_DATA_READY_INTERRUPT:
				b = inportb (p->uart + RECEIVE_BUFFER_REGISTER);
				if (! FULL(p->inputQueue))
				{
					ENQUEUE (p->inputQueue, b);
				}
				else
				{
					p->lineStatus |= LSR_OVERRUN_ERROR;
					p->statusUpdated = true;
				}
				break;

			case IIR_TX_HOLDING_REGISTER_INTERRUPT:
				if (! EMPTY(p->outputQueue))
				{
					DEQUEUE (p->outputQueue, b);
					outportb (p->uart + TRANSMIT_HOLDING_REGISTER, b);
				}
				break;

			case IIR_MODEM_STATUS_INTERRUPT:
				p->modemStatus = (inportb (p->uart + MODEM_STATUS_REGISTER) & MODEM_STATUS_MASK) | p->modemStatusIgnore;
				p->statusUpdated = true;
				break;

			case IIR_LINE_STATUS_INTERRUPT:
				p->lineStatus = inportb (p->uart + LINE_STATUS_REGISTER);
				p->statusUpdated = true;
				break;
		}
		source = inportb (p->uart + INTERRUPT_ID_REGISTER) & 0x07;
	}
	outportb (0x20, 0x20);
}

static void COM1_ISR_8250 (void)
{
	ISR_8250 (handleToPort[0]);
}

static void COM2_ISR_8250 (void)
{
	ISR_8250 (handleToPort[1]);
}



static void ISR_16550 (ComPort *p)
{
	int		count;
	byte	source;
	byte	b;

	disable();
	while((source = inportb (p->uart + INTERRUPT_ID_REGISTER) & 0x07) != 1)
	{
		switch (source)
		{
			case IIR_RX_DATA_READY_INTERRUPT:
				do
				{
					b = inportb (p->uart + RECEIVE_BUFFER_REGISTER);
					if (!FULL(p->inputQueue))
					{
						ENQUEUE (p->inputQueue, b);
					}
					else
					{
						p->lineStatus |= LSR_OVERRUN_ERROR;
						p->statusUpdated = true;
					}
				} while (inportb (p->uart + LINE_STATUS_REGISTER) & LSR_DATA_READY);
				break;

			case IIR_TX_HOLDING_REGISTER_INTERRUPT:
				count = 16;
				while ((! EMPTY(p->outputQueue)) && count--)
				{
					DEQUEUE (p->outputQueue, b);
					outportb (p->uart + TRANSMIT_HOLDING_REGISTER, b);
				}
				break;

			case IIR_MODEM_STATUS_INTERRUPT:
				p->modemStatus = (inportb (p->uart + MODEM_STATUS_REGISTER) & MODEM_STATUS_MASK) | p->modemStatusIgnore;
				p->statusUpdated = true;
				break;

			case IIR_LINE_STATUS_INTERRUPT:
				p->lineStatus = inportb (p->uart + LINE_STATUS_REGISTER);
				p->statusUpdated = true;
				break;
		}
		source = inportb (p->uart + INTERRUPT_ID_REGISTER) & 0x07;
	}

	// check for lost IIR_TX_HOLDING_REGISTER_INTERRUPT on 16550a!
	if (inportb (p->uart + LINE_STATUS_REGISTER ) & LSR_TRANSMITTER_EMPTY)
	{
		count = 16;
		while ((! EMPTY(p->outputQueue)) && count--)
		{
			DEQUEUE (p->outputQueue, b);
			outportb (p->uart + TRANSMIT_HOLDING_REGISTER, b);
		}
	}

	outportb (0x20, 0x20);
}

static void COM1_ISR_16550 (void)
{
	ISR_16550 (handleToPort[0]);
}

static void COM2_ISR_16550 (void)
{
	ISR_16550 (handleToPort[1]);
}


void TTY_GetComPortConfig (int portNumber, int *port, int *irq, int *baud, qboolean *useModem)
{
	ComPort	*p;

	p = handleToPort[portNumber];
	*port = p->uart;
	*irq = p->irq;
	*baud = 115200 / p->baudBits;
	*useModem = p->useModem;
}

void TTY_SetComPortConfig (int portNumber, int port, int irq, int baud, qboolean useModem)
{
	ComPort	*p;
	float	temp;

	if (useModem)
	{
		if (baud == 14400)
			baud = 19200;
		if (baud == 28800)
			baud = 38400;
	}

	p = handleToPort[portNumber];
	p->uart = port;
	p->irq = irq;
	p->baudBits = 115200 / baud;
	p->useModem = useModem;

	if (useModem)
		temp = 1.0;
	else
		temp = 0.0;

	Cvar_SetValue ("_config_com_port", (float)port);
	Cvar_SetValue ("_config_com_irq", (float)irq);
	Cvar_SetValue ("_config_com_baud", (float)baud);
	Cvar_SetValue ("_config_com_modem", temp);
}

void TTY_GetModemConfig (int portNumber, char *dialType, char *clear, char *init, char *hangup)
{
	ComPort	*p;

	p = handleToPort[portNumber];
	*dialType = p->dialType;
	Q_strcpy(clear, p->clear);
	Q_strcpy(init, p->startup);
	Q_strcpy(hangup, p->shutdown);
}

void TTY_SetModemConfig (int portNumber, char *dialType, char *clear, char *init, char *hangup)
{
	ComPort	*p;

	p = handleToPort[portNumber];
	p->dialType = dialType[0];
	Q_strcpy(p->clear, clear);
	Q_strcpy(p->startup, init);
	Q_strcpy(p->shutdown, hangup);

	p->modemInitialized = false;

	Cvar_Set ("_config_modem_dialtype", dialType);
	Cvar_Set ("_config_modem_clear", clear);
	Cvar_Set ("_config_modem_init", init);
	Cvar_Set ("_config_modem_hangup", hangup);
}


static void ResetComPortConfig (ComPort *p)
{
	p->useModem = false;
	p->uartType = UART_AUTO;
	p->uart = ISA_uarts[p->portNumber];
	p->irq = ISA_IRQs[p->portNumber];
	p->modemStatusIgnore = MSR_CD | MSR_CTS | MSR_DSR;
	p->baudBits = 115200 / 57600;
	p->lineControl = LCR_DATA_BITS_8 | LCR_STOP_BITS_1 | LCR_PARITY_NONE;
	Q_strcpy(p->clear, "ATZ");
	Q_strcpy(p->startup, "");
	Q_strcpy(p->shutdown, "AT H");
	p->modemRang = false;
	p->modemConnected = false;
	p->statusUpdated = false;
	p->outputQueue.head = p->outputQueue.tail = 0;
	p->inputQueue.head = p->inputQueue.tail = 0;
}


static void ComPort_Enable(ComPort *p)
{
	void	(*isr)(void);
	int		n;
	byte	b;

	if (p->enabled)
	{
		Con_Printf("Already enabled\n");
		return;
	}

	// disable all UART interrupts
	outportb (p->uart + INTERRUPT_ENABLE_REGISTER, 0);

	// clear out any buffered uncoming data
	while((inportb (p->uart + LINE_STATUS_REGISTER)) & LSR_DATA_READY)
		inportb (p->uart + RECEIVE_BUFFER_REGISTER);

	// get the current line and modem status
	p->modemStatus = (inportb (p->uart + MODEM_STATUS_REGISTER) & MODEM_STATUS_MASK) | p->modemStatusIgnore;
	p->lineStatus = inportb (p->uart + LINE_STATUS_REGISTER);

	// clear any UART interrupts
	do
	{
		n = inportb (p->uart + INTERRUPT_ID_REGISTER) & 7;
		if (n == IIR_RX_DATA_READY_INTERRUPT)
			inportb (p->uart + RECEIVE_BUFFER_REGISTER);
	} while (!(n & 1));

	if (p->uartType == UART_AUTO)
	{
		outportb (p->uart + FIFO_CONTROL_REGISTER, FCR_FIFO_ENABLE);
		b = inportb (p->uart + INTERRUPT_ID_REGISTER);
		if ((b & IIR_FIFO_ENABLED) == IIR_FIFO_ENABLED)
			p->uartType = UART_16550;
		else
			p->uartType = UART_8250;
	}

	// save the old interrupt handler
	_go32_dpmi_get_protected_mode_interrupt_vector(p->irq + 8, &p->protectedModeSaveInfo);

	if (p->uartType == UART_8250)
	{
		outportb (p->uart + FIFO_CONTROL_REGISTER, 0);
		if (p == handleToPort[0])
			isr = COM1_ISR_8250;
		else
			isr = COM2_ISR_8250;
	}
	else
	{
		outportb (p->uart + FIFO_CONTROL_REGISTER, FCR_FIFO_ENABLE | FCR_RCVR_FIFO_RESET | FCR_XMIT_FIFO_RESET | FCR_TRIGGER_08);
		if (p == handleToPort[0])
			isr = COM1_ISR_16550;
		else
			isr = COM2_ISR_16550;
	}

	p->protectedModeInfo.pm_offset = (int)isr;

	n = _go32_dpmi_allocate_iret_wrapper(&p->protectedModeInfo);
	if (n)
	{
		Con_Printf("serial: protected mode callback allocation failed\n");
		return;
	}

	// disable interrupts at the processor
	disable();

	// install our interrupt handlers now
	_go32_dpmi_set_protected_mode_interrupt_vector(p->irq + 8, &p->protectedModeInfo);

	// enable our interrupt at the PIC
	outportb (0x21, inportb (0x21) & ~(1<<p->irq));

	// enable interrupts at the processor
	enable();

	// enable interrupts at the PIC
	outportb (0x20, 0xc2);

	// set baud rate & line control
	outportb (p->uart + LINE_CONTROL_REGISTER, LCR_DLAB | p->lineControl);
	outportb (p->uart, p->baudBits);
	outportb (p->uart + 1, 0);
	outportb (p->uart + LINE_CONTROL_REGISTER, p->lineControl);

	// set modem control register & enable uart interrupt generation
	outportb(p->uart + MODEM_CONTROL_REGISTER, MCR_OUT2 | MCR_RTS | MCR_DTR);

	// enable the individual interrupts at the uart
	outportb (p->uart + INTERRUPT_ENABLE_REGISTER, IER_RX_DATA_READY | IER_TX_HOLDING_REGISTER_EMPTY | IER_LINE_STATUS | IER_MODEM_STATUS);

	p->enabled = true;
}


static void ComPort_Disable(ComPort *p)
{
	if (!p->enabled)
	{
		Con_Printf("Already disabled\n");
		return;
	}

	// disable interrupts at the uart
	outportb (p->uart + INTERRUPT_ENABLE_REGISTER, 0);

	// disable our interrupt at the PIC
	outportb (0x21, inportb (0x21) | (1<<p->irq));

	// disable interrupts at the processor
	disable();

	// restore the old interrupt handler
	_go32_dpmi_set_protected_mode_interrupt_vector(p->irq + 8, &p->protectedModeSaveInfo);
	_go32_dpmi_free_iret_wrapper(&p->protectedModeInfo);

	// enable interrupts at the processor
	enable();

	p->enabled = false;
}


static int CheckStatus (ComPort *p)
{
	int		ret = 0;

	if (p->statusUpdated)
	{
		p->statusUpdated = false;

		if (p->lineStatus & (LSR_OVERRUN_ERROR | LSR_PARITY_ERROR | LSR_FRAMING_ERROR | LSR_BREAK_DETECT))
		{
			if (p->lineStatus & LSR_OVERRUN_ERROR)
				Con_DPrintf ("Serial overrun error\n");
			if (p->lineStatus & LSR_PARITY_ERROR)
				Con_DPrintf ("Serial parity error\n");
			if (p->lineStatus & LSR_FRAMING_ERROR)
				Con_DPrintf ("Serial framing error\n");
			if (p->lineStatus & LSR_BREAK_DETECT)
				Con_DPrintf ("Serial break detect\n");
			ret = ERR_TTY_LINE_STATUS;
		}

		if ((p->modemStatus & MODEM_STATUS_MASK) != MODEM_STATUS_MASK)
		{
			if (!(p->modemStatus & MSR_CTS))
				Con_Printf ("Serial lost CTS\n");
			if (!(p->modemStatus & MSR_DSR))
				Con_Printf ("Serial lost DSR\n");
			if (!(p->modemStatus & MSR_CD))
				Con_Printf ("Serial lost Carrier\n");
			ret = ERR_TTY_MODEM_STATUS;
		}
	}

	return ret;
}


static void Modem_Init(ComPort *p)
{
	double	start;
	char	*response;

	Con_Printf ("Initializing modem...\n");

	// write 0 to MCR, wait 1/2 sec, then write the real value back again
	// I got this from the guys at head-to-head who say it's necessary.
	outportb(p->uart + MODEM_CONTROL_REGISTER, 0);
	start = Sys_FloatTime();
	while ((Sys_FloatTime() - start) < 0.5)
		;
	outportb(p->uart + MODEM_CONTROL_REGISTER, MCR_OUT2 | MCR_RTS | MCR_DTR);
	start = Sys_FloatTime();
	while ((Sys_FloatTime() - start) < 0.25)
		;

	if (*p->clear)
	{
		Modem_Command (p, p->clear);
		start = Sys_FloatTime();
		while(1)
		{
			if ((Sys_FloatTime() - start) > 3.0)
			{
				Con_Printf("No response - clear failed\n");
				p->enabled = false;
				goto failed;
			}
			response = Modem_Response(p);
			if (!response)
				continue;
			if (Q_strncmp(response, "OK", 2) == 0)
				break;
			if (Q_strncmp(response, "ERROR", 5) == 0)
			{
				p->enabled = false;
				goto failed;
			}
		}
	}

	if (*p->startup)
	{
		Modem_Command (p, p->startup);
		start = Sys_FloatTime();
		while(1)
		{
			if ((Sys_FloatTime() - start) > 3.0)
			{
				Con_Printf("No response - init failed\n");
				p->enabled = false;
				goto failed;
			}
			response = Modem_Response(p);
			if (!response)
				continue;
			if (Q_strncmp(response, "OK", 2) == 0)
				break;
			if (Q_strncmp(response, "ERROR", 5) == 0)
			{
				p->enabled = false;
				goto failed;
			}
		}
	}

	p->modemInitialized = true;
	return;

failed:
	if (m_return_onerror)
	{
		key_dest = key_menu;
		m_state = m_return_state;
		m_return_onerror = false;
		Q_strcpy(m_return_reason, "Initialization Failed");
	}
	return;
}


void TTY_Enable(int handle)
{
	ComPort	*p;

	p = handleToPort [handle];
	if (p->enabled)
		return;

	ComPort_Enable(p);

	if (p->useModem && !p->modemInitialized)
		Modem_Init (p);
}


int TTY_Open(int serialPortNumber)
{
	return serialPortNumber;
}


void TTY_Close(int handle)
{
	ComPort	*p;
	double		startTime;

	p = handleToPort [handle];

	startTime = Sys_FloatTime();
	while ((Sys_FloatTime() - startTime) < 1.0)
		if (EMPTY(p->outputQueue))
			break;

	if (p->useModem)
	{
		if (p->modemConnected)
			Modem_Hangup(p);
	}
}


int TTY_ReadByte(int handle)
{
	int		ret;
	ComPort	*p;

	p = handleToPort [handle];

	if ((ret = CheckStatus (p)) != 0)
		return ret;
	
	if (EMPTY (p->inputQueue))
		return ERR_TTY_NODATA;

	DEQUEUE (p->inputQueue, ret);
	return (ret & 0xff);
}


int TTY_WriteByte(int handle, byte data)
{
	ComPort	*p;

	p = handleToPort [handle];
	if (FULL(p->outputQueue))
		return -1;

	ENQUEUE (p->outputQueue, data);
	return 0;
}


void TTY_Flush(int handle)
{
	byte b;
	ComPort	*p;

	p = handleToPort [handle];

	if (inportb (p->uart + LINE_STATUS_REGISTER ) & LSR_TRANSMITTER_EMPTY)
	{
		DEQUEUE (p->outputQueue, b);
		outportb(p->uart, b);
	}
}


int TTY_Connect(int handle, char *host)
{
	double	start;
	ComPort	*p;
	char	*response = NULL;
	keydest_t	save_key_dest;
	byte	dialstring[64];
	byte	b;

	p = handleToPort[handle];

	if ((p->modemStatus & MODEM_STATUS_MASK) != MODEM_STATUS_MASK)
	{
		Con_Printf ("Serial: line not ready (");
		if ((p->modemStatus & MSR_CTS) == 0)
			Con_Printf(" CTS");
		if ((p->modemStatus & MSR_DSR) == 0)
			Con_Printf(" DSR");
		if ((p->modemStatus & MSR_CD) == 0)
			Con_Printf(" CD");
		Con_Printf(" )");
		return -1;
	}

	// discard any scraps in the input buffer
	while (! EMPTY (p->inputQueue))
		DEQUEUE (p->inputQueue, b);

	CheckStatus (p);

	if (p->useModem)
	{
		save_key_dest = key_dest;
		key_dest = key_console;
		key_count = -2;

		Con_Printf ("Dialing...\n");
		sprintf(dialstring, "AT D%c %s\r", p->dialType, host);
		Modem_Command (p, dialstring);
		start = Sys_FloatTime();
		while(1)
		{
			if ((Sys_FloatTime() - start) > 60.0)
			{
				Con_Printf("Dialing failure!\n");
				break;
			}

			Sys_SendKeyEvents ();
			if (key_count == 0)
			{
				if (key_lastpress != K_ESCAPE)
				{
					key_count = -2;
					continue;
				}
				Con_Printf("Aborting...\n");
				while ((Sys_FloatTime() - start) < 5.0)
					;
				disable();
				p->outputQueue.head = p->outputQueue.tail = 0;
				p->inputQueue.head = p->inputQueue.tail = 0;
				outportb(p->uart + MODEM_CONTROL_REGISTER, inportb(p->uart + MODEM_CONTROL_REGISTER) & ~MCR_DTR);
				enable();
				start = Sys_FloatTime();
				while ((Sys_FloatTime() - start) < 0.75)
					;
				outportb(p->uart + MODEM_CONTROL_REGISTER, inportb(p->uart + MODEM_CONTROL_REGISTER) | MCR_DTR);
				response = "Aborted";
				break;
			}

			response = Modem_Response(p);
			if (!response)
				continue;
			if (Q_strncmp(response, "CONNECT", 7) == 0)
			{
				disable();
				p->modemRang = true;
				p->modemConnected = true;
				p->outputQueue.head = p->outputQueue.tail = 0;
				p->inputQueue.head = p->inputQueue.tail = 0;
				enable();
				key_dest = save_key_dest;
				key_count = 0;
				m_return_onerror = false;
				return 0;
			}
			if (Q_strncmp(response, "NO CARRIER", 10) == 0)
				break;
			if (Q_strncmp(response, "NO DIALTONE", 11) == 0)
				break;
			if (Q_strncmp(response, "NO DIAL TONE", 12) == 0)
				break;
			if (Q_strncmp(response, "NO ANSWER", 9) == 0)
				break;
			if (Q_strncmp(response, "BUSY", 4) == 0)
				break;
			if (Q_strncmp(response, "ERROR", 5) == 0)
				break;
		}
		key_dest = save_key_dest;
		key_count = 0;
		if (m_return_onerror)
		{
			key_dest = key_menu;
			m_state = m_return_state;
			m_return_onerror = false;
			Q_strncpy(m_return_reason, response, 31);
		}
		return -1;
	}
	m_return_onerror = false;
	return 0;
}


void TTY_Disconnect(int handle)
{
	ComPort *p;

	p = handleToPort[handle];

	if (p->useModem && p->modemConnected)
		Modem_Hangup(p);
}


qboolean TTY_CheckForConnection(int handle)
{
	ComPort	*p;

	p = handleToPort[handle];

	CheckStatus (p);

	if (p->useModem)
	{
		if (!p->modemRang)
		{
			if (!Modem_Response(p))
				return false;

			if (Q_strncmp(p->buffer, "RING", 4) == 0)
			{
				Modem_Command (p, "ATA");
				p->modemRang = true;
				p->timestamp = net_time;
			}
			return false;
		}
		if (!p->modemConnected)
		{
			if ((net_time - p->timestamp) > 35.0)
			{
				Con_Printf("Unable to establish modem connection\n");
				p->modemRang = false;
				return false;
			}

			if (!Modem_Response(p))
				return false;

			if (Q_strncmp (p->buffer, "CONNECT", 7) != 0)
				return false;

			disable();
			p->modemConnected = true;
			p->outputQueue.head = p->outputQueue.tail = 0;
			p->inputQueue.head = p->inputQueue.tail = 0;
			enable();
			Con_Printf("Modem Connect\n");
			return true;
		}
		return true;
	}

	// direct connect case
	if (EMPTY (p->inputQueue))
		return false;
	return true;
}


qboolean TTY_IsEnabled(int serialPortNumber)
{
	return handleToPort[serialPortNumber]->enabled;
}


qboolean TTY_IsModem(int serialPortNumber)
{
	return handleToPort[serialPortNumber]->useModem;
}


qboolean TTY_OutputQueueIsEmpty(int handle)
{
	return EMPTY(handleToPort[handle]->outputQueue);
}


void Com_f (void)
{
	ComPort	*p;
	int		portNumber;
	int		i;
	int		n;

	// first, determine which port they're messing with
	portNumber = Q_atoi(Cmd_Argv (0) + 3) - 1;
	if (portNumber > 1)
		return;
	p = handleToPort[portNumber];

	if (Cmd_Argc() == 1)
	{
		Con_Printf("Settings for COM%i\n", portNumber + 1);
		Con_Printf("enabled:   %s\n", p->enabled ? "true" : "false");
		Con_Printf("uart:      ");
		if (p->uartType == UART_AUTO)
			Con_Printf("auto\n");
		else if (p->uartType == UART_8250)
			Con_Printf("8250\n");
		else
			Con_Printf("16550\n");
		Con_Printf("port:      %x\n", p->uart);
		Con_Printf("irq:       %i\n", p->irq);
		Con_Printf("baud:      %i\n", 115200 / p->baudBits);	
		Con_Printf("CTS:       %s\n", (p->modemStatusIgnore & MSR_CTS) ? "ignored" : "honored");
		Con_Printf("DSR:       %s\n", (p->modemStatusIgnore & MSR_DSR) ? "ignored" : "honored");
		Con_Printf("CD:        %s\n", (p->modemStatusIgnore & MSR_CD) ? "ignored" : "honored");
		if (p->useModem)
		{
			Con_Printf("type:      Modem\n");
			Con_Printf("clear:     %s\n", p->clear);
			Con_Printf("startup:   %s\n", p->startup);
			Con_Printf("shutdown:  %s\n", p->shutdown);
		}
		else
			Con_Printf("type:      Direct connect\n");

		return;
	}


	if (Cmd_CheckParm ("disable"))
	{
		if (p->enabled)
			ComPort_Disable(p);
		p->modemInitialized = false;
		return;
	}

	if (Cmd_CheckParm ("reset"))
	{
		ComPort_Disable(p);
		ResetComPortConfig (p);
		return;
	}

	if ((i = Cmd_CheckParm ("port")) != 0)
	{
		if (p->enabled)
			{
				Con_Printf("COM port must be disabled to change port\n");
				return;
			}
		p->uart = Q_atoi (Cmd_Argv (i+1));
	}

	if ((i = Cmd_CheckParm ("irq")) != 0)
	{
		if (p->enabled)
			{
				Con_Printf("COM port must be disabled to change irq\n");
				return;
			}
		p->irq = Q_atoi (Cmd_Argv (i+1));
	}

	if ((i = Cmd_CheckParm ("baud")) != 0)
	{
		if (p->enabled)
			{
				Con_Printf("COM port must be disabled to change baud\n");
				return;
			}
		n = Q_atoi (Cmd_Argv (i+1));
		if (n == 0)
			Con_Printf("Invalid baud rate specified\n");
		else
			p->baudBits = 115200 / n;
	}

	if (Cmd_CheckParm ("8250"))
	{
		if (p->enabled)
			{
				Con_Printf("COM port must be disabled to change uart\n");
				return;
			}
		p->uartType = UART_8250;
		}
	if (Cmd_CheckParm ("16550"))
	{
		if (p->enabled)
			{
				Con_Printf("COM port must be disabled to change uart\n");
				return;
			}
		p->uartType = UART_16550;
	}
	if (Cmd_CheckParm ("auto"))
	{
		if (p->enabled)
			{
				Con_Printf("COM port must be disabled to change uart\n");
				return;
			}
		p->uartType = UART_AUTO;
	}

	if (Cmd_CheckParm ("pulse"))
		p->dialType = 'P';
	if (Cmd_CheckParm ("tone"))
		p->dialType = 'T';

	if (Cmd_CheckParm ("direct"))
		p->useModem = false;
	if (Cmd_CheckParm ("modem"))
		p->useModem = true;

	if ((i = Cmd_CheckParm ("clear")) != 0)
	{
		Q_strncpy (p->clear, Cmd_Argv (i+1), 16);
	}

	if ((i = Cmd_CheckParm ("startup")) != 0)
	{
		Q_strncpy (p->startup, Cmd_Argv (i+1), 32);
		p->modemInitialized = false;
	}

	if ((i = Cmd_CheckParm ("shutdown")) != 0)
	{
		Q_strncpy (p->shutdown, Cmd_Argv (i+1), 16);
	}

	if (Cmd_CheckParm ("-cts"))
	{
		p->modemStatusIgnore |= MSR_CTS;
		p->modemStatus |= MSR_CTS;
	}

	if (Cmd_CheckParm ("+cts"))
	{
		p->modemStatusIgnore &= (~MSR_CTS);
		p->modemStatus = (inportb (p->uart + MODEM_STATUS_REGISTER) & MODEM_STATUS_MASK) | p->modemStatusIgnore;
	}

	if (Cmd_CheckParm ("-dsr"))
	{
		p->modemStatusIgnore |= MSR_DSR;
		p->modemStatus |= MSR_DSR;
	}

	if (Cmd_CheckParm ("+dsr"))
	{
		p->modemStatusIgnore &= (~MSR_DSR);
		p->modemStatus = (inportb (p->uart + MODEM_STATUS_REGISTER) & MODEM_STATUS_MASK) | p->modemStatusIgnore;
	}

	if (Cmd_CheckParm ("-cd"))
	{
		p->modemStatusIgnore |= MSR_CD;
		p->modemStatus |= MSR_CD;
	}

	if (Cmd_CheckParm ("+cd"))
	{
		p->modemStatusIgnore &= (~MSR_CD);
		p->modemStatus = (inportb (p->uart + MODEM_STATUS_REGISTER) & MODEM_STATUS_MASK) | p->modemStatusIgnore;
	}

	if (Cmd_CheckParm ("enable"))
	{
		if (!p->enabled)
			ComPort_Enable(p);
		if (p->useModem && !p->modemInitialized)
			Modem_Init (p);
	}
}


int TTY_Init(void)
{
	int		n;
	ComPort *p;

	for (n = 0; n < NUM_COM_PORTS; n++)
	{
		p = (ComPort *)Hunk_AllocName(sizeof(ComPort), "comport");
		if (p == NULL)
			Sys_Error("Hunk alloc failed for com port\n");
		p->next = portList;
		portList = p;
		handleToPort[n] = p;
		p->portNumber = n;
		p->dialType = 'T';
		sprintf(p->name, "com%u", n+1);
		Cmd_AddCommand (p->name, Com_f);
		ResetComPortConfig (p);
	}

	GetComPortConfig = TTY_GetComPortConfig;
	SetComPortConfig = TTY_SetComPortConfig;
	GetModemConfig = TTY_GetModemConfig;
	SetModemConfig = TTY_SetModemConfig;

	return 0;
}


void TTY_Shutdown(void)
{
	int		n;
	ComPort *p;

	for (n = 0; n < NUM_COM_PORTS; n++)
	{
		p = handleToPort[n];
		if (p->enabled)
		{
			while (p->modemConnected)
				NET_Poll();
			ComPort_Disable (p);
		}
	}
}


static int Modem_Command(ComPort *p, char *commandString)
{
	byte	b;

	if (CheckStatus (p))
		return -1;

	disable();
	p->outputQueue.head = p->outputQueue.tail = 0;
	p->inputQueue.head = p->inputQueue.tail = 0;
	enable();
	p->bufferUsed = 0;

	while (*commandString)
		ENQUEUE (p->outputQueue, *commandString++);
	ENQUEUE (p->outputQueue, '\r');

	// get the transmit rolling
	DEQUEUE (p->outputQueue, b);
	outportb(p->uart, b);

	return 0;
}


static char *Modem_Response(ComPort *p)
{
	byte	b;

	if (CheckStatus (p))
		return NULL;

	while (! EMPTY(p->inputQueue))
	{
		DEQUEUE (p->inputQueue, b);

		if (p->bufferUsed == (sizeof(p->buffer) - 1))
			b = '\r';

		if (b == '\r' && p->bufferUsed)
		{
			p->buffer[p->bufferUsed] = 0;
			Con_Printf("%s\n", p->buffer);
			SCR_UpdateScreen ();
			p->bufferUsed = 0;
			return p->buffer;
		}

		if (b < ' ' || b > 'z')
			continue;
		p->buffer[p->bufferUsed] = b;
		p->bufferUsed++;
	}

	return NULL;
}


static void Modem_Hangup2(ComPort *p);
static void Modem_Hangup3(ComPort *p);
static void Modem_Hangup4(ComPort *p);

static void Modem_Hangup(ComPort *p)
{
	Con_Printf("Hanging up modem...\n");
	disable();
	p->modemRang = false;
	p->outputQueue.head = p->outputQueue.tail = 0;
	p->inputQueue.head = p->inputQueue.tail = 0;
	outportb(p->uart + MODEM_CONTROL_REGISTER, inportb(p->uart + MODEM_CONTROL_REGISTER) & ~MCR_DTR);
	enable();
	p->poll.procedure = Modem_Hangup2;
	p->poll.arg = p;
	SchedulePollProcedure(&p->poll, 1.5);
}

static void Modem_Hangup2(ComPort *p)
{
	outportb(p->uart + MODEM_CONTROL_REGISTER, inportb(p->uart + MODEM_CONTROL_REGISTER) | MCR_DTR);
	Modem_Command(p, "+++");
	p->poll.procedure = Modem_Hangup3;
	SchedulePollProcedure(&p->poll, 1.5);
}

static void Modem_Hangup3(ComPort *p)
{
	Modem_Command(p, p->shutdown);
	p->poll.procedure = Modem_Hangup4;
	SchedulePollProcedure(&p->poll, 1.5);
}

static void Modem_Hangup4(ComPort *p)
{
	Modem_Response(p);
	Con_Printf("Hangup complete\n");
	p->modemConnected = false;
}
