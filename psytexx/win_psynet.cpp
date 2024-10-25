/*
    win_psynet.cpp - PsyTexx Synth Sound Net (window handler)
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_button.h"
#include "win_list.h"
#include "win_psynet.h"
#include "win_scrollbar.h"
#include "win_psymenu.h"
#include "win_text.h"
#include "../xm/xm.h"

extern size_t psytexx_echo( PSYTEXX_SYNTH_PARAMETERS );
size_t ( *synths [] )( PSYTEXX_SYNTH_PARAMETERS ) = 
{
    &psytexx_echo,
    0
};
int synths_num = 0;

int net_redraw( net_data *data, int mx, int my, window_manager *wm )
{
    int retval = -1; //Selected item

    if( data->list_window_opened ) return -1;

    draw_window_box( data->this_window, wm ); //draw window box
    send_event( data->win_menu, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    send_event( data->new_button, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );

    int i;
    //Draw all links:
    for( i = 0; i < xm.pnet->items_num; i++ )
    {
	if( xm.pnet->items[ i ].flags )
	{
	    int x = ( ( xm.pnet->items[ i ].x + data->offset_x ) * wm->pscreen_x ) >> 10;
	    int y = ( ( xm.pnet->items[ i ].y + data->offset_y ) * wm->pscreen_y ) >> 10;

	    //There are links?
	    if( xm.pnet->items[ i ].input_num > 0 )
	    {
		for( int l = 0; l < xm.pnet->items[ i ].input_num; l++ )
		{
		    if( xm.pnet->items[ i ].input_links[ l ] >= 0 )
		    {
			int i2 = xm.pnet->items[ i ].input_links[ l ];
			if( i2 < xm.pnet->items_num && xm.pnet->items[ i2 ].flags )
			{
			    int x2 = ( ( xm.pnet->items[ i2 ].x + data->offset_x ) * wm->pscreen_x ) >> 10;
			    int y2 = ( ( xm.pnet->items[ i2 ].y + data->offset_y ) * wm->pscreen_y ) >> 10;
			    draw_line( data->this_window, x, y, x2, y2, 1, wm );
			}
		    }
		}
	    }
	}
    }

    //Draw all net items (synths/effects):
    for( i = 0; i < xm.pnet->items_num; i++ )
    {
	if( xm.pnet->items[ i ].flags )
	{
	    int xsize = 50;
	    int ysize = 30;
	    int x = ( ( xm.pnet->items[ i ].x + data->offset_x ) * wm->pscreen_x ) >> 10;
	    int y = ( ( xm.pnet->items[ i ].y + data->offset_y ) * wm->pscreen_y ) >> 10;
	    x -= xsize / 2;
	    y -= ysize / 2;

	    if( mx >= x && mx < x + xsize && my >= y && my < y + ysize )
		retval = i;
	    if( data->selected_item == i )
	    {
		//Selected:
		pdraw_box( data->this_window, x, y, xsize, ysize, wm->colors[ 8 ], wm ); 
	    }
	    else
	    {
		pdraw_box( data->this_window, x, y, xsize, ysize, wm->colors[ 6 ], wm ); 
	    }
	    //Draw name:
	    draw_string( data->this_window, x + 2, y + 2, wm->colors[ 15 ], xm.pnet->items[ i ].item_name, wm );
	}
    }

    return retval;
}

int new_button_handler( void *user_data, int button_win, window_manager *wm )
{
    net_data *data = (net_data*)user_data;
    set_window_string_controls( data->list_window, "20%", "0", "100%-6", "100%", wm );
    resize_all_windows( wm );
    send_event( data->this_window, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    data->list_window_opened = 1;
    return 0;
}

int item_ok_button_handler( void *user_data, int button_win, window_manager *wm )
{
    net_data *data = (net_data*)user_data;

    //Hide window:
    set_window_string_controls( data->list_window, "0", "0", "0", "0", wm );
    resize_all_windows( wm );
    send_event( data->this_window, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );

    //Create new synth/effect:
    int s = list_get_selected_num( data->items_list, wm );
    if( s >= synths_num )
    {
	psynth_add_synth( 0, 
	    "INSTR", 
	    PSYNTH_FLAG_EXTERNAL_INSTRUMENT, 
	    data->new_item_x, 
	    data->new_item_y, 
	    s - synths_num, 
	    xm.pnet 
	);
    }
    else
    {
	if( s >= 0 )
	psynth_add_synth( synths[ s ], 
	    "SYNTH", 
	    0, 
	    data->new_item_x, 
	    data->new_item_y, 
	    0, 
	    xm.pnet 
	);
    }

    data->list_window_opened = 0;

    return 0;
}

int item_cancel_button_handler( void *user_data, int button_win, window_manager *wm )
{
    net_data *data = (net_data*)user_data;
    set_window_string_controls( data->list_window, "0%", "0", "0", "0%", wm );
    resize_all_windows( wm );
    send_event( data->this_window, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    data->list_window_opened = 0;
    return 0;
}

int net_handler( wm_event *evt, window_manager *wm )
{
    int retval = 0;
    window *win = wm->windows[ evt->event_win ]; //Our window
    net_data *data = (net_data*)win->data;
    int text;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(net_data), "config data" );
	    //Init data:
	    data = (net_data*)win->data;
	    
	    data->this_window = evt->event_win;
	    data->selected_item = -1;
	    data->offset_x = 0;
	    data->offset_y = 0;
	    data->drag_started = 0;
	    data->link_drag = 0;
	    data->new_item_x = 512;
	    data->new_item_y = 512;
	    data->list_window_opened = 0;
	    
	    //Create window with controllers:
	    data->ctls_window = create_window( "window for controllers",
		0, 0, 1, 1, win->color, 1, evt->event_win, &child_handler, wm );
	    set_window_string_controls( data->ctls_window, "0", "0", "20%", "100%", wm );

	    //Create menu window:
	    data->win_menu = create_window( "menu2",
		1, 0, 1, 1, wm->colors[3], 1, evt->event_win, &menu_handler, wm );
	    set_window_string_controls( data->win_menu, "100%-6", "0", "100%", "14", wm );

	    //Button "ADD":
    	    data->new_button = create_window( "add synth button", 0, 0, 1, 1, wm->colors[10], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "NEW", data->new_button, wm );
	    set_window_string_controls( data->new_button, "100%-6", "100%-3", "100%", "100%", wm );
	    button_set_handler( &new_button_handler, (void*)data, data->new_button, wm );


	    
	    //Create window with list of synths:
	    data->list_window = create_window( "list of synths",
		0, 0, 1, 1, win->color, 1, evt->event_win, &child_handler, wm );
	    set_window_string_controls( data->list_window, "0", "0", "0", "0", wm );

	    text = create_window( "name: list_of_synths", 0, 0, 1, 1, wm->colors[5], 1, data->list_window, &button_handler, wm );
	    button_set_name( "SELECT NEW SYNTH:", text, wm );
	    set_window_string_controls( text, "0", "0", "100%", "3", wm );

	    //List of net items:
	    data->items_list = create_window( "list_synths", 0, 0, 1, 1, win->color, 1, data->list_window, &list_handler, wm );
	    set_window_string_controls( data->items_list, "0", "3", "100%", "100%-3", wm );
	    synths_num = 0;
	    for( int sn = 0;; sn++ )
	    {
		if( synths[ sn ] == 0 ) break;
		char *synth_name = (char*)synths[ sn ]( 0, 0, 0, 0, 0, COMMAND_GET_SYNTH_NAME, 0 );
		list_add_item( synth_name, 0, data->items_list, wm );
		synths_num++;
	    }
	    for( int aa = 1; aa <= 0x80; aa ++ )
	    {
		char str[ 16 ];
		str[ 0 ] = 0;
		mem_strcat( str, "Instr. #01" );
		str[ 8 ] = aa >> 4;
		if( str[ 8 ] < 10 ) str[ 8 ] += '0'; else str[ 8 ] = str[ 8 ] - 10 + 'A';
		str[ 9 ] = aa & 15;
		if( str[ 9 ] < 10 ) str[ 9 ] += '0'; else str[ 9 ] = str[ 9 ] - 10 + 'A';
		list_add_item( str, 0, data->items_list, wm );
	    }

	    //OK:
	    data->items_ok = create_window( "select_synth_ok", 0, 0, 1, 1, wm->colors[5], 1, data->list_window, &button_handler, wm );
	    button_set_name( "OK", data->items_ok, wm );
	    set_window_string_controls( data->items_ok, "0", "100%-3", "50%", "100%", wm );
	    button_set_handler( &item_ok_button_handler, (void*)data, data->items_ok, wm );

	    //CANCEL:
	    data->items_cancel = create_window( "select_synth_cancel", 0, 0, 1, 1, wm->colors[5], 1, data->list_window, &button_handler, wm );
	    button_set_name( "CANCEL", data->items_cancel, wm );
	    set_window_string_controls( data->items_cancel, "50%", "100%-3", "100%", "100%", wm );
	    button_set_handler( &item_cancel_button_handler, (void*)data, data->items_cancel, wm );

	    retval = 1;
	    break;
	    
	case EVT_BEFORECLOSE:
	    if( win->data ) mem_free( win->data );
	    retval = 1;
	    break;
	case EVT_SHOW: 
	    //Show window:
	    win->visible = 1; //Make it visible
	    send_event( evt->event_win, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Send DRAW event to all childs
	    retval = 1;
	    break;
	case EVT_HIDE:
	    win->visible = 0;
	    retval = 1;
	    break;
	case EVT_DRAW: 
	    //Draw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_touch_area( evt->event_win, wm );
		net_redraw( data, 0, 0, wm );
	    }
	    retval = 1;
	    break;
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		net_redraw( data, 0, 0, wm );
	    }
	    retval = 1;
	    break;

	case EVT_BUTTONDOWN:
	    if( evt->button & BUTTON_RIGHT )
	    {
		//New:
		data->new_item_x = ( evt->x << 10 ) / wm->pscreen_x;
		data->new_item_y = ( evt->y << 10 ) / wm->pscreen_y;
		new_button_handler( data, data->new_button, wm );
		retval = 1;
	    }
	    if( evt->button & BUTTON_LEFT )
	    {
		data->selected_item = net_redraw( data, evt->x, evt->y, wm );
		data->drag_start_x = evt->x;
		data->drag_start_y = evt->y;
		if( data->selected_item >= 0 && data->selected_item < xm.pnet->items_num )
		{
		    data->drag_item_x = xm.pnet->items[ data->selected_item ].x;
		    data->drag_item_y = xm.pnet->items[ data->selected_item ].y;
		}
		else
		{
		    data->drag_item_x = data->offset_x;
		    data->drag_item_y = data->offset_y;
		}
		data->drag_started = 1;
		net_redraw( data, 0, 0, wm );
		retval = 1;
	    }
	    if( ( evt->button >> 3 ) == KEY_DELETE )
	    {
		if( data->selected_item > 0 && data->selected_item < xm.pnet->items_num )
		{
		    //Delete selected item:
		    psynth_remove_synth( data->selected_item, xm.pnet );
		}
	    }
	    break;
	case EVT_BUTTONUP:
	    if( evt->button & BUTTON_LEFT )
	    {
		if( data->link_drag )
		{
		    int s = net_redraw( data, evt->x, evt->y, wm );
		    if( s != data->selected_item )
		    if( data->selected_item >= 0 && data->selected_item < xm.pnet->items_num )
		    if( s >= 0 && s < xm.pnet->items_num )
		    {
			//We need to make link from one item to another:
			psynth_make_link( s, data->selected_item, xm.pnet );
		    }
		}
		data->link_drag = 0;
		data->drag_started = 0;
		retval = 1;
	    }
	    net_redraw( data, evt->x, evt->y, wm );
	    break;
	case EVT_MOUSEMOVE:
	    if( ( evt->button & BUTTON_LEFT ) && data->drag_started )
	    {
		if( ( ( evt->button >> 3 ) & KEY_SHIFT ) || data->link_drag )
		{
		    data->link_drag = 1;
		    if( data->selected_item >= 0 && data->selected_item < xm.pnet->items_num )
		    {
			net_redraw( data, 0, 0, wm );
			int x1 = ( data->drag_item_x * wm->pscreen_x ) >> 10;
			int y1 = ( data->drag_item_y * wm->pscreen_y ) >> 10;
			draw_line( data->this_window, x1, y1, evt->x, evt->y, 1, wm );
		    }
		}
		else
		{
		    if( data->selected_item >= 0 && data->selected_item < xm.pnet->items_num )
		    {
			int dx = evt->x - data->drag_start_x;
			int dy = evt->y - data->drag_start_y;
			dx <<= 10; dx /= wm->pscreen_x;
			dy <<= 10; dy /= wm->pscreen_y;
			xm.pnet->items[ data->selected_item ].x = data->drag_item_x + dx;
			xm.pnet->items[ data->selected_item ].y = data->drag_item_y + dy;
			net_redraw( data, 0, 0, wm );
		    }
		    if( data->selected_item == -1 )
		    {
			//Move viewport:
			int dx = evt->x - data->drag_start_x;
			int dy = evt->y - data->drag_start_y;
			dx <<= 10; dx /= wm->pscreen_x;
			dy <<= 10; dy /= wm->pscreen_y;
			data->offset_x = data->drag_item_x + dx;
			data->offset_y = data->drag_item_y + dy;
		    }
		    net_redraw( data, 0, 0, wm );
		}
		retval = 1;
	    }
	    break;
    }
    return retval;
}
