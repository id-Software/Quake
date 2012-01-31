#!/bin/sh
# Generate quake-data.spec
# $1 is version
# $2 is release
# $3 is install dir (assumed to be in /var/tmp)
cat <<EOF
%define name quake-data
%define version ${1}
%define release ${2}
%define builddir \$RPM_BUILD_DIR/%{name}-%{version}
Name:		%{name}
Version:	%{version}
Release:	%{release}
Vendor:		id Software
Packager:	Dave "Zoid" Kirsch <zoid@idsoftware.com>
URL:		http://www.idsoftware.com/
Source:		quake-data-%{version}.tar.gz
BuildArchitectures: noarch
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

This package contians the Quake data files needed to play the game.

%install

%files
%attr(644,root,root) $3/comexp.txt
%attr(644,root,root) $3/help.txt
%attr(644,root,root) $3/licinfo.txt
%attr(644,root,root) $3/manual.txt
%attr(644,root,root) $3/readme.txt
%attr(644,root,root) $3/rlicnse.txt
%attr(644,root,root) $3/techinfo.txt
%attr(644,root,root) $3/id1/pak0.pak
%attr(644,root,root) $3/id1/pak1.pak

%post
/sbin/ldconfig

EOF

