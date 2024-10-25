#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

//NEW TABLE: =======================
//(SET IT UP BEFORE WINDOW CREATION)
extern int table_type; 
extern int CREATE_TABLE_WITH_EDIT_BUTTON;
//==================================

enum field_type
{
    FIELD_NONE = 0,
    FIELD_CHANNEL,
    FIELD_PATTERN_NUM,
    FIELD_PATTERNTABLE_POS,
    FIELD_NOTE,
    FIELD_PATTERN_POS,
    FIELD_LAST
};

enum element_type
{
    ELEMENT_NONE = 0,
    ELEMENT_PATTERN_NUM,
    ELEMENT_NOTE,
    ELEMENT_INSTRUMENT,
    ELEMENT_VOLUME,
    ELEMENT_EFFECT,
    ELEMENT_PARAMETER,
    ELEMENT_DIVIDER,
    ELEMENT_LAST
};

extern uint8_t element_xsize[ ELEMENT_LAST ];

#define MAX_ELEMENTS           8
#define TYPE_VSCROLL           1
#define TYPE_HSCROLL           2
#define TYPE_DIVIDERS          4
#define HALIGN                 1
#define VALIGN                 2


//TABLE:
//
//#######################
//#   #
//#   #        H
//#   #
//#######################
//#   #    |    |    |
//#   #    |    |    |
//#   #------------------
//# V #    |    |    |
//#   #    |  CELLS  |
//#   #------------------
//#   #    |    |    |
//
//H - horisontal field
//V - vertical field
//
//One cell = array of elements

//INTERNAL STRUCTURES:

struct table_data
{
    int this_window;

    int h_scroll;	//HScroll window
    int v_scroll;    	//VScroll window

    int type;        	//Table type: TYPE_xxx
    int scroll_size; 	//Size of scrollbars
    
    int vertical_field;		//check the field_type enum
    int horisontal_field; 	//check the field_type enum
    int vertical_field_size;
    int horisontal_field_size;
    
    int elements_align;   		//HALIGN or VALIGN (horisontal or vertical)
    int elements[ MAX_ELEMENTS ]; 	//One cell of the table (visible)
    int elements_num;     		//Number of elements in one cell
    int full_elements[ MAX_ELEMENTS ]; 	//Full cell of the table
    int full_elements_num; 		//Number of elements in full cell
    int cell_xsize;
    int cell_ysize;
    
    int xstart;		//Start position in horisontal field
    int ystart;  	//Start position in vertical field
    int v_offset;	//Offset in vertical field
    int h_offset;	//Offset in horisontal field
    int v_cells; 	//Number of vertical visible cells in current window
    int h_cells; 	//Number of horisontal visible cells in current window
    
    int current_cell;		//Number of current cell
    int current_element; 	//Number of current element
    int current_offset;  	//Number of current element offset
    
    uint8_t scroll_flag;
    
    //Selection:
    int selection;	//0 - finished; 1 - user selecting new region
    int sel_x;
    int sel_y;
    int sel_xsize;
    int sel_ysize;

    int fullscreen_button;
    int edit_button;
    uint8_t fullscreen;		//1 - fullscreen button enabled
    uint8_t fullscreen_status; 	//1 - maximized
    int change_view_button;
    uint8_t change_view;       	//1 - "change view" button enabled
    uint8_t change_view_status;
    const char *old_y_string;
    
    //Tracker specific variables:
    int record_status;   	//Record mode ON/OFF
    COLOR record_color;
    
    void *copy_buffer[3]; 	//Full table (pattern) / Block / Column (channel)
    int copy_buffer_xsize[3];
    int copy_buffer_ysize[3];
};

//FUNCTIONS:

int table_field_values( int field_type ); //Get number of field values
char* table_field_text( int field_type, int value, window_manager* ); //Convert field value to text
int table_field_value( int field_type ); //Get current (real) field value
void table_field_set_value( int field_type, int new_value );
int table_get_element_value( table_data *data, int element_type );
int table_set_element_value( table_data *data, int element_type, int element_value );
void table_cell_draw( table_data *data,
                      int win_num,
		      int x,
		      int y,
		      int highlight,
		      int current_cell_flag,
		      window_manager* );
void table_draw_record_status( int win_num, char, window_manager* );
void table_draw( int win_num, window_manager* );
void table_new( int win_num,
		int type,
                int vertical_field,
		int horisontal_field,
		int elements_align,
		int elements_num,
		window_manager*,
		... );
void table_full_cell( int win_num, int elements_num, window_manager*, ... );
void table_set_type( int type_flags, int win_num, window_manager* );
void table_set_fullscreen( int fullscreen, int win_num, window_manager* );
void table_set_change_view( int change_view, int win_num, window_manager* );
void handle_key( uint key, int win_num, window_manager* );

//HANDLERS:

int change_view_button_handler( void* user_data, int button_win, window_manager* wm );
int fullscreen_button_handler( void* user_data, int button_win, window_manager* wm );
int v_scroll_handler(void *user_data, int scroll_win, window_manager*);
int h_scroll_handler(void *user_data, int scroll_win, window_manager*);

//WINDOW HANDLERS:

int table_handler( wm_event*, window_manager* );
