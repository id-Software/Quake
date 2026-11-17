/*
 * Bounded sampled-color BSP renderer for Sun XGL.
 * Derived from Quake, Copyright (C) 1996-1997 Id Software, Inc.
 * GNU GPL version 2 or (at your option) any later version.
 */
#include "quakedef.h"
#include "xgl_audio.h"
#include "xgl_profile.h"

#define XGL_WORLD_CACHE_BYTES (8UL * 1024UL * 1024UL)
#define XGL_SURFACE_DETAIL_VERTS 12288
#define XGL_WATER_QUEUE 4096
#define XGL_WORLD_BATCH 384
#define XGL_TURB_SCALE (256.0 / (2 * M_PI))

static const float xgl_turb_sin[256] = {
#include "gl_warp_sin.h"
};

typedef struct {
    float st[2];
    byte material[3];
    byte samples[MAXLIGHTMAPS];
} xgl_cached_vertex_t;

typedef struct {
    xgl_native_vertex_t *vertices;
    xgl_cached_vertex_t *data;
    unsigned short *sample_vertex;
    int count, texture, style[MAXLIGHTMAPS], fullbright, queued_frame;
    unsigned int texture_revision, color_revision;
} xgl_surface_mesh_t;

typedef struct {
    xgl_surface_mesh_t coarse, detail;
    int coarse_attempted, detail_attempted;
    vec3_t center;
    int center_valid, detail_frame;
    float area, radius, priority;
} xgl_surface_cache_t;

typedef struct {
    model_t *model;
    msurface_t *surface;
    entity_t *entity;
    float distance;
} xgl_water_surface_t;

static xgl_surface_cache_t *xgl_surfaces;
static model_t *xgl_cached_world;
static size_t xgl_cache_bytes;
static int xgl_surface_count, xgl_detail_used;
static int *xgl_world_order, *xgl_detail_order, xgl_world_count;
static model_t *xgl_pvs_world;
static mleaf_t *xgl_pvs_leaf;
static int xgl_pvs_novis;
static float xgl_cache_step;
static xgl_water_surface_t xgl_water[XGL_WATER_QUEUE];
static int xgl_water_count;

cvar_t xgl_subdivide = {"xgl_subdivide", "128", false, false, 0, NULL};
cvar_t xgl_detailtris = {"xgl_detailtris", "1024", false, false, 0, NULL};
cvar_t xgl_detaildistance = {"xgl_detaildistance", "512", false, false, 0, NULL};

void XGL_SurfaceLightSamples(msurface_t *, const float *, byte[MAXLIGHTMAPS]);
float XGL_SurfaceLightScale(msurface_t *, const byte[MAXLIGHTMAPS]);

void XGL_SubdivideSurface (msurface_t *surface)
{
    /* The loader's sky/water hook deliberately allocates no GL polygons.
     * Ordinary and turbulent surfaces share the bounded cache below. */
    (void)surface;
}

void XGL_WorldShutdown (void)
{
    int i;
    if (xgl_surfaces) {
        XGL_Flush(); /* Native batches may still borrow cached vertex arrays. */
        for (i = 0; i < xgl_surface_count; ++i) {
            free(xgl_surfaces[i].coarse.vertices);
            free(xgl_surfaces[i].coarse.sample_vertex);
            free(xgl_surfaces[i].detail.vertices);
            free(xgl_surfaces[i].detail.sample_vertex);
        }
        free(xgl_surfaces);
    }
    free(xgl_world_order);
    xgl_world_order = xgl_detail_order = NULL;
    xgl_world_count = 0;
    xgl_surfaces = NULL;
    xgl_cached_world = NULL;
    xgl_surface_count = 0;
    xgl_cache_bytes = 0;
    xgl_water_count = 0;
    xgl_pvs_world = NULL;
    xgl_pvs_leaf = NULL;
}

void XGL_WorldNewMap (void)
{
    size_t bytes;
    XGL_WorldShutdown();
    xgl_cached_world = cl.worldmodel;
    xgl_cache_step = xgl_subdivide.value;
    if (!xgl_cached_world || xgl_cached_world->numsurfaces <= 0) return;
    if ((size_t)xgl_cached_world->numsurfaces > XGL_WORLD_CACHE_BYTES /
        (sizeof(*xgl_surfaces) + 2 * sizeof(int))) return;
    bytes = (size_t)xgl_cached_world->numsurfaces * (sizeof(*xgl_surfaces) + 2 * sizeof(int));
    if (bytes <= XGL_WORLD_CACHE_BYTES) {
        xgl_surfaces = (xgl_surface_cache_t *)calloc((size_t)xgl_cached_world->numsurfaces, sizeof(*xgl_surfaces));
        xgl_world_order = (int *)malloc((size_t)xgl_cached_world->numsurfaces * 2 * sizeof(int));
        if (xgl_surfaces && xgl_world_order) {
            xgl_detail_order = xgl_world_order + xgl_cached_world->numsurfaces;
            xgl_surface_count = xgl_cached_world->numsurfaces;
            xgl_cache_bytes = bytes;
        } else {
            free(xgl_surfaces); free(xgl_world_order);
            xgl_surfaces = NULL; xgl_world_order = NULL;
        }
    }
    Con_Printf("XGL: sampled BSP cache limited to 8 MiB; detail depth <= 2; projected-size detail priority; complete coarse fallback\n");
}

static void XGL_SurfacePoint (model_t *model, msurface_t *surface, int index, vec3_t point)
{
    int edge;
    edge = model->surfedges[surface->firstedge + index];
    VectorCopy(model->vertexes[model->edges[edge >= 0 ? edge : -edge].v[edge < 0]].position, point);
}

static void XGL_BrushBasis (entity_t *entity, vec3_t forward, vec3_t right, vec3_t up)
{
    AngleVectors(entity->angles, forward, right, up);
}

static void XGL_BrushPoint (const float *point, entity_t *entity,
                            const float *forward, const float *right,
                            const float *up, float *result)
{
    int i;
    for (i = 0; i < 3; ++i)
        result[i] = entity->origin[i] + point[0] * forward[i] - point[1] * right[i] + point[2] * up[i];
}

static texture_t *XGL_SurfaceTexture (texture_t *base, entity_t *entity)
{
    int tick, loops;
    if (entity && entity->frame && base->alternate_anims) base = base->alternate_anims;
    if (!base->anim_total) return base;
    tick = (int)(cl.time * 10) % base->anim_total;
    if (tick < 0) tick += base->anim_total;
    for (loops = 0; loops < 100; ++loops) {
        if (tick >= base->anim_min && tick < base->anim_max) return base;
        base = base->anim_next;
        if (!base) Host_Error("XGL: broken texture animation");
    }
    Host_Error("XGL: texture animation cycle");
    return base;
}

texture_t *R_TextureAnimation (texture_t *base)
{
    return XGL_SurfaceTexture(base, currententity);
}

static void XGL_InitCachedVertex (xgl_native_vertex_t *draw, xgl_cached_vertex_t *vertex,
                                  msurface_t *surface, const float *point)
{
    int i;
    VectorCopy(point, draw->position);
    for (i = 0; i < 3; ++i)
        draw->normal[i] = surface->plane->normal[i] * ((surface->flags & SURF_PLANEBACK) ? -1 : 1);
    for (i = 0; i < 2; ++i)
        vertex->st[i] = DotProduct(point, surface->texinfo->vecs[i]) + surface->texinfo->vecs[i][3];
    XGL_SurfaceLightSamples(surface, point, vertex->samples);
}

/* The same bounded recursion counts and emits. Texture-space edge span,
 * rather than world area alone, keeps detail useful on oblique materials. */
static int XGL_SubdivideTriangle (msurface_t *surface, const float *a, const float *b,
                                 const float *c, int depth, float step,
                                 xgl_native_vertex_t *output, xgl_cached_vertex_t *data)
{
    vec3_t ab, bc, ca;
    float sa, sb, sc, low, high;
    int axis, split, count, n;
    split = 0;
    if (depth > 0) {
        for (axis = 0; axis < 2; ++axis) {
            sa = DotProduct(a, surface->texinfo->vecs[axis]);
            sb = DotProduct(b, surface->texinfo->vecs[axis]);
            sc = DotProduct(c, surface->texinfo->vecs[axis]);
            low = sa < sb ? sa : sb; if (sc < low) low = sc;
            high = sa > sb ? sa : sb; if (sc > high) high = sc;
            if (high - low > step) split = 1;
        }
    }
    if (!split) {
        if (output) {
            XGL_InitCachedVertex(output, data, surface, a);
            XGL_InitCachedVertex(output + 1, data + 1, surface, b);
            XGL_InitCachedVertex(output + 2, data + 2, surface, c);
        }
        return 3;
    }
    for (axis = 0; axis < 3; ++axis) {
        ab[axis] = (a[axis] + b[axis]) * .5f;
        bc[axis] = (b[axis] + c[axis]) * .5f;
        ca[axis] = (c[axis] + a[axis]) * .5f;
    }
    count = XGL_SubdivideTriangle(surface, a, ab, ca, depth - 1, step, output, data);
    n = XGL_SubdivideTriangle(surface, ab, b, bc, depth - 1, step,
                              output ? output + count : NULL, data ? data + count : NULL);
    count += n;
    n = XGL_SubdivideTriangle(surface, ca, bc, c, depth - 1, step,
                              output ? output + count : NULL, data ? data + count : NULL);
    count += n;
    n = XGL_SubdivideTriangle(surface, ab, bc, ca, depth - 1, step,
                              output ? output + count : NULL, data ? data + count : NULL);
    count += n;
    return count;
}

/* Equal positions on one surface have equal UVs. Sort only an index list,
 * retaining the original triangle order and choosing the earliest corner
 * as the sample source so its color is ready before any duplicate uses it. */
static const xgl_native_vertex_t *xgl_sample_vertices;

static int XGL_SampleVertexCompare(const void *left, const void *right)
{
    int a, b, order;
    a = *(const unsigned short *)left;
    b = *(const unsigned short *)right;
    order = memcmp(xgl_sample_vertices[a].position,
                   xgl_sample_vertices[b].position, sizeof(vec3_t));
    return order ? order : a - b;
}

static void XGL_BuildSampleMap(xgl_surface_mesh_t *mesh)
{
    unsigned short *order;
    size_t bytes;
    int i, first, index;
    if (mesh->count <= 3) return;
    bytes = (size_t)mesh->count * sizeof(*order);
    /* Include temporary sorting storage in the same fixed cache budget. */
    if (bytes * 2 > XGL_WORLD_CACHE_BYTES - xgl_cache_bytes) return;
    mesh->sample_vertex = (unsigned short *)malloc(bytes);
    order = (unsigned short *)malloc(bytes);
    if (!mesh->sample_vertex || !order) {
        free(mesh->sample_vertex); free(order);
        mesh->sample_vertex = NULL;
        return;
    }
    xgl_cache_bytes += bytes * 2;
    for (i = 0; i < mesh->count; ++i) order[i] = (unsigned short)i;
    xgl_sample_vertices = mesh->vertices;
    qsort(order, mesh->count, sizeof(*order), XGL_SampleVertexCompare);
    xgl_sample_vertices = NULL;
    first = order[0];
    for (i = 0; i < mesh->count; ++i) {
        index = order[i];
        if (memcmp(mesh->vertices[index].position, mesh->vertices[first].position,
                   sizeof(vec3_t))) first = index;
        mesh->sample_vertex[index] = (unsigned short)first;
    }
    free(order);
    xgl_cache_bytes -= bytes;
}

static int XGL_BuildSurfaceMeshData (model_t *model, msurface_t *surface,
                                xgl_surface_mesh_t *mesh, int depth)
{
    vec3_t a, b, c;
    int i, count;
    size_t bytes;
    float step;
    if (surface->numedges < 3) return 0;
    step = xgl_subdivide.value;
    if (step < 8) step = 8;
    if (step > 256) step = 256;
    XGL_SurfacePoint(model, surface, 0, a);
    count = 0;
    for (i = 1; i + 1 < surface->numedges; ++i) {
        if ((i & 15) == 1) XGL_AudioUpdate();
        XGL_SurfacePoint(model, surface, i, b);
        XGL_SurfacePoint(model, surface, i + 1, c);
        count += XGL_SubdivideTriangle(surface, a, b, c, depth, step, NULL, NULL);
        if (count > XGL_SURFACE_DETAIL_VERTS) return 0;
    }
    bytes = (size_t)count * (sizeof(*mesh->vertices) + sizeof(*mesh->data));
    if (bytes > XGL_WORLD_CACHE_BYTES - xgl_cache_bytes) return 0;
    mesh->vertices = (xgl_native_vertex_t *)malloc(bytes);
    if (!mesh->vertices) return 0;
    xgl_cache_bytes += bytes;
    mesh->data = (xgl_cached_vertex_t *)(mesh->vertices + count);
    count = 0;
    for (i = 1; i + 1 < surface->numedges; ++i) {
        if ((i & 15) == 1) XGL_AudioUpdate();
        XGL_SurfacePoint(model, surface, i, b);
        XGL_SurfacePoint(model, surface, i + 1, c);
        count += XGL_SubdivideTriangle(surface, a, b, c, depth, step,
                                       mesh->vertices + count, mesh->data + count);
    }
    mesh->count = count;
    if (surface->flags & (SURF_DRAWSKY | SURF_DRAWTURB)) XGL_BuildSampleMap(mesh);
    mesh->texture = -1;
    mesh->fullbright = -1;
    mesh->queued_frame = -1;
    for (i = 0; i < MAXLIGHTMAPS; ++i) mesh->style[i] = -1;
    return 1;
}

static int XGL_BuildSurfaceMesh (model_t *model, msurface_t *surface,
                                xgl_surface_mesh_t *mesh, int depth)
{
    double start;
    int built;
    start = XGL_ProfileDetailNow();
    built = XGL_BuildSurfaceMeshData(model, surface, mesh, depth);
    XGL_ProfileAdd(XGL_PROFILE_WORLD_BUILD, start);
    if (built) {
        XGL_ProfileCount(XGL_PROFILE_WORLD_MESH_BUILDS, 1);
        XGL_ProfileCount(XGL_PROFILE_WORLD_BUILD_VERTICES, (unsigned long)mesh->count);
    } else XGL_ProfileCount(XGL_PROFILE_WORLD_MESH_FAILURES, 1);
    return built;
}

static int XGL_RepeatCoordinate (float coordinate, int size)
{
    int index;
    coordinate -= (float)(int)coordinate;
    if (coordinate < 0) coordinate += 1;
    index = (int)(coordinate * size);
    return index < size ? index : size - 1;
}

/* GLQuake draws the background and cutout foreground as separate passes.
 * Sampling the indexed source lets us compose the two scrolling layers into
 * one RGB sample before the single XGL submission. Index zero is transparent
 * only in the left/foreground half; it is an ordinary background color. */
static void XGL_SampleSky (texture_t *texture, const float *position,
                          float slow, float fast, float rgb[3])
{
    vec3_t direction;
    float scale, s, t;
    int half, x, y, index;
    const byte *pixels;
    VectorSubtract(position, r_origin, direction);
    direction[2] *= 3;
    scale = DotProduct(direction, direction);
    scale = scale > .0001f ? 378.0f / (float)sqrt(scale) : 0;
    s = direction[0] * scale;
    t = direction[1] * scale;
    half = texture->width / 2;
    if (!half || texture->offsets[0] < sizeof(texture_t)) {
        /* A malformed/placeholder sky has no addressable split image. */
        rgb[0] = rgb[1] = rgb[2] = .5f;
        XGL_SampleTexture(texture->gl_texturenum, (s + slow) / 128,
                          (t + slow) / 128, rgb, NULL);
        XGL_PrepareColor(rgb);
        return;
    }
    pixels = (const byte *)texture + texture->offsets[0];
    x = XGL_RepeatCoordinate((s + fast) / 128, half);
    y = XGL_RepeatCoordinate((t + fast) / 128, texture->height);
    index = pixels[y * texture->width + x];
    if (!index) {
        x = XGL_RepeatCoordinate((s + slow) / 128, half);
        y = XGL_RepeatCoordinate((t + slow) / 128, texture->height);
        index = pixels[y * texture->width + half + x];
    }
    XGL_PreparePaletteColor(index, rgb);
}

static void XGL_UpdateSpecialSurface (xgl_surface_mesh_t *mesh, msurface_t *surface,
                                      texture_t *texture, entity_t *entity)
{
    xgl_cached_vertex_t *vertex;
    xgl_native_vertex_t *draw;
    vec3_t forward, right, up, position;
    float os, ot, s, t, phase, slow, fast;
    int i, index, transformed, sky, samples;
    sky = (surface->flags & SURF_DRAWSKY) != 0;
    transformed = sky && entity && entity != &r_worldentity;
    if (transformed) XGL_BrushBasis(entity, forward, right, up);
    /* Bound time before integer lookup conversion, including long sessions. */
    phase = slow = fast = 0;
    if (sky) {
        slow = (float)fmod(realtime * 8, 128);
        fast = (float)fmod(realtime * 16, 128);
    } else phase = (float)fmod(realtime, 2 * M_PI);
    samples = 0;
    for (i = 0; i < mesh->count; ++i) {
        draw = mesh->vertices + i;
        if (mesh->sample_vertex && mesh->sample_vertex[i] != i) {
            VectorCopy(mesh->vertices[mesh->sample_vertex[i]].color.rgb, draw->color.rgb);
            continue;
        }
        ++samples;
        vertex = mesh->data + i;
        if (sky) {
            if (transformed)
                XGL_BrushPoint(draw->position, entity, forward, right, up, position);
            else VectorCopy(draw->position, position);
            XGL_SampleSky(texture, position, slow, fast, draw->color.rgb);
        } else {
            /* Original turbulent UVs omit texture-vector translation and
             * repeat every 64 world texture units, just like EmitWaterPolys. */
            os = vertex->st[0] - surface->texinfo->vecs[0][3];
            ot = vertex->st[1] - surface->texinfo->vecs[1][3];
            s = (os + xgl_turb_sin[(int)((ot * .125f + phase) * XGL_TURB_SCALE) & 255]) / 64;
            t = (ot + xgl_turb_sin[(int)((os * .125f + phase) * XGL_TURB_SCALE) & 255]) / 64;
            if (XGL_SampleTextureIndex(texture->gl_texturenum, s, t, &index))
                XGL_PreparePaletteColor(index, draw->color.rgb);
            else {
                draw->color.rgb[0] = draw->color.rgb[1] = draw->color.rgb[2] = .5f;
                XGL_PrepareColor(draw->color.rgb);
            }
        }
    }
    XGL_ProfileCount(XGL_PROFILE_WORLD_ANIMATED_SAMPLES, (unsigned long)samples);
}

/* A mesh can be reused by more than one brush entity in the same frame.
 * Finish any earlier borrowed submission before changing its colors. The
 * previous frame has already completed, so normal first draws need no flush. */
static void XGL_SurfaceWillChange(xgl_surface_mesh_t *mesh)
{
    if (mesh->queued_frame == r_framecount) XGL_Flush();
    mesh->queued_frame = -1;
}

static void XGL_UpdateSurfaceMesh (xgl_surface_mesh_t *mesh, msurface_t *surface,
                                   texture_t *texture, entity_t *entity)
{
    int i, j, changed, fullbright, texture_changed;
    float rgb[3], light, footprint[2], low, high, value;
    unsigned int texture_revision, color_revision;
    int axis, corner;
    xgl_cached_vertex_t *vertex, *triangle;
    xgl_native_vertex_t *draw;
    double start;
    if (surface->flags & (SURF_DRAWSKY | SURF_DRAWTURB)) {
        XGL_SurfaceWillChange(mesh);
        start = XGL_ProfileDetailNow();
        XGL_UpdateSpecialSurface(mesh, surface, texture, entity);
        XGL_ProfileAdd(XGL_PROFILE_WORLD_ANIMATED, start);
        XGL_ProfileAdd((surface->flags & SURF_DRAWSKY) ?
                       XGL_PROFILE_WORLD_SKY : XGL_PROFILE_WORLD_WATER, start);
        XGL_ProfileCount(XGL_PROFILE_WORLD_ANIMATED_VERTICES, (unsigned long)mesh->count);
        return;
    }
    start = XGL_ProfileDetailNow();
    fullbright = r_fullbright.value != 0;
    texture_revision = XGL_TextureRevision(texture->gl_texturenum);
    color_revision = XGL_ColorRevision();
    texture_changed = mesh->texture != texture->gl_texturenum || mesh->texture_revision != texture_revision;
    changed = texture_changed || mesh->fullbright != fullbright || mesh->color_revision != color_revision;
    for (i = 0; i < MAXLIGHTMAPS; ++i) {
        int value;
        value = surface->styles[i] == 255 ? 0 : d_lightstylevalue[surface->styles[i]];
        if (mesh->style[i] != value) changed = 1;
        mesh->style[i] = value;
    }
    if (!changed) {
        XGL_ProfileAdd(XGL_PROFILE_WORLD_COLORS, start);
        XGL_ProfileCount(XGL_PROFILE_WORLD_COLOR_HITS, 1);
        return;
    }
    XGL_SurfaceWillChange(mesh);
    footprint[0] = footprint[1] = 0;
    for (i = 0, vertex = mesh->data, draw = mesh->vertices; i < mesh->count; ++i, ++vertex, ++draw) {
        if ((i & 255) == 0) XGL_AudioUpdate();
        if (texture_changed) {
            if (i % 3 == 0) {
                triangle = vertex;
                for (axis = 0; axis < 2; ++axis) {
                    low = high = triangle[0].st[axis];
                    for (corner = 1; corner < 3; ++corner) {
                        value = triangle[corner].st[axis];
                        if (value < low) low = value;
                        if (value > high) high = value;
                    }
                    footprint[axis] = (high - low) / (3 * (float)(axis ? texture->height : texture->width));
                }
            }
            rgb[0] = rgb[1] = rgb[2] = .5f;
            XGL_SampleTextureFiltered(texture->gl_texturenum, vertex->st[0] / texture->width,
                              vertex->st[1] / texture->height, footprint[0], footprint[1], rgb, NULL);
            for (j = 0; j < 3; ++j) vertex->material[j] = (byte)(rgb[j] * 255 + .5f);
        }
        light = XGL_SurfaceLightScale(surface, vertex->samples);
        for (j = 0; j < 3; ++j) draw->color.rgb[j] = vertex->material[j] * (light / 255.0f);
        XGL_PrepareColor(draw->color.rgb);
    }
    mesh->texture_revision = texture_revision;
    mesh->color_revision = color_revision;
    mesh->texture = texture->gl_texturenum;
    mesh->fullbright = fullbright;
    XGL_ProfileAdd(XGL_PROFILE_WORLD_COLORS, start);
    XGL_ProfileCount(XGL_PROFILE_WORLD_COLOR_UPDATES, 1);
    XGL_ProfileCount(XGL_PROFILE_WORLD_COLOR_VERTICES, (unsigned long)mesh->count);
}

static void XGL_EmitSurfaceVertices (xgl_surface_mesh_t *mesh,
                                    entity_t *entity, int lit, float opacity,
                                    int persistent)
{
    const xgl_native_vertex_t *source;
    xgl_native_vertex_t *vertices;
    vec3_t forward, right, up;
    int i, j, n, transformed, count;
    source = mesh->vertices; count = mesh->count;
    transformed = entity && entity != &r_worldentity;
    if (!transformed && persistent) {
        XGL_DrawNativeTriangles(source, count, lit, opacity, opacity >= 1);
        mesh->queued_frame = r_framecount;
        c_brush_polys += count / 3;
        return;
    }
    if (transformed) XGL_BrushBasis(entity, forward, right, up);
    while (count > 0) {
        n = count > XGL_WORLD_BATCH ? XGL_WORLD_BATCH : count;
        vertices = XGL_ReserveTriangles(&n, lit, opacity, opacity >= 1);
        if (!vertices) return;
        for (i = 0; i < n; ++i) {
            vertices[i] = source[i];
            if (transformed) {
                XGL_BrushPoint(source[i].position, entity, forward, right, up, vertices[i].position);
                for (j = 0; j < 3; ++j)
                    vertices[i].normal[j] = source[i].normal[0] * forward[j] -
                        source[i].normal[1] * right[j] + source[i].normal[2] * up[j];
            }
        }
        c_brush_polys += n / 3;
        source += n; count -= n;
    }
}

static void XGL_SurfaceBounds(model_t *model, msurface_t *surface, xgl_surface_cache_t *cache)
{
    vec3_t point, origin, previous, a, b, cross;
    float radius_squared, length_squared;
    int i;
    if (cache->center_valid) return;
    VectorCopy(vec3_origin, cache->center);
    cache->area = cache->radius = 0;
    for (i = 0; i < surface->numedges; ++i) {
        XGL_SurfacePoint(model, surface, i, point);
        VectorAdd(cache->center, point, cache->center);
    }
    if (surface->numedges) { VectorScale(cache->center, 1.0f / surface->numedges, cache->center); }
    radius_squared = 0;
    VectorCopy(vec3_origin, origin); VectorCopy(vec3_origin, previous);
    for (i = 0; i < surface->numedges; ++i) {
        XGL_SurfacePoint(model, surface, i, point);
        VectorSubtract(point, cache->center, a);
        length_squared = DotProduct(a, a);
        if (length_squared > radius_squared) radius_squared = length_squared;
        if (i == 0) { VectorCopy(point, origin); }
        else if (i > 1) {
            VectorSubtract(previous, origin, a); VectorSubtract(point, origin, b);
            CrossProduct(a, b, cross);
            cache->area += (float)fabs(DotProduct(cross, surface->plane->normal)) * .5f;
        }
        VectorCopy(point, previous);
    }
    cache->radius = (float)sqrt(radius_squared);
    cache->center_valid = 1;
}

static int XGL_DetailCompare(const void *left, const void *right)
{
    int a, b;
    float difference;
    a = *(const int *)left; b = *(const int *)right;
    difference = xgl_surfaces[b].priority - xgl_surfaces[a].priority;
    if (difference > 0) return 1;
    if (difference < 0) return -1;
    return a < b ? -1 : (a > b ? 1 : 0);
}

/* Reserve the fixed detail allowance for the largest visible projected-size
 * estimates, then submit in the original BSP order. This avoids spending the
 * whole allowance on small faces encountered early in the traversal. */
static void XGL_SelectWorldDetail(void)
{
    xgl_surface_cache_t *cache;
    msurface_t *surface;
    vec3_t delta;
    float distance, range;
    int i, index, candidates;
    candidates = 0;
    for (i = 0; i < xgl_world_count; ++i) {
        if ((i & 15) == 0) XGL_AudioUpdate();
        index = xgl_world_order[i];
        cache = xgl_surfaces + index;
        surface = cl.worldmodel->surfaces + index;
        cache->detail_frame = -1;
        if ((surface->flags & SURF_DRAWTURB) && r_wateralpha.value <= 0) continue;
        /* Coarse meshes get memory first, preserving cheap complete fallback. */
        if (!cache->coarse_attempted) {
            cache->coarse_attempted = 1;
            XGL_BuildSurfaceMesh(cl.worldmodel, surface, &cache->coarse, 0);
        }
        if (!(xgl_detailtris.value > 0 && xgl_detaildistance.value > 0)) continue;
        XGL_SurfaceBounds(cl.worldmodel, surface, cache);
        VectorSubtract(cache->center, r_origin, delta);
        distance = DotProduct(delta, delta);
        range = xgl_detaildistance.value + cache->radius;
        if (distance >= range * range) continue;
        cache->priority = cache->area / (distance + 64);
        xgl_detail_order[candidates++] = index;
    }
    qsort(xgl_detail_order, candidates, sizeof(*xgl_detail_order), XGL_DetailCompare);
    for (i = 0; i < candidates && xgl_detail_used < xgl_detailtris.value; ++i) {
        if ((i & 15) == 0) XGL_AudioUpdate();
        index = xgl_detail_order[i]; cache = xgl_surfaces + index;
        if (!cache->detail_attempted) {
            cache->detail_attempted = 1;
            XGL_BuildSurfaceMesh(cl.worldmodel, cl.worldmodel->surfaces + index, &cache->detail, 2);
        }
        if (cache->detail.vertices && cache->detail.count > cache->coarse.count &&
            xgl_detail_used + cache->detail.count / 3 <= xgl_detailtris.value) {
            cache->detail_frame = r_framecount;
            xgl_detail_used += cache->detail.count / 3;
        }
    }
}

static void XGL_DrawSurface (model_t *model, msurface_t *surface, entity_t *entity, float opacity)
{
    xgl_surface_cache_t *cache;
    xgl_surface_mesh_t *mesh;
    xgl_surface_mesh_t fallback;
    xgl_native_vertex_t triangle[3];
    xgl_cached_vertex_t triangle_data[3];
    texture_t *texture;
    vec3_t a, b, c, center, forward, right, up, worldcenter, delta;
    float distance;
    int index, i, axis, lit, detail;
    texture = XGL_SurfaceTexture(surface->texinfo->texture, entity);
    if (!texture || !texture->width || !texture->height) return;
    lit = !r_fullbright.value && !(surface->flags & (SURF_DRAWSKY | SURF_DRAWTURB));
    cache = NULL; mesh = NULL;
    if (xgl_surfaces && model->surfaces == xgl_cached_world->surfaces) {
        index = (int)(surface - xgl_cached_world->surfaces);
        if (index >= 0 && index < xgl_surface_count) cache = &xgl_surfaces[index];
    }
    if (cache) {
        if (!cache->coarse_attempted) {
            cache->coarse_attempted = 1;
            XGL_BuildSurfaceMesh(model, surface, &cache->coarse, 0);
        }
        mesh = &cache->coarse;
        if (entity == &r_worldentity && xgl_world_order) {
            if (cache->detail_frame == r_framecount && cache->detail.vertices)
                mesh = &cache->detail;
        } else {
            XGL_SurfaceBounds(model, surface, cache);
            VectorCopy(cache->center, center);
            if (entity && entity != &r_worldentity) {
                XGL_BrushBasis(entity, forward, right, up);
                XGL_BrushPoint(center, entity, forward, right, up, worldcenter);
            } else VectorCopy(center, worldcenter);
            VectorSubtract(worldcenter, r_origin, delta);
            distance = DotProduct(delta, delta);
            detail = xgl_detaildistance.value > 0 && distance < xgl_detaildistance.value * xgl_detaildistance.value;
            if (detail && xgl_detail_used < xgl_detailtris.value && !cache->detail_attempted) {
                cache->detail_attempted = 1;
                XGL_BuildSurfaceMesh(model, surface, &cache->detail, 2);
            }
            if (detail && cache->detail.vertices &&
                xgl_detail_used + cache->detail.count / 3 <= xgl_detailtris.value) {
                mesh = &cache->detail;
                xgl_detail_used += mesh->count / 3;
            }
        }
        if (mesh->vertices) {
            XGL_ProfileCount(XGL_PROFILE_WORLD_CACHED_SURFACES, 1);
            XGL_UpdateSurfaceMesh(mesh, surface, texture, entity);
            XGL_EmitSurfaceVertices(mesh, entity, lit, opacity, 1);
            return;
        }
    }
    /* Allocation/budget exhaustion never discards a face. Stream its whole
     * coarse fan without persistent allocation, even for external bmodels. */
    if (surface->numedges < 3) return;
    XGL_ProfileCount(XGL_PROFILE_WORLD_FALLBACK_SURFACES, 1);
    XGL_SurfacePoint(model, surface, 0, a);
    memset(&fallback, 0, sizeof(fallback));
    fallback.vertices = triangle; fallback.data = triangle_data; fallback.count = 3;
    fallback.queued_frame = -1;
    for (i = 1; i + 1 < surface->numedges; ++i) {
        if ((i & 15) == 1) XGL_AudioUpdate();
        XGL_SurfacePoint(model, surface, i, b);
        XGL_SurfacePoint(model, surface, i + 1, c);
        XGL_InitCachedVertex(triangle, triangle_data, surface, a);
        XGL_InitCachedVertex(triangle + 1, triangle_data + 1, surface, b);
        XGL_InitCachedVertex(triangle + 2, triangle_data + 2, surface, c);
        fallback.texture = -1; fallback.fullbright = -1;
        for (axis = 0; axis < MAXLIGHTMAPS; ++axis) fallback.style[axis] = -1;
        XGL_UpdateSurfaceMesh(&fallback, surface, texture, entity);
        XGL_EmitSurfaceVertices(&fallback, entity, lit, opacity, 0);
    }
}

static void XGL_QueueSurface (model_t *model, msurface_t *surface, entity_t *entity)
{
    vec3_t point, delta, forward, right, up, transformed;
    float opacity;
    opacity = r_wateralpha.value;
    if (opacity < 0) opacity = 0;
    if (opacity > 1) opacity = 1;
    if (!(surface->flags & SURF_DRAWTURB) || opacity >= 1) {
        XGL_DrawSurface(model, surface, entity, 1);
        return;
    }
    if (opacity <= 0) return;
    if (xgl_water_count == XGL_WATER_QUEUE) {
        /* Preserve complete visibility even if an extreme map exhausts the
         * bounded sort queue; this overflow alone is drawn opaque. */
        XGL_DrawSurface(model, surface, entity, 1);
        return;
    }
    XGL_SurfacePoint(model, surface, 0, point);
    if (entity != &r_worldentity) {
        XGL_BrushBasis(entity, forward, right, up);
        XGL_BrushPoint(point, entity, forward, right, up, transformed);
    } else VectorCopy(point, transformed);
    VectorSubtract(transformed, r_origin, delta);
    xgl_water[xgl_water_count].model = model;
    xgl_water[xgl_water_count].surface = surface;
    xgl_water[xgl_water_count].entity = entity;
    xgl_water[xgl_water_count].distance = DotProduct(delta, vpn);
    ++xgl_water_count;
}

static int XGL_WaterCompare (const void *a, const void *b)
{
    float difference;
    difference = ((const xgl_water_surface_t *)b)->distance - ((const xgl_water_surface_t *)a)->distance;
    return difference > 0 ? 1 : (difference < 0 ? -1 : 0);
}

void XGL_DrawWater (void)
{
    int i;
    float opacity;
    opacity = r_wateralpha.value;
    if (opacity < 0) opacity = 0;
    if (opacity > 1) opacity = 1;
    qsort(xgl_water, xgl_water_count, sizeof(xgl_water[0]), XGL_WaterCompare);
    for (i = 0; i < xgl_water_count; ++i)
        XGL_DrawSurface(xgl_water[i].model, xgl_water[i].surface, xgl_water[i].entity, opacity);
    xgl_water_count = 0;
}

static void XGL_MarkLeaves (void)
{
    byte *vis;
    mnode_t *node;
    int i;
    /* Retain the PVS inside a leaf, but invalidate it on r_novis changes and
     * map/cache transitions. Surface visibility is still marked each frame. */
    if (xgl_pvs_world == cl.worldmodel && xgl_pvs_leaf == r_viewleaf &&
        xgl_pvs_novis == (r_novis.value != 0)) return;
    xgl_pvs_world = cl.worldmodel;
    xgl_pvs_leaf = r_viewleaf;
    xgl_pvs_novis = r_novis.value != 0;
    ++r_visframecount;
    vis = r_novis.value ? NULL : Mod_LeafPVS(r_viewleaf, cl.worldmodel);
    for (i = 0; i < cl.worldmodel->numleafs; ++i) {
        if (vis && !(vis[i >> 3] & (1 << (i & 7)))) continue;
        node = (mnode_t *)&cl.worldmodel->leafs[i + 1];
        while (node && node->visframe != r_visframecount) {
            node->visframe = r_visframecount;
            node = node->parent;
        }
    }
}

static void XGL_WorldNode (mnode_t *node)
{
    mleaf_t *leaf;
    msurface_t *surface;
    float distance;
    int i, side;
    if (node->contents == CONTENTS_SOLID || node->visframe != r_visframecount) return;
    if (R_CullBox(node->minmaxs, node->minmaxs + 3)) return;
    if (node->contents < 0) {
        leaf = (mleaf_t *)node;
        for (i = 0; i < leaf->nummarksurfaces; ++i) leaf->firstmarksurface[i]->visframe = r_framecount;
        if (leaf->efrags) R_StoreEfrags(&leaf->efrags);
        return;
    }
    distance = DotProduct(r_origin, node->plane->normal) - node->plane->dist;
    side = distance < 0;
    XGL_WorldNode(node->children[side]);
    surface = cl.worldmodel->surfaces + node->firstsurface;
    for (i = 0; i < node->numsurfaces; ++i, ++surface) {
        if (surface->visframe != r_framecount) continue;
        if (!(surface->flags & SURF_UNDERWATER) &&
            ((distance < 0) != ((surface->flags & SURF_PLANEBACK) != 0))) continue;
        if (xgl_world_order)
            xgl_world_order[xgl_world_count++] = (int)(surface - cl.worldmodel->surfaces);
        else XGL_QueueSurface(cl.worldmodel, surface, &r_worldentity);
    }
    XGL_WorldNode(node->children[!side]);
}

void XGL_DrawWorld (void)
{
    int i;
    double start;
    if (xgl_cache_step != xgl_subdivide.value) XGL_WorldNewMap();
    xgl_detail_used = 0;
    xgl_water_count = 0;
    xgl_world_count = 0;
    currententity = &r_worldentity;
    start = XGL_ProfileDetailNow();
    XGL_MarkLeaves();
    XGL_WorldNode(cl.worldmodel->nodes);
    /* Without the cache index, traversal also emits coarse fallback faces.
     * The fallback counter identifies that case in a detailed report. */
    XGL_ProfileAdd(XGL_PROFILE_WORLD_VISIBILITY, start);
    if (xgl_world_order) {
        start = XGL_ProfileDetailNow();
        XGL_SelectWorldDetail();
        XGL_ProfileAdd(XGL_PROFILE_WORLD_DETAIL_SELECT, start);
        for (i = 0; i < xgl_world_count; ++i)
            XGL_QueueSurface(cl.worldmodel, cl.worldmodel->surfaces + xgl_world_order[i], &r_worldentity);
    }
}

void XGL_DrawBrushModel (entity_t *entity)
{
    model_t *model;
    msurface_t *surface;
    vec3_t mins, maxs, delta, local, forward, right, up;
    float distance;
    int i, rotated;
    model = entity->model;
    currententity = entity;
    rotated = entity->angles[0] || entity->angles[1] || entity->angles[2];
    for (i = 0; i < 3; ++i) {
        mins[i] = entity->origin[i] + (rotated ? -model->radius : model->mins[i]);
        maxs[i] = entity->origin[i] + (rotated ? model->radius : model->maxs[i]);
    }
    if (R_CullBox(mins, maxs)) return;
    VectorSubtract(r_origin, entity->origin, delta);
    XGL_BrushBasis(entity, forward, right, up);
    local[0] = DotProduct(delta, forward);
    local[1] = -DotProduct(delta, right);
    local[2] = DotProduct(delta, up);
    surface = model->surfaces + model->firstmodelsurface;
    for (i = 0; i < model->nummodelsurfaces; ++i, ++surface) {
        distance = DotProduct(local, surface->plane->normal) - surface->plane->dist;
        if ((surface->flags & SURF_PLANEBACK) ? distance < -.01f : distance > .01f)
            XGL_QueueSurface(model, surface, entity);
    }
}
