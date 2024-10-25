/*
    win_psyconfig.cpp - instrument editor
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_button.h"
#include "win_psysamples.h"
#include "win_psyinsteditor.h"
#include "win_scrollbar.h"
#include "win_text.h"
#include "../xm/xm.h"

void insteditor_redraw( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    insteditor_data *data = (insteditor_data*)win->data;

    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	scrollbar_set_parameters( data->scroll_volume, 1, 0x40, ins->volume, 20, wm );
	scrollbar_set_parameters( data->scroll_pan, 1, 255, ins->panning, 60, wm );
	scrollbar_set_parameters( data->scroll_finetune, 1, 255, ins->finetune + 128, 60, wm );
	scrollbar_set_parameters( data->scroll_relative, 1, 168, ins->relative_note + 84, 12, wm );
	scrollbar_set_parameters( data->scroll_vibsweep, 1, 255, ins->vibrato_sweep, 60, wm );
	scrollbar_set_parameters( data->scroll_vibdepth, 1, 255, ins->vibrato_depth, 60, wm );
	scrollbar_set_parameters( data->scroll_vibrate, 1, 0x3F, ins->vibrato_rate, 20, wm );
	if( ins->flags & INST_FLAG_NOTE_OFF )
	    button_set_name( "NOTE OFF", data->button_death, wm );
	else
	    button_set_name( "CUT", data->button_death, wm );
    }
    else
    {
	scrollbar_set_parameters( data->scroll_volume, 1, 0, 0, 1, wm );
	scrollbar_set_parameters( data->scroll_pan, 1, 0, 0, 1, wm );
	scrollbar_set_parameters( data->scroll_finetune, 1, 0, 0, 1, wm );
	scrollbar_set_parameters( data->scroll_relative, 1, 0, 0, 1, wm );
	scrollbar_set_parameters( data->scroll_vibsweep, 1, 0, 0, 1, wm );
	scrollbar_set_parameters( data->scroll_vibdepth, 1, 0, 0, 1, wm );
	scrollbar_set_parameters( data->scroll_vibrate, 1, 0, 0, 1, wm );
    }
}

int button_iclose_handler( void *user_data, int win, window_manager *wm )
{
    insteditor_data *data = (insteditor_data*)user_data;    
    
    window *w = wm->windows[ win_samples ];
    samples_data *sdata = (samples_data*)w->data;
    unscale_top_win( sdata, wm );
    
    send_event( data->this_window, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
         
    return 0;
}

int scroll_instvol_handler( void *user_data, int win, window_manager *wm )
{
    insteditor_data *data = (insteditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_volume ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;

    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	ins->volume = (uint8_t)sdata->cur;
    }

    return 0;
}

int scroll_instpan_handler( void *user_data, int win, window_manager *wm )
{
    insteditor_data *data = (insteditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_pan ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;

    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	ins->panning = (uint8_t)sdata->cur;
    }

    return 0;
}

int scroll_instrelative_handler( void *user_data, int win, window_manager *wm )
{
    insteditor_data *data = (insteditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_relative ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    
    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	ins->relative_note = (signed char)sdata->cur - 84;
    }

    return 0;
}

int scroll_instfine_handler( void *user_data, int win, window_manager *wm )
{
    insteditor_data *data = (insteditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_finetune ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;

    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	ins->finetune = (signed char)sdata->cur - 128;
    }

    return 0;
}

int scroll_instsweep_handler( void *user_data, int win, window_manager *wm )
{
    insteditor_data *data = (insteditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_vibsweep ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins ) ins->vibrato_sweep = (uint8_t)sdata->cur;
    return 0;
}

int scroll_instdepth_handler( void *user_data, int win, window_manager *wm )
{
    insteditor_data *data = (insteditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_vibdepth ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins ) ins->vibrato_depth = (uint8_t)sdata->cur;
    return 0;
}

int scroll_instrate_handler( void *user_data, int win, window_manager *wm )
{
    insteditor_data *data = (insteditor_data*)user_data;
    window *swin = wm->windows[ data->scroll_vibrate ];
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins ) ins->vibrato_rate = (uint8_t)sdata->cur;
    return 0;
}

int button_death_handler( void *user_data, int win, window_manager *wm )
{
    insteditor_data *data = (insteditor_data*)user_data;    
    instrument *ins = xm.song->instruments[ current_instrument ];
    if( ins ) ins->flags ^= INST_FLAG_NOTE_OFF;
    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    return 0;
}

int insteditor_handler( wm_event *evt, window_manager *wm )
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    insteditor_data *data = (insteditor_data*)win->data;
    int text;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(insteditor_data), "insteditor data" );
	    //Init data:
	    data = (insteditor_data*)win->data;
	    
	    data->this_window = evt->event_win;
	    
	    data->button_close = create_window( "iclose", 1, 1, 2, 2, wm->colors[ 6 ], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_close, "1", "100%-1", "7", "100%-3", wm );
	    button_set_handler( &button_iclose_handler, (void*)data, data->button_close, wm );
	    button_set_name( "CLOSE", data->button_close, wm );

	    data->pars_window = create_window( "window for inst parameters",
		0, 0, 1, 1, win->color, 0, evt->event_win, &child_handler, wm );
	    set_window_string_controls( data->pars_window, "100%-14", "0", "100%", "100%", wm );

	    //VOLUME:
	    NEW_SCROLL_TYPE = 1;
	    data->scroll_volume = create_window( "instvolume", 1, 1, 2, 2, wm->colors[ 6 ], 1, data->pars_window, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_volume, "100%-14", "1", "100%", "3", wm );
	    scrollbar_set_handler( &scroll_instvol_handler, (void*)data, data->scroll_volume, wm );
	    scrollbar_set_parameters( data->scroll_volume, 1, 0x40, 0x40, 20, wm );
	    scrollbar_set_value_showing( data->scroll_volume, 1, 0, 0, wm );
	    scrollbar_set_step( data->scroll_volume, 1, wm );

	    text = create_window( "textinstvol2", 1, 1, 2, 2, win->color, 1, data->pars_window, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "0", "100%", "1", wm );
	    text_set_text( text, "VOLUME:", wm );
	    text_set_readonly( text, 1, wm );

	    //PANNING:
	    NEW_SCROLL_TYPE = 1;
	    data->scroll_pan = create_window( "instpan", 1, 1, 2, 2, wm->colors[ 6 ], 1, data->pars_window, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_pan, "100%-14", "4", "100%", "6", wm );
	    scrollbar_set_handler( &scroll_instpan_handler, (void*)data, data->scroll_pan, wm );
	    scrollbar_set_parameters( data->scroll_pan, 1, 255, 128, 60, wm );
	    scrollbar_set_value_showing( data->scroll_pan, 1, 0, 0, wm );
	    scrollbar_set_step( data->scroll_pan, 1, wm );

	    text = create_window( "textinstpan2", 1, 1, 2, 2, win->color, 1, data->pars_window, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "3", "100%", "4", wm );
	    text_set_text( text, "PAN:", wm );
	    text_set_readonly( text, 1, wm );

	    //FINETUNE:
	    NEW_SCROLL_TYPE = 1;
	    data->scroll_finetune = create_window( "instfine", 1, 1, 2, 2, wm->colors[ 6 ], 1, data->pars_window, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_finetune, "100%-14", "7", "100%", "9", wm );
	    scrollbar_set_handler( &scroll_instfine_handler, (void*)data, data->scroll_finetune, wm );
	    scrollbar_set_parameters( data->scroll_finetune, 1, 31, 16, 8, wm );
	    scrollbar_set_value_showing( data->scroll_finetune, 1, 1, -128, wm );
	    scrollbar_set_step( data->scroll_finetune, 1, wm );

	    text = create_window( "textinstfine2", 1, 1, 2, 2, win->color, 1, data->pars_window, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "6", "100%", "7", wm );
	    text_set_text( text, "FINETUNE:", wm );
	    text_set_readonly( text, 1, wm );

	    //RELATIVE:
	    NEW_SCROLL_TYPE = 1;
	    data->scroll_relative = create_window( "inst_relative_note", 1, 1, 2, 2, wm->colors[ 6 ], 1, data->pars_window, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_relative, "100%-14", "10", "100%", "12", wm );
	    scrollbar_set_handler( &scroll_instrelative_handler, (void*)data, data->scroll_relative, wm );
	    scrollbar_set_value_showing( data->scroll_relative, 1, 1, -84, wm );
	    scrollbar_set_step( data->scroll_relative, 1, wm );

	    text = create_window( "textrelative2", 1, 1, 2, 2, win->color, 1, data->pars_window, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "9", "100%", "10", wm );
	    text_set_text( text, "RELATIVE NOTE:", wm );
	    text_set_readonly( text, 1, wm );

	    //VIBRATO SWEEP:
	    NEW_SCROLL_TYPE = 1;
	    data->scroll_vibsweep = create_window( "inst_sweep", 1, 1, 2, 2, wm->colors[ 6 ], 1, data->pars_window, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_vibsweep, "100%-14", "13", "100%", "15", wm );
	    scrollbar_set_handler( &scroll_instsweep_handler, (void*)data, data->scroll_vibsweep, wm );
	    scrollbar_set_value_showing( data->scroll_vibsweep, 1, 1, 0, wm );
	    scrollbar_set_step( data->scroll_vibsweep, 1, wm );

	    text = create_window( "textrelative2", 1, 1, 2, 2, win->color, 1, data->pars_window, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "12", "100%", "13", wm );
	    text_set_text( text, "VIBRATO SWEEP:", wm );
	    text_set_readonly( text, 1, wm );

	    //VIBRATO DEPTH:
	    NEW_SCROLL_TYPE = 1;
	    data->scroll_vibdepth = create_window( "inst_depth", 1, 1, 2, 2, wm->colors[ 6 ], 1, data->pars_window, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_vibdepth, "100%-14", "16", "100%", "18", wm );
	    scrollbar_set_handler( &scroll_instdepth_handler, (void*)data, data->scroll_vibdepth, wm );
	    scrollbar_set_value_showing( data->scroll_vibdepth, 1, 1, 0, wm );
	    scrollbar_set_step( data->scroll_vibdepth, 1, wm );

	    text = create_window( "textrelative2", 1, 1, 2, 2, win->color, 1, data->pars_window, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "15", "100%", "16", wm );
	    text_set_text( text, "VIBRATO DEPTH:", wm );
	    text_set_readonly( text, 1, wm );

	    //VIBRATO RATE:
	    NEW_SCROLL_TYPE = 1;
	    data->scroll_vibrate = create_window( "inst_rate", 1, 1, 2, 2, wm->colors[ 6 ], 1, data->pars_window, &scrollbar_handler, wm );
	    set_window_string_controls( data->scroll_vibrate, "100%-14", "19", "100%", "21", wm );
	    scrollbar_set_handler( &scroll_instrate_handler, (void*)data, data->scroll_vibrate, wm );
	    scrollbar_set_value_showing( data->scroll_vibrate, 1, 1, 0, wm );
	    scrollbar_set_step( data->scroll_vibrate, 1, wm );

	    text = create_window( "textrelative2", 1, 1, 2, 2, win->color, 1, data->pars_window, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "18", "100%", "19", wm );
	    text_set_text( text, "VIBRATO RATE:", wm );
	    text_set_readonly( text, 1, wm );

	    //NOTE DEATH TYPE:
	    data->button_death = create_window( "ideath", 1, 1, 2, 2, wm->colors[ 6 ], 1, data->pars_window, &button_handler, wm );
	    set_window_string_controls( data->button_death, "100%-14", "22", "100%", "24", wm );
	    button_set_handler( &button_death_handler, (void*)data, data->button_death, wm );
	    button_set_name( "CUT", data->button_death, wm );

	    text = create_window( "textrelative2", 1, 1, 2, 2, win->color, 1, data->pars_window, &text_handler, wm );
	    set_window_string_controls( text, "100%-14", "21", "100%", "22", wm );
	    text_set_text( text, "NEW NOTE ACT:", wm );
	    text_set_readonly( text, 1, wm );

	    create_scrollarea( data->pars_window, wm );
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
		draw_window_box( evt->event_win, wm ); //draw window box
		draw_horis_line( evt->event_win, 0, 0, win->x_size * wm->char_x, get_color( 0, 0, 0 ), 2, wm );
		insteditor_redraw( evt->event_win, wm );
	    }
	    break;
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_box( evt->event_win, wm ); //draw window box
		draw_horis_line( evt->event_win, 0, 0, win->x_size * wm->char_x, get_color( 0, 0, 0 ), 2, wm );
		insteditor_redraw( evt->event_win, wm );
	    }
	    break;
    }
    return 0;
}
