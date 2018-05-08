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
 
 

#include "OMediaSoundEngine.h"
#include "OMediaError.h"
#include "OMediaSound.h"

OMediaSoundEngine::OMediaSoundEngine(omt_EngineID id, OMediaWindow *master_win) : 
					OMediaEngine(id,master_win)
{
}

OMediaSoundEngine::~OMediaSoundEngine()
{
	delete_all_channels();
}

void OMediaSoundEngine::set_buffer_format(OMediaSound *sound)
{
}

long OMediaSoundEngine::get_max_channels(void)
{
	return 0;
}

void OMediaSoundEngine::delete_all_channels(void)
{
	vector<OMediaSoundChannel*>::iterator	ic;

	for(ic=channels.begin();
		ic!=channels.end();
		ic++)
	{
		delete *(ic);
	}

	channels.erase(channels.begin(),channels.end());
}

void OMediaSoundEngine::compact_buffer(void)
{
}

long OMediaSoundEngine::set_nchannels(long	n)
{
	long max_chan = get_max_channels();
	vector<OMediaSoundChannel*>::iterator	ic;

	long		cursize = channels.size();
	
	if (n==cursize) return n;
	else if (n>cursize)
	{
		long	ntoadd = n-cursize;
		while(ntoadd--)
		{
			if (max_chan!=-1 && (long)channels.size()>=max_chan) break;
		
			OMediaSoundChannel *new_c = create_sound_channel();
			if (!new_c)  omd_EXCEPTION(omcerr_OutOfMemory);
			channels.push_back(new_c);		
		}	
	}
	else
	{
		long	ntorem = cursize-n;
		while(ntorem--)
		{
			ic = channels.end(); ic--;
			delete *(ic);
			channels.erase(ic);		
		}	
	}	
	
	return channels.size();
} 

OMediaSoundChannel	*OMediaSoundEngine::create_sound_channel(void)
{
	return new OMediaSoundChannel;
}


long OMediaSoundEngine::get_nchannels(void)
{
	return channels.size();
}
	
bool OMediaSoundEngine::play(long channel, OMediaSound *sound, 
									bool queue_if_busy, bool loop)
{
	if (channel>=(long)channels.size()) return false;
	
	return channels[channel]->play(sound,queue_if_busy,loop);
}
