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

While the opaque console slides over the view, the 3D clip boundary follows
its lower edge without changing the camera or scaling the exposed scene.
One device pixel overlaps beneath the panel to avoid seams. Fully covered
views submit no 3D geometry; simulation continues normally. Partial coverage
reduces hidden rasterization, but geometry preparation still runs. This is
automatic and does not affect performance with the console closed.
