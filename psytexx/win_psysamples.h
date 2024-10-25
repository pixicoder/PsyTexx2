#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

extern int current_instrument;
extern int current_sample;

//INTERNAL STRUCTURES:

struct samples_data
{
    int this_window;
    int instruments_list;
    int samples_list;
    
    int ins_edit_button;
    int smp_edit_button;
    int ins_page_button;

    int button;
    int ins_or_smp;

    const char *old_ys2;

    int current_instrument;
    int current_sample;
};

//FUNCTIONS:

void samples_refresh( int win_num, window_manager* );  //Refresh instrument list
void samples2_refresh( int win_num, window_manager* ); //Refresh sample list
void unscale_top_win( samples_data *data, window_manager *wm );

//HANDLERS:

int button_insteditor_handler( void *user_data, int win, window_manager *wm );
int button_smpeditor_handler( void *user_data, int win, window_manager *wm );
int smp_list_handler( void* user_data, int list_win, window_manager* );
int instr_list_handler( void* user_data, int list_win, window_manager* );

//WINDOW HANDLERS:

int samples_handler( wm_event*, window_manager* );
