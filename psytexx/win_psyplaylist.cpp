/*
    win_psyplaylist.cpp - playlist handler
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_dialog.h"
#include "win_button.h"
#include "win_files.h"
#include "win_list.h"
#include "win_psyplaylist.h"
#include "win_psymenu.h"
#include "win_psysamples.h"
#include "../xm/xm.h"
#include "../sound/sound.h"

//Some device dependent part ===========

#ifdef OS_LINUX
    #include <stdlib.h>
#endif

uint random_number( void )
{
#ifndef NONPALM
    return (uint)SysRandom( 0 ) << 1;
#else
    return rand() & 0xFFFF;
#endif
}

//=====================================

void playlist_save( const char *filename, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ];
    playlist_data *data = (playlist_data*)win->data;
    
    FILE *f = fopen( filename, "wb" );
    uint8_t temp;
    if( f )
    {
	fwrite( &data->current_selected, 4, 1, f );
	for( int a = 0;; a++ )
	{
	    char *name = list_get_item( a, data->list_files, wm );
	    temp = 0xFF;
	    if( name == 0 ) { fwrite( &temp, 1, 1, f ); break; } //Write EOF
	    int s;
	    //Get item size:
	    for( s = 0;; s++ ) if( name[ s ] == 0 ) break;
	    fwrite( name, s+1, 1, f ); //Write item to a file
	}
	fclose( f );
    }
}

void playlist_load( const char *filename, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ];
    playlist_data *data = (playlist_data*)win->data;
    char name[ MAX_DIR_LENGTH ];
    
    FILE *f = fopen( filename, "rb" );
    if( f )
    {
	fread( &data->current_selected, 4, 1, f );
	list_clear( data->list_files, wm );
	int c = 0;
	int cc;
	for( int a = 0;; a++ )
	{
	    if( feof( f ) != 0 ) break;
	    cc = name[ c++ ] = getc( f );
	    if( cc == 0xFF ) break; //EOF
	    if( cc == 0 ) { list_add_item( name, 0, data->list_files, wm ); c = 0; } //Add new item
	}
	//Set selection:
	window *lwin = wm->windows[ data->list_files ];
	list_data *ldata = (list_data*)lwin->data;
	ldata->selected_item = data->current_selected;
	if( ldata->selected_item < 0 ) ldata->selected_item = 0;
	if( ldata->selected_item >= ldata->items_num ) ldata->selected_item = ldata->items_num - 1;
	//Show it:
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	//Close file:
	fclose( f );
    }
}

void playlist_play_selected( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ];
    playlist_data *data = (playlist_data*)win->data;

    window *lwin = wm->windows[ data->list_files ];
    list_data *ldata = (list_data*)lwin->data;
    
    if( ldata->selected_item < 0 ) ldata->selected_item = 0;
    if( ldata->selected_item >= ldata->items_num ) ldata->selected_item = ldata->items_num - 1;
    
    int selected = ldata->selected_item;
    if( selected >= 0 )
    {
	data->current_selected = selected;
	//Load XM:
	char *name;
	name = list_get_item( selected, data->list_files, wm );
	if( name )
	{
    	    sound_stream_stop();
	    mem_off();
	    load_module( name, &xm );
	    mem_on();
	    sound_stream_play();				
	    samples_refresh( win_samples, wm );
	    
	    xm.status = XM_STATUS_PLAYLIST;
	    clean_channels( &xm );
    	    xm_pat_rewind( &xm );
	
	    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	}
	data->current_selected = selected;
    }
}

void playlist_select_next_track( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ];
    playlist_data *data = (playlist_data*)win->data;
    
    window *lwin = wm->windows[ data->list_files ];
    list_data *ldata = (list_data*)lwin->data;
    if( ldata->selected_item < 0 ) ldata->selected_item = 0;
    if( ldata->selected_item >= ldata->items_num ) ldata->selected_item = ldata->items_num - 1;
    if( data->random_mode )
    {
	int old = ldata->selected_item;
	for(;;)
	{
	    ldata->selected_item = ( random_number() * (ldata->items_num+1) ) >> 16;
	    if( old == ldata->selected_item && ldata->items_num > 1 ) {} else break;
	}
    }
    else
	ldata->selected_item++;
    if( ldata->selected_item >= ldata->items_num ) ldata->selected_item = 0;
    //Make selected item visible:
    list_make_selection_visible( data->list_files, wm );
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
}

void playlist_select_previous_track( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ];
    playlist_data *data = (playlist_data*)win->data;
    
    window *lwin = wm->windows[ data->list_files ];
    list_data *ldata = (list_data*)lwin->data;
    if( ldata->selected_item < 0 ) ldata->selected_item = 0;
    if( ldata->selected_item >= ldata->items_num ) ldata->selected_item = ldata->items_num - 1;
    if( data->random_mode )
    {
	int old = ldata->selected_item;
	for(;;)
	{
	    ldata->selected_item = ( random_number() * (ldata->items_num+1) ) >> 16;
	    if( old == ldata->selected_item && ldata->items_num > 1 ) {} else break;
	}
    }
    else
	ldata->selected_item--;
    if( ldata->selected_item < 0 ) ldata->selected_item = ldata->items_num - 1;
    //Make selected item visible:
    list_make_selection_visible( data->list_files, wm );
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
}

int button_pclose_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;    
    send_event( data->this_window, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    
    window *mwin = wm->windows[ win_menu ];
    menu_data *mdata = (menu_data*)mwin->data;
    mdata->playlist_status = 0;
    
    return 0;
}

int button_addfile_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;    
    char path[ MAX_DIR_LENGTH ];
    
    window *fwin = wm->windows[ win_files ];
    files_data *fdata = (files_data*)fwin->data; //Get data of the "files" window

    char *name = files_get_file( win_files, wm );
    if( name ) if( name[0] ) 
    {
	path[ 0 ] = 0;
	mem_strcat( path, get_disk_name( fdata->disk_number ) );
	mem_strcat( path, fdata->dir_name );
	mem_strcat( path, name );
	list_add_item( path, 0, data->list_files, wm );
    }
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    
    return 0;
}

int button_addfiles_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;
    char path[ MAX_DIR_LENGTH ];
    
    char *name = 0;
    window *fwin = wm->windows[ win_files ];
    files_data *fdata = (files_data*)fwin->data; //Get data of the "files" window
    for( int a = 0;; a++ )
    {
	name = list_get_item( a, fdata->list_window, wm );
	if( name == 0 ) break;
	if( list_get_attr( a, fdata->list_window, wm ) == 0 && name[0] )
	{
	    path[ 0 ] = 0;
	    mem_strcat( path, get_disk_name( fdata->disk_number ) );
	    mem_strcat( path, fdata->dir_name );
	    mem_strcat( path, name );
	    list_add_item( path, 0, data->list_files, wm );
	}
    }
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    
    return 0;
}

int button_delfile_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;    
    
    int selected = list_get_selected_num( data->list_files, wm );
    if( selected >= 0 )
    {
	list_delete_item( selected, data->list_files, wm );
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    
    return 0;
}

int button_pup_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;    
    
    int selected = list_get_selected_num( data->list_files, wm );
    if( selected >= 0 )
    {
	list_move_item_up( selected, data->list_files, wm );
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    
    return 0;
}

int button_pdown_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;    
    
    int selected = list_get_selected_num( data->list_files, wm );
    if( selected >= 0 )
    {
	list_move_item_down( selected, data->list_files, wm );
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    
    return 0;
}

void clear_playlist( void *user_data, int button, window_manager *wm )
{
    playlist_data *ldata = (playlist_data*)user_data;    
    if( button == 1 )
    {
	list_clear( ldata->list_files, wm );
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
}

int button_pclear_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;    
    
    start_dialog( "Clear play list?", "YES", "NO",
	&clear_playlist, data,
	win_dialog, wm );

    return 0;
}

int button_plplay_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;    
    
    data->random_mode = 0;
    playlist_play_selected( win_playlist, wm );
    
    return 0;
}

int button_random_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;    
    
    data->random_mode = 1;
    playlist_play_selected( win_playlist, wm );
    
    return 0;
}

int button_plnext_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;    
    
    playlist_select_next_track( win_playlist, wm );
    if( xm.status == XM_STATUS_PLAYLIST )  playlist_play_selected( win_playlist, wm );
    
    return 0;
}

int button_plprev_handler( void *user_data, int win, window_manager *wm )
{
    playlist_data *data = (playlist_data*)user_data;    
    
    playlist_select_previous_track( win_playlist, wm );
    if( xm.status == XM_STATUS_PLAYLIST )  playlist_play_selected( win_playlist, wm );
    
    return 0;
}

int playlist_handler( wm_event *evt, window_manager *wm )
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    playlist_data *data = (playlist_data*)win->data;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(playlist_data), "playlist data" );
	    //Init data:
	    data = (playlist_data*)win->data;
	    
	    data->this_window = evt->event_win;
	    data->random_mode = 0;
	    data->current_selected = 0;
	    
	    data->button_close = create_window( "pclose", 1, win->y_size-3, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_close, "1", "100%-1", "6", "100%-3", wm );
	    button_set_handler( &button_pclose_handler, (void*)data, data->button_close, wm );
	    button_set_name( "CLOSE", data->button_close, wm );

	    data->button_clear = create_window( "plclear", 1, 1, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_clear, "7", "100%-1", "7+5", "100%-3", wm );
	    button_set_handler( &button_pclear_handler, (void*)data, data->button_clear, wm );
	    button_set_name( "CLEAR", data->button_clear, wm );

	    data->button_play = create_window( "plplay", 1, 1, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_play, "13", "100%-1", "13+4", "100%-3", wm );
	    button_set_handler( &button_plplay_handler, (void*)data, data->button_play, wm );
	    button_set_name( "PLAY", data->button_play, wm );

	    data->button_random = create_window( "rndplay", 1, 1, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_random, "18", "100%-1", "18+11", "100%-3", wm );
	    button_set_handler( &button_random_handler, (void*)data, data->button_random, wm );
	    button_set_name( "RANDOM PLAY", data->button_random, wm );

	    data->button_prev = create_window( "prevsong", 1, 1, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_prev, "30", "100%-1", "30+2", "100%-3", wm );
	    button_set_handler( &button_plprev_handler, (void*)data, data->button_prev, wm );
	    button_set_name( txt_left, data->button_prev, wm );

	    data->button_next = create_window( "nextsong", 1, 1, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_next, "33", "100%-1", "33+2", "100%-3", wm );
	    button_set_handler( &button_plnext_handler, (void*)data, data->button_next, wm );
	    button_set_name( txt_right, data->button_next, wm );

	    data->button_addfile = create_window( "addfile", 1, 1, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_addfile, "1", "1", "9", "3", wm );
	    button_set_handler( &button_addfile_handler, (void*)data, data->button_addfile, wm );
	    button_set_name( "ADD FILE", data->button_addfile, wm );

	    data->button_addfiles = create_window( "addfiles", 7, 1, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_addfiles, "1+9", "1", "10+7", "3", wm );
	    button_set_handler( &button_addfiles_handler, (void*)data, data->button_addfiles, wm );
	    button_set_name( "ADD ALL", data->button_addfiles, wm );

	    data->button_delfile = create_window( "delfile", 7, 1, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_delfile, "18", "1", "18+8", "3", wm );
	    button_set_handler( &button_delfile_handler, (void*)data, data->button_delfile, wm );
	    button_set_name( "DEL FILE", data->button_delfile, wm );

	    data->button_up = create_window( "fileup", 7, 1, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_up, "27", "1", "27+2", "3", wm );
	    button_set_handler( &button_pup_handler, (void*)data, data->button_up, wm );
	    button_set_name( txt_up, data->button_up, wm );

	    data->button_down = create_window( "filedown", 7, 1, 6, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_down, "30", "1", "30+2", "3", wm );
	    button_set_handler( &button_pdown_handler, (void*)data, data->button_down, wm );
	    button_set_name( txt_down, data->button_down, wm );
	    
	    data->list_files = create_window( "playlist files",
	        0, 0, 4, 4,
	        wm->colors[6], 0,
	        evt->event_win,
	        &list_handler, wm );
	    set_window_string_controls( data->list_files, "1", "4", "100%-1", "100%-4", wm );

	    //Load previous playlist:
	    playlist_load( PROP_PLAYLIST, evt->event_win, wm );
																																							
	    break;

	case EVT_BEFORECLOSE:
	    playlist_save( PROP_PLAYLIST, evt->event_win, wm );
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
		list_make_selection_visible( data->list_files, wm );
		draw_window_touch_area( evt->event_win, wm );
		draw_window_box(evt->event_win,wm); //draw window box
	    }
	    break;
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		list_make_selection_visible( data->list_files, wm );
		draw_window_box(evt->event_win,wm); //draw window box
	    }
	    break;
    }
    return 0;
}
