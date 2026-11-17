/* Timer/statistics checks; GPL-2.0-or-later, see COPYING. */
#include "xglbench.h"
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int failed;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "timer test failed: %s\n", message);
        failed = 1;
    }
}

int main(void)
{
    double odd[5], even[4], original[5];
    double before, after, cpu_before, cpu_after;
    volatile double sum;
    int i;
    odd[0] = 9; odd[1] = 2; odd[2] = 7; odd[3] = 1; odd[4] = 3;
    even[0] = 2; even[1] = 20; even[2] = 4; even[3] = 10;
    memcpy(original, odd, sizeof(odd));
    check(xb_median(odd, 5) == 3, "odd sample median");
    check(xb_median(even, 4) == 7, "even sample median");
    check(xb_median(odd, 1) == 9, "single sample median");
    check(!memcmp(odd, original, sizeof(odd)), "statistics changed samples");
    check(xb_median(NULL, 0) == 0, "empty count");
    check(xb_median(NULL, -1) == 0, "negative count");
    check(xb_median(NULL, XB_MAX_REPEATS + 1) == 0, "over-limit count");
    before = xb_now();
    cpu_before = xb_cpu_now();
    sum = 0;
    for (i = 1; i < 100000; ++i) sum += 1.0 / (double)i;
    after = xb_now();
    cpu_after = xb_cpu_now();
    check(sum > 0, "timed work executed");
    check(after >= before, "elapsed clock moved backward");
    check(cpu_after >= cpu_before, "process CPU clock moved backward");
    check(xb_sleep(0.0), "zero-length sleep");
    errno = 0;
    check(!xb_sleep(-0.01) && errno == EINVAL, "negative sleep rejected");
    errno = 0;
    check(!xb_sleep(DBL_MAX) && errno == EINVAL, "oversized sleep rejected");
    errno = 0;
    check(!xb_sleep((double)INT_MAX) && errno == EINVAL, "sleep conversion bound rejected");
    before = xb_now();
    check(xb_sleep(0.02), "short sleep completed");
    after = xb_now();
    /* Allow clock granularity at the lower bound. Scheduler delays are
     * unbounded, so there is intentionally no elapsed upper-bound assertion.
     */
    check(after - before >= 0.01, "short sleep actually paused");
    if (failed) return 1;
    puts("timer tests: median, input bounds, sample preservation, clock ordering and idle sleep passed");
    return 0;
}
