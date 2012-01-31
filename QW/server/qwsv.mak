# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=qwsv - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to qwsv - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "qwsv - Win32 Release" && "$(CFG)" != "qwsv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "qwsv.mak" CFG="qwsv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qwsv - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "qwsv - Win32 Debug" (based on "Win32 (x86) Console Application")
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
# PROP Target_Last_Scanned "qwsv - Win32 Debug"
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "qwsv - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\qwsv.exe"

CLEAN : 
	-@erase "$(INTDIR)\cmd.obj"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\cvar.obj"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\net_chan.obj"
	-@erase "$(INTDIR)\net_wins.obj"
	-@erase "$(INTDIR)\pmove.obj"
	-@erase "$(INTDIR)\pmovetst.obj"
	-@erase "$(INTDIR)\pr_cmds.obj"
	-@erase "$(INTDIR)\pr_edict.obj"
	-@erase "$(INTDIR)\pr_exec.obj"
	-@erase "$(INTDIR)\sv_ccmds.obj"
	-@erase "$(INTDIR)\sv_ents.obj"
	-@erase "$(INTDIR)\sv_init.obj"
	-@erase "$(INTDIR)\sv_main.obj"
	-@erase "$(INTDIR)\sv_move.obj"
	-@erase "$(INTDIR)\sv_phys.obj"
	-@erase "$(INTDIR)\sv_send.obj"
	-@erase "$(INTDIR)\sv_user.obj"
	-@erase "$(INTDIR)\sys_win.obj"
	-@erase "$(INTDIR)\world.obj"
	-@erase "$(INTDIR)\zone.obj"
	-@erase "$(OUTDIR)\qwsv.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /GX /O2 /I "." /I "../client" /D "NDEBUG" /D "SERVERONLY" /D "WIN32" /D "_CONSOLE" /YX /c
# SUBTRACT CPP /Fr
CPP_PROJ=/nologo /ML /GX /O2 /I "." /I "../client" /D "NDEBUG" /D "SERVERONLY"\
 /D "WIN32" /D "_CONSOLE" /Fp"$(INTDIR)/qwsv.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qwsv.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /machine:I386
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /machine:I386\
 /out:"$(OUTDIR)/qwsv.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cmd.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cvar.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\model.obj" \
	"$(INTDIR)\net_chan.obj" \
	"$(INTDIR)\net_wins.obj" \
	"$(INTDIR)\pmove.obj" \
	"$(INTDIR)\pmovetst.obj" \
	"$(INTDIR)\pr_cmds.obj" \
	"$(INTDIR)\pr_edict.obj" \
	"$(INTDIR)\pr_exec.obj" \
	"$(INTDIR)\sv_ccmds.obj" \
	"$(INTDIR)\sv_ents.obj" \
	"$(INTDIR)\sv_init.obj" \
	"$(INTDIR)\sv_main.obj" \
	"$(INTDIR)\sv_move.obj" \
	"$(INTDIR)\sv_phys.obj" \
	"$(INTDIR)\sv_send.obj" \
	"$(INTDIR)\sv_user.obj" \
	"$(INTDIR)\sys_win.obj" \
	"$(INTDIR)\world.obj" \
	"$(INTDIR)\zone.obj"

"$(OUTDIR)\qwsv.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\qwsv.exe" "$(OUTDIR)\qwsv.bsc"

CLEAN : 
	-@erase "$(INTDIR)\cmd.obj"
	-@erase "$(INTDIR)\cmd.sbr"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\common.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\cvar.obj"
	-@erase "$(INTDIR)\cvar.sbr"
	-@erase "$(INTDIR)\mathlib.obj"
	-@erase "$(INTDIR)\mathlib.sbr"
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
	-@erase "$(INTDIR)\pr_cmds.obj"
	-@erase "$(INTDIR)\pr_cmds.sbr"
	-@erase "$(INTDIR)\pr_edict.obj"
	-@erase "$(INTDIR)\pr_edict.sbr"
	-@erase "$(INTDIR)\pr_exec.obj"
	-@erase "$(INTDIR)\pr_exec.sbr"
	-@erase "$(INTDIR)\sv_ccmds.obj"
	-@erase "$(INTDIR)\sv_ccmds.sbr"
	-@erase "$(INTDIR)\sv_ents.obj"
	-@erase "$(INTDIR)\sv_ents.sbr"
	-@erase "$(INTDIR)\sv_init.obj"
	-@erase "$(INTDIR)\sv_init.sbr"
	-@erase "$(INTDIR)\sv_main.obj"
	-@erase "$(INTDIR)\sv_main.sbr"
	-@erase "$(INTDIR)\sv_move.obj"
	-@erase "$(INTDIR)\sv_move.sbr"
	-@erase "$(INTDIR)\sv_phys.obj"
	-@erase "$(INTDIR)\sv_phys.sbr"
	-@erase "$(INTDIR)\sv_send.obj"
	-@erase "$(INTDIR)\sv_send.sbr"
	-@erase "$(INTDIR)\sv_user.obj"
	-@erase "$(INTDIR)\sv_user.sbr"
	-@erase "$(INTDIR)\sys_win.obj"
	-@erase "$(INTDIR)\sys_win.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(INTDIR)\world.obj"
	-@erase "$(INTDIR)\world.sbr"
	-@erase "$(INTDIR)\zone.obj"
	-@erase "$(INTDIR)\zone.sbr"
	-@erase "$(OUTDIR)\qwsv.bsc"
	-@erase "$(OUTDIR)\qwsv.exe"
	-@erase "$(OUTDIR)\qwsv.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /Gm /GX /Zi /Od /I "." /I "../client" /D "_DEBUG" /D "SERVERONLY" /D "WIN32" /D "_CONSOLE" /FR /YX /c
CPP_PROJ=/nologo /MLd /Gm /GX /Zi /Od /I "." /I "../client" /D "_DEBUG" /D\
 "SERVERONLY" /D "WIN32" /D "_CONSOLE" /FR"$(INTDIR)/" /Fp"$(INTDIR)/qwsv.pch"\
 /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/qwsv.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cmd.sbr" \
	"$(INTDIR)\common.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\cvar.sbr" \
	"$(INTDIR)\mathlib.sbr" \
	"$(INTDIR)\model.sbr" \
	"$(INTDIR)\net_chan.sbr" \
	"$(INTDIR)\net_wins.sbr" \
	"$(INTDIR)\pmove.sbr" \
	"$(INTDIR)\pmovetst.sbr" \
	"$(INTDIR)\pr_cmds.sbr" \
	"$(INTDIR)\pr_edict.sbr" \
	"$(INTDIR)\pr_exec.sbr" \
	"$(INTDIR)\sv_ccmds.sbr" \
	"$(INTDIR)\sv_ents.sbr" \
	"$(INTDIR)\sv_init.sbr" \
	"$(INTDIR)\sv_main.sbr" \
	"$(INTDIR)\sv_move.sbr" \
	"$(INTDIR)\sv_phys.sbr" \
	"$(INTDIR)\sv_send.sbr" \
	"$(INTDIR)\sv_user.sbr" \
	"$(INTDIR)\sys_win.sbr" \
	"$(INTDIR)\world.sbr" \
	"$(INTDIR)\zone.sbr"

"$(OUTDIR)\qwsv.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /map /debug /machine:I386
LINK32_FLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /profile\
 /map:"$(INTDIR)/qwsv.map" /debug /machine:I386 /out:"$(OUTDIR)/qwsv.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cmd.obj" \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cvar.obj" \
	"$(INTDIR)\mathlib.obj" \
	"$(INTDIR)\model.obj" \
	"$(INTDIR)\net_chan.obj" \
	"$(INTDIR)\net_wins.obj" \
	"$(INTDIR)\pmove.obj" \
	"$(INTDIR)\pmovetst.obj" \
	"$(INTDIR)\pr_cmds.obj" \
	"$(INTDIR)\pr_edict.obj" \
	"$(INTDIR)\pr_exec.obj" \
	"$(INTDIR)\sv_ccmds.obj" \
	"$(INTDIR)\sv_ents.obj" \
	"$(INTDIR)\sv_init.obj" \
	"$(INTDIR)\sv_main.obj" \
	"$(INTDIR)\sv_move.obj" \
	"$(INTDIR)\sv_phys.obj" \
	"$(INTDIR)\sv_send.obj" \
	"$(INTDIR)\sv_user.obj" \
	"$(INTDIR)\sys_win.obj" \
	"$(INTDIR)\world.obj" \
	"$(INTDIR)\zone.obj"

"$(OUTDIR)\qwsv.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "qwsv - Win32 Release"
# Name "qwsv - Win32 Debug"

!IF  "$(CFG)" == "qwsv - Win32 Release"

!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\model.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_MODEL=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_MODEL=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\model.obj" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_MODEL=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_MODEL=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\model.obj" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"

"$(INTDIR)\model.sbr" : $(SOURCE) $(DEP_CPP_MODEL) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pr_cmds.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_PR_CM=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_PR_CM=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\pr_cmds.obj" : $(SOURCE) $(DEP_CPP_PR_CM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_PR_CM=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_PR_CM=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\pr_cmds.obj" : $(SOURCE) $(DEP_CPP_PR_CM) "$(INTDIR)"

"$(INTDIR)\pr_cmds.sbr" : $(SOURCE) $(DEP_CPP_PR_CM) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pr_edict.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_PR_ED=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_PR_ED=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\pr_edict.obj" : $(SOURCE) $(DEP_CPP_PR_ED) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_PR_ED=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_PR_ED=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\pr_edict.obj" : $(SOURCE) $(DEP_CPP_PR_ED) "$(INTDIR)"

"$(INTDIR)\pr_edict.sbr" : $(SOURCE) $(DEP_CPP_PR_ED) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\pr_exec.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_PR_EX=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_PR_EX=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\pr_exec.obj" : $(SOURCE) $(DEP_CPP_PR_EX) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_PR_EX=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_PR_EX=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\pr_exec.obj" : $(SOURCE) $(DEP_CPP_PR_EX) "$(INTDIR)"

"$(INTDIR)\pr_exec.sbr" : $(SOURCE) $(DEP_CPP_PR_EX) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sv_ccmds.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_CC=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_CC=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_ccmds.obj" : $(SOURCE) $(DEP_CPP_SV_CC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_CC=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_CC=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_ccmds.obj" : $(SOURCE) $(DEP_CPP_SV_CC) "$(INTDIR)"

"$(INTDIR)\sv_ccmds.sbr" : $(SOURCE) $(DEP_CPP_SV_CC) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sv_init.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_IN=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_IN=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_init.obj" : $(SOURCE) $(DEP_CPP_SV_IN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_IN=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_IN=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_init.obj" : $(SOURCE) $(DEP_CPP_SV_IN) "$(INTDIR)"

"$(INTDIR)\sv_init.sbr" : $(SOURCE) $(DEP_CPP_SV_IN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sv_main.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_MA=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_MA=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_main.obj" : $(SOURCE) $(DEP_CPP_SV_MA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_MA=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_MA=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_main.obj" : $(SOURCE) $(DEP_CPP_SV_MA) "$(INTDIR)"

"$(INTDIR)\sv_main.sbr" : $(SOURCE) $(DEP_CPP_SV_MA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sv_move.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_MO=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_MO=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_move.obj" : $(SOURCE) $(DEP_CPP_SV_MO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_MO=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_MO=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_move.obj" : $(SOURCE) $(DEP_CPP_SV_MO) "$(INTDIR)"

"$(INTDIR)\sv_move.sbr" : $(SOURCE) $(DEP_CPP_SV_MO) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sv_phys.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_PH=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_PH=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_phys.obj" : $(SOURCE) $(DEP_CPP_SV_PH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_PH=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_PH=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_phys.obj" : $(SOURCE) $(DEP_CPP_SV_PH) "$(INTDIR)"

"$(INTDIR)\sv_phys.sbr" : $(SOURCE) $(DEP_CPP_SV_PH) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sv_send.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_SE=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_SE=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_send.obj" : $(SOURCE) $(DEP_CPP_SV_SE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_SE=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_SE=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_send.obj" : $(SOURCE) $(DEP_CPP_SV_SE) "$(INTDIR)"

"$(INTDIR)\sv_send.sbr" : $(SOURCE) $(DEP_CPP_SV_SE) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sv_user.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_US=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_US=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_user.obj" : $(SOURCE) $(DEP_CPP_SV_US) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_US=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_US=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_user.obj" : $(SOURCE) $(DEP_CPP_SV_US) "$(INTDIR)"

"$(INTDIR)\sv_user.sbr" : $(SOURCE) $(DEP_CPP_SV_US) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\world.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_WORLD=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_WORLD=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\world.obj" : $(SOURCE) $(DEP_CPP_WORLD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_WORLD=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_WORLD=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\world.obj" : $(SOURCE) $(DEP_CPP_WORLD) "$(INTDIR)"

"$(INTDIR)\world.sbr" : $(SOURCE) $(DEP_CPP_WORLD) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sys_win.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SYS_W=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	{$(INCLUDE)}"\sys\TIMEB.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_SYS_W=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sys_win.obj" : $(SOURCE) $(DEP_CPP_SYS_W) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SYS_W=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	{$(INCLUDE)}"\sys\TIMEB.H"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	
NODEP_CPP_SYS_W=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sys_win.obj" : $(SOURCE) $(DEP_CPP_SYS_W) "$(INTDIR)"

"$(INTDIR)\sys_win.sbr" : $(SOURCE) $(DEP_CPP_SYS_W) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\server.h

!IF  "$(CFG)" == "qwsv - Win32 Release"

!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\quake\v2\client\net.h

!IF  "$(CFG)" == "qwsv - Win32 Release"

!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\sv_ents.c

!IF  "$(CFG)" == "qwsv - Win32 Release"

DEP_CPP_SV_EN=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_EN=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_ents.obj" : $(SOURCE) $(DEP_CPP_SV_EN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"

DEP_CPP_SV_EN=\
	"..\client\spritegn.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\pr_comp.h"\
	".\progdefs.h"\
	".\progs.h"\
	".\qwsvdef.h"\
	".\server.h"\
	".\sys.h"\
	".\world.h"\
	
NODEP_CPP_SV_EN=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

"$(INTDIR)\sv_ents.obj" : $(SOURCE) $(DEP_CPP_SV_EN) "$(INTDIR)"

"$(INTDIR)\sv_ents.sbr" : $(SOURCE) $(DEP_CPP_SV_EN) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\QW\client\mathlib.c
DEP_CPP_MATHL=\
	"..\client\cdaudio.h"\
	"..\client\client.h"\
	"..\client\console.h"\
	"..\client\d_iface.h"\
	"..\client\draw.h"\
	"..\client\gl_model.h"\
	"..\client\glquake.h"\
	"..\client\input.h"\
	"..\client\keys.h"\
	"..\client\menu.h"\
	"..\client\render.h"\
	"..\client\sbar.h"\
	"..\client\screen.h"\
	"..\client\sound.h"\
	"..\client\spritegn.h"\
	"..\client\sys.h"\
	"..\client\vid.h"\
	"..\client\view.h"\
	"..\client\wad.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\..\client\quakedef.h"\
	
NODEP_CPP_MATHL=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

!IF  "$(CFG)" == "qwsv - Win32 Release"


"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\mathlib.obj" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\mathlib.sbr" : $(SOURCE) $(DEP_CPP_MATHL) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\QW\client\net_chan.c
DEP_CPP_NET_C=\
	"..\client\cdaudio.h"\
	"..\client\client.h"\
	"..\client\console.h"\
	"..\client\d_iface.h"\
	"..\client\draw.h"\
	"..\client\gl_model.h"\
	"..\client\glquake.h"\
	"..\client\input.h"\
	"..\client\keys.h"\
	"..\client\menu.h"\
	"..\client\render.h"\
	"..\client\sbar.h"\
	"..\client\screen.h"\
	"..\client\sound.h"\
	"..\client\spritegn.h"\
	"..\client\sys.h"\
	"..\client\vid.h"\
	"..\client\view.h"\
	"..\client\wad.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\..\client\quakedef.h"\
	
NODEP_CPP_NET_C=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

!IF  "$(CFG)" == "qwsv - Win32 Release"


"$(INTDIR)\net_chan.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\net_chan.obj" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\net_chan.sbr" : $(SOURCE) $(DEP_CPP_NET_C) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\QW\client\common.c
DEP_CPP_COMMO=\
	"..\client\cdaudio.h"\
	"..\client\client.h"\
	"..\client\console.h"\
	"..\client\d_iface.h"\
	"..\client\draw.h"\
	"..\client\gl_model.h"\
	"..\client\glquake.h"\
	"..\client\input.h"\
	"..\client\keys.h"\
	"..\client\menu.h"\
	"..\client\render.h"\
	"..\client\sbar.h"\
	"..\client\screen.h"\
	"..\client\sound.h"\
	"..\client\spritegn.h"\
	"..\client\sys.h"\
	"..\client\vid.h"\
	"..\client\view.h"\
	"..\client\wad.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\..\client\quakedef.h"\
	
NODEP_CPP_COMMO=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\qwsvdef.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

!IF  "$(CFG)" == "qwsv - Win32 Release"


"$(INTDIR)\common.obj" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\common.obj" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\common.sbr" : $(SOURCE) $(DEP_CPP_COMMO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\QW\client\pmovetst.c
DEP_CPP_PMOVE=\
	"..\client\cdaudio.h"\
	"..\client\client.h"\
	"..\client\console.h"\
	"..\client\d_iface.h"\
	"..\client\draw.h"\
	"..\client\gl_model.h"\
	"..\client\glquake.h"\
	"..\client\input.h"\
	"..\client\keys.h"\
	"..\client\menu.h"\
	"..\client\render.h"\
	"..\client\sbar.h"\
	"..\client\screen.h"\
	"..\client\sound.h"\
	"..\client\spritegn.h"\
	"..\client\sys.h"\
	"..\client\vid.h"\
	"..\client\view.h"\
	"..\client\wad.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\..\client\quakedef.h"\
	
NODEP_CPP_PMOVE=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

!IF  "$(CFG)" == "qwsv - Win32 Release"


"$(INTDIR)\pmovetst.obj" : $(SOURCE) $(DEP_CPP_PMOVE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\pmovetst.obj" : $(SOURCE) $(DEP_CPP_PMOVE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\pmovetst.sbr" : $(SOURCE) $(DEP_CPP_PMOVE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\QW\client\pmove.c
DEP_CPP_PMOVE_=\
	"..\client\cdaudio.h"\
	"..\client\client.h"\
	"..\client\console.h"\
	"..\client\d_iface.h"\
	"..\client\draw.h"\
	"..\client\gl_model.h"\
	"..\client\glquake.h"\
	"..\client\input.h"\
	"..\client\keys.h"\
	"..\client\menu.h"\
	"..\client\render.h"\
	"..\client\sbar.h"\
	"..\client\screen.h"\
	"..\client\sound.h"\
	"..\client\spritegn.h"\
	"..\client\sys.h"\
	"..\client\vid.h"\
	"..\client\view.h"\
	"..\client\wad.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\..\client\quakedef.h"\
	
NODEP_CPP_PMOVE_=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

!IF  "$(CFG)" == "qwsv - Win32 Release"


"$(INTDIR)\pmove.obj" : $(SOURCE) $(DEP_CPP_PMOVE_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\pmove.obj" : $(SOURCE) $(DEP_CPP_PMOVE_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\pmove.sbr" : $(SOURCE) $(DEP_CPP_PMOVE_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\QW\client\cvar.c
DEP_CPP_CVAR_=\
	"..\client\cdaudio.h"\
	"..\client\client.h"\
	"..\client\console.h"\
	"..\client\d_iface.h"\
	"..\client\draw.h"\
	"..\client\gl_model.h"\
	"..\client\glquake.h"\
	"..\client\input.h"\
	"..\client\keys.h"\
	"..\client\menu.h"\
	"..\client\render.h"\
	"..\client\sbar.h"\
	"..\client\screen.h"\
	"..\client\sound.h"\
	"..\client\spritegn.h"\
	"..\client\sys.h"\
	"..\client\vid.h"\
	"..\client\view.h"\
	"..\client\wad.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\..\client\quakedef.h"\
	
NODEP_CPP_CVAR_=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\qwsvdef.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

!IF  "$(CFG)" == "qwsv - Win32 Release"


"$(INTDIR)\cvar.obj" : $(SOURCE) $(DEP_CPP_CVAR_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\cvar.obj" : $(SOURCE) $(DEP_CPP_CVAR_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\cvar.sbr" : $(SOURCE) $(DEP_CPP_CVAR_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\QW\client\crc.c
DEP_CPP_CRC_C=\
	"..\client\cdaudio.h"\
	"..\client\client.h"\
	"..\client\console.h"\
	"..\client\d_iface.h"\
	"..\client\draw.h"\
	"..\client\gl_model.h"\
	"..\client\glquake.h"\
	"..\client\input.h"\
	"..\client\keys.h"\
	"..\client\menu.h"\
	"..\client\render.h"\
	"..\client\sbar.h"\
	"..\client\screen.h"\
	"..\client\sound.h"\
	"..\client\spritegn.h"\
	"..\client\sys.h"\
	"..\client\vid.h"\
	"..\client\view.h"\
	"..\client\wad.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\..\client\quakedef.h"\
	
NODEP_CPP_CRC_C=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

!IF  "$(CFG)" == "qwsv - Win32 Release"


"$(INTDIR)\crc.obj" : $(SOURCE) $(DEP_CPP_CRC_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\crc.obj" : $(SOURCE) $(DEP_CPP_CRC_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\crc.sbr" : $(SOURCE) $(DEP_CPP_CRC_C) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\QW\client\net_wins.c
DEP_CPP_NET_W=\
	"..\client\cdaudio.h"\
	"..\client\client.h"\
	"..\client\console.h"\
	"..\client\d_iface.h"\
	"..\client\draw.h"\
	"..\client\gl_model.h"\
	"..\client\glquake.h"\
	"..\client\input.h"\
	"..\client\keys.h"\
	"..\client\menu.h"\
	"..\client\render.h"\
	"..\client\sbar.h"\
	"..\client\screen.h"\
	"..\client\sound.h"\
	"..\client\spritegn.h"\
	"..\client\sys.h"\
	"..\client\vid.h"\
	"..\client\view.h"\
	"..\client\wad.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\..\client\quakedef.h"\
	".\..\client\winquake.h"\
	
NODEP_CPP_NET_W=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

!IF  "$(CFG)" == "qwsv - Win32 Release"


"$(INTDIR)\net_wins.obj" : $(SOURCE) $(DEP_CPP_NET_W) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\net_wins.obj" : $(SOURCE) $(DEP_CPP_NET_W) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\net_wins.sbr" : $(SOURCE) $(DEP_CPP_NET_W) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\QW\client\zone.c
DEP_CPP_ZONE_=\
	"..\client\cdaudio.h"\
	"..\client\client.h"\
	"..\client\console.h"\
	"..\client\d_iface.h"\
	"..\client\draw.h"\
	"..\client\gl_model.h"\
	"..\client\glquake.h"\
	"..\client\input.h"\
	"..\client\keys.h"\
	"..\client\menu.h"\
	"..\client\render.h"\
	"..\client\sbar.h"\
	"..\client\screen.h"\
	"..\client\sound.h"\
	"..\client\spritegn.h"\
	"..\client\sys.h"\
	"..\client\vid.h"\
	"..\client\view.h"\
	"..\client\wad.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\..\client\quakedef.h"\
	
NODEP_CPP_ZONE_=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

!IF  "$(CFG)" == "qwsv - Win32 Release"


"$(INTDIR)\zone.obj" : $(SOURCE) $(DEP_CPP_ZONE_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\zone.obj" : $(SOURCE) $(DEP_CPP_ZONE_) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\zone.sbr" : $(SOURCE) $(DEP_CPP_ZONE_) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\Projects\QW\client\cmd.c
DEP_CPP_CMD_C=\
	"..\client\cdaudio.h"\
	"..\client\client.h"\
	"..\client\console.h"\
	"..\client\d_iface.h"\
	"..\client\draw.h"\
	"..\client\gl_model.h"\
	"..\client\glquake.h"\
	"..\client\input.h"\
	"..\client\keys.h"\
	"..\client\menu.h"\
	"..\client\render.h"\
	"..\client\sbar.h"\
	"..\client\screen.h"\
	"..\client\sound.h"\
	"..\client\spritegn.h"\
	"..\client\sys.h"\
	"..\client\vid.h"\
	"..\client\view.h"\
	"..\client\wad.h"\
	".\../client\bothdefs.h"\
	".\../client\bspfile.h"\
	".\../client\cmd.h"\
	".\../client\common.h"\
	".\../client\crc.h"\
	".\../client\cvar.h"\
	".\../client\mathlib.h"\
	".\../client\model.h"\
	".\../client\net.h"\
	".\../client\pmove.h"\
	".\../client\protocol.h"\
	".\../client\zone.h"\
	".\..\client\modelgen.h"\
	".\..\client\quakedef.h"\
	
NODEP_CPP_CMD_C=\
	"..\client\cmdlib.h"\
	"..\client\dictlib.h"\
	"..\client\lbmlib.h"\
	"..\client\scriplib.h"\
	"..\client\trilib.h"\
	

!IF  "$(CFG)" == "qwsv - Win32 Release"


"$(INTDIR)\cmd.obj" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "qwsv - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\cmd.obj" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\cmd.sbr" : $(SOURCE) $(DEP_CPP_CMD_C) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
