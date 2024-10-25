/*
    xm_pattern.cpp - functions for working with a patterns
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "xm.h"
#include "../log/log.h"
#include "../memory/memory.h"

void new_pattern( uint16_t num,
                  uint16_t rows,
		  uint16_t channels,
		  xm_struct *xm )
{
    module *song = xm->song;
    pattern *pat;

    //create pattern structure:
    pat = (pattern*) mem_new( 0, sizeof(pattern), "pattern" );
    
    //save created structure pointer:
    song->patterns[ num ] = pat;
    
    pat->header_length = 9; //pattern header length
    pat->reserved5 = 0;     //non packed
    pat->real_rows = pat->rows = rows;       //number of rows in new pattern
    pat->real_channels = pat->channels = channels;
    pat->data_size = rows * channels * sizeof(xmnote); //physical size of pattern

    //Create memory block for the current pattern:
    pat->pattern_data = (xmnote*) mem_new( HEAP_STORAGE, pat->data_size, "pattern data" );
}

#ifndef XM_PLAYER

//We can change one parameter (rows or channels) only at the time
void resize_pattern( uint16_t num,
                     uint16_t rows,
	    	     uint16_t channels,
		     xm_struct *xm )
{
    module *song = xm->song;
    pattern *pat;

    pat = song->patterns[ num ];
    if( pat == 0 ) return;
    
    if( channels > pat->real_channels || rows > pat->real_rows )
    {
	int new_size = rows * channels * sizeof(xmnote);
	xmnote *new_pat = (xmnote*)mem_new( HEAP_STORAGE, new_size, "pattern data" );
	mem_set( new_pat, new_size, 0 );
	for( int y = 0; y < rows; y++ )
	{
	    for( int x = 0; x < channels; x++ )
	    {	
		if( x < pat->real_channels && y < pat->real_rows )
		    new_pat[ y * channels + x ] = pat->pattern_data[ y * pat->real_channels + x ];
	    }
	}
	mem_free( pat->pattern_data );
	pat->pattern_data = new_pat;
	pat->channels = pat->real_channels = channels;
	pat->rows = pat->real_rows = rows;
    }
    else
    {
	pat->rows = rows;
	pat->channels = channels;
    }
}

#endif

void clear_pattern( uint16_t num, xm_struct *xm )
{
    module *song = xm->song;
    pattern *pat = song->patterns[ num ];
    xmnote *pat_data;
    
    if( pat != 0 )
    {
	//clear pattern data:
        pat_data = pat->pattern_data;
	if( pat_data != 0 )
	{
	    mem_free( pat_data );
	}
	pat->pattern_data = 0;
	//===================
	mem_free( pat );
    }
    song->patterns[num] = 0;
}

void clear_patterns(xm_struct *xm)
{
    uint a;
    //clear patterns:
    for( a=0; a < 256; a++ ) clear_pattern( (uint16_t)a, xm );
		
    module *song = xm->song;
    song->length = 1;
    song->patterns_num = 1;
    song->patterntable[ 0 ] = 0;
    song->restart_position = 0;
    xm->tablepos = 0;
    xm->patternpos = 0;
}

void clean_pattern( uint16_t num, xm_struct *xm )
{
    module *song = xm->song;
    pattern *pat = song->patterns[ num ];
    uint8_t *pat_data;
    uint a, rows, channels;
    
    if( pat != 0 )
    {
	//clean pattern data:
        pat_data = (uint8_t*) pat->pattern_data;
	if( pat_data != 0 )
	{
	    rows = pat->rows;
	    channels = song->channels;
	    for( a = 0; a < rows * channels * sizeof(xmnote); a++ )
	    {
		pat_data[ a ] = 0;
	    }
	}
	//===================
    }
}
