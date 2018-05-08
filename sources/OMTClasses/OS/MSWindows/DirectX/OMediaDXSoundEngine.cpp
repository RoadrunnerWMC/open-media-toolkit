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

#include "OMediaDXSoundEngine.h"
#include "OMediaDXSoundChannel.h"
#include "OMediaDXSound.h"
#include "OMediaWinRtgWindow.h"
#include "OMediaWindow.h"
#include "OMediaError.h"
#include "OMediaSound.h"

OMediaDXSoundEngine::OMediaDXSoundEngine(OMediaWindow *master_window) : OMediaSoundEngine(ommeic_DirectX,
																						  master_window)
{
	HRESULT			hres;		

	dx_primarybuffer = omc_NULL;
	dxsound = omc_NULL;

	hres = DirectSoundCreate(NULL,&dxsound,NULL);
	if (hres!=DS_OK)  omd_OSEXCEPTION(hres);

	omt_RTGDefineLocalTypeObj(OMediaWinRtgWindow,rtg,master_window);	

	hres = dxsound->SetCooperativeLevel(rtg->hwnd,DSSCL_EXCLUSIVE);
	if (hres!=DS_OK)  omd_OSEXCEPTION(hres);

	DSBUFFERDESC				  desc;

	desc.dwSize = sizeof(DSBUFFERDESC);
	desc.dwFlags =DSBCAPS_PRIMARYBUFFER;
	desc.dwBufferBytes = 0;
	desc.dwReserved = 0;
	desc.lpwfxFormat = 0;

	hres = dxsound->CreateSoundBuffer(&desc,&dx_primarybuffer,NULL);
	if (hres!=DS_OK)  omd_OSEXCEPTION(hres);
}

OMediaDXSoundEngine::~OMediaDXSoundEngine()
{
	delete_all_channels();
	delete_all_implementations();

	if (dx_primarybuffer) dx_primarybuffer->Release();
	if (dxsound) dxsound->Release();
}

OMediaDXSound *OMediaDXSoundEngine::get_dxsnd_implementation(OMediaSound *master)
{
	OMediaDXSound *dxsnd;
	
	dxsnd = (OMediaDXSound*)master->find_implementation(this, 0, NULL);
	if (!dxsnd) dxsnd = new OMediaDXSound(this, master);	
	if (dxsnd->dirty) dxsnd->create_sound();

	return dxsnd;
}

void OMediaDXSoundEngine::set_buffer_format(OMediaSound *sound)
{
	OMediaDXSound *dxsnd;

	dxsnd = get_dxsnd_implementation(sound);

	if (dxsnd->dx_soundbuffer)
	{
		HRESULT	res;
		res = dx_primarybuffer-> SetFormat(&dxsnd->dx_wform);
		if (res!=DS_OK)  omd_OSEXCEPTION(res);
	}
}

void OMediaDXSoundEngine::compact_buffer(void)
{
	dxsound->Compact();
}

long OMediaDXSoundEngine::get_max_channels(void)
{
	return -1;
}

OMediaSoundChannel	*OMediaDXSoundEngine::create_sound_channel(void)
{
	OMediaDXSoundChannel	*channel = new OMediaDXSoundChannel;
	channel->its_engine = this;
	return channel;
}

#endif

