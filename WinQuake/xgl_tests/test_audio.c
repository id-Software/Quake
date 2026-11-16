/* Solaris audio adapter tests. Native IO is simulated; no device is opened. */
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/audioio.h>

static int test_open(const char *path, int flags, ...);
static int test_close(int fd);
static int test_fcntl(int fd, int command, ...);
static int test_ioctl(int fd, unsigned long command, ...);
static ssize_t test_write(int fd, const void *data, size_t count);
#define open test_open
#define close test_close
#define fcntl test_fcntl
#define ioctl test_ioctl
#define write test_write
#include "../xgl_snd_sun.c"
#undef open
#undef close
#undef fcntl
#undef ioctl
#undef write

volatile dma_t sn, *shm;
int sound_started, soundtime, paintedtime, snd_blocked;
cvar_t _snd_mixahead = {"_snd_mixahead", "0.1", true, false, 0, NULL};
cvar_t snd_noextraupdate = {"snd_noextraupdate", "0", false, false, 0, NULL};

static int mixahead_registered, close_calls, write_calls, extra_calls, clock_calls;
static int open_failure, fcntl_failure, get_failure, stereo_failure, all_formats_fail;
static int selected_channels, format_calls, warnings, recurse_update;
static int hardware_frames, queue_capacity, max_queued;
static int script[64], script_count, script_pos;
static byte captured[524288];
static int captured_count;
static double clock_time;

void Con_Printf(char *format, ...) { (void)format; }
void Con_SafePrintf(char *format, ...) { (void)format; ++warnings; }
cvar_t *Cvar_FindVar(char *name)
{
    assert(!strcmp(name, "_snd_mixahead"));
    return mixahead_registered ? &_snd_mixahead : NULL;
}
double Sys_FloatTime(void) { ++clock_calls; return clock_time; }
double XGL_ProfileNow(void) { return 0; }
void XGL_ProfileAdd(xgl_profile_section_t section, double start)
{
    assert(section == XGL_PROFILE_AUDIO && start == 0);
}
void S_ExtraUpdate(void)
{
    ++extra_calls;
    if (recurse_update) XGL_AudioUpdate();
}

static int test_open(const char *path, int flags, ...)
{
    assert(!strcmp(path, "/dev/audio"));
    assert(flags == (O_WRONLY | O_NDELAY));
    if (open_failure) { errno = EBUSY; return -1; }
    return 7;
}
static int test_close(int fd) { assert(fd == 7); ++close_calls; return 0; }
static int test_fcntl(int fd, int command, ...)
{
    va_list args;
    assert(fd == 7 && command == F_SETFL);
    va_start(args, command);
    assert(va_arg(args, int) == O_NONBLOCK);
    va_end(args);
    if (fcntl_failure) { errno = EIO; return -1; }
    return 0;
}
static int test_ioctl(int fd, unsigned long command, ...)
{
    va_list args;
    audio_info_t *info;
    assert(fd == 7);
    va_start(args, command);
    info = va_arg(args, audio_info_t *);
    va_end(args);
    if (command == AUDIO_GETINFO) {
        if (get_failure) { errno = EIO; return -1; }
        info->play.samples = (unsigned int)hardware_frames;
        return 0;
    }
    assert(command == AUDIO_SETINFO);
    ++format_calls;
    assert(info->play.encoding == AUDIO_ENCODING_LINEAR);
    assert(info->play.sample_rate == 11025 && info->play.precision == 16);
    assert(info->play.channels == 1 || info->play.channels == 2);
    if (all_formats_fail || (stereo_failure && info->play.channels == 2)) {
        errno = EINVAL;
        return -1;
    }
    selected_channels = (int)info->play.channels;
    return 0;
}
static ssize_t test_write(int fd, const void *data, size_t count)
{
    int take, queued, available;
    assert(fd == 7 && count > 0 && count <= XGL_AUDIO_BUFFER_BYTES);
    ++write_calls;
    take = (int)count;
    if (script_pos < script_count) {
        take = script[script_pos++];
        if (take < 0) { errno = -take; return -1; }
        if ((size_t)take > count) take = (int)count;
    }
    queued = captured_count - hardware_frames * selected_channels * 2;
    if (queue_capacity) {
        available = queue_capacity - queued;
        if (available <= 0) { errno = EAGAIN; return -1; }
        if (take > available) take = available;
    }
    assert(captured_count + take <= (int)sizeof(captured));
    memcpy(captured + captured_count, data, (size_t)take);
    captured_count += take;
    queued += take;
    if (queued > max_queued) max_queued = queued;
    return take;
}

static void reset_test(void)
{
    SNDDMA_Shutdown();
    memset((void *)&sn, 0, sizeof(sn));
    shm = NULL;
    sound_started = soundtime = paintedtime = snd_blocked = 0;
    mixahead_registered = close_calls = write_calls = extra_calls = clock_calls = 0;
    open_failure = fcntl_failure = get_failure = stereo_failure = all_formats_fail = 0;
    selected_channels = format_calls = warnings = recurse_update = 0;
    hardware_frames = queue_capacity = max_queued = 0;
    script_count = script_pos = captured_count = 0;
    clock_time = 0;
    _snd_mixahead.string = "0.1";
    _snd_mixahead.value = 0;
    snd_noextraupdate.value = 0;
}

static byte sample_byte(int absolute)
{
    return (byte)((absolute * 17 + 23) ^ (absolute >> 11));
}

static void mix_range(int first, int last)
{
    int i, bytes;
    bytes = shm->channels * (shm->samplebits / 8);
    for (i = first * bytes; i < last * bytes; ++i)
        shm->buffer[i & (XGL_AUDIO_BUFFER_BYTES - 1)] = sample_byte(i);
    paintedtime = last;
}

static void check_capture(int offset, int first_byte, int count)
{
    int i;
    assert(offset + count <= captured_count);
    for (i = 0; i < count; ++i)
        assert(captured[offset + i] == sample_byte(first_byte + i));
}

static void start_audio(int mono)
{
    reset_test();
    XGL_AudioInit();
    assert(!strcmp(_snd_mixahead.string, "0.3"));
    mixahead_registered = 1;
    _snd_mixahead.value = (float)atof(_snd_mixahead.string);
    stereo_failure = mono;
    assert(SNDDMA_Init());
    sound_started = 1;
    assert(shm == &sn && shm->soundalive && shm->buffer);
    assert(shm->speed == 11025 && shm->samplebits == 16);
    assert(shm->channels == (mono ? 1 : 2));
    assert(format_calls == (mono ? 2 : 1));
    assert(shm->samples * (shm->samplebits / 8) == XGL_AUDIO_BUFFER_BYTES);
    assert(SNDDMA_Init() && format_calls == (mono ? 2 : 1));
}

static void test_frame_cadence(void)
{
    int i, end, previous, ahead, bytes;
    start_audio(0);
    bytes = shm->channels * 2;
    queue_capacity = XGL_AUDIO_BUFFER_BYTES;
    ahead = (int)(_snd_mixahead.value * shm->speed);
    previous = 0;
    for (i = 0; i < 24; ++i) {
        /* Each render stalls for 200 ms. The previous submission must cover
         * playback up to the next update without extra service calls. */
        hardware_frames = i * 2205;
        if (i) assert(captured_count >= hardware_frames * bytes);
        soundtime = SNDDMA_GetSamples();
        end = soundtime + ahead;
        mix_range(previous, end);
        previous = end;
        SNDDMA_Submit();
        assert(xgl_audio_frame == paintedtime && !xgl_audio_byte);
        assert(captured_count == paintedtime * bytes);
        assert(captured_count - hardware_frames * bytes == ahead * bytes);
    }
    check_capture(0, 0, captured_count);
    assert(max_queued <= XGL_AUDIO_BUFFER_BYTES);
    assert(max_queued > 1024 && max_queued >= (int)(.29 * shm->speed) * bytes);
}

static void test_partial_and_backpressure(void)
{
    int previous_count;
    start_audio(0);
    mix_range(0, 3000);
    script[0] = -EINTR; script[1] = 3; script[2] = -EAGAIN;
    script_count = 3;
    SNDDMA_Submit();
    assert(captured_count == 3 && xgl_audio_frame == 0 && xgl_audio_byte == 3);
    script_count = script_pos = 0;
    SNDDMA_Submit();
    assert(captured_count == 12000 && xgl_audio_frame == 3000 && !xgl_audio_byte);
    check_capture(0, 0, captured_count);
    previous_count = write_calls;
    SNDDMA_Submit();
    assert(write_calls == previous_count);

    mix_range(3000, 3500);
    script[0] = 0; script_count = 1; script_pos = 0;
    SNDDMA_Submit();
    assert(captured_count == 12000 && xgl_audio_frame == 3000);
    script_count = script_pos = 0;
    SNDDMA_Submit();
    assert(captured_count == 14000);
    check_capture(0, 0, captured_count);
}

static void test_bounded_queue_and_eintr(void)
{
    int i, calls;
    start_audio(0);
    mix_range(0, 3000);
    queue_capacity = 1000;
    SNDDMA_Submit();
    assert(captured_count == 1000 && xgl_audio_frame == 250);
    hardware_frames = soundtime = 250;
    SNDDMA_Submit();
    assert(captured_count == 2000 && max_queued == 1000);
    queue_capacity = XGL_AUDIO_BUFFER_BYTES;
    for (i = 0; i < XGL_AUDIO_EINTR_LIMIT; ++i) script[i] = -EINTR;
    script_count = XGL_AUDIO_EINTR_LIMIT;
    calls = write_calls;
    SNDDMA_Submit();
    assert(write_calls == calls + XGL_AUDIO_EINTR_LIMIT);
    assert(captured_count == 2000 && xgl_audio_frame == 500);
    script_count = script_pos = 0;
    SNDDMA_Submit();
    assert(captured_count == 12000);
    check_capture(0, 0, captured_count);
}

static void test_ring_wrap_and_stale_data(void)
{
    int frames, first, remaining;
    start_audio(0);
    frames = XGL_AUDIO_BUFFER_BYTES / 4;
    xgl_audio_frame = soundtime = frames - 3;
    mix_range(frames - 3, frames + 9);
    SNDDMA_Submit();
    assert(captured_count == 48 && write_calls == 2);
    check_capture(0, (frames - 3) * 4, 48);

    start_audio(0);
    frames = XGL_AUDIO_BUFFER_BYTES / 4;
    mix_range(0, 10);
    script[0] = 1; script[1] = -EAGAIN; script_count = 2;
    SNDDMA_Submit();
    assert(captured_count == 1 && xgl_audio_byte == 1);
    /* The mixer overwrites even the partially submitted original frame.
     * Its saved last three bytes must complete the PCM frame unchanged. */
    mix_range(10, frames + 100);
    soundtime = 200;
    script_count = script_pos = 0;
    SNDDMA_Submit();
    first = soundtime;
    remaining = (paintedtime - first) * 4;
    assert(captured_count == 4 + remaining);
    check_capture(0, 0, 4);
    check_capture(4, first * 4, remaining);
    assert(xgl_audio_frame == paintedtime && !xgl_audio_byte);

    start_audio(0);
    frames = XGL_AUDIO_BUFFER_BYTES / 4;
    mix_range(0, frames + 50);
    SNDDMA_Submit();
    assert(captured_count == XGL_AUDIO_BUFFER_BYTES);
    check_capture(0, 50 * 4, captured_count);
    /* Engine sample time can restart: resume at its current playback time. */
    captured_count = 0;
    soundtime = 20;
    mix_range(20, 60);
    SNDDMA_Submit();
    assert(captured_count == 160 && xgl_audio_frame == 60);
    check_capture(0, 80, 160);
}

static void test_mono_and_failures(void)
{
    int frames;
    start_audio(1);
    frames = XGL_AUDIO_BUFFER_BYTES / 2;
    xgl_audio_frame = soundtime = frames - 1;
    mix_range(frames - 1, frames + 5);
    script[0] = 1; script_count = 1;
    SNDDMA_Submit();
    assert(captured_count == 12 && !xgl_audio_byte);
    check_capture(0, (frames - 1) * 2, 12);
    hardware_frames = frames + 5;
    assert(SNDDMA_GetSamples() == frames + 5);
    assert(SNDDMA_GetDMAPos() == 5);

    start_audio(0);
    mix_range(0, 1000);
    script[0] = -EIO; script_count = 1;
    SNDDMA_Submit();
    assert(!captured_count && !xgl_audio_frame && !xgl_audio_byte);
    assert(!xgl_audio_inited && !sound_started && !shm->soundalive);
    assert(close_calls == 1 && warnings == 1);
    SNDDMA_Submit();
    assert(write_calls == 1);

    start_audio(0);
    get_failure = 1;
    assert(!SNDDMA_GetSamples());
    assert(!xgl_audio_inited && !sound_started && close_calls == 1);
    reset_test(); open_failure = 1;
    assert(!SNDDMA_Init() && !close_calls && !xgl_audio_inited);
    reset_test(); fcntl_failure = 1;
    assert(!SNDDMA_Init() && close_calls == 1 && !xgl_audio_inited);
    reset_test(); all_formats_fail = 1;
    assert(!SNDDMA_Init() && format_calls == 2 && close_calls == 1);
}

static void test_service_and_settings(void)
{
    int before;
    reset_test();
    XGL_AudioInit();
    XGL_AudioUpdate();
    assert(!clock_calls && !extra_calls);
    mixahead_registered = 1;
    _snd_mixahead.string = "0.05"; _snd_mixahead.value = .05f;
    XGL_AudioInit();
    assert(!strcmp(_snd_mixahead.string, "0.05") && _snd_mixahead.value == .05f);
    assert(SNDDMA_Init()); sound_started = 1;
    clock_time = 1;
    recurse_update = 1;
    XGL_AudioUpdate();
    assert(extra_calls == 1 && clock_calls == 1);
    clock_time = 1.005;
    XGL_AudioUpdate(); assert(extra_calls == 1);
    clock_time = 1.011;
    XGL_AudioUpdate(); assert(extra_calls == 2);
    clock_time = .5;
    XGL_AudioUpdate(); assert(extra_calls == 3);
    before = clock_calls;
    snd_blocked = 1; XGL_AudioUpdate();
    snd_blocked = 0; sound_started = 0; XGL_AudioUpdate();
    sound_started = 1; snd_noextraupdate.value = 1; XGL_AudioUpdate();
    assert(clock_calls == before && extra_calls == 3);
    snd_noextraupdate.value = 0;
    SNDDMA_Shutdown(); XGL_AudioUpdate();
    assert(clock_calls == before && !shm->soundalive);
}

static void test_cache_pointer_deferral(void)
{
    int before;
    start_audio(0);
    clock_time = 1;
    XGL_AudioLock();
    assert(xgl_audio_locks == 1 && extra_calls == 1);
    before = clock_calls;
    clock_time = 2;
    XGL_AudioUpdate();
    XGL_AudioLock();
    XGL_AudioUpdate();
    assert(xgl_audio_locks == 2 && extra_calls == 1 && clock_calls == before);
    XGL_AudioUnlock();
    XGL_AudioUpdate();
    assert(xgl_audio_locks == 1 && extra_calls == 1 && clock_calls == before);
    XGL_AudioUnlock();
    assert(!xgl_audio_locks && extra_calls == 2 && clock_calls == before + 1);
    XGL_AudioUnlock();
    assert(!xgl_audio_locks && extra_calls == 2 && clock_calls == before + 1);

    clock_time = 3;
    XGL_AudioLock();
    assert(xgl_audio_locks == 1 && extra_calls == 3);
    XGL_AudioInit();
    assert(!xgl_audio_locks);
    XGL_AudioUpdate();
    assert(extra_calls == 4);
}

int main(void)
{
    test_frame_cadence();
    test_partial_and_backpressure();
    test_bounded_queue_and_eintr();
    test_ring_wrap_and_stale_data();
    test_mono_and_failures();
    test_service_and_settings();
    test_cache_pointer_deferral();
    puts("XGL Solaris audio: slow frames, full drain, partial IO, ring bounds and service hooks passed (simulated device)");
    return 0;
}
