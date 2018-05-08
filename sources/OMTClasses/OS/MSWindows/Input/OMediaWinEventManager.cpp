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
 
#include "OMediaEventManager.h"
#include "OMediaSupervisor.h"
#include "OMediaThread.h"
#include "OMediaWinRtgWindow.h"
#include "OMediaWindow.h"
#include "OMediaAnimPeriodical.h"


OMediaEventManager *OMediaEventManager::event_manager;

static LONG CALLBACK omt_WinMessageHandler(HWND win, UINT msgCode, WPARAM wParam, LPARAM lParam);

OMediaEventManager::OMediaEventManager(void)
{
	event_manager = this;
	key_trap = omc_NULL;
}

OMediaEventManager::~OMediaEventManager(void)
{
	if (event_manager==this) event_manager=omc_NULL;
}

void *OMediaEventManager::get_local_message_handler(void)
{
	return omt_WinMessageHandler;
}

bool OMediaEventManager::filter_event(void *local_event)
{
	OMediaFilterLocalEvent	filter;

	filter.handled = false;
	filter.local_event = local_event;
	filtermsg_broadcaster.broadcast_message(omsg_LowlevelEventFilter,&filter);
	
	return filter.handled;
}

void OMediaEventManager::spend_time(void)
{
	bool				res,needtranslate;
	MSG					theMessage;
	OMediaWindow		*omtwin;
	OMediaWinRtgWindow	*rtgwin;

	Sleep(1);	// Sleep one millisec

	while (PeekMessage(&theMessage,NULL,0,0,PM_NOREMOVE ))
	{
		res = GetMessage(&theMessage,NULL,0,0)!=0;

		if (!filter_event(&theMessage))
		{
			needtranslate = true;
			omtwin = (OMediaWindow*)GetWindowLong(theMessage.hwnd,GWL_USERDATA);
			if (omtwin)
			{
				rtgwin = (OMediaWinRtgWindow*)omtwin->get_retarget();
				if (rtgwin->accel)
				{
					if (TranslateAccelerator(theMessage.hwnd,rtgwin->accel,&theMessage))
					{
						needtranslate = false;
					}
				}
			}
	
			if (needtranslate) TranslateMessage(&theMessage);
			DispatchMessage(&theMessage);
			
			if (!res && OMediaSupervisor::get_main_supervisor())
				OMediaSupervisor::get_main_supervisor()->listen_to_message(omsg_Quit);
		}
	}	
}

static void omt_oseventextra_to_omtevent(OMediaWinEvent *winevent, OMediaEvent *omtevent)
{
	POINT	cp;

	GetCursorPos(&cp);
	omtevent->mouse_x = cp.x;
	omtevent->mouse_y = cp.y;

	omtevent->win_mouse_x = omtevent->mouse_x - omtevent->window->get_x();
	omtevent->win_mouse_y = omtevent->mouse_y - omtevent->window->get_y();


	omtevent->command_key = 0;

	if (GetAsyncKeyState(VK_SHIFT)&(1<<15)) omtevent->command_key |= omtck_Shift;
	if (GetAsyncKeyState(VK_MENU)&(1<<15)) omtevent->command_key |= omtck_Alt;
	if (GetAsyncKeyState(VK_CONTROL)&(1<<15)) omtevent->command_key |= omtck_Control;

	if ((GetAsyncKeyState(VK_LWIN)&(1<<15)) ||
		(GetAsyncKeyState(VK_RWIN)&(1<<15))) omtevent->command_key |= omtck_Command;
}

static void omt_oseventspkey_to_omtevent(OMediaWinEvent *winevent, OMediaEvent *omtevent)
{
	switch(winevent->wParam)
	{
		case VK_RETURN:
		omtevent->special_key = omtsk_Return;
		break;

		case VK_LEFT:
		omtevent->special_key = omtsk_ArrowLeft;
		break;

		case VK_UP:
		omtevent->special_key = omtsk_ArrowTop;
		break;

		case VK_RIGHT:
		omtevent->special_key = omtsk_ArrowRight;
		break;

		case VK_DOWN:
		omtevent->special_key = omtsk_ArrowBottom;
		break;

		case VK_TAB:
		omtevent->special_key = omtsk_Tab;
		break;

		case VK_BACK:
		omtevent->special_key = omtsk_Backspace;
		break;

		case VK_DELETE:
		omtevent->special_key = omtsk_Delete;
		break;

		case VK_ESCAPE:
		omtevent->special_key = omtsk_Escape;
		break;

		case VK_F1:omtevent->special_key = omtsk_F1;break;
		case VK_F2:omtevent->special_key = omtsk_F2;break;
		case VK_F3:omtevent->special_key = omtsk_F3;break;
		case VK_F4:omtevent->special_key = omtsk_F4;break;
		case VK_F5:omtevent->special_key = omtsk_F5;break;
		case VK_F6:omtevent->special_key = omtsk_F6;break;
		case VK_F7:omtevent->special_key = omtsk_F7;break;
		case VK_F8:omtevent->special_key = omtsk_F8;break;
		case VK_F9:omtevent->special_key = omtsk_F9;break;
		case VK_F10:omtevent->special_key = omtsk_F10;break;
		case VK_F11:omtevent->special_key = omtsk_F11;break;
		case VK_F12:omtevent->special_key = omtsk_F12;break;
		case VK_F13:omtevent->special_key = omtsk_F13;break;
		case VK_F14:omtevent->special_key = omtsk_F14;break;
		case VK_F15:omtevent->special_key = omtsk_F15;break;	
	}
}

void OMediaEventManager::osevent_to_omtevent(void *localevent, OMediaEvent *omtevent)
{
	OMediaWinEvent			*winevent = (OMediaWinEvent*) localevent;
	OMediaWindow			*omtwin;

	omtwin = omtevent->window = (OMediaWindow*)GetWindowLong(winevent->win,GWL_USERDATA);
	if (!omtwin) return;

	switch(winevent->msgCode)
	{	
		case WM_CHAR:
		if ((TCHAR)winevent->wParam==8) break;
		omtevent->type = omtet_KeyDown;
		omtevent->ascii_key = (TCHAR)winevent->wParam;
		omtevent->special_key = omtsk_Null;
		omt_oseventextra_to_omtevent(winevent, omtevent);

		if (key_trap) key_trap->listen_to_message(omsg_KeyDown, omtevent);
		else key_broadcaster.broadcast_message(omsg_KeyDown,omtevent);
		return;

		case WM_LBUTTONDOWN:
		omtevent->type = omtet_LMouseDown;
		omt_oseventextra_to_omtevent(winevent, omtevent);
		send_click_message(omtevent,true, true);
		return;
		
		case WM_LBUTTONUP:
		omtevent->type = omtet_LMouseUp;
		omt_oseventextra_to_omtevent(winevent, omtevent);
		send_click_message(omtevent,false, false);
		return;
		
		case WM_RBUTTONDOWN:
		omtevent->type = omtet_RMouseDown;
		omt_oseventextra_to_omtevent(winevent, omtevent);
		send_click_message(omtevent,false, true);
		return;
		
		case WM_RBUTTONUP:
		omtevent->type = omtet_RMouseUp;
		omt_oseventextra_to_omtevent(winevent, omtevent);
		send_click_message(omtevent,false, false);
		return;

		case WM_KEYDOWN:
		omtevent->type = omtet_KeyDown;
		omtevent->ascii_key = 0;
		omtevent->special_key = omtsk_Null;
		
		omt_oseventextra_to_omtevent(winevent, omtevent);
		omt_oseventspkey_to_omtevent(winevent, omtevent);

		if (key_trap) key_trap->listen_to_message(omsg_KeyDown, omtevent);
		else key_broadcaster.broadcast_message(omsg_KeyDown,omtevent);
		return;
		
		case WM_PAINT:
		{
			omtevent->type = omtet_WindowRefresh;
			((OMediaWinRtgWindow*)omtwin->get_retarget())->process_update_region();
		}	
		break;

		case WM_ACTIVATEAPP:
		{
			static omt_TaskPriority ppriority,tpriority;
			static BOOL deactivated;
		
			deactivated = !((BOOL) winevent->wParam);
			if (deactivated)
			{
				ppriority = OMediaThread::get_current_process_priority();
				tpriority = OMediaThread::get_current_thread_priority();
				OMediaThread::set_current_process_priority(omtpc_Normal);
				OMediaThread::set_current_thread_priority(omtpc_Normal);
			}
			else
			{
				OMediaThread::set_current_process_priority(ppriority);
				OMediaThread::set_current_thread_priority(tpriority);
			}
		}
		break;

		case WM_KEYUP:
		omtevent->type = omtet_KeyUp;
		omtevent->ascii_key = 0;
		omtevent->special_key = omtsk_Null;
		
		omt_oseventextra_to_omtevent(winevent, omtevent);
		omt_oseventspkey_to_omtevent(winevent, omtevent);

		if (key_trap) key_trap->listen_to_message(omsg_KeyUp, omtevent);
		else key_broadcaster.broadcast_message(omsg_KeyUp,omtevent);
		return;

		case WM_ACTIVATE:
		if (LOWORD(winevent->wParam)!=WA_INACTIVE)
		{
			omtevent->type = omtet_WindowActivated;
		}
		else
		{
			omtevent->type = omtet_WindowDeactivated;
		}
		break;

		case WM_CLOSE:
		omtevent->type = omtet_WindowCloseButton;
		break;

		case WM_SIZE:
		case WM_MOVE:
		omtwin->bounds_changed();
		OMediaAnimPeriodical::set_quick_refresh(true);
		OMediaAnimPeriodical::get_anim_periocial()->spend_time();
		OMediaAnimPeriodical::set_quick_refresh(false);
		break;

		case WM_INITMENU:
		update_menu_states();
		break;

		case WM_COMMAND:
		{
			short wID = LOWORD(winevent->wParam);
			OMediaWinRtgWindow *rtgwin = (OMediaWinRtgWindow*)omtwin->get_retarget();
			if (rtgwin)
			{			
				omt_Message msg;
			
				if (rtgwin->menu_to_message(wID, msg))
				{
					if (msg!=omsg_NULL && OMediaSupervisor::get_main_supervisor())
					{
						OMediaSupervisor::get_main_supervisor()->listen_to_message(msg,omtwin);								
					}
				}
			}
		}
		break;

		case WM_GETMINMAXINFO:
		{
			LPMINMAXINFO lpmmi = (LPMINMAXINFO) winevent->lParam;			
			short w,h;

			omtwin->get_max_size(w,h);
			if (w && h) 
			{
				lpmmi->ptMaxTrackSize.x = w;
				lpmmi->ptMaxTrackSize.y = h;
			}

			omtwin->get_min_size(w,h);
			if (w && h) 
			{
				lpmmi->ptMinTrackSize.x = w;
				lpmmi->ptMinTrackSize.y = h;
			}
		}
		break;
	}

	if (omtevent->type != omtet_Null) omt_oseventextra_to_omtevent(winevent,omtevent);
}

void OMediaEventManager::update_menu_states(void) 
{
	OMediaWindow	*omtwin;

	omtwin = OMediaWindow::find_active();
	if (omtwin)
	{	
		OMediaWinRtgWindow *rtgwin = (OMediaWinRtgWindow*)omtwin->get_retarget();
		if (rtgwin) rtgwin->update_menu_states();
	}
}


static LONG CALLBACK omt_WinMessageHandler(HWND win, UINT msgCode, WPARAM wParam, LPARAM lParam)
{
	OMediaEvent		event;
	OMediaWinEvent 	winevent;

	winevent.win = win;
	winevent.msgCode = msgCode;
	winevent.wParam = wParam;
	winevent.lParam = lParam;

	if (OMediaEventManager::get_event_manager())
	{
		OMediaEventManager::get_event_manager()->osevent_to_omtevent(&winevent,&event);
		
		if (event.type != omtet_Null && OMediaSupervisor::get_main_supervisor())
		{
			if (event.type!=omtet_Processed)
				OMediaSupervisor::get_main_supervisor()->listen_to_message(omsg_Event, &event);
			return 0L;
		}
	}
	

	if (winevent.msgCode!=WM_NULL) return DefWindowProc(win,msgCode,wParam,lParam);
	
	
	return 0L;
}


