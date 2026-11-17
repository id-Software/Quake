/* Single-process watchdog; GPL-2.0-or-later, see COPYING.
 * Solaris 2.6 POSIX signals and alarm(), with no worker thread or process.
 */
#if !defined(__sun) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif
#include "xglbench.h"
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t watchdog_active;
static volatile sig_atomic_t watchdog_expiration;
static struct sigaction previous_action;
static int previous_alarm_blocked;

static void watchdog_alarm(int signal_number)
{
    int saved_errno;
    (void)signal_number;
    saved_errno = errno;
    if (!watchdog_active) {
        errno = saved_errno;
        return;
    }
    if (!watchdog_expiration) {
        watchdog_expiration = 1;
        (void)alarm(10);
        errno = saved_errno;
        return;
    }
    /* The driver may be stuck. Do not call XGL, stdio, allocation, cleanup,
     * or jump out of the interrupted stack. Even an async-signal-safe write
     * can block on a full stderr pipe, so forced shutdown performs no I/O.
     */
    _exit(124);
}

int xb_watchdog_start(unsigned int seconds)
{
    struct sigaction action;
    sigset_t block, previous_mask, run_mask;
    int saved_errno;
    if (!seconds) {
        errno = EINVAL;
        return 0;
    }
    sigemptyset(&block);
    sigaddset(&block, SIGALRM);
    if (sigprocmask(SIG_BLOCK, &block, &previous_mask) != 0) return 0;
    if (watchdog_active) {
        (void)sigprocmask(SIG_SETMASK, &previous_mask, NULL);
        errno = EBUSY;
        return 0;
    }
    memset(&action, 0, sizeof(action));
    action.sa_handler = watchdog_alarm;
    sigemptyset(&action.sa_mask);
    /* No SA_RESTART: interrupted blocking calls can return to the main loop
     * for orderly shutdown during the grace period.
     */
    if (sigaction(SIGALRM, &action, &previous_action) != 0) {
        saved_errno = errno;
        (void)sigprocmask(SIG_SETMASK, &previous_mask, NULL);
        errno = saved_errno;
        return 0;
    }
    previous_alarm_blocked = sigismember(&previous_mask, SIGALRM) == 1;
    watchdog_expiration = 0;
    watchdog_active = 1;
    (void)alarm(seconds);
    run_mask = previous_mask;
    sigdelset(&run_mask, SIGALRM);
    if (sigprocmask(SIG_SETMASK, &run_mask, NULL) == 0) return 1;
    saved_errno = errno;
    (void)alarm(0);
    watchdog_active = watchdog_expiration = 0;
    (void)sigaction(SIGALRM, &previous_action, NULL);
    (void)sigprocmask(SIG_SETMASK, &previous_mask, NULL);
    errno = saved_errno;
    return 0;
}

void xb_watchdog_stop(void)
{
    struct sigaction ignore;
    sigset_t block, current_mask;
    int saved_errno;
    if (!watchdog_active) return;
    saved_errno = errno;
    sigemptyset(&block);
    sigaddset(&block, SIGALRM);
    if (sigprocmask(SIG_BLOCK, &block, &current_mask) != 0) return;
    (void)alarm(0);
    watchdog_active = watchdog_expiration = 0;
    /* Ignoring a signal discards pending instances. Otherwise an alarm
     * arriving just before stop could run the restored handler on unblock.
     */
    memset(&ignore, 0, sizeof(ignore));
    ignore.sa_handler = SIG_IGN;
    sigemptyset(&ignore.sa_mask);
    (void)sigaction(SIGALRM, &ignore, NULL);
    (void)sigaction(SIGALRM, &previous_action, NULL);
    if (previous_alarm_blocked) sigaddset(&current_mask, SIGALRM);
    else sigdelset(&current_mask, SIGALRM);
    (void)sigprocmask(SIG_SETMASK, &current_mask, NULL);
    errno = saved_errno;
}

int xb_watchdog_expired(void)
{
    return watchdog_expiration != 0;
}
