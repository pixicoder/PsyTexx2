/*
    win_dialog.cpp - dialog handler
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_button.h"
#include "win_dialog.h"

int dialog_visible = 0;

int ok_button_handler( void *user_data, int button_win, window_manager *wm )
{
    dialog_data *data = (dialog_data*)user_data;
    send_event( data->this_window, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    if( data->handler )
	data->handler( data->user_data, 1, wm );
    wm->mouse_win = data->old_focus_window;
    return 0;
}

int cancel_button_handler( void *user_data, int button_win, window_manager *wm )
{
    dialog_data *data = (dialog_data*)user_data;
    send_event( data->this_window, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    if( data->handler )
	data->handler( data->user_data, 0, wm );
    wm->mouse_win = data->old_focus_window;
    return 0;
}

void start_dialog( const char *text, const char *ok, const char *cancel, 
		   void (*handler)(void*,int,window_manager*), void *user_data,
                   int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    dialog_data *data = (dialog_data*)win->data;

    button_set_name( ok, data->button_ok, wm );
    button_set_name( cancel, data->button_cancel, wm );
    data->handler = handler;
    data->user_data = user_data;
    
    int len = 0;
    for(;;)
    {
	if( text[len++] == 0 ) break;
    }
    data->title_len = len;
    data->title = text;
    
    send_event( win_num, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
    //Set focus to this window:
    data->old_focus_window = wm->mouse_win;
    wm->mouse_win = win_num;
}

void standard_dialog_handler( void *user_data, int button, window_manager *wm )
{
    int *res = (int*)user_data;
    if( button == 1 )
    {
	//Ok:
	*res = 1;
    }
    else
    {
	//Cancel:
	*res = 0;
    }
}

int start_dialog_blocked( 
    const char *text, const char *ok, const char *cancel, 
    window_manager *wm )
{
    int result = 5;
    window *win = wm->windows[ win_dialog ]; //Our window
    dialog_data *data = (dialog_data*)win->data;

    button_set_name( ok, data->button_ok, wm );
    button_set_name( cancel, data->button_cancel, wm );
    data->handler = &standard_dialog_handler;
    data->user_data = &result;
    
    int len = 0;
    for(;;)
    {
	if( text[len++] == 0 ) break;
    }
    data->title_len = len;
    data->title = text;
    
    send_event( win_dialog, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
    //Set focus to this window:
    data->old_focus_window = wm->mouse_win;
    wm->mouse_win = win_dialog;
    
    while( result == 5 )
    {
	if( wm->events_count )
	{
	    int ev_num;
	    for( ev_num = 0; ev_num < wm->events_count; ev_num++ )
	    {
		wm_event *evt = wm->events[ ( wm->event_pnt + ev_num ) & MAX_EVENT ];
		if( evt )
		    if( evt->event_win )
			if( evt->event_type == EVT_BUTTONDOWN && ( evt->button & 3 ) )
			    if( evt->event_win != win_dialog &&
				evt->event_win != data->button_cancel && 
				evt->event_win != data->button_ok )
			    {
				//Was a click on another window:
				send_event( win_dialog, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
				send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
				//wm->mouse_win = data->old_focus_window;
				result = -1;
				//break;
			    }
	    }
	}
	event_loop( wm, 0 );
    }
    return result;
}

int dialog_handler( wm_event *evt, window_manager *wm )
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    dialog_data *data = (dialog_data*)win->data;
    int handled = 0;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(dialog_data), "dialog data" );
	    //Init data:
	    data = (dialog_data*)win->data;
	    
	    data->this_window = evt->event_win;
	    
	    data->current_button = 0;
	    
	    data->button_ok = create_window( "okbutton",
	                                     win->x_size/2 - 9, win->y_size/2, 
	                                     8, 2, get_color( 255, 128, 0 ), 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_ok, "50%-9", "50%", "50%-1", "50%+2", wm );
	    button_set_name( "OK", data->button_ok, wm );
	    button_set_handler( &ok_button_handler, (void*)data, data->button_ok, wm );
	    
	    data->button_cancel = create_window( "cancelbutton",
	                                         win->x_size/2 + 1, win->y_size/2, 
	                                         8, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_cancel, "50%+1", "50%", "50%+9", "50%+2", wm );
	    button_set_name( "CANCEL", data->button_cancel, wm );
	    button_set_handler( &cancel_button_handler, (void*)data, data->button_cancel, wm );
	    
	    data->handler = 0;
	    break;
	    
	case EVT_BEFORECLOSE:
	    if( win->data ) mem_free( win->data );
	    break;
	case EVT_SHOW: 
	    //Show window:
	    win->visible = 1; //Make it visible
	    dialog_visible = 1;
	    data->current_button = 0;
	    button_set_color( get_color( 255, 128, 0 ), data->button_ok, wm );
	    button_set_color( wm->colors[ 6 ], data->button_cancel, wm );			
	    send_event(evt->event_win, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Send DRAW event to all childs
	    break;
	case EVT_HIDE:
	    win->visible = 0;
	    dialog_visible = 0;
	    break;
	case EVT_DRAW: 
	    //Draw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_touch_area( evt->event_win, wm );
		draw_window_box( evt->event_win, wm ); //draw window box
		draw_string( evt->event_win,
		             ( (win->x_size*wm->char_x) - (data->title_len*wm->char_x) ) >> 1,
			     (win->y_size*wm->char_y) / 2 - wm->char_y*2,
			     0, data->title, wm );
		draw_rectangle( evt->event_win, 0, 0, win->x_size * wm->char_x, win->y_size * wm->char_y, get_color( 0, 0, 0 ), wm );
	    }
	    break;
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_box( evt->event_win, wm ); //draw window box
		draw_string( evt->event_win,
		             ( (win->x_size*wm->char_x) - (data->title_len*wm->char_x) ) >> 1,
			     (win->y_size*wm->char_y) / 2 - wm->char_y*2,
			     0, data->title, wm );
		draw_rectangle( evt->event_win, 0, 0, win->x_size * wm->char_x, win->y_size * wm->char_y, get_color( 0, 0, 0 ), wm );
	    }
	    break;
	case EVT_BUTTONDOWN:
	    handled = 1;
	    if( evt->button >> 3 )
	    {
		switch( evt->button >> 3 )
		{
		    case 'y': case 'Y':
		    send_event( data->this_window, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
		    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		    if( data->handler )
			data->handler( data->user_data, 1, wm );
		    wm->mouse_win = data->old_focus_window;
		    handled = 1;
		    break;
		    
		    case 'n': case 'N': case KEY_ESCAPE:
		    send_event( data->this_window, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
		    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		    if( data->handler )
			data->handler( data->user_data, 0, wm );
		    wm->mouse_win = data->old_focus_window;
		    handled = 1;
		    break;
		    
		    case KEY_TAB: case KEY_LEFT: case KEY_RIGHT:
		    data->current_button ^= 1;
		    if( data->current_button == 0 )
		    {
			button_set_color( get_color( 255, 128, 0 ), data->button_ok, wm );
			button_set_color( wm->colors[ 6 ], data->button_cancel, wm );			
		    }
		    else
		    {
			button_set_color( get_color( 255, 128, 0 ), data->button_cancel, wm );
			button_set_color( wm->colors[ 6 ], data->button_ok, wm );			
		    }
		    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		    handled = 1;
		    break;
		    
		    case KEY_ENTER: case ' ':
		    send_event( data->this_window, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
		    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		    if( data->current_button == 0 )
		    {
			if( data->handler )
			    data->handler( data->user_data, 1, wm );
		    }
		    else
		    {
			if( data->handler )
			    data->handler( data->user_data, 0, wm );
		    }
		    wm->mouse_win = data->old_focus_window;
		    handled = 1;
		    break;
		}
	    }
	    break;
    }
    return handled;
}
