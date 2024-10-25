/*
    wm_eventloop.cpp - main event loop and the virtual keyboard
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "wm.h"
#include "../file/file.h"
#include "../time/time.h"
#include "../xm/xm.h"
#include "../psytexx/win_main.h"
#include "../psytexx/win_psypattern.h"
#include "../psytexx/win_psyplaylist.h"
#include "../psytexx/win_text.h"
#include "../psytexx/win_button.h"

#ifdef PALMOS
    #include <PalmOS.h>
#endif

#ifdef OS_LINUX
    extern void small_pause( int milliseconds );
#endif

int current_frame = 0;
int redraw_frame = 4;

void event_loop( window_manager *wm, int loop )
{
    wm_event current_event;
    mem_palm_normal_mode();
    while( 1 )
    {
	//Process our events:
	if( wm->events_count ) 
	{
	    //process event:
	    mem_copy( &current_event, wm->events[ wm->event_pnt ], sizeof(wm_event) );
	    wm->events_count--;
	    if(wm->events_count) { wm->event_pnt++; wm->event_pnt &= MAX_EVENT; }
	    process_event( &current_event, wm );
	}

	current_frame++;
	if( current_frame == redraw_frame )
	{
	    current_frame = 0;
	    if( wm->eventloop_handler ) wm->eventloop_handler( wm );
	    draw_screen( wm );
	}

	if( device_event_handler( wm ) ) break;

	//WM Timer control:
	if( wm->timer_handler && time_ticks() >= wm->timer ) 
	{
	    void (*temp_handler)( void*, window_manager* );
	    temp_handler = wm->timer_handler;
	    wm->timer_handler = 0;
	    temp_handler( wm->timer_handler_data, wm );
	}

	if( loop == 0 ) break;
    }
    mem_palm_our_mode();
}

void set_eventloop_handler( void (*handler)( window_manager* ), window_manager *wm )
{
    wm->eventloop_handler = handler;
}

void send_event(int win_num, 
                int event, 
		int x, 
		int y, 
		int button, 
		int pressure,
		int mode, 
		window_manager* wm)
{
    int event_number;

    if( wm->events_count + 1 <= MAX_EVENTS ) {
    
	wm->events_count ++;
	event_number = (wm->event_pnt + wm->events_count - 1) & MAX_EVENT;

	wm->events[ event_number ]->mode = mode;
	wm->events[ event_number ]->x = x;
	wm->events[ event_number ]->y = y;
	wm->events[ event_number ]->button = button;
	wm->events[ event_number ]->pressure = pressure;
	wm->events[ event_number ]->event_type = event;
	wm->events[ event_number ]->event_win = win_num;
    }
}

int process_event( wm_event *evt, window_manager* wm )
{
    window *win = wm->windows[ evt->event_win ];
    int a;
    int result;
    
    if(win) 
    {
	if( evt->event_type == EVT_SHOW ) { //EVT_SHOW. Send this event to window and to all childs:
	    win->win_handler(evt, wm); //send event to parent window
	    if(win->childs && evt->mode) { //send event to all childs:
	        for(a=0;a<win->childs;a++) {
		    evt->event_win = win->child[a];
		    process_event(evt, wm);
		}
	    }
	} else {                //OTHER EVENT. Sent it to visible windows only:
	    if(win->visible) { //if parent window is visible:
		result = win->win_handler(evt, wm); //send event to parent window
		if( !result ) //If event not handled
		    if( evt->event_type == EVT_BUTTONDOWN || evt->event_type == EVT_BUTTONUP ) //Is it button event?
			if( (evt->button >> 3) && wm->constant_window )  //Is it keyboard button?
			{
			    //Handle this key by the constant handler :
			    evt->event_win = wm->constant_window;
			    wm->windows[ wm->constant_window ]->win_handler( evt, wm );
			}
		if(win->childs && evt->mode) { //send event to all visible childs :
		    for(a=0;a<win->childs;a++) 
			if(wm->windows[win->child[a]]->visible) { //if child window is visible:
			    evt->event_win = win->child[a];
			    process_event(evt, wm);
			}
		}
	    }
	    else
	    {
		//If window is unvisible:
		if( evt->event_type == EVT_BUTTONDOWN || evt->event_type == EVT_BUTTONUP ) //Is it button event?
		    if( (evt->button >> 3) && wm->constant_window )  //Is it keyboard button?
		    {
		        //Handle this key by the constant handler :
		        evt->event_win = wm->constant_window;
		        wm->windows[ wm->constant_window ]->win_handler( evt, wm );
		    }
	    }
	}
    }

    return 1;
}


//#################################
//## SIMPLE WINDOW HANDLERS:     ##
//#################################

int null_handler( wm_event *evt, window_manager *wm )
{
    return 0;
}

int child_handler( wm_event *evt, window_manager *wm )
{
    window *win;
    int y = 0;
    
    switch( evt->event_type ) 
    {
	case EVT_SHOW: 
	    win = wm->windows[ evt->event_win ];
	    win->visible = 1;
	    send_event( evt->event_win, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	    break;
	    
	case EVT_DRAW: 
	    win = wm->windows[ evt->event_win ];
	    draw_touch_box( win->parent_win, win->x, win->y, win->x_size, win->y_size, evt->event_win, wm );
	    draw_window_box( evt->event_win, wm );
	    break;

	case EVT_REDRAW: 
	    draw_window_box( evt->event_win, wm );
	    break;
    }
    return 0;
}

int desktop_handler( wm_event *evt, window_manager *wm )
{
    window *win;
    int y = 0;
    
    switch( evt->event_type ) 
    {
	case EVT_SHOW: 
	    win = wm->windows[ evt->event_win ];
	    win->visible = 1;
	    send_event( evt->event_win, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	    break;
	    
	case EVT_DRAW: 
	    draw_window_touch_area( evt->event_win, wm );
	    draw_window_box( evt->event_win, wm );
	    break;

	case EVT_REDRAW: 
	    draw_window_box( evt->event_win, wm );
	    break;
    }
    return 0;
}

#ifdef TEXTMODE
    #define DOWN_CHAR    'v'
    #define UP_CHAR      '^'
    #define LEFT_CHAR    '<'
    #define RIGHT_CHAR   '>'
#else
    #define DOWN_CHAR    2
    #define UP_CHAR      1
    #define LEFT_CHAR    3
    #define RIGHT_CHAR   4
#endif

int create_scrollarea( int win_num, window_manager *wm )
{
    int retval;
    retval = create_window( "scrollarea", 
                            0, 0, 0, 0,
			    0, 0,
			    win_num,
			    &scrollarea_handler, wm );
    set_window_string_controls( retval, "0", "0", "100%", "100%", wm );
    return retval;
}

int scrollarea_handler( wm_event *evt, window_manager *wm )
{
    window *win;
    win = wm->windows[ evt->event_win ];
    window *p;
    int ptr;
    int x, y;
    
    switch(evt->event_type) 
    {
	case EVT_AFTERCREATE:
	    win->scroll_flags = SF_STATICWIN;
	    break;

	case EVT_SHOW: 
	    win->visible = 1;
	    break;
	    
	case EVT_REDRAW: 
	case EVT_DRAW:
	    p = wm->windows[ win->parent_win ];
	    if( p->scroll_flags & SF_UP || p->scroll_flags & SF_DOWN )
	    {
		ptr = ( ( win->real_y + win->y_size - 1 ) * wm->screen_x ) + ( win->real_x + win->x_size - 1 );
		if( ptr>=0 && ptr < wm->screen_x*wm->screen_y )
		{ //Draw DOWN
		    wm->t_screen[ ptr ] = (uint16_t)evt->event_win;
		    wm->g_matrix[ ptr ] = wm->colors[0];
		    wm->t_matrix[ ptr ] = DOWN_CHAR;
		    wm->w_matrix[ ptr ] = (uint16_t)evt->event_win;
		    wm->matrix_is_empty = 0;
		}
		ptr -= wm->screen_x;
		if( ptr>=0 && ptr < wm->screen_x*wm->screen_y )
		{ //Draw UP
		    wm->t_screen[ ptr ] = (uint16_t)evt->event_win;
		    wm->g_matrix[ ptr ] = wm->colors[0];
		    wm->t_matrix[ ptr ] = UP_CHAR;
		    wm->w_matrix[ ptr ] = (uint16_t)evt->event_win;
		    wm->matrix_is_empty = 0;
		}
		if( p->scroll_flags & SF_LEFT || p->scroll_flags & SF_RIGHT )
		{ 
		    ptr += wm->screen_x - 1;
		    if( ptr>=0 && ptr < wm->screen_x*wm->screen_y )
		    { //Draw RIGHT
		        wm->t_screen[ ptr ] = (uint16_t)evt->event_win;
		        wm->g_matrix[ ptr ] = wm->colors[0];
		        wm->t_matrix[ ptr ] = RIGHT_CHAR;
		        wm->w_matrix[ ptr ] = (uint16_t)evt->event_win;
		        wm->matrix_is_empty = 0;
		    }
		    ptr --;
		    if( ptr>=0 && ptr < wm->screen_x*wm->screen_y )
		    { //Draw LEFT
		        wm->t_screen[ ptr ] = (uint16_t)evt->event_win;
		        wm->g_matrix[ ptr ] = wm->colors[0];
		        wm->t_matrix[ ptr ] = LEFT_CHAR;
		        wm->w_matrix[ ptr ] = (uint16_t)evt->event_win;
		        wm->matrix_is_empty = 0;
		    }
		}
	    }
	    else
	    {
		if( p->scroll_flags & SF_LEFT || p->scroll_flags & SF_RIGHT )
		{ 
		    ptr = ( ( win->real_y + win->y_size - 1 ) * wm->screen_x ) + ( win->real_x + win->x_size - 1 );
		    if( ptr>=0 && ptr < wm->screen_x*wm->screen_y )
		    { //Draw RIGHT
		        wm->t_screen[ ptr ] = (uint16_t)evt->event_win;
		        wm->g_matrix[ ptr ] = wm->colors[0];
		        wm->t_matrix[ ptr ] = RIGHT_CHAR;
		        wm->w_matrix[ ptr ] = (uint16_t)evt->event_win;
		        wm->matrix_is_empty = 0;
		    }
		    ptr --;
		    if( ptr>=0 && ptr < wm->screen_x*wm->screen_y )
		    { //Draw LEFT
		        wm->t_screen[ ptr ] = (uint16_t)evt->event_win;
		        wm->g_matrix[ ptr ] = wm->colors[0];
		        wm->t_matrix[ ptr ] = LEFT_CHAR;
		        wm->w_matrix[ ptr ] = (uint16_t)evt->event_win;
		        wm->matrix_is_empty = 0;
		    }
		}
	    }
	    break;

	case EVT_BUTTONDOWN:
	    if( evt->button & 1 )
	    {
		p = wm->windows[ win->parent_win ];
		x = evt->x / wm->char_x;
		y = evt->y / wm->char_y;
		if( p->scroll_flags & SF_UP || p->scroll_flags & SF_DOWN )
		{
		    //Scroll DOWN:
		    if( p->scroll_flags & SF_DOWN )
		    if( x == win->x_size - 1 && y == win->y_size - 1 ) p->y_offset -= win->y_size/4;
		    //Scroll UP:
		    if( p->scroll_flags & SF_UP )
		    if( x == win->x_size - 1 && y == win->y_size - 2 ) p->y_offset += win->y_size/4;
		    //Scroll RIGHT:
		    if( p->scroll_flags & SF_RIGHT )
		    if( x == win->x_size - 2 && y == win->y_size - 1 ) p->x_offset -= win->x_size/4;
		    //Scroll LEFT:
		    if( p->scroll_flags & SF_LEFT )
		    if( x == win->x_size - 3 && y == win->y_size - 1 ) p->x_offset += win->x_size/4;		
		}
		else
		{
		    if( p->scroll_flags & SF_RIGHT || p->scroll_flags & SF_LEFT )
		    {
			//Scroll RIGHT:
			if( p->scroll_flags & SF_RIGHT )
			if( x == win->x_size - 1 && y == win->y_size - 1 ) p->x_offset -= win->x_size/4;
			//Scroll LEFT:
			if( p->scroll_flags & SF_LEFT )
			if( x == win->x_size - 2 && y == win->y_size - 1 ) p->x_offset += win->x_size/4;		
		    }
		}
		p->scroll_flags &= 0xF0;
		calculate_real_window_position_with_childs( win->parent_win, wm );
		send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	    }
	    break;
    }
    return 0;
}

int kbd_status = 0;
char kbd_codes1[] = "abcdefghijklm.01234";
char kbd_codes2[] = "nopqrstuvwxyz,56789";
int kbd_view = 0;
int kbd_selected = -1;
int prev_kbd_selected = -1;
int kbd_button;
int kbd_button2;
int kbd_button_rec;
char kbd_notes[ 24 ] = { 'z', 's', 'x', 'd', 'c', 'v', 'g', 'b', 'h', 'n', 'j', 'm',
                         'q', '2', 'w', '3', 'e', 'r', '5', 't', '6', 'y', '7', 'u' };

int kbd_next_button_handler( void *user_data, int button_win, window_manager *wm )
{
    int win_num = (size_t)user_data;
    kbd_view++; kbd_view &= 3;
    send_event( win_num, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    return 0;
}

int kbd_prev_button_handler( void *user_data, int button_win, window_manager *wm )
{
    int win_num = (size_t)user_data;
    kbd_view--; kbd_view &= 3;
    send_event( win_num, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    return 0;
}

int kbd_rec_button_handler( void *user_data, int button_win, window_manager *wm )
{
    int win_num = (size_t)user_data;
    push_button( 0, 0, ' ' << 3, 1023, 0, wm );
    push_button( 0, 0, ' ' << 3, 1023, 1, wm );
    send_event( 0, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    return 0;
}

int keyboard_handler(wm_event *evt, window_manager *wm) //Virtual keyboard
{
    window *win;
    win = wm->windows[ evt->event_win ];

    int key_size = 0;
    int i;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->ghost = 1;

	    //Button "NEXT KEYBOARD VIEW":
    	    kbd_button = create_window( "kbd button NEXT", 0, 0, 1, 1, wm->colors[2], 1, evt->event_win, &button_handler, wm );
	    button_set_name( ">", kbd_button, wm );
	    set_window_string_controls( kbd_button, "100%-2", "0", "100%", "100%", wm );
	    button_set_handler( &kbd_next_button_handler, (void*)(size_t)evt->event_win, kbd_button, wm );

	    //Button "PREV KEYBOARD VIEW":
    	    kbd_button2 = create_window( "kbd button PREV", 0, 0, 1, 1, wm->colors[2], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "<", kbd_button2, wm );
	    set_window_string_controls( kbd_button2, "0", "0", "2", "100%", wm );
	    button_set_handler( &kbd_prev_button_handler, (void*)(size_t)evt->event_win, kbd_button2, wm );

	    //Button "RECORD":
    	    kbd_button_rec = create_window( "kbd button REC", 0, 0, 1, 1, wm->colors[2], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "REC", kbd_button_rec, wm );
	    set_window_string_controls( kbd_button_rec, "100%-6", "0", "100%-2", "100%", wm );
	    button_set_handler( &kbd_rec_button_handler, (void*)(size_t)evt->event_win, kbd_button_rec, wm );
	    break;
	    
	case EVT_SHOW: 
	    kbd_status = 1;
	    win->visible = 1;
	    send_event( evt->event_win, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	    break;
	    
	case EVT_HIDE: 
	    kbd_status = 0;
	    win->visible = 0;
	    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw desktop
	    break;

	case EVT_DRAW: 
	case EVT_REDRAW: 
	    if( evt->event_type == EVT_DRAW )
		draw_window_touch_area( evt->event_win, wm );
	    draw_window_box( evt->event_win, wm );
	    if( kbd_view == 0 )
	    {
		key_size = ( ( win->x_size - 8 ) * wm->char_x ) / 24;
		if( key_size > 16 ) key_size = 16;
		for( i = 0; i < 24; i++ )
		{
		    COLOR col = COLORMASK;
		    switch( i ) 
		    {
			case 1: case 3: case 6: case 8: case 10:
			case 12+1: case 12+3: case 12+6: case 12+8: case 12+10:
			    col = wm->colors[ 8 ];
			    break;
		    }
		    if( kbd_selected == i ) col = blend( col, 0, 60 );
		    pdraw_box( evt->event_win, i * key_size + wm->char_x * 2, 0, key_size-1, win->y_size * wm->char_y, col, wm );
		}
	    }
	    else
	    if( kbd_view == 1 )
	    {
		draw_string( evt->event_win, wm->char_x * 3 + 2, wm->char_y / 2, 0, "Oct:", wm );
		draw_string( evt->event_win, wm->char_x * 8 + 2, wm->char_y / 2, 0, "1 2 3 4 5 6 7 8", wm );
		char oct[ 2 ];
		oct[ 0 ] = '1';
		oct[ 1 ] = 0;
		oct[ 0 ] += (char)xm.octave;
		draw_string( evt->event_win, wm->char_x * 8 + 2 + xm.octave * wm->char_x * 2 + 1, wm->char_y / 2, 0, oct, wm );
	    }
	    else
	    if( kbd_view == 2 )
	    {
		char ss[ 2 ];
		ss[ 0 ] = 0;
		ss[ 1 ] = 0;
		for( i = 0;; i++ )
		{
		    if( kbd_codes1[ i ] == 0 ) break;
		    ss[ 0 ] = kbd_codes1[ i ];
		    if( kbd_selected == kbd_codes1[ i ] )
			pdraw_box( evt->event_win, wm->char_x * 2 + i * wm->char_x, 0, wm->char_x, wm->char_y, wm->colors[ 9 ], wm );
		    draw_string( evt->event_win, wm->char_x * 2 + i * wm->char_x, 0, 0, ss, wm );
		}
		for( i = 0;; i++ )
		{
		    if( kbd_codes2[ i ] == 0 ) break;
		    ss[ 0 ] = kbd_codes2[ i ];
		    if( kbd_selected == kbd_codes2[ i ] )
			pdraw_box( evt->event_win, wm->char_x * 2 + i * wm->char_x, wm->char_y, wm->char_x, wm->char_y, wm->colors[ 9 ], wm );
		    draw_string( evt->event_win, wm->char_x * 2 + i * wm->char_x, wm->char_y, 0, ss, wm );
		}
		//Draw ENTER:

		int px = wm->char_x * 22;
		int py = wm->char_y;

		draw_pixel( evt->event_win, px, py-3, 0, wm );
		draw_pixel( evt->event_win, px, py-2, 0, wm );
		draw_pixel( evt->event_win, px, py-1, 0, wm );
		draw_pixel( evt->event_win, px, py-0, 0, wm );
		draw_pixel( evt->event_win, px, py+1, 0, wm );
		draw_pixel( evt->event_win, px, py+2, 0, wm );
		draw_pixel( evt->event_win, px-1, py+2, 0, wm );
		draw_pixel( evt->event_win, px-2, py+2, 0, wm );
		draw_pixel( evt->event_win, px-3, py+2, 0, wm );
		draw_pixel( evt->event_win, px-4, py+2, 0, wm );

		draw_pixel( evt->event_win, px-5, py+2, 0, wm );
		draw_pixel( evt->event_win, px-5, py+3, 0, wm );
		draw_pixel( evt->event_win, px-5, py+1, 0, wm );
		draw_pixel( evt->event_win, px-6, py+2, 0, wm );
	    }
	    else
	    if( kbd_view == 3 )
	    {
		draw_string( evt->event_win, wm->char_x * 2 + 4, wm->char_y / 2, 0, "X", wm );
	    }
	    break;

	case EVT_BUTTONDOWN:
	case EVT_MOUSEMOVE:
	    if( !( evt->button & 1 ) ) break;
	    if( kbd_view == 0 )
	    {
		key_size = ( ( win->x_size - 8 ) * wm->char_x ) / 24;
		if( key_size > 16 ) key_size = 16;
		kbd_selected = ( evt->x - wm->char_x * 2 ) / key_size;
		if( prev_kbd_selected != kbd_selected )
		{
		    //Unpress previous:
		    if( prev_kbd_selected >= 0 && prev_kbd_selected < 24 )
			push_button( 0, 0, kbd_notes[ prev_kbd_selected ] << 3, 1023, 1, wm );
		    //Press new:
		    if( kbd_selected >= 0 && kbd_selected < 24 )
			push_button( 0, 0, kbd_notes[ kbd_selected ] << 3, 1023, 0, wm );
		}
	    }
	    else
	    if( kbd_view == 1 )
	    {
		if( evt->x >= wm->char_x * 8 + 2 &&
		    evt->y <= wm->char_x * 8 + 2 + wm->char_x * 2 * 8 )
		{
		    int o = ( evt->x - ( wm->char_x * 8 + 2 - wm->char_x / 2 ) ) / ( wm->char_x * 2 );
		    if( o <= 7 ) xm.octave = o;
		}
	    }
	    else
	    if( kbd_view == 2 )
	    {
		if( evt->x >= wm->char_x * 2 )
		{
		    int kx = ( evt->x - wm->char_x * 2 ) / wm->char_x;
		    int ky = evt->y / wm->char_y;
		    if( ky == 0 )
		    {
			if( kx < 19 ) kbd_selected = kbd_codes1[ kx ];
			if( kx == 19 || kx == 20 ) kbd_selected = KEY_ENTER;
		    }
		    if( ky == 1 )
		    {
			if( kx < 19 ) kbd_selected = kbd_codes2[ kx ];
			if( kx == 19 || kx == 20 ) kbd_selected = KEY_ENTER;
		    }
		    if( prev_kbd_selected != kbd_selected )
		    {
			//Unpress previous:
			if( prev_kbd_selected >= 0 )
			    push_button( 0, 0, prev_kbd_selected << 3, 1023, 1, wm );
			//Press new:
			if( kbd_selected >= 0 )
			    push_button( 0, 0, kbd_selected << 3, 1023, 0, wm );
		    }
		}
	    }
	    else
	    if( kbd_view == 3 )
	    {
		if( evt->event_type == EVT_BUTTONDOWN )
		{
		    if( evt->x > wm->char_x * 2 && evt->x <= wm->char_x * 4 )
		    {
			//Close kbd:
			win->ys2 = "100%";
			resize_all_windows( wm );
			send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
			kbd_status = 0;
		    }
		}
	    }
	    prev_kbd_selected = kbd_selected;

	    send_event( evt->event_win, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw keyboard
	    break;

	case EVT_BUTTONUP:
	    if( evt->button & 1 ) 
	    {
		if( kbd_view == 0 )
		{
		    if( kbd_selected >= 0 && kbd_selected < 24 )
			push_button( 0, 0, kbd_notes[ kbd_selected ] << 3, 1023, 1, wm );
		    kbd_selected = -1;
		    prev_kbd_selected = -1;
		}
		else
		if( kbd_view == 1 )
		{
		}
		else
		if( kbd_view == 2 )
		{
		    if( kbd_selected >= 0 )
			push_button( 0, 0, kbd_selected << 3, 1023, 1, wm );
		    kbd_selected = -1;
		    prev_kbd_selected = -1;
		}
	    }

	    send_event( evt->event_win, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw keyboard
	    break;
    }
    return 0;
}

//show: -1 - switch; 0 - hide; 1 - show;
void show_keyboard( int kbd_win, window_manager *wm, int show )
{
    window *win = wm->windows[ kbd_win ];
    if( show == -1 )
    {
	if( kbd_status == 0 )
	    show = 1;
	else
	    show = 0;
    }
    if( show == 1 )
    {
        win->ys2 = "100%-2";
        resize_all_windows( wm );
        send_event( kbd_win, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
        send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    else
    {
        win->ys2 = "100%";
        resize_all_windows( wm );
        send_event( kbd_win, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
        send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
}

//#################################
//#################################
//#################################


//#################################
//## DEVICE DEPENDENT FUNCTIONS: ##
//#################################

// device_start(), device_end() and device_event_handler() :

#ifndef NONPALM
    #include "wm_palmos.h"
#endif

#ifdef OS_WIN
    #include "wm_win.h"
#endif

#ifdef OS_LINUX
    #if defined(OPENGL) || defined(X11)
	#include "wm_x11.h"
    #endif
    #ifdef TEXTMODE
	#include "wm_linux_console.h"
    #endif
#endif

//#################################
//#################################
//#################################
