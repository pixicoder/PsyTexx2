#include "psynth_net.h"

#ifdef SLOWMODE
    #define NOPSYNTH
#endif

void psynth_init( psynth_net *pnet )
{
#ifndef NOPSYNTH
    SMEMSET( pnet, 0, sizeof( psynth_net ) ); //Clear main struct
    pnet->items = (psynth_net_item*)SMALLOC( sizeof( psynth_net_item ) * 4 );

    SMEMSET( pnet->items, 0, sizeof( psynth_net_item ) * 4 ); //Clear items
    pnet->items_num = 4;

    //Clear OUTPUT:
    psynth_add_synth( 0, "OUT", PSYNTH_FLAG_OUTPUT, 512, 512, 0, pnet );
#endif
}

void psynth_close( psynth_net *pnet )
{
#ifndef NOPSYNTH
    if( pnet )
    {
	//Remove items:
	if( pnet->items )
	{
	    for( int i = 0; i < pnet->items_num; i++ ) psynth_remove_synth( i, pnet );
	    SFREE( pnet->items );
	}
	//Remove main struct:
	SFREE( pnet );
    }
#endif
}

//Clear the net:
void psynth_clear( psynth_net *pnet )
{
#ifndef NOPSYNTH
    if( pnet )
    {
	//Remove items:
	if( pnet->items )
	{
	    for( int i = 1; i < pnet->items_num; i++ ) psynth_remove_synth( i, pnet );
	}
    }
#endif
}

void recalc_external_instruments( psynth_net *pnet )
{
    for( int cc = 0; cc < 128; cc++ ) 
	pnet->is_external_instr_in_the_net[ cc ] = -1;
    for( int i = 0; i < pnet->items_num; i++ ) 
    {
	if( pnet->items[ i ].flags & PSYNTH_FLAG_EXTERNAL_INSTRUMENT )
	    pnet->is_external_instr_in_the_net[ pnet->items[ i ].instr_num & 127 ] = i;
    }
}

//Clear buffers (output and external instruments) and "rendered" flags:
void psynth_render_clear( psynth_net *pnet )
{
#ifndef NOPSYNTH
    if( pnet )
    {
	if( pnet->items )
	{
	    for( int i = 0; i < pnet->items_num; i++ ) 
	    {
		//Clear "rendered" flags:
		pnet->items[ i ].flags &= ~PSYNTH_FLAG_RENDERED;
		//Clear output:
		if( pnet->items[ i ].flags & PSYNTH_FLAG_EXTERNAL_INSTRUMENT ||
		    pnet->items[ i ].flags & PSYNTH_FLAG_OUTPUT )
		{
		    for( int c = 0; c < PSYNTH_MAX_CHANNELS; c++ )
		    {
			STYPE *ch = pnet->items[ i ].channels[ c ];
			if( ch ) 
			    for( int ss = 0; ss < MAIN_BUFFER_SIZE; ss++ )
				ch[ ss ] = 0;
		    }
		}
		if( pnet->items[ i ].flags & PSYNTH_FLAG_EXTERNAL_INSTRUMENT )
		    pnet->items[ i ].flags |= PSYNTH_FLAG_RENDERED;
	    }
	    if( pnet->items[ 0 ].flags & PSYNTH_FLAG_OUTPUT )
	    {
	    }
	}
    }
#endif
}

int psynth_add_synth(  size_t (*synth)(  
			    PSYTEXX_SYNTH_PARAMETERS
		       ), const char *name, int flags, int x, int y, int instr_num, psynth_net *pnet )
{
#ifndef NOPSYNTH
    if( pnet )
    {
	//Get free item:
	int i;
	for( i = 0; i < pnet->items_num; i++ )
	{
	    if( pnet->items[ i ].flags == 0 ) break;
	}
	if( i == pnet->items_num )
	{
	    //No free item:
	    pnet->items = (psynth_net_item*)SREALLOC( pnet->items, sizeof( psynth_net_item ) * ( pnet->items_num + 4 ) );
	    i = pnet->items_num;
	    pnet->items_num += 4;
	}
	//Add new synth:
	pnet->items[ i ].synth = synth;
	pnet->items[ i ].flags = PSYNTH_FLAG_EXISTS | flags;
	pnet->items[ i ].x = x;
	pnet->items[ i ].y = y;
	pnet->items[ i ].instr_num = instr_num;
	pnet->items[ i ].item_name[ 0 ] = 0;
	pnet->items[ i ].ctls_num = 0;
	if( name )
	    mem_strcat( pnet->items[ i ].item_name, name );
	//Init it:
	int data_size = 0;
	if( synth )
	    data_size = synth( 0, i, 0, 0, 0, COMMAND_GET_DATA_SIZE, (void*)pnet );
	if( data_size )
	    pnet->items[ i ].data_ptr = SMALLOC( data_size );
	else
	    pnet->items[ i ].data_ptr = 0;
	if( synth )
	    synth( pnet->items[ i ].data_ptr, i, 0, 0, 0, COMMAND_INIT, (void*)pnet );
	pnet->items[ i ].input_channels = MAIN_INPUT_CHANNELS;
	pnet->items[ i ].output_channels = MAIN_OUTPUT_CHANNELS;
	if( synth )
	{
	    //Get number of in/out channels:
	    pnet->items[ i ].input_channels = synth( pnet->items[ i ].data_ptr, i, 0, 0, 0, COMMAND_GET_INPUTS_NUM, (void*)pnet );
	    pnet->items[ i ].output_channels = synth( pnet->items[ i ].data_ptr, i, 0, 0, 0, COMMAND_GET_OUTPUTS_NUM, (void*)pnet );
	}
	//Create buffers:
	int i2;
	for( i2 = 0; i2 < PSYNTH_MAX_CHANNELS; i2++ )
	{
	    pnet->items[ i ].channels[ i2 ] = 0;
	}
	int max_channels = pnet->items[ i ].output_channels;
	if( pnet->items[ i ].input_channels > max_channels ) max_channels = pnet->items[ i ].input_channels;
	for( i2 = 0; i2 < max_channels; i2++ )
	{
	    pnet->items[ i ].channels[ i2 ] = (STYPE*)SMALLOC( MAIN_BUFFER_SIZE * sizeof( STYPE ) );
	}
	//Set empty input links:
	pnet->items[ i ].input_num = 0;
	pnet->items[ i ].input_links = 0;

	recalc_external_instruments( pnet );

	return i;
    }
#endif
    return -1;
}

void psynth_remove_synth( int snum, psynth_net *pnet )
{
#ifndef NOPSYNTH
    if( snum >= 0 && snum < pnet->items_num )
    {
	if( pnet->items[ snum ].synth )
	    pnet->items[ snum ].synth( pnet->items[ snum ].data_ptr, snum, 0, 0, 0, COMMAND_CLOSE, (void*)pnet );
	//Remove synth data:
	if( pnet->items[ snum ].data_ptr )
	    SFREE( pnet->items[ snum ].data_ptr ); 
	pnet->items[ snum ].data_ptr = 0;
	//Remove synth channels:
	int i;
	for( i = 0; i < PSYNTH_MAX_CHANNELS; i++ )
	    if( pnet->items[ snum ].channels[ i ] ) 
	    {
		SFREE( pnet->items[ snum ].channels[ i ] );
		pnet->items[ snum ].channels[ i ] = 0;
	    }
	//Remove synth links:
	if( pnet->items[ snum ].input_num && pnet->items[ snum ].input_links )
	{
	    SFREE( pnet->items[ snum ].input_links );
	    pnet->items[ snum ].input_links = 0;
	    pnet->items[ snum ].input_num = 0;
	}
	//Remove links from another items:
	for( i = 0; i < pnet->items_num; i++ )
	{
	    if( pnet->items[ i ].flags )
	    {
		if( pnet->items[ i ].input_num )
		{
		    for( int l = 0; l < pnet->items[ i ].input_num; l++ )
		    {
			if( pnet->items[ i ].input_links[ l ] == snum )
			    pnet->items[ i ].input_links[ l ] = -1;
		    }
		}
	    }
	}
	//Remove synth handler:
	pnet->items[ snum ].synth = 0;

	pnet->items[ snum ].flags = 0;
    }
    recalc_external_instruments( pnet );
#endif
}

//Make link: out.link = in
void psynth_make_link( int out, int in, psynth_net *pnet )
{
#ifndef NOPSYNTH
    psynth_remove_link( out, in, pnet ); //Remove link, if it already exists
    if( pnet->items_num )
    if( out >= 0 && out < pnet->items_num )
    if( in >= 0 && in < pnet->items_num )
    {
	int i = 0;
	//Create array with a links:
	if( pnet->items[ out ].input_num == 0 )
	{
	    pnet->items[ out ].input_links = (int*)SMALLOC( 4 * sizeof( int ) );
	    pnet->items[ out ].input_num = 4;
	    for( i = 0; i < pnet->items[ out ].input_num; i++ ) pnet->items[ out ].input_links[ i ] = -1;
	}
	//Find free link:
	for( i = 0; i < pnet->items[ out ].input_num; i++ )
	{
	    if( pnet->items[ out ].input_links[ i ] < 0 ) break;
	}
	if( i == pnet->items[ out ].input_num )
	{
	    //No free space for new link:
	    pnet->items[ out ].input_links = 
		(int*)SREALLOC( pnet->items[ out ].input_links, 
		                ( pnet->items[ out ].input_num + 4 ) * sizeof( int ) );
	    for( i = pnet->items[ out ].input_num; i < pnet->items[ out ].input_num + 4; i++ ) 
		pnet->items[ out ].input_links[ i ] = -1;
	    i = pnet->items[ out ].input_num;
	    pnet->items[ out ].input_num += 4;
	}
	pnet->items[ out ].input_links[ i ] = in;
    }
#endif
}

//Remove link: out.link = in
void psynth_remove_link( int out, int in, psynth_net *pnet )
{
#ifndef NOPSYNTH
    if( pnet->items_num )
    if( out >= 0 && out < pnet->items_num )
    if( in >= 0 && in < pnet->items_num )
    {
	//Find our link:
	for( int i = 0; i < pnet->items[ out ].input_num; i++ )
	    if( pnet->items[ out ].input_links[ i ] == in )
		pnet->items[ out ].input_links[ i ] = -1;
    }
#endif
}

//(Start item must be 0)
void psynth_render( int start_item, int buf_size, psynth_net *pnet )
{
#ifndef NOPSYNTH
    psynth_net_item *item = &pnet->items[ start_item ];
    if( item->flags && 
	!( item->flags & PSYNTH_FLAG_RENDERED ) && 
	!( item->flags & PSYNTH_FLAG_EXTERNAL_INSTRUMENT )
    )
    {
	int i, ch;
	int inp;
	psynth_net_item *in;
	if( start_item == 0 )
	{
	    //Current item is the main OUTPUT
	    for( inp = 0; inp < item->input_num; inp++ )
	    {
		//For each input:
		if( item->input_links[ inp ] >= 0 && item->input_links[ inp ] < item->input_num )
		{
		    in = &pnet->items[ item->input_links[ inp ] ];
		    if( !( in->flags & PSYNTH_FLAG_RENDERED ) )
		    {
			//This input is not rendered yet. Do it:
			psynth_render( item->input_links[ inp ], buf_size, pnet );
		    }
		    if( in->flags & PSYNTH_FLAG_RENDERED )
		    {
			if( in->flags & PSYNTH_FLAG_EXTERNAL_INSTRUMENT )
			{
			    //It's external instrument. Just add input to output:
			    for( ch = 0; ch < item->output_channels; ch++ )
			    {
				STYPE *in_data = in->channels[ ch ];
				STYPE *out_data = item->channels[ ch ];
				if( in_data && out_data )
				for( i = 0; i < buf_size; i++ )
				{
				    out_data[ i ] += in_data[ i ];
				}
			    }
			}
			else
			{
			    //In's synth or effect:
			    if( in->synth )
				in->synth( in->data_ptr, 
					   item->input_links[ inp ],
			  		   in->channels,
					   item->channels,
					   buf_size,
					   COMMAND_RENDER_ADD,
					   (void*)pnet );
			}
			item->flags |= PSYNTH_FLAG_RENDERED;
		    }
		}
	    }
	}
	else
	{
	    for( inp = 0; inp < item->input_num; inp++ )
	    {
		//For each input:
		if( item->input_links[ inp ] >= 0 && item->input_links[ inp ] < item->input_num )
		{
		    in = &pnet->items[ item->input_links[ inp ] ];
		    if( !( in->flags & PSYNTH_FLAG_RENDERED ) )
		    {
			//This input is not rendered yet. Do it:
			psynth_render( item->input_links[ inp ], buf_size, pnet );
		    }
		    if( in->flags & PSYNTH_FLAG_RENDERED )
		    {
			if( in->flags & PSYNTH_FLAG_EXTERNAL_INSTRUMENT )
			{
			    //It's external instrument:
			    if( !( item->flags & PSYNTH_FLAG_RENDERED ) )
				for( ch = 0; ch < item->output_channels; ch++ )
				{
				    STYPE *in_data = in->channels[ ch ];
				    STYPE *out_data = item->channels[ ch ];
				    if( in_data && out_data )
				    for( i = 0; i < buf_size; i++ )
				    {
					out_data[ i ] = in_data[ i ];
				    }
				}
			    else
				for( ch = 0; ch < item->output_channels; ch++ )
				{
				    STYPE *in_data = in->channels[ ch ];
				    STYPE *out_data = item->channels[ ch ];
				    if( in_data && out_data )
				    for( i = 0; i < buf_size; i++ )
				    {
					out_data[ i ] += in_data[ i ];
				    }
				}
			    item->flags |= PSYNTH_FLAG_RENDERED;
			}
			else
			{
			    //It's some synth of effect:
			    if( in->synth )
			    {
				if( !( item->flags & PSYNTH_FLAG_RENDERED ) )
				{
				    in->synth( in->data_ptr, 
					       item->input_links[ inp ],
			  		       in->channels,
					       item->channels,
					       buf_size,
					       COMMAND_RENDER_REPLACE,
					       (void*)pnet );
				}
				else
				{
				    in->synth( in->data_ptr, 
					       item->input_links[ inp ],
			  		       in->channels,
					       item->channels,
					       buf_size,
					       COMMAND_RENDER_ADD,
					       (void*)pnet );
				}
				item->flags |= PSYNTH_FLAG_RENDERED;
			    }
			}
		    }
		}
	    }
	}

	//All synths was rendered. Result in the OUTPUT item
    }
#endif
}

void psynth_register_ctl( 
    int		synth_id, 
    const char	*ctl_name,  //For example: "Delay", "Feedback"
    const char	*ctl_label, //For example: "dB", "samples"
    CTYPE	ctl_min,
    CTYPE	ctl_max,
    CTYPE	ctl_def,
    CTYPE	*value,
    void	*net )
{
#ifndef NOPSYNTH
    psynth_net *pnet = (psynth_net*)net;
    if( pnet->items_num )
    if( synth_id >= 0 && synth_id < pnet->items_num )
    {
	psynth_control *ctl = &pnet->items[ synth_id ].ctls[ pnet->items[ synth_id ].ctls_num ];
	ctl->ctl_name = ctl_name;
	ctl->ctl_label = ctl_label;
	ctl->ctl_min = ctl_min;
	ctl->ctl_max = ctl_max;
	ctl->ctl_def = ctl_def;
	*value = ctl_def;
	pnet->items[ synth_id ].ctls_num++;
    }
#endif
}
