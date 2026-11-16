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
uses the existing `/dev/audio` driver; CD audio remains the existing null
driver. System services and BSD/UDP networking also use the original
Solaris source files and libraries; no alternate audio or networking driver
is selected for XGL.

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

| Setting | Default | Effect |
| --- | ---: | --- |
| `xgl_subdivide` | 32 | World texture-coordinate subdivision step, clamped to 8–256; smaller adds detail. |
| `xgl_detailtris` | 4096 | Per-frame detailed world/brush triangle allowance; complete coarse faces remain when exhausted. |
| `xgl_detaildistance` | 768 | Distance within which detailed world surfaces are considered. |
| `xgl_alias_subdivide` | 1 | Alias subdivision depth, 0–2; distant models use less detail. |
| `xgl_entity_triangles` | 2048 | Additional alias subdivision triangles per frame; original model triangles remain. |
| `xgl_sprite_cells` | 8 | Sample grid per sprite dimension, clamped to 2–16. |
| `xgl_playercolors` | 1 | Apply original player shirt/pants palette translations to sampled skins. |
| `xgl_ambient` | 1 | Native ambient contribution, clamped to 0–1. |
| `xgl_maxlights` | 4 | Maximum native point lights, clamped to 0–4. |
| `r_wateralpha` | 1 | Water opacity; values below 1 request native blending. |

The original `r_dynamic`, `r_fullbright`, `r_novis`, `r_drawentities`,
`r_drawviewmodel`, `viewsize`, `fov`, `gamma` and `r_speeds` controls are also
available. The initial settings are bounded starting points, not measured
Sun ZX performance recommendations.

The existing Solaris main allocates a 16 MiB engine heap. World geometry
cache storage is capped at another 8 MiB and CPU material pixels at another
8 MiB; native contexts, window buffers, program data and library allocations
are additional. These limits are not a measurement of total process memory.
World cache/detail exhaustion falls back to complete coarse faces; exceeding
the CPU material limit reports an error rather than silently dropping assets.

## Development-host checks

Shareware recognition, CPU texture/mesh and native UI/entity submission tests
need no SDK or game data. They run against controlled engine/backend mocks:

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
native font placement/cache behavior. It does not rasterize those calls.

These checks do not establish Solaris linking, SPARC ABI compatibility,
native font rendering, Sun ZX correctness or performance. Those require a
native build and an actual game run on the target machine. No game assets
are bundled with the host checks. Shareware map startup, gameplay, audio,
font appearance, native lighting and transparency still need testing on
the Sun. No target-machine frame-rate result is claimed for this initial
port.
