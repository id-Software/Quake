/* XGLQuake renderer interface. No OpenGL or native XGL SDK types leak here. */
#ifndef XGL_LOCAL_H
#define XGL_LOCAL_H

#define BACKFACE_EPSILON 0.01

typedef enum {
    pt_static, pt_grav, pt_slowgrav, pt_fire, pt_explode, pt_explode2, pt_blob, pt_blob2
} ptype_t;

typedef struct particle_s {
    vec3_t org;
    float color;
    struct particle_s *next;
    vec3_t vel;
    float ramp, die;
    ptype_t type;
} particle_t;

typedef struct {
    vec3_t position, normal;
    float color[3];
} xgl_vertex_t;

/* Matches XGL's RGB point/color/normal record without requiring the SDK in
 * CPU-only renderer files. The native backend checks size and field offsets.
 * XGL's color union includes an unsigned long, including on host-test builds. */
typedef struct {
    float position[3];
    union { float rgb[3]; unsigned long alignment; } color;
    float normal[3];
} xgl_native_vertex_t;

extern byte xgl_palette[768];
extern entity_t r_worldentity, *currententity;
extern vec3_t modelorg, r_entorigin;
extern int r_visframecount, r_framecount, c_brush_polys, c_alias_polys;
extern mplane_t frustum[4];
extern mleaf_t *r_viewleaf, *r_oldviewleaf;
extern int d_lightstylevalue[256];
extern float r_avertexnormals[162][3];
extern cvar_t r_norefresh, r_drawentities, r_drawviewmodel, r_speeds;
extern cvar_t r_fullbright, r_wateralpha, r_dynamic, r_novis;
extern cvar_t cl_crossx, cl_crossy;
extern float v_blend[4];
extern byte gammatable[256];

void XGL_BeginFrame(void);
/* After BeginFrame and before BeginScene, pass the opaque console height
 * in logical screen pixels. Zero leaves the complete game view visible. */
void XGL_SetConsoleClip(int lines);
void XGL_BeginScene(void);
/* Positions and unit normals are in Quake world coordinates. */
void XGL_DrawTriangles(const xgl_vertex_t *vertices, int count, int lit,
                      float opacity, int zwrite);
void XGL_DrawTrianglesStrided(const xgl_vertex_t *vertices, int count,
                             size_t stride, int lit, float opacity,
                             int zwrite, int prepared_color);
/* Prepared native colors are already gamma corrected. Borrowed vertices
 * must remain unchanged until XGL_Flush, which submits volatile XGL data. */
void XGL_DrawNativeTriangles(const xgl_native_vertex_t *vertices, int count,
                            int lit, float opacity, int zwrite);
/* Reserve up to *count vertices (a multiple of three) in the backend batch.
 * Fill them before any other draw or flush; color preparation is safe.
 * No separate commit is needed.
 * Returns NULL and sets *count to zero when no scene is active. */
xgl_native_vertex_t *XGL_ReserveTriangles(int *count, int lit,
                                         float opacity, int zwrite);
unsigned int XGL_ColorRevision(void);
void XGL_PrepareColor(float rgb[3]);
void XGL_PreparePaletteColor(int index, float rgb[3]);
void XGL_Flush(void);
void XGL_EndScene(void);
void XGL_EndFrame(void);
void XGL_SetViewModel(int enabled);
void XGL_UIText(float x, float y, const char *text, int color,
                float cell_width, float height);
void XGL_UIFill(float x, float y, float width, float height,
                int color, float opacity);
void XGL_UIFillRGB(float x, float y, float width, float height,
                  const float rgb[3], float opacity);
void XGL_UIFlush(void);
void XGL_DrawFlush(void);
int XGL_ReadScreen(byte *rgb, int width, int height);
void XGL_DrawParticle(const particle_t *particle);
void XGL_PolyBlend(void);

int XGL_LoadTexture(char *identifier, int width, int height, byte *data,
                    qboolean mipmap, qboolean alpha);
qboolean XGL_SampleTexture(int id, float s, float t, float rgb[3],
                          const byte *translation);
qboolean XGL_SampleTextureIndex(int id, float s, float t, int *index);
unsigned int XGL_TextureRevision(int id);
qboolean XGL_TextureHasAlpha(int id);
qboolean XGL_SampleTextureFiltered(int id, float s, float t,
                                  float footprint_s, float footprint_t,
                                  float rgb[3], const byte *translation);
void XGL_TextureInit(void);
void XGL_TextureShutdown(void);
void XGL_MakeAliasModelDisplayLists(model_t *model, aliashdr_t *header);
void XGL_SubdivideSurface(msurface_t *surface);

void XGL_WorldNewMap(void);
void XGL_WorldShutdown(void);
void XGL_DrawWorld(void);
void XGL_DrawBrushModel(entity_t *entity);
void XGL_DrawWater(void);
void XGL_DrawEntities(void);
void XGL_DrawViewModel(void);
void XGL_EntitiesInit(void);
void XGL_EntitiesShutdown(void);
void R_TranslatePlayerSkin(int playernum);
int R_LightPoint(vec3_t point);
void R_AnimateLight(void);
qboolean R_CullBox(vec3_t mins, vec3_t maxs);
texture_t *R_TextureAnimation(texture_t *base);
void R_InitParticles(void);
void R_ClearParticles(void);
void R_DrawParticles(void);
void R_ReadPointFile_f(void);
void R_StoreEfrags(efrag_t **ppefrag);
void V_CalcBlend(void);
int XGL_CheckPackDirectory(const char *path, int count, unsigned short crc);
void XGL_SharewareDiagnostics(void);

#endif
