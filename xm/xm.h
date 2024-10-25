#pragma once

#include "../core/core.h"
#include "../file/file.h"
#include "../wm/wm.h"
#include "../psynth/psynth_net.h"

#ifdef NONPALM
    //Max channels (virtual) in XM:
    #define MAX_CHANNELS	128
    #define SUBCHANNELS		4
    #define MAX_REAL_CHANNELS	( MAX_CHANNELS * SUBCHANNELS )
    //For realtime keyboard playing:
    #define CHANNELS_IN_BUFFER	128 
    //Pixel scope size for each channel:
    #define SCOPE_SIZE		256
#else
    //Max channels (virtual) in XM:
    #define MAX_CHANNELS	48
    #define SUBCHANNELS		1
    #define MAX_REAL_CHANNELS	( MAX_CHANNELS * SUBCHANNELS )
    //For realtime keyboard playing:
    #define CHANNELS_IN_BUFFER	16
    //Pixel scope size for each channel:
    #define SCOPE_SIZE		1
#endif
#define ENV_TICKS         512

#define XM_PREC (int)16	    //sample pointer (fixed point) precision 
#define INTERP_PREC ( XM_PREC - 1 ) //sample interpolation precision 

#define SIGNED_SUB64( v1, v1_p, v2, v2_p ) \
{ v1 -= v2; v1_p -= v2_p; \
if( v1_p < 0 ) { v1--; v1_p += ( (int)1 << XM_PREC ); } }

#define SIGNED_ADD64( v1, v1_p, v2, v2_p ) \
{ v1 += v2; v1_p += v2_p; \
if( v1_p > ( (int)1 << XM_PREC ) - 1 ) { v1++; v1_p -= ( (int)1 << XM_PREC ); } }

struct xmnote
{
        uint8_t   n;
        uint8_t   inst;
        uint8_t   vol;
        uint8_t   fx;
	uint8_t   par;

	uint8_t   n1;
	uint8_t   n2;
	uint8_t   n3;
};

struct pattern
{
	uint    header_length;  //*
	uint8_t   reserved5;
	uint8_t   reserved6;
	uint16_t  rows;           //*
	uint16_t  data_size;
	uint16_t  reserved8;
	
	uint16_t  channels;
	uint16_t  real_rows;
	uint16_t  real_channels;
	
	xmnote  *pattern_data;
};

struct sample
{
        uint         length;
        uint         reppnt;
        uint         replen;
        uint8_t        volume;
        signed char  finetune;
	uint8_t        type;
	uint8_t        panning;
	signed char  relative_note;
	uint8_t        reserved2;
        uint8_t        name[22];
	
	signed short *data;  //Sample data
};

#define EXT_INST_BYTES	    3
#define INST_FLAG_NOTE_OFF  1	//Note dead = note off

struct instrument
{
	//Offset in XM file is 0:
	uint    instrument_size;   
	uint8_t   name[22];
	uint8_t   type;
	uint16_t  samples_num;       

	//Offset in XM file is 29:
	//>>> Standard info block:
	uint    sample_header_size;
	uint8_t   sample_number[96];
	uint16_t  volume_points[24];
	uint16_t  panning_points[24];
	uint8_t   volume_points_num;
	uint8_t   panning_points_num;
	uint8_t   vol_sustain;
	uint8_t   vol_loop_start;
	uint8_t   vol_loop_end;	
	uint8_t   pan_sustain;
	uint8_t   pan_loop_start;
	uint8_t   pan_loop_end;
	uint8_t   volume_type;
	uint8_t   panning_type;
	uint8_t   vibrato_type;
	uint8_t   vibrato_sweep;
	uint8_t   vibrato_depth;
	uint8_t   vibrato_rate;
	uint16_t  volume_fadeout;
	//>>> End of standard info block. Size of this block is 212 bytes

	//Offset in XM file is 241:
	//Additional 2 bytes (empty in standard XM file):
	uint8_t        volume;	    //[for PsyTexx only]
        signed char  finetune;	    //[for PsyTexx only]
	
	//Offset in XM file is 243:
	//EXT_INST_BYTES starts here:
	uint8_t   panning;	    //[for PsyTexx only]
	signed char  relative_note; //[for PsyTexx only]
	uint8_t	flags;		    //[for PsyTexx only]
	
	//System use data: (not in XM file)
	uint16_t  volume_env[ ENV_TICKS ];  //each value is 0..1023
	uint16_t  panning_env[ ENV_TICKS ];
	
	sample  *samples[16];
};

struct module
{
	uint8_t   id_text[17];
        uint8_t   name[20];
	uint8_t   reserved1;
	uint8_t   tracker_name[20];
	uint16_t  version;
	uint    header_size;
	uint16_t  length;
	uint16_t  restart_position;
	uint16_t  channels; //Number of virtual (song) channels
	uint16_t  patterns_num;
	uint16_t  instruments_num;
	uint16_t  freq_table;
	uint16_t  tempo;
	uint16_t  bpm;
	uint8_t   patterntable[256];
	
	pattern    *patterns[256];
	instrument *instruments[128];

	//Real channel number = virt_chan_num * SUBCHANNELS + real_channel_num[ virt_chan_num ];
	char    real_channel_num[ MAX_CHANNELS ];
};

struct channel
{
	uint    enable;
	uint    recordable;
	uint    paused;
	int     lvalue;  //For the scope drawing
	int     rvalue;  //For the scope drawing
	int     temp_lvalue;
	int     temp_rvalue;
	int     scope[ SCOPE_SIZE ]; //Normal values are -127...127
    
	sample  *smp;
	instrument *ins;
	int	ins_num;
	int     note;    //current note
	int     period;
        uint    delta;   //integer part
	uint    delta_p; //part after fixed point (16 bits)
        int     ticks;   //integer part
	int     ticks_p; //part after fixed point (16 bits)
	uint    back;    //0: ticks+=delta;  1: ticks-=delta
	
	uint    v_pos;   //volume envelope position
	uint    p_pos;   //panning envelope position
	uint    sustain;
	int     fadeout; //fadeout volume (0..65536). it's 65536 after the note has been triggered
	int     vib_pos; //-32..31 - autovibrato position
	int     cur_frame;//0..vibrato_sweep - current frame for autovibrato

	int     vol;     //0..64 - start volumes (level 1)
	int     vol_after_tremolo; //0..64 - volume after tremolo
	int     pan;     //0..255 - current panning (level 1) (from worknote() function)
	//for volume interpolation (in the envelope() function):
	int     env_start; //first envelope frame flag (after envelope reset)
	int     vol_step;  //current interpolation step (from xx to 0)
	int     l_delta;   //0..1024<<12 - delta for left (12 because 12bits = 4096 - max step)
	int     r_delta;   //0..1024<<12 - delta for right
	int     l_cur;     //0..1024<<12 - current volumes:
	int     r_cur;     //0..1024<<12 
	int     l_old;     //0..1024 - previous volume
	int     r_old;     //0..1024 - previous volume

	//for frequency interpolation (in the envelope() function):
	int     cur_period;  //current period
	int     period_delta;//period delta
	int     old_period;  //period from previous tick

	//effects:
	int     v_down;  //volume down speed
	int     v_up;    //volume up speed
	int     pan_down;//panning down speed
	int     pan_up;  //panning up speed
	
	int     tremolo_type;      //tremolo type: 0 - sine; 1 - ramp down; 2 - square; 3 - random
	int     tremolo_pos;       //-32..31 - tremolo position
	int     tremolo_speed;     //1..15
	int     tremolo_depth;     //1..15
	int     old_tremolo_speed; //previous tremolo speed
	int     old_tremolo_depth; //previous tremolo depth
	int     old_tremolo;       //flag for tremolo control (in effects() function)
	
	int     p_speed;    //period speed
	int     p_period;   //target period
	int     tone_porta; //tone porta flag (porta effect ON/OFF)
	
	int     arpeggio_periods[4]; //periods (deltas) for arpeggio effect
	int     arpeggio_counter;    //0..2
	channel *arpeggio_main_channel;//Parent channel
	
	int     vibrato_type;  //vibrato type: 0 - sine; 1 - ramp down; 2 - square; 3 - random
	int     vibrato_pos;   //-32..31 - vibrato position
	int     vibrato_speed; //1..15
	int     vibrato_depth; //1..15
	int     old_speed;     //previous vibrato speed
	int     old_depth;     //previous vibrato depth
	int     new_period;    //new period after vibrato
	
	int     retrig_rate;   //0..15          - retrigger sample parameter:  E9x effect 
	int     retrig_count;  //0..retrig_rate - retrigger sample counter
	int     note_cut;      //0..15          - length of note (in ticks):   ECx effect
	int     note_delay;    //0..15          - delay length (in ticks + 1): EDx effect
	xmnote  *note_pointer; //note pointer for EDx effect
	
	//old parameters for effects:
	int     old_p_speed;   //old period speed (tone porta)
	int     old_p_speed2;  //old period speed (porta up/down)
	int     old_ticks;     //for 9xx
	int     old_fine_up;   //for E1x
	int     old_fine_down; //for E2x
	int     old_slide_up;  //for EAx
	int     old_slide_down;//for EBx
	int     old_vol_up;    //for Axx
	int     old_vol_down;  //for Axx

	int     ch_effect_freq;
	int     freq_ptr;
	int     freq_r;
	int     freq_l;
	int     ch_effect_bits;
};

enum {
    XM_STATUS_STOP = 0,
    XM_STATUS_PLAY,
    XM_STATUS_PPLAY,
    XM_STATUS_PLAYLIST
};

struct xm_struct
{
    module     *song;
    channel    *channels[ MAX_REAL_CHANNELS ]; //Real channels

    psynth_net *pnet; //Psynth engine

    int        status;           //Current playing status
    int        song_finished;    //Current song finished - we must to load new song (playlist mode)
    int        counter;          //Number of current buffer

    int        freq;             //sound frequency
    uint       *linear_tab;      //linear frequency table
    uint16_t     *stereo_tab;      //stereo (volumes) table
    uint8_t      *vibrato_tab;     //sinus table for vibrato effect
    
    int        chans;            //global number of channels
    
    int        sample_int;       //sample interpolation ON/OFF
    int        freq_int;         //frequency interpolation ON/OFF
    
    int        global_volume;    //64 = normal (1.0)

    int        bpm;              //beats (4 lines) per minute (using for onetick calculate only)
    int        speed;            //speed (ticks per line)
    int        sp;               //dinamic value
    int        onetick;          //one tick size.  one pattern line has 1/2/3/4... ticks
    int        tick_number;      //current tick number (it is 0 after new line start)
    int        patternticks;     //current position (from 0 to one tick)
    int        one_subtick;      //one subtick. 1 tick = 64 subticks   ** for frequency interpolation **
    int        subtick_count;    //from 0 to subtick                   ** for frequency interpolation **
    
    int        tablepos;         //position in pat-table
    int        patternpos;       //position in current pattern
    int        jump_flag;        //flag for jump effects
    int        loop_start;       //loop pattern start
    int        loop_count;       //loop pattern counter;
    
    //Realtime note playing:
    int        octave;		 
    int        cur_channel;      //Current real channel number
    char       channel_busy[ MAX_REAL_CHANNELS ];
    //For real-time playing. Notes from this buffer will be copied to main channels
    channel    ch_buf[ CHANNELS_IN_BUFFER ];
    int        ch_channels[ CHANNELS_IN_BUFFER ];
    int        ch_read_ptr;
    int        ch_write_ptr;

    /*
    uint       eq[ 2048 * 4 ];
    float      i[ 2048 * 4 ];
    float      r[ 2048 * 4 ];
    float      old_amp[ 2048 * 4 ];
    */
};

//Main functions:

void xm_init( xm_struct *xm );
void xm_close( xm_struct *xm );
void xm_set_volume( int volume, xm_struct *xm );

uint read_int(FILE *f);
uint16_t read_int16(FILE *f);

void xm_pat_rewind(xm_struct *xm); //Rewind to the pattern start
void xm_rewind(xm_struct *xm);     //Rewind to the song start
void clear_struct(xm_struct *xm);
void create_envelope(uint16_t *src, uint16_t points, uint16_t *dest);
void load_module( char *filename, xm_struct *xm );
int mod_load( FILE *f, xm_struct *xm );
int xm_load( FILE *f, xm_struct *xm );
int xm_save( const char *filename, xm_struct *xm );

//Channels:

void clear_channels(xm_struct *xm);
void clean_channels(xm_struct *xm);
void new_channels( int number_of_channels, xm_struct *xm );
int play_note( int note_num, int instr_num, int pressure, xm_struct *xm );
void stop_note( int channel_num, xm_struct *xm );

//Instruments:

void new_instrument( uint16_t num, 
                     const char *name, 
                     uint16_t samples, 
                     xm_struct *xm );
void clear_instrument( uint16_t num, xm_struct *xm );
void clear_instruments( xm_struct *xm );
void clear_comments( xm_struct *xm );
void save_instrument( uint16_t num, char *filename, xm_struct *xm );
void load_instrument( uint16_t num, char *filename, xm_struct *xm );
int load_raw_instrument( uint16_t num, FILE *f, char *name, xm_struct *xm, window_manager *wm );
void load_wav_instrument( uint16_t num, FILE *f, char *name, xm_struct *xm );
void load_xi_instrument( uint16_t num, FILE *f, xm_struct *xm );

//Patterns:

void new_pattern( uint16_t num,
                  uint16_t rows,
		  uint16_t channels,
		  xm_struct *xm );
void resize_pattern( uint16_t num,
                     uint16_t rows,
	             uint16_t channels,
		     xm_struct *xm );
void clear_pattern( uint16_t num, xm_struct *xm );
void clear_patterns( xm_struct *xm );
void clean_pattern( uint16_t num, xm_struct *xm );

//Playing:

int xm_callback(void*,int,void*);
void worknote(xmnote*, int, int, xm_struct*);
void envelope(int, xm_struct*);
void effects(channel*, xm_struct*);

//Samples:

void new_sample(uint16_t num, 
                uint16_t ins_num, 
                const char *name,
		int length,      /*length in bytes*/
		int type,
                xm_struct *xm);
void clear_sample(uint16_t num, uint16_t ins_num, xm_struct *xm);
void bytes2frames( sample *smp, xm_struct *xm ); //Convert sample length from bytes to frames
void frames2bytes( sample *smp, xm_struct *xm ); //Convert sample length from frames to bytes (before XM-saving)

//Song:

void close_song(xm_struct *xm);
void clear_song(xm_struct *xm);
void new_song(xm_struct *xm);
void create_silent_song(xm_struct *xm);

void set_bpm( int bpm, xm_struct *xm );
void set_speed( int speed, xm_struct *xm );

//Tables:

extern uint linear_tab[768];
extern uint8_t vibrato_tab[256];
extern uint16_t stereo_tab[256*2]; //l,r, l,r, l,r, ...  min/max: 0/1024

void tables_init();
