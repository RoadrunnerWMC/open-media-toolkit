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


#include "OMediaWinInputEngine.h"

OMediaWinInputEngine::OMediaWinInputEngine(OMediaWindow *master_window, bool quiet_hid, omt_EngineID eid):
										OMediaInputEngine(eid, master_window)

{
	if (!quiet_hid) init_hid();
}

OMediaWinInputEngine::~OMediaWinInputEngine() {}

bool OMediaWinInputEngine::mouse_down(void) 
{
	unsigned short	s = GetAsyncKeyState(VK_LBUTTON);

	return (s & (1<<15)) ? true:false;
}

void OMediaWinInputEngine::get_sysmouse_position(short &x, short &y) 
{
	POINT	cp;

	GetCursorPos(&cp);
	x = cp.x;
	y = cp.y;
}

omt_CommandKey OMediaWinInputEngine::get_command_keys_status(void)
{
	omt_CommandKey	command_key=0;

	if (GetAsyncKeyState(VK_SHIFT)&(1<<15)) command_key |= omtck_Shift;
	if (GetAsyncKeyState(VK_MENU)&(1<<15)) command_key |= omtck_Alt;
	if (GetAsyncKeyState(VK_CONTROL)&(1<<15)) command_key |= omtck_Control;

	if ((GetAsyncKeyState(VK_LWIN)&(1<<15)) ||
		(GetAsyncKeyState(VK_RWIN)&(1<<15))) command_key |= omtck_Command;	

	return command_key;
}


void OMediaWinInputEngine::init_hid(void)
{
	OMediaHumanInterfaceDevice		*hid;

	hdi_list.erase(hdi_list.begin(),hdi_list.end());
	
	// Create keyboard device
	
	hid = new OMediaWinHID_Keyboard(this);
	hdi_list.push_back(hid);
}

	
OMediaWinHID_Keyboard::OMediaWinHID_Keyboard(OMediaInputEngine *engine) : 
						OMediaHumanInterfaceDevice(engine) 
{
	type=omhidtc_Keyboard;
	product_name = "Windows Keyboard";
	instance_name = "Keyboard";
	
	element_list = omc_NULL;
	lock_count = 0;
}

OMediaWinHID_Keyboard::~OMediaWinHID_Keyboard()
{
	while(lock_count) unlock();
} 

static OMediaHIDElement *omf_FIND_ROOT(unsigned char rawk,omt_HIDElementList *element_list)
{
	for(omt_HIDElementList::iterator i=element_list->begin();
		i!=element_list->end();
		i++)		
	{
		if ( ((OMediaWinHIDElement_Key*)(*i))->raw_key==rawk) return (*i);	
	}
	
	return omc_NULL;
}


static void omf_NEW_SPECIAL_KEY(omt_SpecialKey k, char rawk, OMediaWinHID_Keyboard *kboard, omt_HIDElementList *element_list)
{
	OMediaWinHIDElement_Key			*key;

	key = new OMediaWinHIDElement_Key(kboard);
	key->type = omhideltc_SpecialKey; 
	key->special_key = k; 
	key->raw_key = rawk;

	switch(k)
	{
		case omtsk_Escape: key->name ="Escape"; break;	case omtsk_Backspace: key->name ="Backspace"; break;
		case omtsk_Return: key->name ="Return"; break;	case omtsk_Delete: key->name ="Delete"; break;
		case omtsk_Enter: key->name ="Enter"; break;	case omtsk_ArrowLeft: key->name ="Left Arrow"; break;
		case omtsk_ArrowRight: key->name ="Right Arrow"; break;	case omtsk_ArrowTop: key->name ="Top Arrow"; break;
		case omtsk_ArrowBottom: key->name ="Bottom Arrow"; break;	case omtsk_Tab: key->name ="Tab"; break;
		case omtsk_F1: key->name ="F1"; break;			case omtsk_F2: key->name ="F2"; break;
		case omtsk_F3: key->name ="F3"; break;			case omtsk_F4: key->name ="F4"; break;
		case omtsk_F5: key->name ="F5"; break;			case omtsk_F6: key->name ="F6"; break;
		case omtsk_F7: key->name ="F7"; break;			case omtsk_F8: key->name ="F8"; break;
		case omtsk_F9: key->name ="F9"; break;			case omtsk_F10: key->name ="F10"; break;
		case omtsk_F11: key->name ="F11"; break;		case omtsk_F12: key->name ="F12"; break;
		case omtsk_F13: key->name ="F13"; break;		case omtsk_F14: key->name ="F14"; break;
		case omtsk_F15: key->name ="F15"; break;

		case omtsk_Keypad0: key->name ="Keypad 0"; break;
		case omtsk_Keypad1: key->name ="Keypad 1"; break;
		case omtsk_Keypad2: key->name ="Keypad 2"; break;
		case omtsk_Keypad3: key->name ="Keypad 3"; break;
		case omtsk_Keypad4: key->name ="Keypad 4"; break;
		case omtsk_Keypad5: key->name ="Keypad 5"; break;
		case omtsk_Keypad6: key->name ="Keypad 6"; break;
		case omtsk_Keypad7: key->name ="Keypad 7"; break;
		case omtsk_Keypad8: key->name ="Keypad 8"; break;
		case omtsk_Keypad9: key->name ="Keypad 9"; break;
		case omtsk_KeypadDot: key->name ="Keypad Dot"; break;	
	}

	OMediaHIDElement *eroot = omf_FIND_ROOT(rawk,element_list);
	if (eroot) key->set_root(eroot);

	element_list->push_back(key);
}

static void omf_NEW_COMMAND_KEY(omt_CommandKey k, char rawk, OMediaWinHID_Keyboard *kboard, omt_HIDElementList *element_list)  
{
	OMediaWinHIDElement_Key			*key;

	key = new OMediaWinHIDElement_Key(kboard);
	key->type = omhideltc_CommandKey;  
	key->command_key = k; 
	key->raw_key = rawk; 

	switch(k)
	{
		case omtck_Shift:	key->name ="Shift"; 	break;
		case omtck_Command:	key->name ="Command"; 	break;
		case omtck_Control:	key->name ="Control"; 	break;
		case omtck_Alt:		key->name ="Alt"; 		break;
	}	

	OMediaHIDElement *eroot = omf_FIND_ROOT(rawk,element_list);
	if (eroot) key->set_root(eroot);

	element_list->push_back(key);
}

static void omf_NEW_ASCII_KEY(char k, char rawk, OMediaWinHID_Keyboard *kboard, omt_HIDElementList *element_list)  
{
	OMediaWinHIDElement_Key			*key;

	key = new OMediaWinHIDElement_Key(kboard); 
	key->type = omhideltc_ASCIIKey;  
	if (k==' ') key->name = "Space";
	else key->name = k; 
	key->ascii_key = k; 
	key->raw_key = rawk; 

	OMediaHIDElement *eroot = omf_FIND_ROOT(rawk,element_list);
	if (eroot) key->set_root(eroot);
	
	element_list->push_back(key);
}


omt_HIDElementList *OMediaWinHID_Keyboard::lock(void)
{
	if ((lock_count++)>0) return element_list;

	unsigned short i;
	
	element_list = new omt_HIDElementList;

										
	// Ok, create the keyboard elements:

	// Special keys:

	omf_NEW_SPECIAL_KEY(omtsk_Escape, VK_ESCAPE,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Backspace, VK_BACK,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Return, VK_RETURN,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_ArrowLeft, VK_LEFT,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_ArrowRight, VK_RIGHT,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_ArrowTop, VK_UP,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_ArrowBottom, VK_DOWN,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Tab, VK_TAB,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Delete, VK_DELETE,this,element_list);


	omf_NEW_SPECIAL_KEY(omtsk_F1, VK_F1,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F2, VK_F2,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F3, VK_F3,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F4, VK_F4,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F5, VK_F5,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F6, VK_F6,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F7, VK_F7,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F8, VK_F8,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F9, VK_F9,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F10, VK_F10,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F11, VK_F11,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F12, VK_F12,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F13, VK_F13,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F14, VK_F14,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F15, VK_F15,this,element_list);

	omf_NEW_SPECIAL_KEY(omtsk_Keypad0, VK_NUMPAD0,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad1, VK_NUMPAD1,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad2, VK_NUMPAD2,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad3, VK_NUMPAD3,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad4, VK_NUMPAD4,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad5, VK_NUMPAD5,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad6, VK_NUMPAD6,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad7, VK_NUMPAD7,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad8, VK_NUMPAD8,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad9, VK_NUMPAD9,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_KeypadDot, VK_DECIMAL,this,element_list);


	// Command keys:
	
	omf_NEW_COMMAND_KEY(omtck_Shift,VK_SHIFT,this,element_list);
	omf_NEW_COMMAND_KEY(omtck_Command,VK_LWIN,this,element_list);
	omf_NEW_COMMAND_KEY(omtck_Control,VK_CONTROL,this,element_list);
	omf_NEW_COMMAND_KEY(omtck_Alt,VK_MENU,this,element_list);
	
	// ASCII keys:

	unsigned char		res;
	unsigned short		keycode;


	for(i=0;i<=0xFF; i++)
	{
		keycode = i;

		res = (unsigned char)(MapVirtualKey(i,2)&0xFF);
		if (res)
		{
			omf_NEW_ASCII_KEY(res, i,this,element_list);		
		}
	}
	
	return element_list;
}

void OMediaWinHID_Keyboard::unlock(void)
{
	if (lock_count<0)
	{
		lock_count=0;
		return;
	}

	if ((--lock_count)==0)
	{
		for (omt_HIDElementList::iterator i = element_list->begin();
			 i!=element_list->end();
			 i++)
		{
			delete *(i);			
		}
	
		delete element_list;
		element_list = omc_NULL;
	}
}

void OMediaWinHIDElement_Key::read_data(OMediaHIDElementData &data)
{
	if (type == omhideltc_CommandKey && command_key == omtck_Command)
	{
		data.down = ((GetAsyncKeyState(VK_LWIN)&(1<<15)) ||
					(GetAsyncKeyState(VK_RWIN)&(1<<15)));
	}
	else
	{
		data.down = (GetAsyncKeyState(raw_key)&(1<<15))!=0;
	}
}




