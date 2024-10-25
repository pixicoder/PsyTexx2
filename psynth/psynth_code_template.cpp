#include "psynth.h"

//Unique names for objects in your synth:
#define SYNTH_DATA	my_synth_data
#define SYNTH_HANDLER	synth
//And unique parameters:
#define SYNTH_INPUTS	2
#define SYNTH_OUTPUTS	2

struct SYNTH_DATA
{
    //Controls: ############################################################
    CTYPE   ctl_volume;
    //Synth data: ##########################################################
};

size_t SYNTH_HANDLER( 
    PSYTEXX_SYNTH_PARAMETERS
    )
{
    SYNTH_DATA *data = (SYNTH_DATA*)data_ptr;
    size_t retval = 0;

    switch( command )
    {
	case COMMAND_GET_DATA_SIZE:
	    retval = sizeof( SYNTH_DATA );
	    break;

	case COMMAND_GET_SYNTH_NAME:
	    retval = (size_t)"SynthName";
	    break;

	case COMMAND_GET_INPUTS_NUM: retval = SYNTH_INPUTS; break;
	case COMMAND_GET_OUTPUTS_NUM: retval = SYNTH_OUTPUTS; break;

	case COMMAND_GET_WIDTH: retval = 320; break;
	case COMMAND_GET_HEIGHT: retval = 200; break;

	case COMMAND_INIT:
	    psynth_register_ctl( synth_id, "Volume", "amount", 0, 256, 128, &data->ctl_volume, net );
	    break;

	case COMMAND_CLEAN:
	    break;

	case COMMAND_RENDER_REPLACE:

	    break;

	case COMMAND_CLOSE:
	    break;
    }

    return retval;
}
