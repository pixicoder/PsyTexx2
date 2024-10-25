/*
    xm_instrum.cpp - functions for working with instruments
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "xm.h"
#include "../log/log.h"
#include "../memory/memory.h"
#include "../file/file.h"

#ifndef XM_PLAYER
    #include "wm/wm.h"
    #include "../psytexx/win_dialog.h"
    extern window_manager wm;
#endif

void new_instrument( uint16_t num, 
                     const char *name, 
                     uint16_t samples, 
                     xm_struct *xm )
{
    module *song = xm->song;
    instrument *ins;
    int a;
    
    ins = (instrument*) mem_new( HEAP_STORAGE, sizeof(instrument), "instrument" );
    mem_set( ins, sizeof(instrument), 0 );
    
    //save info:
    for(a=0;a<22;a++){
	ins->name[a] = name[a];
	if(name[a]==0) break;
    }
    
    ins->samples_num = samples;
    
    //add NULL samples:
    for(a=0;a<16;a++){
	ins->samples[a] = 0;
    }
    
    //Set some default info about instrument:
    ins->volume_points_num = 1;
    ins->panning_points_num = 1;
    ins->volume = 64;
    ins->panning = 128;
    
    //save created instrument:
    song->instruments[num] = ins;
}

void clear_instrument( uint16_t num, xm_struct *xm )
{
    uint16_t a;
    module *song = xm->song;
    instrument *ins = song->instruments[ num ];
    
    if(ins!=0){
	//clear samples:
	for(a=0;a<16;a++){
	    if( ins->samples[a] != 0 ){
		clear_sample(a,num,xm);
	    }
	}
	//clear instrument:
	mem_free(ins);
	song->instruments[num] = 0;
    }
}

void clear_instruments( xm_struct *xm )
{
    uint a;
    //clear instruments:
    for( a = 0; a < 128; a++ ) clear_instrument( (uint16_t)a, xm );
    xm->song->instruments_num = 0;
}

#ifndef XM_PLAYER

void clear_comments( xm_struct *xm )
{
    uint a, b, c;
    //clear instruments:
    for( a = 0; a < 128; a++ ) 
    {
    	if( xm->song->instruments[ a ] )
	{
	    for( b = 0; b < 22; b++ )
		xm->song->instruments[ a ]->name[ b ] = 0;
	    for( c = 0; c < 16; c++ )
	    {
		if( xm->song->instruments[ a ]->samples[ c ] )
		{
		    for( b = 0; b < 22; b++ )
			xm->song->instruments[ a ]->samples[ c ]->name[ b ] = 0;
		}
	    }
	}
    }
}

void save_instrument( uint16_t num, char *filename, xm_struct *xm )
{
    module *song = xm->song;
    if( song )
    {
	if( song->instruments[ num ] )
	{
	    instrument *ins = song->instruments[ num ];
	    FILE *f = fopen( filename, "wb" );
	    if( f )
	    {
		fwrite( (void*)"Extended Instrument: ", 21, 1, f );
		fwrite( &ins->name, 22, 1, f ); // Instrument name
		
		//Save version:
		char temp[ 30 ];
		temp[ 21 ] = 0x50;
		temp[ 22 ] = 0x50;
		fwrite( temp, 23, 1, f );

		fwrite( ins->sample_number, 208, 1, f ); //Instrument info

		//Extended info:
		fwrite( &ins->volume, 2 + EXT_INST_BYTES, 1, f );
		fwrite( temp, 22 - ( 2 + EXT_INST_BYTES ), 1, f ); //Empty
		
		fwrite( &ins->samples_num, 2, 1, f ); //Samples number

		//Save samples:
		int s;
		sample *smp;
		for( s = 0; s < ins->samples_num; s++ )
		{
		    smp = ins->samples[ s ];
		    if( smp->type & 16 )
		    {
			frames2bytes( smp, xm );
		    }
		    fwrite( smp, 40, 1, f ); //Save sample info
		}

		//Save samples data:
		signed short *s_data; //sample data
		char *cs_data;        //char 8 bit sample data
		int b;
		for( b = 0; b < ins->samples_num; b++ )
		{
		    smp = ins->samples[ b ];
        	    
		    if( smp->length == 0 ) continue;
        	    
		    if( smp->type & 16 )
		    { //16bit sample:
			int len = smp->length;
			short old_s = 0;
			short old_s2 = 0;
			short new_s = 0;
			s_data = (signed short*) smp->data;
			//convert sample:
			int sp;
			for( sp = 0 ; sp < len/2; sp++ )
			{
			    old_s2 = s_data[ sp ];
			    s_data[sp] = old_s2 - old_s;
			    s_data[sp] = s_data[ sp ];
			    old_s = old_s2;
			}
			fwrite( s_data, len, 1, f );
			//convert sample to normal form:
			old_s = 0;
			for( sp = 0; sp < len/2; sp++ )
			{
			    new_s = s_data[ sp ] + old_s;
			    s_data[ sp ] = new_s;
			    old_s = new_s;
			}
			//convert sample info:
			bytes2frames( smp, xm );
		    }
		    else
		    { //8bit sample:
			int len = smp->length;
			signed char c_old_s = 0;
			signed char c_old_s2 = 0;
			signed char c_new_s = 0;
			cs_data = (char*) smp->data;
			//convert sample:
			int sp;
			for( sp = 0 ; sp < len; sp++ )
			{
			    c_old_s2 = cs_data[sp];
			    cs_data[sp] = c_old_s2 - c_old_s;
			    c_old_s = c_old_s2;
			}
			fwrite( cs_data, len, 1, f );
			//convert sample to normal form:
			c_old_s = 0;
			for( sp = 0; sp < len; sp++ )
			{
			    c_new_s = cs_data[sp] + c_old_s;
			    cs_data[sp] = c_new_s;
			    c_old_s = c_new_s;
			}
		    }
		}

		fclose( f );
	    }
	}
    }
}

void load_instrument( uint16_t num, char *filename, xm_struct *xm )
{
    module *song = xm->song;
    int instr_created = 0;
    char temp[8];
    int fn;
    for( fn = 0; ; fn++ ) if( filename[fn] == 0 ) break;
    for( ; fn >= 0; fn-- ) if( filename[fn] == '/' ) { fn++; break; }
    FILE *f = fopen( filename, "rb" );
    if( f )
    {
	fread( temp, 4, 1, f );
	
	if( temp[0] == 'E' && temp[1] == 'x' && temp[2] == 't' )
	{
	    load_xi_instrument( num, f, xm );
	    instr_created = 1;
	}
	if( temp[0] == 'R' && temp[1] == 'I' && temp[2] == 'F' )
	{
	    fread( temp, 8, 1, f );
	    if( temp[4] == 'W' && temp[5] == 'A' && temp[6] == 'V' )
	    {
		load_wav_instrument( num, f, filename + fn, xm );
		instr_created = 1;
	    }
	}
	
	fclose( f );
    }
    if( !instr_created )
    {
	//Instrument type not recognized
	//Load it as raw data:
	FILE *f = fopen( filename, "rb" );
	if( f )
	{
	    instr_created = load_raw_instrument( 
		num, 
		f, 
		filename + fn, 
		xm, 
		&wm );
	    fclose( f );
	}
    }
    if( num + 1 > song->instruments_num && instr_created )
    { //Increment number of instruments:
        song->instruments_num = num + 1;
    }
    for( int a = 0; a < num + 1; a++ )
    {
	if( song->instruments[ a ] == 0 )
	{ //Create new intrument:
	    new_instrument( a, "", 0, xm );
	}	
    }
}

int load_raw_instrument( uint16_t num, FILE *f, char *name, xm_struct *xm, window_manager *wm )
{
    int retval = 0;
    
    if( start_dialog_blocked( "Load as RAW data?", "Yes", "No", wm ) )
    {
	uint16_t bits = 8;
	uint16_t channels = 1;
	if( start_dialog_blocked( "Select bitrate", "16 bits", "8 bits", wm ) )
	    bits = 16;
	if( start_dialog_blocked( "Stereo or Mono?", "Stereo", "Mono", wm ) )	
	    channels = 2;

	module *song = xm->song;
    
	clear_instrument( num, xm );
	new_instrument( num, name, 1, xm );
	instrument *ins = song->instruments[ num ];
	sample *smp;
	char *smp_data;
	
	if( ins )
	{
	    int filesize = 0;
	    for(;;)
	    {
		if( getc( f ) == -1 ) break;
		filesize++;
		if( feof( f ) ) break;
	    }
	    rewind( f );
	    new_sample( 0, num, "", filesize, 16, xm );
	    smp = ins->samples[ 0 ];
	    if( smp )
	    {
	        smp_data = (char*) smp->data;
	        if( smp_data )
		{
		    fread( smp_data, filesize, 1, f ); //read sample data
		    //set sample info:
		    if( bits == 16 ) smp->type = 16; else smp->type = 0;
		    if( channels == 2 ) smp->type |= 64;
		    //convert sample info:
		    bytes2frames( smp, xm );
		}
	    }
	}
    }
    
    return retval;
}

void load_wav_instrument( uint16_t num, FILE *f, char *name, xm_struct *xm )
{
    uint chunk[2]; //Chunk type and size
    uint16_t channels = 1;
    uint16_t bits = 16;
    int other_info;
    module *song = xm->song;
    
    clear_instrument( num, xm );
    new_instrument( num, name, 1, xm );
    instrument *ins = song->instruments[ num ];
    sample *smp;
    char *smp_data;

    if( ins )
    for(;;)
    {
	if( feof( f ) != 0 ) break;
	slog( "WAV: loading CHUNK\n" );
	fread( &chunk, 8, 1, f );
	//slog( (char*)&chunk[0] );
	if( chunk[ 0 ] == 0x20746D66 ) //'fmt ':
	{
	    slog( "WAV: loading FMT\n" );
	    fseek( f, 2, 1 ); //Format
	    fread( &channels, 2, 1, f ); channels = channels;
	    fseek( f, 10, 1 ); //Some info
	    fread( &bits, 2, 1, f ); bits = bits;
	    other_info = 16 - chunk[1];
	    if( other_info ) fseek( f, other_info, 1 );
	} 
	else
	if( chunk[ 0 ] == 0x61746164 ) //'data':
	{
	    slog( "WAV: loading DATA\n" );
	    new_sample( 0, num, "", chunk[1], 16, xm );
	    smp = ins->samples[ 0 ];
	    if( smp )
	    {
	        smp_data = (char*) smp->data;
	        if( smp_data )
		{
		    slog( "WAV: loading SAMPLEDATA\n" );
		    slog( "WAV: size = %d\n", chunk[ 1 ] );
		    fread( smp_data, chunk[ 1 ], 1, f ); //read sample data
		    //set sample info:
		    if( bits == 16 ) smp->type = 16; else smp->type = 0;
		    if( channels == 2 ) smp->type |= 64;
		    //convert sample info:
		    bytes2frames( smp, xm );
		}
	    }
	    break;
	}
	else fseek( f, chunk[ 1 ], 1 );
    }
}

void load_xi_instrument( uint16_t num, FILE *f, xm_struct *xm )
{
    module *song = xm->song;
    char name[32];
    char temp[32];
    if( f )
    {
	fread( name, 21 - 4, 1, f ); // "Extended instrument: " 
	fread( name, 22, 1, f ); // Instrument name
	fread( temp, 23, 1, f );

	//Get version:
	int v1 = temp[ 21 ];
	int v2 = temp[ 22 ];
	int psytexx_ins = 0;
	if( v1 == 0x50 && v2 == 0x50 ) psytexx_ins = 1;
	
	clear_instrument( num, xm );
	new_instrument( num, name, 0, xm );
	
	instrument *ins = song->instruments[ num ];
	
	fread( ins->sample_number, 208, 1, f ); //Instrument info
	if( !psytexx_ins )
	{
	    //Unused 22 bytes :(
	    fread( temp, 22, 1, f );
	}
	else
	{
	    //Yea. 22 bytes for me :)
	    fread( &ins->volume, 2 + EXT_INST_BYTES, 1, f );
	    fread( temp, 22 - ( 2 + EXT_INST_BYTES ), 1, f );
	}
	fread( &ins->samples_num, 2, 1, f ); //Samples number
	
	//Create envelopes:
	create_envelope( ins->volume_points, ins->volume_points_num, ins->volume_env );
	create_envelope( ins->panning_points, ins->panning_points_num, ins->panning_env );
	
	//Load samples:
	int s;
	sample *smp;
	for( s = 0; s < ins->samples_num; s++ )
	{
	    smp = (sample*)mem_new( 0, sizeof( sample ), "sample" );
	    fread( smp, 40, 1, f ); //Load sample info
	    smp->data = 0;
	    if( smp->length )
		smp->data = (signed short*)mem_new( 1, smp->length, "sample data" );
	    smp->data = (signed short*) smp->data;
	    ins->samples[ s ] = smp;
	}
	//Load samples data:
	for( s = 0; s < ins->samples_num; s++ )
	{
	    smp = ins->samples[ s ];
	    char *smp_data = (char*) smp->data;
	    if( smp_data )
	    {
		int len = smp->length;
		fread( smp_data, len, 1, f ); //load data
		//convert it:
		int sp; 
		if( smp->type & 16 )
		{ //16bit sample:
		    signed short old_s = 0;
		    signed short *s_data = (signed short*) smp_data;
		    signed short new_s;
		    for( sp = 0; sp < len/2; sp++ )
		    {
			new_s = s_data[ sp ] + old_s;
			s_data[ sp ] = new_s;
			old_s = new_s;
		    }
		    //convert sample info:
		    bytes2frames( smp, xm );
		}
		else
		{ //8bit sample:
		    signed char c_old_s = 0;
		    signed char *cs_data = (signed char*) smp_data;
		    signed char c_new_s;
		    for( sp = 0; sp < len; sp++ )
		    {
			c_new_s = cs_data[sp] + c_old_s;
			cs_data[sp] = c_new_s;
			c_old_s = c_new_s;
		    }
		}
	    }
	}
    } //if( f )
}

#endif
