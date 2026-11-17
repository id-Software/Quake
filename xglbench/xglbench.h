/* XGLQuake standalone benchmark. GPL-2.0-or-later; see COPYING. */
#ifndef XGLBENCH_H
#define XGLBENCH_H

#include <stddef.h>
#include <stdio.h>

#define XB_VERSION "0.9.1"
#define XB_MAX_QUEUE 8
#define XB_MAX_LIGHTS 4
#define XB_STYLE_LAYERS 4
#define XB_MAX_RESULTS 512
#define XB_MAX_REPEATS 31
#define XB_METHODS 4
#define XB_TEXTURE_WIDTH 64

typedef struct { float x, y, z; } xb_point;
typedef struct { float r, g, b; } xb_rgb;
typedef struct { float u, v, gain; } xb_texcoord;
typedef struct { size_t first_point, point_count, first_facet; } xb_strip;
typedef struct {
    size_t triangles, point_count, strip_point_count, strip_count;
    xb_point *points;                 /* independent triangle corners */
    xb_point *strip_points;           /* equivalent strip geometry */
    xb_rgb *colors;                   /* one color per triangle */
    xb_rgb *base_colors;
    xb_texcoord *texcoords;          /* unwrapped per-corner material coordinates */
    xb_rgb *vertex_materials;        /* optional immutable sampled corner RGB */
    xb_strip *strips;
    size_t bytes;
    /* Optional lighting data. By default material RGB is constant per facet;
     * vertex_material cases instead use independent texture samples at corners. */
    xb_point *facet_normals, *vertex_normals, *centers;
    xb_rgb *vertex_colors;
    float *facet_styles, *vertex_styles;
    float style_weights[XB_STYLE_LAYERS];
    int lighting_ready, style_cache_valid;
} xb_mesh;

enum xb_method { XB_POLYGON, XB_MULTI, XB_TLIST, XB_STRIP };
enum xb_work { XB_DRAW, XB_COLOR_CLEAR, XB_DEPTH_CLEAR, XB_SWAP,
               XB_UI, XB_FRAME, XB_PIPELINE,
               XB_SCENE_FRAME, XB_COLOR_DEPTH_CLEAR };
enum xb_ui { XB_UI_HUD, XB_UI_MENU, XB_UI_CONSOLE };
enum xb_update { XB_STATIC, XB_TRANSFORM, XB_COLORS, XB_REBUILD, XB_LIGHT_UPDATE };
enum xb_shade { XB_FLAT, XB_SMOOTH };
enum xb_light_model { XB_LIGHT_OFF, XB_LIGHT_BAKED, XB_LIGHT_LAMBERT, XB_LIGHT_QUAKE };
enum xb_texture_filter { XB_TEXTURE_OFF, XB_TEXTURE_POINT,
                         XB_TEXTURE_BILINEAR, XB_TEXTURE_TRILINEAR };
enum xb_profile { XB_QUICK, XB_MEDIUM, XB_LONG, XB_EXTRALONG };
enum xb_failure { XB_FAILURE_NONE, XB_FAILURE_ERROR, XB_FAILURE_API,
                  XB_FAILURE_VALIDATION, XB_FAILURE_UNSUPPORTED,
                  XB_FAILURE_TIME_LIMIT, XB_FAILURE_NOT_RUN };

typedef struct {
    int count, directional;
    float ambient, attenuation1, attenuation2;
    xb_point direction, position[XB_MAX_LIGHTS];
    xb_rgb color[XB_MAX_LIGHTS];
    float radius[XB_MAX_LIGHTS];
} xb_light_state;

#define XB_UPDATE_COLORS 1
#define XB_UPDATE_LIGHTS 2

typedef struct {
    char name[80];
    enum xb_work work;
    enum xb_method method;
    enum xb_update update;
    enum xb_ui ui;
    int base_triangles; /* zero selects the global count until options are resolved */
    int batch, depth, zbuffer, overdraw, perspective, connected, retained, clipping, reverse;
    int streaming;
    int vertex_material, cull, mixed_winding, texture_compare;
    float size;
    double area, aspect; /* positive area requests calibrated projected geometry */
    enum xb_shade shade;
    enum xb_light_model lighting;
    enum xb_texture_filter texture_filter;
    int xgl_lighting, lights, directional, styles, moving;
    int sim_fps; /* zero selects the 15 FPS deterministic simulation default */
    int transparency, blend_zwrite;
    float opacity; /* 0..1; used only for blended transparency cases */
} xb_case;

typedef struct {
    int width, height, force_size;
    int repeats, base_triangles, allow_fallback, list_only, queue, custom_timing, sim_fps;
    enum xb_profile profile;
    double seconds, warmup, pause;
    const char *csv_path, *filter, *display;
} xb_options;

/* Logical color/light updates, not upload bytes or hardware work. Preparation
 * includes native conversion/copies even when a logical result is cached. */
typedef struct {
    unsigned long color_updates, light_updates, updated_items, cached_items;
    double updated_prepare, cached_prepare;
} xb_update_stats;

typedef struct {
    xb_case test;
    int width, height, passed, samples;
    enum xb_failure failure;
    unsigned long frames;
    size_t triangles, calls, bytes, native_current_bytes, native_peak_bytes;
    double total[XB_MAX_REPEATS];
    double prepare[XB_MAX_REPEATS];
    double render[XB_MAX_REPEATS];
    double present[XB_MAX_REPEATS];
    double cpu[XB_MAX_REPEATS];
    double drain[XB_MAX_REPEATS];
    double enqueue[XB_MAX_REPEATS];
    xb_update_stats updates[XB_MAX_REPEATS];
    double area_mean, area_min, area_max;
    size_t input_bytes;
    int queue_limit;
    unsigned long drains, max_outstanding;
    double median, low, high, prep_median, render_median, present_median;
    char reason[160];
} xb_result;

/* Portable geometry: no XGL/X11 dependency and no game assets. */
int xb_mesh_build(xb_mesh *mesh, int base_triangles, int depth,
                  float size, int overdraw);
int xb_mesh_build_connected(xb_mesh *mesh, int base_triangles, int depth);
int xb_mesh_build_calibrated(xb_mesh *mesh, int triangles, int width,
                            int height, double area, double aspect);
int xb_mesh_projected_area(const xb_mesh *mesh, int width, int height,
                           double *mean, double *minimum, double *maximum);
int xb_mesh_reverse_draw_order(xb_mesh *mesh);
/* Cache material samples before lighting init; also creates unlit vertex RGB.
 * Call once on a generated mesh. Rejected input leaves the mesh unchanged. */
int xb_mesh_vertex_materials(xb_mesh *mesh, int depth);
/* Texture-comparison controls share untinted RGB with the native texture.
 * Replaces facet colors and resets UV gains; vertex!=0 also caches corners.
 * Run once before lighting. Failure leaves all existing mesh data unchanged. */
int xb_mesh_texture_materials(xb_mesh *mesh, int depth, int vertex);
/* Copy the base image as row-major normalized RGB, beginning at UV (0,0).
 * Requires capacity for XB_TEXTURE_WIDTH squared pixels; extra space is untouched. */
int xb_texture_image(xb_rgb *pixels, size_t count);
void xb_mesh_free(xb_mesh *mesh);
void xb_mesh_colors(xb_mesh *mesh, unsigned long frame);
size_t xb_mesh_expected_triangles(int base_triangles, int depth);
const char *xb_method_name(enum xb_method method);

/* Lighting uses procedural samples, not BSP assets or XGL texture objects.
 * Init owns optional mesh arrays and builds phase-zero colors. Update returns
 * XB_UPDATE_* flags, zero for a cached result, or -1 on invalid input. */
int xb_lighting_init(xb_mesh *mesh, const xb_case *test);
int xb_lighting_update(xb_mesh *mesh, const xb_case *test,
                       unsigned long sequence, xb_light_state *state);
unsigned long xb_lighting_cycle_frames(const xb_case *test);
unsigned long xb_lighting_min_frames(const xb_case *test);
void xb_lighting_state(const xb_case *test, unsigned long sequence,
                       xb_light_state *state);
xb_rgb xb_lighting_evaluate(const xb_light_state *state, const xb_point *position,
                            const xb_point *normal, const xb_rgb *material);

/* Time is elapsed wall time; gethrtime() on Solaris. CPU includes user+system. */
double xb_now(void);
double xb_cpu_now(void);
double xb_median(const double *values, int count);
/* Non-busy wall-time delay. Returns 1 on completion, 0 on invalid input or
 * error (including EINTR); callers can poll events before retrying. */
int xb_sleep(double seconds);
/* Independent SIGALRM watchdog: expiration requests cancellation, then a
 * ten-second grace timer forces _exit(124) without entering XGL from a signal. */
int xb_watchdog_start(unsigned int seconds);
void xb_watchdog_stop(void);
int xb_watchdog_expired(void);
/* Cheap cancellation query for native submission boundaries; no clock read. */
int xb_cancel_requested(void);

/* Backend owns all native arrays; these functions never retain mesh pointers. */
enum xb_failure xb_backend_failure(void);
/* Application-owned native buffers only; excludes opaque XGL/driver storage.
 * Reset before configuring a case; sample before releasing its pipeline slots. */
void xb_backend_memory(size_t *current, size_t *peak);
void xb_backend_memory_reset_peak(void);
int xb_backend_open(const xb_options *options, int width, int height);
void xb_backend_close(void);
int xb_backend_resize(int width, int height);
void xb_backend_dimensions(int *width, int *height);
void xb_backend_info(FILE *out);
int xb_backend_validate(enum xb_method method, char *reason, size_t size);
int xb_backend_configure(const xb_case *test, const xb_mesh *mesh,
                         char *reason, size_t size);
int xb_backend_upload(const xb_mesh *mesh);
void xb_backend_colors(const xb_mesh *mesh);
int xb_backend_lighting(const xb_light_state *state);
void xb_backend_transform(unsigned long frame);
void xb_backend_clear(int color, int depth);
size_t xb_backend_draw(enum xb_method method, int batch);
void xb_backend_sync(void);
void xb_backend_swap(void);
/* Native text/geometry UI. Formatting, layout and submission are included in
 * this call and measured as render time; failure rejects the work item. */
int xb_backend_ui(enum xb_ui kind, unsigned long sequence);
int xb_backend_events(void); /* 0 requests exit; -1 means resized/exposed */
void xb_backend_label(const char *name);
int xb_backend_has_double_buffer(void);
int xb_backend_has_hardware(void);
const char *xb_backend_error(void);
/* Native payload referenced by drawing calls, not measured SBus/DMA bytes. */
size_t xb_backend_input_bytes(enum xb_method method, int batch);
/* Bounded, same-context, no-swap stream. Every queued slot remains immutable
 * until a successful drain. Begin preallocates all native storage. */
int xb_backend_stream_begin(const xb_mesh *mesh, int slots, char *reason, size_t size);
int xb_backend_stream_prepare(int slot, const xb_mesh *mesh);
size_t xb_backend_stream_draw(int slot);
void xb_backend_post(void);
int xb_backend_stream_drain(void);
void xb_backend_stream_end(void);

#endif
