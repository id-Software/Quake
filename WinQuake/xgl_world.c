/*
 * Bounded sampled-color BSP renderer for Sun XGL.
 * Derived from Quake, Copyright (C) 1996-1997 Id Software, Inc.
 * GNU GPL version 2 or (at your option) any later version.
 */
#include "quakedef.h"

#define XGL_WORLD_CACHE_BYTES (8UL * 1024UL * 1024UL)
#define XGL_SURFACE_DETAIL_VERTS 12288
#define XGL_WATER_QUEUE 4096
#define XGL_WORLD_BATCH 384
#define XGL_TURB_SCALE (256.0 / (2 * M_PI))

static const float xgl_turb_sin[256] = {
#include "gl_warp_sin.h"
};

typedef struct {
    xgl_vertex_t draw;
    float st[2];
    byte material[3];
    byte samples[MAXLIGHTMAPS];
} xgl_cached_vertex_t;

typedef struct {
    xgl_cached_vertex_t *vertices;
    int count, texture, style[MAXLIGHTMAPS], fullbright;
} xgl_surface_mesh_t;

typedef struct {
    xgl_surface_mesh_t coarse, detail;
    int coarse_attempted, detail_attempted;
    vec3_t center;
    int center_valid;
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
static model_t *xgl_pvs_world;
static mleaf_t *xgl_pvs_leaf;
static int xgl_pvs_novis;
static float xgl_cache_step;
static xgl_water_surface_t xgl_water[XGL_WATER_QUEUE];
static int xgl_water_count;

cvar_t xgl_subdivide = {"xgl_subdivide", "32", false, false, 0, NULL};
cvar_t xgl_detailtris = {"xgl_detailtris", "4096", false, false, 0, NULL};
cvar_t xgl_detaildistance = {"xgl_detaildistance", "768", false, false, 0, NULL};

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
        for (i = 0; i < xgl_surface_count; ++i) {
            free(xgl_surfaces[i].coarse.vertices);
            free(xgl_surfaces[i].detail.vertices);
        }
        free(xgl_surfaces);
    }
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
    bytes = (size_t)xgl_cached_world->numsurfaces * sizeof(*xgl_surfaces);
    if (bytes <= XGL_WORLD_CACHE_BYTES) {
        xgl_surfaces = (xgl_surface_cache_t *)calloc(1, bytes);
        if (xgl_surfaces) {
            xgl_surface_count = xgl_cached_world->numsurfaces;
            xgl_cache_bytes = bytes;
        }
    }
    Con_Printf("XGL: sampled BSP cache limited to 8 MiB; detail depth <= 2; complete coarse fallback\n");
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

static void XGL_InitCachedVertex (xgl_cached_vertex_t *vertex, msurface_t *surface, const float *point)
{
    int i;
    VectorCopy(point, vertex->draw.position);
    for (i = 0; i < 3; ++i)
        vertex->draw.normal[i] = surface->plane->normal[i] * ((surface->flags & SURF_PLANEBACK) ? -1 : 1);
    for (i = 0; i < 2; ++i)
        vertex->st[i] = DotProduct(point, surface->texinfo->vecs[i]) + surface->texinfo->vecs[i][3];
    XGL_SurfaceLightSamples(surface, point, vertex->samples);
}

/* The same bounded recursion counts and emits. Texture-space edge span,
 * rather than world area alone, keeps detail useful on oblique materials. */
static int XGL_SubdivideTriangle (msurface_t *surface, const float *a, const float *b,
                                 const float *c, int depth, float step,
                                 xgl_cached_vertex_t *output)
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
            XGL_InitCachedVertex(output, surface, a);
            XGL_InitCachedVertex(output + 1, surface, b);
            XGL_InitCachedVertex(output + 2, surface, c);
        }
        return 3;
    }
    for (axis = 0; axis < 3; ++axis) {
        ab[axis] = (a[axis] + b[axis]) * .5f;
        bc[axis] = (b[axis] + c[axis]) * .5f;
        ca[axis] = (c[axis] + a[axis]) * .5f;
    }
    count = XGL_SubdivideTriangle(surface, a, ab, ca, depth - 1, step, output);
    n = XGL_SubdivideTriangle(surface, ab, b, bc, depth - 1, step, output ? output + count : NULL); count += n;
    n = XGL_SubdivideTriangle(surface, ca, bc, c, depth - 1, step, output ? output + count : NULL); count += n;
    n = XGL_SubdivideTriangle(surface, ab, bc, ca, depth - 1, step, output ? output + count : NULL); count += n;
    return count;
}

static int XGL_BuildSurfaceMesh (model_t *model, msurface_t *surface,
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
        XGL_SurfacePoint(model, surface, i, b);
        XGL_SurfacePoint(model, surface, i + 1, c);
        count += XGL_SubdivideTriangle(surface, a, b, c, depth, step, NULL);
        if (count > XGL_SURFACE_DETAIL_VERTS) return 0;
    }
    bytes = (size_t)count * sizeof(*mesh->vertices);
    if (bytes > XGL_WORLD_CACHE_BYTES - xgl_cache_bytes) return 0;
    mesh->vertices = (xgl_cached_vertex_t *)malloc(bytes);
    if (!mesh->vertices) return 0;
    xgl_cache_bytes += bytes;
    count = 0;
    for (i = 1; i + 1 < surface->numedges; ++i) {
        XGL_SurfacePoint(model, surface, i, b);
        XGL_SurfacePoint(model, surface, i + 1, c);
        count += XGL_SubdivideTriangle(surface, a, b, c, depth, step, mesh->vertices + count);
    }
    mesh->count = count;
    mesh->texture = -1;
    mesh->fullbright = -1;
    for (i = 0; i < MAXLIGHTMAPS; ++i) mesh->style[i] = -1;
    return 1;
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
    rgb[0] = xgl_palette[index * 3] / 255.0f;
    rgb[1] = xgl_palette[index * 3 + 1] / 255.0f;
    rgb[2] = xgl_palette[index * 3 + 2] / 255.0f;
}

static void XGL_UpdateSpecialSurface (xgl_surface_mesh_t *mesh, msurface_t *surface,
                                      texture_t *texture, entity_t *entity)
{
    xgl_cached_vertex_t *vertex;
    vec3_t forward, right, up, position;
    float os, ot, s, t, phase, slow, fast;
    int i, transformed, sky;
    sky = (surface->flags & SURF_DRAWSKY) != 0;
    transformed = sky && entity && entity != &r_worldentity;
    if (transformed) XGL_BrushBasis(entity, forward, right, up);
    /* Bound time before integer lookup conversion, including long sessions. */
    phase = slow = fast = 0;
    if (sky) {
        slow = (float)fmod(realtime * 8, 128);
        fast = (float)fmod(realtime * 16, 128);
    } else phase = (float)fmod(realtime, 2 * M_PI);
    for (i = 0, vertex = mesh->vertices; i < mesh->count; ++i, ++vertex) {
        if (sky) {
            if (transformed)
                XGL_BrushPoint(vertex->draw.position, entity, forward, right, up, position);
            else VectorCopy(vertex->draw.position, position);
            XGL_SampleSky(texture, position, slow, fast, vertex->draw.color);
        } else {
            /* Original turbulent UVs omit texture-vector translation and
             * repeat every 64 world texture units, just like EmitWaterPolys. */
            os = vertex->st[0] - surface->texinfo->vecs[0][3];
            ot = vertex->st[1] - surface->texinfo->vecs[1][3];
            s = (os + xgl_turb_sin[(int)((ot * .125f + phase) * XGL_TURB_SCALE) & 255]) / 64;
            t = (ot + xgl_turb_sin[(int)((os * .125f + phase) * XGL_TURB_SCALE) & 255]) / 64;
            vertex->draw.color[0] = vertex->draw.color[1] = vertex->draw.color[2] = .5f;
            XGL_SampleTexture(texture->gl_texturenum, s, t, vertex->draw.color, NULL);
        }
    }
}

static void XGL_UpdateSurfaceMesh (xgl_surface_mesh_t *mesh, msurface_t *surface,
                                   texture_t *texture, entity_t *entity)
{
    int i, j, changed, fullbright, texture_changed;
    float rgb[3], light;
    xgl_cached_vertex_t *vertex;
    if (surface->flags & (SURF_DRAWSKY | SURF_DRAWTURB)) {
        XGL_UpdateSpecialSurface(mesh, surface, texture, entity);
        return;
    }
    fullbright = r_fullbright.value != 0;
    texture_changed = mesh->texture != texture->gl_texturenum;
    changed = texture_changed || mesh->fullbright != fullbright;
    for (i = 0; i < MAXLIGHTMAPS; ++i) {
        int value;
        value = surface->styles[i] == 255 ? 0 : d_lightstylevalue[surface->styles[i]];
        if (mesh->style[i] != value) changed = 1;
        mesh->style[i] = value;
    }
    if (!changed) return;
    for (i = 0, vertex = mesh->vertices; i < mesh->count; ++i, ++vertex) {
        if (texture_changed) {
            XGL_SampleTexture(texture->gl_texturenum, vertex->st[0] / texture->width,
                              vertex->st[1] / texture->height, rgb, NULL);
            for (j = 0; j < 3; ++j) vertex->material[j] = (byte)(rgb[j] * 255 + .5f);
        }
        light = XGL_SurfaceLightScale(surface, vertex->samples);
        for (j = 0; j < 3; ++j) vertex->draw.color[j] = vertex->material[j] * (light / 255.0f);
    }
    mesh->texture = texture->gl_texturenum;
    mesh->fullbright = fullbright;
}

static void XGL_EmitSurfaceVertices (const xgl_cached_vertex_t *source, int count,
                                    entity_t *entity, int lit, float opacity)
{
    xgl_vertex_t vertices[XGL_WORLD_BATCH];
    vec3_t forward, right, up;
    int i, j, n, transformed;
    transformed = entity && entity != &r_worldentity;
    if (transformed) XGL_BrushBasis(entity, forward, right, up);
    while (count > 0) {
        n = count > XGL_WORLD_BATCH ? XGL_WORLD_BATCH : count;
        for (i = 0; i < n; ++i) {
            vertices[i] = source[i].draw;
            if (transformed) {
                XGL_BrushPoint(source[i].draw.position, entity, forward, right, up, vertices[i].position);
                for (j = 0; j < 3; ++j)
                    vertices[i].normal[j] = source[i].draw.normal[0] * forward[j] -
                        source[i].draw.normal[1] * right[j] + source[i].draw.normal[2] * up[j];
            }
        }
        XGL_DrawTriangles(vertices, n, lit, opacity, opacity >= 1);
        c_brush_polys += n / 3;
        source += n; count -= n;
    }
}

static void XGL_DrawSurface (model_t *model, msurface_t *surface, entity_t *entity, float opacity)
{
    xgl_surface_cache_t *cache;
    xgl_surface_mesh_t *mesh;
    xgl_surface_mesh_t fallback;
    xgl_cached_vertex_t triangle[3];
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
        if (!cache->center_valid) {
            cache->center[0] = cache->center[1] = cache->center[2] = 0;
            for (i = 0; i < surface->numedges; ++i) {
                XGL_SurfacePoint(model, surface, i, a);
                VectorAdd(cache->center, a, cache->center);
            }
            if (surface->numedges) VectorScale(cache->center, 1.0f / surface->numedges, cache->center);
            cache->center_valid = 1;
        }
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
        if (mesh->vertices) {
            XGL_UpdateSurfaceMesh(mesh, surface, texture, entity);
            XGL_EmitSurfaceVertices(mesh->vertices, mesh->count, entity, lit, opacity);
            return;
        }
    }
    /* Allocation/budget exhaustion never discards a face. Stream its whole
     * coarse fan without persistent allocation, even for external bmodels. */
    if (surface->numedges < 3) return;
    XGL_SurfacePoint(model, surface, 0, a);
    memset(&fallback, 0, sizeof(fallback));
    fallback.vertices = triangle; fallback.count = 3;
    for (i = 1; i + 1 < surface->numedges; ++i) {
        XGL_SurfacePoint(model, surface, i, b);
        XGL_SurfacePoint(model, surface, i + 1, c);
        XGL_InitCachedVertex(triangle, surface, a);
        XGL_InitCachedVertex(triangle + 1, surface, b);
        XGL_InitCachedVertex(triangle + 2, surface, c);
        fallback.texture = -1; fallback.fullbright = -1;
        for (axis = 0; axis < MAXLIGHTMAPS; ++axis) fallback.style[axis] = -1;
        XGL_UpdateSurfaceMesh(&fallback, surface, texture, entity);
        XGL_EmitSurfaceVertices(triangle, 3, entity, lit, opacity);
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
        XGL_QueueSurface(cl.worldmodel, surface, &r_worldentity);
    }
    XGL_WorldNode(node->children[!side]);
}

void XGL_DrawWorld (void)
{
    if (xgl_cache_step != xgl_subdivide.value) XGL_WorldNewMap();
    xgl_detail_used = 0;
    xgl_water_count = 0;
    currententity = &r_worldentity;
    XGL_MarkLeaves();
    XGL_WorldNode(cl.worldmodel->nodes);
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
