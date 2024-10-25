#include "psynth.h"

//Unique names for objects in your synth:
#define SYNTH_DATA	psytexx_echo_data
#define SYNTH_HANDLER	psytexx_echo
//And unique parameters:
#define SYNTH_INPUTS	2
#define SYNTH_OUTPUTS	2

#define BUFFER_SIZE	44100

struct SYNTH_DATA
{
    //Controls: ############################################################
    CTYPE   ctl_volume;
    //Synth data: ##########################################################
    STYPE   *buf[ SYNTH_OUTPUTS ];
    int	    buf_ptr;
};

size_t SYNTH_HANDLER( 
    PSYTEXX_SYNTH_PARAMETERS
    )
{
    SYNTH_DATA *data = (SYNTH_DATA*)data_ptr;
    size_t retval = 0;
    int i;
    int ch;
    int prev_buf_ptr;

    switch( command )
    {
	case COMMAND_GET_DATA_SIZE:
	    retval = sizeof( SYNTH_DATA );
	    break;

	case COMMAND_GET_SYNTH_NAME:
	    retval = (size_t)"Echo";
	    break;

	case COMMAND_GET_INPUTS_NUM: retval = SYNTH_INPUTS; break;
	case COMMAND_GET_OUTPUTS_NUM: retval = SYNTH_OUTPUTS; break;

	case COMMAND_INIT:
	    psynth_register_ctl( synth_id, "Volume", "amount", 0, 256, 128, &data->ctl_volume, net );
	    for( i = 0; i < SYNTH_OUTPUTS; i++ )
	    {
		data->buf[ i ] = (STYPE*)SMALLOC( BUFFER_SIZE * sizeof( STYPE ) );
	    }
	    data->buf_ptr = 0;
	    for( i = 0; i < SYNTH_OUTPUTS; i++ )
	    {
		if( data->buf[ i ] ) SMEMSET( data->buf[ i ], 0, BUFFER_SIZE * sizeof( STYPE ) );
	    }	    
	    break;

	case COMMAND_CLEAN:
	    data->buf_ptr = 0;
	    for( i = 0; i < SYNTH_OUTPUTS; i++ )
	    {
		if( data->buf[ i ] ) SMEMSET( data->buf[ i ], 0, BUFFER_SIZE * sizeof( STYPE ) );
	    }	    
	    break;

	case COMMAND_RENDER_REPLACE:
	    prev_buf_ptr = data->buf_ptr;
	    for( ch = 0; ch < SYNTH_OUTPUTS; ch++ )
	    {
		data->buf_ptr = prev_buf_ptr;
		STYPE *in = inputs[ ch ];
		STYPE *out = outputs[ ch ];
		STYPE *cbuf = data->buf[ ch ];
		for( i = 0; i < sample_frames; i++ )
		{
		    cbuf[ data->buf_ptr ] += in[ i ];
		    out[ i ] = cbuf[ data->buf_ptr ];
		    cbuf[ data->buf_ptr ] /= 2;
		    data->buf_ptr++;
		    if( data->buf_ptr >= BUFFER_SIZE ) data->buf_ptr = 0;
		}
	    }
	    break;

	case COMMAND_RENDER_ADD:
	    prev_buf_ptr = data->buf_ptr;
	    for( ch = 0; ch < SYNTH_OUTPUTS; ch++ )
	    {
		data->buf_ptr = prev_buf_ptr;
		STYPE *in = inputs[ ch ];
		STYPE *out = outputs[ ch ];
		STYPE *cbuf = data->buf[ ch ];
		for( i = 0; i < sample_frames; i++ )
		{
		    cbuf[ data->buf_ptr ] += in[ i ];
		    out[ i ] += cbuf[ data->buf_ptr ];
		    cbuf[ data->buf_ptr ] /= 2;
		    data->buf_ptr++;
		    if( data->buf_ptr >= BUFFER_SIZE ) data->buf_ptr = 0;
		}
	    }
	    break;

	case COMMAND_CLOSE:
	    for( i = 0; i < SYNTH_OUTPUTS; i++ )
	    {
		if( data->buf[ i ] ) SFREE( data->buf[ i ] );
	    }
	    break;
    }

    return retval;
}
