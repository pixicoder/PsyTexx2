/*
    file.cpp - file management
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#include "file.h"
#include "../log/log.h"
#include "../memory/memory.h"
#include "../time/time.h"

#ifdef NONPALM
    #include <string>
    #include <cstring>
#endif

int disks = 0;                          //Number of local disks
char disk_names[DISKNAME_SIZE*MAX_DISKS];//Disk names. Each name - 4 bytes (with NULL) (for example: "C:/", "H:/")
char current_dir[MAX_DIR_LENGTH];        //Current dir name
char current_filename[MAX_DIR_LENGTH];
char* get_disk_name( int number ) { return disk_names + (DISKNAME_SIZE * number); }

#ifdef OS_WIN
void get_disks(void)
{
    char temp[512];
    int len, p, dp, tmp;
    disks = 0;
    len = GetLogicalDriveStrings( 512, temp );
    for( dp = 0, p = 0; p < len; p++ )
    {
	tmp = (disks*DISKNAME_SIZE) + dp;
	disk_names[ tmp ] = temp[ p ];
	if( disk_names[tmp] == 92 ) disk_names[tmp] = '/';
	if( temp[p] == 0 ) { disks++; dp = 0; continue; }
	dp++;
    }
}
int get_current_disk(void)
{
    char cur_dir[MAX_DIR_LENGTH];
    int a;
    char *disk_name;
    GetCurrentDirectory( MAX_DIR_LENGTH, cur_dir );
    for( a = 0; a < disks; a++ )
    {
	disk_name = get_disk_name( a );
	if( disk_name[0] == cur_dir[0] ) return a;
    }
    return 0;
}
char* get_current_dir(void)
{
    int a;
    int b;
    GetCurrentDirectory( MAX_DIR_LENGTH, current_dir );
    for( a = 0; a < MAX_DIR_LENGTH; a++ ) //Make "/mydir" from "\mydir"
    {
	if( current_dir[ a ] == 92 ) current_dir[ a ] = '/';
    }
    for( a = 0; a < MAX_DIR_LENGTH; a++ ) //Search for first '/'
    {
	if( current_dir[ a ] == '/' ) break;
    }
    for( b = 0; b < MAX_DIR_LENGTH; b++ ) //Make dir without "C:/"
    {
	current_dir[ b ] = current_dir[ b + a + 1 ];
	if( current_dir[ b ] == 0 ) break;
    }
    current_dir[ b ] = '/';
    current_dir[ b + 1 ] = 0;
    return current_dir;
}
#endif
#ifndef NONPALM
uint16_t vfs_volumes = 0;
void get_disks(void)
{
    int d;
    char disk_char = 'B'; //VFS disk name
    disks = 1;
    disk_names[0] = 'A';
    disk_names[1] = ':';
    disk_names[2] = '/';
    disk_names[3] = 0;
    open_vfs(); //Check for VFS volumes
    for( d = 0; d < vfs_volumes; d++ ) 
    {
	disk_names[0 + (DISKNAME_SIZE*d) + DISKNAME_SIZE] = disk_char++;
        disk_names[1 + (DISKNAME_SIZE*d) + DISKNAME_SIZE] = ':';
	disk_names[2 + (DISKNAME_SIZE*d) + DISKNAME_SIZE] = '/';
	disk_names[3 + (DISKNAME_SIZE*d) + DISKNAME_SIZE] = 0;
	disks++;
    }
}
int get_current_disk(void)
{
    return 0;
}
char* get_current_dir(void)
{
    current_dir[ 0 ] = 0;
    return current_dir;
}
#endif
#ifdef OS_UNIX
void get_disks(void)
{
    disks = 1;
    disk_names[0] = '/';
    disk_names[1] = 0;
}
int get_current_disk(void)
{
    return 0;
}
char* get_current_dir(void)
{
    int a;
    int b;
    getcwd( current_dir, MAX_DIR_LENGTH );
    for( a = 0; a < MAX_DIR_LENGTH; a++ ) //Search for first '/'
    {
	if( current_dir[ a ] == '/' ) break;
    }
    for( b = 0; b < MAX_DIR_LENGTH; b++ ) //Make dir without "C:/"
    {
	current_dir[ b ] = current_dir[ b + a + 1 ];
	if( current_dir[ b ] == 0 ) break;
    }
    current_dir[ b ] = '/';
    current_dir[ b + 1 ] = 0;
    return current_dir;    
}
#endif


#ifdef PALMOS

//PalmOS functions:
#define MAX_RECORDS 256
#define MAX_F_POINTERS 2
uint32_t r_size[ MAX_F_POINTERS ][ MAX_RECORDS ]; //size of each record in file
uint32_t f_size[ MAX_F_POINTERS ]; //size of each file (for each file pointer)
uint32_t cur_rec[ MAX_F_POINTERS ]; //current record number (for each file pointer)
int cur_off[ MAX_F_POINTERS ]; //current record offset (for each file pointer)
uint32_t cur_pnt[ MAX_F_POINTERS ]; //current file offset (for each file pointer)
uint32_t recs[ MAX_F_POINTERS ]; //number of records (for each file pointer)
DmOpenRef cur_db[ MAX_F_POINTERS ]; //current DB pointer
LocalID ID; //local ID for DB
uint8_t fp[ MAX_F_POINTERS ] = { 0, 0 }; //for each file pointer: 0-free 1-working
uint8_t* cur_rec_pnt[ MAX_F_POINTERS ];
MemHandle cur_rec_h[ MAX_F_POINTERS ];
FileRef vfs_file[ MAX_F_POINTERS ]; //VFS file or NULL
char vfs_first_read[ MAX_F_POINTERS ]; //1 - after first read
uint8_t write_flag[ MAX_F_POINTERS ]; //0 - read mode; 1 - write mode
uint16_t vfs_volume_numbers[ 8 ];

int open_vfs()
{
    UInt32 vol_iterator = vfsIteratorStart;
    Err error;
    vfs_volumes = 0;
    while( vol_iterator != vfsIteratorStop ) 
    {
	mem_palm_normal_mode(); //#####
	error = VFSVolumeEnumerate( &vfs_volume_numbers[vfs_volumes], &vol_iterator );
	mem_palm_our_mode();    //#####
	if( error == errNone ) vfs_volumes++;
    }
    if( vfs_volumes ) return 1; else return 0;
}

int isBusy( DmOpenRef db, int index )  //Is record busy?
{
    unsigned short attributes;
    DmRecordInfo( db, index, &attributes, 0, 0 );
    if( attributes & dmRecAttrBusy ) return 1;
    return 0;
}

void recordsBusyReset( int f )
{
    int a;
    for( a = 0; a < recs[f]; a++ )
    {
	if( isBusy( cur_db[f], a ) ) DmReleaseRecord( cur_db[f], a, 0 );
    }
}

void get_records_info( DmOpenRef db, uint16_t f_num )
{
    MemHandle cur_h;
    unsigned char *cur;
    uint16_t a;
    recs[ f_num ] = DmNumRecords( db );
    f_size[ f_num ]=0;
    for( a = 0; a < recs[ f_num ]; a++ ) if( isBusy( db, a ) ) { DmReleaseRecord( db, a, 0 ); }
    for( a = 0; a < recs[ f_num ]; a++ ) 
    {
	cur_h = DmGetRecord( db, a );
        cur = (uint8_t*)MemHandleLock( cur_h );
        r_size[ f_num ][ a ]=MemHandleSize( cur_h );
	f_size[ f_num ] += r_size[ f_num ][ a ];
	MemHandleUnlock( cur_h );
    }
}

void create_new_record( DmOpenRef db )
{
    int records = DmNumRecords(db);
    uint16_t new_index = dmMaxRecordIndex;
    DmNewRecord( db, &new_index, 50000 );
}

FILE* fopen( const char *filename, const char *filemode )
{
    uint32_t a;
    if( filemode[0] == 'r' ) 
    { //read file:
	if( ( filename[0] == 'A' && filename[1] == ':' ) || filename[1] != ':' )
	{ //It's PalmOS PDB:
	    if( filename[0] == 'A' && filename[1] == ':' )
		filename += 3; //Skip "A:/"
	    ID = DmFindDatabase( 0, (const char*)filename );
	    if( ID != 0 )
	    { //file exist:
		for(a=0;a<MAX_F_POINTERS;a++) if(fp[a]==0) break; //searching for a free file pointer
		if( a == MAX_F_POINTERS ) return 0; //not enough memory for new pointer
		fp[a] = 1;
		write_flag[a] = 0;
		vfs_file[a] = 0; //Not VFS 
		cur_db[a]=DmOpenDatabase(0,ID,dmModeReadWrite); //open
		get_records_info(cur_db[a],a);
		cur_rec[a]=0;
		cur_off[a]=0;
		cur_pnt[a]=0;
		if( isBusy(cur_db[a],0) ) DmReleaseRecord( cur_db[a], 0, 0 );
		cur_rec_h[a]=DmGetRecord(cur_db[a],0);
		cur_rec_pnt[a] = (uint8_t*)MemHandleLock(cur_rec_h[a]);
		return (FILE*)(a+1);
	    } else /*file not found*/ return 0;
	}
	else
	{ //It's VFS:
	    if( filename[0] - 65 >= disks ) return 0; //no such disk
	    int disk_num = filename[0] - 66;
	    filename += 2; //Skip "X:"
	    if( open_vfs() ) {
		//slog( "open vfs\n" );
		for(a=0;a<MAX_F_POINTERS;a++) if(fp[a]==0) break; //searching for a free file pointer
		if( a == MAX_F_POINTERS ) return 0; //not enough memory for new pointer
		//slog( "vfs ok\n" );
		mem_palm_normal_mode(); //##### 
		Err err = VFSFileOpen( vfs_volume_numbers[ disk_num ], filename, vfsModeRead, &vfs_file[a] );
		mem_palm_our_mode();    //#####
		//slog( "vfs file open ok\n" );
		if( err != errNone ) {slog("error in FileOpen %d\n",err);return 0;} //File not found
		//File found:
		fp[a] = 1;
		write_flag[a] = 0;
		vfs_first_read[a] = 0;
		
		//Create buffer:
		
		char temp_filename[3];
		temp_filename[0] = (char)a + 65;
		temp_filename[1] = 'F';
		temp_filename[2] = 0;
		ID = DmFindDatabase( 0, (const char*)temp_filename );
		if( ID != 0 ) DmDeleteDatabase( 0, ID ); //File exist: delete it
		//Create new database:
		DmCreateDatabase( 0, (const char*)temp_filename, 'ZULU', 'PSYX', 0 );
		ID = DmFindDatabase( 0, (const char*)temp_filename );
		cur_db[a] = DmOpenDatabase( 0, ID, dmModeWrite ); //open
		uint16_t new_rec = 0;
		DmNewRecord( cur_db[a], &new_rec, 60000 );
		DmCloseDatabase( cur_db[a] );
		cur_db[a] = DmOpenDatabase( 0, ID, dmModeReadWrite ); //open it again
		get_records_info(cur_db[a],a);
		cur_rec[a]=0;
		cur_off[a]=0;
		cur_pnt[a]=0;
		if( isBusy(cur_db[a],0) ) DmReleaseRecord( cur_db[a], 0, 0 );
		cur_rec_h[a]=DmGetRecord(cur_db[a],0);
		cur_rec_pnt[a] = (uint8_t*)MemHandleLock(cur_rec_h[a]);
		VFSFileSize( vfs_file[a], (UInt32*)&f_size[a] );
	    
		return (FILE*)(a+1);
	    }
	}
    }
    if( filemode[0] == 'w' ) 
    { //write file:
	if( ( filename[0] == 'A' && filename[1] == ':' ) || filename[1] != ':' )
	{ //It's PalmOS PDB:
	    if( filename[0] == 'A' && filename[1] == ':' )
		filename += 3; //Skip "A:/"
	    //slog( "FS: find database        \n" );
	    ID = DmFindDatabase( 0, (const char*)filename );
	    if( ID != 0 ) DmDeleteDatabase( 0, ID ); //File exist: delete it
	    //Create new database:
	    //slog( "FS create new        \n" );
	    DmCreateDatabase( 0, (const char*)filename, 'ZULU', 'PSYX', 0 );
	    //Open new file:
	    //slog( "FS: open (find)     \n" );
	    ID = DmFindDatabase( 0, (const char*)filename );
	    if( ID != 0 )
	    { //file exist:
		for(a=0;a<MAX_F_POINTERS;a++) if(fp[a]==0) break; //searching for free file pointer
		if( a == MAX_F_POINTERS ) return 0; //not enough memory for new pointer
		fp[a] = 1;
		write_flag[a] = 1;
		vfs_file[a] = 0; //Not VFS 
		//slog( "FS: open             \n" );
		cur_db[a] = DmOpenDatabase( 0, ID, dmModeWrite ); //open
		//slog( "FS: create new record      \n" );
		create_new_record( cur_db[a] );
		//slog( "FS: get info             \n" );
		get_records_info( cur_db[a], a );
		cur_rec[a] = 0;
		cur_off[a] = 0;
		cur_pnt[a] = 0;
		//slog( "FS: release record       \n" );
		if( isBusy(cur_db[a],0) ) DmReleaseRecord( cur_db[a], 0, 0 );
		//slog( "FS: get record           \n" );
		cur_rec_h[a] = DmGetRecord( cur_db[a], 0 );
		//slog( "FS: lock                   \n" );
		cur_rec_pnt[a] = (uint8_t*)MemHandleLock( cur_rec_h[a] );
		return (FILE*)(a+1);
	    } else /*file not found*/ return 0;
	}
	else
	{ //It's VFS:
	    get_disks();
	    if( filename[0] - 65 >= disks ) return 0; //no such disk
	    int disk_num = filename[0] - 66;
	    filename += 2; //Skip "X:"
	    if( open_vfs() ) 
	    {
		for(a=0;a<MAX_F_POINTERS;a++) if(fp[a]==0) break; //searching for free file pointer
		if( a == MAX_F_POINTERS ) return 0; //not enough memory for new pointer
		mem_palm_normal_mode(); //#####
		Err err = VFSFileOpen( vfs_volume_numbers[ disk_num ], filename, vfsModeRead, &vfs_file[a] );
		VFSFileClose( vfs_file[a] ); 
		if( err == errNone )
		{ //File exist:
		    //Delete it:
		    VFSFileDelete( vfs_volume_numbers[ disk_num ], filename ); 
		}
		err = VFSFileOpen( vfs_volume_numbers[ disk_num ], filename, vfsModeCreate|vfsModeWrite, &vfs_file[a] );
		mem_palm_our_mode();    //#####
		if( err != errNone ) return 0; //File open error
		//File found:
		fp[a] = 1;
		write_flag[a] = 1;
		
		//Create buffer:
		
		char temp_filename[3];
		temp_filename[0] = (char)a + 65;
		temp_filename[1] = 'F';
		temp_filename[2] = 0;
		ID = DmFindDatabase( 0, (const char*)temp_filename );
		if( ID != 0 ) DmDeleteDatabase( 0, ID ); //File exist: delete it
		//Create new database:
		DmCreateDatabase( 0, (const char*)temp_filename, 'ZULU', 'PSYX', 0 );
		ID = DmFindDatabase( 0, (const char*)temp_filename );
		cur_db[a] = DmOpenDatabase( 0, ID, dmModeReadWrite ); //open it
		uint16_t new_rec = 0;
		DmNewRecord( cur_db[a], &new_rec, 60000 );
		DmCloseDatabase( cur_db[a] );
		cur_db[a] = DmOpenDatabase( 0, ID, dmModeReadWrite ); //open it again
		get_records_info(cur_db[a],a);
		cur_rec[a]=0;
		cur_off[a]=0;
		cur_pnt[a]=0;
		if( isBusy(cur_db[a],0) ) DmReleaseRecord( cur_db[a], 0, 0 );
		cur_rec_h[a]=DmGetRecord(cur_db[a],0);
		cur_rec_pnt[a] = (uint8_t*)MemHandleLock(cur_rec_h[a]);
		
		return (FILE*)(a+1);
	    }
	}
    }
    return 0;
}

int fclose(FILE *fpp)
{
    uint32_t f=(uint32_t)fpp;
    if(f!=0)
    {
	f--;
	if(fp[f]==1) 
	{
	    if( vfs_file[f] == 0 )
	    { //PDB:
		fp[f] = 0; 
		if(cur_rec_h[f]!=0) MemHandleUnlock(cur_rec_h[f]); //close previous record
		if( write_flag[ f ] )
		{
		    DmResizeRecord( cur_db[f], cur_rec[f], cur_off[f] );
		}
		recordsBusyReset( f );
		DmCloseDatabase(cur_db[f]); return 0; //file close
	    }
	    else
	    { //VFS:
		if( cur_off[ f ] && write_flag[ f ] )
		{ //if there is some unsaved data in a buffer:
		    buffer2vfs(f); //save it
		}
		fp[ f ] = 0;
		mem_palm_normal_mode(); //#####
		VFSFileClose( vfs_file[ f ] );
		mem_palm_our_mode();    //#####
		
		//Close buffer:
		if(cur_rec_h[f]!=0) MemHandleUnlock(cur_rec_h[f]); //close previous record
		recordsBusyReset( f );
		DmCloseDatabase(cur_db[f]); //file close

		//Delete buffer:
		char temp_filename[3];
		temp_filename[0] = (char)f + 65;
		temp_filename[1] = 'F';
		temp_filename[2] = 0;
		ID = DmFindDatabase( 0, (const char*)temp_filename );
		if( ID != 0 ) DmDeleteDatabase( 0, ID ); //File exist: delete it
		return 0;
	    }
	}
    }
    return 1; //error value
}

void buffer2vfs(uint32_t f)
{
    slog( "saving block to VFS\n" );
    UInt32 bytes_written;
    if( cur_off[f] == 0 ) return;
    mem_palm_normal_mode(); //#####
    Err err = VFSFileWrite( vfs_file[f], cur_off[f], cur_rec_pnt[f], &bytes_written );
    mem_palm_our_mode();    //#####
    if( err != errNone ) { slog( "error in buffer2vfs %d\n", err ); }
    cur_off[f] = 0;
}

void vfs2buffer(uint32_t f)
{
    slog( "geting block from VFS\n" );
    UInt32 bytes_read;
    int size = f_size[f] - cur_pnt[f];
    if( size > r_size[f][0] ) size = r_size[f][0];
    if( size <= 0 ) return;
    mem_palm_normal_mode(); //#####
    Err err = VFSFileReadData( vfs_file[f], size, cur_rec_pnt[f], 0, &bytes_read );
    r_size[ f ][ 0 ] = bytes_read;
    mem_palm_our_mode();    //#####
    if( err != errNone ) { slog( "error in vfs2buffer %d\n", err ); }
    vfs_first_read[f] = 1;
}

int next_record(uint32_t f)
{ //go to the next record 
    if( vfs_file[f] )
    { //VFS file (working with a buffer):
	cur_off[f] = 0;
	if( !write_flag[f] )
	{ //Read next block from VFS to our buffer:
	    vfs2buffer(f);
	}
    }
    else
    { //PDB file:
	cur_off[f] = 0;
	cur_rec[f] ++;
	if( cur_rec_h[f] != 0 ) 
	    { MemHandleUnlock(cur_rec_h[f]); cur_rec_h[f] = 0; } //close previous
	if( cur_rec[f] < recs[f] )
	{
    	    if( isBusy(cur_db[f],cur_rec[f]) ) DmReleaseRecord( cur_db[f], cur_rec[f], 0 );
    	    cur_rec_h[f]=DmGetRecord(cur_db[f],cur_rec[f]);
	    cur_rec_pnt[f] = (uint8_t*)MemHandleLock( cur_rec_h[f] );
	}
	else
	{
	    if( write_flag[f] )
	    { //create new record:
		create_new_record( cur_db[f] );
		recs[f]++;
		r_size[f][ recs[f] - 1 ] = 50000;
    		if( isBusy(cur_db[f],cur_rec[f]) ) DmReleaseRecord( cur_db[f], cur_rec[f], 0 );
    		cur_rec_h[f] = DmGetRecord( cur_db[f], cur_rec[f] );
		cur_rec_pnt[f] = (uint8_t*)MemHandleLock( cur_rec_h[f] );
	    }
	}
    }
    return 0;
}

void rewind( FILE *fpp )
{
    uint32_t f = (uint32_t)fpp;
    if( f != 0 )
    {
    	f--;
	cur_off[f] = 0;
	cur_rec[f] = 0;
	cur_pnt[f] = 0;
	if( vfs_file[f] )
	{
	    Err err = VFSFileSeek( vfs_file[f], vfsOriginBeginning, 0 );
	}
	else
	{
	    if( cur_rec_h[f] != 0 ) 
		{ MemHandleUnlock(cur_rec_h[f]); cur_rec_h[f] = 0; } //close previous
	    //Open new record:
    	    if( isBusy(cur_db[f],cur_rec[f]) ) DmReleaseRecord( cur_db[f], cur_rec[f], 0 );
    	    cur_rec_h[f] = DmGetRecord( cur_db[f], cur_rec[f] );
	    cur_rec_pnt[f] = (uint8_t*)MemHandleLock( cur_rec_h[f] );
	}
    }
}

int getc(FILE *fpp)
{
    MemHandle cur_h;
    uint8_t *cur;
    uint8_t res;
    uint32_t bytes_read;
    uint32_t f=(uint32_t)fpp;
    if(f!=0)
    {
    	f--;
	if( vfs_file[f] )
	{
	    if( vfs_first_read[f] == 0 ) vfs2buffer(f); //Fill buffer for first time
	}
	if( cur_pnt[f] < f_size[f] )
	{
	    cur = cur_rec_pnt[f];
	    res = cur[cur_off[f]];
	    //inc position:
	    cur_pnt[f]++;
	    cur_off[f]++; 
	    if( cur_off[f] >= r_size[f][cur_rec[f]] ) 
	    {//if need for next record:
	        next_record(f);
	    }
	    return res;
	} else /*out of file space*/ return -1;
    }
    return -1;
}

uint32_t get_record_rest(FILE *fpp)
{ //get rest space (from current position to the end of current record)
    uint32_t f=(uint32_t)fpp;
    if(f!=0)
    {
	f--;
	if( cur_pnt[f] < f_size[f] || write_flag[f] )
	{
	    return r_size[f][cur_rec[f]]-cur_off[f];
	}
    }
    return 0; //out of file space
}

int ftell ( FILE *fpp )
{
    uint32_t f = (uint32_t)fpp;
    if( f != 0 )
    {
	f--;
	return cur_pnt[ f ];
    }
    return -1;
}

int fseek( FILE* fpp, int offset, int access )
{ //seek forward (file position += offset)
    uint32_t f=(uint32_t)fpp;
    uint32_t rest,off=0;
    if( f != 0 )
    {
	f--;
	if( vfs_file[f] == 0 )
	{ //PDB:
	    if( access == 1 )
	    if( cur_pnt[f] < f_size[f] || write_flag[f] )
	    {
		if( offset < 0 )
		{ //Seek back:
		    off = cur_pnt[ f ] + offset;
		    if( off >= 0 )
		    {
			rewind( fpp );
			fseek( fpp, off, 1 );
		    }
		}
		else //Seek forward:	    
		if( offset <= get_record_rest(fpp) ) 
		{ //offset <= rest record space
		    cur_pnt[f] += offset;
		    cur_off[f] += offset;
		    if(cur_off[f]>=r_size[f][cur_rec[f]]) next_record(f);
		    return offset;
		} 
		else 
		{ //offset is large (few records)
		    for(;;)
		    {
			rest = get_record_rest(fpp);
			if(offset>=rest) offset-=rest; else {rest=offset;offset=0;}
			cur_pnt[f]+=rest; off+=rest; 
			if( cur_pnt[f] >= f_size[f] && !write_flag[f] ) 
			    return off; //out of file space
		        if( offset == 0 ) 
			    { cur_off[f] = rest; return off; } //seekf is ok!
		        next_record(f);
		    }
		}
	    }
	}
	else
	{ //VFS:
	    int old_cur_pnt = cur_pnt[f];
	    FileOrigin origin;
	    if( access == 0 ) { origin = vfsOriginBeginning; cur_pnt[f] = offset; }
	    if( access == 1 ) { origin = vfsOriginCurrent; cur_pnt[f] += offset; }
	    if( access == 2 ) { origin = vfsOriginEnd; cur_pnt[f] = f_size[f] - 1 - offset; }
	    int delta = cur_pnt[f] - old_cur_pnt;
	    cur_off[f] += delta;
	    if( cur_off[f] >= r_size[f][0] || cur_off[f] < 0 )
	    { //We needs new block from VFS:
		mem_palm_normal_mode(); //#####
		Err err = VFSFileSeek( vfs_file[f], vfsOriginBeginning, cur_pnt[f] );
		mem_palm_our_mode();    //#####
		if( err == errNone )
		{
		    cur_off[f] = 0;
		    vfs2buffer(f);		
		    return 1;
		}
	    }
	    else return 1;
	}
    }
    return 0;
}

int feof(FILE *fpp)
{
    uint32_t f=(uint32_t)fpp;
    if(f!=0)
    {
	f--;
	if( cur_pnt[f] >= f_size[f] ) return 1;
    }
    return 0;
}

size_t fread( void* ptr, size_t el_size, size_t elements, FILE* fpp )
{
    uint32_t real_size=el_size*elements;
    uint32_t size; //current block size
    uint8_t *cur;
    uint8_t res;
    uint32_t f=(uint32_t)fpp;
    uint32_t rest,off=0;
    if(f!=0)
    {
	f--;
	if( vfs_file[ f ] )
	{
	    if( vfs_first_read[f] == 0 ) vfs2buffer( f ); //Fill buffer for first time
	}
	if( write_flag[f] ) return 0;
	if( get_record_rest(fpp) >= real_size )
	{ //we need for small block only:
	    MemMove(ptr,cur_rec_pnt[f]+cur_off[f],real_size);
	    fseek( fpp, real_size, 1 );
	    return real_size; //all is ok
	} 
	else
	{ //we need for large block (more than one record)
	    for(;;)
	    {
	        rest = get_record_rest(fpp);
	        if(real_size>=rest) real_size-=rest; else {rest=real_size;real_size=0;}
		if( rest != 0 ) MemMove( (uint8_t*)ptr + off, cur_rec_pnt[f] + cur_off[f], rest );
		off += rest;
		fseek( fpp, rest, 1 );
		if( cur_pnt[f] >= f_size[f] ) return off; //out of file space
		if( real_size == 0 ) return off; //fread is ok!
	    }
	}
    }
    return 0;
}

size_t fwrite( const void* ptr, size_t el_size, size_t elements, FILE* fpp )
{
    uint32_t real_size = el_size * elements;
    uint32_t size; //current block size
    uint8_t *cur;
    uint8_t res;
    uint32_t f=(uint32_t)fpp;
    uint32_t rest,off=0;
    if(f!=0)
    {
	f--;	
	if( write_flag[f] == 0 ) return 0;
	if( get_record_rest(fpp) >= real_size ) 
	{//we need to write small block only:
	    DmWrite( cur_rec_pnt[f], cur_off[f], ptr, real_size );
	    if( vfs_file[f] )
	    {
		cur_off[f] += real_size; cur_pnt[f] += real_size;
		if( cur_off[f] >= r_size[f][0] ) //if we must to save current buffer
		    buffer2vfs(f);
	    }
	    else fseek( fpp, real_size, 1 );
	    return real_size; //all is ok
	} 
	else
	{//we need to write large block (more than one record)
	    for(;;)
	    {
	        rest = get_record_rest(fpp);
	        if( real_size >= rest ) 
		    real_size-=rest; 
		else 
		{ rest=real_size; real_size=0; }
		if( rest != 0 ) DmWrite( cur_rec_pnt[f], cur_off[f], (uint8_t*)ptr + off, rest );
		off += rest;
		if( vfs_file[f] )
		{
		    cur_off[f] += rest; cur_pnt[f] += rest;
		    if( cur_off[f] >= r_size[f][0] ) //if we must to save current buffer
		        buffer2vfs(f);
		}
		else fseek( fpp, rest, 1 );
		if( real_size == 0 ) return off; //fwrite is ok!
	    }
	}
    }
    return 0;
}

int fputc( int val, FILE* fpp )
{
    uint8_t v = (uint8_t)val;
    if( fwrite( &v, 1, 1, fpp ) > 0 )
        return val;
    else
        return -1;
}

int remove( const char *filename )
{
    if( ( filename[0] == 'A' && filename[1] == ':' ) || filename[1] != ':' )
    { //It's PalmOS PDB:
        if( filename[0] == 'A' && filename[1] == ':' )
	    filename += 3; //Skip "A:/"
	ID = DmFindDatabase( 0, (const char*)filename );
	if( ID != 0 ) DmDeleteDatabase( 0, ID ); //File exist: delete it
	return 0;
    }
    else
    { //It's VFS:
        get_disks();
        if( filename[0] - 65 >= disks ) return 1; //no such disk
        int disk_num = filename[0] - 66;
        filename += 2; //Skip "X:"
        if( open_vfs() ) {
	    //Delete file:
	    mem_palm_normal_mode(); //#####
	    VFSFileDelete( vfs_volume_numbers[ disk_num ], filename );
	    mem_palm_our_mode();    //#####
	    return 0;
	}
    }
    return 1;
}

#endif

uint32_t file_size( const char* filename )
{
    uint32_t retval = 0;
#ifndef NONPALM
    //PalmOS:
    FILE *f = fopen( filename, "rb" );
    if( f )
    {
	uint32_t ff = f;
	if( ff != 0 )
	{
	    ff--;
	    retval = f_size[ ff ];
	    fclose( f );
	}
    }
#else
    FILE *f = fopen( filename, "rb" );
    if( f )
    {
	fseek( f, 0, 2 );
	retval = ftell( f );
	fclose( f );
    }
#endif
    return retval;
}


//FIND FILES FUNCTIONS:

#ifndef NONPALM
#define strlen(str) StrLen(str)
#endif

int check_file( char* our_file, find_struct *fs )
{
    int p;
    int mp; //mask pointer
    int equal = 0;
    int str_len;
    
    if(fs->mask) mp = strlen(fs->mask) - 1; else return 1;
    str_len = strlen(our_file);
    p = str_len - 1;
    
    for( ; p>=0 ; p--, mp-- ) {
	if( our_file[p] == '.' ) {
	    if(equal) return 1; //It is our file!
	    else {
		for(;;) { //Is there other file types (in mask[]) ?:
		    if( fs->mask[mp] == '/' ) break; //There is other type
		    mp--;
		    if( mp < 0 ) return 0; //no... it was the last type in mask[]
		}
	    }
	}
	if( mp < 0 ) return 0;
	if( fs->mask[mp] == '/' ) { mp--; p = str_len - 1; }
	char c = our_file[ p ];
	if( c >= 65 && c <= 90 ) c += 0x20; //Make small leters
	if( c != fs->mask[mp] ) 
	{
	    for(;;) { //Is there other file types (in mask[]) ?:
	        if( fs->mask[mp] == '/' ) { p = str_len; mp++; break; } //There is other type
	        mp--;
	        if( mp < 0 ) return 0; //no... it was the last type in mask[]
	    }
	}
	else equal = 1;
    }
    
    return 0;
}

#ifndef NONPALM
//For PalmOS:
int find_first( find_struct *fs )
{
    Err error;
    if( fs->start_dir[0] >= 'B' ) { //Flash-card search:
	if( open_vfs() ) {
	    mem_palm_normal_mode(); //#####
	    error = VFSFileOpen( vfs_volume_numbers[ fs->start_dir[0] - 66 ], fs->start_dir + 2, vfsModeRead, &fs->dir_ref );
	    mem_palm_our_mode();    //#####
	    if( error != errNone ) 
	    {
		return 0; //Dir not found
	    }
	    fs->dir_iterator = vfsIteratorStart;
	    //File_info init:
	    fs->file_info.nameP = fs->name; 
	    fs->file_info.nameBufLen = MAX_FILE_LENGTH;
	    mem_palm_normal_mode(); //#####
	    error = VFSDirEntryEnumerate( fs->dir_ref, (UInt32*)&fs->dir_iterator, &fs->file_info );
	    mem_palm_our_mode();    //#####
	    if( error != errNone ) {
		mem_palm_normal_mode(); //#####
		VFSFileClose( fs->dir_ref );
		mem_palm_our_mode();    //#####
		return 0;
	    }
	    if( fs->file_info.attributes & vfsFileAttrDirectory ) fs->type = TYPE_DIR;
	    else { //If it is a file:
		fs->type = TYPE_FILE;
		if( !check_file( fs->name, fs ) ) return find_next(fs);
	    }
	} else return 0;
    } else { //PDB  search:
	if( DmGetNextDatabaseByTypeCreator( 1, &fs->search_info, 'PSYX', 'ZULU', 0, &fs->card_id, &fs->db_id ) == dmErrCantFind ) 
	    return 0; //no files
	DmDatabaseInfo( fs->card_id, fs->db_id, fs->name, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ); //Get database name
	fs->type = TYPE_FILE;
	if( !check_file( fs->name, fs ) ) return find_next(fs);
    }
    return 1;
}

int find_next( find_struct *fs )
{
    Err error;
    if( fs->start_dir[0] >= 'B' ) { //Flash-card search:
	for(;;) {
	    mem_palm_normal_mode(); //#####
	    error = VFSDirEntryEnumerate( fs->dir_ref, (UInt32*)&fs->dir_iterator, &fs->file_info );
	    mem_palm_our_mode();    //#####
	    if( error != errNone ) {
		mem_palm_normal_mode(); //#####
		VFSFileClose( fs->dir_ref );
		mem_palm_our_mode();    //#####
		return 0; //no more files
	    }
	    if( fs->file_info.attributes & vfsFileAttrDirectory ) { fs->type = TYPE_DIR; return 1; } //Dir founded
	    else { //If it is a file:
		fs->type = TYPE_FILE;
		if( check_file( fs->name, fs ) ) return 1;
	    }
	}
    } else { //PDB  search:
	for(;;) {
	    if( DmGetNextDatabaseByTypeCreator( 0, &fs->search_info, 'PSYX', 'ZULU', 0, &fs->card_id, &fs->db_id ) == dmErrCantFind ) 
		return 0; //no more files
	    DmDatabaseInfo( fs->card_id, fs->db_id, fs->name, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ); //Get database name
	    fs->type = TYPE_FILE;
	    if( check_file( fs->name, fs ) ) return 1;
	}
    }
    return 1;
}

void find_close( find_struct *fs )
{
}
#endif

#ifdef OS_WIN
//For win32:
int find_first( find_struct *fs )
{
    int wp = 0, p = 0;
    fs->win_mask[0] = 0;
    fs->win_start_dir[0] = 0;
    
    //convert start dir from "dir/dir/" to "dir\dir\*.*"
    strcat( fs->win_start_dir, fs->start_dir );
    strcat( fs->win_start_dir, "*.*" );
    for(wp = 0;;wp++) {
	if( fs->win_start_dir[wp] == 0 ) break;
	if( fs->win_start_dir[wp] == '/' ) fs->win_start_dir[wp] = 92;
    }
    
    //do it:
    fs->find_handle = FindFirstFile( fs->win_start_dir, &fs->find_data );
    if( fs->find_handle == INVALID_HANDLE_VALUE ) return 0; //no files found :(  So Alex, you may go to sleep...
    
    //save filename:
    fs->name[0] = 0;
    strcat( fs->name, fs->find_data.cFileName );
    if(fs->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) fs->type = TYPE_DIR; 
    else { //If it is a file:
	fs->type = TYPE_FILE;
	if( !check_file( fs->name, fs ) ) return find_next(fs);
    }
    
    return 1;
}

int find_next( find_struct *fs )
{
    for(;;) {
	if( !FindNextFile( fs->find_handle, &fs->find_data ) ) return 0; //files not found

	//save filename:
        fs->name[0] = 0;
	strcat( fs->name, fs->find_data.cFileName );
	if(fs->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { fs->type = TYPE_DIR; return 1; } //Dir founded
	else { //File founded. Is it our file?
	    fs->type = TYPE_FILE;
	    if( check_file( fs->name, fs ) ) return 1;
	}
    }

    return 1;
}

void find_close( find_struct *fs )
{
    FindClose( fs->find_handle );
}
#endif


#ifdef OS_UNIX
//For unix:

int find_first( find_struct *fs )
{
    DIR *test;
    char test_dir[MAX_DIR_LENGTH];
    
    //convert start dir to unix standard:
    fs->new_start_dir[0] = 0;
    if(fs->start_dir[0] == 0) 
	strcat(fs->new_start_dir, "./"); 
    else 
	strcat(fs->new_start_dir, fs->start_dir);
    
    //open dir and read first entry:
    fs->dir = opendir( fs->new_start_dir );
    if(fs->dir == 0) return 0; //no such dir :(
    fs->current_file = readdir( fs->dir );
    if( !fs->current_file ) return 0; //no files
    
    //copy file name:
    fs->name[0] = 0;
    strcpy( fs->name, fs->current_file->d_name );
    
    //is it dir?
    test_dir[0] = 0;
    strcat(test_dir, fs->new_start_dir);
    strcat(test_dir, fs->current_file->d_name );
    test = opendir( test_dir );
    if(test) fs->type = TYPE_DIR; else fs->type = TYPE_FILE;
    closedir( test );
    if(fs->current_file->d_name[0]=='.') fs->type = TYPE_DIR;
    
    if( fs->type == TYPE_FILE )
	if( !check_file( fs->name, fs ) ) return find_next( fs );
    return 1;
}

int find_next( find_struct *fs )
{
    DIR *test;
    char test_dir[MAX_DIR_LENGTH];
    
    for(;;) {
	//read next entry:
	fs->current_file = readdir( fs->dir );
	if( !fs->current_file ) return 0; //no files
    
	//copy file name:
	fs->name[0] = 0;
	strcpy( fs->name, fs->current_file->d_name );
    
	//is it dir?
	test_dir[0] = 0;
	strcat(test_dir, fs->new_start_dir);
	strcat(test_dir, fs->current_file->d_name );
	test = opendir( test_dir );
	if(test) fs->type = TYPE_DIR; else fs->type = TYPE_FILE;
	closedir( test );
	if(fs->current_file->d_name[0]=='.') fs->type = TYPE_DIR;
	
	if( fs->type == TYPE_FILE )
	{
	    if( check_file( fs->name, fs ) ) return 1;
	}
	else return 1; //Dir founded
    }
}

void find_close( find_struct *fs )
{
    closedir( fs->dir );
}
#endif

void save_string( char *str, uint8_t num, const char *filename )
{
    mem_off();
    
    int b = 0; 
    for(;;) { current_filename[b]=filename[b]; if( filename[b++] == 0 ) break; }
    current_filename[ b - 2 ] = num + 0x61;
    
    FILE *f = fopen( current_filename, "wb" );
    if( f )
    {
	uint16_t a[2];
	a[0] = 44; //It's string file
	a[1] = 0;
	for(;;)
	{
	    if( str[ a[1]++ ] == 0 ) break;
	}
	fwrite( &a, 4, 1, f );
	fwrite( str, a[1], 1, f );
	fclose( f );
    }
    
    mem_on();
}

int load_string( char *str, uint8_t num, const char *filename )
{
    int retval = 0;
    
    int b = 0; 
    for(;;) { current_filename[b]=filename[b]; if( filename[b++] == 0 ) break; }
    current_filename[ b - 2 ] = num + 0x61;

    FILE *f = fopen( current_filename, "rb" );
    if( f )
    {
	uint16_t a[2];
	fread( &a, 4, 1, f );
	if( a[ 0 ] == 44 ) 
	{
	    fread( str, a[1], 1, f );
	    str[ a[1] ] = 0;
	}
	fclose( f );
    }
    else
    {
	retval = -1;
    }
    return retval;
}

void save_int( int num, const char *filename ) //causes the sound to stop for a short time on PalmOS :(
{
    mem_off();
    
    FILE *f = fopen( filename, "wb" );
    if( f )
    {
	fwrite( &num, 4, 1, f );
	fclose( f );
    }
    
    mem_on();
}

int load_int( const char *filename )
{
    int retval = -1;
    FILE *f = fopen( filename, "rb" );
    if( f )
    {
	fread( &retval, 4, 1, f );
	fclose( f );
    }
    return retval;
}

int option_values[ OPT_LAST ];

void read_file_with_options( const char *filename )
{
    char option_name[ 129 ];
    int option = -1;
    for( int o = 0; o < OPT_LAST; o++ ) option_values[ o ] = -1; //Clear options
    FILE *f = fopen( filename, "rb" );
    if( f )
    {
	char c;
	char comment_mode = 0;
	int a;
	for(;;)
	{
	    c = getc( f );
	    if( c == -1 ) break; //Last char in the file
	    if( c == 0xD || c == 0xA ) comment_mode = 0; //Reset comment mode at the line end
	    if( comment_mode == 0 )
	    {
		if( c == '@' ) break; //Last char in the file
		if( c == '/' ) comment_mode = 1; //Comments
		if( (c > 0x40 && c < 0x5B) || (c > 0x60 && c < 0x7B) )
		{
		    //Get option name:
		    for( a = 0; a < 128; a++ )
		    {
			option_name[ a ] = c;
			c = getc( f );
			if( c == ' ' || c == 0x09 ) { option_name[ a + 1 ] = 0; break; }
		    }
		    option_name[ 127 ] = 0;
		    //Compare it:
		    if( mem_strcmp( "width", option_name ) == 0 ) option = OPT_SCREENX;
		    if( mem_strcmp( "height", option_name ) == 0 ) option = OPT_SCREENY;
		    if( mem_strcmp( "flip", option_name ) == 0 ) option = OPT_SCREENFLIP;
		    if( mem_strcmp( "buffer", option_name ) == 0 ) option = OPT_SOUNDBUFFER;
		}
		if( c >= 0x30 && c <= 0x39 ) 
		{
		    //Get option value:
		    int value = c - 0x30;
		    for(;;)
		    {
			c = getc( f );
			if( c >= 0x30 && c <= 0x39 ) { value *= 10; value += c - 0x30; } else break;
		    }
		    //Save option value:
		    if( option >= 0 ) option_values[ option ] = value;
		}
	    }
	}
	fclose( f );
    }
}

int get_option( int option )
{
    return option_values[ option ];
}
