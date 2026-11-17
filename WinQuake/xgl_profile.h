/* Optional XGL renderer profiling. GPL-2.0-or-later. */
#ifndef XGL_PROFILE_H
#define XGL_PROFILE_H

typedef enum {
    XGL_PROFILE_WORLD,
    XGL_PROFILE_ENTITIES,
    XGL_PROFILE_EFFECTS,
    XGL_PROFILE_VIEWMODEL,
    XGL_PROFILE_UI,
    XGL_PROFILE_AUDIO,
    XGL_PROFILE_SUBMIT,
    XGL_PROFILE_SYNC,
    XGL_PROFILE_SWAP,
    XGL_PROFILE_SECTION_COUNT
} xgl_profile_section_t;

typedef enum {
    XGL_PROFILE_TRIANGLES,
    XGL_PROFILE_GLYPHS,
    XGL_PROFILE_POLYGON_CALLS,
    XGL_PROFILE_WAIT_CALLS,
    XGL_PROFILE_COUNTER_COUNT
} xgl_profile_counter_t;

void XGL_ProfileInit(void);
void XGL_ProfileBeginFrame(void);
void XGL_ProfileEndFrame(void);
double XGL_ProfileNow(void);
void XGL_ProfileAdd(xgl_profile_section_t section, double start);
void XGL_ProfileCount(xgl_profile_counter_t counter, unsigned long count);

#endif
