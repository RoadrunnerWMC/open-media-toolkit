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

#include "OMediaFullscreenWindow.h"
#include "OMediaMacRtgWindow.h"
#include "OMediaMacVideoEngine.h"

OMediaFullscreenWindow::OMediaFullscreenWindow(OMediaSupervisor *supervisor,
										bool initially_visible):
										OMediaWindow(supervisor)
{
	retarget = new OMediaMacRtgWindow;
	((OMediaMacRtgWindow*)retarget)->window = this;

	visible = false;
	init_window();
	update_video_engine_connection();
	if (initially_visible) show();
}

OMediaFullscreenWindow::~OMediaFullscreenWindow()
{
	destroy_dependencies();
	cleanup_window();
}

omt_WindowStyle OMediaFullscreenWindow::get_style(void)
{
	return omwstyle_Fullscreen;
}

void OMediaFullscreenWindow::bounds_changed(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);
	
	Rect	portRect;

	GetWindowBounds (rtg->windowptr,kWindowContentRgn,&portRect);

	x = portRect.left;
	y = portRect.top;
	width = portRect.right-portRect.left;
	height = portRect.bottom-portRect.top;

	OMediaWindow::bounds_changed();
}


void OMediaFullscreenWindow::set_title(string str)
{
	// Mac implementation has no title
}

void OMediaFullscreenWindow::init_window(void)
{	
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);
	Rect 	winbounds;
	
	winbounds.left 		= 0;
	winbounds.top 		= 0;
	winbounds.right 	= 640;
	winbounds.bottom 	= 480;

	rtg->windowptr = NewCWindow(NULL, &winbounds,"\p",false,plainDBox, 
										(WindowPtr)-1L,
					 					false,
					 					(long)this);
	x = winbounds.left;
	y = winbounds.top;
	width = winbounds.right-winbounds.left;
	height = winbounds.bottom-winbounds.top;

	set_backcolor(back_color);
}

void OMediaFullscreenWindow::cleanup_window(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	DisposeWindow(rtg->windowptr);
}

void OMediaFullscreenWindow::activate(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	if (!is_active())
	{
		SelectWindow(rtg->windowptr);
	}
}

void OMediaFullscreenWindow::activated(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	rtg->relink_menu();
	OMediaWindow::activated();
}

omt_Bool OMediaFullscreenWindow::is_active(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	return (FrontWindow()==rtg->windowptr);
}

void OMediaFullscreenWindow::show(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	if (!asleep) ShowWindow(rtg->windowptr);
	visible = true;
}

void OMediaFullscreenWindow::hide(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	if (!asleep) HideWindow(rtg->windowptr);
	visible = false;
}

void OMediaFullscreenWindow::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_VideoEngineLinked:
		exit_sleep_mode();
		break;

		case omsg_VideoEngineUnlinked:
		enter_sleep_mode();
		break;
		
		case omsg_ScreenModeChanged:
		update_window_posiz();
		break;	
	}

	OMediaWindow::listen_to_message(msg, param);
}

void OMediaFullscreenWindow::link_video_engine(OMediaVideoEngine *engine)
{
	if (win2engines.size()) return;

	OMediaWindow::link_video_engine(engine);
	update_video_engine_connection();	
}

void OMediaFullscreenWindow::unlink_video_engine(OMediaVideoEngine *engine)
{
	OMediaWindow::unlink_video_engine(engine);
	update_video_engine_connection();
}


void OMediaFullscreenWindow::set_backcolor(const OMediaRGBColor &rgb)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	OMediaWindow::set_backcolor(rgb);

	Rect	r;
	SetRect(&r,0,0,get_width(),get_height());

	InvalWindowRect(rtg->windowptr,&r);
}

void OMediaFullscreenWindow::update_video_engine_connection(void)
{
	if (win2engines.size())
	{
		if ( (*win2engines.begin()).engine->get_state() & omvesc_Linked )
		{
			asleep = true;
			exit_sleep_mode();
		}	
	}
	
	asleep = false;
	enter_sleep_mode();
}

void OMediaFullscreenWindow::enter_sleep_mode(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	if (!asleep)
	{
		asleep = true;
		HideWindow(rtg->windowptr);	
	}
}

void OMediaFullscreenWindow::exit_sleep_mode(void)
{
	if (win2engines.size())
	{
		if ( !((*win2engines.begin()).engine->get_state() & omvesc_Linked) )
			return;
	}
	else return;

	if (asleep) 
	{
		asleep = false;
		update_window_posiz();
	}
}

void OMediaFullscreenWindow::update_window_posiz(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);
	
	rtg->render_port = NULL;

	if (win2engines.size())
	{
		OMediaMacVideoEngine	*eng = (OMediaMacVideoEngine*)(*win2engines.begin()).engine;
	
		if (eng->get_state() & omvesc_Linked )
		{
			OMediaVideoMode *vmode = eng->get_current_video_mode();

			rtg->render_port = eng->get_full_screen_port();

			MoveWindow (rtg->windowptr, vmode->its_card->positionx,
										vmode->its_card->positiony, false);

			SizeWindow(rtg->windowptr, vmode->width, vmode->height, false);
		
			Rect	r;
		
			r.left = 0;
			r.top = 0;
			r.right = vmode->width;
			r.bottom = vmode->height;
			InvalWindowRect(rtg->windowptr,&r);

			if (visible && rtg->render_port==NULL) 
			{
				ShowWindow(rtg->windowptr);	
				SelectWindow(rtg->windowptr);
			}
			else HideWindow(rtg->windowptr);
			
			bounds_changed();
		}
	}
}

