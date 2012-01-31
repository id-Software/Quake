# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=qwcl - Win32 GL Debug
!MESSAGE No configuration specified.  Defaulting to qwcl - Win32 GL Debug.
!ENDIF 

!IF "$(CFG)" != "qwcl - Win32 Release" && "$(CFG)" != "qwcl - Win32 Debug" &&\
 "$(CFG)" != "qwcl - Win32 GL Debug" && "$(CFG)" != "qwcl - Win32 GL Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "qwcl.mak" CFG="qwcl - Win32 GL Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qwcl - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "qwcl - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "qwcl - Win32 GL Debug" (based on "Win32 (x86) Application")
!MESSAGE "qwcl - Win32 GL Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "qwcl - Win32 GL Debug"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "qwcl___W"
# PROP BASE Intermediate_Dir "qwcl___W"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "release"
# PROP Intermediate_Dir "release"
# PROP Target_Dir ""
OUTDIR=.\release
INTDIR=.\release

ALL : "$(OUTDIR)\qwcl.exe"

CLEAN : 
	-@erase "$(INTDIR)\cd_win.obj"
	-@erase "$(INTDIR)\cl_cam.obj"
	-@erase "$(INTDIR)\cl_demo.obj"
	-@erase "$(INTDIR)\cl_ents.obj"
	-@erase "$(INTDIR)\cl_input.obj"
	-@erase "$(INTDIR)\cl_main.obj"
	-@erase "$(INTDIR)\cl_parse.obj"
	-@erase "$(INTDIR)\cl_pred.obj"
	-@erase "$(INTDIR)\cl_tent.obj"
	-@erase "$(INTDIR)\cmd.obj"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\console.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\cvar.obj"
	-@erase "$(INTDIR)\d_draw.obj"
	-@erase "$(INTDIR)\d_draw16.obj"
	-@erase "$(INTDIR)\d_edge.obj"
	-@erase "$(INTDIR)\d_fill.obj"
	-@erase "$(INTDIR)\d_init.obj"
	-@erase "$(INTDIR)\d_modech.obj"
	-@erase "$(INTDIR)\d_part.obj"
	-@erase "$(INTDIR)\d_parta.obj"
	-@erase "$(INTDIR)\d_polysa.obj"
	-@erase "$(INTDIR)\d_polyse.obj"
	-@erase "$(INTDIR)\d_scan.obj"
	-@erase "$(INTDIR)\d_scana.obj"
	-@erase "$(INTDIR)\d_sky.obj"
	-@erase "$(INTDIR)\d_spr8.obj"
	-@erase "$(INTDIR)\d_sprite.obj"
	-@erase "$(INTDIR)\d_surf.obj"
	-@erase "$(INTDIR)\d_vars.obj"
	-@erase "$(INTDIR)\d_varsa.obj"
	-@erase "$(INTDIR)\d_zpoint.obj"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\in_win.obj"
	-@erase "$(INTDIR)\keys.obj"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\md4.obj"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\net_chan.obj"
	-@erase "$(INTDIR)\net_wins.obj"
	-@erase "$(INTDIR)\pmove.obj"
	-@erase "$(INTDIR)\pmovetst.obj"
	-@erase "$(INTDIR)\r_aclip.obj"
	-@erase "$(INTDIR)\r_aclipa.obj"
	-@erase "$(INTDIR)\r_alias.obj"
	-@erase "$(INTDIR)\r_aliasa.obj"
	-@erase "$(INTDIR)\r_bsp.obj"
	-@erase "$(INTDIR)\r_draw.obj"
	-@erase "$(INTDIR)\r_drawa.obj"
	-@erase "$(INTDIR)\r_edge.obj"
	-@erase "$(INTDIR)\r_edgea.obj"
	-@erase "$(INTDIR)\r_efrag.obj"
	-@erase "$(INTDIR)\r_light.obj"
	-@erase "$(INTDIR)\r_main.obj"
	-@erase "$(INTDIR)\r_misc.obj"
	-@erase "$(INTDIR)\r_part.obj"
	-@erase "$(INTDIR)\r_sky.obj"
	-@erase "$(INTDIR)\r_sprite.obj"
	-@erase "$(INTDIR)\r_surf.obj"
	-@erase "$(INTDIR)\r_vars.obj"
	-@erase "$(INTDIR)\r_varsa.obj"
	-@erase "$(INTDIR)\sbar.obj"
	-@erase "$(INTDIR)\screen.obj"
	-@erase "$(INTDIR)\skin.obj"
	-@erase "$(INTDIR)\snd_dma.obj"
	-@erase "$(INTDIR)\snd_mem.obj"
	-@erase "$(INTDIR)\snd_mix.obj"
	-@erase "$(INTDIR)\snd_mixa.obj"
	-@erase "$(INTDIR)\snd_win.obj"
	-@erase "$(INTDIR)\surf16.obj"
	-@erase "$(INTDIR)\surf8.obj"
	-@erase "$(INTDIR)\sys_win.obj"
	-@erase "$(INTDIR)\sys_wina.obj"
	-@erase "$(INTDIR)\vid_win.obj"
	-@erase "$(INTDIR)\view.obj"
	-@erase "$(INTDIR)\wad.obj"
	-@erase "$(INTDIR)\winquake.res"
	-@erase "$(INTDIR)\zone.obj"
	-@erase "$(OUTDIR)\qwcl.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /GX /O2 /I "..\..\scitech\include" /I "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# SUBTRACT CPP /X
CPP_PROJ=/nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/winquake.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qwcl.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ..\..\scitech\lib\win32\vc\mgllt.lib ..\..\dxsdk\sdk\lib\dxguid.lib comctl32.lib wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=..\..\scitech\lib\win32\vc\mgllt.lib\
 ..\..\dxsdk\sdk\lib\dxguid.lib comctl32.lib wsock32.lib winmm.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /incremental:no /pdb:"$(OUTDIR)/qwcl.pdb" /machine:I386\
 /out:"$(OUTDIR)/qwcl.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cd_win.obj" \
	"$(INTDIR)\cl_cam.obj" \
	"$(INTDIR)\cl_demo.obj" \
	"$(INTDIR)\cl_ents.obj" \
	"$(INTDIR)\cl_input.obj" \
	"$(INTDIR)\cl_main.obj" \
	"$(INTDIR)\cl_parse.obj" \
	"$(INTDIR)\cl_pred.obj" \
	"$(INTDIR)\cl_tent.obj" \
	"$(INTDIR)\cmd.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\console.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cvar.obj" \
	"$(INTDIR)\d_draw.obj" \
	"$(INTDIR)\d_draw16.obj" \
	"$(INTDIR)\d_edge.obj" \
	"$(INTDIR)\d_fill.obj" \
	"$(INTDIR)\d_init.obj" \
	"$(INTDIR)\d_modech.obj" \
	"$(INTDIR)\d_part.obj" \
	"$(INTDIR)\d_parta.obj" \
	"$(INTDIR)\d_polysa.obj" \
	"$(INTDIR)\d_polyse.obj" \
	"$(INTDIR)\d_scan.obj" \
	"$(INTDIR)\d_scana.obj" \
	"$(INTDIR)\d_sky.obj" \
	"$(INTDIR)\d_spr8.obj" \
	"$(INTDIR)\d_sprite.obj" \
	"$(INTDIR)\d_surf.obj" \
	"$(INTDIR)\d_vars.obj" \
	"$(INTDIR)\d_varsa.obj" \
	"$(INTDIR)\d_zpoint.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\in_win.obj" \
	"$(INTDIR)\keys.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\md4.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\model.obj" \
	"$(INTDIR)\net_chan.obj" \
	"$(INTDIR)\net_wins.obj" \
	"$(INTDIR)\pmove.obj" \
	"$(INTDIR)\pmovetst.obj" \
	"$(INTDIR)\r_aclip.obj" \
	"$(INTDIR)\r_aclipa.obj" \
	"$(INTDIR)\r_alias.obj" \
	"$(INTDIR)\r_aliasa.obj" \
	"$(INTDIR)\r_bsp.obj" \
	"$(INTDIR)\r_draw.obj" \
	"$(INTDIR)\r_drawa.obj" \
	"$(INTDIR)\r_edge.obj" \
	"$(INTDIR)\r_edgea.obj" \
	"$(INTDIR)\r_efrag.obj" \
	"$(INTDIR)\r_light.obj" \
	"$(INTDIR)\r_main.obj" \
	"$(INTDIR)\r_misc.obj" \
	"$(INTDIR)\r_part.obj" \
	"$(INTDIR)\r_sky.obj" \
	"$(INTDIR)\r_sprite.obj" \
	"$(INTDIR)\r_surf.obj" \
	"$(INTDIR)\r_vars.obj" \
	"$(INTDIR)\r_varsa.obj" \
	"$(INTDIR)\sbar.obj" \
	"$(INTDIR)\screen.obj" \
	"$(INTDIR)\skin.obj" \
	"$(INTDIR)\snd_dma.obj" \
	"$(INTDIR)\snd_mem.obj" \
	"$(INTDIR)\snd_mix.obj" \
	"$(INTDIR)\snd_mixa.obj" \
	"$(INTDIR)\snd_win.obj" \
	"$(INTDIR)\surf16.obj" \
	"$(INTDIR)\surf8.obj" \
	"$(INTDIR)\sys_win.obj" \
	"$(INTDIR)\sys_wina.obj" \
	"$(INTDIR)\vid_win.obj" \
	"$(INTDIR)\view.obj" \
	"$(INTDIR)\wad.obj" \
	"$(INTDIR)\winquake.res" \
	"$(INTDIR)\zone.obj"

"$(OUTDIR)\qwcl.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "qwcl___0"
# PROP BASE Intermediate_Dir "qwcl___0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "debug"
# PROP Intermediate_Dir "debug"
# PROP Target_Dir ""
OUTDIR=.\debug
INTDIR=.\debug

ALL : "$(OUTDIR)\qwcl.exe" "$(OUTDIR)\qwcl.bsc"

CLEAN : 
	-@erase "$(INTDIR)\cd_win.obj"
	-@erase "$(INTDIR)\cd_win.sbr"
	-@erase "$(INTDIR)\cl_cam.obj"
	-@erase "$(INTDIR)\cl_cam.sbr"
	-@erase "$(INTDIR)\cl_demo.obj"
	-@erase "$(INTDIR)\cl_demo.sbr"
	-@erase "$(INTDIR)\cl_ents.obj"
	-@erase "$(INTDIR)\cl_ents.sbr"
	-@erase "$(INTDIR)\cl_input.obj"
	-@erase "$(INTDIR)\cl_input.sbr"
	-@erase "$(INTDIR)\cl_main.obj"
	-@erase "$(INTDIR)\cl_main.sbr"
	-@erase "$(INTDIR)\cl_parse.obj"
	-@erase "$(INTDIR)\cl_parse.sbr"
	-@erase "$(INTDIR)\cl_pred.obj"
	-@erase "$(INTDIR)\cl_pred.sbr"
	-@erase "$(INTDIR)\cl_tent.obj"
	-@erase "$(INTDIR)\cl_tent.sbr"
	-@erase "$(INTDIR)\cmd.obj"
	-@erase "$(INTDIR)\cmd.sbr"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\common.sbr"
	-@erase "$(INTDIR)\console.obj"
	-@erase "$(INTDIR)\console.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\cvar.obj"
	-@erase "$(INTDIR)\cvar.sbr"
	-@erase "$(INTDIR)\d_draw.obj"
	-@erase "$(INTDIR)\d_draw16.obj"
	-@erase "$(INTDIR)\d_edge.obj"
	-@erase "$(INTDIR)\d_edge.sbr"
	-@erase "$(INTDIR)\d_fill.obj"
	-@erase "$(INTDIR)\d_fill.sbr"
	-@erase "$(INTDIR)\d_init.obj"
	-@erase "$(INTDIR)\d_init.sbr"
	-@erase "$(INTDIR)\d_modech.obj"
	-@erase "$(INTDIR)\d_modech.sbr"
	-@erase "$(INTDIR)\d_part.obj"
	-@erase "$(INTDIR)\d_part.sbr"
	-@erase "$(INTDIR)\d_parta.obj"
	-@erase "$(INTDIR)\d_polysa.obj"
	-@erase "$(INTDIR)\d_polyse.obj"
	-@erase "$(INTDIR)\d_polyse.sbr"
	-@erase "$(INTDIR)\d_scan.obj"
	-@erase "$(INTDIR)\d_scan.sbr"
	-@erase "$(INTDIR)\d_scana.obj"
	-@erase "$(INTDIR)\d_sky.obj"
	-@erase "$(INTDIR)\d_sky.sbr"
	-@erase "$(INTDIR)\d_spr8.obj"
	-@erase "$(INTDIR)\d_sprite.obj"
	-@erase "$(INTDIR)\d_sprite.sbr"
	-@erase "$(INTDIR)\d_surf.obj"
	-@erase "$(INTDIR)\d_surf.sbr"
	-@erase "$(INTDIR)\d_vars.obj"
	-@erase "$(INTDIR)\d_vars.sbr"
	-@erase "$(INTDIR)\d_varsa.obj"
	-@erase "$(INTDIR)\d_zpoint.obj"
	-@erase "$(INTDIR)\d_zpoint.sbr"
	-@erase "$(INTDIR)\draw.obj"
	-@erase "$(INTDIR)\draw.sbr"
	-@erase "$(INTDIR)\in_win.obj"
	-@erase "$(INTDIR)\in_win.sbr"
	-@erase "$(INTDIR)\keys.obj"
	-@erase "$(INTDIR)\keys.sbr"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\md4.obj"
	-@erase "$(INTDIR)\md4.sbr"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\menu.sbr"
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
	-@erase "$(INTDIR)\net_chan.obj"
	-@erase "$(INTDIR)\net_chan.sbr"
	-@erase "$(INTDIR)\net_wins.obj"
	-@erase "$(INTDIR)\net_wins.sbr"
	-@erase "$(INTDIR)\pmove.obj"
	-@erase "$(INTDIR)\pmove.sbr"
	-@erase "$(INTDIR)\pmovetst.obj"
	-@erase "$(INTDIR)\pmovetst.sbr"
	-@erase "$(INTDIR)\r_aclip.obj"
	-@erase "$(INTDIR)\r_aclip.sbr"
	-@erase "$(INTDIR)\r_aclipa.obj"
	-@erase "$(INTDIR)\r_alias.obj"
	-@erase "$(INTDIR)\r_alias.sbr"
	-@erase "$(INTDIR)\r_aliasa.obj"
	-@erase "$(INTDIR)\r_bsp.obj"
	-@erase "$(INTDIR)\r_bsp.sbr"
	-@erase "$(INTDIR)\r_draw.obj"
	-@erase "$(INTDIR)\r_draw.sbr"
	-@erase "$(INTDIR)\r_drawa.obj"
	-@erase "$(INTDIR)\r_edge.obj"
	-@erase "$(INTDIR)\r_edge.sbr"
	-@erase "$(INTDIR)\r_edgea.obj"
	-@erase "$(INTDIR)\r_efrag.obj"
	-@erase "$(INTDIR)\r_efrag.sbr"
	-@erase "$(INTDIR)\r_light.obj"
	-@erase "$(INTDIR)\r_light.sbr"
	-@erase "$(INTDIR)\r_main.obj"
	-@erase "$(INTDIR)\r_main.sbr"
	-@erase "$(INTDIR)\r_misc.obj"
	-@erase "$(INTDIR)\r_misc.sbr"
	-@erase "$(INTDIR)\r_part.obj"
	-@erase "$(INTDIR)\r_part.sbr"
	-@erase "$(INTDIR)\r_sky.obj"
	-@erase "$(INTDIR)\r_sky.sbr"
	-@erase "$(INTDIR)\r_sprite.obj"
	-@erase "$(INTDIR)\r_sprite.sbr"
	-@erase "$(INTDIR)\r_surf.obj"
	-@erase "$(INTDIR)\r_surf.sbr"
	-@erase "$(INTDIR)\r_vars.obj"
	-@erase "$(INTDIR)\r_vars.sbr"
	-@erase "$(INTDIR)\r_varsa.obj"
	-@erase "$(INTDIR)\sbar.obj"
	-@erase "$(INTDIR)\sbar.sbr"
	-@erase "$(INTDIR)\screen.obj"
	-@erase "$(INTDIR)\screen.sbr"
	-@erase "$(INTDIR)\skin.obj"
	-@erase "$(INTDIR)\skin.sbr"
	-@erase "$(INTDIR)\snd_dma.obj"
	-@erase "$(INTDIR)\snd_dma.sbr"
	-@erase "$(INTDIR)\snd_mem.obj"
	-@erase "$(INTDIR)\snd_mem.sbr"
	-@erase "$(INTDIR)\snd_mix.obj"
	-@erase "$(INTDIR)\snd_mix.sbr"
	-@erase "$(INTDIR)\snd_mixa.obj"
	-@erase "$(INTDIR)\snd_win.obj"
	-@erase "$(INTDIR)\snd_win.sbr"
	-@erase "$(INTDIR)\surf16.obj"
	-@erase "$(INTDIR)\surf8.obj"
	-@erase "$(INTDIR)\sys_win.obj"
	-@erase "$(INTDIR)\sys_win.sbr"
	-@erase "$(INTDIR)\sys_wina.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\vid_win.obj"
	-@erase "$(INTDIR)\vid_win.sbr"
	-@erase "$(INTDIR)\view.obj"
	-@erase "$(INTDIR)\view.sbr"
	-@erase "$(INTDIR)\wad.obj"
	-@erase "$(INTDIR)\wad.sbr"
	-@erase "$(INTDIR)\winquake.res"
	-@erase "$(INTDIR)\zone.obj"
	-@erase "$(INTDIR)\zone.sbr"
	-@erase "$(OUTDIR)\qwcl.bsc"
	-@erase "$(OUTDIR)\qwcl.exe"
	-@erase "$(OUTDIR)\qwcl.ilk"
	-@erase "$(OUTDIR)\qwcl.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /Gm /GX /Zi /Od /I "..\..\scitech\include" /I "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# SUBTRACT CPP /X
CPP_PROJ=/nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\debug/
CPP_SBRS=.\debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/winquake.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qwcl.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cd_win.sbr" \
	"$(INTDIR)\cl_cam.sbr" \
	"$(INTDIR)\cl_demo.sbr" \
	"$(INTDIR)\cl_ents.sbr" \
	"$(INTDIR)\cl_input.sbr" \
	"$(INTDIR)\cl_main.sbr" \
	"$(INTDIR)\cl_parse.sbr" \
	"$(INTDIR)\cl_pred.sbr" \
	"$(INTDIR)\cl_tent.sbr" \
	"$(INTDIR)\cmd.sbr" \
	"$(INTDIR)\common.sbr" \
	"$(INTDIR)\console.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\cvar.sbr" \
	"$(INTDIR)\d_edge.sbr" \
	"$(INTDIR)\d_fill.sbr" \
	"$(INTDIR)\d_init.sbr" \
	"$(INTDIR)\d_modech.sbr" \
	"$(INTDIR)\d_part.sbr" \
	"$(INTDIR)\d_polyse.sbr" \
	"$(INTDIR)\d_scan.sbr" \
	"$(INTDIR)\d_sky.sbr" \
	"$(INTDIR)\d_sprite.sbr" \
	"$(INTDIR)\d_surf.sbr" \
	"$(INTDIR)\d_vars.sbr" \
	"$(INTDIR)\d_zpoint.sbr" \
	"$(INTDIR)\draw.sbr" \
	"$(INTDIR)\in_win.sbr" \
	"$(INTDIR)\keys.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\md4.sbr" \
	"$(INTDIR)\menu.sbr" \
	"$(INTDIR)\model.sbr" \
	"$(INTDIR)\net_chan.sbr" \
	"$(INTDIR)\net_wins.sbr" \
	"$(INTDIR)\pmove.sbr" \
	"$(INTDIR)\pmovetst.sbr" \
	"$(INTDIR)\r_aclip.sbr" \
	"$(INTDIR)\r_alias.sbr" \
	"$(INTDIR)\r_bsp.sbr" \
	"$(INTDIR)\r_draw.sbr" \
	"$(INTDIR)\r_edge.sbr" \
	"$(INTDIR)\r_efrag.sbr" \
	"$(INTDIR)\r_light.sbr" \
	"$(INTDIR)\r_main.sbr" \
	"$(INTDIR)\r_misc.sbr" \
	"$(INTDIR)\r_part.sbr" \
	"$(INTDIR)\r_sky.sbr" \
	"$(INTDIR)\r_sprite.sbr" \
	"$(INTDIR)\r_surf.sbr" \
	"$(INTDIR)\r_vars.sbr" \
	"$(INTDIR)\sbar.sbr" \
	"$(INTDIR)\screen.sbr" \
	"$(INTDIR)\skin.sbr" \
	"$(INTDIR)\snd_dma.sbr" \
	"$(INTDIR)\snd_mem.sbr" \
	"$(INTDIR)\snd_mix.sbr" \
	"$(INTDIR)\snd_win.sbr" \
	"$(INTDIR)\sys_win.sbr" \
	"$(INTDIR)\vid_win.sbr" \
	"$(INTDIR)\view.sbr" \
	"$(INTDIR)\wad.sbr" \
	"$(INTDIR)\zone.sbr"

"$(OUTDIR)\qwcl.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 ..\..\scitech\lib\win32\vc\mgllt.lib ..\..\dxsdk\sdk\lib\dxguid.lib comctl32.lib wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# SUBTRACT LINK32 /verbose
LINK32_FLAGS=..\..\scitech\lib\win32\vc\mgllt.lib\
 ..\..\dxsdk\sdk\lib\dxguid.lib comctl32.lib wsock32.lib winmm.lib kernel32.lib\
 user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib\
 ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo\
 /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)/qwcl.pdb" /debug\
 /machine:I386 /out:"$(OUTDIR)/qwcl.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cd_win.obj" \
	"$(INTDIR)\cl_cam.obj" \
	"$(INTDIR)\cl_demo.obj" \
	"$(INTDIR)\cl_ents.obj" \
	"$(INTDIR)\cl_input.obj" \
	"$(INTDIR)\cl_main.obj" \
	"$(INTDIR)\cl_parse.obj" \
	"$(INTDIR)\cl_pred.obj" \
	"$(INTDIR)\cl_tent.obj" \
	"$(INTDIR)\cmd.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\console.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cvar.obj" \
	"$(INTDIR)\d_draw.obj" \
	"$(INTDIR)\d_draw16.obj" \
	"$(INTDIR)\d_edge.obj" \
	"$(INTDIR)\d_fill.obj" \
	"$(INTDIR)\d_init.obj" \
	"$(INTDIR)\d_modech.obj" \
	"$(INTDIR)\d_part.obj" \
	"$(INTDIR)\d_parta.obj" \
	"$(INTDIR)\d_polysa.obj" \
	"$(INTDIR)\d_polyse.obj" \
	"$(INTDIR)\d_scan.obj" \
	"$(INTDIR)\d_scana.obj" \
	"$(INTDIR)\d_sky.obj" \
	"$(INTDIR)\d_spr8.obj" \
	"$(INTDIR)\d_sprite.obj" \
	"$(INTDIR)\d_surf.obj" \
	"$(INTDIR)\d_vars.obj" \
	"$(INTDIR)\d_varsa.obj" \
	"$(INTDIR)\d_zpoint.obj" \
	"$(INTDIR)\draw.obj" \
	"$(INTDIR)\in_win.obj" \
	"$(INTDIR)\keys.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\md4.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\model.obj" \
	"$(INTDIR)\net_chan.obj" \
	"$(INTDIR)\net_wins.obj" \
	"$(INTDIR)\pmove.obj" \
	"$(INTDIR)\pmovetst.obj" \
	"$(INTDIR)\r_aclip.obj" \
	"$(INTDIR)\r_aclipa.obj" \
	"$(INTDIR)\r_alias.obj" \
	"$(INTDIR)\r_aliasa.obj" \
	"$(INTDIR)\r_bsp.obj" \
	"$(INTDIR)\r_draw.obj" \
	"$(INTDIR)\r_drawa.obj" \
	"$(INTDIR)\r_edge.obj" \
	"$(INTDIR)\r_edgea.obj" \
	"$(INTDIR)\r_efrag.obj" \
	"$(INTDIR)\r_light.obj" \
	"$(INTDIR)\r_main.obj" \
	"$(INTDIR)\r_misc.obj" \
	"$(INTDIR)\r_part.obj" \
	"$(INTDIR)\r_sky.obj" \
	"$(INTDIR)\r_sprite.obj" \
	"$(INTDIR)\r_surf.obj" \
	"$(INTDIR)\r_vars.obj" \
	"$(INTDIR)\r_varsa.obj" \
	"$(INTDIR)\sbar.obj" \
	"$(INTDIR)\screen.obj" \
	"$(INTDIR)\skin.obj" \
	"$(INTDIR)\snd_dma.obj" \
	"$(INTDIR)\snd_mem.obj" \
	"$(INTDIR)\snd_mix.obj" \
	"$(INTDIR)\snd_mixa.obj" \
	"$(INTDIR)\snd_win.obj" \
	"$(INTDIR)\surf16.obj" \
	"$(INTDIR)\surf8.obj" \
	"$(INTDIR)\sys_win.obj" \
	"$(INTDIR)\sys_wina.obj" \
	"$(INTDIR)\vid_win.obj" \
	"$(INTDIR)\view.obj" \
	"$(INTDIR)\wad.obj" \
	"$(INTDIR)\winquake.res" \
	"$(INTDIR)\zone.obj"

"$(OUTDIR)\qwcl.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GL Debug"
# PROP BASE Intermediate_Dir "GL Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "GLDebug"
# PROP Intermediate_Dir "GLDebug"
# PROP Target_Dir ""
OUTDIR=.\GLDebug
INTDIR=.\GLDebug

ALL : "$(OUTDIR)\glqwcl.exe" "$(OUTDIR)\qwcl.bsc"

CLEAN : 
	-@erase "$(INTDIR)\cd_win.obj"
	-@erase "$(INTDIR)\cd_win.sbr"
	-@erase "$(INTDIR)\cl_cam.obj"
	-@erase "$(INTDIR)\cl_cam.sbr"
	-@erase "$(INTDIR)\cl_demo.obj"
	-@erase "$(INTDIR)\cl_demo.sbr"
	-@erase "$(INTDIR)\cl_ents.obj"
	-@erase "$(INTDIR)\cl_ents.sbr"
	-@erase "$(INTDIR)\cl_input.obj"
	-@erase "$(INTDIR)\cl_input.sbr"
	-@erase "$(INTDIR)\cl_main.obj"
	-@erase "$(INTDIR)\cl_main.sbr"
	-@erase "$(INTDIR)\cl_parse.obj"
	-@erase "$(INTDIR)\cl_parse.sbr"
	-@erase "$(INTDIR)\cl_pred.obj"
	-@erase "$(INTDIR)\cl_pred.sbr"
	-@erase "$(INTDIR)\cl_tent.obj"
	-@erase "$(INTDIR)\cl_tent.sbr"
	-@erase "$(INTDIR)\cmd.obj"
	-@erase "$(INTDIR)\cmd.sbr"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\common.sbr"
	-@erase "$(INTDIR)\console.obj"
	-@erase "$(INTDIR)\console.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\cvar.obj"
	-@erase "$(INTDIR)\cvar.sbr"
	-@erase "$(INTDIR)\gl_draw.obj"
	-@erase "$(INTDIR)\gl_draw.sbr"
	-@erase "$(INTDIR)\gl_mesh.obj"
	-@erase "$(INTDIR)\gl_mesh.sbr"
	-@erase "$(INTDIR)\gl_model.obj"
	-@erase "$(INTDIR)\gl_model.sbr"
	-@erase "$(INTDIR)\gl_ngraph.obj"
	-@erase "$(INTDIR)\gl_ngraph.sbr"
	-@erase "$(INTDIR)\gl_refrag.obj"
	-@erase "$(INTDIR)\gl_refrag.sbr"
	-@erase "$(INTDIR)\gl_rlight.obj"
	-@erase "$(INTDIR)\gl_rlight.sbr"
	-@erase "$(INTDIR)\gl_rmain.obj"
	-@erase "$(INTDIR)\gl_rmain.sbr"
	-@erase "$(INTDIR)\gl_rmisc.obj"
	-@erase "$(INTDIR)\gl_rmisc.sbr"
	-@erase "$(INTDIR)\gl_rsurf.obj"
	-@erase "$(INTDIR)\gl_rsurf.sbr"
	-@erase "$(INTDIR)\gl_screen.obj"
	-@erase "$(INTDIR)\gl_screen.sbr"
	-@erase "$(INTDIR)\gl_test.obj"
	-@erase "$(INTDIR)\gl_test.sbr"
	-@erase "$(INTDIR)\gl_vidnt.obj"
	-@erase "$(INTDIR)\gl_vidnt.sbr"
	-@erase "$(INTDIR)\gl_warp.obj"
	-@erase "$(INTDIR)\gl_warp.sbr"
	-@erase "$(INTDIR)\in_win.obj"
	-@erase "$(INTDIR)\in_win.sbr"
	-@erase "$(INTDIR)\keys.obj"
	-@erase "$(INTDIR)\keys.sbr"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
	-@erase "$(INTDIR)\md4.obj"
	-@erase "$(INTDIR)\md4.sbr"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\menu.sbr"
	-@erase "$(INTDIR)\net_chan.obj"
	-@erase "$(INTDIR)\net_chan.sbr"
	-@erase "$(INTDIR)\net_wins.obj"
	-@erase "$(INTDIR)\net_wins.sbr"
	-@erase "$(INTDIR)\pmove.obj"
	-@erase "$(INTDIR)\pmove.sbr"
	-@erase "$(INTDIR)\pmovetst.obj"
	-@erase "$(INTDIR)\pmovetst.sbr"
	-@erase "$(INTDIR)\r_part.obj"
	-@erase "$(INTDIR)\r_part.sbr"
	-@erase "$(INTDIR)\sbar.obj"
	-@erase "$(INTDIR)\sbar.sbr"
	-@erase "$(INTDIR)\skin.obj"
	-@erase "$(INTDIR)\skin.sbr"
	-@erase "$(INTDIR)\snd_dma.obj"
	-@erase "$(INTDIR)\snd_dma.sbr"
	-@erase "$(INTDIR)\snd_mem.obj"
	-@erase "$(INTDIR)\snd_mem.sbr"
	-@erase "$(INTDIR)\snd_mix.obj"
	-@erase "$(INTDIR)\snd_mix.sbr"
	-@erase "$(INTDIR)\snd_mixa.obj"
	-@erase "$(INTDIR)\snd_win.obj"
	-@erase "$(INTDIR)\snd_win.sbr"
	-@erase "$(INTDIR)\sys_win.obj"
	-@erase "$(INTDIR)\sys_win.sbr"
	-@erase "$(INTDIR)\sys_wina.obj"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\view.obj"
	-@erase "$(INTDIR)\view.sbr"
	-@erase "$(INTDIR)\wad.obj"
	-@erase "$(INTDIR)\wad.sbr"
	-@erase "$(INTDIR)\winquake.res"
	-@erase "$(INTDIR)\zone.obj"
	-@erase "$(INTDIR)\zone.sbr"
	-@erase "$(OUTDIR)\glqwcl.exe"
	-@erase "$(OUTDIR)\glqwcl.ilk"
	-@erase "$(OUTDIR)\glqwcl.pdb"
	-@erase "$(OUTDIR)\qwcl.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
# ADD CPP /nologo /Gm /GX /Zi /Od /I "..\..\scitech\include" /I "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE" /FR /YX /c
# SUBTRACT CPP /X
CPP_PROJ=/nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\GLDebug/
CPP_SBRS=.\GLDebug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/winquake.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qwcl.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cd_win.sbr" \
	"$(INTDIR)\cl_cam.sbr" \
	"$(INTDIR)\cl_demo.sbr" \
	"$(INTDIR)\cl_ents.sbr" \
	"$(INTDIR)\cl_input.sbr" \
	"$(INTDIR)\cl_main.sbr" \
	"$(INTDIR)\cl_parse.sbr" \
	"$(INTDIR)\cl_pred.sbr" \
	"$(INTDIR)\cl_tent.sbr" \
	"$(INTDIR)\cmd.sbr" \
	"$(INTDIR)\common.sbr" \
	"$(INTDIR)\console.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\cvar.sbr" \
	"$(INTDIR)\gl_draw.sbr" \
	"$(INTDIR)\gl_mesh.sbr" \
	"$(INTDIR)\gl_model.sbr" \
	"$(INTDIR)\gl_ngraph.sbr" \
	"$(INTDIR)\gl_refrag.sbr" \
	"$(INTDIR)\gl_rlight.sbr" \
	"$(INTDIR)\gl_rmain.sbr" \
	"$(INTDIR)\gl_rmisc.sbr" \
	"$(INTDIR)\gl_rsurf.sbr" \
	"$(INTDIR)\gl_screen.sbr" \
	"$(INTDIR)\gl_test.sbr" \
	"$(INTDIR)\gl_vidnt.sbr" \
	"$(INTDIR)\gl_warp.sbr" \
	"$(INTDIR)\in_win.sbr" \
	"$(INTDIR)\keys.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\md4.sbr" \
	"$(INTDIR)\menu.sbr" \
	"$(INTDIR)\net_chan.sbr" \
	"$(INTDIR)\net_wins.sbr" \
	"$(INTDIR)\pmove.sbr" \
	"$(INTDIR)\pmovetst.sbr" \
	"$(INTDIR)\r_part.sbr" \
	"$(INTDIR)\sbar.sbr" \
	"$(INTDIR)\skin.sbr" \
	"$(INTDIR)\snd_dma.sbr" \
	"$(INTDIR)\snd_mem.sbr" \
	"$(INTDIR)\snd_mix.sbr" \
	"$(INTDIR)\snd_win.sbr" \
	"$(INTDIR)\sys_win.sbr" \
	"$(INTDIR)\view.sbr" \
	"$(INTDIR)\wad.sbr" \
	"$(INTDIR)\zone.sbr"

"$(OUTDIR)\qwcl.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 ..\..\dxsdk\sdk\lib\dxguid.lib ..\..\scitech\lib\win32\vc\mgllt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 opengl32.lib ..\..\dxsdk\sdk\lib\dxguid.lib comctl32.lib wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"GLDebug/glqwcl.exe"
LINK32_FLAGS=opengl32.lib ..\..\dxsdk\sdk\lib\dxguid.lib comctl32.lib\
 wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/glqwcl.pdb" /debug /machine:I386 /out:"$(OUTDIR)/glqwcl.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cd_win.obj" \
	"$(INTDIR)\cl_cam.obj" \
	"$(INTDIR)\cl_demo.obj" \
	"$(INTDIR)\cl_ents.obj" \
	"$(INTDIR)\cl_input.obj" \
	"$(INTDIR)\cl_main.obj" \
	"$(INTDIR)\cl_parse.obj" \
	"$(INTDIR)\cl_pred.obj" \
	"$(INTDIR)\cl_tent.obj" \
	"$(INTDIR)\cmd.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\console.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cvar.obj" \
	"$(INTDIR)\gl_draw.obj" \
	"$(INTDIR)\gl_mesh.obj" \
	"$(INTDIR)\gl_model.obj" \
	"$(INTDIR)\gl_ngraph.obj" \
	"$(INTDIR)\gl_refrag.obj" \
	"$(INTDIR)\gl_rlight.obj" \
	"$(INTDIR)\gl_rmain.obj" \
	"$(INTDIR)\gl_rmisc.obj" \
	"$(INTDIR)\gl_rsurf.obj" \
	"$(INTDIR)\gl_screen.obj" \
	"$(INTDIR)\gl_test.obj" \
	"$(INTDIR)\gl_vidnt.obj" \
	"$(INTDIR)\gl_warp.obj" \
	"$(INTDIR)\in_win.obj" \
	"$(INTDIR)\keys.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\md4.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\net_chan.obj" \
	"$(INTDIR)\net_wins.obj" \
	"$(INTDIR)\pmove.obj" \
	"$(INTDIR)\pmovetst.obj" \
	"$(INTDIR)\r_part.obj" \
	"$(INTDIR)\sbar.obj" \
	"$(INTDIR)\skin.obj" \
	"$(INTDIR)\snd_dma.obj" \
	"$(INTDIR)\snd_mem.obj" \
	"$(INTDIR)\snd_mix.obj" \
	"$(INTDIR)\snd_mixa.obj" \
	"$(INTDIR)\snd_win.obj" \
	"$(INTDIR)\sys_win.obj" \
	"$(INTDIR)\sys_wina.obj" \
	"$(INTDIR)\view.obj" \
	"$(INTDIR)\wad.obj" \
	"$(INTDIR)\winquake.res" \
	"$(INTDIR)\zone.obj"

"$(OUTDIR)\glqwcl.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GL Release"
# PROP BASE Intermediate_Dir "GL Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "GLRelease"
# PROP Intermediate_Dir "GLRelease"
# PROP Target_Dir ""
OUTDIR=.\GLRelease
INTDIR=.\GLRelease

ALL : "$(OUTDIR)\glqwcl.exe"

CLEAN : 
	-@erase "$(INTDIR)\cd_win.obj"
	-@erase "$(INTDIR)\cl_cam.obj"
	-@erase "$(INTDIR)\cl_demo.obj"
	-@erase "$(INTDIR)\cl_ents.obj"
	-@erase "$(INTDIR)\cl_input.obj"
	-@erase "$(INTDIR)\cl_main.obj"
	-@erase "$(INTDIR)\cl_parse.obj"
	-@erase "$(INTDIR)\cl_pred.obj"
	-@erase "$(INTDIR)\cl_tent.obj"
	-@erase "$(INTDIR)\cmd.obj"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\console.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\cvar.obj"
	-@erase "$(INTDIR)\gl_draw.obj"
	-@erase "$(INTDIR)\gl_mesh.obj"
	-@erase "$(INTDIR)\gl_model.obj"
	-@erase "$(INTDIR)\gl_ngraph.obj"
	-@erase "$(INTDIR)\gl_refrag.obj"
	-@erase "$(INTDIR)\gl_rlight.obj"
	-@erase "$(INTDIR)\gl_rmain.obj"
	-@erase "$(INTDIR)\gl_rmisc.obj"
	-@erase "$(INTDIR)\gl_rsurf.obj"
	-@erase "$(INTDIR)\gl_screen.obj"
	-@erase "$(INTDIR)\gl_test.obj"
	-@erase "$(INTDIR)\gl_vidnt.obj"
	-@erase "$(INTDIR)\gl_warp.obj"
	-@erase "$(INTDIR)\in_win.obj"
	-@erase "$(INTDIR)\keys.obj"
	-@erase "$(INTDIR)\math.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\md4.obj"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\net_chan.obj"
	-@erase "$(INTDIR)\net_wins.obj"
	-@erase "$(INTDIR)\pmove.obj"
	-@erase "$(INTDIR)\pmovetst.obj"
	-@erase "$(INTDIR)\r_part.obj"
	-@erase "$(INTDIR)\sbar.obj"
	-@erase "$(INTDIR)\skin.obj"
	-@erase "$(INTDIR)\snd_dma.obj"
	-@erase "$(INTDIR)\snd_mem.obj"
	-@erase "$(INTDIR)\snd_mix.obj"
	-@erase "$(INTDIR)\snd_mixa.obj"
	-@erase "$(INTDIR)\snd_win.obj"
	-@erase "$(INTDIR)\sys_win.obj"
	-@erase "$(INTDIR)\sys_wina.obj"
	-@erase "$(INTDIR)\view.obj"
	-@erase "$(INTDIR)\wad.obj"
	-@erase "$(INTDIR)\winquake.res"
	-@erase "$(INTDIR)\zone.obj"
	-@erase "$(OUTDIR)\glqwcl.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /GX /O2 /I "..\..\scitech\include" /I "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE" /YX /c
# SUBTRACT CPP /X
CPP_PROJ=/nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\GLRelease/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/winquake.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qwcl.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 ..\..\dxsdk\sdk\lib\dxguid.lib ..\..\scitech\lib\win32\vc\mgllt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 opengl32.lib ..\..\dxsdk\sdk\lib\dxguid.lib comctl32.lib wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"GLRelease/glqwcl.exe"
LINK32_FLAGS=opengl32.lib ..\..\dxsdk\sdk\lib\dxguid.lib comctl32.lib\
 wsock32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/glqwcl.pdb" /machine:I386 /out:"$(OUTDIR)/glqwcl.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cd_win.obj" \
	"$(INTDIR)\cl_cam.obj" \
	"$(INTDIR)\cl_demo.obj" \
	"$(INTDIR)\cl_ents.obj" \
	"$(INTDIR)\cl_input.obj" \
	"$(INTDIR)\cl_main.obj" \
	"$(INTDIR)\cl_parse.obj" \
	"$(INTDIR)\cl_pred.obj" \
	"$(INTDIR)\cl_tent.obj" \
	"$(INTDIR)\cmd.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\console.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cvar.obj" \
	"$(INTDIR)\gl_draw.obj" \
	"$(INTDIR)\gl_mesh.obj" \
	"$(INTDIR)\gl_model.obj" \
	"$(INTDIR)\gl_ngraph.obj" \
	"$(INTDIR)\gl_refrag.obj" \
	"$(INTDIR)\gl_rlight.obj" \
	"$(INTDIR)\gl_rmain.obj" \
	"$(INTDIR)\gl_rmisc.obj" \
	"$(INTDIR)\gl_rsurf.obj" \
	"$(INTDIR)\gl_screen.obj" \
	"$(INTDIR)\gl_test.obj" \
	"$(INTDIR)\gl_vidnt.obj" \
	"$(INTDIR)\gl_warp.obj" \
	"$(INTDIR)\in_win.obj" \
	"$(INTDIR)\keys.obj" \
	"$(INTDIR)\math.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\md4.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\net_chan.obj" \
	"$(INTDIR)\net_wins.obj" \
	"$(INTDIR)\pmove.obj" \
	"$(INTDIR)\pmovetst.obj" \
	"$(INTDIR)\r_part.obj" \
	"$(INTDIR)\sbar.obj" \
	"$(INTDIR)\skin.obj" \
	"$(INTDIR)\snd_dma.obj" \
	"$(INTDIR)\snd_mem.obj" \
	"$(INTDIR)\snd_mix.obj" \
	"$(INTDIR)\snd_mixa.obj" \
	"$(INTDIR)\snd_win.obj" \
	"$(INTDIR)\sys_win.obj" \
	"$(INTDIR)\sys_wina.obj" \
	"$(INTDIR)\view.obj" \
	"$(INTDIR)\wad.obj" \
	"$(INTDIR)\winquake.res" \
	"$(INTDIR)\zone.obj"

"$(OUTDIR)\glqwcl.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "qwcl - Win32 Release"
# Name "qwcl - Win32 Debug"
# Name "qwcl - Win32 GL Debug"
# Name "qwcl - Win32 GL Release"

!IF  "$(CFG)" == "qwcl - Win32 Release"

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\cd_win.c
DEP_CPP_CD_WI=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CD_WI=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\cd_win.obj" : $(SOURCE) $(DEP_CPP_CD_WI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\cd_win.obj" : $(SOURCE) $(DEP_CPP_CD_WI) "$(INTDIR)"

"$(INTDIR)\cd_win.sbr" : $(SOURCE) $(DEP_CPP_CD_WI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\cd_win.obj" : $(SOURCE) $(DEP_CPP_CD_WI) "$(INTDIR)"

"$(INTDIR)\cd_win.sbr" : $(SOURCE) $(DEP_CPP_CD_WI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\cd_win.obj" : $(SOURCE) $(DEP_CPP_CD_WI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cl_cam.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_CL_CA=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_CL_CA=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_cam.obj" : $(SOURCE) $(DEP_CPP_CL_CA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_CL_CA=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_CL_CA=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_cam.obj" : $(SOURCE) $(DEP_CPP_CL_CA) "$(INTDIR)"

"$(INTDIR)\cl_cam.sbr" : $(SOURCE) $(DEP_CPP_CL_CA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_CL_CA=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_CL_CA=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_cam.obj" : $(SOURCE) $(DEP_CPP_CL_CA) "$(INTDIR)"

"$(INTDIR)\cl_cam.sbr" : $(SOURCE) $(DEP_CPP_CL_CA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_CL_CA=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_CL_CA=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_cam.obj" : $(SOURCE) $(DEP_CPP_CL_CA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cl_demo.c
DEP_CPP_CL_DE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CL_DE=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\cl_demo.obj" : $(SOURCE) $(DEP_CPP_CL_DE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\cl_demo.obj" : $(SOURCE) $(DEP_CPP_CL_DE) "$(INTDIR)"

"$(INTDIR)\cl_demo.sbr" : $(SOURCE) $(DEP_CPP_CL_DE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\cl_demo.obj" : $(SOURCE) $(DEP_CPP_CL_DE) "$(INTDIR)"

"$(INTDIR)\cl_demo.sbr" : $(SOURCE) $(DEP_CPP_CL_DE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\cl_demo.obj" : $(SOURCE) $(DEP_CPP_CL_DE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cl_ents.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_CL_EN=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CL_EN=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_ents.obj" : $(SOURCE) $(DEP_CPP_CL_EN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_CL_EN=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CL_EN=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_ents.obj" : $(SOURCE) $(DEP_CPP_CL_EN) "$(INTDIR)"

"$(INTDIR)\cl_ents.sbr" : $(SOURCE) $(DEP_CPP_CL_EN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_CL_EN=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\zone.h"\
	

"$(INTDIR)\cl_ents.obj" : $(SOURCE) $(DEP_CPP_CL_EN) "$(INTDIR)"

"$(INTDIR)\cl_ents.sbr" : $(SOURCE) $(DEP_CPP_CL_EN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_CL_EN=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CL_EN=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_ents.obj" : $(SOURCE) $(DEP_CPP_CL_EN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cl_input.c
DEP_CPP_CL_IN=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CL_IN=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\cl_input.obj" : $(SOURCE) $(DEP_CPP_CL_IN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\cl_input.obj" : $(SOURCE) $(DEP_CPP_CL_IN) "$(INTDIR)"

"$(INTDIR)\cl_input.sbr" : $(SOURCE) $(DEP_CPP_CL_IN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\cl_input.obj" : $(SOURCE) $(DEP_CPP_CL_IN) "$(INTDIR)"

"$(INTDIR)\cl_input.sbr" : $(SOURCE) $(DEP_CPP_CL_IN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\cl_input.obj" : $(SOURCE) $(DEP_CPP_CL_IN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cl_main.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_CL_MA=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_CL_MA=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\cl_main.obj" : $(SOURCE) $(DEP_CPP_CL_MA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_CL_MA=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_CL_MA=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\cl_main.obj" : $(SOURCE) $(DEP_CPP_CL_MA) "$(INTDIR)"

"$(INTDIR)\cl_main.sbr" : $(SOURCE) $(DEP_CPP_CL_MA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_CL_MA=\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	

"$(INTDIR)\cl_main.obj" : $(SOURCE) $(DEP_CPP_CL_MA) "$(INTDIR)"

"$(INTDIR)\cl_main.sbr" : $(SOURCE) $(DEP_CPP_CL_MA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_CL_MA=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\quakedef.h"\
	".\winquake.h"\
	
NODEP_CPP_CL_MA=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\cl_main.obj" : $(SOURCE) $(DEP_CPP_CL_MA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cl_parse.c
DEP_CPP_CL_PA=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CL_PA=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\cl_parse.obj" : $(SOURCE) $(DEP_CPP_CL_PA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\cl_parse.obj" : $(SOURCE) $(DEP_CPP_CL_PA) "$(INTDIR)"

"$(INTDIR)\cl_parse.sbr" : $(SOURCE) $(DEP_CPP_CL_PA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\cl_parse.obj" : $(SOURCE) $(DEP_CPP_CL_PA) "$(INTDIR)"

"$(INTDIR)\cl_parse.sbr" : $(SOURCE) $(DEP_CPP_CL_PA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\cl_parse.obj" : $(SOURCE) $(DEP_CPP_CL_PA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cl_pred.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_CL_PR=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_CL_PR=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_pred.obj" : $(SOURCE) $(DEP_CPP_CL_PR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_CL_PR=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_CL_PR=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_pred.obj" : $(SOURCE) $(DEP_CPP_CL_PR) "$(INTDIR)"

"$(INTDIR)\cl_pred.sbr" : $(SOURCE) $(DEP_CPP_CL_PR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_CL_PR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_CL_PR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_pred.obj" : $(SOURCE) $(DEP_CPP_CL_PR) "$(INTDIR)"

"$(INTDIR)\cl_pred.sbr" : $(SOURCE) $(DEP_CPP_CL_PR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_CL_PR=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_CL_PR=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\cl_pred.obj" : $(SOURCE) $(DEP_CPP_CL_PR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cl_tent.c
DEP_CPP_CL_TE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CL_TE=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\cl_tent.obj" : $(SOURCE) $(DEP_CPP_CL_TE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\cl_tent.obj" : $(SOURCE) $(DEP_CPP_CL_TE) "$(INTDIR)"

"$(INTDIR)\cl_tent.sbr" : $(SOURCE) $(DEP_CPP_CL_TE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\cl_tent.obj" : $(SOURCE) $(DEP_CPP_CL_TE) "$(INTDIR)"

"$(INTDIR)\cl_tent.sbr" : $(SOURCE) $(DEP_CPP_CL_TE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\cl_tent.obj" : $(SOURCE) $(DEP_CPP_CL_TE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cmd.c
DEP_CPP_CMD_C=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CMD_C=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\cmd.obj" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\cmd.obj" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"

"$(INTDIR)\cmd.sbr" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\cmd.obj" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"

"$(INTDIR)\cmd.sbr" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\cmd.obj" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\common.c
DEP_CPP_COMMO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_COMMO=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\qwsvdef.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\common.obj" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\common.obj" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"

"$(INTDIR)\common.sbr" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\common.obj" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"

"$(INTDIR)\common.sbr" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\common.obj" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\console.c
DEP_CPP_CONSO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CONSO=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\console.obj" : $(SOURCE) $(DEP_CPP_CONSO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\console.obj" : $(SOURCE) $(DEP_CPP_CONSO) "$(INTDIR)"

"$(INTDIR)\console.sbr" : $(SOURCE) $(DEP_CPP_CONSO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\console.obj" : $(SOURCE) $(DEP_CPP_CONSO) "$(INTDIR)"

"$(INTDIR)\console.sbr" : $(SOURCE) $(DEP_CPP_CONSO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\console.obj" : $(SOURCE) $(DEP_CPP_CONSO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\crc.c
DEP_CPP_CRC_C=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CRC_C=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\crc.obj" : $(SOURCE) $(DEP_CPP_CRC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\crc.obj" : $(SOURCE) $(DEP_CPP_CRC_C) "$(INTDIR)"

"$(INTDIR)\crc.sbr" : $(SOURCE) $(DEP_CPP_CRC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\crc.obj" : $(SOURCE) $(DEP_CPP_CRC_C) "$(INTDIR)"

"$(INTDIR)\crc.sbr" : $(SOURCE) $(DEP_CPP_CRC_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\crc.obj" : $(SOURCE) $(DEP_CPP_CRC_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\cvar.c
DEP_CPP_CVAR_=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_CVAR_=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\qwsvdef.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\cvar.obj" : $(SOURCE) $(DEP_CPP_CVAR_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\cvar.obj" : $(SOURCE) $(DEP_CPP_CVAR_) "$(INTDIR)"

"$(INTDIR)\cvar.sbr" : $(SOURCE) $(DEP_CPP_CVAR_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\cvar.obj" : $(SOURCE) $(DEP_CPP_CVAR_) "$(INTDIR)"

"$(INTDIR)\cvar.sbr" : $(SOURCE) $(DEP_CPP_CVAR_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\cvar.obj" : $(SOURCE) $(DEP_CPP_CVAR_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_edge.c
DEP_CPP_D_EDG=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_EDG=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_edge.obj" : $(SOURCE) $(DEP_CPP_D_EDG) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_edge.obj" : $(SOURCE) $(DEP_CPP_D_EDG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_edge.sbr" : $(SOURCE) $(DEP_CPP_D_EDG) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_fill.c
DEP_CPP_D_FIL=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_FIL=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_fill.obj" : $(SOURCE) $(DEP_CPP_D_FIL) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_fill.obj" : $(SOURCE) $(DEP_CPP_D_FIL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_fill.sbr" : $(SOURCE) $(DEP_CPP_D_FIL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_init.c
DEP_CPP_D_INI=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_INI=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_init.obj" : $(SOURCE) $(DEP_CPP_D_INI) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_init.obj" : $(SOURCE) $(DEP_CPP_D_INI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_init.sbr" : $(SOURCE) $(DEP_CPP_D_INI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_modech.c
DEP_CPP_D_MOD=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_MOD=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_modech.obj" : $(SOURCE) $(DEP_CPP_D_MOD) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_modech.obj" : $(SOURCE) $(DEP_CPP_D_MOD) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_modech.sbr" : $(SOURCE) $(DEP_CPP_D_MOD) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_part.c
DEP_CPP_D_PAR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_PAR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_part.obj" : $(SOURCE) $(DEP_CPP_D_PAR) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_part.obj" : $(SOURCE) $(DEP_CPP_D_PAR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_part.sbr" : $(SOURCE) $(DEP_CPP_D_PAR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_polyse.c
DEP_CPP_D_POL=\
	".\adivtab.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_POL=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_polyse.obj" : $(SOURCE) $(DEP_CPP_D_POL) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_polyse.obj" : $(SOURCE) $(DEP_CPP_D_POL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_polyse.sbr" : $(SOURCE) $(DEP_CPP_D_POL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_scan.c
DEP_CPP_D_SCA=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_SCA=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_scan.obj" : $(SOURCE) $(DEP_CPP_D_SCA) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_scan.obj" : $(SOURCE) $(DEP_CPP_D_SCA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_scan.sbr" : $(SOURCE) $(DEP_CPP_D_SCA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_sky.c
DEP_CPP_D_SKY=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_SKY=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_sky.obj" : $(SOURCE) $(DEP_CPP_D_SKY) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_sky.obj" : $(SOURCE) $(DEP_CPP_D_SKY) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_sky.sbr" : $(SOURCE) $(DEP_CPP_D_SKY) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_sprite.c
DEP_CPP_D_SPR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_SPR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_sprite.obj" : $(SOURCE) $(DEP_CPP_D_SPR) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_sprite.obj" : $(SOURCE) $(DEP_CPP_D_SPR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_sprite.sbr" : $(SOURCE) $(DEP_CPP_D_SPR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_surf.c
DEP_CPP_D_SUR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_SUR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_surf.obj" : $(SOURCE) $(DEP_CPP_D_SUR) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_surf.obj" : $(SOURCE) $(DEP_CPP_D_SUR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_surf.sbr" : $(SOURCE) $(DEP_CPP_D_SUR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_vars.c
DEP_CPP_D_VAR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_VAR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_vars.obj" : $(SOURCE) $(DEP_CPP_D_VAR) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_vars.obj" : $(SOURCE) $(DEP_CPP_D_VAR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_vars.sbr" : $(SOURCE) $(DEP_CPP_D_VAR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_zpoint.c
DEP_CPP_D_ZPO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_D_ZPO=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\d_zpoint.obj" : $(SOURCE) $(DEP_CPP_D_ZPO) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\d_zpoint.obj" : $(SOURCE) $(DEP_CPP_D_ZPO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\d_zpoint.sbr" : $(SOURCE) $(DEP_CPP_D_ZPO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\draw.c
DEP_CPP_DRAW_=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_DRAW_=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\draw.obj" : $(SOURCE) $(DEP_CPP_DRAW_) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\draw.obj" : $(SOURCE) $(DEP_CPP_DRAW_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\draw.sbr" : $(SOURCE) $(DEP_CPP_DRAW_) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_draw.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1
DEP_CPP_GL_DR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_DR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1
DEP_CPP_GL_DR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_DR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_GL_DR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\zone.h"\
	

BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_draw.obj" : $(SOURCE) $(DEP_CPP_GL_DR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_draw.sbr" : $(SOURCE) $(DEP_CPP_GL_DR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_GL_DR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_DR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\gl_draw.obj" : $(SOURCE) $(DEP_CPP_GL_DR) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_mesh.c
DEP_CPP_GL_ME=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_ME=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_mesh.obj" : $(SOURCE) $(DEP_CPP_GL_ME) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_mesh.sbr" : $(SOURCE) $(DEP_CPP_GL_ME) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\gl_mesh.obj" : $(SOURCE) $(DEP_CPP_GL_ME) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_model.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1
DEP_CPP_GL_MO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_MO=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1
DEP_CPP_GL_MO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_MO=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_GL_MO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\zone.h"\
	

BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_model.obj" : $(SOURCE) $(DEP_CPP_GL_MO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_model.sbr" : $(SOURCE) $(DEP_CPP_GL_MO) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_GL_MO=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_MO=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\gl_model.obj" : $(SOURCE) $(DEP_CPP_GL_MO) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_ngraph.c
DEP_CPP_GL_NG=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_NG=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_ngraph.obj" : $(SOURCE) $(DEP_CPP_GL_NG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_ngraph.sbr" : $(SOURCE) $(DEP_CPP_GL_NG) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\gl_ngraph.obj" : $(SOURCE) $(DEP_CPP_GL_NG) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_refrag.c
DEP_CPP_GL_RE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_RE=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_refrag.obj" : $(SOURCE) $(DEP_CPP_GL_RE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_refrag.sbr" : $(SOURCE) $(DEP_CPP_GL_RE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\gl_refrag.obj" : $(SOURCE) $(DEP_CPP_GL_RE) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_rlight.c
DEP_CPP_GL_RL=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_RL=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_rlight.obj" : $(SOURCE) $(DEP_CPP_GL_RL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_rlight.sbr" : $(SOURCE) $(DEP_CPP_GL_RL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\gl_rlight.obj" : $(SOURCE) $(DEP_CPP_GL_RL) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_rmain.c
DEP_CPP_GL_RM=\
	".\anorm_dots.h"\
	".\anorms.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_RM=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_rmain.obj" : $(SOURCE) $(DEP_CPP_GL_RM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_rmain.sbr" : $(SOURCE) $(DEP_CPP_GL_RM) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\gl_rmain.obj" : $(SOURCE) $(DEP_CPP_GL_RM) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_rmisc.c
DEP_CPP_GL_RMI=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_RMI=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_rmisc.obj" : $(SOURCE) $(DEP_CPP_GL_RMI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_rmisc.sbr" : $(SOURCE) $(DEP_CPP_GL_RMI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\gl_rmisc.obj" : $(SOURCE) $(DEP_CPP_GL_RMI) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_rsurf.c
DEP_CPP_GL_RS=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_RS=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_rsurf.obj" : $(SOURCE) $(DEP_CPP_GL_RS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_rsurf.sbr" : $(SOURCE) $(DEP_CPP_GL_RS) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\gl_rsurf.obj" : $(SOURCE) $(DEP_CPP_GL_RS) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_screen.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1
DEP_CPP_GL_SC=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_SC=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1
DEP_CPP_GL_SC=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_SC=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_GL_SC=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\zone.h"\
	

BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_screen.obj" : $(SOURCE) $(DEP_CPP_GL_SC) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_screen.sbr" : $(SOURCE) $(DEP_CPP_GL_SC) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_GL_SC=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_SC=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\gl_screen.obj" : $(SOURCE) $(DEP_CPP_GL_SC) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_test.c
DEP_CPP_GL_TE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_TE=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_test.obj" : $(SOURCE) $(DEP_CPP_GL_TE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_test.sbr" : $(SOURCE) $(DEP_CPP_GL_TE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\gl_test.obj" : $(SOURCE) $(DEP_CPP_GL_TE) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_vidnt.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1
DEP_CPP_GL_VI=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_GL_VI=\
	"..\..\scitech\include\mglrect.hpp"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1
DEP_CPP_GL_VI=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_GL_VI=\
	"..\..\scitech\include\mglrect.hpp"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_GL_VI=\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	

BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_vidnt.obj" : $(SOURCE) $(DEP_CPP_GL_VI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_vidnt.sbr" : $(SOURCE) $(DEP_CPP_GL_VI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_GL_VI=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\quakedef.h"\
	".\winquake.h"\
	
NODEP_CPP_GL_VI=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\gl_vidnt.obj" : $(SOURCE) $(DEP_CPP_GL_VI) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\gl_warp.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

# PROP Exclude_From_Build 1
DEP_CPP_GL_WA=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\gl_warp_sin.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_WA=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# PROP Exclude_From_Build 1
DEP_CPP_GL_WA=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\gl_warp_sin.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_WA=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_GL_WA=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\gl_warp_sin.h"\
	".\mathlib.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\zone.h"\
	

BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 $(SOURCE) \
	

"$(INTDIR)\gl_warp.obj" : $(SOURCE) $(DEP_CPP_GL_WA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\gl_warp.sbr" : $(SOURCE) $(DEP_CPP_GL_WA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_GL_WA=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\gl_warp_sin.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_GL_WA=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\gl_warp.obj" : $(SOURCE) $(DEP_CPP_GL_WA) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "GLQUAKE"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\in_win.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_IN_WI=\
	"..\..\dxsdk\sdk\inc\dinput.h"\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_IN_WI=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\in_win.obj" : $(SOURCE) $(DEP_CPP_IN_WI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_IN_WI=\
	"..\..\dxsdk\sdk\inc\dinput.h"\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_IN_WI=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\in_win.obj" : $(SOURCE) $(DEP_CPP_IN_WI) "$(INTDIR)"

"$(INTDIR)\in_win.sbr" : $(SOURCE) $(DEP_CPP_IN_WI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_IN_WI=\
	"..\..\dxsdk\sdk\inc\dinput.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	

"$(INTDIR)\in_win.obj" : $(SOURCE) $(DEP_CPP_IN_WI) "$(INTDIR)"

"$(INTDIR)\in_win.sbr" : $(SOURCE) $(DEP_CPP_IN_WI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_IN_WI=\
	"..\..\dxsdk\sdk\inc\dinput.h"\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_IN_WI=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\in_win.obj" : $(SOURCE) $(DEP_CPP_IN_WI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\keys.c
DEP_CPP_KEYS_=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_KEYS_=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\keys.obj" : $(SOURCE) $(DEP_CPP_KEYS_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\keys.obj" : $(SOURCE) $(DEP_CPP_KEYS_) "$(INTDIR)"

"$(INTDIR)\keys.sbr" : $(SOURCE) $(DEP_CPP_KEYS_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\keys.obj" : $(SOURCE) $(DEP_CPP_KEYS_) "$(INTDIR)"

"$(INTDIR)\keys.sbr" : $(SOURCE) $(DEP_CPP_KEYS_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\keys.obj" : $(SOURCE) $(DEP_CPP_KEYS_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\mathlib.c
DEP_CPP_MATHL=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_MATHL=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"

"$(INTDIR)\mathlib.sbr" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"

"$(INTDIR)\mathlib.sbr" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\md4.c

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\md4.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\md4.obj" : $(SOURCE) "$(INTDIR)"

"$(INTDIR)\md4.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\md4.obj" : $(SOURCE) "$(INTDIR)"

"$(INTDIR)\md4.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\md4.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\menu.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_MENU_=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_MENU_=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\menu.obj" : $(SOURCE) $(DEP_CPP_MENU_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_MENU_=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_MENU_=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\menu.obj" : $(SOURCE) $(DEP_CPP_MENU_) "$(INTDIR)"

"$(INTDIR)\menu.sbr" : $(SOURCE) $(DEP_CPP_MENU_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_MENU_=\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	

"$(INTDIR)\menu.obj" : $(SOURCE) $(DEP_CPP_MENU_) "$(INTDIR)"

"$(INTDIR)\menu.sbr" : $(SOURCE) $(DEP_CPP_MENU_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_MENU_=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\quakedef.h"\
	".\winquake.h"\
	
NODEP_CPP_MENU_=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\menu.obj" : $(SOURCE) $(DEP_CPP_MENU_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\model.c
DEP_CPP_MODEL=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_MODEL=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\model.obj" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\model.obj" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\model.sbr" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\net_chan.c
DEP_CPP_NET_C=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_NET_C=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\net_chan.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\net_chan.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"

"$(INTDIR)\net_chan.sbr" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\net_chan.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"

"$(INTDIR)\net_chan.sbr" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\net_chan.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\net_wins.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_NET_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_NET_W=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\net_wins.obj" : $(SOURCE) $(DEP_CPP_NET_W) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_NET_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_NET_W=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\net_wins.obj" : $(SOURCE) $(DEP_CPP_NET_W) "$(INTDIR)"

"$(INTDIR)\net_wins.sbr" : $(SOURCE) $(DEP_CPP_NET_W) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_NET_W=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_NET_W=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\net_wins.obj" : $(SOURCE) $(DEP_CPP_NET_W) "$(INTDIR)"

"$(INTDIR)\net_wins.sbr" : $(SOURCE) $(DEP_CPP_NET_W) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_NET_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_NET_W=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\net_wins.obj" : $(SOURCE) $(DEP_CPP_NET_W) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pmove.c
DEP_CPP_PMOVE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_PMOVE=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\pmove.obj" : $(SOURCE) $(DEP_CPP_PMOVE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\pmove.obj" : $(SOURCE) $(DEP_CPP_PMOVE) "$(INTDIR)"

"$(INTDIR)\pmove.sbr" : $(SOURCE) $(DEP_CPP_PMOVE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\pmove.obj" : $(SOURCE) $(DEP_CPP_PMOVE) "$(INTDIR)"

"$(INTDIR)\pmove.sbr" : $(SOURCE) $(DEP_CPP_PMOVE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\pmove.obj" : $(SOURCE) $(DEP_CPP_PMOVE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pmovetst.c
DEP_CPP_PMOVET=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_PMOVET=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\pmovetst.obj" : $(SOURCE) $(DEP_CPP_PMOVET) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\pmovetst.obj" : $(SOURCE) $(DEP_CPP_PMOVET) "$(INTDIR)"

"$(INTDIR)\pmovetst.sbr" : $(SOURCE) $(DEP_CPP_PMOVET) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\pmovetst.obj" : $(SOURCE) $(DEP_CPP_PMOVET) "$(INTDIR)"

"$(INTDIR)\pmovetst.sbr" : $(SOURCE) $(DEP_CPP_PMOVET) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\pmovetst.obj" : $(SOURCE) $(DEP_CPP_PMOVET) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_aclip.c
DEP_CPP_R_ACL=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_ACL=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_aclip.obj" : $(SOURCE) $(DEP_CPP_R_ACL) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_aclip.obj" : $(SOURCE) $(DEP_CPP_R_ACL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_aclip.sbr" : $(SOURCE) $(DEP_CPP_R_ACL) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_alias.c
DEP_CPP_R_ALI=\
	".\anorms.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_ALI=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_alias.obj" : $(SOURCE) $(DEP_CPP_R_ALI) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_alias.obj" : $(SOURCE) $(DEP_CPP_R_ALI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_alias.sbr" : $(SOURCE) $(DEP_CPP_R_ALI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_bsp.c
DEP_CPP_R_BSP=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_BSP=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_bsp.obj" : $(SOURCE) $(DEP_CPP_R_BSP) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_bsp.obj" : $(SOURCE) $(DEP_CPP_R_BSP) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_bsp.sbr" : $(SOURCE) $(DEP_CPP_R_BSP) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_draw.c
DEP_CPP_R_DRA=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_DRA=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_draw.obj" : $(SOURCE) $(DEP_CPP_R_DRA) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_draw.obj" : $(SOURCE) $(DEP_CPP_R_DRA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_draw.sbr" : $(SOURCE) $(DEP_CPP_R_DRA) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_edge.c
DEP_CPP_R_EDG=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_EDG=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_edge.obj" : $(SOURCE) $(DEP_CPP_R_EDG) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_edge.obj" : $(SOURCE) $(DEP_CPP_R_EDG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_edge.sbr" : $(SOURCE) $(DEP_CPP_R_EDG) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_efrag.c
DEP_CPP_R_EFR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_EFR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_efrag.obj" : $(SOURCE) $(DEP_CPP_R_EFR) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_efrag.obj" : $(SOURCE) $(DEP_CPP_R_EFR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_efrag.sbr" : $(SOURCE) $(DEP_CPP_R_EFR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_light.c
DEP_CPP_R_LIG=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_LIG=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_light.obj" : $(SOURCE) $(DEP_CPP_R_LIG) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_light.obj" : $(SOURCE) $(DEP_CPP_R_LIG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_light.sbr" : $(SOURCE) $(DEP_CPP_R_LIG) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_main.c
DEP_CPP_R_MAI=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_MAI=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_main.obj" : $(SOURCE) $(DEP_CPP_R_MAI) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_main.obj" : $(SOURCE) $(DEP_CPP_R_MAI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_main.sbr" : $(SOURCE) $(DEP_CPP_R_MAI) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_misc.c
DEP_CPP_R_MIS=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_MIS=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_misc.obj" : $(SOURCE) $(DEP_CPP_R_MIS) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_misc.obj" : $(SOURCE) $(DEP_CPP_R_MIS) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_misc.sbr" : $(SOURCE) $(DEP_CPP_R_MIS) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_part.c
DEP_CPP_R_PAR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_PAR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_part.obj" : $(SOURCE) $(DEP_CPP_R_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\r_part.obj" : $(SOURCE) $(DEP_CPP_R_PAR) "$(INTDIR)"

"$(INTDIR)\r_part.sbr" : $(SOURCE) $(DEP_CPP_R_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 0

"$(INTDIR)\r_part.obj" : $(SOURCE) $(DEP_CPP_R_PAR) "$(INTDIR)"

"$(INTDIR)\r_part.sbr" : $(SOURCE) $(DEP_CPP_R_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 0

"$(INTDIR)\r_part.obj" : $(SOURCE) $(DEP_CPP_R_PAR) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_sky.c
DEP_CPP_R_SKY=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_SKY=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_sky.obj" : $(SOURCE) $(DEP_CPP_R_SKY) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_sky.obj" : $(SOURCE) $(DEP_CPP_R_SKY) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_sky.sbr" : $(SOURCE) $(DEP_CPP_R_SKY) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_sprite.c
DEP_CPP_R_SPR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_SPR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_sprite.obj" : $(SOURCE) $(DEP_CPP_R_SPR) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_sprite.obj" : $(SOURCE) $(DEP_CPP_R_SPR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_sprite.sbr" : $(SOURCE) $(DEP_CPP_R_SPR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_surf.c
DEP_CPP_R_SUR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_SUR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_surf.obj" : $(SOURCE) $(DEP_CPP_R_SUR) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_surf.obj" : $(SOURCE) $(DEP_CPP_R_SUR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_surf.sbr" : $(SOURCE) $(DEP_CPP_R_SUR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_vars.c
DEP_CPP_R_VAR=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_R_VAR=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\r_vars.obj" : $(SOURCE) $(DEP_CPP_R_VAR) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\r_vars.obj" : $(SOURCE) $(DEP_CPP_R_VAR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\r_vars.sbr" : $(SOURCE) $(DEP_CPP_R_VAR) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sbar.c
DEP_CPP_SBAR_=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_SBAR_=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\sbar.obj" : $(SOURCE) $(DEP_CPP_SBAR_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\sbar.obj" : $(SOURCE) $(DEP_CPP_SBAR_) "$(INTDIR)"

"$(INTDIR)\sbar.sbr" : $(SOURCE) $(DEP_CPP_SBAR_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\sbar.obj" : $(SOURCE) $(DEP_CPP_SBAR_) "$(INTDIR)"

"$(INTDIR)\sbar.sbr" : $(SOURCE) $(DEP_CPP_SBAR_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\sbar.obj" : $(SOURCE) $(DEP_CPP_SBAR_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\screen.c
DEP_CPP_SCREE=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_SCREE=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\screen.obj" : $(SOURCE) $(DEP_CPP_SCREE) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\screen.obj" : $(SOURCE) $(DEP_CPP_SCREE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\screen.sbr" : $(SOURCE) $(DEP_CPP_SCREE) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\skin.c
DEP_CPP_SKIN_=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_SKIN_=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\skin.obj" : $(SOURCE) $(DEP_CPP_SKIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\skin.obj" : $(SOURCE) $(DEP_CPP_SKIN_) "$(INTDIR)"

"$(INTDIR)\skin.sbr" : $(SOURCE) $(DEP_CPP_SKIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\skin.obj" : $(SOURCE) $(DEP_CPP_SKIN_) "$(INTDIR)"

"$(INTDIR)\skin.sbr" : $(SOURCE) $(DEP_CPP_SKIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\skin.obj" : $(SOURCE) $(DEP_CPP_SKIN_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\snd_dma.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_SND_D=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_D=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_dma.obj" : $(SOURCE) $(DEP_CPP_SND_D) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_SND_D=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_D=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_dma.obj" : $(SOURCE) $(DEP_CPP_SND_D) "$(INTDIR)"

"$(INTDIR)\snd_dma.sbr" : $(SOURCE) $(DEP_CPP_SND_D) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_SND_D=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_D=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_dma.obj" : $(SOURCE) $(DEP_CPP_SND_D) "$(INTDIR)"

"$(INTDIR)\snd_dma.sbr" : $(SOURCE) $(DEP_CPP_SND_D) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_SND_D=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_D=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_dma.obj" : $(SOURCE) $(DEP_CPP_SND_D) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\snd_mem.c
DEP_CPP_SND_M=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_SND_M=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\snd_mem.obj" : $(SOURCE) $(DEP_CPP_SND_M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\snd_mem.obj" : $(SOURCE) $(DEP_CPP_SND_M) "$(INTDIR)"

"$(INTDIR)\snd_mem.sbr" : $(SOURCE) $(DEP_CPP_SND_M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\snd_mem.obj" : $(SOURCE) $(DEP_CPP_SND_M) "$(INTDIR)"

"$(INTDIR)\snd_mem.sbr" : $(SOURCE) $(DEP_CPP_SND_M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\snd_mem.obj" : $(SOURCE) $(DEP_CPP_SND_M) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\snd_mix.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_SND_MI=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_MI=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_mix.obj" : $(SOURCE) $(DEP_CPP_SND_MI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_SND_MI=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_MI=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_mix.obj" : $(SOURCE) $(DEP_CPP_SND_MI) "$(INTDIR)"

"$(INTDIR)\snd_mix.sbr" : $(SOURCE) $(DEP_CPP_SND_MI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_SND_MI=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_MI=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_mix.obj" : $(SOURCE) $(DEP_CPP_SND_MI) "$(INTDIR)"

"$(INTDIR)\snd_mix.sbr" : $(SOURCE) $(DEP_CPP_SND_MI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_SND_MI=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_MI=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_mix.obj" : $(SOURCE) $(DEP_CPP_SND_MI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\snd_win.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_SND_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_W=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_win.obj" : $(SOURCE) $(DEP_CPP_SND_W) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_SND_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_W=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_win.obj" : $(SOURCE) $(DEP_CPP_SND_W) "$(INTDIR)"

"$(INTDIR)\snd_win.sbr" : $(SOURCE) $(DEP_CPP_SND_W) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_SND_W=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_W=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_win.obj" : $(SOURCE) $(DEP_CPP_SND_W) "$(INTDIR)"

"$(INTDIR)\snd_win.sbr" : $(SOURCE) $(DEP_CPP_SND_W) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_SND_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SND_W=\
	"..\..\scitech\include\mglrect.hpp"\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

"$(INTDIR)\snd_win.obj" : $(SOURCE) $(DEP_CPP_SND_W) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sys_win.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_SYS_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SYS_W=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\sys_win.obj" : $(SOURCE) $(DEP_CPP_SYS_W) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_SYS_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_SYS_W=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\sys_win.obj" : $(SOURCE) $(DEP_CPP_SYS_W) "$(INTDIR)"

"$(INTDIR)\sys_win.sbr" : $(SOURCE) $(DEP_CPP_SYS_W) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

DEP_CPP_SYS_W=\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	

"$(INTDIR)\sys_win.obj" : $(SOURCE) $(DEP_CPP_SYS_W) "$(INTDIR)"

"$(INTDIR)\sys_win.sbr" : $(SOURCE) $(DEP_CPP_SYS_W) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

DEP_CPP_SYS_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\quakedef.h"\
	".\winquake.h"\
	
NODEP_CPP_SYS_W=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\sys_win.obj" : $(SOURCE) $(DEP_CPP_SYS_W) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\vid_win.c

!IF  "$(CFG)" == "qwcl - Win32 Release"

DEP_CPP_VID_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_VID_W=\
	"..\..\scitech\include\mglrect.hpp"\
	

"$(INTDIR)\vid_win.obj" : $(SOURCE) $(DEP_CPP_VID_W) "$(INTDIR)"
   $(CPP) /nologo /ML /GX /O2 /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /c $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

DEP_CPP_VID_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	
NODEP_CPP_VID_W=\
	"..\..\scitech\include\mglrect.hpp"\
	

BuildCmds= \
	$(CPP) /nologo /MLd /Gm /GX /Zi /Od /I "..\..\scitech\include" /I\
 "..\..\dxsdk\sdk\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)/"\
 /Fp"$(INTDIR)/qwcl.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\vid_win.obj" : $(SOURCE) $(DEP_CPP_VID_W) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\vid_win.sbr" : $(SOURCE) $(DEP_CPP_VID_W) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1
DEP_CPP_VID_W=\
	"..\..\scitech\include\mgraph.h"\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\cvar.h"\
	".\d_local.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\mathlib.h"\
	".\modelgen.h"\
	".\net.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\sys.h"\
	".\vid.h"\
	".\wad.h"\
	".\winquake.h"\
	".\zone.h"\
	

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1
DEP_CPP_VID_W=\
	"..\..\scitech\include\debug.h"\
	"..\..\scitech\include\mgldos.h"\
	"..\..\scitech\include\mglwin.h"\
	"..\..\scitech\include\mgraph.h"\
	".\d_local.h"\
	".\quakedef.h"\
	".\r_shared.h"\
	".\winquake.h"\
	
NODEP_CPP_VID_W=\
	"..\..\scitech\include\mglrect.hpp"\
	

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\view.c
DEP_CPP_VIEW_=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\r_local.h"\
	".\r_shared.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_VIEW_=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\view.obj" : $(SOURCE) $(DEP_CPP_VIEW_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\view.obj" : $(SOURCE) $(DEP_CPP_VIEW_) "$(INTDIR)"

"$(INTDIR)\view.sbr" : $(SOURCE) $(DEP_CPP_VIEW_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\view.obj" : $(SOURCE) $(DEP_CPP_VIEW_) "$(INTDIR)"

"$(INTDIR)\view.sbr" : $(SOURCE) $(DEP_CPP_VIEW_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\view.obj" : $(SOURCE) $(DEP_CPP_VIEW_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\wad.c
DEP_CPP_WAD_C=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_WAD_C=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\wad.obj" : $(SOURCE) $(DEP_CPP_WAD_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\wad.obj" : $(SOURCE) $(DEP_CPP_WAD_C) "$(INTDIR)"

"$(INTDIR)\wad.sbr" : $(SOURCE) $(DEP_CPP_WAD_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\wad.obj" : $(SOURCE) $(DEP_CPP_WAD_C) "$(INTDIR)"

"$(INTDIR)\wad.sbr" : $(SOURCE) $(DEP_CPP_WAD_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\wad.obj" : $(SOURCE) $(DEP_CPP_WAD_C) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\zone.c
DEP_CPP_ZONE_=\
	".\bothdefs.h"\
	".\bspfile.h"\
	".\cdaudio.h"\
	".\client.h"\
	".\cmd.h"\
	".\common.h"\
	".\console.h"\
	".\crc.h"\
	".\cvar.h"\
	".\d_iface.h"\
	".\draw.h"\
	".\gl_model.h"\
	".\glquake.h"\
	".\input.h"\
	".\keys.h"\
	".\mathlib.h"\
	".\menu.h"\
	".\model.h"\
	".\modelgen.h"\
	".\net.h"\
	".\pmove.h"\
	".\protocol.h"\
	".\quakedef.h"\
	".\render.h"\
	".\sbar.h"\
	".\screen.h"\
	".\sound.h"\
	".\spritegn.h"\
	".\sys.h"\
	".\vid.h"\
	".\view.h"\
	".\wad.h"\
	".\zone.h"\
	
NODEP_CPP_ZONE_=\
	".\cmdlib.h"\
	".\dictlib.h"\
	".\lbmlib.h"\
	".\scriplib.h"\
	".\trilib.h"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\zone.obj" : $(SOURCE) $(DEP_CPP_ZONE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\zone.obj" : $(SOURCE) $(DEP_CPP_ZONE_) "$(INTDIR)"

"$(INTDIR)\zone.sbr" : $(SOURCE) $(DEP_CPP_ZONE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\zone.obj" : $(SOURCE) $(DEP_CPP_ZONE_) "$(INTDIR)"

"$(INTDIR)\zone.sbr" : $(SOURCE) $(DEP_CPP_ZONE_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\zone.obj" : $(SOURCE) $(DEP_CPP_ZONE_) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_draw.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\d_draw.s
InputName=d_draw

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\d_draw.s
InputName=d_draw

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_draw16.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\d_draw16.s
InputName=d_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\d_draw16.s
InputName=d_draw16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_parta.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\d_parta.s
InputName=d_parta

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\d_parta.s
InputName=d_parta

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_polysa.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\d_polysa.s
InputName=d_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\d_polysa.s
InputName=d_polysa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_scana.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\d_scana.s
InputName=d_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\d_scana.s
InputName=d_scana

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_spr8.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\d_spr8.s
InputName=d_spr8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\d_spr8.s
InputName=d_spr8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\d_varsa.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\d_varsa.s
InputName=d_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\d_varsa.s
InputName=d_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\math.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\GLDebug
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# Begin Custom Build - GAS Assembly
OutDir=.\GLRelease
InputPath=.\math.s
InputName=math

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_aclipa.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\r_aclipa.s
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\r_aclipa.s
InputName=r_aclipa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_aliasa.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\r_aliasa.s
InputName=r_aliasa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\r_aliasa.s
InputName=r_aliasa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_drawa.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\r_drawa.s
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\r_drawa.s
InputName=r_drawa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_edgea.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\r_edgea.s
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\r_edgea.s
InputName=r_edgea

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\r_varsa.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\r_varsa.s
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\r_varsa.s
InputName=r_varsa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\snd_mixa.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\GLDebug
InputPath=.\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# Begin Custom Build - GAS Assembly
OutDir=.\GLRelease
InputPath=.\snd_mixa.s
InputName=snd_mixa

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\surf16.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\surf16.s
InputName=surf16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\surf16.s
InputName=surf16

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\surf8.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\surf8.s
InputName=surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\surf8.s
InputName=surf8

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sys_wina.s

!IF  "$(CFG)" == "qwcl - Win32 Release"

# Begin Custom Build - GAS Assembly
OutDir=.\release
InputPath=.\sys_wina.s
InputName=sys_wina

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\debug
InputPath=.\sys_wina.s
InputName=sys_wina

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"

# Begin Custom Build - GAS Assembly
OutDir=.\GLDebug
InputPath=.\sys_wina.s
InputName=sys_wina

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"

# Begin Custom Build - GAS Assembly
OutDir=.\GLRelease
InputPath=.\sys_wina.s
InputName=sys_wina

"$(OUTDIR)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   cl /EP /DGLQUAKE > $(OUTDIR)\$(InputName).spp $(InputPath)
   gas2masm\debug\gas2masm < $(OUTDIR)\$(InputName).spp\
   >$(OUTDIR)\$(InputName).asm
   ml /c /Cp /coff /Fo$(OUTDIR)\$(InputName).obj /Zm /Zi\
   $(OUTDIR)\$(InputName).asm
   del $(OUTDIR)\$(InputName).spp

# End Custom Build

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\winquake.rc
DEP_RSC_WINQU=\
	".\quakeworld.bmp"\
	".\qwcl2.ico"\
	

!IF  "$(CFG)" == "qwcl - Win32 Release"


"$(INTDIR)\winquake.res" : $(SOURCE) $(DEP_RSC_WINQU) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 Debug"


"$(INTDIR)\winquake.res" : $(SOURCE) $(DEP_RSC_WINQU) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Debug"


"$(INTDIR)\winquake.res" : $(SOURCE) $(DEP_RSC_WINQU) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwcl - Win32 GL Release"


"$(INTDIR)\winquake.res" : $(SOURCE) $(DEP_RSC_WINQU) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
