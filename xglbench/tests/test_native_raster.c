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
    Xgl_usgn32 pixels[XB_RASTER_W * XB_RASTER_H];
    int live;
} mock_raster;

static mock_raster rasters[16];
static int system_token, context_token;
static int creations, destructions, live_objects, fail_create_at;
static int bad_depth, bad_width, bad_height, bad_color, null_address;
static int get_pixel_error, obscured_pixel, fail_attachment;
static int depth_queries, width_queries, height_queries, color_queries;
static int raster_probe, pixel_reads, raster_copies;
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
    assert(raster->width * raster->height <= XB_RASTER_W * XB_RASTER_H);
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

void xgl_object_set(Xgl_object object, ...)
{
    va_list args;
    int attribute;
    mock_raster *mask;
    Xgl_pt_i2d *position;
    assert(object == b.ctx);
    va_start(args, object);
    while ((attribute = va_arg(args, int)) != 0) {
        switch (attribute) {
        case XGL_CTX_RASTER_FPAT:
            mask = (mock_raster *)va_arg(args, Xgl_mem_ras);
            assert(mask && mask->live && mask->depth == 1);
            break;
        case XGL_CTX_ROP:
            assert(va_arg(args, int) == XGL_ROP_COPY);
            break;
        case XGL_CTX_NEW_FRAME_ACTION:
        case XGL_3D_CTX_HLHSR_MODE:
        case XGL_CTX_RASTER_FILL_STYLE:
            (void)va_arg(args, int);
            break;
        case XGL_CTX_RASTER_FPAT_POSITION:
            position = va_arg(args, Xgl_pt_i2d *);
            assert(position->x == 13 && position->y == 17);
            break;
        default: assert(!"unexpected HUD context attribute");
        }
    }
    va_end(args);
    if (fail_attachment) inject_error("injected HUD attachment error");
}

Xgl_boolean xgl_context_get_pixel(Xgl_ctx context, Xgl_pt_i2d *point,
                                   Xgl_color *color)
{
    assert(context == b.ctx);
    assert((point->x == 13 || (raster_probe && point->x == 22)) && point->y == 17);
    ++pixel_reads;
    *color = pixel_color;
    if (raster_probe && point->x == 22) memset(color, 0, sizeof(*color));
    if (get_pixel_error) inject_error("injected readback error");
    return obscured_pixel;
}

void xgl_context_new_frame(Xgl_ctx context)
{
    assert(context == b.ctx);
}

void xgl_context_flush(Xgl_ctx context, Xgl_flush_action action)
{
    assert(context == b.ctx && action == XGL_FLUSH_SYNCHRONIZE);
}

void xgl_context_copy_buffer(Xgl_ctx context, Xgl_bounds_i2d *bounds,
                              Xgl_pt_i2d *position, Xgl_ras source)
{
    assert(context == b.ctx && bounds == NULL && source == b.hud);
    assert(position->x == 13 && position->y == 17);
    ++raster_copies;
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
    get_pixel_error = obscured_pixel = fail_attachment = 0;
    depth_queries = width_queries = height_queries = color_queries = 0;
    raster_probe = pixel_reads = raster_copies = 0;
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

static void test_hud_rollback(void)
{
    reset_mock();
    fail_create_at = 1;
    assert(!make_hud());
    assert(b.hud == NULL && b.mask == NULL);
    assert(creations == 1 && destructions == 1 && live_objects == 0);
    reset_mock();
    fail_create_at = 2;
    assert(!make_hud());
    assert(b.hud == NULL && b.mask == NULL);
    assert(creations == 2 && destructions == 2 && live_objects == 0);
    b.errors = 0;
    b.failure = XB_FAILURE_NONE;
    b.error[0] = '\0';
    fail_create_at = 0;
    assert(make_hud());
    assert(b.hud && b.mask && live_objects == 2);
    assert(creations == 4);
    assert(depth_queries >= 3 && width_queries >= 3 && height_queries >= 3);
    assert(color_queries >= 2);
}

static void test_hud_attachment_recovery(void)
{
    Xgl_mem_ras hud, mask;
    reset_mock();
    fail_attachment = 1;
    assert(!make_hud());
    assert(b.hud && b.mask && !b.hud_ready);
    assert(creations == 2 && destructions == 0 && live_objects == 2);
    hud = b.hud;
    mask = b.mask;
    b.errors = 0;
    b.failure = XB_FAILURE_NONE;
    b.error[0] = '\0';
    fail_attachment = 0;
    assert(make_hud());
    assert(b.hud_ready && b.hud == hud && b.mask == mask);
    assert(creations == 2 && destructions == 0 && live_objects == 2);
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

static void test_raster_validation_cache(void)
{
    int masked, fault, reads;
    for (masked = 0; masked < 2; ++masked) {
        for (fault = 0; fault < 3; ++fault) {
            reset_mock();
            b.width = XB_RASTER_W + 26;
            b.height = XB_RASTER_H + 17;
            b.current_z = 1;
            raster_probe = 1;
            pixel_color.rgb.r = fault == 2 ? 0 : 220.0f / 255;
            pixel_color.rgb.g = 32.0f / 255;
            pixel_color.rgb.b = 64.0f / 255;
            get_pixel_error = fault == 0;
            obscured_pixel = fault == 1;
            assert(!validate_raster(masked));
            assert(!b.raster_checked && !b.mask_checked);
            assert(pixel_reads == 1 && raster_copies == 1);
            b.errors = 0;
            b.failure = XB_FAILURE_NONE;
            b.error[0] = '\0';
            get_pixel_error = obscured_pixel = 0;
            pixel_color.rgb.r = 220.0f / 255;
            assert(validate_raster(masked));
            assert(masked ? b.mask_checked : b.raster_checked);
            assert(pixel_reads == (masked ? 3 : 2) && raster_copies == 2);
            reads = pixel_reads;
            assert(validate_raster(masked));
            assert(pixel_reads == reads && raster_copies == 2);
        }
    }
}

int main(void)
{
    test_texture_creation_recovery();
    test_hud_rollback();
    test_hud_attachment_recovery();
    test_raster_inquiries();
    test_readback_error_preservation();
    test_raster_validation_cache();
    reset_mock();
    puts("native raster control-flow tests passed (no graphics/SDK validation)");
    return 0;
}
