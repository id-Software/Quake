/* Host-only entity rendering contract tests. No XGL runtime is simulated. */
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
static void *entity_test_malloc(size_t bytes);
static float entity_test_normalize(float vector[3]);
#define VectorNormalize entity_test_normalize
#define malloc entity_test_malloc
#include "../xgl_entities.c"
#undef malloc
#undef VectorNormalize
extern float VectorNormalize(float vector[3]);

client_state_t cl;
entity_t cl_entities[MAX_EDICTS];
entity_t *cl_visedicts[MAX_VISEDICTS];
int cl_numvisedicts;
viddef_t vid;
entity_t *currententity;
vec3_t r_origin, vpn, vright, vup;
int c_alias_polys;
cvar_t r_fullbright, r_drawentities, r_drawviewmodel, chase_active;
byte xgl_palette[768];

static xgl_vertex_t captured[4096];
static int captured_count, native_lit, material_seen, coverage_mode;
static int sample_calls, test_light, draw_calls, normalizations;
static byte material_pixels[16 * 16];
static xgl_native_vertex_t reserved_vertices[XGL_ENTITY_BATCH_VERTICES];
static int reserved_count, reserved_lit, reserve_calls, prepare_calls;
static int reserve_limit = XGL_ENTITY_BATCH_VERTICES;
static void capture_reserved(void);
static unsigned long profile_counts[XGL_PROFILE_COUNTER_COUNT];
static xgl_profile_draw_scope_t *profile_draw_active;
static xgl_profile_draw_scope_t *profile_draw_stack[4];
static int profile_draw_depth, profile_draw_max_depth;
static int profile_draw_calls[XGL_PROFILE_DRAW_COUNT];
static int profile_sections[XGL_PROFILE_SECTION_COUNT];
static int light_calls;
static vec3_t light_origin;
static unsigned int material_revision;
static int allocation_fail_after = -1;
static int audio_lock_depth, audio_locks, audio_unlocks, expect_alias_lock;
static int test_cull;
static int weapon_scope[8], weapon_scope_count;
static scoreboard_t scores[MAX_SCOREBOARD];
static byte base_colormap[256], player_colormap[256];
static union { double alignment; byte bytes[32768]; } alias_storage;
static model_t alias_model;

static void *entity_test_malloc(size_t bytes)
{
    if (allocation_fail_after == 0) return NULL;
    if (allocation_fail_after > 0) --allocation_fail_after;
    return malloc(bytes);
}

static float entity_test_normalize(float vector[3])
{
    ++normalizations;
    return VectorNormalize(vector);
}

double XGL_ProfileDetailNow(void) { return 0; }
void XGL_ProfileAdd(xgl_profile_section_t section, double start)
{
    assert((unsigned int)section < XGL_PROFILE_SECTION_COUNT);
    assert(start == 0);
    ++profile_sections[section];
}
void XGL_ProfileCount(xgl_profile_counter_t counter, unsigned long count)
{
    assert((unsigned int)counter < XGL_PROFILE_COUNTER_COUNT);
    profile_counts[counter] += count;
}
void XGL_ProfileDrawBegin(xgl_profile_draw_scope_t *scope)
{
    assert(profile_draw_depth < 4);
    profile_draw_stack[profile_draw_depth++] = scope;
    profile_draw_active = scope;
    if (profile_draw_depth > profile_draw_max_depth)
        profile_draw_max_depth = profile_draw_depth;
}
void XGL_ProfileDrawEnd(xgl_profile_draw_scope_t *scope, xgl_profile_draw_t draw)
{
    assert(profile_draw_depth > 0 && profile_draw_active == scope);
    assert(profile_draw_stack[profile_draw_depth - 1] == scope);
    assert((unsigned int)draw < XGL_PROFILE_DRAW_COUNT);
    ++profile_draw_calls[draw];
    --profile_draw_depth;
    profile_draw_active = profile_draw_depth ? profile_draw_stack[profile_draw_depth - 1] : NULL;
}
void Cvar_RegisterVariable(cvar_t *variable) { (void)variable; }
void Con_DPrintf(char *format, ...) { (void)format; }
void Sys_Error(char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    abort();
}
void XGL_AudioLock(void) { ++audio_lock_depth; ++audio_locks; }
void XGL_AudioUnlock(void)
{
    assert(audio_lock_depth > 0);
    capture_reserved();
    --audio_lock_depth; ++audio_unlocks;
}
void *Mod_Extradata(model_t *model)
{
    assert(audio_lock_depth > 0);
    expect_alias_lock = 1;
    return model->cache.data;
}
qboolean R_CullBox(vec3_t mins, vec3_t maxs) { (void)mins; (void)maxs; return test_cull; }
int R_LightPoint(vec3_t point)
{
    ++light_calls;
    VectorCopy(point, light_origin);
    return test_light;
}
unsigned int XGL_TextureRevision(int material) { return material > 0 ? material_revision : 0; }
qboolean XGL_TextureHasAlpha(int material) { (void)material; return coverage_mode != 0; }
void XGL_DrawBrushModel(entity_t *entity) { (void)entity; }
void XGL_SetViewModel(int enabled)
{
    assert(weapon_scope_count < 8);
    weapon_scope[weapon_scope_count++] = enabled;
}
void XGL_DrawTriangles(const xgl_vertex_t *vertices, int count, int lit,
                       float opacity, int zwrite)
{
    int i;
    if (expect_alias_lock) assert(audio_lock_depth > 0);
    assert(count > 0 && count % 3 == 0);
    assert(captured_count + count <= 4096);
    assert(opacity == 1 && zwrite == 1);
    native_lit = lit;
    ++draw_calls;
    for (i = 0; i < count; ++i) {
        assert(fabs(DotProduct(vertices[i].normal, vertices[i].normal) - 1) < .0001);
        captured[captured_count++] = vertices[i];
    }
}
static void capture_reserved(void)
{
    xgl_vertex_t vertices[XGL_ENTITY_BATCH_VERTICES];
    int i, count;
    count = reserved_count;
    if (!count) return;
    reserved_count = 0;
    for (i = 0; i < count; ++i) {
        VectorCopy(reserved_vertices[i].position, vertices[i].position);
        VectorCopy(reserved_vertices[i].normal, vertices[i].normal);
        VectorCopy(reserved_vertices[i].color.rgb, vertices[i].color);
    }
    XGL_DrawTriangles(vertices, count, reserved_lit, 1, 1);
}

xgl_native_vertex_t *XGL_ReserveTriangles(int *count, int lit, float opacity, int zwrite)
{
    capture_reserved();
    assert(*count > 0 && *count % 3 == 0 && opacity == 1 && zwrite == 1);
    if (expect_alias_lock) assert(audio_lock_depth > 0);
    if (*count > reserve_limit) *count = reserve_limit;
    reserved_count = *count; reserved_lit = lit;
    ++reserve_calls;
    return reserved_vertices;
}

void XGL_PrepareColor(float rgb[3])
{
    (void)rgb;
    ++prepare_calls;
}

qboolean XGL_SampleTextureIndex(int material, float s, float t, int *result)
{
    int x, y, index;
    ++sample_calls;
    material_seen = material;
    if (!(s > -1000000000.0f && s < 1000000000.0f &&
          t > -1000000000.0f && t < 1000000000.0f)) return false;
    s -= (float)(int)s; t -= (float)(int)t;
    if (s < 0) s += 1;
    if (t < 0) t += 1;
    x = (int)(s * 16); y = (int)(t * 16);
    if (x > 15) x = 15;
    if (y > 15) y = 15;
    index = material_pixels[y * 16 + x];
    if (coverage_mode == 1 || (coverage_mode == 2 && s < .5f)) index = 255;
    if (coverage_mode && index == 255) return false;
    *result = index;
    return true;
}

static void reset_test(void)
{
    int i;
    assert(!audio_lock_depth && audio_locks == audio_unlocks);
    memset(&cl, 0, sizeof(cl));
    memset(cl_entities, 0, sizeof(cl_entities));
    memset(&vid, 0, sizeof(vid));
    memset(scores, 0, sizeof(scores));
    memset(r_origin, 0, sizeof(r_origin));
    memset(vpn, 0, sizeof(vpn)); vpn[0] = 1;
    memset(vright, 0, sizeof(vright)); vright[1] = -1;
    memset(vup, 0, sizeof(vup)); vup[2] = 1;
    cl.scores = scores;
    cl.maxclients = 1;
    vid.colormap = base_colormap;
    r_fullbright.value = chase_active.value = 0;
    r_drawentities.value = r_drawviewmodel.value = 1;
    xgl_alias_subdivide.value = 0;
    xgl_sprite_cells.value = 2;
    xgl_entity_triangles.value = 2048;
    xgl_playercolors.value = 1;
    xgl_entity_extra_used = 0;
    captured_count = native_lit = material_seen = coverage_mode = weapon_scope_count = 0;
    sample_calls = draw_calls = normalizations = 0; test_light = 128; material_revision = 1;
    assert(!reserved_count);
    reserved_lit = reserve_calls = prepare_calls = 0;
    reserve_limit = XGL_ENTITY_BATCH_VERTICES;
    memset(profile_counts, 0, sizeof(profile_counts));
    assert(!profile_draw_active && !profile_draw_depth);
    profile_draw_max_depth = 0;
    memset(profile_draw_calls, 0, sizeof(profile_draw_calls));
    memset(profile_sections, 0, sizeof(profile_sections));
    light_calls = 0;
    memset(light_origin, 0, sizeof(light_origin));
    /* A repeating 16x16 indexed fixture: UV (.1,.2) samples TOP_RANGE.
     * Distinct palette channels expose player translation and UV changes. */
    for (i = 0; i < 256; ++i) {
        material_pixels[i] = (byte)(i + 223);
        xgl_palette[i * 3] = (byte)(i * 37 + 31);
        xgl_palette[i * 3 + 1] = (byte)(255 - i);
        xgl_palette[i * 3 + 2] = (byte)i;
    }
    allocation_fail_after = -1;
    audio_locks = audio_unlocks = expect_alias_lock = test_cull = 0;
    XGL_EntitiesInit();
}

static aliashdr_t *make_alias(int count)
{
    aliashdr_t *header;
    trivertx_t *poses;
    int *commands;
    int i, vertices;
    float uv[2];
    memset(&alias_storage, 0, sizeof(alias_storage));
    memset(&alias_model, 0, sizeof(alias_model));
    header = (aliashdr_t *)alias_storage.bytes;
    header->scale[0] = header->scale[1] = header->scale[2] = 1;
    vertices = count < 0 ? -count : count;
    header->numposes = 2;
    header->poseverts = vertices;
    header->numtris = vertices - 2;
    header->numframes = header->numskins = 1;
    header->frames[0].firstpose = 0;
    header->frames[0].numposes = 2;
    header->frames[0].interval = .1f;
    header->gl_texturenum[0][0] = 10;
    header->gl_texturenum[0][1] = 11;
    header->gl_texturenum[0][2] = 12;
    header->gl_texturenum[0][3] = 13;
    header->commands = sizeof(*header);
    commands = (int *)((byte *)header + header->commands);
    commands[0] = count;
    for (i = 0; i < vertices; ++i) {
        uv[0] = .1f * (i + 1); uv[1] = .2f * (i + 1);
        memcpy(&commands[1 + 2 * i], uv, sizeof(uv));
    }
    commands[1 + 2 * vertices] = 0;
    header->posedata = header->commands + (2 + 2 * vertices) * sizeof(int);
    poses = (trivertx_t *)((byte *)header + header->posedata);
    for (i = 0; i < 2 * vertices; ++i) poses[i].lightnormalindex = 5;
    strcpy(alias_model.name, "progs/player.mdl");
    alias_model.type = mod_alias;
    alias_model.radius = 32;
    alias_model.cache.data = header;
    cl_entities[1].model = &alias_model;
    cl_entities[1].colormap = player_colormap;
    return header;
}

static void test_alias_pose_transform_translation(void)
{
    aliashdr_t *header;
    trivertx_t *poses;
    entity_t *entity;
    reset_test();
    header = make_alias(3);
    poses = (trivertx_t *)((byte *)header + header->posedata);
    header->scale[0] = 1; header->scale[1] = 2; header->scale[2] = 3;
    header->scale_origin[0] = 5; header->scale_origin[1] = 6; header->scale_origin[2] = 7;
    poses[3].v[0] = 10;
    poses[4].v[0] = 11;
    poses[5].v[0] = 10; poses[5].v[1] = 1;
    entity = &cl_entities[1];
    entity->origin[0] = 100; entity->origin[1] = 200; entity->origin[2] = 300;
    entity->angles[YAW] = 90;
    cl.time = .15;
    scores[0].colors = 0xe1;
    XGL_DrawAlias(entity);
    assert(captured_count == 3 && native_lit == 2 && material_seen == 11);
    assert(fabs(captured[0].position[0] - 94) < .0001);
    assert(fabs(captured[0].position[1] - 215) < .0001);
    assert(fabs(captured[0].position[2] - 307) < .0001);
    assert(fabs(captured[0].color[0] - xgl_palette[239 * 3] / 255.0f) < .0001);
    assert(fabs(captured[0].color[2] - 239.0 / 255) < .0001);
    assert(xgl_player_translation[0][TOP_RANGE + 15] == 224);
    assert(xgl_player_translation[0][BOTTOM_RANGE] == 16);
    assert(xgl_player_translation[0][BOTTOM_RANGE + 15] == 31);
    assert(xgl_player_translation[0][255] == 255);
    assert(entity->angles[PITCH] == 0 && entity->angles[YAW] == 90);
    captured_count = 0;
    xgl_alias_subdivide.value = 2;
    xgl_entity_triangles.value = 0;
    XGL_DrawAlias(entity);
    assert(captured_count == 3); /* exhausted detail budget retains base mesh */
    captured_count = 0;
    xgl_alias_subdivide.value = 1;
    xgl_entity_triangles.value = 2048;
    XGL_DrawAlias(entity);
    assert(captured_count == 12 && xgl_entity_extra_used == 3);
}

static void test_alias_primitive_winding(void)
{
    aliashdr_t *header;
    trivertx_t *poses;
    vec3_t ab, ac, normal;
    int fan, triangle;
    for (fan = 0; fan < 2; ++fan) {
        reset_test();
        header = make_alias(fan ? -4 : 4);
        header->frames[0].numposes = 1;
        poses = (trivertx_t *)((byte *)header + header->posedata);
        poses[1].v[0] = 1;
        poses[2].v[0] = fan ? 1 : 0; poses[2].v[1] = 1;
        poses[3].v[0] = fan ? 0 : 1; poses[3].v[1] = 1;
        XGL_DrawAlias(&cl_entities[1]);
        assert(captured_count == 6);
        for (triangle = 0; triangle < 2; ++triangle) {
            VectorSubtract(captured[triangle * 3 + 1].position, captured[triangle * 3].position, ab);
            VectorSubtract(captured[triangle * 3 + 2].position, captured[triangle * 3].position, ac);
            CrossProduct(ab, ac, normal);
            assert(normal[2] > .99f);
        }
    }
}

static void test_sprite_groups_coverage_axes(void)
{
    msprite_t sprite;
    mspriteframe_t frames[2];
    struct { int numframes; float *intervals; mspriteframe_t *frames[2]; } group;
    float intervals[2];
    entity_t entity;
    model_t model;
    int i, type, cells, coverage, triangles;
    reset_test();
    memset(&sprite, 0, sizeof(sprite));
    memset(frames, 0, sizeof(frames));
    memset(&entity, 0, sizeof(entity));
    memset(&model, 0, sizeof(model));
    for (i = 0; i < 2; ++i) {
        frames[i].width = frames[i].height = 8;
        frames[i].left = frames[i].down = -4;
        frames[i].right = frames[i].up = 4;
        frames[i].gl_texturenum = 20 + i;
        group.frames[i] = &frames[i];
    }
    intervals[0] = .1f; intervals[1] = .3f;
    group.numframes = 2; group.intervals = intervals;
    sprite.numframes = 1;
    sprite.frames[0].type = SPR_GROUP;
    sprite.frames[0].frameptr = (mspriteframe_t *)&group;
    model.cache.data = &sprite; model.type = mod_sprite;
    entity.model = &model; entity.origin[0] = 10;
    cl.time = .2;
    for (type = SPR_VP_PARALLEL_UPRIGHT; type <= SPR_VP_PARALLEL_ORIENTED; ++type) {
        sprite.type = type;
        captured_count = draw_calls = 0;
        XGL_DrawSprite(&entity);
        assert(captured_count == 24 && native_lit == 0 && material_seen == 21);
        assert(draw_calls == 1);
        for (i = 0; i < captured_count; ++i) {
            assert(fabs(captured[i].position[0] - 10) < .0001);
            assert(fabs(captured[i].position[1]) <= 4 && fabs(captured[i].position[2]) <= 4);
        }
    }
    coverage_mode = 1; captured_count = 0;
    XGL_DrawSprite(&entity);
    assert(captured_count == 0);
    coverage_mode = 2; captured_count = 0;
    XGL_DrawSprite(&entity);
    assert(captured_count == 12);
    for (cells = 8; cells <= 16; cells += 8) {
        xgl_sprite_cells.value = (float)cells;
        triangles = cells * cells * 2;
        for (coverage = 0; coverage <= 2; ++coverage) {
            coverage_mode = coverage;
            captured_count = draw_calls = sample_calls = 0;
            XGL_DrawSprite(&entity);
            if (coverage == 1) {
                assert(!captured_count && !draw_calls);
                assert(sample_calls == triangles);
            } else {
                assert(captured_count == triangles * 3 / (coverage ? 2 : 1));
                assert(draw_calls == (captured_count + XGL_ENTITY_BATCH_VERTICES - 1) /
                                     XGL_ENTITY_BATCH_VERTICES);
                assert(sample_calls == triangles + captured_count);
            }
        }
    }
}

static void test_view_weapon_scope(void)
{
    reset_test();
    make_alias(3);
    cl.viewent.model = &alias_model;
    cl.stats[STAT_HEALTH] = 100;
    XGL_DrawViewModel();
    assert(captured_count == 3 && weapon_scope_count == 2);
    assert(weapon_scope[0] == 1 && weapon_scope[1] == 0);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_SETUP] == 1);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_DRAW] == 1);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_FINISH] == 1);
    assert(!profile_draw_calls[XGL_PROFILE_DRAW_ALIAS] && !profile_draw_active);
    cl.items = IT_INVISIBILITY;
    XGL_DrawViewModel();
    assert(captured_count == 3 && weapon_scope_count == 2);
    cl.items = 0; cl.stats[STAT_HEALTH] = 0;
    XGL_DrawViewModel();
    assert(captured_count == 3 && weapon_scope_count == 2);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_SETUP] == 1);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_DRAW] == 1);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_FINISH] == 1);
    assert(!profile_draw_active);
}

static void test_entity_profile_categories(void)
{
    model_t brush, sprite;
    reset_test();
    make_alias(3);
    memset(&brush, 0, sizeof(brush)); brush.type = mod_brush;
    memset(&sprite, 0, sizeof(sprite)); sprite.type = mod_sprite;
    cl_entities[2].model = &brush;
    cl_entities[3].model = &sprite;
    cl_visedicts[0] = &cl_entities[1];
    cl_visedicts[1] = &cl_entities[2];
    cl_visedicts[2] = &cl_entities[3];
    cl_visedicts[3] = NULL;
    cl_visedicts[4] = &cl_entities[4];
    cl_numvisedicts = 5;
    XGL_DrawEntities();
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS] == 1);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_BRUSH] == 1);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_SPRITE] == 1);
    assert(!profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_DRAW] && !profile_draw_active);
    /* Call counts include dispatches culled or rejected by the renderer. */
    test_cull = 1;
    XGL_DrawEntities();
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS] == 2);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_BRUSH] == 2);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_SPRITE] == 2);
    r_drawentities.value = 0;
    XGL_DrawEntities();
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS] == 2 && !profile_draw_active);
    cl_numvisedicts = 0;
}

static void test_alias_lighting_selection(void)
{
    aliashdr_t *header;
    int mode, pass, lit;
    float expected;
    for (mode = 0; mode < 4; ++mode) {
        reset_test();
        header = make_alias(3);
        header->frames[0].numposes = 1;
        if (mode == 1) strcpy(alias_model.name, "progs/flame.mdl");
        else if (mode == 2) strcpy(alias_model.name, "progs/flame2.mdl");
        else strcpy(alias_model.name, "progs/test.mdl");
        r_fullbright.value = mode == 3;
        test_light = 64;
        lit = mode == 0;
        expected = xgl_palette[0] * (1.0f / 255.0f) * (lit ? .5f : 1);
        for (pass = 0; pass < 2; ++pass) {
            captured_count = 0;
            XGL_DrawAlias(&cl_entities[1]);
            assert(captured_count == 3 && native_lit == (lit ? 2 : 0));
            assert(fabs(captured[0].color[0] - expected) < .0001);
            assert(light_calls == (lit ? pass + 1 : 0));
            assert(profile_sections[XGL_PROFILE_ALIAS_LIGHT] == light_calls);
            assert(!profile_sections[XGL_PROFILE_WEAPON_LIGHT]);
            assert(!profile_draw_depth);
        }
        assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS_FALLBACK] == 1);
        assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS_OUTPUT] == 1);
    }
    /* Ordinary and translated player lighting keep their dark floors. */
    reset_test();
    make_alias(3);
    cl_entities[1].colormap = vid.colormap;
    test_light = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(light_calls == 1 && native_lit == 2);
    assert(fabs(captured[0].color[0] - xgl_palette[TOP_RANGE * 3] * (1.0f / 255.0f) * .04f) < .0001);
    cl_entities[1].colormap = player_colormap;
    captured_count = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(light_calls == 2 && fabs(captured[0].color[0] - xgl_palette[0] * (1.0f / 255.0f) * .08f) < .0001);
}

static void test_nested_alias_profile_paths(void)
{
    aliashdr_t *header;
    int *commands;
    reset_test();
    header = make_alias(3);
    cl_visedicts[0] = &cl_entities[1];
    cl_numvisedicts = 1;
    XGL_DrawEntities();
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS] == 1);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS_FALLBACK] == 1);
    assert(!profile_draw_calls[XGL_PROFILE_DRAW_ALIAS_OUTPUT]);
    captured_count = 0;
    XGL_DrawEntities();
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS] == 2);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS_FALLBACK] == 1);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS_OUTPUT] == 1);
    commands = (int *)((byte *)header + header->commands);
    commands[0] = 4;
    captured_count = 0;
    XGL_DrawEntities();
    assert(!captured_count && !audio_lock_depth);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS] == 3);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS_FALLBACK] == 2);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_ALIAS_OUTPUT] == 1);
    assert(profile_draw_max_depth == 2 && !profile_draw_depth);
    cl_numvisedicts = 0;
}

static void test_nested_weapon_profile_paths(void)
{
    aliashdr_t *header;
    int *commands;
    reset_test();
    header = make_alias(3);
    cl.viewent.model = &alias_model;
    cl.viewent.origin[0] = 7; cl.viewent.origin[1] = 11; cl.viewent.origin[2] = 13;
    cl.stats[STAT_HEALTH] = 100;
    test_light = 0;
    XGL_DrawViewModel();
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_FALLBACK] == 1);
    assert(!profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_OUTPUT]);
    assert(light_calls == 1 && native_lit == 2);
    assert(fabs(captured[0].color[0] - xgl_palette[TOP_RANGE * 3] * (1.0f / 255.0f) * .20f) < .0001);
    assert(light_origin[0] == 7 && light_origin[1] == 11 && light_origin[2] == 13);
    captured_count = 0;
    XGL_DrawViewModel();
    assert(captured_count == 3 && fabs(captured[0].color[0] - xgl_palette[TOP_RANGE * 3] * (1.0f / 255.0f) * .20f) < .0001);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_FALLBACK] == 1);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_OUTPUT] == 1);
    commands = (int *)((byte *)header + header->commands);
    commands[0] = 4;
    captured_count = 0;
    XGL_DrawViewModel();
    assert(!captured_count && !audio_lock_depth);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_FALLBACK] == 2);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_OUTPUT] == 1);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_SETUP] == 3);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_DRAW] == 3);
    assert(profile_draw_calls[XGL_PROFILE_DRAW_WEAPON_FINISH] == 3);
    assert(!profile_draw_calls[XGL_PROFILE_DRAW_ALIAS_OUTPUT]);
    assert(!profile_draw_calls[XGL_PROFILE_DRAW_ALIAS_FALLBACK]);
    assert(profile_sections[XGL_PROFILE_WEAPON_LIGHT] == 3 && light_calls == 3);
    assert(!profile_sections[XGL_PROFILE_ALIAS_LIGHT]);
    assert(profile_draw_max_depth == 2 && !profile_draw_depth);
}

static void test_alias_color_cache(void)
{
    aliashdr_t *header, *moved;
    trivertx_t *poses;
    union { double alignment; byte bytes[32768]; } moved_storage;
    xgl_vertex_t previous;
    float changed_uv;
    int *commands;
    int i, sampled;
    reset_test();
    header = make_alias(3);
    for (i = 0; i < 4; ++i) header->gl_texturenum[0][i] = 10;
    poses = (trivertx_t *)((byte *)header + header->posedata);
    poses[3].v[0] = 10;
    scores[0].colors = 0x21;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 3); /* no centroid test for an opaque alias skin */
    previous = captured[0];
    captured_count = 0; sample_calls = 0;
    cl.time = .15;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 0 && captured_count == 3);
    assert(captured[0].position[0] == previous.position[0] + 10);
    assert(captured[0].color[0] == previous.color[0]);

    /* Quake may move a cached model. No retained header/command pointers
     * may be required to reuse skin colors or animate the new allocation. */
    memcpy(moved_storage.bytes, alias_storage.bytes, sizeof(alias_storage.bytes));
    moved = (aliashdr_t *)moved_storage.bytes;
    alias_model.cache.data = moved;
    captured_count = 0; sample_calls = 0; test_light = 64;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 0 && captured_count == 3);
    assert(fabs(captured[0].color[0] - previous.color[0] * .5f) < .0001);
    assert(captured[0].position[0] == previous.position[0] + 10);

    captured_count = 0; sample_calls = 0;
    moved->gl_texturenum[0][1] = 11;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 3 && material_seen == 11);
    captured_count = 0; sample_calls = 0;
    moved->gl_texturenum[0][1] = 10;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 0 && captured_count == 3);

    captured_count = 0; sample_calls = 0; test_light = 128;
    scores[0].colors = 0x41;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 3);
    assert(fabs(captured[0].color[2] - 64.0 / 255) < .0001);
    captured_count = 0; sample_calls = 0; xgl_playercolors.value = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 3 && fabs(captured[0].color[2] - TOP_RANGE / 255.0f) < .0001);
    captured_count = 0; sample_calls = 0; xgl_playercolors.value = 1;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 0 && fabs(captured[0].color[2] - 64.0 / 255) < .0001);
    captured_count = 0; sample_calls = 0; ++material_revision;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 3);

    /* A recycled model slot can have different UV commands with the same
     * material ID and revision. Owned command bytes distinguish that case. */
    commands = (int *)((byte *)moved + moved->commands);
    changed_uv = .75f;
    memcpy(commands + 1, &changed_uv, sizeof(changed_uv));
    captured_count = 0; sample_calls = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 3 && fabs(captured[0].color[0] - xgl_palette[75 * 3] / 255.0f) < .0001);

    captured_count = 0; sample_calls = 0;
    xgl_alias_subdivide.value = 1;
    XGL_DrawAlias(&cl_entities[1]);
    sampled = sample_calls;
    assert(sampled == 12 && captured_count == 12);
    captured_count = 0; sample_calls = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(!sample_calls && captured_count == 12);
    assert(xgl_alias_cache_bytes > sizeof(xgl_alias_cache));
    assert(xgl_alias_cache_bytes <= XGL_ALIAS_CACHE_BYTES);
    XGL_EntitiesShutdown();
    assert(xgl_alias_cache_bytes == sizeof(xgl_alias_cache));
}

static void test_alias_cache_coverage_and_capacity(void)
{
    aliashdr_t *header, *large;
    xgl_alias_cache_t *cache;
    int *commands;
    int i, sampled, entries;
    size_t old_bytes;
    reset_test();
    header = make_alias(3);
    coverage_mode = 1;
    XGL_DrawAlias(&cl_entities[1]);
    assert(captured_count == 0 && sample_calls == 1);
    sample_calls = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(captured_count == 0 && sample_calls == 0);
    ++material_revision; coverage_mode = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(captured_count == 3 && sample_calls == 3);

    /* Exhaust entry slots using different materials; eviction must remain
     * bounded, and evicted colors must be sampled again when revisited. */
    for (i = 0; i < XGL_ALIAS_CACHE_SLOTS + 2; ++i) {
        captured_count = 0;
        header->gl_texturenum[0][0] = 100 + i;
        XGL_DrawAlias(&cl_entities[1]);
        assert(captured_count == 3 && xgl_alias_cache_bytes <= XGL_ALIAS_CACHE_BYTES);
    }
    sampled = sample_calls;
    captured_count = 0;
    header->gl_texturenum[0][0] = 10;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == sampled + 3 && captured_count == 3);

    /* Large valid models still force byte-budget eviction before slots
     * fill. Compact indices also allow the largest valid model to fit. */
    large = (aliashdr_t *)calloc(1, sizeof(*large) +
                                (2 + (MAXALIASTRIS + 2) * 2) * sizeof(int));
    assert(large);
    large->commands = sizeof(*large);
    large->poseverts = 514; large->numtris = 512;
    commands = (int *)((byte *)large + large->commands);
    commands[0] = large->poseverts;
    XGL_EntitiesShutdown();
    for (i = 0; i < 10; ++i) {
        cache = XGL_AliasColors(large, 200 + i, 2, NULL);
        assert(cache && xgl_alias_cache_bytes <= XGL_ALIAS_CACHE_BYTES);
    }
    entries = 0;
    for (i = 0; i < XGL_ALIAS_CACHE_SLOTS; ++i)
        if (xgl_alias_cache[i].colors) ++entries;
    assert(entries > 0 && entries < 10);
    large->poseverts = MAXALIASTRIS + 2; large->numtris = MAXALIASTRIS;
    commands[0] = large->poseverts;
    cache = XGL_AliasColors(large, 10, 2, NULL);
    assert(cache && xgl_alias_cache_bytes <= XGL_ALIAS_CACHE_BYTES);
    large->numtris = MAXALIASTRIS + 1;
    old_bytes = xgl_alias_cache_bytes;
    assert(!XGL_AliasColors(large, 10, 2, NULL));
    assert(xgl_alias_cache_bytes == old_bytes);
    free(large);

    /* The ordinary path still supplies complete geometry without a usable
     * material revision/cache, and preserves its original sampling rules. */
    material_revision = 0; sample_calls = 0; captured_count = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(sample_calls == 4 && captured_count == 3);
    XGL_EntitiesShutdown();
}

static void test_alias_cache_allocation_failure(void)
{
    int failure;
    for (failure = 0; failure < 2; ++failure) {
        reset_test();
        make_alias(3);
        allocation_fail_after = failure;
        XGL_DrawAlias(&cl_entities[1]);
        assert(captured_count == 3 && sample_calls == 3);
        assert(xgl_alias_cache_bytes == sizeof(xgl_alias_cache));
        allocation_fail_after = -1; captured_count = 0; sample_calls = 0;
        XGL_DrawAlias(&cl_entities[1]);
        assert(captured_count == 3 && sample_calls == 3);
        captured_count = 0; sample_calls = 0;
        XGL_DrawAlias(&cl_entities[1]);
        assert(captured_count == 3 && sample_calls == 0);
        XGL_EntitiesShutdown();
    }
}

static void compare_capture(const xgl_vertex_t *expected, int count)
{
    int i, j;
    assert(captured_count == count);
    for (i = 0; i < count; ++i) {
        for (j = 0; j < 3; ++j) {
            assert(fabs(captured[i].position[j] - expected[i].position[j]) < .0001);
            assert(fabs(captured[i].normal[j] - expected[i].normal[j]) < .0001);
            assert(fabs(captured[i].color[j] - expected[i].color[j]) < .0001);
        }
    }
}

static void test_alias_pose_cache_equivalence(void)
{
    xgl_vertex_t expected[192];
    aliashdr_t *header;
    trivertx_t *pose;
    entity_t *entity;
    unsigned long hits;
    int i, depth, fan, count;
    for (fan = 0; fan < 2; ++fan) {
        for (depth = 0; depth <= 2; ++depth) {
            reset_test();
            header = make_alias(fan ? -6 : 6);
            header->frames[0].numposes = 1;
            header->scale[0] = .3f; header->scale[1] = 2; header->scale[2] = -.75f;
            header->scale_origin[0] = 5; header->scale_origin[1] = -7;
            pose = (trivertx_t *)((byte *)header + header->posedata);
            for (i = 0; i < header->poseverts; ++i) {
                pose[i].v[0] = (byte)(i * 4 + 3);
                pose[i].v[1] = (byte)(i * 2 + 1);
                pose[i].v[2] = (byte)(i * 3 + 1);
                pose[i].lightnormalindex = (byte)(i == 5 ? 255 : i * 31);
            }
            entity = &cl_entities[1];
            entity->angles[0] = 23; entity->angles[1] = 19; entity->angles[2] = 137;
            entity->origin[0] = 31; entity->origin[1] = -17; entity->origin[2] = 64;
            xgl_alias_subdivide.value = (float)depth;
            XGL_DrawAlias(entity); /* cold sampled-color fallback */
            count = captured_count;
            assert(count == 12 * (1 << (2 * depth)));
            memcpy(expected, captured, (size_t)count * sizeof(*expected));
            captured_count = 0; xgl_entity_extra_used = 0;
            XGL_DrawAlias(entity); /* build normal/topology cache */
            compare_capture(expected, count);
            captured_count = 0; normalizations = 0; xgl_entity_extra_used = 0;
            XGL_DrawAlias(entity);
            compare_capture(expected, count);
            assert(!normalizations && profile_counts[XGL_PROFILE_ALIAS_POSE_HITS] == 1);

            /* Reuse across live position bytes, origins, rotations and model
             * offsets. Neither the key nor cached geometry owns positions. */
            hits = profile_counts[XGL_PROFILE_ALIAS_POSE_HITS];
            pose[0].v[0] += 17;
            header->scale_origin[2] += 9;
            entity->origin[1] += 33; entity->angles[1] += 81;
            captured_count = 0; normalizations = 0; xgl_entity_extra_used = 0;
            test_light = 73;
            XGL_DrawAlias(entity);
            assert(!normalizations && profile_counts[XGL_PROFILE_ALIAS_POSE_HITS] == hits + 1);
            memcpy(expected, captured, (size_t)count * sizeof(*expected));
            captured_count = 0; xgl_entity_extra_used = 0; material_revision = 0;
            XGL_DrawAlias(entity); /* independent recursive reference */
            compare_capture(expected, count);
            material_revision = 1;

            /* Scale and changed normal indices must invalidate the entry;
             * zero scale keeps the fallback's unscaled-normal convention. */
            header->scale[0] = 0; pose[1].lightnormalindex = 4;
            captured_count = 0; normalizations = 0; xgl_entity_extra_used = 0;
            XGL_DrawAlias(entity);
            assert(normalizations > 0);
            memcpy(expected, captured, (size_t)count * sizeof(*expected));
            captured_count = 0; xgl_entity_extra_used = 0; material_revision = 0;
            XGL_DrawAlias(entity);
            compare_capture(expected, count);
            XGL_EntitiesShutdown();
        }
    }
}

static void test_alias_pose_canceling_normals(void)
{
    xgl_vertex_t expected[48];
    aliashdr_t *header;
    trivertx_t *pose;
    int i, opposite;
    reset_test();
    header = make_alias(3);
    header->frames[0].numposes = 1;
    header->scale[0] = .5f; header->scale[1] = 2; header->scale[2] = -3;
    pose = (trivertx_t *)((byte *)header + header->posedata);
    opposite = -1;
    for (i = 0; i < XGL_VERTEX_NORMALS; ++i) {
        if (r_avertexnormals[i][0] == -r_avertexnormals[0][0] &&
            r_avertexnormals[i][1] == -r_avertexnormals[0][1] &&
            r_avertexnormals[i][2] == -r_avertexnormals[0][2]) opposite = i;
    }
    assert(opposite >= 0);
    pose[0].lightnormalindex = pose[2].lightnormalindex = 0;
    pose[1].lightnormalindex = (byte)opposite;
    cl_entities[1].angles[0] = 71; cl_entities[1].angles[1] = 32;
    xgl_alias_subdivide.value = 2;
    XGL_DrawAlias(&cl_entities[1]);
    assert(captured_count == 48);
    memcpy(expected, captured, sizeof(expected));
    captured_count = 0; xgl_entity_extra_used = 0;
    XGL_DrawAlias(&cl_entities[1]);
    compare_capture(expected, 48);
    captured_count = normalizations = 0; xgl_entity_extra_used = 0;
    XGL_DrawAlias(&cl_entities[1]);
    compare_capture(expected, 48);
    assert(!normalizations);
    XGL_EntitiesShutdown();
}

static void test_alias_pose_cache_batching_and_limits(void)
{
    aliashdr_t *header;
    trivertx_t *pose;
    xgl_alias_pose_cache_t *cached_pose;
    xgl_alias_cache_t *colors;
    int i, entries, calls, count;
    reset_test();
    header = make_alias(320);
    header->frames[0].numposes = 1;
    xgl_alias_subdivide.value = 1;
    XGL_DrawAlias(&cl_entities[1]);
    count = captured_count;
    assert(count == 318 * 12);
    calls = (count + XGL_ENTITY_BATCH_VERTICES - 1) / XGL_ENTITY_BATCH_VERTICES;
    assert(draw_calls == calls && calls == 5);
    assert(normalizations == 318 * 3 + 1);
    captured_count = draw_calls = 0; xgl_entity_extra_used = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(captured_count == count && draw_calls == calls);
    captured_count = draw_calls = normalizations = 0; xgl_entity_extra_used = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(captured_count == count && draw_calls == calls && !normalizations);

    /* Shared topology now lets this model fill all pose slots within the
     * byte budget. Changing position alone deliberately does not add poses. */
    pose = (trivertx_t *)((byte *)header + header->posedata);
    for (i = 0; i < XGL_ALIAS_POSE_SLOTS + 4; ++i) {
        pose[0].lightnormalindex = (byte)i;
        captured_count = 0; xgl_entity_extra_used = 0;
        XGL_DrawAlias(&cl_entities[1]);
        assert(captured_count == count && xgl_alias_pose_bytes <= XGL_ALIAS_POSE_BYTES);
    }
    entries = 0;
    for (i = 0; i < XGL_ALIAS_POSE_SLOTS; ++i)
        if (xgl_alias_poses[i].normals) ++entries;
    assert(entries == XGL_ALIAS_POSE_SLOTS);
    assert(profile_counts[XGL_PROFILE_ALIAS_POSE_SLOT_EVICTIONS] > 0);
    assert(profile_counts[XGL_PROFILE_ALIAS_POSE_BYTE_EVICTIONS] == 0);

    /* A valid larger subdivision exceeds the fixed workspace. Decline it
     * without touching old cache storage; rendering uses its batched fallback. */
    colors = XGL_AliasColors(header, 10, 2, NULL);
    assert(colors);
    cached_pose = XGL_AliasPose(header, pose, colors);
    assert(!cached_pose);
    XGL_EntitiesShutdown();
    assert(xgl_alias_pose_bytes == XGL_ALIAS_FIXED_BYTES);
}

static void test_alias_pose_cache_allocation_failure(void)
{
    int failure;
    for (failure = 0; failure < 5; ++failure) {
        reset_test();
        make_alias(3);
        xgl_alias_subdivide.value = 1;
        XGL_DrawAlias(&cl_entities[1]); /* first finish the independent color cache */
        allocation_fail_after = failure;
        captured_count = normalizations = sample_calls = 0; xgl_entity_extra_used = 0;
        XGL_DrawAlias(&cl_entities[1]);
        assert(captured_count == 12 && sample_calls == 0 && normalizations > 0);
        assert(xgl_alias_pose_bytes >= XGL_ALIAS_FIXED_BYTES);
        assert(xgl_alias_pose_bytes <= XGL_ALIAS_POSE_BYTES);
        if (failure < 3) assert(xgl_alias_pose_bytes == XGL_ALIAS_FIXED_BYTES);
        else assert(xgl_alias_pose_bytes > XGL_ALIAS_FIXED_BYTES);
        allocation_fail_after = -1;
        captured_count = 0; xgl_entity_extra_used = 0;
        XGL_DrawAlias(&cl_entities[1]);
        captured_count = normalizations = 0; xgl_entity_extra_used = 0;
        XGL_DrawAlias(&cl_entities[1]);
        assert(captured_count == 12 && !normalizations);
        XGL_EntitiesShutdown();
    }
}

static void test_alias_shared_topology(void)
{
    aliashdr_t *header;
    trivertx_t *pose;
    xgl_alias_cache_t *colors;
    xgl_alias_pose_cache_t *first, *second;
    xgl_alias_topology_t *topology;
    size_t first_bytes;
    unsigned int second_hash;
    int *commands;
    int i, entries;
    reset_test();
    header = make_alias(6);
    pose = (trivertx_t *)((byte *)header + header->posedata);
    colors = XGL_AliasColors(header, 10, 1, NULL);
    assert(colors);
    first = XGL_AliasPose(header, pose, colors);
    assert(first);
    topology = first->topology;
    first_bytes = xgl_alias_pose_bytes;
    pose[0].lightnormalindex = 31;
    second = XGL_AliasPose(header, pose, colors);
    assert(second && second != first && second->topology == topology);
    assert(xgl_alias_pose_bytes - first_bytes ==
           (size_t)header->poseverts + (size_t)topology->vertex_count * sizeof(vec3_t));
    assert(profile_counts[XGL_PROFILE_ALIAS_TOPOLOGY_MISSES] == 1);
    assert(profile_counts[XGL_PROFILE_ALIAS_TOPOLOGY_HITS] == 1);

    /* Hashes only reject mismatches: equal hashes still require exact data. */
    second_hash = second->normal_hash;
    first->normal_hash = second_hash;
    assert(XGL_AliasPose(header, pose, colors) == second);
    commands = (int *)((byte *)header + header->commands);
    ++commands[1];
    colors = XGL_AliasColors(header, 10, 1, NULL);
    assert(colors);
    topology->command_hash = colors->command_hash;
    second = XGL_AliasPose(header, pose, colors);
    assert(second && second->topology != topology);

    /* Replace topology slots and ensure no surviving pose references an
     * evicted command stream, even when the slot address is reused. */
    for (i = 0; i < XGL_ALIAS_TOPOLOGY_SLOTS + 4; ++i) {
        ++commands[1];
        colors = XGL_AliasColors(header, 10, 1, NULL);
        assert(colors && XGL_AliasPose(header, pose, colors));
        assert(xgl_alias_pose_bytes <= XGL_ALIAS_POSE_BYTES);
    }
    entries = 0;
    for (i = 0; i < XGL_ALIAS_TOPOLOGY_SLOTS; ++i)
        if (xgl_alias_topologies[i].indices) ++entries;
    assert(entries == XGL_ALIAS_TOPOLOGY_SLOTS);
    for (i = 0; i < XGL_ALIAS_POSE_SLOTS; ++i) {
        if (!xgl_alias_poses[i].normals) continue;
        assert(xgl_alias_poses[i].topology->indices);
        assert(xgl_alias_poses[i].topology->parents);
    }
    assert(profile_counts[XGL_PROFILE_ALIAS_POSE_SLOT_EVICTIONS] > 0);
    XGL_EntitiesShutdown();
    assert(xgl_alias_pose_bytes == XGL_ALIAS_FIXED_BYTES);
}

static void test_alias_pose_byte_limit(void)
{
    aliashdr_t *header;
    trivertx_t *pose;
    xgl_alias_cache_t *colors;
    int i, entries;
    reset_test();
    header = make_alias(1000);
    pose = (trivertx_t *)((byte *)header + header->posedata);
    colors = XGL_AliasColors(header, 10, 1, NULL);
    assert(colors);
    for (i = 0; i < XGL_ALIAS_POSE_SLOTS + 4; ++i) {
        pose[0].lightnormalindex = (byte)i;
        assert(XGL_AliasPose(header, pose, colors));
        assert(xgl_alias_pose_bytes <= XGL_ALIAS_POSE_BYTES);
    }
    entries = 0;
    for (i = 0; i < XGL_ALIAS_POSE_SLOTS; ++i)
        if (xgl_alias_poses[i].normals) ++entries;
    assert(entries > 0 && entries < XGL_ALIAS_POSE_SLOTS);
    assert(profile_counts[XGL_PROFILE_ALIAS_POSE_BYTE_EVICTIONS] > 0);
    assert(profile_counts[XGL_PROFILE_ALIAS_POSE_SLOT_EVICTIONS] == 0);
    assert(profile_counts[XGL_PROFILE_ALIAS_TOPOLOGY_MISSES] == 1);
    XGL_EntitiesShutdown();
    assert(xgl_alias_pose_bytes == XGL_ALIAS_FIXED_BYTES);
}

static void test_alias_direct_partial_batches(void)
{
    xgl_vertex_t expected[192];
    aliashdr_t *header;
    int count, i, j, distinct;
    reset_test();
    header = make_alias(6);
    header->frames[0].numposes = 1;
    xgl_alias_subdivide.value = 2;
    coverage_mode = 2;
    XGL_DrawAlias(&cl_entities[1]);
    count = captured_count;
    assert(count > 0 && count < 192);
    memcpy(expected, captured, (size_t)count * sizeof(*expected));
    captured_count = 0; xgl_entity_extra_used = 0;
    reserve_limit = 9;
    XGL_DrawAlias(&cl_entities[1]);
    compare_capture(expected, count);
    assert(reserve_calls == (count + reserve_limit - 1) / reserve_limit);
    distinct = 0;
    for (i = 0; i < count; ++i) {
        for (j = 0; j < i; ++j)
            if (!memcmp(expected[i].color, expected[j].color, sizeof(expected[i].color))) break;
        if (j == i) ++distinct;
    }
    assert(prepare_calls == distinct && distinct < count);
    assert(audio_locks == audio_unlocks && !audio_lock_depth);
    /* Rebuilding material colors must also replace the cached visible count. */
    ++material_revision; coverage_mode = 1;
    captured_count = reserve_calls = 0; xgl_entity_extra_used = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(!captured_count);
    xgl_entity_extra_used = 0; sample_calls = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(!captured_count && !reserve_calls && !sample_calls);
    ++material_revision; coverage_mode = 0;
    xgl_entity_extra_used = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(captured_count == 192);
    captured_count = 0; xgl_entity_extra_used = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(captured_count == 192);
    XGL_EntitiesShutdown();
}

static void test_alias_audio_lock_cleanup(void)
{
    aliashdr_t *header;
    int *commands;
    reset_test();
    header = make_alias(3);
    test_cull = 1;
    XGL_DrawAlias(&cl_entities[1]);
    assert(!audio_lock_depth && !audio_locks && !audio_unlocks);
    test_cull = 0; alias_model.cache.data = NULL;
    XGL_DrawAlias(&cl_entities[1]);
    assert(!audio_lock_depth && audio_locks == 1 && audio_unlocks == 1);
    alias_model.cache.data = header; header->poseverts = 0;
    XGL_DrawAlias(&cl_entities[1]);
    assert(!audio_lock_depth && audio_locks == 2 && audio_unlocks == 2);
    header->poseverts = 3;
    commands = (int *)((byte *)header + header->commands);
    commands[0] = 4;
    XGL_DrawAlias(&cl_entities[1]);
    assert(!audio_lock_depth && audio_locks == 3 && audio_unlocks == 3);
    assert(!captured_count);
    commands[0] = 3;
    XGL_DrawAlias(&cl_entities[1]);
    assert(!audio_lock_depth && audio_locks == 4 && audio_unlocks == 4);
    assert(captured_count == 3);
    XGL_EntitiesShutdown();
}

int main(void)
{
    test_alias_pose_transform_translation();
    test_alias_primitive_winding();
    test_sprite_groups_coverage_axes();
    test_view_weapon_scope();
    test_entity_profile_categories();
    test_alias_lighting_selection();
    test_nested_alias_profile_paths();
    test_nested_weapon_profile_paths();
    test_alias_color_cache();
    test_alias_cache_coverage_and_capacity();
    test_alias_cache_allocation_failure();
    test_alias_shared_topology();
    test_alias_pose_byte_limit();
    test_alias_direct_partial_batches();
    test_alias_audio_lock_cleanup();
    test_alias_pose_cache_equivalence();
    test_alias_pose_canceling_normals();
    test_alias_pose_cache_batching_and_limits();
    test_alias_pose_cache_allocation_failure();
    puts("XGL entity contracts passed (CPU geometry; no native rendering)");
    return 0;
}
