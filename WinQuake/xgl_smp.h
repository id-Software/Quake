/* CPU preparation worker for the XGL renderer. GPL-2.0-or-later. */
#ifndef XGL_SMP_H
#define XGL_SMP_H

/* These functions belong to the main thread. Update applies cvar changes
 * at a frame boundary. Shutdown drains accepted jobs and joins the worker. */
void XGL_SMPInit(void);
void XGL_SMPUpdate(void);
void XGL_SMPShutdown(void);
int XGL_SMPActive(void);

/* Submit returns zero if the worker is off or the queue is busy/full. The
 * caller then owns the job and can run it inline. An accepted function runs
 * exactly once, on the worker, with no engine, X11, or XGL calls. Keep its
 * inputs stable and its outputs private until Wait returns. */
int XGL_SMPSubmit(void (*function)(void *), void *argument);
void XGL_SMPWait(void);

#endif
