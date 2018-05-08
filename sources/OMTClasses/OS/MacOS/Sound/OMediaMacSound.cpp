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
 

#include "OMediaSound.h"
#include "OMediaError.h"
#include "OMediaWavFormat.h"
#include "OMediaEndianSupport.h"
#include "OMediaMacRtgSound.h"
#include "OMediaMemTools.h"


void OMediaSound::init_retarget()
{
	retarget = new OMediaMacRtgSound;
}


void OMediaSound::read_class(OMediaStreamOperators &stream)
{
	unsigned long 		l, length,data_length,type;
	char				*buffer,*p;
	unsigned long		*ptr;
	OMediaWavFormatEx	*wavfmt;
	Handle				snd_handle;
	ExtSoundHeader		*machdr;
	OMediaMacRtgSound	*rtg = (OMediaMacRtgSound*)retarget;
	unsigned short		*data_ptr;

	purge();
	
	OMediaDBObject::read_class(stream);
	
	stream>>l;
	if(l!='RIFF') omd_EXCEPTION(omcerr_BadFormat);

	stream>>length;

	stream>>l;
	if(l!='WAVE') omd_EXCEPTION(omcerr_BadFormat);


	length = omd_ReverseLong(length);
	length-=4;

	buffer = new char[length];
	if (!buffer) omd_EXCEPTION(omcerr_OutOfMemory);

	stream.read(buffer,length);

	wavfmt = NULL;
	machdr = NULL;
	ptr = (unsigned long*)buffer;
	while(length)
	{
		type = *ptr++;
		length-=4;
	
		switch(type)
		{
			case 'fmt ':
			ptr++;
			wavfmt = (OMediaWavFormatEx*)ptr;			
			if (wavfmt && machdr) length = 0;
			OMediaMemTools::copy(wavfmt,&rtg->wavheader,16);
			break;
		
			case 'data':
			data_length = *ptr++;
			data_length = omd_ReverseLong(data_length);
			snd_handle = NewHandle(2+2+2+4+2 + 
								   sizeof(SndCommand) + 
								   sizeof(ExtSoundHeader) + 
								   data_length);
								   
			if (!snd_handle) omd_EXCEPTION(omcerr_OutOfMemory);
			
			HLock(snd_handle);
			p = (*snd_handle);
			*((unsigned short*)p) = 1;			p+=2;	// Format type
			*((unsigned short*)p) = 1;			p+=2;	// N synth
			*((unsigned short*)p) = 5;			p+=2;	// First synth ID resource
			*((unsigned long*)p) = initMono;	p+=4;	// Channel init
			*((unsigned short*)p) = 1;			p+=2;	// N sound commands
			*((unsigned short*)p) = bufferCmd;	p+=2;	// Command
			*((unsigned short*)p) = 0;			p+=2;	// param 1
			*((unsigned long*)p) = 20;			p+=4;	// header offset
			
			machdr = (ExtSoundHeader *) p;
			data_ptr = (unsigned short *) ptr;
			if (wavfmt && machdr) length = 0;
			break;
		}
	}
	
	if (!wavfmt || !machdr) omd_EXCEPTION(omcerr_BadFormat);

	machdr->samplePtr = NULL;
	machdr->sampleSize = omd_ReverseShort(wavfmt->bits_per_sample);
	machdr->numChannels = omd_ReverseShort(wavfmt->nchannels);
	machdr->sampleRate = (omd_ReverseLong(wavfmt->samples_per_sec))<<16L;
	machdr->loopStart = 0;		
	machdr->loopEnd = 0;	
	machdr->encode = extSH;	
	machdr->baseFrequency = 0;	
	*((double*)&(machdr->AIFFSampleRate)) = double(omd_ReverseLong(wavfmt->samples_per_sec));
	machdr->markerChunk = NULL;	
	machdr->instrumentChunks = NULL;
	machdr->AESRecording = NULL;
	machdr->numFrames = (data_length/(machdr->sampleSize/8))/machdr->numChannels;		
	 

	sound_secs =  calc_sound_secs(data_length,
							 omd_ReverseLong(wavfmt->samples_per_sec),
							 machdr->numChannels,
							 machdr->sampleSize);

	rtg->sample_area = machdr->sampleArea;
	rtg->sample_size = machdr->sampleSize;
	rtg->sample_length = data_length;
	rtg->sample_freq = machdr->sampleRate>>16L;
	rtg->mac_sample_rate = machdr->sampleRate;
	rtg->mac_rate_ptr = &machdr->sampleRate;

	if (machdr->sampleSize==8) OMediaMemTools::copy(data_ptr,machdr->sampleArea,data_length);
	else if (machdr->sampleSize==16)
	{
		unsigned short *s,*d;
		data_length >>=1;
		
		s = data_ptr;
		d = (unsigned short *) machdr->sampleArea;
		
		while(data_length--)
		{
			*d = omd_ReverseShort(*s);
			d++;
			s++;
		}
	}
	else omd_EXCEPTION(omcerr_BadFormat);

	delete buffer;
	
	rtg->sound = snd_handle;
	rtg->header = machdr;
}

void OMediaSound::write_class(OMediaStreamOperators &stream)
{
	OMediaMacRtgSound	*rtg = (OMediaMacRtgSound*)retarget;

	unsigned long	l;

	OMediaDBObject::write_class(stream);
	if (rtg->sound==NULL) return;

	*rtg->mac_rate_ptr = rtg->mac_sample_rate;
	
	l = 'RIFF';
	stream<<l;
	
	l = 4+4+4+16+4+4+rtg->sample_length;
	l = omd_ReverseLong(l);
	stream<<l;

	l = 'WAVE';
	stream<<l;	
	l = 'fmt ';
	stream<<l;

	l = 16;
	l = omd_ReverseLong(l);
	stream<<l;
		
	stream.write(&rtg->wavheader,16);
		
	l = 'data';
	stream<<l;
	l = rtg->sample_length;
	l = omd_ReverseLong(l);
	stream<<l;

	if (rtg->sample_size==8)
	{
		stream.write(rtg->sample_area,rtg->sample_length);
	}
	else if (rtg->sample_size==16)
	{
		unsigned short *buffer = new unsigned short[rtg->sample_length>>1L];			
		unsigned short *s,*d;
		long data_length = rtg->sample_length>>1L;
		
		s = (unsigned short *)rtg->sample_area;
		d = buffer;
		
		while(data_length--)
		{
			*d = omd_ReverseShort(*s);
			d++;
			s++;
		}
		
		stream.write(buffer,rtg->sample_length);
		
		delete [] buffer;
	}
}

unsigned long OMediaSound::get_approximate_size(void)
{
	unsigned long 		l;
	OMediaMacRtgSound	*rtg = (OMediaMacRtgSound*)retarget;

	l = sizeof(*this) + sizeof(OMediaMacRtgSound);
	if (rtg->sound) l += GetHandleSize(rtg->sound);
	
	return l;
}

void OMediaSound::purge()
{
	OMediaMacRtgSound	*rtg = (OMediaMacRtgSound*)retarget;

	if (rtg->sound)
	{
		HUnlock(rtg->sound);
		DisposeHandle(rtg->sound);
		rtg->sound = NULL;
		sound_secs = 0;
	}
}

