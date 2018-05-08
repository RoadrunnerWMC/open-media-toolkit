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

#ifndef OMEDIA_DXSoundChannel_H
#define OMEDIA_DXSoundChannel_H

#include "OMediaSoundChannel.h"

#include <dsound.h>

class OMediaDXSoundEngine;

class OMediaDXSoundChannel : public OMediaSoundChannel
{
	
	public:
	
	// * Constructor/Destructor

	omtshared OMediaDXSoundChannel();
	omtshared virtual ~OMediaDXSoundChannel();	

	// * play

	omtshared bool play(OMediaSound *sound, bool queue_if_busy, bool loop);

	omtshared virtual void stop(void);

	omtshared virtual	void spend_time(void);

	// * Set volume (0-255)
	
	virtual void set_volume(short v);

	// * Set frequence (0 for default frequency)

	virtual void set_frequency(unsigned long f);


	OMediaDXSoundEngine		*its_engine;

	protected:
	
	LPDIRECTSOUNDBUFFER		dx_buffer;
};


#endif
#endif
