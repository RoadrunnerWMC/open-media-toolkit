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

#include "OMediaMacInputEngine.h"
#include "OMediaTimer.h"
#include "OMediaMacRtgWindow.h"
#include "OMediaWindow.h"

OMediaMacInputEngine::OMediaMacInputEngine(OMediaWindow *master_win, bool quiet_hid, omt_EngineID eid) : OMediaInputEngine(eid, master_win)
{
	if (!quiet_hid) init_hid();
}

OMediaMacInputEngine::~OMediaMacInputEngine() {}

bool OMediaMacInputEngine::mouse_down(void) 
{
	return Button();
}

void OMediaMacInputEngine::get_sysmouse_position(short &x, short &y) 
{
	Point	p;

	if ( !((OMediaMacRtgWindow*)master_window->get_retarget())->windowptr) return;

	SetPort( GetWindowPort (((OMediaMacRtgWindow*)master_window->get_retarget())->windowptr) );
	GetMouse(&p);
	LocalToGlobal(&p);
	
	x = p.h;
	y = p.v;
}


omt_CommandKey OMediaMacInputEngine::get_command_keys_status(void)
{
	omt_CommandKey key = 0;

	unsigned char km[16];

	GetKeys( (long *) km);
	
	if  (( ( km[0x38>>3] >> (0x38 & 7) ) & 1)==1) key|=omtck_Shift;
	if  (( ( km[0x37>>3] >> (0x37 & 7) ) & 1)==1) key|=omtck_Command;
	if  (( ( km[0x3b>>3] >> (0x3b & 7) ) & 1)==1) key|=omtck_Control;
	if  (( ( km[0x3a>>3] >> (0x3a & 7) ) & 1)==1) key|=omtck_Alt;

	return key;
}

void OMediaMacInputEngine::init_hid(void)
{
	OMediaHumanInterfaceDevice		*hid;

	hdi_list.erase(hdi_list.begin(),hdi_list.end());

	// There is no real support on MacOS for low-level keyboard reading.
	// Not even sprocket allows you to access keyboard device. 
	// So I'll emulate it:
	
	// Create keyboard device
	
	hid = new OMediaMacHID_Keyboard(this);
	hdi_list.push_back(hid);
}

	
OMediaMacHID_Keyboard::OMediaMacHID_Keyboard(OMediaInputEngine *engine) : OMediaHumanInterfaceDevice(engine)
{
	type=omhidtc_Keyboard;
	product_name = "Apple Keyboard";
	instance_name = "Keyboard";
	
	element_list = NULL;
	lock_count = 0;
}

OMediaMacHID_Keyboard::~OMediaMacHID_Keyboard()
{
	while(lock_count) unlock();
} 

static OMediaHIDElement *omf_FIND_ROOT(unsigned char rawk,omt_HIDElementList *element_list)
{
	for(omt_HIDElementList::iterator i=element_list->begin();
		i!=element_list->end();
		i++)		
	{
		if ( ((OMediaMacHIDElement_Key*)(*i))->raw_key==rawk) return (*i);	
	}
	
	return NULL;
}


static void omf_NEW_SPECIAL_KEY(omt_SpecialKey k, char rawk, OMediaMacHID_Keyboard *kboard, omt_HIDElementList *element_list)  
{
	OMediaMacHIDElement_Key			*key;

	key = new OMediaMacHIDElement_Key(kboard);
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
                default: break;

	}

	OMediaHIDElement *eroot = omf_FIND_ROOT(rawk,element_list);
	if (eroot) key->set_root(eroot);

	element_list->push_back(key);
}

static void omf_NEW_COMMAND_KEY(omt_CommandKey k, char rawk, OMediaMacHID_Keyboard *kboard, omt_HIDElementList *element_list)  
{
	OMediaMacHIDElement_Key			*key;

	key = new OMediaMacHIDElement_Key(kboard);
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

static void omf_NEW_ASCII_KEY(char k, char rawk, OMediaMacHID_Keyboard *kboard, omt_HIDElementList *element_list)  
{
	OMediaMacHIDElement_Key			*key;

	key = new OMediaMacHIDElement_Key(kboard); 
	key->type = omhideltc_ASCIIKey;  
	if (k==' ') key->name = "Space";
	else key->name = k; 
	key->ascii_key = k; 
	key->raw_key = rawk; 

	OMediaHIDElement *eroot = omf_FIND_ROOT(rawk,element_list);
	if (eroot) key->set_root(eroot);
	
	element_list->push_back(key);
}


omt_HIDElementList *OMediaMacHID_Keyboard::lock(void)
{
	if ((lock_count++)>0) return element_list;

	unsigned short i,j;
	
	element_list = new omt_HIDElementList;

										
	// Ok, create the keyboard elements:

	// Special keys:

	omf_NEW_SPECIAL_KEY(omtsk_Escape, 0x35,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Backspace, 0x33,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Return, 0x24,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Enter, 0x4c,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_ArrowLeft, 0x7b,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_ArrowRight, 0x7c,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_ArrowTop, 0x7e,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_ArrowBottom, 0x7d,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Tab, 0x30,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Delete, 0x75,this,element_list);


	omf_NEW_SPECIAL_KEY(omtsk_F1, 0x7a,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F2, 0x78,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F3, 0x63,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F4, 0x76,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F5, 0x60,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F6, 0x61,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F7, 0x62,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F8, 0x64,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F9, 0x65,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F10, 0x6d,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F11, 0x67,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F12, 0x6f,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F13, 0x69,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F14, 0x6b,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_F15, 0x71,this,element_list);


	omf_NEW_SPECIAL_KEY(omtsk_Keypad0, 82,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad1, 83,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad2, 84,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad3, 85,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad4, 86,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad5, 87,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad6, 88,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad7, 89,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad8, 91,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_Keypad9, 92,this,element_list);
	omf_NEW_SPECIAL_KEY(omtsk_KeypadDot, 65,this,element_list);

	// Command keys:
	
	omf_NEW_COMMAND_KEY(omtck_Shift,0x38,this,element_list);
	omf_NEW_COMMAND_KEY(omtck_Command,0x37,this,element_list);
	omf_NEW_COMMAND_KEY(omtck_Control,0x3b,this,element_list);
	omf_NEW_COMMAND_KEY(omtck_Alt,0x3a,this,element_list);
	
	// ASCII keys:

	Ptr					KCHRPtr;
	unsigned long		res;
	unsigned short		keycode;
	unsigned long		state = 0;

	KCHRPtr = Ptr(GetScriptManagerVariable(smKCHRCache));
	
	
	for(j=0;j<4;j++)
	for(i=0;i<=0x7F; i++)
	{
		keycode = i;
		keycode |= (1<<7);
		
		switch(j)
		{
			case 1: keycode |= shiftKey; break;
			case 2: keycode |= optionKey; break;
			case 3: keycode |= optionKey|shiftKey; break;		
		}
		
		res = KeyTranslate(KCHRPtr, keycode, &state);
		if (res&0xFF)
		{
			omf_NEW_ASCII_KEY((res&0xFF), i,this,element_list);		
		}
	}
	
	return element_list;
}

void OMediaMacHID_Keyboard::unlock(void)
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
		element_list = NULL;
	}
}

void OMediaMacHIDElement_Key::read_data(OMediaHIDElementData &data)
{
	unsigned char km[16];

	GetKeys( (long *) km);

	data.down = ( ( km[raw_key>>3] >> (raw_key & 7) ) & 1)==1;
}



