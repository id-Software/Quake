--------------------------
| wqreadme.txt           |
| WinQuake documentation |
| 3/21/97                |
--------------------------

WinQuake (WQ) is a native Win32 version of Quake, and will run on
either Win95 or Windows NT 4.0 or later.  It is designed to take
advantage of whatever enhanced video, sound, and input capabilities
(such as DirectX or VESA VBE video modes) are present, but has
fallback functionality so it can run on any Win95 or NT 4.0 or later
system, even if neither DirectX nor VESA VBE is available.  You may
experience problems running WQ on some systems, because driver and
operating-system support for game functionality are not yet mature
under Win32, and many bugs and incompatibilities remain in those
components.  If you encounter what seems to be a bug, first please
check through the list of known problems, below.  If your problem
doesn't appear on the list, please fill out and submit the WQ bug
report at http://www.idsoftware.com/contact/.

The material accompanying Quake is the reference for all
non-Windows-related matters concerning WinQuake; in terms of gameplay,
WQ is the same as Quake.  This file contains Windows-related
information only.

The rest of this document is organized as follows:

Installing and running WinQuake
Common problems and workarounds
A bit about how WQ video works
Video command-line switches
A bit about how WQ sound works
Sound command-line switches
Notes on networking
Notes on the mouse
Log of changes to documentation
Special thanks


-----------------------------------
| Installing and running WinQuake |
-----------------------------------

In order to run WinQuake, you must first have Quake installed.
Assuming Quake is installed in the standard directory, c:\quake,
unzip the WinQuake zip file into c:\quake.  The following files
from the zip file must be present in order for WQ to run:

winquake.exe
pmpro16.dll
pmpro32.dll
wdir16.dll
wdir32.dll
wdirnop.com
wdirnop.pif

Then you can run WinQuake by making c:\quake the current directory,
typing "winquake" and pressing the Enter key.  Alternatively, you can
use wq.bat to run WinQuake.  The wq batch file requires one parameter
describing how to configure WQ for performance; just type "wq" to get
a list of the six options.  The first of the six options is

wq fast

This is the same as typing "winquake"; this runs WinQuake in an
aggressive configuration that is likely to yield the best performance
if it runs successfully on your system, but which has a risk of
causing WinQuake or even your system to crash if there are bugs or
incompatibilities in your video or sound drivers.  Alternatively, you
can use

wq safe

to run WinQuake in a conservative configuration, likely to run
on almost all machines with no problems, but possibly with slower
graphics, fewer high-resolution modes, and delayed sound.  Or you
can run

wq verysafe

to run WinQuake in a very conservative configuration that is pretty
much guaranteed to run, but will probably have slow performance, and
will have no sound.  Two other options are

wq fastvid

which has maximum video performance, but greater sound latency (delay
until the sound is heard), and

wq fastsnd

which uses more conservative video modes, but low-latency sound.

(One odd note is that DirectSound has much lower-latency sound than
wave sound, but is currently quite a bit slower overall.  Thus you
may find that "wq fastvid" is actually faster, by as much as 5-10%,
than "wq fast"; however, it may not feel faster, because the sound
will lag.)

Finally, you can use

wq max

which is the same as wq fast, but turns on DirectInput, which
provides more responsive mouse control, but does not work properly
on all systems.

Note that DirectX is not required for WQ to run, but WQ will
automatically take advantage of DirectSound and DirectDraw if they
are present.  If DirectSound is not present, there will generally be
considerable sound latency (sound will become audible several hundred
milliseconds after the event that caused it).  Note also that there
are currently no true DirectSound drivers for Windows NT, so WQ will
always run using wave output on NT, and will consequently have lagged
sound.  See below for information about obtaining DirectX if you do
not have it.

Note that VESA VBE modes aren't required for WQ to run, but WQ will
automatically make VESA modes available if they're present.  Your BIOS
may already have VESA VBE 2.0 support built in, but most BIOSes
don't. Worse, some BIOSes do have VESA VBE 2.0 built-in, but have
buggy implementations, which may prevent you from being able to run
the faster configurations of WQ.  An easy way to get reliable VESA 2.0
support is by obtaining SciTech Display Doctor; see below for
further information. WQ can also use VBE/AF 1.0 and greater modes;
again, SciTech Display Doctor is the commonest way to get VBE/AF
support.

Note that winquake -dedicated completely replaces the old winded
dedicated Win32 server, which is now obsolete.

WinQuake normally uses half the physical memory in your system for its
heap, but not less than 8.5 Mb and not more than 16 Mb.  You can
override this with "-heapsize n", where n is the amount of memory to
allocate for the heap, in Kb.

To use the joystick, you must bring down the console by pressing the
tilde ('~') key, and type "joystick 1<enter>"; you can disable the
joystick with "joystick 0<enter>" at any time.  The joystick setting
remains in effect for subsequent WinQuake sessions until changed, so
you only need to do joystick 1 once to enable the joystick.  If the
joystick somehow causes problems that keep you from being able to run
WinQuake at all, you can start WinQuake -nojoy to complete disable the
joystick for that session.


-----------------------------------
| Common problems and workarounds |
-----------------------------------

WQ crashes or won't run
-----------------------

If WQ refuses to run or crashes on your system, try running
it using "wq safe" or "wq verysafe".  Or you can use command-line
switches:

winquake -nodirectdraw -nowindirect -wavonly

This will almost certainly solve your problem; however, it may result
in  lagged sound (a long delay from action to hearing the sound), may
result in fewer or slower high-res video modes, and the mouse may be
somewhat less responsive.  If this does work, you can try removing
each of the command-line switches until you identify the one that
fixes the problem, thereby sacrificing as little functionality as
possible.

If the above command line does not fix your problems, try:

winquake -dibonly -nosound

which forces WQ into silent operation with bare-bones video support
and no use of DirectInput for mouse input (the normal Windows mouse
APIs are used instead).  Again, if this works, try removing switches
until you identify the needed one.

Both of the above command lines are quick fixes.  Often, the problem
is caused by outdated or buggy DirectX drivers or code, and can
frequently be completely fixed simply by installing the latest
Microsoft-supplied version of DirectX, which you may be able to find
on http://www.microsoft.com/mediadev/download/directx.exe, although
the availability and location of the DirectX file changes
periodically; note that at last check, this is a 3.4 Mb file.  (Be
aware, though, that sometimes Microsoft's DirectX drivers don't
support features that the manufacturers' drivers do support, such as
display refresh rate control.)

One known problem of this sort involves the current SB16 drivers from
Creative Labs, which cause WQ to crash on some machines.  The
DirectSound drivers from Microsoft, available via the above-mentioned
URL, fix this problem.

It can also sometimes help to get the latest Windows drivers for your
video adapter or sound card (although as the SB16 example indicates,
this is not always a good idea), and for video boards that have flash
BIOSes, it can sometimes help to get the latest BIOS upgrade.


How do I select fullscreen or windowed WQ operation?
----------------------------------------------------
Check out WQ's new, spiffy Video menu, accessible from the Options
menu.  There are now two types of modes listed, windowed and
fullscreen.  You can make any of these modes the current and/or
default mode, just as in DOS Quake.  If you make a windowed mode the
default, WQ will still briefly start up in fullscreen mode, then
switch to windowed; if this is a problem, use the -startwindowed
command-line switch.  More complete video control is available
through the console, as described in the "A bit about how WQ video
works" section, below.


Gee, I wish I could use a mouse to play WQ with in a window
-----------------------------------------------------------
You can!  While in a windowed mode, go to the Options menu.  At the
bottom, you'll find a new selection that lets you choose to have the
mouse active when you're in a window.  Of course, if you do this,
you'll have to use the keyboard (Alt-Tab, the Windows key, Ctrl-Esc,
Alt-Esc, or Shift-Alt-Tab) to switch away from WQ.


Serial/modem menu is missing
----------------------------
WQ currently does not support direct connect serial or modem play.


DOS Quake reports unknown variables on startup after running WQ
---------------------------------------------------------------
WQ uses some console variables that do not exist in DOS Quake, and
some of these are automatically archived in config.cfg when you exit
WQ.  If you then start DOS Quake, DOS Quake will complain that it
doesn't recognize those variables.  You will also lose the settings
of these variables when you return to WQ.  Apart from losing the
settings, this is harmless; ignore it.


Problems running WQ on NT 3.51
------------------------------
NT 3.51 isn't supported by WQ.


WQ crashes while switching modes or Alt-Tabbing
-----------------------------------------------
So far, all cases of this seem to be tied to Creative Lab's SB16 sound
drivers, and have been fixed by getting the latest DirectX drivers, as
described above.  Alternatively, you should be able to fix this either
by not switching modes or Alt-Tabbing, or by running -wavonly to
disable DirectSound support.


WQ sometimes runs pretty slowly fullscreen
------------------------------------------
There are several possible reasons for this, starting with "You have a
slow computer."  Assuming that's not the case, if you don't have
either DirectDraw or SciTech Display Doctor installed (see the "A bit
about how WQ video works" section), it would probably be a good thing
to install one or the other, because slow operation can be a result
of slow copying or stretching of pixels to the screen by a Windows
driver, something that's eliminated by both DirectDraw and Display
Doctor.  You can also sometimes get a faster 320x200 mode on Win95 by
doing vid_describemodes, then using vid_mode to select a non-VGA
320x200 mode, as described in the "A bit about how WQ video works"
section.

You can also try using a primary sound buffer on Win95 (this doesn't
work on NT) by using the -primarysound command-line switch; this can
improve performance by several percent, but does not work on all
systems, and can result in odd sound effects on some systems when
minimizing WQ or switching the focus away from it.  If you use this
switch, please don't report sound bugs; it's in there purely for you
to use if it helps you, and we know it has problems on many systems.
Finally, you can use -wavonly to select wave sound; this will increase
your sound latency (sounds will be heard later than they should), but
allows WQ to run 5-10% faster on some systems.  That's about all you
can do to speed up fullscreen WQ on Win95, other than shrinking the
active area of the screen with the screen size control in the Options
menu.

NT 4.0 comes with DirectX installed, but doesn't have any resolutions
lower than 640x480.  In order to support a lower-resolution 320x240
mode, WQ has NT double each pixel in both directions to get enough
pixels for 640x480.  The extra stretching costs some performance, the
result being that NT can seem sluggish on all but high-end Pentiums
and Pentium Pros.  (In fact, depending on the quality of your driver's
stretching code, it can sometimes be faster to run WQ at 640x480 than
320x240-stretched on NT.)  One thing that can help on NT is switching
to 640x480, then using the Options menu to shrink the active area of
the screen.

A common cause of slowness running in a window is having the desktop
run in 16- or 32-bpp mode.  WQ is an 8-bpp application, and it slows
things down if pixels have to be translated from 8-bpp to 16- or
32-bpp.  (Note that this is generally a problem only when running in a
window; fullscreen apps rarely suffer from this.)


Sound is sluggish on NT
-----------------------
NT doesn't have any real DirectSound drivers yet, so there's no way to
do quick-response sound on NT.  When DirectSound drivers for NT
appear, WQ's sound should automatically be snappier.


Sound breaks up or gets choppy, especially in menus
---------------------------------------------------
This is generally a sign that WQ's frame rate is too low on your
system.  Try reducing resolution or shrinking the active area of the
screen.  In some circumstances, it may help to set the console
variable _snd_mixahead to a larger value.


The color black doesn't change with palette flashes sometimes
-------------------------------------------------------------
Normally, DirectDraw lets WQ change all 256 colors, so when a palette
flash happens, we can change all the colors, including black.
However, on NT DirectDraw currently doesn't allow changing black;
likewise, on both NT and Win95, black can't be changed in a window,
either a normal window or fullscreen.  Consequently, in some modes and
in a window, some parts of the WQ screen (such as the sigils on the
status bar and the spray where a shotgun blast hits) stay black when
the palette flashes.  There is no workaround.


Problems can result if Office shortcut bar is running
-----------------------------------------------------
Various odd behaviors, especially with sound, have been reported if
the Office shortcut bar is running while WQ is running.  If you
experience odd problems, you might try shutting down the Office
shortcut bar and see if that fixes anything.


Other apps fail to play sound while WinQuake is running
-------------------------------------------------------
The sound hardware is currently not a fully shareable resource on
Win32.  Consequently, while WQ is running, it always has the sound
hardware allocated to itself, to make sure that sound is never lost to
another app.  This means that normally (when WQ is using DirectSound),
apps that use wave sound (most non-game apps) will not be able to play
sound while WQ is running, even if WQ is minimized or not the active
app, although other DirectSound apps will be able to play sound when
WQ is not the active app.  If WQ is using wave sound rather than
DirectSound (either because -wavonly is used on the command line, or
because there is no DirectSound driver, as is always the case on NT),
then no other app will be able to play any sound while WQ is running,
period.


WQ doesn't have quite the right colors when it’s not the active app
-------------------------------------------------------------------
We're working on fixing this.  But WQ puts everything back again as
soon as it is reactivated, and anyway, when it’s not active, you can’t
actually do anything in WQ, so it doesn’t really matter anyway, right?


Desktop is weird colors when WQ runs windowed
---------------------------------------------
WQ needs all 256 colors to look right and run fast, which causes it to
have to change some of the 20 colors used to draw the desktop.


Sometimes Permedia boards crash unless -nowindirect is used
-----------------------------------------------------------
It looks like this is probably a Permedia driver bug, so it might help
if you get the most recent drivers.


Right-click on WQ button in task bar to close doesn’t work as expected
----------------------------------------------------------------------
In some modes, right-clicking on the WQ task bar button doesn't work
the way you'd expect.  We're trying to fix this, but if it's a
problem, don't right-click.


Screen saver never kicks in when running WQ fullscreen
------------------------------------------------------
It does work windowed, but when WinQuake is fullscreen, it completely
owns the screen and doesn't share it with anyone, even the
screensaver.  If you use Alt-Tab to minimize WQ, the screensaver will
then be enabled, so Alt-Tab away from WQ if you're leaving your
computer alone for a while and want the screensaver to be able to kick
in.


WQ doesn’t work in a window in 16-color mode
--------------------------------------------
That’s 16 *colors*, not 16-bpp.  If you’re still running a 16-color
desktop, run WQ fullscreen.


Can't minimize window while mouse active
----------------------------------------
When running in a window with the mouse active as a WQ input device,
there is no easy way to minimize the window, because the system menu
can't be brought up from the keyboard (because some of you use Alt
and Spacebar for playing the game), and the mouse can't be used to
manipulate the window because it's controlling WQ.  To minimize, you
can disable the mouse for WQ and use it to minimize the window.  Or
on Win95 you can Alt-Tab away from WQ, then use the mouse to
minimize (this doesn't work on NT, where clicking on the window
controls just reactivates WQ).  Or you can bind a key to the
vid_minimize command, as in

bind m "vid_minimize"

and press that key to minimize the window.


Window controls don't work on NT when mouse enabled
---------------------------------------------------
When running in a window on NT with the mouse enabled (so you can use
the mouse to play WQ), if you Alt-Tab away from WQ, then use the mouse
to click on the WQ system menu control, or the minimize, maximize, or
close controls, the controls are ignored and WQ just reactivates.


Mouse sometimes vanishes in system menu on Win95
------------------------------------------------
On Win95, if WQ is running in a window with the mouse enabled (so you
can use the mouse to play WQ), if you Alt-Tab away, then click on the
system menu, the menu comes up, but the mouse vanishes.  However, you
can still use the keyboard to select system menu items, or to exit
the system menu.


WQ behaves oddly if Scandisk starts defragmenting
-------------------------------------------------
If WQ is running fullscreen on Win95 when Scandisk starts an automatic
defragging, WQ is forced to minimize, and when it is brought back up,
may either be in a strange mode where it runs one frame for each
keystroke (in which case Alt-Tab generally fixes things), or may hang
the system.  We don't know what the problem is right now, but you may
want to make sure you don't leave WQ sitting there fullscreen
overnight if you have automatic defragging.


Hang reported with zero sound volume
------------------------------------
When sound is turned all the way down via the WQ menus, hangs have
been reported.


Joystick worked fine with earlier versions of WinQuake but not now
------------------------------------------------------------------
The joystick was enabled by default in earlier versions of
WinQuake, but quite a few people reported serious problems that
forced them to disable the joystick--even some people who didn't
have a joystick attached.  Since most people don't have joysticks,
we've decided to disable the joystick by default, and let people
who do want to use it set joystick 1 in the console (WinQuake
remembers this setting, so this only needs to be done once).


WQ runs very slowly when it has the focus under NT
--------------------------------------------------
In one case, WQ ran very slowly when it had the focus, but fast when
it didn't (obviously this is only visible in windowed modes).  The
problem turned out to be that NT had a Sidewinder 3D Pro joystick
driver installed; when the driver was removed, things were fine.
If you see a similar problem, check whether WQ is detecting that
your system has a joystick when you don't think it should; if so,
try doing "joystick 0", or -nojoy on the command line, and see if
that fixes it.  If so, there's something flaky in your system
joystick setup.


Joystick doesn't seem calibrated properly
-----------------------------------------
WQ relies on the information about your joystick stored in the
system registry.  If the joystick seems miscalibrated, run the
joystick applet and recalibrate and see if that fixes things.


Playdemo fails across multiple levels
-------------------------------------
If "record" is used to record a client-side demo, bad things will
happen on playback via playdemo if a level change is recorded.
(Timedemo works fine.)  This is unfortunate, but WinQuake
internals make this not fixable without a good chance of
breaking something more important, so it'll have to stay this way.


Alt-Tab fullscreen only works sometimes
---------------------------------------
I know it seems that way, but actually the trick is that on Win95
it only works if you let go of Tab before you let go of Alt.
This is due to a Windows quirk involving what key sequences are
passed along, so you'll have to work around it by remembering to
let go of Tab first.


MS-DOS windows get scrunched on Alt-Tab
---------------------------------------
This is a quirk of Windows; when you run WinQuake in a low-res
mode, sometimes when you exit WinQuake or Alt-Tab back to the
desktop, any open MS-DOS windows will be scrunched down to the
size of the low-res mode.  There is no known workaround.


Dprint in progs doesn't work
----------------------------
Dprint means "developer print," so it only works if the developer
console variable is set to 1.  It was a bug in earlier versions that
it worked even when developer was set to 0.


Some DirectDraw modes flicker badly and look wrong
--------------------------------------------------
Page flipping doesn't work properly in some modes on some
systems, particularly when using some DirectDraw modes.  You
can work around this by setting the console variable
vid_nopageflip to 1, then setting the desired mode (note
that the vid_nopageflip setting does not take effect until
the next mode set after the setting is changed).  Bear in
mind, though, that the vid_nopageflip setting is remembered
until it is explicitly changed again, meaning that once you
change it, it thereafter applies to all modes, even if you
exit and restart WinQuake.


The Windows key doesn't do anything fullscreen on Win95
-------------------------------------------------------
True.  This is a minor bug we haven't figured out how to fix yet.
You'll have to use Ctrl-Esc, Alt-Tab, or Alt-Esc to switch away.


My default mode is windowed, but WQ goes fullscreen first
---------------------------------------------------------
For internal reasons, WQ has to pick a single mode to always
initialize when it starts up, before it sets whatever default you've
selected.  We've chosen fullscreen mode, because that's the way most
people will play.  If this is a problem for you, however, you can
run WQ with the -startwindowed command-line parameter.


Some high-resolution video modes flicker or fail to initialize
--------------------------------------------------------------
We think these problems are all fixed, but if not, they have to
do with triple-buffering in some modes on some DirectDraw drivers.
If you encounter this problem, either don't use the problem modes
or try using the -notriplebuf command-line parameter to turn off
triple buffering.  Note, though, that turning off triple-buffering
can reduce performance in some modes, so do this only if needed.


Right-click doesn't work right on minimized WinQuake
----------------------------------------------------
If you right-click on minimized WinQuake on the task bar, the
Close selection in the right-click menu doesn't work; you have
to restore WQ before you can exit it.  Also, the cursor vanishes
over the right-click menu, although it still works.


The screen briefly blanks when you exit WQ
------------------------------------------
We're trying to fix this, but it's not harmful, just a mite ugly.


QBENCH doesn't work with WinQuake
---------------------------------
We've had a report that QBENCH doesn't work with WinQuake, but
haven't had a chance to look into it yet.


MWAVE sound loses focus
-----------------------
We've had a report that on a ThinkPad with MWAVE sound, WQ loses
sound focus (and thus sound) every few seconds.


Desktop doesn't reset to proper resolution on WQ exit
-----------------------------------------------------
We've had a report that on exiting WQ, the desktop didn't reset
to the proper dimensions.  This may be a bug with the Matrox
drivers, but we're not sure.  If it's a problem and newer
drivers don't fix it, you can run -dibonly, which solves the
problem but can cost some performance.


Palette goes bad periodically on #9 Imagine card
------------------------------------------------
There's only one report of this, so maybe it's a flaky board,
or maybe it's a driver bug.  Newer drivers might help.


System with Packard Bell sound card III crashes on CapsLock
-----------------------------------------------------------
This appears to be the result of buggy DirectSound drivers;
-wavonly makes the problem go away.


Dvorak keyboard mapping ignored
-------------------------------
WQ is hardwired for QWERTY.


Cursor messed up after running WQ
---------------------------------
This is a Windows driver bug; the driver isn't restoring the
cursor properly on return from fullscreen WQ to the desktop.
Try newer drivers.


Dedicated server runs very slowly while typing at console
---------------------------------------------------------
When you type at a dedicated server's console, the game runs
very slowly for everyone who's connected.


Ctrl-Alt-Del on NT sometimes doesn't allow return to WQ
-------------------------------------------------------
This happens on some machines while running WQ fullscreen.
If you experience this problem, the only workaround is not
to press Ctrl-Alt-Del while fullscreen; Alt-Tab away first.


Many fast Alt-Tabs on Win95 sometimes disable WQ input
------------------------------------------------------
If you Alt-Tab fast lots of times on Win95 with WQ running
fullscreen, sometimes you end up in fullscreen WQ, with the
game not accepting any keyboard input (so there's no way to
exit).  The only workaround is to not do lots of fast
Alt-Tabs (why you'd want to, I'm not sure).



----------------------------------
| A bit about how WQ video works |
----------------------------------

WQ has the built-in ability to draw into windows (both normal, framed 
desktop windows and fullscreen, borderless windows).  It also has
built- in support for VGA 320x200 graphics, and supports DirectDraw,
VESA VBE 2.0 and VESA VBE/AF (Accelerator Functions) graphics modes,
if those are available.

WQ does not require DirectDraw, but in order for DirectDraw modes to
be available, you must have DirectDraw installed; some systems come
with it preinstalled, but if it's not on your system, you can download
it from http://www.microsoft.com/mediadev/download/directx.exe (the
exact URL may vary), and install it.

WQ does not require VESA VBE, but in order for VESA VBE modes to be
available, your graphics card must be VESA VBE 2.0 or VBE/AF
compliant; a VESA driver can either be built into the BIOS of your
graphics card, or loadable via software.  If you don't have a VESA VBE
driver, Scitech Display Doctor, available from Scitech Software, will
update most graphics cards to VESA VBE 2.0 and VBE/AF.


SciTech Display Doctor
----------------------
If you are having problems with your video drivers, or if you would
like to take a shot at improving your video performance in WQ, you may
want to try out SciTech Display Doctor (SDD).  SDD works on just about
any graphics card and it can do several things that can make WQ run
better on your system:

1. It will update your graphics card to be compatible with VESA VBE
2.0 and VESA VBE/AF (Accelerator Functions). These modes will usually
give you the best performance in WQ (which is often but not always
faster than your current performance).

2. It creates low-resolution modes on your graphics card.
Low-resolution video modes (such as 320x240, 400x300 and 512x384)
allow you to adjust the level of detail in WQ so you can get the best
balance between performance and image quality.

The latest version of SciTech Display Doctor can be obtained from the
following locations:

www: http://www.scitechsoft.com
ftp: ftp.scitechsoft.com
CIS: GO SCITECH
AOL: Keyword SciTech

SciTech can be contacted at:

email: info@scitechsoft.com

SciTech Software, Inc.
505 Wall Street
Chico, CA  95926-1989
916-894-8400
916-894-9069 FAX


Video modes supported in Win95
------------------------------
What all this means is that on Win95, WQ will always be able to run in
the following modes:

1)  in a window
2)  fullscreen 320x200 VGA mode 0x13
3)  fullscreen high-resolution of some sort

Category #3 can be any of several configurations.  On Win95, if either
DirectDraw or VESA VBE modes are available, then all the DirectDraw
and VESA modes will be presented as high-res choices.  (320x200 will
always default to VGA mode 0x13.)  In the case that a given resolution
is supported by both DirectDraw and VESA, the VESA mode will be used.
(However, the command-line switch -nowindirect can turn off VESA modes
entirely.)  If neither DirectDraw nor VESA modes are available, then
high-resolution modes will be provided by using fullscreen, borderless
windows in whatever resolutions the Windows driver supports, usually
starting at 640x480 and going up.


Video Modes Supported in Windows NT
-----------------------------------
NT is similar but not identical, because neither VESA VBE modes nor
VGA mode 0x13 are available.  On NT, WQ will always be able to run in
the following modes:

1)  in a window
2)  fullscreen high-resolution of some sort

On NT, category #2 can be one of two configurations.  If DirectDraw
modes are available, then those will be the high-res choices;
otherwise, fullscreen, borderless windows will be used in whatever
resolutions the driver supports, usually starting at 640x480 and going
up.  Because there is normally no low-resolution mode such as 320x200
or 320x240 on NT, a pseudo low-res mode is created by rendering at
320x240, then stretching the image by doubling it in each direction
while copying it to a 640x480 screen.  However, stretching performance
depends on the driver, and can be slow, so sometimes 640x480 is
actually faster than 320x240 on NT.

The bottom line here is that you can generally just use the Video menu
and pick one of the modes and be happy.  In some cases, though, you
may need to use command-line switches (described next) to get the
types of modes you want.  One useful tip is to go into the console and
do vid_describemodes, which lists all the modes WQ makes available on
your machine given the command-line switches you've used.  Each mode
is followed by the name of the internal WQ driver that supports it, so
you can tell which modes are DirectDraw, VESA, and so on, as follows:

WINDOWED: WQ runs in a normal window
FULLSCREEN DIB: fullscreen borderless window
FULLSCREEN VGA8.DRV: VGA 320x200 mode
FULLSCREEN DDRAW8.DRV: DirectDraw mode
FULLSCREEN LINEAR8.DRV: VESA VBE 2.0+ mode
FULLSCREEN ACCEL8.DRV: VESA VBE/AF (Accelerator Functions) mode
                       (note that WQ does not take advantage of
                       VBE/AF acceleration; so far as WQ is
                       concerned VBE/AF is the same as normal VBE)

You can use vid_mode from the console to set any of these modes.  So,
for example, if you see that there are two 320x200 modes (such as one
VGA mode 0x13, normally mode 3, and one VESA mode, normally mode 4),
you can choose the VESA mode, which will often be faster, with
vid_mode 4.  (You can make it the default by setting
_vid_default_mode_win to the mode number.)

There's more to the windowed modes than you might think.  320x240 is
just what you’d expect, but 640x480 is actually rendered at 320x240
and stretched up to 640x480, because most machines can’t handle the
performance demands of real 640x480 rendering.  Likewise, 800x600 is a
stretched 400x300.  Actually, though, vid_mode 2 (the 800x600 mode) is
a user-configurable mode.  By setting the following console variables,
you can change the characteristics of vid_mode 2:

vid_config_x: width of mode 2 window

vid_config_y: height of mode 2 window

vid_stretch_by_2: whether to render at half-resolution in each
direction and stretch up to the specified size in mode 2, or render at
full resolution.

After setting these variables in the console, do a vid_forcemode 2,
and you’ll have the window you specified.  Note that after making
these changes, the new resolution will show up as the third windowed
mode in the Video menu.

If you don't have WQ mouse play enabled in windowed mode, you can also
go from windowed to fullscreen mode simply by clicking on the maximize
button.  The mode switched to is controlled by the vid_fullscreen_mode
console variable, and defaults to mode 3.

Other video console commands include:

vid_fullscreen: switch to the mode specified by the
vid_fullscreen_mode console variable.

vid_windowed: switch to the mode specified by the vid_windowed_mode
console variable.

Vid_fullscreen and vid_windowed can be bound to keys, so it's possible
to flip between windowed and fullscreen with a single key press.

Also, vid_minimize minimizes the WinQuake window if and only if
WinQuake is running in a windowed mode.  You can bind a key to
the commands "vid_windowed; wait; vid_minimize" to minimize WQ
regardless of whether you're running in windowed or fullscreen mode.

You can turn off page flipping by setting the console variable
vid_nopageflip to 1, then setting a new mode.  (Note that the
vid_nopageflip setting does not take effect until the next mode set.)
Some systems run faster with page flipping turned off; also, page
flipping does not work properly on some adapters, and vid_nopageflip
is a workaround for this.  Note that vid_nopageflip is a persistent
variable; it retains its setting until it is explicitly changed again,
even across multiple WinQuake sessions.

The vid_forcemode console command sets the specified mode, even if
it's the same as the current mode (normally the mode set only happens
if the new mode differs from the current mode).  This is generally
useful only if you've modified the characteristics of video mode 2
(the configurable window) while you're in mode 2, and want to force
the new characteristics to take effect.

Whenever you switch to running WinQuake in a window, the window is
placed at the same location it was in the last time WinQuake ran
in a window.  You can reset the window position to the upper left
by using the -resetwinpos command-line switch.  The window position
is stored in the vid_window_x and vid_window_y console variables.



-------------------------------
| Video command-line switches |
-------------------------------

The full list of video-related command-line switches is:

-dibonly: WQ will use only windows (both normal, framed windows on the
desktop and fullscreen, borderless windows), not any direct hardware
access modes such as DirectDraw or VESA modes, or even VGA 320x200
mode.  This is the closest thing to a guaranteed-to-run fullscreen
mode WQ has.

-nowindirect: WQ will not try to use VESA VBE 2.0 modes, or VBE/AF
1.0 or later modes.  Note that if there are both DirectDraw and VESA
modes for a given resolution, WQ will normally use the VESA mode;
-nowindirect allows DirectDraw modes to be the preferred choice for
all resolutions except 320x200.  This can be useful if WQ is crashing
because of a buggy VESA driver.

-nodirectdraw: WQ will not try to use DirectDraw modes.  This can be
useful if WQ is crashing because of a buggy DirectDraw driver.

-novbeaf: WQ will not try to use VBE/AF 1.0 or later modes.

-startwindowed: WQ will come up in a windowed mode, without going
fullscreen even during initialization.

-noforcevga: normally, WQ uses VGA mode 0x13 for the default 320x200
mode, even if a DirectDraw or VESA 320x200 mode exists.  However,
DirectDraw and VESA modes can be considerably faster than mode 0x13,
because they can set up a linear framebuffer with higher memory
bandwidth.  If you specify -noforcevga, the default 320x200 mode in
the menu will be a DirectDraw or VESA mode if one exists.  This has no
effect on modes selected via the console variable vid_mode, and if
320x200 is already your video mode, -noforcevga doesn't do anything
until you use the menu to select another mode, then select 320x200
again.  (So if your default mode is 320x200 and you then specify
-noforcevga, switch away to some other mode and then back to 320x200
to get the potentially faster 320x200 mode.)  The downside to this
switch is that DirectDraw and VESA modes can cause problems in some
systems, due to driver bugs or hardware incompatibilities; if you
experience problems with this switch, don't use it.

-noautostretch: don't stretch windowed modes selected with
-startwindowed to double resolution.

-nofulldib: don't use fullscreen, borderless windows, even if there
are no DirectDraw or VESA modes available.

-allow360: allow listing of 360-wide modes in the video mode menu.
These are normally filtered out to make sure the menu doesn't get too
full, which could cause high-res modes not to be displayed.

-notriplebuf: prevent triple-buffered page flipping (rather than double-
buffered).  This may result in slower performance, but is a workaround
if you encounter problems with flicker or initialization failure, which
could possibly happen in some modes with some DirectDraw drivers.



----------------------------------
| A bit about how WQ sound works |
----------------------------------

WQ can use either DirectSound or Windows wave output to generate
sound.  If DirectSound is available, it is used; if not, if wave sound
is available it is used; and if neither is available, there is no
sound.  DirectSound results in the best sound quality, and also the
lowest-latency sound; use it if you can, because you will be happier
with the results.  (Note, though, that no NT sound drivers yet support
DirectSound.)  Wave sound will often have high latency, lagging the
events that generate sound by hundreds of milliseconds on some
machines.

You can tell what kind of sound WQ uses on your system by looking at
the startup portion of the console; you will see either "DirectSound
initialized" or "Wave sound initialized" (neither message is printed
if there's no sound).  Any sound failure messages will also be printed
in the startup portion of the console.

Note that WQ generates sound only when it is the active app, the one
with the input focus.



-------------------------------
| Sound command-line switches |
-------------------------------

The full list of sound-related command-line switches is:

-wavonly: don’t use DirectSound, but use wave sound if available.
Note that wave sound is generally faster than DirectSound, but has
considerably greater latency.  This switch is redundant on NT, because
all sound output on current NT drivers is wave sound.

-nosound: don’t output any sound.

-primarysound: use DirectSound primary buffer output.  This is
generally faster than normal secondary buffer output, but does not
work in some systems, and produces odd sound glitches on minimization
and focus switching in other systems.  Use it at your own risk, and
please do not report sound bugs if you're using this switch.

-snoforceformat: WQ will not try to force the sound hardware to 11
KHz, 16 bits per sample.  This may be useful if DirectSound is failing
for no apparent reason, but generally WQ will produce better sound and
better performance if this switch is not used.



-----------------------
| Notes on networking |
-----------------------

The winsock TCP/IP driver will not cause a dial-up internet connection
to automatically start up when Quake is started.  If you start Quake
with it inactive, the connection will be activated when you either try
to connect to a server or search for local servers.

The local IP address will not always be known at startup.  If it is
currently unknown the menu will display "INADDR_ANY".  This will be
replaced with the real address when it is known.  The IP address will
become known when you try to connect to a server, you search for local
servers, or you start a server.

For multi-homed machines (machines with more than one network adapter
and IP adress), you can force WinQuake to bind to a specific IP
address.  There is a command line option "-ip" that takes an IP
address as its parameter.



----------------------
| Notes on the mouse |
----------------------

If DirectInput is installed and functioning, WinQuake can use it for
mouse input, but does not do so automatically because DirectInput does
not work properly on all systems.  DirectInput can be enabled via the
command-line switch -dinput.  If DirectInput is not available or is
not enabled, WinQuake uses the normal Windows mouse APIs instead.
DirectInput provides slightly smoother motion; also, it tends to be
more responsive to fast spinning motions, and we recommend that you use
it if it works properly on your system.  You can determine if WQ uses
DirectInput on your system when you use -dinput by checking for
"DirectInput initialized" in the startup console text.  If not, you
might try installing DirectX 3 (note, though, that as I write this
there is no released DirectInput support for Windows NT, only Win95).



-----------------------------------
| Log of changes to documentation |
-----------------------------------

*** WinQuake 0.994 ***

Fixed bug where in some cases involving IPX, whenever a new person
entered the game, a current player got dumped.

Added DirectInput mouse support, and the -dinput command-line to
enable it.

Added -notriplebuf to disable triple buffering to work around
possible problems with some modes on some DirectDraw drivers.

Added remembering last window position, and restoring that, rather
than centering the window, whenever WinQuake runs in a window.
Can be reset with the -resetwinpos command-line switch.

Added the vid_minimize command, which minimizes WinQuake if and only
if the current mode is windowed.

Made it so WinQuake no longer gets suspended when Alt-Tab is used to
switch away from a fullscreen session.  The means you can Alt-Tab
away from fullscreen WinQuake without losing a connection to a
Quake server.

Added vid_nopageflip console variable to turn off page flipping, and
documented page flipping problems this can be used to work around.

Documented that Del-Ctrl-Alt (in that order) causes WinQuake to
fault on Win95.

Fixed the winsock TCP/IP driver so it will not cause a dial-up
internet connection to automatically start up when Quake is started.
If you start Quake with the internet connection already active, there
will be no difference.  If you start Quake with it inactive, the
connection will be activated when you either try to connect to a
server or search for local servers.

The local IP address will not always be known at startup now.  If it
is currently unknown the menu will display "INADDR_ANY".  This
will be replaced with the real address when it is known.  The IP
address will become known when: you try to connect to a server,
you search for local servers, or you start a server.

For multi-homed machines (machines with more than one network
adapter and IP adress), you can now force WinQuake to bind to a
specific IP adress.  There is a new command line option "-ip" that
takes an IP address as its parameter.

Added vid_fullscreen_mode and ability to select that mode by
clicking on the maximize button.  Added two commands:
vid_fullscreen to switch to vid_fullscreen_mode, and
vid_windowed to switch to vid_windowed mode.

Changed joystick default to disabled; now it only works if the
joystick cvar is set to 1; however, this setting now remains in
effect permanently.  Added joystick documentation below.

Documented dprint only works if developer set to 1.

Documented scrunching of MS-DOS windows on Alt-Tab.

Documented that NT versions earlier to 4.0 are not supported.

Added DirectInput support for devices such as First Person
Gaming's Assassin controller.  All buttons should now
be configurable in WinQuake.

Fixed bug where when low-res fullscreen DIB modes selected from
the menu sometimes ran very slowly or produced garbled displays.

Fixed bug where 1.06 and earlier save files couldn't be loaded
by WinQuake.

Removed "Net play pauses every few seconds" bug; it was determined
not to be a bug, just an artifact of network play.

Noted that even when BIOSes do have VESA 2.0 built-in, it's often
so buggy that WinQuake crashes in faster configurations, and that
SciTech Display Doctor is the easiest way to get reliable VESA
support.

Added note on Alt-Tab only working if Tab released first.


*** WinQuake 0.992 ***

Implemented force_centerview.

Fixed backspace bug in dedicated console.

Made "player entering game" messages and "say" messages visible
in dedicated console.

Added description of -heapsize (how to change default memory
allocation).

Added description of "net play pauses every few seconds" bug.

Added description of "playdemo fails across multiple levels" bug.

Added hooks for QHost; however, WinQuake won't work with QHost
until a new version of QHost 3.0, which uses the hooks, is
released.  QHost 3.0 will not work with WinQuake.

Fixed bug where savegame descriptions weren't always terminated
properly.

Fixed bug where running -dedicated reset part of config.cfg to
defaults.


*** WinQuake 0.991 ***

Fixed problem with pre-1.07 (DOS) clients connecting to WinQuake.

Got rid of "Starting Quake..." dialog when running -dedicated.

Added -novbeaf switch to turn off VBE/AF support in case of problems,
and updated documentation.

Corrected SciTech's U.S. Mail address in documentation.

Added joystick bug decriptions and workarounds.



------------------
| Special thanks |
------------------

Special thanks for help with WinQuake to:

James Barnes
Kendall Bennett
Raymond Chen
John Colleran
Andrew Goossen
Mike Harrington
Chris Hecker
Todd Laney
Scott Ludwig
...and all the beta testers!

======================================================================
End of Document 3/21/97
======================================================================
