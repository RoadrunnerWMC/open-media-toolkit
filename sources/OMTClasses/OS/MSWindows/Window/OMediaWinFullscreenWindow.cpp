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
#include "OMediaWinRtgWindow.h"


static LONG WINAPI omf_DefSingleWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


OMediaFullscreenWindow::OMediaFullscreenWindow(OMediaSupervisor *supervisor,
										bool initially_visible):
					OMediaWindow(supervisor)
{
	retarget = new OMediaWinRtgWindow;
	((OMediaWinRtgWindow*)retarget)->window = this;

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
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);
	
	RECT	r,wr;

	wr.left = wr.top = 0;

	AdjustWindowRectEx(&wr,WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
						rtg->has_menu, WS_EX_TOPMOST);


	GetWindowRect(rtg->hwnd,&r);
	x = r.left - wr.left;
	y = r.top - wr.top;

	GetClientRect(rtg->hwnd,&r);
	width = r.right;
	height = r.bottom;

	OMediaWindow::bounds_changed();
}

void OMediaFullscreenWindow::set_title(string str)
{
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);

	SetWindowText(rtg->hwnd,str.c_str());
}

void OMediaFullscreenWindow::init_window(void)
{	
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);
	WNDCLASS    wc;

    static HINSTANCE hInstance = 0;

    if (!hInstance) 
	{
        hInstance = GetModuleHandle(NULL);
        wc.style         = CS_OWNDC;
        wc.lpfnWndProc   = (WNDPROC)OMediaEventManager::get_event_manager()->get_local_message_handler();
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = "OMT SingleWindow";

        if (!RegisterClass(&wc)) return;
    }

    rtg->hwnd = CreateWindowEx(WS_EX_TOPMOST, "OMT SingleWindow", title.c_str(), 
						WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
                        CW_USEDEFAULT, CW_USEDEFAULT, 
						CW_USEDEFAULT, CW_USEDEFAULT, NULL, 
						NULL, hInstance, NULL);

    if (rtg->hwnd == NULL) return;

	SetWindowLong(rtg->hwnd,GWL_USERDATA,(LONG)this);

	OMediaFullscreenWindow::bounds_changed();

	set_backcolor(back_color);
}

void OMediaFullscreenWindow::cleanup_window(void)
{
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);

	DestroyWindow(rtg->hwnd);
}

void OMediaFullscreenWindow::activate(void)
{
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);

	if (!is_active())
	{
		BringWindowToTop(rtg->hwnd);
	}
}

void OMediaFullscreenWindow::activated(void)
{
	OMediaWindow::activated();
}

omt_Bool OMediaFullscreenWindow::is_active(void)
{
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);

	return (GetForegroundWindow()==rtg->hwnd);
}

void OMediaFullscreenWindow::show(void)
{
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);

	if (!asleep) ShowWindow(rtg->hwnd,SW_SHOW);
	visible = true;
}

void OMediaFullscreenWindow::hide(void)
{
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);

	if (!asleep) ShowWindow(rtg->hwnd,SW_HIDE);
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
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);

	OMediaWindow::set_backcolor(rgb);
	InvalidateRect(rtg->hwnd, NULL, false);
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
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);

	if (!asleep)
	{
		asleep = true;
		ShowWindow(rtg->hwnd,SW_HIDE);
	}
}

void OMediaFullscreenWindow::exit_sleep_mode(void)
{
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);

	if (asleep) 
	{
		asleep = false;
		update_window_posiz();
	}
}

void OMediaFullscreenWindow::update_window_posiz(void)
{
	omt_RTGDefineLocalType(OMediaWinRtgWindow,rtg);

	if (win2engines.size())
	{
		OMediaVideoEngine	*eng = (*win2engines.begin()).engine;
	
		if (eng->get_state() & omvesc_Linked )
		{
			OMediaVideoMode *vmode = eng->get_current_video_mode();

			RECT	r;

			r.left = vmode->its_card->positionx;
			r.top = vmode->its_card->positiony;
			r.right = r.left + vmode->width;
			r.bottom = r.top + vmode->height;

			AdjustWindowRectEx(&r,WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
								rtg->has_menu, WS_EX_TOPMOST);

			MoveWindow(rtg->hwnd,r.left,r.top, 
								r.right-r.left,
								r.bottom-r.top,TRUE);

			if (visible) 
			{
				ShowWindow(rtg->hwnd,SW_SHOW);
			}
			else ShowWindow(rtg->hwnd,SW_HIDE);	

			bounds_changed();
		}
	}
}
