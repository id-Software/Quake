/* Native API state/geometry contracts, not a device renderer or ABI test.
 * Compile against the actual Sun SDK headers; only exercised calls are mocked. */
#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../xgl_backend.c"

viddef_t vid;
refdef_t r_refdef;
vec3_t r_origin, vpn, vright, vup;
client_state_t cl;
dlight_t cl_dlights[MAX_DLIGHTS];
cvar_t r_dynamic = {"r_dynamic", "1", false, false, 0, NULL};
byte xgl_palette[768], gammatable[256];

static int handles[12];
static Xgl_matrix_f3d model_matrix, view_matrix;
static Xgl_bounds_d3d current_window, current_clip;
static Xgl_usgn32 write_mask;
static Xgl_boolean switches[6];
static int actions, orientation, mapping, clip_planes, hlhsr, illumination;
static int clear_calls, swap_calls, polygon_calls, text_calls, extent_calls, flush_calls;
static int transparency, freeze_z;
static float current_ambient, text_height;
static Xgl_color text_color, polygon_color;
static Xgl_pt_color_normal_f3d submitted_vertex;
static Xgl_pt_d3d light_position[6];
static Xgl_pt_f2d recorded_position[4096];
static Xgl_pt_f2d *queued_position[4096];
static Xgl_pt_f2d queued_copy[4096];
static int queued_count;
static Xgl_error_info injected_error;
static jmp_buf expected_failure;
static int expect_failure;
static char fatal_message[1024];

void Sys_Error(char *format, ...)
{
    va_list args;
    va_start(args, format);
    if (expect_failure) {
        vsprintf(fatal_message, format, args);
        va_end(args);
        longjmp(expected_failure, 1);
    }
    vfprintf(stderr, format, args);
    va_end(args);
    fputc('\n', stderr);
    abort();
}

void xgl_object_set(Xgl_object object, ...)
{
    va_list args;
    int attribute, value, light;
    Xgl_color *color;
    va_start(args, object);
    while ((attribute = va_arg(args, int)) != 0) {
        switch (attribute) {
        case XGL_CTX_VDC_WINDOW: current_window = *va_arg(args, Xgl_bounds_d3d *); break;
        case XGL_CTX_VIEW_CLIP_BOUNDS: current_clip = *va_arg(args, Xgl_bounds_d3d *); break;
        case XGL_CTX_DC_VIEWPORT:
            /* Keep device viewport management with the native mapper, as in
             * the hardware-tested benchmark. */
            fprintf(stderr, "Unexpected explicit XGL device viewport\n");
            abort();
            break;
        case XGL_3D_CTX_Z_BUFFER_WRITE_MASK: write_mask = va_arg(args, Xgl_usgn32); break;
        case XGL_3D_CTX_LIGHT_SWITCHES:
            memcpy(switches, va_arg(args, Xgl_boolean *), sizeof(switches)); break;
        case XGL_LIGHT_COLOR:
            color = va_arg(args, Xgl_color *);
            if (object == xb.lights[0]) current_ambient = color->rgb.r;
            break;
        case XGL_CTX_STEXT_COLOR: text_color = *va_arg(args, Xgl_color *); break;
        case XGL_CTX_STEXT_CHAR_HEIGHT: text_height = (float)va_arg(args, double); break;
        case XGL_3D_CTX_SURF_FRONT_TRANSP:
        case XGL_3D_CTX_SURF_BACK_TRANSP:
        case XGL_LIGHT_ATTENUATION_1:
        case XGL_LIGHT_ATTENUATION_2: (void)va_arg(args, double); break;
        case XGL_LIGHT_POSITION:
            for (light = 1; light < 5 && object != xb.lights[light]; ++light) {}
            assert(light < 5);
            light_position[light] = *va_arg(args, Xgl_pt_d3d *);
            break;
        case XGL_CTX_NEW_FRAME_ACTION: actions = va_arg(args, int); break;
        case XGL_CTX_VDC_ORIENTATION: orientation = va_arg(args, int); break;
        case XGL_CTX_VDC_MAP: mapping = va_arg(args, int); break;
        case XGL_CTX_CLIP_PLANES: clip_planes = va_arg(args, int); break;
        case XGL_3D_CTX_HLHSR_MODE: hlhsr = va_arg(args, int); break;
        case XGL_3D_CTX_SURF_FRONT_ILLUMINATION: illumination = va_arg(args, int); break;
        case XGL_3D_CTX_SURF_TRANSP_METHOD: transparency = va_arg(args, int); break;
        case XGL_3D_CTX_BLEND_FREEZE_Z_BUFFER: freeze_z = va_arg(args, int); break;
        case XGL_3D_CTX_SURF_TRANSP_BLEND_EQ:
        case XGL_3D_CTX_BLEND_DRAW_MODE:
        case XGL_CTX_SURF_FRONT_COLOR_SELECTOR:
        case XGL_LIGHT_TYPE: value = va_arg(args, int); (void)value; break;
        default:
            fprintf(stderr, "Uncaptured XGL attribute %d\n", attribute);
            abort();
        }
    }
    va_end(args);
}

void xgl_object_get(Xgl_object object, Xgl_attribute attribute, void *result)
{
    assert(object == xb.system && attribute == XGL_SYS_ST_ERROR_INFO);
    *(Xgl_error_info *)result = injected_error;
}

void xgl_transform_write(Xgl_trans transform, void *matrix)
{
    if (transform == xb.view) memcpy(view_matrix, matrix, sizeof(view_matrix));
    else {
        assert(transform == xb.model);
        memcpy(model_matrix, matrix, sizeof(model_matrix));
    }
}

void xgl_context_flush(Xgl_ctx context, Xgl_flush_action action)
{
    int i;
    assert(action == XGL_FLUSH_SYNCHRONIZE);
    assert(context == xb.scene || context == xb.ui);
    ++flush_calls;
    if (context == xb.ui) {
        for (i = 0; i < queued_count; ++i) {
            assert(queued_position[i]->x == queued_copy[i].x);
            assert(queued_position[i]->y == queued_copy[i].y);
        }
        queued_count = 0;
    }
}

void xgl_context_new_frame(Xgl_ctx context)
{
    assert(context == xb.scene);
    if (actions & XGL_CTX_NEW_FRAME_SWITCH_BUFFER) ++swap_calls;
    if (actions & XGL_CTX_NEW_FRAME_HLHSR_ACTION) {
        assert(write_mask == (Xgl_usgn32)0xffffffffUL);
        assert(transparency == XGL_TRANSP_NONE && !freeze_z);
        ++clear_calls;
    }
}

void xgl_multi_simple_polygon(Xgl_ctx context, Xgl_facet_flags flags,
                              Xgl_facet_list *facets, Xgl_bbox *box,
                              Xgl_usgn32 count, Xgl_pt_list *lists)
{
    (void)flags;
    assert(!box && count > 0);
    assert(context == xb.scene || context == xb.ui);
    ++polygon_calls;
    if (lists[0].pt_type == XGL_PT_COLOR_NORMAL_F3D) {
        assert(lists[0].num_pts == 3 && !facets);
        submitted_vertex = lists[0].pts.color_normal_f3d[0];
    } else {
        assert(facets && facets->facet_type == XGL_FACET_COLOR);
        polygon_color = facets->facets.color_facets[0].color;
    }
}

void xgl_stroke_text_extent(Xgl_ctx context, void *string,
                            Xgl_bounds_f2d *bounds, Xgl_pt_f2d *advance)
{
    int ch;
    assert(context == xb.ui && text_height > 0);
    ch = *(unsigned char *)string;
    assert(ch >= 33 && ch <= 126);
    ++extent_calls;
    bounds->xmin = -.1f * text_height;
    bounds->xmax = .6f * text_height;
    bounds->ymin = 0;
    bounds->ymax = .8f * text_height;
    if (ch == 'g') { bounds->ymin = -.3f * text_height; bounds->ymax = .5f * text_height; }
    if (ch == '|') bounds->xmax = bounds->xmin;
    if (ch == '-') bounds->ymin = bounds->ymax = .3f * text_height;
    if (ch == '.') { bounds->xmax = bounds->xmin; bounds->ymax = bounds->ymin; }
    advance->x = .7f * text_height;
    advance->y = 0;
}

void xgl_stroke_text(Xgl_ctx context, void *string, void *origin, void *direction)
{
    assert(context == xb.ui && !direction && *(char *)string);
    assert(text_calls < 4096 && queued_count < 4096);
    recorded_position[text_calls++] = *(Xgl_pt_f2d *)origin;
    queued_position[queued_count] = (Xgl_pt_f2d *)origin;
    queued_copy[queued_count++] = *(Xgl_pt_f2d *)origin;
}

void xgl_window_raster_resize(Xgl_win_ras raster) { assert(raster == xb.raster); }

static void reset_backend(void)
{
    int i;
    memset(&xb, 0, sizeof(xb));
    memset(&vid, 0, sizeof(vid));
    memset(&r_refdef, 0, sizeof(r_refdef));
    memset(&cl, 0, sizeof(cl));
    memset(cl_dlights, 0, sizeof(cl_dlights));
    memset(r_origin, 0, sizeof(r_origin));
    memset(vpn, 0, sizeof(vpn));
    memset(vright, 0, sizeof(vright));
    memset(vup, 0, sizeof(vup));
    memset(&injected_error, 0, sizeof(injected_error));
    xb.system = &handles[0]; xb.raster = &handles[1];
    xb.scene = &handles[2]; xb.ui = &handles[3];
    xb.model = &handles[4]; xb.view = &handles[5];
    for (i = 0; i < 6; ++i) xb.lights[i] = &handles[6 + i];
    xb.width = vid.width = 640; xb.height = vid.height = 480;
    xb.maximum_z = 16777215;
    xb.frame = 1; xb.current_text_height = -1;
    xb.applied_lit = xb.applied_zwrite = -1; xb.applied_opacity = -1;
    xgl_ambient.value = 1; xgl_maxlights.value = 4; r_dynamic.value = 1;
    for (i = 0; i < 95; ++i) xb.glyphs[i][0] = (char)(i + 32);
    for (i = 0; i < 256; ++i) gammatable[i] = (byte)i;
    for (i = 0; i < XGL_BATCH_TRIANGLES; ++i) {
        xb.lists[i].pt_type = XGL_PT_COLOR_NORMAL_F3D;
        xb.lists[i].num_pts = 3;
        xb.lists[i].pts.color_normal_f3d = xb.points + i * 3;
    }
    clear_calls = swap_calls = polygon_calls = text_calls = extent_calls = flush_calls = 0;
    queued_count = 0;
    actions = orientation = mapping = clip_planes = hlhsr = illumination = 0;
    write_mask = 0; transparency = freeze_z = 0;
    r_refdef.vrect.x = 40; r_refdef.vrect.y = 30;
    r_refdef.vrect.width = 560; r_refdef.vrect.height = 400;
    r_refdef.fov_x = r_refdef.fov_y = 90;
    r_origin[0] = 10; r_origin[1] = 20; r_origin[2] = 30;
    vpn[0] = 1; vright[1] = -1; vup[2] = 1;
}

static void project(float forward, float right, float up, double clip[4])
{
    double world[4];
    int i, j;
    for (i = 0; i < 3; ++i)
        world[i] = r_origin[i] + forward * vpn[i] + right * vright[i] + up * vup[i];
    world[3] = 1;
    for (j = 0; j < 4; ++j) {
        clip[j] = 0;
        for (i = 0; i < 4; ++i) clip[j] += world[i] * view_matrix[i][j];
    }
}

static void check_device_position(const double clip[4], double x, double y)
{
    double device_x, device_y;
    assert(clip[3] > 0);
    device_x = (clip[0] / clip[3] - current_window.xmin) /
        (current_window.xmax - current_window.xmin) * (xb.width - 1);
    device_y = (clip[1] / clip[3] - current_window.ymin) /
        (current_window.ymax - current_window.ymin) * (xb.height - 1);
    assert(fabs(device_x - x) < .001);
    assert(fabs(device_y - y) < .001);
}

static void check_camera_rectangle(double left, double top, double right, double bottom)
{
    double clip[4];
    XGL_BeginScene();
    assert(mapping == XGL_VDC_MAP_ALL && orientation == XGL_Y_DOWN_Z_AWAY);
    assert(hlhsr == XGL_HLHSR_Z_BUFFER && clip_planes != 0);
    assert(current_clip.xmin == -1 && current_clip.xmax == 1);
    assert(current_clip.ymin == -1 && current_clip.ymax == 1);
    assert(current_clip.zmin == 0 && current_clip.zmax == 1);
    assert(current_window.zmin == 0 && current_window.zmax == 1);
    project(0, 0, 0, clip); assert(fabs(clip[3]) < .0001);
    project(100, 0, 0, clip);
    assert(fabs(clip[0]) < .0001 && fabs(clip[1]) < .0001 && fabs(clip[3] - 100) < .0001);
    check_device_position(clip, (left + right) * .5, (top + bottom) * .5);
    project(4, 0, 0, clip); assert(fabs(clip[2] / clip[3]) < .0001);
    project(4096, 0, 0, clip); assert(fabs(clip[2] / clip[3] - 1) < .0001);
    project(3, 0, 0, clip); assert(clip[2] < 0);
    project(8192, 0, 0, clip); assert(clip[2] > clip[3]);
    project(100, 100, 100, clip);
    assert(fabs(clip[0] / clip[3] - 1) < .0001);
    assert(fabs(clip[1] / clip[3] + 1) < .0001);
    check_device_position(clip, right, top);
    project(100, -100, -100, clip);
    check_device_position(clip, left, bottom);
    project(100, -100, 100, clip);
    check_device_position(clip, left, top);
    project(100, 100, -100, clip);
    check_device_position(clip, right, bottom);
    project(100, 101, 0, clip);
    assert(clip[0] > current_clip.xmax * clip[3]);
    project(100, 0, 101, clip);
    assert(clip[1] < current_clip.ymin * clip[3]);
}

static void test_camera(void)
{
    reset_backend();
    check_camera_rectangle(40, 30, 599, 429);
    r_refdef.vrect.x = r_refdef.vrect.y = 0;
    r_refdef.vrect.width = 640; r_refdef.vrect.height = 480;
    check_camera_rectangle(0, 0, 639, 479);
    assert(current_window.xmin == -1 && current_window.xmax == 1);
    assert(current_window.ymin == -1 && current_window.ymax == 1);
    XGLB_Resize(800, 600);
    vid.width = 320; vid.height = 240;
    r_refdef.vrect.x = 17; r_refdef.vrect.y = 13;
    r_refdef.vrect.width = 277; r_refdef.vrect.height = 201;
    check_camera_rectangle(42.5, 32.5, 734, 534);
}

static void test_first_console_frame(void)
{
    float color[3] = {.15f, .1f, .05f};
    reset_backend();
    xb.frame = 0;
    /* The renderer's startup message draws a console frame before any map
     * or scene has been loaded. Exercise that sequence independently. */
    XGL_BeginFrame();
    assert(xb.frame && !xb.scene_active && clear_calls == 1 && !swap_calls);
    assert(mapping == XGL_VDC_MAP_ALL && !clip_planes);
    XGL_UIFillRGB(0, 0, 640, 480, color, 1);
    XGL_UIText(8, 8, "Loading", 1, 8, 8);
    assert(polygon_calls == 1 && text_calls == 7);
    assert(xb.ui_active && !xb.scene_active);
    XGL_EndFrame();
    assert(clear_calls == 1 && swap_calls == 1);
    assert(actions == XGL_CTX_NEW_FRAME_SWITCH_BUFFER);
    assert(!xb.frame && !xb.scene_active && !xb.ui_active && !queued_count);
}

static void test_clear_and_lighting(void)
{
    xgl_vertex_t vertices[3];
    float color[3] = {.2f, .3f, .4f};
    reset_backend();
    XGL_UIFillRGB(0, 0, 640, 480, color, .5f);
    assert(mapping == XGL_VDC_MAP_DEVICE);
    assert(write_mask == 0 && freeze_z && transparency == XGL_TRANSP_BLENDED);
    XGL_BeginFrame();
    assert(clear_calls == 1 && actions == (XGL_CTX_NEW_FRAME_CLEAR | XGL_CTX_NEW_FRAME_HLHSR_ACTION));
    assert(mapping == XGL_VDC_MAP_ALL);
    assert(!clip_planes);
    XGL_BeginScene();
    memset(vertices, 0, sizeof(vertices));
    vertices[0].normal[2] = vertices[1].normal[2] = vertices[2].normal[2] = 1;
    XGL_DrawTriangles(vertices, 3, 2, 1, 1);
    XGL_Flush();
    assert(illumination == XGL_ILLUM_PER_VERTEX && switches[5]);
    assert(fabs(current_ambient - .5f) < .0001);
    assert(submitted_vertex.normal.z == 1);
    XGL_DrawTriangles(vertices, 3, 1, 1, 1);
    XGL_Flush();
    assert(!switches[5] && fabs(current_ambient - 1) < .0001);
    XGL_DrawTriangles(vertices, 3, 0, .5f, 0);
    XGL_Flush();
    assert(illumination == XGL_ILLUM_NONE_INTERP_COLOR && write_mask == 0);
    XGL_SetViewModel(1);
    assert(clear_calls == 2 && actions == XGL_CTX_NEW_FRAME_HLHSR_ACTION);
}

static void test_fonts_and_gamma(void)
{
    int extents, i;
    float original_height, color[3] = {0, .5f, 1};
    xgl_vertex_t vertices[3];
    reset_backend();
    XGL_UIText(10, 20, "Hg|-. ", 1, 8, 8);
    assert(text_calls == 5 && extent_calls == 94);
    for (i = 1; i < 5; ++i) assert(recorded_position[i].y == recorded_position[0].y);
    extents = extent_calls; original_height = text_height;
    XGL_UIText(10, 40, "H", 1, 8, 8);
    assert(extent_calls == extents);
    XGL_UIText(10, 60, "H", 1, 16, 16);
    assert(extent_calls == extents + 94 && text_height > original_height);
    XGL_UIText(10, 80, "H", 1, 8, 8);
    assert(extent_calls == extents + 94 && text_height == original_height);
    XGL_UIFlush();
    assert(queued_count == 0);
    for (i = 0; i < 256; ++i) gammatable[i] = (byte)(255 - i);
    xgl_palette[3] = 64; xgl_palette[4] = 128; xgl_palette[5] = 255;
    XGL_UIText(0, 0, "H", 1, 8, 8);
    assert(fabs(text_color.rgb.r - 191.0 / 255) < .0001);
    assert(fabs(text_color.rgb.g - 127.0 / 255) < .0001 && text_color.rgb.b == 0);
    XGL_UIFillRGB(0, 0, 8, 8, color, 1);
    assert(polygon_color.rgb.r == 1 && polygon_color.rgb.b == 0);
    assert(fabs(polygon_color.rgb.g - 127.0 / 255) < .0001);
    XGL_BeginScene();
    memset(vertices, 0, sizeof(vertices));
    vertices[0].color[0] = -.5f; vertices[0].color[1] = .5f; vertices[0].color[2] = 2;
    XGL_DrawTriangles(vertices, 3, 1, 1, 1); XGL_Flush();
    assert(submitted_vertex.color.rgb.r == 1 && submitted_vertex.color.rgb.b == 0);
    assert(fabs(submitted_vertex.color.rgb.g - 127.0 / 255) < .0001);
}

static void test_point_light_selection(void)
{
    static const float distances[7] = {0, 0, 100, 5, 10, 20, 1};
    int i;
    reset_backend();
    cl.time = 10;
    for (i = 0; i < 7; ++i) {
        cl_dlights[i].die = i == 0 ? 9 : 20;
        cl_dlights[i].radius = i == 1 ? 0 : 100;
        cl_dlights[i].origin[0] = r_origin[0] + distances[i];
        cl_dlights[i].origin[1] = r_origin[1];
        cl_dlights[i].origin[2] = r_origin[2];
    }
    update_lights();
    assert(switches[0] && switches[1] && switches[2] && switches[3] && switches[4]);
    assert(!switches[5]);
    assert(light_position[1].x == r_origin[0] + 1);
    assert(light_position[2].x == r_origin[0] + 5);
    assert(light_position[3].x == r_origin[0] + 10);
    assert(light_position[4].x == r_origin[0] + 20);
    xgl_maxlights.value = 2;
    update_lights();
    assert(switches[1] && switches[2] && !switches[3] && !switches[4]);
    r_dynamic.value = 0;
    update_lights();
    for (i = 1; i < 6; ++i) assert(!switches[i]);
    assert(switches[0]);
}

static void test_error_diagnostics(void)
{
    char first_error[sizeof(xb.error)];
    reset_backend();
    xb.stage = "prepare scene camera";
    injected_error.type = XGL_ERROR_RECOVERABLE;
    injected_error.category = XGL_ERROR_USER;
    injected_error.id = "test-id-123";
    injected_error.msg = "test native failure";
    injected_error.cur_op = "Operator: xgl_object_set";
    injected_error.cur_obj = "Object: context";
    injected_error.operand1 = "Attribute: TEST_ATTRIBUTE";
    injected_error.operand2 = "Value: TEST_VALUE";
    assert(native_error(xb.system) == 1);
    assert(xb.errors == 1 && !xb.fatal);
    assert(strstr(xb.error, "prepare scene camera"));
    assert(strstr(xb.error, "test-id-123"));
    assert(strstr(xb.error, "test native failure"));
    assert(strstr(xb.error, "xgl_object_set"));
    assert(strstr(xb.error, "context"));
    assert(strstr(xb.error, "TEST_ATTRIBUTE"));
    assert(strstr(xb.error, "TEST_VALUE"));
    strcpy(first_error, xb.error);
    xb.stage = "later native operation";
    injected_error.type = XGL_ERROR_NONRECOVERABLE;
    injected_error.msg = "later fatal failure";
    assert(native_error(xb.system) == 1);
    assert(xb.errors == 2 && xb.fatal);
    assert(strcmp(first_error, xb.error) == 0);
    expect_failure = 1;
    if (!setjmp(expected_failure)) {
        native_check();
        assert(!"native_check must reject recorded native errors");
    }
    expect_failure = 0;
    assert(strstr(fatal_message, first_error));

    /* Some Solaris runtimes return empty catalog text, as in the reported
     * startup failure. Stage and operands must still identify the call. */
    reset_backend();
    xb.stage = "prepare frame mapping and clear";
    injected_error.id = "";
    injected_error.msg = "Error number ";
    injected_error.cur_op = "Operator: xgl_object_set";
    injected_error.operand1 = "Attribute: TEST_ATTRIBUTE";
    injected_error.operand2 = "Value: TEST_VALUE";
    assert(native_error(xb.system) == 1);
    assert(strstr(xb.error, "prepare frame mapping and clear"));
    assert(strstr(xb.error, "id=?"));
    assert(strstr(xb.error, "object=?"));
    assert(strstr(xb.error, "TEST_ATTRIBUTE") && strstr(xb.error, "TEST_VALUE"));

    reset_backend();
    injected_error.msg = "";
    injected_error.cur_obj = "";
    injected_error.operand2 = "";
    assert(native_error(xb.system) == 1);
    assert(strstr(xb.error, "id=?") && strstr(xb.error, "op=?"));
    assert(strstr(xb.error, "object=?"));
    assert(strstr(xb.error, "operand1=?") && strstr(xb.error, "operand2=?"));
    assert(!strstr(xb.error, "(null)"));
}

int main(void)
{
    test_first_console_frame();
    test_camera();
    test_clear_and_lighting();
    test_fonts_and_gamma();
    test_point_light_selection();
    test_error_diagnostics();
    puts("XGL backend contracts passed (API state/matrices; no device rendering)");
    return 0;
}
