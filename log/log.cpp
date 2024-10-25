/*
    log.cpp - log management
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "log.h"
#include "../memory/memory.h"
#include "../wm/wm.h"

#define PALMOS_Y_LIMIT 70

static int g_slog_disable_counter = 0;

void slog_disable()
{
    g_slog_disable_counter++;
}

void slog_enable()
{
    if( g_slog_disable_counter > 0 )
	g_slog_disable_counter--;
}

static int g_slog_count = 0;
static char g_slog_tmp_buf[ 256 ];
static char* g_slog_buf = 0;
static int g_slog_buf_size = 0;
static int y = 10;

static const char* g_slog_file = "log.txt";

void slog_set_output_file( const char *filename )
{
    g_slog_file = filename;
}

void slog_reset()
{
#ifdef NONPALM
    remove( g_slog_file );
#endif
}

void slog_close()
{
    if( g_slog_buf && g_slog_buf != g_slog_tmp_buf ) 
    {
	mem_free( g_slog_buf );
	g_slog_buf = g_slog_tmp_buf;
	g_slog_buf_size = 256;
    }
}

void slog( const char* format, ... )
{
    if( g_slog_disable_counter ) return;

    va_list p;
    va_start( p, format );
    if( g_slog_buf_size == 0 )
    {
	g_slog_buf = g_slog_tmp_buf;
	g_slog_buf_size = 256;
	g_slog_buf = (char*)MEM_NEW( HEAP_DYNAMIC, 256 );
    }
    int ptr = 0;
    int ptr2 = 0;
    char num_str[ 64 ];
    int len;

    //Make a number:
    int_to_string( g_slog_count, num_str );
    len = mem_strlen( num_str );
    mem_copy( g_slog_buf, num_str, len );
    g_slog_buf[ len ] = ':';
    g_slog_buf[ len + 1 ] = ' ';
    ptr2 += len + 2;
    g_slog_count++;

    //Make a string:
    for(;;)
    {
	if( format[ ptr ] == 0 ) break;
	if( format[ ptr ] == '%' )
	{
	    if( format[ ptr + 1 ] == 'd' )
	    {
		int arg = va_arg( p, int );
		int_to_string( arg, num_str );
		len = mem_strlen( num_str );
		if( ptr2 + len >= g_slog_buf_size && g_slog_buf != g_slog_tmp_buf )
		{
		    //Resize debug buffer:
		    g_slog_buf_size += 256;
		    g_slog_buf = (char*)mem_resize( g_slog_buf, g_slog_buf_size );
		}
		mem_copy( g_slog_buf + ptr2, num_str, len );
		ptr2 += len;
		ptr++;
	    }
	    else
	    if( format[ ptr + 1 ] == 's' )
	    {
		//ASCII string:
		char *arg2 = va_arg( p, char* );
		if( arg2 )
		{
		    len = mem_strlen( arg2 );
		    if( len )
		    {
			if( ptr2 + len >= g_slog_buf_size && g_slog_buf != g_slog_tmp_buf )
			{
			    //Resize debug buffer:
			    g_slog_buf_size += 256;
			    g_slog_buf = (char*)mem_resize( g_slog_buf, g_slog_buf_size );
			}
			mem_copy( g_slog_buf + ptr2, arg2, len );
			ptr2 += len;
		    }
		}
		ptr++;
	    }
	}
	else
	{
	    g_slog_buf[ ptr2 ] = format[ ptr ];
	    ptr2++;
	    if( ptr2 >= g_slog_buf_size && g_slog_buf != g_slog_tmp_buf )
	    {
		//Resize debug buffer:
		g_slog_buf_size += 256;
		g_slog_buf = (char*)mem_resize( g_slog_buf, g_slog_buf_size );
	    }
	}
	ptr++;
    }
    g_slog_buf[ ptr2 ] = 0;
    va_end( p );

    //Save result:
#ifdef NONPALM
    FILE *f = fopen( g_slog_file, "ab" );
    if( f )
    {
	fprintf( f, "%s", g_slog_buf );
	fclose( f );
    }
    printf( "%s", g_slog_buf );
#else
    //PalmOS:
    int a;
    for( a = 0; a < 128; a++ )
    {
	if( g_slog_buf[ a ] == 0 ) break;
	if( g_slog_buf[ a ] == 0xA ) break;
	if( g_slog_buf[ a ] == 0xD ) break;
    }
    WinDrawChars( "                                                                                ", 80, 0, y );
    WinDrawChars( g_slog_buf, a, 0, y );
    y += 10;
    if( y >= PALMOS_Y_LIMIT ) y = 0;
#endif
}

