#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//PROPERTIES:
#define PROP_PLAYLIST   ".prop_playlist" 

//INTERNAL STRUCTURES:

struct playlist_data
{
    int this_window;
    
    int button_close;
    int button_addfile;
    int button_addfiles;
    int button_delfile;
    int button_up;
    int button_down;
    int button_clear;
    int button_play;
    int button_random;
    int button_next;
    int button_prev;
    
    int list_files;
    
    int current_selected;
    
    int random_mode;
};

//FUNCTIONS:

void playlist_play_selected( int win_num, window_manager *wm );
void playlist_select_next_track( int win_num, window_manager *wm );
void playlist_select_previous_track( int win_num, window_manager *wm );
void playlist_save( const char *filename, int win_num, window_manager *wm );
void playlist_load( const char *filename, int win_num, window_manager *wm );

//HANDLERS:

int button_pclose_handler( void *user_data, int win, window_manager *wm );
int button_addfile_handler( void *user_data, int win, window_manager *wm );
int button_addfiles_handler( void *user_data, int win, window_manager *wm );
int button_delfile_handler( void *user_data, int win, window_manager *wm );
int button_pup_handler( void *user_data, int win, window_manager *wm );
int button_pdown_handler( void *user_data, int win, window_manager *wm );
int button_plplay_handler( void *user_data, int win, window_manager *wm );
int button_random_handler( void *user_data, int win, window_manager *wm );
int button_plnext_handler( void *user_data, int win, window_manager *wm );
int button_plprev_handler( void *user_data, int win, window_manager *wm );

//WINDOW HANDLERS:

int playlist_handler( wm_event*, window_manager* );
