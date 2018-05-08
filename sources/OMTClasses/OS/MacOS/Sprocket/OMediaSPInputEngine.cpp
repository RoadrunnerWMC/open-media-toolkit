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
 
 #include "OMediaBuildSwitch.h"
 
#ifdef omd_ENABLE_INPUTSPROCKET

#include "OMediaSPInputEngine.h"
#include "OMediaError.h"
#include "OMediaString.h"

#include <vector>

static char *omf_ptocstr(unsigned char *pstr)
{
	static char cstr[256];

	BlockMove(pstr+1,cstr,pstr[0]);
	cstr[pstr[0]] = 0;

	return cstr;
}

static short omsp_MouseCount, omsp_JoystickCount,
			 omsp_WheelCount,omsp_PedalsCount,omsp_LeversCount;


//--------------------------------------------
// Driver

OMediaSPInputEngine::OMediaSPInputEngine(OMediaWindow *master): OMediaMacInputEngine(master,true,ommeic_Sprocket)
{
	init_hid();
}

OMediaSPInputEngine::~OMediaSPInputEngine()
{
}

void OMediaSPInputEngine::init_hid(void)
{
	OSStatus err;
	ISpDeviceReference		device;
	short					i;
	vector<OMediaSPHID*>	local_hidlist;
	ISpDeviceDefinition device_def;
	vector<OMediaSPHID*>::iterator di;

	OMediaMacInputEngine::init_hid();	// Let Mac driver initialize keyboard device
	
	// Get Sprocket global element list

	spelement_list = 0;
	err = ISpGetGlobalElementList(&spelement_list);
	if (err!=noErr) omd_OSEXCEPTION(err);
	
	err = ISpElementList_Extract(	spelement_list,
									omt_SPMaxGlobalElements, 
									&spnum_elements,
									spelement_buffer);

	if (err!=noErr) omd_OSEXCEPTION(err);

	omsp_MouseCount = omsp_JoystickCount = 
	 omsp_WheelCount = omsp_PedalsCount = omsp_LeversCount = 0;

	for(i=0;i<spnum_elements;i++)
	{
		ISpElement_GetDevice(spelement_buffer[i], &device);
		
		// First check that device is not already in list
		
		for(di = local_hidlist.begin();
			di!=local_hidlist.end();
			di++)
		{
			if ( (*di)->device==device) break;
		}
		
		if (di==local_hidlist.end())
		{
			// It's a new device, add it if supported

			ISpDevice_GetDefinition(device, sizeof(ISpDeviceDefinition), &device_def);
			
			switch(device_def.theDeviceClass)
			{
				case kISpDeviceClass_Mouse:
				case kISpDeviceClass_Joystick:
				case kISpDeviceClass_Wheel:
				case kISpDeviceClass_Pedals:
				case kISpDeviceClass_Levers:
				case kISpDeviceClass_Gamepad:
				{
					OMediaSPHID *hid;		
					hid = new OMediaSPHID(device,this);
					local_hidlist.push_back(hid);				
				}
				break;
			}		
		}
	}
	
	// Move devices from local to global list

	for(vector<OMediaSPHID*>::iterator di = local_hidlist.begin();
			di!=local_hidlist.end();
			di++)
	{
		hdi_list.push_back(*di);
	}
}

//--------------------------------------------
// Device	

OMediaSPHID::OMediaSPHID(ISpDeviceReference init_device, OMediaInputEngine *engine) : OMediaHumanInterfaceDevice(engine)
{
	ISpDeviceDefinition device_def;

	device = init_device;
	lock_count = 0;
	element_list = NULL;

	ISpDevice_GetDefinition(device, sizeof(ISpDeviceDefinition), &device_def);
	
	// Type and instance

	switch(device_def.theDeviceClass)
	{
		case kISpDeviceClass_Mouse:
		type = omhidtc_Mouse;
		omsp_MouseCount++;
		instance_name  = "Mouse ";
		instance_name += omd_L2STR(omsp_MouseCount);
		hdi_usage.usage_page = omhidupc_GenericDesktopControl;
		hdi_usage.desktop_type = omhidgduc_Mouse;
		break;

		case kISpDeviceClass_Joystick:
		case kISpDeviceClass_Gamepad:
		type = omhidtc_Joystick;
		omsp_JoystickCount++;
		instance_name  = "Joystick "; 
		instance_name += omd_L2STR(omsp_JoystickCount);
		hdi_usage.usage_page = omhidupc_GenericDesktopControl;
		hdi_usage.desktop_type = omhidgduc_Joystick;
		break;

		case kISpDeviceClass_Wheel:
		type = omhidtc_Wheel;
		omsp_WheelCount++;
		instance_name  = "Wheel ";
		instance_name += omd_L2STR(omsp_WheelCount);
		hdi_usage.usage_page = omhidupc_GenericDesktopControl;
		hdi_usage.desktop_type = omhidgduc_Wheel;
		break;

		case kISpDeviceClass_Pedals:
		type = omhidtc_Pedals;
		omsp_PedalsCount++;
		instance_name  = "Pedal ";
		instance_name += omd_L2STR(omsp_PedalsCount);
		hdi_usage.usage_page = omhidupc_SimulationControl;
		hdi_usage.simulation_type = omhidscuc_Accelerator;
		break;

		case kISpDeviceClass_Levers:
		type = omhidtc_Levers;
		omsp_LeversCount++;
		instance_name  = "Lever ";
		instance_name += omd_L2STR(omsp_LeversCount);
		hdi_usage.usage_page = omhidupc_SimulationControl;
		hdi_usage.simulation_type = omhidscuc_Trigger;
		break;
	}		
	
	// Flags
	
	flags = 0;
	
	// Product string

	product_name = omf_ptocstr(device_def.deviceName);
	
}

OMediaSPHID::~OMediaSPHID()
{
	while(lock_count) unlock();
}
	
omt_HIDElementList *OMediaSPHID::lock(void)
{
	short i;

	if ((lock_count++)>0) return element_list;

	OMediaSPInputEngine	*driver = (OMediaSPInputEngine*) get_engine();

	if (type==omhidtc_Mouse) 
	{
		ISpDevices_Activate(1,&device); 
	}
	
	// Create element list


	element_list = new omt_HIDElementList;

	// Search elements that are linked to this device

	for(i=0;i<driver->spnum_elements;i++)
	{
		ISpDeviceReference	e_device;
	
		ISpElement_GetDevice(driver->spelement_buffer[i], &e_device);	
		if (e_device==device)
		{
			// Ok this element should be added to this device
		
			OMediaSPHIDElement	*el;
			el = new OMediaSPHIDElement(this,driver->spelement_buffer[i]);
			element_list->push_back(el);
		}
	}

	return element_list;
}

void OMediaSPHID::unlock(void)
{
	if (lock_count<0)
	{
		lock_count=0;
		return;
	}


	if ((--lock_count)==0)
	{	
		if (type==omhidtc_Mouse) 
		{
			OMediaSPInputEngine	*driver = (OMediaSPInputEngine*) get_engine();

			ISpDevices_Deactivate(1,&device);
		}

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


//--------------------------------------------
// Element

OMediaSPHIDElement::OMediaSPHIDElement(OMediaHumanInterfaceDevice *device,
					   ISpElementReference init_element):OMediaHIDElement(device) 
{
	axis_symetric = true;
	sp_element = init_element;
	ISpElementInfo info;
	ISpElement_GetInfo(sp_element, &info);

	switch(info.theKind)
	{
		case kISpElementKind_Button:
		type = omhideltc_Button;
		break;

		case kISpElementKind_DPad:
		type = omhideltc_DPad;
		break;

		case kISpElementKind_Axis:
		{
			type = omhideltc_Axis;
			ISpAxisConfigurationInfo	axis_info;
			ISpElement_GetConfigurationInfo(sp_element,sizeof(ISpAxisConfigurationInfo),
											&axis_info);
											
			axis_symetric = axis_info.symetricAxis;
		}
		break;
	
		case kISpElementKind_Delta:
		type = omhideltc_Delta;
		break;
		
		case kISpElementKind_Movement:
		type = omhideltc_Movement;
		break;
	}

	axis_type = omhtdat_Null;

	switch(info.theLabel)
	{
		case kISpElementLabel_Axis_XAxis:
		hdi_usage.usage_page = omhidupc_GenericDesktopControl;
		hdi_usage.desktop_type = omhidgduc_X;
		axis_type = omhtdat_X;
		break;

		case kISpElementLabel_Axis_YAxis:
		hdi_usage.usage_page = omhidupc_GenericDesktopControl;
		hdi_usage.desktop_type = omhidgduc_Y;
		axis_type = omhtdat_Y;
		break;

		case kISpElementLabel_Axis_ZAxis:
		hdi_usage.usage_page = omhidupc_GenericDesktopControl;
		hdi_usage.desktop_type = omhidgduc_Z;
		axis_type = omhtdat_Z;
		break;

		case kISpElementLabel_Axis_Rx:
		hdi_usage.usage_page = omhidupc_GenericDesktopControl;
		hdi_usage.desktop_type = omhidgduc_Rx;
		axis_type = omhtdat_rX;
		break;

		case kISpElementLabel_Axis_Ry:
		hdi_usage.usage_page = omhidupc_GenericDesktopControl;
		hdi_usage.desktop_type = omhidgduc_Ry;
		axis_type = omhtdat_rY;
		break;

		case kISpElementLabel_Axis_Rz:
		hdi_usage.usage_page = omhidupc_GenericDesktopControl;
		hdi_usage.desktop_type = omhidgduc_Rx;
		axis_type = omhtdat_rZ;
		break;

		case kISpElementLabel_Axis_RollTrim:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_AxisRollTrim;
		break;

		case kISpElementLabel_Axis_PitchTrim:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_AxisPitchTrim;
		break;

		case kISpElementLabel_Axis_YawTrim:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_AxisYawTrim;
		break;

		case kISpElementLabel_Axis_Gas:
		hdi_usage.usage_page = omhidupc_SimulationControl;
		hdi_usage.simulation_type = omhidscuc_Accelerator;
		break;

		case kISpElementLabel_Axis_Brake:
		hdi_usage.usage_page = omhidupc_SimulationControl;
		hdi_usage.simulation_type = omhidscuc_Brake;
		break;

		case kISpElementLabel_Axis_Clutch:
		hdi_usage.usage_page = omhidupc_SimulationControl;
		hdi_usage.simulation_type = omhidscuc_Clutch;
		break;

		case kISpElementLabel_Axis_Throttle:
		hdi_usage.usage_page = omhidupc_SimulationControl;
		hdi_usage.simulation_type = omhidscuc_Throttle;
		break;

		case kISpElementLabel_Axis_Rudder:
		hdi_usage.usage_page = omhidupc_SimulationControl;
		hdi_usage.simulation_type = omhidscuc_Rudder;
		break;

		case kISpElementLabel_Delta_X:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_DeltaX;
		break;

		case kISpElementLabel_Delta_Y:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_DeltaY;
		break;

		case kISpElementLabel_Delta_Z:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_DeltaZ;
		break;

		case kISpElementLabel_Delta_Rx:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_DeltaRx;
		break;

		case kISpElementLabel_Delta_Ry:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_DeltaRy;
		break;

		case kISpElementLabel_Delta_Rz:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_DeltaRz;
		break;

		case kISpElementLabel_Pad_Move:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_PadMove;
		break;

		case kISpElementLabel_Btn_Fire:
		hdi_usage.usage_page = omhidupc_GameControl;
		hdi_usage.game_type = omhidgmuc_GamepadFireJump;
		break;

		case kISpElementLabel_Btn_SecondaryFire:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_SecondaryFire;
		break;
		
		case kISpElementLabel_Btn_Jump:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_Jump;
		break;

		case kISpElementLabel_Btn_Select:
		hdi_usage.usage_page = omhidupc_GenericDesktopControl;
		hdi_usage.desktop_type = omhidgduc_Select;
		break;

		case kISpElementLabel_Btn_SlideLeft:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_SlideLeft;
		break;

		case kISpElementLabel_Btn_SlideRight:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_SlideRight;
		break;

		case kISpElementLabel_Btn_MoveForward:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_MoveForward;
		break;

		case kISpElementLabel_Btn_MoveBackward:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_MoveBackward;
		break;

		case kISpElementLabel_Btn_TurnLeft:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_TurnLeft;
		break;

		case kISpElementLabel_Btn_TurnRight:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_TurnRight;
		break;
		
		case kISpElementLabel_Btn_LookLeft:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_LookLeft;
		break;

		case kISpElementLabel_Btn_LookRight:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_LookRight;
		break;

		case kISpElementLabel_Btn_LookUp:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_LookUp;
		break;

		case kISpElementLabel_Btn_LookDown:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_LookDown;
		break;

		case kISpElementLabel_Btn_Next:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_Next;
		break;

		case kISpElementLabel_Btn_Previous:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_Previous;
		break;

		case kISpElementLabel_Btn_SideStep:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_SideStep;
		break;

		case kISpElementLabel_Btn_Run:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_Run;
		break;

		case kISpElementLabel_Btn_Look:
		hdi_usage.usage_page = omhidupc_OMTControl;
		hdi_usage.omt_type = omhidomuc_Look;
		break;

		
		default:
		hdi_usage.usage_page = omhidupc_Undefined;
		hdi_usage.unknown_type = 0;
		break;
	}

	name = omf_ptocstr(info.theString);
}


void OMediaSPHIDElement::read_data(OMediaHIDElementData &data)
{
	UInt32 state = 0;	

	switch (type)
	{
		case omhideltc_Button:	
		ISpElement_GetSimpleState(sp_element, &state);
		data.down = state!=0;
		break;
		
		case omhideltc_Delta:
		ISpElement_GetSimpleState(sp_element, &state);
		data.delta = float(long(state))/float(0x147);
		break;
		
		case omhideltc_Axis:
		ISpElement_GetSimpleState(sp_element, &state);
		if (axis_symetric)
			data.axis = ((float(state)-float(kISpAxisMiddle))/float(kISpAxisMaximum))*2.0f;
		else
			data.axis = float(state)/float(kISpAxisMaximum);
		break;

		case omhideltc_DPad:
		ISpElement_GetSimpleState(sp_element, &state);
		data.pad_data = omt_HIDPadData(state);
		break;

		case omhideltc_Movement:
		{
			ISpMovementData	mov_data;
			
			ISpElement_GetComplexState(sp_element,sizeof(ISpMovementData),&mov_data);
			data.mov_data.x_axis = ((float(mov_data.xAxis)-float(kISpAxisMiddle))/float(kISpAxisMaximum))*2.0f;
			data.mov_data.y_axis = ((float(mov_data.yAxis)-float(kISpAxisMiddle))/float(kISpAxisMaximum))*2.0f;	
		}
		break;
	}
}


#endif

