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

#include "OMediaBuildSwitch.h"
#ifdef omd_ENABLE_DIRECTSOUND

#include "OMediaDXSoundChannel.h"
#include "OMediaDXSound.h"
#include "OMediaDXSoundEngine.h"
#include "OMediaSound.h"
#include "OMediaError.h"
#include "OMediaString.h"

static inline long omf_CONVERT_VOLUME(long v)
{
	#define	min_Volume (-2700L)
	#define max_Volume (0L)

	if (v==0) return DSBVOLUME_MIN;
	else if (v==255) return DSBVOLUME_MAX;

	return min_Volume+long((float(max_Volume-min_Volume)/255.0)*float(v));
}

OMediaDXSoundChannel::OMediaDXSoundChannel()
{
	dx_buffer = NULL;
}

OMediaDXSoundChannel::~OMediaDXSoundChannel()
{
	stop();
	if (dx_buffer) 	dx_buffer->Release();
}

bool OMediaDXSoundChannel::play(OMediaSound *sound, 
								  bool queue_if_busy, 
								  bool loop)
{
	OMediaDXSound 					*snd = its_engine->get_dxsnd_implementation(sound);
	bool							chan_busy = false;
	HRESULT							herr;

	if (dx_buffer)
	{
		DWORD	status;
		 herr = dx_buffer->GetStatus(&status);
		if (herr!=DS_OK) omd_OSEXCEPTION(herr);
		if (status&DSBSTATUS_PLAYING) chan_busy=true;
		else
		{
			dx_buffer->Release();
			dx_buffer = NULL;
		}
	}

	if (chan_busy)
	{
		if (queue_if_busy)
		{
			OMediaQueuedSound	qsound(sound,loop);
			sound->inc_wait_count();
			sound_queue.push_back(qsound);
			return true;
		}
		else return false;
	}

	if (!snd->dx_soundbuffer) return false;

	LPDIRECTSOUND				dxsound = its_engine->get_dxsound();

	herr =  dxsound->DuplicateSoundBuffer(snd->dx_soundbuffer,&dx_buffer);
	if (herr!=DS_OK) omd_OSEXCEPTION(herr);

	dx_buffer->SetVolume(omf_CONVERT_VOLUME(volume));

	if (frequence) 
	{
		dx_buffer->SetFrequency(frequence);
	}

	herr = dx_buffer->Play(NULL,NULL,(loop)?DSBPLAY_LOOPING:0L);
	if (herr!=DS_OK) omd_OSEXCEPTION(herr);

	last_played = sound;
	current_looped = loop;	
	current_total_time = long((last_played->get_sound_secs()*1000.0)+0.5);
	timer.start();

	return true;
}

void OMediaDXSoundChannel::stop(void)
{
	if (dx_buffer)
	{
		HRESULT herr;
			
		herr = dx_buffer->Stop();
		if (herr!=DS_OK) omd_OSEXCEPTION(herr);

		dx_buffer->Release();
		dx_buffer = NULL;
	}

	current_looped = false;
	current_total_time = 0;
}

void OMediaDXSoundChannel::spend_time(void)
{
	if (current_looped) return;
	if (sound_queue.size())
	{
		bool							chan_busy = false;
		HRESULT							herr;

		if (dx_buffer)
		{
			DWORD	status;
			 herr = dx_buffer->GetStatus(&status);
			if (herr!=DS_OK) omd_OSEXCEPTION(herr);
			if (status&DSBSTATUS_PLAYING) chan_busy=true;
			else
			{
				dx_buffer->Release();
				dx_buffer = NULL;
			}
		}

		if (!chan_busy)
		{
			LPDIRECTSOUND		dxsound = its_engine->get_dxsound();
			OMediaDXSound 		 *snd;

			last_played = (*sound_queue.begin()).sound;

			current_looped = (*sound_queue.begin()).loop;				
			(*sound_queue.begin()).sound->dec_wait_count();
			
			sound_queue.erase(sound_queue.begin());

			snd = its_engine->get_dxsnd_implementation(last_played);

			herr =  dxsound->DuplicateSoundBuffer(snd->dx_soundbuffer,&dx_buffer);
			if (herr!=DS_OK) omd_OSEXCEPTION(herr);

			herr =  dx_buffer->SetVolume(omf_CONVERT_VOLUME(volume));

			if (frequence) 
			{
				herr =  dx_buffer->SetFrequency(frequence);
			}

			herr = dx_buffer->Play(NULL,NULL,(current_looped)?DSBPLAY_LOOPING:0L);
			if (herr!=DS_OK) omd_OSEXCEPTION(herr);

			current_total_time = long((last_played->get_sound_secs()*1000.0)+0.5);
			timer.start();
		}
	}
}

void OMediaDXSoundChannel::set_volume(short v)
{
	volume = v;
	if (dx_buffer)
	{
		dx_buffer->SetVolume(omf_CONVERT_VOLUME(volume));
	}
}



void OMediaDXSoundChannel::set_frequency(unsigned long f)
{
	frequence = f;
	if (dx_buffer)
	{
		if (frequence)
		{
			if (frequence<DSBFREQUENCY_MIN) 
			{	
				frequence = DSBFREQUENCY_MIN;
			}
			else if (frequence>DSBFREQUENCY_MAX) 
			{
				frequence = DSBFREQUENCY_MAX;			
			}
		}		
		else frequence = DSBFREQUENCY_ORIGINAL;
		
		dx_buffer->SetFrequency(frequence);
	}
}

#endif

