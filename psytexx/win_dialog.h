#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//INTERNAL STRUCTURES:

struct dialog_data
{
    int this_window;
    int button_ok;
    int button_cancel;
    int current_button;
    
    const char *title;
    int title_len;
    
    int old_focus_window;
    
    void (*handler)(void*,int,window_manager*);
    void *user_data;
};

extern int dialog_visible;

//FUNCTIONS:

void start_dialog( const char *text, const char *ok, const char *cancel, 
                   void (*)(void*, int, window_manager*),   //(user data, pressed button, window_manager)
		   void *user_data,
                   int win_num, window_manager* );

int start_dialog_blocked( 
    const char *text, const char *ok, const char *cancel, 
    window_manager* );

//HANDLERS:

int ok_button_handler( void *user_data, int button_win, window_manager* );
int cancel_button_handler( void *user_data, int button_win, window_manager* );

//WINDOW HANDLERS:

int dialog_handler( wm_event*, window_manager* );
