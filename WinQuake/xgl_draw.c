/*
 * Native text/geometry replacements for Quake's 2D artwork.
 * Copyright (C) 2026. GPL-2.0-or-later, like the surrounding Quake sources.
 *
 * The existing menu, console and status-bar code still chooses positions,
 * assets and state. Picture handles retain the original dimensions, but no
 * picture pixels or character atlas are submitted to XGL.
 */
#include "quakedef.h"
#include "xgl_local.h"

#define XGL_DRAW_PICS 512
#define XGL_DRAW_RUN 256
#define UI_WHITE 15
#define UI_GOLD 111
#define UI_DARK 3
#define UI_EDGE 109

typedef struct {
    qpic_t pic;
    char source[MAX_QPATH];
    char name[MAX_QPATH];
    int wad;
} xgl_draw_pic;

static xgl_draw_pic ui_pics[XGL_DRAW_PICS];
static int ui_num_pics;
static char ui_run[XGL_DRAW_RUN];
static int ui_run_length, ui_run_x, ui_run_y, ui_run_color;
static int ui_disc_active;
byte *draw_chars;
qpic_t *draw_disc;
qpic_t *draw_backtile;

static void ui_flush_text(void)
{
    if (!ui_run_length) return;
    ui_run[ui_run_length] = 0;
    XGL_UIText((float)ui_run_x, (float)ui_run_y, ui_run,
               ui_run_color, 8.0f, 8.0f);
    ui_run_length = 0;
}

static void ui_fill(float x, float y, float w, float h, int color, float opacity)
{
    ui_flush_text();
    if (w > 0 && h > 0)
        XGL_UIFill(x, y, w, h, color & 255, opacity);
}

static void ui_text(float x, float y, const char *text, int color,
                    float cell, float height)
{
    ui_flush_text();
    if (text[0] && cell > 0 && height > 0)
        XGL_UIText(x, y, text, color & 255, cell, height);
}

static void ui_border(float x, float y, float w, float h, int color)
{
    ui_fill(x, y, w, 1, color, 1);
    ui_fill(x, y+h-1, w, 1, color, 1);
    ui_fill(x, y, 1, h, color, 1);
    ui_fill(x+w-1, y, 1, h, color, 1);
}

static void ui_label(float x, float y, float w, float h,
                     const char *text, int color)
{
    float cell, height;
    int length;
    length = (int)strlen(text);
    if (!length || w <= 0 || h <= 0) return;
    height = h > 6 ? h-4 : h;
    if (height > 16) height = 16;
    cell = height * .65f;
    if (cell > 10) cell = 10;
    if (cell * length > w-2) cell = (w-2) / length;
    ui_text(x+(w-cell*length)*.5f, y+(h-height)*.5f, text, color, cell, height);
}

static xgl_draw_pic *ui_find_pic(qpic_t *pic)
{
    int i;
    for (i = 0; i < ui_num_pics; ++i)
        if (&ui_pics[i].pic == pic) return &ui_pics[i];
    return NULL;
}

static xgl_draw_pic *ui_pic_slot(const char *source, int wad)
{
    xgl_draw_pic *asset;
    const char *base, *p;
    size_t length;
    int i;
    for (i = 0; i < ui_num_pics; ++i)
        if (ui_pics[i].wad == wad && !strcmp(source, ui_pics[i].source))
            return &ui_pics[i];
    if (ui_num_pics == XGL_DRAW_PICS)
        Sys_Error("XGL Draw: more than %d picture descriptors", XGL_DRAW_PICS);
    if (strlen(source) >= MAX_QPATH)
        Sys_Error("XGL Draw: picture name too long: %s", source);
    asset = &ui_pics[ui_num_pics++];
    memset(asset, 0, sizeof(*asset));
    strcpy(asset->source, source);
    asset->wad = wad;
    base = source;
    for (p = source; *p; ++p)
        if (*p == '/' || *p == '\\') base = p+1;
    length = strlen(base);
    if (length > 4 && !strcmp(base+length-4, ".lmp")) length -= 4;
    memcpy(asset->name, base, length);
    asset->name[length] = 0;
    return asset;
}

static void ui_pic_dimensions(xgl_draw_pic *asset, int width, int height)
{
    if (width < 1 || height < 1 || width > 8192 || height > 8192)
        Sys_Error("XGL Draw: invalid picture dimensions for %s", asset->source);
    asset->pic.width = width;
    asset->pic.height = height;
}

qpic_t *Draw_PicFromWad(char *name)
{
    xgl_draw_pic *asset;
    qpic_t *original;
    char clean[17];
    W_CleanupName(name, clean);
    clean[16] = 0;
    asset = ui_pic_slot(clean, 1);
    if (!asset->pic.width) {
        original = (qpic_t *)W_GetLumpName(name);
        ui_pic_dimensions(asset, original->width, original->height);
    }
    return &asset->pic;
}

qpic_t *Draw_CachePic(char *path)
{
    xgl_draw_pic *asset;
    int handle, length, header[2];
    asset = ui_pic_slot(path, 0);
    if (asset->pic.width) return &asset->pic;
    length = COM_OpenFile(path, &handle);
    if (length < 8 || handle < 0)
        Sys_Error("XGL Draw: failed to read picture header %s", path);
    if (Sys_FileRead(handle, header, 8) != 8) {
        COM_CloseFile(handle);
        Sys_Error("XGL Draw: short picture header %s", path);
    }
    COM_CloseFile(handle);
    ui_pic_dimensions(asset, LittleLong(header[0]), LittleLong(header[1]));
    if (asset->pic.width * asset->pic.height > length-8)
        Sys_Error("XGL Draw: incomplete picture %s", path);
    return &asset->pic;
}

void Draw_Init(void)
{
    ui_run_length = 0;
    ui_disc_active = 0;
    ui_num_pics = 0;
    /* The lump remains engine-owned. It is never read by our drawing path. */
    draw_chars = (byte *)W_GetLumpName("conchars");
    draw_disc = Draw_PicFromWad("disc");
    draw_backtile = Draw_PicFromWad("backtile");
}

void Draw_Character(int x, int y, int num)
{
    int character, color;
    num &= 255;
    if (y <= -8 || y >= (int)vid.height || x <= -8 || x >= (int)vid.width) return;
    color = (num & 128) ? UI_GOLD : UI_WHITE;
    character = num & 127;
    /* Quake's slider cells are not printable characters. */
    if (num >= 128 && num <= 131) {
        if (num == 131) {
            ui_fill(x+2, y+1, 4, 6, UI_GOLD, 1);
        } else {
            ui_fill(x, y+3, 8, 2, UI_EDGE, 1);
            if (num != 129) ui_fill(x+(num == 128 ? 1 : 6), y+1, 1, 6, UI_EDGE, 1);
        }
        return;
    }
    if (character == 10 || character == 11) {
        ui_fill(x+1, y+(character == 10 ? 1 : 6), 6,
                character == 10 ? 6 : 1, color, 1);
        return;
    }
    if (character == 12 || character == 13 || character == 16 || character == 17) {
        ui_flush_text();
        ui_text((float)x, (float)y, character == 17 ? "<" : ">", color, 8, 8);
        return;
    }
    if (character >= 18 && character <= 27) character = '0' + character-18;
    else if (character == 28) character = '.';
    else if (character == 29) character = '<';
    else if (character == 30) character = '=';
    else if (character == 31) character = '>';
    else if (character < 32 || character == 127) character = '?';
    if (ui_run_length && (ui_run_color != color || ui_run_y != y ||
        x != ui_run_x+8*ui_run_length || ui_run_length == XGL_DRAW_RUN-1))
        ui_flush_text();
    if (!ui_run_length) {
        ui_run_x = x;
        ui_run_y = y;
        ui_run_color = color;
    }
    ui_run[ui_run_length++] = (char)character;
}

void Draw_String(int x, int y, char *str)
{
    while (*str) {
        Draw_Character(x, y, (unsigned char)*str++);
        x += 8;
    }
}

static void ui_player(float x, float y, float w, float h, byte *translation)
{
    int shirt, pants;
    shirt = translation ? translation[TOP_RANGE+8] : TOP_RANGE+8;
    pants = translation ? translation[BOTTOM_RANGE+8] : BOTTOM_RANGE+8;
    ui_fill(x+w*.38f, y+h*.08f, w*.24f, h*.20f, 191, 1);
    ui_fill(x+w*.25f, y+h*.30f, w*.50f, h*.32f, shirt, 1);
    ui_fill(x+w*.13f, y+h*.32f, w*.12f, h*.32f, shirt, 1);
    ui_fill(x+w*.75f, y+h*.32f, w*.12f, h*.32f, shirt, 1);
    ui_fill(x+w*.28f, y+h*.62f, w*.19f, h*.28f, pants, 1);
    ui_fill(x+w*.53f, y+h*.62f, w*.19f, h*.28f, pants, 1);
    ui_fill(x+w*.24f, y+h*.90f, w*.23f, h*.08f, 1, 1);
    ui_fill(x+w*.53f, y+h*.90f, w*.23f, h*.08f, 1, 1);
}

static void ui_menu_rows(float x, float y, float width,
                         const char *const *rows, int count)
{
    int i;
    for (i = 0; i < count; ++i)
        ui_text(x, y+20*i+3, rows[i], UI_GOLD,
                 width/(float)strlen(rows[i]) < 8 ? width/(float)strlen(rows[i]) : 8, 14);
}

static void ui_help(float x, float y, float w, float h, int page)
{
    static const char *const pages[6][7] = {
        {"WELCOME TO QUAKE", "Survive, find the exit, defeat the enemy.",
         "New Game starts a single-player game.", "Choose an episode and skill in the start map.",
         "Escape opens the menu.", "Arrow keys / Enter choose menu items.", "Left / Right: more help   Escape: back"},
        {"MOVEMENT (DEFAULT KEYS)", "Arrow keys: move and turn", "Alt + arrows: strafe",
         "Shift: run", "Space: jump / swim up", "Change bindings in Options / Customize Controls.",
         "Left / Right: more help   Escape: back"},
        {"COMBAT (DEFAULT KEYS)", "Control or mouse button 1: attack", "1 through 8: select weapon",
         "Find ammunition before changing weapons.", "Armor absorbs damage; health keeps you alive.",
         "Back away from your own rockets and grenades.", "Left / Right: more help   Escape: back"},
        {"ITEMS AND STATUS", "HUD: armor, health, ammunition", "Inventory: owned weapons and ammunition",
         "Keys open matching doors.", "Quad damage, protection and suits expire.",
         "Collect the rune at the end of each episode.", "Left / Right: more help   Escape: back"},
        {"SAVING AND CONSOLE", "Save and Load are in the Single Player menu.",
         "The tilde key opens the console.", "Type commands or change settings there.",
         "Use Options to adjust sound and controls.", "Pause suspends a single-player game.",
         "Left / Right: more help   Escape: back"},
        {"MULTIPLAYER", "Join Game connects to a server.", "New Game hosts a game.",
         "Setup changes your name and player colors.", "Tab shows scores; T starts a chat message.",
         "TCP/IP support depends on the configured driver.", "Left / Right: more help   Escape: back"}
    };
    int i;
    float cell;
    if (page < 0 || page > 5) page = 0;
    ui_fill(x, y, w, h, UI_DARK, 1);
    ui_border(x, y, w, h, UI_EDGE);
    ui_label(x+8, y+8, w-16, 24, pages[page][0], UI_GOLD);
    for (i = 1; i < 7; ++i) {
        cell = (w-24)/(float)strlen(pages[page][i]);
        if (cell > 6) cell = 6;
        ui_text(x+12, y+36+(i-1)*22, pages[page][i], UI_WHITE, cell, 10);
    }
}

static void ui_picture(int ix, int iy, qpic_t *pic, byte *translation)
{
    static const char *const main_rows[] = {"SINGLE PLAYER", "MULTIPLAYER", "OPTIONS", "HELP", "QUIT"};
    static const char *const single_rows[] = {"NEW GAME", "LOAD", "SAVE"};
    static const char *const multi_rows[] = {"JOIN GAME", "NEW GAME", "SETUP"};
    static const struct { const char *name, *label; } labels[] = {
        {"ttl_main", "QUAKE"}, {"ttl_sgl", "SINGLE PLAYER"},
        {"p_multi", "MULTIPLAYER"}, {"p_load", "LOAD GAME"}, {"p_save", "SAVE GAME"},
        {"p_option", "OPTIONS"}, {"ttl_cstm", "CUSTOMIZE CONTROLS"},
        {"pause", "PAUSED"}, {"loading", "LOADING..."}, {"ranking", "RANKING"},
        {"complete", "LEVEL COMPLETE"}, {"finale", "THE END"},
        {"netmen1", "MODEM"}, {"netmen2", "DIRECT CONNECT"}, {"netmen3", "IPX"},
        {"netmen4", "TCP/IP"}, {"netmen5", "NETWORK"},
        {"dim_modm", "MODEM (UNAVAILABLE)"}, {"dim_drct", "DIRECT (UNAVAILABLE)"},
        {"dim_ipx", "IPX (UNAVAILABLE)"}, {"dim_tcp", "TCP/IP (UNAVAILABLE)"}
    };
    static const struct { const char *name, *label; } icons[] = {
        {"sb_shells", "S"}, {"sb_nails", "N"}, {"sb_rocket", "R"}, {"sb_cells", "C"},
        {"sb_invis", "INV"}, {"sb_invuln", "GOD"}, {"sb_suit", "SUIT"}, {"sb_quad", "QUAD"},
        {"sb_wsuit", "WET"}, {"sb_eshld", "SHLD"},
        {"r_shield1", "SHLD"}, {"r_agrav1", "GRAV"},
        {"r_ammolava", "L"}, {"r_ammomulti", "M"}, {"r_ammoplasma", "P"},
        {"ram", "MEM"}, {"net", "NET"}, {"turtle", "SLOW"}, {"disc", "IO"}
    };
    xgl_draw_pic *asset;
    const char *name, *weapon, *label;
    char text[MAX_QPATH];
    float x, y, w, h;
    int i, color, count;
    if (!pic) return;
    ui_flush_text();
    x = (float)ix; y = (float)iy; w = (float)pic->width; h = (float)pic->height;
    asset = ui_find_pic(pic);
    name = asset ? asset->name : "picture";
    if (!strcmp(name, "mainmenu")) { ui_menu_rows(x,y,w,main_rows,5); return; }
    if (!strcmp(name, "sp_menu")) { ui_menu_rows(x,y,w,single_rows,3); return; }
    if (!strcmp(name, "mp_menu")) { ui_menu_rows(x,y,w,multi_rows,3); return; }
    if (!strncmp(name, "help", 4)) { ui_help(x,y,w,h,atoi(name+4)); return; }
    if (!strncmp(name, "menudot", 7)) {
        ui_label(x,y,w,h,">", UI_GOLD); return;
    }
    if (!strcmp(name, "qplaque")) { ui_label(x,y,w,h,"Q",UI_GOLD); return; }
    if (!strcmp(name, "menuplyr")) { ui_player(x,y,w,h,translation); return; }
    if (!strncmp(name, "box_", 4)) {
        ui_fill(x,y,w,h,UI_DARK,1);
        if (name[4] == 't') ui_fill(x,y,w,1,UI_EDGE,1);
        if (name[4] == 'b') ui_fill(x,y+h-1,w,1,UI_EDGE,1);
        if (name[5] == 'l') ui_fill(x,y,1,h,UI_EDGE,1);
        if (name[5] == 'r') ui_fill(x+w-1,y,1,h,UI_EDGE,1);
        return;
    }
    if (!strcmp(name,"sbar") || !strcmp(name,"ibar") || !strcmp(name,"scorebar") ||
        !strncmp(name,"r_invbar",8) || !strcmp(name,"r_teambord") ||
        !strcmp(name,"bigbox") || !strcmp(name,"backtile")) {
        ui_fill(x,y,w,h,UI_DARK,1);
        ui_border(x,y,w,h,UI_EDGE);
        return;
    }
    if (!strcmp(name,"inter")) {
        ui_fill(x,y,w,h,UI_DARK,1);
        ui_label(x+8,y+8,136,24,"TIME",UI_GOLD);
        ui_label(x+8,y+48,136,24,"SECRETS",UI_GOLD);
        ui_label(x+8,y+88,136,24,"KILLS",UI_GOLD);
        return;
    }
    if (!strcmp(name,"num_colon") || !strcmp(name,"num_slash")) {
        ui_label(x,y,w,h,!strcmp(name,"num_colon") ? ":" : "/",UI_WHITE);
        return;
    }
    if (!strncmp(name,"num_",4) || !strncmp(name,"anum_",5)) {
        label = name + (name[0] == 'a' ? 5 : 4);
        if (!strcmp(label,"minus")) label = "-";
        ui_label(x,y,w,h,label,name[0] == 'a' ? 251 : UI_WHITE);
        return;
    }
    if (!strncmp(name,"inv",3) || !strncmp(name,"r_lava",6) ||
        !strcmp(name,"r_superlava") || !strcmp(name,"r_gren") ||
        !strcmp(name,"r_multirock") || !strcmp(name,"r_plasma")) {
        weapon = strchr(name,'_');
        weapon = weapon ? weapon+1 : name;
        label = "W";
        if (!strcmp(weapon,"shotgun")) label = "SG";
        else if (!strcmp(weapon,"sshotgun")) label = "SSG";
        else if (!strcmp(weapon,"nailgun")) label = "NG";
        else if (!strcmp(weapon,"snailgun")) label = "SNG";
        else if (!strcmp(weapon,"rlaunch")) label = "GL";
        else if (!strcmp(weapon,"srlaunch")) label = "RL";
        else if (!strcmp(weapon,"lightng")) label = "LG";
        else if (strstr(weapon,"prox")) label = "PROX";
        else if (!strcmp(weapon,"laser")) label = "LAS";
        else if (!strcmp(weapon,"mjolnir")) label = "HAM";
        else if (!strcmp(weapon,"lava")) label = "LNG";
        else if (!strcmp(weapon,"superlava")) label = "LSNG";
        else if (!strcmp(weapon,"gren")) label = "MGL";
        else if (!strcmp(weapon,"multirock")) label = "MRL";
        else if (!strcmp(weapon,"plasma")) label = "PLAS";
        color = !strncmp(name,"inv_",4) ? 8 : UI_GOLD;
        if (!strncmp(name,"inva",4) && name[4] >= '1' && name[4] <= '5')
            color = (name[4] & 1) ? UI_WHITE : UI_GOLD;
        ui_fill(x,y,w,h,UI_DARK,1);
        ui_border(x,y,w,h,color);
        ui_label(x+1,y+1,w-2,h-2,label,color);
        return;
    }
    if (!strncmp(name,"sb_armor",8)) {
        color = name[8] == '1' ? 209 : (name[8] == '2' ? 111 : 251);
        ui_fill(x+w*.2f,y+2,w*.6f,h*.6f,color,1);
        ui_fill(x+w*.3f,y+h*.6f,w*.4f,h*.2f,color,1);
        ui_fill(x+w*.4f,y+h*.8f,w*.2f,h*.1f,color,1);
        return;
    }
    if (!strncmp(name,"sb_key",6)) {
        color = name[6] == '1' ? UI_WHITE : UI_GOLD;
        ui_border(x+1,y+1,w*.65f,h*.4f,color);
        ui_fill(x+w*.35f,y+h*.35f,2,h*.55f,color,1);
        ui_fill(x+w*.35f,y+h*.65f,w*.4f,2,color,1);
        return;
    }
    if (!strncmp(name,"face",4)) {
        color = !strncmp(name,"face_p",6) ? 251 : 191;
        ui_fill(x+2,y+1,w-4,h-2,color,1);
        ui_fill(x+w*.23f,y+h*.3f,3,2,1,1);
        ui_fill(x+w*.65f,y+h*.3f,3,2,1,1);
        ui_fill(x+w*.3f,y+h*.72f,w*.4f,2,1,1);
        if (!strcmp(name,"face_quad")) ui_border(x,y,w,h,219);
        else if (!strcmp(name,"face_inv2")) {
            ui_border(x,y,w,h,UI_WHITE);
            ui_border(x+1,y+1,w-2,h-2,UI_GOLD);
        }
        else if (strstr(name,"invul")) ui_border(x,y,w,h,UI_GOLD);
        else if (strstr(name,"invis")) ui_border(x,y,w,h,UI_WHITE);
        else if (name[4] >= '1' && name[4] <= '5')
            ui_fill(x+1,y+h-2,(w-2)*(6-(name[4]-'0'))/5,1,251,1);
        return;
    }
    if (!strncmp(name,"sb_sigil",8)) {
        text[0] = name[8]; text[1] = 0;
        ui_border(x,y,w,h,UI_GOLD);
        ui_label(x,y,w,h,text,UI_GOLD);
        return;
    }
    for (i = 0; i < (int)(sizeof(labels)/sizeof(labels[0])); ++i) {
        if (!strcmp(name,labels[i].name)) {
            ui_label(x,y,w,h,labels[i].label,!strncmp(name,"dim_",4) ? 8 : UI_GOLD);
            return;
        }
    }
    for (i = 0; i < (int)(sizeof(icons)/sizeof(icons[0])); ++i) {
        if (!strcmp(name,icons[i].name)) {
            ui_fill(x,y,w,h,UI_DARK,1);
            ui_border(x,y,w,h,UI_EDGE);
            ui_label(x,y,w,h,icons[i].label,UI_WHITE);
            return;
        }
    }
    /* Mods can add pictures. Keep an identifiable, dimension-preserving
     * control rather than silently leaving its original UI position empty. */
    count = (int)((w-2)/4);
    if (count < 1) count = 1;
    if (count >= (int)sizeof(text)) count = sizeof(text)-1;
    for (i = 0; i < count && name[i]; ++i) {
        text[i] = name[i] == '_' ? ' ' : name[i];
        if (text[i] >= 'a' && text[i] <= 'z') text[i] -= 'a'-'A';
    }
    text[i] = 0;
    ui_fill(x,y,w,h,UI_DARK,1);
    ui_border(x,y,w,h,UI_EDGE);
    ui_label(x,y,w,h,text,UI_WHITE);
}

void Draw_Pic(int x, int y, qpic_t *pic) { ui_picture(x,y,pic,NULL); }
void Draw_TransPic(int x, int y, qpic_t *pic) { ui_picture(x,y,pic,NULL); }
void Draw_TransPicTranslate(int x, int y, qpic_t *pic, byte *translation)
{
    ui_picture(x,y,pic,translation);
}

void Draw_ConsoleBackground(int lines)
{
    char version[64];
    if (lines <= 0) return;
    if (lines > (int)vid.height) lines = (int)vid.height;
    ui_fill(0,0,(float)vid.width,(float)lines,UI_DARK,1);
    ui_fill(0,(float)lines-2,(float)vid.width,2,UI_EDGE,1);
    sprintf(version,"XGLQuake %4.2f",(double)VERSION);
    if (lines >= 12)
        ui_text((float)vid.width-8-6*(float)strlen(version), (float)lines-12,
                version,UI_GOLD,6,8);
}

void Draw_TileClear(int x, int y, int w, int h)
{
    ui_fill((float)x,(float)y,(float)w,(float)h,UI_DARK,1);
}

void Draw_Fill(int x, int y, int w, int h, int color)
{
    ui_fill((float)x,(float)y,(float)w,(float)h,color,1);
}

void Draw_FadeScreen(void)
{
    ui_fill(0,0,(float)vid.width,(float)vid.height,0,.75f);
    S_ExtraUpdate();
}

void Draw_DebugChar(char num) { Draw_Character(vid.width-8,0,(unsigned char)num); }
void Draw_BeginDisc(void) { ui_disc_active = 1; }
void Draw_EndDisc(void) { ui_disc_active = 0; }

void XGL_DrawFlush(void)
{
    ui_flush_text();
    if (ui_disc_active && draw_disc)
        ui_picture(vid.width-draw_disc->width,0,draw_disc,NULL);
    XGL_UIFlush();
}
