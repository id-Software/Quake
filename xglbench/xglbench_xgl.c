/* Native Sun XGL 3.x backend, GPL-2.0-or-later. See COPYING.
 * API references: Sun XGL Reference Manual 802-5864, printed pp. 37-42,
 * 79-84, 94-115, 174, 204-205, 228-233, 274-277, 312-315, 333-334, 387-400,
 * 417-424, 457-459, 467-470, 515, 552-567. No OpenGL is used.
 */
#include "xglbench.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <xgl/xgl.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#define XB_RASTER_W 128
#define XB_RASTER_H 64

/* Every pointer reachable by a retained draw remains stable until completion.
 * Keeping separate native copies also prevents CPU color preparation from
 * overwriting geometry still being consumed by the accelerator.
 */
typedef struct {
    Xgl_pt_f3d *points;
    Xgl_pt_color_f3d *color_points;
    Xgl_pt_list *lists;
    Xgl_color_facet *facets;
    Xgl_facet_list *batch_facets;
    size_t triangles, point_count;
    int pending, prepared;
} xb_native_slot;

typedef struct {
    Display *display;
    int screen, width, height, depth, quit, hardware, current_z, obscured;
    Window window;
    Colormap colormap;
    Atom wm_delete;
    VisualID visual;
    unsigned long red_mask, green_mask, blue_mask;
    Xgl_sys_state system;
    Xgl_win_ras raster;
    Xgl_ctx ctx;
    Xgl_trans model, view;
    Xgl_mem_ras hud, mask;
    Xgl_mem_ras texture_rasters[7];
    Xgl_mipmap_texture texture_mipmap;
    Xgl_tmap texture_map;
    Xgl_boolean texture_switch;
    int texture_attached;
    enum xb_texture_filter texture_filter;
    float *texture_points; /* packed XYZUV, five floats per corner */
    Xgl_inquire inquiry;
    Xgl_usgn32 buffers;
    char device[160], error[256], version[160];
    const char *stage;
    enum xb_failure failure;
    Xgl_pt_f3d *points, *strip_points;
    Xgl_pt_color_f3d *color_points;
    Xgl_pt_color_normal_f3d *lit_points;
    Xgl_color_normal_facet *lit_facets;
    Xgl_pt_list *lists;
    Xgl_color_facet *facets, *restart_facets;
    Xgl_facet_list *batch_facets;
    xb_strip *strips;
    size_t triangles, point_count, strip_point_count, strip_count;
    enum xb_method method;
    int batch, perspective, errors, fatal;
    int shading, hardware_lighting, allow_fallback, light_count, cull;
    Xgl_light lights[XB_MAX_LIGHTS + 1];
    xb_light_state light_state;
    int raster_checked, mask_checked, hud_ready;
    xb_native_slot stream[XB_MAX_QUEUE];
    int stream_active, stream_slots, stream_batch;
    Xgl_boolean stream_previous_volatile;
} xb_native;

static xb_native b;

/* Requested payload bytes for arrays owned by this backend. XGL objects,
 * framebuffer/raster storage, allocator bookkeeping and the inquiry result
 * returned by XGL are outside this count. The union keeps the returned array
 * aligned for the scalar types used in native point/descriptor structures.
 * These counters are independent of b: clearing backend state must never
 * erase the accounting for allocations that still exist.
 */
typedef union {
    size_t bytes;
    long double align_float;
    void *align_pointer;
} xb_native_allocation;

static size_t native_memory_current, native_memory_peak;

static void *native_calloc(size_t count, size_t size)
{
    xb_native_allocation *allocation;
    size_t bytes;
    if (size && count > ((size_t)-1 - sizeof(*allocation)) / size)
        return NULL;
    bytes = count * size;
    if (bytes > (size_t)-1 - native_memory_current) return NULL;
    allocation = (xb_native_allocation *)calloc(1, sizeof(*allocation) + bytes);
    if (!allocation) return NULL;
    allocation->bytes = bytes;
    native_memory_current += bytes;
    if (native_memory_current > native_memory_peak)
        native_memory_peak = native_memory_current;
    return allocation + 1;
}

static void native_free(void *pointer)
{
    xb_native_allocation *allocation;
    if (!pointer) return;
    allocation = (xb_native_allocation *)pointer - 1;
    native_memory_current -= allocation->bytes;
    free(allocation);
}

void xb_backend_memory(size_t *current, size_t *peak)
{
    if (current) *current = native_memory_current;
    if (peak) *peak = native_memory_peak;
}

void xb_backend_memory_reset_peak(void)
{
    native_memory_peak = native_memory_current;
}

/* The cancellation query reads latched flags only: no clock or XGL call in
 * the per-primitive hot loops. Cancellation must not set b.errors/fatal,
 * because synchronization and retained-buffer cleanup still have to run. */
static int validate_raster(int masked);
static int validate_stream(void);
static int validate_lighting(void);
static int validate_culling(void);
static int pixel_matches(int x, int y, const xb_rgb *expected);
static int configure_texture(const xb_case *test);
static int texture_enable(int enabled);
static int set_transparency(int enabled, double opacity, int zwrite);
static int validate_transparency(void);
static int validate_combined_clear(void);

static void free_stream_slots(void)
{
    int i;
    for (i = 0; i < XB_MAX_QUEUE; ++i) {
        native_free(b.stream[i].points);
        native_free(b.stream[i].color_points);
        native_free(b.stream[i].lists);
        native_free(b.stream[i].facets);
        native_free(b.stream[i].batch_facets);
        memset(&b.stream[i], 0, sizeof(b.stream[i]));
    }
    b.stream_slots = 0;
}

static void copy_text(char *dest, size_t size, const char *src)
{
    size_t length;
    if (!size) return;
    if (!src) src = "unknown";
    length = strlen(src);
    if (length >= size) length = size - 1;
    memcpy(dest, src, length);
    dest[length] = '\0';
}

static void fail_kind(enum xb_failure kind, const char *message)
{
    /* A later diagnostic or cleanup must not hide the first native error. */
    if (b.errors) return;
    b.failure = kind;
    copy_text(b.error, sizeof(b.error), message);
}

static void fail(const char *message)
{
    fail_kind(XB_FAILURE_ERROR, message);
}

static void error_context(const char *context)
{
    char previous[sizeof(b.error)];
    if (!b.error[0]) return;
    copy_text(previous, sizeof(previous), b.error);
    sprintf(b.error, "%.75s: %.175s", context, previous);
}

static Xgl_sgn32 xgl_error(Xgl_sys_state system)
{
    Xgl_error_info info;
    const char *stage;
    memset(&info, 0, sizeof(info));
    xgl_object_get(system, XGL_SYS_ST_ERROR_INFO, &info);
    stage = b.stage ? b.stage : "native operation";
    if (!b.errors) {
        b.failure = XB_FAILURE_API;
        sprintf(b.error, "%.36s: %.20s %.100s (%.30s, %.25s, %.25s)",
                stage, info.id ? info.id : "?",
                info.msg ? info.msg : "XGL operation failed",
                info.cur_obj ? info.cur_obj : "?",
                info.operand1 ? info.operand1 : "?",
                info.operand2 ? info.operand2 : "?");
    }
    ++b.errors;
    if (info.type == XGL_ERROR_NONRECOVERABLE) b.fatal = 1;
    fprintf(stderr, "XGL error %s during %s: op=%s object=%s operand1=%s operand2=%s: %s\n",
            info.id ? info.id : "?", stage,
            info.cur_op ? info.cur_op : "?", info.cur_obj ? info.cur_obj : "?",
            info.operand1 ? info.operand1 : "?", info.operand2 ? info.operand2 : "?",
            info.msg ? info.msg : "XGL operation failed");
    return 1;
}

static const char *support(Xgl_inq_support_level value)
{
    if (value == XGL_INQ_HARDWARE) return "hardware";
    if (value == XGL_INQ_SOFTWARE) return "software";
    if (value == XGL_INQ_NOT_SUPPORTED) return "unsupported";
    return "unknown";
}

static void identity(Xgl_trans transform)
{
    Xgl_matrix_f3d matrix;
    int i;
    if (b.errors) return;
    memset(matrix, 0, sizeof(matrix));
    for (i = 0; i < 4; ++i) matrix[i][i] = 1.0f;
    xgl_transform_write(transform, matrix);
}

static void view_matrix(int perspective)
{
    Xgl_matrix_f3d matrix;
    int i;
    if (b.errors) return;
    memset(matrix, 0, sizeof(matrix));
    for (i = 0; i < 4; ++i) matrix[i][i] = 1.0f;
    /* Row-vector homogeneous projection: w = 1 + 0.75*z.
     * This exercises native clipping/divide without pre-dividing vertices.
     * Scene z stays positive; near is smaller z in Y_DOWN_Z_AWAY.
     */
    if (perspective) matrix[2][3] = 0.75f;
    xgl_transform_write(b.view, matrix);
}

static void setup_view(void)
{
    Xgl_bounds_d3d bounds;
    Xgl_hlhsr_data clear;
    Xgl_pt_d3d maximum;
    if (b.errors) return;
    memset(&bounds, 0, sizeof(bounds));
    bounds.xmin = bounds.ymin = -1.0;
    bounds.xmax = bounds.ymax = 1.0;
    bounds.zmin = 0.0;
    bounds.zmax = 1.0;
    xgl_object_set(b.ctx, XGL_CTX_VDC_MAP, XGL_VDC_MAP_ALL,
                   XGL_CTX_VDC_ORIENTATION, XGL_Y_DOWN_Z_AWAY,
                   XGL_CTX_VDC_WINDOW, &bounds,
                   XGL_CTX_VIEW_CLIP_BOUNDS, &bounds,
                   XGL_CTX_CLIP_PLANES, XGL_CLIP_XMIN | XGL_CLIP_XMAX |
                       XGL_CLIP_YMIN | XGL_CLIP_YMAX |
                       XGL_CLIP_ZMIN | XGL_CLIP_ZMAX, 0);
    if (b.errors) return;
    xgl_object_get(b.raster, XGL_DEV_MAXIMUM_COORDINATES, &maximum);
    if (b.errors) return;
    clear.z_buffer.z_value = (float)maximum.z;
    xgl_object_set(b.ctx, XGL_3D_CTX_HLHSR_DATA, &clear, 0);
}

/* Request RGB before depth/dimensions can initialize raster storage. Setting
 * it after those attributes produced di-158 on XGL 3.3/Leo; omitting it left
 * the logical color type non-RGB. Depth alone does not select RGB. The new
 * creation order still needs hardware validation, so retain the inquiries
 * and the copy/texture readback gates before accepting any measurements.
 * Only fully checked objects enter a cache; failed, unreferenced objects can
 * be destroyed on recoverable errors. After a fatal error xgl_close owns all
 * remaining cleanup and no further object operations are attempted.
 */
static void destroy_unreferenced(Xgl_object object)
{
    char reason[sizeof(b.error)];
    enum xb_failure failure;
    if (!object || b.fatal) return;
    copy_text(reason, sizeof(reason), b.error);
    failure = b.failure;
    xgl_object_destroy(object);
    if (reason[0]) {
        copy_text(b.error, sizeof(b.error), reason);
        b.failure = failure;
    }
}

static Xgl_mem_ras make_memory_raster(int depth, int width, int height,
                                    void **address, const char *stage)
{
    Xgl_mem_ras raster;
    Xgl_usgn32 actual_depth, actual_width, actual_height;
    Xgl_color_type color_type;
    const char *previous_stage;
    char message[192];
    *address = NULL;
    if (b.errors || b.fatal || xb_cancel_requested()) return NULL;
    previous_stage = b.stage;
    b.stage = stage;
    if (depth == 32)
        raster = xgl_object_create(b.system, XGL_MEM_RAS, NULL,
                    XGL_DEV_COLOR_TYPE, XGL_COLOR_RGB,
                    XGL_RAS_DEPTH, (Xgl_usgn32)depth,
                    XGL_RAS_WIDTH, (Xgl_usgn32)width,
                    XGL_RAS_HEIGHT, (Xgl_usgn32)height, 0);
    else
        raster = xgl_object_create(b.system, XGL_MEM_RAS, NULL,
                    XGL_RAS_DEPTH, (Xgl_usgn32)depth,
                    XGL_RAS_WIDTH, (Xgl_usgn32)width,
                    XGL_RAS_HEIGHT, (Xgl_usgn32)height, 0);
    if (!raster || b.errors) {
        if (!b.errors) fail("XGL returned no memory raster");
        goto bad;
    }
    actual_depth = actual_width = actual_height = 0;
    xgl_object_get(raster, XGL_RAS_DEPTH, &actual_depth);
    if (b.errors) goto bad;
    xgl_object_get(raster, XGL_RAS_WIDTH, &actual_width);
    if (b.errors) goto bad;
    xgl_object_get(raster, XGL_RAS_HEIGHT, &actual_height);
    if (b.errors) goto bad;
    if (actual_depth != (Xgl_usgn32)depth || actual_width != (Xgl_usgn32)width ||
        actual_height != (Xgl_usgn32)height) {
        sprintf(message, "memory raster dimensions/depth: wanted %dx%dx%d, got %lux%lux%lu",
                width, height, depth, (unsigned long)actual_width,
                (unsigned long)actual_height, (unsigned long)actual_depth);
        fail_kind(XB_FAILURE_VALIDATION, message);
        goto bad;
    }
    if (depth == 32) {
        color_type = XGL_COLOR_INDEX;
        xgl_object_get(raster, XGL_DEV_COLOR_TYPE, &color_type);
        if (b.errors) goto bad;
        if (color_type != XGL_COLOR_RGB) {
            sprintf(message, "32-bit memory raster color type: expected RGB (%d), got %d",
                    (int)XGL_COLOR_RGB, (int)color_type);
            fail_kind(XB_FAILURE_VALIDATION, message);
            goto bad;
        }
    }
    xgl_object_get(raster, XGL_MEM_RAS_IMAGE_BUFFER_ADDR, address);
    if (b.errors) goto bad;
    if (!*address) {
        fail_kind(XB_FAILURE_VALIDATION, "memory raster has no image buffer");
        goto bad;
    }
    b.stage = previous_stage;
    return raster;
bad:
    if (b.failure != XB_FAILURE_API) error_context(stage);
    destroy_unreferenced(raster);
    *address = NULL;
    b.stage = previous_stage;
    return NULL;
}

static int make_hud(void)
{
    Xgl_mem_ras hud, mask;
    Xgl_usgn32 *pixels;
    Xgl_usgn16 *mask_words;
    void *image_address, *mask_address;
    const char *previous_stage;
    int x, y;
    if (b.errors || b.fatal || xb_cancel_requested()) return 0;
    if (!b.hud || !b.mask) {
        image_address = mask_address = NULL;
        hud = make_memory_raster(32, XB_RASTER_W, XB_RASTER_H,
                                 &image_address, "HUD RGB raster creation");
        if (!hud) return 0;
        mask = make_memory_raster(1, XB_RASTER_W, XB_RASTER_H,
                                  &mask_address, "HUD mask raster creation");
        if (!mask) {
            destroy_unreferenced(hud);
            return 0;
        }
        pixels = (Xgl_usgn32 *)image_address;
        mask_words = (Xgl_usgn16 *)mask_address;
        memset(mask_words, 0, (size_t)XB_RASTER_W * XB_RASTER_H / 8);
        for (y = 0; y < XB_RASTER_H; ++y) {
            for (x = 0; x < XB_RASTER_W; ++x) {
                /* Canonical XGL native word, independent of host byte order. */
                pixels[y * XB_RASTER_W + x] =
                    ((Xgl_usgn32)(64 + (y * 3) % 192) << 16) |
                    ((Xgl_usgn32)(32 + (x * 5) % 224) << 8) | 220;
                if (((x / 8) + (y / 8)) % 2 == 0)
                    mask_words[y * (XB_RASTER_W / 16) + x / 16] |=
                        (Xgl_usgn16)(0x8000U >> (x & 15));
            }
        }
        /* Both buffers are initialized before exposing either cached handle.
         * Once attached to the context, keep these valid objects even if a
         * later attribute fails; retry attachment without reallocating them.
         */
        b.hud = hud;
        b.mask = mask;
    }
    previous_stage = b.stage;
    b.stage = "HUD mask attachment";
    xgl_object_set(b.ctx, XGL_CTX_RASTER_FPAT, b.mask,
                   XGL_CTX_ROP, XGL_ROP_COPY, 0);
    b.hud_ready = !b.errors;
    b.stage = previous_stage;
    return b.hud_ready;
}

/* XGL 3.1+ Texture Map objects, Reference pp.106-110,118,339-340,374-378,
 * 461; Programmer's Guide pp.469-482. This is pixel-level texturing. The
 * NO_INTERP (vertex-only texture sampling) mode is deliberately never used.
 */
static int texture_enable(int enabled)
{
    if (b.errors) return 0;
    if (!b.texture_attached) return !enabled;
    b.texture_switch = enabled ? TRUE : FALSE;
    xgl_object_set(b.ctx, XGL_3D_CTX_SURF_FRONT_TMAP_SWITCHES,
                   &b.texture_switch, 0);
    return !b.errors;
}

static Xgl_usgn32 *texture_raster_pixels(int level)
{
    Xgl_mem_ras raster;
    void *pixels;
    int side;
    if (b.errors || b.fatal || xb_cancel_requested()) return NULL;
    if (level < 0 || level >= 7) {
        fail("invalid texture mip level");
        return NULL;
    }
    side = XB_TEXTURE_WIDTH >> level;
    pixels = NULL;
    if (!b.texture_rasters[level]) {
        raster = make_memory_raster(32, side, side, &pixels,
                                    "texture raster creation");
        if (!raster) return NULL;
        b.texture_rasters[level] = raster;
    } else {
        xgl_object_get(b.texture_rasters[level], XGL_MEM_RAS_IMAGE_BUFFER_ADDR, &pixels);
        if (b.errors) return NULL;
        if (!pixels) fail("cached texture raster has no image buffer");
    }
    return (Xgl_usgn32 *)pixels;
}

static Xgl_usgn32 texture_word(const xb_rgb *rgb)
{
    return ((Xgl_usgn32)(rgb->b * 255.0f + .5f) << 16) |
           ((Xgl_usgn32)(rgb->g * 255.0f + .5f) << 8) |
           (Xgl_usgn32)(rgb->r * 255.0f + .5f);
}

static int texture_build_image(int pattern)
{
    xb_rgb image[XB_TEXTURE_WIDTH * XB_TEXTURE_WIDTH], rgb;
    Xgl_usgn32 *pixels;
    Xgl_mipmap_texture mipmap;
    int x, y;
    if (xb_cancel_requested()) return 0;
    pixels = texture_raster_pixels(0);
    if (!pixels) return 0;
    if (!pattern && !xb_texture_image(image, XB_TEXTURE_WIDTH * XB_TEXTURE_WIDTH)) {
        fail("cannot obtain benchmark texture image");
        return 0;
    }
    for (y = 0; y < XB_TEXTURE_WIDTH; ++y) {
        for (x = 0; x < XB_TEXTURE_WIDTH; ++x) {
            if (pattern == 1) {
                /* Asymmetric UV quadrants: red, green, blue, yellow. */
                rgb.r = (x < XB_TEXTURE_WIDTH / 2) == (y < XB_TEXTURE_WIDTH / 2);
                rgb.g = x >= XB_TEXTURE_WIDTH / 2;
                rgb.b = x < XB_TEXTURE_WIDTH / 2 && y >= XB_TEXTURE_WIDTH / 2;
            } else if (pattern == 2) rgb.r = rgb.g = rgb.b = (float)(x & 1);
            else rgb = image[y * XB_TEXTURE_WIDTH + x];
            /* CPU row zero denotes v=0. Memory Raster row zero is the top;
             * XGL texture v=0 is the bottom. Native word layout is 0x00BBGGRR.
             */
            pixels[(XB_TEXTURE_WIDTH - 1 - y) * XB_TEXTURE_WIDTH + x] = texture_word(&rgb);
        }
    }
    if (xb_cancel_requested()) return 0;
    if (!b.texture_mipmap) {
        mipmap = xgl_object_create(b.system, XGL_MIPMAP_TEXTURE, NULL,
                                  XGL_MIPMAP_TEXTURE_LEVELS, (Xgl_usgn32)7, 0);
        if (!mipmap || b.errors) {
            if (!b.errors) fail("XGL returned no mipmap texture");
            destroy_unreferenced(mipmap);
            return 0;
        }
        b.texture_mipmap = mipmap;
    }
    if (xb_cancel_requested()) return 0;
    xgl_mipmap_texture_build(b.texture_mipmap, b.texture_rasters[0],
                             XGL_TEXTURE_BOUNDARY_WRAP, XGL_TEXTURE_BOUNDARY_WRAP);
    return !b.errors;
}

static int texture_set_descriptor(void)
{
    Xgl_texture_general_desc desc;
    Xgl_texture_interp_method min_filter, mag_filter;
    if (!b.texture_attached) {
        /* TMAP_NUM owns the map object. Keep it attached but switched off
         * between cases; xgl_close ultimately releases the entire graph.
         */
        xgl_object_set(b.ctx, XGL_3D_CTX_SURF_FRONT_TMAP_NUM, 1, 0);
        if (b.errors) return 0;
        b.texture_attached = 1;
    }
    if (!b.texture_map) {
        xgl_object_get(b.ctx, XGL_3D_CTX_SURF_FRONT_TMAP, &b.texture_map);
        if (!b.texture_map || b.errors) return 0;
    }
    min_filter = b.texture_filter == XB_TEXTURE_POINT ? XGL_TEXTURE_INTERP_POINT :
                 b.texture_filter == XB_TEXTURE_BILINEAR ? XGL_TEXTURE_INTERP_BILINEAR :
                 XGL_TEXTURE_INTERP_MIPMAP_TRILINEAR;
    mag_filter = b.texture_filter == XB_TEXTURE_POINT ? XGL_TEXTURE_INTERP_POINT :
                                                       XGL_TEXTURE_INTERP_BILINEAR;
    memset(&desc, 0, sizeof(desc));
    desc.texture_type = XGL_TEXTURE_TYPE_MIPMAP;
    desc.comp_info.color_info.num_render_comp_desc = 1;
    desc.comp_info.color_info.num_channels[0] = 3;
    desc.comp_info.color_info.render_component_desc[0].comp = XGL_RENDER_COMP_FINAL_COLOR;
    desc.comp_info.color_info.render_component_desc[0].texture_op = XGL_TEXTURE_OP_REPLACE;
    desc.texture_info.mipmap.texture_map = b.texture_mipmap;
    desc.texture_info.mipmap.max_u_freq = desc.texture_info.mipmap.max_v_freq = 1;
    desc.texture_info.mipmap.u_boundary = desc.texture_info.mipmap.v_boundary = XGL_TEXTURE_BOUNDARY_WRAP;
    desc.texture_info.mipmap.interp_info.filter1 = min_filter;
    desc.texture_info.mipmap.interp_info.filter2 = mag_filter;
    desc.texture_info.mipmap.orientation_matrix[0][0] = 1;
    desc.texture_info.mipmap.orientation_matrix[1][1] = 1;
    xgl_object_set(b.texture_map, XGL_TMAP_COORD_SOURCE, XGL_TEXTURE_COORD_DATA,
                   XGL_TMAP_T0_INDEX, 0, XGL_TMAP_T1_INDEX, 1,
                   XGL_TMAP_DESCRIPTOR, &desc, 0);
    return !b.errors && texture_enable(1);
}

static int texture_probe(float *points)
{
    Xgl_pt_list list;
    if (xb_cancel_requested()) return 0;
    memset(&list, 0, sizeof(list));
    list.pt_type = XGL_PT_DATA_F3D;
    list.num_pts = 3;
    list.num_data_values = 2;
    list.pts.data_f3d = (Xgl_pt_data_f3d *)points;
    xb_backend_clear(1, 1);
    if (b.errors || xb_cancel_requested()) return 0;
    xgl_multi_simple_polygon(b.ctx,
        XGL_FACET_FLAG_SIDES_ARE_3 | XGL_FACET_FLAG_SHAPE_CONVEX,
        NULL, NULL, 1, &list);
    xb_backend_sync();
    return !b.errors && !xb_cancel_requested();
}

static int read_pixel(int x, int y, xb_rgb *rgb)
{
    Xgl_pt_i2d point;
    Xgl_color color;
    Xgl_boolean obscured;
    char context[96];
    point.x = x; point.y = y;
    if (b.errors || xb_cancel_requested()) return 0;
    memset(&color, 0, sizeof(color));
    obscured = xgl_context_get_pixel(b.ctx, &point, &color);
    if (b.errors) {
        sprintf(context, "pixel (%d,%d)", x, y);
        error_context(context);
        return 0;
    }
    if (xb_cancel_requested()) return 0;
    if (obscured) {
        sprintf(context, "validation pixel (%d,%d) obscured; keep the entire window visible", x, y);
        fail_kind(XB_FAILURE_VALIDATION, context);
        return 0;
    }
    rgb->r = color.rgb.r; rgb->g = color.rgb.g; rgb->b = color.rgb.b;
    return 1;
}

static int validate_texture(void)
{
    float points[15];
    Xgl_color fallback;
    Xgl_usgn32 *pixels;
    xb_rgb expected, sample;
    double px, py, u, v, z, w, du, dv, lo, hi;
    static const double bary[5][2] = {{.1,.1},{.35,.1},{.1,.35},{.65,.2},{.2,.65}};
    static const double weights[3] = {.27,.58,.15};
    static const double footprint[4] = {1.1,1.25,1.5,1.75};
    int i, j, phase, level, side, middle, mixed;
    char context[96];
    memset(&fallback, 0, sizeof(fallback));
    fallback.rgb.r = fallback.rgb.b = 1;
    xgl_object_set(b.ctx, XGL_CTX_SURF_FRONT_COLOR_SELECTOR, XGL_SURF_COLOR_CONTEXT,
                   XGL_CTX_SURF_FRONT_COLOR, &fallback,
                   XGL_CTX_GEOM_DATA_IS_VOLATILE, TRUE,
                   XGL_3D_CTX_SURF_TMAP_PERSP_CORRECTION, XGL_TEXTURE_PERSP_PIXEL, 0);
    if (b.errors) return 0;
    identity(b.model);
    view_matrix(0);
    memset(points, 0, sizeof(points));
    for (i = 0; i < 3; ++i) {
        points[i * 5] = i == 1 ? .9f : -.9f;
        points[i * 5 + 1] = i == 2 ? .9f : -.9f;
        points[i * 5 + 2] = .5f;
    }
    for (phase = 0; phase < 2; ++phase) {
        for (i = 0; i < 3; ++i) {
            points[i * 5 + 3] = (i == 1 ? 2.0f : 0.0f) - phase;
            points[i * 5 + 4] = (i == 2 ? 2.0f : 0.0f) - phase;
        }
        if (!texture_probe(points)) return 0;
        for (i = 0; i < 5; ++i) {
            u = 2 * bary[i][0]; v = 2 * bary[i][1];
            u -= floor(u); v -= floor(v);
            expected.r = (u < .5) == (v < .5);
            expected.g = u >= .5;
            expected.b = u < .5 && v >= .5;
            px = (-.9 + 1.8 * bary[i][0] + 1) * .5 * (b.width - 1);
            py = (-.9 + 1.8 * bary[i][1] + 1) * .5 * (b.height - 1);
            if (!pixel_matches((int)px, (int)py, &expected)) {
                if (!xb_cancel_requested()) {
                    sprintf(context, "texture UV (%.4g,%.4g), wrap phase %d", u, v, phase);
                    error_context(context);
                }
                return 0;
            }
        }
    }
    if (b.perspective) {
        view_matrix(1);
        px = py = 0;
        for (i = 0; i < 3; ++i) {
            z = i == 1 ? .9 : .1;
            w = 1 + .75 * z;
            points[i * 5 + 2] = (float)z;
            points[i * 5 + 3] = i == 1 ? 2.0f : 0;
            points[i * 5 + 4] = i == 2 ? 2.0f : 0;
            px += weights[i] * points[i * 5] / w;
            py += weights[i] * points[i * 5 + 1] / w;
        }
        if (!texture_probe(points)) return 0;
        /* Correct projective interpolation gives UV about (.94,.38), green;
         * affine interpolation gives (1.16,.30), which wraps to red.
         */
        expected.r = expected.b = 0; expected.g = 1;
        if (!pixel_matches((int)((px + 1) * .5 * (b.width - 1)),
                           (int)((py + 1) * .5 * (b.height - 1)), &expected)) {
            if (!xb_cancel_requested()) error_context("texture perspective correction");
            return 0;
        }
        view_matrix(0);
    }
    if (!texture_build_image(2)) return 0;
    for (i = 0; i < 3; ++i) {
        points[i * 5] = i == 1 ? .8f : -.8f;
        points[i * 5 + 1] = i == 2 ? .8f : -.8f;
        points[i * 5 + 2] = .5f;
        points[i * 5 + 3] = (float)(i == 1 ? 12.0 / 64 : 10.0 / 64);
        points[i * 5 + 4] = .25f;
    }
    if (!texture_probe(points)) return 0;
    lo = 1; hi = 0; middle = 0;
    for (i = 0; i < 17; ++i) {
        px = (-.72 + 1.28 * i / 16 + 1) * .5 * (b.width - 1);
        py = .15 * (b.height - 1);
        if (!read_pixel((int)px, (int)py, &sample)) return 0;
        if (sample.r < lo) lo = sample.r;
        if (sample.r > hi) hi = sample.r;
        if (sample.r > .1 && sample.r < .9) ++middle;
        if (fabs(sample.r - sample.g) > .065 || fabs(sample.r - sample.b) > .065) {
            fail_kind(XB_FAILURE_VALIDATION, "texture filter calibration is not grayscale"); return 0;
        }
    }
    if (hi - lo < .5 || (b.texture_filter == XB_TEXTURE_POINT ? middle != 0 : middle < 3)) {
        fail_kind(XB_FAILURE_VALIDATION, "texture point/bilinear magnification readback failed"); return 0;
    }
    if (b.texture_filter == XB_TEXTURE_TRILINEAR) {
        /* Diagnostic pyramid: red base, green lower levels. This proves
         * blending BETWEEN levels, which a blurred checker alone cannot.
         * Several sub-two-texel footprints accommodate LOD conventions;
         * nearest-level selection can never produce both red and green.
         */
        for (level = 0; level < 7; ++level) {
            pixels = texture_raster_pixels(level);
            if (!pixels) return 0;
            side = XB_TEXTURE_WIDTH >> level;
            for (j = 0; j < side * side; ++j) pixels[j] = level ? 0x0000ff00UL : 0x000000ffUL;
        }
        xgl_object_set(b.texture_mipmap, XGL_MIPMAP_TEXTURE_MEM_RAS_LIST,
                       b.texture_rasters, 0);
        if (b.errors) return 0;
        mixed = 0;
        for (phase = 0; phase < 4; ++phase) {
            du = .8 * (b.width - 1) * footprint[phase] / XB_TEXTURE_WIDTH;
            dv = .8 * (b.height - 1) * footprint[phase] / XB_TEXTURE_WIDTH;
            for (i = 0; i < 3; ++i) {
                points[i * 5 + 3] = i == 1 ? (float)du : 0;
                points[i * 5 + 4] = i == 2 ? (float)dv : 0;
            }
            if (!texture_probe(points) ||
                !read_pixel(b.width / 3, b.height / 3, &sample)) return 0;
            if (sample.r > .1 && sample.r < .9 && sample.g > .1 && sample.g < .9 &&
                sample.b < .065 && fabs(sample.r + sample.g - 1) < .1) mixed = 1;
        }
        if (!mixed) { fail_kind(XB_FAILURE_VALIDATION, "trilinear mip-level blending readback failed"); return 0; }
    }
    if (!texture_enable(0) || !texture_probe(points)) return 0;
    expected.r = expected.b = 1; expected.g = 0;
    if (!pixel_matches(b.width / 3, b.height / 3, &expected)) {
        if (!xb_cancel_requested()) error_context("texture-off recovery");
        return 0;
    }
    view_matrix(b.perspective);
    xgl_object_set(b.ctx, XGL_CTX_SURF_FRONT_COLOR_SELECTOR, XGL_SURF_COLOR_FACET, 0);
    return !b.errors;
}

static int configure_texture(const xb_case *test)
{
    if (test->texture_filter == XB_TEXTURE_OFF) return 1;
    if (test->texture_filter < XB_TEXTURE_POINT || test->texture_filter > XB_TEXTURE_TRILINEAR ||
        test->method != XB_MULTI || test->shade != XB_FLAT || test->lighting != XB_LIGHT_OFF ||
        test->xgl_lighting || test->cull || test->retained || test->streaming ||
        (test->work != XB_DRAW && test->work != XB_FRAME && test->work != XB_SCENE_FRAME) ||
        (test->update != XB_STATIC && test->update != XB_TRANSFORM)) {
        fail_kind(XB_FAILURE_UNSUPPORTED, "native texturing requires unlit static/transformed multi draws or frames, without culling/retention/streaming");
        return 0;
    }
    if (b.width < 128 || b.height < 128) {
        fail_kind(XB_FAILURE_UNSUPPORTED, "texture readback gates require at least a 128x128 drawable"); return 0;
    }
    b.texture_filter = test->texture_filter;
    if (!texture_build_image(1) || !texture_set_descriptor() || !validate_texture()) return 0;
    /* Each selected filter is proved on calibration images above; restore the
     * shared procedural image and build its full 64..1 pyramid before timing.
     */
    return texture_build_image(0) && texture_enable(1);
}

static int wait_window(int width, int height)
{
    double deadline;
    XWindowAttributes attributes;
    XEvent event;
    fd_set readfds;
    struct timeval timeout;
    int fd;
    deadline = xb_now() + 5.0;
    fd = ConnectionNumber(b.display);
    do {
        if (xb_cancel_requested()) return 0;
        while (!xb_cancel_requested() && XPending(b.display)) {
            XNextEvent(b.display, &event);
            if (event.type == VisibilityNotify)
                b.obscured = event.xvisibility.state != VisibilityUnobscured;
            if (event.type == DestroyNotify) {
                b.window = 0;
                b.quit = 1;
                fail("window destroyed while waiting for resize");
                return 0;
            }
            if (event.type == ClientMessage &&
                (Atom)event.xclient.data.l[0] == b.wm_delete) {
                b.quit = 1;
                fail("window closed while waiting for resize");
                return 0;
            }
        }
        if (xb_cancel_requested()) return 0;
        XGetWindowAttributes(b.display, b.window, &attributes);
        if (attributes.map_state == IsViewable && !b.obscured &&
            attributes.width == width && attributes.height == height) {
            b.width = attributes.width;
            b.height = attributes.height;
            XSync(b.display, False);
            return 1;
        }
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = 20000;
        (void)select(fd + 1, &readfds, NULL, NULL, &timeout);
    } while (xb_now() < deadline);
    fail("requested drawable size was not visible and unobscured within 5 seconds");
    return 0;
}

static int choose_visual(int width, int height, int allow_fallback)
{
    XVisualInfo query, *visuals;
    XSetWindowAttributes attributes;
    Xgl_X_window xwindow;
    Xgl_obj_desc descriptor;
    Xgl_inquire *info;
    int count, i, best, score;
    Window window;
    Colormap colormap;
    memset(&query, 0, sizeof(query));
    query.screen = b.screen;
    query.class = TrueColor;
    visuals = XGetVisualInfo(b.display, VisualScreenMask | VisualClassMask,
                            &query, &count);
    if (!visuals) {
        fail("no TrueColor visual is available on this X screen");
        return 0;
    }
    best = -1;
    for (i = 0; i < count && !xb_cancel_requested(); ++i) {
        if (visuals[i].depth < 24 && !allow_fallback) continue;
        colormap = XCreateColormap(b.display, RootWindow(b.display, b.screen),
                                   visuals[i].visual, AllocNone);
        memset(&attributes, 0, sizeof(attributes));
        attributes.colormap = colormap;
        attributes.background_pixel = 0;
        attributes.border_pixel = 0;
        attributes.backing_store = NotUseful;
        attributes.event_mask = ExposureMask | StructureNotifyMask |
                                KeyPressMask | VisibilityChangeMask;
        window = XCreateWindow(b.display, RootWindow(b.display, b.screen),
                    0, 0, (unsigned int)width, (unsigned int)height, 0,
                    visuals[i].depth, InputOutput, visuals[i].visual,
                    CWColormap | CWBackPixel | CWBorderPixel |
                    CWBackingStore | CWEventMask, &attributes);
        XSync(b.display, False);
        memset(&xwindow, 0, sizeof(xwindow));
        xwindow.X_display = b.display;
        xwindow.X_screen = b.screen;
        xwindow.X_window = window;
        memset(&descriptor, 0, sizeof(descriptor));
        descriptor.win_ras.type = XGL_WIN_X;
        descriptor.win_ras.desc = &xwindow;
        info = xgl_inquire(b.system, &descriptor);
        score = -1;
        if (info && !b.errors) {
            score = (info->dga_flag ? 1000 : 0) +
                    (info->true_color == XGL_INQ_HARDWARE ? 100 : 0) +
                    (info->hlhsr == XGL_INQ_HARDWARE ? 100 : 0) +
                    (info->shading == XGL_INQ_HARDWARE ? 50 : 0) +
                    (info->maximum_buffer >= 2 ? 20 : 0) +
                    (visuals[i].depth == 24 ? 2 : 0);
        }
        if (score > best) {
            if (b.window) XDestroyWindow(b.display, b.window);
            if (b.colormap) XFreeColormap(b.display, b.colormap);
            b.window = window;
            b.colormap = colormap;
            b.depth = visuals[i].depth;
            b.visual = visuals[i].visualid;
            b.red_mask = visuals[i].red_mask;
            b.green_mask = visuals[i].green_mask;
            b.blue_mask = visuals[i].blue_mask;
            b.inquiry = *info;
            copy_text(b.device, sizeof(b.device), info->name);
            b.inquiry.name = b.device;
            best = score;
        } else {
            XDestroyWindow(b.display, window);
            XFreeColormap(b.display, colormap);
        }
        if (info) free(info);
        if (b.errors) break;
    }
    XFree(visuals);
    if (xb_cancel_requested()) return 0;
    if (best < 0 || b.errors) {
        if (!b.errors) fail("no 24/32-bit TrueColor visual could be queried by XGL");
        return 0;
    }
    return 1;
}

int xb_backend_open(const xb_options *options, int width, int height)
{
    Xgl_X_window xwindow;
    Xgl_obj_desc descriptor;
    Xgl_color black;
    XSizeHints hints;
    char *version;
    memset(&b, 0, sizeof(b));
    b.stage = "native initialization";
    if (sizeof(Xgl_usgn32) != 4 || sizeof(void *) != 4 || sizeof(float) != 4) {
        fail("native XGL requires the 32-bit ABI (4-byte pointers, Xgl_usgn32 and float)");
        return 0;
    }
    if (xb_cancel_requested()) {
        copy_text(b.error, sizeof(b.error), "profile time budget reached");
        return 0;
    }
    b.allow_fallback = options->allow_fallback;
    b.display = XOpenDisplay(options->display);
    if (!b.display) {
        fail("cannot open X display; run in the local OpenWindows desktop");
        return 0;
    }
    if (xb_cancel_requested()) return 0;
    b.screen = DefaultScreen(b.display);
    b.system = xgl_open((Xgl_attribute)NULL);
    if (!b.system) { fail("xgl_open failed"); return 0; }
    if (xb_cancel_requested()) return 0;
    xgl_object_set(b.system, XGL_SYS_ST_ERROR_NOTIFICATION_FUNCTION, xgl_error,
                   XGL_SYS_ST_ERROR_DETECTION, TRUE, 0);
    if (b.errors || xb_cancel_requested()) return 0;
    version = NULL;
    xgl_object_get(b.system, XGL_SYS_ST_VERSION, &version);
    if (b.errors || xb_cancel_requested()) return 0;
    copy_text(b.version, sizeof(b.version), version);
    if (!choose_visual(width, height, options->allow_fallback)) return 0;
    b.hardware = b.inquiry.dga_flag &&
        b.inquiry.true_color == XGL_INQ_HARDWARE &&
        b.inquiry.hlhsr == XGL_INQ_HARDWARE &&
        b.inquiry.shading == XGL_INQ_HARDWARE;
    if (!options->allow_fallback && (!b.hardware ||
        b.inquiry.double_buffer != XGL_INQ_HARDWARE ||
        b.inquiry.maximum_buffer < 2)) {
        fail("selected visual lacks local DGA/hardware RGB, shading, Z, or double buffering; use -allow-fallback only for diagnostic runs");
        return 0;
    }
    b.wm_delete = XInternAtom(b.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(b.display, b.window, &b.wm_delete, 1);
    memset(&hints, 0, sizeof(hints));
    hints.flags = PSize;
    hints.width = width;
    hints.height = height;
    XSetWMNormalHints(b.display, b.window, &hints);
    XStoreName(b.display, b.window, "xglbench: initializing (Escape to stop)");
    XMapRaised(b.display, b.window);
    XSync(b.display, False);
    if (!wait_window(width, height) || xb_cancel_requested()) return 0;
    memset(&xwindow, 0, sizeof(xwindow));
    xwindow.X_display = b.display;
    xwindow.X_screen = b.screen;
    xwindow.X_window = b.window;
    memset(&descriptor, 0, sizeof(descriptor));
    descriptor.win_ras.type = options->allow_fallback ? XGL_WIN_X :
                             (XGL_WIN_X | XGL_WIN_X_PROTO_DGA);
    descriptor.win_ras.desc = &xwindow;
    b.raster = xgl_object_create(b.system, XGL_WIN_RAS, &descriptor,
                XGL_DEV_COLOR_TYPE, XGL_COLOR_RGB,
                XGL_WIN_RAS_BACKING_STORE, FALSE,
                XGL_WIN_RAS_BUFFERS_REQUESTED, 2, 0);
    if (!b.raster || b.errors || xb_cancel_requested()) return 0;
    xgl_object_get(b.raster, XGL_WIN_RAS_BUFFERS_ALLOCATED, &b.buffers);
    if (b.errors || xb_cancel_requested()) return 0;
    if (b.buffers < 2 && !options->allow_fallback) {
        fail("XGL allocated fewer than two window buffers");
        return 0;
    }
    xgl_object_set(b.raster, XGL_WIN_RAS_BUF_DISPLAY, 0,
                   XGL_WIN_RAS_BUF_DRAW, b.buffers >= 2 ? 1 : 0,
                   XGL_RAS_SOURCE_BUFFER, XGL_BUFFER_SEL_DRAW, 0);
    if (b.errors || xb_cancel_requested()) return 0;
    memset(&black, 0, sizeof(black));
    b.ctx = xgl_object_create(b.system, XGL_3D_CTX, NULL,
                XGL_CTX_DEVICE, b.raster,
                XGL_CTX_BACKGROUND_COLOR, &black,
                XGL_CTX_DEFERRAL_MODE, XGL_DEFER_ASTI,
                XGL_CTX_GEOM_DATA_IS_VOLATILE, TRUE,
                XGL_CTX_SURF_FRONT_COLOR_SELECTOR, XGL_SURF_COLOR_FACET,
                XGL_3D_CTX_SURF_FRONT_ILLUMINATION, XGL_ILLUM_NONE,
                XGL_CTX_SURF_FRONT_FILL_STYLE, XGL_SURF_FILL_SOLID,
                XGL_CTX_SURF_EDGE_FLAG, FALSE,
                XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_OFF,
                XGL_3D_CTX_SURF_GEOM_NORMAL, XGL_GEOM_NORMAL_FIRST_POINTS,
                XGL_3D_CTX_SURF_FACE_DISTINGUISH, FALSE,
                XGL_3D_CTX_Z_BUFFER_COMP_METHOD, XGL_Z_COMP_LESS_THAN_OR_EQUAL,
                XGL_3D_CTX_Z_BUFFER_WRITE_MASK, (Xgl_usgn32)0xffffffffUL,
                XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_Z_BUFFER, 0);
    if (!b.ctx || b.errors || xb_cancel_requested()) return 0;
    b.current_z = 1;
    xgl_object_get(b.ctx, XGL_CTX_LOCAL_MODEL_TRANS, &b.model);
    if (b.errors || xb_cancel_requested()) return 0;
    xgl_object_get(b.ctx, XGL_CTX_VIEW_TRANS, &b.view);
    if (b.errors || xb_cancel_requested()) return 0;
    xgl_object_set(b.model, XGL_TRANS_DATA_TYPE, XGL_DATA_FLT,
                   XGL_TRANS_DIMENSION, XGL_TRANS_3D, 0);
    if (b.errors || xb_cancel_requested()) return 0;
    xgl_object_set(b.view, XGL_TRANS_DATA_TYPE, XGL_DATA_FLT,
                   XGL_TRANS_DIMENSION, XGL_TRANS_3D, 0);
    if (b.errors || xb_cancel_requested()) return 0;
    identity(b.model);
    view_matrix(0);
    setup_view();
    xb_backend_clear(1, 1);
    xb_backend_sync();
    if (b.buffers >= 2) {
        xb_backend_swap();
        xb_backend_clear(1, 1);
        xb_backend_sync();
    }
    (void)xb_backend_events();
    return !b.errors && !xb_cancel_requested();
}

static void free_geometry(void)
{
    native_free(b.texture_points);
    b.texture_points = NULL;
    native_free(b.points); native_free(b.strip_points); native_free(b.lists);
    native_free(b.color_points); native_free(b.lit_points); native_free(b.lit_facets);
    native_free(b.facets); native_free(b.restart_facets); native_free(b.strips);
    native_free(b.batch_facets);
    b.points = b.strip_points = NULL;
    b.color_points = NULL;
    b.lit_points = NULL;
    b.lit_facets = NULL;
    b.lists = NULL;
    b.facets = b.restart_facets = NULL;
    b.batch_facets = NULL;
    b.strips = NULL;
    b.triangles = b.point_count = b.strip_point_count = b.strip_count = 0;
}

void xb_backend_close(void)
{
    if (b.ctx && !b.errors) xb_backend_sync();
    if (b.system) xgl_close(b.system); /* also destroys every XGL object */
    /* Even after an error, xgl_close ends all native references. */
    free_stream_slots();
    b.stream_active = 0;
    free_geometry();
    if (b.display) {
        if (b.window) XDestroyWindow(b.display, b.window);
        if (b.colormap) XFreeColormap(b.display, b.colormap);
        XCloseDisplay(b.display);
    }
    b.system = NULL;
    b.display = NULL;
    b.ctx = NULL;
}

int xb_backend_resize(int width, int height)
{
    if (b.errors || b.quit || xb_cancel_requested()) return 0;
    xb_backend_sync();
    if (xb_cancel_requested()) return 0;
    XResizeWindow(b.display, b.window, (unsigned int)width, (unsigned int)height);
    XSync(b.display, False);
    if (!wait_window(width, height) || xb_cancel_requested()) return 0;
    xgl_window_raster_resize(b.raster);
    b.raster_checked = b.mask_checked = 0;
    setup_view();
    xb_backend_clear(1, 1);
    xb_backend_sync();
    if (b.buffers >= 2) {
        xb_backend_swap();
        xb_backend_clear(1, 1);
        xb_backend_sync();
    }
    (void)xb_backend_events();
    return !b.errors && !b.quit && !xb_cancel_requested();
}

void xb_backend_dimensions(int *width, int *height)
{
    *width = b.width;
    *height = b.height;
}

void xb_backend_info(FILE *out)
{
    fprintf(out, "XGL version: %s\nDevice: %s\n", b.version, b.device);
    fprintf(out, "Display: %s; screen %d; visual 0x%lx TrueColor depth %d\n",
            b.display ? DisplayString(b.display) : "unopened", b.screen,
            (unsigned long)b.visual, b.depth);
    fprintf(out, "Visual RGB masks: 0x%lx / 0x%lx / 0x%lx; HUD memory raster: "
                 "32-bit native B:G:R words (24 color bits).\n",
            b.red_mask, b.green_mask, b.blue_mask);
    fprintf(out, "Drawable: %dx%d; DGA: %s; buffers allocated: %lu (maximum %ld)\n",
            b.width, b.height, b.inquiry.dga_flag ? "yes" : "no",
            (unsigned long)b.buffers, (long)b.inquiry.maximum_buffer);
    fprintf(out, "Reported acceleration: RGB=%s shading=%s Z=%s double-buffer=%s\n",
            support(b.inquiry.true_color), support(b.inquiry.shading),
            support(b.inquiry.hlhsr), support(b.inquiry.double_buffer));
    fprintf(out, "Swap implementation: %s; transparency=%s; lighting=%s\n",
            b.inquiry.db_buffer_is_copy ? "copy" : "buffer switch",
            support(b.inquiry.transparency), support(b.inquiry.lighting));
    fprintf(out, "Default state: flat facet RGB; illumination off; no textures; culling off; "
                 "six clipping planes; deferred submission; volatile geometry "
                 "except retained-array and pipeline cases.\n");
    fprintf(out, "Raster copies share the 3D context to preserve draw order "
                 "without inter-context synchronization.\n");
    fprintf(out, "Triangle-list facets are padded per batch to num_points-2 as "
                 "documented; readback must pass. No IS_PLANAR flag is used.\n");
    fprintf(out, "Strip inputs share vertex records; batch caps split strips "
                 "with two overlapping vertices. Raster copies: %dx%d.\n",
            XB_RASTER_W, XB_RASTER_H);
    fprintf(out, "Pipeline: CPU facet or vertex RGB; one thread/context; stable native slots; BUFFERS posts, "
                 "SYNCHRONIZE drains before slot reuse; no presentation.\n");
    fprintf(out, "Culling comparisons use unlit multi-polygon geometry with explicit "
                 "first-point normals; opposite-winding color readback gates back-face culling.\n");
    fprintf(out, "Lighting cases: CPU facet/vertex RGB or XGL facet/vertex ambient+diffuse; "
                 "supplied unit normals, both normal-flip options off, specular disabled.\n");
    fprintf(out, "Reported lighting support is coarse; it does not prove every light "
                 "type/count remains accelerated. XGL point positions use double precision.\n");
    fprintf(out, "Native texture cases: 64x64 RGB mipmap, explicit XYZUV, pixel-level "
                 "perspective correction, REPLACE final RGB; point/bilinear/trilinear filters.\n");
    fprintf(out, "Texture execution unclassified; may use software. RGB/shading inquiry "
                 "does not identify the texture implementation. Texture preparation is untimed.\n");
    fprintf(out, "Input bytes count referenced native points/facet colors, including "
                 "strip overlaps and list padding, excluding descriptors; "
                 "not measured SBus/DMA traffic.\n");
    if (!b.hardware || b.buffers < 2)
        fprintf(out, "DIAGNOSTIC FALLBACK: these results do not establish ZX hardware performance.\n");
}

int xb_backend_upload(const xb_mesh *mesh)
{
    size_t i, start, count, batch_count;
    int needs_points, needs_lists;
    if (xb_cancel_requested()) return 0;
    if (b.stream_active) {
        fail("end the active stream before uploading ordinary native geometry");
        return 0;
    }
    if (!mesh || !mesh->triangles || mesh->point_count != mesh->triangles * 3)
        return 0;
    if (b.batch <= 0) b.batch = 128;
    /* Full rebuild is intentionally measurable; static workloads upload once. */
    free_geometry();
    needs_points = b.method != XB_STRIP && b.shading != XB_SMOOTH &&
                   b.texture_filter == XB_TEXTURE_OFF;
    needs_lists = b.method == XB_POLYGON || b.method == XB_MULTI;
    if (needs_points)
        b.points = (Xgl_pt_f3d *)native_calloc(mesh->point_count, sizeof(*b.points));
    if (b.texture_filter != XB_TEXTURE_OFF) {
        if (!mesh->texcoords) { fail("texturing requires per-corner UV data"); return 0; }
        b.texture_points = (float *)native_calloc(mesh->point_count, 5 * sizeof(float));
    }
    if (b.shading == XB_SMOOTH) {
        if (!mesh->vertex_colors || (b.hardware_lighting && !mesh->vertex_normals)) {
            fail("smooth shading requires vertex colors and lit vertices require normals");
            return 0;
        }
        if (b.hardware_lighting)
            b.lit_points = (Xgl_pt_color_normal_f3d *)native_calloc(mesh->point_count,
                                                           sizeof(*b.lit_points));
        else b.color_points = (Xgl_pt_color_f3d *)native_calloc(mesh->point_count,
                                                       sizeof(*b.color_points));
    } else if (b.hardware_lighting) {
        if (!mesh->facet_normals) { fail("facet lighting requires supplied normals"); return 0; }
        b.lit_facets = (Xgl_color_normal_facet *)native_calloc(mesh->triangles,
                                                      sizeof(*b.lit_facets));
    }
    if (b.method == XB_STRIP) {
        b.strip_points = (Xgl_pt_f3d *)native_calloc(mesh->strip_point_count,
                                            sizeof(*b.strip_points));
        b.strips = (xb_strip *)native_calloc(mesh->strip_count, sizeof(*b.strips));
    }
    if (needs_lists)
        b.lists = (Xgl_pt_list *)native_calloc(mesh->triangles, sizeof(*b.lists));
    b.facets = (Xgl_color_facet *)native_calloc(mesh->triangles, sizeof(*b.facets));
    if (b.method == XB_TLIST)
        b.restart_facets = (Xgl_color_facet *)native_calloc(mesh->triangles,
                                                   3 * sizeof(*b.restart_facets));
    batch_count = (mesh->triangles + (size_t)b.batch - 1) / (size_t)b.batch;
    if (b.method == XB_MULTI)
        b.batch_facets = (Xgl_facet_list *)native_calloc(batch_count, sizeof(*b.batch_facets));
    if ((needs_points && !b.points) || (needs_lists && !b.lists) || !b.facets ||
        (b.texture_filter != XB_TEXTURE_OFF && !b.texture_points) ||
        (b.shading == XB_SMOOTH && !(b.hardware_lighting ? (void *)b.lit_points :
                                                          (void *)b.color_points)) ||
        (b.shading != XB_SMOOTH && b.hardware_lighting && !b.lit_facets) ||
        (b.method == XB_STRIP && (!b.strips || !b.strip_points)) ||
        (b.method == XB_TLIST && !b.restart_facets) ||
        (b.method == XB_MULTI && !b.batch_facets)) {
        fail("out of memory building native XGL submission arrays");
        free_geometry();
        return 0;
    }
    b.triangles = mesh->triangles;
    b.point_count = mesh->point_count;
    b.strip_point_count = mesh->strip_point_count;
    b.strip_count = mesh->strip_count;
    if (b.strips)
        memcpy(b.strips, mesh->strips, mesh->strip_count * sizeof(*b.strips));
    for (i = 0; b.points && i < mesh->point_count; ++i) {
        b.points[i].x = mesh->points[i].x;
        b.points[i].y = mesh->points[i].y;
        b.points[i].z = mesh->points[i].z;
    }
    for (i = 0; b.texture_points && i < mesh->point_count; ++i) {
        b.texture_points[i * 5] = mesh->points[i].x;
        b.texture_points[i * 5 + 1] = mesh->points[i].y;
        b.texture_points[i * 5 + 2] = mesh->points[i].z;
        b.texture_points[i * 5 + 3] = mesh->texcoords[i].u;
        b.texture_points[i * 5 + 4] = mesh->texcoords[i].v;
    }
    for (i = 0; b.color_points && i < mesh->point_count; ++i) {
        b.color_points[i].x = mesh->points[i].x;
        b.color_points[i].y = mesh->points[i].y;
        b.color_points[i].z = mesh->points[i].z;
    }
    for (i = 0; b.lit_points && i < mesh->point_count; ++i) {
        b.lit_points[i].x = mesh->points[i].x;
        b.lit_points[i].y = mesh->points[i].y;
        b.lit_points[i].z = mesh->points[i].z;
        b.lit_points[i].normal.x = mesh->vertex_normals[i].x;
        b.lit_points[i].normal.y = mesh->vertex_normals[i].y;
        b.lit_points[i].normal.z = mesh->vertex_normals[i].z;
    }
    for (i = 0; b.lit_facets && i < mesh->triangles; ++i) {
        b.lit_facets[i].normal.x = mesh->facet_normals[i].x;
        b.lit_facets[i].normal.y = mesh->facet_normals[i].y;
        b.lit_facets[i].normal.z = mesh->facet_normals[i].z;
    }
    for (i = 0; b.strip_points && i < mesh->strip_point_count; ++i) {
        b.strip_points[i].x = mesh->strip_points[i].x;
        b.strip_points[i].y = mesh->strip_points[i].y;
        b.strip_points[i].z = mesh->strip_points[i].z;
    }
    for (i = 0; b.lists && i < mesh->triangles; ++i) {
        b.lists[i].pt_type = XGL_PT_F3D;
        b.lists[i].num_pts = 3;
        b.lists[i].num_data_values = 0;
        if (b.texture_points) {
            b.lists[i].pt_type = XGL_PT_DATA_F3D;
            b.lists[i].num_data_values = 2;
            b.lists[i].pts.data_f3d = (Xgl_pt_data_f3d *)(b.texture_points + i * 15);
        } else if (b.lit_points) {
            b.lists[i].pt_type = XGL_PT_COLOR_NORMAL_F3D;
            b.lists[i].pts.color_normal_f3d = &b.lit_points[i * 3];
        } else if (b.color_points) {
            b.lists[i].pt_type = XGL_PT_COLOR_F3D;
            b.lists[i].pts.color_f3d = &b.color_points[i * 3];
        } else b.lists[i].pts.f3d = &b.points[i * 3];
    }
    for (i = 0, start = 0; b.batch_facets && start < mesh->triangles;
         ++i, start += count) {
        count = mesh->triangles - start;
        if (count > (size_t)b.batch) count = (size_t)b.batch;
        b.batch_facets[i].facet_type = XGL_FACET_COLOR;
        b.batch_facets[i].num_facets = (Xgl_usgn32)count;
        if (b.lit_facets) {
            b.batch_facets[i].facet_type = XGL_FACET_COLOR_NORMAL;
            b.batch_facets[i].facets.color_normal_facets = b.lit_facets + start;
        } else b.batch_facets[i].facets.color_facets = b.facets + start;
    }
    xb_backend_colors(mesh);
    return 1;
}

void xb_backend_colors(const xb_mesh *mesh)
{
    size_t i, start, count;
    int batch;
    if (xb_cancel_requested()) return;
    if (!mesh || mesh->triangles != b.triangles) return;
    for (i = 0; b.shading != XB_SMOOTH && i < b.triangles; ++i) {
        b.facets[i].color.rgb.r = mesh->colors[i].r;
        b.facets[i].color.rgb.g = mesh->colors[i].g;
        b.facets[i].color.rgb.b = mesh->colors[i].b;
        if (b.lit_facets) b.lit_facets[i].color = b.facets[i].color;
    }
    for (i = 0; (b.color_points || b.lit_points) && i < b.point_count; ++i) {
        Xgl_color *color;
        color = b.lit_points ? &b.lit_points[i].color : &b.color_points[i].color;
        color->rgb.r = mesh->vertex_colors[i].r;
        color->rgb.g = mesh->vertex_colors[i].g;
        color->rgb.b = mesh->vertex_colors[i].b;
    }
    if (b.method != XB_TLIST) return;
    batch = b.batch > 0 ? b.batch : 128;
    for (start = 0; start < b.triangles; start += count) {
        count = b.triangles - start;
        if (count > (size_t)batch) count = (size_t)batch;
        /* First count records carry actual per-triangle colors. Remaining
         * records satisfy the documented num_points-2 minimum. Validation
         * rejects drivers that interpret restart facet indexing differently.
         */
        for (i = 0; i < count * 3; ++i)
            b.restart_facets[start * 3 + i] = b.facets[start +
                                                (i < count ? i : count - 1)];
    }
}

size_t xb_backend_input_bytes(enum xb_method method, int batch)
{
    size_t i, count, batches, points, facets;
    if (method != b.method) return 0;
    if (b.texture_points) return b.point_count * 5 * sizeof(float);
    if (b.lit_points) return b.point_count * sizeof(*b.lit_points);
    if (b.color_points) return b.point_count * sizeof(*b.color_points);
    if (b.lit_facets)
        return b.point_count * sizeof(*b.points) + b.triangles * sizeof(*b.lit_facets);
    if (batch <= 0) batch = 128;
    points = b.point_count;
    facets = b.triangles;
    if (method == XB_STRIP) {
        points = facets = 0;
        for (i = 0; i < b.strip_count; ++i) {
            count = b.strips[i].point_count - 2;
            batches = (count + (size_t)batch - 1) / (size_t)batch;
            points += count + 2 * batches;
            facets += count;
        }
    } else if (method == XB_TLIST) {
        batches = (b.triangles + (size_t)batch - 1) / (size_t)batch;
        facets = 3 * b.triangles - 2 * batches;
    }
    return points * sizeof(Xgl_pt_f3d) + facets * sizeof(Xgl_color_facet);
}

static int allocate_stream_slot(int index, const xb_mesh *mesh)
{
    xb_native_slot *slot;
    size_t i, start, count, batches;
    slot = &b.stream[index];
    slot->triangles = mesh->triangles;
    slot->point_count = mesh->point_count;
    batches = (mesh->triangles + (size_t)b.stream_batch - 1) /
              (size_t)b.stream_batch;
    if (b.shading == XB_SMOOTH)
        slot->color_points = (Xgl_pt_color_f3d *)native_calloc(mesh->point_count,
                                                      sizeof(*slot->color_points));
    else slot->points = (Xgl_pt_f3d *)native_calloc(mesh->point_count, sizeof(*slot->points));
    slot->lists = (Xgl_pt_list *)native_calloc(mesh->triangles, sizeof(*slot->lists));
    if (b.shading != XB_SMOOTH) {
        slot->facets = (Xgl_color_facet *)native_calloc(mesh->triangles, sizeof(*slot->facets));
        slot->batch_facets = (Xgl_facet_list *)native_calloc(batches, sizeof(*slot->batch_facets));
    }
    if (!(slot->points || slot->color_points) || !slot->lists ||
        (b.shading != XB_SMOOTH && (!slot->facets || !slot->batch_facets))) {
        fail("out of memory allocating stable native stream slots");
        return 0;
    }
    for (i = 0; i < mesh->point_count; ++i) {
        if (slot->color_points) {
            slot->color_points[i].x = mesh->points[i].x;
            slot->color_points[i].y = mesh->points[i].y;
            slot->color_points[i].z = mesh->points[i].z;
            slot->color_points[i].color.rgb.r = mesh->vertex_colors[i].r;
            slot->color_points[i].color.rgb.g = mesh->vertex_colors[i].g;
            slot->color_points[i].color.rgb.b = mesh->vertex_colors[i].b;
        } else {
            slot->points[i].x = mesh->points[i].x;
            slot->points[i].y = mesh->points[i].y;
            slot->points[i].z = mesh->points[i].z;
        }
    }
    for (i = 0; i < mesh->triangles; ++i) {
        slot->lists[i].num_pts = 3;
        if (slot->color_points) {
            slot->lists[i].pt_type = XGL_PT_COLOR_F3D;
            slot->lists[i].pts.color_f3d = slot->color_points + i * 3;
        } else {
            slot->lists[i].pt_type = XGL_PT_F3D;
            slot->lists[i].pts.f3d = slot->points + i * 3;
            slot->facets[i].color.rgb.r = mesh->colors[i].r;
            slot->facets[i].color.rgb.g = mesh->colors[i].g;
            slot->facets[i].color.rgb.b = mesh->colors[i].b;
        }
    }
    for (i = 0, start = 0; slot->batch_facets && start < mesh->triangles;
         ++i, start += count) {
        count = mesh->triangles - start;
        if (count > (size_t)b.stream_batch) count = (size_t)b.stream_batch;
        slot->batch_facets[i].facet_type = XGL_FACET_COLOR;
        slot->batch_facets[i].num_facets = (Xgl_usgn32)count;
        slot->batch_facets[i].facets.color_facets = slot->facets + start;
    }
    slot->prepared = 1;
    return 1;
}

int xb_backend_stream_prepare(int index, const xb_mesh *mesh)
{
    xb_native_slot *slot;
    size_t i;
    if (b.errors || b.fatal || xb_cancel_requested()) return 0;
    if (!b.stream_active || index < 0 || index >= b.stream_slots) {
        fail("invalid native stream slot");
        return 0;
    }
    slot = &b.stream[index];
    if (slot->pending) {
        fail("stream slot is pending; synchronize before replacing its colors");
        return 0;
    }
    if (!mesh || (slot->color_points ? !mesh->vertex_colors : !mesh->colors) ||
        mesh->triangles != slot->triangles ||
        mesh->point_count != slot->point_count) {
        fail("stream preparation requires the original static geometry dimensions");
        return 0;
    }
    /* No allocation or source-mesh reference survives this call. */
    for (i = 0; slot->facets && i < slot->triangles; ++i) {
        slot->facets[i].color.rgb.r = mesh->colors[i].r;
        slot->facets[i].color.rgb.g = mesh->colors[i].g;
        slot->facets[i].color.rgb.b = mesh->colors[i].b;
    }
    for (i = 0; slot->color_points && i < slot->point_count; ++i) {
        slot->color_points[i].color.rgb.r = mesh->vertex_colors[i].r;
        slot->color_points[i].color.rgb.g = mesh->vertex_colors[i].g;
        slot->color_points[i].color.rgb.b = mesh->vertex_colors[i].b;
    }
    slot->prepared = 1;
    return 1;
}

static size_t draw_stream_slot(int index, int clear)
{
    xb_native_slot *slot;
    size_t start, count, calls;
    if (b.errors || b.fatal || xb_cancel_requested()) return 0;
    if (!b.stream_active || index < 0 || index >= b.stream_slots) {
        fail("invalid native stream slot");
        return 0;
    }
    slot = &b.stream[index];
    if (slot->pending || !slot->prepared) {
        fail("stream slot must be initialized and completed before submission");
        return 0;
    }
    /* Mark before any native operation: failure cannot make its references
     * safe to overwrite. Only a successful completion clears this flag.
     */
    slot->pending = 1;
    if (clear) xb_backend_clear(1, 1);
    calls = 0;
    for (start = 0; start < slot->triangles && !b.errors && !xb_cancel_requested(); start += count) {
        count = slot->triangles - start;
        if (count > (size_t)b.stream_batch) count = (size_t)b.stream_batch;
        xgl_multi_simple_polygon(b.ctx,
                XGL_FACET_FLAG_SIDES_ARE_3 | XGL_FACET_FLAG_SHAPE_CONVEX,
                slot->color_points ? NULL : &slot->batch_facets[calls],
                NULL, (Xgl_usgn32)count,
                slot->lists + start);
        ++calls;
    }
    return b.errors || xb_cancel_requested() ? 0 : calls;
}

size_t xb_backend_stream_draw(int index)
{
    return draw_stream_slot(index, 1);
}

void xb_backend_post(void)
{
    if (b.ctx && !b.errors) xgl_context_flush(b.ctx, XGL_FLUSH_BUFFERS);
}

int xb_backend_stream_drain(void)
{
    int i;
    if (!b.ctx || b.errors || b.fatal) return 0;
    xgl_context_flush(b.ctx, XGL_FLUSH_SYNCHRONIZE);
    if (b.errors) return 0;
    for (i = 0; i < b.stream_slots; ++i) b.stream[i].pending = 0;
    return 1;
}

void xb_backend_stream_end(void)
{
    if (!b.stream_active) return;
    if (!xb_backend_stream_drain()) return;
    xgl_object_set(b.ctx, XGL_CTX_GEOM_DATA_IS_VOLATILE,
                   b.stream_previous_volatile, 0);
    if (b.errors) return;
    free_stream_slots();
    b.stream_active = 0;
}

int xb_backend_stream_begin(const xb_mesh *mesh, int slots,
                            char *reason, size_t size)
{
    int i;
    if (xb_cancel_requested()) {
        copy_text(reason, size, "profile time budget reached");
        return 0;
    }
    if (b.errors || b.fatal || b.quit) goto bad;
    xb_backend_stream_end();
    if (b.stream_active || b.errors) goto bad;
    if (b.method != XB_MULTI || b.hardware_lighting ||
        b.texture_filter != XB_TEXTURE_OFF ||
        slots < 1 || slots > XB_MAX_QUEUE || !mesh ||
        !mesh->triangles || mesh->point_count != mesh->triangles * 3 ||
        !mesh->points || (b.shading == XB_SMOOTH ? !mesh->vertex_colors : !mesh->colors)) {
        fail("streaming requires CPU facet/vertex RGB, multi-polygon geometry and 1..8 stable slots");
        goto bad;
    }
    xb_backend_sync();
    if (b.errors || xb_cancel_requested()) goto bad;
    xgl_object_get(b.ctx, XGL_CTX_GEOM_DATA_IS_VOLATILE,
                   &b.stream_previous_volatile);
    if (b.errors || xb_cancel_requested()) goto bad;
    /* Record ownership before changing state so error recovery restores it. */
    b.stream_active = 1;
    b.stream_batch = b.batch;
    xgl_object_set(b.ctx, XGL_CTX_GEOM_DATA_IS_VOLATILE, FALSE, 0);
    if (b.errors || xb_cancel_requested() || !validate_stream()) goto bad;
    free_stream_slots(); /* probe has completed successfully */
    b.stream_slots = slots;
    for (i = 0; i < slots; ++i)
        if (xb_cancel_requested() || !allocate_stream_slot(i, mesh)) goto bad;
    copy_text(reason, size, b.shading == XB_SMOOTH ?
              "queued vertex colors, interpolation and per-item color/Z clear readback passed" :
              "independent queued facet colors and per-item color/Z clear readback passed");
    return 1;
bad:
    copy_text(reason, size, xb_cancel_requested() ? "profile time budget reached" : b.error);
    /* On a native failure end deliberately keeps arrays alive for a later
     * reconfiguration recovery or, ultimately, xgl_close.
     */
    xb_backend_stream_end();
    return 0;
}

int xb_backend_lighting(const xb_light_state *state)
{
    Xgl_color color;
    Xgl_pt_f3d direction;
    Xgl_pt_d3d position;
    int i;
    if (b.errors || b.fatal || xb_cancel_requested()) return 0;
    if (!b.hardware_lighting || !state || state->count != b.light_count ||
        state->count < 1 || state->count > XB_MAX_LIGHTS) {
        fail("light state does not match the configured native light count");
        return 0;
    }
    memset(&color, 0, sizeof(color));
    color.rgb.r = color.rgb.g = color.rgb.b = state->ambient;
    xgl_object_set(b.lights[0], XGL_LIGHT_TYPE, XGL_LIGHT_AMBIENT,
                   XGL_LIGHT_COLOR, &color, 0);
    direction.x = state->direction.x;
    direction.y = state->direction.y;
    direction.z = state->direction.z;
    for (i = 0; i < state->count && !b.errors && !xb_cancel_requested(); ++i) {
        color.rgb.r = state->color[i].r;
        color.rgb.g = state->color[i].g;
        color.rgb.b = state->color[i].b;
        if (state->directional) {
            xgl_object_set(b.lights[i + 1], XGL_LIGHT_TYPE, XGL_LIGHT_DIRECTIONAL,
                XGL_LIGHT_COLOR, &color, XGL_LIGHT_DIRECTION, &direction, 0);
        } else {
            /* Reference pp.314-315 deliberately use different point types:
             * direction is Xgl_pt_f3d; position MUST be Xgl_pt_d3d. */
            position.x = state->position[i].x;
            position.y = state->position[i].y;
            position.z = state->position[i].z;
            xgl_object_set(b.lights[i + 1], XGL_LIGHT_TYPE, XGL_LIGHT_POSITIONAL,
                XGL_LIGHT_COLOR, &color, XGL_LIGHT_POSITION, &position,
                XGL_LIGHT_ATTENUATION_1, (double)state->attenuation1,
                XGL_LIGHT_ATTENUATION_2, (double)state->attenuation2, 0);
        }
    }
    if (b.errors || xb_cancel_requested()) return 0;
    b.light_state = *state;
    return 1;
}

static int configure_lighting(const xb_case *test)
{
    xb_light_state state;
    Xgl_boolean switches[XB_MAX_LIGHTS + 1];
    int i, illumination;
    b.shading = test->shade;
    b.hardware_lighting = test->xgl_lighting;
    b.light_count = test->xgl_lighting ? test->lights : 0;
    if ((test->shade != XB_FLAT || test->lighting != XB_LIGHT_OFF) &&
        (test->method != XB_MULTI && test->method != XB_POLYGON)) {
        fail_kind(XB_FAILURE_UNSUPPORTED, "lighting and smooth-color cases require polygon or multi-polygon draws");
        return 0;
    }
    if (test->work == XB_PIPELINE &&
        (test->method != XB_MULTI || test->xgl_lighting)) {
        fail_kind(XB_FAILURE_UNSUPPORTED, "streaming requires multi-polygon CPU colors; hardware-lighting streams are not validated");
        return 0;
    }
    if (b.hardware_lighting && (test->lighting != XB_LIGHT_LAMBERT ||
        b.light_count < 1 || b.light_count > XB_MAX_LIGHTS)) {
        fail_kind(XB_FAILURE_UNSUPPORTED, "native lighting requires the Lambert model and 1..4 diffuse lights");
        return 0;
    }
    if (b.hardware_lighting && b.inquiry.lighting != XGL_INQ_HARDWARE &&
        (!b.allow_fallback || b.inquiry.lighting == XGL_INQ_NOT_SUPPORTED)) {
        fail_kind(XB_FAILURE_UNSUPPORTED, "XGL does not report hardware lighting; use -allow-fallback for supported software lighting");
        return 0;
    }
    illumination = b.hardware_lighting ?
        (b.shading == XB_SMOOTH ? XGL_ILLUM_PER_VERTEX : XGL_ILLUM_PER_FACET) :
        (b.shading == XB_SMOOTH ? XGL_ILLUM_NONE_INTERP_COLOR : XGL_ILLUM_NONE);
    /* Explicit reset is required when a lit/interpolated case is followed by
     * one of the original unlit tests, including the next resolution's gates.
     * Default LIGHTING_NORMAL_FLIP is TRUE; disable it to match the supplied
     * unit normals used by the CPU reference (Reference pp.114-115). */
    xgl_object_set(b.ctx,
        XGL_CTX_SURF_FRONT_COLOR_SELECTOR, b.shading == XB_SMOOTH ?
            XGL_SURF_COLOR_VERTEX_ILLUM_DEP : XGL_SURF_COLOR_FACET,
        XGL_3D_CTX_SURF_FRONT_ILLUMINATION, illumination,
        XGL_3D_CTX_SURF_NORMAL_FLIP, FALSE,
        XGL_3D_CTX_SURF_LIGHTING_NORMAL_FLIP, FALSE,
        XGL_3D_CTX_SURF_FRONT_AMBIENT, 1.0,
        XGL_3D_CTX_SURF_FRONT_DIFFUSE, 1.0,
        XGL_3D_CTX_SURF_FRONT_SPECULAR, 0.0,
        XGL_3D_CTX_SURF_FRONT_LIGHT_COMPONENT,
            XGL_LIGHT_ENABLE_COMP_AMBIENT | XGL_LIGHT_ENABLE_COMP_DIFFUSE,
        XGL_3D_CTX_LIGHT_NUM, (Xgl_usgn32)(b.hardware_lighting ? b.light_count + 1 : 0), 0);
    if (b.errors) return 0;
    memset(b.lights, 0, sizeof(b.lights));
    if (!b.hardware_lighting) return 1;
    xgl_object_get(b.ctx, XGL_3D_CTX_LIGHTS, b.lights);
    if (b.errors) return 0;
    for (i = 0; i <= b.light_count; ++i) {
        if (!b.lights[i]) { fail("XGL returned a missing Light object"); return 0; }
        switches[i] = TRUE;
    }
    xgl_object_set(b.ctx, XGL_3D_CTX_LIGHT_SWITCHES, switches, 0);
    if (b.errors) return 0;
    xb_lighting_state(test, 0, &state);
    return xb_backend_lighting(&state);
}

int xb_backend_configure(const xb_case *test, const xb_mesh *mesh,
                         char *reason, size_t size)
{
    if (xb_cancel_requested()) {
        b.failure = XB_FAILURE_TIME_LIMIT;
        copy_text(reason, size, "profile time budget reached");
        return 0;
    }
    if (b.fatal || b.quit) { copy_text(reason, size, b.error); return 0; }
    /* A recoverable API error aborts its case. Reset only at an explicit
     * reconfiguration boundary; nonrecoverable errors never resume drawing.
     */
    b.errors = 0;
    b.failure = XB_FAILURE_NONE;
    b.error[0] = '\0';
    b.stage = "case reconfiguration";
    /* Complete any retained data from a preceding recoverable error before
     * changing attributes or releasing the previous native mesh arrays.
     */
    xb_backend_sync();
    if (xb_cancel_requested()) {
        b.failure = XB_FAILURE_TIME_LIMIT;
        copy_text(reason, size, "profile time budget reached");
        return 0;
    }
    if (b.errors) { copy_text(reason, size, b.error); return 0; }
    if (!texture_enable(0)) { copy_text(reason, size, b.error); return 0; }
    b.texture_filter = XB_TEXTURE_OFF;
    xb_backend_stream_end();
    if (b.stream_active || b.errors) {
        copy_text(reason, size, b.error);
        return 0;
    }
    b.batch = test->batch > 0 ? test->batch : 128;
    b.method = test->method;
    b.perspective = test->perspective;
    b.cull = test->cull;
    if (b.cull < 0 || b.cull > 1 ||
        (b.cull && (test->method != XB_MULTI || test->shade != XB_FLAT ||
                    test->lighting != XB_LIGHT_OFF || test->work == XB_PIPELINE))) {
        b.failure = XB_FAILURE_UNSUPPORTED;
        copy_text(reason, size, "back-face culling is validated only for unlit flat multi-polygon cases");
        return 0;
    }
    if (test->retained && (test->method != XB_MULTI || test->update != XB_STATIC)) {
        b.failure = XB_FAILURE_UNSUPPORTED;
        copy_text(reason, size, "retained-array test requires static multi-polygon geometry");
        return 0;
    }
    if ((test->work == XB_SWAP || test->work == XB_FRAME || test->work == XB_SCENE_FRAME) && b.buffers < 2) {
        b.failure = XB_FAILURE_UNSUPPORTED;
        copy_text(reason, size, "two window buffers were not allocated");
        return 0;
    }
    b.current_z = test->zbuffer;
    b.stage = "opaque state reset";
    if (!set_transparency(0, 1.0, 1)) {
        copy_text(reason, size, b.error);
        return 0;
    }
    xgl_object_set(b.ctx, XGL_CTX_GEOM_DATA_IS_VOLATILE,
                   test->retained ? FALSE : TRUE,
                   XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_OFF,
                   XGL_3D_CTX_HLHSR_MODE,
                   b.current_z ? XGL_HLHSR_Z_BUFFER : XGL_HLHSR_NONE, 0);
    identity(b.model);
    view_matrix(test->perspective);
    b.stage = "lighting setup/readback";
    if (!configure_lighting(test) || !validate_lighting()) {
        copy_text(reason, size, b.error);
        return 0;
    }
    b.stage = "culling readback";
    if (b.cull && !validate_culling()) {
        copy_text(reason, size, b.error);
        return 0;
    }
    xgl_object_set(b.ctx, XGL_3D_CTX_SURF_FACE_CULL,
                   b.cull ? XGL_CULL_BACK : XGL_CULL_OFF, 0);
    b.stage = "texture setup/readback";
    if (!configure_texture(test)) {
        copy_text(reason, size, b.error);
        return 0;
    }
    if (test->work == XB_COLOR_DEPTH_CLEAR) {
        b.stage = "combined color/Z clear readback";
        if (!validate_combined_clear()) {
            copy_text(reason, size, b.error);
            return 0;
        }
    }
    if (test->transparency) {
        if (test->transparency != 1 || !(test->opacity >= 0 && test->opacity <= 1) ||
            (test->blend_zwrite != 0 && test->blend_zwrite != 1) ||
            test->method != XB_MULTI || test->work != XB_DRAW || !test->zbuffer ||
            test->shade != XB_FLAT || test->lighting != XB_LIGHT_OFF ||
            test->texture_filter != XB_TEXTURE_OFF || test->cull ||
            test->retained || test->streaming) {
            fail_kind(XB_FAILURE_UNSUPPORTED,
                "blending requires unlit flat multi-polygon draws with Z and no texture/culling/retention/streaming");
            copy_text(reason, size, b.error);
            return 0;
        }
        if (b.inquiry.transparency != XGL_INQ_HARDWARE &&
            (!b.allow_fallback || b.inquiry.transparency == XGL_INQ_NOT_SUPPORTED)) {
            fail_kind(XB_FAILURE_UNSUPPORTED,
                "XGL does not report hardware transparency; use -allow-fallback for a diagnostic path");
            copy_text(reason, size, b.error);
            return 0;
        }
        b.stage = "blending opacity/order/Z readback";
        if (!validate_transparency() ||
            !set_transparency(1, (double)test->opacity, test->blend_zwrite)) {
            copy_text(reason, size, b.error);
            return 0;
        }
    }
    b.stage = "native geometry upload";
    if (mesh && mesh->triangles && !xb_backend_upload(mesh)) {
        copy_text(reason, size, b.error);
        return 0;
    }
    xb_backend_clear(1, 1);
    xb_backend_sync();
    if (xb_cancel_requested()) {
        b.failure = XB_FAILURE_TIME_LIMIT;
        copy_text(reason, size, "profile time budget reached");
        return 0;
    }
    if (b.errors) { copy_text(reason, size, b.error); return 0; }
    if (test->work == XB_RASTER || test->work == XB_MASKED_RASTER ||
        test->work == XB_FRAME) {
        b.stage = "HUD raster readback";
        if (!validate_raster(0) ||
            ((test->work == XB_MASKED_RASTER || test->work == XB_FRAME) &&
             !validate_raster(1))) {
            copy_text(reason, size, b.error);
            return 0;
        }
        xb_backend_clear(1, 1);
        xb_backend_sync();
    }
    if (test->work == XB_COLOR_DEPTH_CLEAR)
        copy_text(reason, size, "combined color/Z clear readback passed");
    if (test->transparency)
        copy_text(reason, size, b.inquiry.transparency == XGL_INQ_HARDWARE ?
            "blended opacity/order/Z readback passed; transparency reported in hardware" :
            "blended opacity/order/Z readback passed; diagnostic transparency fallback");
    if (test->xgl_lighting && b.inquiry.lighting != XGL_INQ_HARDWARE)
        copy_text(reason, size,
            "diagnostic fallback: lighting not reported in hardware; readback passed");
    if (b.texture_filter != XB_TEXTURE_OFF)
        copy_text(reason, size,
            "texture RGB/UV/wrap/filter/off gates passed; execution unclassified, may use software");
    if (xb_cancel_requested()) {
        b.failure = XB_FAILURE_TIME_LIMIT;
        copy_text(reason, size, "profile time budget reached");
        return 0;
    }
    b.stage = "rendering";
    return 1;
}

void xb_backend_transform(unsigned long frame)
{
    Xgl_matrix_f3d matrix;
    double angle, c, s;
    if (b.errors || xb_cancel_requested()) return;
    angle = (double)(frame % 360) * 0.017453292519943295;
    c = cos(angle); s = sin(angle);
    memset(matrix, 0, sizeof(matrix));
    matrix[0][0] = (float)c;
    matrix[0][1] = (float)s;
    matrix[1][0] = (float)-s;
    matrix[1][1] = (float)c;
    matrix[2][2] = matrix[3][3] = 1.0f;
    xgl_transform_write(b.model, matrix);
}

void xb_backend_clear(int color, int depth)
{
    int actions;
    if (b.errors || xb_cancel_requested()) return;
    actions = 0;
    if (color) actions |= XGL_CTX_NEW_FRAME_CLEAR;
    if (depth) actions |= XGL_CTX_NEW_FRAME_HLHSR_ACTION;
    if (depth && !b.current_z)
        xgl_object_set(b.ctx, XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_Z_BUFFER, 0);
    if (b.errors || xb_cancel_requested()) return;
    xgl_object_set(b.ctx, XGL_CTX_NEW_FRAME_ACTION, actions, 0);
    if (b.errors || xb_cancel_requested()) return;
    xgl_context_new_frame(b.ctx);
    if (b.errors || xb_cancel_requested()) return;
    if (depth && !b.current_z)
        xgl_object_set(b.ctx, XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_NONE, 0);
}

size_t xb_backend_draw(enum xb_method method, int batch)
{
    size_t start, count, calls, i, offset, available;
    Xgl_facet facet;
    Xgl_facet_list facets;
    Xgl_pt_list points;
    if (b.errors || xb_cancel_requested()) return 0;
    if (method != b.method) {
        fail("draw method differs from configured native geometry representation");
        ++b.errors;
        return 0;
    }
    if (batch <= 0) batch = 128;
    calls = 0;
    memset(&facets, 0, sizeof(facets));
    memset(&points, 0, sizeof(points));
    facets.facet_type = XGL_FACET_COLOR;
    points.pt_type = XGL_PT_F3D;
    if (method == XB_POLYGON) {
        for (i = 0; i < b.triangles && !b.errors && !xb_cancel_requested(); ++i) {
            if (b.shading == XB_SMOOTH)
                xgl_polygon(b.ctx, XGL_FACET_NONE, NULL, NULL, 1, &b.lists[i]);
            else if (b.lit_facets) {
                facet.color_normal_facet = b.lit_facets[i];
                xgl_polygon(b.ctx, XGL_FACET_COLOR_NORMAL, &facet, NULL, 1, &b.lists[i]);
            } else {
                facet.color_facet = b.facets[i];
                xgl_polygon(b.ctx, XGL_FACET_COLOR, &facet, NULL, 1, &b.lists[i]);
            }
        }
        return xb_cancel_requested() ? 0 : i;
    }
    if (method == XB_STRIP) {
        for (i = 0; i < b.strip_count && !b.errors && !xb_cancel_requested(); ++i) {
            available = b.strips[i].point_count - 2;
            for (offset = 0; offset < available && !b.errors && !xb_cancel_requested(); offset += count) {
                count = available - offset;
                if (count > (size_t)batch) count = (size_t)batch;
                points.num_pts = (Xgl_usgn32)(count + 2);
                points.pts.f3d = b.strip_points + b.strips[i].first_point + offset;
                facets.num_facets = (Xgl_usgn32)count;
                facets.facets.color_facets = b.facets +
                                            b.strips[i].first_facet + offset;
                xgl_triangle_strip(b.ctx, &facets, &points);
                ++calls;
            }
        }
        return xb_cancel_requested() ? 0 : calls;
    }
    for (start = 0; start < b.triangles && !b.errors && !xb_cancel_requested(); start += count) {
        count = b.triangles - start;
        if (count > (size_t)batch) count = (size_t)batch;
        facets.num_facets = (Xgl_usgn32)count;
        facets.facets.color_facets = b.facets ? b.facets + start : NULL;
        if (method == XB_MULTI) {
            xgl_multi_simple_polygon(b.ctx,
                XGL_FACET_FLAG_SIDES_ARE_3 | XGL_FACET_FLAG_SHAPE_CONVEX,
                b.shading == XB_SMOOTH || b.texture_filter != XB_TEXTURE_OFF ?
                    NULL : &b.batch_facets[calls],
                NULL, (Xgl_usgn32)count, b.lists + start);
        } else {
            points.num_pts = (Xgl_usgn32)(count * 3);
            points.pts.f3d = b.points + start * 3;
            facets.num_facets = (Xgl_usgn32)(count * 3 - 2);
            facets.facets.color_facets = b.restart_facets + start * 3;
            xgl_triangle_list(b.ctx, &facets, &points,
                              XGL_TLIST_FLAG_TRI_RESTART);
        }
        ++calls;
    }
    return xb_cancel_requested() ? 0 : calls;
}

void xb_backend_sync(void)
{
    if (b.ctx && !b.errors) xgl_context_flush(b.ctx, XGL_FLUSH_SYNCHRONIZE);
}

void xb_backend_swap(void)
{
    if (b.buffers < 2 || b.errors || xb_cancel_requested()) return;
    /* Complete queued geometry and raster copies before presentation. */
    xb_backend_sync();
    if (b.errors || xb_cancel_requested()) return;
    xgl_object_set(b.ctx, XGL_CTX_NEW_FRAME_ACTION,
                   XGL_CTX_NEW_FRAME_SWITCH_BUFFER, 0);
    if (b.errors || xb_cancel_requested()) return;
    xgl_context_new_frame(b.ctx);
    if (!b.errors) xgl_context_flush(b.ctx, XGL_FLUSH_SYNCHRONIZE);
}

void xb_backend_raster(int masked)
{
    Xgl_pt_i2d position;
    int restore_texture;
    if (b.errors || xb_cancel_requested()) return;
    restore_texture = b.texture_switch;
    if (!texture_enable(0)) return;
    position.x = (b.width - XB_RASTER_W) / 2;
    position.y = b.height - XB_RASTER_H;
    xgl_object_set(b.ctx, XGL_CTX_RASTER_FILL_STYLE,
                   masked ? XGL_RAS_FILL_STIPPLE : XGL_RAS_FILL_COPY,
                   XGL_CTX_RASTER_FPAT_POSITION, &position, 0);
    if (!b.errors && !xb_cancel_requested()) xgl_context_copy_buffer(b.ctx, NULL, &position, b.hud);
    if (!b.errors && !xb_cancel_requested() && restore_texture) (void)texture_enable(1);
}

int xb_backend_events(void)
{
    XEvent event;
    int changed;
    changed = 0;
    while (b.display && XPending(b.display)) {
        XNextEvent(b.display, &event);
        if (event.type == ClientMessage &&
            (Atom)event.xclient.data.l[0] == b.wm_delete) {
            b.quit = 1;
            if (!b.errors) {
                b.error[0] = '\0';
                b.failure = XB_FAILURE_NONE;
            }
        }
        if (event.type == DestroyNotify) {
            b.quit = 1; b.window = 0;
            if (!b.errors) {
                b.error[0] = '\0';
                b.failure = XB_FAILURE_NONE;
            }
        }
        if (event.type == KeyPress &&
            XLookupKeysym(&event.xkey, 0) == XK_Escape) {
            b.quit = 1;
            if (!b.errors) {
                b.error[0] = '\0';
                b.failure = XB_FAILURE_NONE;
            }
        }
        if (b.quit) break;
        if (event.type == ConfigureNotify &&
            (event.xconfigure.width != b.width || event.xconfigure.height != b.height)) {
            xb_backend_sync();
            b.width = event.xconfigure.width;
            b.height = event.xconfigure.height;
            xgl_window_raster_resize(b.raster);
            setup_view();
            changed = 1;
        }
        if (event.type == Expose) changed = 1;
        if (event.type == VisibilityNotify) {
            b.obscured = event.xvisibility.state != VisibilityUnobscured;
            changed = 1;
        }
    }
    if (b.quit || b.fatal) return 0;
    if (b.errors) return -1;
    if (b.obscured)
        fail("window is obscured; keep the entire drawable visible for valid timing");
    return changed || b.obscured ? -1 : 1;
}

void xb_backend_label(const char *name)
{
    char title[160];
    xb_backend_sync();
    sprintf(title, "xglbench %dx%d: %.90s (Escape stops)", b.width, b.height, name);
    XStoreName(b.display, b.window, title);
    XSync(b.display, False);
}

int xb_backend_has_double_buffer(void) { return b.buffers >= 2; }
int xb_backend_has_hardware(void) { return b.hardware; }
const char *xb_backend_error(void) { return b.error; }
enum xb_failure xb_backend_failure(void)
{
    return b.failure != XB_FAILURE_NONE ? b.failure :
           (b.error[0] ? XB_FAILURE_ERROR : XB_FAILURE_NONE);
}

/* Transparency is a surface property, distinct from CPU RGB interpolation.
 * The normal alpha convention is exposed to the harness; XGL takes the
 * complementary transparency (zero means opaque). Arbitrary-background
 * blending is proved by readback before any performance result is accepted.
 */
static int set_transparency(int enabled, double opacity, int zwrite)
{
    if (b.errors || xb_cancel_requested()) return 0;
    xgl_object_set(b.ctx,
        XGL_3D_CTX_SURF_TRANSP_METHOD, enabled ? XGL_TRANSP_BLENDED : XGL_TRANSP_NONE,
        XGL_3D_CTX_SURF_TRANSP_BLEND_EQ, enabled ? XGL_BLEND_ARBITRARY_BG : XGL_BLEND_NONE,
        XGL_3D_CTX_SURF_FRONT_TRANSP, enabled ? 1.0 - opacity : 0.0,
        XGL_3D_CTX_SURF_BACK_TRANSP, enabled ? 1.0 - opacity : 0.0,
        XGL_3D_CTX_BLEND_FREEZE_Z_BUFFER, enabled && !zwrite ? TRUE : FALSE,
        XGL_3D_CTX_BLEND_DRAW_MODE, XGL_BLEND_DRAW_ALL,
        XGL_3D_CTX_Z_BUFFER_WRITE_MASK, (Xgl_usgn32)0xffffffffUL, 0);
    return !b.errors;
}

static int begin_color_probe(Xgl_boolean *previous_volatile, int *previous_z)
{
    *previous_volatile = TRUE;
    *previous_z = b.current_z;
    if (b.errors || xb_cancel_requested()) return 0;
    xgl_object_get(b.ctx, XGL_CTX_GEOM_DATA_IS_VOLATILE, previous_volatile);
    if (b.errors) return 0;
    b.current_z = 1;
    xgl_object_set(b.ctx,
        XGL_CTX_GEOM_DATA_IS_VOLATILE, TRUE,
        XGL_CTX_SURF_FRONT_COLOR_SELECTOR, XGL_SURF_COLOR_FACET,
        XGL_3D_CTX_SURF_FRONT_ILLUMINATION, XGL_ILLUM_NONE,
        XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_OFF,
        XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_Z_BUFFER,
        XGL_3D_CTX_Z_BUFFER_COMP_METHOD, XGL_Z_COMP_LESS_THAN_OR_EQUAL, 0);
    identity(b.model);
    view_matrix(0);
    return set_transparency(0, 1.0, 1);
}

static int end_color_probe(Xgl_boolean previous_volatile, int previous_z, int result)
{
    /* All probe arrays are volatile and every draw is completed before return.
     * Failed state is recovered at the next explicit configure boundary.
     */
    xb_backend_sync();
    b.current_z = previous_z;
    if (!b.errors && !xb_cancel_requested()) {
        if (!set_transparency(0, 1.0, 1)) return 0;
        xgl_object_set(b.ctx, XGL_CTX_GEOM_DATA_IS_VOLATILE, previous_volatile,
            XGL_3D_CTX_HLHSR_MODE, previous_z ? XGL_HLHSR_Z_BUFFER : XGL_HLHSR_NONE, 0);
        view_matrix(b.perspective);
    }
    return result && !b.errors && !xb_cancel_requested();
}

static int validation_triangle(float z, float r, float g, float blue)
{
    Xgl_pt_f3d points[3];
    Xgl_pt_list list;
    Xgl_color_facet color;
    Xgl_facet_list facets;
    if (b.errors || xb_cancel_requested()) return 0;
    memset(&list, 0, sizeof(list));
    memset(&facets, 0, sizeof(facets));
    memset(&color, 0, sizeof(color));
    points[0].x = -.8f; points[0].y = -.8f; points[0].z = z;
    points[1].x = .8f; points[1].y = -.8f; points[1].z = z;
    points[2].x = 0; points[2].y = .8f; points[2].z = z;
    list.pt_type = XGL_PT_F3D; list.num_pts = 3; list.pts.f3d = points;
    color.color.rgb.r = r; color.color.rgb.g = g; color.color.rgb.b = blue;
    facets.facet_type = XGL_FACET_COLOR; facets.num_facets = 1;
    facets.facets.color_facets = &color;
    xgl_multi_simple_polygon(b.ctx,
        XGL_FACET_FLAG_SIDES_ARE_3 | XGL_FACET_FLAG_SHAPE_CONVEX,
        &facets, NULL, 1, &list);
    xb_backend_sync();
    return !b.errors && !xb_cancel_requested();
}

static int probe_pixel(float r, float g, float blue, const char *stage)
{
    xb_rgb expected;
    expected.r = r; expected.g = g; expected.b = blue;
    if (pixel_matches(b.width / 2, b.height / 2, &expected)) return 1;
    if (!xb_cancel_requested()) error_context(stage);
    return 0;
}

static int validate_combined_clear(void)
{
    Xgl_boolean previous_volatile;
    int previous_z, result;
    result = 0;
    if (!begin_color_probe(&previous_volatile, &previous_z)) goto done;
    xb_backend_clear(1, 1);
    if (!validation_triangle(.2f, 1, 0, 0) ||
        !probe_pixel(1, 0, 0, "combined-clear initial color/Z")) goto done;
    xb_backend_clear(1, 1);
    xb_backend_sync();
    if (!probe_pixel(0, 0, 0, "combined-clear color reset")) goto done;
    if (!validation_triangle(.8f, 0, 1, 0) ||
        !probe_pixel(0, 1, 0, "combined-clear depth reset")) goto done;
    result = 1;
done:
    return end_color_probe(previous_volatile, previous_z, result);
}

static int validate_transparency(void)
{
    Xgl_boolean previous_volatile;
    int previous_z, result, step, zwrite, reverse;
    float opacity;
    result = 0;
    if (!begin_color_probe(&previous_volatile, &previous_z)) goto done;
    for (step = 0; step < 3; ++step) {
        opacity = (float)step * .5f;
        if (!set_transparency(0, 1.0, 1)) goto done;
        xb_backend_clear(1, 1);
        if (!validation_triangle(.8f, 0, 0, 1) ||
            !set_transparency(1, (double)opacity, 0) ||
            !validation_triangle(.2f, 1, 0, 0) ||
            !probe_pixel(opacity, 0, 1 - opacity, "blend opacity 0/0.5/1")) goto done;
    }
    /* A blended fragment must still fail against nearer opaque depth. */
    if (!set_transparency(0, 1.0, 1)) goto done;
    xb_backend_clear(1, 1);
    if (!validation_triangle(.2f, 1, 0, 0) ||
        !set_transparency(1, .5, 0) ||
        !validation_triangle(.8f, 0, 1, 0) ||
        !probe_pixel(1, 0, 0, "blended depth rejection")) goto done;
    /* Use the actual freeze flag with a full Z write mask: a separate zero
     * mask would conceal a broken FREEZE_Z_BUFFER implementation. */
    for (zwrite = 0; zwrite <= 1; ++zwrite) {
        if (!set_transparency(0, 1.0, 1)) goto done;
        xb_backend_clear(1, 1);
        if (!set_transparency(1, .5, zwrite) ||
            !validation_triangle(.2f, 1, 0, 0) ||
            !set_transparency(0, 1.0, 1) ||
            !validation_triangle(.5f, 0, 1, 0) ||
            !probe_pixel(zwrite ? .5f : 0, zwrite ? 0 : 1, 0,
                         "blended depth write/freeze")) goto done;
    }
    for (reverse = 0; reverse <= 1; ++reverse) {
        if (!set_transparency(0, 1.0, 1)) goto done;
        xb_backend_clear(1, 1);
        if (!set_transparency(1, .5, 0)) goto done;
        if (reverse) {
            if (!validation_triangle(.8f, 0, 0, 1) ||
                !validation_triangle(.2f, 1, 0, 0)) goto done;
        } else {
            if (!validation_triangle(.2f, 1, 0, 0) ||
                !validation_triangle(.8f, 0, 0, 1)) goto done;
        }
        if (!probe_pixel(reverse ? .5f : .25f, 0, reverse ? .25f : .5f,
                         "blended submission order")) goto done;
    }
    if (!set_transparency(0, 1.0, 1)) goto done;
    xb_backend_clear(1, 1);
    if (!validation_triangle(.5f, 0, 1, 0) ||
        !probe_pixel(0, 1, 0, "opaque state recovery")) goto done;
    result = 1;
done:
    return end_color_probe(previous_volatile, previous_z, result);
}

static int pixel_matches(int x, int y, const xb_rgb *expected)
{
    xb_rgb actual;
    char message[256];
    if (!read_pixel(x, y, &actual)) return 0;
    if (!(fabs(actual.r - expected->r) <= 0.065) ||
        !(fabs(actual.g - expected->g) <= 0.065) ||
        !(fabs(actual.b - expected->b) <= 0.065)) {
        /* Bounded general formats also handle unexpected non-finite values
         * without allowing a diagnostic to overflow its fixed buffer. */
        sprintf(message, "pixel (%d,%d): expected RGB (%.5g,%.5g,%.5g), got (%.5g,%.5g,%.5g)",
                x, y, (double)expected->r, (double)expected->g, (double)expected->b,
                (double)actual.r, (double)actual.g, (double)actual.b);
        fail_kind(XB_FAILURE_VALIDATION, message);
        return 0;
    }
    return 1;
}

static int lighting_probe_draw(const xb_mesh *mesh)
{
    if (b.errors || !xb_backend_upload(mesh)) return 0;
    xb_backend_clear(1, 1);
    if (!xb_backend_draw(b.method, b.batch)) return 0;
    xb_backend_sync();
    return !b.errors;
}

static int validate_culling(void)
{
    Xgl_pt_f3d points[6];
    Xgl_pt_list lists[2];
    Xgl_color_facet colors[2];
    Xgl_facet_list facets;
    Xgl_boolean previous_volatile;
    xb_rgb expected[2], black;
    int i, mode, px, py, result;
    /* With Y_DOWN_Z_AWAY, the viewer lies along negative Z. FIRST_POINTS
     * therefore makes the left (-Z) triangle front-facing and the right
     * (+Z) triangle back-facing. Use no supplied normal or lighting so this
     * gate tests exactly the winding convention of the timed culling cases.
     */
    if (xb_cancel_requested()) return 0;
    memset(points, 0, sizeof(points));
    memset(lists, 0, sizeof(lists));
    memset(colors, 0, sizeof(colors));
    memset(&facets, 0, sizeof(facets));
    memset(expected, 0, sizeof(expected));
    memset(&black, 0, sizeof(black));
    expected[0].r = 1.0f;
    expected[1].g = 1.0f;
    points[0].x = -.9f; points[0].y = -.6f;
    points[1].x = -.9f; points[1].y =  .6f;
    points[2].x = -.1f; points[2].y = -.6f;
    points[3].x =  .1f; points[3].y = -.6f;
    points[4].x =  .9f; points[4].y = -.6f;
    points[5].x =  .1f; points[5].y =  .6f;
    for (i = 0; i < 6; ++i) points[i].z = .5f;
    for (i = 0; i < 2; ++i) {
        lists[i].pt_type = XGL_PT_F3D;
        lists[i].num_pts = 3;
        lists[i].pts.f3d = points + i * 3;
        colors[i].color.rgb.r = expected[i].r;
        colors[i].color.rgb.g = expected[i].g;
        colors[i].color.rgb.b = expected[i].b;
    }
    facets.facet_type = XGL_FACET_COLOR;
    facets.num_facets = 2;
    facets.facets.color_facets = colors;
    previous_volatile = TRUE;
    xgl_object_get(b.ctx, XGL_CTX_GEOM_DATA_IS_VOLATILE, &previous_volatile);
    if (b.errors) return 0;
    /* The tiny probe uses stack descriptors. Unlike retained timed arrays,
     * these must never remain reachable after an error returns to our caller.
     */
    xgl_object_set(b.ctx, XGL_CTX_GEOM_DATA_IS_VOLATILE, TRUE, 0);
    if (b.errors) return 0;
    identity(b.model);
    view_matrix(0);
    result = 0;
    for (mode = 0; mode < 2; ++mode) {
        xgl_object_set(b.ctx, XGL_3D_CTX_SURF_FACE_CULL,
                       mode ? XGL_CULL_BACK : XGL_CULL_OFF, 0);
        if (b.errors) goto done;
        xb_backend_clear(1, 1);
        if (xb_cancel_requested()) goto done;
        xgl_multi_simple_polygon(b.ctx,
                XGL_FACET_FLAG_SIDES_ARE_3 | XGL_FACET_FLAG_SHAPE_CONVEX,
                &facets, NULL, 2, lists);
        xb_backend_sync();
        for (i = 0; i < 2; ++i) {
            px = (int)(((i ? .366666667 : -.633333333) + 1.0) *
                       .5 * (b.width - 1));
            py = (int)(.4 * (b.height - 1));
            if (!pixel_matches(px, py, mode && i ? &black : &expected[i])) {
                if (!xb_cancel_requested()) error_context("front/back winding readback");
                goto done;
            }
        }
    }
    result = 1;
done:
    /* Every draw completed before its local descriptors leave this scope. */
    xb_backend_sync();
    if (!b.errors) {
        xgl_object_set(b.ctx, XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_OFF,
                       XGL_CTX_GEOM_DATA_IS_VOLATILE, previous_volatile, 0);
        view_matrix(b.perspective);
    }
    return result && !b.errors;
}

static int validate_lighting(void)
{
    xb_mesh mesh;
    xb_point points[3], normals[3], facet_normal;
    xb_rgb corners[3], colors[1], expected, reflected[3];
    xb_light_state state, saved;
    double wx, wy, weight[3];
    int i, phase, px, py, result;
    if (b.shading == XB_FLAT && !b.hardware_lighting) return 1;
    memset(&mesh, 0, sizeof(mesh));
    mesh.triangles = 1; mesh.point_count = 3;
    mesh.points = points; mesh.colors = mesh.base_colors = colors;
    mesh.vertex_colors = corners; mesh.vertex_normals = normals;
    mesh.facet_normals = &facet_normal;
    facet_normal.x = facet_normal.y = 0; facet_normal.z = 1;
    for (i = 0; i < 3; ++i) {
        points[i].z = .5f; normals[i] = facet_normal;
    }
    colors[0].r = .9f; colors[0].g = .8f; colors[0].b = .7f;
    saved = b.light_state;
    memset(&state, 0, sizeof(state));
    state.count = b.light_count;
    state.directional = 1; state.direction.z = -1;
    state.ambient = .1f;
    for (i = 0; i < state.count; ++i) {
        state.color[i].r = .6f / (float)state.count;
        state.color[i].g = .6f / (float)state.count;
        state.color[i].b = .6f / (float)state.count;
    }
    result = 0;
    identity(b.model);
    view_matrix(0);
    if (b.hardware_lighting && !xb_backend_lighting(&state)) goto done;
    if (b.shading == XB_SMOOTH) {
        /* Primary-colored corners expose accidental flat shading, wrong color
         * selectors, missing color payloads and hardware vertex lighting that
         * shades a facet only once. Read an interior barycentric mixture. */
        points[0].x = -.8f; points[0].y = -.8f;
        points[1].x =  .8f; points[1].y = -.8f;
        points[2].x = -.8f; points[2].y =  .8f;
        memset(corners, 0, sizeof(corners));
        corners[0].r = .8f; corners[1].g = .8f; corners[2].b = .8f;
        px = (int)(.35 * (b.width - 1));
        py = (int)(.35 * (b.height - 1));
        wx = 2.0 * px / (b.width - 1) - 1.0;
        wy = 2.0 * py / (b.height - 1) - 1.0;
        weight[1] = (wx + .8) / 1.6;
        weight[2] = (wy + .8) / 1.6;
        weight[0] = 1.0 - weight[1] - weight[2];
        memset(&expected, 0, sizeof(expected));
        for (i = 0; i < 3; ++i) {
            /* Distinct unit normals also verify per-vertex normal fetches.
             * Material remains corner-specific only in this readback probe. */
            if (b.hardware_lighting && i) {
                normals[i].z = .6f;
                if (i == 1) normals[i].x = .8f;
                else normals[i].y = .8f;
            }
            reflected[i] = b.hardware_lighting ?
                xb_lighting_evaluate(&state, &points[i], &normals[i], &corners[i]) : corners[i];
            expected.r += (float)(weight[i] * reflected[i].r);
            expected.g += (float)(weight[i] * reflected[i].g);
            expected.b += (float)(weight[i] * reflected[i].b);
        }
        if (!lighting_probe_draw(&mesh) || !pixel_matches(px, py, &expected)) goto done;
        /* Timed CPU paths replace colors without reallocating positions. */
        for (i = 0; i < 3; ++i) corners[i] = colors[0];
        xb_backend_colors(&mesh);
        memset(&expected, 0, sizeof(expected));
        for (i = 0; i < 3; ++i) {
            reflected[i] = b.hardware_lighting ?
                xb_lighting_evaluate(&state, &points[i], &normals[i], &corners[i]) : corners[i];
            expected.r += (float)(weight[i] * reflected[i].r);
            expected.g += (float)(weight[i] * reflected[i].g);
            expected.b += (float)(weight[i] * reflected[i].b);
        }
        xb_backend_clear(1, 1);
        if (!xb_backend_draw(b.method, b.batch)) goto done;
        xb_backend_sync();
        if (!pixel_matches(px, py, &expected)) goto done;
    }
    if (!b.hardware_lighting) { result = 1; goto done; }
    /* Equilateral fixture: all corners are the same distance from lights on
     * its central normal. Flat lighting may choose any vertex (Guide p400),
     * and still has an exact reference value, without assuming a centroid. */
    points[0].x = -.649519053f; points[0].y = -.375f;
    points[1].x =  .649519053f; points[1].y = -.375f;
    points[2].x = 0; points[2].y = .75f;
    px = b.width / 2; py = b.height / 2;
    for (i = 0; i < 3; ++i) { normals[i] = facet_normal; corners[i] = colors[0]; }
    for (phase = 0; phase < 2; ++phase) {
        facet_normal.z = phase ? -1.0f : 1.0f;
        for (i = 0; i < 3; ++i) normals[i] = facet_normal;
        expected = xb_lighting_evaluate(&state, &points[0], &facet_normal, &colors[0]);
        if (!lighting_probe_draw(&mesh) || !pixel_matches(px, py, &expected)) goto done;
    }
    facet_normal.z = 1;
    for (i = 0; i < 3; ++i) normals[i] = facet_normal;
    state.directional = 0;
    state.attenuation1 = .25f; state.attenuation2 = .65f;
    for (i = 0; i < state.count; ++i) {
        state.color[i].r = .9f / (float)state.count;
        state.color[i].g = .75f / (float)state.count;
        state.color[i].b = .6f / (float)state.count;
    }
    for (phase = 0; phase < 2; ++phase) {
        for (i = 0; i < state.count; ++i)
            state.position[i].z = 1.5f + .2f * i + 2.0f * phase;
        if (!xb_backend_lighting(&state)) goto done;
        expected = xb_lighting_evaluate(&state, &points[0], &facet_normal, &colors[0]);
        if (!lighting_probe_draw(&mesh) || !pixel_matches(px, py, &expected)) goto done;
    }
    result = 1;
done:
    if (!b.errors) {
        if (b.hardware_lighting && !xb_backend_lighting(&saved)) result = 0;
        view_matrix(b.perspective);
    }
    return result && !b.errors;
}

static int validate_stream(void)
{
    xb_mesh mesh;
    xb_point points[6];
    xb_rgb vertex_colors[6];
    xb_rgb colors[2], expected[2], black;
    xb_rgb interpolated;
    float x, scale;
    int i, j, px, py, probe_x, probe_y, saved_z, result;
    memset(&mesh, 0, sizeof(mesh));
    mesh.triangles = 2; mesh.point_count = 6;
    mesh.points = points; mesh.colors = colors;
    mesh.vertex_colors = vertex_colors;
    expected[0].r = 1; expected[0].g = expected[0].b = 0;
    expected[1].r = 0; expected[1].g = expected[1].b = 1;
    if (b.shading == XB_SMOOTH) {
        expected[0].r = .6f;
        expected[1].g = expected[1].b = .6f;
    }
    black.r = black.g = black.b = 0;
    saved_z = b.current_z;
    result = 0;
    b.current_z = 1;
    xgl_object_set(b.ctx, XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_Z_BUFFER,
                   XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_OFF, 0);
    identity(b.model);
    view_matrix(0);
    /* Even the depth-one control exercises two independent native slots in
     * this untimed gate. Disjoint patches survive until the joint readback;
     * normal per-item clears would hide mistakes in earlier queued draws.
     */
    b.stream_slots = 2;
    for (i = 0; i < 2; ++i) {
        x = i ? 0.3f : -0.9f;
        points[0].x = x;        points[0].y = -0.85f;
        points[1].x = x + 0.6f; points[1].y = -0.85f;
        points[2].x = x;        points[2].y = -0.5f;
        points[3].x = -0.3f; points[3].y = -0.2f;
        points[4].x = 0.3f;  points[4].y = -0.2f;
        points[5].x = 0;     points[5].y = 0.4f;
        for (j = 0; j < 6; ++j) points[j].z = i ? 0.8f : 0.2f;
        colors[0] = colors[1] = black;
        for (j = 0; j < 6; ++j) vertex_colors[j] = black;
        if (!allocate_stream_slot(i, &mesh)) goto done;
    }
    xb_backend_clear(1, 1);
    for (i = 0; i < 2; ++i) {
        colors[0] = colors[1] = expected[i];
        /* Each disjoint patch has .2/.6/1 corner intensities, whose interior
         * centroid must be .6. This rejects accidentally flat vertex colors
         * independently of the serialized shading gate. Overlapping depth
         * patches use constant vertex colors for an unambiguous Z check.
         */
        for (j = 0; j < 6; ++j) {
            scale = j < 3 ? (float)(2 * j + 1) / 3.0f : 1.0f;
            vertex_colors[j].r = scale * expected[i].r;
            vertex_colors[j].g = scale * expected[i].g;
            vertex_colors[j].b = scale * expected[i].b;
        }
        if (!xb_backend_stream_prepare(i, &mesh)) goto done;
        colors[0] = colors[1] = black; /* CPU source must not be retained. */
        for (j = 0; j < 6; ++j) vertex_colors[j] = black;
        if (!draw_stream_slot(i, 0)) goto done;
        xb_backend_post();
    }
    if (!xb_backend_stream_drain()) goto done;
    py = (int)((-0.733333333 + 1.0) * 0.5 * (b.height - 1));
    for (i = 0; i < 2; ++i) {
        x = i ? 0.5f : -0.7f;
        px = (int)((x + 1.0) * 0.5 * (b.width - 1));
        if (!pixel_matches(px, py, &expected[i])) goto done;
        if (b.shading == XB_SMOOTH) {
            /* A second, noncentral sample distinguishes interpolation from
             * any constant corner or average color. Evaluate its exact pixel
             * barycentrics to avoid a resolution-dependent expected value.
             */
            x = i ? .3f : -.9f;
            probe_x = (int)((x + .12 + 1.0) * .5 * (b.width - 1));
            probe_y = (int)((-.78 + 1.0) * .5 * (b.height - 1));
            scale = (float)((.2 + .4 *
                ((2.0 * probe_x / (b.width - 1) - 1.0 - x) / .6) + .8 *
                ((2.0 * probe_y / (b.height - 1) - 1.0 + .85) / .35)) / .6);
            interpolated.r = expected[i].r * scale;
            interpolated.g = expected[i].g * scale;
            interpolated.b = expected[i].b * scale;
            if (!pixel_matches(probe_x, probe_y, &interpolated)) goto done;
        }
    }
    if (!pixel_matches(b.width / 2, b.height / 2, &expected[0])) goto done;
    /* Reuse only after completion. The second far central triangle must
     * replace the first near triangle, proving the queued Z clear as well
     * as the color clear (the old left patch must disappear).
     */
    for (i = 0; i < 2; ++i) {
        if (!xb_backend_stream_draw(i)) goto done;
        xb_backend_post();
    }
    if (!xb_backend_stream_drain()) goto done;
    px = (int)(0.3 * 0.5 * (b.width - 1));
    if (!pixel_matches(px, py, &black)) goto done;
    px = (int)(1.5 * 0.5 * (b.width - 1));
    if (!pixel_matches(px, py, &expected[1])) goto done;
    if (!pixel_matches(b.width / 2, b.height / 2, &expected[1])) goto done;
    result = 1;
done:
    /* Native failures preserve all queued references until recovery/close. */
    if (!b.errors) {
        b.current_z = saved_z;
        xgl_object_set(b.ctx, XGL_3D_CTX_HLHSR_MODE,
                       saved_z ? XGL_HLHSR_Z_BUFFER : XGL_HLHSR_NONE,
                       XGL_3D_CTX_SURF_FACE_CULL,
                       b.cull ? XGL_CULL_BACK : XGL_CULL_OFF, 0);
        view_matrix(b.perspective);
    }
    return result && !b.errors;
}

static int validate_raster(int masked)
{
    xb_rgb black, hud;
    int px, py, result;
    if ((masked && b.mask_checked) || (!masked && b.raster_checked)) return 1;
    if (b.width < XB_RASTER_W || b.height < XB_RASTER_H) {
        fail_kind(XB_FAILURE_UNSUPPORTED, "raster benchmark requires at least a 128x64 drawable");
        return 0;
    }
    if (!b.hud_ready && !make_hud()) return 0;
    xb_backend_clear(1, 1);
    xb_backend_raster(masked);
    xb_backend_sync();
    px = (b.width - XB_RASTER_W) / 2;
    py = b.height - XB_RASTER_H;
    black.r = black.g = black.b = 0;
    hud.r = 220.0f / 255; hud.g = 32.0f / 255; hud.b = 64.0f / 255;
    result = pixel_matches(px, py, &hud);
    if (result && masked) result = pixel_matches(px + 9, py, &black);
    /* Cache success only. API errors, cancellation and obscured pixels may
     * recover at the next case boundary; never relabel them as a cached RGB
     * mismatch. Rechecking a real mismatch also preserves its precise reason.
     */
    if (result) {
        if (masked) b.mask_checked = 1;
        else b.raster_checked = 1;
    }
    return result;
}

int xb_backend_validate(enum xb_method method, char *reason, size_t size)
{
    xb_mesh mesh;
    xb_case test;
    xb_point points[24], strip_points[16];
    xb_rgb colors[8], black;
    xb_strip strips[4];
    float x, y, dx, dy;
    size_t i, j, base;
    int z, px, py;
    static const float rgb[8][3] = {
        {1,0,0},{0,1,0},{0,0,1},{1,1,0},
        {1,0,1},{0,1,1},{0.5f,0.25f,0},{0.25f,0.5f,1}
    };
    memset(&mesh, 0, sizeof(mesh));
    memset(&test, 0, sizeof(test));
    test.method = method; test.batch = 3; test.zbuffer = 1;
    test.work = XB_DRAW;
    mesh.triangles = 8; mesh.point_count = 24;
    mesh.strip_point_count = 16; mesh.strip_count = 4;
    mesh.points = points; mesh.strip_points = strip_points;
    mesh.colors = colors; mesh.strips = strips;
    for (i = 0; i < 4; ++i) {
        x = (i & 1) ? 0.1f : -0.9f;
        y = (i & 2) ? 0.1f : -0.9f;
        base = i * 4;
        strip_points[base].x = x; strip_points[base].y = y;
        strip_points[base+1].x = x; strip_points[base+1].y = y + 0.8f;
        strip_points[base+2].x = x + 0.8f; strip_points[base+2].y = y;
        strip_points[base+3].x = x + 0.8f; strip_points[base+3].y = y + 0.8f;
        for (j = 0; j < 4; ++j) strip_points[base+j].z = 0.5f;
        points[i*6] = strip_points[base];
        points[i*6+1] = strip_points[base+1];
        points[i*6+2] = strip_points[base+2];
        points[i*6+3] = strip_points[base+2];
        points[i*6+4] = strip_points[base+1];
        points[i*6+5] = strip_points[base+3];
        strips[i].first_point = base;
        strips[i].point_count = 4;
        strips[i].first_facet = i*2;
    }
    for (i = 0; i < 8; ++i) {
        colors[i].r = rgb[i][0]; colors[i].g = rgb[i][1]; colors[i].b = rgb[i][2];
    }
    for (z = 0; z <= 1; ++z) {
        test.zbuffer = z;
        if (!xb_backend_configure(&test, &mesh, reason, size)) return 0;
        xb_backend_draw(method, 3);
        xb_backend_sync();
        for (i = 0; i < 8; ++i) {
            dx = dy = 0;
            for (j = 0; j < 3; ++j) {
                dx += points[i*3+j].x / 3.0f;
                dy += points[i*3+j].y / 3.0f;
            }
            px = (int)((dx + 1.0) * 0.5 * (b.width - 1));
            py = (int)((dy + 1.0) * 0.5 * (b.height - 1));
            if (!pixel_matches(px, py, &colors[i])) goto bad;
        }
    }
    if (method == XB_MULTI) {
        test.retained = 1;
        if (!xb_backend_configure(&test, &mesh, reason, size)) return 0;
        xb_backend_draw(method, 3); xb_backend_sync();
        for (i = 0; i < 8; ++i) {
            dx = dy = 0;
            for (j = 0; j < 3; ++j) {
                dx += points[i*3+j].x / 3.0f;
                dy += points[i*3+j].y / 3.0f;
            }
            px = (int)((dx + 1.0) * 0.5 * (b.width - 1));
            py = (int)((dy + 1.0) * 0.5 * (b.height - 1));
            if (!pixel_matches(px, py, &colors[i])) goto bad;
        }
        test.retained = 0;
    }
    /* Check row-vector model rotation and the homogeneous W divide together.
     * Every facet remains at z=.5, so centroid projection is unambiguous.
     */
    test.perspective = 1;
    if (!xb_backend_configure(&test, &mesh, reason, size)) return 0;
    xb_backend_transform(90);
    xb_backend_draw(method, 3); xb_backend_sync();
    for (i = 0; i < 8; ++i) {
        dx = dy = 0;
        for (j = 0; j < 3; ++j) {
            dx += -points[i*3+j].y / (3.0f * 1.375f);
            dy += points[i*3+j].x / (3.0f * 1.375f);
        }
        px = (int)((dx + 1.0) * 0.5 * (b.width - 1));
        py = (int)((dy + 1.0) * 0.5 * (b.height - 1));
        if (!pixel_matches(px, py, &colors[i])) goto bad;
    }
    test.perspective = 0;
    /* A near red facet followed by a farther blue facet tests real depth. */
    for (i = 0; i < 2; ++i) {
        base = i*3;
        points[base].x = -0.8f; points[base].y = -0.8f;
        points[base+1].x = 0.8f; points[base+1].y = -0.8f;
        points[base+2].x = 0.0f; points[base+2].y = 0.8f;
        for (j = 0; j < 3; ++j) {
            points[base+j].z = i ? 0.8f : 0.2f;
            strip_points[base+j] = points[base+j];
        }
        strips[i].first_point = base; strips[i].point_count = 3;
        strips[i].first_facet = i;
    }
    colors[0].r = 1; colors[0].g = colors[0].b = 0;
    colors[1].b = 1; colors[1].r = colors[1].g = 0;
    mesh.triangles = 2; mesh.point_count = 6;
    mesh.strip_count = 2; mesh.strip_point_count = 6;
    test.zbuffer = 1;
    if (!xb_backend_configure(&test, &mesh, reason, size)) return 0;
    xb_backend_draw(method, 3); xb_backend_sync();
    if (!pixel_matches(b.width / 2, b.height / 2, &colors[0])) goto bad;
    /* Reversing depths after a clear must expose blue, not retain stale red. */
    for (i = 0; i < 6; ++i) {
        points[i].z = strip_points[i].z = i < 3 ? 0.8f : 0.2f;
    }
    if (!xb_backend_upload(&mesh)) goto bad;
    xb_backend_clear(1, 1); xb_backend_draw(method, 3); xb_backend_sync();
    if (!pixel_matches(b.width / 2, b.height / 2, &colors[1])) goto bad;
    /* Both triangles in front of the near plane must be clipped. */
    for (i = 0; i < 6; ++i) points[i].z = strip_points[i].z = -0.5f;
    if (!xb_backend_upload(&mesh)) goto bad;
    xb_backend_clear(1, 1); xb_backend_draw(method, 3); xb_backend_sync();
    black.r = black.g = black.b = 0;
    if (!pixel_matches(b.width / 2, b.height / 2, &black)) goto bad;
    if (b.buffers >= 2) xb_backend_swap();
    copy_text(reason, size, "facet colors, Z/clear, near clipping and transformed perspective readback passed");
    return 1;
bad:
    copy_text(reason, size, b.error);
    return 0;
}
