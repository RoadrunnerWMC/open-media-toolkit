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
#ifndef OMEDIA_EngineFactory_H
#define OMEDIA_EngineFactory_H

#include "OMediaEngineID.h"
#include "OMediaVideoEngine.h"
#include "OMediaSoundEngine.h"
#include "OMediaInputEngine.h"

class OMediaWindow;

class OMediaEngineFactory
{
	public:

	omtshared OMediaEngineFactory();
	omtshared virtual ~OMediaEngineFactory();

	// * Get factory

	omtshared static OMediaEngineFactory *get_factory(void);

	// * Available engines
	
	omt_EngineList		video_engines;			// Video mode managers
	omt_EngineList		sound_engines;			// Sound
	omt_EngineList		input_engines;			// Input

	// * Engine attributes

	omtshared virtual omt_VideoEngineAttr get_video_engine_attr(omt_EngineID id);
	omtshared virtual omt_SoundEngineAttr get_sound_engine_attr(omt_EngineID id);
	omtshared virtual omt_InputEngineAttr get_input_engine_attr(omt_EngineID id);

	// * Create engine
	
	omtshared virtual OMediaVideoEngine *create_video_engine(omt_EngineID id, OMediaWindow *master_window);
	omtshared virtual OMediaSoundEngine *create_sound_engine(omt_EngineID id, OMediaWindow *master_window);
	omtshared virtual OMediaInputEngine *create_input_engine(omt_EngineID id, OMediaWindow *master_window);


	// * Patch library

	inline static void replace_factory(OMediaEngineFactory *e)
	{
		delete factory;
		factory = e;
	}
	
	protected:
	
	omtshared static OMediaEngineFactory *factory;
};



#endif

