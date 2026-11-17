/* CPU indexed-material regression tests. GPL-2.0-or-later. */
#include "quakedef.h"
#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>

byte xgl_palette[768];
static jmp_buf error_jump;
static int expect_error;
static char error_message[256];
void XGL_AudioUpdate(void) {}
void XGL_TextureMarkMap (int id);
void XGL_TextureClearMap (void);

void Sys_Error (char *format, ...)
{
    va_list args;
    va_start (args, format);
    vsnprintf (error_message, sizeof(error_message), format, args);
    va_end (args);
    if (expect_error) longjmp (error_jump, 1);
    fprintf (stderr, "unexpected engine error: %s\n", error_message);
    abort ();
}

static void sample_is (int id, float s, float t, int index, const byte *translation)
{
    float rgb[3];
    int channel, sampled;
    assert (XGL_SampleTextureIndex (id, s, t, &sampled));
    assert ((translation ? translation[sampled] : sampled) == index);
    assert (XGL_SampleTexture (id, s, t, rgb, translation));
    for (channel = 0; channel < 3; ++channel)
        assert (fabs(rgb[channel] - xgl_palette[index*3+channel]/255.0) < .000001);
}

static void test_index_samples(void)
{
    byte pixels[6] = {0, 1, 2, 3, 4, 255};
    byte translation[256];
    float rgb[3], s, t;
    int opaque, cutout, x, y, index, expected, i;
    opaque = XGL_LoadTexture("index-opaque", 3, 2, pixels, false, false);
    cutout = XGL_LoadTexture("index-cutout", 3, 2, pixels, false, true);
    for (i = 0; i < 256; ++i) translation[i] = (byte)(255 - i);
    /* Include exact tile boundaries, negative repetition, and non-power-of-2
     * dimensions; compare the direct index and both legacy palette paths. */
    for (y = -4; y <= 4; ++y)
        for (x = -8; x <= 8; ++x) {
            s = x / 4.0f; t = y / 4.0f;
            expected = pixels[((y % 4 + 4) % 4) / 2 * 3 + ((x % 4 + 4) % 4) * 3 / 4];
            assert(XGL_SampleTextureIndex(opaque, s, t, &index));
            assert(index == expected);
            sample_is(opaque, s, t, expected, NULL);
            sample_is(opaque, s, t, translation[expected], translation);
            index = -1;
            if (expected == 255) {
                assert(!XGL_SampleTextureIndex(cutout, s, t, &index));
                assert(index == -1);
                assert(!XGL_SampleTexture(cutout, s, t, rgb, translation));
            } else {
                assert(XGL_SampleTextureIndex(cutout, s, t, &index));
                assert(index == expected);
            }
        }
    index = -1;
    assert(!XGL_SampleTextureIndex(0, 0, 0, &index));
    assert(!XGL_SampleTextureIndex(-1, 0, 0, &index));
    assert(!XGL_SampleTextureIndex(100000, 0, 0, &index));
    assert(!XGL_SampleTextureIndex(opaque, (float)HUGE_VAL, 0, &index));
    assert(!XGL_SampleTextureIndex(opaque, 0, -(float)HUGE_VAL, &index));
    assert(!XGL_SampleTextureIndex(opaque, (float)strtod("nan", NULL), 0, &index));
    assert(!XGL_SampleTextureIndex(opaque, 0, (float)strtod("nan", NULL), &index));
    assert(!XGL_SampleTextureIndex(opaque, 1e9f, 0, &index));
    assert(!XGL_SampleTextureIndex(opaque, 0, -1e9f, &index));
    assert(!XGL_SampleTextureIndex(opaque, 0, 0, NULL));
    assert(!XGL_SampleTexture(opaque, 0, 0, NULL, NULL));
    assert(index == -1);
    XGL_TextureShutdown();
    assert(!XGL_SampleTextureIndex(opaque, 0, 0, &index));
    assert(index == -1);
}

static void test_filtered_materials(void)
{
    byte checker[16], odd[3] = {0, 0, 255}, opaque[4] = {1, 1, 1, 1};
    byte translation[256];
    float rgb[3], wrapped[3], expected;
    unsigned int revision;
    int id, transparent, i, channel;
    for (i = 0; i < 16; ++i) checker[i] = ((i / 4 + i % 4) & 1) ? 255 : 0;
    id = XGL_LoadTexture("checker", 4, 4, checker, true, false);
    revision = XGL_TextureRevision(id);
    assert(revision && !XGL_TextureHasAlpha(id));
    assert(XGL_SampleTextureFiltered(id, .125f, .125f, 1, 1, rgb, NULL));
    for (channel = 0; channel < 3; ++channel) {
        expected = (xgl_palette[channel] + xgl_palette[255 * 3 + channel]) / 510.0f;
        assert(fabs(rgb[channel] - expected) <= 1.0 / 255);
    }
    assert(XGL_SampleTextureFiltered(id, -.125f, 2.125f, .5f, .5f, wrapped, NULL));
    for (channel = 0; channel < 3; ++channel) assert(fabs(rgb[channel] - wrapped[channel]) < .00001);
    assert(XGL_SampleTextureFiltered(id, .125f, .125f, .1f, .1f, rgb, NULL));
    for (channel = 0; channel < 3; ++channel)
        assert(fabs(rgb[channel] - xgl_palette[channel] / 255.0f) < .00001);
    assert(XGL_LoadTexture("checker", 4, 4, checker, true, false) == id);
    assert(XGL_TextureRevision(id) == revision);
    checker[0] = 17;
    assert(XGL_LoadTexture("checker", 4, 4, checker, true, false) == id);
    assert(XGL_TextureRevision(id) != revision);
    assert(XGL_SampleTextureFiltered(id, .125f, .125f, .1f, .1f, rgb, NULL));
    for (channel = 0; channel < 3; ++channel)
        assert(fabs(rgb[channel] - xgl_palette[17 * 3 + channel] / 255.0f) < .00001);
    for (i = 0; i < 256; ++i) translation[i] = (byte)i;
    translation[0] = translation[255] = translation[17] = 29;
    assert(XGL_SampleTextureFiltered(id, .5f, .5f, 1, 1, rgb, translation));
    for (channel = 0; channel < 3; ++channel)
        assert(fabs(rgb[channel] - xgl_palette[29 * 3 + channel] / 255.0f) < .00001);
    transparent = XGL_LoadTexture("cutout", 4, 4, checker, true, true);
    assert(XGL_TextureHasAlpha(transparent));
    assert(!XGL_SampleTextureFiltered(transparent, .375f, .125f, 1, 1, rgb, NULL));
    assert(!XGL_SampleTextureFiltered(transparent, .375f, .125f, 1, 1, rgb, translation));
    assert(XGL_SampleTextureFiltered(transparent, .125f, .125f, 1, 1, rgb, NULL));
    id = XGL_LoadTexture("opaque-cutout", 2, 2, opaque, true, true);
    assert(!XGL_TextureHasAlpha(id));
    id = XGL_LoadTexture("odd", 3, 1, odd, true, false);
    assert(XGL_SampleTextureFiltered(id, .5f, .5f, 10, 10, rgb, NULL));
    for (channel = 0; channel < 3; ++channel) {
        expected = (2 * xgl_palette[channel] + xgl_palette[255 * 3 + channel]) / 765.0f;
        assert(fabs(rgb[channel] - expected) <= 1.0 / 255);
    }
    assert(!XGL_TextureRevision(0) && !XGL_TextureHasAlpha(0));
    XGL_TextureShutdown();
}

int main (void)
{
    byte pixels[8] = {0, 1, 255, 7, 8, 9, 10, 11};
    byte translate[256];
    byte *large;
    float rgb[3];
    int alias, sprite, replacement, i;
    unsigned int revision;
    char name[32];
    for (i = 0; i < 256; ++i)
    {
        xgl_palette[i*3] = i;
        xgl_palette[i*3+1] = 255-i;
        xgl_palette[i*3+2] = (i*37)&255;
        translate[i] = i;
    }
    translate[1] = 17;
    XGL_TextureInit ();
    alias = XGL_LoadTexture ("progs/player.mdl_0", 4, 2, pixels, true, false);
    assert (alias > 0);
    assert (alias == XGL_LoadTexture ("progs/player.mdl_0", 4, 2, pixels, true, false));
    sample_is (alias, .375f, .25f, 1, NULL);
    sample_is (alias, -.625f, 2.25f, 1, NULL);
    sample_is (alias, 1.375f, -.75f, 17, translate);
    sample_is (alias, .625f, .25f, 255, NULL); /* opaque skins retain index 255 */
    pixels[1] = 29;
    sample_is (alias, .375f, .25f, 1, NULL); /* registry owns its copy */
    assert (alias == XGL_LoadTexture ("progs/player.mdl_0", 4, 2, pixels, false, false));
    sample_is (alias, .375f, .25f, 29, NULL);
    sprite = XGL_LoadTexture ("progs/flame.spr:group0:0", 4, 2, pixels, true, true);
    revision = XGL_TextureRevision(sprite);
    assert(XGL_TextureHasAlpha(sprite) && !XGL_TextureHasAlpha(alias));
    XGL_TextureMarkMap (sprite);
    assert (!XGL_SampleTexture (sprite, .625f, .25f, rgb, NULL));
    translate[255] = 0;
    assert (!XGL_SampleTexture (sprite, .625f, .25f, rgb, translate));
    sample_is (sprite, .875f, .75f, 11, NULL);
    assert (!XGL_SampleTexture (0, 0, 0, rgb, NULL));
    assert (!XGL_SampleTexture (alias, (float)HUGE_VAL, 0, rgb, NULL));
    XGL_TextureClearMap ();
    XGL_TextureClearMap ();
    assert (!XGL_SampleTexture (sprite, 0, 0, rgb, NULL));
    sample_is (alias, .375f, .25f, 29, NULL);
    XGL_TextureInit ();
    sample_is (alias, .375f, .25f, 29, NULL);
    replacement = XGL_LoadTexture ("maps/next.bsp:brick", 4, 2, pixels, true, false);
    assert (replacement == sprite && replacement != alias);
    assert(XGL_TextureRevision(replacement) != revision);
    XGL_TextureMarkMap (replacement);
    XGL_TextureClearMap ();
    sample_is (alias, .375f, .25f, 29, NULL);
    XGL_TextureShutdown ();
    assert (!XGL_SampleTexture (alias, 0, 0, rgb, NULL));

    test_index_samples();
    test_filtered_materials();

    /* Exact budget exhaustion must fail without evicting live alias IDs. */
    large = (byte *)malloc (1024*1024);
    assert (large);
    memset (large, 7, 1024*1024);
    for (i = 0; i < 8; ++i)
    {
        sprintf (name, "large%d", i);
        assert (XGL_LoadTexture (name, 1024, 1024, large, false, false) == i+1);
        if (i == 6) {
            /* This optional mip chain nearly fills the remaining MiB. The
             * eighth indexed asset must evict it rather than fail loading. */
            assert(XGL_SampleTextureFiltered(1, .5f, .5f, 1, 1, rgb, NULL));
        }
    }
    expect_error = 1;
    if (!setjmp(error_jump))
    {
        XGL_LoadTexture ("over-budget", 1, 1, large, false, false);
        assert (!"material budget was not enforced");
    }
    assert (strstr(error_message, "8 MiB"));
    expect_error = 0;
    sample_is (1, 0, 0, 7, NULL);
    sample_is (8, 0, 0, 7, NULL);
    assert(XGL_SampleTextureFiltered(1, .5f, .5f, 1, 1, rgb, NULL));
    for (i = 0; i < 3; ++i) assert(fabs(rgb[i] - xgl_palette[7*3+i]/255.0f) < .00001);
    expect_error = 1;
    if (!setjmp(error_jump))
    {
        XGL_LoadTexture ("oversize", 2048, 2048, large, false, false);
        assert (!"image size bound was not enforced");
    }
    assert (strstr(error_message, "dimensions"));
    expect_error = 0;
    free (large);
    XGL_TextureShutdown ();
    XGL_TextureShutdown ();
    puts ("XGL CPU materials: palette, translation, coverage, repeat, lifecycle and bounds passed");
    return 0;
}
