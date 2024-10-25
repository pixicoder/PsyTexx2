#pragma once

#include "core/core.h"
#include "log/log.h"
#include "wm/wm.h"
#include "xm/xm.h"

//######################################
//## PSYTEXX VARIABLES:               ##
//######################################

extern window_manager wm;     	//Window manager
extern xm_struct xm;	      	//Main XM sound structure;

extern int win_desktop;      	//Desktop window
extern int win_top;
extern int win_files;        	//Files window
extern int win_menu;         	//PsyTexx main menu
extern int win_patterntable; 	//Pattern table
extern int win_patterncontrol;
extern int win_patterneditor;	//Pattern editor
extern int win_samples;      	//Instrument list
extern int win_pattern_prop; 	//Pattern properties
extern int win_config;       	//PsyTexx config window
extern int win_playlist;     	//PsyTexx playlist
extern int win_insteditor;   	//Instrument editor
extern int win_smpeditor;    	//Sample editor
extern int win_net;	      	//Sound net with effects and synths
extern int win_keyboard;     	//Virtual keyboard
extern int win_dialog;
extern int win_popup;

//######################################
//## PSYTEXX FUNCTIONS:               ##
//######################################

void psy_windows_init( void );
void psy_event_loop( void );
void psy_windows_close( void );
void psy_wm_handler( window_manager *wm );

int handler_load_xm( void *user_data, int files_window, window_manager* );
int handler_save_xm( void *user_data, int files_window, window_manager* );
