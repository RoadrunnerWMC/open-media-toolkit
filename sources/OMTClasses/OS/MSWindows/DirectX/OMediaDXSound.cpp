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

#include "OMediaDXSound.h"
#include "OMediaDXSoundEngine.h"
#include "OMediaMemTools.h"
#include "OMediaWinRtgSound.h"

#include "OMediaError.h"

OMediaDXSound::OMediaDXSound(OMediaDXSoundEngine *engine, OMediaSound *master) : 
				OMediaEngineImpSlave(engine,master)
{
	dx_soundbuffer=omc_NULL;
	dirty = true;
}

OMediaDXSound::~OMediaDXSound()
{
	dxpurge();
}

void OMediaDXSound::dxpurge(void)
{
	if (dx_soundbuffer) dx_soundbuffer->Release();
	dx_soundbuffer = omc_NULL;
}

void OMediaDXSound::master_modified(void)
{
	dirty = true;
}

void OMediaDXSound::create_sound(void)
{
	OMediaDXSoundEngine			*sound_mng = (OMediaDXSoundEngine*)get_engine();
	LPDIRECTSOUND				dxsound = sound_mng->get_dxsound();
	DSBUFFERDESC				desc;
	HRESULT						hres;
	OMediaWinRtgSound			*rtg = (OMediaWinRtgSound*)(((OMediaSound*)get_imp_master())->get_retarget());
	OMediaWavFormatEx			*omtwave;
	char						*wavedata;
	long						datalength;

	omtwave = rtg->wavfmt;
	wavedata = rtg->snddata;
	datalength = rtg->data_length;
	dirty = false;

	dxpurge();

	if (!rtg->sound) return;

	dx_wform.wFormatTag = omtwave->format_tag;
	dx_wform.nChannels = omtwave->nchannels;
	dx_wform.nSamplesPerSec = omtwave->samples_per_sec;
	dx_wform.nAvgBytesPerSec = omtwave->bytes_per_sec;
	dx_wform.nBlockAlign = omtwave->block_align;
	dx_wform.wBitsPerSample = omtwave->bits_per_sample;
	dx_wform.cbSize = 0;

	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY |DSBCAPS_STATIC;
	desc.dwBufferBytes = datalength;
	desc.dwReserved = 0;
	desc.lpwfxFormat = &dx_wform;

	hres = dxsound->CreateSoundBuffer(&desc,&dx_soundbuffer,NULL);
	if (hres!=DS_OK) omd_OSEXCEPTION(hres); 

	void		*writep,*writep2;
	DWORD	writes,writes2;

	hres = dx_soundbuffer->Lock(0,datalength,&writep,&writes,&writep2,&writes2,0);
	if (hres!=DS_OK) omd_OSEXCEPTION(hres); 

	OMediaMemTools::copy(wavedata,writep,datalength);
	writes2 = 0;
	writes = datalength;

	hres = dx_soundbuffer->Unlock(writep,writes,writep2,writes2);
	if (hres!=DS_OK) omd_OSEXCEPTION(hres); 
}

#endif
