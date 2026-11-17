# XGLQuake

XGLQuake ports WinQuake 1.06 to Sun XGL on Solaris, targeting a SPARCstation
10 with a 60 MHz SM61 processor, 64 MB RAM, and Sun ZX (Leo) graphics. It
approximates textures with sampled-color polygons and draws the HUD and menus
with native XGL text and geometry. A similar approach can be seen at
[testglobe.dev](https://testglobe.dev/web).

## Quick start

### Build

Install GNU make, GCC, the [Sun XGL SDK](https://archive.org/details/xgl-solaris2.6), and OpenWindows development files.
From the repository root:

```sh
cd WinQuake
make -f Makefile.XGL
```

Run the same make command after source updates; it rebuilds affected files
using generated `.d` dependencies. See [build details](#build-details) for
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
See [game data and display setup](#game-data-and-display-setup) for alternate
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
[full settings table](#renderer-settings) for defaults and other controls.

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
See [profiling details](#profiling-details) for interpreting the results.

### Hardware benchmark

[xglbench](xglbench/README.md) tests basic graphics-card functionality and
performance and generates reports independently of the game.

## Build details

The native target requires 32-bit SPARC Solaris. Development and hardware
testing use Solaris 2.6 and the [tgcware](https://jupiterrise.com/tgcware/)
GNU utilities. The current target is the single-SM61 system described above;
the project's envisaged larger system has two SM61 processors and 128 MB RAM.

Run all `Makefile.XGL` and `Makefile.Solaris` commands in this document from
`WinQuake/`.

The optional dispatcher `make -f Makefile.Solaris xglquake` runs the same
build. Existing software, XIL and OpenGL recipes are independent. Relative to
the repository root, XGL objects are kept in `WinQuake/build/xgl` and the
executable is `WinQuake/xglquake`. All engine objects in this target are
compiled with `XGLQUAKE`, never `GLQUAKE`.

Run the same make command after updating sources; no `clean` is needed for
normal source or header changes. GCC writes a `.d` file beside each object
to track the headers that source actually includes. Objects from older
builds without `.d` files rebuild once to establish these dependencies.
Editing `Makefile.XGL` alone does not force a rebuild. After changing
compiler options, run `make -f Makefile.XGL clean` before rebuilding;
dependency files do not track option changes. `clean` also removes the
generated dependency files.

The SDK paths default to `/opt/SUNWits/Graphics-sw/xgl` and `/usr/openwin`.
Override them if necessary:

```sh
make -f Makefile.XGL XGLHOME=/path/to/xgl OPENWINHOME=/path/to/openwin
```

The target uses the native compiler's 32-bit SPARC default. Use a compiler
configured for that ABI; a 64-bit build is not supported by the original
engine or the XGL renderer. The build enables `-ffast-math`, matching the
original Solaris release recipe, and omits `-ansi`. `CC`, `CPPFLAGS`, `CFLAGS`,
`LANGFLAGS`, `WARNFLAGS`, `LDFLAGS` and `XGL_LIBS` can be overridden on the
command line.

When updating from a build without `-ffast-math`, run
`make -f Makefile.XGL clean` once before rebuilding so every object uses the
new flags. Host regression tests also enable `-ffast-math` by default.

## Game data and display setup

Run game commands from the directory containing `id1/`. The shareware
episode is the initial compatibility baseline. PAK recognition identifies
known file directories; it does not establish complete hardware validation
of every version.

Official Quake 1.00, 1.01 and 1.06 shareware PAK directories are recognized.
For a data directory elsewhere, use `-basedir /path/to/quake`, where that
directory contains `id1/pak0.pak`. Do not use `-game id1` or `-path` to select
the standard shareware data: the engine treats those options as modified
game paths. Startup reports the file count and directory CRC of each PAK
to help identify incompatible, repacked or extra files. Recognition of these
older official PAK versions is specific to the XGL target; registration and
modified-game checks remain enabled.

Use a local display with the Sun ZX's 24-bit TrueColor visual and DGA access.
The renderer requires hardware RGB, Z buffering, shading and two buffers.
`-display`, `-visualid`, `-width`, `-height`, `-winsize` and `-nomouse` select
the display/window/input settings. The video menu offers window sizes.
Original Quake console, save/load, demo, mouse sensitivity and key bindings
remain in the shared engine. The software-only `lcd_x` interlaced framebuffer
mode is not applied to XGL rendering.

### Troubleshooting

If a native XGL call fails, capture both output streams:

```sh
./xglquake -winsize 320 240 +map e1m1 > xglquake.log 2>&1
```

The diagnostic includes the rendering stage, XGL object and operands, error
type/category, and the source line where the error was checked. Some Solaris
installations return an empty error ID and only `Error number` as the message;
the remaining fields are retained to help identify the failing attribute.
Startup console drawing begins before sound initialization, so a graphics
failure at that point does not imply an audio failure.

## Rendering and tuning

World, brush, alias and sprite materials remain CPU palette data. Geometry
is subdivided and sampled into triangle or vertex colors, then submitted to
XGL; the renderer does not upload textures or draw UI image rasters. BSP/PVS
visibility, brush/model transforms, animation and game collision use the
existing engine structures and rules. Sky and water use animated sampled
geometry. Menu, HUD, console and image-based labels use native stroke text
and geometric replacements, so their appearance differs from the original
pixel artwork; unknown pictures receive a labelled fallback.

Texture detail and sprite cutouts are geometric approximations. The XGL
renderer does not implement the OpenGL renderer's optional mirrors, projected
alias shadows or environment-map capture. Those features in the existing
OpenGL target are unchanged.

Baked BSP lighting and animated lightstyles are folded into the sampled
material colors. XGL supplies ambient lighting, up to four nearest active
point lights, and directional fill for alias models. A small material-light
floor permits native lights to affect dark surfaces. This intentionally
approximates Quake's additive dynamic lighting and uses one world shading
pass. Translucent water and screen color effects use native blending;
palette effects appear as native overlay geometry.

Ordinary world materials use cached, filtered CPU color samples. Optional
RGB mip averages share the existing material memory budget; they are never
uploaded as XGL textures. Large visible surfaces receive priority within
the same detail-triangle allowance. The depth-two limit still means large
faces may have coarse patches even when `xgl_subdivide` is small. Filtering
reduces arbitrary single-texel gradients; it cannot reproduce texture detail
smaller than the submitted geometry.

### Native vertex storage

World meshes store contiguous native XGL vertex records separately from
texture/light metadata. Static geometry is queued by reference, avoiding a
conversion and copy every frame; cached colors
change only with material/lightstyle/gamma state. Transformed brush and
cached alias output is written into the backend's batch storage directly.
Particles also write directly into that storage, preparing the palette/gamma
color once per particle instead of converting each corner separately.
The backend checks its vertex layout against the installed SDK and retains
the existing triangle order, shading and 256-triangle batching limit.

### Animated model caching

Animated models cache sampled skin palette indices while retaining current
transforms, lighting, skins and player colors. Cached output prepares each
used palette color's lighting and gamma once per model draw, then reuses its
RGB values at repeated corners. A fixed table of about 3.3 KiB lasts only for
that draw, so lighting and gamma changes take effect immediately. Color
entries use 16 bytes per triangle instead of 40; existing cache limits remain
unchanged. Fallback and sprite drawing retain the generic color conversion
path, including their original cutout coverage and corner-color rules.

Subdivision commands, parent links and triangle indices are shared between
poses; smaller pose entries cache model-local normals. Hashes reject
different command/normal sequences quickly, with exact comparisons before
reuse. Current pose positions are still transformed each frame. All model
cache storage remains bounded, with recursive drawing when caching fails.
Sampled skin caches retain their visible-triangle count, avoiding a repeated
coverage scan when emitting a cached pose.
The two flame models remain unlit and skip the world-lighting lookup.

Sprites collect their sampled triangles in the same bounded fallback buffer
used by alias models. This reduces CPU draw calls while preserving triangle
order and per-triangle cutout sampling. Weapon depth clearing reuses the
tracked opaque/depth-write state; its completion waits remain in place.

### Sky and water sampling

Animated meshes sample matching vertices once per update and reuse their
colors at repeated triangle corners. The sample map is built with the mesh
and charged to the world cache limit. If allocation fails, each triangle
corner is sampled separately. Indexed samples use a
direct palette-to-gamma lookup. Animation is still updated every frame,
including sky scrolling, camera-relative sky projection and turbulent UVs.

### UI batching

Consecutive opaque UI rectangles are batched without changing their drawing
order. Native text runs combine only when measured font advances and spacing
preserve the existing fixed-cell glyph positions. Incompatible metrics,
spaces and clipped cells split runs, retaining individual-glyph drawing as
needed. This does not assume that Roman_M is monospaced. Completion waits are
issued only when native work is pending.

### Renderer settings

| Setting | Default | Effect |
| --- | ---: | --- |
| `xgl_subdivide` | 128 | World texture-coordinate subdivision step, clamped to 8–256; smaller adds detail. |
| `xgl_detailtris` | 1024 | Detailed world/brush triangle allowance; visible world faces are ranked by projected-size estimate, with complete coarse faces when exhausted. |
| `xgl_detaildistance` | 512 | Distance within which detailed world surfaces are considered. |
| `xgl_alias_subdivide` | 1 | Alias subdivision depth, 0–2; distant models use less detail. |
| `xgl_entity_triangles` | 512 | Additional alias subdivision triangles per frame; original model triangles remain. |
| `xgl_sprite_cells` | 8 | Sample grid per sprite dimension, clamped to 2–16. |
| `xgl_playercolors` | 1 | Apply original player shirt/pants palette translations to sampled skins. |
| `xgl_ambient` | 1 | Native ambient contribution, clamped to 0–1. |
| `xgl_maxlights` | 4 | Maximum native point lights, clamped to 0–4. |
| `xgl_profile` | 0 | 1 reports graphics timings and native call counts; 2 adds geometry preparation and cache statistics. Output goes to the terminal once per second. |
| `_snd_mixahead` | 0.3 | Audio lookahead in seconds for XGL; existing archived values override this default. |
| `r_wateralpha` | 1 | Water opacity; values below 1 request native blending. |

Settings in configuration files or command-line options override these defaults.
In particular, `xgl_entity_triangles` is saved in `config.cfg`, so an older
saved value remains in effect until changed.

The original `r_dynamic`, `r_fullbright`, `r_novis`, `r_drawentities`,
`r_drawviewmodel`, `viewsize`, `fov`, `gamma` and `r_speeds` controls are also
available. The initial settings are bounded starting points, not measured
Sun ZX performance recommendations.

### Display mapping

The 3D context uses the benchmark's automatic full-raster VDC mapping.
Quake's view rectangle is selected through the VDC window and view clipping,
without explicitly setting `XGL_CTX_DC_VIEWPORT`. This preserves `viewsize`
and the space reserved for the status bar. Native UI uses device coordinates.

## Memory limits

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

## Solaris audio and hardware interfaces

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

## Profiling details

Use the two [performance commands](#measure-performance) above with the same
resolution, demo, audio lookahead, and detail settings for each comparison.

Keep the console closed during playback and quit after the timedemo result
appears. The console stays visible after playback and has a different workload;
its later profiles must not be averaged into gameplay results. Compare the
timedemo FPS from both runs to measure profiling overhead. A first frame that
builds caches should also be distinguished from sustained rendering. Repeat
the same procedure at another resolution only when testing resolution cost.

### Timing scope

The profiler prints graphics-frame mean/max time and world, entity, effects,
weapon and UI stage times. Native submission, synchronization, buffer-switch and audio
service timings overlap those stages; do not add them to the graphics total.
Counts include actual emitted 3D triangles (including alias subdivision and
particles), native glyphs, polygon calls and completion waits. `gfx` excludes
server work and the final shared sound update, so it is not a whole-game FPS
measurement; use the timedemo result for that comparison. Profiling is off
by default and performs no timing queries when disabled. Terminal reporting
avoids adding in-game console glyphs to the workload being measured.

Level 1 retains the summary report. Level 2 adds more timing queries and
reports where world preparation time goes, vertex packing, alias cache work,
cache use, and native text calls. These detail measurements overlap the main
stages; nested preparation measurements must not be summed into a frame total.
World detail selection includes mesh construction. Ordinary color timing
includes cache validation and refresh; animated timing covers sky/water color
updates. Alias cache/transform timing covers cache lookup/build and the cached
path's live vertex transforms, excluding recursive fallback drawing and final
color/output assembly. Packing excludes native submission and direct native
output; gamma preparation for direct alias output remains in the entity or
weapon stage. A drop in packing time alone is not an FPS gain. Native triangle
batches can span stage boundaries, so the stage that flushes a batch can
include submission work queued by an earlier stage.

Level 2 also separates alias, brush and sprite draw scopes, weapon setup/draw/
finish, and particle/water effects. Each reports wall time, native submission
time and synchronization time in milliseconds per frame. Native times are
included in wall time and refer to calls executed during that scope, including
any geometry queued earlier. The scopes preserve batching and make timing
queries only at level 2. Particle time includes the shared particle simulation;
the effects water scope measures queued water drawing, while the earlier
water color timer measures animated color preparation.

Weapon setup includes draining earlier geometry and issuing the depth clear;
finish includes draining weapon geometry. The separate depth-clear timer
measures the native clear call itself, with completion waits counted under
synchronization. Entity visits/frame counts calls to each model-type drawing
routine, including calls rejected by its visibility checks; it excludes the
view weapon.

The alias entity and weapon detail lines separate world-lighting lookup time
from cached output and fallback drawing. Lighting times only the actual
`R_LightPoint` call; unlit flame models and fullbright drawing skip it.
Cached output covers final position/normal copying, light multiplication,
gamma preparation and native batch reservations after pose transformation.
Lighting/gamma preparation in this path is reused for matching palette
indices within the draw.
Fallback drawing includes uncached vertex transforms, subdivision, skin
sampling and generic vertex packing. Each output/fallback scope reports
wall/submission/synchronization time, including native calls made inside it.
These are nested measurements within the existing entity or weapon scopes,
not additional costs to add to them. Audio service after the draw is outside
these detail scopes.

### Counters

Detailed counts are per-frame averages. World cached draws include meshes
built during that frame, while fallback draws count complete streamed coarse
faces; they are not a cache-hit percentage. Color reuse/updates count mesh
update calls, which can occur multiple times for a streamed face. Alias color
and normal/topology (reported as pose) cache hits, misses, evictions and fallback
draws expose cache pressure without changing cache limits. Compare native
text calls with emitted glyphs to see how much the installed font permits
batching.

The reuse line separates sky and water time and compares actual animated
samples with emitted animated vertices. Native cached vertices are queued
from persistent meshes; native direct vertices are filled in the backend
batch; packed vertices use the general conversion path. The alias preparation
line separates cache work from live transforms and reports shared-topology
hits/misses and pose evictions caused by entry or byte limits. These counters
identify geometry preparation and cache costs. Measure FPS with profiling
disabled.

## Development-host checks

Shareware recognition, CPU texture/mesh, native UI/entity/particle submission, audio
queueing and profiling tests need no SDK or game data. They run against
controlled engine/backend mocks. Alias color checks also use the real CPU
texture sampler to compare cached output with the original color calculation.
Run from `WinQuake/`:

```sh
make -f Makefile.XGL check
```

These tests require a modern host compiler and GNU-compatible linker. They
are separate from the native Solaris build. For complete engine compilation
and a host link with native API placeholders, run:

```sh
make -f Makefile.XGL host-check XGLHOME=/path/to/xgl-sdk
make -f Makefile.XGL native-contract-check XGLHOME=/path/to/xgl-sdk
```

The `host-check` target requires Python 3, a modern host C compiler, and the
actual Sun XGL SDK headers supplied separately. Host-only handwritten X11
and Solaris contracts provide declarations needed where native development
headers are unavailable. They are not substitutes for native libraries.
To use real X11 development headers instead, add
`HOST_X11_INCLUDE=/path/to/include` to the host-check command.
The executable generated by `host-check` is linked only, never executed; native API
placeholders abort if called. This catches unresolved engine symbols and
accidental OpenGL dependencies. The check also compares modified existing
shared engine files against `HEAD` under software and `GLQUAKE` preprocessing to
verify that their existing paths remain unchanged. Temporary artifacts and
the compiler log are reported by the script.

The `native-contract-check` target builds and runs mocked backend tests
against the SDK headers; it does not require Python. It captures SDK calls to check projection,
clipping, lighting selection/state changes, depth-clear state, gamma and
native font placement/cache behavior. It also checks view placement after
resizing, strided/prepared vertex colors, cached/direct geometry batches,
UI batch ordering and lifetimes, completion waits, and error diagnostics
with missing library message fields.
It does not rasterize those calls.

These host checks do not establish Solaris linking, SPARC ABI compatibility,
native font rendering, Sun ZX correctness, or performance. Validate changes
with a native build and game run on the target Sun, including map startup,
gameplay, audio, fonts, lighting, and transparency. No game assets are bundled
with the host checks, and host test results do not establish hardware FPS.
