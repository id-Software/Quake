#!/bin/sh
# Generate qwcl.spec
# $1 is version
# $2 is release
# $3 is install dir (assumed to be in /var/tmp)
cat <<EOF
%define name qwcl
%define version ${1}
%define release ${2}
%define builddir \$RPM_BUILD_DIR/%{name}-%{version}
Name:		%{name}
Version:	%{version}
Release:	%{release}
Vendor:		id Software
Packager:	Dave "Zoid" Kirsch <zoid@idsoftware.com>
URL:		http://www.idsoftware.com/
Source:		qwcl-%{version}.tar.gz
Group:		Games
Copyright:	Restricted
Icon:		quake.gif
BuildRoot:	/var/tmp/%{name}-%{version}
Summary:	QuakeWorld Client

%description
QuakeWorld is an Internet multi-player specific version of Quake. While the 
original version of Quake can be played over the Internet, many users modem 
users - the majority of players, had less than satisfactory play. Symptoms 
like excessive lag - actions actually happening much later than you did them; 
packet loss - the game would freeze and resume several seconds later; and 
various other difficulties plagued users.

After realizing how many people played Quake on the internet, and how many 
wanted to, but couldn't due to the play being unsatisfactory, John Carmack 
of id Software decided to create a version of Quake that was optimized for 
the average modem Internet player. This Internet specific version does only 
one thing, play deathmatch games over a TCP/IP network such as the Internet. 
It has no support for solo play, and you can't do anything with out connecting 
to a special server.

Now in it's second generation, QuakeWorld has been sculpted by countless 
hours of user feedback and tweaking to provide the best multi-player 
experience that can be had from the equipment the average gamer will have. 
All that is required to use QuakeWorld is registered Quake.

%install

%files
%attr(644,root,root) ${3}/README.qwcl
%attr(4755,root,root) ${3}/qwcl
%attr(4755,root,root) ${3}/glqwcl
%attr(4755,root,root) ${3}/glqwcl.glx
%attr(755,root,root) ${3}/glqwcl.3dfxgl
%attr(755,root,root) ${3}/lib3dfxgl.so
%attr(755,root,root) ${3}/libMesaGL.so.2.6
%attr(755,root,root) ${3}/libMesaGL.so.2
%attr(755,root,root) ${3}/libMesaGL.so
%attr(755,root,root) ${3}/libGL.so
%attr(755,root,root) ${3}/qwcl.x11
%attr(755,root,root) ${3}/qw/skins/fixskins.sh
EOF


