#pragma once

#define PSYTEXX_VERSION "PsyTexx2 alpha 0.6"
#define PSYTEXX_DATE    __DATE__

/*
Engine configuration (possible defines)
  COLOR8BITS  		//8 bit color
  COLOR16BITS 		//16 bit color
  COLOR32BITS 		//32 bit color
  OS_UNIX               //OS = some UNIX variation (Linux, FreeBSD...); POSIX-compatible
  OS_LINUX     		//OS = Linux
  OS_WIN       		//OS = Windows
  PALMOS      		//OS = PalmOS
  PALMOS_COMP_MODE 	//DEPRECATED
  NOSTORAGE   		//Do not use the Storage Heap and MemSemaphores
  PALMLOWRES  		//PalmOS low-density screen
  TEXTMODE    		//Linux Text Mode
  X11         		//X11 support
  DIRECTX     		//DirectDraw support
  OPENGL      		//OpenGL support
  GDI         		//GDI support
  SLOWMODE    		//Slow mode for slow devices

Variations
  WIN32:  COLOR32BITS + OS_WIN [ + OPENGL / GDI ]
  LINUX:  COLOR32BITS + OS_LINUX [ + TEXTMODE / OPENGL / X11 ]
  PALMOS: COLOR8BITS + PALMOS + SLOWMODE [ + NOSTORAGE / PALMLOWRES ]
*/

#ifdef OS_LINUX
    #define OS_UNIX
#endif

#ifndef PALMOS
    #include <stdbool.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    #include <string.h>
    #define NONPALM
#else
    #include <stdarg.h>
    #include "palm_functions.h"
#endif

typedef unsigned int uint;
