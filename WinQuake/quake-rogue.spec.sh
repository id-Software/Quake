#!/bin/sh
# Generate quake-rogue.spec
# $1 is version
# $2 is release
# $3 is install dir (assumed to be in /var/tmp)
cat <<EOF
%define name quake-rogue
%define version ${1}
%define release ${2}
%define builddir \$RPM_BUILD_DIR/%{name}-%{version}
Name:		%{name}
Version:	%{version}
Release:	%{release}
Vendor:		id Software/Rogue Entertainment
Packager:	Dave "Zoid" Kirsch <zoid@idsoftware.com>
URL:		http://www.rogue-ent.com/
Source:		quake-rogue-%{version}.tar.gz
BuildArchitectures: noarch
Group:		Games
Copyright:	Restricted
Icon:		quake.gif
BuildRoot:	/var/tmp/%{name}-%{version}
Summary:	Quake Rogue Mission Pack #2: Dissolution of Eternity

%description

16 entirely new, and totally intense levels! The environmental experience of 
Quake is unparalleled. Dissolution of Eternity sustains the same dynamic 
sense of reality and dramatic visuals; yet adds an impression of purpose 
and continuity. This expansion pack is most definitely not a conglomeration 
of clone levels! 

8 new monsters! Quake is solid, hard-hitting action with a host of evil 
enemies. We've made additions to the Quake monster roster to intensify the 
action and fill every dimly lit and evil corner of the game. 

4 new weapons! The Quake weapons are awesome, but we've pushed them beyond 
that with the addition of extreme power-ups to make the arsenal more 
effective, and more deadly. 

Oh yeah, the Power-Ups! 

Single player mode power-ups include:

The Anti-Grav Belt. The theory here is simple. It counters the force of 
gravity and allows the player to make difficult jumps. 

The Power Shield. This 'little gem' significantly decreases the damage you 
receive from most enemy attacks. If you're feelin' frisky in Deathmatch, 
try out the ram attack. It's an added 'weapon' to send your opponents flying. 

Deathmatch will introduce the player to:

The Vengeance Sphere. We'd like you to experience it before we let you in on 
what it does. Here's a hint: Ultimate retribution from the grave! 

Random Respawn. This feature will toss in a bit of Deathmatch variety. Hey, 
we can't be responsible for what awesome power up 'pops up' next. 

Multi-player!

Normal Deathmatch. Friend fraggin' action! 
Team Play. Bring a friend, to kill a foe. 
Tag. You played it as a kid, but nothin' says "Tag!" like a face full of 
Plasma. 

And, a much enhanced version of Capture the Flag! 
Capture the Flag. You've seen it on the Internet; now it's in your hands. 
Additional value, additional excitement, and additional Deathmatch 'down and 
dirty' devastation. CTF is one of the most exciting team multi-player 
directions for Quake to date; and the most popular too! 

Including two brand new CTF features. 

One Flag - It's a mad race for the flag, the loser gets a rocket up his 
privates. 
Three Team - Red, Blue, and the new Rogue (pun intended) Grey team. The good 
news? You can take either flag and bring to it either Red or Blue base. The 
bad news? You don't have your own base! 

Also, a collection of lethal environmental hazards have been placed at every 
turn. They are challenging, and terminal! Here are just a few of them.

The Pendulums - Set to slice the player to shreds when he least expects it! 
Lightning Shooters - Well, they shoot lightning and can be directed at any 
angle to make some rooms extremely hazardous. 
Lava Nail Shooters - Well, they shoot, you get the idea here! 
Earthquakes - Earthshaking environments! Level areas that tremor with 
seismic activity. Now the Quake player can literally quake! 
Buzz Saws - Compact. Cordless. And completely gib-o-matic! These dreaded 
traps are set in floors and ceilings. 

%install

%files
%attr(644,root,root) $3/rogue/pak0.pak
%attr(644,root,root) $3/rogue/docs/manual.doc
%attr(644,root,root) $3/rogue/docs/manual.htm
%attr(644,root,root) $3/rogue/docs/manual.txt
%attr(644,root,root) $3/rogue/docs/readme.doc
%attr(644,root,root) $3/rogue/docs/readme.htm
%attr(644,root,root) $3/rogue/docs/readme.txt
%attr(644,root,root) $3/rogue/docs/ctf.doc
%attr(644,root,root) $3/rogue/docs/ctf.htm
%attr(644,root,root) $3/rogue/docs/ctf.txt

EOF

