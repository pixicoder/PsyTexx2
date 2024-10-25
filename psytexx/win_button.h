#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

extern char txt_up[4];
extern char txt_down[4];
extern char txt_left[4];
extern char txt_right[4];

//INTERNAL STRUCTURES:

struct button_data
{
    int this_window;
    const char *name; 	//Button name
    int len;       	//Name length (in chars)
    char pressed;   	//If pressed
    int color;     	//Button color
    int x;         	//Name x-position (in pixels)
    int y;         	//Name y-position (in pixels)
    char autorepeat;	//Auto-repeat mode ON/OFF
    char autorepeat_pressed;
    int (*handler)(void*,int,void*); 	//User defined handler: 
                                        //handler(void *user_data, int button_window, void *window_manager)
    void *user_data;                   	//Data for handler
};

//FUNCTIONS:

void button_set_color( COLOR color, int win_num, window_manager* );
void button_set_name( const char *name, int win_num, window_manager* ); 
void button_set_handler( int (*)(void*,int,window_manager*), void *user_data, int win_num, window_manager* );
void button_set_autorepeat( char autorepeat, int win_num, window_manager* wm );

//WINDOW HANDLERS:

int button_handler( wm_event*, window_manager* ); //Button handler
