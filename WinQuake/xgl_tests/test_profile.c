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
    va_start(args, format);
    vsprintf(output, format, args);
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
    XGL_ProfileEndFrame();
    assert(start == 0 && clock_calls == 0 && print_calls == 0);
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
    puts("XGL profiling: disabled clock-free path, stage/counter aggregation and terminal reporting passed");
    return 0;
}
