/* Optional XGL renderer profiling. GPL-2.0-or-later.
 * Graphics frame and stage times are wall-clock durations. Native call
 * and audio service times overlap those stages: they must not be added
 * to the frame total.
 * Output goes to the terminal, never the in-game console being measured.
 */
#include "quakedef.h"
#include "xgl_profile.h"

static cvar_t xgl_profile = {"xgl_profile", "0", false, false, 0, NULL};
static int profile_initialized, profile_active, profile_reporting;
static double frame_start, report_start, total_frame, maximum_frame;
static double frame_sections[XGL_PROFILE_SECTION_COUNT];
static double total_sections[XGL_PROFILE_SECTION_COUNT];
static unsigned long frame_counts[XGL_PROFILE_COUNTER_COUNT];
static unsigned long total_counts[XGL_PROFILE_COUNTER_COUNT];
static unsigned long total_frames;

typedef struct {
    double wall, submit, sync;
    unsigned long visits;
} xgl_profile_draw_total_t;

static xgl_profile_draw_total_t frame_draw[XGL_PROFILE_DRAW_COUNT];
static xgl_profile_draw_total_t total_draw[XGL_PROFILE_DRAW_COUNT];
static unsigned long draw_frame;

static void XGL_ProfileReset(void)
{
    total_frame = maximum_frame = 0;
    total_frames = 0;
    memset(total_sections, 0, sizeof(total_sections));
    memset(total_counts, 0, sizeof(total_counts));
    memset(total_draw, 0, sizeof(total_draw));
}

void XGL_ProfileInit(void)
{
    if (profile_initialized) return;
    Cvar_RegisterVariable(&xgl_profile);
    profile_initialized = 1;
}

void XGL_ProfileBeginFrame(void)
{
    profile_active = xgl_profile.value >= 2 ? 2 : xgl_profile.value != 0;
    if (!profile_active) {
        if (profile_reporting) {
            profile_reporting = 0;
            XGL_ProfileReset();
        }
        return;
    }
    frame_start = Sys_FloatTime();
    /* Changing detail level starts a fresh interval: otherwise the detail
     * averages would include frames in which those sections were not timed. */
    if (profile_reporting != profile_active) {
        XGL_ProfileReset();
        report_start = frame_start;
        profile_reporting = profile_active;
    }
    memset(frame_sections, 0, sizeof(frame_sections));
    memset(frame_counts, 0, sizeof(frame_counts));
    if (profile_active == 2) {
        memset(frame_draw, 0, sizeof(frame_draw));
        if (!++draw_frame) ++draw_frame;
    }
}

double XGL_ProfileNow(void)
{
    return profile_active ? Sys_FloatTime() : 0;
}

double XGL_ProfileDetailNow(void)
{
    return profile_active == 2 ? Sys_FloatTime() : 0;
}

void XGL_ProfileAdd(xgl_profile_section_t section, double start)
{
    double elapsed;
    if (!profile_active || (unsigned int)section >= XGL_PROFILE_SECTION_COUNT) return;
    if (section >= XGL_PROFILE_WORLD_VISIBILITY && profile_active != 2) return;
    elapsed = Sys_FloatTime() - start;
    if (elapsed > 0) frame_sections[section] += elapsed;
}

void XGL_ProfileCount(xgl_profile_counter_t counter, unsigned long count)
{
    if (!profile_active || (unsigned int)counter >= XGL_PROFILE_COUNTER_COUNT) return;
    if (counter >= XGL_PROFILE_TEXT_CALLS && profile_active != 2) return;
    frame_counts[counter] += count;
}

void XGL_ProfileDrawBegin(xgl_profile_draw_scope_t *scope)
{
    if (!scope) return;
    scope->frame = 0;
    if (profile_active != 2) return;
    scope->start = Sys_FloatTime();
    scope->submit = frame_sections[XGL_PROFILE_SUBMIT];
    scope->sync = frame_sections[XGL_PROFILE_SYNC];
    scope->frame = draw_frame;
}

void XGL_ProfileDrawEnd(xgl_profile_draw_scope_t *scope, xgl_profile_draw_t draw)
{
    xgl_profile_draw_total_t *total;
    double elapsed;
    if (!scope || profile_active != 2 || !scope->frame ||
        scope->frame != draw_frame || (unsigned int)draw >= XGL_PROFILE_DRAW_COUNT) return;
    scope->frame = 0;
    elapsed = Sys_FloatTime() - scope->start;
    if (elapsed < 0) return;
    total = &frame_draw[draw];
    total->wall += elapsed;
    total->submit += frame_sections[XGL_PROFILE_SUBMIT] - scope->submit;
    total->sync += frame_sections[XGL_PROFILE_SYNC] - scope->sync;
    ++total->visits;
}

void XGL_ProfileEndFrame(void)
{
    double now, elapsed, scale;
    int i;
    if (!profile_active) return;
    now = Sys_FloatTime();
    elapsed = now - frame_start;
    profile_active = 0;
    /* A reset wall clock must not produce negative durations or strand
     * the reporting interval until its previous timestamp catches up. */
    if (elapsed < 0 || now < report_start) {
        profile_reporting = 0;
        XGL_ProfileReset();
        return;
    }
    ++total_frames;
    total_frame += elapsed;
    if (elapsed > maximum_frame) maximum_frame = elapsed;
    for (i = 0; i < XGL_PROFILE_SECTION_COUNT; ++i)
        total_sections[i] += frame_sections[i];
    for (i = 0; i < XGL_PROFILE_COUNTER_COUNT; ++i)
        total_counts[i] += frame_counts[i];
    if (profile_reporting == 2) {
        for (i = 0; i < XGL_PROFILE_DRAW_COUNT; ++i) {
            total_draw[i].wall += frame_draw[i].wall;
            total_draw[i].submit += frame_draw[i].submit;
            total_draw[i].sync += frame_draw[i].sync;
            total_draw[i].visits += frame_draw[i].visits;
        }
    }
    if (now - report_start < 1.0) return;
    scale = 1000.0 / total_frames;
    Sys_Printf("XGL profile: %lu frames; gfx %.2f ms (max %.2f); "
        "world %.2f entities %.2f effects %.2f weapon %.2f UI %.2f audio-overlap %.2f ms\n"
        "  native calls (overlap stages): submit %.2f sync %.2f swap %.2f ms; "
        "per frame: triangles %.1f glyphs %.1f polygon calls %.1f waits %.1f\n",
        total_frames, total_frame * scale, maximum_frame * 1000.0,
        total_sections[XGL_PROFILE_WORLD] * scale,
        total_sections[XGL_PROFILE_ENTITIES] * scale,
        total_sections[XGL_PROFILE_EFFECTS] * scale,
        total_sections[XGL_PROFILE_VIEWMODEL] * scale,
        total_sections[XGL_PROFILE_UI] * scale,
        total_sections[XGL_PROFILE_AUDIO] * scale,
        total_sections[XGL_PROFILE_SUBMIT] * scale,
        total_sections[XGL_PROFILE_SYNC] * scale,
        total_sections[XGL_PROFILE_SWAP] * scale,
        (double)total_counts[XGL_PROFILE_TRIANGLES] / total_frames,
        (double)total_counts[XGL_PROFILE_GLYPHS] / total_frames,
        (double)total_counts[XGL_PROFILE_POLYGON_CALLS] / total_frames,
        (double)total_counts[XGL_PROFILE_WAIT_CALLS] / total_frames);
    if (profile_reporting == 2) {
        Sys_Printf("  detail ms (overlap stages; select includes builds): "
            "visibility %.2f select %.2f build %.2f colors %.2f animated %.2f "
            "pack %.2f alias-cache/transform %.2f\n"
            "  world per frame: cached draws %.1f fallback draws %.1f "
            "builds %.1f failed builds %.1f; color reuse %.1f updates %.1f; "
            "vertices built %.1f recolored %.1f animated %.1f\n"
            "  alias cache per frame: color hits %.1f misses %.1f evictions %.1f; "
            "pose hits %.1f misses %.1f evictions %.1f; fallbacks %.1f; text calls %.1f\n"
            "  reuse: sky %.2f water %.2f ms; animated samples/frame %.1f; "
            "vertices/frame cached native %.1f direct native %.1f packed %.1f\n"
            "  alias preparation (overlap): cache %.2f transform %.2f ms; "
            "topology hits %.1f misses %.1f; pose evictions slot %.1f bytes %.1f\n"
            "  entity scopes wall/submit/sync ms (native calls executed during scope): "
            "alias %.2f/%.2f/%.2f brush %.2f/%.2f/%.2f sprite %.2f/%.2f/%.2f; "
            "visits/frame %.1f/%.1f/%.1f\n"
            "  weapon scopes wall/submit/sync ms: setup %.2f/%.2f/%.2f "
            "draw %.2f/%.2f/%.2f finish %.2f/%.2f/%.2f; depth clear %.2f ms\n"
            "  effects scopes wall/submit/sync ms: particles %.2f/%.2f/%.2f "
            "water %.2f/%.2f/%.2f\n"
            "  alias entity detail (overlap): lighting %.2f ms; "
            "cached final vertex/color output %.2f/%.2f/%.2f; "
            "fallback (transform/subdivide/sample/pack) %.2f/%.2f/%.2f ms wall/submit/sync\n"
            "  view weapon detail (overlap): lighting %.2f ms; "
            "cached final vertex/color output %.2f/%.2f/%.2f; "
            "fallback (transform/subdivide/sample/pack) %.2f/%.2f/%.2f ms wall/submit/sync\n",
            total_sections[XGL_PROFILE_WORLD_VISIBILITY] * scale,
            total_sections[XGL_PROFILE_WORLD_DETAIL_SELECT] * scale,
            total_sections[XGL_PROFILE_WORLD_BUILD] * scale,
            total_sections[XGL_PROFILE_WORLD_COLORS] * scale,
            total_sections[XGL_PROFILE_WORLD_ANIMATED] * scale,
            total_sections[XGL_PROFILE_VERTEX_PACK] * scale,
            total_sections[XGL_PROFILE_ALIAS_PREPARE] * scale,
            (double)total_counts[XGL_PROFILE_WORLD_CACHED_SURFACES] / total_frames,
            (double)total_counts[XGL_PROFILE_WORLD_FALLBACK_SURFACES] / total_frames,
            (double)total_counts[XGL_PROFILE_WORLD_MESH_BUILDS] / total_frames,
            (double)total_counts[XGL_PROFILE_WORLD_MESH_FAILURES] / total_frames,
            (double)total_counts[XGL_PROFILE_WORLD_COLOR_HITS] / total_frames,
            (double)total_counts[XGL_PROFILE_WORLD_COLOR_UPDATES] / total_frames,
            (double)total_counts[XGL_PROFILE_WORLD_BUILD_VERTICES] / total_frames,
            (double)total_counts[XGL_PROFILE_WORLD_COLOR_VERTICES] / total_frames,
            (double)total_counts[XGL_PROFILE_WORLD_ANIMATED_VERTICES] / total_frames,
            (double)total_counts[XGL_PROFILE_ALIAS_COLOR_HITS] / total_frames,
            (double)total_counts[XGL_PROFILE_ALIAS_COLOR_MISSES] / total_frames,
            (double)total_counts[XGL_PROFILE_ALIAS_COLOR_EVICTIONS] / total_frames,
            (double)total_counts[XGL_PROFILE_ALIAS_POSE_HITS] / total_frames,
            (double)total_counts[XGL_PROFILE_ALIAS_POSE_MISSES] / total_frames,
            (double)total_counts[XGL_PROFILE_ALIAS_POSE_EVICTIONS] / total_frames,
            (double)total_counts[XGL_PROFILE_ALIAS_FALLBACKS] / total_frames,
            (double)total_counts[XGL_PROFILE_TEXT_CALLS] / total_frames,
            total_sections[XGL_PROFILE_WORLD_SKY] * scale,
            total_sections[XGL_PROFILE_WORLD_WATER] * scale,
            (double)total_counts[XGL_PROFILE_WORLD_ANIMATED_SAMPLES] / total_frames,
            (double)total_counts[XGL_PROFILE_NATIVE_CACHED_VERTICES] / total_frames,
            (double)total_counts[XGL_PROFILE_NATIVE_DIRECT_VERTICES] / total_frames,
            (double)total_counts[XGL_PROFILE_VERTEX_PACKED] / total_frames,
            total_sections[XGL_PROFILE_ALIAS_CACHE] * scale,
            total_sections[XGL_PROFILE_ALIAS_TRANSFORM] * scale,
            (double)total_counts[XGL_PROFILE_ALIAS_TOPOLOGY_HITS] / total_frames,
            (double)total_counts[XGL_PROFILE_ALIAS_TOPOLOGY_MISSES] / total_frames,
            (double)total_counts[XGL_PROFILE_ALIAS_POSE_SLOT_EVICTIONS] / total_frames,
            (double)total_counts[XGL_PROFILE_ALIAS_POSE_BYTE_EVICTIONS] / total_frames,
            total_draw[XGL_PROFILE_DRAW_ALIAS].wall * scale,
            total_draw[XGL_PROFILE_DRAW_ALIAS].submit * scale,
            total_draw[XGL_PROFILE_DRAW_ALIAS].sync * scale,
            total_draw[XGL_PROFILE_DRAW_BRUSH].wall * scale,
            total_draw[XGL_PROFILE_DRAW_BRUSH].submit * scale,
            total_draw[XGL_PROFILE_DRAW_BRUSH].sync * scale,
            total_draw[XGL_PROFILE_DRAW_SPRITE].wall * scale,
            total_draw[XGL_PROFILE_DRAW_SPRITE].submit * scale,
            total_draw[XGL_PROFILE_DRAW_SPRITE].sync * scale,
            (double)total_draw[XGL_PROFILE_DRAW_ALIAS].visits / total_frames,
            (double)total_draw[XGL_PROFILE_DRAW_BRUSH].visits / total_frames,
            (double)total_draw[XGL_PROFILE_DRAW_SPRITE].visits / total_frames,
            total_draw[XGL_PROFILE_DRAW_WEAPON_SETUP].wall * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_SETUP].submit * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_SETUP].sync * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_DRAW].wall * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_DRAW].submit * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_DRAW].sync * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_FINISH].wall * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_FINISH].submit * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_FINISH].sync * scale,
            total_sections[XGL_PROFILE_WEAPON_CLEAR] * scale,
            total_draw[XGL_PROFILE_DRAW_PARTICLES].wall * scale,
            total_draw[XGL_PROFILE_DRAW_PARTICLES].submit * scale,
            total_draw[XGL_PROFILE_DRAW_PARTICLES].sync * scale,
            total_draw[XGL_PROFILE_DRAW_WATER].wall * scale,
            total_draw[XGL_PROFILE_DRAW_WATER].submit * scale,
            total_draw[XGL_PROFILE_DRAW_WATER].sync * scale,
            total_sections[XGL_PROFILE_ALIAS_LIGHT] * scale,
            total_draw[XGL_PROFILE_DRAW_ALIAS_OUTPUT].wall * scale,
            total_draw[XGL_PROFILE_DRAW_ALIAS_OUTPUT].submit * scale,
            total_draw[XGL_PROFILE_DRAW_ALIAS_OUTPUT].sync * scale,
            total_draw[XGL_PROFILE_DRAW_ALIAS_FALLBACK].wall * scale,
            total_draw[XGL_PROFILE_DRAW_ALIAS_FALLBACK].submit * scale,
            total_draw[XGL_PROFILE_DRAW_ALIAS_FALLBACK].sync * scale,
            total_sections[XGL_PROFILE_WEAPON_LIGHT] * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_OUTPUT].wall * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_OUTPUT].submit * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_OUTPUT].sync * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_FALLBACK].wall * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_FALLBACK].submit * scale,
            total_draw[XGL_PROFILE_DRAW_WEAPON_FALLBACK].sync * scale);
    }
    XGL_ProfileReset();
    report_start = now;
}
