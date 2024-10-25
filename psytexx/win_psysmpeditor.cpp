/*
    win_psyconfig.cpp - sample editor
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_button.h"
#include "win_dialog.h"
#include "win_popup.h"
#include "win_psysamples.h"
#include "win_psysmpeditor.h"
#include "win_scrollbar.h"
#include "win_text.h"
#include "win_psysmpview.h"
#include "../xm/xm.h"

void smpeditor_redraw( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    smpeditor_data *data = (smpeditor_data*)win->data;

    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	sample *smp = ins->samples[ current_sample ];
	if( smp )
	{
	    scrollbar_set_parameters( data->scroll_volume, 1, 0x40, smp->volume, 20, wm );
	    scrollbar_set_parameters( data->scroll_pan, 1, 255, smp->panning, 60, wm );
	    scrollbar_set_parameters( data->scroll_finetune, 1, 255, smp->finetune + 128, 60, wm );
	    scrollbar_set_parameters( data->scroll_relative, 1, 168, smp->relative_note + 84, 12, wm );
	    scrollbar_set_step( data->scroll_volume, 1, wm );
	    scrollbar_set_step( data->scroll_pan, 1, wm );
	    scrollbar_set_step( data->scroll_finetune, 1, wm );
	    scrollbar_set_step( data->scroll_relative, 1, wm );
	    if( ( smp->type & 3 ) == 0 ) button_set_name( "NO LOOP", data->button_loop, wm );
	    if( ( smp->type & 3 ) == 1 ) button_set_name( "FORWARD", data->button_loop, wm );
	    if( ( smp->type & 3 ) == 2 ) button_set_name( "PINGPONG", data->button_loop, wm );
	}
	else
	{
	    scrollbar_set_parameters( data->scroll_volume, 1, 0, 0, 1, wm );
	    scrollbar_set_parameters( data->scroll_pan, 1, 0, 0, 1, wm );
	    scrollbar_set_parameters( data->scroll_finetune, 1, 0, 0, 1, wm );
	    scrollbar_set_parameters( data->scroll_relative, 1, 0, 0, 1, wm );
	}
    }
}

void smpeditor_draw_info( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	sample *smp = ins->samples[ current_sample ];
	if( smp )
	{
	    int bits = 8;
	    int channels = 1;
	    if( smp->type & 16 ) bits = 16;
	    if( smp->type & 64 ) channels = 2;
	    //Draw sample info:
	    char info[ 128 ];
	    char count[ 64 ];
	    info[ 0 ] = 0;
	    if( bits == 8 ) mem_strcat( info, "8bit," ); else mem_strcat( info, "16bit," );
	    if( channels == 1 ) mem_strcat( info, "mono," ); else mem_strcat( info, "stereo," );
	    int_to_string( smp->length, count );
	    mem_strcat( info, count );
	    mem_strcat( info, " samples" );
	    int len = mem_strlen( info );
	    int x = ( win->x_size - 15 - len ) * wm->char_x;
	    if( x < 0 ) x = 0;
	    draw_string( win_num, x, 0, 1, info, wm );
	}
    }
}

int button_loop_handler( void *user_data, int win, window_manager *wm )
{
    smpeditor_data *data = (smpeditor_data*)user_data;    
    if( xm.song )
    {
	instrument *ins = xm.song->instruments[ current_instrument ];
	if( ins )
	{
	    sample *smp = ins->samples[ current_sample ];
	    if( smp )
	    {
		int loop = smp->type & 3;
		if( loop == 0 ) loop = 1;
		else if( loop == 1 ) loop = 2;
		else if( loop == 2 ) loop = 0;
		if( smp->replen == 0 ) smp->replen = 1;
		smp->type &= ~3;
		smp->type |= loop;
		send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	    }
	}
    }
        
    return 0;
}

int button_menu_handler( void *user_data, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Button win
    smpeditor_data *data = (smpeditor_data*)user_data;
    window *smpwin = wm->windows[ data->smpview ]; //Button win
    smpview_data *sdata = (smpview_data*)smpwin->data;
    smpview_edit_op( sdata, 
	start_popup_blocked( SAMPLE_MENU, 
	    		     win->real_x * wm->char_x, 
			     win->real_y * wm->char_y, 
			     wm )
    );
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    return 0;
}

int button_zoomin_handler( void *user_data, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Button win
    smpeditor_data *data = (smpeditor_data*)user_data;
    smpview_zoom( data->smpview, 1, wm );
    return 0;
}

int button_zoomout_handler( void *user_data, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Button win
    smpeditor_data *data = (smpeditor_data*)user_data;
    smpview_zoom( data->smpview, 2, wm );
    return 0;
}

int button_sclose_handler( void *user_data, int win, window_manager *wm )
{
    smpeditor_data *data = (smpeditor_data*)user_data;    
    send_event( data->this_window, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
        
    return 0;
}

int scroll_smpvol_handler( void *user_data, int win, window_manager *wm )
{
    smpeditor_data *data = (smpeditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_volume ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    
    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	sample *smp = ins->samples[ current_sample ];
	if( smp )
	    smp->volume = (uint8_t)sdata->cur;
    }

    return 0;
}

int scroll_smppan_handler( void *user_data, int win, window_manager *wm )
{
    smpeditor_data *data = (smpeditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_pan ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    
    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	sample *smp = ins->samples[ current_sample ];
	if( smp )
	    smp->panning = (uint8_t)sdata->cur;
    }

    return 0;
}

int scroll_smpfine_handler( void *user_data, int win, window_manager *wm )
{
    smpeditor_data *data = (smpeditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_finetune ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    
    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	sample *smp = ins->samples[ current_sample ];
	if( smp )
	    smp->finetune = (signed char)sdata->cur - 128;
    }

    return 0;
}

int scroll_relative_handler( void *user_data, int win, window_manager *wm )
{
    smpeditor_data *data = (smpeditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_relative ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    
    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	sample *smp = ins->samples[ current_sample ];
	if( smp )
	    smp->relative_note = (signed char)sdata->cur - 84;
    }

    return 0;
}

int smpeditor_handler( wm_event *evt, window_manager *wm )
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    smpeditor_data *data = (smpeditor_data*)win->data;
    int text;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(smpeditor_data), "smpeditor data" );
	    //Init data:
	    data = (smpeditor_data*)win->data;
	    
	    data->this_window = evt->event_win;
	    
	    data->button_close = create_window( "sclose", 1, 1, 2, 2, wm->colors[ 6 ], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_close, "1", "100%-1", "7", "100%-3", wm );
	    button_set_handler( &button_sclose_handler, (void*)data, data->button_close, wm );
	    button_set_name( "CLOSE", data->button_close, wm );

	    NEW_SCROLL_TYPE = 1;
	    data->scroll_volume = create_window( "smpvolume", 1, 1, 2, 2, wm->colors[ 6 ], 1, evt->event_win, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_volume, "100%-14", "1", "100%", "3", wm );
	    scrollbar_set_handler( &scroll_smpvol_handler, (void*)data, data->scroll_volume, wm );
	    scrollbar_set_value_showing( data->scroll_volume, 1, 0, 0, wm );

	    text = create_window( "textsmpvol", 1, 1, 2, 2, win->color, 1, evt->event_win, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "0", "100%", "1", wm );
	    text_set_text( text, "VOLUME:", wm );
	    text_set_readonly( text, 1, wm );

	    NEW_SCROLL_TYPE = 1;
	    data->scroll_pan = create_window( "smppan", 1, 1, 2, 2, wm->colors[ 6 ], 1, evt->event_win, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_pan, "100%-14", "4", "100%", "6", wm );
	    scrollbar_set_handler( &scroll_smppan_handler, (void*)data, data->scroll_pan, wm );
	    scrollbar_set_value_showing( data->scroll_pan, 1, 0, 0, wm );

	    text = create_window( "textsmppan", 1, 1, 2, 2, win->color, 1, evt->event_win, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "3", "100%", "4", wm );
	    text_set_text( text, "PAN:", wm );
	    text_set_readonly( text, 1, wm );

	    NEW_SCROLL_TYPE = 1;
	    data->scroll_finetune = create_window( "smpfine", 1, 1, 2, 2, wm->colors[ 6 ], 1, evt->event_win, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_finetune, "100%-14", "7", "100%", "9", wm );
	    scrollbar_set_handler( &scroll_smpfine_handler, (void*)data, data->scroll_finetune, wm );
	    scrollbar_set_value_showing( data->scroll_finetune, 1, 1, -128, wm );

	    text = create_window( "textsmpfine", 1, 1, 2, 2, win->color, 1, evt->event_win, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "6", "100%", "7", wm );
	    text_set_text( text, "FINETUNE:", wm );
	    text_set_readonly( text, 1, wm );

	    NEW_SCROLL_TYPE = 1;
	    data->scroll_relative = create_window( "relative_note", 1, 1, 2, 2, wm->colors[ 6 ], 1, evt->event_win, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_relative, "100%-14", "10", "100%", "12", wm );
	    scrollbar_set_handler( &scroll_relative_handler, (void*)data, data->scroll_relative, wm );
	    scrollbar_set_value_showing( data->scroll_relative, 1, 1, -84, wm );

	    text = create_window( "textrelative", 1, 1, 2, 2, win->color, 1, evt->event_win, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "9", "100%", "10", wm );
	    text_set_text( text, "RELATIVE NOTE:", wm );
	    text_set_readonly( text, 1, wm );

	    data->smpview = create_window( "smp view", 1, 1, 2, 2, win->color, 1, evt->event_win, &smpview_handler, wm );
	    set_window_string_controls( data->smpview, "1", "1", "100% - 15", "100% - 4", wm );

	    //Create LOOP button:
	    data->button_loop = create_window( "buttonloop", 0, 0, 1, 1, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_loop, "8", "100%-1", "8+8", "100%-3", wm );
	    button_set_handler( &button_loop_handler, (void*)data, data->button_loop, wm );
	    button_set_name( "NO LOOP", data->button_loop, wm );

	    //Create MENU button:
	    data->button_menu = create_window( "buttonmenu", 0, 0, 1, 1, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_menu, "17", "100%-1", "17+5", "100%-3", wm );
	    button_set_handler( &button_menu_handler, (void*)data, data->button_menu, wm );
	    button_set_name( "MENU", data->button_menu, wm );

	    //Zoom buttons:
	    data->button_zoomin = create_window( "buttonzoomin", 0, 0, 1, 1, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_zoomin, "23", "100%-1", "23+2", "100%-3", wm );
	    button_set_handler( &button_zoomin_handler, (void*)data, data->button_zoomin, wm );
	    button_set_name( "+", data->button_zoomin, wm );
	    data->button_zoomout = create_window( "buttonzoomout", 0, 0, 1, 1, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_zoomout, "26", "100%-1", "26+2", "100%-3", wm );
	    button_set_handler( &button_zoomout_handler, (void*)data, data->button_zoomout, wm );
	    button_set_name( "-", data->button_zoomout, wm );
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
		draw_window_box( evt->event_win,wm ); //draw window box
		draw_horis_line( evt->event_win, 0, 0, win->x_size * wm->char_x, get_color( 0, 0, 0 ), 2, wm );
		smpeditor_redraw( evt->event_win, wm );
		smpeditor_draw_info( evt->event_win, wm );
	    }
	    break;
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_box(evt->event_win,wm); //draw window box
		draw_horis_line( evt->event_win, 0, 0, win->x_size * wm->char_x, get_color( 0, 0, 0 ), 2, wm );
		smpeditor_redraw( evt->event_win, wm );
		smpeditor_draw_info( evt->event_win, wm );
	    }
	    break;
    }
    return 0;
}
