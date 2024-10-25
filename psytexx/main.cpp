/*
    psytexx.cpp - PsyTexx main()
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "win_main.h"

//################################
//## DEVICE VARIABLES:          ##
//################################

#ifdef PALMOS
    #include <PalmOS.h>
    #define arm_startup __attribute__ ((section ("arm_startup")))
    #include "palm_functions.h"
#endif

//################################
//################################
//################################

//################################
//## APPLICATION MAIN:          ##
//################################

int g_argc = 0;
char* g_argv[ 64 ];

//********************************
//WIN32 MAIN *********************
#ifdef OS_WIN
int APIENTRY WinMain(HINSTANCE hCurrentInst, HINSTANCE hPreviousInst, LPSTR lpszCmdLine, int nCmdShow)
{
    {
#endif
//********************************
//********************************

//********************************
//PALMOS MAIN ********************
#ifdef PALMOS
int ARM_PalmOS_main( const void *emulStateP, void *userData, Call68KFuncType *call68KFuncP ) arm_startup;
int ARM_PalmOS_main( const void *emulStateP, void *userData, Call68KFuncType *call68KFuncP )
{
    {
	volatile void* oldGOT;
	volatile register void* gGOT asm ("r10");
	volatile ARM_INFO* arm_info = (ARM_INFO *)userData;
	oldGOT = (void*)gGOT;
	//gGOT = (void *)arm_info->GOT;
	volatile unsigned int newgot = (unsigned int)arm_info->GOT;
	__asm__ ( "mov r10, %0" : : "r" (newgot) );
	ownID = (unsigned short)arm_info->ID;
	g_form_handler = arm_info->FORMHANDLER; //g_form_handler is defined in palm_functions.cpp
	g_new_screen_size = arm_info->new_screen_size;
	CALL_INIT
	int autooff_time = SysSetAutoOffTime( 0 );
	g_argc = 1;
	g_argv[ 0 ] = (char*)"prog";
	int argc = g_argc;
	char** argv = g_argv;
#endif //PALMOS
//********************************
//********************************

//********************************
//LINUX MAIN *********************
#ifdef OS_LINUX
int main(int argc, char *argv[])
{
    {
	wm.argc = argc;
	wm.argv = argv;
#ifdef TEXTMODE
	char* str;
	if( argc > 1 )
	{
	    str = argv[ 1 ];
	    if( str[ 0 ] == '-' || str[ 0 ] == '?' )
	    {
		printf( "\033[%dm\033[%dm", 37, 40 );
		printf( "PsyTexx 2 for Linux.\n" );
		printf( "\033[%dm\033[%dm", 36, 40 );
		printf( "Command line options:\n" );
		printf( "\033[%dm\033[%dm", 35, 40 );
		printf( "m - auto mouse key up (for some GPM versions)\n" );
		printf( "\033[%dm\033[%dm", 37, 40 );
		return 0;
	    }
	    if( str[ 0 ] == 'm' ) mouse_auto_keyup = 1;
	}
#endif
#endif
//********************************
//********************************

	slog_reset();

	slog( "\n" );
	slog( "\n" );
	slog( PSYTEXX_VERSION "\n" );
	slog( PSYTEXX_DATE "\n" );
	slog( "\n" );
	slog( "STARTING...\n" );
	slog( "\n" );
	slog( "\n" );

	mem_set( &wm, sizeof( window_manager ), 0 );
#ifdef OS_WIN
	wm.hCurrentInst = hCurrentInst;
	wm.hPreviousInst = hPreviousInst; 
	wm.lpszCmdLine = lpszCmdLine;
	wm.nCmdShow = nCmdShow;
#endif

	get_disks();
	psy_windows_init();
	psy_event_loop();
        psy_windows_close();

	slog_close();
	mem_free_all();       //Close all memory blocks

	slog( "\n" );
	slog( "\n" );
	slog( "BYE !\n" );
	slog( "\n" );
	slog( "\n" );

#ifdef PALMOS
	SysSetAutoOffTime( autooff_time );
	//gGOT = (void*)oldGOT;
	newgot = (unsigned int)oldGOT;
	__asm__ ( "mov r10, %0" : : "r" (newgot) );
#endif
    }

    return 0;
}

//################################
//################################
//################################
