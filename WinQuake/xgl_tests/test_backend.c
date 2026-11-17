/* Native API state/geometry contracts, not a device renderer or ABI test.
 * Compile against the actual Sun SDK headers; only exercised calls are mocked. */
#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../xgl_profile.h"
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
static int text_color_sets, audio_updates;
static int transparency, freeze_z;
static int transparency_sets;
static float current_ambient, text_height, current_spacing;
static int spacing_mode, proportional_metrics;
static int vertical_advance;
static Xgl_color text_color, polygon_color;
static Xgl_pt_color_normal_f3d submitted_vertex;
static Xgl_pt_color_normal_f3d submitted_vertices[8192];
static const void *submitted_sources[8192 / 3];
static int submitted_count;
static Xgl_color recorded_quad_colors[1024];
static Xgl_pt_f2d recorded_quad_points[1024][4];
static const Xgl_pt_list *queued_quad_lists[1024];
static const Xgl_color_facet *queued_quad_facets[1024];
static int queued_quad_indices[1024], queued_quad_count;
static int recorded_quads, quad_batches[1024], quad_batch_count;
static char native_events[16384];
static int native_event_count;
static Xgl_pt_d3d light_position[6];
static Xgl_pt_f2d recorded_position[4096];
static Xgl_pt_f2d recorded_glyph_position[8192];
static char recorded_glyph[8192];
static int glyph_count;
static Xgl_pt_f2d *queued_position[4096];
static Xgl_pt_f2d queued_copy[4096];
static const char *queued_string[4096];
static char queued_string_copy[4096][XGL_UI_TEXT_RUN + 1];
static int queued_count;
static unsigned long profile_counts[XGL_PROFILE_COUNTER_COUNT];
static int profile_sections[XGL_PROFILE_SECTION_COUNT];
static Xgl_error_info injected_error;
static jmp_buf expected_failure;
static int expect_failure;
static char fatal_message[1024];

void XGL_ProfileInit(void) {}
void XGL_ProfileBeginFrame(void) {}
void XGL_ProfileEndFrame(void) {}
double XGL_ProfileNow(void) { return 0; }
double XGL_ProfileDetailNow(void) { return 0; }
void XGL_ProfileAdd(xgl_profile_section_t section, double start)
{
    assert((unsigned int)section < XGL_PROFILE_SECTION_COUNT);
    (void)start;
    ++profile_sections[section];
}
void XGL_ProfileCount(xgl_profile_counter_t counter, unsigned long count)
{
    assert((unsigned int)counter < XGL_PROFILE_COUNTER_COUNT);
    profile_counts[counter] += count;
}
void XGL_AudioUpdate(void) { ++audio_updates; }

static void native_event(char event)
{
    assert(native_event_count + 1 < (int)sizeof(native_events));
    native_events[native_event_count++] = event;
    native_events[native_event_count] = 0;
}

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
        case XGL_CTX_STEXT_COLOR:
            text_color = *va_arg(args, Xgl_color *); ++text_color_sets; break;
        case XGL_CTX_STEXT_CHAR_HEIGHT: text_height = (float)va_arg(args, double); break;
        case XGL_CTX_STEXT_CHAR_SPACING: current_spacing = (float)va_arg(args, double); break;
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
        case XGL_3D_CTX_SURF_TRANSP_METHOD:
            transparency = va_arg(args, int); ++transparency_sets; break;
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
    native_event(context == xb.ui ? 'f' : 'F');
    if (context == xb.ui) {
        for (i = 0; i < queued_count; ++i) {
            assert(queued_position[i]->x == queued_copy[i].x);
            assert(queued_position[i]->y == queued_copy[i].y);
            assert(!strcmp(queued_string[i], queued_string_copy[i]));
        }
        for (i = 0; i < queued_quad_count; ++i) {
            assert(queued_quad_lists[i]->pt_type == XGL_PT_F2D);
            assert(queued_quad_lists[i]->num_pts == 4);
            assert(!memcmp(queued_quad_lists[i]->pts.f2d,
                recorded_quad_points[queued_quad_indices[i]],
                sizeof(recorded_quad_points[0])));
            assert(!memcmp(&queued_quad_facets[i]->color,
                &recorded_quad_colors[queued_quad_indices[i]], sizeof(Xgl_color)));
        }
        queued_quad_count = 0;
        queued_count = 0;
    }
}

void xgl_context_new_frame(Xgl_ctx context)
{
    assert(context == xb.scene);
    if (actions & XGL_CTX_NEW_FRAME_SWITCH_BUFFER) {
        native_event('S');
        ++swap_calls;
    }
    if (actions & XGL_CTX_NEW_FRAME_HLHSR_ACTION) {
        assert(write_mask == (Xgl_usgn32)0xffffffffUL);
        assert(transparency == XGL_TRANSP_NONE && !freeze_z);
        native_event('C');
        ++clear_calls;
    }
}

void xgl_multi_simple_polygon(Xgl_ctx context, Xgl_facet_flags flags,
                              Xgl_facet_list *facets, Xgl_bbox *box,
                              Xgl_usgn32 count, Xgl_pt_list *lists)
{
    Xgl_usgn32 i;
    int j;
    (void)flags;
    assert(!box && count > 0);
    assert(context == xb.scene || context == xb.ui);
    ++polygon_calls;
    if (lists[0].pt_type == XGL_PT_COLOR_NORMAL_F3D) {
        assert(lists[0].num_pts == 3 && !facets);
        native_event('G');
        /* The renderer owns an SDK-layout-compatible record. Read its bytes
         * like the external native library rather than making host compiler
         * aliasing assumptions between the two C struct types. */
        memcpy(&submitted_vertex, lists[0].pts.color_normal_f3d, sizeof(submitted_vertex));
        for (i = 0; i < count; ++i) {
            assert(lists[i].pt_type == XGL_PT_COLOR_NORMAL_F3D && lists[i].num_pts == 3);
            assert(submitted_count + 3 <= (int)(sizeof(submitted_vertices) / sizeof(submitted_vertices[0])));
            submitted_sources[submitted_count / 3] = lists[i].pts.color_normal_f3d;
            for (j = 0; j < 3; ++j) {
                memcpy(&submitted_vertices[submitted_count],
                       (const byte *)lists[i].pts.color_normal_f3d + j * sizeof(submitted_vertex),
                       sizeof(submitted_vertex));
                ++submitted_count;
            }
        }
    } else {
        assert(facets && facets->facet_type == XGL_FACET_COLOR);
        /* multi_simple_polygon consumes one list of facet colors, not an
         * array of one-facet descriptors. Every batched quad keeps its color. */
        assert(facets->num_facets == count);
        polygon_color = facets->facets.color_facets[0].color;
        if (context == xb.ui) {
            native_event('Q');
            assert(quad_batch_count < (int)(sizeof(quad_batches) / sizeof(quad_batches[0])));
            quad_batches[quad_batch_count++] = (int)count;
            for (i = 0; i < count; ++i) {
                assert(lists[i].pt_type == XGL_PT_F2D && lists[i].num_pts == 4);
                assert(recorded_quads < (int)(sizeof(recorded_quad_colors) / sizeof(recorded_quad_colors[0])));
                recorded_quad_colors[recorded_quads] = facets->facets.color_facets[i].color;
                memcpy(recorded_quad_points[recorded_quads], lists[i].pts.f2d,
                       sizeof(recorded_quad_points[recorded_quads]));
                assert(queued_quad_count < (int)(sizeof(queued_quad_lists) / sizeof(queued_quad_lists[0])));
                queued_quad_lists[queued_quad_count] = &lists[i];
                queued_quad_facets[queued_quad_count] = &facets->facets.color_facets[i];
                queued_quad_indices[queued_quad_count++] = recorded_quads;
                ++recorded_quads;
            }
        } else native_event('A');
    }
}

static void mock_glyph(int ch, Xgl_bounds_f2d *bounds, Xgl_pt_f2d *advance)
{
    assert(ch >= 33 && ch <= 126);
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
    if (vertical_advance && ch == 'g') advance->y = .1f * text_height;
    if (proportional_metrics && ch == 'i') {
        bounds->xmax = .1f * text_height;
        advance->x = .3f * text_height;
    }
    if (proportional_metrics && ch == 'W') {
        bounds->xmax = .8f * text_height;
        advance->x = .95f * text_height;
    }
}

static float mock_spacing(void)
{
    if (spacing_mode == 2) return 0;
    return current_spacing * (spacing_mode == 1 ? 1 : text_height);
}

void xgl_stroke_text_extent(Xgl_ctx context, void *string,
                            Xgl_bounds_f2d *bounds, Xgl_pt_f2d *advance)
{
    const char *text;
    Xgl_bounds_f2d glyph;
    Xgl_pt_f2d step;
    float cursor, cursor_y;
    int first;
    assert(context == xb.ui && text_height > 0);
    ++extent_calls;
    text = (const char *)string; cursor = cursor_y = 0; first = 1;
    while (*text) {
        mock_glyph((unsigned char)*text++, &glyph, &step);
        glyph.xmin += cursor; glyph.xmax += cursor;
        glyph.ymin += cursor_y; glyph.ymax += cursor_y;
        if (first) { *bounds = glyph; first = 0; }
        else {
            if (glyph.xmin < bounds->xmin) bounds->xmin = glyph.xmin;
            if (glyph.xmax > bounds->xmax) bounds->xmax = glyph.xmax;
            if (glyph.ymin < bounds->ymin) bounds->ymin = glyph.ymin;
            if (glyph.ymax > bounds->ymax) bounds->ymax = glyph.ymax;
        }
        cursor += step.x + mock_spacing();
        cursor_y += step.y;
    }
    advance->x = cursor; advance->y = cursor_y;
}

void xgl_stroke_text(Xgl_ctx context, void *string, void *origin, void *direction)
{
    const char *text;
    Xgl_bounds_f2d bounds;
    Xgl_pt_f2d advance;
    float cursor, cursor_y;
    assert(context == xb.ui && !direction && *(char *)string);
    native_event('T');
    assert(text_calls < 4096 && queued_count < 4096);
    recorded_position[text_calls++] = *(Xgl_pt_f2d *)origin;
    queued_position[queued_count] = (Xgl_pt_f2d *)origin;
    queued_string[queued_count] = (const char *)string;
    assert(strlen((const char *)string) <= XGL_UI_TEXT_RUN);
    strcpy(queued_string_copy[queued_count], (const char *)string);
    queued_copy[queued_count++] = *(Xgl_pt_f2d *)origin;
    text = (const char *)string; cursor = cursor_y = 0;
    while (*text) {
        assert(glyph_count < 8192);
        recorded_glyph[glyph_count] = *text;
        recorded_glyph_position[glyph_count] = *(Xgl_pt_f2d *)origin;
        recorded_glyph_position[glyph_count].y += cursor_y;
        recorded_glyph_position[glyph_count++].x += cursor;
        mock_glyph((unsigned char)*text++, &bounds, &advance);
        cursor += advance.x + mock_spacing();
        cursor_y += advance.y;
    }
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
    xb.frame = 1; xb.current_text_height = xb.current_text_spacing = -1;
    xb.applied_lit = xb.applied_zwrite = -1; xb.applied_opacity = -1;
    xgl_ambient.value = 1; xgl_maxlights.value = 4; r_dynamic.value = 1;
    for (i = 0; i < 95; ++i) xb.glyphs[i][0] = (char)(i + 32);
    for (i = 0; i < 256; ++i) gammatable[i] = (byte)i;
    for (i = 0; i < XGL_BATCH_TRIANGLES; ++i) {
        xb.lists[i].pt_type = XGL_PT_COLOR_NORMAL_F3D;
        xb.lists[i].num_pts = 3;
        xb.lists[i].pts.color_normal_f3d = (Xgl_pt_color_normal_f3d *)(xb.points + i * 3);
    }
    clear_calls = swap_calls = polygon_calls = text_calls = extent_calls = flush_calls = 0;
    text_color_sets = audio_updates = submitted_count = 0;
    glyph_count = spacing_mode = proportional_metrics = vertical_advance = 0;
    current_spacing = 0;
    memset(profile_counts, 0, sizeof(profile_counts));
    memset(profile_sections, 0, sizeof(profile_sections));
    recorded_quads = quad_batch_count = native_event_count = 0;
    native_events[0] = 0;
    queued_count = queued_quad_count = 0;
    actions = orientation = mapping = clip_planes = hlhsr = illumination = 0;
    write_mask = 0; transparency = freeze_z = 0;
    transparency_sets = 0;
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
    assert(polygon_calls == 1 && text_calls == 1 && glyph_count == 7);
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

static void test_weapon_clear_state_reuse(void)
{
    xgl_vertex_t vertices[3];
    int sets, waits, polygons;
    reset_backend(); XGL_BeginScene();
    memset(vertices, 0, sizeof(vertices));
    XGL_DrawTriangles(vertices, 3, 2, 1, 1); XGL_Flush();
    sets = transparency_sets; waits = flush_calls; polygons = polygon_calls;
    assert(illumination == XGL_ILLUM_PER_VERTEX && switches[5]);
    XGL_SetViewModel(1);
    assert(transparency_sets == sets && polygon_calls == polygons);
    assert(flush_calls == waits + 1 && clear_calls == 1);
    assert(actions == XGL_CTX_NEW_FRAME_HLHSR_ACTION);
    assert(xb.applied_opacity == 1 && xb.applied_zwrite == 1);
    assert(illumination == XGL_ILLUM_PER_VERTEX && switches[5]);
    assert(profile_sections[XGL_PROFILE_WEAPON_CLEAR] == 1);
    XGL_DrawTriangles(vertices, 3, 2, 1, 1); XGL_Flush();
    assert(transparency_sets == sets && polygon_calls == polygons + 1);
    assert(profile_counts[XGL_PROFILE_TRIANGLES] == 2);
    XGL_SetViewModel(0);
    assert(clear_calls == 1 && flush_calls == waits + 2);
    assert(profile_sections[XGL_PROFILE_WEAPON_CLEAR] == 1);

    /* A blended preceding layer must restore opaque drawing and depth
     * writes before the clear, then reuse those values for the weapon. */
    reset_backend(); XGL_BeginScene();
    XGL_DrawTriangles(vertices, 3, 0, .5f, 0); XGL_Flush();
    sets = transparency_sets; waits = flush_calls;
    assert(transparency == XGL_TRANSP_BLENDED && freeze_z && !write_mask);
    XGL_SetViewModel(1);
    assert(transparency_sets == sets + 1 && flush_calls == waits + 1);
    assert(transparency == XGL_TRANSP_NONE && !freeze_z);
    assert(write_mask == (Xgl_usgn32)0xffffffffUL);
    assert(xb.applied_opacity == 1 && xb.applied_zwrite == 1);
    assert(illumination == XGL_ILLUM_NONE_INTERP_COLOR);
    assert(profile_sections[XGL_PROFILE_WEAPON_CLEAR] == 1);
    XGL_DrawTriangles(vertices, 3, 2, 1, 1); XGL_Flush();
    assert(transparency_sets == sets + 1 && polygon_calls == 2);
    assert(illumination == XGL_ILLUM_PER_VERTEX && switches[5]);
    assert(profile_counts[XGL_PROFILE_TRIANGLES] == 2);
    XGL_SetViewModel(0);
    assert(flush_calls == waits + 2 && clear_calls == 1);
}

static void test_fonts_and_gamma(void)
{
    int extents, i;
    float original_height, color[3] = {0, .5f, 1};
    xgl_vertex_t vertices[3];
    reset_backend();
    XGL_UIText(10, 20, "Hg|-. ", 1, 8, 8);
    assert(text_calls == 4 && glyph_count == 5 && extent_calls == 97);
    for (i = 1; i < 5; ++i)
        assert(recorded_glyph_position[i].y == recorded_glyph_position[0].y);
    extents = extent_calls; original_height = text_height;
    XGL_UIText(10, 40, "H", 1, 8, 8);
    assert(extent_calls == extents);
    XGL_UIText(10, 60, "H", 1, 16, 16);
    assert(extent_calls == extents + 97 && text_height > original_height);
    XGL_UIText(10, 80, "H", 1, 8, 8);
    assert(extent_calls == extents + 97 && text_height == original_height);
    XGL_UIFlush();
    assert(queued_count == 0);
    for (i = 0; i < 256; ++i) gammatable[i] = (byte)(255 - i);
    xgl_palette[3] = 64; xgl_palette[4] = 128; xgl_palette[5] = 255;
    XGL_BeginFrame();
    XGL_UIText(0, 0, "H", 1, 8, 8);
    assert(fabs(text_color.rgb.r - 191.0 / 255) < .0001);
    assert(fabs(text_color.rgb.g - 127.0 / 255) < .0001 && text_color.rgb.b == 0);
    XGL_UIFillRGB(0, 0, 8, 8, color, 1);
    XGL_UIFlush();
    assert(polygon_color.rgb.r == 1 && polygon_color.rgb.b == 0);
    assert(fabs(polygon_color.rgb.g - 127.0 / 255) < .0001);
    XGL_BeginScene();
    memset(vertices, 0, sizeof(vertices));
    vertices[0].color[0] = -.5f; vertices[0].color[1] = .5f; vertices[0].color[2] = 2;
    XGL_DrawTriangles(vertices, 3, 1, 1, 1); XGL_Flush();
    assert(submitted_vertex.color.rgb.r == 1 && submitted_vertex.color.rgb.b == 0);
    assert(fabs(submitted_vertex.color.rgb.g - 127.0 / 255) < .0001);
}

static void test_strided_prepared_colors(void)
{
    typedef struct {
        xgl_vertex_t draw;
        unsigned long unrelated[5];
    } padded_vertex_t;
    padded_vertex_t vertices[XGL_BATCH_TRIANGLES * 3 + 3];
    xgl_vertex_t raw[3];
    unsigned int revision;
    int i, count;
    reset_backend();
    revision = XGL_ColorRevision();
    assert(revision != 0 && XGL_ColorRevision() == revision);
    XGL_BeginFrame();
    assert(XGL_ColorRevision() == revision);
    for (i = 0; i < 256; ++i) gammatable[i] = (byte)(255 - i);
    XGL_BeginFrame();
    assert(XGL_ColorRevision() != revision);
    revision = XGL_ColorRevision();
    XGL_BeginFrame();
    assert(XGL_ColorRevision() == revision);
    XGL_BeginScene();
    count = (int)(sizeof(vertices) / sizeof(vertices[0]));
    memset(vertices, 0xcd, sizeof(vertices));
    for (i = 0; i < count; ++i) {
        vertices[i].draw.position[0] = (float)i;
        vertices[i].draw.position[1] = (float)i + .25f;
        vertices[i].draw.position[2] = (float)i + .5f;
        vertices[i].draw.normal[0] = 0;
        vertices[i].draw.normal[1] = 0;
        vertices[i].draw.normal[2] = 1;
        vertices[i].draw.color[0] = 0;
        vertices[i].draw.color[1] = .5f;
        vertices[i].draw.color[2] = 1;
        XGL_PrepareColor(vertices[i].draw.color);
    }
    XGL_DrawTrianglesStrided(&vertices[0].draw, count, sizeof(vertices[0]), 1, 1, 1, 1);
    XGL_Flush();
    assert(submitted_count == count && polygon_calls == 2);
    for (i = 0; i < count; ++i) {
        assert(submitted_vertices[i].x == (float)i);
        assert(submitted_vertices[i].y == (float)i + .25f);
        assert(submitted_vertices[i].z == (float)i + .5f);
        assert(submitted_vertices[i].normal.z == 1);
        assert(submitted_vertices[i].color.rgb.r == 1 && submitted_vertices[i].color.rgb.b == 0);
        assert(fabs(submitted_vertices[i].color.rgb.g - 127.0 / 255) < .0001);
    }
    /* The existing entry point still accepts raw colors and applies gamma
     * once; prepared cache colors must not receive that conversion again. */
    memset(raw, 0, sizeof(raw));
    for (i = 0; i < 3; ++i) { raw[i].color[1] = .5f; raw[i].color[2] = 1; }
    XGL_DrawTriangles(raw, 3, 1, 1, 1);
    XGL_Flush();
    assert(submitted_vertex.color.rgb.r == 1 && submitted_vertex.color.rgb.b == 0);
    assert(fabs(submitted_vertex.color.rgb.g - 127.0 / 255) < .0001);
}

static void fill_native_vertices(xgl_native_vertex_t *vertices, int count, int first)
{
    int i;
    for (i = 0; i < count; ++i) {
        vertices[i].position[0] = (float)(first + i);
        vertices[i].position[1] = .25f;
        vertices[i].position[2] = .5f;
        vertices[i].normal[0] = vertices[i].normal[1] = 0;
        vertices[i].normal[2] = 1;
        vertices[i].color.rgb[0] = .2f;
        vertices[i].color.rgb[1] = .4f;
        vertices[i].color.rgb[2] = .6f;
    }
}

static void test_native_batches(void)
{
    xgl_native_vertex_t cached[XGL_BATCH_TRIANGLES * 3 + 3], *reserved;
    xgl_vertex_t raw[3];
    int i, count, total;
    reset_backend();
    count = 3;
    assert(!XGL_ReserveTriangles(&count, 1, 1, 1) && count == 0);
    XGL_BeginScene();
    fill_native_vertices(cached, XGL_BATCH_TRIANGLES * 3 + 3, 0);
    XGL_DrawNativeTriangles(cached, 6, 1, 1, 1);
    assert(xb.count == 6 && !polygon_calls);
    assert((const void *)xb.lists[0].pts.color_normal_f3d == cached);
    count = XGL_BATCH_TRIANGLES * 3;
    reserved = XGL_ReserveTriangles(&count, 1, 1, 1);
    assert(reserved == xb.points + 6 && count == XGL_BATCH_TRIANGLES * 3 - 6);
    fill_native_vertices(reserved, count, 6);
    assert(!polygon_calls); /* a full reservation is not submitted unfilled */
    XGL_Flush();
    assert(submitted_count == XGL_BATCH_TRIANGLES * 3 && polygon_calls == 1);
    assert(submitted_sources[0] == cached && submitted_sources[1] == cached + 3);
    for (i = 0; i < submitted_count; ++i) {
        assert(submitted_vertices[i].x == (float)i);
        assert(submitted_vertices[i].normal.z == 1);
        assert(submitted_vertices[i].color.rgb.r == .2f);
    }
    assert(profile_counts[XGL_PROFILE_NATIVE_CACHED_VERTICES] == 6);
    assert(profile_counts[XGL_PROFILE_NATIVE_DIRECT_VERTICES] == (unsigned long)count);
    assert(!profile_counts[XGL_PROFILE_VERTEX_PACKED]);
    /* Native volatile submission has consumed the records before a cached
     * mesh or the owned batch can be changed/reused. */
    cached[0].position[0] = -99;
    reserved[0].position[0] = -88;
    assert(submitted_vertices[0].x == 0 && submitted_vertices[6].x == 6);

    reset_backend(); XGL_BeginScene();
    fill_native_vertices(cached, XGL_BATCH_TRIANGLES * 3 + 3, 0);
    XGL_DrawNativeTriangles(cached, XGL_BATCH_TRIANGLES * 3 + 3, 1, 1, 1);
    assert(polygon_calls == 1 && xb.count == 3);
    memset(raw, 0, sizeof(raw));
    for (i = 0; i < 3; ++i) { raw[i].position[0] = 771.0f + i; raw[i].color[0] = 1; }
    XGL_DrawTriangles(raw, 3, 1, 1, 1);
    count = 3;
    reserved = XGL_ReserveTriangles(&count, 1, 1, 1);
    assert(reserved && count == 3);
    fill_native_vertices(reserved, count, 774);
    XGL_Flush();
    total = XGL_BATCH_TRIANGLES * 3 + 9;
    assert(submitted_count == total && polygon_calls == 2);
    for (i = 0; i < total; ++i) assert(submitted_vertices[i].x == (float)i);
    assert(submitted_sources[256] == cached + XGL_BATCH_TRIANGLES * 3);
    assert(submitted_sources[257] == xb.points + 3);
    assert(submitted_sources[258] == xb.points + 6);
    assert(profile_counts[XGL_PROFILE_NATIVE_CACHED_VERTICES] == 771);
    assert(profile_counts[XGL_PROFILE_NATIVE_DIRECT_VERTICES] == 3);
    assert(profile_counts[XGL_PROFILE_VERTEX_PACKED] == 3);

    /* State boundaries flush completed borrowed and reserved records. */
    reset_backend(); XGL_BeginScene();
    XGL_DrawNativeTriangles(cached, 3, 1, 1, 1);
    count = 3;
    reserved = XGL_ReserveTriangles(&count, 0, .5f, 0);
    assert(polygon_calls == 1 && count == 3);
    fill_native_vertices(reserved, count, 3);
    XGL_Flush();
    assert(polygon_calls == 2 && submitted_count == 6);
    assert(illumination == XGL_ILLUM_NONE_INTERP_COLOR);
    assert(transparency == XGL_TRANSP_BLENDED && freeze_z && !write_mask);
    count = 3;
    assert(!XGL_ReserveTriangles(&count, 1, 0, 1) && !count);
    XGL_DrawNativeTriangles(cached, 3, 1, 0, 1);
    assert(!xb.count && polygon_calls == 2);
}

static void test_prepared_palette(void)
{
    float expected[3], actual[3];
    int i, component, pass;
    reset_backend();
    for (i = 0; i < 768; ++i) xgl_palette[i] = (byte)(i * 73);
    for (pass = 0; pass < 2; ++pass) {
        for (i = 0; i < 256; ++i) gammatable[i] = (byte)(pass ? 255 - i : i);
        XGL_BeginFrame();
        for (i = 0; i < 256; ++i) {
            for (component = 0; component < 3; ++component)
                expected[component] = xgl_palette[i * 3 + component] / 255.0f;
            XGL_PrepareColor(expected);
            XGL_PreparePaletteColor(i + 256, actual);
            assert(!memcmp(expected, actual, sizeof(expected)));
        }
    }
}

static void check_text_cells(const char *text, float x, float y,
                             float cell, float height, int first)
{
    xgl_font_size_t *font;
    const Xgl_bounds_f2d *bounds;
    float sx, sy, expected;
    int character, index;
    font = font_size(cell, height);
    sx = (float)xb.width / vid.width; sy = (float)xb.height / vid.height;
    index = first;
    while (*text) {
        character = ((unsigned char)*text++) & 127;
        if (character < 32 || character > 126) character = '?';
        if (character != ' ' && x + cell > 0 && x < vid.width &&
            y + height > 0 && y < vid.height) {
            assert(index < glyph_count && recorded_glyph[index] == character);
            bounds = &font->bounds[character - 32];
            expected = x*sx + (cell*sx - (bounds->xmax - bounds->xmin))*.5f - bounds->xmin;
            assert(fabs(recorded_glyph_position[index].x - expected) < .003);
            assert(fabs(recorded_glyph_position[index].y - (y*sy + font->ascent)) < .003);
            ++index;
        }
        x += cell;
    }
    assert(index == glyph_count);
}

static void test_text_runs(void)
{
    char mutable_text[65], long_text[301];
    const char *mixed = "HHiiWW Hg|-. HHH";
    int first, extents, calls, i;
    reset_backend();
    XGL_UIText(8, 16, "HHH Loading", 2, 8, 8);
    assert(text_calls == 2 && glyph_count == 10);
    assert(profile_counts[XGL_PROFILE_TEXT_CALLS] == 2);
    assert(profile_counts[XGL_PROFILE_GLYPHS] == 10);
    check_text_cells("HHH Loading", 8, 16, 8, 8, 0);
    first = glyph_count; extents = extent_calls;
    XGL_UIText(32, 24, "HHH", 3, 8, 8);
    assert(extent_calls == extents && text_calls == 3);
    check_text_cells("HHH", 32, 24, 8, 8, first);
    XGL_UIFlush();
    assert(!queued_count);

    /* Unequal widths/advances keep the old centered glyph origins. A run
     * may end at an unusual glyph but cannot shift following characters. */
    reset_backend(); proportional_metrics = 1;
    XGL_UIText(8, 16, mixed, 2, 8, 8);
    assert(text_calls > 3 && text_calls < glyph_count);
    check_text_cells(mixed, 8, 16, 8, 8, 0);
    first = glyph_count;
    XGL_UIText(-16, 32, "ABHHH", 2, 8, 8);
    check_text_cells("ABHHH", -16, 32, 8, 8, first);
    first = glyph_count;
    XGL_UIText(632, 48, "HHHH", 2, 8, 8);
    check_text_cells("HHHH", 632, 48, 8, 8, first);
    first = glyph_count;
    XGL_UIText(16, -8, "HHHH", 2, 8, 8);
    assert(glyph_count == first);
    first = glyph_count;
    XGL_UIText(16, 64, "\001\310 H", 2, 8, 8);
    check_text_cells("\001\310 H", 16, 64, 8, 8, first);
    XGL_UIFlush();

    /* Calibration uses the native result, including implementations with
     * a different spacing unit or no useful spacing response. */
    reset_backend(); spacing_mode = 1;
    XGL_UIText(0, 0, "HHHH", 2, 8, 8);
    assert(text_calls == 1);
    check_text_cells("HHHH", 0, 0, 8, 8, 0);
    reset_backend(); spacing_mode = 2;
    XGL_UIText(0, 0, "HHHH", 2, 8, 8);
    assert(text_calls == 4 && current_spacing == 0);
    check_text_cells("HHHH", 0, 0, 8, 8, 0);
    reset_backend(); vertical_advance = 1;
    XGL_UIText(0, 0, "HHggHH", 2, 8, 8);
    assert(text_calls == 3);
    check_text_cells("HHggHH", 0, 0, 8, 8, 0);
    XGL_UIFlush();

    /* Neither caller storage nor scratch strings may be borrowed by an
     * asynchronous native submission. Flush checks every saved byte. */
    reset_backend();
    memset(mutable_text, 'H', 64); mutable_text[64] = 0;
    XGL_UIText(0, 0, mutable_text, 2, 8, 8);
    memset(mutable_text, 'Q', 64);
    for (i = 0; i < 79; ++i) XGL_UIText(0, 8, mutable_text, 2, 8, 8);
    assert(text_calls == 80 && glyph_count == 5120 && flush_calls >= 1);
    XGL_UIFlush();
    assert(!queued_count && !xb.ui_text_used);

    reset_backend();
    xb.width = vid.width = 3000;
    memset(long_text, 'H', 300); long_text[300] = 0;
    XGL_UIText(0, 0, long_text, 2, 8, 8);
    assert(text_calls == 3 && glyph_count == 300);
    check_text_cells(long_text, 0, 0, 8, 8, 0);
    calls = text_calls;
    XGLB_Resize(1500, 480);
    first = glyph_count;
    XGL_UIText(0, 0, "HHH", 2, 8, 8);
    assert(text_calls == calls + 1);
    check_text_cells("HHH", 0, 0, 8, 8, first);
    XGL_UIFlush();
}

static void test_ui_batch_order_and_capacity(void)
{
    float red[3] = {1, 0, 0}, green[3] = {0, 1, 0}, blue[3] = {0, 0, 1};
    const char *quad, *text, *alpha;
    int i;
    reset_backend();
    XGL_UIFillRGB(0, 0, 20, 20, red, 1);
    XGL_UIFillRGB(2, 3, 12, 10, green, 1);
    assert(polygon_calls == 0);
    XGL_UIText(3, 4, "H", 1, 8, 8);
    assert(polygon_calls == 1 && quad_batches[0] == 2 && recorded_quads == 2);
    assert(recorded_quad_colors[0].rgb.r == 1 && recorded_quad_colors[1].rgb.g == 1);
    assert(recorded_quad_points[0][0].x == 0 && recorded_quad_points[1][0].x == 2);
    assert(recorded_quad_points[1][0].y == 3 && recorded_quad_points[1][2].y == 13);
    quad = strchr(native_events, 'Q'); text = strchr(native_events, 'T');
    assert(quad && text && quad < text);
    XGL_UIFillRGB(4, 5, 8, 8, blue, 1);
    assert(polygon_calls == 1);
    XGL_UIFillRGB(0, 0, 30, 30, red, .5f);
    assert(polygon_calls == 3 && quad_batches[1] == 1 && recorded_quads == 3);
    assert(recorded_quad_colors[2].rgb.b == 1);
    quad = strchr(text, 'Q'); alpha = strchr(native_events, 'A');
    assert(quad && alpha && text < quad && quad < alpha);
    XGL_UIFillRGB(1, 2, 8, 8, green, 1);
    XGL_EndFrame();
    quad = strchr(alpha, 'Q');
    assert(quad && strchr(quad, 'S') && quad < strchr(quad, 'S'));
    assert(recorded_quads == 4 && recorded_quad_colors[3].rgb.g == 1);

    reset_backend();
    for (i = 0; i < XGL_UI_QUADS + 3; ++i)
        XGL_UIFillRGB((float)(i % 100), 0, 1, 1, i & 1 ? red : green, 1);
    XGL_UIFlush();
    assert(quad_batch_count == 2 && quad_batches[0] == XGL_UI_QUADS && quad_batches[1] == 3);
    assert(recorded_quads == XGL_UI_QUADS + 3);
    for (i = 0; i < recorded_quads; ++i) {
        assert(recorded_quad_points[i][0].x == (float)(i % 100));
        assert(recorded_quad_colors[i].rgb.r == (float)(i & 1));
        assert(recorded_quad_colors[i].rgb.g == (float)(!(i & 1)));
    }
}

static void test_completion_and_attribute_reuse(void)
{
    xgl_vertex_t triangle[3];
    int completed, colors, extents;
    reset_backend();
    XGL_UIFlush(); complete_scene();
    assert(flush_calls == 0);
    XGL_BeginScene();
    memset(triangle, 0, sizeof(triangle));
    XGL_DrawTriangles(triangle, 3, 0, 1, 1);
    XGL_EndScene();
    assert(polygon_calls == 1 && flush_calls == 1);
    completed = flush_calls;
    XGL_EndScene(); complete_scene(); XGL_UIFlush();
    assert(flush_calls == completed);
    XGL_UIText(0, 0, "H", 2, 8, 8);
    colors = text_color_sets;
    XGL_UIText(8, 0, "g", 2, 8, 8);
    assert(text_color_sets == colors);
    XGL_UIFlush();
    assert(flush_calls == completed + 1);
    XGL_UIFlush();
    assert(flush_calls == completed + 1);
    extents = extent_calls;
    XGLB_Resize(800, 600);
    XGL_UIText(0, 0, "H", 2, 8, 8);
    assert(extent_calls > extents);
    XGL_EndFrame();
    assert(swap_calls == 1 && !queued_count);
    completed = flush_calls;
    XGL_EndFrame();
    assert(flush_calls == completed && swap_calls == 1);
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
    float color[3] = {1, 0, 0};
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

    /* Deferred UI geometry must not reach XGL after an unrecoverable error. */
    reset_backend();
    XGL_UIFillRGB(0, 0, 8, 8, color, 1);
    assert(!polygon_calls);
    injected_error.type = XGL_ERROR_NONRECOVERABLE;
    assert(native_error(xb.system) == 1);
    expect_failure = 1;
    if (!setjmp(expected_failure)) {
        XGL_UIFlush();
        assert(!"queued geometry must stop after a fatal XGL error");
    }
    expect_failure = 0;
    assert(!polygon_calls && !flush_calls);
}

int main(void)
{
    test_first_console_frame();
    test_camera();
    test_clear_and_lighting();
    test_weapon_clear_state_reuse();
    test_fonts_and_gamma();
    test_text_runs();
    test_strided_prepared_colors();
    test_native_batches();
    test_prepared_palette();
    test_ui_batch_order_and_capacity();
    test_completion_and_attribute_reuse();
    test_point_light_selection();
    test_error_diagnostics();
    puts("XGL backend contracts passed (API state/matrices; no device rendering)");
    return 0;
}
