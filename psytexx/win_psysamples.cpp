/*
    win_psysamples.cpp - samples window handler
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_button.h"
#include "win_list.h"
#include "win_text.h"
#include "win_psysamples.h"
#include "../xm/xm.h"

int current_instrument = 0; //Current instrument number
int current_sample = 0;     //Current sample number

void samples_refresh( int win_num, window_manager *wm ) //Refresh instrument list
{
    window *win = wm->windows[ win_num ]; //Our window
    samples_data *data = (samples_data*)win->data;

    list_clear( data->instruments_list, wm );
    
    char inst_name[23]; inst_name[22] = 0;

    module *song = xm.song;
    if( song )
    {
	//Instruments:
	for( int i = 0; i < 128; i++ )
	{
	    instrument *inst = song->instruments[ i ];
	    if( inst )
	    { //Instrument exist:
		mem_copy( inst_name, inst->name, 22 );
		list_add_item( inst_name, 2, data->instruments_list, wm );
	    }
	    else
	    {
		list_add_item( "", 0, data->instruments_list, wm );
	    }
	}
    }

    samples2_refresh( win_num, wm ); //Refresh samples
}

void samples2_refresh( int win_num, window_manager *wm ) //Refresh sample list
{
    window *win = wm->windows[ win_num ]; //Our window
    samples_data *data = (samples_data*)win->data;

    list_clear( data->samples_list, wm );

    char samp_name[23]; samp_name[22] = 0;

    module *song = xm.song;
    if( song )
    {
	instrument *inst = song->instruments[ data->current_instrument ];
	if( inst )
	{
	    //Samples:
	    for( int i = 0; i < 16; i++ )
	    {
		sample *smp = inst->samples[ i ];
		if( smp )
		{ //Sample exist:
		    mem_copy( samp_name, smp->name, 22 );
		    list_add_item( samp_name, 2, data->samples_list, wm );
		}
		else
		{
		    list_add_item( "", 0, data->samples_list, wm );
		}
	    }
	}
    }
}

int instr_list_handler( void *user_data, int list_win, window_manager *wm )
{
    window *lwin = wm->windows[ list_win ]; //list win
    list_data *ldata = (list_data*)lwin->data;
    samples_data *data = (samples_data*)user_data;
    
    current_instrument = 
    data->current_instrument = ldata->selected_item;
    samples2_refresh( data->this_window, wm );
    send_event( data->samples_list, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Redraw list
    send_event( win_insteditor, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm);
    send_event( win_smpeditor, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm);

    return 0;
}

int smp_list_handler( void *user_data, int list_win, window_manager *wm )
{
    window *lwin = wm->windows[ list_win ]; //list win
    list_data *ldata = (list_data*)lwin->data;
    samples_data *data = (samples_data*)user_data;
    
    current_sample = data->current_sample = ldata->selected_item;

    send_event( win_insteditor, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm);
    send_event( win_smpeditor, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm);

    return 0;
}

int edit_instr_handler( void *data, int text_window, window_manager *wm ) //When we changing instr name
{
    list_data *ldata = (list_data*)data;
    int a;
    
    char *new_name = text_get_text( ldata->edit_field, wm );

    if( ldata->selected_item < 0 ) return 0;
    if( ldata->selected_item >= ldata->items_num ) return 0;
    if( ldata->selected_item < xm.song->instruments_num )
    {
	//Copy new name from a text field to the song:
	for( a = 0; a < 22; a++ )
	{
	    xm.song->instruments[ ldata->selected_item ]->name[ a ] = new_name[ a ];
	    if( new_name[ a ] == 0 ) break;
	}
    }
    else
    { //We needs to add instruments:
	if( new_name[ 0 ] == 0 ) return 0; //Empty name
	//Add instruments:
	for( a = xm.song->instruments_num; a <= ldata->selected_item; a++ )
	{
	    new_instrument( a, "", 0, &xm );
	}
	//Copy new name from a text field to the song:
	for( a = 0; a < 22; a++ )
	{
	    xm.song->instruments[ ldata->selected_item ]->name[ a ] = new_name[ a ];
	    if( new_name[ a ] == 0 ) break;
	}
	xm.song->instruments_num = (uint16_t)ldata->selected_item + 1;
    }
    
    int temp = ldata->first_item;
    samples_refresh( win_samples, wm );
    ldata->first_item = temp;
    send_event( win_samples, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Redraw all
    
    return 0;
}

int edit_samples_handler( void *data, int text_window, window_manager *wm )
{
    list_data *ldata = (list_data*)data;
    int a;
    
    char *new_name = text_get_text( ldata->edit_field, wm );

    if( ldata->selected_item < 0 ) return 0;
    if( ldata->selected_item >= ldata->items_num ) return 0;
    if( ldata->selected_item < xm.song->instruments[ current_instrument ]->samples_num )
    {
	//Copy new name from a text field to the song:
	for( a = 0; a < 22; a++ )
	{
	    xm.song->instruments[ current_instrument ]->samples[ ldata->selected_item ]->name[ a ] = new_name[ a ];
	    if( new_name[ a ] == 0 ) break;
	}
    }
    else
    { //We needs to add samples:
	if( new_name[ 0 ] == 0 ) return 0; //Empty name
	//Add samples:
	for( a = xm.song->instruments[ current_instrument ]->samples_num; a <= ldata->selected_item; a++ )
	{
	    new_sample( a, (uint16_t)current_instrument, "", 0, 0, &xm );
	}
	//Copy new name from a text field to the song:
	for( a = 0; a < 22; a++ )
	{
	    xm.song->instruments[ current_instrument ]->samples[ ldata->selected_item ]->name[ a ] = new_name[ a ];
	    if( new_name[ a ] == 0 ) break;
	}
	xm.song->instruments[ current_instrument ]->samples_num = (uint16_t)ldata->selected_item + 1;
    }
    
    int temp = ldata->first_item;
    samples2_refresh( win_samples, wm );
    ldata->first_item = temp;
    send_event( win_samples, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Redraw all
    
    return 0;
}

void unscale_top_win( samples_data *data, window_manager *wm )
{
    window *top_win = wm->windows[ win_top ];
    if( wm->screen_y <= 48 && data->old_ys2 != 0 )
    {
	top_win->ys2 = data->old_ys2;
	resize_all_windows( wm );
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	data->old_ys2 = 0;
    }
}

void scale_top_win( samples_data *data, window_manager *wm )
{
    window *top_win = wm->windows[ win_top ];
    if( wm->screen_y <= 48 )
    {
	data->old_ys2 = top_win->ys2;
	top_win->ys2 = "50%<14";
	resize_all_windows( wm );
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
}

int button_insteditor_handler( void *user_data, int win, window_manager *wm )
{
    samples_data *data = (samples_data*)user_data;
    window *w = wm->windows[ win_insteditor ];
    if( w->visible )
    {
	send_event( win_insteditor, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm);
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Redraw all
	unscale_top_win( data, wm );
    }
    else
    {
	send_event( win_smpeditor, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
	send_event( win_insteditor, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm);
	scale_top_win( data, wm );
    }

    return 0;
}

int button_smpeditor_handler( void *user_data, int win, window_manager *wm )
{
    samples_data *data = (samples_data*)user_data;
    window *w = wm->windows[ win_smpeditor ];
    if( w->visible )
    {
	send_event( win_smpeditor, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm);
	send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Redraw all
	unscale_top_win( data, wm );
    }
    else
    {
	send_event( win_insteditor, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
	send_event( win_smpeditor, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm);
	unscale_top_win( data, wm );
    }

    return 0;
}

int button_inspage_handler( void *user_data, int win, window_manager *wm )
{
    window *w = wm->windows[ win_samples ];
    samples_data *data = (samples_data*)w->data;
    window *lw = wm->windows[ data->instruments_list ];
    list_data *ldata = (list_data*)lw->data;
    ldata->first_item = ((size_t)user_data);
    send_event( win_samples, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Redraw all
    return 0;
}

char ins_button_name[ 64 ];
int button_ins_smp_handler( void *user_data, int win, window_manager *wm )
{
    samples_data *data = (samples_data*)user_data;
    data->ins_or_smp ^= 1;
    if( data->ins_or_smp & 1 )
    {
	//Samples:
	ins_button_name[ 0 ] = 0;
	mem_strcat( ins_button_name, "SAMPLES IN INSTR." );
	char txt_num[ 8 ];
	int_to_string_h( current_instrument + 1, txt_num );
	mem_strcat( ins_button_name, txt_num );
	mem_strcat( ins_button_name, ":" );
	button_set_name( ins_button_name, data->button, wm );

	send_event( data->instruments_list, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
	send_event( data->samples_list, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm);
    }
    else
    {
	//Instruments:
	button_set_name( "INSTRUMENTS:", data->button, wm );
	send_event( data->samples_list, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm);
	send_event( data->instruments_list, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
    send_event( win_samples, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Redraw all
    return 0;
}

int samples_handler( wm_event *evt, window_manager *wm )
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    samples_data *data = (samples_data*)win->data;
    int samples_y;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(samples_data), "menu data" );
	    //Init data:
	    data = (samples_data*)win->data;

	    data->current_instrument = 0;
	    data->this_window = evt->event_win;

	    data->ins_or_smp = 0;
	    data->old_ys2 = 0;

#ifdef TEXTMODE
	    samples_y = win->y_size / 2;
#else
	    samples_y = win->y_size / 3;
	    if( samples_y < 5 ) samples_y = 5;
#endif

	    data->samples_list = create_window( "smplist",
	                                        0, win->y_size - samples_y, 
	                                        win->x_size, 
						samples_y,
						wm->colors[6], 0, 
						evt->event_win, 
						&list_handler, wm );
	    //set_window_string_controls( data->samples_list, "0", "100%", "100%", "50% >100%-8", wm );
	    set_window_string_controls( data->samples_list, "0", "2", "100%", "100%", wm );
	    list_set_handler( &smp_list_handler, (void*)data, data->samples_list, wm );
	    list_set_edit_handler( &edit_samples_handler, 0, data->samples_list, wm );

	    data->instruments_list = create_window( "instrlist",
	                                            0, 0, 
	                                            win->x_size, 
						    win->y_size - samples_y, 
						    win->color, 0, 
						    evt->event_win, 
						    &list_handler, wm );
	    //set_window_string_controls( data->instruments_list, "0", "0", "100%", "smplist.y", wm );
	    set_window_string_controls( data->instruments_list, "0", "2", "100%", "100%", wm );
	    list_set_handler( &instr_list_handler, (void*)data, data->instruments_list, wm );
	    list_set_edit_handler( &edit_instr_handler, 0, data->instruments_list, wm );

	    list_set_numbered( 1, 1, data->instruments_list, wm );
	    list_set_numbered( 1, 0, data->samples_list, wm );
	    list_set_editable( 1, data->instruments_list, wm );
	    list_set_editable( 1, data->samples_list, wm );

	    data->button = create_window( "textinsts", 1, 1, 2, 2, wm->colors[ 5 ], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button, "0", "0", "100%", "2", wm );
	    button_set_name( "INSTRUMENTS:", data->button, wm );
	    button_set_handler( &button_ins_smp_handler, (void*)data, data->button, wm );

    	    /*
	    text = create_window( "textsamples", 1, 1, 2, 2, win->color, 1, evt->event_win, &text_handler, wm );
	    set_window_string_controls( text, "50%", "0", "100%", "1", wm );
	    text_set_text( text, "SAMPLES:", wm );
	    text_set_readonly( text, 1, wm );
	    */

	    //EDIT BUTTONS:

	    data->ins_edit_button = create_window( "editins", 0, 0, 2, 2, wm->colors[ 7 ], 1, data->instruments_list, &button_handler, wm );
	    button_set_name( "I.EDIT", data->ins_edit_button, wm );
	    button_set_handler( &button_insteditor_handler, (void*)data, data->ins_edit_button, wm );

	    data->smp_edit_button = create_window( "editsmp", 0, 0, 2, 2, wm->colors[ 7 ], 1, data->instruments_list, &button_handler, wm );
	    button_set_name( "S.EDIT", data->smp_edit_button, wm );
	    button_set_handler( &button_smpeditor_handler, (void*)data, data->smp_edit_button, wm );

	    if( wm->screen_y <= 24 )
	    {
		//Small screen:
		set_window_string_controls( data->ins_edit_button, "100%-8", "0", "100%-2", "2", wm );
		set_window_string_controls( data->smp_edit_button, "100%-8", "2", "100%-2", "4", wm );
	    }
	    else
	    {
		set_window_string_controls( data->ins_edit_button, "100%-8", "2", "100%-2", "4", wm );
		set_window_string_controls( data->smp_edit_button, "100%-8", "4", "100%-2", "6", wm );
	    }

	    //PAGES:

	    data->ins_page_button = create_window( "inspage1", 0, 0, 2, 2, wm->colors[ 5 ], 1, data->instruments_list, &button_handler, wm );
	    button_set_name( "01", data->ins_page_button, wm );
	    set_window_string_controls( data->ins_page_button, "100%-8", "6", "100%-5", "8", wm );
	    button_set_handler( &button_inspage_handler, 0, data->ins_page_button, wm );

	    data->ins_page_button = create_window( "inspage1", 0, 0, 2, 2, wm->colors[ 5 ], 1, data->instruments_list, &button_handler, wm );
	    button_set_name( "40", data->ins_page_button, wm );
	    set_window_string_controls( data->ins_page_button, "100%-5", "6", "100%-2", "8", wm );
	    button_set_handler( &button_inspage_handler, (void*)(0x40-1), data->ins_page_button, wm );

	    data->ins_page_button = create_window( "inspage1", 0, 0, 2, 2, wm->colors[ 5 ], 1, data->instruments_list, &button_handler, wm );
	    button_set_name( "10", data->ins_page_button, wm );
	    set_window_string_controls( data->ins_page_button, "100%-8", "8", "100%-5", "10", wm );
	    button_set_handler( &button_inspage_handler, (void*)(0x10-1), data->ins_page_button, wm );

	    data->ins_page_button = create_window( "inspage1", 0, 0, 2, 2, wm->colors[ 5 ], 1, data->instruments_list, &button_handler, wm );
	    button_set_name( "50", data->ins_page_button, wm );
	    set_window_string_controls( data->ins_page_button, "100%-5", "8", "100%-2", "10", wm );
	    button_set_handler( &button_inspage_handler, (void*)(0x50-1), data->ins_page_button, wm );

	    data->ins_page_button = create_window( "inspage1", 0, 0, 2, 2, wm->colors[ 5 ], 1, data->instruments_list, &button_handler, wm );
	    button_set_name( "20", data->ins_page_button, wm );
	    set_window_string_controls( data->ins_page_button, "100%-8", "10", "100%-5", "12", wm );
	    button_set_handler( &button_inspage_handler, (void*)(0x20-1), data->ins_page_button, wm );

	    data->ins_page_button = create_window( "inspage1", 0, 0, 2, 2, wm->colors[ 5 ], 1, data->instruments_list, &button_handler, wm );
	    button_set_name( "60", data->ins_page_button, wm );
	    set_window_string_controls( data->ins_page_button, "100%-5", "10", "100%-2", "12", wm );
	    button_set_handler( &button_inspage_handler, (void*)(0x60-1), data->ins_page_button, wm );

	    data->ins_page_button = create_window( "inspage1", 0, 0, 2, 2, wm->colors[ 5 ], 1, data->instruments_list, &button_handler, wm );
	    button_set_name( "30", data->ins_page_button, wm );
	    set_window_string_controls( data->ins_page_button, "100%-8", "12", "100%-5", "14", wm );
	    button_set_handler( &button_inspage_handler, (void*)(0x30-1), data->ins_page_button, wm );

	    data->ins_page_button = create_window( "inspage1", 0, 0, 2, 2, wm->colors[ 5 ], 1, data->instruments_list, &button_handler, wm );
	    button_set_name( "70", data->ins_page_button, wm );
	    set_window_string_controls( data->ins_page_button, "100%-5", "12", "100%-2", "14", wm );
	    button_set_handler( &button_inspage_handler, (void*)(0x70-1), data->ins_page_button, wm );

	    samples_refresh( evt->event_win, wm );
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
