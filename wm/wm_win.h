/*
    wm_win.h - platform-dependent module : Windows
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#pragma once

#include <windows.h>

const char *className = "PsyTexx2";
const char *windowName = "PsyTexx2_win32";
int winX = 0, winY = 0;
int winWidth = 800, winHeight = 600;
HDC hDC;
HGLRC hGLRC;
WNDCLASS wndClass;
HWND hWnd = 0;

int shift_status = 0;
int ctrl_status = 0;
int alt_status = 0;
int resulted_status = 0;
int resulted_key = 0;

#ifdef OPENGL
#include <GL/gl.h>
#include "wm_opengl.h"
#endif

window_manager *current_wm;

//#################################
//## DEVICE DEPENDENT FUNCTIONS: ##
//#################################

void SetupPixelFormat(HDC hDC);
LRESULT APIENTRY WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int Win32CreateWindow(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow);
void CreateButtonsTable( window_manager *wm );
#ifdef OPENGL
void gl_init(void);
void gl_resize(void);
#endif
#ifdef DIRECTX
#include "wm_directx.h"
#endif

void device_start( window_manager *wm )
{
    current_wm = wm;

#ifdef GDI
    pscreen_x_size = winWidth = 800;
    pscreen_y_size = winHeight = 600;
#endif
#ifdef DIRECTX
    if( get_option( OPT_SCREENX ) != -1 ) pscreen_x_size = winWidth = get_option( OPT_SCREENX );
    if( get_option( OPT_SCREENY ) != -1 ) pscreen_y_size = winHeight = get_option( OPT_SCREENY );
#endif
#ifdef OPENGL
    pscreen_x_size = winWidth = 1024;
    pscreen_y_size = winHeight = 512;
#endif
    if( get_option( OPT_SCREENX ) != -1 ) pscreen_x_size = winWidth = get_option( OPT_SCREENX );
    if( get_option( OPT_SCREENY ) != -1 ) pscreen_y_size = winHeight = get_option( OPT_SCREENY );

    CreateButtonsTable( wm );
    Win32CreateWindow( wm->hCurrentInst, wm->hPreviousInst, wm->lpszCmdLine, wm->nCmdShow ); //create main window
}

void device_end( window_manager *wm )
{
#ifdef DIRECTX
    dd_close();
    DestroyWindow( hWnd );
#endif
}

int device_event_handler( window_manager *wm )
{
    MSG msg;
    if( wm->exit_flag ) return 1;
    if( PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) ) {
	if( GetMessage(&msg, NULL, 0, 0) == 0 ) return 1;
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	return 0;
    }
    return 0;
}

void SetupPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),  /* size */
        1,                              /* version */
        PFD_SUPPORT_OPENGL |
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,               /* support double-buffering */
        PFD_TYPE_RGBA,                  /* color type */
        16,                             /* prefered color depth */
        0, 0, 0, 0, 0, 0,               /* color bits (ignored) */
        0,                              /* no alpha buffer */
        0,                              /* alpha bits (ignored) */
        0,                              /* no accumulation buffer */
        0, 0, 0, 0,                     /* accum bits (ignored) */
        16,                             /* depth buffer */
        0,                              /* no stencil buffer */
        0,                              /* no auxiliary buffers */
        PFD_MAIN_PLANE,                 /* main layer */
        0,                              /* reserved */
        0, 0, 0,                        /* no layer, visible, damage masks */
    };
    int pixelFormat;

    pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (pixelFormat == 0) {
        MessageBox(WindowFromDC(hDC), "ChoosePixelFormat failed.", "Error",
                MB_ICONERROR | MB_OK);
        exit(1);
    }

    if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) {
        MessageBox(WindowFromDC(hDC), "SetPixelFormat failed.", "Error",
                MB_ICONERROR | MB_OK);
        exit(1);
    }
}

#ifdef OPENGL
#define GET_WINDOW_COORDINATES \
    /*Real coordinates -> window_manager coordinates*/\
    x = lParam & 0xFFFF;\
    y = lParam>>16;\
    x = ( x << 11 ) / winWidth; x *= current_wm->pscreen_x; x >>= 11;\
    y = ( y << 11 ) / winHeight; y *= current_wm->pscreen_y; y >>= 11;
#else
#define GET_WINDOW_COORDINATES \
    /*Real coordinates -> window_manager coordinates*/\
    x = lParam & 0xFFFF;\
    y = lParam>>16;
#endif

LRESULT APIENTRY
WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    int x, y, d, window_num;
    short d2;
    POINT point;
    window *win;
    int old_screen_x, old_screen_y;

    switch (message)
    {
	case WM_CREATE:
	    return 0;
	    break;

	case WM_DESTROY:
	    /* finish OpenGL rendering */
#ifdef DIRECTX
	    //dd_close();
#endif
	    PostQuitMessage(0);
	    return 0;
	    break;

	case WM_SIZE:
	    /* track window size changes */
	    if (hGLRC)
	    {
		winWidth = (int) LOWORD(lParam);
		winHeight = (int) HIWORD(lParam);
#ifdef OPENGL
		gl_resize();
#endif
		return 0;
	    }
#ifdef GDI
	    if( current_wm->char_x > 0 && current_wm->char_y > 0 )
	    {
		old_screen_x = current_wm->screen_x;
		old_screen_y = current_wm->screen_y;
		current_wm->screen_x = (int) LOWORD(lParam) / current_wm->char_x;
		current_wm->screen_y = (int) HIWORD(lParam) / current_wm->char_y;
		current_wm->pscreen_x = current_wm->screen_x * current_wm->char_x;
		current_wm->pscreen_y = current_wm->screen_y * current_wm->char_y;
		if( old_screen_x != current_wm->screen_x || old_screen_y != current_wm->screen_y )
		{
		    resize_all( 1, current_wm ); //Resize window manager
		    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, current_wm );
		}
	    }
#endif
	    break;

	case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
	    redraw_screen( current_wm );
            EndPaint(hWnd, &ps);
            return 0;
        }
        break;
	case 0x020A: //WM_MOUSEWHEEL
	    GET_WINDOW_COORDINATES;
	    point.x = x;
	    point.y = y;
	    ScreenToClient( hWnd, &point );
	    x = point.x;
	    y = point.y;
	    if( x < 0 ) x = 0;
	    if( y < 0 ) y = 0;
	    if( x < current_wm->pscreen_x && y < current_wm->pscreen_y )
	    {
	    	resulted_key = 0;
	    	d = (unsigned int)wParam >> 16;
	    	d2 = (short)d;
	    	if( d2 < 0 ) resulted_key = KEY_SCROLLDOWN << 3;
	    	if( d2 > 0 ) resulted_key = KEY_SCROLLUP << 3;
	    	window_num = current_wm->t_screen[ ((y/current_wm->char_y) * current_wm->screen_x) + (x/current_wm->char_x) ];
	    	win = current_wm->windows[ window_num ];
	    	send_event( window_num, EVT_BUTTONDOWN, x - win->real_x, y - win->real_y, resulted_key, 1023, MODE_WINDOW, current_wm );
	    }
	    break;
	case WM_LBUTTONDOWN:
	    GET_WINDOW_COORDINATES;
	    push_button( x, y, 1 + (resulted_status<<3), 1023, 0, current_wm ); //send message to the PsyTexx window_manager
	    break;
	case WM_LBUTTONUP:
	    GET_WINDOW_COORDINATES;
	    push_button( x, y, 1 + (resulted_status<<3), 1023, 1, current_wm );
	    break;
	case WM_MBUTTONDOWN:
    	    GET_WINDOW_COORDINATES;
	    push_button( x, y, 2 + (resulted_status<<3), 1023, 0, current_wm );
	    break;
	case WM_MBUTTONUP:
	    GET_WINDOW_COORDINATES;
	    push_button( x, y, 2 + (resulted_status<<3), 1023, 1, current_wm );
	    break;
	case WM_RBUTTONDOWN:
	    GET_WINDOW_COORDINATES;
	    push_button( x, y, 4 + (resulted_status<<3), 1023, 0, current_wm );
	    break;
	case WM_RBUTTONUP:
	    GET_WINDOW_COORDINATES;
	    push_button( x, y, 4 + (resulted_status<<3), 1023, 1, current_wm );
	    break;
	case WM_MOUSEMOVE:
    	    GET_WINDOW_COORDINATES;
	    switch( wParam & ~(MK_SHIFT+MK_CONTROL) ) {
		case MK_LBUTTON:
		    push_button( x, y, 1 + (resulted_status<<3), 1023, 2, current_wm );
		    break;
		case MK_MBUTTON:
		    push_button( x, y, 2 + (resulted_status<<3), 1023, 2, current_wm );
		    break;
		case MK_RBUTTON:
		    push_button( x, y, 4 + (resulted_status<<3), 1023, 2, current_wm );
		    break;
	    }
	    break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
    	    if( (int)wParam == VK_SHIFT )
	    	shift_status = KEY_SHIFT;
	    if( (int)wParam == VK_CONTROL )
		ctrl_status = KEY_CTRL;
	    if( message == WM_SYSKEYDOWN )
	        alt_status = KEY_ALT;
	    resulted_status = shift_status | ctrl_status | alt_status;
	    resulted_key = current_wm->buttons_table[(int)wParam];
	    if( resulted_key )
	    {
	    	resulted_key |= resulted_status;
    		push_button( 0, 0, resulted_key << 3, 1023, 0, current_wm );
	    }
	    break;
	case WM_SYSKEYUP:
	case WM_KEYUP:
	    if( (int)wParam == VK_SHIFT )
	    	shift_status = 0;
	    if( (int)wParam == VK_CONTROL )
		ctrl_status = 0;
	    if( message == WM_SYSKEYUP )
		alt_status = 0;
	    resulted_status = shift_status | ctrl_status | alt_status;
	    resulted_key = current_wm->buttons_table[(int)wParam];
	    if( resulted_key )
	    {
		resulted_key |= resulted_status;
		push_button( 0, 0, resulted_key << 3, 1023, 1, current_wm );
	    }
	    break;
	default:
    	    break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int Win32CreateWindow(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow)
{
    /* register window class */
#ifdef DIRECTX
    wndClass.style = CS_BYTEALIGNCLIENT;
#else
    wndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
#endif
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hCurrentInst;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = className;
    RegisterClass(&wndClass);

    /* create window */
    RECT Rect;
    Rect.top = 0;
    Rect.bottom = winHeight;
    Rect.left = 0;
    Rect.right = winWidth;
#ifdef DIRECTX
    AdjustWindowRect( &Rect, WS_EX_TOPMOST, 0 );
    hWnd = CreateWindowEx( WS_EX_TOPMOST,
        className, windowName,
        WS_POPUP,
        0, 0, Rect.right, Rect.bottom,
        NULL, NULL, hCurrentInst, NULL );
#endif //DIRECTX
#ifdef OPENGL
    AdjustWindowRect( &Rect, WS_CAPTION | WS_THICKFRAME, 0 );
    hWnd = CreateWindow(
        className, windowName,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        ( GetSystemMetrics(SM_CXSCREEN) - (Rect.right - Rect.left) ) / 2,
	( GetSystemMetrics(SM_CYSCREEN) - (Rect.bottom - Rect.top) ) / 2,
	Rect.right - Rect.left, Rect.bottom - Rect.top,
        NULL, NULL, hCurrentInst, NULL);
	gl_resize();
#endif //OPENGL
#ifdef GDI
    AdjustWindowRect( &Rect, WS_CAPTION | WS_THICKFRAME, 0 );
    //WS_OVERLAPPED | WS_SYSMENU,
    hWnd = CreateWindow(
	className, windowName,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        ( GetSystemMetrics(SM_CXSCREEN) - (Rect.right - Rect.left) ) / 2,
	( GetSystemMetrics(SM_CYSCREEN) - (Rect.bottom - Rect.top) ) / 2,
        Rect.right - Rect.left, Rect.bottom - Rect.top,
        NULL, NULL, hCurrentInst, NULL
    );
#endif //GDI

    /* display window */
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    hDC = GetDC(hWnd);
    current_wm->hdc = hDC;
#ifdef OPENGL
    /* initialize OpenGL rendering */
    SetupPixelFormat(hDC);
    hGLRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hGLRC);
    gl_init();
#endif
#ifdef DIRECTX
    dd_init();
#endif

    return 0;
}

void CreateButtonsTable( window_manager *wm )
{
    wm->buttons_table[ 0x30 ] = 0x30; // 0 1 2 3 ...
    wm->buttons_table[ 0x31 ] = 0x31;
    wm->buttons_table[ 0x32 ] = 0x32;
    wm->buttons_table[ 0x33 ] = 0x33;
    wm->buttons_table[ 0x34 ] = 0x34;
    wm->buttons_table[ 0x35 ] = 0x35;
    wm->buttons_table[ 0x36 ] = 0x36;
    wm->buttons_table[ 0x37 ] = 0x37;
    wm->buttons_table[ 0x38 ] = 0x38;
    wm->buttons_table[ 0x39 ] = 0x39;

    wm->buttons_table[ 0x41 ] = 0x61; // a b c d e ...
    wm->buttons_table[ 0x42 ] = 0x62;
    wm->buttons_table[ 0x43 ] = 0x63;
    wm->buttons_table[ 0x44 ] = 0x64;
    wm->buttons_table[ 0x45 ] = 0x65;
    wm->buttons_table[ 0x46 ] = 0x66;
    wm->buttons_table[ 0x47 ] = 0x67;
    wm->buttons_table[ 0x48 ] = 0x68;
    wm->buttons_table[ 0x49 ] = 0x69;
    wm->buttons_table[ 0x4A ] = 0x6A;
    wm->buttons_table[ 0x4B ] = 0x6B;
    wm->buttons_table[ 0x4C ] = 0x6C;
    wm->buttons_table[ 0x4D ] = 0x6D;
    wm->buttons_table[ 0x4E ] = 0x6E;
    wm->buttons_table[ 0x4F ] = 0x6F;
    wm->buttons_table[ 0x50 ] = 0x70;
    wm->buttons_table[ 0x51 ] = 0x71;
    wm->buttons_table[ 0x52 ] = 0x72;
    wm->buttons_table[ 0x53 ] = 0x73;
    wm->buttons_table[ 0x54 ] = 0x74;
    wm->buttons_table[ 0x55 ] = 0x75;
    wm->buttons_table[ 0x56 ] = 0x76;
    wm->buttons_table[ 0x57 ] = 0x77;
    wm->buttons_table[ 0x58 ] = 0x78;
    wm->buttons_table[ 0x59 ] = 0x79;
    wm->buttons_table[ 0x5A ] = 0x7A;

    wm->buttons_table[ VK_F1 ] = KEY_F1;
    wm->buttons_table[ VK_F2 ] = KEY_F2;
    wm->buttons_table[ VK_F3 ] = KEY_F3;
    wm->buttons_table[ VK_F4 ] = KEY_F4;
    wm->buttons_table[ VK_F5 ] = KEY_F5;
    wm->buttons_table[ VK_F6 ] = KEY_F6;
    wm->buttons_table[ VK_F7 ] = KEY_F7;
    wm->buttons_table[ VK_F8 ] = KEY_F8;

    wm->buttons_table[ VK_ESCAPE ] = KEY_ESCAPE;
    wm->buttons_table[ VK_SPACE ] = KEY_SPACE;
    wm->buttons_table[ VK_RETURN ] = KEY_ENTER;
    wm->buttons_table[ VK_BACK ] = KEY_BACKSPACE;
    wm->buttons_table[ VK_TAB ] = KEY_TAB;
    wm->buttons_table[ VK_CAPITAL ] = KEY_CAPS;
    wm->buttons_table[ VK_SHIFT ] = 0;
    wm->buttons_table[ VK_CONTROL ] = 0;
    //wm->buttons_table[ VK_ALT ] = 0;

    wm->buttons_table[ VK_UP ] = KEY_UP;
    wm->buttons_table[ VK_DOWN ] = KEY_DOWN;
    wm->buttons_table[ VK_LEFT ] = KEY_LEFT;
    wm->buttons_table[ VK_RIGHT ] = KEY_RIGHT;

    wm->buttons_table[ VK_INSERT ] = KEY_INSERT;
    wm->buttons_table[ VK_DELETE ] = KEY_DELETE;
    wm->buttons_table[ VK_HOME ] = KEY_HOME;
    wm->buttons_table[ VK_END ] = KEY_END;
    wm->buttons_table[ 33 ] = KEY_PAGEUP;
    wm->buttons_table[ 34 ] = KEY_PAGEDOWN;

    wm->buttons_table[ 189 ] = '-'; //  -
    wm->buttons_table[ 187 ] = '='; //  =
    wm->buttons_table[ 219 ] = '['; //  [
    wm->buttons_table[ 221 ] = ']'; //  ]
    wm->buttons_table[ 186 ] = ';'; //  ;
    wm->buttons_table[ 222 ] = 0x27; //  '
    wm->buttons_table[ 188 ] = ','; //  ,
    wm->buttons_table[ 190 ] = '.'; //  .
    wm->buttons_table[ 191 ] = '/'; //  /
    wm->buttons_table[ 220 ] = 0x5C; //  |
    wm->buttons_table[ 192 ] = '`'; //  `
}

//#################################
//#################################
//#################################
