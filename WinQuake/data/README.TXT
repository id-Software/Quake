Welcome to Quake!

This file details how to get Quake running on your system and what to do
if you have problems.  We would like to thank Gandalf Technologies, Inc and
MPath Interactive for the use of their technology.  We would also like to
thank Trent Reznor and Nine Inch Nails for their tremendous contributions
to Quake's entire audio portion.

The NIN logo is a Registered Trademark licensed to Nothing Interactive, Inc.
All Rights Reserved.

Quake System Requirements
-------------------------
IBM PC and Compatibles
Pentium processor or better
VGA Compatible Display or better
8MB RAM minimum, 16MB recommended (16 MB required for running under Win95)
CD-ROM drive Required 
MS-DOS 5.0 or better or Windows 95 (does NOT run under Windows NT)
Hard Drive (30MB for Shareware, 80 MB for Registered)

*** IMPORTANT!: Quake requires a floating point processor.
Systems that do not have an FPU installed will not run Quake -- at all.

*** IMPORTANT Video Adapter Note! ***
On some ATI Mach32 cards, Quake can come up with a garbled video display.
This is due to a problem with the card in which 320x200 mode isn't
initialized correctly.  Workarounds include:

1) If running from Windows, start Quake from an icon, or from a windowed
(not fullscreen) MS-DOS prompt.  If Quake is already running and has
the garbled screen, press Alt-Enter twice to switch to the desktop and
back to fullscreen, and the screen will display properly.

2) If running from DOS, either put the line

vid_mode 1

in id1\autoexec.cfg, or, typing blind, press tilde ('~') to bring down
the console, type

vid_mode 1<enter>

and the screen will display properly.

========================================================================
Here are the text files included with the shareware release of Quake and
what they are:

README.TXT              This file
TECHINFO.TXT            Technical information on Quake's subsystems and
                        their advanced use.
MANUAL.TXT              Text version of the printed game manual
LICINFO.TXT             Info on the various license files included with Quake
SLICNSE.TXT             Shareware Quake end-user license
ORDER.TXT               How to order Quake
HELP.TXT                How to get help with Quake

Here are the text files included with the registered version of Quake and
what they are:

README.TXT              This file
TECHINFO.TXT            Technical information on Quake's subsystems and
                        their advanced use.
MANUAL.TXT              Text version of the printed game manual
LICINFO.TXT             Info on the various license files included with Quake
RLICNSE.TXT             Registered Quake end-user license
COMEXP.TXT              Commercial exploitation agreement
ORDER.TXT               How to order Quake
HELP.TXT                How to get help with Quake


Running Quake
-------------

DOS:  To launch Quake from the DOS Prompt, go to the Quake directory and
simply type "QUAKE" <ENTER>. (no quotes)

Windows 95:  To launch Quake in single player mode, double click on the file
QUAKE.EXE From Windows Explorer.  To run Quake in Multi-Player mode using
the TCP/IP protocol, first check your network settings to ensure the
protocol is installed, then double click on the Q95.BAT file to launch the
game. In this version (v0.91) there is a minor bug that will cause the
Q95.BAT file to exit the first time you run it, without running Quake.
Merely double-click on that file again and it will work.

Audio Setup
-----------

When using a Sound Card with Quake, there are a few setup steps which must
be taken. First, the "BLASTER" environment variable setting must be in your
autoexec.bat (or you can type it in manually from the MS-DOS command prompt).
Running the Sound Blaster utility diagnose.exe will automatically configure
your sound card and put this statement in your autoexec.bat file for you.
A typical blaster setting looks like this (although yours may vary):

SET BLASTER=A220 I5 D1 H5 P330 T6

If you want to play the audio track from the CD-ROM while playing Quake,
you must ensure that the audio cable from the CD-ROM is connected to the
sound card.

If you think your sound card is setup properly and it STILL doesn't work,
check to make sure that your BLASTER environment variable contains the
high DMA setting (H5 in the above example).

If you don't get sound while trying to play the audio track, check to see
if a small cable goes from the back of your CD-ROM player directly to your
sound card.  If the CD-ROM audio cable is connected to your sound board (or
the motherboard in some cases) and you STILL don't hear CD Audio coming from
your speakers, make sure the MIXER program has the CD volume turned up.
You will also need to run the CD-ROM driver MSCDEX.EXE. Here is an example
of the files you should see (yours probably will vary) listed in your
CONFIG.SYS and AUTOEXEC.BAT (explanation is in parentheses):

CONFIG.SYS:

DEVICE=C:\PROSCSI\CDROM.SYS /D:PROCD01 (CD-ROM driver)

AUTOEXEC.BAT:

SET BLASTER=A220 I5 D1 H5 P330 T6 (sound environment variable setting)
C:\WINDOWS\COMMAND\MSCDEX.EXE  /D:PROCD01 /L:D (CD-ROM driver)

===================================================
UltraSound MAX and UltraSound PnP Support for Quake
===================================================

Before running Quake, make sure that your sound card works and your
environment variables are set correctly.

Other UltraSound Cards (ACE & Classic)
--------------------------------------
These drivers are not for the UltraSound ACE or UltraSound Classic
sound cards. We have heard mixed reports that MegaEm or SBOS
have a chance of working with the UltraSound Classic but there is a
short sound F/X delay.

UltraSound PnP and PnP Pro
--------------------------
You must make sure that you do NOT have IWSBOS or MegaEm loaded. 

Setup
-----
Quake will automatically detect that the UltraSound Max or PnP
are installed. It does this by looking at the SET INTERWAVE (PnP)
and SET ULTRA16 (Max) environment variables.

Quake will use the settings found on the SET ULTRASND/ULTRA16 (Max)
and in the IW.INI (PnP) file to determine what port settings to use.

Troubleshooting Windows 95 (DOS Box)
------------------------------------
We recommend that you restart your computer in MS-DOS Mode.  DOS Box 
may or may not work, so use at your own risk.  

CD Audio Input
--------------
If you have not already enabled CD audio output by default you will
need to enable it. For the UltraSound MAX you can run "ULTRINIT -EC".
For the UltraSound PnP you will need to enable the CD audio output
in Win'95 and then restart your computer into MS-DOS.

===================================================
Mouse Setup
-----------

If you are going to use a mouse when playing Quake, you will need to load
your mouse driver.  This should go in the AUTOEXEC.BAT file as well.  Here
is an example:

C:\LOGITECH\MOUSE\MOUSE.EXE (mouse driver)


Booting Clean
-------------

If you are going to be running Quake with only 8 megabytes of RAM, it is best
to boot clean .  You eliminate unwanted utilities or applications from taking
up valuable memory, without having to alter your regular AUTOEXEC.BAT and
CONFIG.SYS. Booting clean can be done in one of two ways. If you have
MS-DOS version 6.xx, booting clean is as simple a pressing the shift key
when you see the words "Starting MS-DOS". If you have MS-DOS ver 5.xx you
will need to make a system disk.

To make a boot disk, type the following from the MS-DOS command prompt:

FORMAT A: /S

1. Make sure that this is a disk you wish to erase.
2. This disk absolutely HAS to be formatted in the A: drive.

To use the system disk, place the disk in the A: drive and reset the
computer.

NOTE: If your sound card requires a driver to be loaded, or you will be
using a mouse, or you will be using Quake's CD audio feature, the system
disk will need to have a CONFIG.SYS and AUTOEXEC.BAT that load the
appropriate drivers.

Creating a Quake Shortcut

As an alternative to making a Boot Disk, Windows 95 users can create a
Quake Shortcut.  By double clicking onthis shortcut, Windows 95 will reboot
in MS-DOS mode and install only the desired drivers, giving you the same
results as using a Boot Disk.  To create a Quake Shortcut, do the following:

1.  Using Explorer, right click and drag the file QUAKE.EXE, from the Quake
    directory, to your desktop.  Windows 95 will make an MS-DOS Icon titled
    "Shortcut to quake".
2.  Right click on the new icon, and from the menu that pops up, choose
    "Properties".  Then choose the "Program" tab at the top.
3.  Now click on the "Advanced..." button near the bottom.  The "Advanced
    Program Settings" window should appear.
4.  Select the "MS-DOS mode" check box and the "Specify a new MS-DOS
    configuration" option button.
5.  Now simply fill in the "CONFIG.SYS for MS-DOS mode:" and "AUTOEXEC.BAT
    for MS-DOS mode:" boxes with the same sound, CD-ROM and mouse settings as
    mentioned above in the Boot Disks section.
6.  Click on "OK" when you are finished.  If you wish, you can change your
    Quake Shortcut Icon to something a little more exciting by clicking on
    "Change Icon...".
7.  To finish, click on "OK" again.
    8.  You can rename your Quake Shortcut by right clicking on the shortcut
    icon, choosing "Rename" and typing in the new name.


======================================================
==                  Known Problems                  ==
======================================================

Problem: Zombies sometime get stuck on the ground and connot get back up.
(You can still hear them, but you cannot kill them. This bug makes it
impossible to get 100% kills on whatever level it occurs on.)
Solution: There is no workaround for this bug.

Problem: It is sometimes possible for the player to get stuck in a room or
in a wall.
Solution: If you get stuck, use the 'kill' console command. It is a good
idea to save your game often.

Problem: View centering problems. Sometimes during a game, the view will not
center properly.  The end result is the player view looking up torwards the
ceiling while walking.
Solution: Exit to the next level or use the 'kill' console command..


======================================================
==                 Troubleshooting                  ==
======================================================

If Quake fails to start up, or has problems not addressed elsewhere in the
documentation, try the -safe command line switch, which disables a number
of parts of Quake that can be problems if there are hardware or configuration
problems.  The -safe command line switch is equivalent to -stdvid, -nosound,
-nonet, and -nocdaudio together.  Those four switches do the following:

-stdvid: disables VESA video modes.

-nosound: disables sound card support.

-nonet: disables network card support.

-nocdaudio: disables CD audio support.

If -safe makes the problem go away, try using each of the switches
individually to isolate the area in which you're experiencing the problem,
then either correct the configuration or hardware problem or play Quake with
that functionality disabled.

If you still have problems, try booting clean in conjunction with
the -safe command line parameter.  For information on booting clean, refer
to the "Booting Clean" section above.

If you experience page faults while running Quarterdeck's QDPMI DPMI server,
this is caused by a bug in QDPMI.  Workarounds:  Remove QDPMI from CONFIG.SYS,
issue the command QDPMI OFF before running QUAKE, or get the update patch
for QDPMI from Quarterdeck.  You may be running QDPMI without knowing it if
you have QEMM installed, because it can be installed as part of the QEMM
installation.


Technical Support
-----------------

If you are having trouble installing or running Quake you can receive
technical support by sending e-mailing to support@idsoftware.com.  You can
also refer to our web page, www.idsoftware.com, or call 1-800-idgames.

When sending support e-mail, cut and paste the following into your e-mail
message and fill in the blanks:

Date:  
Name:  
Phone number:  
E-mail address:  (please include this, we redirect tons of mail)
Game Title:  
Version #:  
Operating system (i.e., DOS 6.0 or Windows 95):
Computer type:  
BIOS date:
BIOS version:
Processor type:  
Processor speed:  
Do you program at school/work?
Do you provide tech. support at school/work?
Please state the problem you encountered:
Please state how to reproduce the problem:

If program crashed with nasty undecipherable techno-garbage, please
look for the eight-digit hex number which comes after "eip="
and write it down here:

** NOTE:  If you are sending a bug report, PLEASE refer to the TECHINFO.TXT
file for the correct form and procedures.


======================================================
==                 Version History                  ==
======================================================
v1.01 -- Bugs fixed
------------------------------------------------------
* Fixed modem code
* Fixed fraglimit & timelimit
* Added NOEXIT cvar (so no one can exit a level)
------------------------------------------------------
v1.00 -- Bugs fixed
------------------------------------------------------
* Gravis Ultrasound audio support (still has bugs)
* More deathmatch start spots on E1M6 and END
* Print server version and PROG CRC on connect
* -dedicated starts start.map if nothing else specified
* fixed lookspring function during net game
* fixed rare crash during long running dedicated server
------------------------------------------------------
v0.94 -- Bugs fixed / Features added -- LIMITED BETA VERSION
------------------------------------------------------
* Totally rewritten menus
* New lighting model with overbrighting
* Parsed lowercase BLASTER parms
* Better Sound Blaster shutdown code
* Rewrote BLASTER initialization
* Fixed DMA channel 0 bugs
* Added SBPro 8 stereo setup
* Fix delayed sound on 8 bit Sound Blasters
* Fixed speed key affecting angle-turning from keyboard
* Fixed "no such Alias frame" bugs
* Fixed Zombie not getting up bug
* Checked for very high joystick values, signalling a failed read
* Unstuck jumping Fiends and Spawn
* Fixed large BModels blinking out in complex areas
* Fixed s_localsound with no sound started
* Saved spawn parms in savegame
* Fixed screenshot save location
* Bind with no arguments no longer clears value
* Allow console in intermission / finale
* Fixed false gib messages
* Full-screen TAB scoreboard in DeathMatch
* Fixed "+playdemo <demo>" from command line
* Trapped overflow in sizebuf messages
* Moveup / movedown in water!
* Fixed-up Talk command
* Added unsupported crosshair option ("crosshair 1" from console)
* Colored chat messages with notify sound
* Fixed "connect during intermission" bug
* Changelevel while demos running no longer crashes
* Fixed changelevel with no map left up loading screen
* Fixed long names entered from the console causing crash
* Stopped demos changing while in the menus

* Fixed modem initialization from menu
* Fixed serial reliable stream getting stalled
* Serial/modem code fixes
	16550a lost transmit buffer empty interrupts
	fixed sometimes processing interrupts from com1 when using com2
	added com3/com4 support from menus
	fixed first character of modem init not getting sent
	saved serial/modem settings in config.cfg
* Fixed name and colors not always sent to server at startup
* Fixed "stopdemo" crashing the system when there wasn't a demo playing
* Added server's TCP/IP and IPX addresses (if available) to status command

* In 0.92, an additional check for a usable VESA video mode was added;
the numpages field was verified to be greater than 0, and no mode was
supported that had numpages set to 0 (which indicates that there's not
enough video memory for that mode).  ATI's VESA driver, m64vbe,
reports 0 for numpages, so VESA video modes that were available in 0.91
were no longer available in 0.92.  This extra numpages check has
been removed.

-----------------------------------------------------------------------
v0.93 -- Never officially released; internal testing only.
-----------------------------------------------------------------------
v0.92 -- Bugs fixed
-----------------------------------------------------------------------
Typing long strings in the hostname or modem init field in the menus caused
crashes.

Under Win95 IPX was detected but not functional, resulting in the game
exiting to DOS.

If -nosound, got "S_LocalSound: can't cache" on every keypress in the menu.

When vid_nopageflip was set to 1 in VESA modes, going underwater resulted in
only the upper left corner of the drawing area being updated.

The single player scoreboard (tab) printed text incorrectly in all modes
greater than 320 pixels wide.

On network connections that dropped packets, the reliable message stream
could get stopped up, resulting in frag counts and talk messages no longer
being delivered, although game movement continued.

The com port settings from the menu were getting saved & restored but
not used.

Direct serial connections did not work with slist.

Quake now checks the vesa information for hardware incabable of page-flipping.

Menu sound sometimes didn't play.

Q95 (qlaunch.exe) frequently failed to execute on the first attempt.

Q95 (quakeudp.dll) was running out of buffers when running a server.

Teams were not being set according to pants colors.


Joystick notes
--------------
Your joystick must be plugged in when Quake is launched.

If you have a joystick plugged in, but don't want to use it in Quake
(it slows the game down a few percent), or you have weird hardware that
doesn't like being tested as a joystick add "-nojoy" to your Quake
command line.

You can turn off joystick reading during the game by typing "joystick 0" at
the Quake command console.

You MUST configure your buttons from the configure keys menu before they will
work.  There is no default configuration. 

If your joystick or interface card improperly sets the third or fourth
joystick buttons, type "joybuttons 2" at the quake console or in your
.CFG file.

The "mlook" button command now lets the joystick as well as the mouse control
pitch angles.

The "sidestep" buttom command works on joysticks as with mice and keyboard
movement.

The "invert mouse up/down" menu option also inverts the joystick pitch
direction.
