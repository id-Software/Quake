/* Watchdog contract tests; subprocesses exist only in this test program.
 * GPL-2.0-or-later, see COPYING.
 */
#if !defined(__sun) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#include "xglbench.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int failed;
static volatile sig_atomic_t previous_called;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "watchdog test failed: %s\n", message);
        failed = 1;
    }
}

static void previous_handler(int signal_number)
{
    (void)signal_number;
    previous_called = 1;
}

static double wall_time(void)
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (double)t.tv_sec + (double)t.tv_usec * 1.0e-6;
}

static int child_status(pid_t child)
{
    struct timeval delay;
    double deadline;
    pid_t result;
    int status;
    /* This is a hang guard, not an upper-bound timing assertion. A broken
     * alarm implementation must not leave the test suite waiting forever.
     */
    deadline = wall_time() + 20.0;
    do {
        result = waitpid(child, &status, WNOHANG);
        if (result == child) return status;
        if (result < 0 && errno != EINTR) break;
        delay.tv_sec = 0;
        delay.tv_usec = 10000;
        (void)select(0, NULL, NULL, NULL, &delay);
    } while (wall_time() < deadline);
    (void)kill(child, SIGKILL);
    while (waitpid(child, &status, 0) < 0 && errno == EINTR) { }
    check(0, "child watchdog test did not finish");
    return -1;
}

static void actual_expiry(void)
{
    sigset_t alarm_set, run_mask, restored_mask;
    double before;
    unsigned int grace;
    sigemptyset(&alarm_set);
    sigaddset(&alarm_set, SIGALRM);
    /* Start must make its alarm deliverable even with an inherited block. */
    if (sigprocmask(SIG_BLOCK, &alarm_set, NULL) != 0) _exit(1);
    before = wall_time();
    if (!xb_watchdog_start(1)) _exit(2);
    if (sigprocmask(SIG_BLOCK, &alarm_set, &run_mask) != 0) _exit(3);
    if (sigismember(&run_mask, SIGALRM) != 0) _exit(4);
    while (!xb_watchdog_expired()) (void)sigsuspend(&run_mask);
    if (wall_time() - before < .5) _exit(5);
    grace = alarm(0);
    if (!grace || grace > 10) _exit(6);
    xb_watchdog_stop();
    if (xb_watchdog_expired() || alarm(0)) _exit(7);
    if (sigprocmask(SIG_BLOCK, NULL, &restored_mask) != 0 ||
        sigismember(&restored_mask, SIGALRM) != 1) _exit(8);
    _exit(0);
}

static void forced_exit(void)
{
    if (!xb_watchdog_start(60)) _exit(1);
    if (raise(SIGALRM) != 0 || !xb_watchdog_expired()) _exit(2);
    /* The second delivery models the grace alarm without an eleven-second
     * test delay. The first delivery's real alarm is inspected separately.
     */
    (void)raise(SIGALRM);
    _exit(3);
}

int main(void)
{
    struct sigaction original, action, restored;
    sigset_t alarm_set, original_mask;
    pid_t child;
    int status, output[2], flags;
    ssize_t bytes;
    char fill[4096];

    errno = 0;
    check(!xb_watchdog_start(0) && errno == EINVAL, "zero timeout rejected");
    check(!xb_watchdog_expired(), "watchdog initially unexpired");
    sigemptyset(&alarm_set);
    sigaddset(&alarm_set, SIGALRM);
    check(sigprocmask(SIG_UNBLOCK, &alarm_set, &original_mask) == 0,
          "save signal mask");
    memset(&action, 0, sizeof(action));
    action.sa_handler = previous_handler;
    sigemptyset(&action.sa_mask);
    check(sigaction(SIGALRM, &action, &original) == 0, "install previous handler");
    check(xb_watchdog_start(60), "start watchdog");
    errno = 0;
    check(!xb_watchdog_start(60) && errno == EBUSY, "duplicate start rejected");
    check(sigprocmask(SIG_BLOCK, &alarm_set, NULL) == 0, "block pending alarm");
    check(raise(SIGALRM) == 0, "queue a pending watchdog signal");
    xb_watchdog_stop();
    check(!previous_called, "stop discarded pending watchdog alarm");
    check(!xb_watchdog_expired(), "stop cleared expiration flag");
    check(alarm(0) == 0, "stop canceled alarm timer");
    check(sigaction(SIGALRM, NULL, &restored) == 0 &&
          restored.sa_handler == previous_handler, "previous handler restored");
    check(raise(SIGALRM) == 0 && previous_called, "restored handler receives later signals");
    xb_watchdog_stop();
    check(xb_watchdog_start(60), "restart after stop");
    xb_watchdog_stop();
    check(sigaction(SIGALRM, &original, NULL) == 0, "restore original handler");
    check(sigprocmask(SIG_SETMASK, &original_mask, NULL) == 0, "restore original mask");

    child = fork();
    check(child >= 0, "fork real-alarm test");
    if (!child) actual_expiry();
    if (child > 0) {
        status = child_status(child);
        check(status >= 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0,
              "real alarm sets expiration, arms grace and restores inherited mask");
    }
    if (pipe(output) != 0) check(0, "create blocked-stderr pipe");
    else {
        /* Fill a pipe without a consumer, then restore blocking I/O before
         * assigning it to child stderr. A handler that writes a diagnostic
         * before _exit would hang here despite write being signal-safe.
         */
        flags = fcntl(output[1], F_GETFL, 0);
        if (flags < 0 || fcntl(output[1], F_SETFL, flags | O_NONBLOCK) < 0) {
            check(0, "make stderr pipe nonblocking for fixture fill");
            close(output[0]); close(output[1]);
            return 1;
        }
        memset(fill, 'x', sizeof(fill));
        do {
            bytes = write(output[1], fill, sizeof(fill));
        } while (bytes > 0 || (bytes < 0 && errno == EINTR));
        /* A failed atomic 4096-byte write can leave a smaller writable tail.
         * Exhaust it too, so even a one-byte handler diagnostic would block.
         */
        do {
            bytes = write(output[1], fill, 1);
        } while (bytes > 0 || (bytes < 0 && errno == EINTR));
        check(bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK), "stderr pipe filled");
        check(fcntl(output[1], F_SETFL, flags) == 0, "restore blocking stderr fixture");
        child = fork();
        check(child >= 0, "fork forced-exit test");
        if (!child) {
            close(output[0]);
            if (dup2(output[1], STDERR_FILENO) < 0) _exit(4);
            close(output[1]);
            forced_exit();
        }
        close(output[1]);
        if (child > 0) {
            status = child_status(child);
            check(status >= 0 && WIFEXITED(status) && WEXITSTATUS(status) == 124,
                  "second alarm forces exit 124 even with blocked stderr");
        }
        close(output[0]);
    }
    if (failed) return 1;
    puts("watchdog tests: real expiry, grace, forced exit, pending-signal cleanup and restoration passed");
    return 0;
}
