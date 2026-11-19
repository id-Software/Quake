/* XGL renderer audio service hooks. GPL-2.0-or-later. */
#ifndef XGL_AUDIO_H
#define XGL_AUDIO_H

/* Call during R_Init, before the shared S_Init registers sound variables. */
void XGL_AudioInit(void);
/* Safe before sound startup, with -nosound, and inside renderer batches. */
void XGL_AudioUpdate(void);
/* Nestable deferral, not a thread mutex. Sound mixing can allocate/move
 * Quake cache blocks. Lock before acquiring a raw movable-cache pointer,
 * then unlock after its last use; queued audio continues playing. */
void XGL_AudioLock(void);
void XGL_AudioUnlock(void);

#endif
