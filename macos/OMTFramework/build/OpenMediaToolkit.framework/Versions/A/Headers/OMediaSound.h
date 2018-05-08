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
#ifndef OMEDIA_Sound_H
#define OMEDIA_Sound_H

#include "OMediaTypes.h"
#include "OMediaRetarget.h"
#include "OMediaDBObject.h"
#include "OMediaSoundEngine.h"
#include "OMediaEngineImplementation.h"


class OMediaSound : public OMediaDBObject, 
					public OMediaEngineImpMaster
{
	public:
	
	// * Constructor/Destructor

	omtshared OMediaSound();
	omtshared virtual ~OMediaSound();	

	omtshared virtual void purge();

	inline OMediaRetarget *get_retarget(void) {return retarget;}

	// * Play

	inline bool play(OMediaSoundEngine *engine, long channel, bool queue_if_busy, bool loop) 
						  {return engine->play(channel,this,queue_if_busy,loop);}


	// * Database/streamer support
	
	omtshared enum { db_type = 'Wave' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void) const;

	// * Waiting counter
	
	inline long get_wait_count(void) const {return wait_count;}
	inline void inc_wait_count(void) {wait_count++;}
	inline void dec_wait_count(void) {wait_count--;}


	// * Sound length in seconds
	
	inline float get_sound_secs(void) const {return sound_secs;}

	protected:
	
	inline float calc_sound_secs(long nbytes,
						   long hz,
						   long nchannels,
						   long nbits) {return (float(nbytes)/float(hz)) / (float(nchannels)*(float(nbits)/8));}
	
	OMediaRetarget				*retarget;
	long						wait_count;
	float						sound_secs;
		
	omtshared virtual void init_retarget();
};


#endif

