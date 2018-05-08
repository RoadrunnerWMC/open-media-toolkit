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
 

#include "OMediaMacSoundChannel.h"
#include "OMediaSound.h"
#include "OMediaError.h"
#include "OMediaMacRtgSound.h"
#include "OMediaFixedPoint.h"

pascal void omf_SoundCallBack(SndChannelPtr chan, SndCommand *cmd);
static SndCallBackUPP	sndcallbackupp;
static bool				snduppready;


OMediaMacSoundChannel::OMediaMacSoundChannel()
{
	last_played = NULL;
	current_total_time = 0;
	current_looped = false;	
	frequence = 0;
	volume = 255;

	OSErr err;
	
	if (!snduppready) 
	{
		snduppready = true;
		sndcallbackupp = NewSndCallBackUPP(omf_SoundCallBack);
	}
	
	channel = NULL;
	err = SndNewChannel(&channel,sampledSynth,initMono,sndcallbackupp);
	if (err) omd_EXCEPTION(omcerr_CantOpen);
	
	channel->userInfo = (long)this;
	A5 = SetCurrentA5();
	rateexp = 0.0;
}

OMediaMacSoundChannel::~OMediaMacSoundChannel()
{
	stop();

	if (channel) SndDisposeChannel(channel,true);
}

bool OMediaMacSoundChannel::play(OMediaSound *sound, bool queue_if_busy, bool loop)
{
	OMediaMacRtgSound 		 *snd_rtg = (OMediaMacRtgSound*) sound->get_retarget();
	SndCommand 				 sndcmd;
	OSErr					err;
	SCStatus				stat;

	err = SndChannelStatus(channel,sizeof(SCStatus),&stat);
	if (err!=noErr) omd_OSEXCEPTION(err);

	if (stat.scChannelBusy)
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

	set_volume(volume);

	if (!frequence) *snd_rtg->mac_rate_ptr = snd_rtg->mac_sample_rate;
	else *snd_rtg->mac_rate_ptr = frequence<<16L;

	last_played = sound;

	sndcmd.cmd = bufferCmd;
	sndcmd.param1 = 0;
	sndcmd.param2 = (long)snd_rtg->header;
	err = SndDoCommand(channel,&sndcmd,false);				
	if (err!=noErr) omd_OSEXCEPTION(err);

	prepare_rate((*snd_rtg->mac_rate_ptr)>>16L);

	current_looped = loop;	
	current_total_time = long((last_played->get_sound_secs()*1000.0)+0.5);
	timer.start();

	if (loop)
	{
		sndcmd.cmd = callBackCmd;
		sndcmd.param1 = 0;	
		sndcmd.param1 = (long)snd_rtg->header;
		err = SndDoCommand(channel,&sndcmd,false);	
		if (err!=noErr) omd_OSEXCEPTION(err);
	}

	return true;
}

void OMediaMacSoundChannel::set_volume(short v)
{
	SndCommand 				 sndcmd;
	OSErr					err;
        unsigned long				lvolume;
        
        lvolume = ((unsigned long)v * 0x100UL) / 0xFFUL;
        lvolume |= lvolume<<16UL;

	volume = v;

	sndcmd.cmd = volumeCmd;
	sndcmd.param1 = 0;
	sndcmd.param2 = lvolume;
	err = SndDoImmediate(channel,&sndcmd);
	if (err!=noErr) omd_OSEXCEPTION(err);
}

void OMediaMacSoundChannel::set_frequency(unsigned long f)
{
	SndCommand 				 sndcmd;
	OSErr					err;

	frequence = f;

	if (frequence && rateexp!=0.0)
	{
		sndcmd.cmd = rateMultiplierCmd;
		sndcmd.param1 = 0;
		sndcmd.param2 = omd_FloatToFixed16_16(float(frequence)/rateexp);
		err = SndDoImmediate(channel,&sndcmd);				
		if (err!=noErr) omd_OSEXCEPTION(err);
	}
}

void OMediaMacSoundChannel::prepare_rate(long sndfreq)
{	
	rateexp = float(sndfreq);
}


void OMediaMacSoundChannel::stop(void)
{
	OSErr					 err;
	SCStatus				 stat;
	SndCommand 				 sndcmd;

	current_looped = false;

	sndcmd.cmd = flushCmd;
	sndcmd.param1 = 0;
	sndcmd.param2 = 0;
	err = SndDoImmediate(channel,&sndcmd);

	err = SndChannelStatus(channel,sizeof(SCStatus),&stat);
	if (err!=noErr) omd_OSEXCEPTION(err);

	if (stat.scChannelBusy)
	{
		sndcmd.cmd = quietCmd;
		sndcmd.param1 = 0;
		sndcmd.param2 = 0;
		err = SndDoImmediate(channel,&sndcmd);
		if (err!=noErr) omd_OSEXCEPTION(err);
	}
	
	current_total_time = 0;
}

void OMediaMacSoundChannel::spend_time(void)
{
	if (current_looped) return;
	if (sound_queue.size())
	{
		OMediaMacRtgSound 		 *snd_rtg = (OMediaMacRtgSound*) ((*(sound_queue.begin())).sound)->get_retarget();
		OSErr					 err;
		SCStatus				 stat;
		SndCommand 				 sndcmd;

		err = SndChannelStatus(channel,sizeof(SCStatus),&stat);
		if (err!=noErr) omd_OSEXCEPTION(err);

		if (!stat.scChannelBusy)
		{
			set_volume(volume);

			if (!frequence) *snd_rtg->mac_rate_ptr = snd_rtg->mac_sample_rate;
			else *snd_rtg->mac_rate_ptr = frequence<<16L;

			last_played = (*sound_queue.begin()).sound;

			sndcmd.cmd = bufferCmd;
			sndcmd.param1 = 0;
			sndcmd.param2 = (long)snd_rtg->header;
			err = SndDoCommand(channel,&sndcmd,false);				
			if (err!=noErr) omd_OSEXCEPTION(err);

			prepare_rate((*snd_rtg->mac_rate_ptr)>>16L);

			current_looped = (*sound_queue.begin()).loop;				
			(*sound_queue.begin()).sound->dec_wait_count();
			
			sound_queue.erase(sound_queue.begin());

			current_total_time = long((last_played->get_sound_secs()*1000.0)+0.5);
			timer.start();

			if (current_looped)
			{
				sndcmd.cmd = callBackCmd;
				sndcmd.param1 = 0;	
				sndcmd.param1 = (long)snd_rtg->header;
				err = SndDoCommand(channel,&sndcmd,false);	
				if (err!=noErr) omd_OSEXCEPTION(err);
			}
		}
	}
}


#pragma profile off

void OMediaMacSoundChannel::process_loop(void)
{
	OMediaMacRtgSound 		 *snd_rtg = (OMediaMacRtgSound*) last_played->get_retarget();

	if (!current_looped) return;

	SndCommand 				 	sndcmd;

	if (!frequence) *snd_rtg->mac_rate_ptr = snd_rtg->mac_sample_rate;
	else *snd_rtg->mac_rate_ptr = frequence<<16L;

	sndcmd.cmd = bufferCmd;
	sndcmd.param1 = 0;
	sndcmd.param2 = interruptcmd->param2;
	SndDoCommand(channel,&sndcmd,false);	

	sndcmd.cmd = callBackCmd;
	sndcmd.param1 = 0;	
	sndcmd.param1 = interruptcmd->param2;
	SndDoCommand(channel,&sndcmd,false);

}


pascal void omf_SoundCallBack(SndChannelPtr macchan, SndCommand *cmd)
{
	OMediaMacSoundChannel		*chan;	
	long						cura5;
	
	chan = (OMediaMacSoundChannel*)macchan->userInfo;
	cura5 = SetA5(chan->A5);

	chan->interruptcmd = cmd;
	chan->process_loop();

	SetA5(cura5);
}

#pragma profile reset



