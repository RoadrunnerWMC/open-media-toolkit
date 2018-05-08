/*****************************************************************

        O P E N      M E D I A     T O O L K I T              V2.5    
 
        Copyright Yves Schmid 1996-2003
 
        See www.garagecube.com for more informations about this library.
        
        Author(s): Yves Schmid
 
        OMT is provided under LGPL:
 
          This library is free software; you can redistribute it and/or
          modify it under the terms of the GNU Lesser General Public
          License as published by the Free Software Foundation; either
          version 2.1 of the License, or (at your option) any later version.

          This library is distributed in the hope that it will be useful,
          but WITHOUT ANY WARRANTY; without even the implied warranty of
          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
          Lesser General Public License for more details.

          You should have received a copy of the GNU Lesser General Public
          License along with this library; if not, write to the Free Software
          Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

          The full text of the license can be found in lgpl.txt          

******************************************************************/


#pragma once
#ifndef OMEDIA_SysDefs_H
#define OMEDIA_SysDefs_H

#include "OMediaBuildSwitch.h"

 
#ifdef _WINDOWS

	#define omd_INTEL
	#define omd_LITTLE_ENDIAN
	#define omd_WINDOWS

#pragma warning( disable : 4786 )  

	
#else
 
	#ifdef __INTEL__

		#define omd_INTEL
		#define omd_LITTLE_ENDIAN
		#define omd_WINDOWS

	#else

		#ifdef WIN32

			#define omd_INTEL
			#define omd_LITTLE_ENDIAN
			#define omd_WINDOWS

		#else

			#define omd_MACOS
			#define omd_BIG_ENDIAN

			#ifdef powerc
				#define omd_POWERPC
			#else
				#define omd_68K
			#endif

		#endif

	#endif
	
#endif

#ifdef __BIG_ENDIAN__
    #ifdef omd_LITTLE_ENDIAN
    #undef omd_LITTLE_ENDIAN
    #endif
    #ifndef omd_BIG_ENDIAN
    #define omd_BIG_ENDIAN
    #endif
#endif

#ifdef __LITTLE_ENDIAN__ 
    #ifdef omd_BIG_ENDIAN
    #undef omd_BIG_ENDIAN
    #endif
    #ifndef omd_LITTLE_ENDIAN
    #define omd_LITTLE_ENDIAN
    #endif
#endif

#ifdef omd_WINDOWS
#include <windows.h>
#ifdef omd_ENABLE_OPENGL
#include <gl/gl.h>
#endif
#endif

#ifdef omd_MACOS
#ifdef __MWERKS__

	#ifdef omd_ENABLE_OPENGL
	#include "agl.h"
	#include "gl.h"
	#endif

#ifdef omd_ENABLE_SPROCKET
	#include <InputSprocket.h>
	#include <DrawSprocket.h>	
#endif

	#include <quicktime.h>

#else

	#include <Carbon/Carbon.h>
	#include <Quicktime/Quicktime.h>

	#ifdef omd_ENABLE_OPENGL
	#include <agl/agl.h>
	#include <agl/gl.h>
	#endif

#endif

#endif
#endif

