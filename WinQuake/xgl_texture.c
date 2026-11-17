/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/* xgl_texture.c: bounded CPU indexed materials, never XGL texture objects. */
#include "quakedef.h"
#include "xgl_local.h"
#include "xgl_audio.h"
#include <stdlib.h>
#include <string.h>

#define XGL_MAX_MATERIALS 2048
#define XGL_MATERIAL_NAME (MAX_QPATH + 32)
#define XGL_MATERIAL_BYTES (8UL * 1024UL * 1024UL)
#define XGL_MAX_IMAGE_PIXELS (1024UL * 1024UL)
#define XGL_FILTER_LEVELS 12

typedef struct {
    char name[XGL_MATERIAL_NAME];
    int width, height;
    qboolean alpha, has_alpha, map_owned;
    byte *pixels, *filtered;
    size_t filter_bytes, filter_offset[XGL_FILTER_LEVELS];
    int filter_levels, filter_attempted;
    unsigned int revision;
} xgl_material_t;

static xgl_material_t xgl_materials[XGL_MAX_MATERIALS];
static size_t xgl_material_bytes;
static unsigned int xgl_material_revision;

/* Called by the model loader; alias/cache and startup materials remain live. */
void XGL_TextureMarkMap (int id);
void XGL_TextureClearMap (void);

void XGL_TextureInit (void)
{
    /* Static storage starts empty. Repeated renderer setup must not invalidate
     * material IDs retained by the engine alias cache. Shutdown owns freeing. */
}

void XGL_TextureShutdown (void)
{
    int i;
    for (i = 0; i < XGL_MAX_MATERIALS; ++i) {
        free (xgl_materials[i].pixels);
        free (xgl_materials[i].filtered);
    }
    memset (xgl_materials, 0, sizeof(xgl_materials));
    xgl_material_bytes = 0;
}

void XGL_TextureMarkMap (int id)
{
    if (id > 0 && id <= XGL_MAX_MATERIALS && xgl_materials[id-1].pixels)
        xgl_materials[id-1].map_owned = true;
}

void XGL_TextureClearMap (void)
{
    int i;
    xgl_material_t *material;
    for (i = 0; i < XGL_MAX_MATERIALS; ++i)
    {
        material = &xgl_materials[i];
        if (!material->pixels || !material->map_owned)
            continue;
        xgl_material_bytes -= (size_t)material->width * material->height + material->filter_bytes;
        free (material->pixels);
        free (material->filtered);
        memset (material, 0, sizeof(*material));
    }
}

/* Filtering is an optional cache. Indexed assets always have priority within
 * the same 8 MiB heap budget, and live material IDs are never evicted. */
static void XGL_ClearTextureFilters(void)
{
    int i;
    xgl_material_t *material;
    for (i = 0; i < XGL_MAX_MATERIALS; ++i) {
        material = &xgl_materials[i];
        xgl_material_bytes -= material->filter_bytes;
        free(material->filtered);
        material->filtered = NULL;
        material->filter_bytes = 0;
        material->filter_levels = material->filter_attempted = 0;
    }
}

int XGL_LoadTexture (char *identifier, int width, int height, byte *data,
                    qboolean mipmap, qboolean alpha)
{
    int i, slot;
    size_t pixels, previous;
    xgl_material_t *material;
    (void)mipmap; /* Original texels are sampled; no GPU texture or mip upload. */
    if (!identifier || strlen(identifier) >= XGL_MATERIAL_NAME || !data ||
        width <= 0 || width > 2048 || height <= 0 || height > 2048 ||
        (size_t)width > XGL_MAX_IMAGE_PIXELS / (size_t)height)
    {
        Sys_Error ("XGL_LoadTexture: invalid name or dimensions (%dx%d)", width, height);
        return 0;
    }
    pixels = (size_t)width * height;
    slot = -1;
    for (i = 0; i < XGL_MAX_MATERIALS; ++i)
    {
        material = &xgl_materials[i];
        if (!material->pixels)
        {
            if (slot < 0) slot = i;
            continue;
        }
        /* Empty identifiers request independent materials, like GL_LoadTexture. */
        if (*identifier && !strcmp(material->name, identifier))
        {
            slot = i;
            break;
        }
    }
    if (slot < 0)
    {
        Sys_Error ("XGL_LoadTexture: %d CPU material slots exhausted", XGL_MAX_MATERIALS);
        return 0;
    }
    material = &xgl_materials[slot];
    previous = material->pixels ? (size_t)material->width * material->height : 0;
    if (previous == pixels && material->width == width && material->height == height &&
        material->alpha == alpha && !memcmp(material->pixels, data, pixels))
        return slot + 1;
    if (xgl_material_bytes - previous - material->filter_bytes > XGL_MATERIAL_BYTES - pixels)
        XGL_ClearTextureFilters();
    if (xgl_material_bytes - previous - material->filter_bytes > XGL_MATERIAL_BYTES - pixels)
    {
        Sys_Error ("XGL_LoadTexture: 8 MiB CPU material budget exhausted loading %s", identifier);
        return 0;
    }
    /* Stable slots preserve aliases whose cached model still references an ID.
     * Release a replacement first so registry allocations stay within budget. */
    free (material->pixels);
    free (material->filtered);
    xgl_material_bytes -= material->filter_bytes;
    material->filtered = NULL;
    material->filter_bytes = 0;
    material->filter_levels = material->filter_attempted = 0;
    material->pixels = (byte *)malloc (pixels);
    if (!material->pixels)
    {
        Sys_Error ("XGL_LoadTexture: unable to allocate %lu indexed bytes", (unsigned long)pixels);
        return 0;
    }
    memcpy (material->pixels, data, pixels);
    strcpy (material->name, identifier);
    material->width = width;
    material->height = height;
    material->alpha = alpha;
    material->has_alpha = alpha && memchr(data, 255, pixels) != NULL;
    if (++xgl_material_revision == 0) ++xgl_material_revision;
    material->revision = xgl_material_revision;
    xgl_material_bytes = xgl_material_bytes - previous + pixels;
    return slot + 1;
}

/* The animated world path uses the palette index directly for gamma-ready
 * colors. Keep wrapping and cutout coverage identical to ordinary samples. */
qboolean XGL_SampleTextureIndex (int id, float s, float t, int *result)
{
    xgl_material_t *material;
    int x, y, index;
    if (!result || id <= 0 || id > XGL_MAX_MATERIALS ||
        !(s > -1000000000.0f && s < 1000000000.0f &&
          t > -1000000000.0f && t < 1000000000.0f))
        return false;
    material = &xgl_materials[id-1];
    if (!material->pixels)
        return false;
    /* Repeat normalized coordinates, including negative world/water UVs.
     * The bounded conversion avoids per-sample floor/fmod library calls. */
    s -= (float)(int)s;
    t -= (float)(int)t;
    if (s < 0) s += 1;
    if (t < 0) t += 1;
    x = (int)(s * material->width);
    y = (int)(t * material->height);
    if (x >= material->width) x = material->width - 1;
    if (y >= material->height) y = material->height - 1;
    index = material->pixels[y * material->width + x];
    if (material->alpha && index == 255)
        return false;
    *result = index;
    return true;
}

qboolean XGL_SampleTexture (int id, float s, float t, float rgb[3],
                           const byte *translation)
{
    int index;
    if (!rgb || !XGL_SampleTextureIndex(id, s, t, &index))
        return false;
    if (translation)
        index = translation[index];
    rgb[0] = xgl_palette[index*3] * (1.0f/255.0f);
    rgb[1] = xgl_palette[index*3+1] * (1.0f/255.0f);
    rgb[2] = xgl_palette[index*3+2] * (1.0f/255.0f);
    return true;
}


unsigned int XGL_TextureRevision(int id)
{
    if (id <= 0 || id > XGL_MAX_MATERIALS || !xgl_materials[id-1].pixels) return 0;
    return xgl_materials[id-1].revision;
}

qboolean XGL_TextureHasAlpha(int id)
{
    if (id <= 0 || id > XGL_MAX_MATERIALS || !xgl_materials[id-1].pixels) return false;
    return xgl_materials[id-1].has_alpha;
}

/* RGB averages never interpolate palette indices. Every level covers the
 * complete repeating image; odd final rows/columns are included. */
static void XGL_BuildTextureFilter(xgl_material_t *material)
{
    size_t bytes, offset;
    int width, height, next_width, next_height, level, x, y, c;
    int sx, sy, x0, x1, y0, y1, count, index, block, weight, wx, wy;
    unsigned int sum;
    const byte *source;
    byte *target;
    if (material->filter_attempted) return;
    material->filter_attempted = 1;
    width = material->width; height = material->height;
    bytes = 0; level = 0;
    while ((width > 1 || height > 1) && level < XGL_FILTER_LEVELS) {
        width = (width + 1) / 2; height = (height + 1) / 2;
        material->filter_offset[level++] = bytes;
        bytes += (size_t)width * height * 3;
    }
    if (!bytes || bytes > XGL_MATERIAL_BYTES - xgl_material_bytes) return;
    material->filtered = (byte *)malloc(bytes);
    if (!material->filtered) return;
    material->filter_bytes = bytes; material->filter_levels = level;
    xgl_material_bytes += bytes;
    source = material->pixels;
    width = material->width; height = material->height;
    for (level = 0; level < material->filter_levels; ++level) {
        next_width = (width + 1) / 2; next_height = (height + 1) / 2;
        block = 1 << level;
        offset = material->filter_offset[level];
        target = material->filtered + offset;
        for (y = 0; y < next_height; ++y) {
            if ((y & 31) == 0) XGL_AudioUpdate();
            y0 = y * 2; y1 = y0 + 2; if (y1 > height) y1 = height;
            for (x = 0; x < next_width; ++x) {
                x0 = x * 2; x1 = x0 + 2; if (x1 > width) x1 = width;
                wx = x1 * block; if (wx > material->width) wx = material->width;
                wy = y1 * block; if (wy > material->height) wy = material->height;
                count = (wx - x0 * block) * (wy - y0 * block);
                for (c = 0; c < 3; ++c) {
                    sum = 0;
                    for (sy = y0; sy < y1; ++sy)
                        for (sx = x0; sx < x1; ++sx) {
                            index = sy * width + sx;
                            wx = material->width - sx * block; if (wx > block) wx = block;
                            wy = material->height - sy * block; if (wy > block) wy = block;
                            weight = wx * wy;
                            sum += weight * (level ? source[index * 3 + c] :
                                xgl_palette[source[index] * 3 + c]);
                        }
                    target[(y * next_width + x) * 3 + c] = (byte)((sum + count / 2) / count);
                }
            }
        }
        source = target; width = next_width; height = next_height;
    }
}

qboolean XGL_SampleTextureFiltered(int id, float s, float t,
                                  float footprint_s, float footprint_t,
                                  float rgb[3], const byte *translation)
{
    xgl_material_t *material;
    float extent, fs, ft, xfrac, yfrac, samples[3], sum[3];
    const byte *pixels;
    int level, width, height, x0, x1, y0, y1, x, y, c, count;
    if (!XGL_SampleTexture(id, s, t, rgb, translation)) return false;
    if (!(footprint_s >= 0 && footprint_s < 1000000000.0f &&
          footprint_t >= 0 && footprint_t < 1000000000.0f)) return true;
    material = &xgl_materials[id-1];
    fs = footprint_s * material->width; ft = footprint_t * material->height;
    extent = fs > ft ? fs : ft;
    if (extent < 2) return true;
    if (!translation && !material->has_alpha) {
        XGL_BuildTextureFilter(material);
        if (material->filtered) {
            level = 0; width = (material->width + 1) / 2;
            height = (material->height + 1) / 2;
            while (extent >= 4 && level + 1 < material->filter_levels) {
                extent *= .5f; ++level;
                width = (width + 1) / 2; height = (height + 1) / 2;
            }
            s -= (float)(int)s; t -= (float)(int)t;
            if (s < 0) s += 1;
            if (t < 0) t += 1;
            fs = s * width - .5f; ft = t * height - .5f;
            x0 = (int)fs; y0 = (int)ft;
            if (fs < 0) --x0;
            if (ft < 0) --y0;
            xfrac = fs - x0; yfrac = ft - y0;
            if (x0 < 0) x0 += width;
            if (y0 < 0) y0 += height;
            x1 = x0 + 1; if (x1 == width) x1 = 0;
            y1 = y0 + 1; if (y1 == height) y1 = 0;
            pixels = material->filtered + material->filter_offset[level];
            for (c = 0; c < 3; ++c) {
                fs = pixels[(y0 * width + x0) * 3 + c] * (1 - xfrac) +
                     pixels[(y0 * width + x1) * 3 + c] * xfrac;
                ft = pixels[(y1 * width + x0) * 3 + c] * (1 - xfrac) +
                     pixels[(y1 * width + x1) * 3 + c] * xfrac;
                rgb[c] = (fs * (1 - yfrac) + ft * yfrac) / 255.0f;
            }
            return true;
        }
    }
    /* Optional-cache exhaustion and translated/cutout materials use a fixed
     * 4x4 footprint average. Coverage remains the original center sample. */
    if (footprint_s > 1) footprint_s = 1;
    if (footprint_t > 1) footprint_t = 1;
    sum[0] = sum[1] = sum[2] = 0; count = 0;
    for (y = 0; y < 4; ++y)
        for (x = 0; x < 4; ++x)
            if (XGL_SampleTexture(id, s + ((x + .5f) / 4 - .5f) * footprint_s,
                                 t + ((y + .5f) / 4 - .5f) * footprint_t, samples, translation)) {
                for (c = 0; c < 3; ++c) sum[c] += samples[c];
                ++count;
            }
    if (count) for (c = 0; c < 3; ++c) rgb[c] = sum[c] / count;
    return true;
}
