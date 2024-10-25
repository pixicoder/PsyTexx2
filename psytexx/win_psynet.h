#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//INTERNAL STRUCTURES:

struct net_data
{
    int this_window;

    int ctls_window;
    int win_menu;
    int new_button;

    int list_window;
    int list_window_opened;
    int items_list;
    int items_ok;
    int items_cancel;

    int selected_item;

    int new_item_x;	//In precents
    int new_item_y;	//In precents

    int link_drag;

    int drag_started;
    int drag_start_x;
    int drag_start_y;
    int drag_item_x;
    int drag_item_y;

    int offset_x;	//In percents (0..1024)
    int offset_y;	//In percents (0..1024)
};

//FUNCTIONS:

void net_redraw( net_data *data, window_manager *wm );

//HANDLERS:

int new_button_handler( void *user_data, int button_win, window_manager *wm );
int item_ok_button_handler( void *user_data, int button_win, window_manager *wm );
int item_cancel_button_handler( void *user_data, int button_win, window_manager *wm );

//WINDOW HANDLERS:

int net_handler( wm_event*, window_manager* );
