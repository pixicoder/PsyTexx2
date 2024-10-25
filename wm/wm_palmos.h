/*
    wm_palmos.h - platform-dependent module : PalmOS
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#pragma once

#include <PalmOS.h>
#include <PenInputMgr.h>
#include "palm_functions.h"

//#################################
//## DEVICE DEPENDENT FUNCTIONS: ##
//#################################

void wait( void )
{
    int cur_ticks = time_ticks();
    int counter = 0;
    while( time_ticks() < cur_ticks + ( time_ticks_per_second() / 2 ) )
    {
	device_event_handler( 0 );
	counter++;
	if( ( counter & 0xFFFF ) == 0 ) slog( "MAIN: waiting...\n" );
    }
}

FormPtr gpForm;
int formIsOpen = 0;
int formResized = 0;

void MainFormResize( FormPtr frmP )
{
    RectangleType dispBounds, bounds;
    
    // Get/Set the window bounds
    WinGetBounds( WinGetDisplayWindow(), &dispBounds );
    WinSetWindowBounds( FrmGetWindowHandle(frmP), &dispBounds );
}

Boolean FormHandler( EventPtr event )
{
    RectangleType bounds;
    UInt32 pinMgrVersion;
    Err err;
    Boolean handled = false;

    return handled;
}

Boolean ApplicationHandleEvent( EventPtr event )
{
    FormPtr frm;
    UInt16 formId;
    Boolean handled = false;
    FormPtr pForm;

    switch( BSwap16(event->eType) )
    {
	case frmLoadEvent:
	    unsigned short *ptr = (unsigned short*)event;
	    pForm = FrmInitForm( BSwap16(ptr[4]) );//BSwap16(event->data.frmLoad.formID) );

	    FrmSetActiveForm( pForm );

	    FrmSetEventHandler( pForm, (Boolean (*)(EventType*))g_form_handler );

	    handled = true;
	    break;
    }
    return handled;
}

void device_start( window_manager *wm )
{
    //Open window:
#ifndef PALMOS_COMP_MODE
    slog( "MAIN: open form\n" );
    FrmGotoForm( 8888 );
    wait();
    /*
    UInt32 pinMgrVersion;
    slog( "MAIN: FtrGet\n" );
    Err err = FtrGet( pinCreator, pinFtrAPIVersion, &pinMgrVersion );
    slog( "MAIN: waiting for form open/resize\n" );
    if ( !err && pinMgrVersion != pinAPIVersion1_0 )
    { //Palm with large screen:
	while( !formResized ) device_event_handler( 0 );
    }
    else
    { //Palm like TungstenT (320x320 or 160x160):
	while( !formIsOpen ) device_event_handler( 0 );
    }
    RectangleType bounds;
    //slog( "MAIN: get form ptr\n" );
    FormType *fp;// = FrmGetFormPtr( 8888 );
    for(;;) 
    { 
        device_event_handler( 0 ); fp = FrmGetFormPtr( 8888 );
        if( fp )
        {
	    FrmGetFormBounds( fp, &bounds );
	    if( bounds.extent.x != 32 || bounds.extent.y != 32 ) break;
	}
    }
    slog( "MAIN: fp = %d\n", (int)fp );
    slog( "MAIN: get form bounds\n" );
    FrmGetFormBounds( fp, &bounds );
    */

    slog( "MAIN: get form size\n" );
#ifdef PALMLOWRES
    pscreen_x_size = 160;//bounds.extent.x;
    pscreen_y_size = 160;//bounds.extent.y;
#else
    pscreen_x_size = 320;//bounds.extent.x << 1;
    pscreen_y_size = 320;//bounds.extent.y << 1;
#endif

#else //PALMOS_COMP_MODE (No forms):

#ifdef PALMLOWRES
    pscreen_x_size = 160;
    pscreen_y_size = 160;
#else
    pscreen_x_size = 320;
    pscreen_y_size = 320;
#endif

#endif
    slog( "MAIN: device start finished\n" );
}

void device_end( window_manager *wm )
{
    //Close window:
#ifndef PALMOS_COMP_MODE
    slog( "MAIN: close all forms\n" );
    FrmCloseAllForms();
    wait();
#else
    slog( "MAIN: device end\n" );
#endif
}

int old_chrr;

int device_event_handler( window_manager *wm )
{
    EventType event;
    UInt16 error;
    int chrr,x,y;
    char keydown = 0;

    EvtGetEvent( &event, 0 );

    if( wm )
    {    
    UInt16 *ptr = (UInt16*)&event;
    chrr = BSwap16( ptr[4] );
    x = BSwap16( ptr[2] );
    y = BSwap16( ptr[3] );
    switch( BSwap16(event.eType) ) 
    {
#ifdef PALMLOWRES
	case penDownEvent: push_button( x, y, 1, 1023, 0, wm ); break;
	case penMoveEvent: push_button( x, y, 1, 1023, 2, wm ); break;
	case penUpEvent:   push_button( x, y, 1, 1023, 1, wm ); break;
#else
	case penDownEvent: push_button( x<<1, y<<1, 1, 1023, 0, wm ); break;
	case penMoveEvent: push_button( x<<1, y<<1, 1, 1023, 2, wm ); break;
	case penUpEvent:   push_button( x<<1, y<<1, 1, 1023, 1, wm ); break;
#endif
	case keyDownEvent:
	    if( chrr >= 11 && chrr <= 12 ) keydown = 1;
	    if( chrr >= 516 && chrr <= 519 ) keydown = 1;
	    uint resulted_key;
	    switch( chrr )
	    {
		case 0x1E: resulted_key = KEY_UP; break;
		case 0x1F: resulted_key = KEY_DOWN; break;
		case 0x1C: resulted_key = KEY_LEFT; break;
		case 0x1D: resulted_key = KEY_RIGHT; break;
		case 0x08: resulted_key = KEY_BACKSPACE; break;
		case 0x0A: resulted_key = KEY_ENTER; break;
		case 0x0B: resulted_key = KEY_UP; break;
		case 0x0C: resulted_key = KEY_DOWN; break;
		default: resulted_key = chrr; if( resulted_key > 255 ) resulted_key = 0; break;
	    }
	    push_button( 0, 0, resulted_key << 3, 1023, 0, wm );
	    break;
	/*case keyUpEvent:
	    chrr = event.data.keyDown.chr;
	    push_button( 0, 0, wm->buttons_table[ chrr ] << 3, 1023, 1, wm );
	    break;*/
    }
    
    chrr = KeyCurrentState();
    if( !keydown )
    {
	if( chrr & 0x1000000 && !( old_chrr & 0x1000000 ) ) { push_button( 0, 0, KEY_LEFT << 3, 1023, 0, wm ); keydown = 1; }
	if( !( chrr & 0x1000000 ) && old_chrr & 0x1000000 ) { push_button( 0, 0, KEY_LEFT << 3, 1023, 1, wm ); }
	if( chrr & 0x2000000 && !( old_chrr & 0x2000000 ) ) { push_button( 0, 0, KEY_RIGHT << 3, 1023, 0, wm ); keydown = 1; }
	if( !( chrr & 0x2000000 ) && old_chrr & 0x2000000 ) { push_button( 0, 0, KEY_RIGHT << 3, 1023, 1, wm ); }
	if( chrr & 0x4000000 && !( old_chrr & 0x4000000 ) ) { push_button( 0, 0, KEY_SPACE << 3, 1023, 0, wm ); keydown = 1; }
	if( !( chrr & 0x4000000 ) && old_chrr & 0x4000000 ) { push_button( 0, 0, KEY_SPACE << 3, 1023, 1, wm ); }
    }
    old_chrr = chrr;

    } //if( wm )

    if( !keydown )
    {
#ifndef PALMOS_COMP_MODE
	if( !SysHandleEvent(&event) && !MenuHandleEvent(0,&event,&error) && !ApplicationHandleEvent(&event) )
	    FrmDispatchEvent(&event);
#else
	if( !SysHandleEvent( &event ) ) MenuHandleEvent( 0, &event,&error );
#endif
    }

#ifndef PALMOS_COMP_MODE
    if( BSwap16( event.eType ) == winDisplayChangedEvent ) formResized = 1;
    if( BSwap16( event.eType ) == frmOpenEvent ) formIsOpen = 1; 
    if( BSwap16( event.eType ) == frmCloseEvent ) formIsOpen = 0;
    if( BSwap16( event.eType ) == winExitEvent ) formIsOpen = 0;
#endif
    if( BSwap16( event.eType ) == appStopEvent ) 
    {
	if( wm ) push_button( pscreen_x_size / 2, pscreen_y_size - 40, KEY_ESCAPE<<3, 1023, 0, wm );
    }

    if( wm ) if( wm->exit_flag ) return 1;

    return 0;
}

//#################################
//#################################
//#################################
