## Profiling details

Use the two [performance commands](../README.md#measure-performance) in the
README with the same resolution, demo, audio lookahead, and detail settings
for each comparison.

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
