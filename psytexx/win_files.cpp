/*
    win_files.cpp - file manager
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_files.h"
#include "win_list.h"
#include "win_button.h"
#include "win_scrollbar.h"
#include "win_text.h"
#include "win_main.h"
#include "win_dialog.h"
#include "win_psymenu.h"
#include "../file/file.h"

void save_current_filename( files_data *data, window_manager *wm ) //causes the sound to stop for a short time on PalmOS
{
    if( data->current_files == 0 )
    {
	char *current_filename = text_get_text( data->text_filename, wm );
	save_string( current_filename, 0, PROP_XMFILE_NAME ); //save current file name
    }
    if( data->current_files == 1 )
    {
	char *current_filename = text_get_text( data->text_filename, wm );
	save_string( current_filename, 0, PROP_INSFILE_NAME ); //save current file name
    }
}

void load_current_filename( files_data *data, window_manager *wm )
{
    window *twin = wm->windows[ data->text_filename ];
    text_data *tdata = (text_data*)twin->data; 
    char temp_str[ MAX_TEXT_LEN ];
    temp_str[ 0 ] = 0;
    if( data->current_files == 0 )
    {
	load_string( temp_str, 0, PROP_XMFILE_NAME ); //load previous file name
	text_set_text( data->text_filename, temp_str, wm );
    }
    if( data->current_files == 1 )
    {
	load_string( temp_str, 0, PROP_INSFILE_NAME ); //load previous file name
	text_set_text( data->text_filename, temp_str, wm );
    }
}

void save_dir( files_data *data, window_manager *wm )
{
    if( data->current_files == 0 )
        save_string( data->dir_name, data->disk_number, PROP_XMFILES_DIR ); //save previous dir
    if( data->current_files == 1 )
        save_string( data->dir_name, data->disk_number, PROP_INSFILES_DIR ); //save previous dir
}

void load_dir( files_data *data, window_manager *wm )
{
    if( data->current_files == 0 )
        load_string( data->dir_name, data->disk_number, PROP_XMFILES_DIR ); //load previous dir
    if( data->current_files == 1 )
        load_string( data->dir_name, data->disk_number, PROP_INSFILES_DIR ); //load previous dir
}

void files_set_save_handler( int (*handler)(void*,int,window_manager*), void *user_data, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    files_data *data = (files_data*)win->data;

    data->handler2 = (int (*)(void*,int,void*))handler;
    data->user_data2 = user_data;
}

void files_set_handler( int (*handler)(void*,int,window_manager*), void *user_data, int win_num, window_manager *wm )
{
    window *win = wm->windows[ win_num ]; //Our window
    files_data *data = (files_data*)win->data;

    data->handler = (int (*)(void*,int,void*))handler;
    data->user_data = user_data;
}

void files_refresh(int win_num, window_manager *wm)
{
    window *win = wm->windows[ win_num ]; //Our window
    files_data *data = (files_data*)win->data;
    find_struct fs;
    
    list_clear( data->list_window, wm ); //Clear list
    list_reset_selection( data->list_window, wm );
    data->full_path[0] = 0; //clear start dir
    //Get start dir:
    mem_strcat( data->full_path, get_disk_name( data->disk_number ) );
    mem_strcat( data->full_path, data->dir_name );
    fs.start_dir = data->full_path;
    fs.mask = data->current_mask;

    //Get files list:
    if( find_first(&fs) ) 
    {
	if( fs.type == TYPE_FILE )
	    list_add_item( fs.name, 0, data->list_window, wm );
	else
	{
	    if( fs.name[0] != '.' ) 
	    { //If it's not "." or ".." dir:
		list_add_item( fs.name, 1, data->list_window, wm );
	    }
	}
        for(;;)
	{
	    if( find_next(&fs) == 0 ) break;
	    if( fs.type == TYPE_FILE )
		list_add_item( fs.name, 0, data->list_window, wm );
	    else 
	    {
		if( fs.name[0] != '.' )
		{ //If it's not "." or ".." dir:
		    list_add_item( fs.name, 1, data->list_window, wm );
		}
	    }
        }
    }
    find_close( &fs );

    list_sort( data->list_window, wm );

    text_set_text( data->text_dirname, data->full_path, wm ); //Set current dir name
}

int files_dir_up(int win_num, window_manager *wm)
{
    window *win = wm->windows[ win_num ]; //Our window
    files_data *data = (files_data*)win->data;
    int a;

    //Go to the parent dir:
    for( a = 0; a < MAX_DIR_LENGTH; a++ )
    {
	if( data->dir_name[ a ] == 0 ) break;
    }

    a -= 2; 
    if( a <= 0 )  { data->dir_name[0] = 0; return 0; }

    for( ; a >= 0; a-- )
    {
	if( data->dir_name[ a ] == '/' ) 
	{
	    data->dir_name[ a+1 ] = 0;
	    break;
	}
    }
    
    if( a <= 0 )  data->dir_name[0] = 0;

    files_refresh( data->this_window, wm );
    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw files window
    
    return 0;
}

char* files_get_file(int win_num, window_manager *wm)
{
    window *win = wm->windows[ win_num ]; //Our window
    files_data *data = (files_data*)win->data;
    char *filename;
    
/*    int selected_item = list_get_selected_num( data->list_window, wm );
    if( selected_item >= 0 )
    {
	if( list_get_attr( selected_item, data->list_window, wm ) == 0 )  //Is it FILE ?
	    return list_get_item( selected_item, data->list_window, wm ); 
	else
	    return 0;
    }
    else return 0;*/
    filename = text_get_text( data->text_filename, wm );
    if( filename[ 0 ] == 0 ) 
	return 0;
    else
	return filename;
}

int button1_handler(void *user_data, int button_win, window_manager *wm)
{
    files_data *data = (files_data*)user_data;
    
    files_dir_up( data->this_window, wm );
    
    save_dir( data, wm );
        
    return 0;
}

int button2_handler(void *user_data, int button_win, window_manager *wm)
{
    files_data *data = (files_data*)user_data;
    
    data->current_mask = 0;
    files_refresh( data->this_window, wm );
    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw files window
    
    return 0;
}

int button_load_handler( void *user_data, int button_win, window_manager *wm )
{
    files_data *data = (files_data*)user_data;

    save_current_filename( data, wm ); //save current filename

    char *filename = files_get_file( data->this_window, wm ); //Get filename
    if( filename == 0 ) return 0; //No files selected or DIR selected

    //Get full file name:
    data->full_path[0] = 0; //clear full name
    mem_strcat( data->full_path, get_disk_name( data->disk_number ) );
    mem_strcat( data->full_path, data->dir_name );
    mem_strcat( data->full_path, filename );
    
    //User defined handler:
    if( data->handler ) data->handler( data->user_data, data->this_window, (void*)wm );
    return 0;
}

int button_save_handler( void *user_data, int button_win, window_manager *wm )
{
    files_data *data = (files_data*)user_data;

    save_current_filename( data, wm ); //save current filename

    char *filename = files_get_file( data->this_window, wm ); //Get filename
    if( filename == 0 ) return 0; //No files selected or DIR selected

    //Get full file name:
    data->full_path[0] = 0; //clear full name
    mem_strcat( data->full_path, get_disk_name( data->disk_number ) );
    mem_strcat( data->full_path, data->dir_name );
    mem_strcat( data->full_path, filename );
    
    //User defined handler:
    if( data->handler2 ) data->handler2( data->user_data2, data->this_window, (void*)wm );
    return 0;
}

void del_file( void *user_data, int button, window_manager *wm )
{
    files_data *data = (files_data*)user_data;
    if( button == 1 )
    { //OK:
	//Delete file:
	remove( data->full_path );
	
	//Redraw window:
	files_refresh( data->this_window, wm );
        send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw files window
    }
}

int button_del_handler( void *user_data, int button_win, window_manager *wm )
{
    files_data *data = (files_data*)user_data;
    
    char *filename = files_get_file( data->this_window, wm ); //Get filename
    if( filename == 0 ) return 0; //No files selected or DIR selected

    //Get full file name:
    data->full_path[0] = 0; //clear full name
    mem_strcat( data->full_path, get_disk_name( data->disk_number ) );
    mem_strcat( data->full_path, data->dir_name );
    mem_strcat( data->full_path, filename );
    
    start_dialog( "Delete selected file?", "YES", "NO", 
                  &del_file, user_data,
		  win_dialog, wm );
    
    return 0;
}

int button_mods_handler( void *user_data, int button, window_manager *wm )
{
    files_data *data = (files_data*)user_data;

    data->current_mask = MODFILES;
    data->current_files = 0;
    load_current_filename( data, wm );
    load_dir( data, wm );
    files_refresh( data->this_window, wm );
    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw files window
    
    return 0;
}

int button_instr_handler( void *user_data, int button, window_manager *wm )
{
    files_data *data = (files_data*)user_data;

    data->current_mask = INSFILES;
    data->current_files = 1;
    load_current_filename( data, wm );
    load_dir( data, wm );
    files_refresh( data->this_window, wm );
    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw files window
    
    return 0;
}

int button_close_handler(void *user_data, int button_win, window_manager *wm)
{
    files_data *data = (files_data*)user_data;
    send_event( win_files, EVT_HIDE, 0, 0, 0, 0, MODE_CHILDS, wm); //Hide files window
    send_event( 0, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Full redraw desktop

    window *mwin = wm->windows[ win_menu ];
    menu_data *mdata = (menu_data*)mwin->data;
    mdata->files_status = 0;

    return 0;
}

int disk_button_handler(void *user_data, int button_win, window_manager *wm)
{
    files_data *data = (files_data*)user_data;
    int a;
    
    for( a = 0; a < disks; a++ )
    {
	if( data->button_disk[a] == button_win )
	{
	    //Refresh file-list:
	    data->dir_name[0] = 0;
	    data->disk_number = (char)a;
	    
	    load_dir( data, wm ); //load previous dir for this disk (if exist)    
	    save_dir( data, wm ); //save current dir
	    
	    files_refresh( data->this_window, wm );
	    send_event( data->this_window, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm ); //Redraw files window
	}
    }
    
    return 0;
}

int list_select_handler( void *user_data, int list_win, window_manager *wm )
{
    window *lwin = wm->windows[ list_win ]; //list win
    list_data *ldata = (list_data*)lwin->data;
    files_data *data = (files_data*)user_data;
    char *dir;
    char *f;
    
    if( list_get_attr( ldata->selected_item, list_win, wm ) == 1 )
    { //Dir selected:
	dir = list_get_item( ldata->selected_item, list_win, wm );
	mem_strcat( data->dir_name, dir );
	mem_strcat( data->dir_name, "/" );
	files_refresh( data->this_window, wm );
	send_event( list_win, EVT_REDRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Send DRAW event to list window
        save_dir( data, wm ); //save current dir
    }
    else
    { //File selected:
	f = list_get_item( ldata->selected_item, list_win, wm );
	text_set_text( data->text_filename, f, wm ); //Set current filename
#ifndef PALMOS
	save_current_filename( data, wm ); //save current filename
#endif
    }
    
    return 0;
}

int files_handler(wm_event *evt, window_manager *wm)
{
    window *win = wm->windows[ evt->event_win ]; //Our window
    files_data *data = (files_data*)win->data;
    int y;
    char *cur_dir;
    
    switch( evt->event_type ) 
    {
	case EVT_AFTERCREATE:
	{
	    win->data = mem_new( HEAP_DYNAMIC, sizeof(files_data), "files data" );
	    //Init data:
	    data = (files_data*)win->data;
	    data->handler = 0;
	    data->this_window = evt->event_win;
	    get_disks(); //Get disk names
	    data->disk_number = (char)get_current_disk(); //Default disk
	    data->dir_name[0] = 0;                        //Default dir
	    cur_dir = get_current_dir();                  //Get current dir
	    mem_strcat( data->dir_name, cur_dir );        //Make it default
	    data->current_files = 0; //XM files;
	    load_dir( data, wm ); //Restore previous dir
	    data->current_mask = MODFILES;
	    //Create list window:
	    data->list_window = create_window( "fileslist",
	                                       9, 0, win->x_size - 9, win->y_size - 2, 
	                                       wm->colors[6], 1, evt->event_win, &list_handler, wm );
	    set_window_string_controls( data->list_window, "9", "0", "100%", "100%-2", wm );
	    list_set_handler( &list_select_handler, (void*)data, data->list_window, wm );
	    //Create buttons:
	    data->button_up1 = create_window( "up1", 5, 0, 3, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "../", data->button_up1, wm );
	    button_set_handler( &button1_handler, (void*)data, data->button_up1, wm );

	    data->button_up2 = create_window( "up2", 5, 3, 3, 2, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "*.*", data->button_up2, wm );
	    button_set_handler( &button2_handler, (void*)data, data->button_up2, wm );

	    int ystep = 3;
	    int ysize = 2;
	    if( wm->screen_y <= 40 )
	    {
		//Small screen:
		ystep = 2;
		ysize = 2;
	    }

#ifdef TEXTMODE
	    ystep = 2;
	    ysize = 1;
	    if( wm->screen_y <= 40 )
	    {
		//Small screen:
		ystep = 1;
		ysize = 1;
	    }
#endif

	    y = 0;
	    data->button_load = create_window( "load", 0, y, 4, ysize, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "LOAD", data->button_load, wm );
	    button_set_handler( &button_load_handler, (void*)data, data->button_load, wm );
	    y += ystep;

	    data->button_save = create_window( "save", 0, y, 4, ysize, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "SAVE", data->button_save, wm );
	    button_set_handler( &button_save_handler, (void*)data, data->button_save, wm );
	    y += ystep;

	    data->button_del = create_window( "del", 0, y, 4, ysize, wm->colors[6], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "DEL", data->button_del, wm );
	    button_set_handler( &button_del_handler, (void*)data, data->button_del, wm );
	    y += ystep;

	    data->button_mods = create_window( "mods", 0, y, 4, ysize, wm->colors[10], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "MODS", data->button_mods, wm );
	    button_set_handler( &button_mods_handler, (void*)data, data->button_mods, wm );
	    y += ystep;

	    data->button_instr = create_window( "inst", 0, y, 4, ysize, wm->colors[10], 1, evt->event_win, &button_handler, wm );
	    button_set_name( "INST", data->button_instr, wm );
	    button_set_handler( &button_instr_handler, (void*)data, data->button_instr, wm );
	    y += ystep;
	    
	    data->button_close = create_window( "fclose", 0, win->y_size-ysize, 4, ysize, wm->colors[10], 1, evt->event_win, &button_handler, wm );
	    set_window_string_controls( data->button_close, "0", "100%-2", "4", "100%", wm );
	    button_set_name( "EXIT", data->button_close, wm );
	    button_set_handler( &button_close_handler, 0, data->button_close, wm );
	    //Create text windows:
	    data->text_file = create_window( "textfile",
	                                     9, win->y_size - 1, 5, 1,
	                                     wm->colors[9], 1, evt->event_win, &text_handler, wm );
	    set_window_string_controls( data->text_file, "9", "100%-1", "9+5", "100%", wm );
	    text_set_text( data->text_file, "FILE:", wm );
	    text_set_readonly( data->text_file, 1, wm );
	    data->text_filename = create_window( "filename",
	                                         9+5, win->y_size - 1, win->x_size - 9 - 5, 1,
	                                         wm->colors[13], 1, evt->event_win, &text_handler, wm );
	    set_window_string_controls( data->text_filename, "9+5", "100%-1", "100%", "100%", wm );
	    data->text_dir = create_window( "textdir",
	                                    9, win->y_size - 2, 5, 1,
	                                    wm->colors[9], 1, evt->event_win, &text_handler, wm );
	    set_window_string_controls( data->text_dir, "9", "100%-2", "9+5", "100%-1", wm );
	    text_set_text( data->text_dir, "DIR:", wm );
	    text_set_readonly( data->text_dir, 1, wm );
	    data->text_dirname = create_window( "dirname",
	                                        9+5, win->y_size - 2, win->x_size - 9 - 5, 1,
	                                        wm->colors[13], 1, evt->event_win, &text_handler, wm );
	    set_window_string_controls( data->text_dirname, "9+5", "100%-2", "100%", "100%-1", wm );
	    text_set_readonly( data->text_dirname, 1, wm );
	    //Create disk buttons:
	    for( y = 0; y < disks; y++ )
	    {
		data->button_disk[y] = create_window( "diskbutton",
		                                      5, y * 3 + 6, 
		                                      3, 2, 
						      wm->colors[6],
						      1,
						      evt->event_win, &button_handler, wm );
		button_set_name( get_disk_name(y), data->button_disk[y], wm );
		button_set_handler( &disk_button_handler, (void*)data, data->button_disk[y], wm );
	    }

	    files_refresh( evt->event_win, wm );
	    load_current_filename( data, wm );
	    break;
	}
	case EVT_BEFORECLOSE:
	    if( win->data ) mem_free( win->data );
	    break;
	case EVT_SHOW: 
	    //Show window:
	    win->visible = 1; //Make it visible
	    send_event(evt->event_win, EVT_DRAW, 0, 0, 0, 0, MODE_CHILDS, wm); //Send DRAW event to all childs
	    break;
	case EVT_HIDE:
	    win->visible = 0;
	    break;
	case EVT_DRAW: 
	    //Draw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_touch_area( evt->event_win, wm );
		draw_window_box( evt->event_win, wm ); //draw window box
	    }
	    break;
	case EVT_REDRAW: 
	    //Redraw window (if it's visible)
	    if( win->visible )
	    {
		draw_window_box( evt->event_win, wm ); //draw window box
	    }
	    break;
    }
    return 0;
}
