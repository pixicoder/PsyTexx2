#pragma once

//Use it in your synths host

#include "psynth.h"

#define MAIN_OUTPUT_CHANNELS	    2
#define MAIN_INPUT_CHANNELS	    2
#ifdef PALMOS
    #define MAIN_BUFFER_SIZE	    1024
#else
    #define MAIN_BUFFER_SIZE	    4096
#endif

//How to use:
//1) psynth_render_clear() - clear buffers (output & external instruments) and "rendered" flags;
//2) Render external synths to buffers;
//3) Render external simple samplers to OUTPUT directly (adding);
//4) Full net rendering (buffer size = user defined; not more then MAIN_BUFFER_SIZE).

void psynth_init( psynth_net *pnet );
void psynth_close( psynth_net *pnet );
void psynth_clear( psynth_net *pnet );
void psynth_render_clear( psynth_net *pnet );
int psynth_add_synth(  size_t (*synth)(  
			    PSYTEXX_SYNTH_PARAMETERS
		       ), const char *name, int flags, int x, int y, int instr_num, psynth_net *pnet );
void psynth_remove_synth( int snum, psynth_net *pnet );
void psynth_make_link( int out, int in, psynth_net *pnet );
void psynth_remove_link( int out, int in, psynth_net *pnet );
void psynth_render( int start_item, int buf_size, psynth_net *pnet );
