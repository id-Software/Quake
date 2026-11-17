/* Console coverage selection; no native device or game data required. */
#include <assert.h>
#include "../xgl_screen.c"

client_state_t cl;
client_static_t cls;
viddef_t vid;
keydest_t key_dest;
qboolean con_forcedup;
double host_frametime;
int con_notifylines;

static model_t world;
static int resize_calls, sbar_calls, console_calls, console_lines, notify_calls;

void Con_CheckResize(void) { ++resize_calls; }
void Sbar_Changed(void) { ++sbar_calls; }
void Con_DrawNotify(void) { ++notify_calls; }
void Con_DrawConsole(int lines, qboolean drawinput)
{
    assert(drawinput);
    ++console_calls;
    console_lines = lines;
}

static void reset_screen(void)
{
    memset(&cl, 0, sizeof(cl));
    memset(&cls, 0, sizeof(cls));
    memset(&vid, 0, sizeof(vid));
    cl.worldmodel = &world;
    cls.signon = SIGNONS;
    vid.width = 640;
    vid.height = 480;
    vid.numpages = 2;
    key_dest = key_game;
    con_forcedup = false;
    host_frametime = .1;
    scr_conspeed.value = 300;
    scr_con_current = scr_conlines = 0;
    scr_drawdialog = scr_drawloading = false;
    clearconsole = clearnotify = con_notifylines = 0;
    resize_calls = sbar_calls = console_calls = notify_calls = 0;
    console_lines = -1;
}

static void test_console_animation(void)
{
    reset_screen();
    key_dest = key_console;
    SCR_SetUpToDrawConsole();
    assert(resize_calls == 1 && sbar_calls == 1 && !con_forcedup);
    assert(scr_conlines == 240 && XGL_ConsoleClipLines() == 30);
    SCR_SetUpToDrawConsole();
    assert(XGL_ConsoleClipLines() == 60);
    host_frametime = 1;
    SCR_SetUpToDrawConsole();
    assert(XGL_ConsoleClipLines() == 240);

    /* Closing console remains opaque after input returns to the game. */
    key_dest = key_game;
    host_frametime = .25;
    SCR_SetUpToDrawConsole();
    assert(scr_conlines == 0 && XGL_ConsoleClipLines() == 165);
    SCR_DrawConsole();
    assert(console_calls == 1 && console_lines == 165 && notify_calls == 0);
    host_frametime = .5;
    SCR_SetUpToDrawConsole();
    assert(XGL_ConsoleClipLines() == 15);
    host_frametime = .1;
    SCR_SetUpToDrawConsole();
    assert(XGL_ConsoleClipLines() == 0);
    SCR_DrawConsole();
    assert(console_calls == 1 && notify_calls == 1);
}

static void test_console_fractional_height(void)
{
    reset_screen();
    key_dest = key_console;
    scr_conspeed.value = 0;
    scr_con_current = 123.75f;
    SCR_SetUpToDrawConsole();
    SCR_DrawConsole();
    assert(XGL_ConsoleClipLines() == 123 && console_lines == 123);

    scr_con_current = .75f;
    SCR_SetUpToDrawConsole();
    SCR_DrawConsole();
    /* Con_DrawConsole returns without drawing when its integer height is 0. */
    assert(XGL_ConsoleClipLines() == 0 && console_lines == 0);
}

static void test_console_ui_branches(void)
{
    int intermission, destination;
    reset_screen();
    scr_con_current = 123.75f;
    for (intermission = 0; intermission <= 3; ++intermission) {
        cl.intermission = intermission;
        for (destination = key_game; destination <= key_menu; ++destination) {
            key_dest = (keydest_t)destination;
            if (key_dest == key_game && (intermission == 1 || intermission == 2))
                assert(XGL_ConsoleClipLines() == 0);
            else
                assert(XGL_ConsoleClipLines() == 123);
        }
    }
    cl.intermission = 0;
    key_dest = key_game;
    scr_drawdialog = true;
    assert(XGL_ConsoleClipLines() == 0);
    scr_drawdialog = false;

    /* Loading setup returns before changing a stale console height. */
    scr_drawloading = true;
    cl.worldmodel = NULL;
    SCR_SetUpToDrawConsole();
    assert(resize_calls == 1 && scr_con_current == 123.75f);
    assert(!con_forcedup && XGL_ConsoleClipLines() == 0);
    scr_drawloading = false;

    /* Menus can add more opaque background; only claim console coverage. */
    cl.worldmodel = &world;
    key_dest = key_menu;
    assert(XGL_ConsoleClipLines() == 123);
    scr_con_current = 0;
    assert(XGL_ConsoleClipLines() == 0);
}

static void test_console_forced_full(void)
{
    reset_screen();
    cl.worldmodel = NULL;
    SCR_SetUpToDrawConsole();
    assert(con_forcedup && scr_conlines == 480);
    assert(XGL_ConsoleClipLines() == 480);
    SCR_DrawConsole();
    assert(console_lines == 480);

    reset_screen();
    cls.signon = SIGNONS - 1;
    SCR_SetUpToDrawConsole();
    assert(con_forcedup && XGL_ConsoleClipLines() == 480);
    cls.signon = SIGNONS;
    SCR_SetUpToDrawConsole();
    assert(!con_forcedup && XGL_ConsoleClipLines() == 450);
}

int main(void)
{
    test_console_animation();
    test_console_fractional_height();
    test_console_ui_branches();
    test_console_forced_full();
    puts("XGL screen: console animation, integer coverage and UI exclusions passed");
    return 0;
}
