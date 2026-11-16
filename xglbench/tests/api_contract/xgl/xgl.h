/* Handwritten Sun XGL API contract. NOT SDK/ABI; never link to real XGL.
 * Data members/prototypes: Sun Reference Manual 802-5864. Numeric values below
 * are deliberately arbitrary and must never be used to call a real library.
 */
#ifndef XB_CONTRACT_XGL_H
#define XB_CONTRACT_XGL_H
#include <stddef.h>
typedef void *Xgl_sys_state;
typedef void *Xgl_win_ras;
typedef void *Xgl_ctx;
typedef void *Xgl_trans;
typedef void *Xgl_mem_ras;
typedef void *Xgl_ras;
typedef void *Xgl_light;
typedef void *Xgl_object;
typedef void *Xgl_mipmap_texture;
typedef void *Xgl_texture;
typedef void *Xgl_tmap;
typedef unsigned char Xgl_usgn8;
typedef unsigned int Xgl_usgn32;
typedef unsigned short Xgl_usgn16;
typedef int Xgl_sgn32;
typedef int Xgl_boolean;
typedef int Xgl_inq_support_level;
typedef int Xgl_facet_type;
typedef int Xgl_facet_flags;
typedef int Xgl_tlist_flags;
typedef int Xgl_flush_action;
typedef int Xgl_texture_boundary;
typedef int Xgl_texture_interp_method;
typedef int Xgl_texture_persp_correction;
typedef int Xgl_transp_method;
typedef int Xgl_blend_eq;
/* The SDK takes an enum, not a pointer, as xgl_open's fixed argument. */
typedef enum { XGL_UNUSED = 0 } Xgl_attribute;
typedef enum {
    XGL_COLOR_INDEX, XGL_COLOR_GRAY, XGL_COLOR_RGB, XGL_COLOR_Z
} Xgl_color_type;
typedef struct { float x, y, z; } Xgl_pt_f3d;
typedef struct { double x, y, z; } Xgl_pt_d3d;
typedef struct { int x, y; } Xgl_pt_i2d;
typedef struct { double xmin, xmax, ymin, ymax, zmin, zmax; } Xgl_bounds_d3d;
typedef struct { int xmin, xmax, ymin, ymax; } Xgl_bounds_i2d;
typedef float Xgl_matrix_f3d[4][4];
typedef float Xgl_matrix_f2d[3][2];
typedef struct xb_contract_bbox Xgl_bbox;
typedef struct { float r, g, b; } Xgl_color_rgb;
typedef union { Xgl_usgn32 index; float gray; Xgl_color_rgb rgb; Xgl_usgn32 z; } Xgl_color;
typedef struct { Xgl_color color; } Xgl_color_facet;
typedef struct { Xgl_color color; Xgl_pt_f3d normal; } Xgl_color_normal_facet;
typedef struct { float x, y, z; Xgl_color color; } Xgl_pt_color_f3d;
typedef struct { float x, y, z; Xgl_color color; Xgl_pt_f3d normal; } Xgl_pt_color_normal_f3d;
typedef struct { float x, y, z, data[1]; } Xgl_pt_data_f3d;
typedef struct { Xgl_color_rgb c1, c2; } Xgl_texture_blend_rgb;
typedef union { Xgl_texture_blend_rgb rgb; } Xgl_texture_blend_op;
typedef struct { Xgl_color_rgb c; } Xgl_texture_decal_rgb;
typedef union { Xgl_texture_decal_rgb rgb; } Xgl_texture_decal_op;
typedef struct { Xgl_color_rgb c; } Xgl_texture_blend_intrinsic_rgb;
typedef union { Xgl_texture_blend_intrinsic_rgb rgb; } Xgl_texture_blend_intrinsic_op;
typedef struct {
    int comp, texture_op;
    union {
        Xgl_texture_blend_op blend;
        Xgl_texture_decal_op decal;
        Xgl_texture_blend_intrinsic_op blend_int;
    } op;
} Xgl_render_component_desc;
typedef struct {
    Xgl_usgn32 num_render_comp_desc;
    Xgl_render_component_desc render_component_desc[2];
    Xgl_usgn32 num_channels[2], channel_number[2];
} Xgl_texture_color_comp_info;
typedef union { Xgl_texture_color_comp_info color_info; } Xgl_texture_comp_info;
typedef struct { Xgl_texture_interp_method filter1, filter2; } Xgl_texture_interp_info;
typedef struct {
    Xgl_texture texture_map;
    float max_u_freq, max_v_freq;
    Xgl_texture_boundary u_boundary, v_boundary;
    Xgl_usgn8 boundary_values[4];
    float depth_interp_factor;
    Xgl_texture_interp_info interp_info;
    Xgl_matrix_f2d orientation_matrix;
} Xgl_texture_general_mipmap_desc;
typedef struct {
    int texture_type;
    Xgl_texture_comp_info comp_info;
    union { Xgl_texture_general_mipmap_desc mipmap; } texture_info;
} Xgl_texture_general_desc;
typedef union {
    Xgl_color_facet color_facet;
    Xgl_color_normal_facet color_normal_facet;
} Xgl_facet;
typedef struct {
    Xgl_facet_type facet_type;
    Xgl_usgn32 num_facets;
    union { Xgl_color_facet *color_facets; Xgl_color_normal_facet *color_normal_facets; } facets;
} Xgl_facet_list;
typedef struct {
    int pt_type;
    Xgl_bbox *bbox;
    Xgl_usgn32 num_pts, num_data_values;
    union { Xgl_pt_f3d *f3d; Xgl_pt_color_f3d *color_f3d;
            Xgl_pt_color_normal_f3d *color_normal_f3d;
            Xgl_pt_data_f3d *data_f3d; } pts;
} Xgl_pt_list;
typedef union { struct { float z_value; } z_buffer; } Xgl_hlhsr_data;
typedef struct {
    int type, category;
    char *id, *msg, *cur_op, *cur_obj, *operand1, *operand2;
} Xgl_error_info;
typedef struct {
    char *name;
    Xgl_boolean dga_flag;
    struct { unsigned int index : 1; unsigned int rgb : 1; } color_type;
    Xgl_sgn32 depth, width, height, maximum_buffer;
    Xgl_boolean db_buffer_is_copy;
    int pt_type, hlhsr_mode;
    Xgl_inq_support_level picking, double_buffer, indexed_color, true_color,
        depth_cueing, lighting, shading, hlhsr, antialiasing;
    Xgl_boolean stereo;
    Xgl_usgn32 extns;
    Xgl_inq_support_level transparency;
} Xgl_inquire;
typedef struct { void *X_display; int X_screen; Xgl_usgn32 X_window; } Xgl_X_window;
typedef union { struct { Xgl_usgn32 type; void *desc; } win_ras; } Xgl_obj_desc;
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
/* Independent bits let the control-flow tests distinguish clear operations. */
#define XGL_CTX_NEW_FRAME_CLEAR 0x02
#define XGL_CTX_NEW_FRAME_HLHSR_ACTION 0x04
#define XGL_CTX_NEW_FRAME_SWITCH_BUFFER 0x08
enum {
    XGL_3D_CTX = 100, XGL_3D_CTX_HLHSR_DATA, XGL_3D_CTX_HLHSR_MODE,
    XGL_3D_CTX_SURF_FACE_CULL, XGL_3D_CTX_SURF_FACE_DISTINGUISH,
    XGL_3D_CTX_SURF_FRONT_ILLUMINATION, XGL_3D_CTX_Z_BUFFER_COMP_METHOD,
    XGL_3D_CTX_Z_BUFFER_WRITE_MASK, XGL_BUFFER_SEL_DRAW,
    XGL_CLIP_XMAX, XGL_CLIP_XMIN, XGL_CLIP_YMAX, XGL_CLIP_YMIN,
    XGL_CLIP_ZMAX, XGL_CLIP_ZMIN, XGL_CTX_BACKGROUND_COLOR,
    XGL_CTX_CLIP_PLANES, XGL_CTX_DEFERRAL_MODE, XGL_CTX_DEVICE,
    XGL_CTX_GEOM_DATA_IS_VOLATILE, XGL_CTX_LOCAL_MODEL_TRANS,
    XGL_CTX_NEW_FRAME_ACTION,
    XGL_CTX_RASTER_FILL_STYLE, XGL_CTX_RASTER_FPAT, XGL_CTX_RASTER_FPAT_POSITION,
    XGL_CTX_ROP, XGL_CTX_SURF_EDGE_FLAG, XGL_CTX_SURF_FRONT_COLOR_SELECTOR,
    XGL_CTX_SURF_FRONT_FILL_STYLE, XGL_CTX_VDC_MAP, XGL_CTX_VDC_ORIENTATION,
    XGL_CTX_VDC_WINDOW, XGL_CTX_VIEW_CLIP_BOUNDS, XGL_CTX_VIEW_TRANS,
    XGL_CULL_OFF, XGL_CULL_BACK, XGL_DATA_FLT, XGL_DEFER_ASTI, XGL_DEV_COLOR_TYPE,
    XGL_DEV_MAXIMUM_COORDINATES, XGL_ERROR_NONRECOVERABLE, XGL_FACET_COLOR,
    XGL_FACET_FLAG_SHAPE_CONVEX, XGL_FACET_FLAG_SIDES_ARE_3,
    XGL_FLUSH_SYNCHRONIZE, XGL_FLUSH_BUFFERS,
    XGL_HLHSR_NONE, XGL_HLHSR_Z_BUFFER, XGL_ILLUM_NONE,
    XGL_INQ_HARDWARE, XGL_INQ_NOT_SUPPORTED, XGL_INQ_SOFTWARE, XGL_MEM_RAS,
    XGL_MEM_RAS_IMAGE_BUFFER_ADDR, XGL_PT_F3D, XGL_RAS_DEPTH, XGL_RAS_FILL_COPY,
    XGL_RAS_FILL_STIPPLE, XGL_RAS_HEIGHT, XGL_RAS_SOURCE_BUFFER, XGL_RAS_WIDTH,
    XGL_ROP_COPY, XGL_SURF_COLOR_FACET, XGL_SURF_FILL_SOLID,
    XGL_SYS_ST_ERROR_DETECTION, XGL_SYS_ST_ERROR_INFO,
    XGL_SYS_ST_ERROR_NOTIFICATION_FUNCTION, XGL_SYS_ST_VERSION,
    XGL_TLIST_FLAG_TRI_RESTART, XGL_TRANS_3D, XGL_TRANS_DATA_TYPE,
    XGL_TRANS_DIMENSION, XGL_VDC_MAP_ALL, XGL_WIN_RAS, XGL_WIN_RAS_BACKING_STORE,
    XGL_WIN_RAS_BUFFERS_ALLOCATED, XGL_WIN_RAS_BUFFERS_REQUESTED,
    XGL_WIN_RAS_BUF_DISPLAY, XGL_WIN_RAS_BUF_DRAW, XGL_WIN_X,
    XGL_WIN_X_PROTO_DGA, XGL_Y_DOWN_Z_AWAY, XGL_Z_COMP_LESS_THAN_OR_EQUAL,
    XGL_3D_CTX_LIGHT_NUM, XGL_3D_CTX_LIGHTS, XGL_3D_CTX_LIGHT_SWITCHES,
    XGL_3D_CTX_SURF_FRONT_AMBIENT, XGL_3D_CTX_SURF_FRONT_DIFFUSE,
    XGL_3D_CTX_SURF_FRONT_SPECULAR, XGL_3D_CTX_SURF_FRONT_LIGHT_COMPONENT,
    XGL_3D_CTX_SURF_LIGHTING_NORMAL_FLIP, XGL_3D_CTX_SURF_NORMAL_FLIP,
    XGL_ILLUM_NONE_INTERP_COLOR, XGL_ILLUM_PER_FACET, XGL_ILLUM_PER_VERTEX,
    XGL_SURF_COLOR_VERTEX_ILLUM_DEP, XGL_PT_COLOR_F3D,
    XGL_PT_COLOR_NORMAL_F3D, XGL_FACET_COLOR_NORMAL, XGL_FACET_NONE,
    XGL_3D_CTX_SURF_GEOM_NORMAL, XGL_GEOM_NORMAL_FIRST_POINTS,
    XGL_LIGHT_AMBIENT, XGL_LIGHT_DIRECTIONAL, XGL_LIGHT_POSITIONAL,
    XGL_LIGHT_TYPE, XGL_LIGHT_COLOR, XGL_LIGHT_DIRECTION, XGL_LIGHT_POSITION,
    XGL_LIGHT_ATTENUATION_1, XGL_LIGHT_ATTENUATION_2,
    XGL_LIGHT_ENABLE_COMP_AMBIENT, XGL_LIGHT_ENABLE_COMP_DIFFUSE,
    XGL_PT_DATA_F3D, XGL_MIPMAP_TEXTURE, XGL_MIPMAP_TEXTURE_LEVELS,
    XGL_MIPMAP_TEXTURE_MEM_RAS_LIST, XGL_TMAP,
    XGL_TMAP_DESCRIPTOR, XGL_TMAP_COORD_SOURCE, XGL_TMAP_PARAM_TYPE,
    XGL_TMAP_T0_INDEX, XGL_TMAP_T1_INDEX, XGL_TEXTURE_COORD_DATA,
    XGL_TEXTURE_PARAM_EXPLICIT, XGL_TEXTURE_TYPE_MIPMAP,
    XGL_TEXTURE_BOUNDARY_WRAP, XGL_TEXTURE_INTERP_POINT,
    XGL_TEXTURE_INTERP_BILINEAR, XGL_TEXTURE_INTERP_MIPMAP_TRILINEAR,
    XGL_RENDER_COMP_FINAL_COLOR, XGL_TEXTURE_OP_REPLACE,
    XGL_3D_CTX_SURF_FRONT_TMAP_NUM, XGL_3D_CTX_SURF_FRONT_TMAP,
    XGL_3D_CTX_SURF_FRONT_TMAP_SWITCHES,
    XGL_3D_CTX_SURF_TMAP_PERSP_CORRECTION, XGL_TEXTURE_PERSP_PIXEL,
    XGL_TEXTURE_PERSP_NONE, XGL_SURF_COLOR_CONTEXT, XGL_CTX_SURF_FRONT_COLOR,
    XGL_3D_CTX_SURF_FRONT_TRANSP, XGL_3D_CTX_SURF_BACK_TRANSP,
    XGL_3D_CTX_SURF_TRANSP_METHOD, XGL_3D_CTX_SURF_TRANSP_BLEND_EQ,
    XGL_3D_CTX_BLEND_FREEZE_Z_BUFFER, XGL_3D_CTX_BLEND_DRAW_MODE,
    XGL_TRANSP_NONE, XGL_TRANSP_BLENDED, XGL_TRANSP_SCREEN_DOOR,
    XGL_BLEND_NONE, XGL_BLEND_ARBITRARY_BG, XGL_BLEND_CONST_BG,
    XGL_BLEND_ADD_TO_BG, XGL_BLEND_DRAW_ALL, XGL_BLEND_DRAW_NOT_BLENDED,
    XGL_BLEND_DRAW_BLENDED
};
Xgl_sys_state xgl_open(Xgl_attribute, ...);
void xgl_close(Xgl_sys_state);
Xgl_object xgl_object_create(Xgl_sys_state, int, Xgl_obj_desc *, ...);
void xgl_object_get(Xgl_object, int, void *);
void xgl_object_set(Xgl_object, ...);
void xgl_object_destroy(Xgl_object);
void xgl_transform_write(Xgl_trans, void *);
Xgl_inquire *xgl_inquire(Xgl_sys_state, Xgl_obj_desc *);
void xgl_window_raster_resize(Xgl_win_ras);
void xgl_context_new_frame(Xgl_ctx);
void xgl_polygon(Xgl_ctx, Xgl_facet_type, Xgl_facet *, Xgl_bbox *,
                 Xgl_usgn32, Xgl_pt_list []);
void xgl_multi_simple_polygon(Xgl_ctx, Xgl_facet_flags, Xgl_facet_list *,
                              Xgl_bbox *, Xgl_usgn32, Xgl_pt_list []);
void xgl_triangle_list(Xgl_ctx, Xgl_facet_list *, Xgl_pt_list *, Xgl_tlist_flags);
void xgl_triangle_strip(Xgl_ctx, Xgl_facet_list *, Xgl_pt_list *);
void xgl_context_flush(Xgl_ctx, Xgl_flush_action);
void xgl_mipmap_texture_build(Xgl_mipmap_texture, Xgl_mem_ras,
                             Xgl_texture_boundary, Xgl_texture_boundary);
void xgl_context_copy_buffer(Xgl_ctx, Xgl_bounds_i2d *, Xgl_pt_i2d *, Xgl_ras);
Xgl_boolean xgl_context_get_pixel(Xgl_ctx, Xgl_pt_i2d *, Xgl_color *);
#endif
