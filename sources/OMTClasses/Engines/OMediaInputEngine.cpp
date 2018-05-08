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
 
 
#include "OMediaInputEngine.h"
#include "OMediaMouseCursor.h"
#include "OMediaVideoEngine.h"

OMediaInputEngine::OMediaInputEngine(omt_EngineID id, OMediaWindow *master_win):
					OMediaEngine(id,master_win) 
{
	mouse_offsetx = mouse_offsety = 0;
}

OMediaInputEngine::~OMediaInputEngine() 
{
	while(hdi_list.size())
	{
		delete *(hdi_list.begin());
		hdi_list.erase(hdi_list.begin());
	}
}

bool OMediaInputEngine::mouse_down(void) {return false;}
void OMediaInputEngine::get_sysmouse_position(short &x, short &y) {x=y=0;}

void OMediaInputEngine::get_mouse_position(short &x, short &y) 
{
	get_sysmouse_position(x,y);

	x += mouse_offsetx;
	y += mouse_offsety;
}

omt_CommandKey OMediaInputEngine::get_command_keys_status(void)
{
	return 0;
}

// HID

OMediaHumanInterfaceDevice::OMediaHumanInterfaceDevice(OMediaInputEngine *eng) 
{
	lock_count = 0;
	type = omhidtc_NULL;
	flags = 0;
	element_list = NULL;
	fftype_list = NULL;
	its_engine = eng;
}

OMediaHumanInterfaceDevice::~OMediaHumanInterfaceDevice() 
{
}

void OMediaHumanInterfaceDevice::start_force_feedback(void) 
{
}

void OMediaHumanInterfaceDevice::stop_force_feedback(void) 
{
}

omt_HIDElementList *OMediaHumanInterfaceDevice::lock(void)
{
	return NULL;
}

void OMediaHumanInterfaceDevice::unlock(void)
{
}


OMediaHIDElement::OMediaHIDElement(OMediaHumanInterfaceDevice *device)
{
	its_device = device;
	type = omhideltc_NULL;
	variation_root = this;
	axis_type = omhtdat_Null;
}

OMediaHIDElement::~OMediaHIDElement()
{
}

void OMediaHIDElement::read_data(OMediaHIDElementData &data)
{
}






