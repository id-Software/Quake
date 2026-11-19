/* Handwritten Solaris audio compile contract. No native ABI or IO emulation. */
#ifndef XGL_TEST_AUDIOIO_H
#define XGL_TEST_AUDIOIO_H
#include <string.h>
typedef struct {
    struct { unsigned int encoding, sample_rate, channels, precision, samples; } play;
} audio_info_t;
#define AUDIO_GETINFO 1
#define AUDIO_SETINFO 2
#define AUDIO_ENCODING_LINEAR 3
#define AUDIO_INITINFO(info) memset((info), 0xff, sizeof(*(info)))
#endif
