/*
    sound_player.cpp - main sound callback
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2007 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "sound.h"

int main_callback( void *userData,
                   int stream,
                   void *_buffer,
                   int frameCount )
{
    //main variables: ============
    signed short *buffer = (signed short*) _buffer;
    int buffer_size = frameCount;
    sound_struct *U = (sound_struct*) userData;
    
    //clear buffer: ==============
    for( int i = 0; i < buffer_size * 2; i += 2 ) { buffer[ i ] = 0; buffer[ i + 1 ] = 0; }
    //============================

    //for stream stop: ===========
    if( U->need_to_stop ) { U->stream_stoped = 1; return 0; }
    //============================
    
    //render piece of sound: =====
    render_piece_of_sound( buffer, buffer_size, U->user_data );
    //============================

    return 0;
}

