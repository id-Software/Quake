/* Native backend control-flow regression tests, GPL-2.0-or-later.
 * Uses deliberately small XGL stubs, not the Sun SDK or a rendering emulator.
 * Link with function/data sections and --gc-sections on a modern GNU host;
 * unrelated native/Xlib entry points are not linked into this executable.
 */
#include <assert.h>
#include <stdarg.h>
#include "../xglbench_xgl.c"

typedef struct {
    Xgl_usgn32 depth, width, height;
    Xgl_color_type color_type;
    Xgl_usgn32 pixels[XB_TEXTURE_WIDTH * XB_TEXTURE_WIDTH];
    int live;
} mock_raster;

static mock_raster rasters[16];
static int system_token, context_token;
static int creations, destructions, live_objects, fail_create_at;
static int bad_depth, bad_width, bad_height, bad_color, null_address;
static int get_pixel_error, obscured_pixel;
static int depth_queries, width_queries, height_queries, color_queries;
static Xgl_color pixel_color;
static const char *error_message;

static void inject_error(const char *message)
{
    error_message = message;
    (void)xgl_error(b.system);
}

int xb_cancel_requested(void) { return 0; }

Xgl_object xgl_object_create(Xgl_sys_state system, int type,
                             Xgl_obj_desc *descriptor, ...)
{
    va_list args;
    int attribute, color_attributes, sizing_attributes;
    mock_raster *raster;
    assert(system == b.system);
    assert(type == XGL_MEM_RAS);
    assert(descriptor == NULL);
    assert(creations < (int)(sizeof(rasters) / sizeof(rasters[0])));
    raster = &rasters[creations++];
    memset(raster, 0, sizeof(*raster));
    /* Depth controls storage; it does not select the logical color type. */
    raster->color_type = XGL_COLOR_INDEX;
    color_attributes = sizing_attributes = 0;
    va_start(args, descriptor);
    while ((attribute = va_arg(args, int)) != 0) {
        switch (attribute) {
        case XGL_DEV_COLOR_TYPE:
            /* Allocation can fix this write-once attribute. Set it first. */
            assert(color_attributes == 0 && sizing_attributes == 0);
            ++color_attributes;
            raster->color_type = (Xgl_color_type)va_arg(args, int);
            assert(raster->color_type == XGL_COLOR_RGB);
            break;
        case XGL_RAS_DEPTH:
            ++sizing_attributes;
            raster->depth = va_arg(args, Xgl_usgn32);
            break;
        case XGL_RAS_WIDTH:
            ++sizing_attributes;
            raster->width = va_arg(args, Xgl_usgn32);
            break;
        case XGL_RAS_HEIGHT:
            ++sizing_attributes;
            raster->height = va_arg(args, Xgl_usgn32);
            break;
        default:
            assert(!"unexpected memory-raster creation attribute");
        }
    }
    va_end(args);
    assert(raster->depth == 1 || raster->depth == 32);
    assert(raster->width * raster->height <= XB_TEXTURE_WIDTH * XB_TEXTURE_WIDTH);
    if (raster->depth == 32) {
        assert(color_attributes == 1 && raster->color_type == XGL_COLOR_RGB);
    } else {
        assert(color_attributes == 0 && raster->color_type == XGL_COLOR_INDEX);
    }
    raster->live = 1;
    ++live_objects;
    if (creations == fail_create_at) inject_error("injected raster creation error");
    /* XGL can return a handle while its notification callback reports failure. */
    return raster;
}

void xgl_object_destroy(Xgl_object object)
{
    mock_raster *raster;
    raster = (mock_raster *)object;
    assert(raster >= rasters && raster < rasters + creations);
    assert(raster->live);
    raster->live = 0;
    ++destructions;
    --live_objects;
}

void xgl_object_get(Xgl_object object, int attribute, void *value)
{
    mock_raster *raster;
    Xgl_error_info *info;
    if (object == b.system) {
        assert(attribute == XGL_SYS_ST_ERROR_INFO);
        info = (Xgl_error_info *)value;
        memset(info, 0, sizeof(*info));
        info->id = "test-error";
        info->msg = (char *)error_message;
        info->cur_op = "test operation";
        info->cur_obj = "test object";
        info->operand1 = "test operand 1";
        info->operand2 = "test operand 2";
        return;
    }
    raster = (mock_raster *)object;
    assert(raster >= rasters && raster < rasters + creations);
    assert(raster->live);
    switch (attribute) {
    case XGL_RAS_DEPTH:
        ++depth_queries;
        *(Xgl_usgn32 *)value = bad_depth ? 8 : raster->depth;
        break;
    case XGL_RAS_WIDTH:
        ++width_queries;
        *(Xgl_usgn32 *)value = bad_width ? raster->width + 1 : raster->width;
        break;
    case XGL_RAS_HEIGHT:
        ++height_queries;
        *(Xgl_usgn32 *)value = bad_height ? raster->height + 1 : raster->height;
        break;
    case XGL_DEV_COLOR_TYPE:
        ++color_queries;
        *(Xgl_color_type *)value = bad_color ? XGL_COLOR_INDEX : raster->color_type;
        break;
    case XGL_MEM_RAS_IMAGE_BUFFER_ADDR:
        *(void **)value = null_address ? NULL : raster->pixels;
        break;
    default: assert(!"unexpected memory-raster inquiry");
    }
}

Xgl_boolean xgl_context_get_pixel(Xgl_ctx context, Xgl_pt_i2d *point,
                                   Xgl_color *color)
{
    assert(context == b.ctx);
    assert(point->x == 13 && point->y == 17);
    *color = pixel_color;
    if (get_pixel_error) inject_error("injected readback error");
    return obscured_pixel;
}

static void reset_mock(void)
{
    int i;
    /* Cleanup belongs to this test fixture, not an emulated xgl_close. */
    for (i = 0; i < creations; ++i)
        if (rasters[i].live) xgl_object_destroy(&rasters[i]);
    assert(live_objects == 0);
    memset(&b, 0, sizeof(b));
    memset(rasters, 0, sizeof(rasters));
    memset(&pixel_color, 0, sizeof(pixel_color));
    b.system = &system_token;
    b.ctx = &context_token;
    creations = destructions = live_objects = fail_create_at = 0;
    bad_depth = bad_width = bad_height = bad_color = null_address = 0;
    get_pixel_error = obscured_pixel = 0;
    depth_queries = width_queries = height_queries = color_queries = 0;
    error_message = NULL;
}

static void test_texture_creation_recovery(void)
{
    Xgl_usgn32 *pixels;
    reset_mock();
    fail_create_at = 1;
    assert(texture_raster_pixels(0) == NULL);
    assert(b.texture_rasters[0] == NULL);
    assert(creations == 1 && destructions == 1 && live_objects == 0);
    assert(strstr(b.error, "injected raster creation error") != NULL);
    assert(xb_backend_failure() == XB_FAILURE_API);
    /* Start another case, as configure() does after a recoverable failure. */
    b.errors = 0;
    b.failure = XB_FAILURE_NONE;
    b.error[0] = '\0';
    fail_create_at = 0;
    pixels = texture_raster_pixels(0);
    assert(pixels && b.texture_rasters[0]);
    assert(creations == 2 && live_objects == 1);
    assert(texture_raster_pixels(0) == pixels);
    assert(creations == 2);
}

static void test_raster_inquiries(void)
{
    int fault;
    char color_diagnostic[128];
    for (fault = 0; fault < 5; ++fault) {
        reset_mock();
        bad_depth = fault == 0;
        bad_width = fault == 1;
        bad_height = fault == 2;
        bad_color = fault == 3;
        null_address = fault == 4;
        assert(texture_raster_pixels(0) == NULL);
        assert(b.texture_rasters[0] == NULL);
        assert(creations == 1 && destructions == 1 && live_objects == 0);
        assert(b.error[0] != '\0');
        assert(xb_backend_failure() == XB_FAILURE_VALIDATION);
        if (bad_color) {
            sprintf(color_diagnostic, "expected RGB (%d), got %d",
                    (int)XGL_COLOR_RGB, (int)XGL_COLOR_INDEX);
            assert(strstr(b.error, color_diagnostic) != NULL);
        }
    }
}

static void test_readback_error_preservation(void)
{
    xb_rgb expected, sample;
    expected.r = 1.0f; expected.g = expected.b = 0.0f;
    reset_mock();
    get_pixel_error = 1;
    assert(!pixel_matches(13, 17, &expected));
    assert(strstr(b.error, "injected readback error") != NULL);
    assert(xb_backend_failure() == XB_FAILURE_API);
    /* Even a simultaneously obscured pixel must not hide the native error. */
    reset_mock();
    get_pixel_error = obscured_pixel = 1;
    assert(!pixel_matches(13, 17, &expected));
    assert(strstr(b.error, "injected readback error") != NULL);
    reset_mock();
    obscured_pixel = 1;
    assert(!pixel_matches(13, 17, &expected));
    assert(strstr(b.error, "obscured") != NULL);
    reset_mock();
    assert(!pixel_matches(13, 17, &expected));
    assert(b.error[0] != '\0');
    reset_mock();
    pixel_color.rgb.r = 1.0f;
    assert(pixel_matches(13, 17, &expected));
    reset_mock();
    pixel_color.rgb.r = (float)strtod("nan", NULL);
    assert(!pixel_matches(13, 17, &expected));
    assert(xb_backend_failure() == XB_FAILURE_VALIDATION);
    reset_mock();
    get_pixel_error = obscured_pixel = 1;
    assert(!read_pixel(13, 17, &sample));
    assert(strstr(b.error, "injected readback error") != NULL);
}

static void test_native_memory_accounting(void)
{
    unsigned char *first, *second;
    size_t current, peak, i;
    xb_backend_memory(&current, &peak);
    assert(current == 0);
    xb_backend_memory_reset_peak();
    first = (unsigned char *)native_calloc(7, 3);
    assert(first != NULL);
    for (i = 0; i < 21; ++i) assert(first[i] == 0);
    memset(first, 0x5a, 21);
    xb_backend_memory(&current, &peak);
    assert(current == 21 && peak == 21);
    second = (unsigned char *)native_calloc(11, 5);
    assert(second != NULL);
    xb_backend_memory(&current, &peak);
    assert(current == 76 && peak == 76);
    native_free(first);
    xb_backend_memory(&current, &peak);
    assert(current == 55 && peak == 76);
    xb_backend_memory_reset_peak();
    xb_backend_memory(&current, &peak);
    assert(current == 55 && peak == 55);
    assert(native_calloc((size_t)-1, 2) == NULL);
    assert(native_calloc(1, (size_t)-1) == NULL);
    xb_backend_memory(&current, &peak);
    assert(current == 55 && peak == 55);
    native_free(second);
    native_free(NULL);
    xb_backend_memory(&current, &peak);
    assert(current == 0 && peak == 55);
    xb_backend_memory_reset_peak();
    xb_backend_memory(&current, &peak);
    assert(current == 0 && peak == 0);
}

static void test_native_mesh_memory_lifetime(void)
{
    xb_mesh mesh;
    xb_point points[3];
    xb_rgb color;
    size_t current, peak, one_slot;
    reset_mock();
    memset(&mesh, 0, sizeof(mesh));
    memset(points, 0, sizeof(points));
    memset(&color, 0, sizeof(color));
    mesh.triangles = 1;
    mesh.point_count = 3;
    mesh.points = points;
    mesh.colors = &color;
    b.method = XB_MULTI;
    b.batch = b.stream_batch = 128;
    assert(xb_backend_upload(&mesh));
    xb_backend_memory(&current, &peak);
    assert(current > 0 && peak >= current);
    free_geometry();
    xb_backend_memory(&current, &peak);
    assert(current == 0 && peak > 0);
    xb_backend_memory_reset_peak();
    assert(allocate_stream_slot(0, &mesh));
    xb_backend_memory(&one_slot, &peak);
    assert(one_slot > 0 && peak == one_slot);
    assert(allocate_stream_slot(1, &mesh));
    xb_backend_memory(&current, &peak);
    assert(current == one_slot * 2 && peak == current);
    free_stream_slots();
    xb_backend_memory(&current, &peak);
    assert(current == 0 && peak == one_slot * 2);
    xb_backend_memory_reset_peak();
}

int main(void)
{
    test_texture_creation_recovery();
    test_raster_inquiries();
    test_readback_error_preservation();
    test_native_memory_accounting();
    test_native_mesh_memory_lifetime();
    reset_mock();
    puts("native texture-raster/allocation tests passed (no graphics/SDK validation)");
    return 0;
}
