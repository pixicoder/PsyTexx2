/*
    win_list.cpp - list handler
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_button.h"
#include "win_scrollbar.h"
#include "win_text.h"
#include "win_list.h"

void list_set_numbered( int numbered_flag, int offset, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;
    
    data->numbered_flag = numbered_flag;
    data->number_offset = offset;
}

void list_set_editable( int editable_flag, int win_num, window_manager* wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;

    data->editable = (char)editable_flag;
}

void list_set_handler( int (*handler)(void*,int,window_manager*), void *user_data, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;

    data->handler = (int (*)(void*,int,void*))handler;
    data->user_data = user_data;
}

void list_set_edit_handler( int (*handler)(void*,int,window_manager*), void *user_data, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;

    text_set_handler( handler, (void*)data, data->edit_field, wm );
}

void list_draw( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;
    int i = 0, y = 0;
    int color;
    char temp[4];

    if( data->editable )
    {
        //Clean previous edit field:
	window *ewin; //Edit window
        ewin = wm->windows[ data->edit_field ];
        draw_touch_box( data->edit_field, 0, 0, ewin->x_size, ewin->y_size, win_num, wm );
	send_event( data->edit_field, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm );    
	ewin->y_size = 0;
    }

    matrix_draw_box( win_num, 
                     0, 0, 
	             win->x_size - SCROLL_SIZE, 
	             win->y_size, 
	             win->color, wm );
    for( i = data->first_item, y = 0; i < data->items_num && y < win->y_size; y++, i++ )
    {
        color = win->color;
#ifdef TEXTMODE
	if( list_get_attr( i, win_num, wm ) == 0 ) color >>= 2;
#else
        if( list_get_attr( i, win_num, wm ) == 1 ) color = wm->colors[ 10 ];
#endif
	if( i == data->selected_item ) color = wm->colors[ 12 ];//color ^= 0xFFFFFFFF;
	if( (COLOR)color == 0 ) color = 1;
	if( color != win->color )
	{
	    matrix_draw_box( win_num, 0, y, 
		             win->x_size,
		             1,
		             color,
		             wm );
	}
	int x_off = 0;
	int attr = ATR_NONE;
	if( data->numbered_flag )
	{ //Draw number:
	    x_off = 2;
	    if( list_get_attr( i, win_num, wm ) == 2 )
	    {
		matrix_draw_box( win_num, 0, y, 
		                 2, 1,
		                 wm->colors[6],
		                 wm );
		attr = ATR_BOLD;
	    }
	    else
	    {
		matrix_draw_box( win_num, 0, y, 
		                 2, 1,
		                 wm->colors[9],
		                 wm );
	    }
	    ext_int_to_string( i + data->number_offset, temp, 2, wm );
    	    matrix_draw_string( win_num, 
        	                0, y, 
	   	                (uint8_t)attr, 
			        temp,
		                wm );
	}
        matrix_draw_string( win_num,
                            x_off, y, 
	      	            ATR_NONE, 
		            list_get_item( i, win_num, wm ),
		            wm );
    }

    //Set scrollbar parameters:
    scrollbar_set_parameters( data->scrollbar, 0, 
                              data->items_num - win->y_size, 
			      data->first_item, 
			      win->y_size, 
			      wm );
}

void list_clear( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;
    
    data->items_num = 0;
    data->first_item = 0;
}

void list_reset_selection( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;
    data->selected_item = -1;
}

void list_add_item( const char *item, char attr, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;
    int ptr, p;

    mem_off();
    
    if( data->items_num == 0 ) 
    {
	data->items_ptr[ 0 ] = 0;
	ptr = 0;
    }
    else 
    {
	ptr = data->items_ptr[ data->items_num ];
    }
    
    data->items[ ptr++ ] = attr | 128;
    for( p = 0; ; p++, ptr++ )
    {
	data->items[ ptr ] = item[ p ];
	if( item[ p ] == 0 ) break;
    }
    ptr++;
    data->items_num++;
    data->items_ptr[ data->items_num ] = ptr;
    
    mem_on();
}

void list_delete_item( int item_num, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;
    int ptr, p;

    if( item_num < data->items_num && item_num >= 0 )
    {
	mem_off();
	
	ptr = data->items_ptr[ item_num ]; //Offset of our item (in the "items")
	
	//Get item size (in chars):
	int size;
	for( size = 0; ; size++ )
	{
	    if( data->items[ ptr + size ] == 0 ) break;
	}
	
	//Delete it:
	for( p = ptr; p < MAX_SIZE - size - 1 ; p++ )
	{
	    data->items[ p ] = data->items[ p + size + 1 ];
	}
	for( p = 0; p < data->items_num; p++ )
	{
	    if( data->items_ptr[ p ] > ptr ) data->items_ptr[ p ] -= size + 1;
	}
	for( p = item_num; p < data->items_num; p++ )
	{
	    if( p + 1 < MAX_ITEMS ) 
		data->items_ptr[ p ] = data->items_ptr[ p + 1 ];
	    else
		data->items_ptr[ p ] = 0;
	}
	data->items_num--;
	if( data->items_num < 0 ) data->items_num = 0;

	if( data->selected_item >= data->items_num ) data->selected_item = data->items_num - 1;
	
	mem_on();
    }
}

void list_move_item_up( int item_num, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;

    if( item_num < data->items_num && item_num >= 0 )
    {
	mem_off();
	if( item_num != 0 )
	{
	    int temp = data->items_ptr[ item_num - 1 ];
	    data->items_ptr[ item_num - 1 ] = data->items_ptr[ item_num ];
	    data->items_ptr[ item_num ] = temp;
	    if( item_num == data->selected_item ) data->selected_item--;
	}
	mem_on();
    }
}	

void list_move_item_down( int item_num, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;

    if( item_num < data->items_num && item_num >= 0 )
    {
	mem_off();
	if( item_num != data->items_num - 1 )
	{
	    int temp = data->items_ptr[ item_num + 1 ];
	    data->items_ptr[ item_num + 1 ] = data->items_ptr[ item_num ];
	    data->items_ptr[ item_num ] = temp;
	    if( item_num == data->selected_item ) data->selected_item++;
	}
	mem_on();
    }
}	

char* list_get_item(int item_num, int win_num, window_manager *wm)
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;
    if( item_num >= data->items_num ) return 0;
    if( item_num >= 0 )
	return data->items + data->items_ptr[ item_num ] + 1;
    else 
	return 0;
}

char list_get_attr(int item_num, int win_num, window_manager *wm)
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;
    if( item_num >= data->items_num ) return 0;
    if( item_num >= 0 )
	return data->items[ data->items_ptr[ item_num ] ] & 127;
    else
	return 0;
}

int list_get_selected_num(int win_num, window_manager *wm)
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;
    return data->selected_item;
}

//Return values:
//1 - item1 > item2
//0 - item1 <= item2
int list_compare_items( int item1, int item2, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;

    char *i1 = data->items + data->items_ptr[ item1 ];
    char *i2 = data->items + data->items_ptr[ item2 ];
    char a1 = i1[ 0 ] & 127;
    char a2 = i2[ 0 ] & 127;
    i1++;
    i2++;
    
    int retval = 0;
    
    //Compare:
    if( a1 != a2 )
    {
	if( a1 == 1 ) retval = 0;
	if( a2 == 1 ) retval = 1;
    }
    else
    {
	for( int a = 0; ; a++ )
	{
	    if( i1[ a ] == 0 ) break;
	    if( i2[ a ] == 0 ) break;
	    if( i1[ a ] < i2[ a ] ) { break; }             //item1 < item2
	    if( i1[ a ] > i2[ a ] ) { retval = 1; break; } //item1 > item2
	}
    }
    
    return retval;
}

void list_sort( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    list_data *data = (list_data*)win->data;

    mem_off();
    for(;;)
    {
	int s = 0;
	for( int a = 0; a < data->items_num - 1; a++ )
	{
	    if( list_compare_items( a, a + 1, win_num, wm ) )
	    {
		s = 1;
		int temp = data->items_ptr[ a + 1 ];
		data->items_ptr[ a + 1 ] = data->items_ptr[ a ];
		data->items_ptr[ a ] = temp;
	    }
	}
	if( s == 0 ) break;
    }
    mem_on();
}

void list_make_selection_visible( int win_num, window_manager *wm )
{
    window *lwin = wm->windows[ win_num ];
    list_data *ldata = (list_data*)lwin->data;
    if( ldata->selected_item < ldata->first_item || ldata->selected_item >= ldata->first_item + lwin->y_size )
    {
        ldata->first_item = ldata->selected_item - ( lwin->y_size >> 1 );
	if( ldata->first_item + lwin->y_size > ldata->items_num ) ldata->first_item = ldata->items_num - lwin->y_size;
        if( ldata->first_item < 0 ) ldata->first_item = 0;
    }													
}

int scroll_handler( void *user_data, int scroll_window, window_manager *wm )
{
    window *swin = wm->windows[ scroll_window ]; //Our window
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    list_data *data = (list_data*)user_data;

    //Redraw list window
    data->first_item = sdata->cur;
    list_draw( data->this_window, wm );
    
    return 0;
}

int list_handler(wm_event *evt, window_manager *wm)
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    list_data *data = (list_data*)win->data;
    int retval = 0;

    window *ewin; //Edit window
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    //Create data:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(list_data), "list data" );
	    //Init data:
	    mem_off();
	    data = (list_data*)win->data;
	    data->this_window = evt->event_win;
	    data->items = (char*)mem_new( HEAP_STORAGE, MAX_SIZE, "list items" );
	    data->items_ptr = (int*)mem_new( HEAP_STORAGE, MAX_ITEMS * 4, "list items ptrs" );
	    data->items_num = 0;
	    data->selected_item = -1;
	    data->first_item = 0;
	    data->handler = 0;
	    data->numbered_flag = 0;
	    data->number_offset = 0;
	    data->edit_field = 0;
	    data->editable = 0;
	    //Create buttons:
	    data->scrollbar = create_window( "listscroll",
	                                     win->x_size - SCROLL_SIZE, 0, SCROLL_SIZE, win->y_size, wm->colors[6], 0,
	                                     evt->event_win, &scrollbar_handler, wm );
	    set_window_string_controls( data->scrollbar, "100%-2", "0", "100%", "100%", wm );
	    scrollbar_set_handler( &scroll_handler, (void*)data, data->scrollbar, wm );
	    
            data->edit_field = create_window( "listedit", 2, 0, win->x_size - 4, 0, wm->colors[10], 1, evt->event_win, &text_handler, wm );
	    												  
	    mem_on();
	    break;
	case EVT_BEFORECLOSE:
	    //Clear data:
	    mem_off();
	    //Delete data:
	    if( win->data ) 
	    {
		mem_free( data->items );
		mem_free( data->items_ptr );
		mem_free( win->data );
	    }
	    mem_on();
	    break;
	case EVT_BUTTONUP:
	    if( evt->button & BUTTON_LEFT )
	    {
		data->selected_item = ( evt->y / wm->char_y ) + data->first_item;
		if( data->selected_item < 0 ) data->selected_item = 0;
		if( data->selected_item >= data->items_num ) data->selected_item = -1;
		else 
		{
		    if( data->handler ) data->handler( data->user_data, evt->event_win, (void*)wm );
		    if( data->editable && data->selected_item != -1 )
		    {
			//Show edit field:
			ewin = wm->windows[ data->edit_field ];
			//Clean previous edit field:
			draw_touch_box( data->edit_field, 0, 0, ewin->x_size, ewin->y_size, evt->event_win, wm );
			//Get new coordinates:
			if( data->numbered_flag ) 
			    ewin->x = 2;
			else
			    ewin->x = 0;
			ewin->y = evt->y / wm->char_y;
			ewin->x_size = win->x_size - 2 - ewin->x;
			ewin->y_size = 1;
			calculate_real_window_position( data->edit_field, wm );
			if( list_get_item( data->selected_item, evt->event_win, wm ) )
			    text_set_text( data->edit_field, list_get_item( data->selected_item, evt->event_win, wm ), wm );
			else
			    text_set_text( data->edit_field, "", wm );
			send_event( data->edit_field, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );    
		    }
		}
	    }
	    break;
	case EVT_BUTTONDOWN:
	    if( data->editable )
	    {
	        //Clean previous edit field:
	        ewin = wm->windows[ data->edit_field ];
	        draw_touch_box( data->edit_field, 0, 0, ewin->x_size, ewin->y_size, evt->event_win, wm );
	    }
	case EVT_MOUSEMOVE:
	    if( evt->button & BUTTON_LEFT )
	    {
		if( (evt->y / wm->char_y) <= 0 )
		{ //Scroll up:
		    if( data->first_item >= 1 ) data->first_item--;
		    scrollbar_set_cur_value( data->scrollbar, data->first_item, wm );
		    send_event( data->scrollbar, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		}
		if( (evt->y / wm->char_y) >= win->y_size )
		{ //Scroll down:
		    if( data->first_item < data->items_num - win->y_size ) data->first_item++;
		    scrollbar_set_cur_value( data->scrollbar, data->first_item, wm );
		    send_event( data->scrollbar, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		    evt->y = (win->y_size - 1) * wm->char_y;
		}
		data->selected_item = ( evt->y / wm->char_y ) + data->first_item;
		if( data->selected_item < 0 ) data->selected_item = 0;
		if( data->selected_item >= data->items_num ) data->selected_item = -1;
		list_draw( evt->event_win, wm );
		break;
	    }
	    if( evt->button >> 3 )
	    {
		switch( evt->button >> 3 )
		{
		    case KEY_SCROLLUP:
		    if( data->first_item >= 1 ) data->first_item--;
		    scrollbar_set_cur_value( data->scrollbar, data->first_item, wm );
		    send_event( data->scrollbar, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		    retval = 1;
		    break;
		    
		    case KEY_SCROLLDOWN:
		    if( data->first_item < data->items_num - win->y_size ) data->first_item++;
		    scrollbar_set_cur_value( data->scrollbar, data->first_item, wm );
		    send_event( data->scrollbar, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		    retval = 1;
		    break;
		}
		list_draw( evt->event_win, wm );
	    }
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
		list_draw( evt->event_win, wm );       //draw list
	    }
	    break;
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_box( evt->event_win, wm ); //draw window box
		list_draw( evt->event_win, wm );       //draw list
	    }
	    break;
    }
    return retval;
}
