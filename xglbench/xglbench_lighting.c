/*
 * Portable lighting experiments. GPL-2.0-or-later; see COPYING.
 *
 * XGL equations: Programmer's Guide 802-5865, printed pp. 398-403.
 * The Lambert path uses ambient + diffuse, no specular reflection. The
 * separately named Quake-like path follows gl_rsurf.c's plane-distance and
 * max(s,t)+min(s,t)/2 falloff, normalized to this procedural scene. It is
 * not a BSP-lightmap renderer or a claim of pixel equivalence to Quake.
 *
 * Geometry lies on a shallow analytic curved surface. Facet normals come
 * from its triangles; vertex normals come from the analytic surface.
 * Default smooth cases vary lighting only. vertex_material cases retain
 * independently sampled corner material RGB through every lighting update.
 * Independent corners preserve discontinuities at material boundaries.
 */
#include "xglbench.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define XB_LIGHTING_MAX_TRIANGLES 262144UL
#define XB_LIGHT_PHASES 8UL

static const char *style_patterns[XB_STYLE_LAYERS] = {
    "mmnmmommommnonmmonqnmmo", "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba",
    "mmamammmmammamamaaamammma", "abcdefghijklmnopqrrqponmlkjihgfedcba"
};

static unsigned long simulation_fps(const xb_case *test)
{
    return test && test->sim_fps ? (unsigned long)test->sim_fps : 15UL;
}

unsigned long xb_lighting_cycle_frames(const xb_case *test)
{
    unsigned long fps, a, b, remainder;
    if (!test || test->sim_fps < 0 || test->sim_fps > 240) return 1;
    if (test->lighting == XB_LIGHT_BAKED && test->styles) {
        fps = simulation_fps(test);
        a = fps; b = 10;
        while (b) { remainder = a % b; a = b; b = remainder; }
        /* Complete the frame/tick cadence, not the full style strings. */
        return fps / a;
    }
    if ((test->lighting == XB_LIGHT_LAMBERT || test->lighting == XB_LIGHT_QUAKE) &&
        test->moving) return XB_LIGHT_PHASES;
    return 1;
}

unsigned long xb_lighting_min_frames(const xb_case *test)
{
    unsigned long frames;
    if (!test || test->sim_fps < 0 || test->sim_fps > 240) return 3;
    if (test->lighting == XB_LIGHT_BAKED && test->styles) {
        frames = (4UL * simulation_fps(test) + 9UL) / 10UL;
        /* Very low simulated rates skip style ticks. Five samples still
         * reach a changed value of the one-layer fixture at 1..9 FPS. */
        if (simulation_fps(test) < 10 && frames < 5) frames = 5;
        return frames < 3 ? 3 : frames;
    }
    if ((test->lighting == XB_LIGHT_LAMBERT || test->lighting == XB_LIGHT_QUAKE) &&
        test->moving) return XB_LIGHT_PHASES;
    return 3;
}

static float unit_color(double value)
{
    if (!(value > 0.0)) return 0.0f;
    if (value > 1.0) return 1.0f;
    return (float)value;
}

static int finite_point(const xb_point *point)
{
    return point->x >= -FLT_MAX && point->x <= FLT_MAX &&
           point->y >= -FLT_MAX && point->y <= FLT_MAX &&
           point->z >= -FLT_MAX && point->z <= FLT_MAX;
}

static int valid_case(const xb_case *test)
{
    if (!test || test->lighting < XB_LIGHT_BAKED ||
        test->lighting > XB_LIGHT_QUAKE ||
        (test->shade != XB_FLAT && test->shade != XB_SMOOTH) ||
        (test->moving != 0 && test->moving != 1) ||
        (test->xgl_lighting != 0 && test->xgl_lighting != 1) ||
        (test->directional != 0 && test->directional != 1) ||
        test->sim_fps < 0 || test->sim_fps > 240 ||
        test->styles < 0 || test->styles > XB_STYLE_LAYERS ||
        (test->vertex_material != 0 && test->vertex_material != 1) ||
        (test->vertex_material && test->shade != XB_SMOOTH))
        return 0;
    if (test->xgl_lighting && test->lighting != XB_LIGHT_LAMBERT)
        return 0;
    if (test->lighting != XB_LIGHT_BAKED &&
        (test->lights < 1 || test->lights > XB_MAX_LIGHTS ||
         (test->directional && test->lights != 1)))
        return 0;
    if (test->lighting == XB_LIGHT_QUAKE && test->directional)
        return 0;
    return 1;
}

static xb_point surface_point(const xb_point *input)
{
    xb_point result;
    result = *input;
    result.z = (float)(0.45 + 0.12 * sin(3.0 * input->x) +
                      0.10 * cos(3.0 * input->y));
    return result;
}

static xb_point surface_normal(const xb_point *position)
{
    xb_point normal;
    double x, y, length;
    x = -0.36 * cos(3.0 * position->x);
    y = 0.30 * sin(3.0 * position->y);
    length = sqrt(x * x + y * y + 1.0);
    normal.x = (float)(x / length);
    normal.y = (float)(y / length);
    normal.z = (float)(1.0 / length);
    return normal;
}

static int facet_normal(const xb_point *a, const xb_point *b,
                         const xb_point *c, xb_point *normal)
{
    double ux, uy, uz, vx, vy, vz, x, y, z, length;
    ux = (double)b->x - a->x; uy = (double)b->y - a->y;
    uz = (double)b->z - a->z;
    vx = (double)c->x - a->x; vy = (double)c->y - a->y;
    vz = (double)c->z - a->z;
    x = uy * vz - uz * vy; y = uz * vx - ux * vz;
    z = ux * vy - uy * vx;
    length = sqrt(x * x + y * y + z * z);
    if (!(length > 0.0 && length <= DBL_MAX)) return 0;
    normal->x = (float)(x / length);
    normal->y = (float)(y / length);
    normal->z = (float)(z / length);
    return 1;
}

static void baked_samples(const xb_point *point, float *samples)
{
    double shadow, dx, dy, value;
    /* A fixed shadow edge and three spatially bounded lightstyle layers are
     * cached once. Their discontinuities are intentional baked information;
     * smooth corner interpolation may soften an edge without enough facets. */
    shadow = point->x > -0.2f && point->y < 0.30f ? 0.25 : 1.0;
    samples[0] = (float)((0.40 + 0.20 * sin(2.1 * point->x + 0.6)) * shadow);
    dx = point->x + 0.50; dy = point->y + 0.25;
    value = 1.0 - sqrt(dx * dx + dy * dy);
    samples[1] = (float)(value > 0.0 ? 0.18 * value : 0.0);
    dx = point->x - 0.45; dy = point->y - 0.35;
    value = 0.80 - sqrt(dx * dx + dy * dy);
    samples[2] = (float)(value > 0.0 ? 0.20 * value : 0.0);
    dx = point->x - 0.10; dy = point->y + 0.65;
    value = 0.65 - sqrt(dx * dx + dy * dy);
    samples[3] = (float)(value > 0.0 ? 0.18 * value : 0.0);
}

void xb_lighting_state(const xb_case *test, unsigned long sequence,
                       xb_light_state *state)
{
    double phase, angle, x, y, length;
    int i;
    if (!state) return;
    memset(state, 0, sizeof(*state));
    if (!valid_case(test) || test->lighting == XB_LIGHT_BAKED) return;
    if (!test->moving) sequence = 0;
    /* A bounded complete cycle gives CPU and native trials identical phase
     * distributions even when their measured frame rates differ. */
    phase = (double)(sequence % XB_LIGHT_PHASES) * 0.78539816339744831;
    state->count = test->lights;
    state->directional = test->directional;
    state->ambient = 0.15f;
    state->attenuation1 = 1.0f;
    state->attenuation2 = 0.75f;
    x = 0.45 * cos(phase); y = 0.35 * sin(phase);
    length = sqrt(x * x + y * y + 1.0);
    /* XGL direction describes light propagation; -direction points to light. */
    state->direction.x = (float)(x / length);
    state->direction.y = (float)(y / length);
    state->direction.z = (float)(-1.0 / length);
    for (i = 0; i < state->count; ++i) {
        angle = phase + i * 1.5707963267948966;
        state->position[i].x = (float)(0.65 * cos(angle));
        state->position[i].y = (float)(0.60 * sin(angle));
        state->position[i].z = (float)(1.10 + 0.10 * sin(angle * 2.0));
        state->radius[i] = 1.10f;
        state->color[i].r = 0.65f;
        state->color[i].g = 0.58f;
        state->color[i].b = 0.50f;
    }
}

xb_rgb xb_lighting_evaluate(const xb_light_state *state, const xb_point *position,
                            const xb_point *normal, const xb_rgb *material)
{
    xb_rgb result;
    double r, g, b, x, y, z, length, dot, attenuation;
    int i;
    /* Normals and directional vectors must be unit length. Init/state
     * normalize them outside the per-sample calculation, as the native
     * submission path also supplies cached unit vectors. */
    result.r = result.g = result.b = 0.0f;
    if (!state || !position || !normal || !material ||
        state->count < 0 || state->count > XB_MAX_LIGHTS) return result;
    r = g = b = state->ambient;
    for (i = 0; i < state->count; ++i) {
        if (state->directional) {
            x = -state->direction.x;
            y = -state->direction.y;
            z = -state->direction.z;
            attenuation = 1.0;
        } else {
            x = (double)state->position[i].x - position->x;
            y = (double)state->position[i].y - position->y;
            z = (double)state->position[i].z - position->z;
            length = sqrt(x * x + y * y + z * z);
            if (!(length > 0.0)) continue;
            x /= length; y /= length; z /= length;
            attenuation = state->attenuation1 + state->attenuation2 * length;
            if (!(attenuation > 0.0)) continue;
            attenuation = 1.0 / attenuation;
        }
        dot = x * normal->x + y * normal->y + z * normal->z;
        if (dot > 0.0) {
            r += state->color[i].r * dot * attenuation;
            g += state->color[i].g * dot * attenuation;
            b += state->color[i].b * dot * attenuation;
        }
    }
    result.r = unit_color(material->r * r);
    result.g = unit_color(material->g * g);
    result.b = unit_color(material->b * b);
    return result;
}

static xb_rgb quake_evaluate(const xb_light_state *state, const xb_point *position,
                             const xb_point *normal, const xb_rgb *material,
                             float baked)
{
    xb_rgb result;
    double r, g, b, x, y, z, plane, radius, sd, td, distance, contribution;
    int i;
    r = g = b = baked;
    for (i = 0; i < state->count; ++i) {
        x = (double)state->position[i].x - position->x;
        y = (double)state->position[i].y - position->y;
        z = (double)state->position[i].z - position->z;
        plane = x * normal->x + y * normal->y + z * normal->z;
        radius = state->radius[i] - fabs(plane);
        if (!(radius > 0.0 && state->radius[i] > 0.0)) continue;
        /* Plane projection, followed by synthetic world-aligned s/t axes.
         * Unlike Lambert lighting, this includes no normal-dot-light term.
         * Units and colored gains are procedural, not Quake's integer grid. */
        sd = fabs(x - normal->x * plane);
        td = fabs(y - normal->y * plane);
        distance = sd > td ? sd + 0.5 * td : td + 0.5 * sd;
        if (distance >= radius) continue;
        contribution = (radius - distance) / state->radius[i];
        r += state->color[i].r * contribution;
        g += state->color[i].g * contribution;
        b += state->color[i].b * contribution;
    }
    result.r = unit_color(material->r * r);
    result.g = unit_color(material->g * g);
    result.b = unit_color(material->b * b);
    return result;
}

static void style_weights(const xb_case *test, unsigned long sequence, float *weights)
{
    /* Quake lightstyles advance at 10 Hz. One simulated work item advances
     * 1/sim_fps second, independent of measured speed. 'm' is normal brightness.
     * These are deterministic test patterns, not map-supplied style strings. */
    int i;
    unsigned long fps, length, tick;
    for (i = 0; i < XB_STYLE_LAYERS; ++i) weights[i] = 0.0f;
    if (!test->styles) { weights[0] = 1.0f; return; }
    fps = simulation_fps(test);
    for (i = 0; i < test->styles; ++i) {
        length = (unsigned long)strlen(style_patterns[i]);
        /* floor(sequence * 10 / fps), modulo the string length. Reduce the
         * whole-second term first so ULONG_MAX sequences cannot overflow;
         * the fractional product is bounded by the 240 FPS input limit. */
        tick = ((sequence / fps) % length) * 10UL +
               ((sequence % fps) * 10UL) / fps;
        weights[i] = (float)(style_patterns[i][tick % length] - 'a') / 12.0f;
    }
}

static int apply_lighting(xb_mesh *mesh, const xb_case *test,
                           unsigned long sequence, xb_light_state *state, int initial)
{
    float weights[XB_STYLE_LAYERS];
    int changed[XB_STYLE_LAYERS], any, dirty, layer;
    size_t i, count, triangle;
    const float *samples;
    const xb_point *position, *normal;
    xb_rgb *color;
    const xb_rgb *material;
    double brightness;
    xb_lighting_state(test, sequence, state);
    if (test->xgl_lighting)
        return initial || test->moving ? XB_UPDATE_LIGHTS : 0;
    if (test->lighting != XB_LIGHT_BAKED && !initial && !test->moving) return 0;
    any = 0;
    if (test->lighting == XB_LIGHT_BAKED) {
        style_weights(test, sequence, weights);
        for (layer = 0; layer < XB_STYLE_LAYERS; ++layer) {
            changed[layer] = !mesh->style_cache_valid ||
                              weights[layer] != mesh->style_weights[layer];
            if (changed[layer]) any = 1;
        }
        if (!any) return 0;
    }
    any = 0;
    count = test->shade == XB_SMOOTH ? mesh->point_count : mesh->triangles;
    for (i = 0; i < count; ++i) {
        if (test->shade == XB_SMOOTH) {
            triangle = i / 3;
            samples = mesh->vertex_styles + i * XB_STYLE_LAYERS;
            position = &mesh->points[i]; normal = &mesh->vertex_normals[i];
            color = &mesh->vertex_colors[i];
        } else {
            triangle = i;
            samples = mesh->facet_styles + i * XB_STYLE_LAYERS;
            position = &mesh->centers[i]; normal = &mesh->facet_normals[i];
            color = &mesh->colors[i];
        }
        material = test->vertex_material ? &mesh->vertex_materials[i] :
                                          &mesh->base_colors[triangle];
        if (test->lighting == XB_LIGHT_BAKED) {
            dirty = !mesh->style_cache_valid;
            for (layer = 0; layer < XB_STYLE_LAYERS; ++layer)
                if (changed[layer] && samples[layer] != 0.0f) dirty = 1;
            if (!dirty) continue;
            brightness = 0.0;
            for (layer = 0; layer < XB_STYLE_LAYERS; ++layer)
                brightness += samples[layer] * weights[layer];
            color->r = unit_color(material->r * brightness);
            color->g = unit_color(material->g * brightness);
            color->b = unit_color(material->b * brightness);
        } else if (test->lighting == XB_LIGHT_QUAKE) {
            *color = quake_evaluate(state, position, normal,
                                    material, samples[0]);
        } else {
            *color = xb_lighting_evaluate(state, position, normal,
                                          material);
        }
        any = 1;
    }
    if (test->lighting == XB_LIGHT_BAKED) {
        memcpy(mesh->style_weights, weights, sizeof(weights));
        mesh->style_cache_valid = 1;
    }
    return any ? XB_UPDATE_COLORS : 0;
}

int xb_lighting_update(xb_mesh *mesh, const xb_case *test,
                       unsigned long sequence, xb_light_state *state)
{
    if (!mesh || !state || !valid_case(test) || !mesh->lighting_ready ||
        !mesh->facet_normals || !mesh->vertex_normals || !mesh->centers ||
        !mesh->vertex_colors || !mesh->facet_styles || !mesh->vertex_styles ||
        (test->vertex_material && !mesh->vertex_materials))
        return -1;
    return apply_lighting(mesh, test, sequence, state, 0);
}

int xb_lighting_init(xb_mesh *mesh, const xb_case *test)
{
    xb_point *facet_normals, *vertex_normals, *centers;
    xb_rgb *vertex_colors;
    float *facet_styles, *vertex_styles;
    xb_point points[3];
    xb_light_state state;
    size_t i, j, count, extra;
    if (!mesh || !valid_case(test) || !mesh->points || !mesh->colors ||
        !mesh->base_colors || !mesh->strip_points ||
        !mesh->triangles || mesh->triangles > XB_LIGHTING_MAX_TRIANGLES ||
        mesh->point_count != mesh->triangles * 3 || mesh->lighting_ready ||
        mesh->facet_normals || mesh->vertex_normals || mesh->centers ||
        mesh->facet_styles || mesh->vertex_styles ||
        (test->vertex_material ? (!mesh->vertex_materials || !mesh->vertex_colors) :
                                 (mesh->vertex_materials || mesh->vertex_colors)))
        return 0;
    for (i = 0; i < mesh->point_count; ++i)
        if (!finite_point(&mesh->points[i])) return 0;
    for (i = 0; i < mesh->strip_point_count; ++i)
        if (!finite_point(&mesh->strip_points[i])) return 0;
    for (i = 0; i < mesh->triangles; ++i)
        if (!(mesh->base_colors[i].r >= 0.0f && mesh->base_colors[i].r <= 1.0f &&
              mesh->base_colors[i].g >= 0.0f && mesh->base_colors[i].g <= 1.0f &&
              mesh->base_colors[i].b >= 0.0f && mesh->base_colors[i].b <= 1.0f)) return 0;
    if (test->vertex_material)
        for (i = 0; i < mesh->point_count; ++i)
            if (!(mesh->vertex_materials[i].r >= 0.0f && mesh->vertex_materials[i].r <= 1.0f &&
                  mesh->vertex_materials[i].g >= 0.0f && mesh->vertex_materials[i].g <= 1.0f &&
                  mesh->vertex_materials[i].b >= 0.0f && mesh->vertex_materials[i].b <= 1.0f)) return 0;
    count = mesh->triangles;
    /* The conservative triangle cap keeps each product safe on 32-bit SPARC. */
    extra = (count * 2 + mesh->point_count) * sizeof(xb_point) +
            mesh->point_count * sizeof(xb_rgb) +
            (count + mesh->point_count) * XB_STYLE_LAYERS * sizeof(float);
    if (mesh->vertex_colors) extra -= mesh->point_count * sizeof(xb_rgb);
    if (mesh->bytes > (size_t)-1 - extra) return 0;
    facet_normals = (xb_point *)malloc(count * sizeof(*facet_normals));
    centers = (xb_point *)malloc(count * sizeof(*centers));
    vertex_normals = (xb_point *)malloc(mesh->point_count * sizeof(*vertex_normals));
    vertex_colors = (xb_rgb *)malloc(mesh->point_count * sizeof(*vertex_colors));
    facet_styles = (float *)malloc(count * XB_STYLE_LAYERS * sizeof(*facet_styles));
    vertex_styles = (float *)malloc(mesh->point_count * XB_STYLE_LAYERS * sizeof(*vertex_styles));
    if (!facet_normals || !centers || !vertex_normals || !vertex_colors ||
        !facet_styles || !vertex_styles) goto failed;
    for (i = 0; i < count; ++i) {
        for (j = 0; j < 3; ++j) {
            points[j] = surface_point(&mesh->points[i * 3 + j]);
            vertex_normals[i * 3 + j] = surface_normal(&points[j]);
            vertex_colors[i * 3 + j] = test->vertex_material ?
                mesh->vertex_materials[i * 3 + j] : mesh->base_colors[i];
            baked_samples(&points[j], vertex_styles + (i * 3 + j) * XB_STYLE_LAYERS);
        }
        if (!facet_normal(&points[0], &points[1], &points[2], &facet_normals[i])) goto failed;
        centers[i].x = (float)(((double)points[0].x + points[1].x + points[2].x) / 3.0);
        centers[i].y = (float)(((double)points[0].y + points[1].y + points[2].y) / 3.0);
        centers[i].z = (float)(((double)points[0].z + points[1].z + points[2].z) / 3.0);
        baked_samples(&centers[i], facet_styles + i * XB_STYLE_LAYERS);
    }
    /* Commit only after allocation and geometry validation succeed. Rejected
     * input leaves the caller's pre-existing mesh and lighting pointers alone. */
    for (i = 0; i < mesh->point_count; ++i) mesh->points[i] = surface_point(&mesh->points[i]);
    for (i = 0; i < mesh->strip_point_count; ++i)
        mesh->strip_points[i] = surface_point(&mesh->strip_points[i]);
    memcpy(mesh->colors, mesh->base_colors, count * sizeof(*mesh->colors));
    mesh->facet_normals = facet_normals; mesh->vertex_normals = vertex_normals;
    free(mesh->vertex_colors);
    mesh->centers = centers; mesh->vertex_colors = vertex_colors;
    mesh->facet_styles = facet_styles; mesh->vertex_styles = vertex_styles;
    mesh->lighting_ready = 1; mesh->style_cache_valid = 0;
    mesh->bytes += extra;
    (void)apply_lighting(mesh, test, 0, &state, 1);
    return 1;
failed:
    free(facet_normals); free(centers); free(vertex_normals);
    free(vertex_colors); free(facet_styles); free(vertex_styles);
    return 0;
}
