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
#include "OMediaWindow.h"
#include "OMediaThread.h"
#include "OMediaMacRtgWindow.h"

#include "OMediaSPVideoEngine.h"
#include "OMediaAnimPeriodical.h"

OMediaEventManager *OMediaEventManager::event_manager;
static RgnHandle empty_rgn;

class OMediaMacKeyDown
{
	public:
	
	omt_CommandKey	cmdkey;
	unsigned char  rawkey;
	unsigned char  ascii;
};

static list<OMediaMacKeyDown> keydown;
static void omt_oseventspkey_to_omtevent(short rawkey, OMediaEvent *omtevent);


OMediaEventManager::OMediaEventManager(void)
{
	::SetEventMask(everyEvent);

	key_trap = NULL;
	event_manager = this;
}

OMediaEventManager::~OMediaEventManager(void)
{
	if (event_manager==this) event_manager=NULL;

	if (empty_rgn) 
	{
		DisposeRgn(empty_rgn);
		empty_rgn = NULL;
	}
}

void *OMediaEventManager::get_local_message_handler(void)
{
	return NULL;	// Not required for the MacOS implementation of OMT
}

void OMediaEventManager::update_menu_states(void) 
{
	OMediaWindow	*omtwin;

	omtwin = OMediaWindow::find_active();
	if (omtwin)
	{	
		OMediaMacRtgWindow *rtgwin = (OMediaMacRtgWindow*)omtwin->get_retarget();
		if (rtgwin) rtgwin->update_menu_states();
	}
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
	OMediaEvent	event;
	EventRecord	theEvent;
	
	if (!empty_rgn) empty_rgn = NewRgn();

	if (OMediaThread::get_current_process_priority()==omtpc_Normal)
	{
		if (WaitNextEvent (everyEvent, &theEvent, 0, empty_rgn))
		{
			if (!filter_event(&theEvent))
			{
				osevent_to_omtevent(&theEvent,&event);
				if (event.type != omtet_Null && OMediaSupervisor::get_main_supervisor())
					OMediaSupervisor::get_main_supervisor()->listen_to_message(omsg_Event, &event);
			}
		}
	}


	list<OMediaMacKeyDown>::iterator 	ki,nki;
	unsigned char 						km[16];
	Point								mp;
	bool								get_key = true;


	for(ki = keydown.begin(); ki!=keydown.end();)
	{
		if (get_key)
		{
			get_key =false;
			GetKeys((long*)km);
			GetMouse(&mp );
		}
		
		nki = ki;
		nki++;

		if (( ( km[(*ki).rawkey>>3] >> ((*ki).rawkey & 7) ) & 1)==0)
		{
			event.type = omtet_KeyUp;
			event.ascii_key = (*ki).ascii;

			omt_oseventspkey_to_omtevent((*ki).rawkey,&event);
			event.mouse_x = mp.h;
			event.mouse_y = mp.v;
			event.command_key = (*ki).cmdkey;

			if (key_trap) key_trap->listen_to_message(omsg_KeyUp, &event);
			else key_broadcaster.broadcast_message(omsg_KeyUp, &event);

			if (event.type != omtet_Null && OMediaSupervisor::get_main_supervisor())
				OMediaSupervisor::get_main_supervisor()->listen_to_message(omsg_Event, &event);

			keydown.erase(ki);
		}	

		ki = nki;
	}

}

static void omt_oseventextra_to_omtevent(EventRecord *macevent, OMediaEvent *omtevent)
{
	if (macevent->modifiers&shiftKey) omtevent->command_key |= omtck_Shift;
	if (macevent->modifiers&cmdKey) omtevent->command_key |= omtck_Command;
	if (macevent->modifiers&optionKey) omtevent->command_key |= omtck_Alt;
	if (macevent->modifiers&controlKey) omtevent->command_key |= omtck_Control;

	omtevent->mouse_x = macevent->where.h;
	omtevent->mouse_y = macevent->where.v;
}

static void omt_oseventspkey_to_omtevent(short rawkey, OMediaEvent *omtevent)
{

	switch(omtevent->ascii_key)
	{	
		case 0x3:
		omtevent->special_key = omtsk_Enter;
		break;
	
		case 0xD:
		omtevent->special_key = omtsk_Return;
		break;
	
		case 0x1C:
		omtevent->special_key = omtsk_ArrowLeft;
		break;
		
		case 0x1D:
		omtevent->special_key = omtsk_ArrowRight;
		break;
		
		case 0x1E:
		omtevent->special_key = omtsk_ArrowTop;
		break;
		
		case 0x1F: 
		omtevent->special_key = omtsk_ArrowBottom;
		break;

		case 0x9:
		omtevent->special_key = omtsk_Tab;
		break;

		case 0x08:
		omtevent->special_key = omtsk_Backspace;
		break;

		case 0x7F:
		omtevent->special_key = omtsk_Delete;
		break;		
	}
	
	switch(rawkey)
	{
		case 0x35:
		omtevent->special_key = omtsk_Escape;
		break;
	
		case 0x7A:omtevent->special_key = omtsk_F1;break;
		case 0x78:omtevent->special_key = omtsk_F2;break;
		case 0x63:omtevent->special_key = omtsk_F3;break;
		case 0x76:omtevent->special_key = omtsk_F4;break;
		case 0x60:omtevent->special_key = omtsk_F5;break;
		case 0x61:omtevent->special_key = omtsk_F6;break;
		case 0x62:omtevent->special_key = omtsk_F7;break;
		case 0x64:omtevent->special_key = omtsk_F8;break;
		case 0x65:omtevent->special_key = omtsk_F9;break;
		case 0x6D:omtevent->special_key = omtsk_F10;break;
		case 0x67:omtevent->special_key = omtsk_F11;break;
		case 0x6F:omtevent->special_key = omtsk_F12;break;
		case 0x69:omtevent->special_key = omtsk_F13;break;
		case 0x6B:omtevent->special_key = omtsk_F14;break;
		case 0x71:omtevent->special_key = omtsk_F15;break;
	}		
}

static void omf_HandleMenu(long res)
{
	OMediaWindow	*omtwin;
	short menu,item;

	menu = HiWord(res);
	item = LoWord(res); 
	
	if (menu)
	{
		omtwin = OMediaWindow::find_active();
		if (omtwin)
		{	
			OMediaMacRtgWindow *rtgwin = (OMediaMacRtgWindow*)omtwin->get_retarget();
			if (rtgwin)
			{			
				omt_Message msg;
			
				if (rtgwin->menu_to_message(menu, item, msg))
				{
					if (msg!=omsg_NULL && OMediaSupervisor::get_main_supervisor())
					{
						OMediaSupervisor::get_main_supervisor()->listen_to_message(msg,omtwin);								
					}
				}							
			}
		}
	}

	HiliteMenu (0); 
}

void OMediaEventManager::osevent_to_omtevent(void *localevent, OMediaEvent *omtevent)
{
	EventRecord		*macevent = (EventRecord*) localevent;
	WindowPtr		whichWindow;
	OMediaWindow	*omtwin;
	short			part;

	switch(macevent->what)
	{			
		case autoKey:					
		case keyDown:
		{
			bool	is_menu = false;
			long	menures;
		
			omtevent->type = omtet_KeyDown;
			omtevent->ascii_key = macevent->message&charCodeMask;
			
			if ( (macevent->modifiers & cmdKey) && (omtevent->ascii_key > 0x20))
			{
				omtwin = OMediaWindow::find_active();
				if (omtwin)
				{			
					OMediaMacRtgWindow *rtgwin = (OMediaMacRtgWindow*)omtwin->get_retarget();
					if (rtgwin && rtgwin->menus.size())
					{
						update_menu_states();					
						menures = MenuKey(omtevent->ascii_key);
						is_menu = (HiWord(menures)!=0);
					}
				}
			}
			
			if (!is_menu)
			{				
				omt_oseventspkey_to_omtevent((macevent->message&keyCodeMask)>>8,omtevent);
				omt_oseventextra_to_omtevent(macevent,omtevent);
				if (key_trap) key_trap->listen_to_message(omsg_KeyDown, omtevent);
				else key_broadcaster.broadcast_message(omsg_KeyDown, omtevent);

				if (macevent->what==keyDown)
				{
					OMediaMacKeyDown	mackeydown;
				
					mackeydown.rawkey = (macevent->message&keyCodeMask)>>8L;
					mackeydown.ascii = omtevent->ascii_key;		
					mackeydown.cmdkey = omtevent->command_key;
					keydown.push_back(mackeydown);
				}
			}
			else 
			{
				omtevent->type = omtet_Null;
				omf_HandleMenu(menures);
			}
		}
		return;
									
		case updateEvt:
		whichWindow = (WindowPtr)macevent->message;
		omtwin = (OMediaWindow*)GetWRefCon(whichWindow);
		if (!omtwin) return;

		((OMediaMacRtgWindow*)omtwin->get_retarget())->process_update_region();

		omtevent->type = omtet_WindowRefresh;

		omtevent->window = omtwin;
		break;
	
		case osEvt:
		if (macevent->message & 0x01000000)		//	Suspend/resume event
		{
			static int sleepCount;
			
			// Only in full-screen...
			
			if (OMediaWindow::get_window_list()->size() &&
				(*OMediaWindow::get_window_list()->begin())->get_style()==omwstyle_Fullscreen)
			{
				list<OMediaWindow*>::iterator wi;
				if (macevent->message & 0x00000001)	//	Resume
				{
					if (sleepCount>0) 
					{								
						OMediaAnimPeriodical::get_anim_periocial()->pause(false);

						for(wi = OMediaWindow::get_window_list()->begin();
							wi!=OMediaWindow::get_window_list()->end();
							wi++)
						{
							(*wi)->exit_sleep_mode();
						}
						
		
						sleepCount=0;
					}

				}
				else
				{
					sleepCount++;			

					OMediaAnimPeriodical::get_anim_periocial()->pause(true);

						// Sleep
					for(wi = OMediaWindow::get_window_list()->begin();
						wi!=OMediaWindow::get_window_list()->end();
						wi++)
					{
						(*wi)->erase_contexts();
						(*wi)->enter_sleep_mode();
					}
				}
			}
		}
		break;
										
		case kHighLevelEvent:
		AEProcessAppleEvent(macevent);
    	break;
								
		case activateEvt:
		whichWindow = (WindowPtr)macevent->message;
		omtwin = (OMediaWindow*)GetWRefCon(whichWindow);
		if (!omtwin) return;
		omtevent->window = omtwin;

		if(macevent->modifiers & activeFlag)
		{
			omtevent->type = omtet_WindowActivated;
		}
		else
		{
			omtevent->type = omtet_WindowDeactivated;
		}
		break;	
	
		case mouseUp:
		omtevent->type = omtet_LMouseUp;
		omt_oseventextra_to_omtevent(macevent,omtevent);
		
		send_click_message(omtevent,false,false);

		part = FindWindow(macevent->where,&whichWindow);
		if (whichWindow)
		{
			omtevent->window = (OMediaWindow*)GetWRefCon(whichWindow);			
			if (omtevent->window)
			{
				omtevent->win_mouse_x = omtevent->mouse_x - omtevent->window->get_x();
				omtevent->win_mouse_y = omtevent->mouse_y - omtevent->window->get_y();
			}
		}
		return;
		
		case mouseDown:
		omtevent->type = omtet_Null;
		omt_oseventextra_to_omtevent(macevent,omtevent);
		
		part = FindWindow(macevent->where,&whichWindow);
		
		omtwin = (OMediaWindow*)GetWRefCon(whichWindow);
		if (omtwin)
                {					
                    omtevent->window = omtwin;
                    omtevent->win_mouse_x = omtevent->mouse_x - omtevent->window->get_x();
                    omtevent->win_mouse_y = omtevent->mouse_y - omtevent->window->get_y();
                }

#ifdef omd_ENABLE_DRAWSPROCKET
		if (part!=inMenuBar && !omtwin && OMediaSPVideoEngine::is_draw_sprocket_on())
		{
			list<OMediaWindow*> *wlist;
			wlist = OMediaWindow::get_window_list();
			if (wlist->size())
			{
				omtwin = (*(wlist->begin()));			
				omtevent->type = omtet_LMouseDown;
				send_click_message(omtevent,true,true);
				return;
			}			
		}
#endif		


		if (!omtwin && part!=inMenuBar) return;

		switch(part)
		{
		
			case inContent:
			if (omtwin->is_active())
			{
				omtevent->type = omtet_LMouseDown;
				send_click_message(omtevent,true,true);
			}
			else 
				omtwin->activate();
			break;
			
			case inMenuBar:
			{
				update_menu_states();
			
				omtevent->type = omtet_Null;
				omf_HandleMenu(MenuSelect(macevent->where));
			}
			break;
			
			case inGrow:
			{
				omtwin->activate();

				Rect r;
				long res;
				
				omtwin->get_min_size(r.left,r.top);
				omtwin->get_max_size(r.right,r.bottom);
				
				res = GrowWindow (whichWindow,macevent->where, &r);
				if (res)
				{
					omtwin->set_size(LoWord(res),HiWord(res));
				}

				omtevent->type = omtet_LMouseDown;
				send_click_message(omtevent,true,true);
			}
			break;
	
			case inGoAway:
			omtwin->activate();
			if (TrackGoAway (whichWindow, macevent->where)) 
			{
				omtevent->type = omtet_WindowCloseButton;
			}
			break;
			
			case inDrag:
			{
				omtwin->activate();
			
				Rect r;
				r.top = -30000;
				r.bottom = 30000;
				r.left = -30000;
				r.right = 30000;
				DragWindow (whichWindow, macevent->where, &r);
				omtwin->bounds_changed();
			}
			break;
			
			case inZoomIn: 
			case inZoomOut: 
			if (TrackBox (whichWindow, macevent->where, part))
			{
				Point	p;
			
				omtwin->activate();
				
				p.h = 400;
				p.v = 300;

				ZoomWindowIdeal (whichWindow,part,&p);

				
				omtwin->bounds_changed();
			}
			break;
			
			default:
			omtevent->type = omtet_Null;
			break;
			
		}
		return;	
	}

	if (omtevent->type != omtet_Null) omt_oseventextra_to_omtevent(macevent,omtevent);
}



