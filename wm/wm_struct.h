#pragma once

#define MAX_WINDOWS 250
#define MAX_EVENTS 64             //must be 2/4/8/16/32...
#define MAX_EVENT (MAX_EVENTS-1)

#include "../core/core.h"
#include "../memory/memory.h"

#ifdef OS_WIN
    #include <windows.h>
#endif

#ifdef OPENGL
    #include <GL/gl.h>
    #include <GL/glu.h>
#ifdef OS_LINUX
    #include <GL/glx.h>
    #include <X11/keysym.h>
    #include <X11/XKBlib.h>
#endif
#endif

#ifdef X11
    #include <X11/Xlib.h>
    #include <X11/Xutil.h>
    #include <X11/keysym.h>
    #include <X11/XKBlib.h>
    #include <stdlib.h>
#endif

#ifdef DIRECTX
    #include "ddraw.h"
#endif

#ifdef OS_LINUX 
#ifdef TEXTMODE
    #include <gpm.h>    //Mouse library for console mode
#endif
    #include <sys/time.h>
#endif

#ifdef COLOR8BITS
    #define COLOR     uint8_t
    #define COLORPTR  uint8_t*
    #define COLORLEN  (int)1
    #define COLORBITS 8
    #define COLORMASK 0xFF
    #define CLEARCOLOR 0
#endif
#ifdef COLOR16BITS
    #define COLOR     uint16_t
    #define COLORPTR  uint16_t*
    #define COLORLEN  (int)2
    #define COLORBITS 16
    #define COLORMASK 0xFFFF
    #define CLEARCOLOR 0
#endif
#ifdef COLOR32BITS
    #define COLOR     uint
    #define COLORPTR  uint*
    #define COLORLEN  (int)4
    #define COLORBITS 32
    #define COLORMASK 0xFFFFFFFF
    #define CLEARCOLOR 0xFF000000
#endif

#ifdef OPENGL
    extern float screen_transparency;
#endif

//Keyboard properties:
#define PROP_KBD_USERFONT ".prop_kbd_userfont"

//Draw char attributes:
#define ATR_NONE    0
#define ATR_BOLD    1
#define ATR_SHADOW  2

enum {
    EVT_NULL = 0,
    EVT_SHOW,
    EVT_HIDE,
    EVT_DRAW,                    //Draw window box in t_screen and g_screen (full draw)
    EVT_REDRAW,                  //Draw window box in g_screen only (redraw)
    EVT_RESIZE,                  //After window resize
    EVT_AFTERCREATE,
    EVT_BEFORECLOSE,
    EVT_BUTTONDOWN,
    EVT_BUTTONUP,
    EVT_MOUSEMOVE,
    EVT_UNFOCUS                  //When user click on other window
};

//event modes:
enum {
    MODE_WINDOW = 0,
    MODE_CHILDS
};

//buttons:
#define BUTTON_LEFT 1
#define BUTTON_MIDDLE 2
#define BUTTON_RIGHT 4
#define BUTTON_KEYBOARD 8

//Keyboard codes:
//There is full ASCII table with 256 key-codes
//Some of them:
#define KEY_SPACE       0x20
#define KEY_ENTER       10
#define KEY_BACKSPACE   127
#define KEY_TAB         9
#define KEY_ESCAPE      27

//Additional keys:
enum {
    KEY_F1 = 256,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_INSERT,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_CAPS,
    KEY_SCROLLUP,
    KEY_SCROLLDOWN,
    KEY_USERFONT
};

//Special key flags:
#define KEY_SHIFT       512
#define KEY_CTRL        1024
#define KEY_ALT         2048

//Scroll flags:
#define SF_LEFT         1
#define SF_RIGHT        2
#define SF_UP           4
#define SF_DOWN         8
#define SF_STATICWIN    16


struct wm_event
{
    int             event_type;                  //event type
    int             event_win;                   //window, event sended to
    int             x;
    int             y;
    int             button;
    int             pressure;                    //Key pressure (0..1023)
    int             mode;                        //0 - send to one window; 1 - send to window and childs
};


struct window
{
    const char      *name;                       //window name
    int             x,y;                         //x/y (in symbols)
    int             x_size,y_size;               //window size (in symbols)
    const char      *xs;			 //control strings ( "1", "2+win.x*4", ... )
    const char      *ys;
    const char      *xs2;
    const char      *ys2;
    int             real_x, real_y;              //real screen x/y (in symbols)
    int             x_offset, y_offset;          //x/y offset of window (it's for scrolling)
    int             scroll_flags;                //scroll flags
    int             color;                       //window color ( 32/16/8 bits )
    int             draw_box;                    //auto draw window box ON/OFF
    int             ghost;                       //allways not focused window (virtual keyboard, for example) ON/OFF
    int             *child;                      //child windows:  child[0], child[1] ... it's resizeble memory block.
    int             childs;                      //number of childs
    int             parent_win;                  //parent window number
    int             visible;                     //visible/unvisible
    int             (*win_handler)(void*,void*); //window handler: (*wm_event, *window_manager)
    void            *data;                       //internal window's data (your own structure)
};


struct window_manager
{
    window          *windows[MAX_WINDOWS];//pointers to windows
    
    wm_event        *events[MAX_EVENTS];  //event stack
    int             event_pnt;            //pointer to next event (if events_count != 0)
    int             events_count;         //number of events to execute
    wm_event        *current_event;       //current event
    int             constant_window;      //constant window, that handle unhandled button-events
    void            (*eventloop_handler)( window_manager* ); //Some user-defined eventloop handler

    uint            timer;                //Activation time (in system ticks)
    void            (*timer_handler)( void*, window_manager* );
    void            *timer_handler_data;
    
    uint8_t         *user_font;           //user font ptr (or NULL)
    int             user_font_x;          //user font char x-size
    int             user_font_y;          //user font char y_size;
    int             user_font_mode;       //user font ON/OFF
    COLOR           colors[16];           //16 colors for WM-elements
    COLOR	    font_shadow_color;
    int             char_x;               //x-size of one symbol
    int             char_y;               //y-size of one symbol
    int             screen_x;             //screen size (x) in symbols
    int             screen_y;             //screen size (y) in symbols
    int             pscreen_x;            //screen size (x) in pixels
    int             pscreen_y;            //screen size (y) in pixels
    int             part_size;            //size of one screen part (in pixels):
                                          //one part is block with height = char_y; width = pscreen_x
    
#ifdef TEXTMODE
    int             *text_screen;         //text screen pointer (each element: char + fcolor + bcolor)
#endif
    COLOR           *g_screen;            //graphics screen pointer (each element is BGR233 / RGB565 / RGB888 )
    unsigned short  *t_screen;            //touch screen pointer    (each element is a window number)
    COLOR           *b_screen;            //background screen (it's NULL by default)
    COLOR           *screen_part;         //active piece of graphics screen: xsize = full; ysize = one symbol size
    uint8_t         *screen_changes;      //parameter for each screen part (each part has height char_y): 
                                          //1 - part has changed pixels
					  //0 - part has no changed pixels
    COLOR           *g_matrix;            //screen_x * screen_y array for matrix drawing (colors)
    uint8_t         *t_matrix;            //screen_x * screen_y array for matrix drawing (text)
    uint8_t         *a_matrix;            //screen_x * screen_y array for matrix drawing (text attributes)
    uint16_t        *w_matrix;            //screen_x * screen_y array for matrix drawing (window number)
    int             matrix_is_empty;

    int             int_to_string_mode;   //0 - dec; 1 - hex;

    int             exit_flag;            //Set it to 1 before exit
    
    int             mouse_win;            //Number of window mouse pressed on.
    char            mouse_button_pressed; //Pressed mouse code (left/right/middle)
    int             current_mouse_x;
    int             current_mouse_y;
    int             ghost_win;            //Number of current ghost window

    int             wm_initialized;       

    uint16_t        buttons_table[256];   //DEVICE DEPENDENT: keyboard table for event-manager
#ifdef DIRECTX
    LPDIRECTDRAWSURFACE lpDDSPrimary;     //DEVICE DEPENDENT: DirectDraw primary surface (for win32)
    LPDIRECTDRAWSURFACE lpDDSBack;        //DEVICE DEPENDENT: DirectDraw back surface (for win32)
#endif
#ifdef GDI
    uint            gdi_bitmap_info[2048];//DEVICE DEPENDENT: GDI bitmap info (for win32)
#endif
#ifdef OS_WIN
    HDC             hdc;                  //DEVICE DEPENDENT: graphics content handler (for win32)
    HINSTANCE       hCurrentInst;
    HINSTANCE       hPreviousInst; 
    LPSTR           lpszCmdLine;
    int             nCmdShow;
#endif
#ifndef NONPALM
    MemHandle       arm_code_handle;      //DEVICE DEPENDENT: ARM render (handle)
    void            *arm_code;            //DEVICE DEPENDENT: ARM render (pointer)
    int             arm_flag;             //DEVICE DEPENDENT: Is it ARM processor? (For old Palms)
    void            *arm_render_data;     //DEVICE DEPENDENT: ARM render data
#endif
#ifdef OS_LINUX
    int             argc;
    char            **argv;
#ifndef TEXTMODE
    Display         *dpy;
    Window          win;
    Visual          *win_visual;
    GC              win_gc;
    XImage          *win_img;
    int 	    win_img_depth;
    char            *win_buffer; 
    int	            win_depth;
#ifdef OPENGL
    GLboolean       doubleBuffer;
#endif //OPENGL
#endif //TEXTMODE
#endif //OS_LINUX
};
