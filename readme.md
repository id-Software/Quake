# XGLQuake
Port of WinQuake 1.06 to XGL for Sun Solaris e.g. Leo/ZX 3D graphics hardware on a SparcStation. Since the ZX lacks texture-mapping, XGLQuake subdivides the polygons and samples the textures to approximate the appearance of the textures. An example of how this works can be seen in a web browser at [testglobe.dev](https://testglobe.dev/web).

Solaris 2.6 was used for development and testing as well as the [tgcware](https://jupiterrise.com/tgcware/) GNU utilities. The target system is one 60MHz SM61 processor, ZX graphics, and 64MB RAM. The ideal system is envisioned to be 2x SM61 processor and 128MB RAM.

## Build
You need XGL libraries and headers installed and configured. The default locations are included in the Makefile. Build from the `WinQuake` directory:

```sh
make -f Makefile.XGL
```

Run the same command after source updates. Generated `.d` files track included headers, so only affected objects rebuild. Existing objects rebuild once to generate these files. Changing compiler options still requires `make -f Makefile.XGL clean` before rebuilding.

The resulting binary is ```xglquake``` which must be placed in your quake folder above the ``id1`` folder and other quake files.

See [the XGL renderer notes](WinQuake/xgl_README.md) for setup, memory limits, and OPT-1 (vertex packing), OPT-2 (model cache reuse), and OPT-3 (sky/water sampling).

## PAKs
This is supposed to work with Quake 1.00 Shareware PAK files but that is untested. If unsure, use a registered copy of 1.06's ```id1/pak0.pak``` and ```id1/pak1.pak```.

## Options
### _snd_mixahead
Prevent audio buffer exhaustion.

```+_snd_mixahead 0.3```

### xgl_alias_subdivide
Limit extra subdivision of alias models. The default is 1; 0 retains the original model triangles with coarser sampled texture colors.

```+xgl_alias_subdivide 0```


### xgl_detailtris
Limit the detailed world/brush triangle allowance. The default is 4096; 1024 reduces detail. Complete coarse faces remain when the allowance is exhausted, so this is not a cap on the total triangles drawn.

```+xgl_detailtris 1024``` 

### xgl_profile
0 disables profiling, 1 reports stage timings, and 2 adds preparation/cache statistics. Use 0 for FPS comparisons and 2 to diagnose remaining costs.

```+xgl_profile 1```


### winsize

```-winsize 640 480```

## Demo

Measure FPS without profiling:

```sh
./xglquake -winsize 640 480 +_snd_mixahead 0.3 +xgl_profile 0 +timedemo demo1 > xglquake-timedemo.txt 2>&1
```

Then collect a detailed profile using the same resolution and detail settings:

```sh
./xglquake -winsize 640 480 +_snd_mixahead 0.3 +xgl_profile 2 +timedemo demo1 > xglquake-profile.txt 2>&1
```

Keep the console closed during playback and quit after each timedemo finishes. Console rendering after the demo is a different workload.

## xglbench
Benchmark to test basic card performance and functionality and generate reports.

