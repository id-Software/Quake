#
# Quake Makefile for Linux 2.0
#
# Aug '98 by Zoid <zoid@idsoftware.com>
#
# ELF only
#

# wyatt: sdl2 audio, mostly borrowed from quakespasm with some tweaks for
# otherwise-vanilla Quake.
# _comment out_ USE_SDL2 definitions to disable (falls back to OSS DMA sound).
# non-1 values might work for SDL 1.x support, but this is currently
# untested (though likely to work).
USE_SDL2=1

ifeq ($(USE_SDL2),1)
SDL_CONFIG ?= sdl2-config
else
SDL_CONFIG ?= sdl-config
endif
SDL_CFLAGS := $(shell $(SDL_CONFIG) --cflags) -DUSE_SDL2
SDL_LIBS   := $(shell $(SDL_CONFIG) --libs)

BASEVERSION=1.09
VERSION=$(BASEVERSION)$(GLIBC)

ifneq (,$(findstring libc6,$(shell if [ -e /lib/libc.so.6 ];then echo libc6;fi)))
GLIBC=-glibc
else
GLIBC=
endif

ifneq (,$(findstring alpha,$(shell uname -m)))
ARCH=axp
else
ARCH=i386
endif
NOARCH=noarch

# Edit this path, or you probably will run into trouble.
MOUNT_DIR=/home/wyatt/development/Quake-LinuxUpdate/WinQuake
MASTER_DIR=/grog/Projects/QuakeMaster
MESA_DIR=/usr/local/src/Mesa-2.6

BUILD_DEBUG_DIR=debug$(ARCH)$(GLIBC)
BUILD_RELEASE_DIR=release$(ARCH)$(GLIBC)

EGCS=/usr/bin/gcc
CC=$(EGCS)

# GCC 10 seems to require -no-pie to function properly. It _definitely_
# does require -fcommon. -m32 is because I am building on a 64-bit machine
# and this game still has some i386 (32-bit) assembly that I haven't replaced
# or otherwise added alternative, portable code for.
# Hopefully, I'll be able to run this game on PowerPC some day.
BASE_CFLAGS=-Dstricmp=strcasecmp -m32 -fcommon -no-pie
# might have to change these, too, if you are on another architecture or
# cross-compiling (don't use 'native'):
RELEASE_CFLAGS=$(BASE_CFLAGS) -g -mtune=native -O3 -march=native
# note that powerpc uses -mcpu instead of -march, if I ever do port it.
DEBUG_CFLAGS=$(BASE_CFLAGS) -g
LDFLAGS=-lm -m32
SVGALDFLAGS=-lvga
XLDFLAGS=-L/usr/X11R6/lib -lX11 -lXext /usr/lib/i386-linux-gnu/libXxf86dga.so.1.0.0
XCFLAGS=-DX11

MESAGLLDFLAGS=-L/usr/lib/X11 -L/usr/local/lib -L$(MESA_DIR)/lib -lMesaGL -lglide2x -lX11 -lXext -ldl
TDFXGLLDFLAGS=-L$(TDFXGL_DIR)/release$(ARCH)$(GLIBC) -l3dfxgl -lglide2x -ldl
GLLDFLAGS=-L/usr/X11/lib -L/usr/local/lib -lGL -lX11 -lXext -ldl /usr/lib/i386-linux-gnu/libXxf86dga.so.1.0.0 /usr/lib/i386-linux-gnu/libXxf86vm.so.1.0.0 -lm
GLCFLAGS=-DGLQUAKE -I$(MESA_DIR)/include -I/usr/include/glide

DO_CC=$(CC) $(CFLAGS) -o $@ -c $<
DO_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) -o $@ -c $<
DO_GL_CC=$(CC) $(CFLAGS) $(GLCFLAGS) $(SDL_CFLAGS) -o $@ -c $<
DO_GL_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) $(GLCFLAGS) $(SDL_CFLAGS) -o $@ -c $<
DO_X11_CC=$(CC) $(CFLAGS) $(XCFLAGS) $(SDL_CFLAGS) -o $@ -c $<
DO_X11_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) $(XCFLAGS) $(SDL_CFLAGS) -o $@ -c $<
DO_O_CC=$(CC) -O $(CFLAGS) -o $@ -c $<
DO_AS=$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
DO_GL_AS=$(CC) $(CFLAGS) $(GLCFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

#############################################################################
# SETUP AND BUILD
#############################################################################

# GLQuake build:
# TARGETS=$(BUILDDIR)/bin/glquake.glx

# X11 Quake (software-rendered) build:
TARGETS=$(BUILDDIR)/bin/quake.x11

build_debug:
	@-mkdir $(BUILD_DEBUG_DIR) \
		$(BUILD_DEBUG_DIR)/bin \
		$(BUILD_DEBUG_DIR)/glquake \
		$(BUILD_DEBUG_DIR)/squake \
		$(BUILD_DEBUG_DIR)/unixded \
		$(BUILD_DEBUG_DIR)/x11
	$(MAKE) targets BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

build_release:
	@-mkdir $(BUILD_RELEASE_DIR) \
		$(BUILD_RELEASE_DIR)/bin \
		$(BUILD_RELEASE_DIR)/glquake \
		$(BUILD_RELEASE_DIR)/squake \
		$(BUILD_RELEASE_DIR)/unixded \
		$(BUILD_RELEASE_DIR)/x11
	$(MAKE) targets BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(RELEASE_CFLAGS)"

all: build_debug build_release

targets: $(TARGETS)

#############################################################################
# X11 Quake
#############################################################################
# REPLACED snd_linux.o with snd_sdl.o from quakespasm
#	$(BUILDDIR)/x11/snd_linux.o \

X11_OBJS = \
	$(BUILDDIR)/x11/cl_demo.o \
	$(BUILDDIR)/x11/cl_input.o \
	$(BUILDDIR)/x11/cl_main.o \
	$(BUILDDIR)/x11/cl_parse.o \
	$(BUILDDIR)/x11/cl_tent.o \
	$(BUILDDIR)/x11/chase.o \
	$(BUILDDIR)/x11/cmd.o \
	$(BUILDDIR)/x11/common.o \
	$(BUILDDIR)/x11/console.o \
	$(BUILDDIR)/x11/crc.o \
	$(BUILDDIR)/x11/cvar.o \
	$(BUILDDIR)/x11/draw.o \
	$(BUILDDIR)/x11/d_edge.o \
	$(BUILDDIR)/x11/d_fill.o \
	$(BUILDDIR)/x11/d_init.o \
	$(BUILDDIR)/x11/d_modech.o \
	$(BUILDDIR)/x11/d_part.o \
	$(BUILDDIR)/x11/d_polyse.o \
	$(BUILDDIR)/x11/d_scan.o \
	$(BUILDDIR)/x11/d_sky.o \
	$(BUILDDIR)/x11/d_sprite.o \
	$(BUILDDIR)/x11/d_surf.o \
	$(BUILDDIR)/x11/d_vars.o \
	$(BUILDDIR)/x11/d_zpoint.o \
	$(BUILDDIR)/x11/host.o \
	$(BUILDDIR)/x11/host_cmd.o \
	$(BUILDDIR)/x11/keys.o \
	$(BUILDDIR)/x11/menu.o \
	$(BUILDDIR)/x11/mathlib.o \
	$(BUILDDIR)/x11/model.o \
	$(BUILDDIR)/x11/net_dgrm.o \
	$(BUILDDIR)/x11/net_loop.o \
	$(BUILDDIR)/x11/net_main.o \
	$(BUILDDIR)/x11/net_vcr.o \
	$(BUILDDIR)/x11/net_udp.o \
	$(BUILDDIR)/x11/net_bsd.o \
	$(BUILDDIR)/x11/nonintel.o \
	$(BUILDDIR)/x11/pr_cmds.o \
	$(BUILDDIR)/x11/pr_edict.o \
	$(BUILDDIR)/x11/pr_exec.o \
	$(BUILDDIR)/x11/r_aclip.o \
	$(BUILDDIR)/x11/r_alias.o \
	$(BUILDDIR)/x11/r_bsp.o \
	$(BUILDDIR)/x11/r_light.o \
	$(BUILDDIR)/x11/r_draw.o \
	$(BUILDDIR)/x11/r_efrag.o \
	$(BUILDDIR)/x11/r_edge.o \
	$(BUILDDIR)/x11/r_misc.o \
	$(BUILDDIR)/x11/r_main.o \
	$(BUILDDIR)/x11/r_sky.o \
	$(BUILDDIR)/x11/r_sprite.o \
	$(BUILDDIR)/x11/r_surf.o \
	$(BUILDDIR)/x11/r_part.o \
	$(BUILDDIR)/x11/r_vars.o \
	$(BUILDDIR)/x11/screen.o \
	$(BUILDDIR)/x11/sbar.o \
	$(BUILDDIR)/x11/sv_main.o \
	$(BUILDDIR)/x11/sv_phys.o \
	$(BUILDDIR)/x11/sv_move.o \
	$(BUILDDIR)/x11/sv_user.o \
	$(BUILDDIR)/x11/zone.o	\
	$(BUILDDIR)/x11/view.o	\
	$(BUILDDIR)/x11/wad.o \
	$(BUILDDIR)/x11/world.o \
	$(BUILDDIR)/x11/cd_linux.o \
	$(BUILDDIR)/x11/sys_linux.o \
	$(BUILDDIR)/x11/vid_x.o \
	$(BUILDDIR)/x11/snd_dma.o \
	$(BUILDDIR)/x11/snd_mem.o \
	$(BUILDDIR)/x11/snd_mix.o \
	\
	$(BUILDDIR)/x11/d_draw.o \
	$(BUILDDIR)/x11/d_draw16.o \
	$(BUILDDIR)/x11/d_parta.o \
	$(BUILDDIR)/x11/d_polysa.o \
	$(BUILDDIR)/x11/d_scana.o \
	$(BUILDDIR)/x11/d_spr8.o \
	$(BUILDDIR)/x11/d_varsa.o \
	$(BUILDDIR)/x11/math.o \
	$(BUILDDIR)/x11/r_aliasa.o \
	$(BUILDDIR)/x11/r_drawa.o \
	$(BUILDDIR)/x11/r_edgea.o \
	$(BUILDDIR)/x11/r_varsa.o \
	$(BUILDDIR)/x11/surf16.o \
	$(BUILDDIR)/x11/surf8.o \
	$(BUILDDIR)/x11/worlda.o \
	$(BUILDDIR)/x11/r_aclipa.o \
	$(BUILDDIR)/x11/snd_mixa.o \
	$(BUILDDIR)/x11/sys_dosa.o

ifeq ($(USE_SDL2),1)
# sdl sound
X11_OBJS := $(X11_OBJS) \
	$(BUILDDIR)/x11/snd_sdl.o
else
# OSS sound
X11_OBJS := $(X11_OBJS) \
	$(BUILDDIR)/x11/snd_linux.o
endif

$(BUILDDIR)/bin/quake.x11 : $(X11_OBJS)
	$(CC) $(CFLAGS) -o $@ $(X11_OBJS) $(XLDFLAGS) $(SDL_LIBS) $(LDFLAGS)

####

$(BUILDDIR)/x11/cl_demo.o :  $(MOUNT_DIR)/cl_demo.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/cl_input.o : $(MOUNT_DIR)/cl_input.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/cl_main.o :  $(MOUNT_DIR)/cl_main.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/cl_parse.o : $(MOUNT_DIR)/cl_parse.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/cl_tent.o :  $(MOUNT_DIR)/cl_tent.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/chase.o :    $(MOUNT_DIR)/chase.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/cmd.o :      $(MOUNT_DIR)/cmd.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/common.o :   $(MOUNT_DIR)/common.c
	$(DO_X11_DEBUG_CC)

$(BUILDDIR)/x11/console.o :  $(MOUNT_DIR)/console.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/crc.o :      $(MOUNT_DIR)/crc.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/cvar.o :     $(MOUNT_DIR)/cvar.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/draw.o :     $(MOUNT_DIR)/draw.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_edge.o :   $(MOUNT_DIR)/d_edge.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_fill.o :   $(MOUNT_DIR)/d_fill.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_init.o :   $(MOUNT_DIR)/d_init.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_modech.o : $(MOUNT_DIR)/d_modech.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_part.o :   $(MOUNT_DIR)/d_part.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_polyse.o : $(MOUNT_DIR)/d_polyse.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_scan.o :   $(MOUNT_DIR)/d_scan.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_sky.o :    $(MOUNT_DIR)/d_sky.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_sprite.o : $(MOUNT_DIR)/d_sprite.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_surf.o :   $(MOUNT_DIR)/d_surf.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_vars.o :   $(MOUNT_DIR)/d_vars.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/d_zpoint.o : $(MOUNT_DIR)/d_zpoint.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/host.o :     $(MOUNT_DIR)/host.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/host_cmd.o : $(MOUNT_DIR)/host_cmd.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/keys.o :     $(MOUNT_DIR)/keys.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/menu.o :     $(MOUNT_DIR)/menu.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/mathlib.o :  $(MOUNT_DIR)/mathlib.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/model.o :    $(MOUNT_DIR)/model.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/net_dgrm.o : $(MOUNT_DIR)/net_dgrm.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/net_loop.o : $(MOUNT_DIR)/net_loop.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/net_main.o : $(MOUNT_DIR)/net_main.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/net_vcr.o :  $(MOUNT_DIR)/net_vcr.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/net_udp.o :  $(MOUNT_DIR)/net_udp.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/net_bsd.o :  $(MOUNT_DIR)/net_bsd.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/nonintel.o : $(MOUNT_DIR)/nonintel.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/pr_cmds.o :  $(MOUNT_DIR)/pr_cmds.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/pr_edict.o : $(MOUNT_DIR)/pr_edict.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/pr_exec.o :  $(MOUNT_DIR)/pr_exec.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_aclip.o :  $(MOUNT_DIR)/r_aclip.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_alias.o :  $(MOUNT_DIR)/r_alias.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_bsp.o :    $(MOUNT_DIR)/r_bsp.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_light.o :  $(MOUNT_DIR)/r_light.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_draw.o :   $(MOUNT_DIR)/r_draw.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_efrag.o :  $(MOUNT_DIR)/r_efrag.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_edge.o :   $(MOUNT_DIR)/r_edge.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_misc.o :   $(MOUNT_DIR)/r_misc.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_main.o :   $(MOUNT_DIR)/r_main.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_sky.o :    $(MOUNT_DIR)/r_sky.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_sprite.o : $(MOUNT_DIR)/r_sprite.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_surf.o :   $(MOUNT_DIR)/r_surf.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_part.o :   $(MOUNT_DIR)/r_part.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/r_vars.o :   $(MOUNT_DIR)/r_vars.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/screen.o :   $(MOUNT_DIR)/screen.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/sbar.o :     $(MOUNT_DIR)/sbar.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/sv_main.o :  $(MOUNT_DIR)/sv_main.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/sv_phys.o :  $(MOUNT_DIR)/sv_phys.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/sv_move.o :  $(MOUNT_DIR)/sv_move.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/sv_user.o :  $(MOUNT_DIR)/sv_user.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/zone.o	:   $(MOUNT_DIR)/zone.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/view.o	:   $(MOUNT_DIR)/view.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/wad.o :      $(MOUNT_DIR)/wad.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/world.o :    $(MOUNT_DIR)/world.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/cd_linux.o : $(MOUNT_DIR)/cd_linux.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/sys_linux.o :$(MOUNT_DIR)/sys_linux.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/vid_x.o: $(MOUNT_DIR)/vid_x.c
	$(DO_O_CC)

$(BUILDDIR)/x11/snd_dma.o :  $(MOUNT_DIR)/snd_dma.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/snd_mem.o :  $(MOUNT_DIR)/snd_mem.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/snd_mix.o :  $(MOUNT_DIR)/snd_mix.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/snd_linux.o :$(MOUNT_DIR)/snd_linux.c
	$(DO_X11_CC)

$(BUILDDIR)/x11/snd_sdl.o :$(MOUNT_DIR)/snd_sdl.c
	$(DO_X11_CC)

#####

$(BUILDDIR)/x11/d_copy.o :   $(MOUNT_DIR)/d_copy.s
	$(DO_AS)

$(BUILDDIR)/x11/d_draw.o :   $(MOUNT_DIR)/d_draw.s
	$(DO_AS)

$(BUILDDIR)/x11/d_draw16.o : $(MOUNT_DIR)/d_draw16.s
	$(DO_AS)

$(BUILDDIR)/x11/d_parta.o :  $(MOUNT_DIR)/d_parta.s
	$(DO_AS)

$(BUILDDIR)/x11/d_polysa.o : $(MOUNT_DIR)/d_polysa.s
	$(DO_AS)

$(BUILDDIR)/x11/d_scana.o :  $(MOUNT_DIR)/d_scana.s
	$(DO_AS)

$(BUILDDIR)/x11/d_spr8.o :   $(MOUNT_DIR)/d_spr8.s
	$(DO_AS)

$(BUILDDIR)/x11/d_varsa.o :  $(MOUNT_DIR)/d_varsa.s
	$(DO_AS)

$(BUILDDIR)/x11/math.o :     $(MOUNT_DIR)/math.s
	$(DO_AS)

$(BUILDDIR)/x11/r_aliasa.o : $(MOUNT_DIR)/r_aliasa.s
	$(DO_AS)

$(BUILDDIR)/x11/r_drawa.o :  $(MOUNT_DIR)/r_drawa.s
	$(DO_AS)

$(BUILDDIR)/x11/r_edgea.o :  $(MOUNT_DIR)/r_edgea.s
	$(DO_AS)

$(BUILDDIR)/x11/r_varsa.o :  $(MOUNT_DIR)/r_varsa.s
	$(DO_AS)

$(BUILDDIR)/x11/surf16.o :   $(MOUNT_DIR)/surf16.s
	$(DO_AS)

$(BUILDDIR)/x11/surf8.o :    $(MOUNT_DIR)/surf8.s
	$(DO_AS)

$(BUILDDIR)/x11/worlda.o :   $(MOUNT_DIR)/worlda.s
	$(DO_AS)

$(BUILDDIR)/x11/r_aclipa.o : $(MOUNT_DIR)/r_aclipa.s
	$(DO_AS)

$(BUILDDIR)/x11/snd_mixa.o : $(MOUNT_DIR)/snd_mixa.s
	$(DO_AS)

$(BUILDDIR)/x11/sys_dosa.o : $(MOUNT_DIR)/sys_dosa.s
	$(DO_AS)

#############################################################################
# GLQuake
#############################################################################

GLQUAKE_OBJS= \
	$(BUILDDIR)/glquake/cl_demo.o \
	$(BUILDDIR)/glquake/cl_input.o \
	$(BUILDDIR)/glquake/cl_main.o \
	$(BUILDDIR)/glquake/cl_parse.o \
	$(BUILDDIR)/glquake/cl_tent.o \
	$(BUILDDIR)/glquake/chase.o \
	$(BUILDDIR)/glquake/cmd.o \
	$(BUILDDIR)/glquake/common.o \
	$(BUILDDIR)/glquake/console.o \
	$(BUILDDIR)/glquake/crc.o \
	$(BUILDDIR)/glquake/cvar.o \
	\
	$(BUILDDIR)/glquake/gl_draw.o \
	$(BUILDDIR)/glquake/gl_mesh.o \
	$(BUILDDIR)/glquake/gl_model.o \
	$(BUILDDIR)/glquake/gl_refrag.o \
	$(BUILDDIR)/glquake/gl_rlight.o \
	$(BUILDDIR)/glquake/gl_rmain.o \
	$(BUILDDIR)/glquake/gl_rmisc.o \
	$(BUILDDIR)/glquake/gl_rsurf.o \
	$(BUILDDIR)/glquake/gl_screen.o \
	$(BUILDDIR)/glquake/gl_test.o \
	$(BUILDDIR)/glquake/gl_warp.o \
	\
	$(BUILDDIR)/glquake/host.o \
	$(BUILDDIR)/glquake/host_cmd.o \
	$(BUILDDIR)/glquake/keys.o \
	$(BUILDDIR)/glquake/menu.o \
	$(BUILDDIR)/glquake/mathlib.o \
	$(BUILDDIR)/glquake/net_dgrm.o \
	$(BUILDDIR)/glquake/net_loop.o \
	$(BUILDDIR)/glquake/net_main.o \
	$(BUILDDIR)/glquake/net_vcr.o \
	$(BUILDDIR)/glquake/net_udp.o \
	$(BUILDDIR)/glquake/net_bsd.o \
	$(BUILDDIR)/glquake/pr_cmds.o \
	$(BUILDDIR)/glquake/pr_edict.o \
	$(BUILDDIR)/glquake/pr_exec.o \
	$(BUILDDIR)/glquake/r_part.o \
	$(BUILDDIR)/glquake/sbar.o \
	$(BUILDDIR)/glquake/sv_main.o \
	$(BUILDDIR)/glquake/sv_phys.o \
	$(BUILDDIR)/glquake/sv_move.o \
	$(BUILDDIR)/glquake/sv_user.o \
	$(BUILDDIR)/glquake/zone.o	\
	$(BUILDDIR)/glquake/view.o	\
	$(BUILDDIR)/glquake/wad.o \
	$(BUILDDIR)/glquake/world.o \
	$(BUILDDIR)/glquake/cd_linux.o \
	$(BUILDDIR)/glquake/sys_linux.o \
	$(BUILDDIR)/glquake/snd_dma.o \
	$(BUILDDIR)/glquake/snd_mem.o \
	$(BUILDDIR)/glquake/snd_mix.o \
	\
	$(BUILDDIR)/glquake/math.o \
	$(BUILDDIR)/glquake/worlda.o \
	$(BUILDDIR)/glquake/snd_mixa.o \
	$(BUILDDIR)/glquake/sys_dosa.o

ifeq ($(USE_SDL2),1)
# sdl sound
GLQUAKE_OBJS := $(GLQUAKE_OBJS) \
	$(BUILDDIR)/glquake/snd_sdl.o
else
# OSS sound
GLQUAKE_OBJS := $(GLQUAkE_OBJS) \
	$(BUILDDIR)/glquake/snd_linux.o
endif

GLSVGA_OBJS=$(BUILDDIR)/glquake/gl_vidlinux.o

GLX_OBJS=$(BUILDDIR)/glquake/gl_vidlinuxglx.o

$(BUILDDIR)/bin/glquake : $(GLQUAKE_OBJS) $(GLSVGA_OBJS)
	$(CC) $(CFLAGS) -o $@ $(GLQUAKE_OBJS) $(GLSVGA_OBJS) $(MESAGLLDFLAGS) $(SVGALDFLAGS) $(SDL_LIBS) $(LDFLAGS)

$(BUILDDIR)/bin/glquake.glx : $(GLQUAKE_OBJS) $(GLX_OBJS)
	$(CC) $(CFLAGS) -o $@ $(GLQUAKE_OBJS) $(GLX_OBJS) $(GLLDFLAGS) $(SDL_LIBS) $(LDFLAGS)

$(BUILDDIR)/bin/glquake.3dfxgl : $(GLQUAKE_OBJS) $(GLSVGA_OBJS)
	$(CC) $(CFLAGS) -o $@ $(GLQUAKE_OBJS) $(GLSVGA_OBJS) $(TDFXGLLDFLAGS) $(SVGALDFLAGS) $(SDL_LIBS) $(LDFLAGS)

$(BUILDDIR)/glquake/cl_demo.o :      $(MOUNT_DIR)/cl_demo.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cl_input.o :     $(MOUNT_DIR)/cl_input.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cl_main.o :      $(MOUNT_DIR)/cl_main.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cl_parse.o :     $(MOUNT_DIR)/cl_parse.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cl_tent.o :      $(MOUNT_DIR)/cl_tent.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/chase.o :        $(MOUNT_DIR)/chase.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cmd.o :          $(MOUNT_DIR)/cmd.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/common.o :       $(MOUNT_DIR)/common.c
	$(DO_GL_DEBUG_CC)

$(BUILDDIR)/glquake/console.o :      $(MOUNT_DIR)/console.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/crc.o :          $(MOUNT_DIR)/crc.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cvar.o :         $(MOUNT_DIR)/cvar.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_draw.o :      $(MOUNT_DIR)/gl_draw.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_mesh.o :      $(MOUNT_DIR)/gl_mesh.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_model.o :     $(MOUNT_DIR)/gl_model.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_refrag.o :    $(MOUNT_DIR)/gl_refrag.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_rlight.o :    $(MOUNT_DIR)/gl_rlight.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_rmain.o :     $(MOUNT_DIR)/gl_rmain.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_rmisc.o :     $(MOUNT_DIR)/gl_rmisc.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_rsurf.o :     $(MOUNT_DIR)/gl_rsurf.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_screen.o :    $(MOUNT_DIR)/gl_screen.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_test.o :      $(MOUNT_DIR)/gl_test.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_vidlinux.o :  $(MOUNT_DIR)/gl_vidlinux.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_vidlinuxglx.o :  $(MOUNT_DIR)/gl_vidlinuxglx.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/gl_warp.o :      $(MOUNT_DIR)/gl_warp.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/host.o :         $(MOUNT_DIR)/host.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/host_cmd.o :     $(MOUNT_DIR)/host_cmd.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/keys.o :         $(MOUNT_DIR)/keys.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/menu.o :         $(MOUNT_DIR)/menu.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/mathlib.o :      $(MOUNT_DIR)/mathlib.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_dgrm.o :     $(MOUNT_DIR)/net_dgrm.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_loop.o :     $(MOUNT_DIR)/net_loop.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_main.o :     $(MOUNT_DIR)/net_main.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_vcr.o :      $(MOUNT_DIR)/net_vcr.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_udp.o :      $(MOUNT_DIR)/net_udp.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/net_bsd.o :      $(MOUNT_DIR)/net_bsd.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/pr_cmds.o :      $(MOUNT_DIR)/pr_cmds.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/pr_edict.o :     $(MOUNT_DIR)/pr_edict.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/pr_exec.o :      $(MOUNT_DIR)/pr_exec.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/r_part.o :       $(MOUNT_DIR)/r_part.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sbar.o :         $(MOUNT_DIR)/sbar.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sv_main.o :      $(MOUNT_DIR)/sv_main.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sv_phys.o :      $(MOUNT_DIR)/sv_phys.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sv_move.o :      $(MOUNT_DIR)/sv_move.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sv_user.o :      $(MOUNT_DIR)/sv_user.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/zone.o	:        $(MOUNT_DIR)/zone.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/view.o	:        $(MOUNT_DIR)/view.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/wad.o :          $(MOUNT_DIR)/wad.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/world.o :        $(MOUNT_DIR)/world.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/cd_linux.o :     $(MOUNT_DIR)/cd_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/sys_linux.o :    $(MOUNT_DIR)/sys_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/snd_dma.o :      $(MOUNT_DIR)/snd_dma.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/snd_mem.o :      $(MOUNT_DIR)/snd_mem.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/snd_mix.o :      $(MOUNT_DIR)/snd_mix.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/snd_linux.o :    $(MOUNT_DIR)/snd_linux.c
	$(DO_GL_CC)

$(BUILDDIR)/glquake/math.o :         $(MOUNT_DIR)/math.s
	$(DO_GL_AS)

$(BUILDDIR)/glquake/worlda.o :       $(MOUNT_DIR)/worlda.s
	$(DO_GL_AS)

$(BUILDDIR)/glquake/snd_mixa.o :     $(MOUNT_DIR)/snd_mixa.s
	$(DO_GL_AS)

$(BUILDDIR)/glquake/sys_dosa.o :     $(MOUNT_DIR)/sys_dosa.s
	$(DO_GL_AS)

#############################################################################
# MISC
#############################################################################

clean: clean-debug clean-release
	rm -f squake.spec glquake.spec quake.x11.spec

clean-debug:
	$(MAKE) clean2 BUILDDIR=$(BUILD_DEBUG_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 BUILDDIR=$(BUILD_RELEASE_DIR) CFLAGS="$(DEBUG_CFLAGS)"

clean2:
	-rm -f $(SQUAKE_OBJS) $(X11_OBJS) $(GLQUAKE_OBJS) $(GLSVGA_OBJS) \
		$(GLX_OBJS)

