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

#include "OMediaBuildSwitch.h"
#ifdef omd_ENABLE_DIRECTSOUND
#ifndef OMEDIA_DXSoundEngine_H
#define OMEDIA_DXSoundEngine_H

#include "OMediaSoundEngine.h"

#include <dsound.h>

class OMediaDXSound;
class OMediaSound;

class OMediaDXSoundEngine : public OMediaSoundEngine
{	
	public:
	
	// * Constructor/Destructor

	omtshared OMediaDXSoundEngine(OMediaWindow *master_window);
	omtshared virtual ~OMediaDXSoundEngine();	

	// * Hardware buffer

	omtshared virtual void set_buffer_format(OMediaSound *sound);
	omtshared virtual void compact_buffer(void);

	// * Channels

	omtshared virtual long get_max_channels(void);  // Return the maximum number of channels supported
													// by the current manager
	
	// * DirectSound object

	inline LPDIRECTSOUND get_dxsound(void) {return dxsound;}


	// * Get dx implementation

	OMediaDXSound *get_dxsnd_implementation(OMediaSound *master);

	protected:	

	omtshared virtual OMediaSoundChannel	*create_sound_channel(void);
	

	LPDIRECTSOUND			dxsound;
	LPDIRECTSOUNDBUFFER		dx_primarybuffer;

};


#endif
#endif
