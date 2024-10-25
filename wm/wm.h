#pragma once

//window size quantum = 1 text character (for example: 8x8);
//you need to redraw all windows, if some window was moved or resized;
//root window or dialog must use draw_window_touch_area() on DRAW event handling;
//child windows (buttons, scrollbars etc) must use draw_touch_box() on DRAW event handling.

#include "../core/core.h"
#include "wm_struct.h"

extern int pscreen_x_size;   //Screen x-size (in pixels)
extern int pscreen_y_size;   //Screen y-size (is pixels)
extern int char_x_size;      //One char (symbol) x-size
extern int char_y_size;      //One char (symbol) y-size

#ifdef OS_LINUX
extern int mouse_auto_keyup;
#endif

extern unsigned char font[2048];

//################################
//## MAIN FUNCTIONS:            ##
//################################

void win_init( window_manager* );			    //init window manager
void win_close( window_manager* );			    //close window manager
void resize_all_windows( window_manager* );		    //resize (using win string controls) all windows
void resize_all( int, window_manager* );		    //resize (reinit) all screens and windows

//################################
//## EVENT FUNCTIONS:           ##
//################################

void event_loop(window_manager*, int loop);		    //main event loop
void set_eventloop_handler( void (*)( window_manager* ), window_manager * ); //set user-defined eventloop handler
int  process_event(wm_event*,window_manager*);		    //process event
void send_event(int,int,int x,int y,int button, int pressure, int mode, window_manager*);
							    //send event to some window
//################################
//## WIN HANDLERS:              ##
//################################

int null_handler(wm_event*,window_manager*);		    //SIMPLE WIN HANDLER: null window
int child_handler(wm_event*,window_manager*);		    //SIMPLE WIN HANDLER: child window
int desktop_handler(wm_event*,window_manager*);	    	    //SIMPLE WIN HANDLER: desktop window
int create_scrollarea(int win_num,window_manager*);	    //
int scrollarea_handler(wm_event*,window_manager*);	    //SIMPLE WIN HANDLER: small scroll buttons for an window
int keyboard_handler(wm_event*,window_manager*);	    //SIMPLE WIN HANDLER: virtual keyboard
void show_keyboard( int kbd_win, window_manager *wm, int show ); //show: -1 - switch; 0 - hide; 1 - show;

//################################
//## TIME FUNCTIONS:            ##
//################################

void wm_timer_set( uint t, void (*)( void*, window_manager* ), void *data, window_manager *wm ); //Set timer. 1024 = one second
void wm_timer_close( window_manager *wm );

//################################
//## CONNECTION WITH THE OS:    ##
//################################

void push_button(int x,int y,int button,int pressure,   //Send button press/move/unpress event to the window manager
                 int type,window_manager *wm);		//x/y: coordinates on the screen (in pixels)
		                                        //button: 1 - left; 2 - middle; 4 - right; 8 - keyboard...
							//pressure: button pressure (for MIDI or Digital Pen)
							//type: 0 - down; 1 - up; 2 - move
							//This function must be called by OS

//################################
//## WINDOW FUNCTIONS:          ##
//################################

int create_window(const char *name,
                   int x,int y,int x_size, int y_size, int color, int draw_box,
                   int parent, 
		   int (*)(wm_event*,window_manager*), 
		   window_manager*);
void close_window(int win_number, window_manager*);
void create_child(int parent, int child_win, window_manager *wm);
void move_window( int win_num, int x, int y, window_manager *wm );
void calculate_real_window_position( int win_num, window_manager *wm ); //Calculate real window values
void calculate_real_window_position_with_childs( int win_num, window_manager *wm ); //Calculate real window values
void set_window_string_controls( int win_num, const char *xs, const char *ys, const char *xs2, const char *ys2, window_manager *wm );

//################################
//## DRAWING FUNCTIONS:         ##
//################################

void int_to_string( int value, char *str );   //DEC
void int_to_string_h( int value, char *str ); //HEX
void ext_int_to_string( int value, char *str, int chars, window_manager* );
int get_color( uint8_t r, uint8_t g, uint8_t b );            //Get color value by RGB
int red( COLOR c );
int green( COLOR c );
int blue( COLOR c );
COLOR blend( COLOR c1, COLOR c2, int value );
void load_font( window_manager* );                           //Load user font (font.bmp)
void load_background( window_manager* );                 //Load user background (back.bmp) and init color gradient
void draw_string(int,int,int,int,const char*,window_manager*); //draw text string (relative x/y in pixels) in some window
void draw_pixel( int win_num, int x, int y, int color, window_manager *wm );
void draw_rectangle( int win_num, int x, int y, int x_size, int y_size, int color, window_manager *wm );
void draw_rectangle2( int win_num, int x, int y, int x_size, int y_size, int color, window_manager *wm );
void draw_line( int win_num, int x1, int y1, int x2, int y2, COLOR color, window_manager *wm );
void draw_vert_line( int win_num, int x, int y, int y_size, int color, int add, window_manager *wm );
void draw_horis_line( int win_num, int x, int y, int x_size, int color, int add, window_manager *wm );
void pdraw_box(int win_num, int x, int y,                      //Draw box (ralative x/y in pixels) in some window
               int x_size, int y_size, int color,
 	       window_manager*);                    
void draw_box(int win_num, int x, int y,                       //Draw box (ralative x/y in symbols) in some window
              int x_size, int y_size, int color,
	      int chr, window_manager*); 
//Draw touch box (for the new child-window) on exist window:
void draw_touch_box( int back_win, int x, int y, int x_size, int y_size,
                     int fore_win, window_manager *wm );
void matrix_draw_string(int,int,int,uint8_t,const char*,window_manager*); //draw text string in matrix
                                                                          //(relative x/y in symbols)
void matrix_draw_box(int win_num, int x, int y,                       //Draw box in matrix
                     int x_size, int y_size, int color,               //(ralative x/y in symbols)
	             window_manager*);                    
void matrix_clear_box(int win_num, int x, int y,                      //Clear box in matrix
                      int x_size, int y_size,                         //(ralative x/y in symbols)
	              window_manager*);                    
void matrix_draw( window_manager* );
void draw_window_box(int,window_manager*);                  //draw window box
//Draw touch box (for the new parent window) over all windows:
void draw_window_touch_area(int,window_manager*);           //draw window touch area
void redraw_screen(window_manager*);                        //redraw full screen

//################################
//## DEVICE DEPENDENT:          ##
//################################

void create_active_screen_part(window_manager*);        //DEVICE DEPENDENT: create active screen part
void close_active_screen_part(window_manager*);         //DEVICE DEPENDENT: close active screen part
void fast_draw_char(int ptr,int,int,uint8_t,window_manager*);//DEVICE DEPENDENT: fast draw symbol
void draw_char(int,int,int,int,window_manager*);        //DEVICE DEPENDENT: draw one symbol (x/y in pixels)
void draw_screen_part(int,window_manager*);             //DEVICE DEPENDENT: draw active screen part
void draw_screen(window_manager*);                      //DEVICE DEPENDENT: draw changed parts of the screen
void set_palette(window_manager*);                      //DEVICE DEPENDENT: set RGB palette

//################################
//## DECLARED IN EVENTLOOP.C:   ##
//################################

int device_event_handler(window_manager*);              //DEVICE DEPENDENT: device event handler (return 1 for EXIT)
void device_start(window_manager*);                     //DEVICE DEPENDENT: device start (before main loop)
void device_end(window_manager*);                       //DEVICE DEPENDENT: device end   (after main loop)
