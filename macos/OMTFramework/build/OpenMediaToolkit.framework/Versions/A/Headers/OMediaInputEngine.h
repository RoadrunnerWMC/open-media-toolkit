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
#ifndef OMEDIA_InputEngine_H
#define OMEDIA_InputEngine_H

#include "OMediaTypes.h"
#include "OMediaRect.h"
#include "OMediaPeriodical.h"
#include "OMediaListener.h"
#include "OMediaEventManager.h"
#include "OMediaUSBHIDUsages.h"
#include "OMediaForceFeedback.h"
#include "OMediaVideoEngine.h"

#include <list>
#include <string>


class OMediaInputEngine;
class OMediaCustomMouse;

// Input Engine attributes

typedef unsigned short omt_InputEngineAttr;
const omt_InputEngineAttr omieaf_Controller 			= (1<<0);
const omt_InputEngineAttr omieaf_ForceFeedback 			= (1<<1);


//********* Human Interface Device (mouse, keyboard, joystick, wheel, etc.)

// For now, keyboard and mouse is not included in OMT HID
// support.

//---- Device element (axis, button, etc.)

enum omt_HIDElementType
{
	omhideltc_Button,
	omhideltc_Axis,
	omhideltc_DPad,
	omhideltc_Delta,
	omhideltc_Movement,

	omhideltc_ASCIIKey,
	omhideltc_SpecialKey,
	omhideltc_CommandKey,

	omhideltc_POV,
	
	omhideltc_NULL
};


class OMediaHumanInterfaceDevice;

enum omt_HIDPadData
{
	omhidpdc_PadIdle,
	omhidpdc_PadLeft,
	omhidpdc_PadUpLeft,
	omhidpdc_PadUp,
	omhidpdc_PadUpRight,
	omhidpdc_PadRight,
	omhidpdc_PadDownRight,
	omhidpdc_PadDown,
	omhidpdc_PadDownLeft
};

class OMediaHIDMovementData
{
	public:
	float			x_axis,y_axis;	// From -1.0 to 1.0
};


class OMediaHIDElementData
{
	public:

	union
	{
		float					axis;		// From -1.0 to 1.0. (axis)
		float					delta;		// 0.0 to x (delta)
		bool					down;		// true if down (button or key)
		omt_HIDPadData			pad_data;	// Pad direction (dpad)
		OMediaHIDMovementData	mov_data;	// x/y axis (movement)
		float					angle;		// angle, 0 to 359 degrees, -1 if centred (POV)
	};
};

enum omt_HIDAxisType
{
	omhtdat_Null,
	omhtdat_X,
	omhtdat_Y,
	omhtdat_Z,
	omhtdat_rX,
	omhtdat_rY,
	omhtdat_rZ
};

class OMediaHIDElement
{
	public:

	// * Construction

	omtshared OMediaHIDElement(OMediaHumanInterfaceDevice *device);
	omtshared virtual ~OMediaHIDElement();

	// * Element informations

	omt_HIDElementType				type;
	OMediaHIDUsage					hdi_usage;

	string							name;
	omt_HIDAxisType					axis_type;

	// * Additional information

	union
	{
		char			ascii_key;		// if type is omhideltc_ASCIIKey
		omt_SpecialKey	special_key;	// if type is omhideltc_SpecialKey
		omt_CommandKey	command_key;	// if type is omhideltc_CommandKey
	};
	
	// * Reading informations

	omtshared virtual void read_data(OMediaHIDElementData &data);

	// * Variation
	
	inline OMediaHIDElement *get_root(void) {return variation_root;}
	// There can be several representations of the same hardware
	// device element. For example a key can have an element for the ascii code
	// 'a' and a second one for the ascii code 'A'. In this case the root
	// is the first	representation of the element.
	// It returns 'this' if it is the root itself (that is the most common
	// situation).

	// * Its device

	inline OMediaHumanInterfaceDevice *get_device(void) {return its_device;}

	protected:

	OMediaHumanInterfaceDevice		*its_device;
	OMediaHIDElement				*variation_root;

};

typedef list<OMediaHIDElement*> omt_HIDElementList;

//---- Device (joystick, wheel, etc.)

enum omt_HIDType
{
	omhidtc_Mouse,
	omhidtc_Keyboard,
	omhidtc_Joystick,
	omhidtc_Wheel,
	omhidtc_Levers,
	omhidtc_Pedals,
	omhidtc_Headtracker,
	omhidtc_Rudder,
	
	omhidtc_NULL
};

typedef unsigned short omt_HIDFlags;
const omt_HIDFlags omhidf_ForceFeedback = (1<<0);	// Has force feedback support

class OMediaHumanInterfaceDevice
{
	public:

	// * Construction
	
	omtshared OMediaHumanInterfaceDevice(OMediaInputEngine *e);
	omtshared virtual ~OMediaHumanInterfaceDevice();

	inline OMediaInputEngine *get_engine(void) {return its_engine;}
	
	// * Device informations

	omt_HIDType			type;
	omt_HIDFlags		flags;
	OMediaHIDUsage		hdi_usage;

	string		product_name;
	string		instance_name;

	// * Locking device
	
	// Please note that you cannot access the element
	// list as long as device is not locked. When the
	// device is unlocked the complete element list
	// is deleted. You should not keep pointer to
	// HID element after device has been unlocked.
	
	// After the mouse has been locked you have no
	// guarantee that system mouse pointer will continue to work.
	
	// After the keyboard has been locked you have no
	// guarantee that you will continue to receive keyboard
	// event. So you have to read keys yourself using key
	// element.
	

	omtshared virtual omt_HIDElementList *lock(void);	// Returned list is read-only, never delete it or modify it.
	omtshared virtual void unlock(void);
	inline bool is_locked(void) const {return lock_count!=0;}	

	// Use the following method only if list is locked
	inline omt_HIDElementList *get_locked_element_list(void) {return element_list;}


	// * Force feedback support (element must be locked before using these methods)
	
	omtshared virtual void start_force_feedback(void);
	omtshared virtual void stop_force_feedback(void);
	inline bool is_force_feedback_started(void) {return fftype_list!=NULL;}

	// Returns ff effect type available. Force feedback must be started first!
	// When force feedback is stopped this list is deleted and should not be used
	// anymore.
	// Unlocking device automatically stops force feedback.

	inline omt_FFEffectTypeList *get_fftype_list(void) {return fftype_list;}

	//.....................................

	protected:

	omt_HIDElementList		*element_list;
	long					lock_count;
	omt_FFEffectTypeList	*fftype_list;
	OMediaInputEngine		*its_engine;
};

typedef list<OMediaHumanInterfaceDevice*> omt_HIDList;



//********* Input Engine

class OMediaInputEngine : public OMediaEngine
{
	public:

	// * Construction

	omtshared OMediaInputEngine(omt_EngineID id, OMediaWindow *master_win);	  
	omtshared virtual ~OMediaInputEngine();

	// * Mouse

	omtshared virtual bool mouse_down(void);
	omtshared virtual void get_mouse_position(short &x, short &y);

	inline void set_mouse_offset(short mx, short my) {mouse_offsetx = mx; mouse_offsety = my;}
	inline short get_mouse_offset_x(void) const {return mouse_offsetx;}
	inline short get_mouse_offset_y(void) const {return mouse_offsety;}

	// * Command keys
	
	omtshared virtual omt_CommandKey get_command_keys_status(void);
	

	// * Direct system mouse (mouse offset is not applied!)

	omtshared virtual void get_sysmouse_position(short &x, short &y);

	// * Human interface devices

	inline omt_HIDList *get_hid(void) {return &hdi_list;}
	
	
	//----- Low-level -----
	
	protected:
	
	short				mouse_offsetx,mouse_offsety;
	omt_HIDList			hdi_list;

};


#endif

