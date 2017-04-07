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
//
// vid_ext.c: extended video modes
// in this implementation, VESA-specific DOS video stuff
//

// TODO: make dependencies on vid_vga.c explicit or eliminate them

#include <stdlib.h>
#include <dos.h>

#include "quakedef.h"
#include "d_local.h"
#include "dosisms.h"
#include "vid_dos.h"
#include <dpmi.h>

#define MODE_SUPPORTED_IN_HW	0x0001
#define COLOR_MODE				0x0008
#define GRAPHICS_MODE			0x0010
#define VGA_INCOMPATIBLE		0x0020
#define LINEAR_FRAME_BUFFER		0x0080

#define LINEAR_MODE				0x4000

#define VESA_DONT_WAIT_VSYNC	0		// when page flipping
#define VESA_WAIT_VSYNC			0x80

#define MAX_VESA_MODES			30	// we'll just take the first 30 if there
									//  are more
typedef struct {
	int			pages[3];			// either 2 or 3 is valid
	int			vesamode;			// LINEAR_MODE set if linear mode
	void		*plinearmem;		// linear address of start of frame buffer
	qboolean	vga_incompatible;
} vesa_extra_t;

static vmode_t		vesa_modes[MAX_VESA_MODES] =
	{{NULL, NULL, "    ********* VESA modes *********    "}};
static vesa_extra_t	vesa_extra[MAX_VESA_MODES];
static char			names[MAX_VESA_MODES][10];

extern regs_t regs;

static int		VID_currentpage;
static int		VID_displayedpage;
static int		*VID_pagelist;
static byte		*VID_membase;
static int		VID_banked;

typedef struct
{
	int modenum;
	int mode_attributes;
	int	winasegment;
	int	winbsegment;
	int	bytes_per_scanline; // bytes per logical scanline (+16)
	int win; // window number (A=0, B=1)
	int win_size; // window size (+6)
	int granularity; // how finely i can set the window in vid mem (+4)
	int width, height; // displayed width and height (+18, +20)
	int bits_per_pixel; // er, better be 8, 15, 16, 24, or 32 (+25)
	int bytes_per_pixel; // er, better be 1, 2, or 4
	int memory_model; // and better be 4 or 6, packed or direct color (+27)
	int num_pages; // number of complete frame buffer pages (+29)
	int red_width; // the # of bits in the red component (+31)
	int red_pos; // the bit position of the red component (+32)
	int green_width; // etc.. (+33)
	int green_pos; // (+34)
	int blue_width; // (+35)
	int blue_pos; // (+36)
	int pptr;
	int	pagesize;
	int	numpages;
} modeinfo_t;

static modeinfo_t modeinfo;

// all bytes to avoid problems with compiler field packing
typedef struct vbeinfoblock_s {
     byte			VbeSignature[4];
     byte			VbeVersion[2];
     byte			OemStringPtr[4];
     byte			Capabilities[4];
     byte			VideoModePtr[4];
     byte			TotalMemory[2];
     byte			OemSoftwareRev[2];
     byte			OemVendorNamePtr[4];
     byte			OemProductNamePtr[4];
     byte			OemProductRevPtr[4];
     byte			Reserved[222];
     byte			OemData[256];
} vbeinfoblock_t;

static int	totalvidmem;
static byte	*ppal;
qboolean	vsync_exists, de_exists;

qboolean VID_ExtraGetModeInfo(int modenum);
int VID_ExtraInitMode (viddef_t *vid, vmode_t *pcurrentmode);
void VID_ExtraSwapBuffers (viddef_t *vid, vmode_t *pcurrentmode,
	vrect_t *rects);


/*
================
VGA_BankedBeginDirectRect
================
*/
void VGA_BankedBeginDirectRect (viddef_t *lvid, struct vmode_s *pcurrentmode,
	int x, int y, byte *pbitmap, int width, int height)
{

	if (!lvid->direct)
		return;

	regs.x.ax = 0x4f05;
	regs.x.bx = 0;
	regs.x.dx = VID_displayedpage;
	dos_int86(0x10);

	VGA_BeginDirectRect (lvid, pcurrentmode, x, y, pbitmap, width, height);

	regs.x.ax = 0x4f05;
	regs.x.bx = 0;
	regs.x.dx = VID_currentpage;
	dos_int86(0x10);
}


/*
================
VGA_BankedEndDirectRect
================
*/
void VGA_BankedEndDirectRect (viddef_t *lvid, struct vmode_s *pcurrentmode,
	int x, int y, int width, int height)
{

	if (!lvid->direct)
		return;

	regs.x.ax = 0x4f05;
	regs.x.bx = 0;
	regs.x.dx = VID_displayedpage;
	dos_int86(0x10);

	VGA_EndDirectRect (lvid, pcurrentmode, x, y, width, height);

	regs.x.ax = 0x4f05;
	regs.x.bx = 0;
	regs.x.dx = VID_currentpage;
	dos_int86(0x10);
}


/*
================
VID_SetVESAPalette
================
*/
void VID_SetVESAPalette (viddef_t *lvid, vmode_t *pcurrentmode,
	unsigned char *pal)
{
	int		i;
	byte	*pp;

	UNUSED(lvid);
	UNUSED(pcurrentmode);

	pp = ppal;

	for (i=0 ; i<256 ; i++)
	{
		pp[2] = pal[0] >> 2;
		pp[1] = pal[1] >> 2;
		pp[0] = pal[2] >> 2;
		pp += 4;
		pal += 3;
	}

	regs.x.ax = 0x4F09;
	regs.x.bx = 0;
	regs.x.cx = 256;
	regs.x.dx = 0;
	regs.x.es = ptr2real(ppal) >> 4;
	regs.x.di = ptr2real(ppal) & 0xf;
	dos_int86(0x10);

	if (regs.x.ax != 0x4f)
		Sys_Error ("Unable to load VESA palette\n");
}




/*
================
VID_ExtraFarToLinear
================
*/
void *VID_ExtraFarToLinear (void *ptr)
{
	int		temp;

	temp = (int)ptr;
	return real2ptr(((temp & 0xFFFF0000) >> 12) + (temp & 0xFFFF));
}


/*
================
VID_ExtraWaitDisplayEnable
================
*/
void VID_ExtraWaitDisplayEnable ()
{
	while ((inportb (0x3DA) & 0x01) == 1)
		;
}


/*
================
VID_ExtraVidLookForState
================
*/
qboolean VID_ExtraVidLookForState (unsigned state, unsigned mask)
{
	int		i;
	double	starttime, time;

	starttime = Sys_FloatTime ();

	do
	{
		for (i=0 ; i<100000 ; i++)
		{
			if ((inportb (0x3DA) & mask) == state)
				return true;
		}

		time = Sys_FloatTime ();
	} while ((time - starttime) < 0.1);

	return false;
}


/*
================
VID_ExtraStateFound
================
*/
qboolean VID_ExtraStateFound (unsigned state)
{
	int		i, workingstate;

	workingstate = 0;

	for (i=0 ; i<10 ; i++)
	{
		if (!VID_ExtraVidLookForState(workingstate, state))
		{
			return false;
		}

		workingstate ^= state;
	}

	return true;
}


/*
================
VID_InitExtra
================
*/
void VID_InitExtra (void)
{
	int				nummodes;
	short			*pmodenums;
	vbeinfoblock_t	*pinfoblock;
	__dpmi_meminfo	phys_mem_info;

	pinfoblock = dos_getmemory(sizeof(vbeinfoblock_t));

	*(long *)pinfoblock->VbeSignature = 'V' + ('B'<<8) + ('E'<<16) + ('2'<<24);

// see if VESA support is available
	regs.x.ax = 0x4f00;
	regs.x.es = ptr2real(pinfoblock) >> 4;
	regs.x.di = ptr2real(pinfoblock) & 0xf;
	dos_int86(0x10);

	if (regs.x.ax != 0x4f)
		return;		// no VESA support

	if (pinfoblock->VbeVersion[1] < 0x02)
		return;		// not VESA 2.0 or greater

	Con_Printf ("VESA 2.0 compliant adapter:\n%s\n",
				VID_ExtraFarToLinear (*(byte **)&pinfoblock->OemStringPtr[0]));

	totalvidmem = *(unsigned short *)&pinfoblock->TotalMemory[0] << 16;

	pmodenums = (short *)
			VID_ExtraFarToLinear (*(byte **)&pinfoblock->VideoModePtr[0]);

// find 8 bit modes until we either run out of space or run out of modes
	nummodes = 0;

	while ((*pmodenums != -1) && (nummodes < MAX_VESA_MODES))
	{
		if (VID_ExtraGetModeInfo (*pmodenums))
		{
			vesa_modes[nummodes].pnext = &vesa_modes[nummodes+1];
			if (modeinfo.width > 999)
			{
				if (modeinfo.height > 999)
				{
					sprintf (&names[nummodes][0], "%4dx%4d", modeinfo.width,
							 modeinfo.height);
					names[nummodes][9] = 0;
				}
				else
				{
					sprintf (&names[nummodes][0], "%4dx%3d", modeinfo.width,
							 modeinfo.height);
					names[nummodes][8] = 0;
				}
			}
			else
			{
				if (modeinfo.height > 999)
				{
					sprintf (&names[nummodes][0], "%3dx%4d", modeinfo.width,
							 modeinfo.height);
					names[nummodes][8] = 0;
				}
				else
				{
					sprintf (&names[nummodes][0], "%3dx%3d", modeinfo.width,
							 modeinfo.height);
					names[nummodes][7] = 0;
				}
			}

			vesa_modes[nummodes].name = &names[nummodes][0];
			vesa_modes[nummodes].width = modeinfo.width;
			vesa_modes[nummodes].height = modeinfo.height;
			vesa_modes[nummodes].aspect =
					((float)modeinfo.height / (float)modeinfo.width) *
					(320.0 / 240.0);
			vesa_modes[nummodes].rowbytes = modeinfo.bytes_per_scanline;
			vesa_modes[nummodes].planar = 0;
			vesa_modes[nummodes].pextradata = &vesa_extra[nummodes];
			vesa_modes[nummodes].setmode = VID_ExtraInitMode;
			vesa_modes[nummodes].swapbuffers = VID_ExtraSwapBuffers;
			vesa_modes[nummodes].setpalette = VID_SetVESAPalette;

			if (modeinfo.mode_attributes & LINEAR_FRAME_BUFFER)
			{
			// add linear bit to mode for linear modes
				vesa_extra[nummodes].vesamode = modeinfo.modenum | LINEAR_MODE;
				vesa_extra[nummodes].pages[0] = 0;
				vesa_extra[nummodes].pages[1] = modeinfo.pagesize;
				vesa_extra[nummodes].pages[2] = modeinfo.pagesize * 2;
				vesa_modes[nummodes].numpages = modeinfo.numpages;

				vesa_modes[nummodes].begindirectrect = VGA_BeginDirectRect;
				vesa_modes[nummodes].enddirectrect = VGA_EndDirectRect;

				phys_mem_info.address = (int)modeinfo.pptr;
				phys_mem_info.size = 0x400000;

				if (__dpmi_physical_address_mapping(&phys_mem_info))
					goto NextMode;

				vesa_extra[nummodes].plinearmem =
						 real2ptr (phys_mem_info.address);
			}
			else
			{
			// banked at 0xA0000
				vesa_extra[nummodes].vesamode = modeinfo.modenum;
				vesa_extra[nummodes].pages[0] = 0;
				vesa_extra[nummodes].plinearmem =
						real2ptr(modeinfo.winasegment<<4);

				vesa_modes[nummodes].begindirectrect =
						VGA_BankedBeginDirectRect;
				vesa_modes[nummodes].enddirectrect = VGA_BankedEndDirectRect;
				vesa_extra[nummodes].pages[1] = modeinfo.pagesize;
				vesa_extra[nummodes].pages[2] = modeinfo.pagesize * 2;
				vesa_modes[nummodes].numpages = modeinfo.numpages;
			}

			vesa_extra[nummodes].vga_incompatible =
					modeinfo.mode_attributes & VGA_INCOMPATIBLE;

			nummodes++;
		}
NextMode:
		pmodenums++;
	}

// add the VESA modes at the start of the mode list (if there are any)
	if (nummodes)
	{
		vesa_modes[nummodes-1].pnext = pvidmodes;
		pvidmodes = &vesa_modes[0];
		numvidmodes += nummodes;
		ppal = dos_getmemory(256*4);
	}

	dos_freememory(pinfoblock);
}


/*
================
VID_ExtraGetModeInfo
================
*/
qboolean VID_ExtraGetModeInfo(int modenum)
{
	char	*infobuf;
	int		numimagepages;

	infobuf = dos_getmemory(256);

	regs.x.ax = 0x4f01;
	regs.x.cx = modenum;
	regs.x.es = ptr2real(infobuf) >> 4;
	regs.x.di = ptr2real(infobuf) & 0xf;
	dos_int86(0x10);
	if (regs.x.ax != 0x4f)
	{
		return false;
	}
	else
	{
		modeinfo.modenum = modenum;
		modeinfo.bits_per_pixel = *(char*)(infobuf+25);
		modeinfo.bytes_per_pixel = (modeinfo.bits_per_pixel+1)/8;
		modeinfo.width = *(short*)(infobuf+18);
		modeinfo.height = *(short*)(infobuf+20);

	// we do only 8-bpp in software
		if ((modeinfo.bits_per_pixel != 8) ||
			(modeinfo.bytes_per_pixel != 1) ||
			(modeinfo.width > MAXWIDTH) ||
			(modeinfo.height > MAXHEIGHT))
		{
			return false;
		}

		modeinfo.mode_attributes = *(short*)infobuf;

	// we only want color graphics modes that are supported by the hardware
		if ((modeinfo.mode_attributes &
			 (MODE_SUPPORTED_IN_HW | COLOR_MODE | GRAPHICS_MODE)) !=
			(MODE_SUPPORTED_IN_HW | COLOR_MODE | GRAPHICS_MODE))
		{
			return false;
		}

	// we only work with linear frame buffers, except for 320x200, which can
	// effectively be linear when banked at 0xA000
		if (!(modeinfo.mode_attributes & LINEAR_FRAME_BUFFER))
		{
			if ((modeinfo.width != 320) || (modeinfo.height != 200))
				return false;
		}

		modeinfo.bytes_per_scanline = *(short*)(infobuf+16);

		modeinfo.pagesize = modeinfo.bytes_per_scanline * modeinfo.height;

		if (modeinfo.pagesize > totalvidmem)
			return false;

	// force to one page if the adapter reports it doesn't support more pages
	// than that, no matter how much memory it has--it may not have hardware
	// support for page flipping
		numimagepages = *(unsigned char *)(infobuf+29);

		if (numimagepages <= 0)
		{
		// wrong, but there seems to be an ATI VESA driver that reports 0
			modeinfo.numpages = 1;
		}
		else if (numimagepages < 3)
		{
			modeinfo.numpages = numimagepages;
		}
		else
		{
			modeinfo.numpages = 3;
		}

		if (*(char*)(infobuf+2) & 5)
		{
			modeinfo.winasegment = *(unsigned short*)(infobuf+8);
			modeinfo.win = 0;
		}
		else if (*(char*)(infobuf+3) & 5)
		{
			modeinfo.winbsegment = *(unsigned short*)(infobuf+8);
			modeinfo.win = 1;
		}
		modeinfo.granularity = *(short*)(infobuf+4) * 1024;
		modeinfo.win_size = *(short*)(infobuf+6) * 1024;
		modeinfo.bits_per_pixel = *(char*)(infobuf+25);
		modeinfo.bytes_per_pixel = (modeinfo.bits_per_pixel+1)/8;
		modeinfo.memory_model = *(unsigned char*)(infobuf+27);
		modeinfo.num_pages = *(char*)(infobuf+29) + 1;

		modeinfo.red_width = *(char*)(infobuf+31);
		modeinfo.red_pos = *(char*)(infobuf+32);
		modeinfo.green_width = *(char*)(infobuf+33);
		modeinfo.green_pos = *(char*)(infobuf+34);
		modeinfo.blue_width = *(char*)(infobuf+35);
		modeinfo.blue_pos = *(char*)(infobuf+36);

		modeinfo.pptr = *(long *)(infobuf+40);

#if 0
		printf("VID: (VESA) info for mode 0x%x\n", modeinfo.modenum);
		printf("  mode attrib = 0x%0x\n", modeinfo.mode_attributes);
		printf("  win a attrib = 0x%0x\n", *(unsigned char*)(infobuf+2));
		printf("  win b attrib = 0x%0x\n", *(unsigned char*)(infobuf+3));
		printf("  win a seg 0x%0x\n", (int) modeinfo.winasegment);
		printf("  win b seg 0x%0x\n", (int) modeinfo.winbsegment);
		printf("  bytes per scanline = %d\n",
				modeinfo.bytes_per_scanline);
		printf("  width = %d, height = %d\n", modeinfo.width,
				modeinfo.height);
		printf("  win = %c\n", 'A' + modeinfo.win);
		printf("  win granularity = %d\n", modeinfo.granularity);
		printf("  win size = %d\n", modeinfo.win_size);
		printf("  bits per pixel = %d\n", modeinfo.bits_per_pixel);
		printf("  bytes per pixel = %d\n", modeinfo.bytes_per_pixel);
		printf("  memory model = 0x%x\n", modeinfo.memory_model);
		printf("  num pages = %d\n", modeinfo.num_pages);
		printf("  red width = %d\n", modeinfo.red_width);
		printf("  red pos = %d\n", modeinfo.red_pos);
		printf("  green width = %d\n", modeinfo.green_width);
		printf("  green pos = %d\n", modeinfo.green_pos);
		printf("  blue width = %d\n", modeinfo.blue_width);
		printf("  blue pos = %d\n", modeinfo.blue_pos);
		printf("  phys mem = %x\n", modeinfo.pptr);
#endif
	}

	dos_freememory(infobuf);

	return true;
}


/*
================
VID_ExtraInitMode
================
*/
int VID_ExtraInitMode (viddef_t *lvid, vmode_t *pcurrentmode)
{
	vesa_extra_t	*pextra;
	int				pageoffset;

	pextra = pcurrentmode->pextradata;

	if (vid_nopageflip.value)
		lvid->numpages = 1;
	else
		lvid->numpages = pcurrentmode->numpages;

// clean up any old vid buffer lying around, alloc new if needed
	if (!VGA_FreeAndAllocVidbuffer (lvid, lvid->numpages == 1))
		return -1;	// memory alloc failed

// clear the screen and wait for the next frame. VGA_pcurmode, which
// VGA_ClearVideoMem relies on, is guaranteed to be set because mode 0 is
// always the first mode set in a session
	if (VGA_pcurmode)
		VGA_ClearVideoMem (VGA_pcurmode->planar);

// set the mode
	regs.x.ax = 0x4f02;
	regs.x.bx = pextra->vesamode;
	dos_int86(0x10);

	if (regs.x.ax != 0x4f)
		return 0;

	VID_banked = !(pextra->vesamode & LINEAR_MODE);
	VID_membase = pextra->plinearmem;
	VGA_width = lvid->width;
	VGA_height = lvid->height;
	VGA_rowbytes = lvid->rowbytes;

	lvid->colormap = host_colormap;

	VID_pagelist = &pextra->pages[0];

// wait for display enable by default only when triple-buffering on a VGA-
// compatible machine that actually has a functioning display enable status
	vsync_exists = VID_ExtraStateFound (0x08);
	de_exists = VID_ExtraStateFound (0x01);

	if (!pextra->vga_incompatible  &&
		(lvid->numpages == 3)      &&
		de_exists                  &&
		(_vid_wait_override.value == 0.0))
	{
		Cvar_SetValue ("vid_wait", (float)VID_WAIT_DISPLAY_ENABLE);

		VID_displayedpage = 0;
		VID_currentpage = 1;
	}
	else
	{
		if ((lvid->numpages == 1) && (_vid_wait_override.value == 0.0))
		{
			Cvar_SetValue ("vid_wait", (float)VID_WAIT_NONE);
			VID_displayedpage = VID_currentpage = 0;
		}
		else
		{
			Cvar_SetValue ("vid_wait", (float)VID_WAIT_VSYNC);

			VID_displayedpage = 0;

			if (lvid->numpages > 1)
				VID_currentpage = 1;
			else
				VID_currentpage = 0;
		}
	}

// TODO: really should be a call to a function
	pageoffset = VID_pagelist[VID_displayedpage];

	regs.x.ax = 0x4f07;
	regs.x.bx = 0x80;	// wait for vsync so we know page 0 is visible
	regs.x.cx = pageoffset % VGA_rowbytes;
	regs.x.dx = pageoffset / VGA_rowbytes;
	dos_int86(0x10);

	if (VID_banked)
	{
		regs.x.ax = 0x4f05;
		regs.x.bx = 0;
		regs.x.dx = VID_currentpage;
		dos_int86(0x10);

		VGA_pagebase = VID_membase;
	}
	else
	{
		VGA_pagebase = VID_membase + VID_pagelist[VID_currentpage];
	}

	if (lvid->numpages > 1)
	{
		lvid->buffer = VGA_pagebase;
		lvid->conbuffer = lvid->buffer;
	}
	else
	{
		lvid->rowbytes = lvid->width;
	}

	lvid->direct = VGA_pagebase;
	lvid->conrowbytes = lvid->rowbytes;
	lvid->conwidth = lvid->width;
	lvid->conheight = lvid->height;

	lvid->maxwarpwidth = WARP_WIDTH;
	lvid->maxwarpheight = WARP_HEIGHT;

	VGA_pcurmode = pcurrentmode;

	D_InitCaches (vid_surfcache, vid_surfcachesize);

	return 1;
}


/*
================
VID_ExtraSwapBuffers
================
*/
void VID_ExtraSwapBuffers (viddef_t *lvid, vmode_t *pcurrentmode,
	vrect_t *rects)
{
	int	pageoffset;

	UNUSED(rects);
	UNUSED(pcurrentmode);

	pageoffset = VID_pagelist[VID_currentpage];

// display the newly finished page
	if (lvid->numpages > 1)
	{
	// page flipped
		regs.x.ax = 0x4f07;
	
		if (vid_wait.value != VID_WAIT_VSYNC)
		{
			if ((vid_wait.value == VID_WAIT_DISPLAY_ENABLE) && de_exists)
				VID_ExtraWaitDisplayEnable ();
	
			regs.x.bx = VESA_DONT_WAIT_VSYNC;
		}
		else
		{
			regs.x.bx = VESA_WAIT_VSYNC;	// double buffered has to wait
		}

		regs.x.cx = pageoffset % VGA_rowbytes;
		regs.x.dx = pageoffset / VGA_rowbytes;
		dos_int86(0x10);
	
		VID_displayedpage = VID_currentpage;
		if (++VID_currentpage >= lvid->numpages)
			VID_currentpage = 0;
	
	//
	// set the new write window if this is a banked mode; otherwise, set the
	// new address to which to write
	//
		if (VID_banked)
		{
			regs.x.ax = 0x4f05;
			regs.x.bx = 0;
			regs.x.dx = VID_currentpage;
			dos_int86(0x10);
		}
		else
		{
			lvid->direct = lvid->buffer;	// direct drawing goes to the
											//  currently displayed page
			lvid->buffer = VID_membase + VID_pagelist[VID_currentpage];
			lvid->conbuffer = lvid->buffer;
		}
	
		VGA_pagebase = lvid->buffer;
	}
	else
	{
	// non-page-flipped
		if (vsync_exists && (vid_wait.value == VID_WAIT_VSYNC))
		{
			VGA_WaitVsync ();
		}

		while (rects)
		{
			VGA_UpdateLinearScreen (
					lvid->buffer + rects->x + (rects->y * lvid->rowbytes),
		 			VGA_pagebase + rects->x + (rects->y * VGA_rowbytes),
					rects->width,
					rects->height,
					lvid->rowbytes,
					VGA_rowbytes);

			rects = rects->pnext;
		}
	}
}

