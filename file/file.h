#pragma once

#include "../core/core.h"

#ifdef OS_WIN
    #include "windows.h"
#endif

#ifdef OS_UNIX
    #include "dirent.h" //for file find
    #include "unistd.h" //for current dir getting 
#endif

#define MAX_DISKS 16
#define DISKNAME_SIZE 4
#define MAX_DIR_LENGTH 512
#define MAX_FILE_LENGTH 256
extern int disks;                    //number of disks
extern char disk_names[4*MAX_DISKS];  //disk names
extern char current_dir[MAX_DIR_LENGTH];
void get_disks(void);                 //get info about local disks
char* get_disk_name( int number );   //get name of local disk
int get_current_disk(void);          //get number of the current disk
char* get_current_dir(void);          //get current dir (without disk. example: "mydir/")

#ifdef PALMOS

#include "VFSMgr.h"
//PalmOS functions:
#define MAX_RECORDS 256
#define MAX_F_POINTERS 2
extern uint32_t r_size[MAX_F_POINTERS][MAX_RECORDS]; //size of each record in file
extern uint32_t f_size[MAX_F_POINTERS]; //size of each file (for each file pointer)
extern uint32_t cur_rec[MAX_F_POINTERS]; //current record number (for each file pointer)
extern int cur_off[MAX_F_POINTERS]; //current record offset (for each file pointer)
extern uint32_t cur_pnt[MAX_F_POINTERS]; //current file offset (for each file pointer)
extern uint32_t recs[MAX_F_POINTERS]; //number of records (for each file pointer)
extern DmOpenRef cur_db[MAX_F_POINTERS]; //current DB pointer
extern LocalID ID; //local ID for DB
extern uint8_t fp[MAX_F_POINTERS]; //for each file pointer: 0-free 1-working
extern uint8_t *cur_rec_pnt[MAX_F_POINTERS];
extern MemHandle cur_rec_h[MAX_F_POINTERS];
extern FileRef vfs_file[ MAX_F_POINTERS ];
extern uint8_t write_flag[ MAX_F_POINTERS ];
extern uint16_t vfs_volume_numbers[8];
extern uint16_t vfs_volumes;

int open_vfs(void); //Return 1 if successful. vfs_volume_number = number of first finded VFS volume (flash-card)
int isBusy( DmOpenRef db, int index );  //Is record busy?
void recordsBusyReset( int f );
void get_records_info( DmOpenRef db, uint16_t f_num );
void vfs2buffer( uint32_t f );
void buffer2vfs( uint32_t f );
int next_record( uint32_t f );
uint32_t get_record_rest( FILE* fpp );

#endif

uint32_t file_size( const char *filename );

//FIND FILE FUNCTIONS:

//type in find_struct:
enum {
    TYPE_FILE = 0,
    TYPE_DIR
};

struct find_struct
{ //structure for file searching functions
    const char *start_dir; //Example: "c:/mydir/" "d:/"
    const char *mask;      //Example: "xm/mod/it" (or NULL for all files)
    
    char name[MAX_FILE_LENGTH];  //Finded file's name
    char type;                   //Finded file's type: 0 - file; 1 - dir

#ifndef NONPALM
    uint16_t card_id;
    LocalID db_id;
    DmSearchStateType search_info;
    FileRef dir_ref;        //VFS: reference to the start_dir
    uint32_t dir_iterator;     //VFS: dir iterator
    FileInfoType file_info; //VFS: file info
#endif
#ifdef OS_WIN
    WIN32_FIND_DATA find_data;
    HANDLE find_handle;
    char win_mask[MAX_FILE_LENGTH];      //Example: "*.xm *.mod *.it"
    char win_start_dir[MAX_DIR_LENGTH];  //Example: "mydir\*.xm"
#endif
#ifdef OS_UNIX
    DIR *dir;
    struct dirent *current_file;
    char new_start_dir[MAX_DIR_LENGTH]; 
#endif
};

int find_first( find_struct* );  //Return values: 0 - no files
int find_next( find_struct* );   //Return values: 0 - no files
void find_close( find_struct* );

void save_string( char *str, uint8_t num, const char *filename ); //save null-terminated string to the file
int load_string( char *str, uint8_t num, const char *filename ); //load null-terminated string from file (or -1 if no such file)
void save_int( int num, const char *filename );
int load_int( const char *filename ); // -1 if no such file

enum
{
    OPT_SCREENX = 0,
    OPT_SCREENY,
    OPT_SCREENFLIP,
    OPT_SOUNDBUFFER,
    OPT_LAST
};

void read_file_with_options( const char *filename );
int get_option( int option ); //Return -1 if no such option
