/* Native UI submission tests, GPL-2.0-or-later.
 * These stubs inspect API calls and ownership, not font rendering or XGL ABI.
 */
#include <assert.h>
#include <stdarg.h>
#include "../xglbench_xgl.c"

typedef struct {
    int live, draw_buffer, new_frame_action, volatile_geometry, color_selector;
    Xgl_object device;
    double text_height;
    Xgl_pt_f2d text_up;
    Xgl_color text_color;
} mock_ui_context;

static mock_ui_context contexts[8];
static int system_token, world_token, raster_token, font_token;
static int creations, destructions, main_flushes, ui_flushes;
static int quad_calls, quad_count, text_calls, extent_calls, main_state_changes;
static int missing_font, create_error, text_error, empty_extent, wide_font, completion_error;
static int main_complete, ui_pending;
static const char *error_message;
typedef struct {
    float xmin, xmax, ymin, ymax;
    Xgl_color color;
} mock_paint;
static mock_paint paints[2][128];
static int paint_counts[2], validation_mode, blank_text, omit_geometry, pinned_buffer;
static int readback_error, pixel_reads, depth_zero_clears, main_actions, new_frames;
static Xgl_usgn32 draw_buffer;
static Xgl_color main_background, buffer_background[2];
static Xgl_hlhsr_data main_depth;

int xb_cancel_requested(void) { return 0; }

static void inject_ui_error(const char *message)
{
    error_message = message;
    (void)xgl_error(b.system);
}

static void set_attributes(mock_ui_context *context, va_list args)
{
    int attribute;
    while ((attribute = va_arg(args, int)) != 0) {
        switch (attribute) {
        case XGL_CTX_DEVICE: context->device = va_arg(args, Xgl_object); break;
        case XGL_CTX_RENDER_BUFFER: context->draw_buffer = va_arg(args, int); break;
        case XGL_CTX_NEW_FRAME_ACTION: context->new_frame_action = va_arg(args, int); break;
        case XGL_CTX_GEOM_DATA_IS_VOLATILE: context->volatile_geometry = va_arg(args, int); break;
        case XGL_CTX_SURF_FRONT_COLOR_SELECTOR: context->color_selector = va_arg(args, int); break;
        case XGL_CTX_STEXT_CHAR_HEIGHT: context->text_height = va_arg(args, double); break;
        case XGL_CTX_STEXT_CHAR_UP_VECTOR: context->text_up = *va_arg(args, Xgl_pt_f2d *); break;
        case XGL_CTX_STEXT_COLOR: context->text_color = *va_arg(args, Xgl_color *); break;
        case XGL_CTX_SFONT_0: assert(va_arg(args, Xgl_sfont) == &font_token); break;
        case XGL_CTX_STEXT_CHAR_SPACING:
        case XGL_CTX_STEXT_CHAR_EXPANSION_FACTOR:
            (void)va_arg(args, double);
            break;
        case XGL_CTX_DEFERRAL_MODE:
        case XGL_CTX_VDC_MAP:
        case XGL_CTX_VDC_ORIENTATION:
        case XGL_CTX_SURF_FRONT_FILL_STYLE:
        case XGL_CTX_SURF_EDGE_FLAG:
        case XGL_CTX_STEXT_PRECISION:
        case XGL_CTX_STEXT_CHAR_ENCODING:
        case XGL_CTX_STEXT_PATH:
        case XGL_CTX_STEXT_ALIGN_HORIZ:
        case XGL_CTX_STEXT_ALIGN_VERT:
            (void)va_arg(args, int);
            break;
        default: assert(!"unexpected UI context attribute");
        }
    }
}

Xgl_object xgl_object_create(Xgl_sys_state system, int type,
                             Xgl_obj_desc *descriptor, ...)
{
    mock_ui_context *context;
    va_list args;
    assert(system == b.system && type == XGL_2D_CTX && descriptor == NULL);
    /* Any attempt to allocate a UI memory raster fails this type assertion. */
    assert(creations < 8);
    context = &contexts[creations++];
    memset(context, 0, sizeof(*context));
    context->new_frame_action = -1;
    context->live = 1;
    va_start(args, descriptor);
    set_attributes(context, args);
    va_end(args);
    if (create_error) inject_ui_error("injected UI context creation error");
    return context;
}

void xgl_object_destroy(Xgl_object object)
{
    mock_ui_context *context;
    context = (mock_ui_context *)object;
    assert(context >= contexts && context < contexts + creations && context->live);
    context->live = 0;
    ++destructions;
}

void xgl_object_set(Xgl_object object, ...)
{
    va_list args;
    mock_ui_context *context;
    int attribute;
    if (object == b.ctx || object == b.raster) {
        assert(validation_mode);
        va_start(args, object);
        while ((attribute = va_arg(args, int)) != 0) {
            if (object == b.raster) {
                assert(attribute == XGL_WIN_RAS_BUF_DRAW);
                draw_buffer = va_arg(args, Xgl_usgn32);
                assert(draw_buffer < 2);
            } else {
                ++main_state_changes;
                switch (attribute) {
                case XGL_CTX_BACKGROUND_COLOR: main_background = *va_arg(args, Xgl_color *); break;
                case XGL_3D_CTX_HLHSR_DATA: main_depth = *va_arg(args, Xgl_hlhsr_data *); break;
                case XGL_CTX_NEW_FRAME_ACTION: main_actions = va_arg(args, int); break;
                case XGL_3D_CTX_HLHSR_MODE: (void)va_arg(args, int); break;
                default: assert(!"unexpected validation state change");
                }
            }
        }
        va_end(args);
        return;
    }
    context = (mock_ui_context *)object;
    assert(context >= contexts && context < contexts + creations && context->live);
    va_start(args, object);
    set_attributes(context, args);
    va_end(args);
}

void xgl_object_get(Xgl_object object, int attribute, void *value)
{
    Xgl_error_info *info;
    if (object == b.system) {
        if (attribute == XGL_SYS_ST_ERROR_INFO) {
            info = (Xgl_error_info *)value;
            memset(info, 0, sizeof(*info));
            info->id = "test-ui";
            info->msg = (char *)error_message;
            info->cur_op = "mock UI operation";
        } else {
            assert(attribute == XGL_SYS_ST_SFONT_DIRECTORY);
            *(char **)value = "/mock/stroke-fonts";
        }
    } else if (object == b.ctx) {
        assert(validation_mode);
        if (attribute == XGL_CTX_BACKGROUND_COLOR) *(Xgl_color *)value = main_background;
        else if (attribute == XGL_CTX_NEW_FRAME_ACTION) *(Xgl_usgn32 *)value = (Xgl_usgn32)main_actions;
        else {
            assert(attribute == XGL_3D_CTX_HLHSR_DATA);
            *(Xgl_hlhsr_data *)value = main_depth;
        }
    } else if (object == b.raster) {
        assert(validation_mode && attribute == XGL_WIN_RAS_BUF_DRAW);
        *(Xgl_usgn32 *)value = draw_buffer;
    } else if (object == &font_token) {
        assert(attribute == XGL_SFONT_NAME);
        *(char **)value = "Roman_M";
    } else {
        assert(attribute == XGL_CTX_SFONT_0);
        *(Xgl_sfont *)value = missing_font ? NULL : &font_token;
    }
}

void xgl_context_new_frame(Xgl_ctx context)
{
    assert(validation_mode && context == b.ctx);
    assert(!ui_pending);
    ++new_frames;
    main_complete = 0;
    if (main_actions & XGL_CTX_NEW_FRAME_CLEAR) {
        buffer_background[draw_buffer] = main_background;
        paint_counts[draw_buffer] = 0;
    }
    if ((main_actions & XGL_CTX_NEW_FRAME_HLHSR_ACTION) && main_depth.z_buffer.z_value == 0)
        ++depth_zero_clears;
}

static void paint_region(float xmin, float xmax, float ymin, float ymax, Xgl_color color)
{
    int buffer, index;
    if (!validation_mode) return;
    buffer = pinned_buffer ? 0 : (int)draw_buffer;
    index = paint_counts[buffer]++;
    assert(index < 128);
    paints[buffer][index].xmin = xmin;
    paints[buffer][index].xmax = xmax;
    paints[buffer][index].ymin = ymin;
    paints[buffer][index].ymax = ymax;
    paints[buffer][index].color = color;
}

void xgl_context_flush(Xgl_ctx context, Xgl_flush_action action)
{
    assert(action == XGL_FLUSH_SYNCHRONIZE);
    if (context == b.ctx) {
        assert(!ui_pending);
        ++main_flushes;
        main_complete = 1;
    } else {
        assert(context == b.ui_ctx);
        ++ui_flushes;
        if (completion_error) {
            inject_ui_error("injected UI completion error");
            return;
        }
        ui_pending = 0;
    }
}

static mock_ui_context *drawing_context(Xgl_ctx context)
{
    mock_ui_context *ui;
    assert(context == b.ui_ctx && context != b.ctx && main_complete);
    ui = (mock_ui_context *)context;
    assert(ui->live && ui->device == b.raster);
    assert(ui->draw_buffer == XGL_RENDER_DRAW_BUFFER);
    assert(ui->new_frame_action == XGL_CTX_NEW_FRAME_NONE);
    assert(ui->volatile_geometry);
    ui_pending = 1;
    return ui;
}

void xgl_multi_simple_polygon(Xgl_ctx context, Xgl_facet_flags flags,
                               Xgl_facet_list *facets, Xgl_bbox *bounds,
                               Xgl_usgn32 count, Xgl_pt_list *lists)
{
    mock_ui_context *ui;
    Xgl_usgn32 i, j;
    assert(flags == (XGL_FACET_FLAG_SIDES_ARE_4 | XGL_FACET_FLAG_SHAPE_CONVEX));
    ui = drawing_context(context);
    assert(ui->color_selector == XGL_SURF_COLOR_FACET);
    assert(bounds == NULL && count > 0);
    assert(facets && facets->facet_type == XGL_FACET_COLOR && facets->num_facets == count);
    for (i = 0; i < count; ++i) {
        assert(lists[i].pt_type == XGL_PT_F2D && lists[i].num_pts == 4);
        assert(lists[i].pts.f2d[2].x > lists[i].pts.f2d[0].x);
        assert(lists[i].pts.f2d[2].y > lists[i].pts.f2d[0].y);
        for (j = 0; j < 4; ++j) {
            assert(lists[i].pts.f2d[j].x >= 0 && lists[i].pts.f2d[j].x <= b.width);
            assert(lists[i].pts.f2d[j].y >= 0 && lists[i].pts.f2d[j].y <= b.height);
        }
        if (!omit_geometry)
            paint_region(lists[i].pts.f2d[0].x, lists[i].pts.f2d[2].x,
                         lists[i].pts.f2d[0].y, lists[i].pts.f2d[2].y,
                         facets->facets.color_facets[i].color);
    }
    ++quad_calls;
    quad_count += (int)count;
}

static void mock_text_bounds(mock_ui_context *ui, const char *string,
                              Xgl_bounds_f2d *bounds)
{
    double width;
    const char *p;
    width = 0;
    for (p = string; *p; ++p)
        width += ui->text_height * (*p == 'W' ? .8 : (*p == 'i' ? .22 : .55));
    if (wide_font) width *= 10;
    bounds->xmin = (float)(-.1 * ui->text_height);
    bounds->xmax = empty_extent ? bounds->xmin : (float)(width + bounds->xmin);
    bounds->ymin = (float)(-.2 * ui->text_height);
    bounds->ymax = (float)(.8 * ui->text_height);
}

void xgl_stroke_text_extent(Xgl_ctx context, void *string,
                            Xgl_bounds_f2d *bounds, Xgl_pt_f2d *concat)
{
    mock_ui_context *ui;
    ui = (mock_ui_context *)context;
    assert(ui->live && string && ((char *)string)[0]);
    ++extent_calls;
    mock_text_bounds(ui, (char *)string, bounds);
    concat->x = bounds->xmax - bounds->xmin;
    concat->y = 0;
}

void xgl_stroke_text(Xgl_ctx context, void *string, void *origin, void *direction)
{
    mock_ui_context *ui;
    Xgl_pt_f2d *point;
    Xgl_bounds_f2d bounds;
    ui = drawing_context(context);
    point = (Xgl_pt_f2d *)origin;
    assert(string && ((char *)string)[0] && direction == NULL);
    assert(point->x >= 0 && point->x < b.width && point->y >= 0 && point->y <= b.height);
    assert(ui->text_height > 0 && ui->text_height < b.height);
    assert(ui->text_up.x == 0 && ui->text_up.y == 1);
    mock_text_bounds(ui, (char *)string, &bounds);
    assert(point->x + bounds.xmin >= 0 && point->x + bounds.xmax <= b.width);
    assert(point->y - bounds.ymax >= 0 && point->y - bounds.ymin <= b.height);
    if (!blank_text)
        paint_region(point->x + bounds.xmin, point->x + bounds.xmax,
                     point->y - bounds.ymax, point->y - bounds.ymin, ui->text_color);
    ++text_calls;
    if (text_error) inject_ui_error("injected stroke-text error");
}

Xgl_boolean xgl_context_get_pixel(Xgl_ctx context, Xgl_pt_i2d *point, Xgl_color *color)
{
    int i;
    mock_paint *paint;
    assert(validation_mode && context == b.ctx && !ui_pending);
    ++pixel_reads;
    *color = buffer_background[draw_buffer];
    for (i = 0; i < paint_counts[draw_buffer]; ++i) {
        paint = &paints[draw_buffer][i];
        if (point->x >= paint->xmin && point->x <= paint->xmax &&
            point->y >= paint->ymin && point->y <= paint->ymax) *color = paint->color;
    }
    if (readback_error) inject_ui_error("injected UI readback error");
    return FALSE;
}

void xgl_context_copy_buffer(Xgl_ctx context, Xgl_bounds_i2d *bounds,
                              Xgl_pt_i2d *position, Xgl_ras source)
{
    (void)context; (void)bounds; (void)position; (void)source;
    assert(!"native UI must not copy raster buffers");
}

static void reset_mock(void)
{
    int i;
    for (i = 0; i < creations; ++i)
        if (contexts[i].live) xgl_object_destroy(&contexts[i]);
    memset(&b, 0, sizeof(b));
    memset(contexts, 0, sizeof(contexts));
    b.system = &system_token;
    b.ctx = &world_token;
    b.raster = &raster_token;
    b.width = 320;
    b.height = 240;
    b.buffers = 2;
    b.current_z = 1;
    b.hardware_lighting = 1;
    b.texture_switch = TRUE;
    creations = destructions = main_flushes = ui_flushes = 0;
    quad_calls = quad_count = text_calls = extent_calls = main_state_changes = 0;
    missing_font = create_error = text_error = empty_extent = wide_font = 0;
    completion_error = 0;
    main_complete = ui_pending = 0;
    error_message = NULL;
    memset(paints, 0, sizeof(paints));
    memset(paint_counts, 0, sizeof(paint_counts));
    memset(&main_background, 0, sizeof(main_background));
    memset(buffer_background, 0, sizeof(buffer_background));
    main_background.rgb.r = .12f;
    main_background.rgb.g = .23f;
    main_background.rgb.b = .34f;
    main_depth.z_buffer.z_value = .75f;
    draw_buffer = 1;
    validation_mode = blank_text = omit_geometry = pinned_buffer = 0;
    readback_error = pixel_reads = depth_zero_clears = main_actions = 0;
    new_frames = 0;
}

static void test_ui_submission(void)
{
    static const int dimensions[3][2] = {{320,240}, {640,480}, {320,200}};
    int mode, size, previous_text, previous_quads, cached_extents;
    for (size = 0; size < 3; ++size) {
      for (mode = XB_UI_HUD; mode <= XB_UI_CONSOLE; ++mode) {
        reset_mock();
        b.width = dimensions[size][0];
        b.height = dimensions[size][1];
        assert(ensure_ui());
        assert(extent_calls > 0);
        cached_extents = extent_calls;
        assert(xb_backend_ui((enum xb_ui)mode, 0));
        assert(quad_calls > 0 && quad_count > 0 && text_calls > 0);
        assert(main_flushes > 0 && ui_flushes > 0 && !ui_pending);
        assert(main_state_changes == 0 && b.current_z == 1 && b.hardware_lighting == 1 && b.texture_switch);
        previous_text = text_calls;
        previous_quads = quad_calls;
        main_complete = 0;
        assert(xb_backend_ui((enum xb_ui)mode, 37));
        assert(creations == 1 && text_calls > previous_text && quad_calls > previous_quads);
        assert(extent_calls == cached_extents);
        assert(main_flushes >= 2 && ui_flushes >= 2 && !ui_pending);
        assert(xb_backend_ui((enum xb_ui)mode, 100));
      }
    }
}

static void test_ui_failures(void)
{
    reset_mock();
    create_error = 1;
    assert(!ensure_ui());
    assert(xb_backend_failure() == XB_FAILURE_API && quad_calls == 0 && text_calls == 0);
    assert(b.ui_ctx == NULL && destructions == 1);
    b.errors = 0;
    b.failure = XB_FAILURE_NONE;
    b.error[0] = '\0';
    create_error = 0;
    assert(ensure_ui() && creations == 2);
    reset_mock();
    missing_font = 1;
    assert(!ensure_ui());
    assert(b.error[0] && quad_calls == 0 && text_calls == 0);
    assert(b.ui_ctx == NULL && destructions == 1);
    reset_mock();
    empty_extent = 1;
    assert(!ensure_ui());
    assert(xb_backend_failure() == XB_FAILURE_VALIDATION && !b.ui_ready);
    assert(quad_calls == 0 && text_calls == 0);
    reset_mock();
    wide_font = 1;
    assert(ensure_ui());
    assert(!xb_backend_ui(XB_UI_HUD, 0));
    assert(xb_backend_failure() == XB_FAILURE_VALIDATION && quad_calls == 0 && text_calls == 0);
    reset_mock();
    b.width = 128;
    b.height = 64;
    assert(!ensure_ui());
    assert(xb_backend_failure() == XB_FAILURE_UNSUPPORTED && creations == 0);
    reset_mock();
    assert(ensure_ui());
    text_error = 1;
    assert(!xb_backend_ui(XB_UI_HUD, 0));
    assert(xb_backend_failure() == XB_FAILURE_API);
    assert(strstr(b.error, "injected stroke-text error") != NULL);
    assert(!ui_pending && ui_flushes > 0);
    reset_mock();
    assert(ensure_ui());
    assert(!xb_backend_ui((enum xb_ui)-1, 0));
    assert(quad_calls == 0 && text_calls == 0);
}

static void assert_validation_restored(void)
{
    assert(draw_buffer == 1 && !ui_pending);
    assert(fabs(main_background.rgb.r - .12f) < .00001);
    assert(fabs(main_background.rgb.g - .23f) < .00001);
    assert(fabs(main_background.rgb.b - .34f) < .00001);
    assert(main_depth.z_buffer.z_value == .75f && main_actions == 0);
    assert(((mock_ui_context *)b.ui_ctx)->text_height == b.ui_char_height);
}

static void test_ui_completion_retry(void)
{
    int presentation, previous_main_flushes, previous_ui_flushes;
    for (presentation = 0; presentation < 2; ++presentation) {
        reset_mock();
        validation_mode = 1;
        assert(ensure_ui());
        completion_error = 1;
        assert(!xb_backend_ui(XB_UI_HUD, 37));
        assert(xb_backend_failure() == XB_FAILURE_API);
        assert(strstr(b.error, "injected UI completion error") != NULL);
        assert(b.ui_pending && ui_pending && new_frames == 0);
        previous_main_flushes = main_flushes;
        previous_ui_flushes = ui_flushes;
        b.errors = 0;
        b.failure = XB_FAILURE_NONE;
        b.error[0] = '\0';
        /* Another failed completion cannot permit main work or a swap. */
        if (presentation) xb_backend_swap();
        else xb_backend_sync();
        assert(b.ui_pending && ui_pending && b.errors);
        assert(main_flushes == previous_main_flushes && new_frames == 0);
        assert(ui_flushes == previous_ui_flushes + 1);
        b.errors = 0;
        b.failure = XB_FAILURE_NONE;
        b.error[0] = '\0';
        completion_error = 0;
        if (presentation) xb_backend_swap();
        else {
            xb_backend_sync();
            xb_backend_clear(1, 1);
        }
        assert(!b.ui_pending && !ui_pending && !b.errors);
        assert(main_flushes > previous_main_flushes && new_frames == 1);
        assert(ui_flushes == previous_ui_flushes + 2);
    }
}

static void test_ui_readback_gate(void)
{
    int mode, fault, previous_reads;
    for (mode = XB_UI_HUD; mode <= XB_UI_CONSOLE; ++mode) {
        reset_mock();
        validation_mode = 1;
        assert(validate_ui((enum xb_ui)mode));
        assert(b.ui_checked[mode] && pixel_reads > 0 && depth_zero_clears >= 2);
        assert_validation_restored();
        previous_reads = pixel_reads;
        assert(validate_ui((enum xb_ui)mode));
        assert(pixel_reads == previous_reads);
    }
    for (fault = 0; fault < 4; ++fault) {
        reset_mock();
        validation_mode = 1;
        blank_text = fault == 0;
        omit_geometry = fault == 1;
        pinned_buffer = fault == 2;
        readback_error = fault == 3;
        assert(!validate_ui(XB_UI_HUD));
        assert(!b.ui_checked[XB_UI_HUD]);
        assert(xb_backend_failure() == (readback_error ? XB_FAILURE_API : XB_FAILURE_VALIDATION));
        assert_validation_restored();
        b.errors = 0;
        b.failure = XB_FAILURE_NONE;
        b.error[0] = '\0';
        blank_text = omit_geometry = pinned_buffer = readback_error = 0;
        assert(validate_ui(XB_UI_HUD));
        assert(b.ui_checked[XB_UI_HUD]);
        assert_validation_restored();
    }
}

int main(void)
{
    test_ui_submission();
    test_ui_failures();
    test_ui_completion_retry();
    test_ui_readback_gate();
    reset_mock();
    puts("native text/geometry UI submission tests passed (no graphics/SDK validation)");
    return 0;
}
