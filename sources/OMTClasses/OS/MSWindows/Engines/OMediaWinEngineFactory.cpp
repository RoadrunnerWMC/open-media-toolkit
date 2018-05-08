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

#include "OMediaEngineFactory.h"
#include "OMediaBuildSwitch.h"
#include "OMediaError.h"
#include "OMediaWinVideoEngine.h"
#include "OMediaWinSoundEngine.h"
#include "OMediaWinInputEngine.h"
#include "OMediaMemTools.h"

#ifdef omd_ENABLE_DIRECTINPUT
#include "OMediaDXInputEngine.h"
#endif

#ifdef omd_ENABLE_DIRECTSOUND
#include "OMediaDXSoundEngine.h"
#endif

#ifdef omd_ENABLE_DIRECTDRAW
#include "OMediaDXVideoEngine.h"
#endif


OMediaEngineFactory *OMediaEngineFactory::factory = NULL;


OMediaEngineFactory::OMediaEngineFactory()
{	
	// Check for DirectX 7

	OSVERSIONINFO	osversion;
	bool			use_dx = false;

	OMediaMemTools::zero(&osversion,sizeof(OSVERSIONINFO));
	osversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (GetVersionEx(&osversion))
	{
		if (osversion.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) use_dx = true;
		else if (osversion.dwPlatformId==VER_PLATFORM_WIN32_NT)
		{
			if (osversion.dwMajorVersion>4) use_dx = true;
		}
	}

	if (use_dx)
	{

#ifdef omd_ENABLE_DIRECTINPUT
	input_engines.push_back(ommeic_DirectX);
#endif

#ifdef omd_ENABLE_DIRECTDRAW
	video_engines.push_back(ommeic_DirectX);
#endif

	}

#ifdef omd_ENABLE_DIRECTSOUND
	sound_engines.push_back(ommeic_DirectX);
#endif


	video_engines.push_back(ommeic_OS);
	sound_engines.push_back(ommeic_OS);
	input_engines.push_back(ommeic_OS);
}


OMediaEngineFactory::~OMediaEngineFactory()  {}


OMediaEngineFactory *OMediaEngineFactory::get_factory(void)
{
	if (!factory) factory = new OMediaEngineFactory;	
	return factory;
}


omt_VideoEngineAttr OMediaEngineFactory::get_video_engine_attr(omt_EngineID id)
{
#ifdef omd_ENABLE_DIRECTDRAW
	if (id==ommeic_DirectX || id==ommeic_Best)
	{
		return omveaf_CanChangeDepth|omveaf_CanChangeResolution;
	}
#endif
	
	if (id==ommeic_OS || id==ommeic_Best)
	{	
		// Changing depth and resolution requires DX
		return 0;
	}


	
	omd_EXCEPTION(omcerr_InvalidParamater);

	return 0;
}

omt_SoundEngineAttr OMediaEngineFactory::get_sound_engine_attr(omt_EngineID id)
{
#ifdef omd_ENABLE_DIRECTSOUND
	if (id==ommeic_DirectX || id==ommeic_Best)
	{
		return omseaf_NoChannelLimitation|omseaf_Volume|omseaf_Frequence;
	}
#endif

	if (id==ommeic_OS || id==ommeic_Best)
	{
		// Without DX, only basic sound output
		return 0L;
	}

	omd_EXCEPTION(omcerr_InvalidParamater);	
	return 0;
}

omt_InputEngineAttr OMediaEngineFactory::get_input_engine_attr(omt_EngineID id)
{

#ifdef omd_ENABLE_DIRECTINPUT

	if (id==ommeic_DirectX || id==ommeic_Best)
	{
		return omieaf_Controller|omieaf_ForceFeedback;
	}

#endif

	if (id==ommeic_OS || id==ommeic_Best)
	{
		return 0;
	}



	omd_EXCEPTION(omcerr_InvalidParamater);	
	return 0;
}



OMediaVideoEngine *OMediaEngineFactory::create_video_engine(omt_EngineID id, OMediaWindow *master_window)
{
#ifdef omd_ENABLE_DIRECTDRAW
	if (id==ommeic_DirectX || id==ommeic_Best)
	{
		return new OMediaDXVideoEngine(master_window);
	}
#endif

	if (id==ommeic_OS || id==ommeic_Best)
	{
		return new OMediaWinVideoEngine(master_window);
	}


	omd_EXCEPTION(omcerr_InvalidParamater);	
	return NULL;
}

OMediaSoundEngine *OMediaEngineFactory::create_sound_engine(omt_EngineID id, OMediaWindow *master_window)
{

#ifdef omd_ENABLE_DIRECTSOUND
	if (id==ommeic_DirectX || id==ommeic_Best)
	{
		return new OMediaDXSoundEngine(master_window);
	}
#endif

	if (id==ommeic_OS || id==ommeic_Best)
	{
		return new OMediaWinSoundEngine(master_window);
	}



	omd_EXCEPTION(omcerr_InvalidParamater);	
	return NULL;
}

OMediaInputEngine *OMediaEngineFactory::create_input_engine(omt_EngineID id, OMediaWindow *master_window)
{

#ifdef omd_ENABLE_DIRECTINPUT

	if (id==ommeic_DirectX || id==ommeic_Best)
	{
		return new OMediaDXInputEngine(master_window);
	}

#endif


	if (id==ommeic_OS || id==ommeic_Best)
	{
		return new OMediaWinInputEngine(master_window);
	}


	omd_EXCEPTION(omcerr_InvalidParamater);	
	return NULL;
}


//------ Engine factory auto-destructor

class OMediaEngineFactoryAutoDtor
{
	public:

	OMediaEngineFactoryAutoDtor();
	virtual ~OMediaEngineFactoryAutoDtor();

	static OMediaEngineFactoryAutoDtor destroy;
};

OMediaEngineFactoryAutoDtor OMediaEngineFactoryAutoDtor::destroy;
OMediaEngineFactoryAutoDtor::OMediaEngineFactoryAutoDtor() {}
OMediaEngineFactoryAutoDtor::~OMediaEngineFactoryAutoDtor() 
{
	OMediaEngineFactory::replace_factory(NULL);
}
