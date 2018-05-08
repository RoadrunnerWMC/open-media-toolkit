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
#ifndef OMEDIA_SoundChannel_H
#define OMEDIA_SoundChannel_H

#include "OMediaTypes.h"
#include "OMediaPeriodical.h"
#include "OMediaTimer.h"

#include <list>

class OMediaSound;

class OMediaQueuedSound
{
	public:
	OMediaQueuedSound() {}
	OMediaQueuedSound(OMediaSound *s, bool l) {sound = s; loop = l;}
	
	OMediaSound	*sound;
	bool	loop;
};

class OMediaSoundChannel : public OMediaPeriodical
{
	public:
	
	// * Constructor/Destructor

	omtshared OMediaSoundChannel();
	omtshared virtual ~OMediaSoundChannel();


	// * play

	omtshared virtual bool play(OMediaSound *sound, bool queue_if_busy, bool loop);

	omtshared virtual void stop(void);
	omtshared virtual void flush(OMediaSound *sound =NULL);


	omtshared virtual	void spend_time(void);

	inline OMediaSound *get_last_played(void) {return last_played;}
	inline void clear_last_played(void) {last_played = NULL;}


	omtshared virtual	void get_playing_sound_info(unsigned long &time_elapsed, unsigned long &total_time);
						// Returns 0,0 if no sound is playing (millisecs)

	inline bool get_current_looped(void) const {return current_looped;}

	omtshared virtual OMediaSound *get_playing_sound(void);

	
	// * Queue

	inline list<OMediaQueuedSound> *get_sound_queue(void) {return &sound_queue;}

	// * Set volume (0-255)
	
	virtual void set_volume(short v);
	inline short get_volume(void) const {return volume;}

	// * Set frequence (0 for default frequency)

	virtual void set_frequency(unsigned long f);
	inline unsigned long get_frequency(void) const {return frequence;}

	// * Low-level --------

	protected:
	
	list<OMediaQueuedSound>			sound_queue;
	OMediaSound						*last_played;
	unsigned long					current_total_time;
	OMediaTimer						timer;
	bool							current_looped;
	unsigned long					frequence;
	short							volume;
};


#endif

