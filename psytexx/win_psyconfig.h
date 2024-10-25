#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//PROPERTIES:
#define PROP_VOLUME    ".prop_config_vol"

//INTERNAL STRUCTURES:

struct config_data
{
    int this_window;
    
    int scroll_volume;
    int button_close;
    
    int volume_changed; //-1 - not changed; 0... - changed volume;
};

//FUNCTIONS:

//HANDLERS:

int scroll_vol_handler( void *user_data, int win, window_manager* );
int button_cclose_handler( void *user_data, int win, window_manager* );

//WINDOW HANDLERS:

int config_handler( wm_event*, window_manager* );

