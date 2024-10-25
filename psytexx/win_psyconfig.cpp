/*
    win_psyconfig.cpp - config window
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_button.h"
#include "win_psyconfig.h"
#include "win_scrollbar.h"
#include "win_psymenu.h"
#include "win_text.h"
#include "../xm/xm.h"

int button_cclose_handler( void *user_data, int win, window_manager *wm )
{
    config_data *data = (config_data*)user_data;
    send_event( data->this_window, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    
    window *mwin = wm->windows[ win_menu ];
    menu_data *mdata = (menu_data*)mwin->data;
    mdata->config_status = 0;
    
    return 0;
}

int scroll_vol_handler( void *user_data, int win, window_manager *wm )
{
    config_data *data = (config_data*)user_data;
    window *swin = wm->windows[ data->scroll_volume ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    
    int vol = sdata->cur;
    xm_set_volume( vol, &xm );

    data->volume_changed = vol;

    return 0;
}

int config_handler(wm_event *evt, window_manager *wm)
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    config_data *data = (config_data*)win->data;
    int text;
    int old_vol;
    int scroll_size;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(config_data), "config data" );
	    //Init data:
	    data = (config_data*)win->data;
	    
	    data->this_window = evt->event_win;

	    data->volume_changed = -1;
	    
	    data->button_close = create_window( "cclose", 1, win->y_size-3, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_close, "1", "100%-1", "7", "100%-3", wm );
	    button_set_handler( &button_cclose_handler, (void*)data, data->button_close, wm );
	    button_set_name( "CLOSE", data->button_close, wm );

	    scroll_size = 38;
	    NEW_SCROLL_TYPE = 1;
	    data->scroll_volume = create_window( "volume", 1, win->y_size-3-3, scroll_size, 2, wm->colors[6], 1, evt->event_win, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_volume, "1", "100%-4", "100%-1 <39", "100%-6", wm );
	    scrollbar_set_handler( &scroll_vol_handler, (void*)data, data->scroll_volume, wm );
	    scrollbar_set_parameters( data->scroll_volume, 1, 128, 32, 20, wm );
	    //Check for previous saved volume:
	    old_vol = load_int( PROP_VOLUME );
	    if( old_vol != -1 )
	    {
		xm_set_volume( old_vol, &xm );
		scrollbar_set_cur_value( data->scroll_volume, old_vol, wm );
	    }

	    text = create_window( "textvolume", 1, win->y_size-3-3-2, 8, 1, win->color, 1, evt->event_win, &text_handler, wm );
	    set_window_string_controls( text, "1", "100%-7", "11", "100%-8", wm );
	    text_set_text( text, "VOLUME:", wm );
	    text_set_readonly( text, 1, wm );

	    text = create_window( "version2", 1, 1, 32, 1, win->color, 1, evt->event_win, &text_handler, wm );
	    text_set_text( text, PSYTEXX_VERSION, wm );
	    text_set_readonly( text, 1, wm );
	    text = create_window( "date2", 1, 2, 32, 1, win->color, 1, evt->event_win, &text_handler, wm );
	    text_set_text( text, PSYTEXX_DATE, wm );
	    text_set_readonly( text, 1, wm );
	    
	    data->volume_changed = -1;
	    break;
	    
	case EVT_BEFORECLOSE:
	    if( data->volume_changed ) save_int( data->volume_changed, PROP_VOLUME );
	    if( win->data ) mem_free( win->data );
	    break;
	case EVT_SHOW: 
	    //Show window:
	    win->visible = 1; //Make it visible
	    send_event(evt->event_win, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Send DRAW event to all childs
	    break;
	case EVT_HIDE:
	    win->visible = 0;
	    break;
	case EVT_DRAW: 
	    //Draw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_touch_area( evt->event_win, wm );
		draw_window_box(evt->event_win,wm); //draw window box
		draw_vert_line( evt->event_win, win->x_size * wm->char_x - 1, 0, win->y_size * wm->char_y, get_color( 0, 0, 0 ), 2, wm );
	    }
	    break;
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_box(evt->event_win,wm); //draw window box
		draw_vert_line( evt->event_win, win->x_size * wm->char_x - 1, 0, win->y_size * wm->char_y, get_color( 0, 0, 0 ), 2, wm );
	    }
	    break;
    }
    return 0;
}
