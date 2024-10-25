/*
    SunDog ARM ELF starter for PalmOS
    by Alexander Zolotov / nightradio@gmail.com / WarmPlace.ru
    This file is public domain
*/

//WARNING !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//This version can't execute ARM-code with functions, declared as "static"
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <PalmOS.h>
#include <PenInputMgr.h>
#include <VFSMgr.h>
#include "PceNativeCall.h"

#define ByteSwap16(n) (	((((unsigned long) n) <<8 ) & 0xFF00) | \
			((((unsigned long) n) >>8 ) & 0x00FF) )

#define ByteSwap32(n) (	((((unsigned long) n) << 24 ) & 0xFF000000) | \
			((((unsigned long) n) << 8 ) & 0x00FF0000) | \
			((((unsigned long) n) >> 8 ) & 0x0000FF00) | \
			((((unsigned long) n) >> 24 ) & 0x000000FF) )

struct ARM_INFO
{
    void* GLOBALS;
    void* GOT;
    void* FORMHANDLER;
    long ID;
    long* new_screen_size;
};

long g_new_screen_size[ 4 ] = { 0, 0, 0, 0 };

FormPtr gpForm;

Boolean FormHandler( EventPtr event )
{
    Err err;
    UInt32 pinMgrVersion;
    RectangleType bounds;
    Boolean handled = false;
    switch( event->eType )
    {
	case frmOpenEvent:
	    gpForm = FrmGetActiveForm();

	    err = FtrGet( pinCreator, pinFtrAPIVersion, &pinMgrVersion );
	    if ( !err && pinMgrVersion != pinAPIVersion1_0 )
	    {
		FrmSetDIAPolicyAttr( gpForm, frmDIAPolicyCustom );
		PINSetInputTriggerState( pinInputTriggerEnabled );
		//PINSetInputAreaState( 2 ); //pinInputAreaHide
		//StatHide(); //Hide control panel
	    }
	    else
	    { //No pinAPI: (TungstenT for example)
	    }

	    WinGetBounds( WinGetDisplayWindow(), &bounds );
	    g_new_screen_size[ 0 ] = ByteSwap32( (long)bounds.topLeft.x );
	    g_new_screen_size[ 1 ] = ByteSwap32( (long)bounds.topLeft.y );
	    g_new_screen_size[ 2 ] = ByteSwap32( (long)bounds.extent.x );
	    g_new_screen_size[ 3 ] = ByteSwap32( (long)bounds.extent.y );

	    FrmDrawForm( gpForm );
	    
	    handled = true;
	    
	    break;
	
	case winDisplayChangedEvent:
	    WinGetBounds( WinGetDisplayWindow(), &bounds );
	    WinSetBounds( WinGetActiveWindow(), &bounds );
	    g_new_screen_size[ 0 ] = ByteSwap32( (long)bounds.topLeft.x );
	    g_new_screen_size[ 1 ] = ByteSwap32( (long)bounds.topLeft.y );
	    g_new_screen_size[ 2 ] = ByteSwap32( (long)bounds.extent.x );
	    g_new_screen_size[ 3 ] = ByteSwap32( (long)bounds.extent.y );
	    handled = true;
	    break;
	
	case frmCloseEvent:
    	    FrmEraseForm( gpForm );
	    FrmDeleteForm( gpForm );
    	    gpForm = 0;
    	    handled = true;
    	    break;
    }
    return handled;
}

#define memNewChunkFlagNonMovable    0x0200
#define memNewChunkFlagAllowLarge    0x1000
SysAppInfoPtr ai1, ai2, appInfo;
SysAppInfoPtr SysGetAppInfo( SysAppInfoPtr* rootAppPP, SysAppInfoPtr* actionCodeAppPP ) SYS_TRAP( sysTrapSysUIBusy );

void* bmem_new_ext( unsigned long size )
{
    unsigned short ownID = appInfo->memOwnerID;
    return MemChunkNew( MemHeapID( 0, 0 ), size, ownID | memNewChunkFlagNonMovable | memNewChunkFlagAllowLarge );
}

void bmem_free( void* ptr )
{
    MemPtrFree( ptr );
}

void start_arm_code( void )
{
    ARM_INFO arm_info;
    unsigned long* text_section; //Main ARM code
    unsigned long* section2; 	 //Data + Rel + Got
    unsigned long* got_section;  //Global offset table: offset = rel[ got[ var number ] ];
    unsigned char* data_section; //Binary data of the ELF
    unsigned long* rel_section;  //.rel section

    MemHandle mem_handle_r;
    MemHandle mem_handle_d[ 16 ];
    unsigned long mem_handle_d_size[ 16 ];
    MemHandle mem_handle_g;
    MemHandle mem_handle_c[ 16 ];
    unsigned long arm_size[ 16 ];
    unsigned long xtra;
    MemHandle mem_handle;

    // GET REL SECTION:
    mem_handle_r = DmGetResource( 'armr', 0 );
    long rel_size = MemHandleSize( mem_handle_r );
    long rxtra = 4 - (rel_size & 3); if( rxtra == 4 ) rxtra = 0;

    // GET DATA SECTIONS:
    unsigned long data_size = 0;
    int dr;
    for( dr = 0; dr < 16; dr++ )
    {
	mem_handle_d[ dr ] = DmGetResource( 'armd', dr );
	if( mem_handle_d[ dr ] )
	{
	    mem_handle_d_size[ dr ] = MemHandleSize( mem_handle_d[ dr ] );
	}
	else 
	{
	    mem_handle_d_size[ dr ] = 0;
	}
	data_size += mem_handle_d_size[ dr ];
    }
    unsigned long dxtra = 4 - (data_size & 3); if( dxtra == 4 ) dxtra = 0;

    // GET GOT SECTION:
    mem_handle_g = DmGetResource( 'armg', 0 );
    unsigned long got_size = MemHandleSize( mem_handle_g );
    unsigned long gxtra = 4 - (got_size & 3); if( gxtra == 4 ) gxtra = 0;

    // GET CODE SECTIONS:
    long code_size = 0;
    int cr;
    for( cr = 0; cr < 16; cr++ )
    {
	mem_handle_c[ cr ] = DmGetResource( 'armc', cr );
	if( mem_handle_c[ cr ] )
	{
	    arm_size[ cr ] = MemHandleSize( mem_handle_c[ cr ] );
	}
	else 
	{
	    arm_size[ cr ] = 0;
	}
	code_size += arm_size[ cr ];
    }
    xtra = 4 - ( code_size & 3 ); if( xtra == 4 ) xtra = 0;

    // CREATE ELF BUFFER:
    text_section = (unsigned long*)bmem_new_ext( code_size );
    section2 = (unsigned long*)bmem_new_ext( 
	data_size + dxtra + 
	rel_size + rxtra + 
	got_size + gxtra
    );

    // COPY ALL SECTIONS TO THE BUFFER:
    long offset = 0;
    for( cr = 0; cr < 16; cr++ )
    {
	if( arm_size[ cr ] )
	{
	    MemMove( text_section + offset, MemHandleLock( mem_handle_c[ cr ] ), arm_size[ cr ] );
	    offset += arm_size[ cr ] / 4;
	}
    }
    offset = 0;
    for( dr = 0; dr < 16; dr++ )
    {
	if( mem_handle_d_size[ dr ] )
	{
	    MemMove( section2 + offset, MemHandleLock( mem_handle_d[ dr ] ), mem_handle_d_size[ dr ] );
	    offset += mem_handle_d_size[ dr ] / 4;
	}
    }
    MemMove( section2+((data_size+dxtra)/4), MemHandleLock( mem_handle_r ), rel_size + rxtra );
    MemMove( section2+((data_size+dxtra)/4)+((rel_size+rxtra)/4), MemHandleLock( mem_handle_g ), got_size + gxtra );

    // CLOSE RESOURCES:
    MemHandleUnlock( mem_handle_r );
    MemHandleUnlock( mem_handle_g );
    for( cr = 0; cr < 16; cr++ )
    {
	if( mem_handle_c[ cr ] )
	{
	    MemHandleUnlock( mem_handle_c[ cr ] );
	    DmReleaseResource( mem_handle_c[ cr ] );
	}
	if( mem_handle_d[ cr ] )
	{
	    MemHandleUnlock( mem_handle_d[ cr ] );
	    DmReleaseResource( mem_handle_d[ cr ] );
	}
    }
    DmReleaseResource( mem_handle_r );
    DmReleaseResource( mem_handle_g );

    // FIXING GOT and RELOCATION REFERENCES :
    rel_section = section2+((data_size+dxtra)/4);
    got_section = section2+((data_size+dxtra)/4)+((rel_size+rxtra)/4);
    unsigned long a, res;
    for( a = 0; a < got_size / 4; a++ )
    {
	res = ByteSwap32( got_section[ a ] );
	if( res < code_size ) 
	{
	    res += (unsigned long)text_section;
	}
	else
	{
	    if( res >= code_size )
	    {
		res -= code_size;
		res += (unsigned long)section2;
	    }
	}
	got_section[ a ] = ByteSwap32( res );
    }
    for( a = 0; a < rel_size / 4; a++ )
    {
	res = ByteSwap32( rel_section[ a ] );
	if( res != 0 )
	{
	    if( res < code_size ) 
	    {
		res += (unsigned long)text_section;
	    }
	    else
	    {
		if( res >= code_size )
		{
		    res -= code_size;
		    res += (unsigned long)section2;
		}
	    }
	    rel_section[ a ] = ByteSwap32( res );
	}
    }
    unsigned long* real_got = section2+((data_size+dxtra)/4)+((rel_size+rxtra)/4);

    // START ARM CODE :
    WinDrawChars( "SUNDOG ARM STARTER " __DATE__ "                        ", 32, 10, 0 );
    arm_info.GLOBALS = (void*)ByteSwap32( text_section );
    arm_info.GOT = (void*)ByteSwap32( real_got );
    arm_info.ID = (long)ByteSwap32( appInfo->memOwnerID );
    arm_info.FORMHANDLER = (void*)ByteSwap32( &FormHandler );
    arm_info.new_screen_size = (long*)ByteSwap32( &g_new_screen_size );
    res = PceNativeCall( (NativeFuncType*)text_section, &arm_info );

    // FREE DATA :
    bmem_free( text_section );
    bmem_free( section2 );
}

const char* g_filetypes[] =
{
    ".wav", "/AUDIO/",
    ".wave", "/AUDIO/",
    ".xi", "/AUDIO/",
    ".sunvox", "/MUSIC/",
    ".xm", "/MUSIC/",
    ".mod", "/MUSIC/",
    "~"
};

UInt32 PilotMain( UInt16 launchCode, void* cmdPBP, UInt16 launchFlags )
{
    if( launchCode == sysAppLaunchCmdNormalLaunch )
    {
    	appInfo = SysGetAppInfo( &ai1, &ai2 );

	int fp = 0;
	while( 1 )
	{
	    if( g_filetypes[ fp ][ 0 ] == '~' ) break;
	    VFSRegisterDefaultDirectory( g_filetypes[ fp ], expMediaType_Any, g_filetypes[ fp + 1 ] );
	    fp += 2;
	}

	start_arm_code();
    }
    return 0;
}
