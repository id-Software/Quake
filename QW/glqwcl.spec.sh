#!/bin/sh
# Generate qwcl.spec
# $1 is version
# $2 is release
# $3 is install dir (assumed to be in /var/tmp)
cat <<EOF
%define name glqwcl
%define version ${1}
%define release ${2}
%define builddir \$RPM_BUILD_DIR/%{name}-%{version}
Name:		%{name}
Version:	%{version}
Release:	%{release}
Vendor:		id Software
Packager:	Dave "Zoid" Kirsch <zoid@idsoftware.com>
URL:		http://www.idsoftware.com/
Source:		glqwcl-%{version}.tar.gz
Group:		Games
Copyright:	Restricted
Icon:		quake.gif
BuildRoot:	/var/tmp/%{name}-%{version}
Summary:	OpenGL QuakeWorld Client

%description
"Quake is the biggest, baddest, and bloodiest 3-D action game ever 
conceived" - PC GAMER

"The most important PC game ever" - PC ZONE

""Quake": Bloody Amazing" - USA TODAY

"The Vanguard of a terrifying new level of immersive interactivity" - 
COMPUTER GAMING WORLD

%install

%files
%attr(644,root,root) ${3}/readme.glqwcl
%attr(4755,root,root) ${3}/glqwcl
%attr(4755,root,root) ${3}/glqwcl.glx
%attr(755,root,root) ${3}/glqwcl.3dfxgl
%attr(755,root,root) ${3}/lib3dfxgl.so
EOF


