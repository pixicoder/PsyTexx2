#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//INTERNAL STRUCTURES:

struct channels_data
{
    int this_window;

    int mouse_button;
};

//FUNCTIONS:

void channels_redraw( int win_num, window_manager* );

//WINDOW HANDLERS:

int channels_handler( wm_event*, window_manager* ); //Channels (scopes) handler
