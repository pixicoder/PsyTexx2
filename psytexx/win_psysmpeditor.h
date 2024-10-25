#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//PROPERTIES:

//INTERNAL STRUCTURES:

struct smpeditor_data
{
    int this_window;
    
    int scroll_volume;
    int scroll_pan;
    int scroll_finetune;
    int scroll_relative;
    int smpview;
    
    int button_close;
    int button_loop;
    int button_menu;
    int button_zoomin;
    int button_zoomout;
};

//FUNCTIONS:

void smpeditor_redraw( int win_num, window_manager *wm );
void smpeditor_draw_info( int win_num, window_manager *wm );

//HANDLERS:

int scroll_smpvol_handler( void *user_data, int win, window_manager* );
int scroll_smppan_handler( void *user_data, int win, window_manager* );
int scroll_smpfine_handler( void *user_data, int win, window_manager* );
int button_loop_handler( void *user_data, int win, window_manager* );
int button_menu_handler( void *user_data, int win_num, window_manager *wm );
int button_zoomin_handler( void *user_data, int win_num, window_manager *wm );
int button_zoomout_handler( void *user_data, int win_num, window_manager *wm );
int button_sclose_handler( void *user_data, int win, window_manager* );

//WINDOW HANDLERS:

int smpeditor_handler( wm_event*, window_manager* );
