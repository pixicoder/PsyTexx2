/*
    wm_linux_console.h - platform-dependent module : Linux console
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#pragma once

//#################################
//## DEVICE DEPENDENT FUNCTIONS: ##
//#################################

#include <termios.h>
//#include <linux/keyboard.h>
#include <linux/kd.h>
//#include <linux/vt.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h> //timeval struct

static pthread_t pt; //our mouse thread
static struct termio savetty, settty; //for set_term() and reset_term()
extern int xmouse_flag;
int mouse_auto_keyup;
static Gpm_Connect conn;

window_manager *current_wm;

#define KEYBOARD "/dev/tty"
static int fd; //File descriptor for keyboard
//Original keyboard modes:
static int old_kbd_mode = 0;
static termios old;
static pthread_t key_th; //our keyboard thread

void get_terminal_size(void);
void set_term();
void reset_term();
int mouse_handler(Gpm_Event *event, void *data);
void *mouse_thread(void *arg);
void mouse_init(void);
void mouse_close(void);
int keyboard_init(void);
int keyboard_close(void);

void small_pause( int milliseconds )
{
    timeval t;
    t.tv_sec = 0;
    t.tv_usec = (int) (milliseconds % 1000) * 1000;
    select( 0 + 1, 0, 0, 0, &t );
    draw_screen( current_wm );
}

void device_start( window_manager *wm )
{
    current_wm = wm;
    char_x_size = 1;
    char_y_size = 1;
    pscreen_x_size = 80;
    pscreen_y_size = 40;
    mouse_init();
    //keyboard_init();
    printf( "\e[%d;%dH", pscreen_y_size+1, 1 ); //"CSI n ; m H" Moves the cursor to row n, column m. The values are 1-based
    printf( " -= Music Rules the World =-                          " );
}

void device_end( window_manager *wm )
{
    //keyboard_close();
    mouse_close();
    printf( "\e[0m" ); //All attributes become turned off
    //printf( "\e[l" ); //??
    printf( "\e[999B"); //Cursor down
    printf( "\e[999D"); //Cursor Back
}

int device_event_handler( window_manager *wm )
{
    if( wm->events_count == 0 ) small_pause( 20 );
    if( wm->exit_flag ) return 1;
    return 0;
}

//Get console window size:
void get_terminal_size(void)
{
    int read_fd;
    char str[256];
    int x,y;
    fprintf(stderr,"\e[r");	  /* clear scrolling region */
    fprintf(stderr,"\e[255;255H"); /* go to lower right of screen */
    fprintf(stderr,"\e[6n");	  /* query current cursor location */
    read_fd = fileno(stdin);
    read(read_fd,str,256);
    sscanf(str, "\e[%d;%dR", &pscreen_y_size, &pscreen_x_size);
    pscreen_y_size--;
}

//Make possible getchar() without "enter" at the end of string
void set_term()
{
    ioctl(0, TCGETA, &savetty);
    ioctl(0, TCGETA, &settty);
    settty.c_lflag &= ICANON;
    settty.c_lflag &= ECHO;
    ioctl(0, TCSETAF, &settty);
}

//Reset terminal at the end of our program
void reset_term()
{
    ioctl(0, TCSETAF, &savetty);
}

int mouse_handler(Gpm_Event *event, void *data)
{
    int buttons = 0, type = 0;

    if(event->type & GPM_DOWN) type = 0;
    if(event->type & GPM_UP) type = 1;
    if(event->type & GPM_MOVE) type = 2;
    if(event->buttons & GPM_B_LEFT) buttons |= 1;
    if(event->buttons & GPM_B_MIDDLE) buttons |= 2;
    if(event->buttons & GPM_B_RIGHT) buttons |= 4;
    if(event->buttons & GPM_B_UP) buttons = KEY_SCROLLUP << 3;
    if(event->buttons & GPM_B_DOWN) buttons = KEY_SCROLLDOWN << 3;

    push_button( event->x * wm.char_x, 
                 event->y * wm.char_y,
		 buttons, 1023, type, &wm );
    if( mouse_auto_keyup )
    {
	if( type == 0 && buttons != ( KEY_SCROLLDOWN << 3 ) )
	{ //Auto key up:
	    type = 1;
	    push_button( event->x * wm.char_x, 
        	         event->y * wm.char_y,
			 buttons, 1023, type, &wm );
	}
    }

    return 0;
}

//Mouse and keyboard events processing:  (Console version)
void *mouse_thread(void *arg)
{
    int c;
    char temp[128];
    
    while((c=Gpm_Getc(stdin)) != EOF) 
    { 
	if( c == 27 )
	{ //ESCAPE codes:
	    c = Gpm_Getc( stdin ); //Get next key
	    switch( c )
	    {
		case 91:
		c = Gpm_Getc( stdin );
		switch( c )
		{
		    case 68: // LEFT
		    push_button( 0, 0, KEY_LEFT << 3, 1023, 0, current_wm );
		    break;

		    case 67: // RIGHT
		    push_button( 0, 0, KEY_RIGHT << 3, 1023, 0, current_wm );
		    break;

		    case 65: // UP
		    push_button( 0, 0, KEY_UP << 3, 1023, 0, current_wm );
		    break;

		    case 66: // DOWN
		    push_button( 0, 0, KEY_DOWN << 3, 1023, 0, current_wm );
		    break;
		    
		    case 50: 
		    c = Gpm_Getc( stdin );
		    if( c == 126 ) // INSERT
			push_button( 0, 0, KEY_INSERT << 3, 1023, 0, current_wm );
		    break;
		    
		    case 51:
		    c = Gpm_Getc( stdin );
		    if( c == 126 ) // DELETE
			push_button( 0, 0, KEY_DELETE << 3, 1023, 0, current_wm );
		    break;
		    
		    case 53:
		    c = Gpm_Getc( stdin );
		    if( c == 126 ) // PAGEUP
			push_button( 0, 0, KEY_PAGEUP << 3, 1023, 0, current_wm );
		    break;
		    
		    case 54:
		    c = Gpm_Getc( stdin );
		    if( c == 126 ) // PAGEDOWN
			push_button( 0, 0, KEY_PAGEDOWN << 3, 1023, 0, current_wm );
		    break;
		    
		    case 52:
		    c = Gpm_Getc( stdin );
		    if( c == 126 ) // END
			push_button( 0, 0, KEY_END << 3, 1023, 0, current_wm );
		    break;
		    
		    case 49: // F5 F6 F7 F8 ...
		    c = Gpm_Getc( stdin );
		    switch( c )
		    {
			case 126: //HOME (console version)
			push_button( 0, 0, KEY_HOME << 3, 1023, 0, current_wm );
			break;
			
			case 53: // F5
			c = Gpm_Getc( stdin );
			if( c == 126 )
			    push_button( 0, 0, KEY_F5 << 3, 1023, 0, current_wm );
			if( c == 59 )
			{
			    c = Gpm_Getc( stdin );
			    if( c == 50 )
			    { // SHIFT + F5
				push_button( 0, 0, (KEY_F5+KEY_SHIFT) << 3, 1023, 0, current_wm );
			    }
			}
			break;

			case 55: // F6
			c = Gpm_Getc( stdin );
			if( c == 126 )
			    push_button( 0, 0, KEY_F6 << 3, 1023, 0, current_wm );
			if( c == 59 )
			{
			    c = Gpm_Getc( stdin );
			    if( c == 50 )
			    { // SHIFT + F6
				push_button( 0, 0, (KEY_F6+KEY_SHIFT) << 3, 1023, 0, current_wm );
			    }
			}
			break;

			case 56: // F7
			c = Gpm_Getc( stdin );
			if( c == 126 )
			    push_button( 0, 0, KEY_F7 << 3, 1023, 0, current_wm );
			if( c == 59 )
			{
			    c = Gpm_Getc( stdin );
			    if( c == 50 )
			    { // SHIFT + F7
				push_button( 0, 0, (KEY_F7+KEY_SHIFT) << 3, 1023, 0, current_wm );
			    }
			}
			break;

			case 57: // F8
			c = Gpm_Getc( stdin );
			if( c == 126 )
			    push_button( 0, 0, KEY_F8 << 3, 1023, 0, current_wm );
			if( c == 59 )
			{
			    c = Gpm_Getc( stdin );
			    if( c == 50 )
			    { // SHIFT + F8
				push_button( 0, 0, (KEY_F8+KEY_SHIFT) << 3, 1023, 0, current_wm );
			    }
			}
			break;
		    }
		    break;
		}
		break;
		
		case 79:
		c = Gpm_Getc( stdin );
		switch( c )
		{
		    case 72: // HOME
		    push_button( 0, 0, KEY_HOME << 3, 1023, 0, current_wm );
		    break;
		    
		    case 70: // END
		    push_button( 0, 0, KEY_END << 3, 1023, 0, current_wm );
		    break;
		    
		    case 50: // SHIFT +
		    c = Gpm_Getc( stdin );
		    switch( c )
		    {
			case 80: // F1
			push_button( 0, 0, (KEY_F1+KEY_SHIFT) << 3, 1023, 0, current_wm );
			break;

			case 81: // F2
			push_button( 0, 0, (KEY_F2+KEY_SHIFT) << 3, 1023, 0, current_wm );
			break;

			case 82: // F3
			push_button( 0, 0, (KEY_F3+KEY_SHIFT) << 3, 1023, 0, current_wm );
			break;

			case 83: // F4
			push_button( 0, 0, (KEY_F4+KEY_SHIFT) << 3, 1023, 0, current_wm );
			break;
		    }
		    break;
		    
		    case 80: // F1
		    push_button( 0, 0, KEY_F1 << 3, 1023, 0, current_wm );
		    break;

		    case 81: // F2
		    push_button( 0, 0, KEY_F2 << 3, 1023, 0, current_wm );
		    break;

		    case 82: // F3
		    push_button( 0, 0, KEY_F3 << 3, 1023, 0, current_wm );
		    break;

		    case 83: // F4
		    push_button( 0, 0, KEY_F4 << 3, 1023, 0, current_wm );
		    break;
		}
		break;
		
		case 27: //just an ESC key:
		push_button( 0, 0, KEY_ESCAPE << 3, 1023, 0, current_wm );
		break;
	    }
	}
	else
	{
	    //other key-codes:
	    push_button( 0, 0, c << 3, 1023, 0, current_wm );
	    //if( c == 'q' ) wm.exit_flag = 1; 
	}
    }
    
    printf("Mouse thread closed\n");
    pthread_exit(0);
    return 0;
}

//Mouse and console init:
void mouse_init(void)
{
    //GPM init:
    //conn.eventMask  = ~GPM_MOVE;   /* Want to know about all the events */
    //conn.defaultMask = GPM_MOVE;   /* don't handle anything by default  */
    conn.eventMask  = ~0;   /* Want to know about all the events */
    conn.defaultMask = 0;   /* don't handle anything by default  */
    conn.minMod     = 0;    /* want everything                   */
    conn.maxMod     = 0;    /* all modifiers included            */

    if(Gpm_Open(&conn, 0) == -1)  printf("Cannot connect to mouse server");

    //GPM_XTERM_ON;
    gpm_handler = mouse_handler; //our mouse handler 
    gpm_zerobased = 1;
    gpm_visiblepointer = 1;
    set_term();
    get_terminal_size();         //get size of our screen

    //create thread for mouse processing:
    if( pthread_create(&pt,NULL,mouse_thread,0) != 0)
    {
	printf("Can't create mouse thread :(\n");
	return;
    }
}

void mouse_close(void)
{
    reset_term();
    pthread_cancel( pt );
    printf("GPM_FD = %d\n",gpm_fd);
    Gpm_Close();
}

int keyboard_init(void)
{
    char *kbd;
    if( !(kbd = getenv("CONSOLE")) )
	kbd = (char*)KEYBOARD;
    fd = open( kbd, O_RDONLY );//| O_NONBLOCK );
    if( fd < 0 )
    {
	slog( "Can't open keyboard\n" );
	return -1;
    }

    //Save previous settings:
    if( ioctl( fd, KDGKBMODE, &old_kbd_mode ) < 0 )
    {
	slog( "Can't save previous keyboard settings\n" );
	switch( errno )
	{
	    case EBADF: slog( "invalid descriptor\n" );
	    case EFAULT: slog( "memory error\n" );
	    case ENOTTY: slog( "not character device\n" );
	    case EINVAL: slog( "invalid request\n" );
	}
	return -1;
    }
    if( tcgetattr( fd, &old ) < 0 )
    {
	slog( "Can't save previous keyboard settings (2)\n" );
	return -1;
    }

    //Set medium-raw keyboard mode: (for scancodes getting)
    termios n;
    n = old;
    n.c_lflag &= ~( ICANON | ECHO /*| ISIG*/ );
    n.c_iflag &= ~( ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON /*| BRKINT*/ );  // :-(    )
    n.c_cc[ VMIN ] = 0;
    n.c_cc[ VTIME ] = 0;
    if( tcsetattr( fd, TCSAFLUSH, &n ) < 0 )
    {
	slog( "tcsetattr() error in keyboard_init()\n" );
	keyboard_close();
	return -1;
    }
    if( ioctl( fd, KDSKBMODE, K_MEDIUMRAW ) < 0 )
    {
	slog( "Can't set medium-raw mode\n" );
	keyboard_close();
	return -1;
    }

    return 0;
}

int keyboard_close(void)
{
    if( fd >= 0 )
    {
	//pthread_cancel( (uint)&key_th );
	//reset terminal mode:
	if( ioctl( fd, KDSKBMODE, old_kbd_mode ) < 0 )
	{
	    slog( "ioctl() error in keyboard_close()\n" );
	    return -1;
	}
	tcsetattr( fd, TCSAFLUSH, &old );
	close( fd );
    }
    fd = -1;

    return 0;
}

//#################################
//#################################
//#################################
