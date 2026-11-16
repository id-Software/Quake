/* Test double for portable CLI/report tests, NOT a graphics implementation.
 * GPL-2.0-or-later; see COPYING.
 */
#include "xglbench.h"
#include <stdlib.h>
#include <string.h>

#ifdef XB_PROFILE_TEST_CLOCK
int xb_test_paused(void);
void xb_test_advance(double seconds);
static double test_draw_advance;
static unsigned long test_draw_advance_at;
#endif

static int test_width;
static int test_height;
static size_t test_triangles;
static size_t test_strips;
static unsigned long test_draws;
static unsigned long test_fault_draw;
static int test_fault_pending;
static int test_fault_fired;
static const char *test_fault;
static const char *test_error = "";
static enum xb_failure test_failure;
static int test_batch = 128;
static int test_zbuffer;
static xb_case test_case;
static int test_lighting_bad;
static unsigned long test_material_checksum;
static unsigned long test_vertex_material_checksum;
static unsigned long test_light_updates;
static unsigned long test_fail_light_update;
static xb_rgb *test_native_colors;
static size_t test_native_color_count;
static unsigned long test_native_color_checksum;
static xb_light_state test_lights;
static float *test_native_texture_points;
static size_t test_texture_point_count;
static unsigned long test_texture_checksum;
static int test_texture_active;
static int test_texture_bad;

typedef struct {
    xb_rgb *colors;
    unsigned long checksum;
    int pending;
} test_slot;

static test_slot test_slots[XB_MAX_QUEUE];
static int test_slot_count;
static int test_outstanding;
static int test_peak;
static size_t test_stream_triangles;
static size_t test_stream_color_count;
static unsigned long test_submitted;
static unsigned long test_completed;
static unsigned long test_nonempty_drains;
static unsigned long test_fail_drain;
static double test_drain_delay;
static int test_stream_bad;
static int test_stream_fail(const char *error);
static unsigned long test_checksum(const xb_rgb *colors, size_t count);
static unsigned long test_bytes_checksum(const void *data, size_t count);
static void test_log(const char *operation, int width, int height);

static const char *test_filter_name(enum xb_texture_filter filter)
{
    if (filter == XB_TEXTURE_POINT) return "point";
    if (filter == XB_TEXTURE_BILINEAR) return "bilinear";
    if (filter == XB_TEXTURE_TRILINEAR) return "trilinear";
    return "off";
}

static int test_texture_fail(const char *error, char *reason, size_t size)
{
    test_error = error;
    test_texture_bad = 1;
    test_log("texture-error", (int)test_case.texture_filter, 0);
    if (size) {
        strncpy(reason, error, size - 1);
        reason[size - 1] = '\0';
    }
    return 0;
}

static void test_log(const char *operation, int width, int height)
{
    const char *path;
    FILE *file;
    path = getenv("XB_TEST_LOG");
    if (!path) return;
    file = fopen(path, "a");
    if (!file) return;
    fprintf(file, "%s %d %d\n", operation, width, height);
    fclose(file);
}

int xb_backend_open(const xb_options *options, int width, int height)
{
    const char *draw, *delay, *drain, *light;
    (void)options;
    test_width = width;
    test_height = height;
    test_draws = 0;
    test_fault_pending = test_fault_fired = 0;
    test_error = "";
    test_failure = XB_FAILURE_NONE;
    test_stream_bad = 0;
    test_lighting_bad = 0;
    test_light_updates = 0;
    test_native_colors = NULL;
    test_native_color_count = 0;
    test_native_texture_points = NULL;
    test_texture_point_count = 0;
    test_texture_active = test_texture_bad = 0;
    memset(&test_case, 0, sizeof(test_case));
    memset(&test_lights, 0, sizeof(test_lights));
    test_slot_count = test_outstanding = test_peak = 0;
    test_submitted = test_completed = test_nonempty_drains = 0;
    memset(test_slots, 0, sizeof(test_slots));
    delay = getenv("XB_TEST_DRAIN_DELAY_MS");
    test_drain_delay = delay ? atof(delay) * .001 : 0;
    drain = getenv("XB_TEST_FAIL_DRAIN");
    test_fail_drain = drain ? strtoul(drain, NULL, 10) : 0;
    light = getenv("XB_TEST_FAIL_LIGHT_UPDATE");
    test_fail_light_update = light ? strtoul(light, NULL, 10) : 0;
    test_fault = getenv("XB_TEST_FAULT");
    draw = getenv("XB_TEST_FAULT_DRAW");
    test_fault_draw = draw ? strtoul(draw, NULL, 10) : 5;
    test_log("open", width, height);
#ifdef XB_PROFILE_TEST_CLOCK
    delay = getenv("XB_TEST_OPEN_ADVANCE");
    if (delay) xb_test_advance(atof(delay));
    delay = getenv("XB_TEST_DRAW_ADVANCE");
    test_draw_advance = delay ? atof(delay) : 0;
    draw = getenv("XB_TEST_DRAW_ADVANCE_AT");
    test_draw_advance_at = draw ? strtoul(draw, NULL, 10) : 0;
#endif
    return 1;
}

void xb_backend_close(void)
{
    int i;
    for (i = 0; i < test_slot_count; ++i) free(test_slots[i].colors);
    memset(test_slots, 0, sizeof(test_slots));
    test_slot_count = 0;
    free(test_native_colors);
    test_native_colors = NULL;
    free(test_native_texture_points);
    test_native_texture_points = NULL;
    test_log("close", test_width, test_height);
}

int xb_backend_resize(int width, int height)
{
    if (test_width == width && test_height == height) return 1;
    test_width = width;
    test_height = height;
    test_log("resize", width, height);
    return 1;
}

void xb_backend_dimensions(int *width, int *height)
{
    *width = test_width;
    *height = test_height;
}

void xb_backend_info(FILE *out)
{
    fprintf(out, "TEST DOUBLE: NO GRAPHICS, NO VALID HARDWARE PERFORMANCE DATA\n");
    fprintf(out, "Portable scheduling/report tests only; window=%dx%d\n",
            test_width, test_height);
}

int xb_backend_validate(enum xb_method method, char *reason, size_t size)
{
    const char *failed;
    test_failure = XB_FAILURE_NONE;
    test_error = "";
    failed = getenv("XB_TEST_FAIL_METHOD");
    if (failed && !strcmp(failed, xb_method_name(method))) {
        test_failure = XB_FAILURE_VALIDATION;
        if (size) {
            strncpy(reason, "injected test-only validation failure", size - 1);
            reason[size - 1] = '\0';
        }
        return 0;
    }
    if (size) reason[0] = '\0';
    return 1;
}

static int test_lighting_fail(const char *error)
{
    test_error = error;
    test_lighting_bad = 1;
    test_log("lighting-error", (int)test_case.lighting, (int)test_case.shade);
    return 0;
}

static int test_unit_normal(const xb_point *normal)
{
    double length;
    length = (double)normal->x * normal->x +
             (double)normal->y * normal->y +
             (double)normal->z * normal->z;
    /* Ordered comparisons reject NaNs as well as incorrect normalization. */
    return length >= .999 && length <= 1.001;
}

static int test_color_valid(const xb_rgb *color)
{
    return color->r >= 0 && color->r <= 1 &&
           color->g >= 0 && color->g <= 1 &&
           color->b >= 0 && color->b <= 1;
}

static int test_lighting_mesh(const xb_mesh *mesh)
{
    size_t i;
    if (!mesh || !mesh->lighting_ready || !mesh->triangles ||
        mesh->point_count != mesh->triangles * 3 || !mesh->base_colors ||
        !mesh->colors || !mesh->facet_normals || !mesh->vertex_normals ||
        !mesh->centers || !mesh->vertex_colors || !mesh->facet_styles ||
        !mesh->vertex_styles || (test_case.vertex_material && !mesh->vertex_materials))
        return test_lighting_fail("mock lighting geometry is incomplete");
    for (i = 0; i < mesh->triangles; ++i) {
        if (!test_unit_normal(&mesh->facet_normals[i]) ||
            !test_color_valid(&mesh->colors[i]) ||
            !test_color_valid(&mesh->base_colors[i]))
            return test_lighting_fail("mock facet normal/color is invalid");
        if (test_case.xgl_lighting &&
            memcmp(&mesh->colors[i], &mesh->base_colors[i], sizeof(xb_rgb)))
            return test_lighting_fail("hardware lighting received already-lit facet RGB");
    }
    for (i = 0; i < mesh->point_count; ++i) {
        if (!test_unit_normal(&mesh->vertex_normals[i]) ||
            !test_color_valid(&mesh->vertex_colors[i]))
            return test_lighting_fail("mock vertex normal/color is invalid");
        if (test_case.xgl_lighting &&
            memcmp(&mesh->vertex_colors[i], test_case.vertex_material ?
                   &mesh->vertex_materials[i] : &mesh->base_colors[i / 3], sizeof(xb_rgb)))
            return test_lighting_fail("hardware lighting received already-lit vertex RGB");
    }
    return 1;
}

int xb_backend_configure(const xb_case *test, const xb_mesh *mesh,
                         char *reason, size_t size)
{
    const char *failed;
    size_t i, positive, negative;
    const xb_point *p;
    double cross;
    test_case = *test;
    test_batch = test->batch > 0 ? test->batch : 128;
    test_zbuffer = test->zbuffer;
    test_lighting_bad = 0;
    test_error = "";
    test_failure = XB_FAILURE_NONE;
    test_light_updates = 0;
    test_texture_active = 0;
    test_texture_bad = 0;
    memset(&test_lights, 0, sizeof(test_lights));
    test_log("lighting-configure", (int)test->lighting, (int)test->shade);
    test_log("lighting-compute", test->xgl_lighting, test->lights);
    test_log("lighting-style", test->styles, test->moving);
    test_log("culling-configure", test->cull, (int)test->method);
    test_log("material-configure", test->vertex_material, (int)test->shade);
    test_log("texture-configure", (int)test->texture_filter, test->texture_compare);
    if (test->texture_compare) {
        if (!mesh || mesh->point_count != mesh->triangles * 3 || !mesh->texcoords)
            return test_texture_fail("texture comparison lacks per-corner UV coordinates", reason, size);
        for (i = 0; i < mesh->point_count; ++i)
            if (!(mesh->texcoords[i].u > -1000000 && mesh->texcoords[i].u < 1000000 &&
                  mesh->texcoords[i].v > -1000000 && mesh->texcoords[i].v < 1000000) ||
                mesh->texcoords[i].gain != 1)
                return test_texture_fail("texture comparison has invalid UV or unmatched material gain", reason, size);
    }
    if (test->texture_filter != XB_TEXTURE_OFF) {
        if (!test->texture_compare || test->method != XB_MULTI || test->shade != XB_FLAT ||
            test->lighting != XB_LIGHT_OFF || test->vertex_material || test->cull ||
            (test->work != XB_DRAW && test->work != XB_FRAME) ||
            (test->update != XB_STATIC && test->update != XB_TRANSFORM))
            return test_texture_fail("unsupported mock native texture workload", reason, size);
        failed = getenv("XB_TEST_FAIL_TEXTURE");
        if (failed && (!strcmp(failed, "all") ||
                       !strcmp(failed, test_filter_name(test->texture_filter)))) {
            test_failure = XB_FAILURE_UNSUPPORTED;
            return test_texture_fail("injected texture mapping unavailable; no untextured fallback", reason, size);
        }
        test_log("texture-setup", (int)test->texture_filter, XB_TEXTURE_WIDTH);
    }
    if (!strncmp(test->name, "cull/", 5)) {
        positive = negative = 0;
        for (i = 0; i < mesh->triangles; ++i) {
            p = mesh->points + i * 3;
            cross = (double)(p[1].x - p[0].x) * (p[2].y - p[0].y) -
                    (double)(p[1].y - p[0].y) * (p[2].x - p[0].x);
            if (cross > 0) ++positive;
            if (cross < 0) ++negative;
        }
        test_log("culling-winding", (int)positive, (int)negative);
        if (positive + negative != mesh->triangles ||
            (test->mixed_winding ? positive != negative : positive != 0))
            return test_lighting_fail("culling fixture has incorrect triangle winding");
    }
    if (size) reason[0] = '\0';
    failed = getenv("XB_TEST_FAIL_LIGHTING");
    if (test->lighting != XB_LIGHT_OFF && failed && *failed) {
        test_failure = XB_FAILURE_VALIDATION;
        test_lighting_fail("injected lighting readback validation failure");
        if (size) {
            strncpy(reason, test_error, size - 1);
            reason[size - 1] = '\0';
        }
        return 0;
    }
    if (test->lighting != XB_LIGHT_OFF) {
        if ((test->method != XB_MULTI && test->method != XB_POLYGON) ||
            !test_lighting_mesh(mesh)) {
            if (size) {
                strncpy(reason, test_error, size - 1);
                reason[size - 1] = '\0';
            }
            return 0;
        }
        test_material_checksum = test_checksum(mesh->base_colors, mesh->triangles);
        test_vertex_material_checksum = test->vertex_material ?
            test_checksum(mesh->vertex_materials, mesh->point_count) : 0;
        xb_lighting_state(test, 0, &test_lights);
    }
    if (test->vertex_material) {
        if (test->shade != XB_SMOOTH || !mesh || !mesh->vertex_colors ||
            !mesh->vertex_materials || mesh->point_count != mesh->triangles * 3)
            return test_lighting_fail("invalid mock vertex-material geometry");
        for (i = 0; i < mesh->point_count; ++i)
            if (!test_color_valid(&mesh->vertex_colors[i]))
                return test_lighting_fail("invalid mock vertex-material color");
        if (test->lighting == XB_LIGHT_OFF &&
            memcmp(mesh->vertex_colors, mesh->vertex_materials, mesh->point_count * sizeof(xb_rgb)))
            return test_lighting_fail("unlit texture case changed sampled vertex material");
    }
    if (!xb_backend_upload(mesh)) return 0;
    if (test->texture_filter != XB_TEXTURE_OFF && size) {
        strncpy(reason, "TEST DOUBLE: native texture scheduling only; acceleration unclassified", size - 1);
        reason[size - 1] = '\0';
    }
    return 1;
}

int xb_backend_upload(const xb_mesh *mesh)
{
    const xb_rgb *colors;
    size_t i;
    free(test_native_colors);
    test_native_colors = NULL;
    test_native_color_count = 0;
    test_triangles = mesh->triangles;
    test_strips = mesh->strip_count;
    free(test_native_texture_points);
    test_native_texture_points = NULL;
    test_texture_point_count = 0;
    if (test_case.texture_filter != XB_TEXTURE_OFF) {
        test_texture_point_count = mesh->point_count;
        test_native_texture_points = (float *)malloc(mesh->point_count * 5 * sizeof(float));
        if (!test_native_texture_points)
            return test_texture_fail("mock texture payload allocation failed", NULL, 0);
        for (i = 0; i < mesh->point_count; ++i) {
            test_native_texture_points[i * 5] = mesh->points[i].x;
            test_native_texture_points[i * 5 + 1] = mesh->points[i].y;
            test_native_texture_points[i * 5 + 2] = mesh->points[i].z;
            test_native_texture_points[i * 5 + 3] = mesh->texcoords[i].u;
            test_native_texture_points[i * 5 + 4] = mesh->texcoords[i].v;
        }
        test_texture_checksum = test_bytes_checksum(test_native_texture_points,
                                   mesh->point_count * 5 * sizeof(float));
        test_log("texture-payload", (int)mesh->point_count, 5);
    }
    if (test_case.lighting != XB_LIGHT_OFF || test_case.vertex_material) {
        test_native_color_count = test_case.shade == XB_SMOOTH ?
                                  mesh->point_count : mesh->triangles;
        test_native_colors = (xb_rgb *)malloc(test_native_color_count * sizeof(xb_rgb));
        if (!test_native_colors)
            return test_lighting_fail("mock lighting color allocation failed");
        colors = test_case.shade == XB_SMOOTH ? mesh->vertex_colors : mesh->colors;
        memcpy(test_native_colors, colors, test_native_color_count * sizeof(xb_rgb));
        test_native_color_checksum = test_checksum(test_native_colors, test_native_color_count);
        test_log("native-color-layout", (int)test_case.shade, (int)test_native_color_count);
    }
    return 1;
}

void xb_backend_colors(const xb_mesh *mesh)
{
    const xb_rgb *colors;
    unsigned long checksum;
    if (test_case.lighting == XB_LIGHT_OFF) return;
    if (!test_lighting_mesh(mesh)) return;
    if (test_case.xgl_lighting) {
        test_lighting_fail("XGL lighting unexpectedly uploaded CPU-lit colors");
        return;
    }
    if (test_checksum(mesh->base_colors, mesh->triangles) != test_material_checksum ||
        (test_case.vertex_material && test_checksum(mesh->vertex_materials,
          mesh->point_count) != test_vertex_material_checksum)) {
        test_lighting_fail("lighting update mutated cached material RGB");
        return;
    }
    colors = test_case.shade == XB_SMOOTH ? mesh->vertex_colors : mesh->colors;
    checksum = test_checksum(colors, test_native_color_count);
    test_log("lighting-colors", (int)test_case.shade,
             checksum != test_native_color_checksum);
    memcpy(test_native_colors, colors, test_native_color_count * sizeof(xb_rgb));
    test_native_color_checksum = checksum;
}

int xb_backend_lighting(const xb_light_state *state)
{
    size_t i;
    if (!test_case.xgl_lighting || test_case.lighting != XB_LIGHT_LAMBERT ||
        !state || state->count != test_case.lights || state->count < 1 ||
        state->count > XB_MAX_LIGHTS || state->directional != test_case.directional)
        return test_lighting_fail("invalid mock light-state update");
    if (!(state->ambient >= 0 && state->ambient <= 1) ||
        !(state->attenuation1 > 0) || !(state->attenuation2 >= 0))
        return test_lighting_fail("invalid mock light coefficients");
    for (i = 0; i < (size_t)state->count; ++i)
        if (!test_color_valid(&state->color[i]))
            return test_lighting_fail("invalid mock light color");
    ++test_light_updates;
    if (test_fail_light_update && test_light_updates == test_fail_light_update)
        return test_lighting_fail("injected light-state update failure");
    test_log("lighting-update", state->count, state->directional);
    test_lights = *state;
    return 1;
}
void xb_backend_transform(unsigned long frame)
{
    (void)frame;
    if (test_case.texture_compare)
        test_log("texture-transform", test_case.perspective, (int)test_case.texture_filter);
}
void xb_backend_clear(int color, int depth)
{
    if (test_slot_count) test_log("queue-clear", color, depth);
    if (test_case.work == XB_FRAME) test_log("frame-clear", color, depth);
}

size_t xb_backend_draw(enum xb_method method, int batch)
{
    double until;
    ++test_draws;
    test_texture_active = test_case.texture_filter != XB_TEXTURE_OFF;
    if (test_case.texture_compare) {
        test_log("texture-draw", test_texture_active, (int)test_case.texture_filter);
        if (test_texture_active && (method != XB_MULTI || !test_native_texture_points ||
            test_texture_point_count != test_triangles * 3 ||
            test_texture_checksum != test_bytes_checksum(test_native_texture_points,
                test_texture_point_count * 5 * sizeof(float)))) {
            test_texture_fail("mock native texture input changed during timed drawing", NULL, 0);
            return 0;
        }
    }
    if (test_case.work == XB_FRAME) test_log("frame-draw", (int)method, batch);
#ifdef XB_PROFILE_TEST_CLOCK
    if (test_draw_advance_at && test_draws == test_draw_advance_at) {
        test_log("draw-advance", (int)test_draws, (int)test_draw_advance);
        xb_test_advance(test_draw_advance);
    }
#endif
    if (test_fault && !test_fault_fired) {
        /* Force the .001-second regression trial to end at its minimum
         * frame count. This is test scheduling, not simulated GPU work. */
        until = xb_now() + .002;
        while (xb_now() < until) { }
        if (test_draws >= test_fault_draw) {
            test_fault_pending = test_fault_fired = 1;
            test_log(test_fault, test_width, test_height);
        }
    }
    if (method == XB_POLYGON) return test_triangles;
    if (method == XB_STRIP) return test_strips;
    if (batch < 1) batch = 1;
    return (test_triangles + (size_t)batch - 1) / (size_t)batch;
}

void xb_backend_sync(void)
{
    if (test_outstanding)
        test_stream_fail("stream work synchronized outside its completion drain");
}
void xb_backend_swap(void)
{
    if (test_slot_count) test_stream_fail("streaming test unexpectedly swapped");
    if (test_case.work == XB_FRAME) test_log("frame-swap", 0, 0);
}
void xb_backend_raster(int masked)
{
    int texture_was_active;
    texture_was_active = test_texture_active;
    test_texture_active = 0;
    if (test_case.texture_compare) test_log("texture-hud", test_texture_active, masked);
    if (test_slot_count) test_stream_fail("streaming test unexpectedly drew a HUD");
    if (test_case.work == XB_FRAME) test_log("frame-hud", masked, 0);
    test_texture_active = texture_was_active;
}
int xb_backend_events(void)
{
#ifdef XB_PROFILE_TEST_CLOCK
    if (getenv("XB_TEST_CANCEL_ON_PAUSE") && xb_test_paused()) {
        test_log("pause-cancel", test_width, test_height);
        test_error = "";
    test_failure = XB_FAILURE_NONE;
        return 0;
    }
#endif
    if (test_stream_bad || test_lighting_bad) return 0;
    /* Unsupported mapping is a recoverable case failure, including while the
     * runner polls during protective rest before reconfiguring the next case. */
    if (test_texture_bad) return -1;
    if (!test_fault_pending) return 1;
    test_fault_pending = 0;
    if (!strcmp(test_fault, "native-error")) {
        test_failure = XB_FAILURE_API;
        test_error = "injected native completion failure";
        return 0;
    }
    if (!strcmp(test_fault, "resize")) {
        test_width += 16;
        return -1;
    }
    if (!strcmp(test_fault, "expose")) return -1;
    if (!strcmp(test_fault, "interrupt")) return 0;
    return 1;
}
void xb_backend_label(const char *name) { (void)name; }
int xb_backend_has_double_buffer(void) { return 1; }
int xb_backend_has_hardware(void) { return 0; }
const char *xb_backend_error(void) { return test_error; }
enum xb_failure xb_backend_failure(void)
{
    return test_failure != XB_FAILURE_NONE ? test_failure :
           (*test_error ? XB_FAILURE_ERROR : XB_FAILURE_NONE);
}

size_t xb_backend_input_bytes(enum xb_method method, int batch)
{
    size_t points, facets;
    (void)batch;
    /* A test-double payload estimate only, never native transfer bandwidth. */
    points = method == XB_STRIP ? test_triangles + test_strips * 2 : test_triangles * 3;
    facets = method == XB_TLIST ? test_triangles * 3 : test_triangles;
    if (test_case.texture_filter != XB_TEXTURE_OFF) return points * 5 * sizeof(float);
    if (test_case.lighting != XB_LIGHT_OFF || test_case.vertex_material) {
        if (test_case.shade == XB_SMOOTH)
            return points * (sizeof(xb_point) + sizeof(xb_rgb) +
                   (test_case.xgl_lighting ? sizeof(xb_point) : 0));
        return points * sizeof(xb_point) + facets *
               (sizeof(xb_rgb) + (test_case.xgl_lighting ? sizeof(xb_point) : 0));
    }
    return points * sizeof(xb_point) + facets * sizeof(xb_rgb);
}

static int test_stream_fail(const char *error)
{
    test_error = error;
    test_stream_bad = 1;
    test_log("queue-error", test_outstanding, test_slot_count);
    return 0;
}

static unsigned long test_checksum(const xb_rgb *colors, size_t count)
{
    return test_bytes_checksum(colors, count * sizeof(*colors));
}

static unsigned long test_bytes_checksum(const void *data, size_t count)
{
    const unsigned char *bytes;
    size_t i;
    unsigned long checksum;
    bytes = (const unsigned char *)data;
    checksum = 0;
    for (i = 0; i < count; ++i)
        checksum = checksum * 33UL + bytes[i];
    return checksum;
}

int xb_backend_stream_begin(const xb_mesh *mesh, int slots, char *reason, size_t size)
{
    int i;
    const xb_rgb *colors;
    if (size) reason[0] = '\0';
    if (!mesh || !mesh->triangles || !mesh->colors || slots < 1 || slots > XB_MAX_QUEUE)
        return test_stream_fail("invalid mock stream setup");
    if (test_slot_count || test_outstanding)
        return test_stream_fail("previous mock stream was not released");
    if (test_case.xgl_lighting || test_case.method != XB_MULTI ||
        test_case.texture_filter != XB_TEXTURE_OFF)
        return test_stream_fail("unsupported mock streaming lighting or primitive");
    if (test_case.lighting != XB_LIGHT_OFF && !test_lighting_mesh(mesh)) return 0;
    test_stream_triangles = mesh->triangles;
    test_stream_color_count = test_case.shade == XB_SMOOTH ? mesh->point_count : mesh->triangles;
    colors = test_case.shade == XB_SMOOTH ? mesh->vertex_colors : mesh->colors;
    if (!colors) return test_stream_fail("missing mock stream color payload");
    test_outstanding = test_peak = 0;
    test_submitted = test_completed = test_nonempty_drains = 0;
    for (i = 0; i < slots; ++i) {
        test_slots[i].colors = (xb_rgb *)malloc(test_stream_color_count * sizeof(xb_rgb));
        if (!test_slots[i].colors) {
            test_slot_count = i;
            xb_backend_stream_end();
            return test_stream_fail("mock stream allocation failed");
        }
        memcpy(test_slots[i].colors, colors, test_stream_color_count * sizeof(xb_rgb));
        test_slots[i].pending = 0;
    }
    test_slot_count = slots;
    test_log("queue-begin", slots, (int)mesh->triangles);
    test_log("queue-color-layout", (int)test_case.shade, (int)test_stream_color_count);
    return 1;
}

int xb_backend_stream_prepare(int slot, const xb_mesh *mesh)
{
    const xb_rgb *colors;
    size_t count;
    unsigned long checksum;
    if (slot < 0 || slot >= test_slot_count || !mesh ||
        mesh->triangles != test_stream_triangles || !mesh->colors)
        return test_stream_fail("invalid mock stream slot");
    if (test_slots[slot].pending)
        return test_stream_fail("in-flight stream slot overwritten before drain");
    if (test_case.lighting != XB_LIGHT_OFF) {
        if (!test_lighting_mesh(mesh)) return 0;
        if (test_checksum(mesh->base_colors, mesh->triangles) != test_material_checksum ||
            (test_case.vertex_material && test_checksum(mesh->vertex_materials,
              mesh->point_count) != test_vertex_material_checksum))
            return test_lighting_fail("stream update mutated cached material RGB");
    }
    count = test_case.shade == XB_SMOOTH ? mesh->point_count : mesh->triangles;
    colors = test_case.shade == XB_SMOOTH ? mesh->vertex_colors : mesh->colors;
    if (count != test_stream_color_count || !colors)
        return test_stream_fail("stream update changed native color layout");
    checksum = test_checksum(colors, count);
    if (test_case.lighting != XB_LIGHT_OFF)
        test_log("lighting-colors", (int)test_case.shade,
                 checksum != test_checksum(test_slots[slot].colors, count));
    /* The source may be reused immediately. No allocation occurs here. */
    memcpy(test_slots[slot].colors, colors, count * sizeof(xb_rgb));
    test_log("queue-prepare", slot, test_outstanding);
    return 1;
}

size_t xb_backend_stream_draw(int slot)
{
    if (slot < 0 || slot >= test_slot_count || test_slots[slot].pending) {
        test_stream_fail("invalid or still-pending mock stream draw slot");
        return 0;
    }
    xb_backend_clear(1, test_zbuffer);
    test_slots[slot].pending = 1;
    test_slots[slot].checksum = test_checksum(test_slots[slot].colors, test_stream_color_count);
    ++test_outstanding;
    ++test_submitted;
    if (test_outstanding > test_peak) test_peak = test_outstanding;
    test_log("queue-draw", slot, test_outstanding);
    if (test_outstanding > test_slot_count) {
        test_stream_fail("mock stream queue exceeded its limit");
        return 0;
    }
    /* Reuse the existing final-draw fault injection without completing work. */
    return xb_backend_draw(XB_MULTI, test_batch);
}

void xb_backend_post(void)
{
    test_log("queue-post", test_outstanding, (int)test_completed);
}

int xb_backend_stream_drain(void)
{
    double until;
    int i;
    if (test_outstanding) {
        ++test_nonempty_drains;
        if (test_fail_drain && test_nonempty_drains == test_fail_drain) {
            test_log("queue-drain-failed", test_outstanding, (int)test_nonempty_drains);
            return test_stream_fail("injected stream completion failure");
        }
        /* A controlled delay detects timers stopped before completion. It is
         * deliberately not a device performance simulation. */
        until = xb_now() + test_drain_delay;
        while (xb_now() < until) { }
    }
    for (i = 0; i < test_slot_count; ++i) {
        if (!test_slots[i].pending) continue;
        if (test_slots[i].checksum != test_checksum(test_slots[i].colors, test_stream_color_count))
            return test_stream_fail("native colors changed while mock queue retained them");
        test_slots[i].pending = 0;
        ++test_completed;
    }
    test_log("queue-drain", test_outstanding, (int)test_completed);
    test_outstanding = 0;
    return 1;
}

void xb_backend_stream_end(void)
{
    int i;
    if (!test_slot_count) return;
    /* Mirrors native cleanup: finish retained data before releasing it. */
    if (test_outstanding && !xb_backend_stream_drain()) return;
    test_log("queue-end", (int)test_submitted, (int)test_completed);
    for (i = 0; i < test_slot_count; ++i) free(test_slots[i].colors);
    memset(test_slots, 0, sizeof(test_slots));
    test_slot_count = 0;
}
