/*
    memory.cpp - memory management
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

//2024: merged with memory.cpp from SunDog 2012.11.05 (with PalmOS support)

#include "memory.h"
#include "../log/log.h"
#include "../file/file.h"

//Special things for PalmOS:
#ifndef NONPALM
    #define memNewChunkFlagNonMovable    0x0200
    #define memNewChunkFlagAllowLarge    0x1000  // this is not in the sdk *g*
    #define USE_FILES                    1
    SysAppInfoPtr ai1, ai2, appInfo;
    unsigned short ownID;
#endif

#ifdef NONPALM
    #include <stdio.h>
    #include <stdlib.h>
    #include <string>
    #include <cstring>
#endif

#ifdef OS_WIN
    #include <windows.h>
#endif

mem_block_struct* dstart = 0;
mem_block_struct* sstart = 0;
mem_block_struct* prev_dblock = 0;  //Previous mem block in dynamic heap
mem_block_struct* prev_sblock = 0;  //Previous mem block in storage heap
size_t dsize = 0;
size_t ssize = 0;
size_t max_dsize = 0;
size_t max_ssize = 0;
size_t g_mem_error = 0;

int mem_free_all()
{
#ifdef MEM_FAST_MODE
    return 0;
#else
    int rv = 0;
    mem_block_struct* next;
    int mnum = 0;
    int mlimit = 10;

    mem_block_struct* sstart2 = sstart;
    if( sstart2 ) slog( "MEMORY CLEANUP (STORAGE)\n" );
    mnum = 0;
    while( sstart2 )
    {
#ifdef MEM_USE_NAMES
        char name[ MEM_MAX_NAME_SIZE ];
        name[ 0 ] = 0;
        mem_strcat( name, (const char*)sstart2->name );
#endif
        size_t size;
        next = sstart2->next;
        size = sstart2->size;
        sstart2 = next;
        if( mnum < mlimit )
        {
#ifdef MEM_USE_NAMES
            slog( "FREE %d %s\n", (int)size, name );
#else
            slog( "FREE %d\n", (int)size );
#endif
        }
        mnum++;
    }
    while( sstart )
    {
        size_t size;
        next = sstart->next;
        size = sstart->size;
        simple_mem_free( sstart );
        sstart = next;
    }

    mem_block_struct* dstart2 = dstart;
    if( dstart2 ) slog( "MEMORY CLEANUP (DYNAMIC)\n" );
    mnum = 0;
    while( dstart2 )
    {
#ifdef MEM_USE_NAMES
        char name[ MEM_MAX_NAME_SIZE ];
        name[ 0 ] = 0;
        mem_strcat( name, (const char*)dstart2->name );
#endif
        size_t size;
        next = dstart2->next;
        size = dstart2->size;
        dstart2 = next;
        if( mnum < mlimit )
        {
#ifdef MEM_USE_NAMES
            slog( "FREE %d %s\n", (int)size, name );
#else
            slog( "FREE %d\n", (int)size );
#endif
        }
        mnum++;
    }
    while( dstart )
    {
        size_t size;
        next = dstart->next;
        size = dstart->size;
        simple_mem_free( dstart );
        dstart = next;
    }

#ifdef USE_FILES
    remove( "mem_storage" );
    remove( "mem_dynamic" );
#endif

    dstart = 0;
    sstart = 0;
    prev_dblock = 0;
    prev_sblock = 0;

    slog( "Max dynamic memory used: %d\n", (int)max_dsize );
    slog( "Max storage memory used: %d\n", (int)max_ssize );
    slog( "%d %d\n", (int)dsize, (int)ssize );
    if( dsize || ssize ) rv = 1;

    return rv;
#endif //not MEM_FAST_MODE
}

void* mem_new( uint32_t heap, size_t size, const char* name )
{
#ifdef PALMOS
#ifndef NOSTORAGE
#ifdef USE_FILES
    static bool mem_manager_started = 0;
    if( mem_manager_started == 0 )
    {
        mem_manager_started = 1;
        FILE *f = fopen( "mem_storage", "rb" );
        if( f )
        {
            slog( "MEMORY CLEANUP\n" );
            mem_block_struct* ptr;
            mem_block_struct* next;
            fread( &ptr, sizeof( void* ), 1, f );
            if( ptr )
            for(;;)
            {
                slog( "FREE %d\n", (int)ptr->size );
                next = ptr->next;
                simple_mem_free( ptr );
                if( next == 0 ) break;
                ptr = next;
            }
            fclose( f );
        }
    }
#endif
#endif
#endif

    size_t real_size = size;
    size_t new_size = size + sizeof( mem_block_struct ); //Add structure with info to our memory block
#ifdef PALMOS
    //PalmOS:
    #ifdef NOSTORAGE
    heap = ( heap & (~HEAP_MASK) ) + HEAP_DYNAMIC;
    #endif
    mem_block_struct* m = (mem_block_struct*)MemChunkNew( MemHeapID( 0, heap & HEAP_MASK ), new_size, ownID | memNewChunkFlagNonMovable | memNewChunkFlagAllowLarge );
#else
    mem_block_struct* m = (mem_block_struct*)malloc( new_size );
#endif

    //Save info about new memory block:
    if( m )
    {
        mem_off();

        m->size = size;
        m->heap = heap + 123456;
#ifdef MEM_USE_NAMES
        char* mname = (char*)m->name;
        for( int np = 0; np < 15; np++ ) { mname[ np ] = name[ np ]; if( name[ np ] == 0 ) break; }
        mname[ 15 ] = 0;
#endif

#ifndef MEM_FAST_MODE
        //sundog_mutex_lock( &g_mem_mutex );

        if( ( heap & HEAP_MASK ) == HEAP_DYNAMIC )
        {
            m->prev = prev_dblock;
            m->next = 0;
            if( prev_dblock == 0 )
            {
                //It is the first block. Save address:
#ifdef MEM_USE_FILES
                FILE *f = fopen( "mem_dynamic", "wb" );
                if( f )
                {
                    fwrite( &m, sizeof( void* ), 1, f );
                    fclose( f );
                }
#endif
                dstart = m;
                prev_dblock = m;
            }
            else
            {
                //It is not the first block:
                prev_dblock->next = m;
                prev_dblock = m;
            }
        }
        else
        {
            m->prev = prev_sblock;
            m->next = 0;
            if( prev_sblock == 0 )
            {
                //It is the first block. Save address:
#ifdef MEM_USE_FILES
                FILE *f = fopen( "mem_storage", "wb" );
                if( f )
                {
                    fwrite( &m, sizeof( void* ), 1, f );
                    fclose( f );
                }
#endif
                sstart = m;
                prev_sblock = m;
            }
            else
            {
                //It is not the first block:
                prev_sblock->next = m;
                prev_sblock = m;
            }
        }
#endif //not MEM_FAST_MODE

        if( ( heap & HEAP_MASK ) == HEAP_DYNAMIC ) { dsize += real_size; if( dsize > max_dsize ) max_dsize = dsize; }
        else
        if( ( heap & HEAP_MASK ) == HEAP_STORAGE ) { ssize += real_size; if( ssize > max_ssize ) max_ssize = ssize; }

#ifndef MEM_FAST_MODE
        //sundog_mutex_unlock( &g_mem_mutex );
#endif

        mem_on();
    }
    else
    {
        slog( "MEM ALLOC ERROR %d %s\n", (int)size, name );
        if( g_mem_error == 0 )
        {
            g_mem_error = size;
        }
#ifdef PALMOS
        slog( "####\n" );
        slog( "####\n" );
        slog( "####\n" );
#endif
        return 0;
    }

    char* rv = (char*)m;
    return (void*)( rv + sizeof( mem_block_struct ) );
}

void simple_mem_free( void* ptr )
{
#ifdef PALMOS
    MemPtrFree( ptr );
#else
    free( ptr );
#endif
}

void mem_free( void* ptr )
{
    if( ptr == 0 ) return;

    mem_block_struct* m = (mem_block_struct*)( (char*)ptr - sizeof( mem_block_struct ) );

    uint32_t heap = mem_get_heap( ptr );

#ifndef MEM_FAST_MODE
    //sundog_mutex_lock( &g_mem_mutex );
#endif

    if( heap == HEAP_DYNAMIC ) dsize -= m->size;
    else if( heap == HEAP_STORAGE ) ssize -= m->size;

#ifndef MEM_FAST_MODE
    mem_off();
    mem_block_struct* prev = m->prev;
    mem_block_struct* next = m->next;
    if( prev && next )
    {
        prev->next = next;
        next->prev = prev;
    }
    if( prev && next == 0 )
    {
        prev->next = 0;
        if( heap == HEAP_DYNAMIC ) prev_dblock = prev; else prev_sblock = prev;
    }
    if( prev == 0 && next )
    {
        next->prev = 0;
        if( heap == HEAP_DYNAMIC )
        {
#ifdef USE_FILES
            FILE *f = fopen( "mem_dynamic", "wb" );
            if( f )
            {
                fwrite( &next, sizeof( void* ), 1, f );
                fclose( f );
            }
#endif
            dstart = next;
        }
        else
        {
#ifdef USE_FILES
            FILE *f = fopen( "mem_storage", "wb" );
            if( f )
            {
                fwrite( &next, sizeof( void* ), 1, f );
                fclose( f );
            }
#endif
            sstart = next;
        }
    }
    if( prev == 0 && next == 0 )
    {
        if( heap == HEAP_DYNAMIC )
        {
            prev_dblock = 0;
#ifdef USE_FILES
            remove( "mem_dynamic" );
#endif
            dstart = 0;
        }
        else
        {
            prev_sblock = 0;
#ifdef USE_FILES
            remove( "mem_storage" );
#endif
            sstart = 0;
        }
    }
    mem_on();

    //sundog_mutex_unlock( &g_mem_mutex );
#endif //not MEM_FAST_MODE

    simple_mem_free( m );
}

void mem_set( void *ptr, size_t size, unsigned char value )
{
    if( ptr == 0 ) return;
#ifdef NONPALM
    memset(ptr,value,size);
#else
    MemSet(ptr,size,value);
#endif
}

void* mem_resize( void* ptr, size_t new_size )
{
    if( ptr == 0 )
    {
        return MEM_NEW( HEAP_DYNAMIC, new_size );
    }

    size_t old_size = mem_get_size( ptr );

    if( old_size == new_size ) return ptr;

    mem_off();
#ifdef PALMOS
    //free() + new():
    void* new_ptr = mem_new( mem_get_heap( ptr ), new_size, "resized block" );
    if( new_ptr == 0 ) { mem_on(); return 0; }
    if( old_size > new_size )
        mem_copy( new_ptr, ptr, new_size );
    else
        mem_copy( new_ptr, ptr, old_size );
    mem_free( ptr );
#else
    //realloc():
#ifdef MEM_FAST_MODE
    mem_block_struct* m = (mem_block_struct*)( (char*)ptr - sizeof( mem_block_struct ) );
    mem_block_struct* new_m = (mem_block_struct*)realloc( m, new_size + sizeof( mem_block_struct ) );
    void* new_ptr = (void*)( (char*)new_m + sizeof( mem_block_struct ) );
    new_m->size = new_size;
    uint32_t heap = mem_get_heap( new_ptr );
    if( heap == HEAP_DYNAMIC ) { dsize += new_size - old_size; if( dsize > max_dsize ) max_dsize = dsize; }
    else
    if( heap == HEAP_STORAGE ) { ssize += new_size - old_size; if( ssize > max_ssize ) max_ssize = ssize; }
#else
    //sundog_mutex_lock( &g_mem_mutex );
    int change_prev_block = 0;
    mem_block_struct* m = (mem_block_struct*)( (char*)ptr - sizeof( mem_block_struct ) );
    if( prev_dblock == m ) change_prev_block |= 1;
    if( prev_sblock == m ) change_prev_block |= 2;
    mem_block_struct* new_m = (mem_block_struct*)realloc( m, new_size + sizeof( mem_block_struct ) );
    void* new_ptr = (void*)( (char*)new_m + sizeof( mem_block_struct ) );
    if( change_prev_block & 1 ) prev_dblock = new_m;
    if( change_prev_block & 2 ) prev_sblock = new_m;
    new_m->size = new_size;
    mem_block_struct* prev = new_m->prev;
    mem_block_struct* next = new_m->next;
    uint32_t heap = mem_get_heap( new_ptr );
    if( heap == HEAP_DYNAMIC && prev == 0 )
    {
        dstart = new_m;
#ifdef USE_FILES
        FILE *f = fopen( "mem_dynamic", "wb" );
        if( f )
        {
            fwrite( &dstart, sizeof( void* ), 1, f );
            fclose( f );
        }
#endif
    }
    if( heap == HEAP_STORAGE && prev == 0 )
    {
        sstart = new_m;
#ifdef USE_FILES
        FILE *f = fopen( "mem_storage", "wb" );
        if( f )
        {
            fwrite( &sstart, sizeof( void* ), 1, f );
            fclose( f );
        }
#endif
    }
    if( prev != 0 )
    {
        prev->next = new_m;
    }
    if( next != 0 )
    {
        next->prev = new_m;
    }
    if( heap == HEAP_DYNAMIC ) { dsize += new_size - old_size; if( dsize > max_dsize ) max_dsize = dsize; }
    else
    if( heap == HEAP_STORAGE ) { ssize += new_size - old_size; if( ssize > max_ssize ) max_ssize = ssize; }
    //sundog_mutex_unlock( &g_mem_mutex );
#endif //not MEM_FAST_MODE
#endif //not PALMOS
    if( old_size < new_size )
    {
        mem_set( (char*)new_ptr + old_size, new_size - old_size, 0 );
    }
    mem_on();

    return new_ptr;
}

void mem_copy( void *dest, const void *src, size_t size )
{
    if( dest == 0 || src == 0 ) return;
#ifdef NONPALM
    memcpy( dest, src, size );
#else
    MemMove( dest, src, size ); //It's for dinamic heap only!!
#endif
}

int mem_cmp( const char *p1, const char *p2, size_t size )
{
    if( p1 == 0 || p2 == 0 ) return 0;
#ifdef NONPALM
    return memcmp( p1, p2, size );
#else
    return MemCmp( p1, p2, size ); //It's for dinamic heap only!!
#endif
}

void mem_strcat( char *dest, const char* src )
{
    if( dest == 0 || src == 0 ) return;
#ifndef NONPALM
    StrCat( dest, src );
#else
    strcat( dest, src );
#endif
}

int mem_strcmp( const char *s1, const char *s2 )
{
#ifndef NONPALM
    return StrCompare( s1, s2 );
#else
    return strcmp( s1, s2 );
#endif
}

int mem_strlen( const char *s )
{
    if( s == 0 ) return 0;
    int a;
    for( a = 0;; a++ ) if( s[ a ] == 0 ) break;
    return a;
}

char *mem_strdup( const char *s1 )
{
    int len = mem_strlen( s1 );
    char *newstr = (char*)MEM_NEW( HEAP_DYNAMIC, len + 1 );
    mem_copy( newstr, s1, len + 1 );
    return newstr;
}

uint32_t mem_get_heap( void* ptr )
{
    if( ptr == 0 ) return 0;
    mem_block_struct* m = (mem_block_struct*)( (char*)ptr - sizeof( mem_block_struct ) );
    return ( m->heap - 123456 ) & HEAP_MASK;
}

size_t mem_get_size( void* ptr )
{
    if( ptr == 0 ) return 0;
    mem_block_struct* m = (mem_block_struct*)( (char*)ptr - sizeof( mem_block_struct ) );
    return m->size;
}

char* mem_get_name( void* ptr )
{
#ifdef MEM_USE_NAMES
    if( ptr == 0 ) return 0;
    mem_block_struct* m = (mem_block_struct*)( (char*)ptr - sizeof( mem_block_struct ) );
    return m->name;
#else
    return 0;
#endif
}

int off_count = 0;
void mem_on(void)
{
#ifndef NONPALM
#ifndef NOSTORAGE
    off_count--;
    if( off_count == 0 )
	MemSemaphoreRelease(1);
#endif
#endif
}

void mem_off(void)
{
#ifndef NONPALM
#ifndef NOSTORAGE
    if( off_count == 0 )
	MemSemaphoreReserve(1);
    off_count++;
#endif
#endif
}

void mem_palm_normal_mode(void)
{
#ifndef NONPALM
#ifndef NOSTORAGE
    if( off_count > 0 )
    { //At the moment mem protection is off:
	MemSemaphoreRelease(1); //mem protection on
    }
#endif
#endif
}

void mem_palm_our_mode(void)
{
#ifndef NONPALM
#ifndef NOSTORAGE
    if( off_count > 0 )
    {
	MemSemaphoreReserve(1); //mem protection off
    }
#endif
#endif
}
