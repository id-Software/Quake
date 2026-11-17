/* Native Sun XGL renderer for XGLQuake. GPL-2.0-or-later.
 * Uses the XGL 3.x SDK directly: no OpenGL and no texture/raster UI uploads.
 */
#include "quakedef.h"
#include "xgl_local.h"
#include "xgl_backend.h"
#include "xgl_audio.h"
#include "xgl_profile.h"
#include <xgl/xgl.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Both cached world vertices and the owned batch use this native record.
 * Check the SDK layout at compile time rather than assuming host/Sun sizes. */
typedef char xgl_native_vertex_layout[
    sizeof(xgl_native_vertex_t) == sizeof(Xgl_pt_color_normal_f3d) &&
    offsetof(xgl_native_vertex_t, position) == offsetof(Xgl_pt_color_normal_f3d, x) &&
    offsetof(xgl_native_vertex_t, color) == offsetof(Xgl_pt_color_normal_f3d, color) &&
    offsetof(xgl_native_vertex_t, normal) == offsetof(Xgl_pt_color_normal_f3d, normal)
    ? 1 : -1];

#define XGL_BATCH_TRIANGLES 256
#define XGL_FONT_CACHE 128
#define XGL_POINT_LIGHTS 4
#define XGL_UI_GLYPHS 2048
#define XGL_UI_TEXT_BYTES (XGL_UI_GLYPHS * 2)
#define XGL_UI_TEXT_RUN 128
#define XGL_UI_QUADS 256

typedef struct {
    int valid, batchable;
    float cell, height, requested, ascent, spacing, extra_advance;
    Xgl_bounds_f2d bounds[95];
    Xgl_pt_f2d advance[95];
} xgl_font_size_t;

typedef struct {
    Xgl_sys_state system;
    Xgl_win_ras raster;
    Xgl_ctx scene, ui;
    Xgl_trans model, view;
    Xgl_light lights[6];
    Xgl_boolean light_switches[6];
    Xgl_usgn32 buffers;
    xgl_native_vertex_t points[XGL_BATCH_TRIANGLES * 3];
    Xgl_pt_list lists[XGL_BATCH_TRIANGLES];
    xgl_font_size_t fonts[XGL_FONT_CACHE];
    Xgl_pt_f2d ui_positions[XGL_UI_GLYPHS];
    char ui_text[XGL_UI_TEXT_BYTES];
    Xgl_pt_f2d ui_points[XGL_UI_QUADS][4];
    Xgl_pt_list ui_lists[XGL_UI_QUADS];
    Xgl_color_facet ui_colors[XGL_UI_QUADS];
    Xgl_facet_list ui_facets[XGL_UI_QUADS];
    char glyphs[95][2];
    int width, height, frame, scene_active, ui_active, overlay_3d, console_lines;
    int count, lit, zwrite, applied_lit, applied_zwrite;
    int errors, fatal, font_next, ui_glyph_count, ui_quad_count, ui_text_used;
    int scene_pending, ui_pending, ui_quad_first, text_color_valid;
    Xgl_color text_color;
    byte gamma_snapshot[256];
    float gamma_output[256];
    unsigned int color_revision;
    float opacity, applied_opacity, current_text_height, current_text_spacing;
    double maximum_z;
    const char *stage;
    char error[768];
} xgl_backend_t;

static xgl_backend_t xb;
static cvar_t xgl_ambient = {"xgl_ambient", "1", true, false, 0, NULL};
static cvar_t xgl_maxlights = {"xgl_maxlights", "4", true, false, 0, NULL};

static void update_gamma(void);

static float bound_color(float value)
{
    if (!(value >= 0)) return 0;
    return value > 1 ? 1 : value;
}

static float gamma_color(float value)
{
    if (!xb.color_revision) update_gamma();
    return xb.gamma_output[(int)(bound_color(value) * 255.0f + .5f)];
}

static void update_gamma(void)
{
    int i;
    if (!xb.color_revision || memcmp(xb.gamma_snapshot, gammatable, 256)) {
        memcpy(xb.gamma_snapshot, gammatable, 256);
        for (i = 0; i < 256; ++i) xb.gamma_output[i] = gammatable[i] / 255.0f;
        if (!++xb.color_revision) ++xb.color_revision;
        xb.text_color_valid = 0;
    }
}

unsigned int XGL_ColorRevision(void)
{
    if (!xb.color_revision) update_gamma();
    return xb.color_revision;
}

void XGL_PrepareColor(float rgb[3])
{
    int i;
    for (i = 0; i < 3; ++i) rgb[i] = gamma_color(rgb[i]);
}

void XGL_PreparePaletteColor(int index, float rgb[3])
{
    if (!xb.color_revision) update_gamma();
    index &= 255;
    rgb[0] = xb.gamma_output[xgl_palette[index * 3]];
    rgb[1] = xb.gamma_output[xgl_palette[index * 3 + 1]];
    rgb[2] = xb.gamma_output[xgl_palette[index * 3 + 2]];
}

static const char *error_detail(const char *text)
{
    return text && *text ? text : "?";
}

static Xgl_sgn32 native_error(Xgl_sys_state system)
{
    Xgl_error_info info;
    memset(&info, 0, sizeof(info));
    xgl_object_get(system, XGL_SYS_ST_ERROR_INFO, &info);
    if (!xb.errors)
        sprintf(xb.error,
            "%.48s: id=%.24s type=%d category=%d: %.128s "
            "(op=%.40s; object=%.40s; operand1=%.80s; operand2=%.80s)",
            xb.stage ? xb.stage : "native operation", error_detail(info.id),
            (int)info.type, (int)info.category, error_detail(info.msg),
            error_detail(info.cur_op), error_detail(info.cur_obj),
            error_detail(info.operand1), error_detail(info.operand2));
    ++xb.errors;
    if (info.type == XGL_ERROR_NONRECOVERABLE) xb.fatal = 1;
    /* Solaris installations can return an empty ID and only "Error number"
     * as the message. Keep the object/operands, where XGL identifies the
     * rejected attribute. Use stderr: Con_Printf can re-enter rendering. */
    fprintf(stderr,
        "XGL error during %s: id=%s type=%d category=%d\n"
        "  op=%s; object=%s\n  operand1=%s; operand2=%s\n  message=%s\n",
        xb.stage ? xb.stage : "native operation", error_detail(info.id),
        (int)info.type, (int)info.category,
        error_detail(info.cur_op), error_detail(info.cur_obj),
        error_detail(info.operand1), error_detail(info.operand2), error_detail(info.msg));
    return 1;
}

static void native_check_at(int line)
{
    if (xb.errors) Sys_Error("XGL at xgl_backend.c:%d: %s", line, xb.error);
}

#define native_check() native_check_at(__LINE__)

static void identity_transform(Xgl_trans transform)
{
    Xgl_matrix_f3d matrix;
    int i;
    memset(matrix, 0, sizeof(matrix));
    for (i = 0; i < 4; ++i) matrix[i][i] = 1;
    xb.stage = "identity transform";
    xgl_transform_write(transform, matrix);
    native_check();
}

static void palette_color(int index, Xgl_color *color)
{
    if (!xb.color_revision) update_gamma();
    index &= 255;
    memset(color, 0, sizeof(*color));
    color->rgb.r = xb.gamma_output[xgl_palette[index * 3]];
    color->rgb.g = xb.gamma_output[xgl_palette[index * 3 + 1]];
    color->rgb.b = xb.gamma_output[xgl_palette[index * 3 + 2]];
}

static void complete_scene(void)
{
    double start;
    xb.stage = "scene completion";
    if (xb.scene && xb.scene_pending && !xb.fatal) {
        XGL_AudioUpdate();
        start = XGL_ProfileNow();
        xgl_context_flush(xb.scene, XGL_FLUSH_SYNCHRONIZE);
        XGL_ProfileAdd(XGL_PROFILE_SYNC, start);
        XGL_ProfileCount(XGL_PROFILE_WAIT_CALLS, 1);
        native_check();
        xb.scene_pending = 0;
        XGL_AudioUpdate();
    }
    native_check();
}

static void submit_ui_quads(void)
{
    Xgl_facet_list *facets;
    double start;
    int first, count;
    first = xb.ui_quad_first;
    count = xb.ui_quad_count - first;
    if (!count) return;
    native_check();
    facets = &xb.ui_facets[first];
    facets->facet_type = XGL_FACET_COLOR;
    facets->num_facets = (Xgl_usgn32)count;
    facets->facets.color_facets = &xb.ui_colors[first];
    xb.stage = "UI quad submission";
    start = XGL_ProfileNow();
    xgl_multi_simple_polygon(xb.ui,
        XGL_FACET_FLAG_SIDES_ARE_4 | XGL_FACET_FLAG_SHAPE_CONVEX,
        facets, NULL, (Xgl_usgn32)count, &xb.ui_lists[first]);
    XGL_ProfileAdd(XGL_PROFILE_SUBMIT, start);
    XGL_ProfileCount(XGL_PROFILE_POLYGON_CALLS, 1);
    native_check();
    xb.ui_quad_first = xb.ui_quad_count;
    xb.ui_pending = 1;
    XGL_AudioUpdate();
}

void XGL_UIFlush(void)
{
    double start;
    submit_ui_quads();
    xb.stage = "UI completion";
    if (xb.ui && xb.ui_pending && !xb.fatal) {
        XGL_AudioUpdate();
        start = XGL_ProfileNow();
        xgl_context_flush(xb.ui, XGL_FLUSH_SYNCHRONIZE);
        XGL_ProfileAdd(XGL_PROFILE_SYNC, start);
        XGL_ProfileCount(XGL_PROFILE_WAIT_CALLS, 1);
        native_check();
        xb.ui_pending = 0;
        XGL_AudioUpdate();
    }
    native_check();
    xb.ui_glyph_count = xb.ui_quad_count = xb.ui_quad_first = 0;
    xb.ui_text_used = 0;
}

static void set_transparency(float opacity, int zwrite)
{
    int blended;
    blended = opacity < .9999f;
    xb.stage = blended ? "blended surface state" : "opaque surface state";
    xgl_object_set(xb.scene,
        XGL_3D_CTX_SURF_TRANSP_METHOD, blended ? XGL_TRANSP_BLENDED : XGL_TRANSP_NONE,
        XGL_3D_CTX_SURF_TRANSP_BLEND_EQ, blended ? XGL_BLEND_ARBITRARY_BG : XGL_BLEND_NONE,
        XGL_3D_CTX_SURF_FRONT_TRANSP, blended ? 1.0 - (double)opacity : 0.0,
        XGL_3D_CTX_SURF_BACK_TRANSP, blended ? 1.0 - (double)opacity : 0.0,
        XGL_3D_CTX_BLEND_FREEZE_Z_BUFFER, blended && !zwrite ? TRUE : FALSE,
        XGL_3D_CTX_BLEND_DRAW_MODE, XGL_BLEND_DRAW_ALL,
        XGL_3D_CTX_Z_BUFFER_WRITE_MASK, (Xgl_usgn32)(zwrite ? 0xffffffffUL : 0), 0);
    native_check();
}

static void apply_draw_state(void)
{
    Xgl_color ambient;
    float amount;
    if (xb.applied_lit != xb.lit) {
        xb.stage = "surface illumination";
        xgl_object_set(xb.scene,
            XGL_CTX_SURF_FRONT_COLOR_SELECTOR, XGL_SURF_COLOR_VERTEX_ILLUM_DEP,
            XGL_3D_CTX_SURF_FRONT_ILLUMINATION,
                xb.lit ? XGL_ILLUM_PER_VERTEX : XGL_ILLUM_NONE_INTERP_COLOR, 0);
        native_check();
        amount = bound_color(xgl_ambient.value) * (xb.lit == 2 ? .5f : 1.0f);
        memset(&ambient, 0, sizeof(ambient));
        ambient.rgb.r = ambient.rgb.g = ambient.rgb.b = amount;
        xb.stage = "ambient light";
        xgl_object_set(xb.lights[0], XGL_LIGHT_COLOR, &ambient, 0);
        native_check();
        xb.light_switches[5] = xb.lit == 2 ? TRUE : FALSE;
        xb.stage = "light switches";
        xgl_object_set(xb.scene, XGL_3D_CTX_LIGHT_SWITCHES, xb.light_switches, 0);
        native_check();
        xb.applied_lit = xb.lit;
    }
    if (xb.applied_opacity != xb.opacity || xb.applied_zwrite != xb.zwrite) {
        set_transparency(xb.opacity, xb.zwrite);
        xb.applied_opacity = xb.opacity;
        xb.applied_zwrite = xb.zwrite;
    }
    native_check();
}

void XGL_Flush(void)
{
    double start;
    int triangles;
    if (!xb.count) return;
    apply_draw_state();
    triangles = xb.count / 3;
    xb.stage = "triangle submission";
    start = XGL_ProfileNow();
    xgl_multi_simple_polygon(xb.scene,
        XGL_FACET_FLAG_SIDES_ARE_3 | XGL_FACET_FLAG_SHAPE_CONVEX,
        NULL, NULL, (Xgl_usgn32)triangles, xb.lists);
    XGL_ProfileAdd(XGL_PROFILE_SUBMIT, start);
    XGL_ProfileCount(XGL_PROFILE_TRIANGLES, (unsigned long)triangles);
    XGL_ProfileCount(XGL_PROFILE_POLYGON_CALLS, 1);
    /* GEOM_DATA_IS_VOLATILE copies inputs at submission; batching storage
     * may be reused while the accelerator completes earlier work. */
    xb.count = 0;
    xb.scene_pending = 1;
    native_check();
    XGL_AudioUpdate();
}

static int triangle_state(int count, int lit, float opacity, int zwrite)
{
    if (!xb.frame || !xb.scene_active || count <= 0) return 0;
    if (count % 3) Sys_Error("XGL_DrawTriangles: vertex count is not a multiple of three");
    opacity = bound_color(opacity);
    if (opacity <= 0) return 0;
    lit = lit < 0 ? 0 : (lit > 2 ? 2 : lit);
    zwrite = zwrite != 0;
    if (xb.count && (xb.count == XGL_BATCH_TRIANGLES * 3 ||
        xb.lit != lit || xb.opacity != opacity || xb.zwrite != zwrite)) XGL_Flush();
    xb.lit = lit; xb.opacity = opacity; xb.zwrite = zwrite;
    return 1;
}

xgl_native_vertex_t *XGL_ReserveTriangles(int *count, int lit,
                                         float opacity, int zwrite)
{
    xgl_native_vertex_t *vertices;
    int available, i;
    if (!count) return NULL;
    if (!triangle_state(*count, lit, opacity, zwrite)) {
        *count = 0;
        return NULL;
    }
    available = XGL_BATCH_TRIANGLES * 3 - xb.count;
    if (*count > available) *count = available;
    vertices = xb.points + xb.count;
    for (i = 0; i < *count; i += 3)
        xb.lists[(xb.count + i) / 3].pts.color_normal_f3d =
            (Xgl_pt_color_normal_f3d *)(vertices + i);
    xb.count += *count;
    XGL_ProfileCount(XGL_PROFILE_NATIVE_DIRECT_VERTICES, (unsigned long)*count);
    /* The caller fills the reserved records before another backend call.
     * In particular, do not submit a full batch until it has been filled. */
    return vertices;
}

void XGL_DrawNativeTriangles(const xgl_native_vertex_t *vertices, int count,
                            int lit, float opacity, int zwrite)
{
    int available, take, i;
    if (!vertices || !triangle_state(count, lit, opacity, zwrite)) return;
    while (count) {
        available = XGL_BATCH_TRIANGLES * 3 - xb.count;
        take = count < available ? count : available;
        for (i = 0; i < take; i += 3)
            xb.lists[(xb.count + i) / 3].pts.color_normal_f3d =
                (Xgl_pt_color_normal_f3d *)(vertices + i);
        xb.count += take;
        XGL_ProfileCount(XGL_PROFILE_NATIVE_CACHED_VERTICES, (unsigned long)take);
        vertices += take;
        count -= take;
        if (xb.count == XGL_BATCH_TRIANGLES * 3) XGL_Flush();
    }
}

void XGL_DrawTrianglesStrided(const xgl_vertex_t *vertices, int count,
                             size_t stride, int lit, float opacity,
                             int zwrite, int prepared_color)
{
    xgl_native_vertex_t *out;
    const byte *source;
    const xgl_vertex_t *vertex;
    double packing_start;
    int i, available, take;
    if (!vertices || !triangle_state(count, lit, opacity, zwrite)) return;
    if (stride < sizeof(*vertices)) Sys_Error("XGL_DrawTriangles: invalid vertex stride");
    source = (const byte *)vertices;
    while (count) {
        available = XGL_BATCH_TRIANGLES * 3 - xb.count;
        take = count < available ? count : available;
        packing_start = XGL_ProfileDetailNow();
        for (i = 0; i < take; i += 3)
            xb.lists[(xb.count + i) / 3].pts.color_normal_f3d =
                (Xgl_pt_color_normal_f3d *)(xb.points + xb.count + i);
        for (i = 0; i < take; ++i) {
            vertex = (const xgl_vertex_t *)source;
            out = &xb.points[xb.count + i];
            out->position[0] = vertex->position[0];
            out->position[1] = vertex->position[1];
            out->position[2] = vertex->position[2];
            out->normal[0] = vertex->normal[0];
            out->normal[1] = vertex->normal[1];
            out->normal[2] = vertex->normal[2];
            if (prepared_color) {
                out->color.rgb[0] = vertex->color[0];
                out->color.rgb[1] = vertex->color[1];
                out->color.rgb[2] = vertex->color[2];
            } else {
                out->color.rgb[0] = gamma_color(vertex->color[0]);
                out->color.rgb[1] = gamma_color(vertex->color[1]);
                out->color.rgb[2] = gamma_color(vertex->color[2]);
            }
            source += stride;
        }
        XGL_ProfileAdd(XGL_PROFILE_VERTEX_PACK, packing_start);
        XGL_ProfileCount(XGL_PROFILE_VERTEX_PACKED, (unsigned long)take);
        xb.count += take;
        count -= take;
        if (xb.count == XGL_BATCH_TRIANGLES * 3) XGL_Flush();
    }
}

void XGL_DrawTriangles(const xgl_vertex_t *vertices, int count, int lit,
                       float opacity, int zwrite)
{
    XGL_DrawTrianglesStrided(vertices, count, sizeof(*vertices), lit, opacity, zwrite, 0);
}

static void update_lights(void)
{
    int chosen[XGL_POINT_LIGHTS], i, j, insert, limit;
    double distances[XGL_POINT_LIGHTS], distance, dx, dy, dz;
    Xgl_pt_d3d position;
    Xgl_color color;
    for (i = 0; i < XGL_POINT_LIGHTS; ++i) { chosen[i] = -1; distances[i] = 1e30; }
    limit = (int)xgl_maxlights.value;
    if (limit < 0) limit = 0;
    if (limit > XGL_POINT_LIGHTS) limit = XGL_POINT_LIGHTS;
    if (!r_dynamic.value) limit = 0;
    for (i = 0; i < MAX_DLIGHTS; ++i) {
        if (cl_dlights[i].die < cl.time || cl_dlights[i].radius <= 0) continue;
        dx = cl_dlights[i].origin[0] - r_origin[0];
        dy = cl_dlights[i].origin[1] - r_origin[1];
        dz = cl_dlights[i].origin[2] - r_origin[2];
        distance = dx*dx + dy*dy + dz*dz;
        for (insert = 0; insert < limit && distance >= distances[insert]; ++insert) {}
        if (insert == limit) continue;
        for (j = limit - 1; j > insert; --j) { distances[j] = distances[j-1]; chosen[j] = chosen[j-1]; }
        distances[insert] = distance; chosen[insert] = i;
    }
    memset(&color, 0, sizeof(color));
    color.rgb.r = color.rgb.g = color.rgb.b = 1;
    xb.light_switches[0] = TRUE;
    xb.light_switches[5] = FALSE;
    for (i = 0; i < XGL_POINT_LIGHTS; ++i) {
        xb.light_switches[i+1] = chosen[i] >= 0 ? TRUE : FALSE;
        if (chosen[i] < 0) continue;
        j = chosen[i];
        position.x = cl_dlights[j].origin[0];
        position.y = cl_dlights[j].origin[1];
        position.z = cl_dlights[j].origin[2];
        xb.stage = "point light";
        xgl_object_set(xb.lights[i+1], XGL_LIGHT_TYPE, XGL_LIGHT_POSITIONAL,
            XGL_LIGHT_COLOR, &color, XGL_LIGHT_POSITION, &position,
            XGL_LIGHT_ATTENUATION_1, 1.0,
            XGL_LIGHT_ATTENUATION_2, 1.0 / (double)cl_dlights[j].radius, 0);
        native_check();
    }
    xb.stage = "light switches";
    xgl_object_set(xb.scene, XGL_3D_CTX_LIGHT_SWITCHES, xb.light_switches, 0);
    native_check();
}

void XGL_BeginFrame(void)
{
    if (!xb.system) return;
    XGL_ProfileBeginFrame();
    XGL_AudioUpdate();
    update_gamma();
    XGL_Flush(); XGL_UIFlush(); complete_scene();
    xb.frame = 1; xb.scene_active = 0; xb.ui_active = 0; xb.overlay_3d = 0;
    xb.console_lines = 0;
    set_transparency(1, 1);
    xb.stage = "frame clear state";
    xgl_object_set(xb.scene,
        XGL_CTX_VDC_MAP, XGL_VDC_MAP_ALL,
        XGL_CTX_CLIP_PLANES, 0,
        XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_Z_BUFFER,
        XGL_CTX_NEW_FRAME_ACTION, XGL_CTX_NEW_FRAME_CLEAR | XGL_CTX_NEW_FRAME_HLHSR_ACTION, 0);
    native_check();
    xb.stage = "frame clear";
    xgl_context_new_frame(xb.scene);
    xb.scene_pending = 1;
    native_check();
}

void XGL_SetConsoleClip(int lines)
{
    if (lines < 0) lines = 0;
    if ((unsigned int)lines > vid.height) lines = (int)vid.height;
    xb.console_lines = lines;
}

void XGL_BeginScene(void)
{
    Xgl_matrix_f3d matrix;
    Xgl_bounds_d3d bounds, window;
    double sx, sy, left, top, span_x, span_y, console_edge;
    double near_clip, far_clip, a, offset, tx, ty;
    int i;
    if (!xb.frame) return;
    XGL_UIFlush(); XGL_Flush();
    xb.scene_active = xb.ui_active = xb.overlay_3d = 0;
    memset(&bounds, 0, sizeof(bounds));
    bounds.xmin = bounds.ymin = -1; bounds.xmax = bounds.ymax = 1; bounds.zmax = 1;
    sx = (double)xb.width / vid.width; sy = (double)xb.height / vid.height;
    left = r_refdef.vrect.x * sx; top = r_refdef.vrect.y * sy;
    span_x = r_refdef.vrect.width * sx - 1;
    span_y = r_refdef.vrect.height * sy - 1;
    if (span_x <= 0 || span_y <= 0) Sys_Error("XGL: invalid view rectangle");
    /* Use the benchmark's automatic full-raster mapping. Express Quake's
     * smaller view rectangle through the VDC window instead of setting
     * DC_VIEWPORT explicitly. Clipping and the camera stay in [-1,1], so
     * geometry cannot spill into the status bar or view-size borders. */
    window = bounds;
    window.xmin = -1 - 2 * left / span_x;
    window.xmax = window.xmin + 2 * (xb.width - 1) / span_x;
    window.ymin = -1 - 2 * top / span_y;
    window.ymax = window.ymin + 2 * (xb.height - 1) / span_y;
    /* Keep the complete view's camera and mapping. Only move the existing
     * upper clip plane, leaving one device pixel under the opaque panel so
     * raster edge rounding cannot expose a seam. Empty bounds are invalid:
     * a completely covered view instead leaves 3D submission disabled. */
    console_edge = xb.console_lines * sy - 1;
    if (console_edge >= top + span_y) return;
    if (console_edge > top)
        bounds.ymin = -1 + 2 * (console_edge - top) / span_y;
    xb.stage = "scene mapping";
    xgl_object_set(xb.scene,
        XGL_CTX_VDC_MAP, XGL_VDC_MAP_ALL,
        XGL_CTX_VDC_ORIENTATION, XGL_Y_DOWN_Z_AWAY,
        XGL_CTX_VDC_WINDOW, &window, XGL_CTX_VIEW_CLIP_BOUNDS, &bounds,
        XGL_CTX_CLIP_PLANES, XGL_CLIP_XMIN | XGL_CLIP_XMAX | XGL_CLIP_YMIN |
            XGL_CLIP_YMAX | XGL_CLIP_ZMIN | XGL_CLIP_ZMAX,
        XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_Z_BUFFER, 0);
    native_check();
    near_clip = 4; far_clip = 4096;
    a = far_clip / (far_clip - near_clip);
    offset = -near_clip * a;
    tx = 1.0 / tan(r_refdef.fov_x * M_PI / 360.0);
    ty = 1.0 / tan(r_refdef.fov_y * M_PI / 360.0);
    memset(matrix, 0, sizeof(matrix));
    for (i = 0; i < 3; ++i) {
        matrix[i][0] = (float)(vright[i] * tx);
        matrix[i][1] = (float)(-vup[i] * ty);
        matrix[i][2] = (float)(vpn[i] * a);
        matrix[i][3] = vpn[i];
        matrix[3][0] -= r_origin[i] * matrix[i][0];
        matrix[3][1] -= r_origin[i] * matrix[i][1];
        matrix[3][2] -= r_origin[i] * matrix[i][2];
        matrix[3][3] -= r_origin[i] * matrix[i][3];
    }
    matrix[3][2] += (float)offset;
    identity_transform(xb.model);
    xb.stage = "camera transform";
    xgl_transform_write(xb.view, matrix);
    native_check();
    update_lights();
    xb.applied_lit = xb.applied_zwrite = -1; xb.applied_opacity = -1;
    xb.scene_active = 1; xb.ui_active = 0; xb.overlay_3d = 0;
    native_check();
}

void XGL_SetViewModel(int enabled)
{
    double start;
    if (!xb.frame || !xb.scene_active) return;
    XGL_Flush(); complete_scene();
    if (enabled) {
        /* Weapon is the last 3D layer; clear depth only, retaining the world
         * color. No extra world render or texture/lightmap pass is needed. */
        if (xb.applied_opacity != 1 || xb.applied_zwrite != 1) {
            set_transparency(1, 1);
            xb.applied_opacity = 1; xb.applied_zwrite = 1;
        }
        xb.stage = "weapon depth clear state";
        xgl_object_set(xb.scene, XGL_CTX_NEW_FRAME_ACTION, XGL_CTX_NEW_FRAME_HLHSR_ACTION, 0);
        native_check();
        xb.stage = "weapon depth clear";
        start = XGL_ProfileDetailNow();
        xgl_context_new_frame(xb.scene);
        XGL_ProfileAdd(XGL_PROFILE_WEAPON_CLEAR, start);
        xb.scene_pending = 1;
        native_check();
    }
}

void XGL_EndScene(void)
{
    XGL_Flush(); complete_scene();
    xb.scene_active = 0;
}

static void begin_ui(void)
{
    if (!xb.frame) return;
    if (xb.scene_active) XGL_EndScene();
    if (!xb.ui_active) { complete_scene(); xb.ui_active = 1; }
}

static void text_spacing(float spacing)
{
    if (xb.current_text_spacing == spacing) return;
    xb.stage = "UI text spacing";
    xgl_object_set(xb.ui, XGL_CTX_STEXT_CHAR_SPACING, (double)spacing, 0);
    native_check();
    xb.current_text_spacing = spacing;
}

static void font_run_spacing(xgl_font_size_t *font, double cell)
{
    Xgl_bounds_f2d pair, wider, fitted;
    Xgl_pt_f2d advance;
    const Xgl_bounds_f2d *capital;
    double unit, spacing;
    char sample[3];
    /* XGL implementations can report spacing in font-dependent units.
     * Calibrate from measured ink separation, not an assumed em size, and
     * verify the result before using it. Single glyphs remain the fallback.
     * This does not assume that Roman_M has fixed-width characters. */
    sample[0] = sample[1] = 'H'; sample[2] = 0;
    capital = &font->bounds['H' - 32];
    xb.stage = "font run measurement";
    xgl_stroke_text_extent(xb.ui, sample, &pair, &advance);
    native_check();
    if (fabs(pair.xmin - capital->xmin) > .002 ||
        fabs(pair.xmax - capital->xmax - font->advance['H' - 32].x) > .002)
        return;
    text_spacing(1);
    xb.stage = "font spacing measurement";
    xgl_stroke_text_extent(xb.ui, sample, &wider, &advance);
    native_check();
    unit = wider.xmax - pair.xmax;
    if (unit > 0 && unit < 10000 &&
        fabs(wider.xmin - pair.xmin) <= .002 &&
        fabs(wider.ymin - pair.ymin) <= .002 &&
        fabs(wider.ymax - pair.ymax) <= .002) {
        spacing = (cell - font->advance['H' - 32].x) / unit;
        /* A fitted glyph normally needs modest positive spacing. Avoid
         * relying on negative spacing or unusually large native values. */
        if (spacing >= 0 && spacing <= 16) {
            text_spacing((float)spacing);
            xb.stage = "font fitted spacing measurement";
            xgl_stroke_text_extent(xb.ui, sample, &fitted, &advance);
            native_check();
            if (fabs(fitted.xmax - capital->xmax - cell) <= .002 &&
                fabs(fitted.xmin - pair.xmin) <= .002 &&
                fabs(fitted.ymin - pair.ymin) <= .002 &&
                fabs(fitted.ymax - pair.ymax) <= .002) {
                font->spacing = (float)spacing;
                font->extra_advance = fitted.xmax - pair.xmax;
                font->batchable = 1;
            }
        }
    }
    text_spacing(font->spacing);
}

static xgl_font_size_t *font_size(float cell, float height)
{
    xgl_font_size_t *font;
    Xgl_pt_f2d advance;
    double usage, ratio, requested, sx, sy;
    float width, tall, ascent, descent;
    int i, attempt, slot;
    for (slot = 0; slot < XGL_FONT_CACHE; ++slot)
        if (xb.fonts[slot].valid && xb.fonts[slot].cell == cell && xb.fonts[slot].height == height)
            return &xb.fonts[slot];
    slot = xb.font_next++ % XGL_FONT_CACHE;
    font = &xb.fonts[slot];
    memset(font, 0, sizeof(*font));
    font->cell = cell; font->height = height;
    sx = (double)xb.width / vid.width; sy = (double)xb.height / vid.height;
    requested = height * sy * .85;
    text_spacing(0);
    for (attempt = 0; attempt < 6; ++attempt) {
        usage = 1;
        ascent = descent = 0;
        xb.stage = "font metrics height";
        xgl_object_set(xb.ui, XGL_CTX_STEXT_CHAR_HEIGHT, requested, 0);
        native_check();
        xb.stage = "font measurement";
        for (i = 1; i < 95; ++i) {
            xgl_stroke_text_extent(xb.ui, xb.glyphs[i], &font->bounds[i], &advance);
            native_check();
            font->advance[i] = advance;
            width = font->bounds[i].xmax - font->bounds[i].xmin;
            tall = font->bounds[i].ymax - font->bounds[i].ymin;
            /* '-'/'_' and '|' may have zero ink height/width. Their
             * advancing text cells are valid even for degenerate bounds. */
            if (!(width >= 0 && width < 10000 && tall >= 0 && tall < 10000 &&
                  advance.x > 0 && advance.x < 10000))
                Sys_Error("XGL stroke font has invalid bounds for character %c", i + 32);
            if (font->bounds[i].ymax > ascent) ascent = font->bounds[i].ymax;
            if (font->bounds[i].ymin < descent) descent = font->bounds[i].ymin;
            ratio = width / (cell * sx * .95);
            if (ratio > usage) usage = ratio;
        }
        ratio = (ascent - descent) / (height * sy * .95);
        if (ratio > usage) usage = ratio;
        if (!(font->bounds['H'-32].xmax > font->bounds['H'-32].xmin &&
              font->bounds['H'-32].ymax > font->bounds['H'-32].ymin))
            Sys_Error("XGL stroke font returned empty capital-letter bounds");
        if (usage <= 1) break;
        requested *= .96 / usage;
        if (requested < .2 || attempt == 5) Sys_Error("XGL stroke font does not fit %.1fx%.1f UI cells", cell, height);
    }
    font->requested = (float)requested;
    font->ascent = ascent;
    font_run_spacing(font, cell * sx);
    font->valid = 1;
    xb.current_text_height = font->requested;
    return font;
}

void XGL_UIText(float x, float y, const char *text, int color, float cell_width, float height)
{
    xgl_font_size_t *font;
    Xgl_pt_f2d *position;
    Xgl_bounds_f2d *bounds;
    Xgl_color rgb;
    char run[XGL_UI_TEXT_RUN + 1], *saved;
    double start, next_origin, desired_origin;
    float sx, sy, width, origin_x, origin_y, next_x;
    int character, next_character, previous, length;
    if (!xb.frame || !text || cell_width <= 0 || height <= 0) return;
    native_check();
    begin_ui();
    /* Submit rectangles before subsequent glyphs, retaining painter order
     * without synchronizing two primitives on the same UI context. */
    submit_ui_quads();
    font = font_size(cell_width, height);
    xb.stage = "UI text attributes";
    if (xb.current_text_height != font->requested) {
        xgl_object_set(xb.ui, XGL_CTX_STEXT_CHAR_HEIGHT, (double)font->requested, 0);
        native_check();
        xb.current_text_height = font->requested;
    }
    text_spacing(font->spacing);
    palette_color(color, &rgb);
    if (!xb.text_color_valid || memcmp(&xb.text_color, &rgb, sizeof(rgb))) {
        xgl_object_set(xb.ui, XGL_CTX_STEXT_COLOR, &rgb, 0);
        xb.text_color = rgb;
        xb.text_color_valid = 1;
    }
    native_check();
    sx = (float)xb.width / vid.width; sy = (float)xb.height / vid.height;
    while (*text) {
        character = ((unsigned char)*text) & 127;
        if (character < 32 || character > 126) character = '?';
        if (character != ' ' && x + cell_width > 0 && x < vid.width && y + height > 0 && y < vid.height) {
            character -= 32;
            bounds = &font->bounds[character];
            width = bounds->xmax - bounds->xmin;
            origin_x = x * sx + (cell_width*sx - width)*.5f - bounds->xmin;
            origin_y = y * sy + font->ascent;
            run[0] = (char)(character + 32); length = 1;
            previous = character; next_origin = origin_x;
            while (font->batchable && length < XGL_UI_TEXT_RUN && text[length]) {
                next_character = ((unsigned char)text[length]) & 127;
                if (next_character < 32 || next_character > 126) next_character = '?';
                next_x = x + length * cell_width;
                /* Spaces and clipped cells split runs; their width never
                 * depends on the native font's space or missing glyph. */
                if (next_character == ' ' || next_x + cell_width <= 0 || next_x >= vid.width)
                    break;
                next_character -= 32;
                bounds = &font->bounds[next_character];
                desired_origin = next_x * sx +
                    (cell_width*sx - (bounds->xmax - bounds->xmin))*.5f - bounds->xmin;
                next_origin += font->advance[previous].x + font->extra_advance;
                if (!(fabs(next_origin - desired_origin) <= .002 &&
                      fabs(font->advance[previous].y) <= .00001)) break;
                run[length++] = (char)(next_character + 32);
                previous = next_character;
            }
            run[length] = 0;
            if (xb.ui_glyph_count == XGL_UI_GLYPHS ||
                xb.ui_text_used + length + 1 > XGL_UI_TEXT_BYTES) XGL_UIFlush();
            position = &xb.ui_positions[xb.ui_glyph_count++];
            position->x = origin_x; position->y = origin_y;
            saved = xb.ui_text + xb.ui_text_used;
            memcpy(saved, run, (size_t)length + 1);
            xb.ui_text_used += length + 1;
            xb.stage = "UI text run submission";
            start = XGL_ProfileNow();
            xgl_stroke_text(xb.ui, saved, position, NULL);
            XGL_ProfileAdd(XGL_PROFILE_SUBMIT, start);
            XGL_ProfileCount(XGL_PROFILE_GLYPHS, (unsigned long)length);
            XGL_ProfileCount(XGL_PROFILE_TEXT_CALLS, 1);
            xb.ui_pending = 1;
            native_check();
        } else length = 1;
        text += length;
        x += length * cell_width;
    }
    native_check();
    XGL_AudioUpdate();
}

void XGL_UIFillRGB(float x, float y, float width, float height, const float rgb[3], float opacity)
{
    Xgl_pt_f2d points2[4];
    Xgl_pt_f3d points3[4];
    Xgl_pt_list list;
    Xgl_facet_list facets;
    Xgl_color_facet color;
    double start;
    float sx, sy;
    int i, index;
    if (!xb.frame || width <= 0 || height <= 0 || opacity <= 0) return;
    begin_ui();
    sx = (float)xb.width / vid.width; sy = (float)xb.height / vid.height;
    if (x < 0) { width += x; x = 0; }
    if (y < 0) { height += y; y = 0; }
    if (x + width > vid.width) width = vid.width - x;
    if (y + height > vid.height) height = vid.height - y;
    if (width <= 0 || height <= 0) return;
    points2[0].x = x*sx; points2[0].y = y*sy;
    points2[1].x = (x+width)*sx; points2[1].y = y*sy;
    points2[2].x = (x+width)*sx; points2[2].y = (y+height)*sy;
    points2[3].x = x*sx; points2[3].y = (y+height)*sy;
    memset(&list, 0, sizeof(list)); memset(&facets, 0, sizeof(facets));
    memset(&color, 0, sizeof(color));
    color.color.rgb.r = gamma_color(rgb[0]);
    color.color.rgb.g = gamma_color(rgb[1]);
    color.color.rgb.b = gamma_color(rgb[2]);
    facets.facet_type = XGL_FACET_COLOR; facets.num_facets = 1;
    facets.facets.color_facets = &color; list.num_pts = 4;
    if (opacity >= .9999f) {
        if (xb.ui_quad_count == XGL_UI_QUADS) XGL_UIFlush();
        index = xb.ui_quad_count++;
        memcpy(xb.ui_points[index], points2, sizeof(points2));
        list.pt_type = XGL_PT_F2D; list.pts.f2d = xb.ui_points[index];
        xb.ui_lists[index] = list; xb.ui_colors[index] = color;
        /* Consecutive opaque rectangles share one native polygon call.
         * Persistent arrays remain alive until the UI completion point. */
    } else {
        /* Blend through the 3D context using device coordinates and no Z;
         * opaque UI text/geometry continues on the shared 2D draw buffer. */
        XGL_UIFlush();
        if (!xb.overlay_3d) {
            identity_transform(xb.model); identity_transform(xb.view);
            xb.stage = "overlay mapping";
            xgl_object_set(xb.scene,
                XGL_CTX_VDC_MAP, XGL_VDC_MAP_DEVICE,
                XGL_CTX_VDC_ORIENTATION, XGL_Y_DOWN_Z_AWAY,
                XGL_CTX_CLIP_PLANES, 0,
                XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_NONE,
                XGL_CTX_SURF_FRONT_COLOR_SELECTOR, XGL_SURF_COLOR_FACET,
                XGL_3D_CTX_SURF_FRONT_ILLUMINATION, XGL_ILLUM_NONE, 0);
            native_check();
            xb.overlay_3d = 1;
        }
        set_transparency(bound_color(opacity), 0);
        for (i = 0; i < 4; ++i) { points3[i].x = points2[i].x; points3[i].y = points2[i].y; points3[i].z = 0; }
        list.pt_type = XGL_PT_F3D; list.pts.f3d = points3;
        xb.stage = "overlay submission";
        start = XGL_ProfileNow();
        xgl_multi_simple_polygon(xb.scene, XGL_FACET_FLAG_SIDES_ARE_4 | XGL_FACET_FLAG_SHAPE_CONVEX,
                                &facets, NULL, 1, &list);
        XGL_ProfileAdd(XGL_PROFILE_SUBMIT, start);
        XGL_ProfileCount(XGL_PROFILE_POLYGON_CALLS, 1);
        xb.scene_pending = 1;
        native_check();
        complete_scene();
    }
    native_check();
}

void XGL_UIFill(float x, float y, float width, float height, int index, float opacity)
{
    float rgb[3];
    index &= 255;
    rgb[0] = xgl_palette[index*3] / 255.0f;
    rgb[1] = xgl_palette[index*3+1] / 255.0f;
    rgb[2] = xgl_palette[index*3+2] / 255.0f;
    XGL_UIFillRGB(x, y, width, height, rgb, opacity);
}

void XGL_EndFrame(void)
{
    double start;
    if (!xb.frame) return;
    XGL_Flush(); XGL_UIFlush(); complete_scene();
    xb.stage = "buffer switch state";
    xgl_object_set(xb.scene, XGL_CTX_NEW_FRAME_ACTION, XGL_CTX_NEW_FRAME_SWITCH_BUFFER, 0);
    native_check();
    xb.stage = "buffer switch";
    start = XGL_ProfileNow();
    xgl_context_new_frame(xb.scene);
    XGL_ProfileAdd(XGL_PROFILE_SWAP, start);
    xb.scene_pending = 1;
    native_check();
    complete_scene();
    xb.frame = xb.scene_active = xb.ui_active = 0;
    XGL_ProfileEndFrame();
}

int XGL_ReadScreen(byte *rgb, int width, int height)
{
    Xgl_pt_i2d point;
    Xgl_color color;
    int x, y, obscured;
    if (!xb.system || !rgb || width <= 0 || height <= 0 || xb.errors) return 0;
    XGL_Flush(); XGL_UIFlush(); complete_scene();
    xb.stage = "screenshot source";
    xgl_object_set(xb.raster, XGL_RAS_SOURCE_BUFFER,
                   xb.frame ? XGL_BUFFER_SEL_DRAW : XGL_BUFFER_SEL_DISPLAY, 0);
    native_check();
    xb.stage = "screenshot readback";
    for (y = 0; y < height; ++y) {
        point.y = (int)((double)y * xb.height / height);
        for (x = 0; x < width; ++x) {
            point.x = (int)((double)x * xb.width / width);
            obscured = xgl_context_get_pixel(xb.scene, &point, &color);
            if (obscured || xb.errors) {
                if (!xb.fatal) xgl_object_set(xb.raster, XGL_RAS_SOURCE_BUFFER, XGL_BUFFER_SEL_DRAW, 0);
                Con_Printf("XGL screenshot readback failed or was obscured\n");
                return 0;
            }
            *rgb++ = (byte)(bound_color(color.rgb.r) * 255 + .5f);
            *rgb++ = (byte)(bound_color(color.rgb.g) * 255 + .5f);
            *rgb++ = (byte)(bound_color(color.rgb.b) * 255 + .5f);
        }
    }
    xb.stage = "screenshot source restore";
    xgl_object_set(xb.raster, XGL_RAS_SOURCE_BUFFER, XGL_BUFFER_SEL_DRAW, 0);
    return !xb.errors;
}

void XGLB_Resize(int width, int height)
{
    if (!xb.system || width < 1 || height < 1) return;
    XGL_Flush(); XGL_UIFlush(); complete_scene();
    xb.width = width; xb.height = height;
    xb.stage = "window resize";
    xgl_window_raster_resize(xb.raster);
    memset(xb.fonts, 0, sizeof(xb.fonts));
    xb.current_text_height = xb.current_text_spacing = -1;
    native_check();
}

void XGLB_Shutdown(void)
{
    if (xb.system) {
        /* xgl_close destroys child objects, including queued volatile work. */
        xgl_close(xb.system);
    }
    memset(&xb, 0, sizeof(xb));
}

void XGLB_Init(Display *display, Window window, int width, int height)
{
    Xgl_X_window native_window;
    Xgl_obj_desc descriptor;
    Xgl_color color;
    Xgl_pt_f2d up;
    Xgl_pt_f3d direction;
    Xgl_pt_d3d maximum;
    Xgl_hlhsr_data clear;
    Xgl_sfont font;
    Xgl_inquire *inquiry;
    char *name;
    int i;
    if (sizeof(Xgl_usgn32) != 4 || sizeof(void *) != 4 || sizeof(float) != 4)
        Sys_Error("XGLQuake requires the 32-bit Sun XGL ABI");
    memset(&xb, 0, sizeof(xb));
    xb.width = width; xb.height = height;
    xb.current_text_height = xb.current_text_spacing = -1;
    xb.stage = "native initialization";
    Cvar_RegisterVariable(&xgl_ambient); Cvar_RegisterVariable(&xgl_maxlights);
    xb.system = xgl_open((Xgl_attribute)NULL);
    if (!xb.system) Sys_Error("xgl_open failed");
    xgl_object_set(xb.system, XGL_SYS_ST_ERROR_NOTIFICATION_FUNCTION, native_error,
                   XGL_SYS_ST_ERROR_DETECTION, TRUE, 0);
    memset(&native_window, 0, sizeof(native_window));
    native_window.X_display = display; native_window.X_screen = DefaultScreen(display);
    native_window.X_window = window;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.win_ras.type = XGL_WIN_X; descriptor.win_ras.desc = &native_window;
    inquiry = xgl_inquire(xb.system, &descriptor);
    native_check();
    if (!inquiry) Sys_Error("XGL could not inquire the window visual");
    Con_SafePrintf("XGL device: %s; RGB=%d Z=%d shading=%d lighting=%d transparency=%d\n",
        inquiry->name ? inquiry->name : "unknown", inquiry->true_color, inquiry->hlhsr,
        inquiry->shading, inquiry->lighting, inquiry->transparency);
    if (!inquiry->dga_flag || inquiry->true_color != XGL_INQ_HARDWARE ||
        inquiry->hlhsr != XGL_INQ_HARDWARE || inquiry->shading != XGL_INQ_HARDWARE ||
        inquiry->maximum_buffer < 2) {
        free(inquiry);
        Sys_Error("XGLQuake needs local DGA hardware RGB/Z/shading and two buffers");
    }
    free(inquiry);
    descriptor.win_ras.type = XGL_WIN_X | XGL_WIN_X_PROTO_DGA;
    xb.raster = xgl_object_create(xb.system, XGL_WIN_RAS, &descriptor,
        XGL_DEV_COLOR_TYPE, XGL_COLOR_RGB, XGL_WIN_RAS_BACKING_STORE, FALSE,
        XGL_WIN_RAS_BUFFERS_REQUESTED, 2, 0);
    native_check();
    if (!xb.raster) Sys_Error("XGL window raster creation failed");
    xgl_object_get(xb.raster, XGL_WIN_RAS_BUFFERS_ALLOCATED, &xb.buffers);
    native_check();
    if (xb.buffers < 2) Sys_Error("XGL did not allocate two window buffers");
    xgl_object_set(xb.raster, XGL_WIN_RAS_BUF_DISPLAY, 0, XGL_WIN_RAS_BUF_DRAW, 1,
                   XGL_RAS_SOURCE_BUFFER, XGL_BUFFER_SEL_DRAW, 0);
    memset(&color, 0, sizeof(color));
    xb.scene = xgl_object_create(xb.system, XGL_3D_CTX, NULL,
        XGL_CTX_DEVICE, xb.raster, XGL_CTX_RENDER_BUFFER, XGL_RENDER_DRAW_BUFFER,
        XGL_CTX_BACKGROUND_COLOR, &color, XGL_CTX_DEFERRAL_MODE, XGL_DEFER_ASTI,
        XGL_CTX_GEOM_DATA_IS_VOLATILE, TRUE,
        XGL_CTX_SURF_FRONT_FILL_STYLE, XGL_SURF_FILL_SOLID,
        XGL_CTX_SURF_EDGE_FLAG, FALSE,
        XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_OFF,
        XGL_3D_CTX_SURF_FACE_DISTINGUISH, FALSE,
        XGL_3D_CTX_SURF_NORMAL_FLIP, FALSE,
        XGL_3D_CTX_SURF_LIGHTING_NORMAL_FLIP, FALSE,
        XGL_3D_CTX_SURF_FRONT_AMBIENT, 1.0,
        XGL_3D_CTX_SURF_FRONT_DIFFUSE, 1.0,
        XGL_3D_CTX_SURF_FRONT_SPECULAR, 0.0,
        XGL_3D_CTX_SURF_FRONT_LIGHT_COMPONENT, XGL_LIGHT_ENABLE_COMP_AMBIENT | XGL_LIGHT_ENABLE_COMP_DIFFUSE,
        XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_Z_BUFFER,
        XGL_3D_CTX_Z_BUFFER_COMP_METHOD, XGL_Z_COMP_LESS_THAN_OR_EQUAL,
        XGL_3D_CTX_LIGHT_NUM, (Xgl_usgn32)6, 0);
    native_check();
    if (!xb.scene) Sys_Error("XGL 3D context creation failed");
    xgl_object_get(xb.scene, XGL_CTX_LOCAL_MODEL_TRANS, &xb.model);
    xgl_object_get(xb.scene, XGL_CTX_VIEW_TRANS, &xb.view);
    native_check();
    if (!xb.model || !xb.view) Sys_Error("XGL did not create scene transforms");
    xgl_object_set(xb.model, XGL_TRANS_DATA_TYPE, XGL_DATA_FLT, XGL_TRANS_DIMENSION, XGL_TRANS_3D, 0);
    xgl_object_set(xb.view, XGL_TRANS_DATA_TYPE, XGL_DATA_FLT, XGL_TRANS_DIMENSION, XGL_TRANS_3D, 0);
    identity_transform(xb.model); identity_transform(xb.view);
    xgl_object_get(xb.raster, XGL_DEV_MAXIMUM_COORDINATES, &maximum);
    native_check();
    xb.maximum_z = maximum.z;
    memset(&clear, 0, sizeof(clear)); clear.z_buffer.z_value = (float)maximum.z;
    xgl_object_set(xb.scene, XGL_3D_CTX_HLHSR_DATA, &clear, 0);
    xgl_object_get(xb.scene, XGL_3D_CTX_LIGHTS, xb.lights);
    native_check();
    for (i = 0; i < 6; ++i) if (!xb.lights[i]) Sys_Error("XGL did not create all requested lights");
    xgl_object_set(xb.lights[0], XGL_LIGHT_TYPE, XGL_LIGHT_AMBIENT, 0);
    color.rgb.r = color.rgb.g = color.rgb.b = .5f;
    direction.x = -.577350269f; direction.y = -.577350269f; direction.z = -.577350269f;
    xgl_object_set(xb.lights[5], XGL_LIGHT_TYPE, XGL_LIGHT_DIRECTIONAL,
                   XGL_LIGHT_COLOR, &color, XGL_LIGHT_DIRECTION, &direction, 0);
    up.x = 0; up.y = 1;
    color.rgb.r = color.rgb.g = color.rgb.b = 1;
    xb.ui = xgl_object_create(xb.system, XGL_2D_CTX, NULL,
        XGL_CTX_DEVICE, xb.raster, XGL_CTX_RENDER_BUFFER, XGL_RENDER_DRAW_BUFFER,
        XGL_CTX_DEFERRAL_MODE, XGL_DEFER_ASAP,
        XGL_CTX_NEW_FRAME_ACTION, XGL_CTX_NEW_FRAME_NONE,
        XGL_CTX_GEOM_DATA_IS_VOLATILE, TRUE,
        XGL_CTX_VDC_MAP, XGL_VDC_MAP_DEVICE, XGL_CTX_VDC_ORIENTATION, XGL_Y_DOWN_Z_AWAY,
        XGL_CTX_SURF_FRONT_COLOR_SELECTOR, XGL_SURF_COLOR_FACET,
        XGL_CTX_SURF_FRONT_FILL_STYLE, XGL_SURF_FILL_SOLID, XGL_CTX_SURF_EDGE_FLAG, FALSE,
        XGL_CTX_STEXT_PRECISION, XGL_STEXT_PRECISION_STROKE,
        XGL_CTX_STEXT_CHAR_ENCODING, XGL_SINGLE_STR | XGL_CHAR_MBY,
        XGL_CTX_STEXT_CHAR_UP_VECTOR, &up,
        XGL_CTX_STEXT_PATH, XGL_STEXT_PATH_RIGHT,
        XGL_CTX_STEXT_ALIGN_HORIZ, XGL_STEXT_ALIGN_HORIZ_LEFT,
        XGL_CTX_STEXT_ALIGN_VERT, XGL_STEXT_ALIGN_VERT_BASE,
        XGL_CTX_STEXT_COLOR, &color, 0);
    native_check();
    if (!xb.ui) Sys_Error("XGL native text context creation failed");
    font = NULL; name = NULL;
    xgl_object_get(xb.ui, XGL_CTX_SFONT_0, &font);
    native_check();
    if (!font) Sys_Error("XGL native stroke font is unavailable");
    xgl_object_get(font, XGL_SFONT_NAME, &name);
    native_check();
    Con_SafePrintf("XGL stroke font: %s; fixed-cell native glyph placement\n", name ? name : "unknown");
    for (i = 0; i < 95; ++i) { xb.glyphs[i][0] = (char)(i + 32); xb.glyphs[i][1] = 0; }
    for (i = 0; i < XGL_BATCH_TRIANGLES; ++i) {
        xb.lists[i].pt_type = XGL_PT_COLOR_NORMAL_F3D;
        xb.lists[i].num_pts = 3;
        xb.lists[i].pts.color_normal_f3d = (Xgl_pt_color_normal_f3d *)(xb.points + i*3);
    }
    (void)font_size(8, 8); (void)font_size(16, 16);
    native_check();
    Con_SafePrintf("XGL: sampled colors, native ambient/point lighting, %d-triangle batches\n", XGL_BATCH_TRIANGLES);
}
