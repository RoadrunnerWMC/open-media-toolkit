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

#include "OMediaBuildSwitch.h"
#ifdef omd_ENABLE_DIRECTINPUT

#ifndef OMEDIA_DXForceFeedback_H
#define OMEDIA_DXForceFeedback_H

#include "OMediaForceFeedback.h"

#include <dinput.h>

class OMediaDXFFEffectType;

class OMediaDXFFEffect : public OMediaFFEffect
{
	public:

	omtshared OMediaDXFFEffect(OMediaDXFFEffectType *type);
	omtshared virtual ~OMediaDXFFEffect();

	// * Playback

	omtshared virtual void play(bool solo);

	omtshared virtual void stop(void);

	// * Changes have no effect until update method is called

	omtshared virtual void update(void);

	void update_dx_parameters(bool);


	LPDIRECTINPUTEFFECT	di_fx;
};


class OMediaDXFFEffectType : public OMediaFFEffectType
{
	public:

	// * Construction

	omtshared OMediaDXFFEffectType(OMediaHumanInterfaceDevice *hid,
								   omt_FFEffectType		type,
								   omt_FFEffectSubType	sub_type,
								   string name,
								   omt_FFEffectTypeFlags static_flags,
								   omt_FFEffectTypeFlags dynamic_flags);

	omtshared virtual ~OMediaDXFFEffectType();

	omtshared virtual OMediaFFEffect *create_effect(void);


	GUID guid;
};



#endif
#endif



