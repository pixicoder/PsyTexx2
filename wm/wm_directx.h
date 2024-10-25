/*
    wm_directx.h - platform-dependent module : DirectDraw (Win32)
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#pragma once

#include "ddraw.h"

LPDIRECTDRAW lpDD; // DirectDraw object
LPDIRECTDRAWSURFACE lpDDSPrimary; // DirectDraw primary surface
LPDIRECTDRAWSURFACE lpDDSBack; // DirectDraw back surface
LPDIRECTDRAWSURFACE lpDDSOne;
LPDIRECTDRAWCLIPPER lpClipper;
HRESULT ddrval;

int dd_init(void)
{
    DDSURFACEDESC ddsd;
    DDSCAPS ddscaps;
    HRESULT ddrval;

	ddrval = DirectDrawCreate( NULL, &lpDD, NULL );
	if( ddrval != DD_OK ) 
	{
		MessageBox( hWnd,"DirectDrawCreate error","PsyTexx Error",MB_OK);
        return 1;
	}
	ddrval = lpDD->SetCooperativeLevel( hWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN );
	if( ddrval != DD_OK ) 
	{
		MessageBox( hWnd,"SetCooperativeLevel error","PsyTexx Error",MB_OK);
        return 1;
	}
	ddrval = lpDD->SetDisplayMode( pscreen_x_size, pscreen_y_size, COLORBITS ); 
	if( ddrval != DD_OK ) 
	{ 
		MessageBox( hWnd,"SetDisplayMode error","PsyTexx Error",MB_OK);
        return 1;
	}

    // Create the primary surface with 1 back buffer
    memset( &ddsd, 0, sizeof(ddsd) );
    ddsd.dwSize = sizeof( ddsd );
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
	ddsd.dwBackBufferCount = 1;

	ddrval = lpDD->CreateSurface(&ddsd, &lpDDSPrimary, NULL); 
	if( ddrval != DD_OK ) 
	{ 
		switch( ddrval )
		{
			case DDERR_UNSUPPORTEDMODE:
			MessageBox( hWnd,"CreateSurface DDERR_UNSUPPORTEDMODE","PsyTexx Error",MB_OK);
			break;
			case DDERR_PRIMARYSURFACEALREADYEXISTS:
			MessageBox( hWnd,"CreateSurface DDERR_PRIMARYSURFACEALREADYEXISTS ","PsyTexx Error",MB_OK);
			break;
			case DDERR_OUTOFVIDEOMEMORY:
			MessageBox( hWnd,"CreateSurface DDERR_OUTOFVIDEOMEMORY  ","PsyTexx Error",MB_OK);
			break;
			case DDERR_OUTOFMEMORY:
			MessageBox( hWnd,"CreateSurface DDERR_OUTOFMEMORY","PsyTexx Error",MB_OK);
			break;
			case DDERR_NOZBUFFERHW:
			MessageBox( hWnd,"CreateSurface DDERR_NOZBUFFERHW","PsyTexx Error",MB_OK);
			break;
			case DDERR_NOOVERLAYHW:
			MessageBox( hWnd,"CreateSurface DDERR_NOOVERLAYHW","PsyTexx Error",MB_OK);
			break;
			case DDERR_NOMIPMAPHW:
			MessageBox( hWnd,"CreateSurface DDERR_NOMIPMAPHW","PsyTexx Error",MB_OK);
			break;
			case DDERR_NOEXCLUSIVEMODE:
			MessageBox( hWnd,"CreateSurface DDERR_NOEXCLUSIVEMODE","PsyTexx Error",MB_OK);
			break;
			case DDERR_NOEMULATION:
			MessageBox( hWnd,"CreateSurface DDERR_NOEMULATION","PsyTexx Error",MB_OK);
			break;
			case DDERR_NODIRECTDRAWHW:
			MessageBox( hWnd,"CreateSurface DDERR_NODIRECTDRAWHW","PsyTexx Error",MB_OK);
			break;
			case DDERR_NOCOOPERATIVELEVELSET:
			MessageBox( hWnd,"CreateSurface DDERR_NOCOOPERATIVELEVELSET","PsyTexx Error",MB_OK);
			break;
			case DDERR_NOALPHAHW:
			MessageBox( hWnd,"CreateSurface DDERR_NOALPHAHW","PsyTexx Error",MB_OK);
			break;
			case DDERR_INVALIDPIXELFORMAT:
			MessageBox( hWnd,"CreateSurface DDERR_INVALIDPIXELFORMAT","PsyTexx Error",MB_OK);
			break;
			case DDERR_INVALIDPARAMS:
			MessageBox( hWnd,"CreateSurface DDERR_INVALIDPARAMS","PsyTexx Error",MB_OK);
			break;
			case DDERR_INVALIDOBJECT:
			MessageBox( hWnd,"CreateSurface DDERR_INVALIDOBJECT","PsyTexx Error",MB_OK);
			break;
			case DDERR_INVALIDCAPS:
			MessageBox( hWnd,"CreateSurface DDERR_INVALIDCAPS","PsyTexx Error",MB_OK);
			break;
			case DDERR_INCOMPATIBLEPRIMARY:
			MessageBox( hWnd,"CreateSurface DDERR_INCOMPATIBLEPRIMARY","PsyTexx Error",MB_OK);
			break;
		}
        return 1;
	} 

	ddscaps.dwCaps = DDSCAPS_BACKBUFFER; 
	ddrval = lpDDSPrimary->GetAttachedSurface(&ddscaps, &lpDDSBack); 
	if( ddrval != DD_OK )
	{ 
		MessageBox( hWnd,"GetAttachedSurface error","PsyTexx Error",MB_OK);
        return 1;
	}

	current_wm->lpDDSBack = lpDDSBack;
	current_wm->lpDDSPrimary = lpDDSPrimary;

	return 0;
}

int dd_close()
{
    if(lpDD != NULL) 
    { 
        if(lpDDSPrimary != NULL) 
        { 
            lpDDSPrimary->Release(); 
            lpDDSPrimary = NULL; 
        } 
        lpDD->Release(); 
        lpDD = NULL; 
    }
	return 0;
}
