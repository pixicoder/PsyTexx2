#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//PROPERTIES:

//INTERNAL STRUCTURES:

struct insteditor_data
{
    int this_window;
    
    int pars_window;

    int scroll_volume;
    int scroll_pan;
    int scroll_finetune;
    int scroll_relative;
    int scroll_vibsweep;
    int scroll_vibdepth;
    int scroll_vibrate;
    int button_death;
    
    int button_close;
};

//FUNCTIONS:

void insteditor_redraw( int win_num, window_manager *wm );

//HANDLERS:

int scroll_instvol_handler( void *user_data, int win, window_manager* );
int scroll_instpan_handler( void *user_data, int win, window_manager* );
int scroll_instfine_handler( void *user_data, int win, window_manager* );
int scroll_instrelative_handler( void *user_data, int win, window_manager *wm );
int scroll_instsweep_handler( void *user_data, int win, window_manager *wm );
int scroll_instdepth_handler( void *user_data, int win, window_manager *wm );
int scroll_instrate_handler( void *user_data, int win, window_manager *wm );
int button_iclose_handler( void *user_data, int win, window_manager* );

//WINDOW HANDLERS:

int insteditor_handler( wm_event*, window_manager* );
