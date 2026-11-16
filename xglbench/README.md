# xglbench

`xglbench` is a standalone Sun XGL benchmark for choosing the rendering path
for XGLQuake. It compares CPU-sampled material colors with optional native
XGL texture mapping, using procedural geometry and a generated RGB image.
It needs no Quake executable, PAK files or OpenGL library. Its current scope
is the future `WinQuake`-based `xglquake` renderer.

The intended measurement machine is Solaris 2.6 on a SPARCstation 10 with a
Sun ZX/Leo board and the corresponding XGL runtime/device pipeline. The
benchmark does not assume that every XGL operation is hardware accelerated.
No measured ZX results from this program are supplied with the source. The
historical rates discussed below are reference points from the Leo whitepaper.

The target workstation has a 60 MHz processor, 64 MB RAM, and Sun ZX/Leo
24-bit 3D graphics. Every case uses **one application thread**; there are no
SMP tests or worker threads. Start with the default 512 base triangles,
320x240, and a small queue. No allocation is sized from the machine's RAM.
The initial goal is 15 FPS, a 66.667 ms whole-engine frame budget. Full-frame
results report their share of that budget; gameplay, visibility, audio and
other engine work still require time.

## Build on Solaris

Use GNU make and an ANSI C compiler, with the XGL and OpenWindows development
headers and libraries installed:

```sh
cd xglbench
gmake
```

The default paths follow the XGL Programmer's Guide:

```sh
gmake XGLHOME=/opt/SUNWits/Graphics-sw/xgl OPENWINHOME=/usr/openwin
```

`CC`, `CFLAGS`, `CPPFLAGS`, `LANGFLAGS`, `WARNFLAGS`, `LDFLAGS`, `XGL_LIBS`,
and `XGL_RPATH` can be overridden. For example, a compiler that does not
accept GCC's language/warning options can use:

```sh
gmake CC=cc LANGFLAGS= WARNFLAGS= CFLAGS=-O
```

The build follows the native 32-bit SPARC toolchain. It does not request
UltraSPARC instructions, 64-bit code, fast-math, or architecture-specific
assembly. There are no dependencies on engine objects or changes to the
engine makefiles. Runtime library paths are embedded with Solaris `-R`;
the XGL device pipeline must also be installed for the selected framebuffer.
`LANGFLAGS` defaults to empty so the compiler's normal language mode can
expose the installed Solaris/XGL interfaces. `__EXTENSIONS__` remains enabled
for Solaris interfaces such as `gethrtime()`. The source remains compatible
with C90; no strict-language flag is required.

`gmake sdk-check` checks the configured header locations. Compilation and
linking against the real installed SDK remain the authoritative API check.

Use the XGL installation supplied for the target Solaris system; do not
assume a newer SDK or a particular release from the OS name alone. The
benchmark records the runtime version and device capabilities. The benchmark
retains ANSI C and the native 32-bit build. Native mapping uses the Texture
Map interface introduced in XGL 3.1; XGL 3.0 SDK compatibility is not claimed.
The existing lighting backend also uses 3.1 attributes. Other device pipelines may skip
unsupported or incorrect paths. Actual cross-version compatibility still
requires builds and readback checks against those installed SDKs/runtimes.

## Run

Version 0.7.1 defaults to the **quick** profile at **320x240**, with a one-minute
elapsed-time limit:

```sh
./xglbench >zx-results.txt 2>&1
./xglbench -medium -csv zx-medium.csv >zx-medium.txt 2>&1
./xglbench -long -csv zx-long.csv >zx-long.txt 2>&1
./xglbench -extralong -csv zx-full.csv >zx-full.txt 2>&1
```

The profiles prioritize comparisons, use short measurement intervals, and rest
the graphics device after setup and between cases. Quick, medium and long
allow at most one, three and seven minutes including rest, with headroom for
setup and readback validation. Every profile has a finite elapsed-time limit
and a further ten-second grace period for stopping
and cleanup. Extra-long selects the complete case registry with a 45-minute
limit.

| Profile | Cases per resolution | Resolutions | Seconds/trial | Repeats | Warm-up/case | Minimum idle pause | Elapsed-time limit |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `-quick` (default) | 16 | 320x240 | 0.2 | 1 | 0.02 s | 1.5 s | 60 s |
| `-medium` | 34 | 320x240 | 0.25 | 2 | 0.03 s | 2.5 s | 180 s |
| `-long` | 34 | 320x240, 640x480 | 0.4 | 2 | 0.05 s | 3.0 s | 420 s |
| `-extralong` | 217 | 320x240, 640x480 | 0.5 | 3 | 0.1 s | 2.0 s | 2700 s (45 min) |

Quick prioritizes sampled flat/smooth draw colors at depth zero, four
primitive submission methods, sampled flat/smooth complete frames at depth
zero, matched CPU/XGL flat/smooth point-light draws at depth one, cached baked
flat/smooth lighting at depth one, and sampled flat/smooth colors at depth one.
Medium adds sampled complete frames at depth one, four-layer lightstyle
updates, CPU/XGL lit frames, mixed-winding culling, color/depth clears,
depth-two sampled colors, and serialized/streaming cached/relight pairs.
Long runs the same 34 cases at both resolutions with longer trials.
Native texture mapping is optional: use an explicit `-test` filter or
`-extralong`, which retains the complete 217-case registry. The later sections
describe all available families; a short profile need not include every case
described there.

Keep both standard output and standard error when collecting a hardware run,
as in the examples above. XGL error details and setup-stage diagnostics are
written to standard error. After rebuilding, first check HUD/raster recovery
and a small complete-frame group before repeating the long profile:

```sh
./xglbench -test hud/ -winsize 320 240 -csv zx-raster.csv >zx-raster.txt 2>&1
./xglbench -test texture-map/facet-flat/frame/depth-0 -winsize 320 240 >zx-frame.txt 2>&1
```

RGB memory rasters request `XGL_DEV_COLOR_TYPE` before depth and dimensions.
The ZX runs showed that setting it after storage attributes triggered a
write-once error, while omitting it did not select RGB. The revised order
still requires verification on the ZX. Allocation metadata and actual
opaque/masked pixel readback must pass before HUD/full-frame timings are
accepted; a 32-bit allocation by itself does not establish the color format.

The elapsed-time limits include opening XGL, setup, validation, measurements
and idle pauses. At the limit, the program stops starting new work and discards
any incomplete current case. During an orderly stop, that case is marked
skipped with failure kind `time-limit`, and the terminal summary reports how
many selected cases were not run. Each unattempted case has a `not-run` CSV
summary row with empty timing fields. Keep the terminal report with the CSV
for device metadata and error details.

A POSIX `SIGALRM` timer runs in the same process, without another application
thread or SMP work. If a blocked draw or cleanup still prevents termination
after the additional ten-second grace period, its signal handler calls
`_exit(124)`. This forced exit bypasses normal XGL cleanup and buffered output
flushing. Completed cases are flushed as they are reported, but the last CSV
record or terminal summary can be incomplete after a forced exit. This timer
depends on OS signal delivery; it cannot recover a kernel/driver hang that
prevents delivery or guarantee that graphics hardware stops immediately.

After setup and between cases, the scheduled idle period is the greater of
the configured minimum pause and **twice the preceding active elapsed time**.
Case activity includes configuration, preview, warm-up and measured trials.
These pauses follow device completion and stay outside all reported rendering
and CPU intervals. Events are still polled during pauses, so Escape or closing
the window can stop the run. `-pause 0` removes only the fixed minimum; the
two-to-one idle-to-active rule remains in effect.

Setup, readback validation and minimum work counts vary with the machine, so
profiles may finish early or reach their limits before attempting every
selected case. Extra-long's default workload and rest periods take roughly
35 minutes before setup and validation; its 45-minute limit can leave cases
unattempted on slower systems. These measures reduce continuous benchmark
activity, but do not measure temperature or guarantee protection for aging
graphics hardware.

The Leo paper describes 11 custom ASICs plus memory in a compact two-board
assembly measuring 5.7 x 6.7 x 0.6 inches, fitting a standard 2S SBus slot
(printed pages 101 and 107, section 9). Its documented 1280x1024, 76 Hz video
path continues scanning out during application idle (pages 102, 105 and 107,
section 7). A 320x240 benchmark window changes neither the board's display
mode nor its power state. Pauses stop benchmark commands; two-to-one rest does
not imply two-thirds less power or heat. Airflow around this compact assembly
matters, but the paper supplies no fan layout, required airflow, temperature
thresholds, or temperature-sensor/shutdown guarantees. Cooling decisions need
the actual workstation's service information and condition; this benchmark
cannot infer a safe thermal cutoff.

An explicit `-winsize` runs only that resolution, without changing the profile
timings. Multi-resolution profiles resize the same
window between suites and keep each resolution's results separate. Resize and
warm-up work are outside the measured rendering intervals.

```sh
./xglbench -winsize 800 600
./xglbench -list
./xglbench -extralong -list
./xglbench -test submit/multi_polygon/batch-128 -winsize 320 240
./xglbench -extralong -test pipeline/ -queue 2 -csv pipeline.csv
./xglbench -quick -test lighting/ -seconds .2 -warmup .02 -pause 0 -csv lighting.csv
```

`-test TEXT` selects matching names from the **full 217-case registry**, even
when they are outside the profile's usual subset. The chosen profile still
supplies resolutions, timings, pauses and any elapsed-time limit.
`-list` shows the selected profile or filter without opening XGL;
use `-extralong -list` to see the entire registry.

Explicit `-seconds`, `-warmup`, `-repeats` or `-pause` values override that
setting. This is independent of
argument order: `-seconds .2 -quick` and `-quick -seconds .2` have the same
effect. Unspecified settings still come from the selected profile.
Timing overrides retain the selected profile's elapsed-time limit,
ten-second grace period and adaptive idle rule. There is no unlimited mode.
Choose extra-long when a filtered comparison needs more time, up to its
45-minute limit.

Options:

| Option | Meaning |
| --- | --- |
| `-quick`, `-medium`, `-long`, `-extralong` | Choose a profile; quick is the default. |
| `-winsize W H` | Run one requested drawable size; width 128 through 4096, height 64 through 4096. |
| `-seconds N` | Override the minimum measured duration per trial. |
| `-warmup N` | Override unmeasured warm-up duration. |
| `-repeats N` | Override trials per case, range 1 through 31. |
| `-pause N` | Override the minimum idle pause in seconds; zero retains the adaptive two-to-one rest period. |
| `-triangles N` | Positive even base triangle count, at most 16384; default 512. |
| `-queue N` | Maximum outstanding work items in streaming cases, 1 through 8; default 2. Serialized controls use one slot. |
| `-test TEXT` | Select matching names from the full registry, retaining the profile's other settings. |
| `-csv PATH` | Save machine-readable results as well as the terminal report. |
| `-display DISPLAY` | Select an X display. Local DGA is the intended path. |
| `-allow-fallback` | Permit a software/non-DGA configuration for diagnosis. |
| `-list` | List selected case names without opening a display. |
| `-help` | Print usage without opening a display. |

The graphics window shows test geometry; the launching terminal receives the
report. Progress goes to standard error. CSV output is optional and its name
is supplied by the caller. Keep the window fully visible and unobscured, and
avoid moving or resizing it during a trial. Run on the workstation's local
OpenWindows display rather than through SSH X forwarding. Background work
and window-system activity can materially affect a 60 MHz machine. There is
no randomized test order, so rerun close comparisons and control changes in
background load. The quick profile's single trial is an initial comparison,
not an assessment of run-to-run variation.

## Questions the suite addresses

* **Which primitive submission API is best?** Compare individual polygons,
  batched simple polygons, restart triangle lists, and triangle strips using
  matching positions and facet colors. Batch-size sweeps expose call overhead.
* **Does strip encoding help?** Connected strip workloads are distinct from
  disconnected patch workloads. LeoCommand expands strips into independent
  triangles before LeoFloat processing, so compact input can save bandwidth
  and submission work without implying downstream shared-vertex transform
  reuse. The compared methods still describe matching triangles and colors.
* **Does retaining native arrays help?** A static-mesh case compares XGL's
  retained geometry-data lifetime contract with volatile arrays using the
  same geometry and batch size. It is separate from XGL display-list caches.
* **Is the bottleneck geometry or pixels?** Compare triangle sizes, overlap,
  Z buffering, near-plane clipping, and the two resolutions. Overlap tests
  include both near-first and far-first submission to expose depth rejection
  effects. A fourfold increase in pixel area
  with little elapsed-time change suggests a geometry/CPU limit; it does not
  identify the exact limiting device stage.
* **Does triangle shape matter at a controlled area?** Calibrated cases use
  projected areas of 4, 16, 64, 100, 256, and 1024 square pixels. Equal-area
  100-pixel cases also compare width/height ratios of 1, 16, and 1/16. Leo's
  five horizontally interleaved drawing banks make thin triangles worth
  checking separately from total triangle count.
* **Does posting work without an immediate completion wait help?** Paired
  serialized and streaming cases use the same cached or relit meshes at
  subdivision depths 0, 1, and 2. A bounded queue allows the single CPU thread
  to prepare the next item while previously posted XGL work can execute.
* **How expensive is subdivision?** Depths 0, 1, and 2 produce 1, 4, and 16
  triangles per base triangle. Cached and regenerated geometry are separate
  cases. Subdivision colors come from a procedural material sampled on the
  CPU, not from an XGL texture object.
* **What changes with animation?** Static meshes, changing model transforms,
  and changing facet colors exercise different preparation and submission
  costs.
* **Can XGL calculate lighting more cheaply than the CPU?** Separate CPU and
  XGL lighting paths use the same Lambert model, with directional and moving
  positional lights. A separate Quake-like model preserves a finite light
  radius and surface-distance falloff instead of treating XGL lighting as an
  exact replacement for Quake's world lighting.
* **Does smooth shading help?** CPU-supplied vertex RGB with XGL interpolation
  is distinct from XGL-calculated vertex lighting. Both are compared with
  flat facets. Original lighting cases keep material constant within each
  triangle. Separate vertex-material cases sample the CPU image at each
  corner, so interpolation can approximate texture variation as well.
* **What does baked lighting cost?** Cached procedural lighting samples are
  separate from recombining animated lightstyle layers. This tests the work
  needed to update supplied colors without reconstructing the baked field
  from XGL light sources.
* **How much frame overhead remains?** Color clears, depth clears, swaps,
  and full-frame workloads separate fixed work from primitive throughput.
  Full-frame cases include a masked HUD copy as well as the scene and swap.
* **Can a practical HUD be cheap?** Opaque and one-bit-masked raster-copy
  cases exercise 2D images without creating a triangle per image pixel.
* **Is actual XGL texture mapping usable?** Optional point, bilinear and
  trilinear texture modes use the same CPU-generated image as matched
  flat-facet and smooth-corner controls. Draw-only, complete-frame and
  tilted perspective-motion cases separate mapping cost from presentation.
* **Does hardware backface rejection help?** Paired `cull/*` cases submit
  identical all-front or mixed-winding independent triangles with culling
  off or backface culling on. These measure device rejection, not CPU-side
  visibility or BSP traversal.

The generated scenes deliberately include conditions that are expensive or
unrepresentative of an ordinary frame. A large-overlap test is a stress test,
not an estimate of average Quake overdraw. No test result is a prediction of
whole-engine frame rate.

## Texture sampling and combined workloads

The added cases keep the original measurements available for comparison:

| Cases | What changes |
| --- | --- |
| `texture/{facet-flat,vertex-smooth}/depth-{0,1,2}` | Compare one filtered material color per triangle with independently sampled corner colors, with XGL lighting disabled. |
| `frame/lighting/{cpu-flat,cpu-smooth,xgl-flat,xgl-smooth}/point-1/depth-{0,1,2}` | Moving Lambert light, color/Z clear, geometry, masked HUD, buffer swap and completion. |
| `frame/lighting/{cpu-flat,cpu-smooth}/quake-point-1/depth-{0,1,2}` | The same complete-frame path with CPU baked-plus-dynamic Quake-like lighting. |
| `pipeline/{serialized,streaming}/quake-{cpu-flat,cpu-smooth}/depth-{0,1,2}` | Bounded one-thread CPU/device overlap with actual CPU light evaluation and native color updates; no swaps. |
| `lighting/submit/{polygon,multi_polygon}/{cpu-flat,cpu-smooth,xgl-flat,xgl-smooth}/point-1/depth-1` | Matched lit geometry/materials using individual calls or batches of 128 polygons. |
| `cull/{all-front,mixed}/{off,back}` | Device backface rejection; mixed cases reverse every other independent triangle. |

Smooth variants in these new combined workloads use vertex-sampled material
RGB. Their flat variants use the existing facet samples. Compare matching
CPU/XGL smooth cases or matching serialized/streaming cases to isolate the
compute or scheduling choice. Flat versus smooth also changes sample count,
material interpolation and payload; it is not a pure lighting-offload comparison.

Vertex samples use unwrapped UV coordinates and the same CPU image/filter as
facet samples. RGB materials are cached before timing; dynamic illumination
is always applied to those immutable colors, never to the previous lit result.
Independent corners keep distinct UVs at material boundaries. Sampling/filter
detail is tied to the subdivision level here, not a perspective-aware pixel
footprint. These fixtures measure costs and show approximations; they do not
choose an adaptive LOD policy or eliminate the need to inspect texture seams,
shadow edges and motion on actual Quake geometry.

The culling probe verifies that front and back winding behave as intended
before accepting a case. With mixed winding, backface culling intentionally
changes the visible image. Rates still count **submitted** triangles, including
those rejected; do not interpret the improvement as faster rasterization of
the same number of visible triangles. The all-front pair measures overhead
when there is nothing to reject. CPU culling and strip culling are not tested.

For an initial run on a smaller machine, select one family at a time:

```sh
./xglbench -test texture/ -winsize 320 240 -quick
./xglbench -test frame/lighting/ -winsize 320 240 -triangles 128 -csv frames.csv
./xglbench -test quake-cpu-smooth -winsize 320 240 -queue 1
./xglbench -test quake-cpu-smooth -winsize 320 240 -queue 2
./xglbench -test cull/ -winsize 320 240
```

The queue is immutable application geometry storage awaiting device completion,
not a thread count. Queue one is the serialized reference. No test uses the
second host processor explicitly. Real SDK and on-device readback validation
remain required before treating any timings as hardware evidence.

## Native XGL texture mapping

`texture-map/*` adds 35 optional cases. It does not change the original
sampled-color cases or make texture mapping a requirement for their execution.

| Cases | Workload |
| --- | --- |
| `texture-map/{facet-flat,vertex-smooth,point,bilinear,trilinear}/draw/depth-{0,1,2}` | Matching geometry and source image; draw completion without timed presentation. |
| `texture-map/{facet-flat,vertex-smooth,point,bilinear,trilinear}/frame/depth-{0,1,2}` | The same modes with color/Z clear, masked HUD, swap and completion. |
| `texture-map/{facet-flat,vertex-smooth,point,bilinear,trilinear}/perspective-motion/depth-1` | A rotating, tilted plane whose depth varies within triangles, exercising perspective mapping. |

The `facet-flat` and `vertex-smooth` controls approximate the texture with
cached CPU colors. The three native modes supply explicit unwrapped UVs to
XGL, which samples a 64x64 RGB mipmap through a Texture Map object. The image
is the same procedural brick/checker material used by the CPU sampler. All
five modes omit the older fixtures' synthetic per-patch brightness gain, so
they compare raw material colors with no dynamic or baked lighting. They
use the same geometry, depths, transforms and buffer operations. Their images
are intentionally different approximations, not a claim of identical filtering.

Native mapping replaces final RGB. Point and bilinear modes compare nearest
and bilinear sampling; trilinear filtering also blends adjacent mip levels
during minification. Magnification uses point sampling for point mode and
bilinear sampling for the other two. Coordinates repeat in both directions;
the tests include negative and seam-crossing UVs. Texture creation, upload,
mipmap generation, native array allocation and readback validation occur
before timing. Completed textured drawing, state changes needed during a
frame, and presentation where requested are measured. Texture allocation and
upload performance are not measured by these cases.

**Do not infer texture acceleration from the device name or DGA support.**
The documented device inquiry does not expose a texture-acceleration field.
XGL may use its software path for texture mapping even when other operations
are accelerated. That is an intended comparison and does not require
`-allow-fallback`; the existing requirements for the base display/geometry
path still apply. Results label texture execution `unclassified`. They show
the cost of the installed XGL pipeline, not proof of hardware texturing.

Unsupported mapping, failed texture readback, or a rendering error leaves
the requested case skipped. It never becomes an apparently successful
untextured fallback. Texture state is disabled for ordinary cases and HUD
copies; a failure to reset that state can also skip subsequent cases, and
fatal native errors still stop the run. The texture readback gates require
a drawable at least 128x128 pixels. Native texturing is currently
limited to unlit, non-streaming multi-polygon draws and complete frames.
Combining native texturing with baked/dynamic illumination, strip submissions
or retained streaming is a subsequent experiment.

```sh
./xglbench -test texture-map/ -winsize 320 240 -quick
./xglbench -test texture-map/point/ -winsize 320 240 -csv texture-point.csv
./xglbench -test texture-map/trilinear/frame/ -winsize 320 240 -triangles 128
```

Start with a short run on slower hardware, then use longer trials for useful
comparisons. Texture-mapped low-subdivision geometry might outperform a much
denser sampled-color approximation; it might also be substantially slower.
Both outcomes are useful. Compare the visible result and completed frame time
at a suitable detail level instead of ranking by triangle rate alone.

## Calibrated geometry and the whitepaper

The fixed-area cases specify geometric area in screen pixels, before edge
coverage and rasterization rules. Their requested area is held constant at
both window sizes; it does not grow fourfold at 640x480. The report includes
the area range computed from the generated coordinates under the reference
orthographic mapping. This mapping is the one used by calibrated cases, so
their reported area matches the requested area within floating-point
precision. Triangles remain inside the viewport. If a requested shape cannot
fit a forced window size, the case is skipped rather than silently shrinking
the triangles.
The square-aspect cases are named `area/{4,16,64,100,256,1024}px`;
`shape/100px/horizontal-16x1` and `shape/100px/vertical-1x16` compare the
elongated shapes with `area/100px`.
The generator retains the requested triangle count, so controlled overlap
may be needed when the triangles cannot all fit without overlap. The
`area/` and `shape/` triangles use a constant depth and the less-than-or-equal
Z comparison, so later triangles at that depth can pass even where they
overlap. These are geometry/raster stress tests, not a promise of scene-like
spatial coverage.

The original patch scenes retain their normalized dimensions instead. Their
nominal reference area grows approximately fourfold when the window changes
from 320x240 to 640x480. For the default 512-triangle normal-size scene, nominal
triangle areas are about 88, 22, and 5.5 pixels at subdivision depths 0, 1,
and 2 in the smaller window. The larger window gives approximately 351, 88,
and 22 pixels. The fixed 128x64 HUD raster also keeps its pixel dimensions
at both window sizes. Compare resolution effects within the appropriate
workload family.

Michael F. Deering and Scott R. Nelson's *Leo: A System for Cost Effective
3D Shaded Graphics* provides the architectural context:

* Printed pages 103-104 describe DMA from application memory, hardware input
  conversion, and expansion of strip input into isolated triangles before
  distribution to the four LeoFloat processors. A retained-array contract
  or a buffer-post operation does not itself prove that a particular XGL
  driver selected DMA.
* Pages 105-106 describe five horizontally interleaved LeoDraw units, each
  with edge walking and span interpolation. The size and shape tests examine
  behavior related to that organization without claiming to measure an
  individual internal unit.
* Page 107 reports 210,000 isolated 100-pixel triangles per second with
  lighting, Gouraud shading, Z buffering and depth cueing enabled. The paper
  says that workload is VRAM limited and that smaller triangles render
  faster. It also reports a special window/color/Z clear under 200
  microseconds and 24-bit image uploads at 10 million pixels per second.

Those are historical system results under the paper's conditions, not
pass/fail thresholds or expected Quake frame rates. The lighting cases below
do not reproduce the paper's complete measurement setup. In particular, its
reported rate used one directional diffuse light plus ambient light; it does
not establish the cost of several moving positional lights. Subdivision
increases setup and input work while approximately preserving the covered
area; dividing the published triangle rate by a new triangle count is not a
reliable FPS model.
The original flat and smooth lighting experiments keep material sampling on
the CPU. Optional native mapping is isolated in `texture-map/*`; every case
continues to use one application thread.

## Lighting and smooth shading

The original `colors/changing-light`, `subdivide/relight`, `frame/relight`,
and `pipeline/*/relight` cases retain their inexpensive synthetic brightness
ramp. They measure color preparation and submission, without light positions,
normals or distance calculations. The `lighting/` family adds those costs
explicitly rather than changing the meaning of the earlier cases.

Four rendering paths distinguish interpolation from illumination:

| Path | Color calculation | XGL rendering |
| --- | --- | --- |
| `cpu-flat` | CPU evaluates one color per facet. | `XGL_ILLUM_NONE`, one supplied RGB value per facet. |
| `cpu-smooth` | CPU evaluates RGB at each vertex. | `XGL_ILLUM_NONE_INTERP_COLOR` interpolates supplied vertex colors. |
| `xgl-flat` | XGL evaluates lighting once per facet. | `XGL_ILLUM_PER_FACET`, using supplied material colors and facet normals. |
| `xgl-smooth` | XGL evaluates lighting at each vertex. | `XGL_ILLUM_PER_VERTEX` interpolates the resulting colors. |

The family names identify the comparison being run:

| Prefix | Workload |
| --- | --- |
| `lighting/baked/{cpu-flat,cpu-smooth}/cached/depth-{0,1,2}` | Cached procedural baked lighting. |
| `lighting/baked/{cpu-flat,cpu-smooth}/styles-{1,4}/depth-{0,1,2}` | Recombine changing lightstyle samples. |
| `lighting/matched/{path}/directional/depth-{0,1,2}` | Moving directional light with ambient illumination. |
| `lighting/matched/{path}/point-1/depth-{0,1,2}` | One moving positional light with ambient illumination. |
| `lighting/matched/{path}/point-{2,4}/depth-1` | Two or four moving positional lights with ambient illumination. |
| `lighting/static/{path}/directional/depth-1` | Fixed directional light; CPU colors and native light state are prepared before timing. |
| `lighting/area-100/{path}/point-1` | One moving positional light with fixed 100-pixel projected triangle areas at both resolutions. |
| `lighting/quake-like/{cpu-flat,cpu-smooth}/point-{1,2,4}/depth-1` | CPU baked-plus-dynamic lighting with a Quake-like finite-radius falloff. |

`{path}` means each of the four paths above. Braces describe the available
case names; they are not literal `-test` syntax. For example, use
`-test lighting/matched/xgl-smooth/point-1` or `-test lighting/baked/`.
The positional count excludes the ambient contribution. It is a benchmark
sweep, not an asserted XGL hardware limit.

The lighting scene has a shallow curved profile. Flat paths use geometric
facet normals; smooth paths use unit normals from the underlying curved
surface. In the original lighting cases each triangle retains one CPU-sampled
material RGB value, copied to all three corners before lighting. Those smooth
cases interpolate lighting only. The new vertex-material variants described
above also interpolate texture samples. Independent corners preserve material
boundaries. Neither approach restores detail absent from its samples. The projected
triangle areas still describe geometric coverage, not a count of shaded
pixels. Baked shadow edges can also soften when vertex interpolation spans
them; extra geometry is still needed to preserve a sharp boundary.

The matched CPU and XGL cases use ambient plus diffuse Lambert lighting,
with specular reflections disabled. Positional attenuation follows XGL's
`1 / (constant + linear * distance)` model. Directional and positional
workloads are separate, and the positional cases sweep the number of active
lights. Light creation, native allocation and scene construction are outside
timing; animated cases include updates to light state or supplied colors.
XGL's automatic lighting-normal reversal is disabled so that it does not
silently change the normals used by the CPU comparison.

For flat positional lighting, the CPU uses the triangle centroid, while XGL
may evaluate lighting at a device-selected vertex. These cases compare
closely related workloads, not identical sample positions or images. The
directional pairs have no position-dependent light calculation. Smooth
CPU/XGL pairs evaluate the same light model at the same vertex positions.
The window preview shows a representative phase. These throughput cases do
not swap buffers during measurement, so the preview is not a moving-light
animation on the normal double-buffer path. Native readback probes check
interpolation and responses to changed lights outside timing. Perceived
lighting quality still needs an engine experiment, especially for large flat
triangles under moving point lights.

Baked-light cases use procedural sample layers, not BSP data or a Quake map.
The cached variant prepares lit colors before timing. Animated variants
recombine one or four lightstyle layers and retain cached results when the
style coefficients have not changed. The deterministic simulation advances
one frame per work item, with a 10 Hz style tick every six simulated frames;
it is not paced by wall-clock time. Positional light animation likewise uses
the work-item sequence. The source material colors remain
immutable; repeated updates do not multiply an already illuminated color.
This is an approximation of the work involved in sampling and updating
Quake lighting, not a complete implementation of its surface cache.

Before each animated-style trial, the colors and style cache are reset to
phase zero outside timing. A successful measured trial runs at least 24 work items
and ends on a complete six-item tick. This ensures that even short trials
exercise actual style changes, including the one-layer pattern's changes at
items 12 and 18 (zero-based sequence). On slower hardware, this minimum work
can extend a trial beyond its requested minimum duration. If the profile's
elapsed-time limit interrupts this work, the incomplete case is skipped.

The separate Quake-like dynamic-light cases combine baked illumination with
a finite-radius, surface-distance lighting contribution on the CPU. They
are intentionally identified separately from the Lambert comparison:
Quake's world-light falloff does not include the same normal-dot-light
factor or attenuation equation as XGL positional lighting. The synthetic
surface axes, floating-point units and colored light gains are not Quake's
integer lightmap grid or a claim of pixel-identical engine output. Neither these
synthetic scenes nor fixed-function XGL lights reconstruct the visibility
and shadow information stored in an actual Quake lightmap.

There is no claim of a faithful single-pass baked-plus-XGL-dynamic hybrid.
The desired composition is material color multiplied by the sum of baked
and dynamic illumination. Lighting a color that already includes baked
illumination instead modulates that baked result. The documented XGL model
has ambient, diffuse and specular components, with no separate emissive
material term to make this combination automatic. Any future hybrid that
uses extra state changes or passes must include their cost.

Compare CPU/XGL lighting using **completed `total_ms`**, which includes CPU
evaluation, color conversion/upload or native light-state updates, drawing
and completion. Comparing `render_ms` alone would omit the CPU work being
considered for offload. Lighting facets/second also uses `total_ms`. Cached
CPU lighting is a useful control: static lighting need not be recomputed
every frame just because XGL can calculate it. The reported input payload
includes the vertex colors and normals used by each path; smoother or
hardware-lit variants can require more input data.

The original `lighting/*` cases remain serialized, single-threaded tests.
The new `lighting/submit/*` pairs add individual polygons, and `frame/lighting/*`
includes presentation. CPU Quake-like lighting also has bounded streaming
cases; native XGL light-state updates are deliberately excluded from streaming.
Lit or interpolated strips and restart lists remain untested. The
device's reported lighting/shading support is a coarse capability indication,
not proof that every selected light configuration runs entirely in Leo
hardware. Unsupported or incorrect paths remain skipped results, and real
Solaris/ZX measurements are required to establish a useful rendering choice.
Successful cases can also carry a note, printed in the console and saved in
the CSV `reason` field, such as a reported software-lighting fallback when
fallback is allowed. Read that note before treating a result as an
accelerated lighting measurement.

## Bounded single-thread streaming

`pipeline/{serialized,streaming}/{cached,relight}/depth-{0,1,2}` are matched
throughput experiments using batched simple polygons. Each work item clears
color and depth, then draws the mesh. There is no HUD copy or buffer swap in
these cases, so presentation synchronization does not obscure the comparison.
They are separate from the existing `frame/*` tests, which include both HUD
and presentation.

The `quake-cpu-flat` and `quake-cpu-smooth` pipeline variants use the same
finite-radius light evaluation as the corresponding new frame cases, instead
of the original relight fixture's simple brightness ramp. Each slot owns its
facet or vertex RGB array. Native readback checks include both flat and smooth
queued-color paths, and a pending slot cannot be recolored before completion.

The serialized control completes each item before preparing another. The
streaming version posts each item with `XGL_FLUSH_BUFFERS` and drains after
at most `-queue` items. Both use preallocated native slots and the same
geometry/color workload. Preparation may overlap already posted device work;
all XGL/Xlib calls still occur on the one owner thread. Thread count or SMP
performance is not being tested.

Slot contents and descriptors remain unchanged while XGL can reference them.
A slot becomes reusable only after a successful `XGL_FLUSH_SYNCHRONIZE`
completion drain. Queue storage is distinct from the two framebuffer buffers.
Allocation, validation and warm-up happen before measurement. Trial timing
includes pipeline fill and the final completion drain, and no result is
accepted with unfinished work. An unbounded queue would measure enqueue
capacity rather than completed throughput and is intentionally not used.

The total is elapsed wall time per completed work item, including loop and
event-polling overhead. `prepare_ms`, `enqueue_ms`, and `drain_ms` describe
time spent by the host thread; the pipeline's `render_ms` field is empty.
Enqueue time is not rendering duration, and drain time is the remaining wait
after work that may already have overlapped preparation. Do not add presumed
CPU and GPU stage durations together to reconstruct frame latency. Pipeline
facets/second uses completed-item total time. These no-swap items are not
displayed frames or Quake FPS. A queue of one is a useful control; larger
queues should be interpreted together with their outstanding-work bound,
not as free throughput with no latency tradeoff.

## Reading the report

Read the configuration and validation results before comparing rates. Device
name, actual drawable size, color depth, DGA availability, acceleration
reports, and allocated buffers describe the path that was actually opened.
The presence of an XGL function in the SDK does not prove it uses Leo
hardware. Fallback results must not be presented as ZX hardware results.

Rendering timings include waiting for XGL completion at measurement
boundaries. They are elapsed host times, not hardware GPU-counter readings.
CPU preparation, completed rendering, and presentation are reported
separately where the case performs that work. Swap-inclusive time can be
limited by display synchronization; it must not be used alone to rank
primitive throughput.

For the original serialized cases, `total_ms` includes the reset clears
before drawing; ordinary draw-case `render_ms` excludes those clears.
`frame/*` includes clear, draw and masked HUD in `render_ms` and swaps in
`present_ms`. Event polling is outside these elapsed frame totals. Process
CPU time includes the trial's polling and loop overhead.
Reported mesh memory counts the portable arrays only, excluding native XGL
arrays, framebuffer storage and driver allocations. Geometry rebuilding also
includes allocation, both the portable independent-triangle and strip
representations, and native-array conversion. This is the preparation cost
of the benchmark harness, not an optimized engine mesh-generation or CPU
projection implementation. It is an intentionally costly alternative to
reusing caches.

Estimated native input bytes count the vertex/facet payload referenced by
drawing calls, including strip-boundary repetition and triangle-list facet
padding where applicable. They are not measured SBus traffic or DMA byte
counts and do not include an unknown driver's internal representation. Use
them to understand why equal triangle counts can have different input costs,
not to calculate actual bus utilization.

CSV contains `trial` records and a `summary` record per successful case.
Skipped cases contain a summary with status `skipped` and the reason; zero
fields on those records are not measurements. The appended `failure_kind`
column distinguishes `api-error`, `validation-failed`, `unsupported`,
`time-limit`, and other `error` outcomes; successful rows use `none`.
During an orderly stop, selected cases not attempted receive summary rows
with status and failure kind `not-run` and empty timing fields. Their count
also appears in the terminal summary. Forced termination after the
ten-second grace period can leave the final output record incomplete.
The `frames_all_trials` field
is the total completed-item count for the case even on individual trial
rows; for pipeline cases those items are not presented frames. The
`timing_mode` field distinguishes the timing contracts.
The `retained` flag is one for pipeline cases because timed draws retain
their native slot arrays, including when slot colors change between drains.
`queue_limit`, `max_queued`, and `drain_calls` describe the bounded streaming run.
`max_queued` is the maximum number of items submitted since a successful
drain across all measured trials; it is a software bound, not a measured
hardware queue depth. `drain_calls` is the total number of measured drains
across those trials. Like `frames_all_trials`, both fields repeat the case's
aggregate metadata on individual trial rows.

`material_sample` records `facet`, `vertices`, or native `texels`; `cull` and `winding` describe
the culling fixture. Successful `frame/*` rows also have `frame_budget_ms`
(66.667 ms) and `frame_budget_fraction` (`total_ms` divided by that budget).
Those two fields are empty for skipped and non-frame cases. A fraction below
one only means this synthetic render workload fits the reference budget;
it does not establish 15 FPS in Quake. Portable/mock results have no hardware
performance meaning even when their fractions are small.

Native texture cases use `material_sample=texels`. The additional
`texture_mapping`, `texture_filter`, `texture_execution`, `texture_width`
and `texture_height` columns identify the requested native mode and image
size. They describe the requested configuration even for a skipped case;
always check `status` before using its timing fields. A successful mapped row
uses `native-xgl`, its filter name, `unclassified`, and `64,64`. Untextured
rows use `none,off,none` and empty dimensions. Browser/WebGL performance is
not measured by this executable.

The `nominal_area_*` columns report geometric pixel area under the reference
orthographic mapping, before model transforms or clipping. They do not
measure the actual projection in perspective or animated-transform cases.
`input_payload_bytes` reports the estimated native payload per item. Keep
the text report with the CSV to retain device/build metadata and
interpretation.

Lighting rows use `timing_mode=lighting-serialized`. The appended `shading`
field distinguishes `flat` and `smooth`, while `lighting_model` identifies
`none`, `baked`, `lambert` or `quake-like`. `lighting_compute` is `none`,
`cpu` or `xgl`; smooth shading alone does not imply XGL illumination.
`light_count` excludes ambient light, and `light_motion` is one only when
the light changes with the work-item sequence. `style_layers` counts the
source layers: cached baked and Quake-like lighting use one, and animated
baked cases use one or four. `lighting_sample` is `none`, `centroid`,
`vertices` or `device-vertex`, making the positional flat-lighting sample
difference explicit. Compare rows with matching model and sample semantics
before interpreting any timing ratio as an offload benefit.

Repeated trials report a median and range. These describe the repeated-trial
measurements, not a per-frame 95th/99th-percentile latency distribution.
Run longer trials and more repeats when methods are close. Record compiler
options and the workstation configuration alongside reports when comparing
builds or machines.

Readback checks precede timing and can disqualify submission paths that
produce incorrect colors or geometry. A failed or unsupported case should
remain visible as a skipped result rather than becoming a zero-time winner.
These checks are focused regression probes, not a full XGL conformance test.

The report's recommendations are starting points for an engine experiment.
Recheck the winner using representative Quake visibility, animation,
lighting, and HUD workloads. In particular, a fixed subdivision level can
look acceptable on small faces while still losing detail on large walls.
Any renderer triangle budget should reduce refinement rather than discard
visible base geometry.

The application is single-threaded. Two installed processors do not imply
twice the rendering throughput. The benchmark does not make concurrent calls
into XGL or claim to measure a multithreaded renderer.

## Portable checks

On a development host without Solaris/XGL:

```sh
make check
```

This compiles geometry and lighting-math tests, plus a separate
`build/xglbench-test` executable linked with `tests/test_backend.c`. That
backend is a test double used for argument handling, case scheduling,
resize sequencing, queue ownership,
lighting-state versus color updates, and report/CSV checks. Its deferred-queue
bookkeeping does not emulate a ZX. It identifies its output as
**NO GRAPHICS / NO VALID HARDWARE PERFORMANCE DATA**. Its timing numbers
have no graphics meaning.
It is never linked into the native `xglbench` executable.
Fault injection also checks that a native completion error, exposure, user
interruption, or unexpected resize cannot turn a partial trial into a valid
measurement. An unexpected size change stops the sequence so that subsequent
results cannot be labeled with the old dimensions.

A separate `build/xglbench-profile-test` uses a simulated clock to check all
profile selections, mandatory idle scheduling, fixed limits despite option
overrides, and rejection of interrupted results without waiting through full
runs. Watchdog tests exercise real alarm delivery, the ten-second grace timer,
and forced process exit even when standard error is a full, unread pipe.

On a modern development host, `make syntax-check` checks C syntax and selected
API usage against the hand-written declarations in `tests/api_contract/`.
Those declarations are documentation-derived test scaffolding, **not the Sun
SDK**. This check cannot validate binary ABI compatibility, the arguments
passed through XGL's variadic attribute interfaces, native behavior, or
performance. It is optional and separate from `make check`.

`make native-contract-check` runs focused host tests of the native backend's
memory-raster setup and readback error handling using mock XGL functions.
It checks partial-object cleanup and retries, validated raster properties,
and preservation of native errors. This separate target uses GNU-compatible
section garbage collection to link only the tested backend functions; it
does not require or emulate the XGL runtime and is not a Solaris build target.

Passing portable checks does not validate compilation against the proprietary
XGL headers, correct native rendering, or ZX performance. Those require the
Solaris build and native run. `make clean` removes only this folder's generated
binary and `build/` directory.

## References and license

The native implementation uses the supplied XGL Reference Manual and XGL
Programmer's Guide, notably facet colors/illumination, multi-simple polygons,
triangle lists/strips, raster masks, device inquiry, synchronization, and
window buffering. See source comments for API-specific details. The OpenGL
1.1 specification is conceptual background, not a dependency or a Leo
performance specification. The calibrated and streaming tests are informed
by the supplied *Leo: A System for Cost Effective 3D Shaded Graphics*
architectural whitepaper, especially printed pages 103-107.
The illumination modes and positional-light attenuation are described in
the Reference Manual, printed pages 101-103 and 312, and the Programmer's
Guide, pages 398-403. The Quake-like comparison follows the surface-distance
approach in `WinQuake/gl_rsurf.c`; it does not link engine code or load assets.

The benchmark source is GPL-2.0-or-later. `COPYING` is duplicated from the
repository's `gnu.txt`. All benchmark files and generated build artifacts
reside in `xglbench/`.
