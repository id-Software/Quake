/* Deterministic profiling checks: no XGL, clock or terminal dependencies. */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../quakedef.h"
#include "../xgl_profile.h"

static cvar_t *profile_setting;
static double clock_time;
static int clock_calls, register_calls, print_calls;
static char output[4096];

double Sys_FloatTime(void)
{
    ++clock_calls;
    return clock_time;
}

void Cvar_RegisterVariable(cvar_t *variable)
{
    assert(!strcmp(variable->name, "xgl_profile"));
    assert(!strcmp(variable->string, "0"));
    assert(!variable->archive);
    profile_setting = variable;
    variable->value = (float)atof(variable->string);
    ++register_calls;
}

void Sys_Printf(char *format, ...)
{
    va_list args;
    /* Keep the whole latest report, including optional detail lines. */
    if (!strncmp(format, "XGL profile:", 12)) output[0] = 0;
    va_start(args, format);
    vsprintf(output + strlen(output), format, args);
    va_end(args);
    ++print_calls;
}

static void disable_profile(void)
{
    profile_setting->value = 0;
    XGL_ProfileBeginFrame();
    XGL_ProfileEndFrame();
    output[0] = 0;
    clock_calls = print_calls = 0;
}

static void test_disabled(void)
{
    double start;
    disable_profile();
    XGL_ProfileBeginFrame();
    start = XGL_ProfileNow();
    XGL_ProfileAdd(XGL_PROFILE_WORLD, start);
    XGL_ProfileCount(XGL_PROFILE_TRIANGLES, 999);
    assert(XGL_ProfileDetailNow() == 0);
    XGL_ProfileAdd(XGL_PROFILE_WORLD_BUILD, 0);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_COLOR_HITS, 999);
    XGL_ProfileEndFrame();
    assert(start == 0 && clock_calls == 0 && print_calls == 0);
}

static void test_detail_levels(void)
{
    double start;
    int calls;
    disable_profile();
    profile_setting->value = 1;
    clock_time = 100; XGL_ProfileBeginFrame();
    calls = clock_calls;
    assert(XGL_ProfileDetailNow() == 0);
    XGL_ProfileAdd(XGL_PROFILE_WORLD_BUILD, 0);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_COLOR_HITS, 99);
    assert(clock_calls == calls);
    XGL_ProfileCount(XGL_PROFILE_TRIANGLES, 999);
    clock_time = 100.5; XGL_ProfileEndFrame();
    assert(print_calls == 0);

    /* Changing mode drops the partial interval so disabled detail samples
     * never enter a detailed report's frame denominator. */
    profile_setting->value = 2;
    clock_time = 101; XGL_ProfileBeginFrame();
    clock_time = 101.02; start = XGL_ProfileDetailNow();
    clock_time = 101.12; XGL_ProfileAdd(XGL_PROFILE_WORLD_VISIBILITY, start);
    clock_time = 101.2; start = XGL_ProfileDetailNow();
    clock_time = 101.3; XGL_ProfileAdd(XGL_PROFILE_WORLD_BUILD, start);
    XGL_ProfileCount(XGL_PROFILE_WORLD_MESH_BUILDS, 2);
    XGL_ProfileCount(XGL_PROFILE_WORLD_MESH_FAILURES, 1);
    XGL_ProfileCount(XGL_PROFILE_WORLD_BUILD_VERTICES, 96);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_COLOR_HITS, 3);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_COLOR_MISSES, 2);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_POSE_HITS, 1);
    XGL_ProfileCount(XGL_PROFILE_TEXT_CALLS, 4);
    clock_time = 101.5; XGL_ProfileEndFrame();
    assert(print_calls == 0);
    clock_time = 101.6; XGL_ProfileBeginFrame();
    clock_time = 101.7; start = XGL_ProfileDetailNow();
    clock_time = 101.9; XGL_ProfileAdd(XGL_PROFILE_WORLD_BUILD, start);
    XGL_ProfileCount(XGL_PROFILE_WORLD_MESH_BUILDS, 1);
    XGL_ProfileCount(XGL_PROFILE_WORLD_BUILD_VERTICES, 6);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_COLOR_HITS, 1);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_POSE_HITS, 3);
    XGL_ProfileCount(XGL_PROFILE_TEXT_CALLS, 2);
    clock_time = 102.2; XGL_ProfileEndFrame();
    assert(print_calls == 2);
    assert(strstr(output, "2 frames; gfx 550.00"));
    assert(strstr(output, "triangles 0.0"));
    assert(strstr(output, "visibility 50.00 select 0.00 build 150.00"));
    assert(strstr(output, "builds 1.5 failed builds 0.5"));
    assert(strstr(output, "vertices built 51.0"));
    assert(strstr(output, "color hits 2.0 misses 1.0 evictions 0.0"));
    assert(strstr(output, "pose hits 2.0 misses 0.0 evictions 0.0"));
    assert(strstr(output, "text calls 3.0"));
    calls = clock_calls;
    assert(XGL_ProfileDetailNow() == 0);
    XGL_ProfileAdd(XGL_PROFILE_WORLD_BUILD, 0);
    assert(clock_calls == calls);

    clock_time = 103; XGL_ProfileBeginFrame();
    XGL_ProfileCount(XGL_PROFILE_ALIAS_COLOR_HITS, 500);
    clock_time = 103.1; XGL_ProfileEndFrame();
    profile_setting->value = 1;
    clock_time = 103.2; XGL_ProfileBeginFrame();
    clock_time = 104.3; XGL_ProfileEndFrame();
    assert(print_calls == 3);
    assert(strstr(output, "1 frames; gfx 1100.00"));
    assert(!strstr(output, "detail ms") && !strstr(output, "alias cache"));

    /* Level 2 also respects backwards clocks and validates enum ranges
     * without any timer calls for malformed instrumentation identifiers. */
    profile_setting->value = 2;
    clock_time = 110; XGL_ProfileBeginFrame();
    calls = clock_calls;
    XGL_ProfileAdd((xgl_profile_section_t)-1, 0);
    XGL_ProfileAdd(XGL_PROFILE_SECTION_COUNT, 0);
    XGL_ProfileCount((xgl_profile_counter_t)-1, 1);
    assert(clock_calls == calls);
    start = XGL_ProfileDetailNow();
    clock_time = 10; XGL_ProfileAdd(XGL_PROFILE_WORLD_BUILD, start);
    XGL_ProfileEndFrame();
    assert(print_calls == 3);
    clock_time = 11; XGL_ProfileBeginFrame();
    clock_time = 12.1; XGL_ProfileEndFrame();
    assert(print_calls == 5 && strstr(output, "build 0.00"));
}

static void test_aggregation(void)
{
    double world_start, call_start;
    disable_profile();
    profile_setting->value = 1;
    clock_time = 10;
    XGL_ProfileBeginFrame();
    clock_time = 10.05; world_start = XGL_ProfileNow();
    clock_time = 10.10; call_start = XGL_ProfileNow();
    clock_time = 10.125; XGL_ProfileAdd(XGL_PROFILE_SYNC, call_start);
    clock_time = 10.25; XGL_ProfileAdd(XGL_PROFILE_WORLD, world_start);
    clock_time = 10.26; call_start = XGL_ProfileNow();
    clock_time = 10.285; XGL_ProfileAdd(XGL_PROFILE_SUBMIT, call_start);
    XGL_ProfileCount(XGL_PROFILE_TRIANGLES, 30);
    XGL_ProfileCount(XGL_PROFILE_GLYPHS, 2);
    XGL_ProfileCount(XGL_PROFILE_POLYGON_CALLS, 2);
    XGL_ProfileCount(XGL_PROFILE_WAIT_CALLS, 1);
    clock_time = 10.5; XGL_ProfileEndFrame();
    assert(print_calls == 0);

    clock_time = 10.6; XGL_ProfileBeginFrame();
    clock_time = 10.65; world_start = XGL_ProfileNow();
    clock_time = 10.8; XGL_ProfileAdd(XGL_PROFILE_WORLD, world_start);
    XGL_ProfileCount(XGL_PROFILE_TRIANGLES, 10);
    XGL_ProfileCount(XGL_PROFILE_GLYPHS, 6);
    XGL_ProfileCount(XGL_PROFILE_POLYGON_CALLS, 4);
    XGL_ProfileCount(XGL_PROFILE_WAIT_CALLS, 3);
    clock_time = 11.2; XGL_ProfileEndFrame();
    assert(print_calls == 1);
    assert(strstr(output, "2 frames; gfx 550.00 ms (max 600.00)"));
    assert(strstr(output, "world 175.00"));
    assert(strstr(output, "native calls (overlap stages): submit 12.50 sync 12.50"));
    assert(strstr(output, "triangles 20.0 glyphs 4.0 polygon calls 3.0 waits 2.0"));

    /* Output is aggregated once per interval; a new interval has no stale
     * geometry counts or elapsed time from the previous report. */
    clock_time = 11.3; XGL_ProfileBeginFrame();
    clock_time = 11.8; XGL_ProfileEndFrame();
    assert(print_calls == 1);
    clock_time = 11.9; XGL_ProfileBeginFrame();
    clock_time = 12.21; XGL_ProfileEndFrame();
    assert(print_calls == 2);
    assert(strstr(output, "2 frames; gfx 405.00 ms (max 500.00)"));
    assert(strstr(output, "world 0.00"));
    assert(strstr(output, "triangles 0.0 glyphs 0.0 polygon calls 0.0 waits 0.0"));
}

static void test_toggle_and_clock_reset(void)
{
    double start;
    int calls;
    disable_profile();
    profile_setting->value = 1;
    clock_time = 20; XGL_ProfileBeginFrame();
    XGL_ProfileCount(XGL_PROFILE_TRIANGLES, 99);
    clock_time = 20.5; XGL_ProfileEndFrame();
    disable_profile();
    assert(clock_calls == 0);
    profile_setting->value = 1;
    clock_time = 30; XGL_ProfileBeginFrame();
    clock_time = 31.1; XGL_ProfileEndFrame();
    assert(print_calls == 1);
    assert(strstr(output, "1 frames; gfx 1100.00"));
    assert(strstr(output, "triangles 0.0"));

    /* Finishing a frame closes the sample window: stray calls must not
     * invoke the clock or contaminate the following report. */
    calls = clock_calls;
    assert(XGL_ProfileNow() == 0);
    XGL_ProfileAdd(XGL_PROFILE_WORLD, 0);
    XGL_ProfileCount(XGL_PROFILE_TRIANGLES, 999);
    XGL_ProfileEndFrame();
    assert(clock_calls == calls && print_calls == 1);

    clock_time = 40; XGL_ProfileBeginFrame();
    start = XGL_ProfileNow();
    clock_time = 5; XGL_ProfileAdd(XGL_PROFILE_WORLD, start);
    XGL_ProfileEndFrame();
    assert(print_calls == 1);
    clock_time = 6; XGL_ProfileBeginFrame();
    clock_time = 7.1; XGL_ProfileEndFrame();
    assert(print_calls == 2 && strstr(output, "world 0.00"));
}

int main(void)
{
    XGL_ProfileInit();
    XGL_ProfileInit();
    assert(register_calls == 1 && profile_setting && !profile_setting->value);
    test_disabled();
    test_aggregation();
    test_toggle_and_clock_reset();
    test_detail_levels();
    puts("XGL profiling: clock-free disabled/detail-off paths, mode changes, stage/cache aggregation and terminal reporting passed");
    return 0;
}
