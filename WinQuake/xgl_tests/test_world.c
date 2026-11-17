/* Host renderer contracts; these tests do not simulate XGL or GPU timing. */
#include <assert.h>
#include <stdarg.h>
#include "../xgl_world.c"

client_state_t cl;
lightstyle_t cl_lightstyle[MAX_LIGHTSTYLES];
entity_t r_worldentity, *currententity;
vec3_t r_origin, vpn;
mleaf_t *r_viewleaf;
int r_framecount, r_visframecount, c_brush_polys;
cvar_t r_fullbright, r_wateralpha, r_novis;
double realtime;
byte xgl_palette[768];

static model_t test_model;
static msurface_t test_surface;
static mplane_t test_plane;
static mtexinfo_t test_texinfo;
static texture_t test_texture;
static mvertex_t test_vertices[4];
static medge_t test_edges[4];
static int test_surfedges[4];
static mnode_t test_node;
static mleaf_t test_leaves[3];
static msurface_t *test_marks[1];
static byte test_samples[81];
static efrag_t test_efrag;
static byte test_vis;
static int pvs_calls, sample_calls, efrag_calls, reject_box;
static xgl_vertex_t captured[1024];
static int captured_count, captured_lit, captured_zwrite;
static int captured_calls, varying_material, prepare_calls, filter_calls;
static unsigned int color_revision, texture_revision;
static float gamma_scale;
static float captured_opacity;
static unsigned long profile_counts[XGL_PROFILE_COUNTER_COUNT];
static int profile_sections[XGL_PROFILE_SECTION_COUNT];
static byte contour_pixels[128 * 128];
static int contour_pixel_view, contour_alpha, pixel_view_calls;

double XGL_ProfileDetailNow(void) { return 1; }
void XGL_ProfileAdd(xgl_profile_section_t section, double start)
{
    assert(section >= XGL_PROFILE_WORLD_VISIBILITY && section < XGL_PROFILE_SECTION_COUNT);
    assert(start == 1);
    ++profile_sections[section];
}
void XGL_ProfileCount(xgl_profile_counter_t counter, unsigned long count)
{
    assert((unsigned int)counter < XGL_PROFILE_COUNTER_COUNT);
    profile_counts[counter] += count;
}

void Con_Printf(char *format, ...) { (void)format; }
void XGL_AudioUpdate(void) {}
unsigned int XGL_TextureRevision(int id) { assert(id == 1 || id == 2); return texture_revision; }
qboolean XGL_TextureHasAlpha(int id)
{
    assert(id == 1 || id == 2);
    return contour_alpha;
}
qboolean XGL_TexturePixels(int id, const byte **pixels, int *width, int *height)
{
    assert(id == 1 || id == 2);
    ++pixel_view_calls;
    if (!contour_pixel_view) return false;
    *pixels = contour_pixels;
    *width = *height = 128;
    return true;
}
unsigned int XGL_ColorRevision(void) { return color_revision; }
void XGL_PrepareColor(float rgb[3])
{
    int i;
    ++prepare_calls;
    for (i = 0; i < 3; ++i) rgb[i] *= gamma_scale;
}
void Sys_Error(char *format, ...)
{
    va_list args;
    va_start(args, format); vfprintf(stderr, format, args); va_end(args);
    abort();
}
void Host_Error(char *format, ...)
{
    va_list args;
    va_start(args, format); vfprintf(stderr, format, args); va_end(args);
    abort();
}
qboolean R_CullBox(vec3_t mins, vec3_t maxs)
{
    (void)mins; (void)maxs;
    return reject_box;
}
byte *Mod_LeafPVS(mleaf_t *leaf, model_t *model)
{
    (void)leaf; (void)model;
    ++pvs_calls;
    return &test_vis;
}
void R_StoreEfrags(efrag_t **efrags)
{
    assert(*efrags == &test_efrag);
    ++efrag_calls;
}
qboolean XGL_SampleTexture(int texture, float s, float t, float rgb[3], const byte *translation)
{
    int x, y, index, channel;
    assert(texture == 1 || texture == 2);
    assert(translation == NULL);
    ++sample_calls;
    if (contour_pixel_view) {
        s -= (float)floor(s); t -= (float)floor(t);
        x = (int)(s * 128); y = (int)(t * 128);
        if (x > 127) x = 127;
        if (y > 127) y = 127;
        index = contour_pixels[y * 128 + x];
        for (channel = 0; channel < 3; ++channel)
            rgb[channel] = xgl_palette[index * 3 + channel] / 255.0f;
        return true;
    }
    rgb[0] = .8f; rgb[1] = .4f; rgb[2] = .2f;
    if (varying_material) {
        rgb[0] = s - (float)floor(s);
        rgb[1] = t - (float)floor(t);
    }
    return true;
}
qboolean XGL_SampleTextureFiltered(int texture, float s, float t, float fs, float ft,
                                  float rgb[3], const byte *translation)
{
    assert(fs >= 0 && ft >= 0);
    ++filter_calls;
    return XGL_SampleTexture(texture, s, t, rgb, translation);
}

void XGL_DrawTrianglesStrided(const xgl_vertex_t *vertices, int count, size_t stride,
                              int lit, float opacity, int zwrite, int prepared)
{
    int i;
    const xgl_vertex_t *vertex;
    assert(prepared && stride >= sizeof(*vertices));
    assert(count > 0 && count % 3 == 0 && captured_count + count <= 1024);
    for (i = 0; i < count; ++i) {
        vertex = (const xgl_vertex_t *)((const byte *)vertices + i * stride);
        assert(fabs(DotProduct(vertex->normal, vertex->normal) - 1) < .0001);
        captured[captured_count++] = *vertex;
    }
    captured_lit = lit; captured_opacity = opacity; captured_zwrite = zwrite;
    ++captured_calls;
}

static void begin_frame(void)
{
    ++r_framecount;
    captured_count = captured_calls = 0;
    memset(profile_counts, 0, sizeof(profile_counts));
    memset(profile_sections, 0, sizeof(profile_sections));
    XGL_DrawWorld();
}

static float captured_area(void)
{
    int i;
    float area;
    vec3_t a, b, cross;
    area = 0;
    for (i = 0; i < captured_count; i += 3) {
        VectorSubtract(captured[i+1].position, captured[i].position, a);
        VectorSubtract(captured[i+2].position, captured[i].position, b);
        CrossProduct(a, b, cross);
        area += Length(cross) * .5f;
    }
    return area;
}

static void setup_world(void)
{
    int i;
    XGL_WorldShutdown();
    memset(&test_model, 0, sizeof(test_model));
    memset(&test_surface, 0, sizeof(test_surface));
    memset(&test_texinfo, 0, sizeof(test_texinfo));
    memset(&test_texture, 0, sizeof(test_texture));
    memset(test_vertices, 0, sizeof(test_vertices));
    memset(test_edges, 0, sizeof(test_edges));
    memset(&test_plane, 0, sizeof(test_plane));
    memset(&test_node, 0, sizeof(test_node));
    memset(test_leaves, 0, sizeof(test_leaves));
    memset(&r_worldentity, 0, sizeof(r_worldentity));
    memset(&cl, 0, sizeof(cl));
    test_vertices[1].position[0] = 128;
    test_vertices[2].position[0] = test_vertices[2].position[1] = 128;
    test_vertices[3].position[1] = 128;
    for (i = 0; i < 4; ++i) {
        test_edges[i].v[0] = i; test_edges[i].v[1] = (i + 1) % 4;
        test_surfedges[i] = i;
    }
    test_plane.normal[2] = 1;
    test_plane.type = PLANE_Z;
    test_texinfo.vecs[0][0] = test_texinfo.vecs[1][1] = 1;
    test_texture.width = test_texture.height = 128;
    test_texture.gl_texturenum = 1;
    test_texinfo.texture = &test_texture;
    test_surface.plane = &test_plane;
    test_surface.texinfo = &test_texinfo;
    test_surface.numedges = 4;
    test_surface.extents[0] = test_surface.extents[1] = 128;
    memset(test_samples, 64, sizeof(test_samples));
    test_surface.samples = test_samples;
    memset(test_surface.styles, 255, sizeof(test_surface.styles));
    test_surface.styles[0] = 0;
    test_node.plane = &test_plane;
    test_node.numsurfaces = 1;
    test_node.children[0] = (mnode_t *)&test_leaves[1];
    test_node.children[1] = (mnode_t *)&test_leaves[2];
    test_leaves[0].contents = CONTENTS_SOLID;
    for (i = 1; i < 3; ++i) {
        test_leaves[i].contents = CONTENTS_EMPTY;
        test_leaves[i].parent = &test_node;
    }
    test_marks[0] = &test_surface;
    test_leaves[1].firstmarksurface = test_marks;
    test_leaves[1].nummarksurfaces = 1;
    test_leaves[1].efrags = &test_efrag;
    test_model.numleafs = 2;
    test_model.leafs = test_leaves;
    test_model.nodes = &test_node;
    test_model.surfaces = &test_surface;
    test_model.numsurfaces = test_model.nummodelsurfaces = 1;
    test_model.vertexes = test_vertices;
    test_model.edges = test_edges;
    test_model.surfedges = test_surfedges;
    test_model.lightdata = test_samples;
    cl.worldmodel = r_worldentity.model = &test_model;
    r_origin[0] = r_origin[1] = 64; r_origin[2] = 32;
    vpn[0] = 1; vpn[1] = vpn[2] = 0;
    r_viewleaf = &test_leaves[1];
    r_fullbright.value = r_novis.value = 0;
    r_wateralpha.value = 1;
    xgl_subdivide.value = 32;
    xgl_detailtris.value = 4096;
    xgl_detaildistance.value = 768;
    xgl_contours.value = 1;
    for (i = 0; i < 256; ++i) d_lightstylevalue[i] = 256;
    pvs_calls = sample_calls = efrag_calls = reject_box = captured_count = 0;
    captured_calls = varying_material = prepare_calls = filter_calls = 0;
    color_revision = texture_revision = 1; gamma_scale = 1;
    contour_pixel_view = contour_alpha = pixel_view_calls = 0;
    for (i = 0; i < 256; ++i)
        xgl_palette[i * 3] = xgl_palette[i * 3 + 1] = xgl_palette[i * 3 + 2] = (byte)i;
    realtime = 0;
    test_vis = 1;
    XGL_WorldNewMap();
}

static void test_animated_water_and_sky(void)
{
    texture_t *sky;
    byte *pixels;
    float first_color;
    vec3_t first_position;
    int i, x, y, samples;
    setup_world();
    xgl_detailtris.value = 0;
    varying_material = 1;
    test_surface.flags = SURF_DRAWTURB;
    begin_frame();
    assert(captured_count == 6 && captured_calls == 1 && captured_lit == 0);
    first_color = captured[0].color[0];
    VectorCopy(captured[0].position, first_position);
    samples = sample_calls;
    realtime = .25;
    begin_frame();
    assert(captured_count == 6 && captured_calls == 1 && sample_calls == samples + 6);
    assert(fabs(captured[0].color[0] - first_color) > .01);
    assert(VectorCompare(captured[0].position, first_position));
    /* Dynamic lighting never changes the water or sky's unlit material path. */
    assert(captured_lit == 0 && captured_zwrite == 1);
    samples = sample_calls;
    reject_box = 1;
    realtime = .5;
    begin_frame();
    assert(captured_count == 0 && sample_calls == samples);

    setup_world();
    xgl_detailtris.value = 0;
    test_surface.flags = SURF_DRAWSKY;
    sky = (texture_t *)calloc(1, sizeof(texture_t) + 256 * 128);
    assert(sky != NULL);
    sky->width = 256; sky->height = 128;
    sky->offsets[0] = sizeof(texture_t);
    sky->gl_texturenum = 1;
    test_texinfo.texture = sky;
    pixels = (byte *)sky + sky->offsets[0];
    for (i = 0; i < 256; ++i)
        xgl_palette[i * 3] = xgl_palette[i * 3 + 1] = xgl_palette[i * 3 + 2] = i;
    for (y = 0; y < 128; ++y)
        for (x = 0; x < 128; ++x) pixels[y * 256 + 128 + x] = 1 + x;
    r_origin[0] = r_origin[1] = 0;
    begin_frame();
    assert(captured_count == 6 && captured_calls == 1 && captured_lit == 0);
    assert(fabs(captured[0].color[0] - 1.0f / 255) < .0001);
    assert(sample_calls == 0); /* split image samples bypass the full-image registry */
    realtime = 1;
    begin_frame();
    assert(captured_count == 6 && captured_calls == 1);
    assert(fabs(captured[0].color[0] - 9.0f / 255) < .0001);
    /* A nonzero foreground texel wins at the faster 16-unit scroll. */
    pixels[16 * 256 + 16] = 200;
    begin_frame();
    assert(captured_calls == 1);
    assert(fabs(captured[0].color[0] - 200.0f / 255) < .0001);
    pixels[16 * 256 + 16] = 0;
    realtime = 0;
    r_origin[0] = 16;
    begin_frame();
    assert(captured_calls == 1 && captured_lit == 0);
    assert(fabs(captured[0].color[0] - 1.0f / 255) > .01);
    XGL_WorldShutdown();
    free(sky);
}

static void test_cache_and_styles(void)
{
    float original;
    int count, samples, prepares;
    setup_world();
    begin_frame();
    assert(captured_count == 96 && captured_lit == 1);
    assert(captured_zwrite == 1 && captured_opacity == 1);
    assert(fabs(captured_area() - 128 * 128) < .01);
    assert(xgl_cache_bytes <= XGL_WORLD_CACHE_BYTES);
    original = captured[0].color[0];
    samples = sample_calls; count = captured_count; prepares = prepare_calls;
    assert(filter_calls == count);
    begin_frame();
    assert(captured_count == count && sample_calls == samples && pvs_calls == 1);
    assert(efrag_calls == 2 && prepare_calls == prepares);
    gamma_scale = .5f; ++color_revision;
    begin_frame();
    assert(sample_calls == samples && prepare_calls == prepares + count);
    assert(fabs(captured[0].color[0] - original * .5f) < .0001);
    gamma_scale = 1; ++color_revision;
    begin_frame();
    assert(sample_calls == samples);
    assert(fabs(captured[0].color[0] - original) < .0001);
    d_lightstylevalue[0] = 128;
    begin_frame();
    assert(sample_calls == samples);
    assert(fabs(captured[0].color[0] - original * .5f) < .0001);
    r_fullbright.value = 1;
    begin_frame();
    assert(captured_lit == 0 && sample_calls == samples);
    assert(fabs(captured[0].color[0] - .8f) < .0001);
    ++texture_revision;
    begin_frame();
    assert(sample_calls == samples + count);
    test_texture.gl_texturenum = 2;
    begin_frame();
    assert(sample_calls == samples + count * 2);
}

static void test_pvs_and_fallback(void)
{
    setup_world();
    xgl_detailtris.value = 0;
    begin_frame();
    assert(captured_count == 6 && pvs_calls == 1);
    test_vis = 2;
    r_viewleaf = &test_leaves[2];
    begin_frame();
    assert(captured_count == 0 && pvs_calls == 2);
    r_novis.value = 1;
    begin_frame();
    assert(captured_count == 6 && pvs_calls == 2);
    r_novis.value = 0;
    begin_frame();
    assert(captured_count == 0 && pvs_calls == 3);
    setup_world();
    xgl_cache_bytes = XGL_WORLD_CACHE_BYTES;
    begin_frame();
    assert(captured_count == 6 && xgl_surfaces[0].coarse.vertices == NULL);
    assert(fabs(captured_area() - 128 * 128) < .01);
    reject_box = 1;
    begin_frame();
    assert(captured_count == 0);
}

static void test_profile_cache_events(void)
{
    setup_world();
    begin_frame();
    assert(profile_counts[XGL_PROFILE_WORLD_MESH_BUILDS] == 2);
    assert(profile_counts[XGL_PROFILE_WORLD_BUILD_VERTICES] == 6 + 96);
    assert(profile_counts[XGL_PROFILE_WORLD_MESH_FAILURES] == 0);
    assert(profile_counts[XGL_PROFILE_WORLD_CACHED_SURFACES] == 1);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_UPDATES] == 1);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_VERTICES] == 96);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_HITS] == 0);
    assert(profile_sections[XGL_PROFILE_WORLD_VISIBILITY] == 1);
    assert(profile_sections[XGL_PROFILE_WORLD_DETAIL_SELECT] == 1);
    assert(profile_sections[XGL_PROFILE_WORLD_BUILD] == 2);
    assert(profile_sections[XGL_PROFILE_WORLD_COLORS] == 1);

    begin_frame();
    assert(profile_counts[XGL_PROFILE_WORLD_MESH_BUILDS] == 0);
    assert(profile_counts[XGL_PROFILE_WORLD_CACHED_SURFACES] == 1);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_HITS] == 1);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_UPDATES] == 0);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_VERTICES] == 0);
    assert(profile_sections[XGL_PROFILE_WORLD_BUILD] == 0);

    ++color_revision;
    begin_frame();
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_UPDATES] == 1);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_VERTICES] == 96);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_HITS] == 0);
    test_surface.flags = SURF_DRAWTURB;
    begin_frame();
    assert(profile_counts[XGL_PROFILE_WORLD_ANIMATED_VERTICES] == 96);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_UPDATES] == 0);
    assert(profile_sections[XGL_PROFILE_WORLD_ANIMATED] == 1);
    assert(profile_sections[XGL_PROFILE_WORLD_COLORS] == 0);

    setup_world();
    xgl_cache_bytes = XGL_WORLD_CACHE_BYTES;
    begin_frame();
    assert(profile_counts[XGL_PROFILE_WORLD_MESH_BUILDS] == 0);
    assert(profile_counts[XGL_PROFILE_WORLD_MESH_FAILURES] == 2);
    assert(profile_counts[XGL_PROFILE_WORLD_CACHED_SURFACES] == 0);
    assert(profile_counts[XGL_PROFILE_WORLD_FALLBACK_SURFACES] == 1);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_UPDATES] == 2);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_VERTICES] == 6);
    assert(captured_count == 6);
    begin_frame();
    /* A failed persistent allocation is not retried every frame. The
     * complete streamed fallback remains visible and counted each time. */
    assert(profile_counts[XGL_PROFILE_WORLD_MESH_FAILURES] == 0);
    assert(profile_counts[XGL_PROFILE_WORLD_FALLBACK_SURFACES] == 1);
    assert(captured_count == 6);
}

static void test_brush_water_and_point_light(void)
{
    entity_t entity;
    byte special_samples[MAXLIGHTMAPS];
    setup_world();
    assert(R_LightPoint(r_origin) == 64);
    xgl_detailtris.value = 0;
    begin_frame();
    memset(&entity, 0, sizeof(entity));
    entity.model = &test_model;
    entity.origin[0] = 10;
    entity.angles[1] = 90;
    captured_count = 0;
    XGL_DrawBrushModel(&entity);
    assert(captured_count == 6);
    assert(fabs(captured[0].position[0] - 10) < .001);
    assert(fabs(captured[1].position[0] - 10) < .001);
    assert(fabs(captured[1].position[1] - 128) < .001);
    assert(fabs(captured_area() - 128 * 128) < .01);
    test_surface.flags = SURF_DRAWTURB;
    /* The loader expands water extents beyond its on-disk lightmap. Special
     * surfaces must never index the tiny original sample block with them. */
    test_surface.extents[0] = test_surface.extents[1] = 16384;
    XGL_SurfaceLightSamples(&test_surface, test_vertices[2].position, special_samples);
    assert(special_samples[0] == 0);
    r_wateralpha.value = .5f;
    XGL_WorldNewMap();
    begin_frame();
    assert(captured_count == 0 && xgl_water_count == 1);
    XGL_DrawWater();
    assert(captured_count == 6 && captured_lit == 0);
    assert(captured_opacity == .5f && captured_zwrite == 0);
    assert(xgl_water_count == 0);
}

static void test_projected_detail_priority(void)
{
    msurface_t surfaces[2], *marks[2];
    mvertex_t vertices[8];
    medge_t edges[8];
    int surfedges[8], i, j;
    float minimum, maximum;
    setup_world();
    surfaces[0] = surfaces[1] = test_surface;
    memset(vertices, 0, sizeof(vertices));
    memset(edges, 0, sizeof(edges));
    for (i = 0; i < 2; ++i) {
        minimum = i ? 0 : 32; maximum = i ? 128 : 96;
        vertices[i * 4].position[0] = vertices[i * 4].position[1] = minimum;
        vertices[i * 4 + 1].position[0] = maximum;
        vertices[i * 4 + 1].position[1] = minimum;
        vertices[i * 4 + 2].position[0] = vertices[i * 4 + 2].position[1] = maximum;
        vertices[i * 4 + 3].position[0] = minimum;
        vertices[i * 4 + 3].position[1] = maximum;
        for (j = 0; j < 4; ++j) {
            edges[i * 4 + j].v[0] = i * 4 + j;
            edges[i * 4 + j].v[1] = i * 4 + (j + 1) % 4;
            surfedges[i * 4 + j] = i * 4 + j;
        }
        surfaces[i].firstedge = i * 4;
        marks[i] = &surfaces[i];
    }
    test_model.surfaces = surfaces;
    test_model.numsurfaces = test_model.nummodelsurfaces = 2;
    test_model.vertexes = vertices; test_model.edges = edges; test_model.surfedges = surfedges;
    test_node.numsurfaces = 2;
    test_leaves[1].firstmarksurface = marks; test_leaves[1].nummarksurfaces = 2;
    xgl_detailtris.value = 32;
    XGL_WorldNewMap();
    begin_frame();
    /* BSP order reaches the smaller face first, but the larger screen-area
     * estimate receives the detail budget. Both faces still draw, in order. */
    assert(xgl_detail_used == 32 && captured_count == 6 + 96);
    assert(xgl_surfaces[0].detail_frame != r_framecount);
    assert(xgl_surfaces[1].detail_frame == r_framecount);
    assert(captured[0].position[0] == 32 && captured[6].position[0] == 0);
    assert(fabs(captured_area() - (64 * 64 + 128 * 128)) < .01);
    assert(xgl_cache_bytes <= XGL_WORLD_CACHE_BYTES);
    xgl_detailtris.value = 0;
    begin_frame();
    assert(captured_count == 12 && xgl_detail_used == 0);
    assert(fabs(captured_area() - (64 * 64 + 128 * 128)) < .01);
    xgl_detailtris.value = 32;
    surfaces[1].flags = SURF_DRAWTURB; r_wateralpha.value = 0;
    begin_frame();
    assert(captured_count == 24 && xgl_detail_used == 8 && !xgl_water_count);
    assert(xgl_surfaces[0].detail_frame == r_framecount);
    assert(xgl_surfaces[1].detail_frame != r_framecount);
    XGL_WorldShutdown();
}

static void contour_panel(int boundary)
{
    int x, y;
    contour_pixel_view = 1;
    for (y = 0; y < 128; ++y)
        for (x = 0; x < 128; ++x)
            contour_pixels[y * 128 + x] = x < boundary ? 24 : 232;
}

static float edge_side(const float *a, const float *b, float x, float y)
{
    return (b[0] - a[0]) * (y - a[1]) - (b[1] - a[1]) * (x - a[0]);
}

static void assert_contour_coverage(void)
{
    const float *a, *b, *c;
    float x, y, area, ab, bc, ca;
    int i, j, k, covered, on_edge;
    assert(captured_count >= 6 && captured_count <= 96 && captured_count % 3 == 0);
    assert(xgl_detail_used <= xgl_detailtris.value);
    assert(fabs(captured_area() - 128 * 128) < .1);
    for (i = 0; i < captured_count; ++i) {
        assert(captured[i].position[0] >= -.001 && captured[i].position[0] <= 128.001);
        assert(captured[i].position[1] >= -.001 && captured[i].position[1] <= 128.001);
        assert(fabs(captured[i].position[2]) < .00001);
    }
    for (i = 0; i < captured_count; i += 3) {
        a = captured[i].position; b = captured[i + 1].position; c = captured[i + 2].position;
        area = edge_side(a, b, c[0], c[1]);
        assert(area > .00001); /* Original face winding and no zero-area triangles. */
    }
    /* Area equality alone can hide a hole canceled by an overlapping region.
     * Jittered points strictly inside triangles must be covered exactly once. */
    for (j = 0; j < 17; ++j) for (i = 0; i < 19; ++i) {
        x = 128 * (i + .327f) / 19;
        y = 128 * (j + .619f) / 17;
        covered = on_edge = 0;
        for (k = 0; k < captured_count; k += 3) {
            a = captured[k].position; b = captured[k + 1].position; c = captured[k + 2].position;
            ab = edge_side(a, b, x, y); bc = edge_side(b, c, x, y); ca = edge_side(c, a, x, y);
            if (ab >= -.001 && bc >= -.001 && ca >= -.001) {
                ++covered;
                if (fabs(ab) <= .001 || fabs(bc) <= .001 || fabs(ca) <= .001) on_edge = 1;
            }
        }
        assert(covered >= 1);
        if (!on_edge) assert(covered == 1);
    }
}

static void assert_positions_equal(const xgl_vertex_t *before, int count)
{
    int i;
    assert(captured_count == count);
    for (i = 0; i < count; ++i)
        assert(!memcmp(before[i].position, captured[i].position, sizeof(vec3_t)));
}

static int has_vertical_contour(float boundary)
{
    const float *a, *b;
    int i, edge;
    for (i = 0; i < captured_count; i += 3) for (edge = 0; edge < 3; ++edge) {
        a = captured[i + edge].position;
        b = captured[i + (edge + 1) % 3].position;
        if (fabs(a[0] - boundary) < 4 && fabs(b[0] - boundary) < 4 &&
            fabs(a[1] - b[1]) > 16) return 1;
    }
    return 0;
}

static void test_contour_cache_lighting_and_replacement(void)
{
    xgl_vertex_t first[1024];
    int count, views, samples, i;
    setup_world();
    contour_panel(43);
    r_fullbright.value = 1;
    begin_frame();
    assert(xgl_surfaces[0].detail.contour);
    assert(profile_counts[XGL_PROFILE_CONTOUR_ATTEMPTS] == 1);
    assert(profile_counts[XGL_PROFILE_CONTOUR_BUILDS] == 1);
    assert(profile_counts[XGL_PROFILE_CONTOUR_SAMPLES] > 0);
    assert(profile_counts[XGL_PROFILE_CONTOUR_EVALUATIONS] > 0);
    assert_contour_coverage();
    assert(has_vertical_contour(43));
    count = captured_count; views = pixel_view_calls; samples = sample_calls;
    memcpy(first, captured, count * sizeof(*first));
    assert(views == 1);
    begin_frame();
    assert_positions_equal(first, count);
    assert(pixel_view_calls == views && sample_calls == samples);
    assert(profile_counts[XGL_PROFILE_CONTOUR_ATTEMPTS] == 0);
    assert(profile_counts[XGL_PROFILE_CONTOUR_SAMPLES] == 0);
    assert(profile_counts[XGL_PROFILE_CONTOUR_EVALUATIONS] == 0);

    gamma_scale = .5f; ++color_revision;
    begin_frame();
    assert_positions_equal(first, count);
    assert(pixel_view_calls == views && sample_calls == samples);
    for (i = 0; i < count; ++i)
        assert(fabs(captured[i].color[0] - first[i].color[0] * .5f) < .0001);
    gamma_scale = 1; ++color_revision;
    r_fullbright.value = 0;
    begin_frame();
    memcpy(first, captured, count * sizeof(*first));
    d_lightstylevalue[0] = 128;
    begin_frame();
    assert_positions_equal(first, count);
    assert(pixel_view_calls == views && sample_calls == samples);
    for (i = 0; i < count; ++i)
        assert(fabs(captured[i].color[0] - first[i].color[0] * .5f) < .0001);

    contour_panel(91); ++texture_revision;
    begin_frame();
    assert(xgl_surfaces[0].detail.contour && pixel_view_calls == views + 1);
    assert_contour_coverage();
    assert(has_vertical_contour(91));
    views = pixel_view_calls;
    test_texture.gl_texturenum = 2;
    begin_frame();
    assert(xgl_surfaces[0].detail.contour && pixel_view_calls == views + 1);
    assert_contour_coverage();

    views = pixel_view_calls;
    xgl_contours.value = 0;
    begin_frame();
    assert(!xgl_surfaces[0].detail.contour && captured_count == 96);
    assert(pixel_view_calls == views);
    xgl_contours.value = 1;
    begin_frame();
    assert(xgl_surfaces[0].detail.contour && pixel_view_calls == views + 1);
    assert_contour_coverage();
}

static void test_contour_uv_orientation(void)
{
    const float *a, *b;
    float sa, sb, boundary, length;
    int mode, i, edge, found;
    for (mode = 0; mode < 4; ++mode) {
        setup_world();
        contour_panel(43);
        boundary = 43;
        if (mode == 0) {
            test_texinfo.vecs[0][0] = -1;
            test_texinfo.vecs[0][3] = 128;
        } else if (mode == 1) {
            test_texinfo.vecs[0][0] = 0; test_texinfo.vecs[0][1] = 1;
            test_texinfo.vecs[1][0] = -1; test_texinfo.vecs[1][1] = 0;
            test_texinfo.vecs[1][3] = 128;
        } else if (mode == 2) test_texinfo.vecs[0][1] = .25f;
        else {
            test_texinfo.vecs[0][3] = -128;
            boundary = -85;
        }
        begin_frame();
        assert(xgl_surfaces[0].detail.contour);
        assert_contour_coverage();
        found = 0;
        for (i = 0; i < captured_count; i += 3) for (edge = 0; edge < 3; ++edge) {
            a = captured[i + edge].position;
            b = captured[i + (edge + 1) % 3].position;
            sa = DotProduct(a, test_texinfo.vecs[0]) + test_texinfo.vecs[0][3];
            sb = DotProduct(b, test_texinfo.vecs[0]) + test_texinfo.vecs[0][3];
            length = (a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]);
            if (fabs(sa - boundary) < 4 && fabs(sb - boundary) < 4 && length > 256)
                found = 1;
        }
        assert(found);
    }
}

static void test_contour_exclusions_and_budget(void)
{
    int which;
    for (which = 0; which < 6; ++which) {
        setup_world();
        contour_panel(43);
        if (which == 0) contour_alpha = 1;
        if (which == 1) {
            test_texture.anim_total = test_texture.anim_max = 2;
            test_texture.anim_next = &test_texture;
        }
        if (which == 2) test_texture.alternate_anims = &test_texture;
        if (which == 3) test_texture.anim_next = &test_texture;
        if (which == 4) test_surface.flags = SURF_DRAWTURB;
        if (which == 5) test_surface.flags = SURF_DRAWSKY;
        begin_frame();
        assert(!xgl_surfaces[0].detail.contour && pixel_view_calls == 0);
        assert_contour_coverage();
    }
    setup_world();
    contour_panel(43);
    xgl_detailtris.value = 0;
    begin_frame();
    assert(captured_count == 6 && xgl_detail_used == 0 && pixel_view_calls == 0);
    assert_contour_coverage();
    setup_world();
    contour_panel(43);
    xgl_cache_bytes = XGL_WORLD_CACHE_BYTES;
    begin_frame();
    assert(captured_count == 6 && xgl_detail_used == 0 && pixel_view_calls == 0);
    assert_contour_coverage();
    begin_frame();
    assert(captured_count == 6 && pixel_view_calls == 0);
}

static void test_contour_existing_two_triangle_budget(void)
{
    int x, y;
    setup_world();
    contour_pixel_view = 1;
    for (y = 0; y < 128; ++y)
        for (x = 0; x < 128; ++x)
            contour_pixels[y * 128 + x] = x + y >= 128 ? 232 : 24;
    xgl_subdivide.value = 128;
    xgl_detailtris.value = 2;
    begin_frame();
    /* A useful contour can change the diagonal without adding triangles.
     * Equal-sized coarse/detail meshes must still select that contour. */
    assert(xgl_surfaces[0].detail.contour);
    assert(xgl_surfaces[0].detail.count == xgl_surfaces[0].coarse.count);
    assert(xgl_surfaces[0].detail_frame == r_framecount);
    assert(captured_count == 6 && xgl_detail_used == 2);
    assert(fabs(captured[0].color[0] - captured[3].color[0]) > .1);
    assert_contour_coverage();
}

static void test_contour_one_analysis_per_frame_and_inline_brush(void)
{
    msurface_t surfaces[2], *marks[2];
    model_t inline_model;
    entity_t inline_entity;
    int views;
    setup_world();
    contour_panel(43);
    surfaces[0] = surfaces[1] = test_surface;
    marks[0] = &surfaces[0]; marks[1] = &surfaces[1];
    test_model.surfaces = surfaces;
    test_model.numsurfaces = test_model.nummodelsurfaces = 2;
    test_node.numsurfaces = 2;
    test_leaves[1].firstmarksurface = marks;
    test_leaves[1].nummarksurfaces = 2;
    XGL_WorldNewMap();
    begin_frame();
    assert(pixel_view_calls == 1);
    assert(xgl_surfaces[0].detail.contour + xgl_surfaces[1].detail.contour == 1);
    begin_frame();
    assert(pixel_view_calls == 2);
    assert(xgl_surfaces[0].detail.contour && xgl_surfaces[1].detail.contour);
    begin_frame();
    assert(pixel_view_calls == 2);

    /* Inline brush model shares all backing arrays with the world. Its
     * faces must not acquire contour meshes even when they are nearby. */
    test_model.nummodelsurfaces = 1;
    test_node.numsurfaces = 1;
    test_leaves[1].nummarksurfaces = 1;
    inline_model = test_model;
    inline_model.firstmodelsurface = 1; inline_model.nummodelsurfaces = 1;
    memset(&inline_entity, 0, sizeof(inline_entity));
    inline_entity.model = &inline_model;
    XGL_WorldNewMap();
    begin_frame();
    assert(xgl_surfaces[0].detail.contour && !xgl_surfaces[1].detail.contour);
    views = pixel_view_calls;
    ++r_framecount; captured_count = 0;
    XGL_DrawBrushModel(&inline_entity);
    assert(captured_count == 96 && !xgl_surfaces[1].detail.contour);
    assert(pixel_view_calls == views);
    assert_contour_coverage();
    XGL_WorldShutdown();
}

int main(void)
{
    test_cache_and_styles();
    test_projected_detail_priority();
    test_pvs_and_fallback();
    test_profile_cache_events();
    test_brush_water_and_point_light();
    test_animated_water_and_sky();
    test_contour_cache_lighting_and_replacement();
    test_contour_uv_orientation();
    test_contour_exclusions_and_budget();
    test_contour_existing_two_triangle_budget();
    test_contour_one_analysis_per_frame_and_inline_brush();
    XGL_WorldShutdown();
    assert(xgl_cache_bytes == 0 && xgl_surfaces == NULL);
    puts("XGL world/light contracts passed");
    return 0;
}
