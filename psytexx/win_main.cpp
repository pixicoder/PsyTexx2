/*
    win_main.cpp - main PsyTexx functions
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_files.h"
#include "win_psymenu.h"
#include "win_psytable.h"
#include "win_psysamples.h"
#include "win_psypattern.h"
#include "win_psypatterntab.h"
#include "win_psyconfig.h"
#include "win_psynet.h"
#include "win_psyplaylist.h"
#include "win_psychannels.h"
#include "win_psyinsteditor.h"
#include "win_psysmpeditor.h"
#include "win_dialog.h"
#include "win_popup.h"
#include "win_text.h"
#include "../file/file.h"
#include "../time/time.h"
#include "../xm/xm.h"
#include "../sound/sound.h"

//######################################
//## PSYTEXX VARIABLES:               ##
//######################################

window_manager wm;

int win_desktop;       	//Desktop window
int win_top;
int win_files;         	//File-manager's window
int win_menu;          	//PsyTexx main menu
int win_patterntable;  	//Pattern table
int win_patterncontrol;	//Pattern control window
int win_patterneditor; 	//Pattern editor
int win_samples;       	//Instrument list
int win_pattern_prop;  	//Pattern properties
int win_config;        	//PsyTexx config window
int win_playlist;      	//PsyTexx playlist
int win_insteditor;    	//Instrument editor
int win_smpeditor;     	//Sample editor
int win_net;		//Sound net with effects and synths
int win_keyboard;      	//Virtual keyboard (keyboard_handler declared in the window manager)
int win_dialog;
int win_popup;

//######################################
//## PSYTEXX FUNCTIONS:               ##
//######################################

xm_struct xm; //Main XM sound structure;

void *user_sound_data = 0; //For sound manager
void render_piece_of_sound( signed short *buffer, int buffer_size, void *user_data )
{
    xm_callback( buffer, buffer_size, user_data );
}

void psy_windows_init( void )
{
#ifdef PALMOS
    slog_disable();
#endif

    //Load config file:
    read_file_with_options( "psytexx_config.ini" );

    //Init windows manager:
    win_init( &wm );

    //Load background:
    load_background( &wm );

    //Start sound stream:
    user_sound_data = (void*)&xm;
    xm_init( &xm );
    sound_stream_init();
    sound_stream_play();

    //Create desktop window:
    win_desktop = create_window( "desktop", 0, 0, wm.screen_x, wm.screen_y, wm.colors[0], 0, 0, &desktop_handler, &wm );
    set_window_string_controls( win_desktop, "0", "0", "100%", "100%", &wm );
    send_event( win_desktop, EVT_SHOW, 0, 0, 0, 0, MODE_WINDOW, &wm ); //Show it
    
    //Create top window on the desktop:
    int top = win_top = create_window( "topwin", 0, 0, 1, 1, wm.colors[0], 0, 0, &desktop_handler, &wm );
    set_window_string_controls( top, "0", "0", "100%", "50% < 28", &wm );
    send_event( top, EVT_SHOW, 0, 0, 0, 0, MODE_WINDOW, &wm ); //Show it

    //Create menu window:
    win_menu = create_window( "menu",
                              1, 0, 1, 1, wm.colors[3], 1, top, &menu_handler, &wm );
    set_window_string_controls( win_menu, "100%-6", "0", "100%", "100%", &wm );
    send_event( win_menu, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it

    //Create pattern table window:
    table_type = TYPE_VSCROLL;
    win_patterntable = create_window( "pattable", 0, 0, 1, 1, wm.colors[3], 0, top, &table_handler, &wm );
    table_new( win_patterntable,
               TYPE_VSCROLL,
	       FIELD_PATTERNTABLE_POS,
	       FIELD_NONE,
	       HALIGN,
	       1,
	       &wm,
	       (int)ELEMENT_PATTERN_NUM );
    set_window_string_controls( win_patterntable, "0", "0", "8", "100%-6-4", &wm );
    send_event( win_patterntable, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it

    //Create pattern control window (ins, del, +, -):
    win_patterncontrol = create_window( "patcontrol",
                                        0, 1, 1, 1, 
                                        wm.colors[3], 1, top, &patterntab_handler, &wm );
    set_window_string_controls( win_patterncontrol, "0", "pattable.y2", "pattable.x2", "100%-6", &wm );
    send_event( win_patterncontrol, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it

    //Create "pattern properties" window:
    win_pattern_prop = create_window(
	"patprops", 
	1, 1, 1, 1,
	wm.colors[1],
	1,
	top,
	&pattern_handler,
	&wm );
    set_window_string_controls( win_pattern_prop, "0", "100%-6", "menu.x", "100%", &wm );
    send_event( win_pattern_prop, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it

    //Create instrument list:
    win_samples = create_window( "samples",
                                 1, 0, 1, 1,
		                 wm.colors[3],
			         0,
		                 top,
	      	                 &samples_handler,
		                 &wm );
    set_window_string_controls( win_samples, "menu.x", "0", "pattable.x2 > menu.x-32", "patprops.y", &wm );
    send_event( win_samples, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it

    //Create empty win:
    if( 1 )
    {
	int temp_win;
	int temp_win2 = create_window( 
	    "empty",
	    1, 0, 1, 1,
	    wm.colors[3],
	    1,
	    top,
	    &desktop_handler, 
	    &wm );
	set_window_string_controls( temp_win2, "pattable.x2", "0", "samples.x", "patprops.y", &wm );
	send_event( temp_win2, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it
	
	temp_win = create_window( "version", 1, 1, 32, 1, wm.colors[3], 1, temp_win2, &text_handler, &wm );
	text_set_text( temp_win, PSYTEXX_VERSION, &wm );
	text_set_readonly( temp_win, 1, &wm );
	send_event( temp_win, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it
	
	temp_win = create_window( "date", 1, 2, 32, 1, wm.colors[3], 1, temp_win2, &text_handler, &wm );
	text_set_text( temp_win, PSYTEXX_DATE, &wm );
	text_set_readonly( temp_win, 1, &wm );
	send_event( temp_win, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it
    }

    win_keyboard = create_window( 
        "kbd",
	0, 0,
	1, 1,
	wm.colors[3],
	1,
	win_desktop,
	&keyboard_handler,
	&wm );
#ifdef PALMOS
    set_window_string_controls( win_keyboard, "0", "100%", "100%", "100%-2", &wm );
    send_event( win_keyboard, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it
#else
    set_window_string_controls( win_keyboard, "0", "100%", "100%", "100%", &wm );
#endif

    //Create pattern editor:
    table_type = TYPE_VSCROLL | TYPE_HSCROLL;
    CREATE_TABLE_WITH_EDIT_BUTTON = 1;
    win_patterneditor = create_window( "pateditor",
                                       0, 1, 10, 10, 
				       wm.colors[3],
				       0,
				       0, 
				       &table_handler, 
				       &wm );
    table_new( win_patterneditor,
               TYPE_VSCROLL | TYPE_HSCROLL | TYPE_DIVIDERS,
	       FIELD_PATTERN_POS,
	       FIELD_CHANNEL,
	       HALIGN,
	       5,
	       &wm,
	       (int)ELEMENT_NOTE,
	       (int)ELEMENT_INSTRUMENT,
	       (int)ELEMENT_VOLUME,
	       (int)ELEMENT_EFFECT,
	       (int)ELEMENT_PARAMETER );
    table_full_cell( win_patterneditor, 5, &wm,
	       (int)ELEMENT_NOTE,
	       (int)ELEMENT_INSTRUMENT,
	       (int)ELEMENT_VOLUME,
	       (int)ELEMENT_EFFECT,
	       (int)ELEMENT_PARAMETER );
    table_set_fullscreen( 1, win_patterneditor, &wm );
    table_set_change_view( 1, win_patterneditor, &wm );
    set_window_string_controls( win_patterneditor, "0", "topwin.y2", "100%", "kbd.y", &wm );
    send_event( win_patterneditor, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it
    wm.constant_window = win_patterneditor; //Make it constant window

    //Create "instrument editor" window:
    win_insteditor = create_window( "insteditor",
	0, 0, 1, 1,
	wm.colors[3],
	1,
	win_desktop,
	&insteditor_handler,
	&wm );
    set_window_string_controls( win_insteditor, "0", "topwin.y2", "100%", "100%", &wm );

    //Create "sample editor" window:
    win_smpeditor = create_window( "smpeditor",
	0, 0, 1, 1,
	wm.colors[3],
	1,
	win_desktop,
	&smpeditor_handler,
	&wm );
    set_window_string_controls( win_smpeditor, "0", "topwin.y2", "100%", "100%", &wm );

    //Create "config" window:
    win_config = create_window( "config",
                                0, 0, 1, 1,
		                wm.colors[3],
			        1,
		                win_desktop,
	      	                &config_handler,
		                &wm );
    set_window_string_controls( win_config, "0", "0", "70%", "topwin.y2", &wm );

    //Create "playlist" window:
    win_playlist = create_window( "playlist",
	0, 0, 1, 1,
	wm.colors[3],
	1,
	win_desktop,
	&playlist_handler,
	&wm );
    set_window_string_controls( win_playlist, "0", "topwin.y2", "100%", "100%", &wm );

    //Create "files" window:
    win_files = create_window( "files",
                               0, 0, 1, 1,
		               wm.colors[3],
			       1,
		               win_desktop,
	      	               &files_handler,
		               &wm );
    set_window_string_controls( win_files, "0", "0", "70%", "topwin.y2", &wm );
    files_set_save_handler( &handler_save_xm, 0, win_files, &wm );
    files_set_handler( &handler_load_xm, 0, win_files, &wm );
    //send_event( win_files, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, &wm ); //Show it

    /*
    //CRASHES PALMOS :(
    win_net = create_window( "net win",
                              0, 0, 1, 1,
		              wm.colors[3],
			      1,
		              win_desktop,
	      	              &net_handler,
		              &wm );
    set_window_string_controls( win_net, "0", "0", "100%", "kbd.y", &wm );
    */

    win_dialog = create_window( 
        "dialog",
	1, 1, 1, 1,
	wm.colors[0],
	1,
	win_desktop,
	&dialog_handler,
	&wm );
    set_window_string_controls( win_dialog, "50%-20 >0", "50%-4", "50%+20 <100%", "50%+4", &wm );

    win_popup = create_window( 
        "popup",
	1, 1, 8, 8,
	wm.colors[0],
	1,
	win_desktop,
	&popup_handler,
	&wm );

    set_eventloop_handler( &psy_wm_handler, &wm );

    resize_all_windows( &wm );
}

void psy_event_loop( void )
{
    event_loop( &wm, 1 ); //Window-manager's event-loop
}

void psy_windows_close( void )
{
    sound_stream_close(); //Close sound stream
    xm_close( &xm );
    win_close( &wm );     //Close window manager
    slog_enable();
}

// PsyTexx Window Manager handler:

int patternpos;
int tablepos;
int patternnum;
uint16_t channels;
int bpm;
int speed;
int patsize;
int seconds;
int old_channels;
int old_bpm;
int old_speed;
int old_patsize;
int old_patternpos;
int old_tablepos;
int old_patternnum;
int old_seconds;
int old_counter;
module *song;
pattern *pat;

void psy_wm_handler( window_manager *wm )
{
    patternpos = xm.patternpos;
    tablepos = xm.tablepos;
    song = xm.song;
    bpm = xm.bpm;
    speed = xm.speed;
    seconds = time_seconds();
    if( song )
    {
        patternnum = song->patterntable[ tablepos ];
        channels = song->channels;
    }
    //Redraw PsyTexx windows (pattern editor / pattern table ...)
    if( tablepos != old_tablepos )
    {
        send_event( win_patterntable, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    if( song )
    {
        pat = song->patterns[ patternnum ];
        if( pat )
	{
	    patsize = pat->rows;
	    if( patsize != old_patsize )
	    {
	        text_set_value( text_patsize, patsize, wm );
	        send_event( text_patsize, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	        send_event( win_patterneditor, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //redraw pattern
	    }
	    else
	    {
	        if( patternpos != old_patternpos || tablepos != old_tablepos || patternnum != old_patternnum )
		    send_event( win_patterneditor, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //redraw pattern
	    }
	}
    }
    if( channels != old_channels )
    {
        text_set_value( text_channels, channels, wm );
        send_event( text_channels, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    if( bpm != old_bpm )
    {
        text_set_value( text_bpm, bpm, wm );
        send_event( text_bpm, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    if( speed != old_speed )
    {
        text_set_value( text_speed, speed, wm );
        send_event( text_speed, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    if( xm.song_finished )
    {
        xm.song_finished = 0;
        //Playlist mode: loading new song:
        playlist_select_next_track( win_playlist, wm );
        playlist_play_selected( win_playlist, wm );
    }
    //Redraw channels:
    if( old_counter != xm.counter )
	psypattern_redraw_channels( win_pattern_prop, wm );
    //Redraw time and record status:
    if( seconds != old_seconds ) 
    {
	psypattern_draw_time( win_pattern_prop, wm );
	table_draw_record_status( win_patterneditor, 1, wm );
    }
    
    old_counter = xm.counter;
    old_channels = channels;
    old_patternpos = patternpos;
    old_tablepos = tablepos;
    old_patternnum = patternnum;
    old_bpm = bpm;
    old_speed = speed;
    old_patsize = patsize;
    old_seconds = seconds;
}







int handler_load_xm( void *user_data, int files_window, window_manager *wm )
{
    window *win = wm->windows[ files_window ]; //Our window
    files_data *data = (files_data*)win->data;
    
    //User pressed "LOAD" buttton:
    if( data->current_files == 0 )
    {
	//Load XM:
	sound_stream_stop();
	load_module( data->full_path, &xm );
	play_start_time = time_ticks();
	sound_stream_play();
    }
    if( data->current_files == 1 )
    {
	//Load XI:
	sound_stream_stop();
	clean_channels( &xm );
        mem_off();
	load_instrument( (uint16_t)current_instrument, data->full_path, &xm );
        mem_on();
	sound_stream_play();
    }
    
    samples_refresh( win_samples, wm );

    send_event( win_desktop, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw screen
    
    return 0;
}

void save_xm( void *user_data, int button, window_manager *wm )
{
    files_data *data = (files_data*)user_data;

    if( button == 1 )
    { //OK
	int a = 0;
	char *path = data->full_path;
	for(;;) { if( path[a] == 0 ) break; a++; }
	if( data->current_files == 0 )
	{
	    int add_xm = 1;
	    if( path[a-1] == 'm' && path[a-2] == 'x' && path[a-3] == '.' ) add_xm = 0;
	    if( path[a-1] == 'M' && path[a-2] == 'X' && path[a-3] == '.' ) add_xm = 0;
	    if( add_xm )
	    {
		if( path[a-1] == '.' ) a--;
		path[a++] = '.';
		path[a++] = 'x';
		path[a++] = 'm';
		path[a++] = 0;
	    }
	    sound_stream_stop();
    	    mem_off();
    	    xm_save( data->full_path, &xm );
    	    mem_on();
    	    sound_stream_play();
	}
	if( data->current_files == 1 )
	{
	    int add_xi = 1;
	    if( path[a-1] == 'i' && path[a-2] == 'x' && path[a-3] == '.' ) add_xi = 0;
	    if( path[a-1] == 'I' && path[a-2] == 'X' && path[a-3] == '.' ) add_xi = 0;
	    if( add_xi )
	    {
		if( path[a-1] == '.' ) a--;
		path[a++] = '.';
		path[a++] = 'x';
		path[a++] = 'i';
		path[a++] = 0;
	    }
	    sound_stream_stop();
    	    mem_off();
    	    save_instrument( (uint16_t)current_instrument, data->full_path, &xm );
    	    mem_on();
    	    sound_stream_play();
	}

        files_refresh( data->this_window, wm );    
	send_event( win_desktop, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw screen
    }
}

int handler_save_xm( void *user_data, int files_window, window_manager *wm )
{
    window *win = wm->windows[ files_window ]; //Our window
    files_data *data = (files_data*)win->data;
    
    //User pressed "SAVE" buttton:
    FILE *f = fopen( data->full_path, "rb" );
    if( f )
    {
	//File exist:
	fclose( f );
	start_dialog( "Overwrite?", "YES", "NO", 
	              &save_xm, data, 
		      win_dialog, wm );
    }
    else
    {
	save_xm( data, 1, wm );
    }
    
    return 0;
}
