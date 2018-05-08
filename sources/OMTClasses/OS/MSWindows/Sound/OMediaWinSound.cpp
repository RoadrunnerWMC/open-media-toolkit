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
#include "OMediaWinRtgSound.h"
#include "OMediaEndianSupport.h"

void OMediaSound::init_retarget()
{
	retarget = new OMediaWinRtgSound;
}
void OMediaSound::read_class(OMediaStreamOperators &stream)
{
	unsigned long 				 l, length,type;
	OMediaWinRtgSound	 *rtg = (OMediaWinRtgSound*)retarget;
	char								*buffer,*snddata;
	unsigned long				 *ptr;
	OMediaWavFormatEx	*wavfmt;
	long								data_length;

	purge();
	
	OMediaDBObject::read_class(stream);
	
	stream>>l;
	if(l!='RIFF') omd_EXCEPTION(omcerr_BadFormat);

	stream>>length;

	length = omd_ReverseLong(length);

	stream>>l;
	if(l!='WAVE') omd_EXCEPTION(omcerr_BadFormat);

	length += 8;
	
	stream.setposition(-12,omcfr_Current);
	
	rtg->sound = new char[length];
	if (!rtg->sound) omd_EXCEPTION(omcerr_OutOfMemory);
	rtg->sound_size = length;
	stream.read(rtg->sound, length);

	length -=8;
	buffer = rtg->sound;
	buffer += 8;
	data_length = -1;
	wavfmt = omc_NULL;

	ptr = (unsigned long*)buffer;
	while(length)
	{
		type = *ptr++;
		length -=4;
		type = omd_ReverseLong(type);

		switch(type)
		{
			case 'fmt ':
			ptr++;
			wavfmt = (OMediaWavFormatEx*)ptr;
			break;
		
			case 'data':
			data_length = *ptr++;
			snddata = (char*)ptr;
			break;
		}
		
		if (wavfmt && data_length!=-1) break;
	}

	if (wavfmt==omc_NULL || data_length==-1) omd_EXCEPTION(omcerr_BadFormat);

	sound_secs =  calc_sound_secs(data_length,
							 (wavfmt->samples_per_sec),
							 (wavfmt->nchannels),
							 (wavfmt->bits_per_sample));

	rtg->wavfmt = wavfmt;
	rtg->snddata = snddata;
	rtg->data_length = data_length;
}

void OMediaSound::write_class(OMediaStreamOperators &stream)
{
	OMediaWinRtgSound	*rtg = (OMediaWinRtgSound*)retarget;

	OMediaDBObject::write_class(stream);

	if (rtg->sound)
	{
		stream.write(rtg->sound,rtg->sound_size);
	}
}

unsigned long OMediaSound::get_approximate_size(void)
{
	unsigned long 		l;
	OMediaWinRtgSound	*rtg = (OMediaWinRtgSound*)retarget;

	l = sizeof(*this) + sizeof(OMediaWinRtgSound);
	if (rtg->sound) l += rtg->sound_size;
	
	return l;
}

void OMediaSound::purge()
{
	OMediaWinRtgSound	*rtg = (OMediaWinRtgSound*)retarget;

	delete_imp_slaves();

	delete rtg->sound;
	rtg->sound = omc_NULL;	
	rtg->sound_size = 0;
	rtg->wavfmt = omc_NULL;
	rtg->snddata = omc_NULL;
	rtg->data_length = 0;
		
	sound_secs = 0;
}

