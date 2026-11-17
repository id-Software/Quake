/* XGLQuake standalone benchmark. GPL-2.0-or-later; see COPYING. */
#if !defined(__sun) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#include "xglbench.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>

double xb_now(void)
{
#if defined(__sun)
    return (double)gethrtime() * 1.0e-9;
#elif defined(CLOCK_MONOTONIC)
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) {
        perror("clock_gettime");
        exit(1);
    }
    return (double)t.tv_sec + (double)t.tv_nsec * 1.0e-9;
#else
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1.0e-6;
#endif
}

double xb_cpu_now(void)
{
    struct rusage r;
    if (getrusage(RUSAGE_SELF, &r) != 0) return 0.0;
    return (double)r.ru_utime.tv_sec + (double)r.ru_stime.tv_sec +
           ((double)r.ru_utime.tv_usec + (double)r.ru_stime.tv_usec) * 1.0e-6;
}

int xb_sleep(double seconds)
{
    struct timeval delay;
    double micros;
    /* These comparisons also reject NaN and infinities without relying on
     * C99 floating-point helpers. Keep conversion safe on 32-bit Solaris.
     */
    if (!(seconds >= 0.0 && seconds < (double)INT_MAX)) {
        errno = EINVAL;
        return 0;
    }
    if (seconds == 0.0) return 1;
    delay.tv_sec = (long)seconds;
    micros = (seconds - (double)delay.tv_sec) * 1000000.0;
    delay.tv_usec = (long)micros;
    if ((double)delay.tv_usec < micros) ++delay.tv_usec;
    if (delay.tv_usec >= 1000000) {
        ++delay.tv_sec;
        delay.tv_usec = 0;
    }
    /* EINTR deliberately returns to the caller, which owns event polling
     * and any retry. No busy wait, clock adjustment or unbounded retry.
     */
    return select(0, NULL, NULL, NULL, &delay) == 0;
}

static int xb_double_compare(const void *a, const void *b)
{
    double x, y;
    x = *(const double *)a;
    y = *(const double *)b;
    return x < y ? -1 : x > y ? 1 : 0;
}

double xb_median(const double *values, int count)
{
    double copy[XB_MAX_REPEATS];
    int i;
    if (count < 1 || count > XB_MAX_REPEATS) return 0.0;
    for (i = 0; i < count; ++i) copy[i] = values[i];
    qsort(copy, (size_t)count, sizeof(copy[0]), xb_double_compare);
    if (count & 1) return copy[count / 2];
    return (copy[count / 2 - 1] + copy[count / 2]) * 0.5;
}
