/*
    win_psysmpview.cpp - sample viewer
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"
#include "win_scrollbar.h"
#include "win_popup.h"
#include "win_psymenu.h"
#include "win_psysamples.h"
#include "win_psysmpview.h"
#include "../xm/xm.h"

enum
{
    OP_SELECTALL = 0,
    OP_CUT,
    OP_COPY,
    OP_PASTE,
    OP_CROP,
    OP_CLEAR,
    OP_SETLOOP,
    OP_SMOOTH,
    OP_NORMALIZE,
    OP_REVERSE
};

int dont_rescale_view = 0;

void smpview_edit_op( smpview_data *data, int op_num )
{
    mem_off();
    int i, i2;
    instrument *ins = 0;
    if( xm.song ) ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	sample *smp = ins->samples[ current_sample ];
	if( smp && smp->data )
	{
	    if( op_num == OP_SELECTALL )
	    {
		data->cursor = 0;
		data->selected_size = smp->length;
	    }

	    int p1 = data->cursor;
	    int p2 = data->cursor + data->selected_size;
	    int tmp;
	    if( p2 < p1 ) { tmp = p1; p1 = p2; p2 = tmp; }
	    if( p1 < 0 ) p1 = 0;
	    if( p1 > (int)smp->length ) goto bad_region;
	    if( p2 < 0 ) goto bad_region;
	    if( p2 > (int)smp->length ) p2 = (int)smp->length;
	    if( p2 == 0 ) goto bad_region;
	    if( p1 == p2 && op_num != OP_PASTE) goto bad_region;
	    
	    //Correct loop:
	    if( op_num == OP_CUT )
	    {
		if( p1 < (int)smp->reppnt && p2 < (int)smp->reppnt )
		{
		    smp->reppnt -= p2 - p1;
		}
	    }
	    else
	    if( op_num == OP_CROP )
	    {
		if( p1 < (int)smp->reppnt )
		{
		    smp->reppnt -= p1;
		}
		if( (int)smp->reppnt + (int)smp->replen > p2 - p1 ) smp->replen = ( p2 - p1 ) - smp->reppnt;
	    }
	    else
	    if( op_num == OP_SETLOOP )
	    {
		int new_reppnt = p1;
		int new_replen = p2 - p1;
		if( new_reppnt < 0 )
		{
		    new_replen -= -new_reppnt;
		    new_reppnt = 0;
		}
		if( new_reppnt + new_replen > (int)smp->length )
		{
		    new_replen -= new_reppnt + new_replen - smp->length;
		}
		smp->reppnt = new_reppnt;
		smp->replen = new_replen;
		if( ( smp->type & 3 ) == 0 ) smp->type |= 1;
	    }

	    //Calc len:
	    int len = p2 - p1;
	    int norm_len = len;
	    int bits = 8;
	    int channels = 1;
	    if( smp->type & 16 ) bits = 16;
	    if( smp->type & 64 ) channels = 2;
	    len *= bits / 8;
	    len *= channels;
	    p1 *= bits / 8;
	    p1 *= channels;
	    p2 *= bits / 8;
	    p2 *= channels;
	    int smp_len = smp->length;
	    smp_len *= bits / 8;
	    smp_len *= channels;

	    if( op_num == OP_CUT )
	    {    	    
		//Create new buffer:
		if( data->copy_buffer ) mem_free( data->copy_buffer );
		data->copy_buffer = mem_new( HEAP_STORAGE, len, "smp copy buffer" );
    	    
		//Copy data to buffer:
		mem_copy( data->copy_buffer, (char*)(smp->data) + p1, len );
    	    
		//Delete this data:
		mem_copy( (char*)(smp->data) + p1, (char*)(smp->data) + p2, smp_len - p2 );
		smp->data = (short*)mem_resize( smp->data, smp_len - len );
    	    
		//Set new smp len:
		smp->length -= norm_len;

		//Clear selected region:
		data->selected_size = 0;
		dont_rescale_view = 1;
	    }
	    else
	    if( op_num == OP_COPY )
	    {
		//Create new buffer:
		if( data->copy_buffer ) mem_free( data->copy_buffer );
		data->copy_buffer = mem_new( HEAP_STORAGE, len, "smp copy buffer" );
    	    
		//Copy data to buffer:
		mem_copy( data->copy_buffer, (char*)(smp->data) + p1, len );
	    }
	    else
	    if( op_num == OP_PASTE && data->copy_buffer && mem_get_size( data->copy_buffer ) > 0 )
	    {
		int cs = mem_get_size( data->copy_buffer );

		//Resize sample
		smp->data = (short*)mem_resize( smp->data, smp_len + cs );

		//Make space for new data:
		char *smp_p = (char*)smp->data;
		for( i = smp_len + cs - 1; i >= p1 + cs; i-- )
		    smp_p[ i ] = smp_p[ i - cs ];

		//Paste data:
		mem_copy( (char*)(smp->data) + p1, data->copy_buffer, cs );

		//Set new smp len:
		smp->length += ( cs / channels ) / ( bits / 8 );
	    }
	    else
	    if( op_num == OP_CROP )
	    {
		//Create new sample:
		short *new_smp_ptr = (short*)mem_new( HEAP_STORAGE, len, "new smp buffer" );
		mem_copy( new_smp_ptr, (char*)(smp->data) + p1, len );
		mem_free( smp->data );
		smp->data = new_smp_ptr;
		smp->length = ( len / channels ) / ( bits / 8 );
		data->cursor = 0;
	    }
	    else
	    if( op_num == OP_CLEAR )
	    {
		mem_set( (char*)(smp->data) + p1, len, 0 );
	    }
	    else
	    if( op_num == OP_SETLOOP )
	    {
	    }
	    else
	    if( op_num == OP_SMOOTH )
	    {
		p1 = ( p1 / channels ) / ( bits / 8 );
		p2 = ( p2 / channels ) / ( bits / 8 );
		int v1, v2;
		if( bits == 8 )
		{
		    char *csmp = (char*)smp->data;
		    if( channels == 1 )
		    {
			for( i = p1; i < p2; i++ )
			{  
			    if( i - 1 < 0 || i - 1 >= (int)smp->length ) v1 = 0; else v1 = csmp[ i - 1 ];
			    if( i + 1 < 0 || i + 1 >= (int)smp->length ) v2 = 0; else v2 = csmp[ i + 1 ];
			    v1 += v2; v1 >>= 1;
			    csmp[ i ] = (char)v1;
			}
		    }
		    else
		    {
			for( i2 = 0; i2 < 2; i2++ )
			for( i = p1 * 2 + i2; i < p2 * 2; i += 2 )
			{  
			    if( i - 2 < 0 || i - 2 >= (int)smp->length * 2 ) v1 = 0; else v1 = csmp[ i - 2 ];
			    if( i + 2 < 0 || i + 2 >= (int)smp->length * 2 ) v2 = 0; else v2 = csmp[ i + 2 ];
			    v1 += v2; v1 >>= 1;
			    csmp[ i ] = (char)v1;
			}
		    }
		}
		else
		{
		    short *ssmp = smp->data;
		    if( channels == 1 )
		    {
			for( i = p1; i < p2; i++ )
			{  
			    if( i - 1 < 0 || i - 1 >= (int)smp->length ) v1 = 0; else v1 = ssmp[ i - 1 ];
			    if( i + 1 < 0 || i + 1 >= (int)smp->length ) v2 = 0; else v2 = ssmp[ i + 1 ];
			    v1 += v2; v1 >>= 1;
			    ssmp[ i ] = (short)v1;
			}
		    }
		    else
		    {
			for( i2 = 0; i2 < 2; i2++ )
			for( i = p1 * 2 + i2; i < p2 * 2; i += 2 )
			{  
			    if( i - 2 < 0 || i - 2 >= (int)smp->length * 2 ) v1 = 0; else v1 = ssmp[ i - 2 ];
			    if( i + 2 < 0 || i + 2 >= (int)smp->length * 2 ) v2 = 0; else v2 = ssmp[ i + 2 ];
			    v1 += v2; v1 >>= 1;
			    ssmp[ i ] = (short)v1;
			}
		    }
		}
	    }
	    else
	    if( op_num == OP_NORMALIZE )
	    {
		int max_val = 0;
		if( bits == 8 )
		{
		    char *csmp = (char*)smp->data;
		    for( i = p1; i < p2; i++ ) 
		    {
			i2 = csmp[ i ];
			if( i2 < 0 ) i2 = -i2;
			if( i2 > max_val ) max_val = i2;
		    }
		    if( max_val && max_val < 127 ) 
		    for( i = p1; i < p2; i++ )
		    {
			i2 = ( 127 << 10 ) / max_val;
			i2 *= (int)csmp[ i ];
			i2 >>= 10;
			csmp[ i ] = (char)i2;
		    }
		}
		else
		{
		    short *ssmp = smp->data;
		    p1 /= 2;
		    p2 /= 2;
		    for( i = p1; i < p2; i++ ) 
		    {
			i2 = ssmp[ i ];
			if( i2 < 0 ) i2 = -i2;
			if( i2 > max_val ) max_val = i2;
		    }
		    if( max_val && max_val < 32767 ) 
		    for( i = p1; i < p2; i++ )
		    {
			i2 = ( 32767 << 10 ) / max_val;
			i2 *= (int)ssmp[ i ];
			i2 >>= 10;
			ssmp[ i ] = (short)i2;
		    }
		}
	    }
	    else
	    if( op_num == OP_REVERSE )
	    {
		if( bits == 8 && channels == 1 )
		{
		    char *csmp = (char*)smp->data;
		    i2 = p2 - 1;
		    i = p1;
		    for( ; i < i2; i++, i2-- )
		    {
			char val1 = csmp[ i ];
			char val2 = csmp[ i2 ];
			csmp[ i2 ] = val1;
			csmp[ i ] = val2;
		    }
		}
		if( ( bits == 8 && channels == 2 ) || ( bits == 16 && channels == 1 ) )
		{
		    p1 = ( p1 / channels ) / ( bits / 8 );
		    p2 = ( p2 / channels ) / ( bits / 8 );
		    uint16_t *ssmp = (uint16_t*)smp->data;
		    i2 = p2 - 1;
		    i = p1;
		    for( ; i < i2; i++, i2-- )
		    {
			uint16_t val1 = ssmp[ i ];
			uint16_t val2 = ssmp[ i2 ];
			ssmp[ i2 ] = val1;
			ssmp[ i ] = val2;
		    }
		}
		if( bits == 16 && channels == 2 )
		{
		    p1 = ( p1 / channels ) / ( bits / 8 );
		    p2 = ( p2 / channels ) / ( bits / 8 );
		    uint *ssmp = (uint*)smp->data;
		    i2 = p2 - 1;
		    i = p1;
		    for( ; i < i2; i++, i2-- )
		    {
			uint val1 = ssmp[ i ];
			uint val2 = ssmp[ i2 ];
			ssmp[ i2 ] = val1;
			ssmp[ i ] = val2;
		    }
		}
	    }
	}
    }
bad_region:
    mem_on();
}

int smpview_scrollbar_handler( void *user_data, int scroll_window, window_manager *wm )
{
    window *swin = wm->windows[ scroll_window ]; //Our window
    scrollbar_data *sdata = (scrollbar_data*)swin->data;
    smpview_data *data = (smpview_data*)user_data;
    window *win = wm->windows[ data->this_window ];

    //Redraw sample window
    int x_psize = win->x_size * wm->char_x;
    instrument *ins = 0;
    if( xm.song ) ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	sample *smp = ins->samples[ current_sample ];
	if( smp && smp->data )
	{
	    data->offset = sdata->cur << data->precision_shift;
	    data->offset /= data->delta;
	    data->offset *= data->delta;
	    if( sdata->max == 0 )
	    {
		data->offset = - (int)( ( ( x_psize - smp->length / data->delta ) * data->delta ) / 2 );
	    }
	    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	}
    }
    
    return 0;
}

//inout: 1 - in; 2 - out
void smpview_zoom( int win_num, int inout, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    smpview_data *data = (smpview_data*)win->data;
    instrument *ins = 0;
    if( xm.song ) ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	sample *smp = ins->samples[ current_sample ];
	if( smp && smp->data )
	{
	    //Get Y center of window:
	    int y_psize = ( win->y_size - 2 ) * wm->char_y / 2;
	    //Get X size of window:
	    int x_psize = win->x_size * wm->char_x;

	    if( inout == 1 )
	    {
		if( data->delta > 1 )
		{
		    //Center:
		    data->offset += ( x_psize * data->delta ) / 4;
		    //Size:
		    data->delta /= 2;
		}
		send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	    }
	    if( inout == 2 )
	    {
		if( (int)smp->length > x_psize * data->delta )
		{
		    //Center:
		    data->offset -= ( x_psize * data->delta ) / 2;
		    //Size:
		    data->delta *= 2;
		}
		send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	    }
	}
    }
}

void smpview_redraw( int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    smpview_data *data = (smpview_data*)win->data;
    instrument *ins = 0;
    if( xm.song ) ins = xm.song->instruments[ current_instrument ];
    if( ins )
    {
	sample *smp = ins->samples[ current_sample ];
	if( smp && smp->data )
	{
	    //If sample exists:
	    //Get Y center of window:
	    int y_psize = ( win->y_size - 2 ) * wm->char_y / 2;
	    //Get X size of window:
	    int x_psize = win->x_size * wm->char_x;
	    int old_val = 0;
	    int old_val1 = 0;
	    int old_val2 = 0;
	    if( data->prev_smp_data != smp->data || data->prev_smp_len != smp->length )
	    {
		if( dont_rescale_view == 0 )
		{
		    if( (int)smp->length > x_psize )
		    {
			//If not enough space for sample on the screen:
			data->delta = smp->length / x_psize;
			if( smp->length % x_psize ) data->delta++;
		    }
		    else
		    {
			data->delta = 1;
		    }
		    data->offset = - (int)( ( ( x_psize - smp->length / data->delta ) * data->delta ) / 2 );
		}
		dont_rescale_view = 0;
		data->prev_smp_data = smp->data;
		data->prev_smp_len = smp->length;
	    }
	    int ptr = data->offset;
	    int bits = 8;
	    int channels = 1;
	    if( smp->type & 16 ) bits = 16;
	    if( smp->type & 64 ) channels = 2;
	    if( channels == 2 ) y_psize >>= 1;

	    //Draw empty areas:
	    if( data->offset < 0 )
	    {
		pdraw_box( win_num, 
		           0, 0, 
			   -data->offset / data->delta, y_psize * 2 * channels, 
			   blend( win->color, 0, 20 ), wm );
	    }
	    int part2 = x_psize * data->delta + data->offset;
	    if( part2 >= (int)smp->length )
	    {
		part2 = ( smp->length - data->offset ) / data->delta;
		pdraw_box( win_num, 
		           part2, 0, 
			   x_psize - part2, y_psize * 2 * channels, 
			   blend( win->color, 0, 20 ), wm );
	    }

	    //Draw cursor:
	    int cur_x = ( data->cursor - data->offset ) / data->delta;
	    int selected_size = data->selected_size / data->delta;
	    if( selected_size < 0 ) 
	    {
		cur_x += selected_size;
		selected_size = -selected_size;
	    }
	    if( selected_size == 0 )
	    {
		if( data->cursor >= 0 && data->cursor < (int)smp->length )
		{
		    pdraw_box( win_num, cur_x, 0, 1, y_psize * 2 * channels, get_color( 255, 170, 80 ), wm );
		}
	    }
	    else
	    {
		int limit1 = -data->offset / data->delta;
		int limit2 = ( -data->offset + smp->length ) / data->delta;
		pdraw_box( win_num, cur_x, 0, selected_size, y_psize * 2 * channels, blend( wm->colors[ 1 ], get_color( 0, 0, 255 ), 40 ), wm );
		if( cur_x < limit1 && cur_x + selected_size > limit1 )
		{
		    pdraw_box( win_num, 
			       limit1, 0,
			       1, y_psize * 2 * channels,
			       get_color( 120, 120, 120 ), wm );
		}
		if( cur_x < limit2 && cur_x + selected_size > limit2 )
		{
		    pdraw_box( win_num, 
			       limit2, 0,
			       1, y_psize * 2 * channels,
			       get_color( 120, 120, 120 ), wm );
		}
	    }

	    //Draw loop:
	    if( smp->type & 3 )
	    {
		COLOR rep_color = get_color( 150, 150, 255 );
		cur_x = ( smp->reppnt - data->offset ) / data->delta;
	        pdraw_box( win_num, cur_x, 0, 1, y_psize * 2 * channels, rep_color, wm );
		pdraw_box( win_num, cur_x, 0, 16, 16, rep_color, wm );
		cur_x = ( smp->reppnt + smp->replen - data->offset ) / data->delta;
	        pdraw_box( win_num, cur_x, 0, 1, y_psize * 2 * channels, rep_color, wm );
		pdraw_box( win_num, cur_x - 15, y_psize * 2 * channels - 16, 16, 16, rep_color, wm );
	    }

	    //Draw sample:
	    for( int x = 0; x < win->x_size * wm->char_x; x++ )
	    {
		if( channels == 1 && ptr >= 0 && ptr < (int)smp->length )
		{
		    int val;
		    if( x & 1 ) 
		    {
			draw_pixel( win_num, x, 16, wm->colors[ 12 ], wm );
			draw_pixel( win_num, x, y_psize, wm->colors[ 12 ], wm );
			draw_pixel( win_num, x, y_psize * 2 - 17, wm->colors[ 12 ], wm );
		    }
		    if( bits == 8 )
		    {
			signed char *sp = (signed char*)smp->data;
			val = sp[ ptr ];
			val *= y_psize - 1;
			val >>= 7;
		    }
		    else
		    {
			val = smp->data[ ptr ];
			val *= y_psize - 1;
			val >>= 15;
		    }
		    if( val != old_val && ptr && x )
		    {
			int v1 = val;
			int v2 = old_val;
			int temp;
			if( v1 < v2 ) { temp = v1; v1 = v2; v2 = temp; }
			pdraw_box( win_num, x, y_psize - v1, 1, v1 - v2, wm->colors[ 15 ], wm );
		    }
		    else draw_pixel( win_num, x, y_psize - val, wm->colors[ 15 ], wm );
		    old_val = val;
		}
		if( channels == 2 && ptr >= 0 && ptr < (int)smp->length )
		{
		    if( x & 1 ) 
		    {
			draw_pixel( win_num, x, y_psize, wm->colors[ 12 ], wm );
			draw_pixel( win_num, x, y_psize * 3, wm->colors[ 12 ], wm );
		    }
		    int val1, val2;
		    if( bits == 8 )
		    {
			signed char *sp = (signed char*)smp->data;
			val1 = sp[ ptr * 2 ];
			val1 *= y_psize - 1;
			val1 >>= 7;
			val2 = sp[ ptr * 2 + 1 ];
			val2 *= y_psize - 1;
			val2 >>= 7;
		    }
		    else
		    {
			val1 = smp->data[ ptr * 2 ];
			val1 *= y_psize - 1;
			val1 >>= 15;
			val2 = smp->data[ ptr * 2 + 1 ];
			val2 *= y_psize - 1;
			val2 >>= 15;
		    }
		    if( val1 != old_val1 && ptr && x )
		    { //Draw left channel:
			int temp;
			int v1 = val1;
			int v2 = old_val1;
			if( v1 < v2 ) { temp = v1; v1 = v2; v2 = temp; }
			pdraw_box( win_num, x, y_psize - v1, 1, v1 - v2, wm->colors[ 15 ], wm );
		    }
		    else draw_pixel( win_num, x, y_psize - val1, wm->colors[ 15 ], wm );
		    if( val2 != old_val2 && ptr && x )
		    { //Draw right channel:
			int temp;
			int v1 = val2;
			int v2 = old_val2;
			if( v1 < v2 ) { temp = v1; v1 = v2; v2 = temp; }
			pdraw_box( win_num, x, y_psize * 3 - v1, 1, v1 - v2, wm->colors[ 15 ], wm );
		    }
		    else draw_pixel( win_num, x, y_psize * 3 - val2, wm->colors[ 15 ], wm );
		    old_val1 = val1;
		    old_val2 = val2;
		}
		ptr += data->delta;
	    }

	    //Set scrollbar parameters:
	    int bar_len;
	    int off;
	    if( (int)smp->length < x_psize * data->delta )
	    {
		scrollbar_set_parameters( data->scrollbar, 1, 0, 0, 0, wm );
		scrollbar_set_step( data->scrollbar, data->delta, wm );
	    }
	    else
	    {
		bar_len = smp->length - x_psize * data->delta;
    		if( bar_len < ( 1 << 20 ) )
		{
		    data->precision_shift = 0;
		    off = data->offset;
		    if( off < 0 ) off = 0;
		    if( off >= bar_len ) off = bar_len;
		    scrollbar_set_parameters( data->scrollbar, 1, bar_len, off, x_psize * data->delta, wm );
		    scrollbar_set_step( data->scrollbar, data->delta, wm );
		}
		else
		{
		    //Sample length is very large. Lets decrease scrollbar precision:
		    int shift = 0;
		    for( shift = 1; shift <= 12; shift++ )
		    {
			bar_len >>= 1;
			if( bar_len < ( 1 << 20 ) ) break;
		    }
		    data->precision_shift = shift;
		    off = data->offset >> shift;
		    if( off < 0 ) off = 0;
		    if( off >= bar_len ) off = bar_len;
		    scrollbar_set_parameters( data->scrollbar, 1, bar_len, off, ( x_psize * data->delta ) >> shift, wm );		    
		    scrollbar_set_step( data->scrollbar, data->delta >> shift, wm );
		}
	    }
	}
    }
}

int smpview_handler( wm_event *evt, window_manager *wm )
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    smpview_data *data = (smpview_data*)win->data;
    instrument *ins = 0;
    sample *smp;
    if( xm.song ) ins = xm.song->instruments[ current_instrument ];
    if( ins ) smp = ins->samples[ current_sample ];
    int retval = 0;

    //Get Y center of window:
    int y_psize = ( win->y_size - 2 ) * wm->char_y / 2;
    //Get X size of window:
    int x_psize = win->x_size * wm->char_x;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(smpview_data), "sample view data" );
	    //Init data:
	    data = (smpview_data*)win->data;
	    data->this_window = evt->event_win;
	    data->offset = 0;
	    data->delta = 1;
	    data->cursor = -1;
	    data->selected_size = 0;
	    data->dragmode = 0;
	    data->copy_buffer = 0;

	    //Create scrollbar:
	    NEW_SCROLL_TYPE = 1;
	    data->scrollbar = create_window( "smpscroll",
	                                     0, 0, 1, 1, wm->colors[6], 0,
	                                     evt->event_win, &scrollbar_handler, wm );
	    set_window_string_controls( data->scrollbar, "0", "100%-2", "100%", "100%", wm );
	    scrollbar_set_parameters( data->scrollbar, 1, 0, 0, 0, wm );
	    scrollbar_set_handler( &smpview_scrollbar_handler, (void*)data, data->scrollbar, wm );
	    data->precision_shift = 0;
	    break;
	    
	case EVT_BEFORECLOSE:
	    if( data->copy_buffer ) mem_free( data->copy_buffer );
	    if( win->data ) mem_free( win->data );
	    break;
	case EVT_SHOW: 
	    //Show window:
	    win->visible = 1; //Make it visible
	    send_event( evt->event_win, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Send DRAW event to all childs
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
		smpview_redraw( evt->event_win, wm );
	    }
	    break;
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_box( evt->event_win, wm ); //draw window box
		smpview_redraw( evt->event_win, wm );
	    }
	    break;
	case EVT_BUTTONDOWN:
	    if( evt->button >> 3 )
	    {
		if( ins )
		{
		    smp = ins->samples[ current_sample ];
		    if( smp && smp->data )
		    {
			switch( ( evt->button >> 3 ) & 511 )
			{
			    case KEY_DELETE:
				smpview_edit_op( data, OP_CUT );
				send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
				break;

			    case KEY_INSERT:
				smpview_edit_op( data, OP_PASTE );
				send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
				break;

			    case 'c': case 'C':
				if( ( evt->button >> 3 ) & KEY_CTRL )
				{
				    //COPY:
				    smpview_edit_op( data, OP_COPY );
				    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
				}
				break;

			    case 'v': case 'V':
				if( ( evt->button >> 3 ) & KEY_CTRL )
				{
				    //PASTE:
				    smpview_edit_op( data, OP_PASTE );
				    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
				}
				break;

			    case 'a': case 'A':
				if( ( evt->button >> 3 ) & KEY_CTRL )
				{
				    //Select all:
				    data->cursor = 0;
				    data->selected_size = smp->length;
				    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
				}
				break;

			    case KEY_LEFT:
				if( ( evt->button >> 3 ) & KEY_SHIFT )
				{
				    data->selected_size -= data->delta;
				}
				else
				{
				    data->cursor -= data->delta;
				    if( data->cursor < data->offset ) data->offset -= data->delta;
				}
				send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
				retval = 1;
				break;

			    case KEY_RIGHT:
				if( ( evt->button >> 3 ) & KEY_SHIFT )
				{
				    data->selected_size += data->delta;
				}
				else
				{
				    data->cursor += data->delta;
				    if( data->cursor >= data->offset + x_psize * data->delta ) data->offset += data->delta;
				}
				send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
				retval = 1;
				break;

			    case KEY_SCROLLUP:
				if( data->delta > 1 )
				{
				    //Center:
				    data->offset += ( x_psize * data->delta ) / 4;
				    //Size:
				    data->delta /= 2;
				}
				send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
				retval = 1;
				break;
        		    
			    case KEY_SCROLLDOWN:
				if( (int)smp->length > x_psize * data->delta )
				{
				    //Center:
				    data->offset -= ( x_psize * data->delta ) / 2;
				    //Size:
				    data->delta *= 2;
				}
				send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
				retval = 1;
				break;
			}
		    }
		}
	    }
	    if( evt->button & BUTTON_MIDDLE )
	    {
		//Start drag:
		data->start_x = evt->x;
		data->start_offset = data->offset;
		data->dragmode = 1;
		retval = 1;
	    }
	    if( evt->button & BUTTON_LEFT )
	    {
		//Set cursor:
		data->start_x = evt->x;
		data->start_y = evt->y;
	        data->dragmode = 1;
		if( ins )
		{
		    smp = ins->samples[ current_sample ];
		    if( smp && smp->type & 3 )
		    {
			data->start_reppnt1 = smp->reppnt;
			data->start_reppnt2 = smp->reppnt + smp->replen;
			if( evt->y < 16 && evt->x >= ( (int)smp->reppnt - data->offset ) / data->delta && 
					evt->x < ( (int)smp->reppnt - data->offset ) / data->delta + 16 )
			{
			    //Selected peppnt cursor:
			    data->dragmode = 2;
			}
			if( evt->y > ( y_psize * 2 ) - 16 
			    && evt->x < ( (int)smp->reppnt + (int)smp->replen - data->offset ) / data->delta && 
			       evt->x >= ( (int)smp->reppnt + (int)smp->replen - data->offset ) / data->delta - 16 )
			{
			    //Selected reppnt2 cursor:
			    data->dragmode = 3;
			}
		    }
		}
		if( data->dragmode == 1 )
		{
		    data->cursor = data->offset + evt->x * data->delta;
		    data->selected_size = 0;
		}
		send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		retval = 1;
	    }
	    if( evt->button & BUTTON_RIGHT )
	    {
		smpview_edit_op( data, 
		start_popup_blocked( SAMPLE_MENU, 
		                     evt->x + win->real_x * wm->char_x, 
				     evt->y + win->real_y * wm->char_y, 
				     wm )
		);
		send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
	    }
	    break;

	case EVT_MOUSEMOVE:
	    if( !data->dragmode ) break;
	    if( evt->button & BUTTON_MIDDLE )
	    {
		//Drag:
		data->offset = data->start_offset - ( evt->x - data->start_x ) * data->delta;
		send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		retval = 1;
	    }
	    if( evt->button & BUTTON_LEFT )
	    {
		if( data->dragmode > 1 )
		{
		    //Set loop pointers:
		    if( ins )
		    {
			smp = ins->samples[ current_sample ];
			if( smp && smp->type & 3 )
			{
			    int reppnt1 = data->start_reppnt1;
			    int reppnt2 = data->start_reppnt2;
			    if( data->start_y < 16 )
			    {
				reppnt1 = data->start_reppnt1 + ( evt->x - data->start_x ) * data->delta;
				if( reppnt1 < 0 ) reppnt1 = 0;
				if( reppnt1 >= (int)smp->length ) reppnt1 = smp->length - 1;
			    }
			    else
			    {
				reppnt2 = data->start_reppnt2 + ( evt->x - data->start_x ) * data->delta;
				if( reppnt2 < 0 ) reppnt2 = 0;
				if( reppnt2 >= (int)smp->length ) reppnt2 = smp->length - 1;
			    }
			    if( reppnt2 < reppnt1 ) { int tmp = reppnt1; reppnt1 = reppnt2; reppnt2 = tmp; }
			    int len = reppnt2 - reppnt1;
			    if( len < 1 ) len = 1;
			    smp->reppnt = reppnt1;
			    smp->replen = len;
			}
		    }
		}
		else
		{
		    //Set cursor:
		    data->selected_size = ( data->offset + evt->x * data->delta ) - data->cursor;
		}
	        send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm );
		retval = 1;
	    }
	    break;

	case EVT_BUTTONUP:
	    if( evt->button & 3 )
	    {
		data->dragmode = 0;
	    }
	    break;
    }
    
    return retval;
}
