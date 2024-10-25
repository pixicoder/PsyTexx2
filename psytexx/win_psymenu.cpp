/*
    win_psymenu.cpp - menu handler
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_popup.h"
#include "win_dialog.h"
#include "win_button.h"
#include "win_psytable.h"
#include "win_psysamples.h"
#include "win_psypattern.h"
#include "win_psymenu.h"
#include "../time/time.h"
#include "../xm/xm.h"

int play_button_handler( void *user_data, int button_win, window_manager *wm )
{
    //Edit mode off:
    window *win = wm->windows[ win_patterntable ];
    table_data *tdata = (table_data*)win->data;
    tdata->record_status = 0;

    win = wm->windows[ win_patterneditor ];
    tdata = (table_data*)win->data;
    tdata->record_status = 0;

    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );

    xm.status = XM_STATUS_PLAY;
    clean_channels( &xm );
    xm_pat_rewind( &xm );
    
    play_start_time = time_ticks();
    return 0;
}

int patplay_button_handler( void *user_data, int button_win, window_manager *wm )
{
    //Edit mode off:
    window *win = wm->windows[ win_patterntable ];
    table_data *tdata = (table_data*)win->data;
    tdata->record_status = 0;

    win = wm->windows[ win_patterneditor ];
    tdata = (table_data*)win->data;
    tdata->record_status = 0;

    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    
    xm.status = XM_STATUS_PPLAY;
    clean_channels( &xm );
    xm_pat_rewind( &xm );

    play_start_time = time_ticks();
    return 0;
}

int stop_button_handler( void *user_data, int button_win, window_manager *wm )
{
    xm.status = XM_STATUS_STOP;
    clean_channels( &xm );
    return 0;
}

int net_button_handler( void *user_data, int button_win, window_manager *wm )
{
    window *win = wm->windows[ win_net ];
    if( win->visible == 0 )
	send_event( win_net, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
    else
    {
	send_event( win_net, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    return 0;
}

int files_button_handler( void *user_data, int button_win, window_manager *wm )
{
    menu_data *data = (menu_data*)user_data;
    if( data->files_status == 0 )
    {
	send_event( win_files, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
	data->files_status = 1;
    }
    else
    {
	send_event( win_files, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	data->files_status = 0;
    }
    return 0;
}

int playlist_button_handler( void *user_data, int button_win, window_manager *wm )
{
    menu_data *data = (menu_data*)user_data;
    if( data->playlist_status == 0 )
    {
	send_event( win_playlist, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
	data->playlist_status = 1;
    }
    else
    {
	send_event( win_playlist, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	data->playlist_status = 0;
    }
    return 0;
}

int config_button_handler( void *user_data, int button_win, window_manager *wm )
{
    menu_data *data = (menu_data*)user_data;
    if( data->config_status == 0 )
    {
	send_event( win_config, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
	data->config_status = 1;
    }
    else
    {
	send_event( win_config, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	data->config_status = 0;
    }
    return 0;
}

int kbd_button_handler( void *user_data, int button_win, window_manager *wm )
{
    show_keyboard( win_keyboard, wm, -1 );
    return 0;
}

void clear_comments_handler( void *user_data, int button, window_manager *wm )
{
    if( button == 1 )
    { //OK
	mem_off();
	clear_comments( &xm );
	samples_refresh( win_samples, wm );
	mem_on();
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
}

void clear_instruments_handler( void *user_data, int button, window_manager *wm )
{
    if( button == 1 )
    { //OK
	mem_off();
	xm.status = XM_STATUS_STOP;
	clean_channels( &xm );
	clear_instruments( &xm );
	samples_refresh( win_samples, wm );
#ifndef NOPSYNTH
	psynth_clear( xm.pnet );
#endif
	mem_on();
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    start_dialog( "Clear comments?", "YES", "NO",
                  &clear_comments_handler, 0,
                  win_dialog, wm );
}

void clear_patterns_handler( void *user_data, int button, window_manager *wm )
{
    if( button == 1 )
    { //OK
	mem_off();
	xm.status = XM_STATUS_STOP;
	clean_channels( &xm );
	clear_patterns( &xm );
	mem_on();
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    start_dialog( "Clear instruments?", "YES", "NO",
                  &clear_instruments_handler, 0,
                  win_dialog, wm );
}

int clear_button_handler( void *user_data, int button_win, window_manager *wm )
{
    menu_data *data = (menu_data*)user_data;

    start_dialog( "Clear patterns?", "YES", "NO",
                  &clear_patterns_handler, 0,
                  win_dialog, wm );
 
    return 0;
}

int menu_button_handler( void *user_data, int button_win, window_manager *wm )
{
    window *win = wm->windows[ button_win ]; //Button win
    menu_data *data = (menu_data*)user_data;

    switch( 
    start_popup_blocked( "---------\nFILES\n---------\nCONFIG\nCHANGE FONT\n---------\nPLAY LIST\nKEYBOARD\nCLEAR\n---------\nEXIT\n---------",
	win->real_x * wm->char_x,
	win->real_y * wm->char_y,
	wm ) )
    {
	case 1: files_button_handler( user_data, button_win, wm ); break;
	case 3: config_button_handler( user_data, button_win, wm ); break;
	case 4: 
		if( wm->user_font_mode )
		{ //Set built-in font:
		    wm->char_x = 8;
		    wm->char_y = 8;
		    wm->screen_x = wm->pscreen_x / wm->char_x;
		    wm->screen_y = wm->pscreen_y / wm->char_y;
		    resize_all( 0, wm );
		    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw all
		    wm->user_font_mode = 0;
		    //Save status:
		    save_int( 0, PROP_KBD_USERFONT );
		}
		else if( wm->user_font )
		{ //Set external user font:
		    wm->char_x = wm->user_font_x;
		    wm->char_y = wm->user_font_y;
		    wm->screen_x = wm->pscreen_x / wm->char_x;
		    wm->screen_y = wm->pscreen_y / wm->char_y;
		    resize_all( 0, wm );
		    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw all
		    wm->user_font_mode = 1;
		    save_int( 1, PROP_KBD_USERFONT );
		}
	    break;
	case 6: playlist_button_handler( user_data, button_win, wm ); break;
	case 7: kbd_button_handler( user_data, button_win, wm ); break;
	case 8: clear_button_handler( user_data, button_win, wm ); break;
	case 10: if( start_dialog_blocked( "Exit to OS?", "YES", "NO", wm ) == 1 ) wm->exit_flag = 1; break;
    }

    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );

    return 0;
}

int menu_handler(wm_event *evt, window_manager *wm)
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    menu_data *data = (menu_data*)win->data;
    int y = 0;
    int ystep = 2;
    int ysize = 1;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(menu_data), "menu data" );
	    //Init data:
	    data = (menu_data*)win->data;

	    data->menu_button = create_window( "menubutton", 0, y, 6, ysize + 1, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "MENU", data->menu_button, wm );
	    button_set_handler( &menu_button_handler, data, data->menu_button, wm );
	    y += ystep;
	    
	    data->play_button = create_window( "play", 0, y, 6, ysize + 1, wm->colors[10], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "PLAY", data->play_button, wm );
	    button_set_handler( &play_button_handler, 0, data->play_button, wm );
	    y += ystep;
	    
	    data->patplay_button = create_window( "patplay", 0, y, 6, ysize + 1, wm->colors[10], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "P.PLAY", data->patplay_button, wm );
	    button_set_handler( &patplay_button_handler, 0, data->patplay_button, wm );
	    y += ystep;
	    
	    data->stop_button = create_window( "stop", 0, y, 6, ysize + 1, wm->colors[10], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "STOP", data->stop_button, wm );
	    button_set_handler( &stop_button_handler, 0, data->stop_button, wm );
	    y += ystep;

	    //Sound Net window is experimental and is no longer used here. It has grown into a separate application - SunVox
	    /*data->net_button = create_window( "sound net button", 0, y, 6, ysize + 1, wm->colors[10], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "NET", data->net_button, wm );
	    button_set_handler( &net_button_handler, 0, data->net_button, wm );
	    y += ystep;*/

	    data->files_button = create_window( "bfiles", 0, y, 6, ysize + 1, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "FILES", data->files_button, wm );
	    button_set_handler( &files_button_handler, data, data->files_button, wm );
	    y += ystep;

	    /*data->playlist_button = create_window( "bplist", 0, y, 6, ysize + 1, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "P.LIST", data->playlist_button, wm );
	    button_set_handler( &playlist_button_handler, data, data->playlist_button, wm );
	    y += ystep;
	    
	    data->kbd_button = create_window( "bkbd", 0, y, 6, ysize + 1, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "KEYBRD", data->kbd_button, wm );
	    button_set_handler( &kbd_button_handler, data, data->kbd_button, wm );
	    y += ystep;*/

	    data->config_button = create_window( "bconfig", 0, y, 6, ysize + 1, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "CONFIG", data->config_button, wm );
	    button_set_handler( &config_button_handler, data, data->config_button, wm );
	    y += ystep;

	    /*data->clear_button = create_window( "bclear", 0, y, 6, ysize + 1, wm->colors[10], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "CLEAR", data->clear_button, wm );
	    button_set_handler( &clear_button_handler, data, data->clear_button, wm );
	    y += ystep;*/

	    data->files_status = 0;
	    data->config_status = 0;
	    data->playlist_status = 0;
	    
	    create_scrollarea( evt->event_win, wm );
	    break;
	    
	case EVT_BEFORECLOSE:
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
	    }
	    break;
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_box(evt->event_win,wm); //draw window box
	    }
	    break;
    }
    return 0;
}
