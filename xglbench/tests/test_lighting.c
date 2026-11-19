/* Portable lighting invariants. GPL-2.0-or-later; see ../COPYING. */
#include "../xglbench.h"

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void close_value(double actual, double expected)
{
    assert(fabs(actual - expected) < 0.000003);
}

static void close_color(xb_rgb actual, xb_rgb expected)
{
    close_value(actual.r, expected.r);
    close_value(actual.g, expected.g);
    close_value(actual.b, expected.b);
}

static int same_color(xb_rgb a, xb_rgb b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

static int same_point(xb_point a, xb_point b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

static double length_squared(xb_point value)
{
    return (double)value.x * value.x + (double)value.y * value.y +
           (double)value.z * value.z;
}

static void test_case(xb_case *test, enum xb_light_model model, enum xb_shade shade)
{
    memset(test, 0, sizeof(*test));
    test->lighting = model; test->shade = shade;
    test->lights = 1; test->moving = 1; test->method = XB_MULTI;
}

static void equations(void)
{
    xb_light_state state;
    xb_point p, n;
    xb_rgb material, result;
    memset(&state, 0, sizeof(state));
    memset(&p, 0, sizeof(p)); memset(&n, 0, sizeof(n));
    n.z = 1.0f;
    material.r = 0.4f; material.g = 0.6f; material.b = 0.8f;
    state.count = 1; state.directional = 1; state.ambient = 0.2f;
    state.direction.z = -1;
    state.color[0].r = 0.5f; state.color[0].g = 0.25f; state.color[0].b = 0.125f;
    result = xb_lighting_evaluate(&state, &p, &n, &material);
    close_value(result.r, 0.28); close_value(result.g, 0.27); close_value(result.b, 0.26);
    n.z = -1;
    result = xb_lighting_evaluate(&state, &p, &n, &material);
    close_value(result.r, 0.08); close_value(result.g, 0.12); close_value(result.b, 0.16);
    n.z = 1; state.directional = 0;
    state.position[0].z = 2; state.attenuation1 = 1; state.attenuation2 = 0.5f;
    result = xb_lighting_evaluate(&state, &p, &n, &material);
    close_value(result.r, 0.18); close_value(result.g, 0.195); close_value(result.b, 0.21);
    /* The two positional terms are additive before material modulation. */
    state.count = 2;
    state.position[1] = state.position[0]; state.color[1] = state.color[0];
    result = xb_lighting_evaluate(&state, &p, &n, &material);
    close_value(result.r, 0.28); close_value(result.g, 0.27); close_value(result.b, 0.26);
    state.ambient = 10;
    result = xb_lighting_evaluate(&state, &p, &n, &material);
    close_value(result.r, 1); close_value(result.g, 1); close_value(result.b, 1);
    state.count = 0; state.ambient = 0.2f;
    result = xb_lighting_evaluate(&state, &p, &n, &material);
    close_value(result.r, 0.08); close_value(result.g, 0.12); close_value(result.b, 0.16);
    result = xb_lighting_evaluate(NULL, &p, &n, &material);
    close_value(result.r, 0); close_value(result.g, 0); close_value(result.b, 0);
}

static void geometry_and_materials(void)
{
    xb_mesh mesh;
    xb_case test;
    xb_light_state state;
    xb_rgb *materials;
    xb_point a, b, c;
    size_t before, expected, i, j, k, first, facet;
    double mean0, low0, high0, mean1, low1, high1;
    int shade, depth, different_normals;
    for (depth = 0; depth <= 2; ++depth) {
        for (shade = XB_FLAT; shade <= XB_SMOOTH; ++shade) {
            memset(&mesh, 0, sizeof(mesh));
            assert(xb_mesh_build(&mesh, 8, depth, 0.85f, 0));
            assert(xb_mesh_projected_area(&mesh, 320, 240, &mean0, &low0, &high0));
            before = mesh.bytes;
            materials = (xb_rgb *)malloc(mesh.triangles * sizeof(*materials));
            assert(materials != NULL);
            memcpy(materials, mesh.base_colors, mesh.triangles * sizeof(*materials));
            test_case(&test, XB_LIGHT_LAMBERT, (enum xb_shade)shade);
            assert(xb_lighting_init(&mesh, &test));
            assert(mesh.lighting_ready && mesh.facet_normals && mesh.vertex_normals &&
                   mesh.centers && mesh.vertex_colors && mesh.facet_styles && mesh.vertex_styles);
            expected = (mesh.triangles * 2 + mesh.point_count) * sizeof(xb_point) +
                       mesh.point_count * sizeof(xb_rgb) +
                       (mesh.triangles + mesh.point_count) * XB_STYLE_LAYERS * sizeof(float);
            assert(mesh.bytes == before + expected);
            assert(xb_mesh_projected_area(&mesh, 320, 240, &mean1, &low1, &high1));
            close_value(mean0, mean1); close_value(low0, low1); close_value(high0, high1);
            xb_lighting_state(&test, 0, &state);
            different_normals = 0;
            for (i = 0; i < mesh.triangles; ++i) {
                close_value(length_squared(mesh.facet_normals[i]), 1.0);
                assert(same_color(mesh.base_colors[i], materials[i]));
                if (test.shade == XB_FLAT)
                    close_color(mesh.colors[i], xb_lighting_evaluate(&state, &mesh.centers[i],
                        &mesh.facet_normals[i], &materials[i]));
                else assert(same_color(mesh.colors[i], materials[i]));
                for (j = 0; j < 3; ++j) {
                    k = i * 3 + j;
                    close_value(length_squared(mesh.vertex_normals[k]), 1.0);
                    if (!same_point(mesh.facet_normals[i], mesh.vertex_normals[k])) different_normals = 1;
                    assert(mesh.points[k].z > 0.0f && mesh.points[k].z < 1.0f);
                    if (test.shade == XB_SMOOTH)
                        close_color(mesh.vertex_colors[k], xb_lighting_evaluate(&state, &mesh.points[k],
                            &mesh.vertex_normals[k], &materials[i]));
                    else assert(same_color(mesh.vertex_colors[k], materials[i]));
                }
            }
            assert(different_normals);
            /* Changing z preserves list/strip equality, including winding. */
            for (i = 0; i < mesh.strip_count; ++i) {
                first = mesh.strips[i].first_point;
                for (j = 0; j + 2 < mesh.strips[i].point_count; ++j) {
                    a = mesh.strip_points[first + j]; b = mesh.strip_points[first + j + 1];
                    c = mesh.strip_points[first + j + 2];
                    if (j & 1) { a = mesh.strip_points[first + j + 1]; b = mesh.strip_points[first + j]; }
                    facet = mesh.strips[i].first_facet + j;
                    assert(same_point(a, mesh.points[facet * 3]));
                    assert(same_point(b, mesh.points[facet * 3 + 1]));
                    assert(same_point(c, mesh.points[facet * 3 + 2]));
                }
            }
            assert(xb_lighting_update(&mesh, &test, 40, &state) == XB_UPDATE_COLORS);
            assert(xb_lighting_update(&mesh, &test, 0, &state) == XB_UPDATE_COLORS);
            assert(!memcmp(mesh.base_colors, materials, mesh.triangles * sizeof(*materials)));
            free(materials); xb_mesh_free(&mesh);
            assert(!mesh.points && !mesh.facet_normals && !mesh.vertex_colors && !mesh.lighting_ready);
        }
    }
}

static void smoothing_and_native_materials(void)
{
    xb_mesh mesh;
    xb_case test;
    xb_light_state first, later;
    xb_rgb *snapshot;
    size_t i;
    int changed;
    memset(&mesh, 0, sizeof(mesh));
    assert(xb_mesh_build(&mesh, 2, 0, 0.85f, 0));
    test_case(&test, XB_LIGHT_LAMBERT, XB_SMOOTH);
    assert(xb_lighting_init(&mesh, &test));
    changed = 0;
    for (i = 1; i < 3; ++i)
        if (!same_color(mesh.vertex_colors[0], mesh.vertex_colors[i])) changed = 1;
    assert(changed); /* Same material, distinct illumination at the corners. */
    snapshot = (xb_rgb *)malloc(mesh.point_count * sizeof(*snapshot));
    assert(snapshot != NULL);
    memcpy(snapshot, mesh.vertex_colors, mesh.point_count * sizeof(*snapshot));
    assert(xb_lighting_update(&mesh, &test, 51, &later) == XB_UPDATE_COLORS);
    assert(memcmp(snapshot, mesh.vertex_colors, mesh.point_count * sizeof(*snapshot)) != 0);
    assert(xb_lighting_update(&mesh, &test, 0, &later) == XB_UPDATE_COLORS);
    assert(!memcmp(snapshot, mesh.vertex_colors, mesh.point_count * sizeof(*snapshot)));
    free(snapshot); xb_mesh_free(&mesh);

    memset(&mesh, 0, sizeof(mesh));
    assert(xb_mesh_build(&mesh, 8, 1, 0.85f, 0));
    test.xgl_lighting = 1;
    assert(xb_lighting_init(&mesh, &test));
    assert(xb_lighting_update(&mesh, &test, 51, &later) == XB_UPDATE_LIGHTS);
    for (i = 0; i < mesh.triangles; ++i) assert(same_color(mesh.colors[i], mesh.base_colors[i]));
    for (i = 0; i < mesh.point_count; ++i) assert(same_color(mesh.vertex_colors[i], mesh.base_colors[i / 3]));
    xb_lighting_state(&test, 0, &first);
    assert(!same_point(first.position[0], later.position[0]));
    xb_lighting_state(&test, 8, &later);
    assert(!memcmp(&first, &later, sizeof(first)));
    test.moving = 0;
    assert(xb_lighting_update(&mesh, &test, 50, &later) == 0);
    assert(!memcmp(&first, &later, sizeof(first)));
    xb_mesh_free(&mesh);
    test.xgl_lighting = 0;
    memset(&mesh, 0, sizeof(mesh));
    assert(xb_mesh_build(&mesh, 2, 0, 0.85f, 0));
    assert(xb_lighting_init(&mesh, &test));
    assert(xb_lighting_update(&mesh, &test, 50, &later) == 0);
    xb_mesh_free(&mesh);
}

static void lightstyles(void)
{
    xb_mesh mesh;
    xb_case test;
    xb_light_state state;
    xb_rgb *colors, *materials;
    float *samples;
    size_t count, i, triangle;
    int shade, styles, layer;
    double value;
    xb_rgb *output;
    const float *coefficient;
    for (shade = XB_FLAT; shade <= XB_SMOOTH; ++shade) {
        for (styles = 0; styles <= 4; styles += styles == 0 ? 1 : 3) {
            memset(&mesh, 0, sizeof(mesh));
            assert(xb_mesh_build(&mesh, 8, 1, 0.85f, 0));
            test_case(&test, XB_LIGHT_BAKED, (enum xb_shade)shade);
            test.sim_fps = 60; /* Explicit legacy cadence remains available. */
            test.styles = styles;
            assert(xb_lighting_init(&mesh, &test));
            count = shade == XB_SMOOTH ? mesh.point_count : mesh.triangles;
            output = shade == XB_SMOOTH ? mesh.vertex_colors : mesh.colors;
            coefficient = shade == XB_SMOOTH ? mesh.vertex_styles : mesh.facet_styles;
            colors = (xb_rgb *)malloc(count * sizeof(*colors));
            materials = (xb_rgb *)malloc(mesh.triangles * sizeof(*materials));
            samples = (float *)malloc(count * XB_STYLE_LAYERS * sizeof(*samples));
            assert(colors && materials && samples);
            memcpy(colors, output, count * sizeof(*colors));
            memcpy(materials, mesh.base_colors, mesh.triangles * sizeof(*materials));
            memcpy(samples, coefficient, count * XB_STYLE_LAYERS * sizeof(*samples));
            assert(xb_lighting_update(&mesh, &test, 1, &state) == 0);
            assert(!memcmp(colors, output, count * sizeof(*colors)));
            if (!styles) {
                assert(xb_lighting_update(&mesh, &test, 999, &state) == 0);
            } else if (styles == 1) {
                assert(xb_lighting_update(&mesh, &test, 6, &state) == 0);
                assert(xb_lighting_update(&mesh, &test, 12, &state) == XB_UPDATE_COLORS);
            } else {
                assert(xb_lighting_update(&mesh, &test, 6, &state) == XB_UPDATE_COLORS);
            }
            for (i = 0; i < count; ++i) {
                triangle = shade == XB_SMOOTH ? i / 3 : i;
                value = 0;
                for (layer = 0; layer < XB_STYLE_LAYERS; ++layer)
                    value += coefficient[i * XB_STYLE_LAYERS + layer] * mesh.style_weights[layer];
                close_value(output[i].r, materials[triangle].r * value);
                close_value(output[i].g, materials[triangle].g * value);
                close_value(output[i].b, materials[triangle].b * value);
            }
            if (styles) assert(xb_lighting_update(&mesh, &test, 0, &state) == XB_UPDATE_COLORS);
            assert(!memcmp(colors, output, count * sizeof(*colors)));
            assert(!memcmp(materials, mesh.base_colors, mesh.triangles * sizeof(*materials)));
            assert(!memcmp(samples, coefficient, count * XB_STYLE_LAYERS * sizeof(*samples)));
            free(colors); free(materials); free(samples); xb_mesh_free(&mesh);
        }
    }
}

static void simulation_cadence(void)
{
    static const char *patterns[XB_STYLE_LAYERS] = {
        "mmnmmommommnonmmonqnmmo", "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba",
        "mmamammmmammamamaaamammma", "abcdefghijklmnopqrrqponmlkjihgfedcba"
    };
    static const int rates[3] = {0, 15, 60};
    xb_mesh mesh;
    xb_case test;
    xb_light_state state;
    float previous;
    unsigned long frame, fps, tick, period, cycle, minimum;
    int rate, layer, changes, seen_change;
    for (rate = 0; rate < 3; ++rate) {
        memset(&mesh, 0, sizeof(mesh));
        assert(xb_mesh_build(&mesh, 8, 0, 0.85f, 0));
        test_case(&test, XB_LIGHT_BAKED, XB_FLAT);
        test.styles = 4; test.sim_fps = rates[rate];
        fps = rates[rate] ? (unsigned long)rates[rate] : 15UL;
        assert(xb_lighting_init(&mesh, &test));
        assert(xb_lighting_cycle_frames(&test) == (fps == 15 ? 3UL : 6UL));
        assert(xb_lighting_min_frames(&test) == (fps == 15 ? 6UL : 24UL));
        for (frame = 0; frame < fps * 3; ++frame) {
            assert(xb_lighting_update(&mesh, &test, frame, &state) >= 0);
            tick = frame * 10UL / fps;
            for (layer = 0; layer < XB_STYLE_LAYERS; ++layer)
                close_value(mesh.style_weights[layer],
                    (patterns[layer][tick % strlen(patterns[layer])] - 'a') / 12.0);
        }
        /* The timestamp may exceed the multiplication range on 32-bit SPARC.
         * A known whole-string period supplies an independent bounded oracle. */
        assert(xb_lighting_update(&mesh, &test, ULONG_MAX, &state) >= 0);
        for (layer = 0; layer < XB_STYLE_LAYERS; ++layer) {
            period = fps * (unsigned long)strlen(patterns[layer]);
            tick = ((ULONG_MAX % period) * 10UL / fps) % strlen(patterns[layer]);
            close_value(mesh.style_weights[layer], (patterns[layer][tick] - 'a') / 12.0);
        }
        xb_mesh_free(&mesh);
    }
    /* A complete minimum trial must exercise a real update at every accepted
     * rate, including low rates that skip over some 10 Hz style values. */
    for (rate = 1; rate <= 240; ++rate) {
        memset(&mesh, 0, sizeof(mesh));
        assert(xb_mesh_build(&mesh, 2, 0, 0.85f, 0));
        test_case(&test, XB_LIGHT_BAKED, XB_FLAT);
        test.styles = 1; test.sim_fps = rate;
        assert(xb_lighting_init(&mesh, &test));
        cycle = xb_lighting_cycle_frames(&test);
        minimum = xb_lighting_min_frames(&test);
        minimum = ((minimum + cycle - 1) / cycle) * cycle;
        assert((cycle * 10UL) % (unsigned long)rate == 0);
        seen_change = 0;
        for (frame = 0; frame < minimum; ++frame) {
            previous = mesh.style_weights[0];
            changes = xb_lighting_update(&mesh, &test, frame, &state);
            if (mesh.style_weights[0] == previous) assert(changes == 0);
            else { assert(changes == XB_UPDATE_COLORS); seen_change = 1; }
        }
        assert(seen_change);
        xb_mesh_free(&mesh);
    }
}

static void animation_cycles(void)
{
    xb_case test;
    xb_light_state phases[8], state;
    unsigned long frame;
    int directional, model, native;
    for (model = XB_LIGHT_LAMBERT; model <= XB_LIGHT_QUAKE; ++model) {
        for (directional = 0; directional <= (model == XB_LIGHT_LAMBERT); ++directional) {
            test_case(&test, (enum xb_light_model)model, XB_SMOOTH);
            test.directional = directional;
            assert(xb_lighting_cycle_frames(&test) == 8);
            assert(xb_lighting_min_frames(&test) == 8);
            for (frame = 0; frame < 8; ++frame) {
                xb_lighting_state(&test, frame, &phases[frame]);
                if (frame) assert(memcmp(&phases[0], &phases[frame], sizeof(state)) != 0);
            }
            for (native = 0; native <= (model == XB_LIGHT_LAMBERT); ++native) {
                test.xgl_lighting = native;
                for (frame = 0; frame < 24; ++frame) {
                    xb_lighting_state(&test, frame, &state);
                    assert(!memcmp(&state, &phases[frame % 8], sizeof(state)));
                }
                xb_lighting_state(&test, ULONG_MAX, &state);
                assert(!memcmp(&state, &phases[ULONG_MAX % 8], sizeof(state)));
            }
            test.moving = 0;
            assert(xb_lighting_cycle_frames(&test) == 1);
            assert(xb_lighting_min_frames(&test) == 3);
            xb_lighting_state(&test, 7, &state);
            assert(!memcmp(&state, &phases[0], sizeof(state)));
        }
    }
    test_case(&test, XB_LIGHT_BAKED, XB_FLAT);
    assert(xb_lighting_cycle_frames(&test) == 1);
    assert(xb_lighting_min_frames(&test) == 3);
}

static void quake_falloff(void)
{
    xb_mesh mesh;
    xb_case test;
    xb_light_state state;
    xb_rgb before;
    double contribution;
    memset(&mesh, 0, sizeof(mesh));
    assert(xb_mesh_build(&mesh, 2, 0, 0.85f, 0));
    test_case(&test, XB_LIGHT_QUAKE, XB_FLAT);
    assert(xb_lighting_init(&mesh, &test));
    xb_lighting_state(&test, 0, &state);
    mesh.centers[0] = state.position[0]; mesh.centers[0].z -= 0.5f;
    mesh.facet_normals[0].x = mesh.facet_normals[0].y = 0;
    mesh.facet_normals[0].z = 1;
    mesh.facet_styles[0] = 0.2f;
    assert(xb_lighting_update(&mesh, &test, 0, &state) == XB_UPDATE_COLORS);
    contribution = (state.radius[0] - 0.5) / state.radius[0];
    close_value(mesh.colors[0].r, mesh.base_colors[0].r * (0.2 + state.color[0].r * contribution));
    close_value(mesh.colors[0].g, mesh.base_colors[0].g * (0.2 + state.color[0].g * contribution));
    before = mesh.colors[0];
    /* This Quake-like model uses plane distance, not a Lambert N dot L. */
    mesh.facet_normals[0].z = -1;
    assert(xb_lighting_update(&mesh, &test, 0, &state) == XB_UPDATE_COLORS);
    close_color(mesh.colors[0], before);
    mesh.centers[0].x += state.radius[0] * 2;
    assert(xb_lighting_update(&mesh, &test, 0, &state) == XB_UPDATE_COLORS);
    close_value(mesh.colors[0].r, mesh.base_colors[0].r * 0.2);
    close_value(mesh.colors[0].g, mesh.base_colors[0].g * 0.2);
    close_value(mesh.colors[0].b, mesh.base_colors[0].b * 0.2);
    xb_mesh_free(&mesh);
}

static void invalid_inputs(void)
{
    xb_mesh mesh;
    xb_case test;
    xb_light_state state;
    xb_point saved, *points;
    size_t bytes;
    volatile double huge;
    memset(&mesh, 0, sizeof(mesh));
    test_case(&test, XB_LIGHT_LAMBERT, XB_FLAT);
    assert(!xb_lighting_init(NULL, &test));
    assert(!xb_lighting_init(&mesh, &test));
    assert(xb_lighting_update(&mesh, &test, 0, &state) == -1);
    assert(xb_mesh_build(&mesh, 2, 0, 0.85f, 0));
    points = mesh.points; bytes = mesh.bytes;
    test.lights = XB_MAX_LIGHTS + 1;
    assert(!xb_lighting_init(&mesh, &test)); test.lights = 1;
    test.sim_fps = -1;
    assert(!xb_lighting_init(&mesh, &test)); test.sim_fps = 241;
    assert(!xb_lighting_init(&mesh, &test)); test.sim_fps = 0;
    test.shade = (enum xb_shade)99;
    assert(!xb_lighting_init(&mesh, &test)); test.shade = XB_FLAT;
    test.lighting = XB_LIGHT_BAKED; test.xgl_lighting = 1;
    assert(!xb_lighting_init(&mesh, &test));
    test.lighting = XB_LIGHT_LAMBERT; test.xgl_lighting = 0;
    saved = mesh.points[1]; mesh.points[1] = mesh.points[0];
    assert(!xb_lighting_init(&mesh, &test)); mesh.points[1] = saved;
    huge = HUGE_VAL; mesh.points[1].x = (float)huge;
    assert(!xb_lighting_init(&mesh, &test)); mesh.points[1] = saved;
    assert(mesh.points == points && mesh.bytes == bytes && !mesh.lighting_ready && !mesh.facet_normals);
    assert(xb_lighting_init(&mesh, &test));
    assert(!xb_lighting_init(&mesh, &test));
    assert(xb_lighting_update(&mesh, &test, 0, NULL) == -1);
    xb_mesh_free(&mesh);
}

static void vertex_texture_lighting(void)
{
    xb_mesh mesh;
    xb_case test;
    xb_light_state state;
    xb_rgb *materials, *snapshot, expected;
    size_t i, bytes, before, extra;
    int model, native;
    memset(&mesh, 0, sizeof(mesh));
    for (model = XB_LIGHT_BAKED; model <= XB_LIGHT_QUAKE; ++model) {
        for (native = 0; native <= (model == XB_LIGHT_LAMBERT); ++native) {
            assert(xb_mesh_build(&mesh, 6, 2, 0.85f, 0));
            test_case(&test, (enum xb_light_model)model, XB_SMOOTH);
            test.vertex_material = 1; test.xgl_lighting = native;
            assert(!xb_lighting_init(&mesh, &test));
            assert(xb_mesh_vertex_materials(&mesh, 2));
            bytes = mesh.point_count * sizeof(xb_rgb);
            materials = (xb_rgb *)malloc(bytes);
            snapshot = (xb_rgb *)malloc(bytes);
            assert(materials && snapshot);
            memcpy(materials, mesh.vertex_materials, bytes);
            before = mesh.bytes;
            extra = (mesh.triangles * 2 + mesh.point_count) * sizeof(xb_point) +
                    (mesh.triangles + mesh.point_count) * XB_STYLE_LAYERS * sizeof(float);
            assert(xb_lighting_init(&mesh, &test));
            assert(mesh.bytes == before + extra);
            if (native) assert(!memcmp(mesh.vertex_colors, materials, bytes));
            if (!native && model == XB_LIGHT_LAMBERT) {
                xb_lighting_state(&test, 0, &state);
                for (i = 0; i < mesh.point_count; ++i) {
                    expected = xb_lighting_evaluate(&state, &mesh.points[i],
                                                    &mesh.vertex_normals[i], &materials[i]);
                    close_color(mesh.vertex_colors[i], expected);
                }
            }
            if (model == XB_LIGHT_BAKED)
                for (i = 0; i < mesh.point_count; ++i) {
                    expected.r = materials[i].r * mesh.vertex_styles[i * XB_STYLE_LAYERS];
                    expected.g = materials[i].g * mesh.vertex_styles[i * XB_STYLE_LAYERS];
                    expected.b = materials[i].b * mesh.vertex_styles[i * XB_STYLE_LAYERS];
                    close_color(mesh.vertex_colors[i], expected);
                }
            assert(xb_lighting_update(&mesh, &test, 17, &state) >= 0);
            memcpy(snapshot, mesh.vertex_colors, bytes);
            assert(xb_lighting_update(&mesh, &test, 51, &state) >= 0);
            assert(xb_lighting_update(&mesh, &test, 17, &state) >= 0);
            assert(!memcmp(snapshot, mesh.vertex_colors, bytes));
            assert(!memcmp(materials, mesh.vertex_materials, bytes));
            free(materials); free(snapshot); xb_mesh_free(&mesh);
        }
    }
}

int main(void)
{
    equations(); geometry_and_materials(); smoothing_and_native_materials();
    lightstyles(); simulation_cadence(); animation_cycles(); quake_falloff();
    invalid_inputs(); vertex_texture_lighting();
    puts("lighting tests: equations, materials, 15/60 FPS style cadence/cache, overflow, complete animation phases, finite falloff and cleanup passed");
    return 0;
}
