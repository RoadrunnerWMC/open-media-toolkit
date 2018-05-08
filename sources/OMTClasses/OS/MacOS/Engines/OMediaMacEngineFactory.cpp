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
#include "OMediaMacVideoEngine.h"
#include "OMediaMacSoundEngine.h"
#include "OMediaMacInputEngine.h"

#ifdef omd_ENABLE_INPUTSPROCKET	
#include "OMediaSPInputEngine.h"
#endif

#ifdef omd_ENABLE_DRAWSPROCKET	
#include "OMediaSPVideoEngine.h"
#endif

#ifdef __MWERKS__

static UInt32 CheckMacOSX (void);

static UInt32 CheckMacOSX (void)
{
	UInt32 response;
    
	if ((Gestalt(gestaltSystemVersion, (SInt32 *) &response) == noErr) && (response >= 0x01000))
		return response;
	else
		return 0;
}

#endif

OMediaEngineFactory *OMediaEngineFactory::factory = NULL;


OMediaEngineFactory::OMediaEngineFactory()
{
#ifdef omd_ENABLE_INPUTSPROCKET

	// Sprocket available ?

/*	Typedef pascal OSStatus (*ISpStartupProcPtr) (void);
	
	OSErr	err;
	CFragConnectionID	connID	= kInvalidID;
	NSetTrapAddressProcPtr	myNsetTrapAddressProcPtr = nil; 

	err = GetSharedLibrary( "\pInputSprocketLib", kCompiledCFragArch, 
	kReferenceCFrag, &connID, NULL, NULL ); 

	if ( err == noErr )
	{

	err = FindSymbol( connID, "\pNSetTrapAddress", (Ptr *) & ISpStartupProcPtr, NULL );
	} 

	if ( err == noErr )
	{

	//	Routine is available!
	(*myNsetTrapAddressProcPtr ) ( trapAddr, trapNum, tTyp );
	}*/

	if (!CheckMacOSX())
		input_engines.push_back(ommeic_Sprocket);
#endif

#ifdef omd_ENABLE_DRAWSPROCKET
	video_engines.push_back(ommeic_Sprocket);
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
	if (id==ommeic_OS || id==ommeic_Sprocket || id==ommeic_Best)
	{	
		omt_VideoEngineAttr	attr = omveaf_CanChangeDepth;
		long					value = 0;
		unsigned long			displayMgrVersion;
		long					displayMgrPresent;

		Gestalt(gestaltDisplayMgrAttr,&value);
		Gestalt(gestaltDisplayMgrVers, (long*)&displayMgrVersion);

		displayMgrPresent=value&(1<<gestaltDisplayMgrPresent);
		if (displayMgrPresent && displayMgrVersion >= 0x00020000) attr |=omveaf_CanChangeResolution;
		
		return attr;
	}
	
	omd_EXCEPTION(omcerr_InvalidParamater);

	return 0;
}


omt_SoundEngineAttr OMediaEngineFactory::get_sound_engine_attr(omt_EngineID id)
{
	if (id==ommeic_OS || id==ommeic_Best)
	{
		return omseaf_NoChannelLimitation|omseaf_Volume|omseaf_Frequence;
	}

	omd_EXCEPTION(omcerr_InvalidParamater);	
	return 0;
}



omt_InputEngineAttr OMediaEngineFactory::get_input_engine_attr(omt_EngineID id)
{
#ifdef omd_ENABLE_INPUTSPROCKET	

	if ((id==ommeic_Sprocket || id==ommeic_Best) && !CheckMacOSX())

	{
		return omieaf_Controller;
	}

#endif

	if (id==ommeic_OS || id==ommeic_Best)
	{
		return 0;
	}


	omd_EXCEPTION(omcerr_InvalidParamater);	
	return 0;
}



OMediaVideoEngine *OMediaEngineFactory::create_video_engine(omt_EngineID id, OMediaWindow *master_win)
{
#ifdef omd_ENABLE_DRAWSPROCKET	
	if (id==ommeic_Sprocket || id==ommeic_Best)
	{
		return new OMediaSPVideoEngine(master_win);
	}
#endif

	if (id==ommeic_OS || id==ommeic_Best)
	{
		return new OMediaMacVideoEngine(master_win);
	}

	omd_EXCEPTION(omcerr_InvalidParamater);	
	return NULL;
}

OMediaSoundEngine *OMediaEngineFactory::create_sound_engine(omt_EngineID id, OMediaWindow *master_win)
{
	if (id==ommeic_OS || id==ommeic_Best)
	{
		return new OMediaMacSoundEngine(master_win);
	}

	omd_EXCEPTION(omcerr_InvalidParamater);	
	return NULL;
}

OMediaInputEngine *OMediaEngineFactory::create_input_engine(omt_EngineID id, OMediaWindow *master_win)
{
#ifdef omd_ENABLE_INPUTSPROCKET	
	if ((id==ommeic_Sprocket || id==ommeic_Best) && !CheckMacOSX())
	{
		return new OMediaSPInputEngine(master_win);
	}
#endif

	if (id==ommeic_OS || id==ommeic_Best)
	{
		return new OMediaMacInputEngine(master_win);
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
