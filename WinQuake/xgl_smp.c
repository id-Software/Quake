/* CPU preparation worker for the XGL renderer. GPL-2.0-or-later. */
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "quakedef.h"
#include "xgl_profile.h"
#include "xgl_smp.h"

#define XGL_SMP_QUEUE_SIZE 8

typedef struct {
    void (*function)(void *);
    void *argument;
    int profile;
} xgl_smp_job_t;

static cvar_t xgl_smp = {"xgl_smp", "-1", true, false, 0, NULL};
static int xgl_smp_registered, xgl_smp_initialized, xgl_smp_active;
static int xgl_smp_applied_mode;
static long xgl_smp_online;
static pthread_t xgl_smp_worker;

/* The mutex protects the queue, the in-flight flag, and the stop request.
 * The worker releases it during computation. The main thread owns the
 * configuration, worker handle, and engine console. */
static pthread_mutex_t xgl_smp_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t xgl_smp_work = PTHREAD_COND_INITIALIZER;
static pthread_cond_t xgl_smp_idle = PTHREAD_COND_INITIALIZER;
static xgl_smp_job_t xgl_smp_queue[XGL_SMP_QUEUE_SIZE];
static int xgl_smp_head, xgl_smp_count, xgl_smp_running, xgl_smp_stop;
static unsigned long xgl_smp_profiled_jobs;
static double xgl_smp_profiled_seconds;

static void *xgl_smp_worker_main(void *unused)
{
    xgl_smp_job_t job;
    struct timeval start, end;
    double elapsed;
    int timed;
    (void)unused;
    pthread_mutex_lock(&xgl_smp_mutex);
    for (;;) {
        while (!xgl_smp_count && !xgl_smp_stop)
            pthread_cond_wait(&xgl_smp_work, &xgl_smp_mutex);
        if (!xgl_smp_count) break;
        job = xgl_smp_queue[xgl_smp_head];
        xgl_smp_head = (xgl_smp_head + 1) % XGL_SMP_QUEUE_SIZE;
        --xgl_smp_count;
        xgl_smp_running = 1;
        pthread_mutex_unlock(&xgl_smp_mutex);
        timed = job.profile && gettimeofday(&start, NULL) == 0;
        job.function(job.argument);
        elapsed = 0;
        if (timed && gettimeofday(&end, NULL) == 0)
            elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6;
        pthread_mutex_lock(&xgl_smp_mutex);
        if (job.profile) {
            ++xgl_smp_profiled_jobs;
            if (elapsed > 0) xgl_smp_profiled_seconds += elapsed;
        }
        xgl_smp_running = 0;
        if (!xgl_smp_count) pthread_cond_signal(&xgl_smp_idle);
    }
    pthread_mutex_unlock(&xgl_smp_mutex);
    return NULL;
}

static int xgl_smp_start_worker(void)
{
    pthread_attr_t attributes;
    int error;
    error = pthread_attr_init(&attributes);
    if (error) return error;
#if defined(__sun) || defined(__sun__)
    /* Solaris 2.6 multiplexes default user threads over LWPs. Give this
     * worker its own LWP so CPU work can actually overlap the main thread. */
    error = pthread_attr_setscope(&attributes, PTHREAD_SCOPE_SYSTEM);
#endif
    if (!error) {
        xgl_smp_stop = 0;
        error = pthread_create(&xgl_smp_worker, &attributes, xgl_smp_worker_main, NULL);
    }
    pthread_attr_destroy(&attributes);
    xgl_smp_active = !error;
    return error;
}

static void xgl_smp_stop_worker(void)
{
    pthread_mutex_lock(&xgl_smp_mutex);
    xgl_smp_stop = 1;
    pthread_cond_signal(&xgl_smp_work);
    pthread_mutex_unlock(&xgl_smp_mutex);
    pthread_join(xgl_smp_worker, NULL);
    xgl_smp_profiled_jobs = 0;
    xgl_smp_profiled_seconds = 0;
    xgl_smp_active = 0;
}

void XGL_SMPUpdate(void)
{
    const char *name, *status;
    int mode, wanted, error;
    if (!xgl_smp_initialized) return;
    mode = xgl_smp.value < 0 ? -1 : xgl_smp.value > 0;
    if (mode == xgl_smp_applied_mode) return;
    xgl_smp_applied_mode = mode;
    wanted = mode == -1 ? xgl_smp_online > 1 : mode;
    name = mode == -1 ? "auto" : mode ? "forced on" : "forced off";
    error = 0;
    if (wanted && !xgl_smp_active) error = xgl_smp_start_worker();
    if (!wanted && xgl_smp_active) xgl_smp_stop_worker();

    /* SafePrintf writes to the terminal and the game console without
     * triggering a nested screen update during startup or a mode change. */
    status = xgl_smp_active ? "one preparation worker active" : "single-threaded";
    if (xgl_smp_online > 0)
        Con_SafePrintf("XGL SMP: %s, %ld online CPU(s), %s\n", name, xgl_smp_online, status);
    else
        Con_SafePrintf("XGL SMP: %s, online CPU count unavailable, %s\n", name, status);
    if (error) Con_SafePrintf("XGL SMP worker startup failed: %s\n", strerror(error));
}

void XGL_SMPInit(void)
{
    if (xgl_smp_initialized) return;
    if (!xgl_smp_registered) {
        Cvar_RegisterVariable(&xgl_smp);
        xgl_smp_registered = 1;
    }
    xgl_smp_online = sysconf(_SC_NPROCESSORS_ONLN);
    xgl_smp_applied_mode = -2;
    xgl_smp_initialized = 1;
    XGL_SMPUpdate();
}

void XGL_SMPShutdown(void)
{
    if (xgl_smp_active) xgl_smp_stop_worker();
    xgl_smp_initialized = 0;
}

int XGL_SMPActive(void)
{
    return xgl_smp_active;
}

int XGL_SMPSubmit(void (*function)(void *), void *argument)
{
    int tail, accepted;
    if (!xgl_smp_active || !function || pthread_mutex_trylock(&xgl_smp_mutex)) return 0;
    accepted = xgl_smp_count < XGL_SMP_QUEUE_SIZE;
    if (accepted) {
        tail = (xgl_smp_head + xgl_smp_count) % XGL_SMP_QUEUE_SIZE;
        xgl_smp_queue[tail].function = function;
        xgl_smp_queue[tail].argument = argument;
        xgl_smp_queue[tail].profile = XGL_ProfileDetailEnabled();
        ++xgl_smp_count;
        if (xgl_smp_count == 1) pthread_cond_signal(&xgl_smp_work);
    }
    pthread_mutex_unlock(&xgl_smp_mutex);
    return accepted;
}

void XGL_SMPWait(void)
{
    double start, seconds;
    unsigned long jobs;
    int pending;
    if (!xgl_smp_active) return;
    pthread_mutex_lock(&xgl_smp_mutex);
    pending = xgl_smp_count || xgl_smp_running;
    start = pending ? XGL_ProfileDetailNow() : 0;
    while (xgl_smp_count || xgl_smp_running)
        pthread_cond_wait(&xgl_smp_idle, &xgl_smp_mutex);
    jobs = xgl_smp_profiled_jobs;
    seconds = xgl_smp_profiled_seconds;
    xgl_smp_profiled_jobs = 0;
    xgl_smp_profiled_seconds = 0;
    pthread_mutex_unlock(&xgl_smp_mutex);
    if (pending) XGL_ProfileAdd(XGL_PROFILE_SMP_WAIT, start);
    if (jobs) XGL_ProfileSMPWork(jobs, seconds);
}
