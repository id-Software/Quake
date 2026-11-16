/* XGLQuake standalone benchmark. GPL-2.0-or-later; see COPYING. */
#include "xglbench.h"
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

#define XB_MAX_CASES 256
#define XB_TARGET_FRAME_SECONDS (1.0 / 15.0)
#if XB_MAX_RESULTS < 2 * XB_MAX_CASES
#error Result capacity must cover both default resolutions
#endif
static int xb_quit;
static int xb_budget_reached;
static double xb_deadline;

static const char *profile_name(enum xb_profile profile)
{
    static const char *names[] = {"quick", "medium", "long", "extralong"};
    return names[profile];
}

static double profile_budget(const xb_options *o)
{
    static const double budgets[] = {60, 180, 420, 2700};
    return budgets[o->profile];
}

int xb_cancel_requested(void)
{
    return xb_budget_reached || xb_watchdog_expired();
}

/* Stop new work at completed phase boundaries. The independent watchdog
 * forces process exit after the grace period if an XGL call never returns. */
static int budget_expired(void)
{
    if (xb_watchdog_expired() || (xb_deadline > 0 && xb_now() >= xb_deadline))
        xb_budget_reached = 1;
    return xb_budget_reached;
}

static void usage(FILE *f)
{
    fprintf(f,
        "xglbench " XB_VERSION " - native Sun XGL geometry and lighting benchmark\n"
        "Usage: xglbench [options]\n"
        "  -quick             Default: representative tests, about 1 minute, 320x240\n"
        "  -medium            More comparisons at 320x240, about 3 minutes\n"
        "  -long              Broader comparisons at both sizes, about 7 minutes\n"
        "  -extralong         Full suite at both sizes; 45-minute maximum\n"
        "  -winsize W H       Run only this drawable size\n"
        "  -seconds S         Minimum elapsed time per trial (profile default)\n"
        "  -warmup S          Warm-up per case, outside measurements\n"
        "  -repeats N         Repeated trials, 1..31 (profile default)\n"
        "  -pause S           Minimum idle seconds (always also >= 2x active time)\n");
    fprintf(f, "  -queue N           Streaming window, 1..8 work items (default 2)\n");
    fprintf(f,
        "  -triangles N       Base count, positive even number <=16384 (default 512)\n"
        "  -csv PATH          Write machine-readable per-trial and summary results\n"
        "  -test TEXT         Select matching names from the complete case registry\n"
        "  -display DISPLAY   X display (default DISPLAY environment variable)\n"
        "  -allow-fallback    Permit non-DGA/software/single-buffer investigation\n");
    fprintf(f,
        "  -list              List selected cases without opening XGL\n"
        "  -help              Show this help\n"
        "Choose one profile. Explicit timing/repeat/pause options override its\n"
        "defaults regardless of argument order; the profile time limit still applies.\n"
        "Pauses are outside measurements. A watchdog forces exit after 10 seconds\n"
        "of shutdown grace if a rendering call or cleanup does not return.\n"
        "Escape or closing the window stops the run. Keep the window unobscured.\n");
}

static int parse_long(const char *s, long lo, long hi, int *result)
{
    char *end;
    long value;
    if (!s || !*s) return 0;
    errno = 0;
    value = strtol(s, &end, 10);
    if (errno || *end || value < lo || value > hi) return 0;
    *result = (int)value;
    return 1;
}

static int parse_double(const char *s, double lo, double hi, double *result)
{
    char *end;
    double value;
    if (!s || !*s) return 0;
    errno = 0;
    value = strtod(s, &end);
    if (errno || *end || !(value >= lo && value <= hi)) return 0;
    *result = value;
    return 1;
}

static int options(int argc, char **argv, xb_options *o)
{
    int i, profile_seen;
    enum xb_profile profile;
    static const double seconds[] = {.2, .25, .4, .5};
    static const double warmup[] = {.02, .03, .05, .1};
    static const double pause[] = {1.5, 2.5, 3, 2};
    static const int repeats[] = {1, 2, 2, 3};
    memset(o, 0, sizeof(*o));
    o->width = 320; o->height = 240;
    o->seconds = o->warmup = o->pause = -1;
    o->profile = XB_QUICK;
    o->base_triangles = 512;
    o->queue = 2;
    profile_seen = 0;
    for (i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-help") || !strcmp(argv[i], "--help")) {
            usage(stdout); return 2;
        } else if (!strcmp(argv[i], "-quick") || !strcmp(argv[i], "-medium") ||
                   !strcmp(argv[i], "-long") || !strcmp(argv[i], "-extralong")) {
            profile = !strcmp(argv[i], "-quick") ? XB_QUICK :
                      !strcmp(argv[i], "-medium") ? XB_MEDIUM :
                      !strcmp(argv[i], "-long") ? XB_LONG : XB_EXTRALONG;
            if (profile_seen && o->profile != profile) {
                fprintf(stderr, "Choose only one run profile.\n"); return 0;
            }
            o->profile = profile; profile_seen = 1;
        } else if (!strcmp(argv[i], "-list")) {
            o->list_only = 1;
        } else if (!strcmp(argv[i], "-allow-fallback")) {
            o->allow_fallback = 1;
        } else if (!strcmp(argv[i], "-winsize")) {
            if (i + 2 >= argc || !parse_long(argv[i + 1], 128, 4096, &o->width) ||
                !parse_long(argv[i + 2], 64, 4096, &o->height)) goto invalid;
            i += 2; o->force_size = 1;
        } else if (!strcmp(argv[i], "-seconds")) {
            if (++i >= argc || !parse_double(argv[i], .001, 3600, &o->seconds)) goto invalid;
            o->custom_timing = 1;
        } else if (!strcmp(argv[i], "-warmup")) {
            if (++i >= argc || !parse_double(argv[i], 0, 3600, &o->warmup)) goto invalid;
            o->custom_timing = 1;
        } else if (!strcmp(argv[i], "-repeats")) {
            if (++i >= argc || !parse_long(argv[i], 1, XB_MAX_REPEATS, &o->repeats)) goto invalid;
            o->custom_timing = 1;
        } else if (!strcmp(argv[i], "-pause")) {
            if (++i >= argc || !parse_double(argv[i], 0, 3600, &o->pause)) goto invalid;
            o->custom_timing = 1;
        } else if (!strcmp(argv[i], "-queue")) {
            if (++i >= argc || !parse_long(argv[i], 1, XB_MAX_QUEUE, &o->queue)) goto invalid;
        } else if (!strcmp(argv[i], "-triangles")) {
            if (++i >= argc || !parse_long(argv[i], 2, 16384, &o->base_triangles) ||
                (o->base_triangles & 1)) goto invalid;
        } else if (!strcmp(argv[i], "-csv")) {
            if (++i >= argc || !*argv[i]) goto invalid;
            o->csv_path = argv[i];
        } else if (!strcmp(argv[i], "-test")) {
            if (++i >= argc || !*argv[i]) goto invalid;
            o->filter = argv[i];
        } else if (!strcmp(argv[i], "-display")) {
            if (++i >= argc || !*argv[i]) goto invalid;
            o->display = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]); return 0;
        }
    }
    if (o->seconds < 0) o->seconds = seconds[o->profile];
    if (o->warmup < 0) o->warmup = warmup[o->profile];
    if (o->pause < 0) o->pause = pause[o->profile];
    if (!o->repeats) o->repeats = repeats[o->profile];
    return 1;
invalid:
    fprintf(stderr, "Invalid or missing argument near %s\n", argv[i < argc ? i : argc - 1]);
    return 0;
}

static xb_case *add_case(xb_case *cases, int *count, const char *name)
{
    xb_case *c;
    if (*count >= XB_MAX_CASES) return NULL;
    c = &cases[(*count)++];
    memset(c, 0, sizeof(*c));
    strcpy(c->name, name);
    c->method = XB_MULTI; c->work = XB_DRAW;
    c->batch = 128; c->zbuffer = 1; c->size = .85f;
    return c;
}

static void add_lighting_case(xb_case *cases, int *count, const char *name,
                              int path, enum xb_light_model model, int lights,
                              int directional, int moving, int styles,
                              int depth, double area)
{
    xb_case *c;
    c = add_case(cases, count, name);
    c->shade = path & 1 ? XB_SMOOTH : XB_FLAT;
    c->xgl_lighting = path >= 2;
    c->lighting = model;
    c->lights = lights; c->directional = directional; c->moving = moving;
    c->styles = styles; c->depth = depth; c->area = area; c->aspect = 1;
    if (moving || styles) c->update = XB_LIGHT_UPDATE;
}

static int make_cases(xb_case *cases)
{
    int n, m, b, d, z, i, mode, update, path, styles, lights;
    int batches[4];
    int areas[6];
    char name[80];
    xb_case *c;
    const char *sizes[3];
    const char *paths[4];
    const char *texture_modes[5];
    float scale[3];
    n = 0;
    batches[0] = 8; batches[1] = 32; batches[2] = 128; batches[3] = 512;
    c = add_case(cases, &n, "submit/polygon/batch-1");
    c->method = XB_POLYGON; c->batch = 1;
    for (m = XB_MULTI; m <= XB_TLIST; ++m) {
        for (b = 0; b < 4; ++b) {
            sprintf(name, "submit/%s/batch-%d", xb_method_name((enum xb_method)m), batches[b]);
            c = add_case(cases, &n, name);
            c->method = (enum xb_method)m; c->batch = batches[b];
        }
    }
    c = add_case(cases, &n, "submit/strip/natural-length");
    c->method = XB_STRIP;
    c = add_case(cases, &n, "storage/multi_polygon/retained-arrays");
    c->retained = 1;
    for (m = XB_POLYGON; m <= XB_STRIP; ++m) {
        sprintf(name, "connected/%s/depth-1", xb_method_name((enum xb_method)m));
        c = add_case(cases, &n, name);
        c->method = (enum xb_method)m; c->connected = 1; c->depth = 1;
        if (m == XB_POLYGON) c->batch = 1;
    }
    sizes[0] = "small"; sizes[1] = "medium"; sizes[2] = "large";
    scale[0] = .25f; scale[1] = .85f; scale[2] = 3.0f;
    for (i = 0; i < 3; ++i) {
        for (z = 0; z <= 1; ++z) {
            sprintf(name, "fill/%s/z-%s", sizes[i], z ? "on" : "off");
            c = add_case(cases, &n, name); c->size = scale[i]; c->zbuffer = z;
        }
    }
    for (z = 0; z <= 1; ++z) {
        sprintf(name, "overdraw/z-%s", z ? "on" : "off");
        c = add_case(cases, &n, name); c->overdraw = 1; c->zbuffer = z;
    }
    c = add_case(cases, &n, "overdraw/far-to-near/z-on");
    c->overdraw = 1; c->reverse = 1;
    c = add_case(cases, &n, "transform/changing-model"); c->update = XB_TRANSFORM;
    c = add_case(cases, &n, "transform/perspective"); c->perspective = 1;
    c = add_case(cases, &n, "transform/perspective-changing");
    c->perspective = 1; c->update = XB_TRANSFORM;
    c = add_case(cases, &n, "clip/near-crossing"); c->clipping = 1;
    c = add_case(cases, &n, "clip/near-crossing-perspective");
    c->clipping = 1; c->perspective = 1;
    c = add_case(cases, &n, "colors/changing-light"); c->update = XB_COLORS;
    for (d = 0; d <= 2; ++d) {
        sprintf(name, "subdivide/cached/depth-%d", d);
        c = add_case(cases, &n, name); c->depth = d;
        sprintf(name, "subdivide/relight/depth-%d", d);
        c = add_case(cases, &n, name); c->depth = d; c->update = XB_COLORS;
        sprintf(name, "subdivide/rebuild/depth-%d", d);
        c = add_case(cases, &n, name); c->depth = d; c->update = XB_REBUILD;
        sprintf(name, "frame/cached/depth-%d", d);
        c = add_case(cases, &n, name); c->depth = d; c->work = XB_FRAME;
        sprintf(name, "frame/relight/depth-%d", d);
        c = add_case(cases, &n, name); c->depth = d; c->work = XB_FRAME; c->update = XB_COLORS;
    }
    c = add_case(cases, &n, "overhead/color-clear"); c->work = XB_COLOR_CLEAR;
    c = add_case(cases, &n, "overhead/depth-clear"); c->work = XB_DEPTH_CLEAR;
    c = add_case(cases, &n, "overhead/swap"); c->work = XB_SWAP;
    c = add_case(cases, &n, "hud/opaque-copy"); c->work = XB_RASTER; c->zbuffer = 0;
    c = add_case(cases, &n, "hud/masked-copy"); c->work = XB_MASKED_RASTER; c->zbuffer = 0;
    areas[0] = 4; areas[1] = 16; areas[2] = 64;
    areas[3] = 100; areas[4] = 256; areas[5] = 1024;
    for (i = 0; i < 6; ++i) {
        sprintf(name, "area/%dpx", areas[i]);
        c = add_case(cases, &n, name); c->area = areas[i]; c->aspect = 1.0;
    }
    c = add_case(cases, &n, "shape/100px/horizontal-16x1");
    c->area = 100; c->aspect = 16;
    c = add_case(cases, &n, "shape/100px/vertical-1x16");
    c->area = 100; c->aspect = 1.0 / 16.0;
    for (mode = 0; mode <= 1; ++mode) {
        for (update = 0; update <= 1; ++update) {
            for (d = 0; d <= 2; ++d) {
                sprintf(name, "pipeline/%s/%s/depth-%d", mode ? "streaming" : "serialized",
                    update ? "relight" : "cached", d);
                c = add_case(cases, &n, name);
                c->work = XB_PIPELINE; c->streaming = mode; c->depth = d;
                c->update = update ? XB_COLORS : XB_STATIC;
            }
        }
    }
    paths[0] = "cpu-flat"; paths[1] = "cpu-smooth";
    paths[2] = "xgl-flat"; paths[3] = "xgl-smooth";
    for (path = 0; path < 2; ++path) {
        for (d = 0; d <= 2; ++d) {
            sprintf(name, "lighting/baked/%s/cached/depth-%d", paths[path], d);
            add_lighting_case(cases, &n, name, path, XB_LIGHT_BAKED, 0, 0, 0, 0, d, 0);
            for (styles = 1; styles <= 4; styles += 3) {
                sprintf(name, "lighting/baked/%s/styles-%d/depth-%d", paths[path], styles, d);
                add_lighting_case(cases, &n, name, path, XB_LIGHT_BAKED, 0, 0, 0, styles, d, 0);
            }
        }
    }
    for (path = 0; path < 4; ++path) {
        for (d = 0; d <= 2; ++d) {
            sprintf(name, "lighting/matched/%s/directional/depth-%d", paths[path], d);
            add_lighting_case(cases, &n, name, path, XB_LIGHT_LAMBERT, 1, 1, 1, 0, d, 0);
            sprintf(name, "lighting/matched/%s/point-1/depth-%d", paths[path], d);
            add_lighting_case(cases, &n, name, path, XB_LIGHT_LAMBERT, 1, 0, 1, 0, d, 0);
        }
        for (lights = 2; lights <= 4; lights += 2) {
            sprintf(name, "lighting/matched/%s/point-%d/depth-1", paths[path], lights);
            add_lighting_case(cases, &n, name, path, XB_LIGHT_LAMBERT, lights, 0, 1, 0, 1, 0);
        }
        sprintf(name, "lighting/static/%s/directional/depth-1", paths[path]);
        add_lighting_case(cases, &n, name, path, XB_LIGHT_LAMBERT, 1, 1, 0, 0, 1, 0);
        sprintf(name, "lighting/area-100/%s/point-1", paths[path]);
        add_lighting_case(cases, &n, name, path, XB_LIGHT_LAMBERT, 1, 0, 1, 0, 0, 100);
    }
    for (path = 0; path < 2; ++path) {
        for (lights = 1; lights <= 4; lights *= 2) {
            sprintf(name, "lighting/quake-like/%s/point-%d/depth-1", paths[path], lights);
            add_lighting_case(cases, &n, name, path, XB_LIGHT_QUAKE, lights, 0, 1, 0, 1, 0);
        }
    }
    /* Compare material interpolation separately from lighting interpolation.
     * Geometry and CPU texture image match; only the sampling policy changes. */
    for (path = 0; path < 2; ++path) {
        for (d = 0; d <= 2; ++d) {
            sprintf(name, "texture/%s/depth-%d", path ? "vertex-smooth" : "facet-flat", d);
            c = add_case(cases, &n, name); c->depth = d;
            c->shade = path ? XB_SMOOTH : XB_FLAT;
            c->vertex_material = path;
        }
    }
    /* Completed frames include moving lights, color/Z clear, HUD and swap.
     * The Quake-like model remains separate from the matched Lambert model. */
    for (path = 0; path < 4; ++path) {
        for (d = 0; d <= 2; ++d) {
            sprintf(name, "frame/lighting/%s/point-1/depth-%d", paths[path], d);
            add_lighting_case(cases, &n, name, path, XB_LIGHT_LAMBERT, 1, 0, 1, 0, d, 0);
            c = &cases[n - 1]; c->work = XB_FRAME; c->vertex_material = path & 1;
            if (path < 2) {
                sprintf(name, "frame/lighting/%s/quake-point-1/depth-%d", paths[path], d);
                add_lighting_case(cases, &n, name, path, XB_LIGHT_QUAKE, 1, 0, 1, 0, d, 0);
                c = &cases[n - 1]; c->work = XB_FRAME; c->vertex_material = path & 1;
            }
        }
    }
    /* One application thread, overlapping CPU relighting with device work.
     * Native light-state mutation is deliberately outside this experiment. */
    for (mode = 0; mode <= 1; ++mode) {
        for (path = 0; path < 2; ++path) {
            for (d = 0; d <= 2; ++d) {
                sprintf(name, "pipeline/%s/quake-%s/depth-%d",
                    mode ? "streaming" : "serialized", paths[path], d);
                add_lighting_case(cases, &n, name, path, XB_LIGHT_QUAKE, 1, 0, 1, 0, d, 0);
                c = &cases[n - 1]; c->work = XB_PIPELINE;
                c->streaming = mode; c->vertex_material = path & 1;
            }
        }
    }
    for (m = XB_POLYGON; m <= XB_MULTI; ++m) {
        for (path = 0; path < 4; ++path) {
            sprintf(name, "lighting/submit/%s/%s/point-1/depth-1",
                xb_method_name((enum xb_method)m), paths[path]);
            add_lighting_case(cases, &n, name, path, XB_LIGHT_LAMBERT, 1, 0, 1, 0, 1, 0);
            c = &cases[n - 1]; c->method = (enum xb_method)m;
            c->batch = m == XB_POLYGON ? 1 : 128; c->vertex_material = path & 1;
        }
    }
    for (path = 0; path < 2; ++path) {
        for (mode = 0; mode < 2; ++mode) {
            sprintf(name, "cull/%s/%s", path ? "mixed" : "all-front", mode ? "back" : "off");
            c = add_case(cases, &n, name);
            c->mixed_winding = path; c->cull = mode;
        }
    }
    texture_modes[0] = "facet-flat"; texture_modes[1] = "vertex-smooth";
    texture_modes[2] = "point"; texture_modes[3] = "bilinear";
    texture_modes[4] = "trilinear";
    for (mode = 0; mode < 5; ++mode) {
        for (path = 0; path < 2; ++path) {
            for (d = 0; d <= 2; ++d) {
                sprintf(name, "texture-map/%s/%s/depth-%d", texture_modes[mode],
                    path ? "frame" : "draw", d);
                c = add_case(cases, &n, name);
                c->depth = d; c->work = path ? XB_FRAME : XB_DRAW;
                c->texture_compare = 1;
                c->vertex_material = mode == 1;
                c->shade = mode == 1 ? XB_SMOOTH : XB_FLAT;
                c->texture_filter = mode < 2 ? XB_TEXTURE_OFF :
                                      (enum xb_texture_filter)(mode - 1);
            }
        }
        sprintf(name, "texture-map/%s/perspective-motion/depth-1", texture_modes[mode]);
        c = add_case(cases, &n, name);
        c->depth = 1; c->texture_compare = 1;
        c->perspective = 1; c->update = XB_TRANSFORM;
        c->vertex_material = mode == 1;
        c->shade = mode == 1 ? XB_SMOOTH : XB_FLAT;
        c->texture_filter = mode < 2 ? XB_TEXTURE_OFF :
                              (enum xb_texture_filter)(mode - 1);
    }
    return n;
}

/* Sampled-color defaults keep related comparisons together. Native texture
 * mapping remains available through -test and the full extralong registry.
 * Leave setup/readback headroom within the unchanged cooling/time budgets.
 */
static int select_cases(const xb_case *cases, int count, const xb_options *o, int *order)
{
    static const char *quick[] = {
        "texture-map/facet-flat/draw/depth-0",
        "texture-map/vertex-smooth/draw/depth-0",
        "submit/polygon/batch-1", "submit/multi_polygon/batch-128",
        "submit/triangle_list/batch-128", "submit/strip/natural-length",
        "texture-map/facet-flat/frame/depth-0",
        "texture-map/vertex-smooth/frame/depth-0",
        "lighting/matched/cpu-flat/point-1/depth-1",
        "lighting/matched/xgl-flat/point-1/depth-1",
        "lighting/matched/cpu-smooth/point-1/depth-1",
        "lighting/matched/xgl-smooth/point-1/depth-1",
        "lighting/baked/cpu-flat/cached/depth-1",
        "lighting/baked/cpu-smooth/cached/depth-1",
        "texture/facet-flat/depth-1", "texture/vertex-smooth/depth-1"
    };
    static const char *long_extra[] = {
        "texture-map/facet-flat/frame/depth-1",
        "texture-map/vertex-smooth/frame/depth-1",
        "lighting/baked/cpu-flat/styles-4/depth-1",
        "lighting/baked/cpu-smooth/styles-4/depth-1",
        "frame/lighting/cpu-flat/point-1/depth-0",
        "frame/lighting/xgl-flat/point-1/depth-0",
        "frame/lighting/cpu-smooth/point-1/depth-0",
        "frame/lighting/xgl-smooth/point-1/depth-0",
        "cull/mixed/off", "cull/mixed/back",
        "overhead/color-clear", "overhead/depth-clear",
        "texture/facet-flat/depth-2", "texture/vertex-smooth/depth-2",
        "pipeline/serialized/cached/depth-1", "pipeline/streaming/cached/depth-1",
        "pipeline/serialized/relight/depth-1", "pipeline/streaming/relight/depth-1"
    };
    const char *name;
    int i, j, n, quick_count, wanted;
    n = 0;
    if (o->filter || o->profile == XB_EXTRALONG) {
        for (i = 0; i < count; ++i)
            if (!o->filter || strstr(cases[i].name, o->filter)) order[n++] = i;
        return n;
    }
    quick_count = (int)(sizeof(quick) / sizeof(quick[0]));
    wanted = quick_count;
    if (o->profile >= XB_MEDIUM) wanted += (int)(sizeof(long_extra) / sizeof(long_extra[0]));
    for (i = 0; i < wanted; ++i) {
        name = i < quick_count ? quick[i] : long_extra[i - quick_count];
        for (j = 0; j < count && strcmp(cases[j].name, name); ++j) {}
        if (j == count) {
            fprintf(stderr, "Profile refers to missing case: %s\n", name);
            return -1;
        }
        order[n++] = j;
    }
    return n;
}

static int build_mesh(xb_mesh *mesh, const xb_options *o, const xb_case *c,
                      int width, int height)
{
    int ok;
    size_t i;
    xb_point point;
    if (c->area > 0) ok = xb_mesh_build_calibrated(mesh, o->base_triangles,
                                                 width, height, c->area, c->aspect);
    else if (c->connected) ok = xb_mesh_build_connected(mesh, o->base_triangles, c->depth);
    else ok = xb_mesh_build(mesh, o->base_triangles, c->depth, c->size, c->overdraw);
    if (ok && c->reverse) ok = xb_mesh_reverse_draw_order(mesh);
    if (ok && !strncmp(c->name, "cull/", 5)) {
        /* Only unlit multi-polygon cases use this independent-corner fixture;
         * the strip representation is deliberately not submitted. Under the
         * XGL Y_DOWN_Z_AWAY view, -Z faces the viewer. The ordinary generator
         * produces +Z normals, so reverse all corners for all-front, or only
         * even facets to leave the odd half back-facing in the mixed fixture. */
        for (i = 0; i < mesh->triangles; i += c->mixed_winding ? 2 : 1) {
            point = mesh->points[i * 3 + 1];
            mesh->points[i * 3 + 1] = mesh->points[i * 3 + 2];
            mesh->points[i * 3 + 2] = point;
        }
    }
    if (ok && c->clipping) {
        for (i = 0; i < mesh->point_count; ++i)
            mesh->points[i].z = (mesh->points[i].x + .2f) * .8f;
        for (i = 0; i < mesh->strip_point_count; ++i)
            mesh->strip_points[i].z = (mesh->strip_points[i].x + .2f) * .8f;
    }
    if (ok && c->texture_compare && c->perspective) {
        /* A tilted plane varies W within each triangle: a front-parallel
         * rotating patch would not exercise perspective-correct UVs. */
        for (i = 0; i < mesh->point_count; ++i)
            mesh->points[i].z = .45f + .35f * mesh->points[i].x;
        for (i = 0; i < mesh->strip_point_count; ++i)
            mesh->strip_points[i].z = .45f + .35f * mesh->strip_points[i].x;
    }
    if (ok && c->texture_compare &&
        !xb_mesh_texture_materials(mesh, c->depth, c->vertex_material)) {
        xb_mesh_free(mesh);
        return 0;
    }
    if (ok && !c->texture_compare && c->vertex_material &&
        !xb_mesh_vertex_materials(mesh, c->depth)) {
        xb_mesh_free(mesh);
        return 0;
    }
    if (ok && c->lighting != XB_LIGHT_OFF && !xb_lighting_init(mesh, c)) {
        xb_mesh_free(mesh);
        return 0;
    }
    return ok;
}

typedef struct { double total, prepare, render, present; size_t calls; } xb_frame;

static int one_frame(const xb_options *o, const xb_case *c, xb_mesh *mesh,
                     unsigned long sequence, xb_frame *f)
{
    double start, prepared, render_start, rendered, end;
    int width, height, changes;
    xb_light_state state;
    memset(f, 0, sizeof(*f));
    if (budget_expired()) return 0;
    start = xb_now();
    if (c->update == XB_REBUILD) {
        xb_backend_dimensions(&width, &height);
        if (!build_mesh(mesh, o, c, width, height) || !xb_backend_upload(mesh)) return 0;
    } else if (c->update == XB_COLORS) {
        xb_mesh_colors(mesh, sequence);
        xb_backend_colors(mesh);
    } else if (c->update == XB_TRANSFORM) {
        xb_backend_transform(sequence);
    } else if (c->update == XB_LIGHT_UPDATE) {
        changes = xb_lighting_update(mesh, c, sequence, &state);
        if (changes < 0) return 0;
        if (changes & XB_UPDATE_COLORS) xb_backend_colors(mesh);
        if ((changes & XB_UPDATE_LIGHTS) && !xb_backend_lighting(&state)) return 0;
    }
    prepared = xb_now();
    if (budget_expired()) goto cancelled;
    if (c->work == XB_DRAW || c->work == XB_RASTER || c->work == XB_MASKED_RASTER) {
        /* Identical depth history for every frame. Exclude this reset from render_ms. */
        xb_backend_clear(1, c->zbuffer);
        xb_backend_sync();
    }
    if (budget_expired()) goto cancelled;
    render_start = xb_now();
    switch (c->work) {
    case XB_DRAW:
        f->calls = xb_backend_draw(c->method, c->batch); break;
    case XB_FRAME:
        xb_backend_clear(1, c->zbuffer);
        if (budget_expired()) goto cancelled;
        f->calls = xb_backend_draw(c->method, c->batch);
        if (budget_expired()) goto cancelled;
        xb_backend_raster(1);
        break;
    case XB_COLOR_CLEAR: xb_backend_clear(1, 0); break;
    case XB_DEPTH_CLEAR: xb_backend_clear(0, 1); break;
    case XB_RASTER: xb_backend_raster(0); break;
    case XB_MASKED_RASTER: xb_backend_raster(1); break;
    case XB_SWAP: break;
    case XB_PIPELINE: return 0; /* has a separate bounded-window timing path */
    }
    if (c->work != XB_SWAP) xb_backend_sync();
    rendered = xb_now();
    if (budget_expired()) goto cancelled;
    if (c->work == XB_SWAP || c->work == XB_FRAME) {
        xb_backend_swap();
        xb_backend_sync();
    }
    end = xb_now();
    if (budget_expired()) goto cancelled;
    f->prepare = prepared - start;
    f->render = c->work == XB_SWAP ? 0 : rendered - render_start;
    f->present = (c->work == XB_SWAP || c->work == XB_FRAME) ? end - rendered : 0;
    f->total = end - start;
    return 1;
cancelled:
    /* Drain only what was already submitted. The watchdog remains armed
     * during this wait, native cleanup and output flushing. */
    xb_backend_sync();
    return 0;
}

static enum xb_failure backend_failure(void)
{
    enum xb_failure failure;
    failure = xb_backend_failure();
    return failure == XB_FAILURE_NONE ? XB_FAILURE_ERROR : failure;
}

static int events(xb_result *r)
{
    int result, width, height;
    result = xb_backend_events();
    if (!result) {
        r->failure = backend_failure();
        xb_quit = 1;
        if (*xb_backend_error()) {
            strncpy(r->reason, xb_backend_error(), sizeof(r->reason) - 1);
            r->reason[sizeof(r->reason) - 1] = '\0';
        } else strcpy(r->reason, "interrupted by user");
        return 0;
    }
    if (result < 0) {
        r->failure = backend_failure();
        xb_backend_dimensions(&width, &height);
        if (width != r->width || height != r->height) {
            strcpy(r->reason, "drawable changed size during trial; run stopped to preserve size labels");
            xb_quit = 1;
        } else if (*xb_backend_error()) {
            strncpy(r->reason, xb_backend_error(), sizeof(r->reason) - 1);
            r->reason[sizeof(r->reason) - 1] = '\0';
        } else strcpy(r->reason, "window exposed/obscured during trial; measurement discarded");
        return 0;
    }
    if (budget_expired()) {
        r->failure = XB_FAILURE_TIME_LIMIT;
        strcpy(r->reason, "profile time budget reached");
        return 0;
    }
    return 1;
}

/* Sleep after completed work, never inside a measured trial. Expose events
 * here do not invalidate an earlier sample. Resizing would invalidate the
 * next case's labels, so it stops the run; recoverable case errors are reset
 * by the next configure(), just as they are without pauses.
 */
static void idle_pause(double seconds, int width, int height)
{
    double until, left;
    int result, actual_width, actual_height;
    if (seconds <= 0 || xb_quit || budget_expired()) return;
    xb_backend_sync();
    fprintf(stderr, "Idle pause: %.2f seconds\n", seconds);
    until = xb_now() + seconds;
    while (!xb_quit && !budget_expired()) {
        result = xb_backend_events();
        if (!result) {
            fprintf(stderr, "Run stopped during idle pause: %s\n",
                *xb_backend_error() ? xb_backend_error() : "interrupted by user");
            xb_quit = 1;
            return;
        }
        xb_backend_dimensions(&actual_width, &actual_height);
        if (actual_width != width || actual_height != height) {
            fprintf(stderr, "Drawable changed size during idle pause; run stopped.\n");
            xb_quit = 1;
            return;
        }
        left = until - xb_now();
        if (xb_deadline > 0 && left > xb_deadline - xb_now())
            left = xb_deadline - xb_now();
        if (left <= 0) break;
        if (left > .1) left = .1;
        if (!xb_sleep(left) && errno != EINTR) {
            perror("idle pause"); xb_quit = 1; return;
        }
    }
}

static void rest_after_work(const xb_options *o, double active, int width, int height)
{
    double rest;
    rest = active * 2;
    if (rest < o->pause) rest = o->pause;
    idle_pause(rest, width, height);
}

typedef struct {
    unsigned long items, drains, maximum;
    size_t calls;
    double prepare, enqueue, drain;
} xb_window;

static void backend_reason(xb_result *r, const char *fallback)
{
    const char *message;
    if (budget_expired()) {
        r->failure = XB_FAILURE_TIME_LIMIT;
        strcpy(r->reason, "profile time budget reached");
        return;
    }
    r->failure = backend_failure();
    message = xb_backend_error();
    if (!*message) message = fallback;
    strncpy(r->reason, message, sizeof(r->reason) - 1);
    r->reason[sizeof(r->reason) - 1] = '\0';
}

static int pipeline_window(xb_result *r, xb_mesh *mesh,
                           unsigned long *sequence, xb_window *window)
{
    int slot, okay, changes;
    xb_light_state state;
    size_t calls;
    double start;
    memset(window, 0, sizeof(*window));
    okay = 1;
    for (slot = 0; slot < r->queue_limit; ++slot) {
        if (!events(r)) { okay = 0; break; }
        start = xb_now();
        if (r->test.update == XB_COLORS) {
            xb_mesh_colors(mesh, *sequence);
            if (!xb_backend_stream_prepare(slot, mesh)) {
                backend_reason(r, "stream color preparation failed"); okay = 0; break;
            }
        } else if (r->test.update == XB_LIGHT_UPDATE) {
            changes = xb_lighting_update(mesh, &r->test, *sequence, &state);
            if (changes < 0 || (changes & XB_UPDATE_LIGHTS)) {
                backend_reason(r, "stream lighting requires CPU-computed colors"); okay = 0; break;
            }
            /* Each reusable slot may contain an older sequence's colors,
             * including when a cached lightstyle did not change this item. */
            if (!xb_backend_stream_prepare(slot, mesh)) {
                backend_reason(r, "stream lighting color preparation failed"); okay = 0; break;
            }
        }
        window->prepare += xb_now() - start;
        if (budget_expired()) {
            backend_reason(r, "profile time budget reached"); okay = 0; break;
        }
        start = xb_now();
        calls = xb_backend_stream_draw(slot);
        if (!calls) {
            backend_reason(r, "stream submission failed"); okay = 0; break;
        }
        if (budget_expired()) {
            backend_reason(r, "profile time budget reached"); okay = 0; break;
        }
        xb_backend_post();
        window->enqueue += xb_now() - start;
        window->calls = calls;
        ++window->items;
        ++*sequence;
        window->maximum = window->items;
    }
    /* Always attempt the final drain, including interrupted/partial windows.
     * Neither FLUSH_BUFFERS nor a drawing call releases retained arrays. */
    start = xb_now();
    if (!xb_backend_stream_drain()) {
        if (okay) backend_reason(r, "stream completion drain failed");
        okay = 0;
    }
    window->drain = xb_now() - start;
    window->drains = 1;
    if (okay && !events(r)) okay = 0;
    return okay;
}

static int measure_pipeline(const xb_options *o, xb_result *r, xb_mesh *mesh)
{
    xb_window window;
    double start, cpu, elapsed, prepare, enqueue, drain;
    unsigned long sequence, items, drains, maximum;
    int trial, okay;
    r->queue_limit = r->test.streaming ? o->queue : 1;
    if (!xb_backend_stream_begin(mesh, r->queue_limit, r->reason, sizeof(r->reason))) {
        r->failure = backend_failure();
        return 0;
    }
    okay = 0;
    sequence = 0;
    start = xb_now();
    do {
        if (!pipeline_window(r, mesh, &sequence, &window)) goto done;
    } while (xb_now() - start < o->warmup);
    for (trial = 0; trial < o->repeats; ++trial) {
        sequence = 0; items = 0; drains = 0; maximum = 0;
        prepare = enqueue = drain = 0;
        cpu = xb_cpu_now();
        start = xb_now();
        do {
            if (!pipeline_window(r, mesh, &sequence, &window)) goto done;
            items += window.items; drains += window.drains;
            if (window.maximum > maximum) maximum = window.maximum;
            prepare += window.prepare; enqueue += window.enqueue; drain += window.drain;
            r->calls = window.calls;
            elapsed = xb_now() - start;
        } while (items < 3 || elapsed < o->seconds);
        /* elapsed spans first preparation through final completion, including
         * window fill/drain and event polling. No queued work is counted early. */
        r->cpu[trial] = (xb_cpu_now() - cpu) / (double)items;
        r->total[trial] = elapsed / (double)items;
        r->prepare[trial] = prepare / (double)items;
        r->enqueue[trial] = enqueue / (double)items;
        r->drain[trial] = drain / (double)items;
        r->frames += items; r->drains += drains;
        if (maximum > r->max_outstanding) r->max_outstanding = maximum;
        ++r->samples;
    }
    okay = 1;
done:
    xb_backend_stream_end();
    if (okay && !events(r)) okay = 0;
    return okay;
}

static void measure(const xb_options *o, xb_result *r, const int *valid,
                    const enum xb_failure *validation_failure,
                    char validation[XB_METHODS][160])
{
    xb_mesh mesh;
    xb_frame f;
    xb_light_state state;
    double start, cpu, sum, prep, render, present;
    unsigned long frames, sequence;
    int trial, uses_mesh, preview, style_changes, animated_styles;
    const xb_case *c;
    c = &r->test;
    r->failure = XB_FAILURE_ERROR;
    animated_styles = c->lighting == XB_LIGHT_BAKED && c->styles > 0;
    memset(&mesh, 0, sizeof(mesh));
    uses_mesh = c->work == XB_DRAW || c->work == XB_FRAME || c->work == XB_PIPELINE;
    if (uses_mesh && !valid[c->method]) {
        r->failure = validation_failure[c->method];
        strcpy(r->reason, validation[c->method]); return;
    }
    if ((c->work == XB_SWAP || c->work == XB_FRAME) && !xb_backend_has_double_buffer()) {
        r->failure = XB_FAILURE_UNSUPPORTED;
        strcpy(r->reason, "two hardware/window buffers not allocated"); return;
    }
    if (uses_mesh && !build_mesh(&mesh, o, c, r->width, r->height)) {
        strcpy(r->reason, c->area > 0 ? "calibrated triangle does not fit, or allocation/input limit failure" :
                                      "geometry allocation or input limit failure"); return;
    }
    if (budget_expired()) {
        r->failure = XB_FAILURE_TIME_LIMIT;
        strcpy(r->reason, "profile time budget reached"); goto done;
    }
    if (!xb_backend_configure(c, &mesh, r->reason, sizeof(r->reason))) {
        r->failure = backend_failure();
        goto done;
    }
    if (budget_expired()) {
        r->failure = XB_FAILURE_TIME_LIMIT;
        strcpy(r->reason, "profile time budget reached"); goto done;
    }
    /* configure() owns the initial upload; updates upload inside one_frame(). */
    r->triangles = mesh.triangles;
    r->bytes = mesh.bytes;
    if (uses_mesh) {
        if (!xb_mesh_projected_area(&mesh, r->width, r->height,
                                   &r->area_mean, &r->area_min, &r->area_max)) {
            strcpy(r->reason, "invalid projected area in generated mesh"); goto done;
        }
        r->input_bytes = xb_backend_input_bytes(c->method, c->batch);
    }
    xb_backend_label(c->name);
    /* Show a representative image even for tests which exclude presentation. */
    xb_backend_clear(1, c->zbuffer);
    if (uses_mesh) xb_backend_draw(c->method, c->batch);
    if (c->work == XB_RASTER || c->work == XB_MASKED_RASTER)
        xb_backend_raster(c->work == XB_MASKED_RASTER);
    xb_backend_sync();
    if (xb_backend_has_double_buffer()) xb_backend_swap();
    xb_backend_sync();
    preview = xb_backend_events();
    if (!preview) {
        r->failure = backend_failure();
        xb_quit = 1;
        if (*xb_backend_error()) {
            strncpy(r->reason, xb_backend_error(), sizeof(r->reason) - 1);
            r->reason[sizeof(r->reason) - 1] = '\0';
        } else strcpy(r->reason, "interrupted by user");
        goto done;
    }
    /* Resize/expose from setup are consumed before warm-up; later events invalidate. */
    if (c->work == XB_PIPELINE) {
        if (!measure_pipeline(o, r, &mesh)) goto done;
        goto summarize;
    }
    sequence = 0;
    start = xb_now();
    do {
        if (!events(r)) goto done;
        if (!one_frame(o, c, &mesh, sequence++, &f)) {
            backend_reason(r, "warm-up preparation/native submission failed"); goto done;
        }
    } while (xb_now() - start < o->warmup);
    for (trial = 0; trial < o->repeats; ++trial) {
        if (c->lighting != XB_LIGHT_OFF) sequence = 0;
        if (animated_styles) {
            /* Every trial starts with the same style cache. Otherwise the
             * final warm-up/timed phase can add a spurious reset upload. */
            style_changes = xb_lighting_update(&mesh, c, 0, &state);
            if (style_changes < 0) {
                backend_reason(r, "resetting lightstyle cache failed"); goto done;
            }
            if (style_changes & XB_UPDATE_COLORS) xb_backend_colors(&mesh);
            xb_backend_sync();
            if (!events(r)) goto done;
        }
        frames = 0; sum = 0; prep = 0; render = 0; present = 0;
        cpu = xb_cpu_now(); start = xb_now();
        do {
            if (!events(r)) goto done;
            if (!one_frame(o, c, &mesh, sequence++, &f)) {
                backend_reason(r, "preparation/native submission failed"); goto done;
            }
            ++frames;
            sum += f.total; prep += f.prepare; render += f.render; present += f.present;
            r->calls = f.calls;
        } while (frames < (unsigned long)(animated_styles ? 24 : 3) ||
                 xb_now() - start < o->seconds || (animated_styles && frames % 6));
        /* A fault in the final draw/flush must not turn into a successful trial. */
        if (!events(r)) goto done;
        r->cpu[trial] = (xb_cpu_now() - cpu) / (double)frames;
        r->total[trial] = sum / (double)frames;
        r->prepare[trial] = prep / (double)frames;
        r->render[trial] = render / (double)frames;
        r->present[trial] = present / (double)frames;
        r->frames += frames; ++r->samples;
    }
summarize:
    r->median = xb_median(r->total, r->samples);
    r->prep_median = xb_median(r->prepare, r->samples);
    r->render_median = xb_median(r->render, r->samples);
    r->present_median = xb_median(r->present, r->samples);
    r->low = r->high = r->total[0];
    for (trial = 1; trial < r->samples; ++trial) {
        if (r->total[trial] < r->low) r->low = r->total[trial];
        if (r->total[trial] > r->high) r->high = r->total[trial];
    }
    r->passed = 1;
    r->failure = XB_FAILURE_NONE;
done:
    xb_mesh_free(&mesh);
}

static void csv_text(FILE *f, const char *s)
{
    fputc('"', f);
    while (*s) { if (*s == '"') fputc('"', f); fputc(*s++, f); }
    fputc('"', f);
}

static const char *lighting_name(const xb_case *c)
{
    switch (c->lighting) {
    case XB_LIGHT_BAKED: return "baked";
    case XB_LIGHT_LAMBERT: return "lambert";
    case XB_LIGHT_QUAKE: return "quake-like";
    default: return "none";
    }
}

static const char *lighting_compute(const xb_case *c)
{
    return c->lighting == XB_LIGHT_OFF ? "none" : (c->xgl_lighting ? "xgl" : "cpu");
}

static const char *lighting_sample(const xb_case *c)
{
    if (c->lighting == XB_LIGHT_OFF) return "none";
    if (c->shade == XB_SMOOTH) return "vertices";
    return c->xgl_lighting ? "device-vertex" : "centroid";
}

static int style_layers(const xb_case *c)
{
    if (c->lighting == XB_LIGHT_BAKED) return c->styles ? c->styles : 1;
    return c->lighting == XB_LIGHT_QUAKE ? 1 : 0;
}

static const char *texture_filter_name(enum xb_texture_filter filter)
{
    switch (filter) {
    case XB_TEXTURE_POINT: return "point";
    case XB_TEXTURE_BILINEAR: return "bilinear";
    case XB_TEXTURE_TRILINEAR: return "trilinear";
    default: return "off";
    }
}

static const char *failure_name(enum xb_failure failure)
{
    static const char *names[] = {
        "none", "error", "api-error", "validation-failed", "unsupported", "time-limit", "not-run"
    };
    return names[failure];
}

static const char *failure_label(enum xb_failure failure)
{
    static const char *labels[] = {
        "PASS", "ERROR", "API ERROR", "VALIDATION FAILED", "UNSUPPORTED", "TIME LIMIT", "NOT RUN"
    };
    return labels[failure];
}

static void csv_row(FILE *f, const xb_result *r, const char *record, int trial,
                    double total, double prep, double render, double present, double cpu)
{
    double rate, duration, enqueue, drain;
    int pipeline;
    if (!f) return;
    pipeline = r->test.work == XB_PIPELINE;
    duration = pipeline || r->test.lighting != XB_LIGHT_OFF ? total : render;
    rate = r->triangles && duration > 0 ? (double)r->triangles / duration : 0;
    enqueue = drain = 0;
    if (r->passed && pipeline) {
        enqueue = trial > 0 ? r->enqueue[trial - 1] : xb_median(r->enqueue, r->samples);
        drain = trial > 0 ? r->drain[trial - 1] : xb_median(r->drain, r->samples);
    }
    fprintf(f, "%s,%d,%d,", record, r->width, r->height);
    csv_text(f, r->test.name);
    fprintf(f, ",%s,%d,%s,%d,%d,%d,%d,%d,%lu,%lu,%lu,%lu,",
            r->passed ? "ok" : (r->failure == XB_FAILURE_NOT_RUN ? "not-run" : "skipped"), trial, xb_method_name(r->test.method),
            r->test.batch, r->test.depth, r->test.zbuffer, r->test.connected,
            pipeline || r->test.retained,
            (unsigned long)r->triangles, (unsigned long)r->calls,
            (unsigned long)r->bytes, r->frames);
    if (r->failure == XB_FAILURE_NOT_RUN) fputs(",,,,,,", f);
    else {
        fprintf(f, "%.9f,%.9f,", total * 1000, prep * 1000);
        if (!pipeline) fprintf(f, "%.9f", render * 1000);
        fprintf(f, ",%.9f,%.9f,%.3f,", present * 1000, cpu * 1000, rate);
    }
    csv_text(f, r->reason);
    fprintf(f, ",%s,%d,%lu,%lu,",
        pipeline ? (r->test.streaming ? "pipeline-streaming" : "pipeline-serialized") :
            (r->test.lighting != XB_LIGHT_OFF ? "lighting-serialized" : "serialized"),
        r->queue_limit, r->max_outstanding, r->drains);
    if (r->failure == XB_FAILURE_NOT_RUN) fputs(",,", f);
    else fprintf(f, "%.9f,%.9f,", enqueue * 1000, drain * 1000);
    fprintf(f, "%.6f,%.6f,%.6f,%lu,", r->area_mean, r->area_min,
            r->area_max, (unsigned long)r->input_bytes);
    fprintf(f, "%s,%s,%s,%d,%d,%d,%s,%s,%s,%s,", r->test.shade == XB_SMOOTH ? "smooth" : "flat",
        lighting_name(&r->test), lighting_compute(&r->test), r->test.lights,
        r->test.moving, style_layers(&r->test), lighting_sample(&r->test),
        r->test.texture_filter != XB_TEXTURE_OFF ? "texels" :
            (r->test.vertex_material ? "vertices" : "facet"),
        r->test.cull ? "back" : "off", r->test.mixed_winding ? "mixed" : "front");
    if (r->passed && r->test.work == XB_FRAME)
        fprintf(f, "%.9f,%.9f", XB_TARGET_FRAME_SECONDS * 1000,
                total / XB_TARGET_FRAME_SECONDS);
    else fputc(',', f);
    fprintf(f, ",%s,%s,%s,",
        r->test.texture_filter != XB_TEXTURE_OFF ? "native-xgl" : "none",
        texture_filter_name(r->test.texture_filter),
        r->test.texture_filter != XB_TEXTURE_OFF ? "unclassified" : "none");
    if (r->test.texture_filter != XB_TEXTURE_OFF)
        fprintf(f, "%d,%d", XB_TEXTURE_WIDTH, XB_TEXTURE_WIDTH);
    else fputc(',', f);
    fprintf(f, ",%s\n", failure_name(r->failure));
}

static void print_result(FILE *csv, const xb_result *r)
{
    int i;
    double rate, duration;
    const char *unit;
    if (!r->passed) {
        printf("%-38s %s: %s\n", r->test.name, failure_label(r->failure), r->reason);
        csv_row(csv, r, "summary", -1, 0, 0, 0, 0, 0);
        return;
    }
    duration = r->test.work == XB_PIPELINE || r->test.lighting != XB_LIGHT_OFF ?
               r->median : r->render_median;
    rate = r->triangles && duration > 0 ? (double)r->triangles / duration : 0;
    printf("%-38s %9.3f %9.3f ", r->test.name, r->median * 1000, r->prep_median * 1000);
    if (r->test.work == XB_PIPELINE) printf("%9s ", "n/a");
    else printf("%9.3f ", r->render_median * 1000);
    printf("%9.3f %10.0f %6lu\n", r->present_median * 1000, rate, (unsigned long)r->calls);
    if (*r->reason) printf("  note: %s\n", r->reason);
    unit = r->test.work == XB_PIPELINE ? "item" : "frame";
    printf("  trial-mean range %.3f..%.3f ms; %lu %ss, %lu facets/%s, "
           "%.1f KiB portable mesh, CPU %.3f ms/%s\n",
        r->low * 1000, r->high * 1000, r->frames, unit, (unsigned long)r->triangles, unit,
        (double)r->bytes / 1024, xb_median(r->cpu, r->samples) * 1000, unit);
    if (r->triangles)
        printf("  nominal area %.3f px/facet (%.3f..%.3f); input payload %lu bytes/%s\n",
            r->area_mean, r->area_min, r->area_max, (unsigned long)r->input_bytes, unit);
    if (r->test.work == XB_PIPELINE)
        printf("  host enqueue %.3f ms/item; drain %.3f ms/item; max queued %lu/%d; "
               "%lu timed drains; no swaps\n",
            xb_median(r->enqueue, r->samples) * 1000, xb_median(r->drain, r->samples) * 1000,
            r->max_outstanding, r->queue_limit, r->drains);
    if (r->test.lighting != XB_LIGHT_OFF) {
        printf("  %s %s lighting; %s; %d %s light(s)%s; %d style layer(s); samples %s\n",
            lighting_compute(&r->test), lighting_name(&r->test),
            r->test.shade == XB_SMOOTH ? "smooth vertex colors" : "flat facet colors",
            r->test.lights, r->test.moving ? "moving" : "fixed",
            r->test.lighting == XB_LIGHT_LAMBERT ? " + ambient" : "",
            style_layers(&r->test),
            lighting_sample(&r->test));
        printf("  lighting facets/sec uses total_ms (CPU preparation + reset + completed draw).\n");
    }
    if (r->test.vertex_material)
        printf("  Material RGB sampled at each corner; interpolation includes texture color.\n");
    if (r->test.texture_filter != XB_TEXTURE_OFF)
        printf("  Native XGL %s texture mapping, %dx%d RGB image; execution unclassified.\n"
               "  Software texturing is allowed; device acceleration reports do not classify it.\n"
               "  Texture upload/mipmap setup and readback are outside timing; completed draws are timed.\n",
               texture_filter_name(r->test.texture_filter), XB_TEXTURE_WIDTH, XB_TEXTURE_WIDTH);
    if (!strncmp(r->test.name, "cull/", 5))
        printf("  Backface culling %s; %s winding. Facet rate counts submitted triangles,\n"
               "  including rejected backfaces; this is not a CPU-culling comparison.\n",
               r->test.cull ? "on" : "off", r->test.mixed_winding ? "mixed" : "all-front");
    if (r->test.work == XB_FRAME)
        printf("  15 FPS reference: %.1f%% of 66.667 ms; %.3f ms budget remaining.\n"
               "  Synthetic render workload only; engine work and polling still need time.\n",
               100.0 * r->median / XB_TARGET_FRAME_SECONDS,
               (XB_TARGET_FRAME_SECONDS - r->median) * 1000);
    for (i = 0; i < r->samples; ++i)
        csv_row(csv, r, "trial", i + 1, r->total[i], r->prepare[i],
                r->render[i], r->present[i], r->cpu[i]);
    csv_row(csv, r, "summary", -1, r->median, r->prep_median,
            r->render_median, r->present_median, xb_median(r->cpu, r->samples));
}

static const xb_result *find_result(const xb_result *r, int count,
                                   int width, int height, const char *name)
{
    int i;
    for (i = 0; i < count; ++i)
        if (r[i].passed && r[i].width == width && r[i].height == height &&
            !strcmp(r[i].test.name, name)) return &r[i];
    return NULL;
}

static void findings(const xb_result *r, int count, int width, int height)
{
    int i, d, update, shade, lights;
    const xb_result *best, *poly, *connected, *a, *b;
    char name[80];
    double ratio;
    best = NULL; connected = NULL;
    printf("\nFindings for %dx%d (synthetic workload, not Quake FPS):\n", width, height);
    for (i = 0; i < count; ++i) {
        if (!r[i].passed || r[i].width != width || r[i].height != height) continue;
        if (!strncmp(r[i].test.name, "submit/", 7) && r[i].render_median > 0 &&
            (!best || r[i].render_median < best->render_median)) best = &r[i];
        if (!strncmp(r[i].test.name, "connected/", 10) && r[i].render_median > 0 &&
            (!connected || r[i].render_median < connected->render_median)) connected = &r[i];
        if (r[i].samples > 1 && r[i].median > 0 && (r[i].high - r[i].low) / r[i].median > .15)
            printf("  Variable timing: %s (trial-mean range exceeds 15%% of median).\n", r[i].test.name);
    }
    if (best) {
        printf("  Lowest measured independent-mesh render time: %s, %.3f ms.\n",
            best->test.name, best->render_median * 1000);
        poly = find_result(r, count, width, height, "submit/polygon/batch-1");
        if (poly && best->render_median > 0)
            printf("  %.2fx throughput relative to individual polygon calls.\n",
                poly->render_median / best->render_median);
        printf("  This is a candidate, not a statistically established universal winner.\n");
    }
    if (connected)
        printf("  Lowest measured connected-grid render time: %s, %.3f ms.\n",
            connected->test.name, connected->render_median * 1000);
    if (best || connected)
        printf("  Leo expands strips before geometry processing: compact input may save\n"
               "  input bandwidth/submission work, not downstream shared-vertex transforms.\n");
    a = find_result(r, count, width, height, "submit/multi_polygon/batch-128");
    b = find_result(r, count, width, height, "storage/multi_polygon/retained-arrays");
    if (a && b && a->render_median > 0)
        printf("  Retained/volatile native-array render-time ratio: %.2f (same mesh/batch).\n",
            b->render_median / a->render_median);
    a = find_result(r, count, width, height, "subdivide/cached/depth-0");
    if (a) {
        for (d = 1; d <= 2; ++d) {
            sprintf(name, "subdivide/cached/depth-%d", d);
            b = find_result(r, count, width, height, name);
            if (b && a->render_median > 0)
                printf("  Depth %d: %lu facets, %.2fx baseline render time.\n",
                    d, (unsigned long)b->triangles, b->render_median / a->render_median);
        }
    }
    a = find_result(r, count, width, height, "subdivide/cached/depth-1");
    b = find_result(r, count, width, height, "subdivide/rebuild/depth-1");
    if (a && b && a->median > 0)
        printf("  Rebuilding depth-1 geometry each frame costs %.2fx the cached total; "
               "rebuild preparation %.3f ms/frame.\n", b->median / a->median, b->prep_median * 1000);
    a = find_result(r, count, width, height, "hud/opaque-copy");
    b = find_result(r, count, width, height, "hud/masked-copy");
    if (a && b && a->render_median > 0)
        printf("  Masked/opaque HUD copy time ratio: %.2f.\n", b->render_median / a->render_median);
    a = find_result(r, count, width, height, "overdraw/z-on");
    b = find_result(r, count, width, height, "overdraw/far-to-near/z-on");
    if (a && b && a->render_median > 0)
        printf("  Far-to-near/near-to-far overdraw time ratio: %.2f (same geometry).\n",
            b->render_median / a->render_median);
    a = find_result(r, count, width, height, "area/100px");
    if (a && a->render_median > 0) {
        b = find_result(r, count, width, height, "shape/100px/horizontal-16x1");
        if (b) printf("  Equal 100px area: wide/square render-time ratio %.2f.\n",
                      b->render_median / a->render_median);
        b = find_result(r, count, width, height, "shape/100px/vertical-1x16");
        if (b) printf("  Equal 100px area: tall/square render-time ratio %.2f.\n",
                      b->render_median / a->render_median);
    }
    for (update = 0; update <= 1; ++update) {
        for (d = 0; d <= 2; ++d) {
            sprintf(name, "pipeline/serialized/%s/depth-%d", update ? "relight" : "cached", d);
            a = find_result(r, count, width, height, name);
            sprintf(name, "pipeline/streaming/%s/depth-%d", update ? "relight" : "cached", d);
            b = find_result(r, count, width, height, name);
            if (a && b && b->median > 0)
                printf("  %s depth-%d: queue %d gives %.2fx completed throughput vs queue 1\n"
                       "    (same clear+mesh work, single thread, no swaps; %.3f vs %.3f ms/item).\n",
                    update ? "Relit" : "Cached", d, b->queue_limit, a->median / b->median,
                    b->median * 1000, a->median * 1000);
        }
    }
    for (shade = 0; shade < 2; ++shade) {
        for (lights = 0; lights <= 4; ++lights) {
            if (lights == 3) continue;
            if (lights == 0)
                sprintf(name, "lighting/matched/cpu-%s/directional/depth-1", shade ? "smooth" : "flat");
            else sprintf(name, "lighting/matched/cpu-%s/point-%d/depth-1", shade ? "smooth" : "flat", lights);
            a = find_result(r, count, width, height, name);
            if (lights == 0)
                sprintf(name, "lighting/matched/xgl-%s/directional/depth-1", shade ? "smooth" : "flat");
            else sprintf(name, "lighting/matched/xgl-%s/point-%d/depth-1", shade ? "smooth" : "flat", lights);
            b = find_result(r, count, width, height, name);
            if (a && b && b->median > 0) {
                printf("  Lambert %s, %d %s light(s), depth 1: XGL %.2fx CPU total throughput\n"
                       "    (%.3f vs %.3f ms, including light/color update costs).\n",
                    shade ? "smooth" : "flat", lights ? lights : 1,
                    lights ? "positional" : "directional", a->median / b->median,
                    b->median * 1000, a->median * 1000);
                if (lights && !shade)
                    printf("    Flat positional samples differ: CPU centroid, XGL device-selected vertex.\n");
            }
        }
        sprintf(name, "lighting/baked/cpu-%s/cached/depth-1", shade ? "smooth" : "flat");
        a = find_result(r, count, width, height, name);
        sprintf(name, "lighting/baked/cpu-%s/styles-4/depth-1", shade ? "smooth" : "flat");
        b = find_result(r, count, width, height, name);
        if (a && b && a->median > 0)
            printf("  Baked %s: animated four-layer/cached total-time ratio %.2f (different layer workloads).\n",
                shade ? "smooth" : "flat", b->median / a->median);
    }
    a = find_result(r, count, width, height, "lighting/baked/cpu-flat/cached/depth-1");
    b = find_result(r, count, width, height, "lighting/baked/cpu-smooth/cached/depth-1");
    if (a && b && a->median > 0)
        printf("  Cached baked smooth/flat total-time ratio %.2f; compare lighting gradients visually.\n",
            b->median / a->median);
    for (d = 0; d <= 2; ++d) {
        sprintf(name, "frame/relight/depth-%d", d);
        a = find_result(r, count, width, height, name);
        if (a && a->median > 0) {
            ratio = a->present_median / a->median;
            printf("  Relit depth-%d synthetic frame: %.3f ms; presentation %.0f%%.\n",
                d, a->median * 1000, ratio * 100);
            printf("    Leaves %.3f ms of a 15 FPS budget for game/visibility/audio "
                   "(this scene only).\n", 1000.0 / 15.0 - a->median * 1000);
        }
    }
    printf("  Choose depth 1 only if its measured frame budget leaves enough CPU time;\n"
           "  reserve depth 2 for selected surfaces and retain a complete coarse mesh.\n");
}

static void compare_sizes(const xb_result *r, int count)
{
    int i;
    const xb_result *large;
    printf("\nResolution comparison: 640x480 / 320x240 (4x window area)\n");
    for (i = 0; i < count; ++i) {
        if (!r[i].passed || r[i].width != 320 || r[i].height != 240) continue;
        large = find_result(r, count, 640, 480, r[i].test.name);
        if (!large || r[i].median <= 0) continue;
        printf("  %-38s ", r[i].test.name);
        if (r[i].test.work == XB_PIPELINE || r[i].render_median <= 0) printf("render    n/a; ");
        else printf("render %6.2fx; ", large->render_median / r[i].render_median);
        printf("total %6.2fx%s\n", large->median / r[i].median,
            r[i].test.area > 0 ? " (fixed triangle area/count)" : "");
    }
    printf("  Weak render-time scaling is consistent with geometry/CPU/dispatch limits;\n"
           "  stronger scaling indicates pixel-work sensitivity. Neither identifies a\n"
           "  unique bottleneck. Swap timing can be quantized by display refresh.\n"
           "  Calibrated area/shape cases keep triangle area/count fixed; clears still grow.\n"
           "  HUD copies remain 128x64 pixels at both sizes; their work does not quadruple.\n");
}

int main(int argc, char **argv)
{
    xb_options o;
    xb_case cases[XB_MAX_CASES];
    xb_result *results, *r;
    FILE *csv;
    int parsed, count, selected, i, s, m, nresult, w, h, successes, sizes;
    int valid[XB_METHODS], order[XB_MAX_CASES], planned, attempted, failures[XB_FAILURE_NOT_RUN + 1];
    enum xb_failure validation_failure[XB_METHODS];
    double run_start, active_start, active, nominal, budget;
    char validation[XB_METHODS][160];
    struct utsname host;
    parsed = options(argc, argv, &o);
    if (parsed == 2) return 0;
    if (!parsed) { usage(stderr); return 2; }
    count = make_cases(cases);
    selected = select_cases(cases, count, &o, order);
    if (selected < 0) return 1;
    if (o.list_only)
        for (i = 0; i < selected; ++i) puts(cases[order[i]].name);
    if (!selected) { fprintf(stderr, "No test names match the filter.\n"); return 2; }
    if (o.list_only) return 0;
    sizes = o.force_size || o.profile <= XB_MEDIUM ? 1 : 2;
    planned = selected * sizes;
    active = o.warmup + o.seconds * o.repeats;
    nominal = active * 2;
    if (nominal < o.pause) nominal = o.pause;
    nominal = planned * active + (planned - 1) * nominal;
    budget = profile_budget(&o);
    printf("Profile: %s; %d selected cases x %d resolution(s) = %d planned\n",
        profile_name(o.profile), selected, sizes, planned);
    printf("Nominal active + minimum idle: %.1f seconds before setup, validation and slow-frame overhead.\n",
        nominal);
    if (budget > 0)
        printf("Time limit: %.0f seconds including setup and idle; incomplete tests are discarded.\n", budget);
    if (o.custom_timing) printf("Custom trial/idle settings; profile time limit is unchanged.\n");
    printf("Idle after setup and between cases: at least %.2f seconds or twice preceding active time.\n"
           "Idle time is outside measurements; no temperature monitoring or thermal guarantee.\n"
           "Watchdog: force process exit after 10 seconds of shutdown grace.\n", o.pause);
    fflush(stdout);
    csv = NULL;
    if (o.csv_path) {
        csv = fopen(o.csv_path, "w");
        if (!csv) { perror(o.csv_path); return 1; }
        fprintf(csv, "record,width,height,test,status,trial,method,batch,depth,zbuffer,connected,retained,"
                     "triangles,calls,mesh_bytes,frames_all_trials,total_ms,prepare_ms,render_ms,"
                     "present_ms,cpu_ms,triangles_per_second,reason,");
        fprintf(csv, "timing_mode,queue_limit,max_queued,drain_calls,enqueue_ms,drain_ms,"
                     "nominal_area_mean_px,nominal_area_min_px,nominal_area_max_px,input_payload_bytes,");
        fprintf(csv, "shading,lighting_model,lighting_compute,light_count,light_motion,style_layers,lighting_sample,"
                     "material_sample,cull,winding,frame_budget_ms,frame_budget_fraction,"
                     "texture_mapping,texture_filter,texture_execution,texture_width,texture_height,failure_kind\n");
    }
    results = (xb_result *)calloc(XB_MAX_RESULTS, sizeof(*results));
    if (!results) { if (csv) fclose(csv); fprintf(stderr, "Out of memory.\n"); return 1; }
    run_start = xb_now();
    if (budget > 0) xb_deadline = run_start + budget;
    if (!xb_watchdog_start((unsigned int)budget)) {
        perror("cannot start run watchdog");
        free(results); if (csv) fclose(csv); return 1;
    }
    if (!xb_backend_open(&o, o.width, o.height)) {
        fprintf(stderr, "XGL setup failed: %s\n", xb_backend_error());
        xb_backend_close(); free(results); if (csv) fclose(csv);
        xb_watchdog_stop(); return 1;
    }
    printf("xglbench " XB_VERSION " - native XGL geometry and lighting investigation\n");
    if (uname(&host) == 0)
        printf("Host: %s %s %s %s\n", host.nodename, host.sysname, host.release, host.machine);
    printf("Build: %s %s; pointer %lu bytes; float %lu bytes\n", __DATE__, __TIME__,
        (unsigned long)sizeof(void *), (unsigned long)sizeof(float));
    xb_backend_info(stdout);
    printf("Settings: %d base triangles; %.3f seconds/trial; %d repeats; %.3f seconds warm-up; queue %d\n",
        o.base_triangles, o.seconds, o.repeats, o.warmup, o.queue);
    printf("Sampled-color cases use CPU material RGB; texture-map cases also compare native XGL texturing.\n"
           "All timed rendering waits for completion. No result is a GPU-only timer.\n"
           "Totals include reset clears; draw render_ms excludes the reset. frame/* includes\n"
           "clear+draw+masked HUD in render_ms and swap in present_ms. Their event polling is untimed.\n");
    printf("Pipeline cases use one thread, retained slots and bounded post/drain windows.\n"
           "Their total includes clear+draw, preparation, event polling and final completion;\n"
           "facets/sec uses this completed-item total, with render_ms empty (no swaps).\n"
           "Enqueue/drain are host intervals, not GPU-stage times. max_queued counts software\n"
           "submissions since a drain, not hardware occupancy. Queue 1 is the matched control.\n");
    printf(
           "Preparation includes native-array conversion/allocation where applicable.\n"
           "Rebuild cases also recreate portable list AND strip representations; this\n"
           "is harness preparation cost, not an optimized engine implementation.\n"
           "Statistics are medians/ranges of per-trial means, not frame-latency percentiles.\n"
           "Mesh bytes exclude XGL/native buffers and driver allocations.\n");
    printf("Nominal area is reference orthographic pixel area before transforms/clipping.\n"
           "Calibrated area/shape cases keep exact geometric area and count at both sizes;\n"
           "overlap is intentional when necessary, at equal Z with LEQUAL depth testing.\n"
           "Input payload estimates exclude descriptors/driver packing; they are not measured\n"
           "SBus or DMA traffic. Posting or retaining arrays does not prove DMA is active.\n");
    printf("Whitepaper context: 210K isolated 100px lit Gouraud/Z/depth-cued triangles/sec\n"
           "was a VRAM-limited historical result. These tests do not reproduce that exact\n"
           "workload; treat it as context, not an expected result or a pass/fail threshold.\n");
    printf("Lighting cases separate CPU flat/smooth colors from XGL facet/vertex illumination.\n"
           "Original smooth cases preserve facet material; vertex-material cases also interpolate texture color.\n"
           "Compare completed total_ms: XGL light-state changes and CPU recoloring are preparation.\n"
           "Lighting facets/sec also uses total_ms. Point-lit flat pairs share equations but\n"
           "not sample locations (CPU centroid vs device-chosen XGL vertex).\n");
    printf("Lambert tests use ambient+diffuse only; Quake-like radius falloff is a separate model.\n"
           "Baked/style samples are procedural. No BSP loading or baked+XGL-light hybrid is implied.\n"
           "Lightstyles use deterministic 10Hz simulation ticks at six work items/tick, not wall time.\n"
           "Normals and sample caches are built before timing; updated RGB/native light state is timed.\n");
    printf("Animated-style trials reset their cache before timing, run at least 24 items,\n"
           "and finish a six-item tick so even short trials exercise actual color changes.\n");
    printf("All cases use one application thread; no SMP or worker-thread tests are included.\n"
           "Full-frame budget reference is 15 FPS (66.667 ms), not a promised Quake frame rate.\n"
           "Allow time for engine logic, visibility, audio, input and workloads absent here.\n");
    printf("Native texturing is optional and may execute in software, even on a hardware DGA pipeline.\n"
           "Unsupported/incorrect mapping is skipped, never replaced by a timed untextured fallback.\n"
           "texture-map/* compares the same raw image without synthetic patch tint or lighting.\n");
    if (!xb_backend_has_hardware())
        printf("FALLBACK/TEST RUN: results do not establish the accelerated ZX performance path.\n");
    nresult = 0; successes = 0;
    for (s = 0; s < sizes && !xb_quit && !budget_expired(); ++s) {
        active_start = s ? xb_now() : run_start;
        w = o.force_size ? o.width : (s ? 640 : 320);
        h = o.force_size ? o.height : (s ? 480 : 240);
        if (s && !xb_backend_resize(w, h)) {
            fprintf(stderr, "Resize failed: %s\n", xb_backend_error()); xb_quit = 1; break;
        }
        xb_backend_dimensions(&w, &h);
        printf("\nDrawable: %dx%d\n", w, h);
        for (m = 0; m < XB_METHODS && !budget_expired(); ++m) {
            validation[m][0] = '\0';
            valid[m] = xb_backend_validate((enum xb_method)m, validation[m], sizeof(validation[m]));
            validation_failure[m] = valid[m] ? XB_FAILURE_NONE : xb_backend_failure();
            if (!valid[m] && validation_failure[m] == XB_FAILURE_NONE)
                validation_failure[m] = XB_FAILURE_VALIDATION;
            printf("Validation %-8s: %s %s\n", xb_method_name((enum xb_method)m),
                valid[m] ? "PASS" : "FAIL", validation[m]);
        }
        rest_after_work(&o, xb_now() - active_start, w, h);
        printf("%-38s %9s %9s %9s %9s %10s %6s\n", "Test", "total_ms", "prep_ms",
            "render_ms", "swap_ms", "facets/sec", "calls");
        for (i = 0; i < selected && !xb_quit && !budget_expired(); ++i) {
            r = &results[nresult++]; r->test = cases[order[i]]; r->width = w; r->height = h;
            fprintf(stderr, "[%dx%d] %s\n", w, h, r->test.name);
            active_start = xb_now();
            measure(&o, r, valid, validation_failure, validation);
            active = xb_now() - active_start;
            print_result(csv, r);
            if (r->passed) ++successes;
            fflush(stdout);
            if (csv && fflush(csv) != 0) {
                perror("writing CSV"); xb_quit = 1;
            }
            if (nresult < planned) rest_after_work(&o, active, w, h);
        }
        findings(results, nresult, w, h);
    }
    if (sizes > 1 && !xb_quit) compare_sizes(results, nresult);
    attempted = nresult;
    while (nresult < planned) {
        r = &results[nresult];
        r->test = cases[order[nresult % selected]];
        r->width = o.force_size ? o.width : (nresult / selected ? 640 : 320);
        r->height = o.force_size ? o.height : (nresult / selected ? 480 : 240);
        r->failure = XB_FAILURE_NOT_RUN;
        strcpy(r->reason, xb_budget_reached ? "not run: profile time budget reached" :
                                           "not run: run interrupted or setup failed");
        csv_row(csv, r, "summary", -1, 0, 0, 0, 0, 0);
        ++nresult;
    }
    memset(failures, 0, sizeof(failures));
    for (i = 0; i < nresult; ++i) ++failures[results[i].failure];
    if (xb_budget_reached)
        printf("\nProfile time budget reached; %d selected cases not run.\n", planned - attempted);
    printf("\nCompleted %d cases; %d skipped/failed. %s\n", successes, attempted - successes,
        xb_quit ? "Run interrupted or output/resize failed." :
        xb_budget_reached ? "Stopped at profile time limit." : "Run finished.");
    printf("Coverage: %d planned; %d passed; %d API errors; %d validation failures; "
           "%d unsupported; %d other errors; %d time-limit discards; %d not run.\n",
           planned, successes, failures[XB_FAILURE_API], failures[XB_FAILURE_VALIDATION],
           failures[XB_FAILURE_UNSUPPORTED], failures[XB_FAILURE_ERROR],
           failures[XB_FAILURE_TIME_LIMIT], failures[XB_FAILURE_NOT_RUN]);
    printf("Elapsed run time: %.1f seconds (including setup and idle).\n", xb_now() - run_start);
    xb_backend_close();
    free(results);
    if (csv && fclose(csv) != 0) { perror("closing CSV"); xb_watchdog_stop(); return 1; }
    if (fflush(stdout) != 0) { perror("writing report"); xb_watchdog_stop(); return 1; }
    xb_watchdog_stop();
    return xb_quit || !successes ? 1 : 0;
}
