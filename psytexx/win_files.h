#pragma once

#include "../core/core.h"
#include "../wm/wm.h"
#include "../file/file.h"

//FILE TYPES:
#define MODFILES           "xm/XM/mod/MOD"
#define INSFILES           "xi/XI/wav/WAV"

//PROPERTIES:
#define PROP_XMFILES_DIR   ".prop_files_xm_"
#define PROP_INSFILES_DIR  ".prop_files_ins_"
#define PROP_XMFILE_NAME   ".prop_files_xmname_"
#define PROP_INSFILE_NAME  ".prop_files_insname_"

//INTERNAL STRUCTURES:

struct files_data
{
    int this_window;  		//this window handler
    int list_window;    	//list window handler
    char disk_number;    	//current disk number
    char dir_name[MAX_DIR_LENGTH]; 	//current dir name (example: "mydir/")
    char full_path[MAX_DIR_LENGTH]; 	//full path (disk name + dir name [+ filename])
    const char *current_mask;  
    int button_up1;     	//".." button
    int button_up2;     	//"*.*" button
    int button_disk[16];
    int button_load;
    int button_save;
    int button_del;
    int button_mods;
    int button_instr;
    int button_close;
    int current_files;  	//XM, INSTRUMENTS ...
    int text_filename; 	 	//Text windows:
    int text_file;      	// ...
    int text_dirname;   	// ...
    int text_dir;       	// ...
    
    int (*handler)(void*,int,void*); 	//User defined "LOAD" handler: 
                                        //handler(void *user_data, int files_window, void *window_manager)
    void *user_data;                   	//Data for handler

    int (*handler2)(void*,int,void*);	//User defined "SAVE" handler: 
                                        //handler2(void *user_data, int files_window, void *window_manager)
    void *user_data2;                  	//Data for handler2
};

//FUNCTIONS:

void files_set_save_handler( int (*)(void*,int,window_manager*), void *user_data, int win_num, window_manager* );
void files_set_handler( int (*)(void*,int,window_manager*), void *user_data, int win_num, window_manager* );
void files_refresh( int win_num, window_manager* );   	//Create files list
int files_dir_up( int win_num, window_manager* );    	//Go to the parent dir ("../" button)
char* files_get_file( int win_num, window_manager* ); 	//Get full selected file name or NULL

//HANDLERS:

int button1_handler( void* user_data, int button_win, window_manager* );
int button2_handler( void* user_data, int button_win, window_manager* );
int button_load_handler( void* user_data, int button_win, window_manager* );
int button_save_handler( void* user_data, int button_win, window_manager* );
int button_del_handler( void* user_data, int button_win, window_manager* );
int button_mods_handler( void* user_data, int button_win, window_manager* );
int button_instr_handler( void* user_data, int button_win, window_manager* );
int button_close_handler( void* user_data, int button_win, window_manager* );
int disk_button_handler( void* user_data, int button_win, window_manager* );
int list_select_handler( void* user_data, int list_win, window_manager* );

//WINDOW HANDLERS:

int files_handler( wm_event*, window_manager* ); //File manager
