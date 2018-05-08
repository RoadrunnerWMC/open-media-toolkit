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


#include "OMediaForceFeedback.h"
#include "OMediaInputEngine.h"

#define omd_FFSECOND 1000000

OMediaFFEffectType::OMediaFFEffectType(OMediaHumanInterfaceDevice *hid)
{
	its_hid			= hid;
	type			= omffetc_Null;
	sub_type		= omffstc_Null;
	dynamic_flags	= static_flags = 0;
}

OMediaFFEffectType::~OMediaFFEffectType()
{
	while(working_effects.getlisteners()->size())
	{
		delete *(working_effects.getlisteners()->begin());
	}
}

OMediaFFEffect *OMediaFFEffectType::create_effect(void)
{
	return NULL;
}

//----------------------------------------------------------------

OMediaFFEffect::OMediaFFEffect(OMediaFFEffectType *type)
{
	its_type = type;

	// Set default values

	duration		= omd_FFINFINITE;
	sample_period	= 0;
	gain			= 1.0;

	// Search for x/y axes
	omt_HIDElementList::iterator i;
	omt_HIDElementList	*elist = its_type->get_hid()->get_locked_element_list();
	
	axes[0] = axes[1] = axes[2] = NULL;

	for(i=elist->begin();i!=elist->end();i++)
	{
		if ((*i)->axis_type==omhtdat_X && !axes[0])			axes[0] = (*i);
		else if ((*i)->axis_type==omhtdat_Y && !axes[1])	axes[1] = (*i);
	}

	direction.set(1.0f,0,0);

	apply_envelope = false;
	envelope.attack_level = 0.0f;
	envelope.attack_time = omd_FFSECOND*1;
	envelope.fade_level = 0.0f;
	envelope.fade_time = omd_FFSECOND*1;

	dirty_flags = 0;

	constant.magnitude = 0.0f;

	ramp.start = 0.0f;
	ramp.end = 1.0f;

	periodic.magnitude = 1.0f;
	periodic.offset = 0.0f;
	periodic.phase = 0.0f;
	periodic.period = omd_FFSECOND/2;
}

OMediaFFEffect::~OMediaFFEffect() {}

void OMediaFFEffect::play(bool solo)
{
}

void OMediaFFEffect::stop(void)
{
}

void OMediaFFEffect::update(void)
{
}

