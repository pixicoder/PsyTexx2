#pragma once

//Use it in your synth code

//SunDog Engine include files:
#include "../core/core.h"
#include "../memory/memory.h"

enum {
    COMMAND_NOP = 0,
    COMMAND_GET_DATA_SIZE,
    COMMAND_GET_SYNTH_NAME,
    COMMAND_GET_INPUTS_NUM,
    COMMAND_GET_OUTPUTS_NUM,
    COMMAND_GET_WIDTH,
    COMMAND_GET_HEIGHT,
    COMMAND_INIT,
    COMMAND_CLEAN,              //Clean all data in sound buffers to 0
    COMMAND_RENDER_REPLACE,	//Replace data in the destination buffer
    COMMAND_RENDER_ADD,		//Add data to the destination buffer
    COMMAND_CLOSE
};

#define PSYTEXX_SYNTH_PARAMETERS \
    void *data_ptr, \
    int synth_id, \
    STYPE **inputs, \
    STYPE **outputs, \
    int sample_frames, \
    int command, \
    void *net

//Control type:
typedef int	CTYPE;

//Sample operations (selected by user):
//==== Float ====
//Sample type:
/*
typedef float	STYPE;
#define INT16_TO_STYPE( res, val ) { res = (float)val / (float)32768; }
#define STYPE_TO_INT16( res, val ) { \
    int temp_res; \
    temp_res = (int)( val * (float)32768 ); \
    if( temp_res > 32767 ) res = 32767; else \
    if( temp_res < -32768 ) res = -32768; else \
    res = (signed short)temp_res; \
}
*/
//==== Int ====
//Sample type:
typedef int	STYPE;
#define INT16_TO_STYPE( res, val ) { res = (int)val; }
#define STYPE_TO_INT16( res, val ) { \
    if( val > 32767 ) res = 32767; else \
    if( val < -32768 ) res = -32768; else \
    res = (signed short)val; \
}

struct psynth_control
{
    const char	    *ctl_name;  //For example: "Delay", "Feedback"
    const char	    *ctl_label; //For example: "dB", "samples"
    CTYPE	    ctl_min;
    CTYPE	    ctl_max;
    CTYPE	    ctl_def;
};

//One item of external net:
#define PSYNTH_FLAG_EXISTS		1
#define PSYNTH_FLAG_OUTPUT		2
#define PSYNTH_FLAG_EXTERNAL_INSTRUMENT	4
#define PSYNTH_FLAG_RENDERED		8
#define PSYNTH_MAX_CHANNELS		8
struct psynth_net_item
{
    int		    flags;

    char	    item_name[ 32 ];

    size_t	    (*synth)(  
			PSYTEXX_SYNTH_PARAMETERS
		    );
    void	    *data_ptr;
    STYPE	    *channels[ PSYNTH_MAX_CHANNELS ];

    int		    x, y;   //In percents (0..1024)
    int		    instr_num;

    //Number of channels:
    int		    input_channels;
    int		    output_channels;

    //Links to an input synths:
    int		    *input_links;
    int		    input_num;

    //Controllers:
    psynth_control  ctls[ 16 ];
    int		    ctls_num;
};

//External sound net (created by host):
struct psynth_net
{
    //Net items (nodes):
    psynth_net_item	*items;
    int			items_num;
    //Some info:
    int			sampling_freq;
    char		is_external_instr_in_the_net[ 128 ];
};

extern void psynth_register_ctl( 
    int		synth_id, 
    const char	*ctl_name,  //For example: "Delay", "Feedback"
    const char	*ctl_label, //For example: "dB", "samples"
    CTYPE	ctl_min,
    CTYPE	ctl_max,
    CTYPE	ctl_def,
    CTYPE	*value,
    void	*net );

//Platform dependent functions:
#define SMALLOC( size ) MEM_NEW( HEAP_DYNAMIC, size )
#define SREALLOC( ptr, size ) mem_resize( ptr, size )
#define SMEMSET( ptr, val, size ) mem_set( ptr, size, val )
#define SFREE( ptr ) mem_free( ptr )
