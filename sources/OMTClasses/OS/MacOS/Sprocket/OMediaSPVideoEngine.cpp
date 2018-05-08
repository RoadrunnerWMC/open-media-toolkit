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
#ifdef omd_ENABLE_DRAWSPROCKET

#include "OMediaSPVideoEngine.h"
#include "OMediaString.h"
#include "OMediaError.h"
#include "OMediaDefMessages.h"
#include "OMediaRenderer.h"
#include "OMediaWindow.h"
#include "OMediaEventManager.h"

#ifdef omd_ENABLE_OPENGL
#include "OMediaGLRenderer.h"
#endif

#define FADE_GAMMA

long OMediaSPVideoEngine::draw_sprocked_on;

const static RGBColor rgbBlack = { 0x0000, 0x0000, 0x0000 };

OMediaSPVideoEngine::OMediaSPVideoEngine(OMediaWindow *master) : OMediaMacVideoEngine(master,ommeic_Sprocket)
{
	OSStatus	err;
	
	OMediaEventManager::get_event_manager()->get_msg_filter_broadcaster()->addlistener(this);

	if (!draw_sprocked_on) 
	{
		err = DSpStartup();
		if (err!=noErr) omd_OSEXCEPTION(err);
		DSpSetBlankingColor(&rgbBlack);
	}
	draw_sprocked_on++;
}

OMediaSPVideoEngine::~OMediaSPVideoEngine()
{
	unlink();
	
	draw_sprocked_on--;
	if (!draw_sprocked_on) DSpShutdown();
}

void OMediaSPVideoEngine::unlink(void) 
{
	if (!(state&omvesc_Linked)) return;
	state&=~omvesc_VideoModeSet;
	
	OMediaMacVideoEngine::unlink();
}

void OMediaSPVideoEngine::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_LowlevelEventFilter:
		if (full_screen_entered)
		{
			OMediaFilterLocalEvent	*filter = (OMediaFilterLocalEvent*)param;
			Boolean	processed;
			EventRecord	*event = (EventRecord*)filter->local_event;
			static bool firstResumeEvent = true;
			bool doProcess;
			
			if (event->what==osEvt)
			{
				if (firstResumeEvent)
				{
					firstResumeEvent = false;
					doProcess = false;
				}
				else doProcess = true;
			}
			else doProcess = true;
			
			if (doProcess)
			{			
				DSpProcessEvent (event,&processed);
				filter->handled = processed;
				if (processed) 
					abort_broadcast();
			}
		}
		break;
	
		default:
		OMediaMacVideoEngine::listen_to_message(msg, param);
		break;
	}
}

void OMediaSPVideoEngine::enter_fullscreen(void)
{
	if (!full_screen_entered && (state&omvesc_Linked))
	{	
		GDHandle				device = (GDHandle)linked_card->private_id;
		OSStatus				err;
		DisplayIDType			display_id = 0;


		DMGetDisplayIDByGDevice(device,&display_id,true);
		
		err = DSpGetFirstContext(display_id,&context_ref);
		if (err!=noErr) omd_OSEXCEPTION(err);

		do
		{	
			err = DSpContext_GetAttributes (context_ref, &context_attr);
			if (err!=noErr) omd_OSEXCEPTION(err);

			if ((context_attr.frequency>>16L) == current_video_mode->refresh_rate &&
				context_attr.displayWidth == current_video_mode->width &&
				context_attr.displayHeight == current_video_mode->height &&
				context_attr.displayBestDepth == current_video_mode->depth)
			{
				break;	// This is our context			
			}

			err = DSpGetNextContext (context_ref, &context_ref);
			if (err!=noErr)
			{
				if (kDSpContextNotFoundErr != err)  omd_OSEXCEPTION(err);
				context_ref = 0;
			}
		} 
		while (context_ref);

		if (!context_ref) omd_EXCEPTION(omcerr_CantFindCurrentVMode);
	
		context_attr.pageCount			= 1; 
		context_attr.contextOptions		= 0 | kDSpContextOption_DontSyncVBL;
	
		err = DSpContext_Reserve(context_ref, &context_attr );
		if (err!=noErr) omd_OSEXCEPTION(err);

#ifdef FADE_GAMMA
		if (current_video_mode!=default_video_mode) DSpContext_FadeGammaOut (NULL, NULL);
#endif		
		
		err = DSpContext_SetState (context_ref, kDSpContextState_Active); // activate our context


#ifdef FADE_GAMMA
		if (current_video_mode!=default_video_mode) DSpContext_FadeGammaIn (NULL, NULL);
#endif

		if (noErr != err && err!=kDSpConfirmSwitchWarning)
		{
			DSpContext_Release (context_ref);
			omd_OSEXCEPTION(err);
		}
		
		
		full_screen_entered = true;
	}
}

CGrafPtr OMediaSPVideoEngine::get_full_screen_port(void)
{
	CGrafPtr port;
	
	if (full_screen_entered)  DSpContext_GetFrontBuffer (context_ref, &port);

	return port;
}


void OMediaSPVideoEngine::exit_fullscreen(void)
{
	if (full_screen_entered)
	{
		full_screen_entered = false;

#ifdef FADE_GAMMA
		if (current_video_mode!=default_video_mode) DSpContext_FadeGammaOut (NULL, NULL);
#endif

		DSpContext_SetState (context_ref,kDSpContextState_Inactive);
		
#ifdef FADE_GAMMA
		if (current_video_mode!=default_video_mode) DSpContext_FadeGammaIn (NULL, NULL);
#endif		
		
		DSpContext_Release(context_ref);
	}
}

void OMediaSPVideoEngine::set_mode(OMediaVideoMode *vmode)
{

	if (!(state&omvesc_Linked)) omd_EXCEPTION(omcerr_NoVideoCardLinked);
	if (master_window->get_style()!=omwstyle_Fullscreen) omd_EXCEPTION(omcerr_CantChangeVideoMode);
	if (current_video_mode==vmode) return;

	deselect_renderer();

	if (!vmode) vmode = default_video_mode;	
	
	broadcast_message(omsg_ScreenModeWillChange,this);
		
	exit_fullscreen();
	
	current_video_mode = vmode;
	if (current_video_mode!=default_video_mode) state|=omvesc_VideoModeSet;
	else state&=~omvesc_VideoModeSet;

	enter_fullscreen();

	renderer_map.erase(renderer_map.begin(),renderer_map.end());
	update_renderer_defs();
	
	broadcast_message(omsg_ScreenModeChanged,this);
}



#endif

