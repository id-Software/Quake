# Initial Solaris XGL renderer

`xglquake` is a separate WinQuake renderer for a 32-bit SPARC Solaris system
with Sun XGL and OpenWindows development files. Its initial hardware target
is a 60 MHz SPARCstation 10 with 64 MB RAM and a Sun ZX (Leo).

Build from this directory with GNU make and GCC:

```sh
make -f Makefile.XGL
```

The optional dispatcher `make -f Makefile.Solaris xglquake` runs the same
build. Existing software, XIL and OpenGL recipes are independent. XGL objects
are kept in `build/xgl`; the executable is `xglquake`. All engine objects in
this target are compiled with `XGLQUAKE`, never `GLQUAKE`.

The SDK paths default to `/opt/SUNWits/Graphics-sw/xgl` and `/usr/openwin`.
Override them if necessary:

```sh
make -f Makefile.XGL XGLHOME=/path/to/xgl OPENWINHOME=/path/to/openwin
```

The target uses the native compiler's 32-bit SPARC default. Use a compiler
configured for that ABI; a 64-bit build is not supported by the original
engine or this initial port. The build deliberately has no `-ansi` flag and
does not enable `-ffast-math`. `CC`, `CPPFLAGS`, `CFLAGS`, `LANGFLAGS`,
`WARNFLAGS`, `LDFLAGS` and `XGL_LIBS` can be overridden on the command line.

Place your shareware `pak0.pak` in `id1/` under the working
directory used to start the game. Game data is not included in this port.
The shareware episode is the initial compatibility baseline. Solaris audio
uses `/dev/audio`, 11,025 Hz 16-bit stereo, and the original mono fallback.
The XGL-specific `xgl_snd_sun.c` retains those device interfaces with a larger
mixing buffer and corrected nonblocking submission. CD audio remains the
existing null driver. System services and BSD/UDP networking use the
original Solaris source files and libraries.

Start the shareware map or its recorded benchmark demo from the directory
containing `id1/pak0.pak`:

```sh
./xglquake -winsize 320 240 +map e1m1
./xglquake -winsize 320 240 +timedemo demo1
```

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

Texture detail and sprite cutouts are geometric approximations. This first
target does not implement the OpenGL renderer's optional mirrors, projected
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

Opaque, non-animated world faces can also use contour-aware detail. A small
texture-space sample grid finds strong color boundaries, and convex polygon
cuts follow those boundaries where they reduce color error. Straight borders
become polygon edges; curved outlines become several short edges. Each region
keeps its own material color across duplicated boundary vertices, while baked
lightstyles and native lighting still shade the geometry in one pass. All new
vertices interpolate the original face plane: there is no relief or change to
collision geometry.

Contour detail replaces the face's ordinary subdivided mesh within its existing
triangle allowance and the global `xgl_detailtris` budget. It never adds a
second rendering pass. Analysis is cached per surface and attempted for at most
one eligible visible face each frame, so newly encountered faces improve
progressively. Warm frames do no contour analysis. Animated textures, alternate
animation chains, sky, water, cutouts and moving brush surfaces retain their
existing paths. Noisy or unsuitable patterns, excessive repetition, allocation
failure and insufficient budgets retain the ordinary complete mesh.

The initial bounded analysis uses a 16-by-16 grid with extra observations beside
detected boundaries, at most 64 candidate lines,
150,000 sample/line evaluations and 128 output triangles per face (or its
smaller existing allowance). It considers at most four texture periods along
each UV axis. Features finer than the sample grid can be missed. The temporary
workspace is less than 128 KiB and is released after each attempt; the result
reuses the existing world-mesh allocation within the 8 MiB cache. This first
implementation targets basic architectural shapes, not faithful reproduction
of every texel.

Static world colors are gamma-converted when material/lightstyle/gamma state
changes, then submitted directly from their strided cache. Animated models
reuse sampled skin colors across poses while retaining current transforms,
lighting, skins and player colors. A bounded cache also reuses indexed
subdivision operations and model-local normals. Current pose positions are
still read and transformed each frame; cache exhaustion keeps the original
recursive drawing path. Both paths assemble larger vertex blocks before
calling the backend, and a gamma lookup table avoids repeated conversion
division for each color channel.

Consecutive opaque UI rectangles are batched without changing their drawing
order. Native text runs combine only when measured font advances and spacing
preserve the existing fixed-cell glyph positions. Incompatible metrics,
spaces and clipped cells split runs, retaining individual-glyph drawing as
needed. This does not assume that Roman_M is monospaced. Completion waits are
issued only when native work is pending.

| Setting | Default | Effect |
| --- | ---: | --- |
| `xgl_subdivide` | 32 | World texture-coordinate subdivision step, clamped to 8–256; smaller adds detail. |
| `xgl_detailtris` | 4096 | Detailed world/brush triangle allowance; visible world faces are ranked by projected-size estimate, with complete coarse faces when exhausted. |
| `xgl_detaildistance` | 768 | Distance within which detailed world surfaces are considered. |
| `xgl_contours` | 1 | Fit opaque static world detail to strong texture contours; 0 uses the previous subdivision. Changing it rebuilds the world cache. |
| `xgl_alias_subdivide` | 1 | Alias subdivision depth, 0–2; distant models use less detail. |
| `xgl_entity_triangles` | 2048 | Additional alias subdivision triangles per frame; original model triangles remain. |
| `xgl_sprite_cells` | 8 | Sample grid per sprite dimension, clamped to 2–16. |
| `xgl_playercolors` | 1 | Apply original player shirt/pants palette translations to sampled skins. |
| `xgl_ambient` | 1 | Native ambient contribution, clamped to 0–1. |
| `xgl_maxlights` | 4 | Maximum native point lights, clamped to 0–4. |
| `xgl_profile` | 0 | 1 reports graphics timings and native call counts; 2 adds geometry preparation and cache statistics. Output goes to the terminal once per second. |
| `_snd_mixahead` | 0.3 | Audio lookahead in seconds for XGL; existing archived values override this default. |
| `r_wateralpha` | 1 | Water opacity; values below 1 request native blending. |

The original `r_dynamic`, `r_fullbright`, `r_novis`, `r_drawentities`,
`r_drawviewmodel`, `viewsize`, `fov`, `gamma` and `r_speeds` controls are also
available. The initial settings are bounded starting points, not measured
Sun ZX performance recommendations.

The 3D context uses the benchmark's automatic full-raster VDC mapping.
Quake's view rectangle is selected through the VDC window and view clipping,
without explicitly setting `XGL_CTX_DC_VIEWPORT`. This preserves `viewsize`
and the space reserved for the status bar. Native UI uses device coordinates.

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

## Audio and hardware profiling

The original Solaris submission path writes at most 1,024 bytes per update,
only about 23 ms of stereo audio. The XGL path submits all available mixed
sound, retains unwritten data on partial writes/backpressure, and services
the mixer during rendering and cache preparation. Its default 0.3-second
lookahead trades some sound latency for tolerance of long frames. A single
blocking graphics call longer than the queued audio can still cause a gap;
this is not an independent audio thread.

An existing `config.cfg` may restore `_snd_mixahead 0.1`. Explicitly select
0.3 and use the same resolution, demo and detail settings for each comparison.
First measure whole-game performance without profiling:

```sh
./xglquake -winsize 640 480 +_snd_mixahead 0.3 +xgl_profile 0 +timedemo demo1 > xglquake-timedemo.txt 2>&1
```

Quit after the timedemo finishes, then repeat with detailed profiling:

```sh
./xglquake -winsize 640 480 +_snd_mixahead 0.3 +xgl_profile 2 +timedemo demo1 > xglquake-profile.txt 2>&1
```

Keep the console closed during playback and quit after the timedemo result
appears. The console stays visible after playback and has a different workload;
its later profiles must not be averaged into gameplay results. Compare the
timedemo FPS from both runs to measure profiling overhead. A first frame that
builds caches should also be distinguished from sustained rendering. Repeat
the same procedure at another resolution only when testing resolution cost.

The profiler
prints graphics-frame mean/max time and world, entity, effects, weapon and
UI stage times. Native submission, synchronization, buffer-switch and audio
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
color/output assembly. Packing excludes native submission. Native triangle
batches can span stage boundaries, so the stage that flushes a batch can
include submission work queued by an earlier stage.

Detailed counts are per-frame averages. World cached draws include meshes
built during that frame, while fallback draws count complete streamed coarse
faces; they are not a cache-hit percentage. Color reuse/updates count mesh
update calls, which can occur multiple times for a streamed face. Alias color
and normal/topology (reported as pose) cache hits, misses, evictions and fallback
draws expose cache pressure without changing cache limits. Compare native
text calls with emitted glyphs to see how much the installed font permits
batching.

Level 2 also reports contour analysis time and per-frame attempts, successful
builds, texture samples and candidate evaluations. Contour time overlaps world
detail selection. Compare `xgl_contours 0` and `1` with the same demo, resolution
and triangle settings, using profiling off for FPS. The first traversal includes
progressive analysis; revisit the same scene to distinguish preparation from
cached drawing. Host synthetic-image tests establish geometry coverage and
shape approximation, not SPARC preparation time or Sun ZX rendering speed.

## Development-host checks

Shareware recognition, CPU texture/mesh, native UI/entity submission, audio
queueing and profiling tests need no SDK or game data. They run against
controlled engine/backend mocks:

```sh
make -f Makefile.XGL check
```

The contour suite includes straight panels, diagonal borders, brick joints,
circles and arches, compared with the existing 32-triangle subdivision and
filtered vertex-color sampling. It checks image error, coverage, winding,
planarity, repeating/transformed UVs, work and triangle limits, and fallbacks.
An optional dump produces source, baseline and contour material images:

```sh
mkdir -p /tmp/xgl-contour-preview
./build/xgl-tests/test_contour --dump /tmp/xgl-contour-preview
python3 xgl_tests/contour_preview.py /tmp/xgl-contour-preview /tmp/xgl-contour-preview/comparison.svg
```

Those previews are unlit CPU comparisons, not captures from XGL hardware.

These tests require a modern host compiler and GNU-compatible linker. They
are separate from the native Solaris build. For complete engine compilation
and a host link with native API placeholders, run:

```sh
make -f Makefile.XGL host-check XGLHOME=/path/to/xgl-sdk
make -f Makefile.XGL native-contract-check XGLHOME=/path/to/xgl-sdk
```

This optional check requires Python 3, a modern host C compiler, and the
actual Sun XGL SDK headers supplied separately. Host-only handwritten X11
and Solaris contracts provide declarations needed where native development
headers are unavailable. They are not substitutes for native libraries.
To use real X11 development headers instead, add
`HOST_X11_INCLUDE=/path/to/include` to the host-check command.
The generated executable is linked only, never executed; native API
placeholders abort if called. This catches unresolved engine symbols and
accidental OpenGL dependencies. The check also compares modified existing
shared engine files against `HEAD` under software and `GLQUAKE` preprocessing to
verify that their existing paths remain unchanged. Temporary artifacts and
the compiler log are reported by the script.

The optional native contract test captures SDK calls to check projection,
clipping, lighting selection/state changes, depth-clear state, gamma and
native font placement/cache behavior. It also checks view placement after
resizing, strided/prepared vertex colors, UI batch ordering and lifetimes,
completion waits, and error diagnostics with missing library message fields.
It does not rasterize those calls.

These checks do not establish Solaris linking, SPARC ABI compatibility,
native font rendering, Sun ZX correctness or performance. Those require a
native build and an actual game run on the target machine. No game assets
are bundled with the host checks. Shareware map startup, gameplay, audio,
font appearance, native lighting and transparency still need testing on
the Sun. No target-machine frame-rate result is claimed for this initial
port.
