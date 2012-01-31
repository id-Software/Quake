#!/bin/sh
# Generate quake-shareware.spec
# $1 is version
# $2 is release
# $3 is install dir (assumed to be in /var/tmp)
cat <<EOF
%define name quake
%define version ${1}-shareware
%define release ${2}
%define builddir \$RPM_BUILD_DIR/%{name}-%{version}
Name:		%{name}
Version:	%{version}
Release:	%{release}
Vendor:		id Software
Packager:	Dave "Zoid" Kirsch <zoid@idsoftware.com>
URL:		http://www.idsoftware.com/
Source:		quake-%{version}.tar.gz
Group:		Games
Copyright:	Restricted
Icon:		quake.gif
BuildRoot:	/var/tmp/%{name}-%{version}
Summary:	Quake for Linux

%description
"Quake is the biggest, baddest, and bloodiest 3-D action game ever 
conceived" - PC GAMER

"The most important PC game ever" - PC ZONE

""Quake": Bloody Amazing" - USA TODAY

"The Vanguard of a terrifying new level of immersive interactivity" - 
COMPUTER GAMING WORLD

From the creators of DOOM and DOOM II comes the most intense, technologically
advanced 3-D experience ever captured on CD ROM.  Features free and fluid
motion, ambient sound and lighting, and unmatched multiplayer capabilities
(play with up to 15 others).

Included in this archive are several different versions of Quake.

- SQuake for SVGALib Console Graphics
- GLQuake for 3DFX and other glX based hardware OpenGL
- Quake.X11 for running Quake under X11

%install

%files
%attr(644,root,root) $3/README
%attr(4755,root,root) $3/squake
%attr(4755,root,root) $3/glquake
%attr(4755,root,root) $3/glquake.glx
%attr(4755,root,root) $3/glquake.3dfxgl
%attr(755,root,root) $3/quake.x11
%attr(644,root,root) $3/help.txt
%attr(644,root,root) $3/licinfo.txt
%attr(644,root,root) $3/manual.txt
%attr(644,root,root) $3/readme.txt
%attr(644,root,root) $3/slicnse.txt
%attr(644,root,root) $3/techinfo.txt
%attr(644,root,root) $3/id1/pak0.pak
%attr(755,root,root) /usr/lib/lib3dfxgl.so
%attr(755,root,root) /usr/lib/libMesaGL.so.2.6

%post
/sbin/ldconfig

EOF

