/*
    xm_sndout.cpp - cross-platform sound output
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "sound.h"
#include "../log/log.h"
#include "../file/file.h"
#include "../time/time.h"

sound_struct snd; //Main sound structure;

//################################
//## LINUX:                     ##
//################################
#define LINUX_SOUND_MODE 1 //0 - OSS; 1 - SDL2;

#ifdef OS_LINUX
#include <pthread.h>

#if LINUX_SOUND_MODE == 0 //OSS:
#include <linux/soundcard.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
int dsp;
pthread_t pth;
void *sound_thread (void *arg)
{
    char* buf = (char*)malloc( snd.buflen * 4 );
    int len = snd.buflen;
    for(;;) {
	main_callback( (sound_struct *)arg, 0, buf, len );
	if( dsp >= 0 ) write( dsp, buf, len * 4 ); else break;
    }
    free( buf );
    pthread_exit(0);
    return 0;
}
#endif

#if LINUX_SOUND_MODE == 1 //SDL:
#include "SDL2/SDL.h"
bool sdl_sound_output = 0;
static void sdl_audio_callback( void* udata, Uint8* stream, int len )
{
    main_callback( (sound_struct *)udata, 0, stream, len / 4 );
}
#endif

#endif //OS_LINUX

//################################
//## PALMOS:                    ##
//################################
#ifdef PALMOS
#include "PalmOS.h"
SndStreamRef main_stream = 0;
MemHandle ARM_code_handler;
char *ARM_code;
char paused = 0;
#endif

//################################
//## WIN32:                     ##
//################################
#ifdef OS_WIN
#include "dsound.h"
#define NUMEVENTS 2
LPDIRECTSOUND               lpds;
DSBUFFERDESC                dsbdesc;
LPDIRECTSOUNDBUFFER         lpdsb = 0;
LPDIRECTSOUNDBUFFER         lpdsbPrimary;
LPDIRECTSOUNDNOTIFY         lpdsNotify;
WAVEFORMATEX                *pwfx;
HMMIO                       hmmio;
MMCKINFO                    mmckinfoData, mmckinfoParent;
DSBPOSITIONNOTIFY           rgdsbpn[NUMEVENTS];
HANDLE                      rghEvent[NUMEVENTS];

//Sound thread:
HANDLE sound_thread;
SECURITY_ATTRIBUTES atr;
bool StreamToBuffer( uint dwPos )
{
    int            lNumToWrite;
    DWORD           dwStartOfs;
    VOID            *lpvPtr1, *lpvPtr2;
    DWORD           dwBytes1, dwBytes2;
    static DWORD    dwStopNextTime = 0xFFFF;

    if( dwStopNextTime == dwPos )   // All data has been played
    {
	lpdsb->Stop();
	dwStopNextTime = 0xFFFF;
	return TRUE;
    }

    if( dwStopNextTime != 0xFFFF )  // No more to stream, but keep
		                        // playing to end of data
        return TRUE;

    if( dwPos == 0 )
	dwStartOfs = rgdsbpn[NUMEVENTS - 1].dwOffset;
    else
	dwStartOfs = rgdsbpn[dwPos-1].dwOffset;

    lNumToWrite = (int) rgdsbpn[dwPos].dwOffset - dwStartOfs;
    if (lNumToWrite < 0) lNumToWrite += dsbdesc.dwBufferBytes;

    IDirectSoundBuffer_Lock(lpdsb,
                dwStartOfs,       // Offset of lock start
                lNumToWrite,      // Number of bytes to lock
                &lpvPtr1,         // Address of lock start
                &dwBytes1,        // Count of bytes locked
                &lpvPtr2,         // Address of wrap around
                &dwBytes2,        // Count of wrap around bytes
                0);               // Flags

    //Write data to the locked buffer:
    main_callback( &snd, 0, lpvPtr1, dwBytes1 >> 2 );

    IDirectSoundBuffer_Unlock( lpdsb, lpvPtr1, dwBytes1, lpvPtr2, dwBytes2 );

    return TRUE;
}

DWORD __stdcall sound_callback( void* par )
{
    while( 1 )
    {
	DWORD dwEvt = MsgWaitForMultipleObjects(
			    NUMEVENTS,      // How many possible events
			    rghEvent,       // Location of handles
			    FALSE,          // Wait for all?
			    INFINITE,       // How long to wait
			    QS_ALLINPUT);   // Any message is an event

	dwEvt -= WAIT_OBJECT_0;

	// If the event was set by the buffer, there's input
	// to process. 

	if( dwEvt < NUMEVENTS ) 
	{
	    if( lpdsb )	StreamToBuffer( dwEvt ); // copy data to output stream
	}
    }
    return 0;
}

LPGUID guids[ 128 ];
int guids_num = 0;

BOOL CALLBACK DSEnumCallback (
    LPGUID GUID,
    LPCSTR Description,
    LPCSTR Module,
    VOID *Context
)
{
    slog( "Found sound device %d: %s\n", guids_num, Description );
    guids[ guids_num ] = GUID;
    guids_num++;
    return 1;
}
#endif

//################################
//## OTHER FUNCTIONS:           ##
//################################
int temp_variable = 0;
void temp_function(void)
{
    temp_variable++;
    if( temp_variable == 65000 ) slog( "temp function is working\n" );
}

void sound_stream_init(void)
{
#ifdef OS_LINUX
    int temp;
#endif
#ifndef NONPALM
    uint processor; //Processor type
#endif

    snd.user_data = user_sound_data;
    snd.buflen = 1024;
#ifdef OS_WIN
    snd.buflen = 2560;
#endif
    if( get_option( OPT_SOUNDBUFFER ) != -1 ) snd.buflen = get_option( OPT_SOUNDBUFFER );

#ifdef OS_LINUX
#if LINUX_SOUND_MODE == 0 //OSS:
    dsp = open ( "/dev/dsp", O_WRONLY, 0 );
    //dsp = -1;
    if( dsp == -1 )
    {
        slog( "Can't open sound device\n" );
        return;
    }
    temp = 1;
    ioctl (dsp, SNDCTL_DSP_STEREO, &temp);
    temp = 16;
    ioctl (dsp, SNDCTL_DSP_SAMPLESIZE, &temp);
    temp = 44100;
    ioctl (dsp, SNDCTL_DSP_SPEED, &temp);
    temp = 16 << 16 | 8;
    ioctl (dsp, SNDCTL_DSP_SETFRAGMENT, &temp);
    ioctl (dsp, SNDCTL_DSP_GETBLKSIZE, &temp);
    //Create sound thread:
    if( pthread_create ( &pth, NULL, sound_thread, &snd ) != 0 )
    {
        printf ("Can't create sound thread!\n");
        return;
    }
#endif //OSS
#if LINUX_SOUND_MODE == 1 //SDL:
    SDL_Init( 0 );
    SDL_AudioSpec a;
    a.freq = 44100;
    a.format = AUDIO_S16;
    a.channels = 2;
    a.samples = snd.buflen;
    a.callback = sdl_audio_callback;
    a.userdata = &snd;
    if( SDL_OpenAudio( &a, NULL ) < 0 )
    {
        printf( "Couldn't open audio: %s\n", SDL_GetError() );
        return;
    }
    sdl_sound_output = 1;
    SDL_PauseAudio( 0 );
#endif //SDL
#endif

#ifdef PALMOS
    ARM_code = (char*)main_callback;
    FtrGet( sysFileCSystem, sysFtrNumProcessorID, &processor );
    if( sysFtrNumProcessorIsARM( processor ) )
    SndStreamCreate( &main_stream,
                     sndOutput,
		     44100,
		     sndInt16Little,
		     sndStereo,
		     (SndStreamBufferCallback) ARM_code,
		     &snd,
		     4096,
		     1 );
    else main_stream = 0;
#endif

#ifdef OS_WIN
    HWND hWnd = GetForegroundWindow();
    if( hWnd == NULL )
    {
        hWnd = GetDesktopWindow();
    }
    LPVOID EnumContext = 0;
    DirectSoundEnumerate( DSEnumCallback, EnumContext );
    if FAILED( DirectSoundCreate( 0, &lpds, NULL ) )
    {
        MessageBox( hWnd,"DSound: DirectSoundCreate error","Error",MB_OK);
        return;
    }
    if FAILED( IDirectSound_SetCooperativeLevel(
	lpds, hWnd, DSSCL_PRIORITY ) )
    {
	MessageBox( hWnd,"DSound: SetCooperativeLevel error","Error",MB_OK);
	return;
    }

    WAVEFORMATEX wfx;
    memset( &wfx, 0, sizeof(WAVEFORMATEX) ); 
    wfx.wFormatTag = WAVE_FORMAT_PCM; 
    wfx.nChannels = 2; 
    wfx.nSamplesPerSec = 44100; 
    wfx.wBitsPerSample = 16; 
    wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    pwfx = &wfx;

    // Secondary buffer:
	
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); 
    dsbdesc.dwSize = sizeof(DSBUFFERDESC); 
    dsbdesc.dwFlags = 
            DSBCAPS_GETCURRENTPOSITION2   // Always a good idea
            | DSBCAPS_GLOBALFOCUS         // Allows background playing
            | DSBCAPS_CTRLPOSITIONNOTIFY; // Needed for notification
 
    // The size of the buffer is arbitrary, but should be at least
    // two seconds, to keep data writes well ahead of the play
    // position.
 
    dsbdesc.dwBufferBytes = snd.buflen * 4 * 2;
    dsbdesc.lpwfxFormat = pwfx;

    if FAILED( IDirectSound_CreateSoundBuffer(
               lpds, &dsbdesc, &lpdsb, NULL) )
    {
	MessageBox( hWnd,"DSound: Create secondary buffer error","Error",MB_OK);
	return;
    }

    //Create buffer events:
	
    for (int i = 0; i < NUMEVENTS; i++)
    {
        rghEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (NULL == rghEvent[i]) 
	{
	    MessageBox( hWnd,"DSound: Create event error","Error",MB_OK);
	    return;
	}
    }
    rgdsbpn[ 0 ].dwOffset = 0;
    rgdsbpn[ 0 ].hEventNotify = rghEvent[ 0 ];
    rgdsbpn[ 1 ].dwOffset = ( dsbdesc.dwBufferBytes / 2 );
    rgdsbpn[ 1 ].hEventNotify = rghEvent[ 1 ];
	
    if FAILED( lpdsb->QueryInterface( IID_IDirectSoundNotify, (VOID **)&lpdsNotify ) )
    {
    	MessageBox( hWnd,"DSound: QueryInterface error","Error",MB_OK);
    	return;
    }
 
    if FAILED(IDirectSoundNotify_SetNotificationPositions(
             lpdsNotify, NUMEVENTS, rgdsbpn))
    {
        IDirectSoundNotify_Release(lpdsNotify);
	MessageBox( hWnd,"DSound: SetNotificationPositions error","Error",MB_OK);
	return;
    }

    IDirectSoundBuffer_Play( lpdsb, 0, 0, DSBPLAY_LOOPING );

    //Create main thread:
    atr.nLength = sizeof(atr);
    atr.lpSecurityDescriptor = 0;
    atr.bInheritHandle = 0;
    sound_thread = CreateThread( &atr, 8000, &sound_callback, &snd, 0, 0 );
    SetThreadPriority( sound_thread, THREAD_PRIORITY_TIME_CRITICAL );
#endif
}

void sound_stream_play(void)
{
    snd.user_data = user_sound_data;

#ifdef OS_LINUX
#if LINUX_SOUND_MODE == 0 //OSS:
    if( dsp >= 0 )
#endif
#if LINUX_SOUND_MODE == 1 //SDL:
    if( sdl_sound_output )
#endif
    {
	snd.need_to_stop = 0;
    }
#endif

#ifndef NONPALM
    if( main_stream )
    {
	snd.need_to_stop = 0;
	if( paused ) 
	{
	    SndStreamPause( main_stream, 0 );
	    paused = 0;
	}
	else SndStreamStart( main_stream );
    }
#endif

#ifdef OS_WIN
    if( lpdsb )
    {
	snd.need_to_stop = 0;
    }
#endif
}

void sound_stream_stop(void)
{
#ifdef OS_LINUX
#if LINUX_SOUND_MODE == 0 //OSS:
    if( dsp >= 0 )
#endif
#if LINUX_SOUND_MODE == 1 //SDL:
    if( sdl_sound_output )
#endif
    {
	snd.stream_stoped = 0;
	snd.need_to_stop = 1;
	while( snd.stream_stoped == 0 ) time_sleep( 1 );
    }
#endif

#ifndef NONPALM
    if( main_stream )
    {
	snd.stream_stoped = 0;
	snd.need_to_stop = 1;
	while( snd.stream_stoped == 0 ) { temp_function(); }
	SndStreamPause( main_stream, 1 );
	paused = 1;
    }
#endif

#ifdef OS_WIN
    if( lpdsb )
    {
	snd.stream_stoped = 0;
	snd.need_to_stop = 1;
	while( snd.stream_stoped == 0 ) time_sleep( 1 );
    }
#endif
}

void sound_stream_close(void)
{
    sound_stream_stop();

#ifdef OS_LINUX
#if LINUX_SOUND_MODE == 0 //OSS:
    int our_dsp = dsp;
    dsp = -1;
    if( our_dsp >= 0 ) close( our_dsp );
#endif //OSS
#if LINUX_SOUND_MODE == 1 //SDL:
    SDL_CloseAudio();
    SDL_Quit();
    sdl_sound_output = 0;
#endif //SDL
#endif

#ifndef NONPALM
    if( main_stream )
    {
	SndStreamDelete( main_stream );
    }
#endif

#ifdef OS_WIN
    CloseHandle( sound_thread );
    if( lpdsb )
    {
        if (lpdsNotify)
	    lpdsNotify->Release();
	if (lpds)
	    lpds->Release();
    }
#endif
}
