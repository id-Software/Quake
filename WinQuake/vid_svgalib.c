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
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <stdarg.h>
#include <stdio.h>
#include <signal.h>

#include <asm/io.h>

#include "vga.h"
#include "vgakeyboard.h"
#include "vgamouse.h"

#include "quakedef.h"
#include "d_local.h"

#define stringify(m) { #m, m }

unsigned short       d_8to16table[256];
static byte		*vid_surfcache;
static int		VID_highhunkmark;

int num_modes;
vga_modeinfo *modes;
int current_mode;

int num_shades=32;

struct
{
	char *name;
	int num;
} mice[] =
{
	stringify(MOUSE_MICROSOFT),
	stringify(MOUSE_MOUSESYSTEMS),
	stringify(MOUSE_MMSERIES),
	stringify(MOUSE_LOGITECH),
	stringify(MOUSE_BUSMOUSE),
	stringify(MOUSE_PS2),
};

static unsigned char scantokey[128];
static byte vid_current_palette[768];

int num_mice = sizeof (mice) / sizeof(mice[0]);

int	d_con_indirect = 0;

int		svgalib_inited=0;
int		UseMouse = 1;
int		UseDisplay = 1;
int		UseKeyboard = 1;

int		mouserate = MOUSE_DEFAULTSAMPLERATE;

cvar_t		vid_mode = {"vid_mode","5",false};
cvar_t		vid_redrawfull = {"vid_redrawfull","0",false};
cvar_t		vid_waitforrefresh = {"vid_waitforrefresh","0",true};
 
char	*framebuffer_ptr;

cvar_t  mouse_button_commands[3] =
{
    {"mouse1","+attack"},
    {"mouse2","+strafe"},
    {"mouse3","+forward"},
};

int     mouse_buttons;
int     mouse_buttonstate;
int     mouse_oldbuttonstate;
float   mouse_x, mouse_y;
float	old_mouse_x, old_mouse_y;
int		mx, my;

cvar_t	m_filter = {"m_filter","0"};

static byte     backingbuf[48*24];

int		VGA_width, VGA_height, VGA_rowbytes, VGA_bufferrowbytes, VGA_planar;
byte	*VGA_pagebase;

void VGA_UpdatePlanarScreen (void *srcbuffer);

void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
	int i, j, k, plane, reps, repshift, offset, vidpage, off;

	if (!svgalib_inited || !vid.direct || !vga_oktowrite()) return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	} else {
		reps = 1;
		repshift = 0;
	}

	vidpage = 0;
	vga_setpage(0);

	if (VGA_planar)
	{
		for (plane=0 ; plane<4 ; plane++)
		{
		// select the correct plane for reading and writing
			outb(0x02, 0x3C4);
			outb(1 << plane, 0x3C5);
			outb(4, 0x3CE);
			outb(plane, 0x3CF);

			for (i=0 ; i<(height << repshift) ; i += reps)
			{
				for (k=0 ; k<reps ; k++)
				{
					for (j=0 ; j<(width >> 2) ; j++)
					{
						backingbuf[(i + k) * 24 + (j << 2) + plane] =
								vid.direct[(y + i + k) * VGA_rowbytes +
								(x >> 2) + j];
						vid.direct[(y + i + k) * VGA_rowbytes + (x>>2) + j] =
								pbitmap[(i >> repshift) * 24 +
								(j << 2) + plane];
					}
				}
			}
		}
	} else {
		for (i=0 ; i<(height << repshift) ; i += reps)
		{
			for (j=0 ; j<reps ; j++)
			{
				offset = x + ((y << repshift) + i + j) * vid.rowbytes;
				off = offset % 0x10000;
				if ((offset / 0x10000) != vidpage) {
					vidpage=offset / 0x10000;
					vga_setpage(vidpage);
				}
				memcpy (&backingbuf[(i + j) * 24],
						vid.direct + off, width);
				memcpy (vid.direct + off,
						&pbitmap[(i >> repshift)*width], width);
			}
		}
	}
}

void D_EndDirectRect (int x, int y, int width, int height)
{
	int i, j, k, plane, reps, repshift, offset, vidpage, off;

	if (!svgalib_inited || !vid.direct || !vga_oktowrite()) return;

	if (vid.aspect > 1.5)
	{
		reps = 2;
		repshift = 1;
	} else {
		reps = 1;
		repshift = 0;
	}

	vidpage = 0;
	vga_setpage(0);

	if (VGA_planar)
	{
		for (plane=0 ; plane<4 ; plane++)
		{
		// select the correct plane for writing
			outb(2, 0x3C4);
			outb(1 << plane, 0x3C5);
			outb(4, 0x3CE);
			outb(plane, 0x3CF);

			for (i=0 ; i<(height << repshift) ; i += reps)
			{
				for (k=0 ; k<reps ; k++)
				{
					for (j=0 ; j<(width >> 2) ; j++)
					{
						vid.direct[(y + i + k) * VGA_rowbytes + (x>>2) + j] =
								backingbuf[(i + k) * 24 + (j << 2) + plane];
					}
				}
			}
		}
	} else {
		for (i=0 ; i<(height << repshift) ; i += reps)
		{
			for (j=0 ; j<reps ; j++)
			{
				offset = x + ((y << repshift) + i + j) * vid.rowbytes;
				off = offset % 0x10000;
				if ((offset / 0x10000) != vidpage) {
					vidpage=offset / 0x10000;
					vga_setpage(vidpage);
				}
				memcpy (vid.direct + off, 
						&backingbuf[(i +j)*24],
						width);
			}
		}
	}
}

/*
=================
VID_Gamma_f

Keybinding command
=================
*/
void VID_Gamma_f (void)
{
	float	gamma, f, inf;
	unsigned char	palette[768];
	int		i;

	if (Cmd_Argc () == 2)
	{
		gamma = Q_atof (Cmd_Argv(1));

		for (i=0 ; i<768 ; i++)
		{
			f = pow ( (host_basepal[i]+1)/256.0 , gamma );
			inf = f*255 + 0.5;
			if (inf < 0)
				inf = 0;
			if (inf > 255)
				inf = 255;
			palette[i] = inf;
		}

		VID_SetPalette (palette);

		vid.recalc_refdef = 1;				// force a surface cache flush
	}
}

void VID_DescribeMode_f (void)
{
	int modenum;
	
	modenum = Q_atoi (Cmd_Argv(1));
	if ((modenum >= num_modes) || (modenum < 0 ) || !modes[modenum].width)
		Con_Printf("Invalid video mode: %d!\n",modenum);
	Con_Printf("%d: %d x %d - ",modenum,modes[modenum].width,modes[modenum].height);
	if (modes[modenum].bytesperpixel == 0)
		Con_Printf("ModeX\n");
	else
		Con_Printf("%d bpp\n", modes[modenum].bytesperpixel<<3);
}

void VID_DescribeModes_f (void)
{
	int i;
	
	for (i=0;i<num_modes;i++)
		if (modes[i].width) {
			Con_Printf("%d: %d x %d - ", i, modes[i].width,modes[i].height);
			if (modes[i].bytesperpixel == 0)
				Con_Printf("ModeX\n");
			else
				Con_Printf("%d bpp\n", modes[i].bytesperpixel<<3);
		}
}

/*
================
VID_NumModes
================
*/
int VID_NumModes ()
{
	int i,i1=0;
	
	for (i=0;i<num_modes;i++)
		i1+=(modes[i].width?1:0);
	return (i1);
}

void VID_NumModes_f (void)
{
	Con_Printf("%d modes\n",VID_NumModes());
}

void VID_Debug_f (void)
{
	Con_Printf("mode: %d\n",current_mode);
	Con_Printf("height x width: %d x %d\n",vid.height,vid.width);
	Con_Printf("bpp: %d\n",modes[current_mode].bytesperpixel*8);
	Con_Printf("vid.aspect: %f\n",vid.aspect);
}



void VID_InitModes(void)
{

	int i;

// get complete information on all modes

	num_modes = vga_lastmodenumber()+1;
	modes = Z_Malloc(num_modes * sizeof(vga_modeinfo));
	for (i=0 ; i<num_modes ; i++)
	{
		if (vga_hasmode(i))
			Q_memcpy(&modes[i], vga_getmodeinfo(i), sizeof (vga_modeinfo));
		else
			modes[i].width = 0; // means not available
	}

// filter for modes i don't support

	for (i=0 ; i<num_modes ; i++)
	{
		if (modes[i].bytesperpixel != 1 && modes[i].colors != 256) 
			modes[i].width = 0;
	}

}

int get_mode(char *name, int width, int height, int depth)
{

	int i;
	int ok, match;

	match = (!!width) + (!!height)*2 + (!!depth)*4;

	if (name)
	{
		i = vga_getmodenumber(name);
		if (!modes[i].width)
		{
			Sys_Printf("Mode [%s] not supported\n", name);
			i = G320x200x256;
		}
	}
	else
	{
		for (i=0 ; i<num_modes ; i++)
			if (modes[i].width)
			{
				ok = (modes[i].width == width)
					+ (modes[i].height == height)*2
					+ (modes[i].bytesperpixel == depth/8)*4;
				if ((ok & match) == ok)
					break;
			}
		if (i==num_modes)
		{
			Sys_Printf("Mode %dx%d (%d bits) not supported\n",
				width, height, depth);
			i = G320x200x256;
		}
	}

	return i;

}

int matchmouse(int mouse, char *name)
{
	int i;
	for (i=0 ; i<num_mice ; i++)
		if (!strcmp(mice[i].name, name))
			return i;
	return mouse;
}

#if 0

void vtswitch(int newconsole)
{

	int fd;
	struct vt_stat x;

// switch consoles and wait until reactivated
	fd = open("/dev/console", O_RDONLY);
	ioctl(fd, VT_GETSTATE, &x);
	ioctl(fd, VT_ACTIVATE, newconsole);
	ioctl(fd, VT_WAITACTIVE, x.v_active);
	close(fd);

}

#endif

void keyhandler(int scancode, int state)
{
	
	int sc;

	sc = scancode & 0x7f;
//	Con_Printf("scancode=%x (%d%s)\n", scancode, sc, scancode&0x80?"+128":"");
	Key_Event(scantokey[sc], state == KEY_EVENTPRESS);

}

void VID_Shutdown(void)
{

	if (!svgalib_inited) return;

//	printf("shutdown graphics called\n");
	if (UseKeyboard)
		keyboard_close();
	if (UseDisplay)
		vga_setmode(TEXT);
//	printf("shutdown graphics finished\n");

	svgalib_inited = 0;

}

void VID_ShiftPalette(unsigned char *p)
{
	VID_SetPalette(p);
}

void VID_SetPalette(byte *palette)
{

	static int tmppal[256*3];
	int *tp;
	int i;

	if (!svgalib_inited)
		return;

	memcpy(vid_current_palette, palette, sizeof(vid_current_palette));

	if (vga_getcolors() == 256)
	{

		tp = tmppal;
		for (i=256*3 ; i ; i--)
			*(tp++) = *(palette++) >> 2;

		if (UseDisplay && vga_oktowrite())
			vga_setpalvec(0, 256, tmppal);

	}
}

int VID_SetMode (int modenum, unsigned char *palette)
{
	int bsize, zsize, tsize;

	if ((modenum >= num_modes) || (modenum < 0) || !modes[modenum].width)
	{
		Cvar_SetValue ("vid_mode", (float)current_mode);
		
		Con_Printf("No such video mode: %d\n",modenum);
		
		return 0;
	}

	Cvar_SetValue ("vid_mode", (float)modenum);
	
	current_mode=modenum;

	vid.width = modes[current_mode].width;
	vid.height = modes[current_mode].height;

	VGA_width = modes[current_mode].width;
	VGA_height = modes[current_mode].height;
	VGA_planar = modes[current_mode].bytesperpixel == 0;
	VGA_rowbytes = modes[current_mode].linewidth;
	vid.rowbytes = modes[current_mode].linewidth;
	if (VGA_planar) {
		VGA_bufferrowbytes = modes[current_mode].linewidth * 4;
		vid.rowbytes = modes[current_mode].linewidth*4;
	}

	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
	vid.colormap = (pixel_t *) host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.conrowbytes = vid.rowbytes;
	vid.conwidth = vid.width;
	vid.conheight = vid.height;
	vid.numpages = 1;
	
	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;

	// alloc zbuffer and surface cache
	if (d_pzbuffer) {
		D_FlushCaches();
		Hunk_FreeToHighMark (VID_highhunkmark);
		d_pzbuffer = NULL;
		vid_surfcache = NULL;
	}

	bsize = vid.rowbytes * vid.height;
	tsize = D_SurfaceCacheForRes (vid.width, vid.height);
	zsize = vid.width * vid.height * sizeof(*d_pzbuffer);

	VID_highhunkmark = Hunk_HighMark ();

	d_pzbuffer = Hunk_HighAllocName (bsize+tsize+zsize, "video");

	vid_surfcache = ((byte *)d_pzbuffer) + zsize;

	vid.conbuffer = vid.buffer = (pixel_t *)(((byte *)d_pzbuffer) + zsize + tsize);

	D_InitCaches (vid_surfcache, tsize);

// get goin'

	vga_setmode(current_mode);
	VID_SetPalette(palette);

	VGA_pagebase = vid.direct = framebuffer_ptr = (char *) vga_getgraphmem();
//		if (vga_setlinearaddressing()>0)
//			framebuffer_ptr = (char *) vga_getgraphmem();
	if (!framebuffer_ptr)
		Sys_Error("This mode isn't hapnin'\n");

	vga_setpage(0);

	svgalib_inited=1;

	vid.recalc_refdef = 1;				// force a surface cache flush

	return 0;
}

void VID_Init(unsigned char *palette)
{

	int i;
	int w, h, d;

	if (svgalib_inited)
		return;

//	Cmd_AddCommand ("gamma", VID_Gamma_f);

	if (UseDisplay)
	{
		vga_init();

		VID_InitModes();

		Cvar_RegisterVariable (&vid_mode);
		Cvar_RegisterVariable (&vid_redrawfull);
		Cvar_RegisterVariable (&vid_waitforrefresh);
		
		Cmd_AddCommand("vid_nummodes", VID_NumModes_f);
		Cmd_AddCommand("vid_describemode", VID_DescribeMode_f);
		Cmd_AddCommand("vid_describemodes", VID_DescribeModes_f);
		Cmd_AddCommand("vid_debug", VID_Debug_f);

	// interpret command-line params

		w = h = d = 0;
		if (getenv("GSVGAMODE"))
			current_mode = get_mode(getenv("GSVGAMODE"), w, h, d);
		else if (COM_CheckParm("-mode"))
			current_mode = get_mode(com_argv[COM_CheckParm("-mode")+1], w, h, d);
		else if (COM_CheckParm("-w") || COM_CheckParm("-h")
			|| COM_CheckParm("-d"))
		{
			if (COM_CheckParm("-w"))
				w = Q_atoi(com_argv[COM_CheckParm("-w")+1]);
			if (COM_CheckParm("-h"))
				h = Q_atoi(com_argv[COM_CheckParm("-h")+1]);
			if (COM_CheckParm("-d"))
				d = Q_atoi(com_argv[COM_CheckParm("-d")+1]);
			current_mode = get_mode(0, w, h, d);
		}
		else
			current_mode = G320x200x256;

	// set vid parameters
		VID_SetMode(current_mode, palette);

		VID_SetPalette(palette);

		// we do want to run in the background when switched away
		vga_runinbackground(1);	
	}

	if (COM_CheckParm("-nokbd")) UseKeyboard = 0;

	if (UseKeyboard)
	{
		for (i=0 ; i<128 ; i++)
			scantokey[i] = ' ';

		scantokey[42] = K_SHIFT;
		scantokey[54] = K_SHIFT;
		scantokey[72] = K_UPARROW;
		scantokey[103] = K_UPARROW;
		scantokey[80] = K_DOWNARROW;
		scantokey[108] = K_DOWNARROW;
		scantokey[75] = K_LEFTARROW;
		scantokey[105] = K_LEFTARROW;
		scantokey[77] = K_RIGHTARROW;
		scantokey[106] = K_RIGHTARROW;
		scantokey[29] = K_CTRL;
		scantokey[97] = K_CTRL;
		scantokey[56] = K_ALT;
		scantokey[100] = K_ALT;
//		scantokey[58] = JK_CAPS;
//		scantokey[69] = JK_NUM_LOCK;
		scantokey[71] = K_HOME;
		scantokey[73] = K_PGUP;
		scantokey[79] = K_END;
		scantokey[81] = K_PGDN;
		scantokey[82] = K_INS;
		scantokey[83] = K_DEL;
		scantokey[1 ] = K_ESCAPE;
		scantokey[28] = K_ENTER;
		scantokey[15] = K_TAB;
		scantokey[14] = K_BACKSPACE;
		scantokey[119] = K_PAUSE;
    	scantokey[57] = ' ';

		scantokey[102] = K_HOME;
		scantokey[104] = K_PGUP;
		scantokey[107] = K_END;
		scantokey[109] = K_PGDN;
		scantokey[110] = K_INS;
		scantokey[111] = K_DEL;

		scantokey[2] = '1';
		scantokey[3] = '2';
		scantokey[4] = '3';
		scantokey[5] = '4';
		scantokey[6] = '5';
		scantokey[7] = '6';
		scantokey[8] = '7';
		scantokey[9] = '8';
		scantokey[10] = '9';
		scantokey[11] = '0';
		scantokey[12] = '-';
		scantokey[13] = '=';
		scantokey[41] = '`';
		scantokey[26] = '[';
		scantokey[27] = ']';
		scantokey[39] = ';';
		scantokey[40] = '\'';
		scantokey[51] = ',';
		scantokey[52] = '.';
		scantokey[53] = '/';
		scantokey[43] = '\\';

		scantokey[59] = K_F1;
		scantokey[60] = K_F2;
		scantokey[61] = K_F3;
		scantokey[62] = K_F4;
		scantokey[63] = K_F5;
		scantokey[64] = K_F6;
		scantokey[65] = K_F7;
		scantokey[66] = K_F8;
		scantokey[67] = K_F9;
		scantokey[68] = K_F10;
		scantokey[87] = K_F11;
		scantokey[88] = K_F12;
		scantokey[30] = 'a';
		scantokey[48] = 'b';
		scantokey[46] = 'c';
        scantokey[32] = 'd';       
        scantokey[18] = 'e';       
        scantokey[33] = 'f';       
        scantokey[34] = 'g';       
        scantokey[35] = 'h';       
        scantokey[23] = 'i';       
        scantokey[36] = 'j';       
        scantokey[37] = 'k';       
        scantokey[38] = 'l';       
        scantokey[50] = 'm';       
        scantokey[49] = 'n';       
        scantokey[24] = 'o';       
        scantokey[25] = 'p';       
        scantokey[16] = 'q';       
        scantokey[19] = 'r';       
        scantokey[31] = 's';       
        scantokey[20] = 't';       
        scantokey[22] = 'u';       
        scantokey[47] = 'v';       
        scantokey[17] = 'w';       
        scantokey[45] = 'x';       
        scantokey[21] = 'y';       
        scantokey[44] = 'z';       

		if (keyboard_init())
			Sys_Error("keyboard_init() failed");
		keyboard_seteventhandler(keyhandler);
	}

}

void VID_Update(vrect_t *rects)
{
	if (!svgalib_inited)
		return;

	if (!vga_oktowrite())
		return; // can't update screen if it's not active

	if (vid_waitforrefresh.value)
		vga_waitretrace();

	if (VGA_planar)
		VGA_UpdatePlanarScreen (vid.buffer);

	else if (vid_redrawfull.value) {
		int total = vid.rowbytes * vid.height;
		int offset;

		for (offset=0;offset<total;offset+=0x10000) {
			vga_setpage(offset/0x10000);
			memcpy(framebuffer_ptr,
					vid.buffer + offset,
					((total-offset>0x10000)?0x10000:(total-offset)));
		}
	} else {
		int ycount;
		int offset;
		int vidpage=0;

		vga_setpage(0);

		while (rects)
		{
			ycount = rects->height;
			offset = rects->y * vid.rowbytes + rects->x;
			while (ycount--)
			{
				register int i = offset % 0x10000;
	
				if ((offset / 0x10000) != vidpage) {
					vidpage=offset / 0x10000;
					vga_setpage(vidpage);
				}
				if (rects->width + i > 0x10000) {
					memcpy(framebuffer_ptr + i, 
							vid.buffer + offset, 
							0x10000 - i);
					vga_setpage(++vidpage);
					memcpy(framebuffer_ptr,
							vid.buffer + offset + 0x10000 - i, 
							rects->width - 0x10000 + i);
				} else
					memcpy(framebuffer_ptr + i, 
							vid.buffer + offset, 
							rects->width);
				offset += vid.rowbytes;
			}
	
			rects = rects->pnext;
		}
	}
	
	if (vid_mode.value != current_mode)
		VID_SetMode ((int)vid_mode.value, vid_current_palette);
}

static int dither;

void VID_DitherOn(void)
{
    if (dither == 0)
    {
//		R_ViewChanged (&vrect, sb_lines, vid.aspect);
        dither = 1;
    }
}

void VID_DitherOff(void)
{
    if (dither)
    {
//		R_ViewChanged (&vrect, sb_lines, vid.aspect);
        dither = 0;
    }
}

void Sys_SendKeyEvents(void)
{
	if (!svgalib_inited)
		return;

	if (UseKeyboard)
		while (keyboard_update());
}

void Force_CenterView_f (void)
{
	cl.viewangles[PITCH] = 0;
}


void mousehandler(int buttonstate, int dx, int dy)
{
	mouse_buttonstate = buttonstate;
	mx += dx;
	my += dy;
}

void IN_Init(void)
{

	int mtype;
	char *mousedev;
	int mouserate;

	if (UseMouse)
	{

		Cvar_RegisterVariable (&mouse_button_commands[0]);
		Cvar_RegisterVariable (&mouse_button_commands[1]);
		Cvar_RegisterVariable (&mouse_button_commands[2]);
		Cvar_RegisterVariable (&m_filter);
		Cmd_AddCommand ("force_centerview", Force_CenterView_f);

		mouse_buttons = 3;

		mtype = vga_getmousetype();

		mousedev = "/dev/mouse";
		if (getenv("MOUSEDEV")) mousedev = getenv("MOUSEDEV");
		if (COM_CheckParm("-mdev"))
			mousedev = com_argv[COM_CheckParm("-mdev")+1];

		mouserate = 1200;
		if (getenv("MOUSERATE")) mouserate = atoi(getenv("MOUSERATE"));
		if (COM_CheckParm("-mrate"))
			mouserate = atoi(com_argv[COM_CheckParm("-mrate")+1]);

//		printf("Mouse: dev=%s,type=%s,speed=%d\n",
//			mousedev, mice[mtype].name, mouserate);
		if (mouse_init(mousedev, mtype, mouserate))
		{
			Con_Printf("No mouse found\n");
			UseMouse = 0;
		}
		else
			mouse_seteventhandler(mousehandler);

	}

}

void IN_Shutdown(void)
{
	if (UseMouse)
		mouse_close();
}

/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	if (UseMouse && cls.state != ca_dedicated)
	{
		// poll mouse values
		while (mouse_update())
			;

		// perform button actions
		if ((mouse_buttonstate & MOUSE_LEFTBUTTON) &&
			!(mouse_oldbuttonstate & MOUSE_LEFTBUTTON))
			Key_Event (K_MOUSE1, true);
		else if (!(mouse_buttonstate & MOUSE_LEFTBUTTON) &&
			(mouse_oldbuttonstate & MOUSE_LEFTBUTTON))
			Key_Event (K_MOUSE1, false);

		if ((mouse_buttonstate & MOUSE_RIGHTBUTTON) &&
			!(mouse_oldbuttonstate & MOUSE_RIGHTBUTTON))
			Key_Event (K_MOUSE2, true);
		else if (!(mouse_buttonstate & MOUSE_RIGHTBUTTON) &&
			(mouse_oldbuttonstate & MOUSE_RIGHTBUTTON))
			Key_Event (K_MOUSE2, false);

		if ((mouse_buttonstate & MOUSE_MIDDLEBUTTON) &&
			!(mouse_oldbuttonstate & MOUSE_MIDDLEBUTTON))
			Key_Event (K_MOUSE3, true);
		else if (!(mouse_buttonstate & MOUSE_MIDDLEBUTTON) &&
			(mouse_oldbuttonstate & MOUSE_MIDDLEBUTTON))
			Key_Event (K_MOUSE3, false);

		mouse_oldbuttonstate = mouse_buttonstate;
	}
}

/*
===========
IN_Move
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
	if (!UseMouse)
		return;

	// poll mouse values
	while (mouse_update())
		;

	if (m_filter.value)
	{
		mouse_x = (mx + old_mouse_x) * 0.5;
		mouse_y = (my + old_mouse_y) * 0.5;
	}
	else
	{
		mouse_x = mx;
		mouse_y = my;
	}
	old_mouse_x = mx;
	old_mouse_y = my;
	mx = my = 0; // clear for next update

	mouse_x *= sensitivity.value;
	mouse_y *= sensitivity.value;

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mouse_x;
	else
		cl.viewangles[YAW] -= m_yaw.value * mouse_x;
	
	if (in_mlook.state & 1)
		V_StopPitchDrift ();
		
	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * mouse_y;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * mouse_y;
		else
			cmd->forwardmove -= m_forward.value * mouse_y;
	}
}

void IN_Move (usercmd_t *cmd)
{
	IN_MouseMove(cmd);
}


/*
================
VID_ModeInfo
================
*/
char *VID_ModeInfo (int modenum)
{
	static char	*badmodestr = "Bad mode number";
	static char modestr[40];

	if (modenum == 0)
	{
		sprintf (modestr, "%d x %d, %d bpp",
				 vid.width, vid.height, modes[current_mode].bytesperpixel*8);
		return (modestr);
	}
	else
	{
		return (badmodestr);
	}
}

