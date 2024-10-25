#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

#define SCROLL_SIZE 2

//INTERNAL STRUCTURES:

#ifdef NOSTORAGE
    #define MAX_SIZE 16000
    #define MAX_ITEMS (int)4000
#else
    #define MAX_SIZE 65000
    #define MAX_ITEMS (int)16000
#endif

struct list_data
{
    int this_window;
    int button_up;  	//UP button
    int button_down;	//DOWN button
    int scrollbar;  	//scrollbar window
    char *items;     	//Items data
    int *items_ptr; 	//Item pointers
    int items_num;  	//Number of items; 
    int first_item; 	//First showed item;
    int selected_item;

    int numbered_flag;
    int number_offset;
    
    int pressed_button; //For the list handler

    char editable;   	//Editable flag (1/0)
    int edit_field; 	//Text window for editable lists

    int (*handler)(void*,int,void*); 	//User defined handler: 
                                        //handler(void *user_data, int list_window, void *window_manager)
    void *user_data;                   	//Data for handler
};

//FUNCTIONS:

void list_set_numbered( int numbered_flag, int offset, int win_num, window_manager* );
void list_set_editable( int editable_flag, int win_num, window_manager* );
void list_set_handler( int (*)(void*,int,window_manager*), void *user_data, int win_num, window_manager* );
void list_set_edit_handler( int (*)(void*,int,window_manager*), void *user_data, int win_num, window_manager* );
void list_draw( int win_num, window_manager* );                           //Redraw list-window content
void list_clear( int win_num, window_manager* );                          //Clear list-window
void list_reset_selection( int win_num, window_manager* );
void list_add_item( const char *item, char attr, int win_num, window_manager* );//Add new item (with attributes) to list-window
void list_delete_item( int item_num, int win_num, window_manager* );
void list_move_item_up( int item_num, int win_num, window_manager* );
void list_move_item_down( int item_num, int win_num, window_manager* );
char list_get_attr( int item_num, int win_num, window_manager* );        //Get item's attribute
char* list_get_item( int item_num, int win_num, window_manager* );       //Get item from the list-window
int list_get_selected_num( int win_num, window_manager* );               //Get selected item number (or -1)
int list_compare_items( int item1, int item2, int win_num, window_manager *wm );
void list_sort( int win_num, window_manager *wm );
void list_make_selection_visible( int win_num, window_manager *wm );

//HANDLERS:

int scroll_handler( void* user_data, int scroll_window, window_manager* );

//WINDOW HANDLERS:

int list_handler( wm_event*, window_manager* ); //List manager
