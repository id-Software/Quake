/* Particle output contracts; no native device or particle simulation. */
#include <assert.h>
#include <float.h>
#include "../xgl_effects.c"

vec3_t r_origin, vpn, vright, vup;
byte xgl_palette[768], gammatable[256];
float v_blend[4];
refdef_t r_refdef;

#define TEST_BATCH_VERTICES 6
static xgl_native_vertex_t batch[TEST_BATCH_VERTICES + 2];
static xgl_native_vertex_t expected[TEST_BATCH_VERTICES];
static byte guard[sizeof(xgl_native_vertex_t)];
static int used, active, reserve_calls, palette_calls, flush_calls, emitted;

static void close_float(float actual, float wanted)
{
    assert(fabs((double)actual - wanted) <=
           4 * FLT_EPSILON * (1 + fabs((double)wanted)));
}

static void check_batch(void)
{
    int i, j;
    assert(!memcmp(batch, guard, sizeof(guard)));
    assert(!memcmp(batch + TEST_BATCH_VERTICES + 1, guard, sizeof(guard)));
    for (i = 0; i < used; ++i) {
        for (j = 0; j < 3; ++j) {
            close_float(batch[i + 1].position[j], expected[i].position[j]);
            close_float(batch[i + 1].normal[j], expected[i].normal[j]);
            close_float(batch[i + 1].color.rgb[j], expected[i].color.rgb[j]);
        }
    }
}

xgl_native_vertex_t *XGL_ReserveTriangles(int *count, int lit,
                                         float opacity, int zwrite)
{
    xgl_native_vertex_t *result;
    ++reserve_calls;
    assert(*count == 3 && lit == 0 && opacity == 1 && zwrite == 1);
    check_batch(); /* Previous reservations must already contain final data. */
    if (!active) {
        *count = 0;
        return NULL;
    }
    if (used == TEST_BATCH_VERTICES) {
        ++flush_calls;
        emitted += used;
        used = 0;
    }
    result = batch + 1 + used;
    memset(result, 0xa5, 3 * sizeof(*result));
    used += 3;
    return result;
}

void XGL_PreparePaletteColor(int index, float rgb[3])
{
    int i;
    ++palette_calls;
    assert(index >= 0 && index <= 255);
    for (i = 0; i < 3; ++i)
        rgb[i] = gammatable[xgl_palette[index * 3 + i]] / 255.0f;
}

void XGL_UIFillRGB(float x, float y, float width, float height,
                   const float rgb[3], float opacity)
{
    /* PolyBlend shares this source file but is not used by particle draws. */
    (void)x; (void)y; (void)width; (void)height; (void)rgb; (void)opacity;
    assert(0);
}

/* Reference the previous generic triangle plus backend gamma conversion.
 * Compute it before drawing, then compare the reserved native records. */
static void reference_particle(const particle_t *particle,
                                xgl_native_vertex_t vertices[3])
{
    float distance, scale, raw;
    int i, j, color, index;
    distance = (particle->org[0] - r_origin[0]) * vpn[0] +
               (particle->org[1] - r_origin[1]) * vpn[1] +
               (particle->org[2] - r_origin[2]) * vpn[2];
    scale = distance < 20 ? 1.0f : 1.0f + distance * .004f;
    color = (int)particle->color & 255;
    for (i = 0; i < 3; ++i) {
        for (j = 0; j < 3; ++j) {
            vertices[i].position[j] = particle->org[j];
            vertices[i].normal[j] = vpn[j];
            raw = xgl_palette[color * 3 + j] / 255.0f;
            index = (int)(raw * 255.0f + .5f);
            vertices[i].color.rgb[j] = gammatable[index] / 255.0f;
        }
    }
    for (j = 0; j < 3; ++j) {
        vertices[1].position[j] += vup[j] * 1.5f * scale;
        vertices[2].position[j] += vright[j] * 1.5f * scale;
    }
}

static void draw_and_check(particle_t *particle)
{
    xgl_native_vertex_t reference[3];
    particle_t before;
    int slot, palettes, reserves;
    before = *particle;
    reference_particle(particle, reference);
    slot = used == TEST_BATCH_VERTICES ? 0 : used;
    palettes = palette_calls; reserves = reserve_calls;
    XGL_DrawParticle(particle);
    assert(palette_calls == palettes + 1 && reserve_calls == reserves + 1);
    assert(!memcmp(&before, particle, sizeof(before)));
    memcpy(expected + slot, reference, sizeof(reference));
    check_batch();
}

int main(void)
{
    particle_t particle;
    xgl_native_vertex_t snapshot[TEST_BATCH_VERTICES + 2];
    float distances[5] = {-10, 0, 19, 20, 200};
    int i, palettes, draws;
    memset(guard, 0x5a, sizeof(guard));
    memcpy(batch, guard, sizeof(guard));
    memcpy(batch + TEST_BATCH_VERTICES + 1, guard, sizeof(guard));
    for (i = 0; i < 256; ++i) {
        xgl_palette[i * 3] = (byte)i;
        xgl_palette[i * 3 + 1] = (byte)(255 - i);
        xgl_palette[i * 3 + 2] = (byte)(i ^ 85);
        gammatable[i] = (byte)(255 - i);
    }
    memset(&particle, 0, sizeof(particle));
    particle.vel[0] = 17; particle.die = 34; particle.ramp = 3;
    particle.type = pt_grav;
    particle.next = &particle;
    r_origin[0] = 7; r_origin[1] = -3; r_origin[2] = 12;
    vpn[0] = 1; vright[1] = 1; vup[2] = 1;
    active = 1;
    draws = 0;
    for (i = 0; i < 5; ++i) {
        particle.org[0] = r_origin[0] + distances[i];
        particle.org[1] = 9; particle.org[2] = -5;
        particle.color = i == 0 ? -1.25f : i * 73.5f;
        draw_and_check(&particle); ++draws;
    }
    /* Non-axis-aligned camera; palette and gamma edits must be seen at once. */
    vpn[0] = .6f; vpn[1] = .8f;
    vright[0] = -.8f; vright[1] = .6f;
    vup[0] = vup[1] = 0; vup[2] = 1;
    particle.org[0] = 131; particle.org[1] = -68; particle.org[2] = 21;
    for (i = 0; i < 256; ++i) {
        particle.color = (float)i;
        draw_and_check(&particle); ++draws;
    }
    for (i = 0; i < 256; ++i) {
        xgl_palette[i * 3] = 0;
        xgl_palette[i * 3 + 1] = 127;
        xgl_palette[i * 3 + 2] = 255;
        gammatable[i] = (byte)(i / 2);
    }
    draw_and_check(&particle); ++draws;
    assert(emitted + used == draws * 3 && flush_calls > 100);
    /* An inactive scene must neither touch storage nor prepare a color. */
    active = 0;
    palettes = palette_calls;
    memcpy(snapshot, batch, sizeof(batch));
    XGL_DrawParticle(&particle);
    assert(palette_calls == palettes && !memcmp(snapshot, batch, sizeof(batch)));
    check_batch();
    puts("XGL particle contracts passed (native output, palette/gamma, batching and input preservation)");
    return 0;
}
