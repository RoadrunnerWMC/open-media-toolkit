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
#ifndef OMEDIA_SPVideoEngine_H
#define OMEDIA_SPVideoEngine_H

#include "OMediaBuildSwitch.h"
#ifdef omd_ENABLE_DRAWSPROCKET

#include "OMediaMacVideoEngine.h"

//---------------------------------------------------
// * Video engine

class OMediaSPVideoEngine : public OMediaMacVideoEngine
{
	public:

	// * Constructor

	omtshared OMediaSPVideoEngine(OMediaWindow *master);
	omtshared virtual ~OMediaSPVideoEngine();


	// * Change mode

	omtshared virtual void set_mode(OMediaVideoMode *mode);	// Set video mode. Video engine must be
															// linked to the video card first.


	omtshared virtual void unlink(void);

	// * Listen to message
						
	omtshared virtual void listen_to_message(omt_Message msg, void *param =NULL);

	// * Draw sprocked
	
	static bool is_draw_sprocket_on() {return draw_sprocked_on;}

	// * Fullscreen retarget port
	
	virtual CGrafPtr get_full_screen_port(void);

	protected:

	omtshared virtual void enter_fullscreen(void);
	omtshared virtual void exit_fullscreen(void);
	
	static long draw_sprocked_on;
	DSpContextReference		context_ref;
	DSpContextAttributes	context_attr;

};


#endif
#endif

