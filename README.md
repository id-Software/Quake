# XGLQuake

XGLQuake ports WinQuake 1.06 to Sun XGL on Solaris, targeting a SPARCstation
10 with a 60 MHz SM61 processor, 64 MB RAM, and Sun ZX (Leo) graphics. It
approximates textures with sampled-color polygons and draws the HUD and menus
with native XGL text and geometry. A similar approach can be seen at
[testglobe.dev](https://testglobe.dev/web).

## Quick start

### Build
see [build details](docs/build.md) for information.

Install GNU make, GCC (for example [tgcware](https://jupiterrise.com/tgcware/sunos5.6_sparc/stable/)), the [Sun XGL SDK](https://archive.org/details/xgl-solaris2.6), and OpenWindows development files.
From the repository root:

```sh
cd WinQuake
make -f Makefile.XGL
```

Run the same make command after source updates; it rebuilds affected files
using generated `.d` dependencies. See [build details](docs/build.md) for
SDK paths and when a clean build is needed.

### Game data and launch

Place the resulting `xglquake` executable in your Quake directory, alongside
`id1/`. Shareware uses `id1/pak0.pak`; registered Quake 1.06 uses `id1/pak0.pak`
and `id1/pak1.pak`. Game data is not included.

Run from that Quake directory:

```sh
./xglquake -winsize 320 240 +_snd_mixahead 0.3 +map e1m1
```

Official Quake 1.00, 1.01, and 1.06 shareware PAK directories are recognized.
See [game data and display setup](docs/setup.md) for alternate
locations and shareware restrictions.

### Common options

| Command-line option | Purpose |
| --- | --- |
| `-winsize 640 480` | Set the window size. |
| `+_snd_mixahead 0.3` | Buffer 0.3 seconds of audio. |
| `+xgl_alias_subdivide 0` | Keep original model triangles without extra subdivision. |
| `+xgl_detailtris 1024` | Set the detailed world triangle allowance; complete coarse faces remain. |
| `+xgl_profile 0` | Disable profiling for FPS measurements. Use 1 for summary timings or 2 for detail. |

Lower subdivision makes sampled textures and lighting coarser. The detail
allowance is not a cap on all drawn triangles. See the
[full settings table](docs/render.md#renderer-settings) for defaults and other controls.

### Measure performance

Measure FPS without profiling:

```sh
./xglquake -winsize 640 480 +_snd_mixahead 0.3 +xgl_profile 0 +timedemo demo1 > xglquake-timedemo.txt 2>&1
```

Then collect a detailed profile with the same resolution and detail settings:

```sh
./xglquake -winsize 640 480 +_snd_mixahead 0.3 +xgl_profile 2 +timedemo demo1 > xglquake-profile.txt 2>&1
```

Keep the console closed during playback and quit after each timedemo finishes.
See [profiling details](docs/profile.md#profiling-details) for interpreting the results.

## Hardware
### XGL benchmark

[xglbench](xglbench/README.md) tests basic graphics-card functionality and
performance and generates reports independently of the game.

### Memory limits

The existing Solaris main allocates a 16 MiB engine heap. World geometry
cache storage is capped at another 8 MiB, CPU material pixels and optional
filter levels at another 8 MiB, and alias color caches at 1 MiB. The alias
normal/topology cache, its fixed entry table and reusable work array together
are capped at another 1 MiB. The XGL audio ring is 32 KiB.
Native contexts, window buffers, program data and
library allocations are additional. These limits are not a measurement of
total process memory.
World cache/detail exhaustion falls back to complete coarse faces; exceeding
the CPU material limit reports an error rather than silently dropping assets.

### Solaris audio and hardware interfaces

Solaris audio uses `/dev/audio`, 11,025 Hz 16-bit stereo, and the original
mono fallback. The XGL-specific `xgl_snd_sun.c` retains those device interfaces
with a larger mixing buffer and corrected nonblocking submission. CD audio
remains the existing null driver. System services and BSD/UDP networking use
the original Solaris source files and libraries.

The original Solaris submission path writes at most 1,024 bytes per update,
only about 23 ms of stereo audio. The XGL path submits all available mixed
sound, retains unwritten data on partial writes/backpressure, and services
the mixer during rendering and cache preparation. Its default 0.3-second
lookahead trades some sound latency for tolerance of long frames. A single
blocking graphics call longer than the queued audio can still cause a gap;
this is not an independent audio thread.

An existing `config.cfg` may restore `_snd_mixahead 0.1`; use
`+_snd_mixahead 0.3` to override that value for a run.

## Renderer
see [renderer](docs/render.md) for information.

## Profiling
see [profiling](docs/profile.md) for information.

## Testing
see [testing](docs/test.md) for information.
