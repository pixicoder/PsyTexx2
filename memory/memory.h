#pragma once

#include "../core/core.h"

enum
{
    HEAP_DYNAMIC = 0,
    HEAP_STORAGE
};
#define HEAP_MASK               3

#ifndef MEM_FAST_MODE
    #ifndef PALMOS
	#define MEM_USE_NAMES
    #endif
#endif

#define MEM_MAX_NAME_SIZE 16
struct mem_block_struct
{
    uint32_t heap;
    size_t size;
#ifdef MEM_USE_NAMES
    char name[ MEM_MAX_NAME_SIZE ];
#endif
#ifndef MEM_FAST_MODE
    mem_block_struct* next;
    mem_block_struct* prev;
#endif
};

#define MEM_NEW( heap, size ) mem_new( heap, size, __FUNCTION__ )

//mem_new:
//heap - heap number:
//0 = dynamic heap for small blocks
//1 = storage heap for large static blocks
//size - block size
int mem_free_all();
void* mem_new( unsigned int heap, size_t size, const char *name );
void mem_free( void *ptr );
void simple_mem_free( void* ptr );
void mem_set( void *ptr, size_t size, unsigned char value );
void* mem_resize( void *ptr, size_t size );
void mem_copy( void *dest, const void *src, size_t size );
int mem_cmp( const char *p1, const char *p2, size_t size );
void mem_strcat( char *dest, const char *src );
int mem_strcmp( const char *s1, const char *s2 );
int mem_strlen( const char *s );
char *mem_strdup( const char *s1 );

//Get info about memory block:
uint32_t mem_get_heap( void *ptr );
size_t mem_get_size( void *ptr );
char *mem_get_name( void *ptr );

//Palm specific:
void mem_on(void);  //Storage protection ON
void mem_off(void); //Storage protection OFF
void mem_palm_normal_mode(void); //Switch to normal mode (Storage protection ON)
void mem_palm_our_mode(void);    //Switch back to our mode (Storage protection is ON or OFF)

#ifdef PALMOS
#include <PalmOS.h>
extern SysAppInfoPtr ai1, ai2, appInfo;
extern unsigned short ownID;
#endif
