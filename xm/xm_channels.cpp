/*
    xm_channels.cpp - functions for working with channels
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "xm.h"
#include "../log/log.h"
#include "../memory/memory.h"

#define GET_FREQ(per)  ( linear_tab[ per % 768 ] >> (per/768) )
#define GET_DELTA(f)   ( ( ( f << 10 ) / xm->freq ) << ( XM_PREC - 10 ) )

void clear_channels(xm_struct *xm)
{
    int a;
    
    for( a = 0; a < MAX_REAL_CHANNELS; a++ )
    {
	if( xm->channels[ a ] != 0 ) 
	{
	    mem_free( (void*) xm->channels[ a ] );
	    xm->channels[ a ] = 0;
	}
    }
}

void new_channels( int number_of_channels, xm_struct *xm )
{
    int a;
    channel *c;
    
    xm->chans = number_of_channels;

    for( a = 0; a < number_of_channels; a++ )
    {
	if( xm->channels[ a ] == 0 )
	{
	    c = (channel*) mem_new( 0, sizeof(channel), "channel" );
	    mem_set( c, sizeof(channel), 0 );
	    c->enable = 1;
	    c->recordable = 1;

	    xm->channels[ a ] = c;
	}
    }
}

void clean_channels( xm_struct *xm )
{
    int number_of_channels = xm->chans;
    int a;
    channel* c;

    for( a = 0; a < number_of_channels; a++ )
    {
	if( xm->channels[ a ] != 0 )
	{
	    c = xm->channels[ a ];
	    int enable = c->enable;
	    mem_set( c, sizeof(channel), 0 );
	    c->enable = enable;
	    c->recordable = 1;
	}
    }
}

#ifndef XM_PLAYER

int play_note( int note_num, int instr_num, int pressure, xm_struct *xm )
{
    int cur_channel = xm->cur_channel;
    module *song = xm->song;
    int channels = song->channels * SUBCHANNELS;

    if( cur_channel >= channels ) cur_channel = 0;
    
    //Searching for the empty channel (only in PLAY mode):
    int c = 0, c2 = 0;
    channel *ch;
    if( xm->status )
    {
	for( c = 0; c < channels; c += SUBCHANNELS )
	{
	    ch = xm->channels[ c ];
	    if( ch )
	    if( ch->smp == 0 && ch->enable && xm->channel_busy[ c ] == 0 ) { cur_channel = c; break; }
	}
    }
    
    if( c == channels || xm->status == 0 ) //If there is no empty channel:
    { 
	//Searching for the first active channel:
	for( c = 1; c < channels; c += SUBCHANNELS )
	{
	    c2 = cur_channel + c; if( c2 >= channels ) c2 -= channels;
	    ch = xm->channels[ c2 ];
	    if( ch )
	    if( ch->enable && !ch->sustain && xm->channel_busy[ c ] == 0 ) { cur_channel = c2; break; }
	}
	if( c == channels ) return -1; //No active channels :(
    }
    
    //Save current channel number:
    xm->cur_channel = cur_channel;
    
    //Play note:
    instrument *ins = song->instruments[ instr_num ];
    if( ins )
    {
	int smp_num = ins->sample_number[ note_num ];
	sample *smp;
	if( ins->samples_num && smp_num < ins->samples_num )
	    smp = ins->samples[ smp_num ];
	else
	    smp = 0;
	if( smp )
	{
	    if( smp->data && smp->length )
	    {
		channel *ch2 = &xm->ch_buf[ xm->ch_write_ptr & ( CHANNELS_IN_BUFFER - 1 ) ]; 
		xm->ch_channels[ xm->ch_write_ptr & ( CHANNELS_IN_BUFFER - 1 ) ] = cur_channel; 

		int *linear_tab = (int*)xm->linear_tab;
		ch2->smp = 0;
	    
		ch2->pan = smp->panning;
		ch2->vol = ( (int)smp->volume * pressure ) >> 10;
		//Retring. envelope:
		ch2->v_pos = 0;
		ch2->p_pos = 0;
		ch2->sustain = 1;
		ch2->fadeout = 65536;
		ch2->vib_pos = 0;
		ch2->cur_frame = 0;
		ch2->env_start = 1;
		//Retrig. sample:
		ch2->ticks = 0;
		ch2->back = 0;
		//Set note:
		note_num += smp->relative_note;
		note_num += ins->relative_note;
		note_num += xm->octave * 12;
		if( note_num > 119 ) note_num = 119;
		if( note_num < 0 ) note_num = 0;
		int period = 7680 - (note_num*64) - (smp->finetune>>1) - (ins->finetune>>1);
		ch2->p_period = ch2->period = period;
		ch2->new_period = period;
		int freq = GET_FREQ(period);
		ch2->delta = GET_DELTA(freq);
		ch2->delta_p = ch2->delta & ( ( 1 << XM_PREC ) - 1 );
		ch2->delta >>= XM_PREC;
		//Set sample:
		ch2->ins_num = instr_num;
		ch2->ins = ins;
		ch2->smp = smp;

		int new_wr = ( xm->ch_write_ptr + 1 ) & ( CHANNELS_IN_BUFFER - 1 );
		xm->ch_write_ptr = new_wr; 

		xm->channel_busy[ cur_channel ] = 1;
	    }
	}
    }
    
    return cur_channel;
}

void stop_note( int channel_num, xm_struct *xm )
{
    channel *ch2 = &xm->ch_buf[ xm->ch_write_ptr & ( CHANNELS_IN_BUFFER - 1 ) ]; 
    xm->ch_channels[ xm->ch_write_ptr & ( CHANNELS_IN_BUFFER - 1 ) ] = channel_num; 

    ch2->ins_num = -1;

    int new_wr = ( xm->ch_write_ptr + 1 ) & ( CHANNELS_IN_BUFFER - 1 );
    xm->ch_write_ptr = new_wr; 

    /*    
    channel *ch = xm->channels[ channel_num ];
    if( ch )
    {
	ch->sustain = 0;
	instrument *ins = ch->ins;
	if( ins )
	if( !(ins->volume_type & 1) ) ch->smp = 0; //No envelope
	xm->channel_busy[ channel_num ] = 0;
    }
    */
}

#endif
