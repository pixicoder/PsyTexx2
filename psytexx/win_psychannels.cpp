/*
    win_psychannels.cpp - channels (scopes) handler
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_psychannels.h"
#include "../xm/xm.h"

char small_font[ 3 * 5 * 11 ] =
{
    1,1,1,
    1,0,1,
    1,0,1,
    1,0,1,
    1,1,1,
    
    1,1,0,
    0,1,0,
    0,1,0,
    0,1,0,
    1,1,1,
    
    1,1,1,
    0,0,1,
    1,1,1,
    1,0,0,
    1,1,1,
    
    1,1,1,
    0,0,1,
    0,1,1,
    0,0,1,
    1,1,1,
    
    1,0,1,
    1,0,1,
    1,1,1,
    0,0,1,
    0,0,1,
    
    1,1,1,
    1,0,0,
    1,1,1,
    0,0,1,
    1,1,1,
    
    1,1,1,
    1,0,0,
    1,1,1,
    1,0,1,
    1,1,1,
    
    1,1,1,
    0,0,1,
    0,0,1,
    0,0,1,
    0,0,1,
    
    1,1,1,
    1,1,1,
    1,1,1,
    1,1,1,
    1,1,1,
    
    1,1,1,
    1,0,1,
    1,1,1,
    0,0,1,
    1,1,1,

    1,1,1,
    1,0,0,
    1,1,1,
    1,0,0,
    1,0,0
};

void draw_small_char( int win_num, int x, int y, int c, int color, window_manager *wm )
{
#ifndef TEXTMODE
    window *win = wm->windows[ win_num ];
    int wx = win->real_x;
    int wy = win->real_y;
    x += ( wx * wm->char_x ); y += ( wy * wm->char_y );
    if( x < 0 ) return;
    if( y < 0 ) return;
    if( x >= wm->pscreen_x ) return;
    if( y >= wm->pscreen_y ) return;
    int pnt = ( ( y / wm->char_y ) * wm->screen_x ) + ( x / wm->char_x );
    int g_pnt = ( y * wm->pscreen_x ) + x;
    
    if( c == 0 ) return;
    c -= '0';
    c *= 15;
    
    if( wm->t_screen[ pnt ] == win_num )
    {
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt ++;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt ++;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt += wm->pscreen_x - 2;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt ++;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt ++;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt += wm->pscreen_x - 2;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt ++;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt ++;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt += wm->pscreen_x - 2;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt ++;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt ++;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt += wm->pscreen_x - 2;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt ++;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt ++;
	if( small_font[ c++ ] ) wm->g_screen[ g_pnt ] = color; g_pnt += wm->pscreen_x - 2;
    }
#endif
}

void channels_redraw( int win_num, window_manager* wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    channels_data *data = (channels_data*)win->data;
    char temp_str[ 8 ];

    //draw_window_box( win_num, wm ); //draw window box
    
    int xsize = win->x_size * wm->char_x; //X-size of the window
    int c = xm.song->channels;            //Number of channels
    if( c & 1 ) c++;
    int csize = (xsize << 8 ) / ( c / 2 );//Size of one channel (real = csize >> 8)
    int real_csize = csize >> 8;
    int maxvalue = wm->char_y * 2;        //Max channel value
    int value;                            //Current channel value
    int value2;
    int value3;
    int cur_channel = 0;                  //Current channel number
    int x;

    pdraw_box( win_num, 0, 0, win->x_size * wm->char_x, win->y_size * wm->char_y, win->color, wm );
/*
    for( x = 0; x < 256; x++ )
    {
	value = xm.eq[ x ];
	value = ( maxvalue * value ) >> 8;
	pdraw_box( win_num, x, maxvalue - value, 1, value, 0, wm );
    }
    return;
*/
    
    int sc;     //subchannel number
    int sc_val; //subchannel value
    int sc_val_l;
    int sc_val_r;
    for( int yoff = 0; yoff < maxvalue * 2; yoff += maxvalue )
    for( x = 0; x < xsize << 8; x += csize )
    {
	//Draw background:
	if( xm.channels[ cur_channel ]->enable )
	    pdraw_box( win_num, x >> 8, yoff, real_csize, maxvalue, win->color, wm );
	else
	    pdraw_box( win_num, x >> 8, yoff, real_csize, maxvalue, 1, wm );
    
	//Draw left channel:
	sc_val = 0;
	for( sc = 0; sc < SUBCHANNELS; sc++ )
	    sc_val += xm.channels[ cur_channel + sc ]->lvalue;
	sc_val_l = sc_val;
	value = ( sc_val * maxvalue ) >> 14;
	if( value >= maxvalue ) value = maxvalue - 1;
	value2 = sc_val >> 10; if( value2 > 15 ) value2 = 15; value2 = 15 - value2;
	pdraw_box( win_num, x >> 8, maxvalue - value + yoff, ( real_csize >> 1 ), value, wm->colors[ value2 ], wm );

	//Draw right channel:
	sc_val = 0;
	for( sc = 0; sc < SUBCHANNELS; sc++ )
	    sc_val += xm.channels[ cur_channel + sc ]->rvalue;
	sc_val_r = sc_val;
	value = ( sc_val * maxvalue ) >> 14;
	if( value >= maxvalue ) value = maxvalue - 1;
	value3 = value2;
	value2 = sc_val >> 10; if( value2 > 15 ) value2 = 15; value2 = 15 - value2;
	value3 += value2;
	value3 >>= 1;
	value3 -= 8;
	if( value3 < 0 ) value3 = 0;
	pdraw_box( win_num, ( x >> 8 ) + ( real_csize >> 1 ), maxvalue - value + yoff, ( real_csize >> 1 ) - 1, value, wm->colors[ value2 ], wm );

	//Draw wave:
#if SCOPE_SIZE > 4
	if( sc_val_l || sc_val_r )
	{
	    int sx = 0;
	    for( int xx = 1; xx < real_csize - 1; xx++ )
	    {
		if( sx >= SCOPE_SIZE ) break;

		sc_val = 0;
		for( sc = 0; sc < SUBCHANNELS; sc++ )
		    sc_val += xm.channels[ cur_channel + sc ]->scope[ sx ];

		value = ( sc_val * ( maxvalue >> 1 ) ) >> 7;
		draw_pixel( win_num, ( x >> 8 ) + xx, ( maxvalue >> 1 ) - value + yoff, wm->colors[ value3 ], wm );
		if( c > 6 ) sx += 2; else sx++;
	    }
	}
#endif

	//Draw channel name:
	if( xm.channels[ cur_channel ]->enable )
	{
	    int_to_string( cur_channel / SUBCHANNELS, temp_str );
	    draw_small_char( win_num, ( x >> 8 ) + 1, 1 + yoff, temp_str[ 0 ], wm->colors[ 0 ], wm );
	    draw_small_char( win_num, ( x >> 8 ) + 1 + 4, 1 + yoff, temp_str[ 1 ], wm->colors[ 0 ], wm );
	}
	else
	{
	    draw_small_char( win_num, ( x >> 8 ) + 1, 1 + yoff, '0', wm->colors[ 0 ], wm );
	    draw_small_char( win_num, ( x >> 8 ) + 1 + 4, 1 + yoff, '0'+10, wm->colors[ 0 ], wm );
	    draw_small_char( win_num, ( x >> 8 ) + 1 + 8, 1 + yoff, '0'+10, wm->colors[ 0 ], wm );
	}
	
	//Draw channel activity:
	if( sc_val_l || sc_val_r )
	    draw_pixel( win_num, ( x >> 8 ) + 1, maxvalue - 2 + yoff, wm->colors[ 0 ], wm );
		
	cur_channel += SUBCHANNELS;
	if( cur_channel / SUBCHANNELS == c / 2 ) break;
	if( cur_channel / SUBCHANNELS >= c ) break;
    }
}

int channels_handler( wm_event *evt, window_manager *wm )
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    channels_data *data = (channels_data*)win->data;
    int chan_number;
    int disabled_channels;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(channels_data), "channels data" );
	    //Init data:
	    data = (channels_data*)win->data;
	    data->this_window = evt->event_win;
	    data->mouse_button = 0;
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
		channels_redraw( evt->event_win, wm );
	    }
	    break;

	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		channels_redraw( evt->event_win, wm );
	    }
	    break;

	case EVT_BUTTONDOWN:
	    if( evt->button & BUTTON_LEFT ) data->mouse_button |= 1;
	    if( evt->button & BUTTON_RIGHT ) data->mouse_button |= 2;
	    if( evt->button & BUTTON_RIGHT )
	    {
	        //Make solo channel:
	        disabled_channels = 0;
	        for( chan_number = 0; chan_number < xm.song->channels; chan_number++ )
	    	    if( xm.channels[ chan_number ]->enable == 0 ) disabled_channels = 1;
		for( chan_number = 0; chan_number < xm.song->channels * SUBCHANNELS; chan_number++ )
		    xm.channels[ chan_number ]->enable = disabled_channels;
		if( disabled_channels == 0 ) 
		{
		    //Make one solo channel:
		    chan_number = ( ( evt->x << 8 ) / ( win->x_size * wm->char_x ) ) * ( xm.song->channels / 2 );
		    chan_number >>= 8;
		    if( evt->y > ( ( win->y_size / 2 ) * wm->char_y ) ) chan_number += xm.song->channels / 2;
		    int sc;
		    for( sc = 0; sc < SUBCHANNELS; sc++ )
			xm.channels[ chan_number * SUBCHANNELS + sc ]->enable ^= 1;
		}
	    }
	    if( evt->button & BUTTON_LEFT )
	    {
	        //ON/OFF one channel:
	        chan_number = ( ( evt->x << 8 ) / ( win->x_size * wm->char_x ) ) * ( xm.song->channels / 2 );
	        chan_number >>= 8;
	        if( evt->y > ( ( win->y_size / 2 ) * wm->char_y ) ) chan_number += xm.song->channels / 2;
		int sc;
		for( sc = 0; sc < SUBCHANNELS; sc++ )
		    xm.channels[ chan_number * SUBCHANNELS + sc ]->enable ^= 1;
	    }
	    break;

	case EVT_BUTTONUP:
	    if( evt->button & BUTTON_LEFT ) data->mouse_button &= 0xFE;
	    if( evt->button & BUTTON_RIGHT ) data->mouse_button &= 0xFD;
	    break;
    }
    return 0;
}
