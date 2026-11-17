/* Native probe control-flow tests, GPL-2.0-or-later.
 * A single-pixel model checks the submitted state and probe expectations.
 * It is not an XGL implementation, rasterizer, ABI test, or hardware result.
 */
#include <assert.h>
#include <stdarg.h>
#include "../xglbench_xgl.c"

enum mock_fault {
    MOCK_CORRECT, MOCK_NO_COLOR_CLEAR, MOCK_NO_DEPTH_CLEAR,
    MOCK_OPACITY_REVERSED, MOCK_BAD_BLEND, MOCK_IGNORE_FREEZE,
    MOCK_ALWAYS_FREEZE, MOCK_NO_DEPTH_TEST, MOCK_READBACK_ERROR
};

static int system_token, context_token, model_token, view_token;
static enum mock_fault fault;
static int actions, depth_mode, blend_method, blend_equation, freeze_z;
static int draw_mode, color_selector, illumination, volatile_geometry;
static Xgl_usgn32 zwrite_mask;
static double transparency, back_transparency;
static Xgl_color framebuffer, context_color;
static float depth_value;
static unsigned int draws, clears, readbacks;

int xb_cancel_requested(void) { return 0; }

void xgl_object_set(Xgl_object object, ...)
{
    va_list args;
    int attribute;
    assert(object == b.ctx);
    va_start(args, object);
    while ((attribute = va_arg(args, int)) != 0) {
        switch (attribute) {
        case XGL_CTX_NEW_FRAME_ACTION: actions = va_arg(args, int); break;
        case XGL_3D_CTX_HLHSR_MODE: depth_mode = va_arg(args, int); break;
        case XGL_3D_CTX_SURF_TRANSP_METHOD: blend_method = va_arg(args, int); break;
        case XGL_3D_CTX_SURF_TRANSP_BLEND_EQ: blend_equation = va_arg(args, int); break;
        case XGL_3D_CTX_BLEND_FREEZE_Z_BUFFER: freeze_z = va_arg(args, int); break;
        case XGL_3D_CTX_BLEND_DRAW_MODE: draw_mode = va_arg(args, int); break;
        case XGL_3D_CTX_Z_BUFFER_WRITE_MASK: zwrite_mask = va_arg(args, Xgl_usgn32); break;
        case XGL_3D_CTX_SURF_FRONT_TRANSP: transparency = va_arg(args, double); break;
        case XGL_3D_CTX_SURF_BACK_TRANSP: back_transparency = va_arg(args, double); break;
        case XGL_CTX_GEOM_DATA_IS_VOLATILE: volatile_geometry = va_arg(args, int); break;
        case XGL_CTX_SURF_FRONT_COLOR_SELECTOR: color_selector = va_arg(args, int); break;
        case XGL_CTX_SURF_FRONT_COLOR: context_color = *va_arg(args, Xgl_color *); break;
        case XGL_3D_CTX_SURF_FRONT_ILLUMINATION: illumination = va_arg(args, int); break;
        case XGL_3D_CTX_SURF_FACE_CULL:
        case XGL_3D_CTX_Z_BUFFER_COMP_METHOD:
            (void)va_arg(args, int);
            break;
        default: assert(!"unmodeled probe attribute");
        }
    }
    va_end(args);
}

void xgl_object_get(Xgl_object object, int attribute, void *value)
{
    Xgl_error_info *info;
    if (object == b.system) {
        assert(attribute == XGL_SYS_ST_ERROR_INFO);
        info = (Xgl_error_info *)value;
        memset(info, 0, sizeof(*info));
        info->id = "test-readback";
        info->msg = "injected native readback error";
        info->cur_op = "xgl_context_get_pixel";
        return;
    }
    assert(object == b.ctx);
    switch (attribute) {
    case XGL_CTX_GEOM_DATA_IS_VOLATILE: *(Xgl_boolean *)value = volatile_geometry; break;
    default: assert(!"unmodeled probe inquiry");
    }
}

void xgl_transform_write(Xgl_trans transform, void *matrix)
{
    Xgl_matrix_f3d *values;
    int i, j;
    assert(transform == b.model || transform == b.view);
    values = (Xgl_matrix_f3d *)matrix;
    for (i = 0; i < 4; ++i)
        for (j = 0; j < 4; ++j)
            assert((*values)[i][j] == (i == j ? 1.0f : 0.0f));
}

void xgl_context_new_frame(Xgl_ctx context)
{
    assert(context == b.ctx);
    ++clears;
    if ((actions & XGL_CTX_NEW_FRAME_CLEAR) && fault != MOCK_NO_COLOR_CLEAR)
        memset(&framebuffer, 0, sizeof(framebuffer));
    if ((actions & XGL_CTX_NEW_FRAME_HLHSR_ACTION) && fault != MOCK_NO_DEPTH_CLEAR)
        depth_value = 1.0f;
}

void xgl_context_flush(Xgl_ctx context, Xgl_flush_action action)
{
    assert(context == b.ctx && action == XGL_FLUSH_SYNCHRONIZE);
}

void xgl_multi_simple_polygon(Xgl_ctx context, Xgl_facet_flags flags,
                               Xgl_facet_list *facets, Xgl_bbox *bounds,
                               Xgl_usgn32 count, Xgl_pt_list *lists)
{
    const Xgl_pt_f3d *points;
    Xgl_color source;
    float z;
    double opacity;
    int blended, writes_depth;
    (void)flags;
    assert(context == b.ctx && bounds == NULL && count == 1);
    assert(lists->num_pts == 3 && lists->pt_type == XGL_PT_F3D);
    assert(volatile_geometry && illumination == XGL_ILLUM_NONE);
    points = lists->pts.f3d;
    /* The probes use one flat triangle containing the sample at the center. */
    assert(points[0].x < 0 && points[0].y < 0);
    assert(points[1].x > 0 && points[1].y < 0);
    assert(points[2].x == 0 && points[2].y > 0);
    assert(points[0].z == points[1].z && points[1].z == points[2].z);
    z = points[0].z;
    assert(z >= 0 && z <= 1);
    if (color_selector == XGL_SURF_COLOR_CONTEXT) source = context_color;
    else {
        assert(color_selector == XGL_SURF_COLOR_FACET);
        assert(facets && facets->facet_type == XGL_FACET_COLOR && facets->num_facets == 1);
        source = facets->facets.color_facets[0].color;
    }
    ++draws;
    if (depth_mode == XGL_HLHSR_Z_BUFFER && fault != MOCK_NO_DEPTH_TEST && z > depth_value)
        return;
    blended = blend_method == XGL_TRANSP_BLENDED;
    writes_depth = !blended || !freeze_z;
    if (blended) {
        assert(blend_equation == XGL_BLEND_ARBITRARY_BG);
        assert(draw_mode == XGL_BLEND_DRAW_ALL);
        assert(transparency >= 0 && transparency <= 1 && transparency == back_transparency);
        opacity = fault == MOCK_OPACITY_REVERSED ? transparency : 1.0 - transparency;
        if (fault == MOCK_BAD_BLEND) opacity *= .5;
        framebuffer.rgb.r = (float)(source.rgb.r * opacity + framebuffer.rgb.r * (1.0 - opacity));
        framebuffer.rgb.g = (float)(source.rgb.g * opacity + framebuffer.rgb.g * (1.0 - opacity));
        framebuffer.rgb.b = (float)(source.rgb.b * opacity + framebuffer.rgb.b * (1.0 - opacity));
        if (fault == MOCK_IGNORE_FREEZE) writes_depth = 1;
        if (fault == MOCK_ALWAYS_FREEZE) writes_depth = 0;
    } else {
        assert(blend_method == XGL_TRANSP_NONE);
        framebuffer = source;
    }
    if (depth_mode == XGL_HLHSR_Z_BUFFER && zwrite_mask && writes_depth)
        depth_value = z;
}

Xgl_boolean xgl_context_get_pixel(Xgl_ctx context, Xgl_pt_i2d *point, Xgl_color *color)
{
    assert(context == b.ctx && point->x == b.width / 2 && point->y == b.height / 2);
    ++readbacks;
    *color = framebuffer;
    if (fault == MOCK_READBACK_ERROR) (void)xgl_error(b.system);
    return FALSE;
}

static void reset_mock(enum mock_fault injected)
{
    memset(&b, 0, sizeof(b));
    b.system = &system_token;
    b.ctx = &context_token;
    b.model = &model_token;
    b.view = &view_token;
    b.width = b.height = 128;
    b.current_z = 1;
    b.inquiry.transparency = XGL_INQ_HARDWARE;
    fault = injected;
    actions = 0;
    depth_mode = XGL_HLHSR_Z_BUFFER;
    blend_method = XGL_TRANSP_NONE;
    blend_equation = XGL_BLEND_NONE;
    freeze_z = TRUE;
    draw_mode = XGL_BLEND_DRAW_ALL;
    color_selector = XGL_SURF_COLOR_FACET;
    illumination = XGL_ILLUM_NONE;
    volatile_geometry = TRUE;
    zwrite_mask = (Xgl_usgn32)-1;
    transparency = back_transparency = 0;
    memset(&framebuffer, 0, sizeof(framebuffer));
    memset(&context_color, 0, sizeof(context_color));
    depth_value = 1.0f;
    draws = clears = readbacks = 0;
}

static void test_combined_clear_gate(void)
{
    enum mock_fault failures[3];
    int i;
    reset_mock(MOCK_CORRECT);
    volatile_geometry = FALSE;
    b.current_z = 0;
    depth_mode = XGL_HLHSR_NONE;
    assert(validate_combined_clear());
    assert(draws >= 2 && clears >= 2 && readbacks >= 3);
    assert(!volatile_geometry && !b.current_z && depth_mode == XGL_HLHSR_NONE);
    assert(blend_method == XGL_TRANSP_NONE && transparency == 0);
    failures[0] = MOCK_NO_COLOR_CLEAR;
    failures[1] = MOCK_NO_DEPTH_CLEAR;
    failures[2] = MOCK_READBACK_ERROR;
    for (i = 0; i < 3; ++i) {
        reset_mock(failures[i]);
        assert(!validate_combined_clear());
        assert(b.error[0] != '\0');
        assert(xb_backend_failure() == (failures[i] == MOCK_READBACK_ERROR ?
                                       XB_FAILURE_API : XB_FAILURE_VALIDATION));
    }
}

static void test_transparency_gate(void)
{
    enum mock_fault failures[6];
    int i;
    reset_mock(MOCK_CORRECT);
    volatile_geometry = FALSE;
    b.current_z = 0;
    depth_mode = XGL_HLHSR_NONE;
    assert(validate_transparency());
    assert(draws >= 12 && readbacks >= 8);
    assert(!volatile_geometry && !b.current_z && depth_mode == XGL_HLHSR_NONE);
    assert(blend_method == XGL_TRANSP_NONE && transparency == 0 && !freeze_z);
    failures[0] = MOCK_OPACITY_REVERSED;
    failures[1] = MOCK_BAD_BLEND;
    failures[2] = MOCK_IGNORE_FREEZE;
    failures[3] = MOCK_ALWAYS_FREEZE;
    failures[4] = MOCK_NO_DEPTH_TEST;
    failures[5] = MOCK_READBACK_ERROR;
    for (i = 0; i < 6; ++i) {
        reset_mock(failures[i]);
        assert(!validate_transparency());
        assert(b.error[0] != '\0');
        assert(xb_backend_failure() == (failures[i] == MOCK_READBACK_ERROR ?
                                       XB_FAILURE_API : XB_FAILURE_VALIDATION));
    }
}

int main(void)
{
    test_combined_clear_gate();
    test_transparency_gate();
    puts("native clear/transparency probe tests passed (no graphics/SDK validation)");
    return 0;
}
