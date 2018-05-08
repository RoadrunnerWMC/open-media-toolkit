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
 

#pragma once
#ifndef OMEDIA_EventManager_H
#define OMEDIA_EventManager_H

#include "OMediaTypes.h"
#include "OMediaPeriodical.h"
#include "OMediaBroadcaster.h"

class OMediaEvent;
class OMediaWindow;

class OMediaEventManager : public OMediaPeriodical
{
	public:

	// * Construction

	omtshared OMediaEventManager(void);		
	omtshared virtual ~OMediaEventManager(void);

	// * Process

	omtshared virtual	void spend_time(void);

	// * Get main event manager (should not exist more than one by application)
	
	inline static OMediaEventManager *get_event_manager(void) {return event_manager;}


	// * Broadcasters
	
		// Mouse click
		
	inline OMediaBroadcaster *getclickbroadcaster(void) {return &click_broadcaster;}

		// Keys

	inline OMediaBroadcaster *getkeybroadcaster(void) {return &key_broadcaster;}
	inline void set_key_focus(OMediaListener *focus) {key_trap = focus;}
	inline OMediaListener *get_key_focus(void) const {return key_trap;}

		// Message filter

	inline OMediaBroadcaster *get_msg_filter_broadcaster(void) {return &filtermsg_broadcaster;}


	// * Get local OS message handler
	omtshared static void *get_local_message_handler(void);
	omtshared virtual void osevent_to_omtevent(void *localevent, OMediaEvent *event);

	// * Menu states

	omtshared virtual void update_menu_states(void);


	protected:

	inline void send_click_message(OMediaEvent *event, bool action_button,bool down)
	{
		if (action_button && down)
		{
			OMediaListener *old_key_trap = key_trap;
			key_trap = NULL;
			click_broadcaster.broadcast_message(omsg_MouseClick,event);
			if (key_trap!=old_key_trap)
			{
				if (old_key_trap) old_key_trap->listen_to_message(omsg_LostFocus);
			}
		}
		else
		{
			click_broadcaster.broadcast_message((down)?omsg_MouseClick:omsg_MouseClick_Up,event);
		}
	}

	omtshared virtual bool filter_event(void *local_event);


	omtshared static OMediaEventManager 	*event_manager;
	
	OMediaBroadcaster			click_broadcaster;
	OMediaBroadcaster			key_broadcaster, filtermsg_broadcaster;
	OMediaListener				*key_trap;
};

class OMediaFilterLocalEvent
{
	public:

	void 	*local_event;
	bool	handled;
};


enum omt_EventType
{
	omtet_Null,
	omtet_LMouseDown,
	omtet_LMouseUp,
	omtet_RMouseDown,
	omtet_RMouseUp,
	omtet_KeyDown,
	omtet_KeyUp,
	omtet_WindowRefresh,
	omtet_WindowCloseButton,
	omtet_WindowActivated,
	omtet_WindowDeactivated,
	
	omtet_Custom,
	omtet_Processed
};

typedef unsigned short omt_CommandKey;
const omt_CommandKey omtck_Null	= 0;
const omt_CommandKey omtck_Shift = 1<<0;
const omt_CommandKey omtck_Command = 1<<1;
const omt_CommandKey omtck_Control = 1<<2;
const omt_CommandKey omtck_Alt = 1<<3;


enum omt_SpecialKey
{
	omtsk_Null,
	omtsk_Escape,
	omtsk_Backspace,
	omtsk_Return,
	omtsk_Delete,
	omtsk_Enter,
	omtsk_ArrowLeft,
	omtsk_ArrowRight,
	omtsk_ArrowTop,
	omtsk_ArrowBottom,
	omtsk_Tab,
	
	omtsk_F1,omtsk_F2,omtsk_F3,omtsk_F4,omtsk_F5,
	omtsk_F6,omtsk_F7,omtsk_F8,omtsk_F9,omtsk_F10,
	omtsk_F11,omtsk_F12,omtsk_F13,omtsk_F14,omtsk_F15,	

	omtsk_Keypad0,	omtsk_Keypad1,	omtsk_Keypad2, omtsk_Keypad3,
	omtsk_Keypad4,	omtsk_Keypad5,	omtsk_Keypad6, omtsk_Keypad7,
	omtsk_Keypad8,	omtsk_Keypad9,	omtsk_KeypadDot
};

class OMediaEvent
{
	public:
	
	inline OMediaEvent(void) 
	{
		type=omtet_Null; command_key = omtck_Null; 
		special_key =omtsk_Null; ascii_key = 0; 
		user=0;
		window=NULL;
		win_mouse_x = win_mouse_y = 0;
	}

	omt_EventType			type;
	omt_CommandKey			command_key;
	omt_SpecialKey			special_key;
	char					ascii_key;
	long					mouse_x,mouse_y;			// desktop relative
	long					win_mouse_x,win_mouse_y;	// window relative
	void					*user;
	
	OMediaWindow	*window;
};



#endif

