#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//INTERNAL STRUCTURES:

struct menu_data
{
    int play_button;
    int patplay_button;
    int stop_button;
    int net_button;
    int files_button;
    int playlist_button;
    int kbd_button;
    int config_button;
    int clear_button;

    int menu_button;
    
    int files_status;
    int config_status;
    int playlist_status;
};

//FUNCTIONS:

//HANDLERS:

int play_button_handler( void *user_data, int button_win, window_manager* );
int patplay_button_handler( void *user_data, int button_win, window_manager* );
int stop_button_handler( void *user_data, int button_win, window_manager* );
int net_button_handler( void *user_data, int button_win, window_manager* );
int files_button_handler( void *user_data, int button_win, window_manager* );
int kbd_button_handler( void *user_data, int button_win, window_manager* );
int config_button_handler( void *user_data, int button_win, window_manager* );
int playlist_button_handler( void *user_data, int button_win, window_manager* );
int clear_button_handler( void *user_data, int button_win, window_manager* );

//WINDOW HANDLERS:

int menu_handler( wm_event*, window_manager* );
