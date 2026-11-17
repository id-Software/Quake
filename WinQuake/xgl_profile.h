/* Optional XGL renderer profiling. GPL-2.0-or-later. */
#ifndef XGL_PROFILE_H
#define XGL_PROFILE_H

typedef enum {
    XGL_PROFILE_WORLD,
    XGL_PROFILE_ENTITIES,
    XGL_PROFILE_EFFECTS,
    XGL_PROFILE_VIEWMODEL,
    XGL_PROFILE_UI,
    XGL_PROFILE_AUDIO,
    XGL_PROFILE_SUBMIT,
    XGL_PROFILE_SYNC,
    XGL_PROFILE_SWAP,
    /* Detail timers are enabled only by xgl_profile 2. They overlap the
     * renderer stages above; detail selection also includes mesh builds. */
    XGL_PROFILE_WORLD_VISIBILITY,
    XGL_PROFILE_WORLD_DETAIL_SELECT,
    XGL_PROFILE_WORLD_BUILD,
    XGL_PROFILE_WORLD_COLORS,
    XGL_PROFILE_WORLD_ANIMATED,
    XGL_PROFILE_VERTEX_PACK,
    XGL_PROFILE_ALIAS_PREPARE,
    XGL_PROFILE_WORLD_SKY,
    XGL_PROFILE_WORLD_WATER,
    XGL_PROFILE_ALIAS_CACHE,
    XGL_PROFILE_ALIAS_TRANSFORM,
    XGL_PROFILE_WEAPON_CLEAR,
    XGL_PROFILE_ALIAS_LIGHT,
    XGL_PROFILE_WEAPON_LIGHT,
    XGL_PROFILE_SECTION_COUNT
} xgl_profile_section_t;

typedef enum {
    XGL_PROFILE_TRIANGLES,
    XGL_PROFILE_GLYPHS,
    XGL_PROFILE_POLYGON_CALLS,
    XGL_PROFILE_WAIT_CALLS,
    /* Detail counters use per-frame averages in the report. Cached world
     * draws include a mesh built earlier in the same frame; they are not
     * lookup hits and must not be divided by the build count as a hit rate. */
    XGL_PROFILE_TEXT_CALLS,
    XGL_PROFILE_WORLD_MESH_BUILDS,
    XGL_PROFILE_WORLD_MESH_FAILURES,
    XGL_PROFILE_WORLD_CACHED_SURFACES,
    XGL_PROFILE_WORLD_FALLBACK_SURFACES,
    XGL_PROFILE_WORLD_COLOR_HITS,
    XGL_PROFILE_WORLD_COLOR_UPDATES,
    XGL_PROFILE_WORLD_BUILD_VERTICES,
    XGL_PROFILE_WORLD_COLOR_VERTICES,
    XGL_PROFILE_WORLD_ANIMATED_VERTICES,
    XGL_PROFILE_ALIAS_COLOR_HITS,
    XGL_PROFILE_ALIAS_COLOR_MISSES,
    XGL_PROFILE_ALIAS_COLOR_EVICTIONS,
    XGL_PROFILE_ALIAS_POSE_HITS,
    XGL_PROFILE_ALIAS_POSE_MISSES,
    XGL_PROFILE_ALIAS_POSE_EVICTIONS,
    XGL_PROFILE_ALIAS_FALLBACKS,
    XGL_PROFILE_WORLD_ANIMATED_SAMPLES,
    XGL_PROFILE_NATIVE_CACHED_VERTICES,
    XGL_PROFILE_NATIVE_DIRECT_VERTICES,
    XGL_PROFILE_VERTEX_PACKED,
    XGL_PROFILE_ALIAS_POSE_SLOT_EVICTIONS,
    XGL_PROFILE_ALIAS_POSE_BYTE_EVICTIONS,
    XGL_PROFILE_ALIAS_TOPOLOGY_HITS,
    XGL_PROFILE_ALIAS_TOPOLOGY_MISSES,
    XGL_PROFILE_COUNTER_COUNT
} xgl_profile_counter_t;

typedef enum {
    XGL_PROFILE_DRAW_ALIAS,
    XGL_PROFILE_DRAW_BRUSH,
    XGL_PROFILE_DRAW_SPRITE,
    XGL_PROFILE_DRAW_WEAPON_SETUP,
    XGL_PROFILE_DRAW_WEAPON_DRAW,
    XGL_PROFILE_DRAW_WEAPON_FINISH,
    XGL_PROFILE_DRAW_PARTICLES,
    XGL_PROFILE_DRAW_WATER,
    XGL_PROFILE_DRAW_ALIAS_OUTPUT,
    XGL_PROFILE_DRAW_WEAPON_OUTPUT,
    XGL_PROFILE_DRAW_ALIAS_FALLBACK,
    XGL_PROFILE_DRAW_WEAPON_FALLBACK,
    XGL_PROFILE_DRAW_COUNT
} xgl_profile_draw_t;

/* Stack-owned detail snapshot. Native deltas measure calls executed during
 * the scope; a batch may contain geometry queued by an earlier scope.
 * Nested scope totals overlap their enclosing scope. */
typedef struct {
    double start, submit, sync;
    unsigned long frame;
} xgl_profile_draw_scope_t;

void XGL_ProfileInit(void);
void XGL_ProfileBeginFrame(void);
void XGL_ProfileEndFrame(void);
double XGL_ProfileNow(void);
double XGL_ProfileDetailNow(void);
void XGL_ProfileAdd(xgl_profile_section_t section, double start);
void XGL_ProfileCount(xgl_profile_counter_t counter, unsigned long count);
void XGL_ProfileDrawBegin(xgl_profile_draw_scope_t *scope);
void XGL_ProfileDrawEnd(xgl_profile_draw_scope_t *scope, xgl_profile_draw_t draw);

#endif
