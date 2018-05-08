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
 
#include "OMediaTypes.h"

#include "OMediaWinSoundChannel.h"
#include "OMediaSound.h"
#include "OMediaWinRtgSound.h"
#include "OMediaError.h"


OMediaWinSoundChannel::OMediaWinSoundChannel()
{
	last_played = omc_NULL;
	current_total_time = 0;
	current_looped = false;
	volume = 255;
	frequence = 0;
}

OMediaWinSoundChannel::~OMediaWinSoundChannel()
{
	stop();
}

bool OMediaWinSoundChannel::play(OMediaSound *sound, bool queue_if_busy,bool loop)
{
	OMediaWinRtgSound 		 *snd_rtg = (OMediaWinRtgSound*) sound->get_retarget();
	DWORD								flags;
	
	if (!snd_rtg->sound) return false;

	flags = SND_ASYNC|SND_MEMORY|SND_NODEFAULT|SND_NOSTOP|SND_NOWAIT;
	if (loop) flags |=SND_LOOP;
	
	if (!PlaySound((LPCSTR)snd_rtg->sound,
					omc_NULL, 
					flags))
	{
		if (queue_if_busy)
		{
			OMediaQueuedSound	qsound(sound, loop) ;

			sound->inc_wait_count();
			sound_queue.push_back(qsound);
			return true;
		}
		else return false;
	}

	last_played = sound;
	current_looped = loop;

	current_total_time = long((last_played->get_sound_secs()*1000.0)+0.5);
	timer.start();

	return true;
}


void OMediaWinSoundChannel::stop(void)
{
	PlaySound(omc_NULL, omc_NULL, SND_PURGE);
	current_total_time = 0;
	current_looped = false;
}


void OMediaWinSoundChannel::spend_time(void)
{
	if (sound_queue.size())
	{
		OMediaWinRtgSound 		 *snd_rtg = (OMediaWinRtgSound*) (*(sound_queue.begin())).sound->get_retarget();
		DWORD							flags = SND_ASYNC|SND_MEMORY|SND_NODEFAULT|SND_NOSTOP|SND_NOWAIT;

		if ((*(sound_queue.begin())).loop) flags |= SND_LOOP;

		if (PlaySound((LPCSTR)snd_rtg->sound,
					omc_NULL, 
					flags))
		{
			last_played = (*sound_queue.begin()).sound;
			current_looped = (*sound_queue.begin()).loop;

			(*sound_queue.begin()).sound->dec_wait_count();
			sound_queue.erase(sound_queue.begin());

			current_total_time = long((last_played->get_sound_secs()*1000.0)+0.5);
			timer.start();
		}
	}
}

void OMediaWinSoundChannel::set_volume(short v) {}
void OMediaWinSoundChannel::set_frequency(unsigned long f) {}

