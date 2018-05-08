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
#ifndef OMEDIA_BuildSwitch_H
#define OMEDIA_BuildSwitch_H

// DirectX

#ifndef omd_DISABLE_DIRECTX

	#define omd_ENABLE_DIRECTX
	
	#ifdef omd_ENABLE_DIRECTX
	
		#define omd_ENABLE_DIRECT3D
	
		#define omd_ENABLE_DIRECTSOUND
		#define omd_ENABLE_DIRECTDRAW
		#define omd_ENABLE_DIRECTINPUT
		#define omd_ENABLE_DIRECTDRAW_EXCLUSIVE_MODE	
	#endif

#endif     
   
// Sprocket
// For now enable only the Sprockets on CodeWarrior
#ifdef __MWERKS__
#ifndef omd_DISABLE_SPROCKET

	#define omd_ENABLE_SPROCKET

	#ifdef omd_ENABLE_SPROCKET
	#define omd_ENABLE_INPUTSPROCKET		// Not available in Carbon at this time
	#define omd_ENABLE_DRAWSPROCKET
	#endif

#endif
#endif

// OpenGL

#ifndef omd_DISABLE_OPENGL
#define omd_ENABLE_OPENGL
#endif

// OMT Software renderer

//#define omd_DISABLE_OMTRENDERER		//+++ NO SOFTWARE RENDERER
//#define omd_DISABLE_OMTRASTERIZER		//+++ NO SOFTWARE RENDERER

#ifndef omd_DISABLE_OMTRENDERER
#define omd_ENABLE_OMTRENDERER			
#define omd_ENABLE_OMTRASTERIZER
#endif

#ifndef omd_DISABLE_OMTRASTERIZER
#ifndef omd_ENABLE_OMTRASTERIZER
#define omd_ENABLE_OMTRASTERIZER
#endif
#endif

#define omd_MAX_PIPELINE_LIGHTS	8			// Maximum light per object for the software renderer
                                                        // 8 by default (like OpenGL)
#endif

#ifdef omd_ENABLE_SPROCKET
#define CALL_IN_SPOCKETS_BUT_NOT_IN_CARBON 1
#endif



