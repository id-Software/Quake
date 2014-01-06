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
// Quake is a trademark of Id Software, Inc., (c) 1996 Id Software, Inc. All
// rights reserved.

#include <dpmi.h>
#include "quakedef.h"
#include "dosisms.h"

extern	cvar_t	bgmvolume;

#define ADDRESS_MODE_HSG		0
#define ADDRESS_MODE_RED_BOOK	1

#define STATUS_ERROR_BIT	0x8000
#define STATUS_BUSY_BIT		0x0200
#define STATUS_DONE_BIT		0x0100
#define STATUS_ERROR_MASK	0x00ff

#define ERROR_WRITE_PROTECT		0
#define ERROR_UNKNOWN_UNIT		1
#define ERROR_DRIVE_NOT_READY	2
#define ERROR_UNKNOWN_COMMAND	3
#define ERROR_CRC_ERROR			4
#define ERROR_BAD_REQUEST_LEN	5
#define ERROR_SEEK_ERROR		6
#define ERROR_UNKNOWN_MEDIA		7
#define ERROR_SECTOR_NOT_FOUND	8
#define ERROR_OUT_OF_PAPER		9
#define ERROR_WRITE_FAULT		10
#define ERROR_READ_FAULT		11
#define ERROR_GENERAL_FAILURE	12
#define ERROR_RESERVED_13		13
#define ERROR_RESERVED_14		14
#define ERROR_BAD_DISK_CHANGE	15

#define COMMAND_READ			3
#define COMMAND_WRITE			12
#define COMMAND_PLAY_AUDIO		132
#define COMMAND_STOP_AUDIO		133
#define COMMAND_RESUME_AUDIO	136

#define READ_REQUEST_AUDIO_CHANNEL_INFO		4
#define READ_REQUEST_DEVICE_STATUS			6
#define READ_REQUEST_MEDIA_CHANGE			9
#define READ_REQUEST_AUDIO_DISK_INFO		10
#define READ_REQUEST_AUDIO_TRACK_INFO		11
#define READ_REQUEST_AUDIO_STATUS			15

#define WRITE_REQUEST_EJECT					0
#define WRITE_REQUEST_RESET					2
#define WRITE_REQUEST_AUDIO_CHANNEL_INFO	3

#define STATUS_DOOR_OPEN					0x00000001
#define STATUS_DOOR_UNLOCKED				0x00000002
#define STATUS_RAW_SUPPORT					0x00000004
#define STATUS_READ_WRITE					0x00000008
#define STATUS_AUDIO_SUPPORT				0x00000010
#define STATUS_INTERLEAVE_SUPPORT			0x00000020
#define STATUS_BIT_6_RESERVED				0x00000040
#define STATUS_PREFETCH_SUPPORT				0x00000080
#define STATUS_AUDIO_MANIPLUATION_SUPPORT	0x00000100
#define STATUS_RED_BOOK_ADDRESS_SUPPORT		0x00000200

#define MEDIA_NOT_CHANGED		1
#define MEDIA_STATUS_UNKNOWN	0
#define MEDIA_CHANGED			-1

#define AUDIO_CONTROL_MASK				0xd0
#define AUDIO_CONTROL_DATA_TRACK		0x40
#define AUDIO_CONTROL_AUDIO_2_TRACK		0x00
#define AUDIO_CONTROL_AUDIO_2P_TRACK	0x10
#define AUDIO_CONTROL_AUDIO_4_TRACK		0x80
#define AUDIO_CONTROL_AUDIO_4P_TRACK	0x90

#define AUDIO_STATUS_PAUSED				0x0001

#pragma pack(1)

struct playAudioRequest
{
	char	addressingMode;
	int		startLocation;
	int		sectors;
};

struct readRequest
{
	char	mediaDescriptor;
	short	bufferOffset;
	short	bufferSegment;
	short	length;
	short	startSector;
	int		volumeID;
};

struct writeRequest
{
	char	mediaDescriptor;
	short	bufferOffset;
	short	bufferSegment;
	short	length;
	short	startSector;
	int		volumeID;
};

struct cd_request
{
	char	headerLength;
	char	unit;
	char	command;
	short	status;
	char	reserved[8];
	union
	{
		struct	playAudioRequest	playAudio;
		struct	readRequest			read;
		struct	writeRequest		write;
	} x;
};


struct audioChannelInfo_s
{
	char	code;
	char	channel0input;
	char	channel0volume;
	char	channel1input;
	char	channel1volume;
	char	channel2input;
	char	channel2volume;
	char	channel3input;
	char	channel3volume;
};

struct deviceStatus_s
{
	char	code;
	int		status;
};

struct mediaChange_s
{
	char	code;
	char	status;
};

struct audioDiskInfo_s
{
	char	code;
	char	lowTrack;
	char	highTrack;
	int		leadOutStart;
};

struct audioTrackInfo_s
{
	char	code;
	char	track;
	int		start;
	char	control;
};

struct audioStatus_s
{
	char	code;
	short	status;
	int		PRstartLocation;
	int		PRendLocation;
};

struct reset_s
{
	char	code;
};

union readInfo_u
{
	struct audioChannelInfo_s	audioChannelInfo;
	struct deviceStatus_s		deviceStatus;
	struct mediaChange_s		mediaChange;
	struct audioDiskInfo_s		audioDiskInfo;
	struct audioTrackInfo_s		audioTrackInfo;
	struct audioStatus_s		audioStatus;
	struct reset_s				reset;
};

#pragma pack()

#define MAXIMUM_TRACKS			100

typedef struct
{
	int			start;
	int			length;
	qboolean	isData;
} track_info;

typedef struct
{
	qboolean	valid;
	int			leadOutAddress;
	track_info	track[MAXIMUM_TRACKS];
	byte		lowTrack;
	byte		highTrack;
} cd_info;

static struct cd_request	*cdRequest;
static union readInfo_u		*readInfo;
static cd_info				cd;

static qboolean	playing = false;
static qboolean	wasPlaying = false;
static qboolean	mediaCheck = false;
static qboolean	initialized = false;
static qboolean	enabled = true;
static qboolean playLooping = false;
static short	cdRequestSegment;
static short	cdRequestOffset;
static short	readInfoSegment;
static short	readInfoOffset;
static byte 	remap[256];
static byte		cdrom;
static byte		playTrack;
static byte		cdvolume;


static int RedBookToSector(int rb)
{
	byte	minute;
	byte	second;
	byte	frame;

	minute = (rb >> 16) & 0xff;
	second = (rb >> 8) & 0xff;
	frame = rb & 0xff;
	return minute * 60 * 75 + second * 75 + frame;
}


static void CDAudio_Reset(void)
{
	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_WRITE;
	cdRequest->status = 0;

	cdRequest->x.write.mediaDescriptor = 0;
	cdRequest->x.write.bufferOffset = readInfoOffset;
	cdRequest->x.write.bufferSegment = readInfoSegment;
	cdRequest->x.write.length = sizeof(struct reset_s);
	cdRequest->x.write.startSector = 0;
	cdRequest->x.write.volumeID = 0;

	readInfo->reset.code = WRITE_REQUEST_RESET;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);
}


static void CDAudio_Eject(void)
{
	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_WRITE;
	cdRequest->status = 0;

	cdRequest->x.write.mediaDescriptor = 0;
	cdRequest->x.write.bufferOffset = readInfoOffset;
	cdRequest->x.write.bufferSegment = readInfoSegment;
	cdRequest->x.write.length = sizeof(struct reset_s);
	cdRequest->x.write.startSector = 0;
	cdRequest->x.write.volumeID = 0;

	readInfo->reset.code = WRITE_REQUEST_EJECT;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);
}


static int CDAudio_GetAudioTrackInfo(byte track, int *start)
{
	byte	control;

	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_READ;
	cdRequest->status = 0;

	cdRequest->x.read.mediaDescriptor = 0;
	cdRequest->x.read.bufferOffset = readInfoOffset;
	cdRequest->x.read.bufferSegment = readInfoSegment;
	cdRequest->x.read.length = sizeof(struct audioTrackInfo_s);
	cdRequest->x.read.startSector = 0;
	cdRequest->x.read.volumeID = 0;

	readInfo->audioTrackInfo.code = READ_REQUEST_AUDIO_TRACK_INFO;
	readInfo->audioTrackInfo.track = track;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	if (cdRequest->status & STATUS_ERROR_BIT)
	{
		Con_DPrintf("CDAudio_GetAudioTrackInfo %04x\n", cdRequest->status & 	0xffff);
		return -1;
	}

	*start = readInfo->audioTrackInfo.start;
	control = readInfo->audioTrackInfo.control & AUDIO_CONTROL_MASK;
	return (control & AUDIO_CONTROL_DATA_TRACK);
}


static int CDAudio_GetAudioDiskInfo(void)
{
	int n;

	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_READ;
	cdRequest->status = 0;

	cdRequest->x.read.mediaDescriptor = 0;
	cdRequest->x.read.bufferOffset = readInfoOffset;
	cdRequest->x.read.bufferSegment = readInfoSegment;
	cdRequest->x.read.length = sizeof(struct audioDiskInfo_s);
	cdRequest->x.read.startSector = 0;
	cdRequest->x.read.volumeID = 0;

	readInfo->audioDiskInfo.code = READ_REQUEST_AUDIO_DISK_INFO;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	if (cdRequest->status & STATUS_ERROR_BIT)
	{
		Con_DPrintf("CDAudio_GetAudioDiskInfo %04x\n", cdRequest->status & 	0xffff);
		return -1;
	}

	cd.valid = true;
	cd.lowTrack = readInfo->audioDiskInfo.lowTrack;
	cd.highTrack = readInfo->audioDiskInfo.highTrack;
	cd.leadOutAddress = readInfo->audioDiskInfo.leadOutStart;

	for (n = cd.lowTrack; n <= cd.highTrack; n++)
	{
		cd.track[n].isData = CDAudio_GetAudioTrackInfo (n, &cd.track[n].start);
		if (n > cd.lowTrack)
		{
			cd.track[n-1].length = RedBookToSector(cd.track[n].start) - RedBookToSector(cd.track[n-1].start);
			if (n == cd.highTrack)
				cd.track[n].length = RedBookToSector(cd.leadOutAddress) - RedBookToSector(cd.track[n].start);
		}
	}

	return 0;
}


static int CDAudio_GetAudioStatus(void)
{
	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_READ;
	cdRequest->status = 0;

	cdRequest->x.read.mediaDescriptor = 0;
	cdRequest->x.read.bufferOffset = readInfoOffset;
	cdRequest->x.read.bufferSegment = readInfoSegment;
	cdRequest->x.read.length = sizeof(struct audioStatus_s);
	cdRequest->x.read.startSector = 0;
	cdRequest->x.read.volumeID = 0;

	readInfo->audioDiskInfo.code = READ_REQUEST_AUDIO_STATUS;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	if (cdRequest->status & STATUS_ERROR_BIT)
		return -1;
	return 0;
}


static int CDAudio_MediaChange(void)
{
	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_READ;
	cdRequest->status = 0;

	cdRequest->x.read.mediaDescriptor = 0;
	cdRequest->x.read.bufferOffset = readInfoOffset;
	cdRequest->x.read.bufferSegment = readInfoSegment;
	cdRequest->x.read.length = sizeof(struct mediaChange_s);
	cdRequest->x.read.startSector = 0;
	cdRequest->x.read.volumeID = 0;

	readInfo->mediaChange.code = READ_REQUEST_MEDIA_CHANGE;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	return readInfo->mediaChange.status;
}


// we set the volume to 0 first and then to the desired volume
// some cd-rom drivers seem to need it done this way
void CDAudio_SetVolume (byte volume)
{
	if (!initialized || !enabled)
		return;

	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_WRITE;
	cdRequest->status = 0;

	cdRequest->x.read.mediaDescriptor = 0;
	cdRequest->x.read.bufferOffset = readInfoOffset;
	cdRequest->x.read.bufferSegment = readInfoSegment;
	cdRequest->x.read.length = sizeof(struct audioChannelInfo_s);
	cdRequest->x.read.startSector = 0;
	cdRequest->x.read.volumeID = 0;

	readInfo->audioChannelInfo.code = WRITE_REQUEST_AUDIO_CHANNEL_INFO;
	readInfo->audioChannelInfo.channel0input = 0;
	readInfo->audioChannelInfo.channel0volume = 0;
	readInfo->audioChannelInfo.channel1input = 1;
	readInfo->audioChannelInfo.channel1volume = 0;
	readInfo->audioChannelInfo.channel2input = 2;
	readInfo->audioChannelInfo.channel2volume = 0;
	readInfo->audioChannelInfo.channel3input = 3;
	readInfo->audioChannelInfo.channel3volume = 0;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	readInfo->audioChannelInfo.channel0volume = volume;
	readInfo->audioChannelInfo.channel1volume = volume;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	cdvolume = volume;
}


void CDAudio_Play(byte track, qboolean looping)
{
	int		volume;

	if (!initialized || !enabled)
		return;
	
	if (!cd.valid)
		return;

	track = remap[track];

	if (playing)
	{
		if (playTrack == track)
			return;
		CDAudio_Stop();
	}

	playLooping = looping;

	if (track < cd.lowTrack || track > cd.highTrack)
	{
		Con_DPrintf("CDAudio_Play: Bad track number %u.\n", track);
		return;
	}

	playTrack = track;

	if (cd.track[track].isData)
	{
		Con_DPrintf("CDAudio_Play: Can not play data.\n");
		return;
	}

	volume = (int)(bgmvolume.value * 255.0);
	if (volume < 0)
	{
		Cvar_SetValue ("bgmvolume", 0.0);
		volume = 0;
	}
	else if (volume > 255)
	{
		Cvar_SetValue ("bgmvolume", 1.0);
		volume = 255;
	}
	CDAudio_SetVolume (volume);

	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_PLAY_AUDIO;
	cdRequest->status = 0;

	cdRequest->x.playAudio.addressingMode = ADDRESS_MODE_RED_BOOK;
	cdRequest->x.playAudio.startLocation = cd.track[track].start;
	cdRequest->x.playAudio.sectors = cd.track[track].length;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	if (cdRequest->status & STATUS_ERROR_BIT)
	{
		Con_DPrintf("CDAudio_Play: track %u failed\n", track);
		cd.valid = false;
		playing = false;
		return;
	}

	playing = true;
}


void CDAudio_Stop(void)
{
	if (!initialized || !enabled)
		return;
	
	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_STOP_AUDIO;
	cdRequest->status = 0;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	wasPlaying = playing;
	playing = false;
}


void CDAudio_Pause(void)
{
	CDAudio_Stop();
}


void CDAudio_Resume(void)
{
	if (!initialized || !enabled)
		return;
	
	if (!cd.valid)
		return;

	if (!wasPlaying)
		return;
	
	cdRequest->headerLength = 13;
	cdRequest->unit = 0;
	cdRequest->command = COMMAND_RESUME_AUDIO;
	cdRequest->status = 0;

	regs.x.ax = 0x1510;
	regs.x.cx = cdrom;
	regs.x.es = cdRequestSegment;
	regs.x.bx = cdRequestOffset;
	dos_int86 (0x2f);

	playing = true;
}


static void CD_f (void)
{
	char	*command;
	int		ret;
	int		n;
	int		startAddress;

	if (Cmd_Argc() < 2)
		return;

	command = Cmd_Argv (1);

	if (Q_strcasecmp(command, "on") == 0)
	{
		enabled = true;
		return;
	}

	if (Q_strcasecmp(command, "off") == 0)
	{
		if (playing)
			CDAudio_Stop();
		enabled = false;
		return;
	}

	if (Q_strcasecmp(command, "reset") == 0)
	{
		enabled = true;
		if (playing)
			CDAudio_Stop();
		for (n = 0; n < 256; n++)
			remap[n] = n;
		CDAudio_Reset();
		CDAudio_GetAudioDiskInfo();
		return;
	}

	if (Q_strcasecmp(command, "remap") == 0)
	{
		ret = Cmd_Argc() - 2;
		if (ret <= 0)
		{
			for (n = 1; n < 256; n++)
				if (remap[n] != n)
					Con_Printf("  %u -> %u\n", n, remap[n]);
			return;
		}
		for (n = 1; n <= ret; n++)
			remap[n] = Q_atoi(Cmd_Argv (n+1));
		return;
	}

	if (!cd.valid)
	{
		Con_Printf("No CD in player.\n");
		return;
	}

	if (Q_strcasecmp(command, "play") == 0)
	{
		CDAudio_Play(Q_atoi(Cmd_Argv (2)), false);
		return;
	}

	if (Q_strcasecmp(command, "loop") == 0)
	{
		CDAudio_Play(Q_atoi(Cmd_Argv (2)), true);
		return;
	}

	if (Q_strcasecmp(command, "stop") == 0)
	{
		CDAudio_Stop();
		return;
	}

	if (Q_strcasecmp(command, "pause") == 0)
	{
		CDAudio_Pause();
		return;
	}

	if (Q_strcasecmp(command, "resume") == 0)
	{
		CDAudio_Resume();
		return;
	}

	if (Q_strcasecmp(command, "eject") == 0)
	{
		if (playing)
			CDAudio_Stop();
		CDAudio_Eject();
		cd.valid = false;
		return;
	}

	if (Q_strcasecmp(command, "info") == 0)
	{
		Con_Printf("%u tracks\n", cd.highTrack - cd.lowTrack + 1);
		for (n = cd.lowTrack; n <= cd.highTrack; n++)
		{
			ret = CDAudio_GetAudioTrackInfo (n, &startAddress);
			Con_Printf("Track %2u: %s at %2u:%02u\n", n, ret ? "data " : "music", (startAddress >> 16) & 0xff, (startAddress >> 8) & 0xff);
		}
		if (playing)
			Con_Printf("Currently %s track %u\n", playLooping ? "looping" : "playing", playTrack);
		Con_Printf("Volume is %u\n", cdvolume);
		CDAudio_MediaChange();
		Con_Printf("Status %04x\n", cdRequest->status & 0xffff);
		return;
	}
}


void CDAudio_Update(void)
{
	int		ret;
	int		newVolume;
	static	double lastUpdate;

	if (!initialized || !enabled)
		return;

	if ((realtime - lastUpdate) < 0.25)
		return;
	lastUpdate = realtime;

	if (mediaCheck)
	{
		static	double lastCheck;

		if ((realtime - lastCheck) < 5.0)
			return;
		lastCheck = realtime;

		ret = CDAudio_MediaChange();
		if (ret == MEDIA_CHANGED)
		{
			Con_DPrintf("CDAudio: media changed\n");
			playing = false;
			wasPlaying = false;
			cd.valid = false;
			CDAudio_GetAudioDiskInfo();
			return;
		}
	}

	newVolume = (int)(bgmvolume.value * 255.0);
	if (newVolume != cdvolume)
	{
		if (newVolume < 0)
		{
			Cvar_SetValue ("bgmvolume", 0.0);
			newVolume = 0;
		}
		else if (newVolume > 255)
		{
			Cvar_SetValue ("bgmvolume", 1.0);
			newVolume = 255;
		}
		CDAudio_SetVolume (newVolume);
	}

	if (playing)
	{
		CDAudio_GetAudioStatus();
		if ((cdRequest->status & STATUS_BUSY_BIT) == 0)
		{
			playing = false;
			if (playLooping)
				CDAudio_Play(playTrack, true);
		}
	}
}


int CDAudio_Init(void)
{
	char	*memory;
	int		n;

	if (cls.state == ca_dedicated)
		return -1;

	if (COM_CheckParm("-nocdaudio"))
		return -1;

	if (COM_CheckParm("-cdmediacheck"))
		mediaCheck = true;

	regs.x.ax = 0x1500;
	regs.x.bx = 0;
	dos_int86 (0x2f);
	if (regs.x.bx == 0)
	{
		Con_NotifyBox (
			"MSCDEX not loaded, music is\n"
			"disabled.  Use \"-nocdaudio\" if you\n"
			"wish to avoid this message in the\n"
			"future.  See README.TXT for help.\n"
			);			
		return -1;
	}
	if (regs.x.bx > 1)
		Con_DPrintf("CDAudio_Init: First CD-ROM drive will be used\n");
	cdrom = regs.x.cx;

	regs.x.ax = 0x150c;
	regs.x.bx = 0;
	dos_int86 (0x2f);
	if (regs.x.bx == 0)
	{
		Con_NotifyBox (
			"MSCDEX version 2.00 or later\n"
			"required for music. See README.TXT\n"
			"for help.\n"
			);			
		Con_DPrintf("CDAudio_Init: MSCDEX version 2.00 or later required.\n");
		return -1;
	}

	memory = dos_getmemory(sizeof(struct cd_request
) + sizeof(union readInfo_u));
	if (memory == NULL)
	{
		Con_DPrintf("CDAudio_Init: Unable to allocate low memory.\n");
		return -1;
	}

	cdRequest = (struct cd_request *)memory;
	cdRequestSegment = ptr2real(cdRequest) >> 4;
	cdRequestOffset = ptr2real(cdRequest) & 0xf;

	readInfo = (union readInfo_u *)(memory + sizeof(struct cd_request));
	readInfoSegment = ptr2real(readInfo) >> 4;
	readInfoOffset = ptr2real(readInfo) & 0xf;

	for (n = 0; n < 256; n++)
		remap[n] = n;
	initialized = true;

	CDAudio_SetVolume (255);
	if (CDAudio_GetAudioDiskInfo())
	{
		Con_Printf("CDAudio_Init: No CD in player.\n");
		enabled = false;
	}

	Cmd_AddCommand ("cd", CD_f);

	Con_Printf("CD Audio Initialized\n");

	return 0;
}


void CDAudio_Shutdown(void)
{
	if (!initialized)
		return;
	CDAudio_Stop();
}
