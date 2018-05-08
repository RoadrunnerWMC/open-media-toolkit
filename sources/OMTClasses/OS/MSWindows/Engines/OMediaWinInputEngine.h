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
#ifndef OMEDIA_WinInputEngine_H
#define OMEDIA_WinInputEngine_H

#include "OMediaInputEngine.h"


class OMediaWinHIDElement_Key : public OMediaHIDElement
{
	public:
	
	OMediaWinHIDElement_Key(OMediaHumanInterfaceDevice *device):OMediaHIDElement(device) {}

	inline void set_root(OMediaHIDElement *e) {variation_root = e;}

	omtshared virtual void read_data(OMediaHIDElementData &data);

	unsigned char raw_key;
};

class OMediaWinHID_Keyboard : public OMediaHumanInterfaceDevice
{
	public:
	
	omtshared OMediaWinHID_Keyboard(OMediaInputEngine *engine);
	omtshared virtual ~OMediaWinHID_Keyboard();
	
	omtshared virtual omt_HIDElementList *lock(void);
	omtshared virtual void unlock(void);
};


class OMediaWinInputEngine : public OMediaInputEngine
{
	public:

	// * Construction

	omtshared OMediaWinInputEngine(OMediaWindow *master_window, bool quiet_hid =false, omt_EngineID eid = ommeic_OS); 
	omtshared virtual ~OMediaWinInputEngine();

	// * Command keys
	
	omtshared virtual omt_CommandKey get_command_keys_status(void);
	

	// * Direct system mouse (override)

	omtshared virtual bool mouse_down(void);
	omtshared virtual void get_sysmouse_position(short &x, short &y);

	protected:
	
	virtual void init_hid(void);
	
};

#endif

