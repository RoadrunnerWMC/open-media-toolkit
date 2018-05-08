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

#include "OMediaDXForceFeedback.h"
#include "OMediaDXInputEngine.h"

#include "OMediaError.h"

static BOOL CALLBACK DIEnumEffectsProc(LPCDIEFFECTINFO pei, LPVOID pv);

//---------------------------------------------------------------
// HID

void OMediaDXHID::start_force_feedback(void)
{
	if (lock_count==0) omd_STREXCEPTION("Force feedback started on unlocked HID");
	stop_force_feedback(); 

	fftype_list = new omt_FFEffectTypeList;

	dxdevice->EnumEffects(DIEnumEffectsProc,this,DIEFT_ALL);

	DIPROPDWORD DIPropAutoCenter;
	HRESULT		res;
	 
	DIPropAutoCenter.diph.dwSize = sizeof(DIPropAutoCenter);
	DIPropAutoCenter.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	DIPropAutoCenter.diph.dwObj = 0;
	DIPropAutoCenter.diph.dwHow = DIPH_DEVICE;
	DIPropAutoCenter.dwData = DIPROPAUTOCENTER_OFF;

	dxdevice->Unacquire();
	res = dxdevice->SetProperty(DIPROP_AUTOCENTER, &DIPropAutoCenter.diph);
}

void OMediaDXHID::stop_force_feedback(void)
{
	if (!fftype_list) return;

	for(omt_FFEffectTypeList::iterator i = 	fftype_list->begin();
		i != fftype_list->end();
		i++) delete *i;

	delete fftype_list;
	fftype_list = omc_NULL;

	DIPROPDWORD DIPropAutoCenter;
 
	DIPropAutoCenter.diph.dwSize = sizeof(DIPropAutoCenter);
	DIPropAutoCenter.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	DIPropAutoCenter.diph.dwObj = 0;
	DIPropAutoCenter.diph.dwHow = DIPH_DEVICE;
	DIPropAutoCenter.dwData = DIPROPAUTOCENTER_ON;
 
	dxdevice->Unacquire();
	dxdevice->SetProperty(DIPROP_AUTOCENTER, &DIPropAutoCenter.diph);
}

 
static BOOL CALLBACK DIEnumEffectsProc(LPCDIEFFECTINFO pei, LPVOID pv)
{
	OMediaHumanInterfaceDevice	*hid= (OMediaHumanInterfaceDevice*)pv;
	omt_FFEffectType			type;
	omt_FFEffectSubType			sub_type;
	string						name;
	omt_FFEffectTypeFlags		static_flags = 0;
	omt_FFEffectTypeFlags		dynamic_flags = 0;

	if (pei->guid==GUID_ConstantForce) 
		{type=omffetc_ConstantForce; sub_type = omffstc_Standard;}
	
	else if (pei->guid==GUID_RampForce) 
		{type=omffetc_RampForce; sub_type = omffstc_Standard;}

	else if (pei->guid==GUID_Square)
		{type=omffetc_Periodic; sub_type = omffstc_Square;}

	else if (pei->guid==GUID_Sine) 
		{type=omffetc_Periodic; sub_type = omffstc_Sine;}
	
	else if (pei->guid==GUID_Triangle) 
		{type=omffetc_Periodic; sub_type = omffstc_Triangle;}	

	else if (pei->guid==GUID_SawtoothUp)
		{type=omffetc_Periodic; sub_type = omffstc_SawtoothUp;}

	else if (pei->guid==GUID_SawtoothDown)
		{type=omffetc_Periodic; sub_type = omffstc_SawtoothDown;}
	
	else if (pei->guid==GUID_Spring) 
		{type=omffetc_Condition; sub_type = omffstc_Spring;}
	
	else if (pei->guid==GUID_Damper)
		{type=omffetc_Condition; sub_type = omffstc_Damper;}
	
	else if (pei->guid==GUID_Inertia) 
		{type=omffetc_Condition; sub_type = omffstc_Inertia;}
	
	else if (pei->guid==GUID_Friction)
		{type=omffetc_Condition; sub_type = omffstc_Friction;}
	
	else 
	{
		/* .// Disable custom effects
		sub_type = omffstc_Custom;
		if (pei->dwEffType&DIEFT_CONSTANTFORCE ) type = omffetc_ConstantForce;
		else if (pei->dwEffType&DIEFT_CONDITION) type = omffetc_Condition;
		else if (pei->dwEffType&DIEFT_PERIODIC ) type = omffetc_Periodic;
		else if (pei->dwEffType&DIEFT_RAMPFORCE ) type = omffetc_RampForce;
		else */

		return DIENUM_CONTINUE;

	}

	name = pei->tszName;
	if (pei->dwStaticParams&DIEP_DURATION )		static_flags |=omfftf_Duration;
	if (pei->dwStaticParams&DIEP_ENVELOPE )		static_flags |=omfftf_Envelope;
	if (pei->dwStaticParams&DIEP_GAIN )			static_flags |=omfftf_Gain;
	if (pei->dwStaticParams&DIEP_SAMPLEPERIOD ) static_flags |=omfftf_SamplePeriod;

	if (pei->dwDynamicParams&DIEP_DURATION )		dynamic_flags |=omfftf_Duration;
	if (pei->dwDynamicParams&DIEP_ENVELOPE )		dynamic_flags |=omfftf_Envelope;
	if (pei->dwDynamicParams&DIEP_GAIN )			dynamic_flags |=omfftf_Gain;
	if (pei->dwDynamicParams&DIEP_SAMPLEPERIOD )	dynamic_flags |=omfftf_SamplePeriod;

	if (pei->dwEffType&DIEFT_DEADBAND)			 static_flags |=omfftf_DeadBand;
	if (pei->dwEffType&DIEFT_FFATTACK)			 static_flags |=omfftf_EnvAttack;
	if (pei->dwEffType&DIEFT_FFFADE)			 static_flags |=omfftf_EnvFade;
	if (pei->dwEffType&DIEFT_POSNEGCOEFFICIENTS) static_flags |=omfftf_PosNegCoef;
	if (pei->dwEffType&DIEFT_POSNEGSATURATION)	 static_flags |=omfftf_PosNegSat;
	if (pei->dwEffType&DIEFT_SATURATION)		 static_flags |=omfftf_Saturation;

	OMediaDXFFEffectType	*fxtype;




	fxtype = new OMediaDXFFEffectType(hid, type, sub_type, name,static_flags,dynamic_flags);
	fxtype->guid = pei->guid;

	hid->get_fftype_list()->push_back(fxtype);

	return DIENUM_CONTINUE;
}

//------------------------------------------------------
// Effect type

OMediaDXFFEffectType::OMediaDXFFEffectType(OMediaHumanInterfaceDevice *hid,
								   omt_FFEffectType type,
   								   omt_FFEffectSubType	sub_type,
								   string name,
								   omt_FFEffectTypeFlags static_flags,
								   omt_FFEffectTypeFlags dynamic_flags):
							OMediaFFEffectType(hid)
{
	this->type = type;
	this->sub_type = sub_type;
	this->name = name;
	this->static_flags = static_flags;
	this->dynamic_flags = dynamic_flags;
}

OMediaDXFFEffectType::~OMediaDXFFEffectType() {}

OMediaFFEffect *OMediaDXFFEffectType::create_effect(void)
{
	OMediaDXFFEffect	*fx;

	fx = new OMediaDXFFEffect(this);
	working_effects.addlistener(fx);

	return fx;		
}


//------------------------------------------------------
// Effect

OMediaDXFFEffect::OMediaDXFFEffect(OMediaDXFFEffectType *type):
					OMediaFFEffect(type)
{

	di_fx = NULL;
	((OMediaDXHID*)type->get_hid())->dxdevice->CreateEffect(type->guid,NULL,&di_fx,NULL);

	update_dx_parameters(true);
}

OMediaDXFFEffect::~OMediaDXFFEffect()
{
	if (di_fx) di_fx->Release();
}

void OMediaDXFFEffect::play(bool solo)
{
	if (di_fx) 
	{
		HRESULT res;
		res = di_fx->Start(1, (solo)?DIES_SOLO:0L);
	}
}

void OMediaDXFFEffect::stop(void)
{
	if (di_fx) di_fx->Stop();
}

void OMediaDXFFEffect::update(void)
{
	update_dx_parameters(false);
}

void OMediaDXFFEffect::update_dx_parameters(bool full)
{
	HRESULT		res;
	DIEFFECT	fxdef;

	if (!di_fx) return;

	// Try to import default parameters

	DWORD		axes[3];
	LONG		dir[3];
	short		s;

	DIENVELOPE	envelope;

	DICONSTANTFORCE	constant;
	DIRAMPFORCE		ramp;
	DIPERIODIC		periodic;
	DICONDITION		condition;

	envelope.dwSize = sizeof(DIENVELOPE);
	dir[0] = dir[1] = dir[2] = 0;

	ZeroMemory(&fxdef,sizeof(fxdef));
	fxdef.dwSize = sizeof(DIEFFECT);
	fxdef.dwFlags = DIEFF_CARTESIAN|DIEFF_OBJECTOFFSETS;

	s=0;
	if (this->axes[0])
	{
		dir[s] = (long)(this->direction.x*float(0xFFFL));
		axes[s++] = ((OMediaDXHIDElement*)this->axes[0])->data_offset;
	}

	if (this->axes[1])
	{
		dir[s] = (long)(-this->direction.y*float(0xFFFL));
		axes[s++] = ((OMediaDXHIDElement*)this->axes[1])->data_offset;
	}

	if (this->axes[2])
	{
		dir[s] = (long)(this->direction.z*float(0xFFFL));
		axes[s++] = ((OMediaDXHIDElement*)this->axes[2])->data_offset;
	}

	fxdef.cAxes = s;
	fxdef.rgdwAxes = axes;	
	fxdef.rglDirection = dir;

	if (apply_envelope)
	{
		envelope.dwAttackLevel = long(this->envelope.attack_level*10000.0f);
		envelope.dwAttackTime = this->envelope.attack_time;
		envelope.dwFadeLevel = long(this->envelope.fade_level*10000.0f);
		envelope.dwFadeTime = this->envelope.fade_time;

		fxdef.lpEnvelope = &envelope;
	}
	else fxdef.lpEnvelope = NULL;


    fxdef.dwDuration = duration; 
    fxdef.dwSamplePeriod = sample_period;
    fxdef.dwGain = (long)(gain*10000.0f);
    fxdef.dwTriggerButton = DIEB_NOTRIGGER;  

	switch(its_type->get_type())
	{
		case omffetc_ConstantForce:
		fxdef.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
		fxdef.lpvTypeSpecificParams = &constant;
		constant.lMagnitude = (long)(this->constant.magnitude*10000.0f);
		break;

		case omffetc_RampForce:
		fxdef.cbTypeSpecificParams = sizeof(DIRAMPFORCE);
		fxdef.lpvTypeSpecificParams = &ramp;
		ramp.lStart = (long)(this->ramp.start*10000.0f);
		ramp.lEnd = (long)(this->ramp.end*10000.0f);		
		break;

		case omffetc_Periodic:
		fxdef.cbTypeSpecificParams = sizeof(DIPERIODIC);
		fxdef.lpvTypeSpecificParams = &periodic;
		periodic.dwMagnitude = long(this->periodic.magnitude*10000.0f);
		periodic.lOffset = long(this->periodic.offset*10000.0f);
		periodic.dwPhase = long(this->periodic.phase*35999.0f);
		periodic.dwPeriod = long(this->periodic.period);
		break;

		case omffetc_Condition:
		fxdef.cbTypeSpecificParams = sizeof(DICONDITION);
		fxdef.lpvTypeSpecificParams = &condition;
		condition.lOffset = long(this->condition.offset*10000.0f);
		condition.lPositiveCoefficient = long(this->condition.positive_coefficient*10000.0f);
		condition.lNegativeCoefficient = long(this->condition.negative_coefficient*10000.0f);
		condition.dwPositiveSaturation = long(this->condition.positive_saturation*10000.0f);
		condition.dwNegativeSaturation = long(this->condition.negative_saturation*10000.0f);
		condition.lDeadBand = long(this->condition.dead_band*10000.0f);
		break;
	}

	DWORD	flags;

	if (full)
	{
		flags = DIEP_AXES|DIEP_DIRECTION|DIEP_DURATION|DIEP_GAIN|
					DIEP_SAMPLEPERIOD|DIEP_TYPESPECIFICPARAMS;

		if (apply_envelope) flags |= DIEP_ENVELOPE;
	}
	else
	{
		flags = 0;
		if (dirty_flags&omffedf_Duration)		flags |=DIEP_DURATION;
		if (dirty_flags&omffedf_SamplePeriod)	flags |=DIEP_SAMPLEPERIOD;
		if (dirty_flags&omffedf_Gain)			flags |=DIEP_GAIN;
		if (dirty_flags&omffedf_ExtType)		flags |=DIEP_TYPESPECIFICPARAMS;
		if (dirty_flags&omffedf_Axes)			flags |=DIEP_AXES;
		if (dirty_flags&omffedf_Direction)		flags |=DIEP_DIRECTION;

		if (apply_envelope && dirty_flags&omffedf_Envelope) flags |=DIEP_ENVELOPE;

		if (!flags) return;
	}
 
	res = di_fx->SetParameters(&fxdef,flags);
	if (res==DI_DOWNLOADSKIPPED)
	{
		((OMediaDXHID*)its_type->get_hid())->dxdevice->Acquire();
		di_fx->Download();
	}

	dirty_flags = 0;
}

#endif

