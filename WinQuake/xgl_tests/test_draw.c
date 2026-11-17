/* Host-only semantic UI contract checks. No XGL font or rasterization model. */
#include <assert.h>
#include <stdarg.h>
#include "../xgl_draw.c"

typedef struct {
    int kind, color;
    float x, y, width, height;
    char text[256];
} test_op;

static test_op ops[1024];
static int num_ops, file_opens, file_reads, file_closes, file_header[2];
static qpic_t wad_picture;
viddef_t vid;
static int identity_long(int value) { return value; }
int (*LittleLong)(int) = identity_long;

void Sys_Error(char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    abort();
}

void S_ExtraUpdate(void) {}

void W_CleanupName(char *in, char *out)
{
    int i;
    memset(out, 0, 16);
    for (i = 0; i < 16 && in[i]; ++i)
        out[i] = in[i] >= 'A' && in[i] <= 'Z' ? in[i]+'a'-'A' : in[i];
}

void *W_GetLumpName(char *name)
{
    wad_picture.width = !strcmp(name,"backtile") ? 64 : 24;
    wad_picture.height = wad_picture.width;
    return &wad_picture;
}

int COM_OpenFile(char *name, int *handle)
{
    ++file_opens;
    *handle = 1;
    if (strstr(name,"menu.lmp")) {
        file_header[0] = 144;
        file_header[1] = !strcmp(name,"gfx/mainmenu.lmp") ? 100 : 60;
    } else if (strstr(name,"menuplyr")) {
        file_header[0] = 64;
        file_header[1] = 64;
    } else {
        file_header[0] = 80;
        file_header[1] = 32;
    }
    return 8+file_header[0]*file_header[1];
}

int Sys_FileRead(int handle, void *data, int count)
{
    assert(handle == 1 && count == 8);
    ++file_reads;
    memcpy(data,file_header,8);
    return 8;
}

void COM_CloseFile(int handle)
{
    assert(handle == 1);
    ++file_closes;
}

static test_op *next_op(int kind)
{
    test_op *op;
    assert(num_ops < (int)(sizeof(ops)/sizeof(ops[0])));
    op = &ops[num_ops++];
    memset(op,0,sizeof(*op));
    op->kind = kind;
    return op;
}

void XGL_UIText(float x, float y, const char *text, int color,
                float cell, float height)
{
    test_op *op;
    op = next_op('T');
    assert(strlen(text) < sizeof(op->text) && cell > 0 && height > 0);
    strcpy(op->text,text);
    op->x=x; op->y=y; op->width=cell; op->height=height; op->color=color;
}

void XGL_UIFill(float x, float y, float width, float height, int color, float opacity)
{
    test_op *op;
    assert(width > 0 && height > 0 && opacity >= 0 && opacity <= 1);
    op = next_op('Q');
    op->x=x; op->y=y; op->width=width; op->height=height; op->color=color;
}

void XGL_UIFlush(void) { (void)next_op('F'); }

static void reset_ops(void)
{
    assert(!ui_run_length);
    num_ops=0;
}

static int has_text(const char *text)
{
    int i;
    for (i=0;i<num_ops;++i)
        if (ops[i].kind=='T' && !strcmp(ops[i].text,text)) return 1;
    return 0;
}

static void test_metadata_and_menus(void)
{
    qpic_t *main_pic, *sp, *mp, *wad;
    int opens;
    main_pic=Draw_CachePic("gfx/mainmenu.lmp");
    assert(main_pic->width==144 && main_pic->height==100);
    opens=file_opens;
    assert(Draw_CachePic("gfx/mainmenu.lmp")==main_pic && file_opens==opens);
    assert(file_opens==file_reads && file_reads==file_closes);
    wad=Draw_PicFromWad("NUM_0");
    assert(wad->width==24 && wad->height==24 && Draw_PicFromWad("num_0")==wad);
    reset_ops();
    Draw_TransPic(72,32,main_pic);
    assert(num_ops==5 && has_text("SINGLE PLAYER") && has_text("MULTIPLAYER"));
    assert(has_text("OPTIONS") && has_text("HELP") && has_text("QUIT"));
    assert(ops[0].x==72 && ops[0].y==35 && ops[4].y==115);
    sp=Draw_CachePic("gfx/sp_menu.lmp");
    reset_ops();
    Draw_Pic(72,32,sp);
    assert(num_ops==3 && has_text("NEW GAME") && has_text("LOAD") && has_text("SAVE"));
    mp=Draw_CachePic("gfx/mp_menu.lmp");
    reset_ops();
    Draw_Pic(72,32,mp);
    assert(num_ops==3 && has_text("JOIN GAME") && has_text("NEW GAME") && has_text("SETUP"));
}

static void test_cells_and_order(void)
{
    int i;
    reset_ops();
    Draw_Character(0,-4,'A');
    Draw_Character(8,-4,'B');
    assert(num_ops==0);
    Draw_Fill(0,10,10,4,7);
    assert(num_ops==2 && ops[0].kind=='T' && !strcmp(ops[0].text,"AB"));
    assert(ops[0].y==-4 && ops[0].width==8 && ops[0].color==UI_WHITE);
    assert(ops[1].kind=='Q');
    Draw_Character(16,16,'C'+128);
    Draw_Character(24,16,'D'+128);
    Draw_Character(32,16,'E');
    assert(ops[2].kind=='T' && !strcmp(ops[2].text,"CD") && ops[2].color==UI_GOLD);
    XGL_DrawFlush();
    assert(num_ops==5 && !strcmp(ops[3].text,"E") && ops[3].x==32 && ops[4].kind=='F');
    reset_ops();
    for(i=0;i<4;++i) Draw_Character(i*8,32,128+i);
    assert(num_ops>0);
    for(i=0;i<num_ops;++i) assert(ops[i].kind=='Q');
    reset_ops();
    Draw_Character(0,-8,'X');
    Draw_Character(320,0,'X');
    assert(!num_ops && !ui_run_length);
}

static void test_pictures_and_translation(void)
{
    byte translation[256];
    qpic_t *pic;
    int i, shirt, pants;
    for(i=0;i<256;++i) translation[i]=(byte)i;
    translation[TOP_RANGE+8]=42;
    translation[BOTTOM_RANGE+8]=73;
    pic=Draw_CachePic("gfx/menuplyr.lmp");
    reset_ops();
    Draw_TransPicTranslate(10,20,pic,translation);
    shirt=pants=0;
    for(i=0;i<num_ops;++i) {
        assert(ops[i].kind=='Q');
        if(ops[i].color==42) shirt=1;
        if(ops[i].color==73) pants=1;
    }
    assert(shirt && pants && pic->width==64 && pic->height==64);
    reset_ops();
    Draw_Pic(0,0,Draw_PicFromWad("num_colon"));
    assert(has_text(":"));
    Draw_Pic(24,0,Draw_PicFromWad("num_slash"));
    assert(has_text("/"));
    Draw_Pic(48,0,Draw_PicFromWad("anum_7"));
    assert(has_text("7"));
    reset_ops();
    Draw_Pic(0,0,Draw_PicFromWad("inv_shotgun"));
    assert(has_text("SG") && ops[num_ops-1].color==8);
    reset_ops();
    Draw_Pic(0,0,Draw_PicFromWad("inv2_shotgun"));
    assert(has_text("SG") && ops[num_ops-1].color==UI_GOLD);
    reset_ops();
    Draw_Pic(0,0,Draw_PicFromWad("inva1_shotgun"));
    assert(has_text("SG") && ops[num_ops-1].color==UI_WHITE);
    reset_ops();
    Draw_Pic(0,0,Draw_PicFromWad("inva2_shotgun"));
    assert(has_text("SG") && ops[num_ops-1].color==UI_GOLD);
    reset_ops();
    Draw_Pic(0,0,Draw_PicFromWad("face_inv2"));
    shirt=pants=0;
    for(i=0;i<num_ops;++i) {
        if(ops[i].color==UI_WHITE) shirt=1;
        if(ops[i].color==UI_GOLD) pants=1;
    }
    assert(shirt && pants);
    pic=Draw_CachePic("gfx/custom_mod.lmp");
    reset_ops();
    Draw_Pic(0,0,pic);
    assert(num_ops>1 && has_text("CUSTOM MOD"));
    reset_ops();
    Draw_BeginDisc();
    assert(num_ops==0);
    XGL_DrawFlush();
    assert(has_text("IO") && ops[num_ops-1].kind=='F');
    Draw_EndDisc();
    reset_ops();
    XGL_DrawFlush();
    assert(num_ops==1 && ops[0].kind=='F');
}

int main(void)
{
    memset(&vid,0,sizeof(vid));
    vid.width=vid.conwidth=320;
    vid.height=vid.conheight=240;
    /* Null pixel buffers intentionally prove these adapters never touch a
     * software framebuffer. Native calls are recorded, not rendered. */
    Draw_Init();
    assert(draw_disc->width==24 && draw_backtile->width==64);
    test_metadata_and_menus();
    test_cells_and_order();
    test_pictures_and_translation();
    puts("XGL UI adapter semantic tests passed (no device rendering)");
    return 0;
}
