#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

extern int text_bpm;
extern int text_speed;
extern int text_patsize;
extern int text_add;
extern int text_channels;

extern uint psytexx_start_time;
extern uint play_start_time;
extern uint playing_time;

//INTERNAL STRUCTURES:

struct pattern_data
{
    int this_window;	//this window handler
    //Text windows:
    int text_bpm;      	//Global BPM
    int text_speed;    	//Global Speed
    int text_patsize;   //Current pattern size
    int text_add;       //Write note step
    int text_channels;  //Number of channels
    //Channels window:
    int channels;
};

//FUNCTIONS:

void psypattern_redraw_channels( int win_num, window_manager *wm );
void psypattern_draw_time( int win_num, window_manager *wm );

//HANDLERS:

int text_bpm_handler( void* user_data, int button_win, window_manager* );
int text_speed_handler( void* user_data, int button_win, window_manager* );
int text_patsize_handler( void* user_data, int button_win, window_manager* );
int text_add_handler( void* user_data, int button_win, window_manager* );
int text_channels_handler( void* user_data, int button_win, window_manager* );

//WINDOW HANDLERS:

int pattern_handler( wm_event*, window_manager* ); //Pattern properties
