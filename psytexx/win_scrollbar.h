#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

#define BUTTON_SIZE 2
#define BUTTON_SIZE_STR "2"

extern char NEW_SCROLL_TYPE;

//INTERNAL STRUCTURES:

struct scrollbar_data
{
    char type;		//0 - vertical; 1 - horisontal
    int max;        	//Max value
    int cur;        	//Current value
    int scroll_size;	//Scrollbar size (in pixels) without buttons
    int slider_size_items;	//Size of real slider (in items)
    int slider_size;       	//Size of real slider (in pixels)
    int slider_pos;        	//Slider position (in pixels)
    int one_pixel_size;		//Size of one pixel (in items). 10 bit fixed point value
    int step;       		//One step size (default = 4)

    int this_window;
    int button_up;
    int button_down;

    char pressed;	//If pressed
    
    char we_are_in_slider;
    int start_x;
    int start_y;
    int start_value;
    
    char show_value_flag;
    char show_value_type;	//0 - hex; 1 - dec
    int show_value_offset;
    
    int (*handler)(void*,int,void*); 	//User defined handler: 
                                        //handler(void *user_data, int scrollbar_window, void *window_manager)
    void *user_data;                   	//Data for handler
};

//FUNCTIONS:

void scrollbar_check( int win_num, window_manager* );
void scrollbar_draw( int win_num, window_manager* );
void scrollbar_set_parameters( int win_num, int type, int max, int cur, int slider_size_items, window_manager* );
void scrollbar_set_cur_value( int win_num, int cur, window_manager* );
void scrollbar_set_step( int win_num, int step, window_manager* );
void scrollbar_set_value_showing( int win_num, 
                                  char show_value_flag, 
				  char show_value_type, 
				  int show_value_offset, 
				  window_manager *wm );
void scrollbar_set_handler( int (*)(void*,int,window_manager*), void *user_data, int win_num, window_manager* );

//HANDLERS:

int up_handler( void* user_data, int button_win, window_manager* );
int down_handler( void* user_data, int button_win, window_manager* );

//WINDOW HANDLERS:

int scrollbar_handler( wm_event*, window_manager* ); //Scrollbar handler
