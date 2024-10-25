/*
    xm_sample.cpp - functions for working with samples
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "xm.h"
#include "../log/log.h"
#include "../memory/memory.h"
#include "../file/file.h"

void new_sample( uint16_t num, 
                 uint16_t ins_num, 
                 const char *name,
		 int length,      /*length in bytes*/
		 int type,
                 xm_struct *xm )
{
    module *song = xm->song;
    instrument *ins = song->instruments[ ins_num ];
    sample *smp; //current sample
    signed short *data;
    int a;
    int created_size;

    smp = (sample*) mem_new( 0, sizeof(sample), "sample" );
    
    //save sample parameters:
    for( a = 0 ; a < 22 ; a++ )
    {
	smp->name[a] = name[a];
	if( name[ a ] == 0 ) break;
    }
    
    //create NULL sample:
    smp->data = 0;

    //create sample data:
    slog( "SMP: Creating the sample data\n" );
    if( length ) 
    {
	data = (signed short*) mem_new( 1, length, "sample data" );
	smp->data = (signed short*) data;
	created_size = length;
    }else{ //for NULL sample
	created_size = 0;
    }
    
    smp->length = created_size;
    smp->type = (uint8_t) type;

    smp->volume = 0x40;
    smp->panning = 0x80;
    smp->relative_note = 0;
    smp->finetune = 0;
    smp->reppnt = 0;
    smp->replen = 0;
    
    //save created sample:
    ins->samples[num] = smp;
    slog( "SMP: Sample was created\n" );
}

void clear_sample(uint16_t num, uint16_t ins_num, xm_struct *xm)
{
    module *song = xm->song;
    instrument *ins = song->instruments[ ins_num ];
    sample *smp;
    signed short *smp_data;
    
    if( ins != 0 )
    {
	smp = ins->samples[ num ];
	if( smp != 0 )
	{
	    //clear sample data:
	    smp_data = (signed short*) smp->data;
	    if( smp_data != 0 )
	    {
	        mem_free( smp_data );
	    }
	    smp->data = 0;
	    //==================
	    mem_free( smp );
	    ins->samples[num] = 0;
	}
    }
}

void bytes2frames( sample *smp, xm_struct *xm )
{
    int bits = 8;
    int channels = 1;
    if( smp->type & 16 ) bits = 16;
    if( smp->type & 64 ) channels = 2;
    smp->length = smp->length / ( ( bits / 8 ) * channels );
    smp->reppnt = smp->reppnt / ( ( bits / 8 ) * channels );
    smp->replen = smp->replen / ( ( bits / 8 ) * channels );
}

void frames2bytes( sample *smp, xm_struct *xm )
{
    int bits = 8;
    int channels = 1;
    if( smp->type & 16 ) bits = 16;
    if( smp->type & 64 ) channels = 2;
    smp->length = smp->length * ( ( bits / 8 ) * channels );
    smp->reppnt = smp->reppnt * ( ( bits / 8 ) * channels );
    smp->replen = smp->replen * ( ( bits / 8 ) * channels );
}

