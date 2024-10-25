/*
    win_psytable.cpp - PsyTexx universal table
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_button.h"
#include "win_scrollbar.h"
#include "win_psytable.h"
#include "win_psysamples.h"
#include "win_psypattern.h"
#include "win_text.h"
#include "win_dialog.h"
#include "../xm/xm.h"
#include "../sound/sound.h"

int CREATE_TABLE_WITH_EDIT_BUTTON = 0;

#define copy_buffer_size 128000
#define copy_small_buffer_size 1024

int table_type = TYPE_HSCROLL | TYPE_VSCROLL;
uint8_t element_xsize[ ELEMENT_LAST ] = { 0, 3, 3, 2, 2, 1, 2, 1 };
uint8_t element_mono[ ELEMENT_LAST ] = { 0, 0, 1, 0, 0, 0, 0, 0 }; //Is element monolith? (like the NOTE)
uint16_t element_values[ ELEMENT_LAST ] = { //Number of element values 
    0,   //NONE
    256, //PATTERN_NUM
    98,  //NOTE
    129, //INSTRUMENT  (from 1 to 128)
    256, //VOLUME
    32,  //EFFECT
    256, //PARAMETER
};
int field_current_value[ FIELD_LAST ] = { 0 }; //This table is used when drawing. There are not real values!
char temp_str[16]; //temp string
const char *notes = "C-C#D-D#E-F-F#G-G#A-A#B-";
const char *hex1 = "0123456789ABCDEF";
const char *hex2 = "01234-+DUSVPRLM"; //VOLUME
const char *hex3 = "0123456789ABCDEFGHI.KL..P.R.T..X"; //EFFECT NUMBER
char keynotes_table[ 256 ]; //keynotes_table[ key code ] = note number ( -1 = no note )
int16_t key_channel[ 256 ];   //Number of channel for each pressed key ( -1 = no channel )

//Current song:
module *current_song;
pattern *current_pattern;
xmnote *current_pattern_data;
int current_number_of_channels;
int real_pattern_channels;
int current_channel = 0;
int current_pattern_num = 0;
int current_note = 0; //for pianoroll?
//============

int v_scroll_handler( void *user_data, int scroll_win, window_manager *wm )
{
    window *win = wm->windows[ scroll_win ];
    scrollbar_data *data = (scrollbar_data*)win->data;
    table_data *tdata = (table_data*)user_data;
    
    table_field_set_value( tdata->vertical_field, data->cur ); //Set new value
    tdata->scroll_flag = 1;
    table_draw( tdata->this_window, wm ); //Redraw

    return 0;
}

int h_scroll_handler( void *user_data, int scroll_win, window_manager *wm )
{
    window *win = wm->windows[ scroll_win ];
    scrollbar_data *data = (scrollbar_data*)win->data;
    table_data *tdata = (table_data*)user_data;
    
    table_field_set_value( tdata->horisontal_field, data->cur ); //Set new value
    tdata->scroll_flag = 1;
    table_draw( tdata->this_window, wm ); //Redraw
    
    return 0;
}

int table_field_values( int field_type ) //Get number of field values
{
    pattern *p;
    module *s;
    
    if( current_song )
    {
	s = current_song;
	switch( field_type )
	{
    	    case FIELD_CHANNEL: return s->channels; break;
	    case FIELD_PATTERN_NUM: return s->patterns_num; break;
	    case FIELD_PATTERNTABLE_POS: return s->length; break;
	    case FIELD_NOTE: return 64; break;
	    case FIELD_PATTERN_POS: 
	        p = s->patterns[ s->patterntable[ xm.tablepos ] ];
	        return p->rows; break;
	    default: return 1; break;
	}
    }
    return 0;
}

char* table_field_text( int field_type, int value, window_manager *wm ) //Convert field value to text
{
    switch( field_type )
    {
        case FIELD_CHANNEL:
    	    int_to_string( value, temp_str );
	    break;
	case FIELD_PATTERN_NUM:
	case FIELD_PATTERNTABLE_POS:
	case FIELD_NOTE:
	case FIELD_PATTERN_POS:
    	    int_to_string_h( value, temp_str );
	    break;
    }

    return temp_str;
}

int table_field_value( int field_type ) //Get current (real) field value
{
    if( current_song )
    {
	switch( field_type )
	{
    	    case FIELD_CHANNEL: return current_channel; break;
	    case FIELD_PATTERN_NUM: return current_pattern_num; break;
	    case FIELD_PATTERNTABLE_POS: return xm.tablepos; break;
	    case FIELD_NOTE: return current_note; break;
	    case FIELD_PATTERN_POS: return xm.patternpos; break; 
	    default: return 0; break;
	}
    }
    return 0;
}

void table_field_set_value( int field_type, int new_value )
{
    if( current_song )
    {
	switch( field_type )
	{
    	    case FIELD_CHANNEL: current_channel = new_value; break;
	    case FIELD_PATTERN_NUM: current_pattern_num = new_value; break;
	    case FIELD_PATTERNTABLE_POS: 
		xm.tablepos = new_value; 
		xm.patternpos = 0;
		break;
	    case FIELD_NOTE: current_note = new_value; break;
	    case FIELD_PATTERN_POS: xm.patternpos = new_value; break; 
	}
    }    
}

#define CURRENT_PATTERN_NUM \
current_song->patterntable[ field_current_value[ FIELD_PATTERNTABLE_POS ] ]

#define CURRENT_NOTE \
current_pattern_data[ \
( real_pattern_channels * field_current_value[ FIELD_PATTERN_POS ] ) \
+ field_current_value[ FIELD_CHANNEL ] ].n

#define CURRENT_INSTRUMENT \
current_pattern_data[ \
( real_pattern_channels * field_current_value[ FIELD_PATTERN_POS ] ) \
+ field_current_value[ FIELD_CHANNEL ] ].inst

#define CURRENT_VOLUME \
current_pattern_data[ \
( real_pattern_channels * field_current_value[ FIELD_PATTERN_POS ] ) \
+ field_current_value[ FIELD_CHANNEL ] ].vol

#define CURRENT_EFFECT \
current_pattern_data[ \
( real_pattern_channels * field_current_value[ FIELD_PATTERN_POS ] ) \
+ field_current_value[ FIELD_CHANNEL ] ].fx

#define CURRENT_PARAMETER \
current_pattern_data[ \
( real_pattern_channels * field_current_value[ FIELD_PATTERN_POS ] ) \
+ field_current_value[ FIELD_CHANNEL ] ].par

int table_get_element_value( table_data *data, int element_type )
{
    switch( element_type )
    {
	case ELEMENT_PATTERN_NUM:
	    return (int)CURRENT_PATTERN_NUM;

	case ELEMENT_NOTE:
	    return (int)CURRENT_NOTE;

	case ELEMENT_INSTRUMENT:
	    return (int)CURRENT_INSTRUMENT;

	case ELEMENT_VOLUME:
	    return (int)CURRENT_VOLUME;

	case ELEMENT_EFFECT:
	    return (int)CURRENT_EFFECT;

	case ELEMENT_PARAMETER:
	    return (int)CURRENT_PARAMETER;
    }
    return 0;
}

int table_set_element_value( table_data *data, int element_type, int element_value )
{
    mem_off();
    switch( element_type )
    {
	case ELEMENT_PATTERN_NUM:
	    CURRENT_PATTERN_NUM = (uint8_t)element_value;
	    break;

	case ELEMENT_NOTE:
	    CURRENT_NOTE = (uint8_t)element_value;
	    break;

	case ELEMENT_INSTRUMENT:
	    CURRENT_INSTRUMENT = (uint8_t)element_value;
	    break;

	case ELEMENT_VOLUME:
	    CURRENT_VOLUME = (uint8_t)element_value;
	    break;

	case ELEMENT_EFFECT:
	    CURRENT_EFFECT = (uint8_t)element_value;
	    break;

	case ELEMENT_PARAMETER:
	    CURRENT_PARAMETER = (uint8_t)element_value;
	    break;
    }
    mem_on();
    return 0;
}

void table_cell_draw( table_data *data, 
                      int win_num, 
		      int x, 
		      int y, 
		      int highlight, 
		      int current_cell_flag,
		      window_manager *wm ) //Draw one cell
{
    int a;
    int element; //Current element
    int current_element_value = 0;
    int cur_note;
    int cur_octave;
    
    if( current_song )
    {
	for( a = 0; a < data->elements_num; a++ )
	{
	    //Get element type:
	    element = data->elements[ a ];
	
	    //Get element value:
	    int atr_shadow = 0;
	    switch( element )
	    {
		case ELEMENT_PATTERN_NUM:
		    current_element_value = CURRENT_PATTERN_NUM;
		    ext_int_to_string( current_element_value, 
			temp_str, element_xsize[ ELEMENT_PATTERN_NUM ], wm );
		    break;
		case ELEMENT_NOTE:
		    cur_note = CURRENT_NOTE;
		    if( !cur_note ) 
		    {
			temp_str[ 0 ] = '.';
			temp_str[ 1 ] = '.';
			temp_str[ 2 ] = '.';
			temp_str[ 3 ] = 0;
			break;
		    }
		    if( cur_note == 97 )
		    {
			temp_str[ 0 ] = '-';
			temp_str[ 1 ] = '-';
			temp_str[ 2 ] = '-';
			temp_str[ 3 ] = 0;
			break;
		    }
		    if( cur_note == 98 )
		    {
			temp_str[ 0 ] = '<';
			temp_str[ 1 ] = '-';
			temp_str[ 2 ] = '-';
			temp_str[ 3 ] = 0;
			break;
		    }
		    cur_note--; 
		    cur_octave = cur_note / 12;
		    cur_note = cur_note - ( cur_octave * 12 );
		    cur_note <<= 1;
		    temp_str[ 0 ] = notes[ cur_note ];
		    temp_str[ 1 ] = notes[ cur_note+1 ];
		    temp_str[ 2 ] = (char)cur_octave + 48;
		    temp_str[ 3 ] = 0;
		    break;
		case ELEMENT_INSTRUMENT:
		    cur_note = CURRENT_INSTRUMENT;
		    if( cur_note )
			ext_int_to_string( cur_note, temp_str, element_xsize[ ELEMENT_INSTRUMENT ], wm );
		    else
			temp_str[ 0 ] = 0;
		    break;
		case ELEMENT_VOLUME:
		    cur_note = CURRENT_VOLUME;
		    if( cur_note )
		    {
			cur_note -= 0x10;
			temp_str[ 0 ] = hex2[ cur_note >> 4 ];
			temp_str[ 1 ] = hex1[ cur_note & 15 ];
			temp_str[ 2 ] = 0;
		    }
		    else
		    {
			temp_str[ 0 ] = '.';
			temp_str[ 1 ] = '.';
			temp_str[ 2 ] = 0;
		    }
		    atr_shadow = ATR_SHADOW;
		    break;
		case ELEMENT_EFFECT:
		    cur_note = CURRENT_EFFECT;
		    temp_str[ 0 ] = hex3[ cur_note & 31 ];
		    temp_str[ 1 ] = 0;
		    break;
		case ELEMENT_PARAMETER:
		    cur_note = CURRENT_PARAMETER;
		    ext_int_to_string( cur_note, temp_str, element_xsize[ ELEMENT_PARAMETER ], wm );
		    break;
		case ELEMENT_DIVIDER:
		    //Not used
		    temp_str[ 0 ] = ':'; 
		    temp_str[ 1 ] = 0;
		    break;
	    }
	    
	    //Draw element:
	    //Draw cursor:
	    int cc;
	    if( data->record_status )
		cc = wm->colors[0];
	    else
		cc = get_color( 255, 180, 0 );
	    if( current_cell_flag && data->current_element == a )
	    {
		if( element_mono[ element ] )
		{
		    matrix_draw_box( win_num, 
			             x, y,
			             element_xsize[ element ], 1,
		                     cc,
			             wm );
		}
		else
		{
		    matrix_draw_box( win_num, 
			             x + data->current_offset, y,
			             1, 1,
		                     cc,
			             wm );
		}
	    }
	    //Draw element string:
	    if( highlight || current_cell_flag )
		matrix_draw_string( win_num, 
	    	            	    x, y, ATR_BOLD | atr_shadow,
			    	    temp_str,
			    	    wm );
	    else
		matrix_draw_string( win_num, 
	                    	    x, y, ATR_NONE | atr_shadow,
			    	    temp_str,
			    	    wm );
	
	    if( data->elements_align & HALIGN ) 
	    {
		x += element_xsize[ element ];
	    }
	    else
	    {
		y++;
	    }
	}
    }
}

void table_draw_record_status( int win_num, char change_color, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    table_data *data = (table_data*)win->data;
    int rcolor;
    if( change_color )
    {
	if( data->record_color != get_color( 255, 255, 255 ) ) 
	    data->record_color = get_color( 255, 255, 255 );
	else
	    data->record_color = get_color( 255, 0, 0 );
    }
    if( data->record_status ) rcolor = data->record_color; else rcolor = wm->colors[6];
    if( data->horisontal_field != FIELD_NONE )
    if( data->vertical_field != FIELD_NONE )
    {
	matrix_draw_box(
	    win_num, 
	    0, 0, 
	    data->vertical_field_size, 
	    data->horisontal_field_size, 
	    rcolor, wm );	
	if( data->record_status ) 
	    matrix_draw_string( win_num, 0, 0, ATR_NONE, "REC", wm );
    }
}

void table_draw( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    table_data *data = (table_data*)win->data;
    int xstart;                   //current h-field value
    int ystart;                   //current v-field value
    int x, y;                     //Current coordinates (in symbols)
    int start_x = 0, start_y = 0; //Start point for cells drawing
    uint color;                   //Current cell color
    int h_field_values;
    int v_field_values;
    int highlight;
    int current_cell_flag;
    int page, sub_page;
    
    //Set field offset and number of cells:
    data->v_offset = ( (win->y_size - data->horisontal_field_size) / 2 ) / data->cell_ysize;
    data->h_offset = 0;

    data->v_cells = ( win->y_size - data->horisontal_field_size ) / data->cell_ysize;
    data->h_cells = ( win->x_size - data->vertical_field_size ) / data->cell_xsize;

    //Current song:
    current_song = xm.song;
    if( current_song ) 
    {
	current_pattern = current_song->patterns[ current_song->patterntable[ xm.tablepos ] ];
	if( current_pattern == 0 )
	{ //If there is no such pattern:
	    //Increment number of patterns in the song (if need):
	    if( current_song->patterntable[ xm.tablepos ] >= current_song->patterns_num )
	    	current_song->patterns_num = current_song->patterntable[ xm.tablepos ] + 1;
	    //Create new pattern:
	    mem_off();
	    new_pattern( current_song->patterntable[ xm.tablepos ],
	                 64,
			 current_song->channels,
			 &xm );
	    clean_pattern( current_song->patterntable[ xm.tablepos ], &xm );
	    mem_on();
	    current_pattern = current_song->patterns[ current_song->patterntable[ xm.tablepos ] ];
	}
	current_pattern_data = current_pattern->pattern_data;
	if( current_pattern_data == 0 )
	{ //If there is no pattern data:
	    //Create new pattern:
	    mem_off();
	    clear_pattern( (uint16_t)xm.tablepos, &xm ); //delete empty pattern
	    new_pattern( current_song->patterntable[ xm.tablepos ],
	                 64,
			 current_song->channels,
			 &xm ); //create new
	    clean_pattern( current_song->patterntable[ xm.tablepos ], &xm );
	    mem_on();
	    current_pattern = current_song->patterns[ current_song->patterntable[ xm.tablepos ] ];
	}
	current_pattern_data = current_pattern->pattern_data;
	current_number_of_channels = current_song->channels;
	real_pattern_channels = current_pattern->real_channels;
    }
    //============

    //Get number of field values:
    h_field_values = table_field_values( data->horisontal_field );
    v_field_values = table_field_values( data->vertical_field );
    
    //Get current field values:
    data->xstart = table_field_value( data->horisontal_field );
    data->ystart = table_field_value( data->vertical_field );

    //Set current cell visible (HORISONTAL):
    xstart = data->xstart - data->h_offset;
    int dont_move_view_area = 0;
    if( ( data->horisontal_field == FIELD_PATTERNTABLE_POS ||
	  data->horisontal_field == FIELD_PATTERN_POS ) &&
	  xm.status )
	dont_move_view_area = 1;
    if( data->current_cell >= xstart + data->h_cells )
    {
        if( data->scroll_flag || dont_move_view_area )
        {
	    //Scrolling. Move cursor position - make it visible
    	    data->current_cell = xstart + data->h_cells - 1;
	}
	else
	{
	    //Move view area:
	    data->xstart += data->current_cell - (xstart + data->h_cells - 1);
	    table_field_set_value( data->horisontal_field, data->xstart );
	}
    }
    if( data->current_cell < xstart )
    {
        if( data->scroll_flag || dont_move_view_area )
        {
	    data->current_cell = xstart;
	}
	else
	{
	    data->xstart += data->current_cell - xstart;
	    table_field_set_value( data->horisontal_field, data->xstart );
	}
    }
    data->scroll_flag = 0;

    //Draw small empty box in the corner:
    table_draw_record_status( win_num, 0, wm );

    //Draw horisontal field:
    uint8_t attr = 0;
    if( data->horisontal_field != FIELD_NONE )
    {
	xstart = data->xstart - data->h_offset;
	ystart = data->ystart - data->v_offset;

	if( data->vertical_field != FIELD_NONE ) start_x = data->vertical_field_size;
	start_y = data->horisontal_field_size;

	matrix_draw_box( win_num, 
	                 start_x, 0, 
		         win->x_size - start_x, 
		         data->horisontal_field_size, 
		         wm->colors[ 9 ], wm );	
			 
	int cur_value;
	cur_value = data->current_cell;
		   
	for( x = start_x; x < win->x_size; x += data->cell_xsize, xstart++ )
	{
	    if( xstart == cur_value ) //Current value (highlighted)
	    {
		matrix_draw_box( 
		    win_num, 
	            x, 0, 
		    data->cell_xsize, 
		    data->horisontal_field_size, 
		    wm->colors[ 4 ], wm );
		attr = ATR_BOLD;
	    }
	    else attr = ATR_NONE;
	    if( xstart >= h_field_values ) break; //No more values in horisontal field
	    if( xstart < 0 ) continue; //Negative value!
	    matrix_draw_string( win_num, 
	                        x, 0,
			        attr, 
			        table_field_text( data->horisontal_field, xstart, wm ), 
			        wm );
	}
    }
    
    //Draw vertical field:
    if( data->vertical_field != FIELD_NONE )
    {
	start_x = data->vertical_field_size;
	
	xstart = data->xstart - data->h_offset;
	ystart = data->ystart - data->v_offset;
	
	matrix_draw_box( win_num, 
	                 0, start_y, 
		         data->vertical_field_size, 
		         win->y_size - start_y, 
		         wm->colors[ 9 ], wm );

	int cur_value;
	cur_value = data->ystart;
	
	for( y = start_y; y < win->y_size; y += data->cell_ysize, ystart++ )
	{
	    if( ystart == cur_value ) //Current value (highlighted)
	    {
		matrix_draw_box( 
		    win_num, 
	            0, y, 
		    data->vertical_field_size, 
		    data->cell_ysize, 
		    wm->colors[ 4 ], wm );
		attr = ATR_BOLD;
	    }
	    else attr = ATR_NONE;
	    if( ystart >= v_field_values ) break; //No more values in vertical field
	    if( ystart < 0 ) continue; //Negative value!
	    matrix_draw_string( win_num, 
	                        0, y, 
			        attr, 
			        table_field_text( data->vertical_field, ystart, wm ), 
			        wm );
	}
    }
    
    //Draw cells:
    if( data->elements_num )
    {
	int new_color;
	
	ystart = data->ystart - data->v_offset;
	
	//Draw background:
	matrix_draw_box( win_num, 
	                 start_x, 
	 	         start_y, 
	 	         win->x_size - start_x, 
	 	         win->y_size - start_y, 
		         wm->colors[6], 
		         wm );

	int c_add; //color add
	for( y = start_y; y < win->y_size && ystart < v_field_values; y += data->cell_ysize, ystart++ )
	{
	    field_current_value[ data->vertical_field ] = ystart; //Set current field value (for cell drawing)
	    xstart = data->xstart - data->h_offset;
	    for( x = start_x; x < win->x_size && xstart < h_field_values; x += data->cell_xsize, xstart++ )
	    {
		highlight = 0;
		color = wm->colors[6];
		current_cell_flag = 0;
		c_add = 0;
		if( !(xstart&1) ) { c_add = 5; color = wm->colors[6+c_add]; }
		if( (ystart & 15) == 0 ) color = wm->colors[3];
		else if( (ystart & 7) == 0 ) color = wm->colors[3+c_add];
		    else if( (ystart & 3) == 0 ) color = wm->colors[9+c_add];

		if( ( data->sel_xsize > 0 && xstart >= data->sel_x && xstart < data->sel_x + data->sel_xsize ) ||
		    ( data->sel_xsize < 0 && xstart <= data->sel_x && xstart > data->sel_x + data->sel_xsize ) )
		if( ( data->sel_ysize > 0 && ystart >= data->sel_y && ystart < data->sel_y + data->sel_ysize ) ||
		    ( data->sel_ysize < 0 && ystart <= data->sel_y && ystart > data->sel_y + data->sel_ysize ) )
		    color = get_color( 90, 90, 255 ); //^= 0xFFFFFFFF;

		if( ystart == data->ystart ) 
		{ //Current line:
		    highlight = 0xFFFFFFF;
		    if( xstart == data->current_cell ) 
		    { //Current cell:
		        highlight = 0;
		        current_cell_flag = 1;
		        color = get_color( 255, 0, 0 ); //wm->colors[13]; //Color of the current line
		    }
		}
		    
		field_current_value[ data->horisontal_field ] = xstart; //Set current field value (for cell drawing)
		
		if( xstart < 0 ) continue; //Negative value!
		if( ystart < 0 ) continue; //Negative value!

		//Draw cell background (when it necessary)
		int temp_sub = 0;
		if( data->type & TYPE_DIVIDERS ) temp_sub = 1;
		//if( highlight )
		if( 0 )
		{
		    new_color = wm->colors[ 4 ];
		    if( !(new_color & COLORMASK) ) new_color = wm->colors[ 15 ]; //We can't use 0 color in matrix draw
		    matrix_draw_box( win_num, x, y, 
		                     data->cell_xsize - temp_sub, 
		                     data->cell_ysize, 
		                     new_color, 
		                     wm );
		}
		else
		if( color != wm->colors[6] )
		{
		    if( !(color & COLORMASK) ) color = wm->colors[ 15 ]; //We can't use 0 color in matrix draw
		    matrix_draw_box( win_num, x, y, 
		                     data->cell_xsize - temp_sub, 
		                     data->cell_ysize, 
		                     color, 
		                     wm );
		}

		//Draw cell content:
		table_cell_draw( data, win_num, x, y, highlight, current_cell_flag, wm );
		//Draw divider:
		if( temp_sub )
		    matrix_draw_string( win_num, x + data->cell_xsize - 1, y, ATR_BOLD, "|", wm );															    
	    }
	}
    }

    //Set scrollbars parameters:
    if( data->h_scroll >= 0 )
    {
	if( data->v_scroll >= 0 )
	    page = ( win->x_size - data->vertical_field_size - 2 ) / data->cell_xsize;
	else
	    page = ( win->x_size - data->vertical_field_size ) / data->cell_xsize;
	sub_page = 1;
	switch( data->horisontal_field )
	{
	    case FIELD_CHANNEL: 
		sub_page = page;
		break;
	}
	scrollbar_set_parameters( data->h_scroll, 1, h_field_values - sub_page, data->xstart, page, wm );
	scrollbar_draw( data->h_scroll, wm );
    }
    if( data->v_scroll >= 0 )
    {
	if( data->h_scroll >= 0 )
	    page = ( win->y_size - data->horisontal_field_size - 2 ) / data->cell_ysize;
	else
	    page = ( win->y_size - data->horisontal_field_size ) / data->cell_ysize;
	sub_page = 1;
	switch( data->vertical_field )
	{
	    case FIELD_CHANNEL: 
		//sub_page = page;
		break;
	}
	scrollbar_set_parameters( data->v_scroll, 0, v_field_values - sub_page, data->ystart, page, wm );
	scrollbar_draw( data->v_scroll, wm );
    }
    //========================
}

void table_new( int win_num,
		int type,
                int vertical_field,
		int horisontal_field,
		int elements_align,
		int elements_num,
		window_manager *wm,
		... )
{
    window *win = wm->windows[ win_num ]; //Our window
    table_data *data = (table_data*)win->data;
    va_list p;
    int a, size, new_size;

    //New table init:
    data->type = type;
    data->vertical_field = vertical_field;
    data->horisontal_field = horisontal_field;
    data->elements_align = elements_align;
    data->elements_num = elements_num;
    
    //Create one cell:
    va_start( p, wm );
    for( a = 0; a < elements_num; a++ )
    {
	data->elements[ a ] = va_arg( p, int );
	if( data->full_elements_num == 0 ) data->full_elements[ a ] = data->elements[ a ];
    }
    
    if( data->full_elements_num == 0 ) data->full_elements_num = elements_num;
    
    //Set vertical field size:
    if( vertical_field != FIELD_NONE )
    {
	switch( vertical_field )
	{
	    case FIELD_CHANNEL: size = 3; break;
	    case FIELD_PATTERN_NUM: size = 3; break;
	    case FIELD_PATTERNTABLE_POS: size = 3; break;
	    case FIELD_NOTE: size = 3; break;
	    case FIELD_PATTERN_POS: size = 3; break;
	    default: size = 3; break;
	}
    } else size = 0;
    data->vertical_field_size = size;
    
    //Set horisontal field size:
    if( horisontal_field != FIELD_NONE )
    {
	switch( horisontal_field )
	{
	    case FIELD_CHANNEL: size = 1; break;
	    case FIELD_PATTERN_NUM: size = 1; break;
	    case FIELD_PATTERNTABLE_POS: size = 1; break;
	    case FIELD_NOTE: size = 1; break;
	    case FIELD_PATTERN_POS: size = 1; break;
	    default: size = 1; break;
	}    
    } else size = 0;
    data->horisontal_field_size = size;

    //Set cell size:
    if( elements_align & HALIGN )
    {
	size = 0;
	for( a = 0; a < elements_num; a++ )
	{
	    size += element_xsize[ data->elements[ a ] ];
	}
	if( type & TYPE_DIVIDERS ) size++;
	data->cell_xsize = size;
	data->cell_ysize = 1;
    }
    else
    {
	size = 0;
	for( a = 0; a < elements_num; a++ )
	{
	    new_size = element_xsize[ data->elements[ a ] ];
	    if( new_size > size ) size = new_size;
	}
	if( type & TYPE_DIVIDERS ) size++;
	data->cell_xsize = size;
	data->cell_ysize = elements_num;
    }
    
    va_end( p );

    //Create copy buffers:
    int copy_size;
    if( data->vertical_field == 0 || data->horisontal_field == 0 ) 
	copy_size = copy_small_buffer_size; //Is it pattern table?
    else
	copy_size = copy_buffer_size; //..or something huge
    data->copy_buffer[0] = mem_new( HEAP_STORAGE, copy_size, "copy. b. 1" );
    data->copy_buffer[1] = mem_new( HEAP_STORAGE, copy_size, "copy. b. 2" );
    data->copy_buffer[2] = mem_new( HEAP_STORAGE, copy_size, "copy. b. 3" );
}

void table_full_cell( int win_num, int elements_num, window_manager *wm, ... )
{
    window *win = wm->windows[ win_num ]; //Our window
    table_data *data = (table_data*)win->data;
    va_list p;
    int a;

    data->full_elements_num = elements_num;
    
    //Create one cell:
    va_start( p, wm );
    for( a = 0; a < elements_num; a++ )
    {
	data->full_elements[ a ] = va_arg( p, int );
    }
}

void table_set_type( int type_flags, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    table_data *data = (table_data*)win->data;
    
    data->type = type_flags;
    
    if( !(type_flags & TYPE_HSCROLL) )
    {
	if( data->h_scroll >= 0 )
	{ //close h_scroll
	    close_window( data->h_scroll, wm );
	    data->h_scroll = -1;
	}
    }
    if( !(type_flags & TYPE_VSCROLL) )
    {
	if( data->v_scroll >= 0 )
	{ //close v_scroll
	    close_window( data->v_scroll, wm );
	    data->v_scroll = -1;
	}
    }
}

int fullscreen_button_handler( void* user_data, int button_win, window_manager* wm )
{
    table_data *data = (table_data*)user_data;
    window *win = wm->windows[ data->this_window ]; //Our window

    if( data->fullscreen_status == 0 ) 
    {
	data->old_y_string = win->ys;
	win->ys = "0";
	data->fullscreen_status = 1;
	button_set_name( txt_down, data->fullscreen_button, wm );
    }
    else
    {
	win->ys = data->old_y_string;
	data->fullscreen_status = 0;
	button_set_name( txt_up, data->fullscreen_button, wm );
    }
    resize_all_windows( wm );
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    return 0;
}

void table_set_fullscreen( int fullscreen, int win_num, window_manager* wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    table_data *data = (table_data*)win->data;
    data->fullscreen = (uint8_t)fullscreen;
    if( fullscreen )
    {
	data->fullscreen_button = create_window(
	    "tablefs",
	    0, 0, 2, 2, wm->colors[ 12 ], 1, win_num, &button_handler, wm );
	button_set_name( txt_up, data->fullscreen_button, wm );
	if( data->v_scroll )
	    set_window_string_controls( data->fullscreen_button, "100%-4", "0", "100%-2", "2", wm );
	else
	    set_window_string_controls( data->fullscreen_button, "100%-2", "0", "100%", "2", wm );
	button_set_handler( &fullscreen_button_handler, (void*)data, data->fullscreen_button, wm );
	send_event( data->fullscreen_button, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
    }
}

int change_view_button_handler( void* user_data, int button_win, window_manager* wm )
{
    table_data *data = (table_data*)user_data;
    window *win = wm->windows[ data->this_window ]; //Our window

    if( data->change_view_status == 0 ) 
    {
	data->change_view_status = 1;
	table_new( win_patterneditor,
               TYPE_VSCROLL | TYPE_HSCROLL | TYPE_DIVIDERS,
	       FIELD_CHANNEL,
	       FIELD_PATTERN_POS,
	       HALIGN,
	       2,
	       wm,
	       (int)ELEMENT_NOTE,
	       (int)ELEMENT_INSTRUMENT );
	table_full_cell( win_patterneditor, 5, wm,
	       (int)ELEMENT_NOTE,
	       (int)ELEMENT_INSTRUMENT,
	       (int)ELEMENT_VOLUME,
	       (int)ELEMENT_EFFECT,
	       (int)ELEMENT_PARAMETER );
    }
    else
    {
	table_new( win_patterneditor,
               TYPE_VSCROLL | TYPE_HSCROLL | TYPE_DIVIDERS,
	       FIELD_PATTERN_POS,
	       FIELD_CHANNEL,
	       HALIGN,
	       5,
	       wm,
	       (int)ELEMENT_NOTE,
	       (int)ELEMENT_INSTRUMENT,
	       (int)ELEMENT_VOLUME,
	       (int)ELEMENT_EFFECT,
	       (int)ELEMENT_PARAMETER );
	data->change_view_status = 0;
    }
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
    return 0;
}

void table_set_change_view( int change_view, int win_num, window_manager* wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    table_data *data = (table_data*)win->data;
    data->change_view = (uint8_t)change_view;
    if( change_view )
    {
	/*
	data->change_view_button = create_window(
	    "tablecv",
	    0, 0, 2, 2, wm->colors[ 12 ], 1, win_num, &button_handler, wm );
	button_set_name( "V", data->change_view_button, wm );
	if( data->v_scroll )
	    set_window_string_controls( data->change_view_button, "100%-6", "0", "100%-4", "2", wm );
	else
	    set_window_string_controls( data->change_view_button, "100%-4", "0", "100%", "2", wm );
	button_set_handler( &change_view_button_handler, (void*)data, data->change_view_button, wm );
	send_event( data->change_view_button, EVT_SHOW, 0, 0, 0, 0, MODE_CHILDS, wm );
	*/
    }
}

#define GOTO_NEXT_LINE \
value = table_field_value( field ); \
value += text_get_value( text_add, wm ); \
if( value >= table_field_values( field ) ) \
    value -= table_field_values( field ); \
table_field_set_value( field, value ); \
table_draw( win_num, wm );

#define GOTO_PREVIOUS_LINE \
value = table_field_value( field ); \
value --; \
if( value < 0 ) \
    value = table_field_values( field ) - 1; \
table_field_set_value( field, value ); \
table_draw( win_num, wm );

#define PUSH_CURRENT_POSITION \
xs = field_current_value[ data->vertical_field ]; \
ys = field_current_value[ data->horisontal_field ];

#define POP_CURRENT_POSITION \
field_current_value[ data->vertical_field ] = xs; \
field_current_value[ data->horisontal_field ] = ys;

// operation : 0 - copy; 1 - cut; 2 - paste; 3 - transpose down; 4 - transpose up
void block_operation( int buf_num, int value_size, int operation,
                      int x, int y, int xsize, int ysize,
                      int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    table_data *data = (table_data*)win->data;

    //Correct parameters:
    if( xsize < 0 )
    {
	x = x + xsize + 1;
	xsize = -xsize;
    }
    if( ysize < 0 )
    {
	y = y + ysize + 1;
	ysize = -ysize;
    }

    //Get table bounds:
    int xbound = table_field_values( data->horisontal_field );
    int ybound = table_field_values( data->vertical_field );

    //Set current field values:
    field_current_value[ data->vertical_field ] = y;
    field_current_value[ data->horisontal_field ] = x;

    if( operation == 2 ) 
    { // Paste:
	xsize = data->copy_buffer_xsize[ buf_num ];
	ysize = data->copy_buffer_ysize[ buf_num ];
    }

    uint8_t *buf = (uint8_t*)data->copy_buffer[ buf_num ];
    uint *lbuf = (uint*)buf;
    int e, elnum = 0;
    int cx, cy;
    for( cy = 0; cy < ysize; cy++ )
    {
	for( cx = 0; cx < xsize; cx++ )
	{
	    if( operation == 3 || operation == 4 )
	    {
		//Transpose note:
		mem_off(); //Memory protection OFF (PalmOS)
		if( field_current_value[ data->horisontal_field ] < xbound &&
		    field_current_value[ data->vertical_field ] < ybound ) //Check for the bounds
		{
		    int cur_val;
		    for( e = 0; e < data->full_elements_num; e++ )
			if( data->full_elements[ e ] == ELEMENT_NOTE )
			{
			    cur_val = table_get_element_value( data, data->full_elements[e] );
			    if( cur_val >= 1 && cur_val <= 96 )
			    {
				if( operation == 3 )
				{
				    cur_val--; //Trans. down
				    if( cur_val < 1 ) cur_val = 1;
				}
				else
				{
				    cur_val++; //Trans. up
				    if( cur_val > 96 ) cur_val = 96;
				}
				table_set_element_value( data, data->full_elements[e], cur_val ); //Save note
			    }
			}
		}
		mem_on(); //Memory protection ON (PalmOS)
	    }
	    if( operation == 0 || operation == 1 )
	    { // Copy cell to the buffer :
		mem_off(); //Memory protection OFF (PalmOS)
		if( field_current_value[ data->horisontal_field ] < xbound &&
		    field_current_value[ data->vertical_field ] < ybound ) //Check for the bounds
		{
		    if( value_size == 1 )
			for( e = 0; e < data->full_elements_num; e++ )
	    		    buf[elnum++] = (uint8_t)table_get_element_value( data, data->full_elements[e] );
	    	    else
			for( e = 0; e < data->full_elements_num; e++ )
	    		    lbuf[elnum++] = table_get_element_value( data, data->full_elements[e] );
		}
		else
		{ //We are out of table bounds:
		    if( value_size == 1 )
			for( e = 0; e < data->full_elements_num; e++ ) buf[elnum++] = 0;
		    else
			for( e = 0; e < data->full_elements_num; e++ ) lbuf[elnum++] = 0;
		}
		mem_on(); //Memory protection ON (PalmOS)
	    }
	    if( operation == 1 ) // Set NULL cell
		if( field_current_value[ data->horisontal_field ] < xbound &&
		    field_current_value[ data->vertical_field ] < ybound ) //Check for the bounds
		    for( e = 0; e < data->full_elements_num; e++ )
			table_set_element_value( data, data->full_elements[e], 0 );
	    if( operation == 2 ) // Paste some cell
		if( field_current_value[ data->horisontal_field ] < xbound &&
		    field_current_value[ data->vertical_field ] < ybound ) //Check for the bounds
		    for( e = 0; e < data->full_elements_num; e++ )
			if( value_size == 1 )
			    table_set_element_value( data, data->full_elements[e], buf[ elnum++ ] );
			else
			    table_set_element_value( data, data->full_elements[e], lbuf[ elnum++ ] );
		else
		    for( e = 0; e < data->full_elements_num; e++ ) elnum++; //Out of table bounds
	    field_current_value[ data->horisontal_field ] ++;
	}
	field_current_value[ data->vertical_field ] ++;
	field_current_value[ data->horisontal_field ] = x;
    }

    data->copy_buffer_xsize[ buf_num ] = xsize;
    data->copy_buffer_ysize[ buf_num ] = ysize;
}

void handle_key( uint key, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    table_data *data = (table_data*)win->data;
    int xstart;
    int ystart;
    
    int value; //for the GOTO_NEXT_LINE macros
    int xs,ys; //...
    
    //Get current cell position:
    int field;
    int position;
    
    xstart = data->current_cell;
    position = ystart = data->ystart;
    field = data->vertical_field;

    //Set current field values:
    field_current_value[ data->vertical_field ] = ystart;
    field_current_value[ data->horisontal_field ] = xstart;
    
    //Get current element:
    int element = data->elements[ data->current_element ];
    
    //Handle key:
    int handled = 0;
    if( key & KEY_SHIFT )
    {
	if( (key & 511) == '=' ) key = '+';
    }
    if( key == KEY_BACKSPACE )
    {
	if( current_song )
	{
	    PUSH_CURRENT_POSITION;
	    field_current_value[ field ]--;
	    if( field_current_value[ field ] < 0 )  field_current_value[ field ] = 0;
	    int element_value[ 32 ];
	    for(;;)
	    {
		field_current_value[ field ]++;
		if( field_current_value[ field ] >= table_field_values( field ) ) 
		{
		    //Set last NULL cell:
		    field_current_value[ field ]--;
		    for( int e = 0; e < data->full_elements_num; e++ )
			table_set_element_value( data, data->full_elements[e], 0 );
		    break;
		}
		//get some cell:
		int e;
		for( e = 0; e < data->full_elements_num; e++ )
		    element_value[e] = table_get_element_value( data, data->full_elements[e] );
		//Go back
		field_current_value[ field ]--;
		//Set cell values:
		for( e = 0; e < data->full_elements_num; e++ )
		    table_set_element_value( data, data->full_elements[e], element_value[e] );
		field_current_value[ field ]++;
	    }
	    POP_CURRENT_POSITION;
	    GOTO_PREVIOUS_LINE;
	    handled = 1;
	}
    }
    if( key == KEY_INSERT && !handled )
    {
	if( current_song )
	{
	    PUSH_CURRENT_POSITION;
	    field_current_value[ field ] = table_field_values( field ) - 1;
	    int element_value[ 32 ];
	    for(;;)
	    {
	        if( field_current_value[ field ] == position )
		    break;
		//get some cell:
		field_current_value[ field ]--;
		if( field_current_value[ field ] >= 0 ) //2024 fix: field_current_value -> field_current_value[ field ]
		    for( int e = 0; e < data->full_elements_num; e++ )
			element_value[e] = table_get_element_value( data, data->full_elements[e] );
		else
		    for( int e = 0; e < data->full_elements_num; e++ ) 
			element_value[e] = 0;
		//Go forward
		field_current_value[ field ]++;
		//Set cell values:
		for( int e = 0; e < data->full_elements_num; e++ )
		    table_set_element_value( data, data->full_elements[e], element_value[e] );
		//Go back
		field_current_value[ field ]--;
	    }
	    //Set current cell values to NULL:
	    for( int e = 0; e < data->full_elements_num; e++ )
	        table_set_element_value( data, data->full_elements[e], 0 );
	    POP_CURRENT_POSITION;
	    field_current_value[ field ]++;
	    if( field_current_value[ field ] >= table_field_values( field ) )
		field_current_value[ field ] = 0;
	    table_field_set_value( field, field_current_value[ field ] );
	    handled = 1;
	}
    }
    if( key == KEY_CAPS || key == '`' )
    {
	if( current_song )
	{
	    mem_off();
	    CURRENT_NOTE = 97;
	    mem_on();
	    GOTO_NEXT_LINE;
	    handled = 1;
	}
    }
    int cur_number = -1; //Entered number
    int cur_mask;   //Current number mask
    int cur_off;    //Offset of the current number (and mask)
    int cur_val;    //Current cell value
    int s;
    if( current_song && !handled )
    switch( element )
    {
	case ELEMENT_PATTERN_NUM:
	if( key == KEY_DELETE ) 
	{
	    CURRENT_PATTERN_NUM = 0;
	}
	else
	{
	//Key -> number :
	if( key >= '0' && key <= '9' )
	    cur_number = key - '0';
	if( key >= 'a' && key <= 'f' )
	    cur_number = key - 'a' + 10;
	if( key >= 'A' && key <= 'F' )
	    cur_number = key - 'A' + 10;
	//Get offset:
	cur_off = element_xsize[ element ] - data->current_offset - 1;
	cur_off *= 4;
	//Create number mask:
	cur_mask = 15;
	//Get element value:
	cur_number <<= cur_off;
	cur_mask <<= cur_off;
	cur_mask ^= 0xFFFFFFF;
	cur_val = CURRENT_PATTERN_NUM;
	cur_val &= cur_mask;
	cur_val |= cur_number;
	//Check for the bounds:
	if( cur_val >= element_values[ element ] ) cur_val = element_values[ element ] - 1;
	//Save it:
	CURRENT_PATTERN_NUM = (uint8_t)cur_val;
	}
	break;
	
	
	case ELEMENT_NOTE:
	mem_off();
	if( key == KEY_DELETE ) 
	{
	    CURRENT_NOTE = 0;
	    CURRENT_INSTRUMENT = 0;
	    GOTO_NEXT_LINE;
	}
	else
	{
	cur_number = keynotes_table[ key & 255 ];
	if( key == KEY_ENTER ) cur_number = 33;
	if( key == 0x5C ) // '\'
	{
	    CURRENT_NOTE = 98;
	    CURRENT_INSTRUMENT = 0;
	    GOTO_NEXT_LINE;
	}
	else
	if( cur_number >= 0 ) 
	{
	    cur_number += xm.octave * 12;
	    if( cur_number < 96 )
	    {
		CURRENT_NOTE = (uint8_t)cur_number + 1;
		CURRENT_INSTRUMENT = (uint8_t)current_instrument + 1;
		GOTO_NEXT_LINE;
	    }
	}
	}
	mem_on();
	break;
	
	
	case ELEMENT_INSTRUMENT:
	mem_off();
	if( key == KEY_DELETE ) 
	{
	    CURRENT_INSTRUMENT = 0;
	    GOTO_NEXT_LINE;
	}
	else
	{
	//Key -> number :
	if( key >= '0' && key <= '9' )
	    cur_number = key - '0';
	if( key >= 'a' && key <= 'f' )
	    cur_number = key - 'a' + 10;
	if( key >= 'A' && key <= 'F' )
	    cur_number = key - 'A' + 10;
	if( cur_number != -1 )
	{
	    //Get offset:
	    cur_off = element_xsize[ element ] - data->current_offset - 1;
	    cur_off *= 4;
	    //Create number mask:
	    cur_mask = 15;
	    //Get element value:
	    cur_number <<= cur_off;
	    cur_mask <<= cur_off;
	    cur_mask ^= 0xFFFFFFF;
	    cur_val = CURRENT_INSTRUMENT;
	    cur_val &= cur_mask;
	    cur_val |= cur_number;
	    //Check for the bounds:
	    if( cur_val < element_values[ element ] )
	    {
		//Save it:
		CURRENT_INSTRUMENT = (uint8_t)cur_val;
		GOTO_NEXT_LINE;
	    }
	}
	}
	mem_on();
	break;
	
	
	case ELEMENT_VOLUME:
	mem_off();
	if( key == KEY_DELETE ) 
	{
	    CURRENT_VOLUME = 0;
	    GOTO_NEXT_LINE;
	}
	else
	{
	if( key > 0x60 && key < 0x7B )  key -= 0x20; //Make capital char
	if( data->current_offset == 0 )
	    for( s = 0; ; s++ )
	    {
		if( hex2[ s ] == 0 ) break;
		if( hex2[ s ] == (uint8_t)key ) 
		{
		    cur_val = CURRENT_VOLUME;
		    cur_val &= 0x0F;
		    cur_val |= ( (s+1) << 4 );
		    if( s == 4 ) CURRENT_VOLUME = 0x50; //0x40
			else CURRENT_VOLUME = (uint8_t)cur_val;
		    GOTO_NEXT_LINE;
		    break;
		}
	    }
	else
	    for( s = 0; ; s++ )
	    {
		if( hex1[ s ] == 0 ) break;
		if( hex1[ s ] == (uint8_t)key ) 
		{
		    cur_val = CURRENT_VOLUME;
		    cur_val &= 0xF0;
		    cur_val |= s;
		    if( ( cur_val & 0xF0 ) == 0x50 ) cur_val = 0x50;
		    if( ( cur_val & 0xF0 ) == 0 ) cur_val |= 0x10;
		    CURRENT_VOLUME = (uint8_t)cur_val;
		    GOTO_NEXT_LINE;
		    break;
		}
	    }
	}
	mem_on();
	break;
	
	
	case ELEMENT_EFFECT:
	mem_off();
	if( key == KEY_DELETE ) 
	{
	    CURRENT_EFFECT = 0;
	    CURRENT_PARAMETER = 0;
	    GOTO_NEXT_LINE;
	}
	else
	{
	if( key > 0x60 && key < 0x7B )  key -= 0x20; //Make capital char
	for( s = 0; ; s++ )
	{
	    if( hex3[ s ] == 0 ) break;
	    if( hex3[ s ] == (uint8_t)key ) 
		CURRENT_EFFECT = (uint8_t)s;
	}
	GOTO_NEXT_LINE;
	}
	mem_on();
	break;
	
	
	case ELEMENT_PARAMETER:
	mem_off();
	if( key == KEY_DELETE ) 
	{
	    CURRENT_EFFECT = 0;
	    CURRENT_PARAMETER = 0;
	    GOTO_NEXT_LINE;
	}
	else
	{
	//Key -> number :
	if( key >= '0' && key <= '9' )
	    cur_number = key - '0';
	if( key >= 'a' && key <= 'f' )
	    cur_number = key - 'a' + 10;
	if( key >= 'A' && key <= 'F' )
	    cur_number = key - 'A' + 10;
	if( cur_number != -1 )
	{
	    //Get offset:
	    cur_off = element_xsize[ element ] - data->current_offset - 1;
	    cur_off *= 4;
	    //Create number mask:
	    cur_mask = 15;
	    //Get element value:
	    cur_number <<= cur_off;
	    cur_mask <<= cur_off;
	    cur_mask ^= 0xFFFFFFF;
	    cur_val = CURRENT_PARAMETER;
	    cur_val &= cur_mask;
	    cur_val |= cur_number;
	    //Save it:
	    CURRENT_PARAMETER = (uint8_t)cur_val;
	    GOTO_NEXT_LINE;
	}
	}
	mem_on();
	break;
    }
    
    table_draw( win_num, wm );
}

void exit_handler( void *user_data, int button, window_manager *wm )
{
    if( button == 1 )
    { //Exit to OS:
	wm->exit_flag = 1;
    }
}

int table_handler( wm_event *evt, window_manager *wm )
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    table_data *data = (table_data*)win->data;
    int a, b;
    int empty_space;
    int element;
    int value;
    int field_type;
    int note_to_play;
    uint button;
    int xoffset, yoffset;
    int retval = 0;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(table_data), "menu data" );
	    //Init data:
	    data = (table_data*)win->data;
	    mem_set( data, sizeof(table_data), 0 );
	    mem_set( field_current_value, FIELD_LAST * 4, 0 );
	    //Create keynotes table:
	    for( a = 0; a < 256; a++ ) key_channel[a] = -1;
	    mem_set( keynotes_table, 256, 0xFE );
	    keynotes_table[ 'z' ] = 0;
	    keynotes_table[ 's' ] = 1;
	    keynotes_table[ 'x' ] = 2;
	    keynotes_table[ 'd' ] = 3;
	    keynotes_table[ 'c' ] = 4;
	    keynotes_table[ 'v' ] = 5;
	    keynotes_table[ 'g' ] = 6;
	    keynotes_table[ 'b' ] = 7;
	    keynotes_table[ 'h' ] = 8;
	    keynotes_table[ 'n' ] = 9;
	    keynotes_table[ 'j' ] = 10;
	    keynotes_table[ 'm' ] = 11;
	    keynotes_table[ ',' ] = 12;
	    keynotes_table[ 'l' ] = 13;
	    keynotes_table[ '.' ] = 14;
	    keynotes_table[ ';' ] = 15;
	    keynotes_table[ '/' ] = 16;
	    keynotes_table[ 'q' ] = 12;
	    keynotes_table[ '2' ] = 13;
	    keynotes_table[ 'w' ] = 14;
	    keynotes_table[ '3' ] = 15;
	    keynotes_table[ 'e' ] = 16;
	    keynotes_table[ 'r' ] = 17;
	    keynotes_table[ '5' ] = 18;
	    keynotes_table[ 't' ] = 19;
	    keynotes_table[ '6' ] = 20;
	    keynotes_table[ 'y' ] = 21;
	    keynotes_table[ '7' ] = 22;
	    keynotes_table[ 'u' ] = 23;
	    keynotes_table[ 'i' ] = 24;
	    keynotes_table[ '9' ] = 25;
	    keynotes_table[ 'o' ] = 26;
	    keynotes_table[ '0' ] = 27;
	    keynotes_table[ 'p' ] = 28;
	    keynotes_table[ '[' ] = 29;
	    keynotes_table[ '=' ] = 30;
	    keynotes_table[ ']' ] = 31;
	    xm.octave = 4;
	    //=====================
	    data->this_window = evt->event_win;
	    data->scroll_size = 2;
	    data->type = TYPE_HSCROLL | TYPE_VSCROLL;
	    //Create childs:
	    if( table_type & TYPE_HSCROLL )
	    {
		//if( table_type & TYPE_VSCROLL ) empty_space = data->scroll_size; else empty_space = 0;
		empty_space = 0;
		NEW_SCROLL_TYPE = 1;
		data->h_scroll = create_window( "hscroll",
		                                0, 
	    	                                win->y_size - data->scroll_size, 
		    	    			win->x_size - empty_space, 
		    			        data->scroll_size, 
						wm->colors[6],
						0, 
					        evt->event_win, 
						&scrollbar_handler, 
						wm );
		set_window_string_controls( data->h_scroll, "0", "100%-2", "100%", "100%", wm );
		scrollbar_set_handler( &h_scroll_handler, (void*)data, data->h_scroll, wm );
		scrollbar_set_step( data->h_scroll, 1, wm );
	    } else data->h_scroll = -1;
	    if( table_type & TYPE_VSCROLL )
	    {
		if( table_type & TYPE_HSCROLL ) empty_space = data->scroll_size; else empty_space = 0;
		data->v_scroll = create_window( "vscroll",
		                                win->x_size - data->scroll_size,
	                                	0, 
					        data->scroll_size, 
				    		win->y_size - empty_space, 
						wm->colors[6], 
						0,
						evt->event_win, 
						&scrollbar_handler, 
						wm );
		if( empty_space )
		    set_window_string_controls( data->v_scroll, "100%-2", "0", "100%", "100%-2", wm );
		else
		    set_window_string_controls( data->v_scroll, "100%-2", "0", "100%", "100%", wm );
		scrollbar_set_handler( &v_scroll_handler, (void*)data, data->v_scroll, wm );
		scrollbar_set_step( data->v_scroll, 1, wm );
	    } else data->v_scroll = -1;

	    //Create EDIT menu:
	    if( CREATE_TABLE_WITH_EDIT_BUTTON )
	    {
		data->edit_button = create_window( "editpattern", 0, 0, 2, 2, wm->colors[ 12 ], 1, evt->event_win, &button_handler, wm );
		button_set_name( "EDIT", data->edit_button, wm );
		if( data->v_scroll )
		    set_window_string_controls( data->edit_button, "100%-9", "0", "100%-4", "2", wm );
		else
		    set_window_string_controls( data->edit_button, "100%-7", "0", "100%-2", "2", wm );
	    }
	    else data->edit_button = 0;
	    CREATE_TABLE_WITH_EDIT_BUTTON = 0;
	break;
	    
	case EVT_BEFORECLOSE:
	    if( data->copy_buffer[0] ) mem_free( data->copy_buffer[0] );
	    if( data->copy_buffer[1] ) mem_free( data->copy_buffer[1] );
	    if( data->copy_buffer[2] ) mem_free( data->copy_buffer[2] );
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
		table_draw( evt->event_win, wm );      //draw table content
	    }
	    break;
	    
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_box( evt->event_win, wm ); //draw window box
		table_draw( evt->event_win, wm );      //draw table content
	    }
	    break;
	
	case EVT_BUTTONDOWN:
	    if( evt->button >> 3 )
		if( ( evt->button >> 3 ) & KEY_SHIFT )
		{ //Start selection:
		    if( data->selection == 0 && ( ( evt->button >> 3 ) & 511 ) > KEY_F8 )
		    {
		        data->selection = 1;
		        data->sel_x = data->current_cell;
		        data->sel_y = table_field_value( data->vertical_field );
		        data->sel_xsize = 1;
		        data->sel_ysize = 1;
		    }
		}
		else
		{
		    data->selection = 0;
		}
	    
	    if( evt->button == BUTTON_LEFT )
	    {
		{
		    //Go to cell:
		    //Set X :
		    data->current_cell = ( ( evt->x / wm->char_x ) - data->vertical_field_size ) / data->cell_xsize;
		    xoffset = ( ( evt->x / wm->char_x ) - data->vertical_field_size ) - ( data->current_cell * data->cell_xsize );
		    data->current_cell += data->xstart;
		    if( data->current_cell >= table_field_values( data->horisontal_field ) )
		    {
			data->current_cell = table_field_values( data->horisontal_field ) - 1;
			xoffset = data->cell_xsize - 1;
		    }
		    if( xoffset < 0 )
		    {
			data->current_cell--;
			if( data->current_cell < 0 ) { data->current_cell = 0; xoffset = 0; } else xoffset = data->cell_xsize - 1;
		    }
		    if( data->current_cell < 0 ) { data->current_cell = 0; xoffset = 0; }
		    if( evt->y / wm->char_y > data->horisontal_field_size )
		    {
			//Set Y:
			value = ( ( evt->y / wm->char_y ) - data->horisontal_field_size ) / data->cell_ysize;
			yoffset = ( ( evt->y / wm->char_y ) - data->horisontal_field_size ) - ( value * data->cell_ysize );
			value += data->ystart - data->v_offset;
			if( value < 0 ) value = 0;
			if( value >= table_field_values( data->vertical_field ) ) value = table_field_values( data->vertical_field ) - 1;
			table_field_set_value( data->vertical_field, value );
			//Set element and offset:
#ifndef NONPALM
			if( data->elements_align == VALIGN )
			{
			    data->current_element = yoffset;
			    element = data->elements[ data->current_element ];
			    data->current_offset = xoffset;
			    if( data->current_offset >= element_xsize[ element ] || element_mono[ element ] )
				data->current_offset = 0;
			}
			else
			{
			    for( a = 0, b = 0; a < data->elements_num; a++ )
			    {
		    		if( xoffset < b ) break;
		    		element = data->elements[ a ];
		    		b += element_xsize[ element ];
			    }
			    a--;
			    element = data->elements[ a ];
			    b -= element_xsize[ element ];
			    data->current_element = a;
			    element = data->elements[ data->current_element ];
			    data->current_offset = xoffset - b;
			    if( data->current_offset >= element_xsize[ element ] || element_mono[ element ] )
		    	    data->current_offset = 0;
			}
#else
			data->current_element = 0;
			data->current_offset = 0;
#endif
			data->sel_xsize = 0;
			data->sel_ysize = 0;
		    }
		}
		table_draw( evt->event_win, wm );
	    }
	    if( evt->button >> 3 )
	    {
		field_type = data->vertical_field;
		element = data->elements[ data->current_element ];
		if( (evt->button >> 3) & KEY_SHIFT )
		{ //Shift is pressed:
		    if( ((evt->button >> 3) & 511) == KEY_TAB )
		    {
		        if( data->current_element == 0 && data->current_offset == 0 )
			    data->current_cell--;
		        data->current_element = 0;
		        data->current_offset = 0;
		        if( data->current_cell < 0 )
			    data->current_cell = table_field_values( data->horisontal_field ) - 1;
			table_draw( evt->event_win, wm );
		    }
		}
		switch( ( evt->button >> 3 ) & 511 )
		{
		    case KEY_TAB:
		        if( !( (evt->button >> 3) & KEY_SHIFT ) )
		        {
		    	    data->current_cell++;
			    data->current_element = 0;
			    data->current_offset = 0;
			    if( data->current_cell >= table_field_values( data->horisontal_field ) )
			        data->current_cell = 0;
			    table_draw( evt->event_win, wm );
			}
			break;
		    case KEY_RIGHT:
		        if( data->selection )
		        { //During the region selection:
			    data->current_cell++;
			    if( data->current_cell >= table_field_values( data->horisontal_field ) )
			        data->current_cell = 0;
			    data->current_offset = 0;
			    data->current_element = 0;
			}
			else
			{
			    data->current_offset++;
		    	    if( data->current_offset >= element_xsize[ element ] ||
				element_mono[ element ] )
			    {
			        data->current_offset = 0;
			        data->current_element++;
			        if( data->current_element >= data->elements_num )
			        {
			    	    data->current_element = 0;
				    data->current_cell++;
				    if( data->current_cell >= table_field_values( data->horisontal_field ) )
				        data->current_cell = 0;
				}
			    }
			}
			table_draw( evt->event_win, wm );
			break;
		    case KEY_LEFT:
		        if( data->selection )
		        { //During the region selection:
		    	    if( data->current_offset == 0 && data->current_element == 0 )
				data->current_cell--;
			    if( data->current_cell < 0 )
			        data->current_cell = table_field_values( data->horisontal_field ) - 1;
			    data->current_offset = 0;
			    data->current_element = 0;
			}
			else
			{
			    data->current_offset--;
		    	    if( data->current_offset < 0 || element_mono[ element ] )
			    {
			        data->current_element--;
			        if( data->current_element < 0 )
			        {
			            data->current_element = data->elements_num - 1;
				    data->current_cell--;
				    if( data->current_cell < 0 )
				    {
				        data->current_cell = table_field_values( data->horisontal_field ) - 1;
				    }
				}
				element = data->elements[ data->current_element ];
				data->current_offset = element_xsize[ element ] - 1;
			    }
			}
			table_draw( evt->event_win, wm );
			break;
		    case KEY_SCROLLUP:
		    case KEY_UP:
		        value = table_field_value( data->vertical_field );
		        value--;
		        if( value < 0 ) value = table_field_values( data->vertical_field ) - 1;
		        table_field_set_value( data->vertical_field, value );
		        table_draw( evt->event_win, wm );
		        break;
		    case KEY_SCROLLDOWN:
		    case KEY_DOWN:
		        value = table_field_value( data->vertical_field );
		        value++;
		        if( value >= table_field_values( data->vertical_field ) ) value = 0;
		        table_field_set_value( data->vertical_field, value );
		        table_draw( evt->event_win, wm );
		        break;
		}

		switch( evt->button >> 3 )
		{
		    case KEY_ALT | KEY_F1:
			block_operation( 1, 1, 3,   
			                 data->sel_x, data->sel_y,  
					 data->sel_xsize, data->sel_ysize,
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_ALT | KEY_F2:
			block_operation( 1, 1, 4,   
			                 data->sel_x, data->sel_y,  
					 data->sel_xsize, data->sel_ysize,
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_SHIFT | KEY_DELETE:
			block_operation( 1, 1, 1,   
			                 data->sel_x, data->sel_y,  
					 data->sel_xsize, data->sel_ysize,
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_ALT | KEY_F4:
			block_operation( 1, 1, 0,
			                 data->sel_x, data->sel_y,  
					 data->sel_xsize, data->sel_ysize,
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_SHIFT | KEY_INSERT:
			block_operation( 1, 1, 2,
			                 data->current_cell, table_field_value( data->vertical_field ),  
					 1, 1,
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_CTRL | KEY_F1:
			block_operation( 0, 1, 3,   
			                 0, 0,  
					 table_field_values( data->horisontal_field ),
					 table_field_values( data->vertical_field ),
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_CTRL | KEY_F2:
			block_operation( 0, 1, 4,   
			                 0, 0,  
					 table_field_values( data->horisontal_field ),
					 table_field_values( data->vertical_field ),
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_CTRL | KEY_F3:
			block_operation( 0, 1, 1,   
			                 0, 0,  
					 table_field_values( data->horisontal_field ),
					 table_field_values( data->vertical_field ),
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_CTRL | KEY_F4:
			block_operation( 0, 1, 0,   
			                 0, 0,  
					 table_field_values( data->horisontal_field ),
					 table_field_values( data->vertical_field ),
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_CTRL | KEY_F5:
			block_operation( 0, 1, 2,   
			                 0, 0,  
					 table_field_values( data->horisontal_field ),
					 table_field_values( data->vertical_field ),
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_SHIFT | KEY_F1:
			block_operation( 2, 1, 3,   
			                 data->current_cell, 0,  
					 1, table_field_values( data->vertical_field ),
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_SHIFT | KEY_F2:
			block_operation( 2, 1, 4,   
			                 data->current_cell, 0,  
					 1, table_field_values( data->vertical_field ),
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_SHIFT | KEY_F3:
			block_operation( 2, 1, 1,   
			                 data->current_cell, 0,  
					 1, table_field_values( data->vertical_field ),
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_SHIFT | KEY_F4:
			block_operation( 2, 1, 0,   
			                 data->current_cell, 0,  
					 1, table_field_values( data->vertical_field ),
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_SHIFT | KEY_F5:
			block_operation( 2, 1, 2,   
			                 data->current_cell, 0,  
					 1, table_field_values( data->vertical_field ),
					 evt->event_win, wm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_PAGEUP:
		        value = table_field_value( field_type );
			value -= 16;
			if( value < 0 ) value = 0;
			table_field_set_value( field_type, value );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_PAGEDOWN:
			value = table_field_value( field_type );
			value += 16;
			if( value >= table_field_values( field_type ) ) 
			    value = table_field_values( field_type ) - 1;
			table_field_set_value( field_type, value );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_HOME:
		        table_field_set_value( field_type, 0 );
		        table_draw( evt->event_win, wm );
		        break;
		    case KEY_END:
		        value = table_field_values( field_type ) - 1;
		        table_field_set_value( field_type, value );
		        table_draw( evt->event_win, wm );
		        break;
		    case ' ':
			data->record_status ^= 1;
			if( data->record_status ) clean_channels( &xm );
			table_draw( evt->event_win, wm );
			break;
		    case KEY_F1: xm.octave = 0; break;
		    case KEY_F2: xm.octave = 1; break;
		    case KEY_F3: xm.octave = 2; break;
		    case KEY_F4: xm.octave = 3; break;
		    case KEY_F5: xm.octave = 4; break;
		    case KEY_F6: xm.octave = 5; break;
		    case KEY_F7: xm.octave = 6; break;
		    case KEY_F8: xm.octave = 7; break;
		    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		    case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
		    case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
		    case 's': case 't': case 'u': case 'v': case 'w': case 'x':
		    case 'y': case 'z':
		    case '0': case '1': case '2': case '3': case '4': case '5':
		    case '6': case '7': case '8': case '9':
		    case '`':
		    case KEY_CAPS:
		    case '+':
		    case '-':
		    case '=':
		    case '[':
		    case ']':
		    case ';':
		    case 0x27:
		    case 0x5C:
		    case ',':
		    case '.':
		    case '/':
		    case KEY_ENTER:
		    case KEY_DELETE:
		    case KEY_BACKSPACE:
		    case KEY_INSERT:
		    case KEY_SHIFT | '=':
			button = evt->button >> 3;
			if( data->elements[ data->current_element ] == ELEMENT_NOTE )
			{
			    note_to_play = keynotes_table[ button & 255 ];
			    if( button == KEY_ENTER ) note_to_play = 33;
			    if( note_to_play >= 0 && key_channel[ button & 255 ] == -1 )
			    { //Try to play note:
				key_channel[ button & 255 ] = (int16_t)
				play_note( note_to_play, current_instrument, evt->pressure, &xm );
			    }
			}
			//Handle key:
			if( data->record_status )
			    handle_key( button, evt->event_win, wm );
			break;
		    case KEY_ESCAPE:
			//!! Wrong place for this thing !!
			start_dialog( "Exit to OS?", "YES", "NO", 
			              &exit_handler, 0, 
				      win_dialog, wm );
			break;
		    case KEY_CTRL | 's':
			//!! Wrong place for this thing !!
			sound_stream_stop();
    			mem_off();
    			xm_save( "BACKUP.XM", &xm );
    			mem_on();
			sound_stream_play();
			break;
		}
	    }

	    if( evt->button >> 3 )
		if( ( evt->button >> 3 ) & KEY_SHIFT )
		{ //Continue selection:
		    if( data->selection == 1 && ( ( evt->button >> 3 ) & 511 ) > KEY_F8 )
		    {
		        data->sel_xsize = data->current_cell - data->sel_x;
		        data->sel_ysize = table_field_value( data->vertical_field ) - data->sel_y;
			if( data->sel_xsize < 0 ) data->sel_xsize--; else data->sel_xsize++;
			if( data->sel_ysize < 0 ) data->sel_ysize--; else data->sel_ysize++;
			table_draw( evt->event_win, wm );      //draw table content
		    }
		}

	    retval = 1;
	    break;

	case EVT_BUTTONUP:
	    if( evt->button >> 3 )
	    {
		if( (evt->button >> 3) < 256 )
		{
		    if( key_channel[ evt->button >> 3 ] >= 0 )
		    {
			stop_note( key_channel[ evt->button >> 3 ], &xm );
			key_channel[ evt->button >> 3 ] = -1;
		    }
		}
	    }
	    retval = 1;
    }
    return retval;
}
