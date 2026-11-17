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
    XGL_ProfileAdd(XGL_PROFILE_WORLD_SKY, 0);
    XGL_ProfileAdd(XGL_PROFILE_ALIAS_TRANSFORM, 0);
    XGL_ProfileAdd(XGL_PROFILE_ALIAS_LIGHT, 0);
    XGL_ProfileAdd(XGL_PROFILE_WEAPON_LIGHT, 0);
    XGL_ProfileCount(XGL_PROFILE_NATIVE_CACHED_VERTICES, 999);
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
    XGL_ProfileAdd(XGL_PROFILE_ALIAS_CACHE, 0);
    XGL_ProfileAdd(XGL_PROFILE_ALIAS_LIGHT, 0);
    XGL_ProfileAdd(XGL_PROFILE_WEAPON_LIGHT, 0);
    XGL_ProfileCount(XGL_PROFILE_WORLD_ANIMATED_SAMPLES, 99);
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
    XGL_ProfileCount(XGL_PROFILE_WORLD_ANIMATED_SAMPLES, 15);
    XGL_ProfileCount(XGL_PROFILE_NATIVE_CACHED_VERTICES, 48);
    XGL_ProfileCount(XGL_PROFILE_NATIVE_DIRECT_VERTICES, 12);
    XGL_ProfileCount(XGL_PROFILE_VERTEX_PACKED, 6);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_TOPOLOGY_HITS, 3);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_TOPOLOGY_MISSES, 1);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_POSE_SLOT_EVICTIONS, 2);
    XGL_ProfileCount(XGL_PROFILE_ALIAS_POSE_BYTE_EVICTIONS, 4);
    clock_time = 101.31; start = XGL_ProfileDetailNow();
    clock_time = 101.33; XGL_ProfileAdd(XGL_PROFILE_WORLD_SKY, start);
    clock_time = 101.34; start = XGL_ProfileDetailNow();
    clock_time = 101.38; XGL_ProfileAdd(XGL_PROFILE_WORLD_WATER, start);
    clock_time = 101.39; start = XGL_ProfileDetailNow();
    clock_time = 101.41; XGL_ProfileAdd(XGL_PROFILE_ALIAS_CACHE, start);
    clock_time = 101.42; start = XGL_ProfileDetailNow();
    clock_time = 101.48; XGL_ProfileAdd(XGL_PROFILE_ALIAS_TRANSFORM, start);
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
    assert(strstr(output, "sky 10.00 water 20.00 ms; animated samples/frame 7.5"));
    assert(strstr(output, "vertices/frame cached native 24.0 direct native 6.0 packed 3.0"));
    assert(strstr(output, "cache 10.00 transform 30.00 ms; topology hits 1.5 misses 0.5"));
    assert(strstr(output, "pose evictions slot 1.0 bytes 2.0"));
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

static void record_draw_scope(xgl_profile_draw_t draw, double wall,
                              double submit, double sync)
{
    xgl_profile_draw_scope_t scope;
    double start, native_start;
    start = clock_time;
    XGL_ProfileDrawBegin(&scope);
    native_start = XGL_ProfileNow();
    clock_time += submit;
    XGL_ProfileAdd(XGL_PROFILE_SUBMIT, native_start);
    native_start = XGL_ProfileNow();
    clock_time += sync;
    XGL_ProfileAdd(XGL_PROFILE_SYNC, native_start);
    if (draw == XGL_PROFILE_DRAW_WEAPON_SETUP) {
        native_start = XGL_ProfileDetailNow();
        clock_time += .004;
        XGL_ProfileAdd(XGL_PROFILE_WEAPON_CLEAR, native_start);
    }
    clock_time = start + wall;
    XGL_ProfileDrawEnd(&scope, draw);
}

static void test_draw_scope_aggregation(void)
{
    double start;
    int i;
    disable_profile();
    profile_setting->value = 2;
    clock_time = 200; XGL_ProfileBeginFrame();
    /* Native calls made before a draw are excluded from its snapshots. */
    start = XGL_ProfileNow();
    clock_time += .010; XGL_ProfileAdd(XGL_PROFILE_SUBMIT, start);
    start = XGL_ProfileNow();
    clock_time += .003; XGL_ProfileAdd(XGL_PROFILE_SYNC, start);
    for (i = 0; i < XGL_PROFILE_DRAW_COUNT; ++i)
        record_draw_scope((xgl_profile_draw_t)i, .040, .010, .005);
    clock_time = 200.5; XGL_ProfileEndFrame();
    assert(print_calls == 0);
    clock_time = 200.6; XGL_ProfileBeginFrame();
    record_draw_scope(XGL_PROFILE_DRAW_ALIAS, .100, .020, .010);
    clock_time = 201.1; XGL_ProfileEndFrame();
    assert(print_calls == 2);
    assert(strstr(output, "native calls (overlap stages): submit 75.00 sync 36.50"));
    assert(strstr(output, "native calls executed during scope"));
    assert(strstr(output, "alias 70.00/15.00/7.50 brush 20.00/5.00/2.50 sprite 20.00/5.00/2.50"));
    assert(strstr(output, "visits/frame 1.0/0.5/0.5"));
    assert(strstr(output, "setup 20.00/5.00/2.50 draw 20.00/5.00/2.50 finish 20.00/5.00/2.50"));
    assert(strstr(output, "depth clear 2.00 ms"));
    assert(strstr(output, "particles 20.00/5.00/2.50 water 20.00/5.00/2.50"));
    assert(strstr(output, "alias entity detail (overlap): lighting 0.00 ms; "
        "cached final vertex/color output 20.00/5.00/2.50; "
        "fallback (transform/subdivide/sample/pack) 20.00/5.00/2.50"));
    assert(strstr(output, "view weapon detail (overlap): lighting 0.00 ms; "
        "cached final vertex/color output 20.00/5.00/2.50; "
        "fallback (transform/subdivide/sample/pack) 20.00/5.00/2.50"));

    clock_time = 201.2; XGL_ProfileBeginFrame();
    clock_time = 202.3; XGL_ProfileEndFrame();
    assert(print_calls == 4);
    assert(strstr(output, "alias 0.00/0.00/0.00 brush 0.00/0.00/0.00 sprite 0.00/0.00/0.00"));
    assert(strstr(output, "visits/frame 0.0/0.0/0.0"));
    assert(strstr(output, "depth clear 0.00 ms"));
}

static void test_draw_scope_guards(void)
{
    xgl_profile_draw_scope_t scope, stale;
    int calls, i;
    disable_profile();
    for (i = 0; i < XGL_PROFILE_DRAW_COUNT; ++i) {
        XGL_ProfileDrawBegin(&scope);
        XGL_ProfileDrawEnd(&scope, (xgl_profile_draw_t)i);
    }
    assert(clock_calls == 0);
    profile_setting->value = 1;
    clock_time = 300; XGL_ProfileBeginFrame();
    calls = clock_calls;
    for (i = 0; i < XGL_PROFILE_DRAW_COUNT; ++i) {
        XGL_ProfileDrawBegin(&scope);
        XGL_ProfileDrawEnd(&scope, (xgl_profile_draw_t)i);
    }
    XGL_ProfileAdd(XGL_PROFILE_WEAPON_CLEAR, 0);
    assert(clock_calls == calls);
    clock_time = 300.1; XGL_ProfileEndFrame();

    profile_setting->value = 2;
    clock_time = 301; XGL_ProfileBeginFrame();
    XGL_ProfileDrawBegin(&stale);
    clock_time = 301.1; XGL_ProfileEndFrame();
    clock_time = 301.2; XGL_ProfileBeginFrame();
    calls = clock_calls;
    XGL_ProfileDrawEnd(&stale, XGL_PROFILE_DRAW_ALIAS);
    XGL_ProfileDrawBegin(NULL);
    XGL_ProfileDrawEnd(NULL, XGL_PROFILE_DRAW_ALIAS);
    assert(clock_calls == calls);
    XGL_ProfileDrawBegin(&scope);
    calls = clock_calls;
    XGL_ProfileDrawEnd(&scope, (xgl_profile_draw_t)-1);
    XGL_ProfileDrawEnd(&scope, XGL_PROFILE_DRAW_COUNT);
    assert(clock_calls == calls);
    clock_time = 301.3; XGL_ProfileDrawEnd(&scope, XGL_PROFILE_DRAW_ALIAS);
    calls = clock_calls;
    XGL_ProfileDrawEnd(&scope, XGL_PROFILE_DRAW_ALIAS);
    assert(clock_calls == calls);
    clock_time = 302.1; XGL_ProfileEndFrame();
    assert(print_calls == 2);
    assert(strstr(output, "alias 50.00/0.00/0.00"));
    assert(strstr(output, "visits/frame 0.5/0.0/0.0"));

    /* A backwards clock invalidates the frame and all draw totals. */
    clock_time = 303; XGL_ProfileBeginFrame();
    XGL_ProfileDrawBegin(&scope);
    clock_time = 3; XGL_ProfileDrawEnd(&scope, XGL_PROFILE_DRAW_ALIAS);
    XGL_ProfileEndFrame();
    assert(print_calls == 2);
    clock_time = 4; XGL_ProfileBeginFrame();
    clock_time = 5.1; XGL_ProfileEndFrame();
    assert(print_calls == 4);
    assert(strstr(output, "alias 0.00/0.00/0.00"));
    assert(strstr(output, "visits/frame 0.0/0.0/0.0"));

    /* Changing modes discards a partial detailed interval. */
    clock_time = 5.2; XGL_ProfileBeginFrame();
    record_draw_scope(XGL_PROFILE_DRAW_ALIAS, .040, .010, .005);
    XGL_ProfileDrawBegin(&stale);
    clock_time = 5.3; XGL_ProfileEndFrame();
    profile_setting->value = 1;
    clock_time = 5.4; XGL_ProfileBeginFrame();
    clock_time = 5.5; XGL_ProfileEndFrame();
    profile_setting->value = 2;
    clock_time = 5.6; XGL_ProfileBeginFrame();
    calls = clock_calls;
    XGL_ProfileDrawEnd(&stale, XGL_PROFILE_DRAW_ALIAS);
    assert(clock_calls == calls);
    clock_time = 6.7; XGL_ProfileEndFrame();
    assert(print_calls == 6);
    assert(strstr(output, "alias 0.00/0.00/0.00"));
    assert(strstr(output, "visits/frame 0.0/0.0/0.0"));
    calls = clock_calls;
    XGL_ProfileDrawBegin(&scope);
    XGL_ProfileDrawEnd(&scope, XGL_PROFILE_DRAW_ALIAS);
    assert(clock_calls == calls);
}

static void test_nested_alias_details(void)
{
    xgl_profile_draw_scope_t outer;
    double start;
    disable_profile();
    profile_setting->value = 2;
    clock_time = 400; XGL_ProfileBeginFrame();
    clock_time = 400.01;
    XGL_ProfileDrawBegin(&outer);
    start = XGL_ProfileDetailNow();
    clock_time += .012; XGL_ProfileAdd(XGL_PROFILE_ALIAS_LIGHT, start);
    record_draw_scope(XGL_PROFILE_DRAW_ALIAS_OUTPUT, .040, .010, .005);
    record_draw_scope(XGL_PROFILE_DRAW_ALIAS_FALLBACK, .060, .020, .007);
    XGL_ProfileDrawEnd(&outer, XGL_PROFILE_DRAW_ALIAS);

    XGL_ProfileDrawBegin(&outer);
    start = XGL_ProfileDetailNow();
    clock_time += .018; XGL_ProfileAdd(XGL_PROFILE_WEAPON_LIGHT, start);
    record_draw_scope(XGL_PROFILE_DRAW_WEAPON_OUTPUT, .080, .030, .009);
    record_draw_scope(XGL_PROFILE_DRAW_WEAPON_FALLBACK, .100, .040, .011);
    XGL_ProfileDrawEnd(&outer, XGL_PROFILE_DRAW_WEAPON_DRAW);
    clock_time = 400.5; XGL_ProfileEndFrame();
    assert(print_calls == 0);

    clock_time = 400.6; XGL_ProfileBeginFrame();
    XGL_ProfileDrawBegin(&outer);
    start = XGL_ProfileDetailNow();
    clock_time += .008; XGL_ProfileAdd(XGL_PROFILE_ALIAS_LIGHT, start);
    record_draw_scope(XGL_PROFILE_DRAW_ALIAS_OUTPUT, .020, .004, .002);
    XGL_ProfileDrawEnd(&outer, XGL_PROFILE_DRAW_ALIAS);

    XGL_ProfileDrawBegin(&outer);
    start = XGL_ProfileDetailNow();
    clock_time += .006; XGL_ProfileAdd(XGL_PROFILE_WEAPON_LIGHT, start);
    record_draw_scope(XGL_PROFILE_DRAW_WEAPON_FALLBACK, .040, .012, .004);
    XGL_ProfileDrawEnd(&outer, XGL_PROFILE_DRAW_WEAPON_DRAW);
    clock_time = 401.1; XGL_ProfileEndFrame();
    assert(print_calls == 2);

    /* Nested scopes retain independent wall/native deltas; native calls
     * appear once in frame totals and in both enclosing and nested scopes. */
    assert(strstr(output, "native calls (overlap stages): submit 58.00 sync 19.00"));
    assert(strstr(output, "alias 70.00/17.00/7.00 brush 0.00/0.00/0.00"));
    assert(strstr(output, "draw 122.00/41.00/12.00 finish 0.00/0.00/0.00"));
    assert(strstr(output, "alias entity detail (overlap): lighting 10.00 ms; "
        "cached final vertex/color output 30.00/7.00/3.50; "
        "fallback (transform/subdivide/sample/pack) 30.00/10.00/3.50"));
    assert(strstr(output, "view weapon detail (overlap): lighting 12.00 ms; "
        "cached final vertex/color output 40.00/15.00/4.50; "
        "fallback (transform/subdivide/sample/pack) 70.00/26.00/7.50"));

    clock_time = 401.2; XGL_ProfileBeginFrame();
    clock_time = 402.3; XGL_ProfileEndFrame();
    assert(print_calls == 4);
    assert(strstr(output, "alias entity detail (overlap): lighting 0.00 ms; "
        "cached final vertex/color output 0.00/0.00/0.00; "
        "fallback (transform/subdivide/sample/pack) 0.00/0.00/0.00"));
    assert(strstr(output, "view weapon detail (overlap): lighting 0.00 ms; "
        "cached final vertex/color output 0.00/0.00/0.00; "
        "fallback (transform/subdivide/sample/pack) 0.00/0.00/0.00"));
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
    test_draw_scope_aggregation();
    test_draw_scope_guards();
    test_nested_alias_details();
    puts("XGL profiling: clock-free disabled/detail-off paths, mode changes, stage/cache aggregation and terminal reporting passed");
    return 0;
}
