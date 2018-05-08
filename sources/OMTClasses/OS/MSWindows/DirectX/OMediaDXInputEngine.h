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

#pragma once

#ifndef OMEDIA_DXInputDriver_H
#define OMEDIA_DXInputDriver_H

#include "OMediaWinInputEngine.h"
#include "OMediaPeriodical.h"


#include <DInput.h>


class OMediaDXHIDElement : public OMediaHIDElement
{
	public:
	
	omtshared OMediaDXHIDElement(OMediaHumanInterfaceDevice *device);

	omtshared virtual void read_data(OMediaHIDElementData &data);

	BYTE	dxinstance;
	GUID	guid;
	long	data_offset;
	float	axis_range[2];
	float	axis_middle;
};

class OMediaDXHID : public OMediaHumanInterfaceDevice,
					public OMediaPeriodical
{
	public:
	
	omtshared OMediaDXHID(OMediaInputEngine *engine);
	omtshared virtual ~OMediaDXHID();
	
	omtshared virtual omt_HIDElementList *lock(void);
	omtshared virtual void unlock(void);

	omtshared virtual void spend_time(void);	

	omtshared virtual void start_force_feedback(void);
	omtshared virtual void stop_force_feedback(void);


	GUID					guid;
	LPDIRECTINPUTDEVICE2	dxdevice;
	long					datasize;
	DIDATAFORMAT			dformat;
	unsigned char			*data_buffer;
};


class OMediaDXInputEngine : public OMediaWinInputEngine
{
	public:

	// * Construction

	omtshared OMediaDXInputEngine(OMediaWindow *master_window);	  
	omtshared virtual ~OMediaDXInputEngine();

	LPDIRECTINPUT	dxbase;

	protected:
	
	virtual void init_hid(void);
};


#endif
#endif

