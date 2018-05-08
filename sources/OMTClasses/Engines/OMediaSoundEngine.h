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
#ifndef OMEDIA_SoundEngine_H
#define OMEDIA_SoundEngine_H

#include "OMediaSoundChannel.h"
#include "OMediaEngine.h"

// Sound attributes

typedef unsigned short omt_SoundEngineAttr;
const omt_SoundEngineAttr omseaf_NoChannelLimitation	= (1<<0); 	// Unlimited number of channels (realtime mixing)
const omt_SoundEngineAttr omseaf_Volume					= (1<<1);	// Volume can be changed
const omt_SoundEngineAttr omseaf_Frequence 				= (1<<2);	// Frequence can be changed


class OMediaSoundEngine : public OMediaEngine
{
	public:

	omtshared OMediaSoundEngine(omt_EngineID id, OMediaWindow *master_win);
	omtshared virtual ~OMediaSoundEngine();

	// * Hardware buffer

	omtshared virtual void set_buffer_format(OMediaSound *sound);
	omtshared virtual void compact_buffer(void);

	// * Channels

	omtshared virtual long get_max_channels(void); // Return the maximum number of channels supported
										 // by the current manager
	
	omtshared virtual long set_nchannels(long	n);	// Set the number of channels reserved by the manager.
										// Return the same number or less if it was not
										// possible to allocate enough channels. 

	omtshared virtual long get_nchannels(void);	// Return the number of allocated channels .

	inline OMediaSoundChannel *get_channel(long c) {return channels[c];}


	// * Sound
	
	omtshared virtual bool play(long channel, 
									OMediaSound *sound, 
									bool queue_if_busy, 
									bool loop);



	protected:	

	omtshared virtual OMediaSoundChannel	*create_sound_channel(void);

	omtshared virtual void delete_all_channels(void);


	vector<OMediaSoundChannel*>	channels;
};



#endif

