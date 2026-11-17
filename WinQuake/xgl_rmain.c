/*
 * Sun XGL renderer entry points. Derived from Quake's GPL renderer.
 * Copyright (C) 1996-1997 Id Software, Inc.
 * GNU GPL version 2 or (at your option) any later version.
 */
#include "quakedef.h"
#include "xgl_audio.h"
#include "xgl_profile.h"

entity_t r_worldentity, *currententity;
qboolean r_cache_thrash;
vec3_t modelorg, r_entorigin;
vec3_t vup, vpn, vright, r_origin;
int r_visframecount, r_framecount;
int c_brush_polys, c_alias_polys;
mplane_t frustum[4];
refdef_t r_refdef;
mleaf_t *r_viewleaf, *r_oldviewleaf;
texture_t *r_notexture_mip;

cvar_t r_norefresh = {"r_norefresh", "0", false, false, 0, NULL};
cvar_t r_drawentities = {"r_drawentities", "1", false, false, 0, NULL};
cvar_t r_drawviewmodel = {"r_drawviewmodel", "1", false, false, 0, NULL};
cvar_t r_speeds = {"r_speeds", "0", false, false, 0, NULL};
cvar_t r_fullbright = {"r_fullbright", "0", false, false, 0, NULL};
cvar_t r_wateralpha = {"r_wateralpha", "1", false, false, 0, NULL};
cvar_t r_dynamic = {"r_dynamic", "1", false, false, 0, NULL};
cvar_t r_novis = {"r_novis", "0", false, false, 0, NULL};
extern cvar_t xgl_subdivide, xgl_detailtris, xgl_detaildistance;
void V_SetContentsColor(int contents);

void R_InitTextures (void)
{
    int x, y, mip, size;
    byte *pixels;
    r_notexture_mip = (texture_t *)Hunk_AllocName(sizeof(texture_t) + 340, "notexture");
    r_notexture_mip->width = r_notexture_mip->height = 16;
    strcpy(r_notexture_mip->name, "xgl_missing");
    size = sizeof(texture_t);
    for (mip = 0; mip < 4; ++mip) {
        r_notexture_mip->offsets[mip] = size;
        pixels = (byte *)r_notexture_mip + size;
        for (y = 0; y < (16 >> mip); ++y)
            for (x = 0; x < (16 >> mip); ++x)
                *pixels++ = ((y < (8 >> mip)) ^ (x < (8 >> mip))) ? 0 : 15;
        size += (16 >> mip) * (16 >> mip);
    }
    r_notexture_mip->gl_texturenum = XGL_LoadTexture("xgl_missing", 16, 16,
        (byte *)r_notexture_mip + r_notexture_mip->offsets[0], false, false);
}

static void XGL_TimeRefresh_f (void)
{
    double start, elapsed;
    float angle;
    int i;
    if (cls.state != ca_connected || !cl.worldmodel) return;
    angle = r_refdef.viewangles[1];
    start = Sys_FloatTime();
    for (i = 0; i < 128; ++i) {
        r_refdef.viewangles[1] = i * (360.0f / 128);
        XGL_BeginFrame();
        R_RenderView();
        XGL_EndFrame();
    }
    elapsed = Sys_FloatTime() - start;
    r_refdef.viewangles[1] = angle;
    if (elapsed > 0) Con_Printf("%f seconds (%f fps)\n", elapsed, 128 / elapsed);
}

void R_Init (void)
{
    XGL_ProfileInit();
    XGL_AudioInit();
    Cvar_RegisterVariable(&r_norefresh);
    Cvar_RegisterVariable(&r_drawentities);
    Cvar_RegisterVariable(&r_drawviewmodel);
    Cvar_RegisterVariable(&r_speeds);
    Cvar_RegisterVariable(&r_fullbright);
    Cvar_RegisterVariable(&r_wateralpha);
    Cvar_RegisterVariable(&r_dynamic);
    Cvar_RegisterVariable(&r_novis);
    Cvar_RegisterVariable(&xgl_subdivide);
    Cvar_RegisterVariable(&xgl_detailtris);
    Cvar_RegisterVariable(&xgl_detaildistance);
    Cmd_AddCommand("timerefresh", XGL_TimeRefresh_f);
    Cmd_AddCommand("pointfile", R_ReadPointFile_f);
    XGL_TextureInit();
    XGL_EntitiesInit();
    R_InitParticles();
    Con_Printf("XGL renderer: sampled material geometry, native lighting and native UI\n");
}

void R_NewMap (void)
{
    int i;
    for (i = 0; i < 256; ++i) d_lightstylevalue[i] = 264;
    memset(&r_worldentity, 0, sizeof(r_worldentity));
    r_worldentity.model = cl.worldmodel;
    currententity = &r_worldentity;
    for (i = 0; i <= cl.worldmodel->numleafs; ++i) cl.worldmodel->leafs[i].efrags = NULL;
    r_viewleaf = r_oldviewleaf = NULL;
    R_ClearParticles();
    XGL_WorldNewMap();
}

qboolean R_CullBox (vec3_t mins, vec3_t maxs)
{
    int i;
    for (i = 0; i < 4; ++i)
        if (BoxOnPlaneSide(mins, maxs, &frustum[i]) == 2) return true;
    return false;
}

static void XGL_SetFrustum (void)
{
    double sine, cosine, angle;
    int i, j, axis;
    /* Each axis uses its own FOV, including non-square and reduced views. */
    for (axis = 0; axis < 2; ++axis) {
        angle = (axis ? r_refdef.fov_y : r_refdef.fov_x) * (M_PI / 360.0);
        sine = sin(angle); cosine = cos(angle);
        for (j = 0; j < 3; ++j) {
            frustum[axis * 2].normal[j] = vpn[j] * sine + (axis ? vup[j] : vright[j]) * cosine;
            frustum[axis * 2 + 1].normal[j] = vpn[j] * sine - (axis ? vup[j] : vright[j]) * cosine;
        }
    }
    for (i = 0; i < 4; ++i) {
        frustum[i].type = PLANE_ANYZ;
        frustum[i].dist = DotProduct(r_origin, frustum[i].normal);
        frustum[i].signbits = 0;
        for (j = 0; j < 3; ++j)
            if (frustum[i].normal[j] < 0) frustum[i].signbits |= 1 << j;
    }
}

void R_RenderView (void)
{
    double start, section_start;
    xgl_profile_draw_scope_t draw_scope;
    if (r_norefresh.value || !cl.worldmodel) return;
    start = r_speeds.value ? Sys_FloatTime() : 0;
    if (cl.maxclients > 1 && r_fullbright.value) Cvar_Set("r_fullbright", "0");
    R_AnimateLight();
    ++r_framecount;
    VectorCopy(r_refdef.vieworg, r_origin);
    VectorCopy(r_origin, modelorg);
    AngleVectors(r_refdef.viewangles, vpn, vright, vup);
    r_oldviewleaf = r_viewleaf;
    r_viewleaf = Mod_PointInLeaf(r_origin, cl.worldmodel);
    V_SetContentsColor(r_viewleaf->contents);
    V_CalcBlend();
    XGL_SetFrustum();
    c_brush_polys = c_alias_polys = 0;
    r_cache_thrash = false;
    XGL_BeginScene();
    section_start = XGL_ProfileNow();
    XGL_DrawWorld();
    XGL_ProfileAdd(XGL_PROFILE_WORLD, section_start);
    /* Like GLQuake, service the original sound mixer during long frames. */
    XGL_AudioUpdate();
    section_start = XGL_ProfileNow();
    XGL_DrawEntities();
    XGL_ProfileAdd(XGL_PROFILE_ENTITIES, section_start);
    XGL_AudioUpdate();
    section_start = XGL_ProfileNow();
    XGL_ProfileDrawBegin(&draw_scope);
    R_DrawParticles();
    XGL_ProfileDrawEnd(&draw_scope, XGL_PROFILE_DRAW_PARTICLES);
    XGL_ProfileAdd(XGL_PROFILE_EFFECTS, section_start);
    XGL_AudioUpdate();
    section_start = XGL_ProfileNow();
    XGL_ProfileDrawBegin(&draw_scope);
    XGL_DrawWater();
    XGL_ProfileDrawEnd(&draw_scope, XGL_PROFILE_DRAW_WATER);
    XGL_ProfileAdd(XGL_PROFILE_EFFECTS, section_start);
    XGL_AudioUpdate();
    section_start = XGL_ProfileNow();
    XGL_DrawViewModel();
    XGL_ProfileAdd(XGL_PROFILE_VIEWMODEL, section_start);
    XGL_AudioUpdate();
    XGL_EndScene();
    if (r_speeds.value)
        Con_Printf("%3.1f ms %4i wpoly %4i epoly\n", (Sys_FloatTime() - start) * 1000,
                   c_brush_polys, c_alias_polys);
}

void D_FlushCaches (void)
{
    XGL_WorldShutdown();
}
