#!/bin/sh
# Generate quake-hipnotic.spec
# $1 is version
# $2 is release
# $3 is install dir (assumed to be in /var/tmp)
cat <<EOF
%define name quake-hipnotic
%define version ${1}
%define release ${2}
%define builddir \$RPM_BUILD_DIR/%{name}-%{version}
Name:		%{name}
Version:	%{version}
Release:	%{release}
Vendor:		id Software/Ritual Entertainment
Packager:	Dave "Zoid" Kirsch <zoid@idsoftware.com>
URL:		http://www.ritual.com/
Source:		quake-hipnotic-%{version}.tar.gz
BuildArchitectures: noarch
Group:		Games
Copyright:	Restricted
Icon:		quake.gif
BuildRoot:	/var/tmp/%{name}-%{version}
Summary:	Quake Hipnotic Mission Pack #1: Scourge of Armagon

%description

The Scourge of Armagon is the highly acclaimed mission pack for id Software's 
Quake. Scourge offers 3 new episodes that continue off where Quake ended. Add 
some incredible new effects such as rotating doors and objects, new creatures 
to battle, and new weapons and powerups to assist you in these battles. 

Looking for a way to breath some new life into Quake? Get Mission Pack #1: 
Scourge of Armagon.

Here is a breakdown of exactly what you will get inside Scourge of Armagon:

18 Beastly Levels
Strafe and slaughter your way through 18 levels contained in three 
monster-infested episodes. Fight your way through the intense Military 
Base and Research Facilities, sneak around in the Ancient Realm, and dash 
into the final domain controlled by Armagon. You’ll also be able to tear 
your friends apart in an all-new DeathMatch level: Edge of Oblivion.

All-New Flesh-Ripping Demons
Gremlins: Small, blood-thirsty creatures that attack in swarms and can even 
steal your weapons. Watch out or these little bastards will turn them back on 
you!

Centroids: Armor-plated scorpion cyborgs with two nail guns fused to their 
monstrous bodies. These bad boy monsters can shred you to bits in just 
seconds. 

Armagon: This is the father of all monsters. This huge metallic behemoth will 
rock your world with rockets and laser fire, so be prepared to die. 

Never-Before-Seen Weapons
Mjolnir: When this war hammer pounds the floor, it fires an electrical force 
along the ground, striking and shocking several enemies with its lethal 
current.

Laser Cannon: Disintegrate scores of creatures with scorching laser blasts. 
Use the rebounding properties of this weapon to tag your enemies around 
corners.

Proximity Mines: Gameplay reaches new heights with motion sensitive mines. 
Plant these nasty toys on walls, floors, or even the ceiling and watch as 
unsuspecting foes are blown into little bits.

Inventive New Powerups
Empathy Shield: This shield allows you to turn the table on those unwitting 
opponents. Whatever damage you take they receive half of it back in kind.

Horn of Conjuring: Blow this horn and out of the mystical bowels of eternity 
a new ally will appear. This horn allows you to call upon any of the 
creatures in the game to come to your aid in the fight against Armagon.

Wet Suit: What better way to survive under water longer. Not only that, those 
flippers help speed you along as you paddle your way through the dangers that 
lucky deep under the surface of those calm waters.

Environmental Dangers
Undertake atmospheric nightmares like lightning traps, floating spike mines, 
gondolas, outdoor environments and stunning architecture never seen before in 
Quake. Travel through the dangerous and treacherous mines under the Military 
Base. Creep through the catacombs and dark cathedrals that lie in wait for 
you in the Ancient Realms. Swallow your fear as you wind our way through the 
immense fortress that protect the Lair of Armagon.

System Requirements

Full Version of QUAKE required to operate. 
CD ROM drive required for Installation 
Linux 2.0
16 MB RAM 
Hard Disk Space: 40 MB for Mission Pack 
VGA & SVGA graphics support 
Mouse support (3 Button Mouse recommended) 
Supports IP (Internet) play 

%install

%files
%attr(644,root,root) $3/hipnotic/pak0.pak
%attr(644,root,root) $3/hipnotic/config.cfg
%attr(644,root,root) $3/hipnotic/docs/manual.doc
%attr(644,root,root) $3/hipnotic/docs/manual.htm
%attr(644,root,root) $3/hipnotic/docs/manual.txt
%attr(644,root,root) $3/hipnotic/docs/readme.doc
%attr(644,root,root) $3/hipnotic/docs/readme.htm
%attr(644,root,root) $3/hipnotic/docs/readme.txt

EOF

