/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* The GL model loader's indexed skins and pose command lists remain CPU
 * data. XGL receives sampled RGB triangles and world-space normals, never
 * texture objects. Native lighting supplies moving-light illumination. */
#include "quakedef.h"
#include "xgl_local.h"
#include "xgl_audio.h"

#define XGL_VERTEX_NORMALS 162
#define XGL_ALIAS_CACHE_SLOTS 16
#define XGL_ALIAS_CACHE_BYTES (1024UL * 1024UL)

/* Particle effects also use this original Quake normal table. */
float r_avertexnormals[XGL_VERTEX_NORMALS][3] = {
#include "anorms.h"
};

static cvar_t xgl_alias_subdivide = {"xgl_alias_subdivide", "1", true, false, 0, NULL};
static cvar_t xgl_sprite_cells = {"xgl_sprite_cells", "8", true, false, 0, NULL};
static cvar_t xgl_entity_triangles = {"xgl_entity_triangles", "2048", true, false, 0, NULL};
static cvar_t xgl_playercolors = {"xgl_playercolors", "1", true, false, 0, NULL};
static byte xgl_player_translation[MAX_SCOREBOARD][256];
static int xgl_player_color[MAX_SCOREBOARD];
static int xgl_entity_extra_used;

typedef struct {
    vec3_t position;
    vec3_t normal;
    float s, t;
} xgl_entity_vertex_t;

typedef struct {
    float rgb[3][3];
    int visible;
} xgl_alias_color_t;

typedef struct {
    int material, depth, command_words, triangle_count, translated, ready;
    unsigned int revision, used;
    byte translation[256];
    int *commands;
    xgl_alias_color_t *colors;
    size_t bytes;
} xgl_alias_cache_t;

typedef struct {
    xgl_alias_cache_t *cache;
    int index, building, opaque;
} xgl_alias_draw_t;

typedef struct {
    vec3_t normals[XGL_VERTEX_NORMALS];
    byte valid[XGL_VERTEX_NORMALS];
} xgl_alias_normals_t;

/* Keep only owned command bytes and sampled colors, never pointers into the
 * engine's movable alias cache. The cap includes the fixed entry table. */
static xgl_alias_cache_t xgl_alias_cache[XGL_ALIAS_CACHE_SLOTS];
static size_t xgl_alias_cache_bytes = sizeof(xgl_alias_cache);
static unsigned int xgl_alias_cache_used;

static void XGL_FreeAliasColors(xgl_alias_cache_t *cache)
{
    free(cache->commands);
    free(cache->colors);
    xgl_alias_cache_bytes -= cache->bytes;
    memset(cache, 0, sizeof(*cache));
}

void XGL_EntitiesShutdown(void)
{
    int i;
    for (i = 0; i < XGL_ALIAS_CACHE_SLOTS; ++i)
        XGL_FreeAliasColors(&xgl_alias_cache[i]);
    xgl_alias_cache_used = 0;
}

static int XGL_OldestAliasColors(void)
{
    int i, oldest;
    oldest = -1;
    for (i = 0; i < XGL_ALIAS_CACHE_SLOTS; ++i) {
        if (!xgl_alias_cache[i].colors) continue;
        if (oldest < 0 || xgl_alias_cache[i].used < xgl_alias_cache[oldest].used)
            oldest = i;
    }
    return oldest;
}

static xgl_alias_cache_t *XGL_AliasColors(const aliashdr_t *header,
                                       int material, int depth,
                                       const byte *translation)
{
    const int *commands, *cursor;
    xgl_alias_cache_t *cache;
    unsigned int revision;
    size_t bytes;
    int count, consumed, triangles, words, i, slot;
    revision = XGL_TextureRevision(material);
    if (!revision || depth < 0 || depth > 2 || header->poseverts < 3 ||
        header->poseverts > MAXALIASTRIS * 3 ||
        header->numtris < 1 || header->numtris > MAXALIASTRIS) return NULL;
    commands = (const int *)((const byte *)header + header->commands);
    cursor = commands;
    consumed = triangles = 0;
    while ((count = *cursor++) != 0) {
        if (count > header->poseverts - consumed || count < -(header->poseverts - consumed))
            return NULL;
        if (count < 0) count = -count;
        if (count < 3) return NULL;
        consumed += count; triangles += count - 2;
        cursor += count * 2;
    }
    if (triangles != header->numtris) return NULL;
    words = (int)(cursor - commands);
    triangles *= 1 << (2 * depth);
    if (++xgl_alias_cache_used == 0) {
        for (i = 0; i < XGL_ALIAS_CACHE_SLOTS; ++i) xgl_alias_cache[i].used = 0;
        xgl_alias_cache_used = 1;
    }
    slot = -1;
    for (i = 0; i < XGL_ALIAS_CACHE_SLOTS; ++i) {
        cache = &xgl_alias_cache[i];
        if (!cache->colors) { if (slot < 0) slot = i; continue; }
        if (cache->material != material || cache->revision != revision ||
            cache->depth != depth || cache->command_words != words ||
            cache->translated != (translation != NULL) ||
            cache->triangle_count != triangles) continue;
        if (translation && memcmp(cache->translation, translation, 256)) continue;
        if (memcmp(cache->commands, commands, (size_t)words * sizeof(int))) continue;
        cache->used = xgl_alias_cache_used;
        return cache;
    }
    bytes = (size_t)words * sizeof(int) + (size_t)triangles * sizeof(xgl_alias_color_t);
    if (bytes > XGL_ALIAS_CACHE_BYTES - sizeof(xgl_alias_cache)) return NULL;
    if (slot < 0) slot = XGL_OldestAliasColors();
    if (slot < 0) return NULL;
    XGL_FreeAliasColors(&xgl_alias_cache[slot]);
    while (bytes > XGL_ALIAS_CACHE_BYTES - xgl_alias_cache_bytes) {
        i = XGL_OldestAliasColors();
        if (i < 0) return NULL;
        XGL_FreeAliasColors(&xgl_alias_cache[i]);
    }
    cache = &xgl_alias_cache[slot];
    cache->commands = (int *)malloc((size_t)words * sizeof(int));
    cache->colors = (xgl_alias_color_t *)malloc((size_t)triangles * sizeof(xgl_alias_color_t));
    if (!cache->commands || !cache->colors) {
        XGL_FreeAliasColors(cache);
        return NULL;
    }
    memcpy(cache->commands, commands, (size_t)words * sizeof(int));
    if (translation) memcpy(cache->translation, translation, 256);
    cache->translated = translation != NULL;
    cache->material = material; cache->revision = revision; cache->depth = depth;
    cache->command_words = words; cache->triangle_count = triangles;
    cache->used = xgl_alias_cache_used; cache->bytes = bytes;
    xgl_alias_cache_bytes += bytes;
    return cache;
}

static int XGL_EntitySetting(float value, int low, int high)
{
    if (!(value >= low)) return low;
    if (value > high) return high;
    return (int)value;
}

void XGL_EntitiesInit(void)
{
    int i;
    XGL_EntitiesShutdown();
    Cvar_RegisterVariable(&xgl_alias_subdivide);
    Cvar_RegisterVariable(&xgl_sprite_cells);
    Cvar_RegisterVariable(&xgl_entity_triangles);
    Cvar_RegisterVariable(&xgl_playercolors);
    for (i = 0; i < MAX_SCOREBOARD; ++i) xgl_player_color[i] = -1;
}

void R_TranslatePlayerSkin(int playernum)
{
    int i, top, bottom;
    byte *table;
    if (playernum < 0 || playernum >= cl.maxclients ||
        playernum >= MAX_SCOREBOARD || !cl.scores) return;
    table = xgl_player_translation[playernum];
    top = cl.scores[playernum].colors & 0xf0;
    bottom = (cl.scores[playernum].colors & 15) << 4;
    for (i = 0; i < 256; ++i) table[i] = (byte)i;
    for (i = 0; i < 16; ++i) {
        table[TOP_RANGE + i] = (byte)(top < 128 ? top + i : top + 15 - i);
        table[BOTTOM_RANGE + i] = (byte)(bottom < 128 ? bottom + i : bottom + 15 - i);
    }
    xgl_player_color[playernum] = cl.scores[playernum].colors;
}

static const byte *XGL_EntityTranslation(entity_t *entity)
{
    int i;
    if (!xgl_playercolors.value || !cl.scores || entity->colormap == vid.colormap)
        return NULL;
    /* Pointer equality avoids subtracting unrelated entity pointers, as the
     * view weapon and temporary entities are not members of cl_entities. */
    for (i = 0; i < cl.maxclients && i < MAX_SCOREBOARD; ++i) {
        if (entity != &cl_entities[i + 1]) continue;
        if (xgl_player_color[i] != cl.scores[i].colors)
            R_TranslatePlayerSkin(i);
        return xgl_player_translation[i];
    }
    return NULL;
}

static void XGL_EntityMidpoint(const xgl_entity_vertex_t *a,
                              const xgl_entity_vertex_t *b,
                              xgl_entity_vertex_t *out)
{
    int i;
    for (i = 0; i < 3; ++i) {
        out->position[i] = (a->position[i] + b->position[i]) * .5f;
        out->normal[i] = (a->normal[i] + b->normal[i]) * .5f;
    }
    if (VectorNormalize(out->normal) == 0) {
        VectorCopy(a->normal, out->normal);
    }
    out->s = (a->s + b->s) * .5f;
    out->t = (a->t + b->t) * .5f;
}

static void XGL_EntityTriangleCached(const xgl_entity_vertex_t *a,
                              const xgl_entity_vertex_t *b,
                              const xgl_entity_vertex_t *c,
                              int material, const byte *translation,
                              float light, int lit, int depth,
                              xgl_alias_draw_t *draw)
{
    const xgl_entity_vertex_t *source[3];
    xgl_entity_vertex_t ab, bc, ca;
    xgl_vertex_t vertices[3];
    xgl_alias_color_t sampled, *colors;
    float center[3];
    int i, j;
    if (depth > 0) {
        XGL_EntityMidpoint(a, b, &ab);
        XGL_EntityMidpoint(b, c, &bc);
        XGL_EntityMidpoint(c, a, &ca);
        XGL_EntityTriangleCached(a, &ab, &ca, material, translation, light, lit, depth - 1, draw);
        XGL_EntityTriangleCached(&ab, b, &bc, material, translation, light, lit, depth - 1, draw);
        XGL_EntityTriangleCached(&ca, &bc, c, material, translation, light, lit, depth - 1, draw);
        XGL_EntityTriangleCached(&ab, &bc, &ca, material, translation, light, lit, depth - 1, draw);
        return;
    }
    colors = &sampled;
    if (draw && draw->cache && draw->index < draw->cache->triangle_count)
        colors = &draw->cache->colors[draw->index++];
    /* Alpha index 255 is coverage, not a black vertex. The centroid decides
     * coverage of each small sprite triangle. Transparent corners inherit
     * its opaque color so that interpolation cannot introduce black fringes. */
    source[0] = a; source[1] = b; source[2] = c;
    if (colors == &sampled || draw->building) {
        colors->visible = 0;
        center[0] = center[1] = center[2] = 0;
        /* Opaque alias skins need no coverage sample. Sprites retain their
         * centroid coverage rule; cached aliases also retain alpha holes. */
        if ((!draw || !draw->opaque) &&
            !XGL_SampleTexture(material, (a->s + b->s + c->s) / 3,
                               (a->t + b->t + c->t) / 3, center, translation)) return;
        for (i = 0; i < 3; ++i) {
            if (!XGL_SampleTexture(material, source[i]->s, source[i]->t,
                                   colors->rgb[i], translation)) {
                VectorCopy(center, colors->rgb[i]);
            }
        }
        colors->visible = 1;
    }
    if (!colors->visible) return;
    for (i = 0; i < 3; ++i) {
        VectorCopy(source[i]->position, vertices[i].position);
        VectorCopy(source[i]->normal, vertices[i].normal);
        for (j = 0; j < 3; ++j) vertices[i].color[j] = colors->rgb[i][j] * light;
    }
    XGL_DrawTriangles(vertices, 3, lit, 1.0f, 1);
}

static void XGL_EntityTriangle(const xgl_entity_vertex_t *a,
                              const xgl_entity_vertex_t *b,
                              const xgl_entity_vertex_t *c,
                              int material, const byte *translation,
                              float light, int lit, int depth)
{
    XGL_EntityTriangleCached(a, b, c, material, translation, light, lit, depth, NULL);
}

static int XGL_AliasDepth(entity_t *entity, int triangles)
{
    vec3_t distance;
    int depth, extra, budget;
    depth = XGL_EntitySetting(xgl_alias_subdivide.value, 0, 2);
    VectorSubtract(entity->origin, r_origin, distance);
    if (entity != &cl.viewent) {
        if (DotProduct(distance, distance) > 512.0f * 512.0f) depth = 0;
        else if (depth > 1 && DotProduct(distance, distance) > 192.0f * 192.0f) depth = 1;
    }
    budget = XGL_EntitySetting(xgl_entity_triangles.value, 0, 32768);
    while (depth > 0) {
        extra = triangles * ((1 << (2 * depth)) - 1);
        if (extra <= budget - xgl_entity_extra_used) {
            xgl_entity_extra_used += extra;
            break;
        }
        --depth;
    }
    return depth;
}

static void XGL_AliasVertex(const trivertx_t *pose, const aliashdr_t *header,
                           const vec3_t forward, const vec3_t right,
                           const vec3_t up, const vec3_t origin,
                           const int *uv, xgl_entity_vertex_t *out,
                           xgl_alias_normals_t *normals)
{
    vec3_t local, normal;
    int i, index;
    /* Read the embedded floats without strict-aliasing or SPARC alignment
     * assumptions beyond the loader's aligned int command buffer. */
    memcpy(&out->s, uv, sizeof(float));
    memcpy(&out->t, uv + 1, sizeof(float));
    index = pose->lightnormalindex;
    if (index >= XGL_VERTEX_NORMALS) index = 0;
    for (i = 0; i < 3; ++i) {
        local[i] = pose->v[i] * header->scale[i] + header->scale_origin[i];
    }
    if (!normals->valid[index]) {
        for (i = 0; i < 3; ++i)
            normal[i] = header->scale[i] != 0 ?
                r_avertexnormals[index][i] / header->scale[i] : r_avertexnormals[index][i];
        for (i = 0; i < 3; ++i)
            normals->normals[index][i] = forward[i] * normal[0] -
                right[i] * normal[1] + up[i] * normal[2];
        VectorNormalize(normals->normals[index]);
        normals->valid[index] = 1;
    }
    for (i = 0; i < 3; ++i) {
        out->position[i] = origin[i] + forward[i] * local[0] -
            right[i] * local[1] + up[i] * local[2];
        out->normal[i] = normals->normals[index][i];
    }
}

static void XGL_DrawAlias(entity_t *entity)
{
    aliashdr_t *header;
    const trivertx_t *pose;
    const byte *translation;
    const int *order;
    xgl_entity_vertex_t a, b, v;
    xgl_alias_draw_t draw;
    xgl_alias_normals_t normals;
    vec3_t angles, forward, right, up, mins, maxs;
    float light, radius;
    double time;
    int i, frame, posenum, animation, skin, material, count, fan, consumed, depth, lit;
    if (entity != &cl.viewent) {
        radius = entity->model->radius;
        for (i = 0; i < 3; ++i) {
            mins[i] = entity->origin[i] - radius;
            maxs[i] = entity->origin[i] + radius;
        }
        /* A loader without a valid radius must not reduce an animated model
         * to a point for culling. The XGL loader computes pose-wide bounds. */
        if (radius > 0 && R_CullBox(mins, maxs)) return;
    }
    /* A sound cache miss can move or evict Quake alias-cache allocations.
     * Service audio before acquiring pointers, then defer mixer calls until
     * the last header/pose/command access. Native batches still submit as
     * usual; their audio hooks respect this short, nestable cache lock. */
    XGL_AudioLock();
    header = (aliashdr_t *)Mod_Extradata(entity->model);
    if (!header || header->poseverts <= 0 || header->numposes <= 0 || header->numskins <= 0) {
        XGL_AudioUnlock();
        return;
    }
    frame = entity->frame;
    if (frame < 0 || frame >= header->numframes) frame = 0;
    posenum = header->frames[frame].firstpose;
    time = cl.time + entity->syncbase;
    if (header->frames[frame].numposes > 1 && header->frames[frame].interval > 0) {
        animation = (int)(time / header->frames[frame].interval) % header->frames[frame].numposes;
        if (animation < 0) animation += header->frames[frame].numposes;
        posenum += animation;
    }
    if (posenum < 0 || posenum >= header->numposes) posenum = 0;
    skin = entity->skinnum;
    if (skin < 0 || skin >= header->numskins) skin = 0;
    animation = ((int)(time * 10)) & 3;
    material = header->gl_texturenum[skin][animation];
    translation = XGL_EntityTranslation(entity);
    lit = r_fullbright.value ? 0 : 2;
    light = lit ? R_LightPoint(entity->origin) / 128.0f : 1.0f;
    if (!strcmp(entity->model->name, "progs/flame.mdl") ||
        !strcmp(entity->model->name, "progs/flame2.mdl")) { light = 1; lit = 0; }
    if (entity == &cl.viewent && light < .20f) light = .20f;
    if (translation && light < .08f) light = .08f;
    if (light < .04f) light = .04f;
    if (light > 1) light = 1;
    VectorCopy(entity->angles, angles);
    /* The original alias path rotates -pitch; brush entities use the other
     * convention. Negating only this local copy preserves entity state. */
    angles[PITCH] = -angles[PITCH];
    AngleVectors(angles, forward, right, up);
    pose = (const trivertx_t *)((const byte *)header + header->posedata) + posenum * header->poseverts;
    order = (const int *)((const byte *)header + header->commands);
    depth = XGL_AliasDepth(entity, header->numtris);
    memset(&draw, 0, sizeof(draw));
    memset(normals.valid, 0, sizeof(normals.valid));
    draw.cache = XGL_AliasColors(header, material, depth, translation);
    draw.building = draw.cache && !draw.cache->ready;
    draw.opaque = XGL_TextureRevision(material) != 0 && !XGL_TextureHasAlpha(material);
    consumed = 0;
    c_alias_polys += header->numtris;
    while ((count = *order++) != 0) {
        if (count > header->poseverts - consumed || count < -(header->poseverts - consumed)) {
            XGL_AudioUnlock();
            Con_DPrintf("XGL: invalid alias commands in %s\n", entity->model->name);
            return;
        }
        fan = count < 0;
        if (fan) count = -count;
        for (i = 0; i < count; ++i) {
            XGL_AliasVertex(pose++, header, forward, right, up, entity->origin, order, &v, &normals);
            order += 2;
            if (i == 0) a = v;
            else if (i == 1) b = v;
            else {
                if (!fan && (i & 1))
                    XGL_EntityTriangleCached(&b, &a, &v, material, translation, light, lit, depth, &draw);
                else XGL_EntityTriangleCached(&a, &b, &v, material, translation, light, lit, depth, &draw);
                if (!fan) a = b;
                b = v;
            }
        }
        consumed += count;
    }
    if (draw.cache && draw.index == draw.cache->triangle_count) draw.cache->ready = 1;
    XGL_AudioUnlock();
}

static mspriteframe_t *XGL_SpriteFrame(entity_t *entity, msprite_t *sprite)
{
    mspritegroup_t *group;
    double time, cycle, target;
    int frame, i;
    if (sprite->numframes <= 0) return NULL;
    frame = entity->frame;
    if (frame < 0 || frame >= sprite->numframes) frame = 0;
    if (sprite->frames[frame].type == SPR_SINGLE) return sprite->frames[frame].frameptr;
    group = (mspritegroup_t *)sprite->frames[frame].frameptr;
    if (!group || group->numframes <= 0) return NULL;
    cycle = group->intervals[group->numframes - 1];
    if (!(cycle > 0)) return group->frames[0];
    time = cl.time + entity->syncbase;
    target = time - floor(time / cycle) * cycle;
    for (i = 0; i < group->numframes - 1; ++i)
        if (target < group->intervals[i]) break;
    return group->frames[i];
}

static int XGL_SpriteAxes(entity_t *entity, int type, vec3_t right, vec3_t up, vec3_t normal)
{
    vec3_t direction;
    float angle, sr, cr;
    int i;
    if (type == SPR_ORIENTED) {
        AngleVectors(entity->angles, normal, right, up);
        VectorScale(normal, -1, normal);
    } else if (type == SPR_VP_PARALLEL_UPRIGHT || type == SPR_FACING_UPRIGHT) {
        if (type == SPR_FACING_UPRIGHT) {
            VectorSubtract(entity->origin, r_origin, direction);
        } else {
            VectorCopy(vpn, direction);
        }
        direction[2] = 0;
        if (VectorNormalize(direction) < .001f) return 0;
        up[0] = up[1] = 0; up[2] = 1;
        right[0] = direction[1]; right[1] = -direction[0]; right[2] = 0;
        VectorScale(direction, -1, normal);
    } else if (type == SPR_VP_PARALLEL_ORIENTED) {
        angle = entity->angles[ROLL] * (M_PI / 180);
        sr = sin(angle); cr = cos(angle);
        for (i = 0; i < 3; ++i) {
            right[i] = vright[i] * cr + vup[i] * sr;
            up[i] = vright[i] * -sr + vup[i] * cr;
            normal[i] = -vpn[i];
        }
    } else {
        VectorCopy(vright, right);
        VectorCopy(vup, up);
        VectorScale(vpn, -1, normal);
    }
    return 1;
}

static void XGL_SpriteVertex(const mspriteframe_t *frame, const vec3_t origin,
                            const vec3_t right, const vec3_t up, const vec3_t normal,
                            float s, float t, xgl_entity_vertex_t *vertex)
{
    float x, y;
    int i;
    x = frame->left + (frame->right - frame->left) * s;
    y = frame->up + (frame->down - frame->up) * t;
    for (i = 0; i < 3; ++i) {
        vertex->position[i] = origin[i] + right[i] * x + up[i] * y;
        vertex->normal[i] = normal[i];
    }
    /* Keep endpoint samples on the outermost texel rather than wrapping
     * exactly 1.0 back to the opposite edge of a sprite image. */
    vertex->s = (.5f + s * (frame->width - 1)) / frame->width;
    vertex->t = (.5f + t * (frame->height - 1)) / frame->height;
}

static void XGL_DrawSprite(entity_t *entity)
{
    msprite_t *sprite;
    mspriteframe_t *frame;
    xgl_entity_vertex_t corners[4];
    vec3_t right, up, normal, origin, mins, maxs;
    int x, y, cells, i;
    float s0, s1, t0, t1, radius;
    sprite = (msprite_t *)entity->model->cache.data;
    if (!sprite) return;
    frame = XGL_SpriteFrame(entity, sprite);
    if (!frame || frame->width <= 0 || frame->height <= 0) return;
    if (!XGL_SpriteAxes(entity, sprite->type, right, up, normal)) return;
    VectorMA(entity->origin, sprite->beamlength, normal, origin);
    radius = entity->model->radius;
    if (radius > 0) {
        for (i = 0; i < 3; ++i) {
            mins[i] = origin[i] - radius;
            maxs[i] = origin[i] + radius;
        }
        if (R_CullBox(mins, maxs)) return;
    }
    cells = XGL_EntitySetting(xgl_sprite_cells.value, 2, 16);
    for (y = 0; y < cells; ++y) {
        t0 = (float)y / cells; t1 = (float)(y + 1) / cells;
        for (x = 0; x < cells; ++x) {
            s0 = (float)x / cells; s1 = (float)(x + 1) / cells;
            XGL_SpriteVertex(frame, origin, right, up, normal, s0, t0, &corners[0]);
            XGL_SpriteVertex(frame, origin, right, up, normal, s1, t0, &corners[1]);
            XGL_SpriteVertex(frame, origin, right, up, normal, s1, t1, &corners[2]);
            XGL_SpriteVertex(frame, origin, right, up, normal, s0, t1, &corners[3]);
            XGL_EntityTriangle(&corners[0], &corners[1], &corners[2],
                               frame->gl_texturenum, NULL, 1, 0, 0);
            XGL_EntityTriangle(&corners[0], &corners[2], &corners[3],
                               frame->gl_texturenum, NULL, 1, 0, 0);
        }
    }
}

void XGL_DrawEntities(void)
{
    int i;
    xgl_entity_extra_used = 0;
    if (!r_drawentities.value) return;
    for (i = 0; i < cl_numvisedicts; ++i) {
        currententity = cl_visedicts[i];
        if (!currententity || !currententity->model) continue;
        if (currententity->model->type == mod_alias) XGL_DrawAlias(currententity);
        else if (currententity->model->type == mod_brush) XGL_DrawBrushModel(currententity);
    }
    for (i = 0; i < cl_numvisedicts; ++i) {
        currententity = cl_visedicts[i];
        if (currententity && currententity->model && currententity->model->type == mod_sprite)
            XGL_DrawSprite(currententity);
    }
}

void XGL_DrawViewModel(void)
{
    if (!r_drawviewmodel.value || !r_drawentities.value || chase_active.value ||
        (cl.items & IT_INVISIBILITY) || cl.stats[STAT_HEALTH] <= 0 ||
        !cl.viewent.model || cl.viewent.model->type != mod_alias) return;
    currententity = &cl.viewent;
    /* Finish world work before clearing depth for the weapon. The backend
     * retains world color and resumes normal depth state afterwards. */
    XGL_SetViewModel(1);
    XGL_DrawAlias(currententity);
    XGL_SetViewModel(0);
}
