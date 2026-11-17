/* Deterministic wall clock for portable profile scheduling tests only.
 * This never measures rendering or hardware performance.
 * GPL-2.0-or-later; see COPYING.
 */
#define xb_now xb_real_now
#define xb_cpu_now xb_real_cpu_now
#define xb_sleep xb_real_sleep
#include "../xglbench_timer.c"
#undef xb_now
#undef xb_cpu_now
#undef xb_sleep

static double test_time;
static double test_sleep_time;
static double test_step;
static int test_clock_ready;
static int test_did_pause;

static void test_clock_init(void)
{
    const char *value;
    if (test_clock_ready) return;
    value = getenv("XB_TEST_CLOCK_STEP");
    test_step = value ? atof(value) : .01;
    if (!(test_step > 0 && test_step <= 3600)) {
        fprintf(stderr, "invalid test-only virtual clock step\n");
        exit(2);
    }
    test_clock_ready = 1;
}

double xb_now(void)
{
    test_clock_init();
    test_time += test_step;
    return test_time;
}

double xb_cpu_now(void)
{
    test_clock_init();
    return (test_time - test_sleep_time) * .25;
}

int xb_sleep(double seconds)
{
    const char *path;
    FILE *file;
    test_clock_init();
    if (!(seconds >= 0 && seconds <= 3600)) return 0;
    test_time += seconds;
    test_sleep_time += seconds;
    if (seconds > 0) test_did_pause = 1;
    path = getenv("XB_TEST_LOG");
    if (path) {
        file = fopen(path, "a");
        if (file) {
            fprintf(file, "clock-sleep %.9f %.9f\n", seconds, test_time);
            fclose(file);
        }
    }
    return 1;
}

int xb_test_paused(void)
{
    return test_did_pause;
}

/* Simulate blocking setup/drawing; advancing work is not an idle pause. */
void xb_test_advance(double seconds)
{
    test_clock_init();
    if (seconds > 0) test_time += seconds;
}

