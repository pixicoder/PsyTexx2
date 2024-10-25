/*
    This file is public domain
*/

#ifndef __PALM_ARM_FUNCTIONS__
#define __PALM_ARM_FUNCTIONS__

#include <PalmOS.h>
#include "VFSMgr.h"
#include "PceNativeCall.h"

// Main structure for the ARM-main function
struct ARM_INFO
{
    void *GLOBALS;
    void *GOT;
    void *FORMHANDLER;
    long ID;
    long *new_screen_size;
};

#define BSwap16(n) (	((((unsigned long) n) <<8 ) & 0xFF00) | \
			((((unsigned long) n) >>8 ) & 0x00FF) )

#define BSwap32(n) (	((((unsigned long) n) << 24 ) & 0xFF000000) | \
			((((unsigned long) n) << 8 ) & 0x00FF0000) | \
			((((unsigned long) n) >> 8 ) & 0x0000FF00) | \
			((((unsigned long) n) >> 24 ) & 0x000000FF) )

// local definition of the emulation state structure
struct EmulStateType 
{
	UInt32 instr;
	UInt32 regData[8];
	UInt32 regAddress[8];
	UInt32 regPC;
};
extern void *g_form_handler;
extern long *g_new_screen_size;
extern EmulStateType *emulStatePtr;
extern Call68KFuncType *call68KFuncPtr;
extern unsigned char args_stack[ 4 * 32 ];
extern unsigned char args_ptr;

#define CALL_INIT \
emulStatePtr = (EmulStateType*)emulStateP; \
call68KFuncPtr = call68KFuncP;

#define CALL args_ptr = 0;

#define P1( par ) \
args_stack[ args_ptr ] = (unsigned char)par; args_ptr++; \
args_stack[ args_ptr ] = 0; args_ptr++;

#define P2( par ) \
args_stack[ args_ptr ] = (unsigned char)((unsigned short)par>>8); args_ptr++; \
args_stack[ args_ptr ] = (unsigned char)((unsigned short)par&255); args_ptr++;

#define P4( par ) \
args_stack[ args_ptr ] = (unsigned char)((unsigned long)par>>24); args_ptr++; \
args_stack[ args_ptr ] = (unsigned char)((unsigned long)par>>16); args_ptr++; \
args_stack[ args_ptr ] = (unsigned char)((unsigned long)par>>8); args_ptr++; \
args_stack[ args_ptr ] = (unsigned char)((unsigned long)par&255); args_ptr++;

#define TRAP( trap ) \
(call68KFuncPtr) (emulStatePtr,PceNativeTrapNo(trap),args_stack,args_ptr); 

#define TRAPP( trap ) \
(call68KFuncPtr) (emulStatePtr,PceNativeTrapNo(trap),args_stack,args_ptr|kPceNativeWantA0); 

#define	MemPtrFree(p) MemChunkFree(p)

//Posix compatibility:

typedef char		int8_t;
typedef unsigned char	uint8_t;
typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef int             int32_t;
typedef unsigned int    uint32_t;
typedef unsigned int    size_t;
typedef unsigned int	FILE;

void* malloc( size_t size );
void* realloc( void* ptr, size_t size );
void free( void* ptr );
void* memcpy( void* destination, const void* source, size_t num );
int strcmp( const char* str1, const char* str2 );
const char* strstr( const char* str1, const char* str2 );
int memcmp( const char* p1, const char* p2, size_t size );
size_t strlen( const char* str1 );
char* strdup( const char* s1 );

FILE* fopen( const char* filename, const char* filemode );
int fclose( FILE* fpp );
void rewind( FILE* fpp );
int getc( FILE* fpp );
int ftell ( FILE* fpp );
int fseek( FILE* fpp, int offset, int access );
int feof( FILE* fpp );
size_t fread( void* ptr, size_t el_size, size_t elements, FILE* fpp );
size_t fwrite( const void* ptr, size_t el_size, size_t elements, FILE* fpp );
int fputc( int val, FILE* fpp );
int remove( const char* filename );

int sprintf( char* dest_str, const char* str, ... );

#endif //__PALM_ARM_FUNCTIONS__
