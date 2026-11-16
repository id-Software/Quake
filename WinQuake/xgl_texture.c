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
#include <stdlib.h>
#include <string.h>

#define XGL_MAX_MATERIALS 2048
#define XGL_MATERIAL_NAME (MAX_QPATH + 32)
#define XGL_MATERIAL_BYTES (8UL * 1024UL * 1024UL)
#define XGL_MAX_IMAGE_PIXELS (1024UL * 1024UL)

typedef struct {
    char name[XGL_MATERIAL_NAME];
    int width, height;
    qboolean alpha, map_owned;
    byte *pixels;
} xgl_material_t;

static xgl_material_t xgl_materials[XGL_MAX_MATERIALS];
static size_t xgl_material_bytes;

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
    for (i = 0; i < XGL_MAX_MATERIALS; ++i)
        free (xgl_materials[i].pixels);
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
        xgl_material_bytes -= (size_t)material->width * material->height;
        free (material->pixels);
        memset (material, 0, sizeof(*material));
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
    if (xgl_material_bytes - previous > XGL_MATERIAL_BYTES - pixels)
    {
        Sys_Error ("XGL_LoadTexture: 8 MiB CPU material budget exhausted loading %s", identifier);
        return 0;
    }
    /* Stable slots preserve aliases whose cached model still references an ID.
     * Release a replacement first so registry allocations stay within budget. */
    free (material->pixels);
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
    xgl_material_bytes = xgl_material_bytes - previous + pixels;
    return slot + 1;
}

qboolean XGL_SampleTexture (int id, float s, float t, float rgb[3],
                           const byte *translation)
{
    xgl_material_t *material;
    int x, y, index;
    if (!rgb || id <= 0 || id > XGL_MAX_MATERIALS ||
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
    if (translation)
        index = translation[index];
    rgb[0] = xgl_palette[index*3] * (1.0f/255.0f);
    rgb[1] = xgl_palette[index*3+1] * (1.0f/255.0f);
    rgb[2] = xgl_palette[index*3+2] * (1.0f/255.0f);
    return true;
}
