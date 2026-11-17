/* Compare cached alias colors with the original public RGB sampler. */
#include <assert.h>
#include <float.h>
#include <stdarg.h>
#include <stdlib.h>
#include "../xgl_entities.c"

#define TEST_TRIANGLES 256
#define TEST_VERTICES (TEST_TRIANGLES * 3)
#define RESERVE_VERTICES 9

byte xgl_palette[768], gammatable[256];
static xgl_entity_vertex_t input[TEST_TRIANGLES][3];
static xgl_alias_color_t colors[TEST_TRIANGLES];
static unsigned short indices[TEST_VERTICES];
static xgl_alias_cache_t cache;
static xgl_alias_topology_t topology;
static xgl_alias_pose_cache_t pose;
static xgl_alias_draw_t draw;
static xgl_vertex_t actual[TEST_VERTICES], expected[TEST_VERTICES];
static xgl_native_vertex_t reserved[RESERVE_VERTICES];
static int actual_count, expected_count, reserved_count;
static int prepare_calls, reserve_calls, refuse_reservation;
static int test_material;
static const byte *test_translation;

void Sys_Error(char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    abort();
}

/* The backend receives stored floats, clamps, rounds to an 8-bit input and
 * applies gammatable. Preserve that storage boundary under -ffast-math. */
static float backend_color(float input_color)
{
    volatile float stored = input_color;
    float value = stored;
    if (!(value >= 0)) value = 0;
    else if (value > 1) value = 1;
    return gammatable[(int)(value * 255.0f + .5f)] / 255.0f;
}

void XGL_PrepareColor(float rgb[3])
{
    int i;
    ++prepare_calls;
    for (i = 0; i < 3; ++i) rgb[i] = backend_color(rgb[i]);
}

void XGL_DrawTriangles(const xgl_vertex_t *vertices, int count, int lit,
                       float opacity, int zwrite)
{
    int i, j;
    assert(count > 0 && count % 3 == 0 && lit == 2);
    assert(opacity == 1 && zwrite == 1);
    assert(actual_count + count <= TEST_VERTICES);
    for (i = 0; i < count; ++i) {
        actual[actual_count] = vertices[i];
        for (j = 0; j < 3; ++j)
            actual[actual_count].color[j] = backend_color(vertices[i].color[j]);
        ++actual_count;
    }
}

static void finish_reservation(void)
{
    int i;
    assert(actual_count + reserved_count <= TEST_VERTICES);
    for (i = 0; i < reserved_count; ++i) {
        VectorCopy(reserved[i].position, actual[actual_count].position);
        VectorCopy(reserved[i].normal, actual[actual_count].normal);
        VectorCopy(reserved[i].color.rgb, actual[actual_count].color);
        ++actual_count;
    }
    reserved_count = 0;
}

xgl_native_vertex_t *XGL_ReserveTriangles(int *count, int lit,
                                         float opacity, int zwrite)
{
    finish_reservation();
    assert(*count > 0 && *count % 3 == 0 && lit == 2);
    assert(opacity == 1 && zwrite == 1);
    ++reserve_calls;
    if (refuse_reservation) return NULL;
    if (*count > RESERVE_VERTICES) *count = RESERVE_VERTICES;
    reserved_count = *count;
    memset(reserved, 0x7f, sizeof(reserved));
    return reserved;
}

static void reset_output(void)
{
    assert(!reserved_count);
    actual_count = prepare_calls = reserve_calls = 0;
    refuse_reservation = 0;
    draw.count = draw.index = 0;
}

static void setup_fixture(int triangles, int material, const byte *translation)
{
    int i, j, index;
    assert(triangles > 0 && triangles <= TEST_TRIANGLES);
    reset_output();
    memset(&cache, 0, sizeof(cache));
    memset(&topology, 0, sizeof(topology));
    memset(&pose, 0, sizeof(pose));
    memset(&draw, 0, sizeof(draw));
    memset(colors, 0, sizeof(colors));
    memset(input, 0, sizeof(input));
    cache.colors = colors;
    cache.triangle_count = topology.triangle_count = triangles;
    topology.indices = indices;
    pose.topology = &topology;
    draw.cache = &cache;
    draw.building = 1;
    draw.opaque = XGL_TextureRevision(material) != 0 && !XGL_TextureHasAlpha(material);
    test_material = material;
    test_translation = translation;
    for (i = 0; i < triangles; ++i) {
        for (j = 0; j < 3; ++j) {
            index = i * 3 + j;
            input[i][j].position[0] = i * 2.0f + (j == 1);
            input[i][j].position[1] = (j == 2);
            input[i][j].position[2] = i * .125f;
            input[i][j].normal[2] = 1;
            input[i][j].s = (i + .25f) / 256.0f;
            input[i][j].t = .25f;
            indices[index] = index;
            VectorCopy(input[i][j].position, xgl_alias_work[index].position);
            VectorCopy(input[i][j].normal, xgl_alias_work[index].normal);
        }
    }
}

/* Reference the old algorithm, including its float RGB intermediate and
 * centroid coverage rule. It deliberately never reads the new index cache. */
static void legacy_output(float light)
{
    float center[3], raw[3];
    volatile float lit_color;
    int i, j, k;
    expected_count = 0;
    for (i = 0; i < cache.triangle_count; ++i) {
        center[0] = center[1] = center[2] = 0;
        if (!draw.opaque &&
            !XGL_SampleTexture(test_material,
                (input[i][0].s + input[i][1].s + input[i][2].s) / 3,
                (input[i][0].t + input[i][1].t + input[i][2].t) / 3,
                center, test_translation)) continue;
        for (j = 0; j < 3; ++j) {
            if (!XGL_SampleTexture(test_material, input[i][j].s,
                                  input[i][j].t, raw, test_translation))
                VectorCopy(center, raw);
            VectorCopy(input[i][j].position, expected[expected_count].position);
            VectorCopy(input[i][j].normal, expected[expected_count].normal);
            for (k = 0; k < 3; ++k) {
                lit_color = raw[k] * light;
                expected[expected_count].color[k] = backend_color(lit_color);
            }
            ++expected_count;
        }
    }
}

static void compare_output(void)
{
    int i, j;
    assert(actual_count == expected_count);
    for (i = 0; i < expected_count; ++i) {
        for (j = 0; j < 3; ++j) {
            assert(actual[i].position[j] == expected[i].position[j]);
            assert(actual[i].normal[j] == expected[i].normal[j]);
            if (actual[i].color[j] != expected[i].color[j]) {
                fprintf(stderr, "alias color mismatch at vertex %d channel %d: %.9g != %.9g\n",
                        i, j, actual[i].color[j], expected[i].color[j]);
                abort();
            }
        }
    }
}

static void generic_output(float light)
{
    int i;
    reset_output();
    for (i = 0; i < cache.triangle_count; ++i)
        XGL_EntityTriangleCached(&input[i][0], &input[i][1], &input[i][2],
            test_material, test_translation, light, 2, 0, &draw);
    XGL_AliasFlush(&draw, 2);
    assert(draw.index == cache.triangle_count);
    assert(prepare_calls == 0 && reserve_calls == 0);
    if (draw.building) {
        cache.visible_triangles = 0;
        for (i = 0; i < cache.triangle_count; ++i)
            if (colors[i].visible) ++cache.visible_triangles;
        cache.ready = 1;
        draw.building = 0;
    }
    legacy_output(light);
    compare_output();
}

static void native_output(float light, int distinct_colors)
{
    reset_output();
    XGL_EmitAliasPose(&pose, &draw, light, 2);
    finish_reservation();
    legacy_output(light);
    compare_output();
    assert(prepare_calls == distinct_colors);
    assert(reserve_calls == (expected_count + RESERVE_VERTICES - 1) / RESERVE_VERTICES);
}

static void set_gamma(int mode)
{
    int i;
    for (i = 0; i < 256; ++i) {
        if (mode == 0) gammatable[i] = i;
        else if (mode == 1) gammatable[i] = (i * i + 127) / 255;
        else gammatable[i] = 255 - i;
    }
}

static void test_palette_and_light(int material)
{
    static const float extra_lights[] = {
        .04f, .08f, .20f, .5f - FLT_EPSILON, .5f, .5f + FLT_EPSILON,
        1.0f, 0.0f, -0.5f, 2.0f
    };
    int gamma, k;
    setup_fixture(256, material, NULL);
    generic_output(.20f);
    for (k = 0; k < 256; ++k) {
        assert(colors[k].visible);
        assert(colors[k].index[0] == k && colors[k].index[1] == k && colors[k].index[2] == k);
    }
    for (gamma = 0; gamma < 3; ++gamma) {
        set_gamma(gamma);
        /* R_LightPoint/128 plus the minimum weapon/player/alias values. */
        for (k = 1; k <= 128; ++k) native_output(k / 128.0f, 256);
        for (k = 0; k < (int)(sizeof(extra_lights) / sizeof(extra_lights[0])); ++k) {
            generic_output(extra_lights[k]);
            native_output(extra_lights[k], 256);
        }
    }
    /* A warm index cache must observe palette updates on the next draw. */
    for (k = 0; k < 768; ++k) xgl_palette[k] ^= 255;
    generic_output(.20f);
    native_output(.20f, 256);
    for (k = 0; k < 768; ++k) xgl_palette[k] ^= 255;
    reset_output();
    refuse_reservation = 1;
    XGL_EmitAliasPose(&pose, &draw, .20f, 2);
    assert(actual_count == 0 && prepare_calls == 0 && reserve_calls == 1);
}

static void test_wrapped_and_invalid(int material)
{
    setup_fixture(2, material, NULL);
    set_gamma(2); /* Raw black becomes white, unlike nonblack palette zero. */
    input[0][0].s = -.25f;
    input[0][1].s = 1.25f;
    input[0][2].s = 2.5f;
    input[0][0].t = -2.25f;
    input[1][0].s = 1e20f;
    input[1][1].s = input[1][2].s = 0;
    generic_output(.5f);
    assert(colors[0].index[0] == 192 && colors[0].index[1] == 64 && colors[0].index[2] == 128);
    assert(colors[1].index[0] == -1 && colors[1].index[1] == 0 && colors[1].index[2] == 0);
    assert(actual[3].color[1] != actual[4].color[1]);
    generic_output(.20f);
    native_output(.20f, 5);

    setup_fixture(1, 0, NULL);
    generic_output(.5f);
    assert(!colors[0].visible && cache.visible_triangles == 0);
    native_output(.5f, 0);
}

static void test_alpha_and_translation(void)
{
    byte pixels[4] = {5, 255, 10, 30};
    byte translation[256];
    int material, i;
    for (i = 0; i < 256; ++i) translation[i] = i;
    translation[10] = 255;
    translation[255] = 5;
    material = XGL_LoadTexture("alias-alpha-reference", 4, 1, pixels, false, true);
    setup_fixture(5, material, translation);
    set_gamma(1);
    /* Centroid is raw index 10. Its translated 255 remains opaque; raw 255
     * at a corner inherits that centroid instead of translation[255]. */
    input[0][0].s = .65f; input[0][1].s = .4f; input[0][2].s = .85f;
    for (i = 0; i < 3; ++i) input[1][i].s = .4f;
    input[2][0].s = .85f; input[2][1].s = .65f; input[2][2].s = .4f;
    /* Invalid corners cancel at the centroid and inherit its valid color. */
    input[3][0].s = 1.5e9f; input[3][1].s = -1.5e9f; input[3][2].s = .1f;
    for (i = 0; i < 3; ++i) input[4][i].s = .65f;
    generic_output(.20f);
    assert(cache.visible_triangles == 4 && !colors[1].visible);
    assert(colors[0].index[0] == 255 && colors[0].index[1] == 255 && colors[0].index[2] == 30);
    assert(colors[3].index[0] == 5 && colors[3].index[1] == 5);
    native_output(.20f, 3);
    set_gamma(2);
    generic_output(.08f);
    native_output(.08f, 3);
}

int main(void)
{
    byte pixels[256];
    int i, material;
    for (i = 0; i < 256; ++i) {
        pixels[i] = i;
        xgl_palette[i * 3] = i;
        xgl_palette[i * 3 + 1] = 255 - i;
        xgl_palette[i * 3 + 2] = i ^ 85;
    }
    set_gamma(0);
    material = XGL_LoadTexture("alias-palette-reference", 256, 1, pixels, false, false);
    test_palette_and_light(material);
    test_wrapped_and_invalid(material);
    test_alpha_and_translation();
    XGL_TextureShutdown();
    puts("alias colors match legacy sampling, lighting and gamma");
    return 0;
}
