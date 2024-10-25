#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//INTERNAL STRUCTURES:

struct popup_data
{
    int this_window;

    const char *text;

    int result;
    int selected;

    int old_focus_window;
};

//FUNCTIONS:

int start_popup_blocked( 
    const char *text,
    int x, int y,
    window_manager* );

//HANDLERS:

//WINDOW HANDLERS:

int popup_handler( wm_event*, window_manager* );
