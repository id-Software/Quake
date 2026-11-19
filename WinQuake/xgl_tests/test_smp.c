/* Exercise the real pthread worker and queue with controlled platform probes. */
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static long online_cpus;
static int creation_error, threads_created;

static long test_sysconf(int name)
{
    assert(name == _SC_NPROCESSORS_ONLN);
    return online_cpus;
}

static int test_pthread_create(pthread_t *thread, const pthread_attr_t *attributes,
                               void *(*entry)(void *), void *argument)
{
    int error;
    if (creation_error) return creation_error;
    error = pthread_create(thread, attributes, entry, argument);
    if (!error) ++threads_created;
    return error;
}

#define sysconf test_sysconf
#define pthread_create test_pthread_create
#include "../xgl_smp.c"
#undef pthread_create
#undef sysconf

#include "xgl_profile.h"

static cvar_t *mode_variable;
static pthread_t owner;
static unsigned long reported_jobs;
static int detail_enabled;

void Cvar_RegisterVariable(cvar_t *variable)
{
    assert(!mode_variable && !strcmp(variable->name, "xgl_smp"));
    variable->value = (float)atof(variable->string);
    assert(variable->value == -1);
    mode_variable = variable;
}

static char console_message[512];
static char console_error[512];

void Con_SafePrintf(char *format, ...)
{
    va_list args;
    char message[512];
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    if (strstr(message, "startup failed"))
        strcpy(console_error, message);
    else
        strcpy(console_message, message);
}

static void check_status(const char *mode, const char *cpus, const char *status)
{
    assert(strstr(console_message, "XGL SMP:"));
    assert(strstr(console_message, mode));
    assert(strstr(console_message, cpus));
    assert(strstr(console_message, status));
}

static void set_mode(int value)
{
    mode_variable->value = (float)value;
    XGL_SMPUpdate();
}

int XGL_ProfileDetailEnabled(void)
{
    assert(pthread_equal(pthread_self(), owner));
    return detail_enabled;
}

double XGL_ProfileDetailNow(void)
{
    assert(pthread_equal(pthread_self(), owner));
    return detail_enabled ? 1.0 : 0.0;
}

void XGL_ProfileAdd(xgl_profile_section_t section, double start)
{
    assert(pthread_equal(pthread_self(), owner));
    assert(section == XGL_PROFILE_SMP_WAIT);
    (void)start;
}

void XGL_ProfileSMPWork(unsigned long jobs, double seconds)
{
    assert(pthread_equal(pthread_self(), owner));
    assert(seconds >= 0);
    reported_jobs += jobs;
}

static pthread_mutex_t gate_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gate_changed = PTHREAD_COND_INITIALIZER;
static int entered, released;

static void hold_worker(void *unused)
{
    (void)unused;
    assert(!pthread_equal(pthread_self(), owner));
    pthread_mutex_lock(&gate_mutex);
    entered = 1;
    pthread_cond_signal(&gate_changed);
    while (!released) pthread_cond_wait(&gate_changed, &gate_mutex);
    pthread_mutex_unlock(&gate_mutex);
}

static int completed;
static void check_sequence(void *value)
{
    assert(!pthread_equal(pthread_self(), owner));
    assert(*(int *)value == completed);
    ++completed;
}

static void submit_eventually(void (*work)(void *), void *argument)
{
    while (!XGL_SMPSubmit(work, argument)) sched_yield();
}

static void test_queue(void)
{
    int tickets[4096], i, queued;

    detail_enabled = 1;
    submit_eventually(hold_worker, NULL);
    pthread_mutex_lock(&gate_mutex);
    while (!entered) pthread_cond_wait(&gate_changed, &gate_mutex);
    /* The worker has released its queue lock and is blocked in hold_worker. */
    for (queued = 0; queued < 4096; ++queued) {
        tickets[queued] = queued;
        if (!XGL_SMPSubmit(check_sequence, &tickets[queued])) break;
    }
    assert(queued > 0 && queued < 4096);
    assert(completed == 0);
    released = 1;
    pthread_cond_signal(&gate_changed);
    pthread_mutex_unlock(&gate_mutex);
    XGL_SMPWait();
    assert(completed == queued);
    assert(reported_jobs == (unsigned long)queued + 1);
    XGL_SMPWait();
    assert(reported_jobs == (unsigned long)queued + 1);
    detail_enabled = 0;

    /* Wrap the ring repeatedly. Switching off must finish all accepted work. */
    for (i = queued; i < 4096; ++i) {
        tickets[i] = i;
        submit_eventually(check_sequence, &tickets[i]);
    }
    set_mode(0);
    assert(!XGL_SMPActive());
    check_status("forced off", "1 online CPU", "single-threaded");
    assert(completed == 4096);
    assert(!XGL_SMPSubmit(check_sequence, &tickets[0]));
}

int main(void)
{
    int before;
    owner = pthread_self();
    /* Bound deadlock regressions instead of hanging the whole test suite. */
    alarm(30);
    online_cpus = 1;
    XGL_SMPInit();
    assert(mode_variable && !XGL_SMPActive() && !threads_created);
    check_status("auto", "1 online CPU", "single-threaded");
    XGL_SMPWait();
    set_mode(1);
    assert(XGL_SMPActive() && threads_created == 1);
    check_status("forced on", "1 online CPU", "one preparation worker active");
    test_queue();
    set_mode(-1);
    assert(!XGL_SMPActive());
    XGL_SMPShutdown();

    online_cpus = -1;
    XGL_SMPInit();
    check_status("auto", "online CPU count unavailable", "single-threaded");
    assert(!XGL_SMPActive());
    XGL_SMPShutdown();

    online_cpus = 2;
    creation_error = EAGAIN;
    XGL_SMPInit();
    check_status("auto", "2 online CPU", "single-threaded");
    assert(strstr(console_error, strerror(EAGAIN)));
    assert(!XGL_SMPActive());
    XGL_SMPWait();
    assert(!XGL_SMPSubmit(check_sequence, NULL));
    creation_error = 0;
    set_mode(0);
    set_mode(-1);
    assert(XGL_SMPActive());
    check_status("auto", "2 online CPU", "one preparation worker active");
    before = threads_created;
    XGL_SMPUpdate();
    assert(XGL_SMPActive() && threads_created == before);
    XGL_SMPShutdown();
    XGL_SMPShutdown();
    assert(!XGL_SMPActive());

    XGL_SMPInit();
    assert(XGL_SMPActive());
    XGL_SMPShutdown();
    alarm(0);
    puts("SMP worker tests passed (CPU detection, overrides, startup failure, FIFO, saturation, draining and restart)");
    return 0;
}
