/*
 * XGL renderer lighting. Derived from Quake's GPL renderer.
 * Copyright (C) 1996-1997 Id Software, Inc.
 * Distributed under GNU GPL version 2 or (at your option) any later version.
 */
#include "quakedef.h"

int d_lightstylevalue[256];

void R_AnimateLight (void)
{
    int tick, i, value;
    tick = (int)(cl.time * 10);
    for (i = 0; i < 256; ++i) {
        value = 256;
        if (i < MAX_LIGHTSTYLES && cl_lightstyle[i].length) {
            int index;
            index = tick % cl_lightstyle[i].length;
            if (index < 0) index += cl_lightstyle[i].length;
            value = (cl_lightstyle[i].map[index] - 'a') * 22;
            if (value < 0) value = 0;
        }
        d_lightstylevalue[i] = value;
    }
}

/* Samples are cached separately for every style. Changing a style never
 * resamples a texture or traces the BSP; native point lights handle moving
 * lights without recoloring the CPU mesh. */
void XGL_SurfaceLightSamples (msurface_t *surface, const float *position,
                              byte samples[MAXLIGHTMAPS])
{
    int width, height, x, y, nextx, nexty, layer, stride;
    float s, t, fx, fy, a, b, value;
    byte *map;
    memset(samples, 0, MAXLIGHTMAPS);
    if (!surface->samples ||
        (surface->flags & (SURF_DRAWTILED | SURF_DRAWSKY | SURF_DRAWTURB))) return;
    width = (surface->extents[0] >> 4) + 1;
    height = (surface->extents[1] >> 4) + 1;
    if (width <= 0 || height <= 0) return;
    s = (DotProduct(position, surface->texinfo->vecs[0]) +
         surface->texinfo->vecs[0][3] - surface->texturemins[0]) / 16.0f;
    t = (DotProduct(position, surface->texinfo->vecs[1]) +
         surface->texinfo->vecs[1][3] - surface->texturemins[1]) / 16.0f;
    if (s < 0) s = 0;
    if (t < 0) t = 0;
    if (s > width - 1) s = width - 1;
    if (t > height - 1) t = height - 1;
    x = (int)s; y = (int)t;
    nextx = x + 1 < width ? x + 1 : x;
    nexty = y + 1 < height ? y + 1 : y;
    fx = s - x; fy = t - y;
    stride = width * height;
    map = surface->samples;
    for (layer = 0; layer < MAXLIGHTMAPS && surface->styles[layer] != 255; ++layer) {
        a = map[y * width + x] * (1 - fx) + map[y * width + nextx] * fx;
        b = map[nexty * width + x] * (1 - fx) + map[nexty * width + nextx] * fx;
        value = a * (1 - fy) + b * fy;
        samples[layer] = (byte)(value + .5f);
        map += stride;
    }
}

float XGL_SurfaceLightScale (msurface_t *surface, const byte samples[MAXLIGHTMAPS])
{
    int layer, value;
    float scale;
    if (r_fullbright.value || !cl.worldmodel->lightdata ||
        (surface->flags & (SURF_DRAWSKY | SURF_DRAWTURB | SURF_DRAWTILED))) return 1;
    value = 0;
    for (layer = 0; layer < MAXLIGHTMAPS && surface->styles[layer] != 255; ++layer)
        value += samples[layer] * d_lightstylevalue[surface->styles[layer]];
    /* GLQuake's lightmap has a 2x overbright factor. A small floor allows
     * multiplicative native diffuse lighting to illuminate dark materials;
     * this is an intentional approximation of Quake's additive dlights. */
    scale = value / (128.0f * 255.0f);
    if (scale < .08f) scale = .08f;
    if (scale > 1) scale = 1;
    return scale;
}

static int XGL_RecursiveLightPoint (mnode_t *node, const float *start, const float *end)
{
    vec3_t midpoint;
    float front, back, fraction, s, t;
    msurface_t *surface;
    byte samples[MAXLIGHTMAPS];
    int side, hit, i, layer, value;
    if (node->contents < 0) return -1;
    front = DotProduct(start, node->plane->normal) - node->plane->dist;
    back = DotProduct(end, node->plane->normal) - node->plane->dist;
    side = front < 0;
    if ((back < 0) == side)
        return XGL_RecursiveLightPoint(node->children[side], start, end);
    fraction = front / (front - back);
    for (i = 0; i < 3; ++i) midpoint[i] = start[i] + fraction * (end[i] - start[i]);
    hit = XGL_RecursiveLightPoint(node->children[side], start, midpoint);
    if (hit >= 0) return hit;
    surface = cl.worldmodel->surfaces + node->firstsurface;
    for (i = 0; i < node->numsurfaces; ++i, ++surface) {
        if (surface->flags & (SURF_DRAWTILED | SURF_DRAWSKY | SURF_DRAWTURB)) continue;
        s = DotProduct(midpoint, surface->texinfo->vecs[0]) +
            surface->texinfo->vecs[0][3] - surface->texturemins[0];
        t = DotProduct(midpoint, surface->texinfo->vecs[1]) +
            surface->texinfo->vecs[1][3] - surface->texturemins[1];
        if (s < 0 || t < 0 || s > surface->extents[0] || t > surface->extents[1]) continue;
        XGL_SurfaceLightSamples(surface, midpoint, samples);
        value = 0;
        for (layer = 0; layer < MAXLIGHTMAPS && surface->styles[layer] != 255; ++layer)
            value += samples[layer] * d_lightstylevalue[surface->styles[layer]];
        return value >> 8;
    }
    return XGL_RecursiveLightPoint(node->children[!side], midpoint, end);
}

int R_LightPoint (vec3_t position)
{
    vec3_t end;
    int value;
    if (!cl.worldmodel || !cl.worldmodel->lightdata || r_fullbright.value) return 255;
    VectorCopy(position, end);
    end[2] -= 2048;
    value = XGL_RecursiveLightPoint(cl.worldmodel->nodes, position, end);
    return value < 0 ? 0 : value;
}

void R_PushDlights (void)
{
    /* The client retains its dlight lifetime/radius updates. XGL_BeginScene
     * selects active native lights; there are no CPU dynamic lightmaps. */
}
