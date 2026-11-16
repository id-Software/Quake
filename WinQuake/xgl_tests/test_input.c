/* Host-only event/command adapter tests. X11 calls are queued test doubles;
 * this does not connect to a display or validate the native Xlib ABI. */
#include <assert.h>
#include <math.h>
#include "../xgl_vid.c"

client_state_t cl;
keydest_t key_dest;
qboolean keydown[256], noclip_anglehack;
kbutton_t in_mlook, in_strafe;
cvar_t sensitivity, lookstrafe, m_pitch, m_yaw, m_forward, m_side;

static int display_token;
static XEvent queue[128];
static int queue_count, queue_read;
static KeySym symbols[256];
static struct { int key, down; } keys[256];
static int key_count_test, held_forward, held_attack, held_shift;
static int grabs, ungrabs, warps, flushes, grab_result, key_clears;
static int pitch_stops, resize_calls, requested_width, requested_height;
static int backend_resizes, status_redraws, console_resizes;

KeySym XLookupKeysym(XKeyEvent *event, int index)
{
    assert(index==0 && event->keycode<256);
    return symbols[event->keycode];
}

/* Reproduce shifted printable lookup too: using this for physical key IDs
 * must fail the digit-release test, as it did in the original adapter. */
int XLookupString(XKeyEvent *event, char *text, int size, KeySym *symbol,
                   XComposeStatus *compose)
{
    int character;
    (void)compose;
    *symbol=symbols[event->keycode];
    character=(int)*symbol;
    if(character<32 || character>126 || !size) return 0;
    if(event->state & 1U) {
        if(character=='1') character='!';
        else if(character=='[') character='{';
        else if(character>='a' && character<='z') character+='A'-'a';
    }
    text[0]=(char)character;
    return 1;
}

int XPending(Display *display)
{
    assert(display==xgl_display);
    return queue_count-queue_read;
}
int XNextEvent(Display *display, XEvent *event)
{
    assert(display==xgl_display && queue_read<queue_count);
    *event=queue[queue_read++];
    return 0;
}
int XPeekEvent(Display *display, XEvent *event)
{
    assert(display==xgl_display && queue_read<queue_count);
    *event=queue[queue_read];
    return 0;
}
int XGrabPointer(Display *display, Window window, Bool owner,
                   unsigned int mask, int pointer_mode, int keyboard_mode,
                   Window confine, Cursor cursor, Time time)
{
    assert(display==xgl_display && window==xgl_window && confine==xgl_window);
    assert(owner==True && (mask & PointerMotionMask));
    assert(pointer_mode==GrabModeAsync && keyboard_mode==GrabModeAsync);
    (void)cursor; (void)time;
    ++grabs;
    return grab_result;
}
int XUngrabPointer(Display *display, Time time)
{
    assert(display==xgl_display);
    (void)time;
    ++ungrabs;
    return 0;
}
int XWarpPointer(Display *display, Window source, Window destination,
                  int sx, int sy, unsigned int sw, unsigned int sh, int x, int y)
{
    assert(display==xgl_display && source==None && destination==xgl_window);
    assert(x==xgl_width/2 && y==xgl_height/2);
    (void)sx; (void)sy; (void)sw; (void)sh;
    ++warps;
    return 0;
}
int XFlush(Display *display) { assert(display==xgl_display); ++flushes; return 0; }
int XSync(Display *display, Bool discard)
{
    assert(display==xgl_display && !discard);
    return 0;
}
int XResizeWindow(Display *display, Window window, unsigned int width, unsigned int height)
{
    assert(display==xgl_display && window==xgl_window);
    ++resize_calls;
    requested_width=(int)width;
    requested_height=(int)height;
    return 0;
}

void Key_Event(int key, qboolean down)
{
    assert(key>=0 && key<256 && key_count_test<256);
    keys[key_count_test].key=key;
    keys[key_count_test++].down=down;
    keydown[key]=down;
    if(key=='w') held_forward=down;
    if(key==K_MOUSE1) held_attack=down;
    if(key==K_SHIFT) held_shift=down;
}
void Key_ClearStates(void)
{
    /* Match keys.c: clearing this array does not release +commands. */
    memset(keydown,0,sizeof(keydown));
    ++key_clears;
}
void V_StopPitchDrift(void) { ++pitch_stops; }
void Sbar_Changed(void) { ++status_redraws; }
void Con_CheckResize(void) { ++console_resizes; }
void XGLB_Resize(int width, int height)
{
    assert(width==(int)vid.width && height==(int)vid.height);
    ++backend_resizes;
}
void Cbuf_AddText(char *text) { assert(!strcmp(text,"quit\n")); }
void Sys_Quit(void) { assert(!"unexpected quit"); abort(); }

static XEvent *event_slot(int type)
{
    XEvent *event;
    assert(queue_count<128);
    event=&queue[queue_count++];
    memset(event,0,sizeof(*event));
    event->type=type;
    return event;
}
static void key_event(int type, unsigned int code, unsigned int state)
{
    XEvent *event;
    event=event_slot(type);
    event->xkey.keycode=code;
    event->xkey.state=state;
    event->xkey.time=(Time)queue_count;
}
static void button_event(int type, unsigned int button)
{
    event_slot(type)->xbutton.button=button;
}
static void motion_event(int x, int y)
{
    XEvent *event;
    event=event_slot(MotionNotify);
    event->xmotion.x=x;
    event->xmotion.y=y;
}
static int releases(int key)
{
    int i,count;
    count=0;
    for(i=0;i<key_count_test;++i)
        if(keys[i].key==key && !keys[i].down) ++count;
    return count;
}

static void reset_test(void)
{
    memset(&cl,0,sizeof(cl));
    memset(&vid,0,sizeof(vid));
    memset(keydown,0,sizeof(keydown));
    memset(symbols,0,sizeof(symbols));
    memset(keys,0,sizeof(keys));
    xgl_display=(Display *)&display_token;
    xgl_window=1;
    xgl_width=320; xgl_height=240;
    vid.width=vid.conwidth=320;
    vid.height=vid.conheight=240;
    xgl_mouse=xgl_focused=1;
    xgl_grabbed=0;
    xgl_last_x=160; xgl_last_y=120;
    key_dest=key_game;
    in_strafe.state=in_mlook.state=0;
    noclip_anglehack=false;
    sensitivity.value=2;
    m_pitch.value=.5f; m_yaw.value=.25f;
    m_forward.value=3; m_side.value=4;
    lookstrafe.value=xgl_mouse_filter.value=0;
    IN_ClearStates();
    queue_count=queue_read=key_count_test=0;
    held_forward=held_attack=held_shift=0;
    grabs=ungrabs=warps=flushes=key_clears=0;
    pitch_stops=resize_calls=backend_resizes=status_redraws=console_resizes=0;
    requested_width=requested_height=0;
    grab_result=GrabSuccess;
    symbols[10]=XK_Shift_L;
    symbols[20]='1'; symbols[21]='w'; symbols[22]='[';
}

static void test_shift_identity(void)
{
    int code;
    for(code=20;code<=22;code+=2) {
        reset_test();
        key_dest=key_console;
        key_event(KeyPress,10,0);
        key_event(KeyPress,(unsigned int)code,1);
        key_event(KeyRelease,10,1);
        key_event(KeyRelease,(unsigned int)code,0);
        Sys_SendKeyEvents();
        assert(key_count_test==4 && !grabs);
        assert(keys[1].key==(int)symbols[code] && keys[1].down);
        assert(keys[3].key==(int)symbols[code] && !keys[3].down);
        assert(!held_shift && !keydown[symbols[code]]);
    }
}

static void test_focus_and_grabs(void)
{
    reset_test();
    IN_Commands();
    assert(grabs==1 && xgl_grabbed && warps==1);
    key_event(KeyPress,21,0);
    key_event(KeyPress,10,0);
    button_event(ButtonPress,1);
    Sys_SendKeyEvents();
    assert(held_forward && held_attack && held_shift);
    (void)event_slot(FocusOut);
    Sys_SendKeyEvents();
    assert(!held_forward && !held_attack && !held_shift);
    assert(releases('w')==1 && releases(K_MOUSE1)==1 && releases(K_SHIFT)==1);
    assert(key_clears==1 && !xgl_grabbed && ungrabs==1);
    IN_Commands();
    assert(grabs==1);
    reset_test();
    IN_Commands();
    button_event(ButtonPress,1);
    Sys_SendKeyEvents();
    key_dest=key_menu;
    IN_Commands();
    assert(!held_attack && releases(K_MOUSE1)==1 && !xgl_grabbed && ungrabs==1);
    IN_Commands();
    assert(grabs==1);
    key_dest=key_game;
    cl.paused=true;
    IN_Commands();
    assert(!xgl_grabbed && grabs==1);
    cl.paused=false;
    grab_result=1;
    IN_Commands();
    assert(!xgl_grabbed && grabs==2 && warps==1);
    grab_result=GrabSuccess;
    IN_Commands();
    assert(xgl_grabbed && grabs==3 && warps==2);
}

static void test_buttons_and_motion(void)
{
    usercmd_t command;
    reset_test();
    IN_Commands();
    button_event(ButtonPress,4); button_event(ButtonRelease,4);
    button_event(ButtonPress,5); button_event(ButtonRelease,5);
    button_event(ButtonPress,2); button_event(ButtonRelease,2);
    button_event(ButtonPress,3); button_event(ButtonRelease,3);
    Sys_SendKeyEvents();
    assert(key_count_test==8 && keys[0].key==K_MWHEELUP && keys[2].key==K_MWHEELDOWN);
    assert(keys[4].key==K_MOUSE3 && keys[6].key==K_MOUSE2);
    assert(releases(K_MWHEELUP)==1 && releases(K_MWHEELDOWN)==1);
    motion_event(170,115);
    Sys_SendKeyEvents();
    assert(warps==2);
    memset(&command,0,sizeof(command));
    IN_Move(&command);
    assert(cl.viewangles[YAW]==-5 && command.forwardmove==30 && command.sidemove==0);
    motion_event(160,120); /* The warp notification must add no movement. */
    Sys_SendKeyEvents();
    memset(&command,0,sizeof(command));
    IN_Move(&command);
    assert(cl.viewangles[YAW]==-5 && command.forwardmove==0);
    in_mlook.state=1;
    xgl_mouse_x=2; xgl_mouse_y=4;
    IN_Move(&command);
    assert(cl.viewangles[YAW]==-6 && cl.viewangles[PITCH]==4 && pitch_stops==1);
    xgl_mouse_y=1000;
    IN_Move(&command);
    assert(cl.viewangles[PITCH]==80);
    xgl_mouse_y=-1000;
    IN_Move(&command);
    assert(cl.viewangles[PITCH]==-70);
    in_strafe.state=1;
    xgl_mouse_x=3; xgl_mouse_y=2;
    memset(&command,0,sizeof(command));
    IN_Move(&command);
    assert(command.sidemove==24 && command.forwardmove==-12 && cl.viewangles[YAW]==-6);
    noclip_anglehack=true;
    xgl_mouse_y=2;
    memset(&command,0,sizeof(command));
    IN_Move(&command);
    assert(command.upmove==-12 && command.forwardmove==0);
    in_strafe.state=0; lookstrafe.value=1;
    xgl_mouse_x=3; xgl_mouse_y=2;
    memset(&command,0,sizeof(command));
    IN_Move(&command);
    assert(command.sidemove==24 && cl.viewangles[YAW]==-6 && cl.viewangles[PITCH]==-68);
    IN_ClearStates();
    xgl_mouse_filter.value=1; lookstrafe.value=0; in_mlook.state=0;
    xgl_mouse_x=10;
    IN_Move(&command);
    assert(fabs(cl.viewangles[YAW]+8.5f)<.00001);
    IN_Move(&command);
    assert(fabs(cl.viewangles[YAW]+11.0f)<.00001);
}

static void test_resize_limits(void)
{
    XEvent *event;
    reset_test();
    key_dest=key_menu;
    xgl_mouse_x=20;
    event=event_slot(ConfigureNotify);
    event->xconfigure.width=640;
    event->xconfigure.height=480;
    Sys_SendKeyEvents();
    assert(vid.width==640 && vid.height==480 && vid.conwidth==640 && vid.conheight==480);
    assert(backend_resizes==1 && status_redraws==1 && console_resizes==1);
    assert(vid.recalc_refdef && xgl_mouse_x==0 && !resize_calls);
    event=event_slot(ConfigureNotify);
    event->xconfigure.width=1;
    event->xconfigure.height=1;
    Sys_SendKeyEvents();
    assert(resize_calls==1 && requested_width==320 && requested_height==200);
    assert(vid.width==320 && vid.height==200 && backend_resizes==2);
    event=event_slot(ConfigureNotify);
    event->xconfigure.width=320;
    event->xconfigure.height=200;
    Sys_SendKeyEvents();
    assert(resize_calls==1 && backend_resizes==2);
    event=event_slot(ConfigureNotify);
    event->xconfigure.width=5000;
    event->xconfigure.height=9000;
    Sys_SendKeyEvents();
    assert(resize_calls==2 && requested_width==4096 && requested_height==4096);
    assert(vid.width==4096 && vid.height==4096 && backend_resizes==3);
    assert(!grabs);
}

int main(void)
{
    test_shift_identity();
    test_focus_and_grabs();
    test_buttons_and_motion();
    test_resize_limits();
    puts("XGL input adapter tests passed (no X server)");
    return 0;
}
