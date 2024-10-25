#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

#define MAX_TEXT_LEN 512

extern int NEW_TEXT_SIZE; 

//INTERNAL STRUCTURES:

struct text_data
{
    int this_window;
    
    int readonly;
    uint8_t *text;
    int text_len;
    int cursor_position;
    int shift_status;
    
    int numerical_flag;	//Simple text field or numerical ( 0 or 1 )
    int button_left;    //Buttons for numerical text field
    int button_right;   //...
    int min_value;      //Value bounds for numerical text field
    int max_value;      //...
    
    uint8_t *caption;   //Text field caption ( default value = 0 )
    
    int active;         // = 0 after ENTER

    int (*handler)(void*,int,void*); 	//User defined handler: 
                                        //handler(void *user_data, int text_window, void *window_manager)
    void *user_data;                   	//Data for handler
};

//FUNCTIONS:

void text_set_text( int win_num, const char *text, window_manager* );
char* text_get_text( int win_num, window_manager* );
void text_set_readonly( int win_num, int readonly, window_manager* );
void text_set_numerical( int win_num, int numerical, window_manager* );
void text_set_caption( int win_num, const char *caption, window_manager* );
void text_set_bounds( int win_num, int min, int max, window_manager* );
void text_set_value( int win_num, int value, window_manager* );
int text_get_value( int win_num, window_manager* );
void text_set_handler( int (*)(void*,int,window_manager*), void *user_data, int win_num, window_manager* );

//HANDLERS:
int button_left_handler( void* user_data, int button_win, window_manager* );
int button_right_handler( void* user_data, int button_win, window_manager* );

//WINDOW HANDLERS:

int text_handler( wm_event*, window_manager* );
