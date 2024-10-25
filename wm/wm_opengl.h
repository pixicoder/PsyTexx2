/*
    wm_opengl.h - platform-dependent module : OpenGL functions
    This file is part of the PsyTexx2.
    Copyright (C) 2002 - 2024 Alexander Zolotov <nightradio@gmail.com>
    WarmPlace.ru
*/

#pragma once

//#################################
//## DEVICE DEPENDENT FUNCTIONS: ##
//#################################

void gl_init( void )
{
    /* set viewing projection */
    glMatrixMode( GL_PROJECTION );
    glFrustum( -0.5F, 0.5F, -0.5F, 0.5F, 0.0F, 3.0F );

    /* position viewer */
    glMatrixMode( GL_MODELVIEW );
    //glTranslatef(0.0F, 0.0F, -2.2F);

    glClearDepth( 1.0f );
    glDepthFunc( GL_LESS );
    glEnable( GL_DEPTH_TEST );
    //glEnable( GL_LIGHTING );
    //glEnable( GL_LIGHT0 );

    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, 1 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

    glEnable( GL_COLOR_MATERIAL );
    glEnable( GL_BLEND );
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_LINE_SMOOTH_HINT );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glPixelTransferi( GL_INDEX_SHIFT, 0 );
    glPixelTransferi( GL_INDEX_OFFSET, 0 );
#ifdef COLOR8BITS
    glPixelTransferi( GL_MAP_COLOR, GL_TRUE );
#endif
}

void gl_resize( void )
{
    /* set viewport to cover the window */
    glViewport( 0, 0, winWidth, winHeight );
}

//#################################
//#################################
//#################################
