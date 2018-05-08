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
 

#include "OMediaSoundChannel.h"
#include "OMediaSound.h"
#include "OMediaError.h"

OMediaSoundChannel::OMediaSoundChannel()
{
	last_played = NULL;
	current_total_time = 0;
	current_looped = false;	
	frequence = 0;
	volume = 255;
}

OMediaSoundChannel::~OMediaSoundChannel()
{
	stop();
}

bool OMediaSoundChannel::play(OMediaSound *sound, bool queue_if_busy, bool loop)
{
	return false;
}

void OMediaSoundChannel::set_volume(short v)
{
}

void OMediaSoundChannel::set_frequency(unsigned long f)
{
}

void OMediaSoundChannel::get_playing_sound_info(unsigned long &time_elapsed, unsigned long &total_time)
{
	bool		nosound = false;

	if (current_total_time==0) nosound = true;
	else
	{
		time_elapsed = timer.getelapsed();
		if (time_elapsed>current_total_time) nosound = true;
		total_time = current_total_time;
	}	
	
	if (nosound) time_elapsed = total_time = current_total_time = 0;
}

OMediaSound *OMediaSoundChannel::get_playing_sound(void)
{
	bool		nosound = false;
	unsigned long time_elapsed;

	if (current_total_time==0) nosound = true;
	else
	{
		time_elapsed = timer.getelapsed();
		if (time_elapsed>current_total_time) nosound = true;
	}	
	
	if (nosound) 
	{
		current_total_time = 0;
		return NULL;
	}

	return last_played;
}

void OMediaSoundChannel::stop(void)
{
}

void OMediaSoundChannel::flush(OMediaSound *sound)
{
	stop();

	if (!sound) sound_queue.erase(sound_queue.begin(), sound_queue.end());
	else
	{
		list<OMediaQueuedSound>::iterator i,ni;
		
		for(i=sound_queue.begin();
			i!=sound_queue.end();)
		{
			ni = i; ni++;
		
			if ((*i).sound==sound) sound_queue.erase(i);
		
			i = ni;
		}
	}
}

void OMediaSoundChannel::spend_time(void)
{
}


