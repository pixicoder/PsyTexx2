/*
    time.cpp - time management
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "time.h"

#ifdef OS_LINUX
    #include <time.h>
#endif
#ifndef NONPALM
    #include <PalmOS.h>
#endif
#ifdef OS_WIN
    #include <windows.h>
    #include <time.h>
#endif

uint time_hours( void )
{
#ifdef OS_LINUX
    //LINUX:
    time_t t;
    time( &t );
    return localtime( &t )->tm_hour;
#endif
#ifndef NONPALM
    //PALM:
    DateTimeType t;
    TimSecondsToDateTime( TimGetSeconds(), &t );
    return t.hour;
#endif
#ifdef OS_WIN
    //WINDOWS:
    time_t t;
    time( &t );
    return localtime( &t )->tm_hour;
#endif
}

uint time_minutes( void )
{
#ifdef OS_LINUX
    //LINUX:
    time_t t;
    time( &t );
    return localtime( &t )->tm_min;
#endif
#ifndef NONPALM
    //PALM:
    DateTimeType t;
    TimSecondsToDateTime( TimGetSeconds(), &t );
    return t.minute;
#endif
#ifdef OS_WIN
    //WINDOWS:
    time_t t;
    time( &t );
    return localtime( &t )->tm_min;
#endif
}

uint time_seconds( void )
{
#ifdef OS_LINUX
    //LINUX:
    time_t t;
    time( &t );
    return localtime( &t )->tm_sec;
#endif
#ifndef NONPALM
    //PALM:
    DateTimeType t;
    TimSecondsToDateTime( TimGetSeconds(), &t );
    return t.second;
#endif
#ifdef OS_WIN
    //WINDOWS:
    time_t t;
    time( &t );
    return localtime( &t )->tm_sec;
#endif
}

uint time_ticks_per_second( void )
{
#ifdef OS_LINUX
    //LINUX:
    return 1000;
#endif
#ifndef NONPALM
    //PALM:
    return SysTicksPerSecond();
#endif
#ifdef OS_WIN
    //WINDOWS:
    return 1000;
#endif
}

uint time_ticks( void )
{
#ifdef OS_LINUX
    //LINUX:
    time_t tt;
    time( &tt );
    
    timespec t;
    clock_gettime( CLOCK_REALTIME, &t );
    return ( localtime( &tt )->tm_hour * 3600000 ) + 
           ( localtime( &tt )->tm_min * 60000 ) + 
	   ( localtime( &tt )->tm_sec * 1000 ) +
	   ( t.tv_nsec / 1000000 );
#endif
#ifndef NONPALM
    //PALM:
    return TimGetTicks();
#endif
#ifdef OS_WIN
    //WINDOWS:
    return GetTickCount();
#endif
}

void time_sleep( int milliseconds )
{
#ifdef OS_UNIX
    #if defined(OS_APPLE)
        while( 1 )
        {
            int t = milliseconds;
            if( t > 1000 ) t = 1000;
            usleep( t * 1000 );
            milliseconds -= t;
            if( milliseconds <= 0 ) break;
        }
    #else
        #ifdef OS_EMSCRIPTEN
            emscripten_sleep( milliseconds );
        #else
            timeval t;
            t.tv_sec = milliseconds / 1000;
            t.tv_usec = ( milliseconds % 1000 ) * 1000;
            select( 0 + 1, 0, 0, 0, &t );
        #endif
    #endif
#endif
#if defined(OS_WIN) || defined(OS_WINCE)
    Sleep( milliseconds );
#endif
#ifdef PALMOS
    volatile uint t = time_ticks();
    uint len = ( milliseconds * time_ticks_per_second() ) / 1000;
    while( 1 )
    {
        volatile uint t2 = time_ticks();
        if( t2 - t >= len ) break;
    }
#endif
}
