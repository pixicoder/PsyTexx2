/*
    xm_song.cpp - functions for working with song
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "xm.h"
#include "../log/log.h"
#include "../memory/memory.h"

void close_song(xm_struct *xm)
{
    module *song = xm->song;
    if( song ) mem_free( song );
    xm->song = 0;
}

void clear_song(xm_struct *xm)
{
    uint a;
    //clear patterns and instruments:
    for(a=0;a<256;a++) clear_pattern((uint16_t)a,xm);
    for(a=0;a<128;a++) clear_instrument((uint16_t)a,xm);

    module *song = xm->song;
    const char *temp_s = "Extended Module: ";
    int tp;
    for( tp = 0; tp < 17; tp++ ) song->id_text[ tp ] = temp_s[ tp ];
    song->reserved1 = 0x1A;
    temp_s = "PsyTexx2            ";
    for( tp = 0; tp < 20; tp++ ) song->tracker_name[ tp ] = temp_s[ tp ];
    for( tp = 0; tp < 20; tp++ ) song->name[ tp ] = 0;
    song->version = 0x0104;
    song->header_size = 0x114;
    song->length = 1;
    song->patterns_num = 1;
    song->patterntable[ 0 ] = 0;
    song->instruments_num = 0;
    song->restart_position = 0;
    xm->tablepos = 0;
    xm->patternpos = 0;

#ifndef NOPSYNTH
    psynth_clear( xm->pnet );
#endif
}


void new_song( xm_struct *xm )
{
    uint a;
    module *song = xm->song;

#ifndef NOPSYNTH
    psynth_clear( xm->pnet );
#endif

    //create new song:
    if(song != 0){
	clear_song(xm);
	mem_free(song);

	song = (module*) mem_new( 0, sizeof( module ), "song" );
	xm->song = song;
    }else{ //for FIRST song creation:
	song = (module*) mem_new( 0, sizeof( module ), "song" );
	xm->song = song;
	//clear new song:
	for( a = 0; a < 128; a++ ) song->instruments[ a ] = 0;
	for( a = 0; a < 256; a++ ) song->patterns[ a ] = 0;
	//===============
    }
    
    clear_song(xm);
}

void create_silent_song(xm_struct *xm)
{
    module *song = xm->song;
    //Create simple silent song:
    song->channels = 4;
    new_pattern( 0, 64, 4, xm );
    clean_pattern( 0, xm );
}

void set_bpm( int bpm, xm_struct *xm )
{
    xm->bpm = bpm;
    xm->onetick = ( ( xm->freq * 25 ) << 8 ) / ( xm->bpm * 10 );
    xm->patternticks = xm->onetick + 1;
}

void set_speed( int speed, xm_struct *xm )
{
    xm->speed = speed;
    xm->sp = xm->speed;
}
