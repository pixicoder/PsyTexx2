#pragma once

//Main sound header

#include "../core/core.h"

//Structures:

enum
{
    STATUS_STOP = 0,
    STATUS_PLAY,
};

struct sound_struct
{
    int		status;		//Current playing status
    int		need_to_stop;	//Set it to 1 if you want to stop sound stream
    volatile int stream_stoped;	//If stream really stoped

    void	*user_data;	//Data for user defined render_piece_of_sound()
    
    int		buflen;		//Number of frames
};

//Variables:

extern void *user_sound_data;

#ifdef OS_LINUX
extern int dsp;
#endif

//Functions:

int main_callback( void*, int, void*, int );
extern void render_piece_of_sound( signed short *buffer, int buffer_size, void *user_data );

void sound_stream_init(void);
void sound_stream_play(void);
void sound_stream_stop(void);
void sound_stream_close(void);
