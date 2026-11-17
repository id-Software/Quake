# XGLQuake
Port of WinQuake 1.06 to XGL for Sun Solaris e.g. Leo/ZX 3D graphics hardware on a SparcStation. Since the ZX lacks texture-mapping, XGLQuake subdivides the polygons and samples the textures to approximate the appearance of the textures. An example of how this works can be seen in a web browser at [testglobe.dev](https://testglobe.dev/web).

Solaris 2.6 was used for development and testing as well as the [tgcware](https://jupiterrise.com/tgcware/) GNU utilities. The target system is one 60MHz SM61 processor, ZX graphics, and 64MB RAM. The ideal system is envisioned to be 2x SM61 processor and 128MB RAM.

## Build
You need XGL libraries and headers installed and configured. The default locations are included in the Makefile.

```make -f Makefile.XGL```

The resulting binary is ```xglquake``` which must be placed in your quake folder above the ``id1`` folder and other quake files.

## PAKs
This is supposed to work with Quake 1.00 Shareware PAK files but that is untested. If unsure, use a registered copy of 1.06's ```id1/pak0.pak``` and ```id1/pak1.pak```.

## Options
### winsize
``-winsize 640 480``
### _snd_mixahead
``+_snd_mixahead 0.3``

## Demo
``./xglquake -winsize 640 480 +_snd_mixahead 0.3 +xgl_profile 0 +timedemo demo1 > xglquake-timedemo.txt 2>&1``
