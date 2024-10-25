#pragma once

#include "../core/core.h"
#include "../wm/wm.h"

#define SAMPLE_MENU "SELECT ALL\nCUT\nCOPY\nPASTE\nCROP\nCLEAR\nSET LOOP\nSMOOTH\nNORMALIZE\nREVERSE"

//PROPERTIES:

//INTERNAL STRUCTURES:

struct smpview_data
{
    int this_window;

    int scrollbar;
    int precision_shift;

    short *prev_smp_data;
    int prev_smp_len;

    void *copy_buffer;

    int start_x;
    int start_y;
    int start_offset;
    int start_reppnt1;
    int start_reppnt2;
    int dragmode; //1 - cursor drag; 2 - reppnt1; 3 - reppnt2

    int offset;
    int delta;		    //Number of samples in one pixel

    int cursor;
    int selected_size;
};

//FUNCTIONS:

void smpview_edit_op( smpview_data *data, int op_num );
void smpview_zoom( int win_num, int inout, window_manager *wm );
void smpview_redraw( int win_num, window_manager *wm );

//HANDLERS:

//WINDOW HANDLERS:

int smpview_handler( wm_event*, window_manager* );
