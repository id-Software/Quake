/* Host renderer contracts; these tests do not simulate XGL or GPU timing. */
#include <assert.h>
#include <float.h>
#include <stdarg.h>
#include <stdlib.h>
static int fail_alloc_after = -1;
static void *test_malloc(size_t bytes)
{
    if (fail_alloc_after == 0) return NULL;
    if (fail_alloc_after > 0) --fail_alloc_after;
    return malloc(bytes);
}
#define malloc test_malloc
#include "../xgl_world.c"
#undef malloc

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
static xgl_native_vertex_t captured[1024];
static const xgl_native_vertex_t *queued[1024];
static xgl_native_vertex_t queued_copy[1024];
static int queued_count, flush_calls, palette_calls, failed_sample;
static int captured_count, captured_lit, captured_zwrite;
static int captured_calls, varying_material, prepare_calls, filter_calls;
static int filter_variation;
static float filter_min_s, filter_max_s, filter_min_t, filter_max_t;
static unsigned int color_revision, texture_revision;
static float gamma_scale;
static int gamma_curve;
static float captured_opacity;
static unsigned long profile_counts[XGL_PROFILE_COUNTER_COUNT];
static int profile_sections[XGL_PROFILE_SECTION_COUNT];

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
unsigned int XGL_ColorRevision(void) { return color_revision; }
void XGL_PrepareColor(float rgb[3])
{
    int i;
    ++prepare_calls;
    for (i = 0; i < 3; ++i) {
        if (gamma_curve) rgb[i] *= rgb[i];
        rgb[i] *= gamma_scale;
    }
}
void XGL_PreparePaletteColor(int index, float rgb[3])
{
    int i;
    assert(index >= 0 && index <= 255);
    ++palette_calls;
    for (i = 0; i < 3; ++i) {
        rgb[i] = xgl_palette[index * 3 + i] / 255.0f;
        if (gamma_curve) rgb[i] *= rgb[i];
        rgb[i] *= gamma_scale;
    }
}
void XGL_Flush(void)
{
    int i;
    ++flush_calls;
    for (i = 0; i < queued_count; ++i)
        assert(!memcmp(queued[i], &queued_copy[i], sizeof(*queued[i])));
    queued_count = 0;
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
qboolean XGL_SampleTextureIndex(int texture, float s, float t, int *index)
{
    assert(texture == 1 || texture == 2);
    ++sample_calls;
    if (failed_sample) return false;
    s -= (float)floor(s); t -= (float)floor(t);
    *index = ((int)(s * 255) + (int)(t * 127)) & 255;
    return true;
}
qboolean XGL_SampleTexture(int texture, float s, float t, float rgb[3], const byte *translation)
{
    int i, index;
    assert(texture == 1 || texture == 2);
    assert(translation == NULL);
    if (varying_material) {
        if (!XGL_SampleTextureIndex(texture, s, t, &index)) return false;
        for (i = 0; i < 3; ++i) rgb[i] = xgl_palette[index * 3 + i] / 255.0f;
    } else {
        ++sample_calls;
        rgb[0] = .8f; rgb[1] = .4f; rgb[2] = .2f;
    }
    return true;
}

qboolean XGL_SampleTextureFiltered(int texture, float s, float t, float fs, float ft,
                                  float rgb[3], const byte *translation)
{
    qboolean sampled;
    assert(fs >= 0 && ft >= 0);
    ++filter_calls;
    if (fs < filter_min_s) filter_min_s = fs;
    if (fs > filter_max_s) filter_max_s = fs;
    if (ft < filter_min_t) filter_min_t = ft;
    if (ft > filter_max_t) filter_max_t = ft;
    sampled = XGL_SampleTexture(texture, s, t, rgb, translation);
    if (sampled && filter_variation) {
        rgb[0] /= 1 + fs;
        rgb[1] /= 1 + ft;
        rgb[2] /= 1 + fs + ft;
    }
    return sampled;
}

void XGL_DrawNativeTriangles(const xgl_native_vertex_t *vertices, int count,
                             int lit, float opacity, int zwrite)
{
    int i;
    assert(count > 0 && count % 3 == 0 && captured_count + count <= 1024);
    assert(queued_count + count <= 1024);
    for (i = 0; i < count; ++i) {
        assert(fabs(DotProduct(vertices[i].normal, vertices[i].normal) - 1) < .0001);
        captured[captured_count++] = vertices[i];
        queued[queued_count] = vertices + i;
        queued_copy[queued_count++] = vertices[i];
    }
    captured_lit = lit; captured_opacity = opacity; captured_zwrite = zwrite;
    ++captured_calls;
}

xgl_native_vertex_t *XGL_ReserveTriangles(int *count, int lit, float opacity, int zwrite)
{
    xgl_native_vertex_t *vertices;
    assert(*count > 0 && *count % 3 == 0);
    if (*count > 12) *count = 12; /* Exercise callers handling partial space. */
    assert(captured_count + *count <= 1024);
    vertices = captured + captured_count;
    captured_count += *count;
    captured_lit = lit; captured_opacity = opacity; captured_zwrite = zwrite;
    ++captured_calls;
    return vertices;
}

static void begin_frame(void)
{
    XGL_Flush(); /* The preceding frame has completed. */
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
    fail_alloc_after = -1;
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
    for (i = 0; i < 256; ++i) d_lightstylevalue[i] = 256;
    pvs_calls = sample_calls = efrag_calls = reject_box = captured_count = 0;
    captured_calls = varying_material = prepare_calls = filter_calls = 0;
    filter_variation = 0;
    filter_min_s = filter_min_t = FLT_MAX;
    filter_max_s = filter_max_t = 0;
    color_revision = texture_revision = 1; gamma_scale = 1; gamma_curve = 0;
    palette_calls = failed_sample = flush_calls = 0;
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
    first_color = captured[0].color.rgb[0];
    VectorCopy(captured[0].position, first_position);
    samples = sample_calls;
    realtime = .25;
    begin_frame();
    assert(captured_count == 6 && captured_calls == 1 && sample_calls == samples + 4);
    assert(fabs(captured[0].color.rgb[0] - first_color) > .01);
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
    assert(fabs(captured[0].color.rgb[0] - 1.0f / 255) < .0001);
    assert(sample_calls == 0); /* split image samples bypass the full-image registry */
    realtime = 1;
    begin_frame();
    assert(captured_count == 6 && captured_calls == 1);
    assert(fabs(captured[0].color.rgb[0] - 9.0f / 255) < .0001);
    /* A nonzero foreground texel wins at the faster 16-unit scroll. */
    pixels[16 * 256 + 16] = 200;
    begin_frame();
    assert(captured_calls == 1);
    assert(fabs(captured[0].color.rgb[0] - 200.0f / 255) < .0001);
    pixels[16 * 256 + 16] = 0;
    realtime = 0;
    r_origin[0] = 16;
    begin_frame();
    assert(captured_calls == 1 && captured_lit == 0);
    assert(fabs(captured[0].color.rgb[0] - 1.0f / 255) > .01);
    XGL_WorldShutdown();
    free(sky);
}

/* Reference the original nearest-texel animation; edge continuity must
 * retain these samples without introducing texture filtering. */
static void water_sample_at(msurface_t *surface, texture_t *texture,
                            const float *position, float rgb[3])
{
    float os, ot, s, t, phase;
    os = DotProduct(position, surface->texinfo->vecs[0]);
    ot = DotProduct(position, surface->texinfo->vecs[1]);
    phase = (float)fmod(realtime, 2 * M_PI);
    s = (os + xgl_turb_sin[(int)((ot * .125f + phase) * XGL_TURB_SCALE) & 255]) / 64;
    t = (ot + xgl_turb_sin[(int)((os * .125f + phase) * XGL_TURB_SCALE) & 255]) / 64;
    assert(XGL_SampleTexture(texture->gl_texturenum, s, t, rgb, NULL));
    XGL_PrepareColor(rgb);
}

static int inside_edge(const float *position, const float *a, const float *b,
                        float *fraction)
{
    vec3_t edge, offset;
    float length, t;
    int axis;
    VectorSubtract(b, a, edge);
    VectorSubtract(position, a, offset);
    length = DotProduct(edge, edge);
    if (length == 0) return 0;
    t = DotProduct(offset, edge) / length;
    if (t <= .000001f || t >= .999999f) return 0;
    for (axis = 0; axis < 3; ++axis)
        if (fabs(offset[axis] - t * edge[axis]) > .0001) return 0;
    *fraction = t;
    return 1;
}

/* Every emitted edge must have the same endpoints on both sides. A vertex
 * strictly inside somebody else's edge would reproduce the original seam. */
static void check_conforming_mesh(void)
{
    int i, j, edge, channel;
    float fraction;
    for (i = 0; i < captured_count; ++i) {
        for (j = 0; j < captured_count; j += 3)
            for (edge = 0; edge < 3; ++edge)
                assert(!inside_edge(captured[i].position,
                    captured[j + edge].position,
                    captured[j + (edge + 1) % 3].position, &fraction));
        for (j = i + 1; j < captured_count; ++j)
            if (VectorCompare(captured[i].position, captured[j].position))
                for (channel = 0; channel < 3; ++channel)
                    assert(fabs(captured[i].color.rgb[channel] -
                                captured[j].color.rgb[channel]) < .000001);
    }
}

static void test_water_edge_continuity(void)
{
    static const double times[] = {0, .25, 1, 1000000.25};
    xgl_native_vertex_t original[30];
    xgl_surface_mesh_t *mesh;
    entity_t entity;
    unsigned short *map;
    float expected[3], fraction;
    vec3_t a = {0, 0, 0}, c = {64, 64, 0};
    int phase, i, channel;
    setup_world();
    varying_material = 1;
    test_surface.flags = SURF_DRAWTURB;
    test_vertices[1].position[0] = 512;
    test_vertices[2].position[0] = test_vertices[2].position[1] = 64;
    test_vertices[3].position[1] = 64;
    xgl_subdivide.value = 128;
    for (phase = 0; phase < 4; ++phase) {
        realtime = times[phase]; gamma_curve = phase & 1; gamma_scale = .65f;
        begin_frame();
        assert(captured_count == 30 && captured_lit == 0);
        assert(captured_zwrite == 1 && captured_opacity == 1 && !filter_calls);
        assert(fabs(captured_area() - 18432) < .01);
        mesh = &xgl_surfaces[0].detail;
        assert(mesh->count == captured_count);
        if (!phase) memcpy(original, captured, sizeof(original));
        check_conforming_mesh();
        for (i = 0; i < captured_count; ++i) {
            assert(VectorCompare(captured[i].position, original[i].position));
            assert(VectorCompare(captured[i].normal, original[i].normal));
            assert(captured[i].position[2] == 0);
            water_sample_at(&test_surface, &test_texture, captured[i].position, expected);
            for (channel = 0; channel < 3; ++channel)
                assert(fabs(captured[i].color.rgb[channel] - expected[channel]) < .000001);
            /* Refinement never introduces a differently sampled midpoint
             * on A-C, which is also an edge of the unsplit A-C-D face. */
            assert(!inside_edge(captured[i].position, a, c, &fraction));
        }
    }
    /* Edge correction is independent of the optional duplicate sample map. */
    memcpy(original, captured, sizeof(original));
    map = mesh->sample_vertex; mesh->sample_vertex = NULL;
    XGL_UpdateSurfaceMesh(mesh, &test_surface, &test_texture, &r_worldentity);
    for (i = 0; i < mesh->count; ++i)
        for (channel = 0; channel < 3; ++channel)
            assert(fabs(mesh->vertices[i].color.rgb[channel] - original[i].color.rgb[channel]) < .000001);
    mesh->sample_vertex = map;

    /* A rotated/transformed brush retains the same local water colors. */
    XGL_Flush(); captured_count = captured_calls = 0; xgl_detail_used = 0;
    memset(&entity, 0, sizeof(entity));
    entity.model = &test_model;
    entity.angles[1] = 90;
    entity.origin[0] = 11; entity.origin[1] = -7; entity.origin[2] = 19;
    XGL_DrawSurface(&test_model, &test_surface, &entity, 1);
    assert(captured_count == 30 && !filter_calls);
    for (i = 0; i < captured_count; ++i) {
        assert(fabs(captured[i].position[0] - (11 - original[i].position[1])) < .001);
        assert(fabs(captured[i].position[1] - (-7 + original[i].position[0])) < .001);
        assert(captured[i].position[2] == 19);
        for (channel = 0; channel < 3; ++channel)
            assert(fabs(captured[i].color.rgb[channel] - original[i].color.rgb[channel]) < .000001);
    }
    XGL_WorldShutdown();
}

static void test_surface_boundary_tjunctions(int turbulent)
{
    static const float xy[10][2] = {
        {128, 0}, {128, 64}, {128, 128}, {0, 128}, {0, 0},
        {128, 0}, {256, 0}, {256, 128}, {128, 128}, {128, 64}
    };
    msurface_t surfaces[2], *marks[2];
    mvertex_t vertices[10];
    medge_t edges[10];
    xgl_native_vertex_t previous[256];
    int surfedges[10], i, j, k, phase, detailed, cost[2], left_count, shared, previous_count;
    float rgb[3], area;
    vec3_t side1, side2, cross;
    setup_world();
    varying_material = 1; filter_variation = !turbulent;
    previous_count = 0;
    memset(vertices, 0, sizeof(vertices));
    memset(edges, 0, sizeof(edges));
    for (i = 0; i < 2; ++i) {
        surfaces[i] = test_surface;
        surfaces[i].flags = turbulent ? SURF_DRAWTURB : 0;
        surfaces[i].numedges = 5; surfaces[i].firstedge = i * 5;
        marks[i] = &surfaces[i];
        for (j = 0; j < 5; ++j) {
            vertices[i * 5 + j].position[0] = xy[i * 5 + j][0];
            vertices[i * 5 + j].position[1] = xy[i * 5 + j][1];
            edges[i * 5 + j].v[0] = i * 5 + j;
            edges[i * 5 + j].v[1] = i * 5 + (j + 1) % 5;
            surfedges[i * 5 + j] = i * 5 + j;
        }
    }
    test_model.surfaces = surfaces;
    test_model.numsurfaces = test_model.nummodelsurfaces = 2;
    test_model.vertexes = vertices; test_model.edges = edges; test_model.surfedges = surfedges;
    test_node.numsurfaces = 2;
    test_leaves[1].firstmarksurface = marks; test_leaves[1].nummarksurfaces = 2;
    XGL_WorldNewMap();
    begin_frame();
    for (i = 0; i < 2; ++i) {
        assert(xgl_surfaces[i].coarse.count == 9);
        cost[i] = xgl_surfaces[i].detail.count / 3;
        assert(cost[i] > 3);
    }
    for (phase = 0; phase < 4; ++phase) {
        detailed = phase & 1;
        r_origin[0] = detailed ? 192 : 64;
        xgl_detailtris.value = cost[detailed];
        realtime = phase * .37; gamma_curve = turbulent && phase >= 2; gamma_scale = .7f;
        ++color_revision;
        begin_frame();
        assert(xgl_surfaces[detailed].detail_frame == r_framecount);
        assert(xgl_surfaces[1 - detailed].detail_frame != r_framecount);
        assert(captured_count == cost[detailed] * 3 + 9);
        if (turbulent) assert(!filter_calls);
        else {
            assert(filter_calls > 0);
            assert(fabs(filter_min_s - 1.0 / 12) < .000001 && filter_min_s == filter_max_s);
            assert(fabs(filter_min_t - 1.0 / 12) < .000001 && filter_min_t == filter_max_t);
        }
        assert(xgl_detail_used == cost[detailed]);
        assert(fabs(captured_area() - 32768) < .01);
        left_count = detailed ? 9 : cost[0] * 3;
        shared = 0;
        check_conforming_mesh();
        for (i = 0; i < captured_count; i += 3) {
            VectorSubtract(captured[i + 1].position, captured[i].position, side1);
            VectorSubtract(captured[i + 2].position, captured[i].position, side2);
            CrossProduct(side1, side2, cross);
            area = cross[2];
            assert(area > 0); /* No degenerate fan skips the middle BSP vertex. */
        }
        for (i = 0; i < captured_count; ++i) {
            assert(captured[i].position[2] == 0 && captured[i].normal[2] == 1);
            if (turbulent) {
                j = i >= left_count;
                water_sample_at(&surfaces[j], &test_texture, captured[i].position, rgb);
                for (j = 0; j < 3; ++j)
                    assert(fabs(captured[i].color.rgb[j] - rgb[j]) < .000001);
            } else {
                /* Same material point retains its color as the two faces
                 * exchange the limited detail allowance. */
                for (j = 0; j < previous_count; ++j)
                    if (VectorCompare(captured[i].position, previous[j].position))
                        for (k = 0; k < 3; ++k)
                            assert(fabs(captured[i].color.rgb[k] - previous[j].color.rgb[k]) < .000001);
            }
            if (captured[i].position[0] != 128) continue;
            ++shared;
            assert(captured[i].position[1] == 0 || captured[i].position[1] == 64 ||
                   captured[i].position[1] == 128);
        }
        assert(shared && xgl_cache_bytes <= XGL_WORLD_CACHE_BYTES);
        assert(captured_count <= 256);
        memcpy(previous, captured, captured_count * sizeof(*captured));
        previous_count = captured_count;
    }
    if (!turbulent) {
        /* Continuity must not erase a real lightstyle difference across
         * otherwise matching material coordinates. */
        surfaces[1].styles[0] = 1; d_lightstylevalue[1] = 128;
        gamma_curve = 0; gamma_scale = 1;
        ++color_revision;
        begin_frame();
        shared = 0;
        for (i = 0; i < 9; ++i)
            for (j = 9; j < captured_count; ++j)
                if (VectorCompare(captured[i].position, captured[j].position)) {
                    ++shared;
                    for (k = 0; k < 3; ++k)
                        assert(fabs(captured[j].color.rgb[k] - captured[i].color.rgb[k] * .5f) < .000001);
                }
        assert(shared);
        surfaces[1].styles[0] = 0;
    }
    /* Allocation failures and the exhausted world cache keep complete,
     * conforming coarse geometry, including the collinear boundary point. */
    for (phase = 0; phase < 5; ++phase) {
        fail_alloc_after = -1;
        XGL_WorldNewMap();
        if (phase == 4) xgl_cache_bytes = XGL_WORLD_CACHE_BYTES;
        else fail_alloc_after = phase;
        xgl_detailtris.value = 4096;
        begin_frame();
        assert(captured_count >= 18 && fabs(captured_area() - 32768) < .01);
        assert(xgl_cache_bytes <= XGL_WORLD_CACHE_BYTES);
        check_conforming_mesh();
        fail_alloc_after = -1;
        XGL_WorldShutdown();
    }
    /* Reversing the boundary retains the same complete polygon with
     * reversed triangle winding, including both collinear boundary points. */
    for (i = 0; i < 2; ++i)
        for (j = 0; j < 5; ++j) {
            edges[i * 5 + j].v[0] = i * 5 + (5 - j) % 5;
            edges[i * 5 + j].v[1] = i * 5 + (9 - j) % 5;
        }
    xgl_detailtris.value = 0;
    XGL_WorldNewMap();
    begin_frame();
    assert(captured_count == 18 && fabs(captured_area() - 32768) < .01);
    check_conforming_mesh();
    for (i = 0; i < captured_count; i += 3) {
        VectorSubtract(captured[i + 1].position, captured[i].position, side1);
        VectorSubtract(captured[i + 2].position, captured[i].position, side2);
        CrossProduct(side1, side2, cross);
        assert(cross[2] < 0);
    }
    XGL_WorldShutdown();
}

static void test_architecture_filter_footprints(void)
{
    static const float step[] = {3, 32, 600};
    static const float expected_s[] = {1.0f / 24, 1.0f / 6, 4.0f / 3};
    static const float expected_t[] = {1.0f / 96, 1.0f / 24, 1.0f / 3};
    int i;
    for (i = 0; i < 3; ++i) {
        setup_world();
        varying_material = filter_variation = 1;
        test_texture.width = 64; test_texture.height = 256;
        xgl_subdivide.value = step[i];
        begin_frame();
        assert(filter_calls > 0);
        assert(fabs(filter_min_s - expected_s[i]) < .000001 && filter_min_s == filter_max_s);
        assert(fabs(filter_min_t - expected_t[i]) < .000001 && filter_min_t == filter_max_t);
        check_conforming_mesh();
    }
}


static void test_cache_and_styles(void)
{
    float original;
    int count, samples, prepares;
    setup_world();
    begin_frame();
    assert(captured_count == 54 && captured_lit == 1);
    assert(captured_zwrite == 1 && captured_opacity == 1);
    assert(fabs(captured_area() - 128 * 128) < .01);
    assert(xgl_cache_bytes <= XGL_WORLD_CACHE_BYTES);
    original = captured[0].color.rgb[0];
    samples = sample_calls; count = captured_count; prepares = prepare_calls;
    assert(filter_calls == count);
    begin_frame();
    assert(captured_count == count && sample_calls == samples && pvs_calls == 1);
    assert(efrag_calls == 2 && prepare_calls == prepares);
    gamma_scale = .5f; ++color_revision;
    begin_frame();
    assert(sample_calls == samples && prepare_calls == prepares + count);
    assert(fabs(captured[0].color.rgb[0] - original * .5f) < .0001);
    gamma_scale = 1; ++color_revision;
    begin_frame();
    assert(sample_calls == samples);
    assert(fabs(captured[0].color.rgb[0] - original) < .0001);
    d_lightstylevalue[0] = 128;
    begin_frame();
    assert(sample_calls == samples);
    assert(fabs(captured[0].color.rgb[0] - original * .5f) < .0001);
    r_fullbright.value = 1;
    begin_frame();
    assert(captured_lit == 0 && sample_calls == samples);
    assert(fabs(captured[0].color.rgb[0] - .8f) < .0001);
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
    assert(profile_counts[XGL_PROFILE_WORLD_BUILD_VERTICES] == 6 + 54);
    assert(profile_counts[XGL_PROFILE_WORLD_MESH_FAILURES] == 0);
    assert(profile_counts[XGL_PROFILE_WORLD_CACHED_SURFACES] == 1);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_UPDATES] == 1);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_VERTICES] == 54);
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
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_VERTICES] == 54);
    assert(profile_counts[XGL_PROFILE_WORLD_COLOR_HITS] == 0);
    test_surface.flags = SURF_DRAWTURB;
    begin_frame();
    assert(profile_counts[XGL_PROFILE_WORLD_ANIMATED_VERTICES] == 54);
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
    xgl_detailtris.value = 18;
    XGL_WorldNewMap();
    begin_frame();
    /* BSP order reaches the smaller face first, but the larger screen-area
     * estimate receives the detail budget. Both faces still draw, in order. */
    assert(xgl_detail_used == 18 && captured_count == 6 + 54);
    assert(xgl_surfaces[0].detail_frame != r_framecount);
    assert(xgl_surfaces[1].detail_frame == r_framecount);
    assert(captured[0].position[0] == 32 && captured[6].position[0] == 0);
    assert(fabs(captured_area() - (64 * 64 + 128 * 128)) < .01);
    assert(xgl_cache_bytes <= XGL_WORLD_CACHE_BYTES);
    xgl_detailtris.value = 0;
    begin_frame();
    assert(captured_count == 12 && xgl_detail_used == 0);
    assert(fabs(captured_area() - (64 * 64 + 128 * 128)) < .01);
    xgl_detailtris.value = 18;
    surfaces[1].flags = SURF_DRAWTURB; r_wateralpha.value = 0;
    begin_frame();
    assert(captured_count == 18 && xgl_detail_used == 6 && !xgl_water_count);
    assert(xgl_surfaces[0].detail_frame == r_framecount);
    assert(xgl_surfaces[1].detail_frame != r_framecount);
    XGL_WorldShutdown();
}

static void compare_sample_map(xgl_surface_mesh_t *mesh, texture_t *texture,
                                entity_t *entity)
{
    float colors[96][3];
    unsigned short *map;
    int i, samples;
    assert(mesh->count <= 96 && mesh->sample_vertex);
    XGL_UpdateSurfaceMesh(mesh, &test_surface, texture, entity);
    for (i = 0; i < mesh->count; ++i)
        VectorCopy(mesh->vertices[i].color.rgb, colors[i]);
    samples = sample_calls;
    map = mesh->sample_vertex;
    mesh->sample_vertex = NULL;
    XGL_UpdateSurfaceMesh(mesh, &test_surface, texture, entity);
    if (test_surface.flags & SURF_DRAWTURB) assert(sample_calls == samples + mesh->count);
    for (i = 0; i < mesh->count; ++i)
        assert(VectorCompare(mesh->vertices[i].color.rgb, colors[i]));
    mesh->sample_vertex = map;
}

static void test_shared_animated_samples(void)
{
    xgl_surface_mesh_t *mesh;
    texture_t *sky;
    entity_t entity;
    byte *pixels;
    size_t bytes;
    int i, failed, count, before, channel;
    float expected[3];
    setup_world();
    varying_material = 1;
    test_surface.flags = SURF_DRAWTURB;
    begin_frame();
    mesh = &xgl_surfaces[0].detail;
    assert(captured_count == 54 && sample_calls == 12 && palette_calls == 12);
    assert(profile_counts[XGL_PROFILE_WORLD_ANIMATED_VERTICES] == 54);
    assert(profile_counts[XGL_PROFILE_WORLD_ANIMATED_SAMPLES] == 12);
    assert(profile_sections[XGL_PROFILE_WORLD_WATER] == 1);
    assert(profile_sections[XGL_PROFILE_WORLD_SKY] == 0);
    /* Same positions share samples, without changing any emitted triangle. */
    for (i = 0; i < mesh->count; ++i) {
        assert(mesh->sample_vertex[i] <= i);
        assert(VectorCompare(mesh->vertices[i].position,
                             mesh->vertices[mesh->sample_vertex[i]].position));
    }
    test_texinfo.vecs[0][3] = -17; test_texinfo.vecs[1][3] = 23;
    for (i = 0; i < mesh->count; ++i) {
        mesh->data[i].st[0] -= 17; mesh->data[i].st[1] += 23;
    }
    for (i = 0; i < 4; ++i) {
        realtime = i == 3 ? 1000000.25 : (i - 1) * .25;
        gamma_scale = .65f;
        compare_sample_map(mesh, &test_texture, &r_worldentity);
        /* Every vertex retains the original nearest-texel animation. */
        for (count = 0; count < mesh->count; ++count) {
            water_sample_at(&test_surface, &test_texture,
                mesh->vertices[count].position, expected);
            /* Fast math may reassociate the mock's palette/gamma scaling.
             * Allow float rounding, far below a change of one palette byte. */
            for (channel = 0; channel < 3; ++channel)
                assert(fabs((double)expected[channel] -
                            mesh->vertices[count].color.rgb[channel]) <= FLT_EPSILON);
        }
    }
    failed_sample = 1;
    XGL_UpdateSurfaceMesh(mesh, &test_surface, &test_texture, &r_worldentity);
    for (i = 0; i < mesh->count; ++i)
        assert(mesh->vertices[i].color.rgb[0] == .5f * gamma_scale);

    setup_world();
    test_surface.flags = SURF_DRAWSKY;
    sky = (texture_t *)calloc(1, sizeof(texture_t) + 256 * 128);
    assert(sky);
    sky->width = 256; sky->height = 128; sky->offsets[0] = sizeof(texture_t);
    sky->gl_texturenum = 1; test_texinfo.texture = sky;
    pixels = (byte *)sky + sky->offsets[0];
    for (i = 0; i < 256 * 128; ++i) pixels[i] = (byte)(i * 17 + i / 256);
    begin_frame();
    mesh = &xgl_surfaces[0].detail;
    assert(captured_count == 96 && palette_calls == 25 && prepare_calls == 0);
    assert(profile_counts[XGL_PROFILE_WORLD_ANIMATED_SAMPLES] == 25);
    assert(profile_sections[XGL_PROFILE_WORLD_SKY] == 1);
    memset(&entity, 0, sizeof(entity));
    entity.model = &test_model;
    entity.angles[1] = 27; entity.origin[0] = -31; entity.origin[2] = 15;
    realtime = 1000000.25; gamma_scale = .7f;
    compare_sample_map(mesh, sky, &entity);
    XGL_WorldShutdown();
    free(sky);

    /* Optional sample-map allocation failure leaves the full mesh working.
     * Exercise each allocation, including cleanup when the sort list fails. */
    for (failed = 0; failed < 3; ++failed) {
        setup_world();
        test_surface.flags = SURF_DRAWTURB; xgl_detailtris.value = 0;
        bytes = xgl_cache_bytes;
        fail_alloc_after = failed;
        begin_frame();
        assert(captured_count == 6 && xgl_surfaces[0].coarse.sample_vertex == NULL);
        if (!failed) assert(xgl_cache_bytes == bytes);
        else assert(xgl_cache_bytes == bytes + 6 * (sizeof(xgl_native_vertex_t) + sizeof(xgl_cached_vertex_t)));
        assert(sample_calls == 6);
    }
    setup_world();
    test_surface.flags = SURF_DRAWTURB; xgl_detailtris.value = 0;
    bytes = 6 * (sizeof(xgl_native_vertex_t) + sizeof(xgl_cached_vertex_t));
    xgl_cache_bytes = XGL_WORLD_CACHE_BYTES - bytes - 4 * 6 + 1;
    begin_frame();
    assert(captured_count == 6 && xgl_surfaces[0].coarse.vertices);
    assert(xgl_surfaces[0].coarse.sample_vertex == NULL && sample_calls == 6);
    assert(xgl_cache_bytes <= XGL_WORLD_CACHE_BYTES);
    before = flush_calls;
    XGL_WorldShutdown();
    assert(!queued_count && flush_calls == before + 1 && !xgl_cache_bytes);
}

static void test_borrowed_mesh_lifetime(void)
{
    int before;
    float old_color;
    setup_world();
    xgl_detailtris.value = 0;
    begin_frame();
    assert(queued_count == 6);
    before = flush_calls;
    XGL_DrawSurface(&test_model, &test_surface, &r_worldentity, 1);
    assert(flush_calls == before && queued_count == 12); /* Unchanged cache batches. */
    old_color = captured[0].color.rgb[0];
    gamma_scale = .5f; ++color_revision;
    XGL_DrawSurface(&test_model, &test_surface, &r_worldentity, 1);
    assert(flush_calls == before + 1 && queued_count == 6);
    assert(captured[12].color.rgb[0] == old_color * .5f);
    XGL_WorldShutdown(); /* The mock checks pointers before the arrays are freed. */
    assert(!queued_count);

    setup_world();
    test_surface.flags = SURF_DRAWTURB; xgl_detailtris.value = 0;
    begin_frame();
    before = flush_calls; old_color = captured[0].color.rgb[0];
    realtime = .25;
    XGL_DrawSurface(&test_model, &test_surface, &r_worldentity, 1);
    assert(flush_calls == before + 1 && queued_count == 6);
    assert(captured[6].color.rgb[0] != old_color);
    XGL_WorldShutdown();
}

int main(void)
{
    test_water_edge_continuity();
    test_surface_boundary_tjunctions(1);
    test_surface_boundary_tjunctions(0);
    test_architecture_filter_footprints();
    test_shared_animated_samples();
    test_borrowed_mesh_lifetime();
    test_cache_and_styles();
    test_projected_detail_priority();
    test_pvs_and_fallback();
    test_profile_cache_events();
    test_brush_water_and_point_light();
    test_animated_water_and_sky();
    XGL_WorldShutdown();
    assert(xgl_cache_bytes == 0 && xgl_surfaces == NULL);
    puts("XGL world/light contracts passed");
    return 0;
}
