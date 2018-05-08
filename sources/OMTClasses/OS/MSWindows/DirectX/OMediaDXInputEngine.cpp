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
#ifdef omd_ENABLE_DIRECTINPUT

#include "OMediaDXInputEngine.h"
#include "OMediaWinStoreStartInfo.h"
#include "OMediaError.h"
#include "OMediaWinRtgWindow.h"
#include "OMediaWindow.h"


static BOOL CALLBACK DIEnumDeviceObjectsProc(LPCDIDEVICEOBJECTINSTANCE lpddoi,LPVOID pvRef);
static BOOL CALLBACK DIEnumDevicesProc(LPCDIDEVICEINSTANCE lpddi,LPVOID pvRef); 
static HRESULT IDirectInput_CreateDevice2(LPDIRECTINPUT pdi, 
                                   REFGUID rguid, 
                                   LPDIRECTINPUTDEVICE2 *ppdev2, 
                                   LPUNKNOWN punkOuter);


OMediaDXInputEngine::OMediaDXInputEngine(OMediaWindow *master_window):
						OMediaWinInputEngine(master_window,true,ommeic_DirectX)
{
	HRESULT	res;

	dxbase = omc_NULL;
	res = DirectInputCreate(OMediaWinStoreStartInfo::theInstance,
							DIRECTINPUT_VERSION,
							&dxbase,
							omc_NULL);

	if (res!=DI_OK) omd_OSEXCEPTION(res);

	init_hid();
}

OMediaDXInputEngine::~OMediaDXInputEngine()
{
	while(hdi_list.size())
	{
		delete *(hdi_list.begin());
		hdi_list.erase(hdi_list.begin());
	}

	if (dxbase) dxbase->Release();
}

void OMediaDXInputEngine::init_hid(void)
{
	// Use Windows API for keyboard
	OMediaWinInputEngine::init_hid();

	// Enumerate devices
	dxbase->EnumDevices(0L, DIEnumDevicesProc, this, DIEDFL_ATTACHEDONLY); 

}

 
static BOOL CALLBACK DIEnumDevicesProc(LPCDIDEVICEINSTANCE lpddi,LPVOID pvRef) 
{
	OMediaDXInputEngine	*driver = (OMediaDXInputEngine*)pvRef;
	OMediaDXHID			*dxhid;

	dxhid = new OMediaDXHID(driver);

	dxhid->product_name	 = lpddi->tszProductName;
	dxhid->instance_name = lpddi->tszInstanceName;
	dxhid->guid			 = lpddi->guidInstance;

	switch(GET_DIDEVICE_TYPE(lpddi->dwDevType))
	{
		case DIDEVTYPE_MOUSE:
		dxhid->type = omhidtc_Mouse;
		break;

		case DIDEVTYPE_JOYSTICK:
		switch(GET_DIDEVICE_SUBTYPE(lpddi->dwDevType))
		{
			case DIDEVTYPEJOYSTICK_WHEEL:
			dxhid->type = omhidtc_Wheel;
			break;

			case DIDEVTYPEJOYSTICK_HEADTRACKER:
			dxhid->type = omhidtc_Headtracker;
			break;

			case DIDEVTYPEJOYSTICK_RUDDER:
			dxhid->type = omhidtc_Rudder;
			break;
	
			default:
			dxhid->type = omhidtc_Joystick;
			break;
		}
		break;

		default:
		delete dxhid;
		return DIENUM_CONTINUE; 
	}

	dxhid->hdi_usage.usage_page = (omt_HIDUsagePage)lpddi->wUsagePage;
	dxhid->hdi_usage.desktop_type = (omt_HIDGenericDesktopUsage)lpddi->wUsage;

	LPDIRECTINPUTDEVICE2	ppdev2 = omc_NULL;
	HRESULT res = IDirectInput_CreateDevice2(driver->dxbase, 
											 dxhid->guid,
											 &ppdev2,
											 omc_NULL);

	if (res!=DI_OK) omd_OSEXCEPTION(res);


	DIDEVCAPS	caps;
	caps.dwSize = sizeof(DIDEVCAPS);

	res = ppdev2->GetCapabilities(&caps);
	if (res!=DI_OK) omd_OSEXCEPTION(res);

	dxhid->flags = (caps.dwFlags&DIDC_FORCEFEEDBACK)?omhidf_ForceFeedback:0;

	ppdev2->Release();

	driver->get_hid()->push_back(dxhid);

	return DIENUM_CONTINUE; 
}


static HRESULT IDirectInput_CreateDevice2(LPDIRECTINPUT pdi, 
                                   REFGUID rguid, 
                                   LPDIRECTINPUTDEVICE2 *ppdev2, 
                                   LPUNKNOWN punkOuter) 
{ 
    LPDIRECTINPUTDEVICE pdev;     
	HRESULT hres;  

    hres = pdi->CreateDevice(rguid, &pdev, punkOuter); 
    if (SUCCEEDED(hres)) 
	{ 
	    hres = pdev->QueryInterface(IID_IDirectInputDevice2, (LPVOID *)ppdev2);
		IDirectInputDevice_Release(pdev);     
	} 
	else 
	{    
		*ppdev2 = omc_NULL;     
	} 
    return hres; 
} 


//--------------------------------------------------------------

OMediaDXHID::OMediaDXHID(OMediaInputEngine *engine) : OMediaHumanInterfaceDevice(engine) 
{
	dxdevice = omc_NULL;
	data_buffer = omc_NULL;
	datasize=0;
	dformat.rgodf = omc_NULL;
}

OMediaDXHID::~OMediaDXHID() 
{
	while(lock_count) unlock();
}
	
omt_HIDElementList *OMediaDXHID::lock(void)
{
	if ((lock_count++)>0) return element_list;

	OMediaDXInputEngine	*driver = (OMediaDXInputEngine*)its_engine;


	element_list = new omt_HIDElementList;

	HRESULT res = IDirectInput_CreateDevice2(driver->dxbase, 
											 guid,
											 &dxdevice,
											 omc_NULL);

	if (res!=DI_OK) omd_OSEXCEPTION(res);

	omt_RTGDefineLocalTypeObj(OMediaWinRtgWindow,rtg,its_engine->get_supervisor_window());	

	res = dxdevice->SetCooperativeLevel(rtg->hwnd,DISCL_EXCLUSIVE|DISCL_FOREGROUND);
	if (res!=DI_OK) omd_OSEXCEPTION(res);

	res = dxdevice->EnumObjects(DIEnumDeviceObjectsProc, this, DIDFT_ALL);  
	if (res!=DI_OK) omd_OSEXCEPTION(res);

	// Prepare data buffer

	LPDIOBJECTDATAFORMAT	oformat;
	long					p;
	omt_HIDElementList::iterator i;
	
	datasize=0;
	p = 0;

	oformat = new DIOBJECTDATAFORMAT[element_list->size()];

	// First check 4 bytes aligned data

	for(i=element_list->begin();
		i!=element_list->end();
		i++)
	{
		OMediaDXHIDElement	*dxe;

		dxe = (OMediaDXHIDElement*)(*i);

		if (dxe->guid==GUID_Button) continue;
		
		dxe->data_offset = datasize;

		oformat[p].pguid = 0;
		oformat[p].dwOfs = datasize;
		oformat[p].dwFlags = 0;

		if (dxe->guid==GUID_POV)			
			oformat[p].dwType = DIDFT_POV|DIDFT_MAKEINSTANCE(dxe->dxinstance);
		else
			oformat[p].dwType = DIDFT_AXIS|DIDFT_MAKEINSTANCE(dxe->dxinstance);
		
		datasize+=4;
		p++;
	}

	// Now check 1 byte data

	for(i=element_list->begin();
		i!=element_list->end();
		i++)
	{
		OMediaDXHIDElement	*dxe;

		dxe = (OMediaDXHIDElement*)(*i);

		if (dxe->guid!=GUID_Button) continue;

		oformat[p].pguid = 0;
		oformat[p].dwOfs = datasize;
		oformat[p].dwType = DIDFT_BUTTON|DIDFT_MAKEINSTANCE(dxe->dxinstance);
		oformat[p].dwFlags = 0;

		dxe->data_offset = datasize;
		datasize++;
		p++;
	}

	if (datasize)
	{
		datasize += 4-(datasize&3);

		// Allocate buffer and set data format

		data_buffer = new unsigned char[datasize];
		ZeroMemory(data_buffer,datasize);

		dformat.dwSize = sizeof(DIDATAFORMAT);
		dformat.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
		dformat.dwFlags = 0;
		dformat.dwDataSize = datasize;
		dformat.dwNumObjs = p;
		dformat.rgodf = oformat;

		res = dxdevice->SetDataFormat(&dformat);

		// Set/get properties

		DIPROPRANGE	diprange;
		DIPROPDWORD	dipdword;

		diprange.diph.dwSize  = sizeof(DIPROPRANGE);
		diprange.diph.dwHeaderSize = sizeof(DIPROPHEADER);

		dipdword.diph.dwSize  = sizeof(DIPROPDWORD);
		dipdword.diph.dwHeaderSize = sizeof(DIPROPHEADER);

		// Set axis mode:
			
		dipdword.diph.dwHow = DIPH_DEVICE;
		dipdword.diph.dwObj = 0;
		dipdword.dwData = (type==omhidtc_Mouse)?DIPROPAXISMODE_REL:DIPROPAXISMODE_ABS;

		res = dxdevice->SetProperty(DIPROP_AXISMODE,&dipdword.diph);

		// Get axis range
		
		diprange.diph.dwHow = DIPH_BYOFFSET;

		for(i=element_list->begin();
			i!=element_list->end();
			i++)
		{
			OMediaDXHIDElement	*dxe;
			dxe = (OMediaDXHIDElement*)(*i);

			diprange.diph.dwObj = dxe->data_offset;

			if(dxe->type==omhideltc_Axis)
			{
				res = dxdevice->GetProperty(DIPROP_RANGE,&diprange.diph);
				dxe->axis_range[0] = (float)diprange.lMin;
				dxe->axis_range[1] = (float)diprange.lMax;
				dxe->axis_middle = (dxe->axis_range[1]-dxe->axis_range[0])/2.0f;
			}
		}
	}

	// Acquire

	res = dxdevice->Acquire();

	return element_list;
}

void OMediaDXHID::unlock(void)
{
	if (lock_count<0)
	{
		lock_count=0;
		return;
	}

	if ((--lock_count)==0)
	{
		stop_force_feedback();

		for (omt_HIDElementList::iterator i = element_list->begin();
			 i!=element_list->end();
			 i++)
		{
			delete *(i);			
		}
	
		delete element_list;
		element_list = omc_NULL;

		dxdevice->Unacquire();
		dxdevice->Release();
		dxdevice = omc_NULL;

		if (datasize)
		{
			delete [] dformat.rgodf;
			dformat.rgodf = omc_NULL;
		}

		delete [] data_buffer;
		data_buffer = omc_NULL;
		datasize=0;
	}
}

void OMediaDXHID::spend_time(void)
{
	if (data_buffer)
	{
		HRESULT	res;

		dxdevice->Poll();
		res = dxdevice->GetDeviceState(datasize, data_buffer);
		if (res!=DI_OK)
		{
			for(long l=0;l<datasize;l++) data_buffer[l] = 0;
			if (res==DIERR_INPUTLOST  || res==DIERR_NOTACQUIRED) dxdevice->Acquire();
		}
	}
}

static BOOL CALLBACK DIEnumDeviceObjectsProc(LPCDIDEVICEOBJECTINSTANCE lpddoi,LPVOID pvRef)
{
	OMediaDXHID	*hid = (OMediaDXHID*)pvRef;
	OMediaDXHIDElement	*e;

	e = new OMediaDXHIDElement(hid);
	e->dxinstance = DIDFT_GETINSTANCE(lpddoi->dwType);
	e->guid = lpddoi->guidType;
	e->axis_type = omhtdat_Null;
	e->hdi_usage.usage_page = (omt_HIDUsagePage)lpddoi->wUsagePage;
	e->hdi_usage.desktop_type = (omt_HIDGenericDesktopUsage)lpddoi->wUsage;

	if (lpddoi->guidType==GUID_XAxis)
	{
		e->axis_type = omhtdat_X;

		if ( ((lpddoi->dwType&DIDFT_RELAXIS) && hid->type==omhidtc_Mouse) ||
			 !(lpddoi->dwType&DIDFT_ABSAXIS))
			e->type = omhideltc_Delta;
		else
			e->type = omhideltc_Axis;
	}
	else if (lpddoi->guidType==GUID_YAxis)
	{
		e->axis_type = omhtdat_Y;

		if ( ((lpddoi->dwType&DIDFT_RELAXIS) && hid->type==omhidtc_Mouse) ||
			 !(lpddoi->dwType&DIDFT_ABSAXIS))
			e->type = omhideltc_Delta;
		else
			e->type = omhideltc_Axis;
	}
	else if (lpddoi->guidType==GUID_ZAxis)
	{
		e->axis_type = omhtdat_Z;

		if ( ((lpddoi->dwType&DIDFT_RELAXIS) && hid->type==omhidtc_Mouse) ||
			 !(lpddoi->dwType&DIDFT_ABSAXIS))
			e->type = omhideltc_Delta;
		else
			e->type = omhideltc_Axis;
	}
	else if (lpddoi->guidType==GUID_RxAxis)
	{
		e->axis_type = omhtdat_rX;

		if ( ((lpddoi->dwType&DIDFT_RELAXIS) && hid->type==omhidtc_Mouse) ||
			 !(lpddoi->dwType&DIDFT_ABSAXIS))
			e->type = omhideltc_Delta;
		else
			e->type = omhideltc_Axis;
	}
	else if (lpddoi->guidType==GUID_RyAxis)
	{
		e->axis_type = omhtdat_rY;

		if ( ((lpddoi->dwType&DIDFT_RELAXIS) && hid->type==omhidtc_Mouse) ||
			 !(lpddoi->dwType&DIDFT_ABSAXIS))
			e->type = omhideltc_Delta;
		else
			e->type = omhideltc_Axis;
	}
	else if (lpddoi->guidType==GUID_RzAxis)
	{
		e->axis_type = omhtdat_rZ;

		if ( ((lpddoi->dwType&DIDFT_RELAXIS) && hid->type==omhidtc_Mouse) ||
			 !(lpddoi->dwType&DIDFT_ABSAXIS))
			e->type = omhideltc_Delta;
		else
			e->type = omhideltc_Axis;
	}
	else if (lpddoi->guidType==GUID_Slider)
	{
		if ( !(lpddoi->dwType&DIDFT_ABSAXIS) ) e->type = omhideltc_Delta;
		else e->type = omhideltc_Axis;
	}
	else if (lpddoi->guidType==GUID_Button)
	{
		e->type = omhideltc_Button;
	}
	else if (lpddoi->guidType==GUID_POV)
	{
		e->type = omhideltc_POV;
	}

	else
	{
		delete e;
		return DIENUM_CONTINUE;
	}

	e->name = lpddoi->tszName;

	hid->get_locked_element_list()->push_back(e);

	return DIENUM_CONTINUE;
}

OMediaDXHIDElement::OMediaDXHIDElement(OMediaHumanInterfaceDevice *device):
					OMediaHIDElement(device)
{
	dxinstance = omc_NULL;
}

void OMediaDXHIDElement::read_data(OMediaHIDElementData &data)
{
	OMediaDXHID	*dxhid = (OMediaDXHID*)its_device;

	unsigned char	*datap = dxhid->data_buffer+data_offset;
	float			f;

	switch(type)
	{
		case omhideltc_Axis:
		f = (float)(*((long*)datap));
		data.axis = ((f-axis_middle)/axis_range[1])*2.0f;
		if (axis_type==omhtdat_Y) data.axis = -data.axis;
		break;

		case omhideltc_Button:
		data.down = ((*((BYTE*)datap))&(1<<7))!=0;
		break;

		case omhideltc_Delta:
		data.delta = (float)(*((long*)datap));
		if (axis_type==omhtdat_Y) data.delta = -data.delta;
		break;

		case omhideltc_POV:
		{
			DWORD l;

			l = (*((long*)datap));
			if (LOWORD(l) == 0xFFFF)
			{
				data.angle = -1.0f;
			}
			else
			{
				data.angle = float(l) * (1.0f/100.0f);
			}
		}
		break;
	}
}



#endif



