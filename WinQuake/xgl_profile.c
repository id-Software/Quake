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

static void XGL_ProfileReset(void)
{
    total_frame = maximum_frame = 0;
    total_frames = 0;
    memset(total_sections, 0, sizeof(total_sections));
    memset(total_counts, 0, sizeof(total_counts));
}

void XGL_ProfileInit(void)
{
    if (profile_initialized) return;
    Cvar_RegisterVariable(&xgl_profile);
    profile_initialized = 1;
}

void XGL_ProfileBeginFrame(void)
{
    profile_active = xgl_profile.value != 0;
    if (!profile_active) {
        if (profile_reporting) {
            profile_reporting = 0;
            XGL_ProfileReset();
        }
        return;
    }
    frame_start = Sys_FloatTime();
    if (!profile_reporting) {
        XGL_ProfileReset();
        report_start = frame_start;
        profile_reporting = 1;
    }
    memset(frame_sections, 0, sizeof(frame_sections));
    memset(frame_counts, 0, sizeof(frame_counts));
}

double XGL_ProfileNow(void)
{
    return profile_active ? Sys_FloatTime() : 0;
}

void XGL_ProfileAdd(xgl_profile_section_t section, double start)
{
    double elapsed;
    if (!profile_active || (unsigned int)section >= XGL_PROFILE_SECTION_COUNT) return;
    elapsed = Sys_FloatTime() - start;
    if (elapsed > 0) frame_sections[section] += elapsed;
}

void XGL_ProfileCount(xgl_profile_counter_t counter, unsigned long count)
{
    if (!profile_active || (unsigned int)counter >= XGL_PROFILE_COUNTER_COUNT) return;
    frame_counts[counter] += count;
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
    XGL_ProfileReset();
    report_start = now;
}
