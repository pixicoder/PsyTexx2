#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//INTERNAL STRUCTURES:

struct patterntab_data
{
    int this_window;	//this window handler
    //Buttons:
    int button_inc;
    int button_dec;
    int button_ins;
    int button_del;
};

//FUNCTIONS:

//HANDLERS:

int button_inc_handler( void* user_data, int button_win, window_manager* );
int button_dec_handler( void* user_data, int button_win, window_manager* );
int button_ins_handler( void* user_data, int button_win, window_manager* );
int button_del_handler2( void* user_data, int button_win, window_manager* );

//WINDOW HANDLERS:

int patterntab_handler( wm_event*, window_manager* ); //Pattern properties
