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
#include "dosisms.h"

int BLASTER_GetDMAPos(void);

/*
===============================================================================
GUS SUPPORT

===============================================================================
*/

qboolean GUS_Init (void);
int GUS_GetDMAPos (void);
void GUS_Shutdown (void);


/*
===============================================================================

BLASTER SUPPORT

===============================================================================
*/

short *dma_buffer=0;
static int dma_size;
static	int dma;

static	int dsp_port;
static	int irq;
static	int low_dma;
static	int high_dma;
static	int mixer_port;
static	int mpu401_port;

int dsp_version;
int dsp_minor_version;

int timeconstant=-1;


void PrintBits (byte b)
{
	int	i;
	char	str[9];
	
	for (i=0 ; i<8 ; i++)
		str[i] = '0' + ((b & (1<<(7-i))) > 0);
		
	str[8] = 0;
	Con_Printf ("%s (%i)", str, b);
}

void SB_Info_f(void)
{
	Con_Printf ("BLASTER=%s\n", getenv("BLASTER"));
	Con_Printf("dsp version=%d.%d\n", dsp_version, dsp_minor_version);
	Con_Printf("dma=%d\n", dma);
	if (timeconstant != -1)
		Con_Printf("timeconstant=%d\n", timeconstant);
	Con_Printf("dma position:%i\n", BLASTER_GetDMAPos ());
}

// =======================================================================
// Interprets BLASTER variable
// =======================================================================

int GetBLASTER(void)
{
	char *BLASTER;
	char *param;

	BLASTER = getenv("BLASTER");
	if (!BLASTER)
		return 0;

	param = strchr(BLASTER, 'A');
	if (!param)
		param = strchr(BLASTER, 'a');
	if (!param)
		return 0;
	sscanf(param+1, "%x", &dsp_port);

	param = strchr(BLASTER, 'I');
	if (!param)
		param = strchr(BLASTER, 'i');
	if (!param)
		return 0;
	sscanf(param+1, "%d", &irq);

	param = strchr(BLASTER, 'D');
	if (!param)
		param = strchr(BLASTER, 'd');
	if (!param)
		return 0;
	sscanf(param+1, "%d", &low_dma);

	param = strchr(BLASTER, 'H');
	if (!param)
		param = strchr(BLASTER, 'h');
	if (param)
		sscanf(param+1, "%d", &high_dma);

	param = strchr(BLASTER, 'M');
	if (!param)
		param = strchr(BLASTER, 'm');
	if (param)
		sscanf(param+1, "%x", &mixer_port);
	else
		mixer_port = dsp_port;

	param = strchr(BLASTER, 'P');
	if (!param)
		param = strchr(BLASTER, 'p');
	if (param)
		sscanf(param+1, "%x", &mpu401_port);

	return 1;

}

// ==================================================================
// Resets DSP.  Returns 0 on success.
// ==================================================================

int ResetDSP(void)
{
	volatile int i;

	dos_outportb(dsp_port + 6, 1);
	for (i=65536 ; i ; i--) ;
	dos_outportb(dsp_port + 6, 0);
	for (i=65536 ; i ; i--)
	{
		if (!(dos_inportb(dsp_port + 0xe) & 0x80)) continue;
		if (dos_inportb(dsp_port + 0xa) == 0xaa) break;
	}
	if (i) return 0;
	else return 1;

}

int ReadDSP(void)
{
	while (!(dos_inportb(dsp_port+0xe)&0x80)) ;
	return dos_inportb(dsp_port+0xa);
}

void WriteDSP(int val)
{
	while ((dos_inportb(dsp_port+0xc)&0x80)) ;
	dos_outportb(dsp_port+0xc, val);
}

int ReadMixer(int addr)
{
	dos_outportb(mixer_port+4, addr);
	return dos_inportb(mixer_port+5);
}

void WriteMixer(int addr, int val)
{
	dos_outportb(mixer_port+4, addr);
	dos_outportb(mixer_port+5, val);
}

int		oldmixervalue;

/*
================
StartSB

================
*/
void StartSB(void)
{
	int		i;

// version 4.xx startup code
	if (dsp_version >= 4)
	{
		Con_Printf("Version 4 SB startup\n");
		WriteDSP(0xd1); // turn on speaker

		WriteDSP(0x41);

		WriteDSP(shm->speed>>8);
		WriteDSP(shm->speed&0xff);

		WriteDSP(0xb6);	// 16-bit output
		WriteDSP(0x30);	// stereo
		WriteDSP((shm->samples-1) & 0xff);	// # of samples - 1
		WriteDSP((shm->samples-1) >> 8);
	}
// version 3.xx startup code
	else if (dsp_version == 3)
	{
		Con_Printf("Version 3 SB startup\n");
		WriteDSP(0xd1); // turn on speaker

		oldmixervalue = ReadMixer (0xe);
		WriteMixer (0xe, oldmixervalue | 0x2);// turn on stereo

		WriteDSP(0x14);			// send one byte
		WriteDSP(0x0);
		WriteDSP(0x0);

		for (i=0 ; i<0x10000 ; i++)
			dos_inportb(dsp_port+0xe);		// ack the dsp
		
		timeconstant = 65536-(256000000/(shm->channels*shm->speed));
		WriteDSP(0x40);
		WriteDSP(timeconstant>>8);

		WriteMixer (0xe, ReadMixer(0xe) | 0x20);// turn off filter

		WriteDSP(0x48);
		WriteDSP((shm->samples-1) & 0xff);	// # of samples - 1
		WriteDSP((shm->samples-1) >> 8);

		WriteDSP(0x90); // high speed 8 bit stereo
	}
// normal speed mono
	else
	{
		Con_Printf("Version 2 SB startup\n");
		WriteDSP(0xd1); // turn on speaker

		timeconstant = 65536-(256000000/(shm->channels*shm->speed));
		WriteDSP(0x40);
		WriteDSP(timeconstant>>8);

		WriteDSP(0x48);
		WriteDSP((shm->samples-1) & 0xff);	// # of samples - 1
		WriteDSP((shm->samples-1) >> 8);

		WriteDSP(0x1c); // normal speed 8 bit mono
	}
}

static int page_reg[] = { 0x87, 0x83, 0x81, 0x82, 0x8f, 0x8b, 0x89, 0x8a };
static int addr_reg[] = { 0, 2, 4, 6, 0xc0, 0xc4, 0xc8, 0xcc };
static int count_reg[] = { 1, 3, 5, 7, 0xc2, 0xc6, 0xca, 0xce };

static int mode_reg;
static int flipflop_reg;
static int disable_reg;
static int clear_reg;

/*
================
StartDMA

================
*/
void StartDMA(void)
{
	int mode;
	int realaddr;

	realaddr = ptr2real(dma_buffer);

// use a high dma channel if specified
	if (high_dma && dsp_version >= 4)	// 8 bit snd can never use 16 bit dma
		dma = high_dma;
	else
		dma = low_dma;

	Con_Printf ("Using DMA channel %i\n", dma);

	if (dma > 3)
	{
		mode_reg = 0xd6;
		flipflop_reg = 0xd8;
		disable_reg = 0xd4;
		clear_reg = 0xdc;
	}
	else
	{
		mode_reg = 0xb;
		flipflop_reg = 0xc;
		disable_reg = 0xa;
		clear_reg = 0xe;
	}

	dos_outportb(disable_reg, dma|4);	// disable channel
	// set mode- see "undocumented pc", p.876
	mode =	(1<<6)	// single-cycle
		+(0<<5)		// address increment
		+(1<<4)		// auto-init dma
		+(2<<2)		// read
		+(dma&3);	// channel #
	dos_outportb(mode_reg, mode);
	
// set address
	// set page
	dos_outportb(page_reg[dma], realaddr >> 16);

	if (dma > 3)
	{	// address is in words
		dos_outportb(flipflop_reg, 0);		// prepare to send 16-bit value
		dos_outportb(addr_reg[dma], (realaddr>>1) & 0xff);
		dos_outportb(addr_reg[dma], (realaddr>>9) & 0xff);

		dos_outportb(flipflop_reg, 0);		// prepare to send 16-bit value
		dos_outportb(count_reg[dma], ((dma_size>>1)-1) & 0xff);
		dos_outportb(count_reg[dma], ((dma_size>>1)-1) >> 8);
	}
	else
	{	// address is in bytes
		dos_outportb(flipflop_reg, 0);		// prepare to send 16-bit value
		dos_outportb(addr_reg[dma], realaddr & 0xff);
		dos_outportb(addr_reg[dma], (realaddr>>8) & 0xff);

		dos_outportb(flipflop_reg, 0);		// prepare to send 16-bit value
		dos_outportb(count_reg[dma], (dma_size-1) & 0xff);
		dos_outportb(count_reg[dma], (dma_size-1) >> 8);
	}

	dos_outportb(clear_reg, 0);		// clear write mask
	dos_outportb(disable_reg, dma&~4);
}


/*
==================
BLASTER_Init

Returns false if nothing is found.
==================
*/
qboolean BLASTER_Init(void)
{
	int 	size;
	int 	realaddr;
	int 	rc;
	int		p;
	
	shm = 0;
	rc = 0;

//
// must have a blaster variable set
//
	if (!GetBLASTER())
	{
		Con_NotifyBox (
		"The BLASTER environment variable\n"
		"is not set, sound effects are\n"
		"disabled.  See README.TXT for help.\n"
		);			
		return 0;
	}

	if (ResetDSP())
	{
		Con_Printf("Could not reset SB");
		return 0;
	}

//
// get dsp version
//
	WriteDSP(0xe1);
	dsp_version = ReadDSP();
	dsp_minor_version = ReadDSP();

// we need at least v2 for auto-init dma
	if (dsp_version < 2)
	{
		Con_Printf ("Sound blaster must be at least v2.0\n");
		return 0;
	}

// allow command line parm to set quality down
	p = COM_CheckParm ("-dsp");
	if (p && p < com_argc - 1)
	{
		p = Q_atoi (com_argv[p+1]);
		if (p < 2 || p > 4)
			Con_Printf ("-dsp parameter can only be 2, 3, or 4\n");
		else if (p > dsp_version)
			Con_Printf ("Can't -dsp %i on v%i hardware\n", p, dsp_version);
		else
			dsp_version = p;
	}	


// everyone does 11khz sampling rate unless told otherwise
	shm = &sn;
	shm->speed = 11025;
	rc = COM_CheckParm("-sspeed");
	if (rc)
		shm->speed = Q_atoi(com_argv[rc+1]);

// version 4 cards (sb 16) do 16 bit stereo
	if (dsp_version >= 4)
	{
		shm->channels = 2;
		shm->samplebits = 16;
	}
// version 3 cards (sb pro) do 8 bit stereo
	else if (dsp_version == 3)
	{
		shm->channels = 2;
		shm->samplebits = 8;	
	}
// v2 cards do 8 bit mono
	else
	{
		shm->channels = 1;
		shm->samplebits = 8;	
	}

	
	Cmd_AddCommand("sbinfo", SB_Info_f);
	size = 4096;

// allocate 8k and get a 4k-aligned buffer from it
	dma_buffer = dos_getmemory(size*2);
	if (!dma_buffer)
	{
		Con_Printf("Couldn't allocate sound dma buffer");
		return false;
	}

	realaddr = ptr2real(dma_buffer);
	realaddr = (realaddr + size) & ~(size-1);
	dma_buffer = (short *) real2ptr(realaddr);
	dma_size = size;

	memset(dma_buffer, 0, dma_size);

	shm->soundalive = true;
	shm->splitbuffer = false;

	shm->samples = size/(shm->samplebits/8);
	shm->samplepos = 0;
	shm->submission_chunk = 1;
	shm->buffer = (unsigned char *) dma_buffer;
	shm->samples = size/(shm->samplebits/8);

	StartDMA();
	StartSB();

	return true;
}


/*
==============
BLASTER_GetDMAPos

return the current sample position (in mono samples read)
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up.
===============
*/
int BLASTER_GetDMAPos(void)
{
	int count;

// this function is called often.  acknowledge the transfer completions
// all the time so that it loops
	if (dsp_version >= 4)
		dos_inportb(dsp_port+0xf);	// 16 bit audio
	else
		dos_inportb(dsp_port+0xe);	// 8 bit audio

// clear 16-bit reg flip-flop
// load the current dma count register
	if (dma < 4)
	{
		dos_outportb(0xc, 0);
		count = dos_inportb(dma*2+1);
		count += dos_inportb(dma*2+1) << 8;
		if (shm->samplebits == 16)
			count /= 2;
		count = shm->samples - (count+1);
	}
	else
	{
		dos_outportb(0xd8, 0);
		count = dos_inportb(0xc0+(dma-4)*4+2);
		count += dos_inportb(0xc0+(dma-4)*4+2) << 8;
		if (shm->samplebits == 8)
			count *= 2;
		count = shm->samples - (count+1);
	}

//	Con_Printf("DMA pos = 0x%x\n", count);

	shm->samplepos = count & (shm->samples-1);
	return shm->samplepos;

}

/*
==============
BLASTER_Shutdown

Reset the sound device for exiting
===============
*/
void BLASTER_Shutdown(void)
{
	if (dsp_version >= 4)
	{
	}
	else if (dsp_version == 3)
	{
		ResetDSP ();			// stop high speed mode
		WriteMixer (0xe, oldmixervalue); // turn stereo off and filter on
	}
	else
	{
	
	}
	
	WriteDSP(0xd3); // turn off speaker
	ResetDSP ();

	dos_outportb(disable_reg, dma|4);	// disable dma channel
}



/*
===============================================================================

INTERFACE

===============================================================================
*/

typedef enum
{
	dma_none,
	dma_blaster,
	dma_gus
} dmacard_t;

dmacard_t	dmacard;

/*
==================
SNDDM_Init

Try to find a sound device to mix for.
Returns false if nothing is found.
Returns true and fills in the "shm" structure with information for the mixer.
==================
*/
qboolean SNDDMA_Init(void)
{
	if (GUS_Init ())
	{
		dmacard = dma_gus;
		return true;
	}
	if (BLASTER_Init ())
	{
		dmacard = dma_blaster;
		return true;
	}
	
	dmacard = dma_none;
	
	return false;
}


/*
==============
SNDDMA_GetDMAPos

return the current sample position (in mono samples, not stereo)
inside the recirculating dma buffer, so the mixing code will know
how many sample are required to fill it up.
===============
*/
int SNDDMA_GetDMAPos(void)
{
	switch (dmacard)
	{
	case dma_blaster:
		return BLASTER_GetDMAPos ();
	case dma_gus:
		return GUS_GetDMAPos ();
	case dma_none:
		break;
	}
	
	return 0;
}

/*
==============
SNDDMA_Shutdown

Reset the sound device for exiting
===============
*/
void SNDDMA_Shutdown(void)
{
	switch (dmacard)
	{
	case dma_blaster:
		BLASTER_Shutdown ();
		break;
	case dma_gus:
		GUS_Shutdown ();
		break;
	case dma_none:
		break;
	}

	dmacard = dma_none;
	return;
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit(void)
{
}

