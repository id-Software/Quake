/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

Solaris /dev/audio support for XGLQuake, derived from snd_sun.c. The native
device, 11025 Hz linear format, and stereo-to-mono fallback are unchanged.
This target has its own bounded ring and nonblocking submission path so
slow graphics frames do not impose the original 1024-byte write limit.
*/
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/audioio.h>
#include <errno.h>
#include <string.h>
#include "quakedef.h"
#include "xgl_audio.h"
#include "xgl_profile.h"

#define XGL_AUDIO_BUFFER_BYTES 32768
#define XGL_AUDIO_EINTR_LIMIT 8
#define XGL_AUDIO_SERVICE_INTERVAL .010

static int xgl_audio_fd = -1;
static int xgl_audio_inited;
static audio_info_t xgl_audio_info;
static byte xgl_audio_buffer[XGL_AUDIO_BUFFER_BYTES];
static int xgl_audio_frame;
static int xgl_audio_byte;
static byte xgl_audio_partial[4];
static int xgl_audio_busy, xgl_audio_have_time, xgl_audio_locks;
static double xgl_audio_last_time;

extern int sound_started, soundtime;
extern cvar_t _snd_mixahead, snd_noextraupdate;

void XGL_AudioInit(void)
{
    /* This runs before shared S_Init. Cvar_RegisterVariable will copy this
     * initial string; subsequently executed config/console settings win.
     * Never replace an already registered variable or its allocated string. */
    if (!Cvar_FindVar("_snd_mixahead")) _snd_mixahead.string = "0.3";
    xgl_audio_busy = xgl_audio_have_time = xgl_audio_locks = 0;
}

void XGL_AudioUpdate(void)
{
    double now, start;
    if (!sound_started || snd_blocked > 0 || !xgl_audio_inited ||
        snd_noextraupdate.value || xgl_audio_busy || xgl_audio_locks) return;
    now = Sys_FloatTime();
    if (xgl_audio_have_time && now >= xgl_audio_last_time &&
        now - xgl_audio_last_time < XGL_AUDIO_SERVICE_INTERVAL) return;
    /* Sys_FloatTime uses the platform clock. A backward adjustment restarts
     * the interval rather than suppressing sound until the clock catches up. */
    xgl_audio_last_time = now;
    xgl_audio_have_time = 1;
    xgl_audio_busy = 1;
    start = XGL_ProfileNow();
    S_ExtraUpdate();
    XGL_ProfileAdd(XGL_PROFILE_AUDIO, start);
    xgl_audio_busy = 0;
}

void XGL_AudioLock(void)
{
    /* Service before the caller acquires a movable model-cache pointer.
     * S_ExtraUpdate can load an uncached sound and compact that same cache. */
    XGL_AudioUpdate();
    ++xgl_audio_locks;
}

void XGL_AudioUnlock(void)
{
    if (!xgl_audio_locks) return;
    --xgl_audio_locks;
    if (!xgl_audio_locks) XGL_AudioUpdate();
}

static void xgl_audio_fail(const char *operation, int error)
{
    if (xgl_audio_fd >= 0) close(xgl_audio_fd);
    xgl_audio_fd = -1;
    xgl_audio_inited = 0;
    sound_started = 0;
    if (shm) shm->soundalive = false;
    /* Sound servicing can happen inside rendering; avoid Con_Printf's
     * startup-screen recursion when a native audio operation fails. */
    Con_SafePrintf("XGL Solaris audio %s failed: %s\n", operation, strerror(error));
}

qboolean SNDDMA_Init(void)
{
    if (xgl_audio_inited) return true;
    shm = &sn;
    shm->splitbuffer = 0;
    shm->soundalive = false;
    xgl_audio_fd = open("/dev/audio", O_WRONLY | O_NDELAY);
    if (xgl_audio_fd < 0) {
        xgl_audio_fail("open /dev/audio", errno);
        return false;
    }
    if (ioctl(xgl_audio_fd, AUDIO_GETINFO, &xgl_audio_info) < 0) {
        xgl_audio_fail("AUDIO_GETINFO", errno);
        return false;
    }
    if (fcntl(xgl_audio_fd, F_SETFL, O_NONBLOCK) < 0) {
        xgl_audio_fail("nonblocking setup", errno);
        return false;
    }
    AUDIO_INITINFO(&xgl_audio_info);
    shm->speed = 11025;
    xgl_audio_info.play.encoding = AUDIO_ENCODING_LINEAR;
    xgl_audio_info.play.sample_rate = 11025;
    xgl_audio_info.play.channels = 2;
    xgl_audio_info.play.precision = 16;
    if (ioctl(xgl_audio_fd, AUDIO_SETINFO, &xgl_audio_info) < 0) {
        xgl_audio_info.play.encoding = AUDIO_ENCODING_LINEAR;
        xgl_audio_info.play.sample_rate = 11025;
        xgl_audio_info.play.channels = 1;
        xgl_audio_info.play.precision = 16;
        if (ioctl(xgl_audio_fd, AUDIO_SETINFO, &xgl_audio_info) < 0) {
            xgl_audio_fail("16-bit linear format selection", errno);
            return false;
        }
        shm->channels = 1;
        Con_Printf("16 bit mono sound initialized\n");
    } else {
        shm->channels = 2;
        Con_Printf("16 bit stereo sound initialized\n");
    }
    shm->samplebits = 16;
    shm->samples = XGL_AUDIO_BUFFER_BYTES / 2;
    shm->samplepos = 0;
    shm->submission_chunk = 1;
    shm->buffer = xgl_audio_buffer;
    memset(xgl_audio_buffer, 0, sizeof(xgl_audio_buffer));
    xgl_audio_frame = xgl_audio_byte = 0;
    xgl_audio_have_time = 0;
    xgl_audio_inited = 1;
    shm->soundalive = true;
    return true;
}

int SNDDMA_GetSamples(void)
{
    if (!xgl_audio_inited) return 0;
    if (ioctl(xgl_audio_fd, AUDIO_GETINFO, &xgl_audio_info) < 0) {
        xgl_audio_fail("AUDIO_GETINFO", errno);
        return 0;
    }
    return (int)xgl_audio_info.play.samples;
}

int SNDDMA_GetDMAPos(void)
{
    unsigned int frames;
    if (!xgl_audio_inited) return 0;
    frames = (unsigned int)SNDDMA_GetSamples();
    return (int)((frames * (unsigned int)shm->channels) % (unsigned int)shm->samples);
}

void SNDDMA_Shutdown(void)
{
    if (xgl_audio_fd >= 0) close(xgl_audio_fd);
    xgl_audio_fd = -1;
    xgl_audio_inited = 0;
    xgl_audio_frame = xgl_audio_byte = 0;
    xgl_audio_have_time = 0;
    if (shm) shm->soundalive = false;
}

static int xgl_audio_write(const byte *data, int bytes)
{
    ssize_t written;
    int attempts, error;
    for (attempts = 0; attempts < XGL_AUDIO_EINTR_LIMIT; ++attempts) {
        written = write(xgl_audio_fd, data, (size_t)bytes);
        if (written >= 0) return (int)written;
        error = errno;
        if (error == EINTR) continue;
        if (error == EAGAIN || error == EWOULDBLOCK) return 0;
        xgl_audio_fail("write", error);
        return 0;
    }
    return 0;
}

void SNDDMA_Submit(void)
{
    int frame_bytes, ring_frames, oldest, bytes, index, written;
    if (!xgl_audio_inited || !shm) return;
    frame_bytes = shm->channels * (shm->samplebits / 8);
    ring_frames = XGL_AUDIO_BUFFER_BYTES / frame_bytes;

    /* Finish any partially accepted PCM frame from a saved copy before
     * discarding old ring data. Otherwise a short byte write followed by a
     * mixer-ring overwrite could shift stereo channels or sample bytes. */
    for (;;) {
        while (xgl_audio_byte) {
            written = xgl_audio_write(xgl_audio_partial + xgl_audio_byte,
                                      frame_bytes - xgl_audio_byte);
            if (!written) return;
            xgl_audio_byte += written;
            if (xgl_audio_byte == frame_bytes) {
                ++xgl_audio_frame;
                xgl_audio_byte = 0;
            }
        }
        oldest = paintedtime - ring_frames;
        if (oldest < soundtime) oldest = soundtime;
        if (oldest > paintedtime) return;
        if (xgl_audio_frame < oldest || xgl_audio_frame > paintedtime)
            xgl_audio_frame = oldest;
        if (xgl_audio_frame == paintedtime) return;
        index = (xgl_audio_frame & (ring_frames - 1)) * frame_bytes;
        bytes = (paintedtime - xgl_audio_frame) * frame_bytes;
        if (bytes > XGL_AUDIO_BUFFER_BYTES - index)
            bytes = XGL_AUDIO_BUFFER_BYTES - index;
        written = xgl_audio_write(xgl_audio_buffer + index, bytes);
        if (!written) return;
        xgl_audio_frame += written / frame_bytes;
        xgl_audio_byte = written % frame_bytes;
        if (xgl_audio_byte) {
            memcpy(xgl_audio_partial,
                xgl_audio_buffer + index + written - xgl_audio_byte,
                (size_t)frame_bytes);
            /* Keep this tail stable if backpressure postpones completion
             * until after the next mixer update wraps around the ring. */
        }
    }
}
