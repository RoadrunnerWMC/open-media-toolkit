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
#ifndef OMEDIA_ForceFeedback_H
#define OMEDIA_ForceFeedback_H

#include <vector>
#include <string>

#include "OMediaRetarget.h"
#include "OMediaListener.h"
#include "OMediaBroadcaster.h"
#include "OMedia3DVector.h"


class OMediaHumanInterfaceDevice;
class OMediaFFEffect;

#define omd_FFINFINITE 0xFFFFFFFF

//------------------------------------------

enum omt_FFEffectType
{
	omffetc_Null,

	omffetc_ConstantForce,	// Constant force, OMediaFFConstant
	omffetc_RampForce,		// Ramp force, OMediaFFRamp
	omffetc_Periodic,		// Periodic effect, OMediaFFPeriodic
	omffetc_Condition		// Conditions, OMediaFFCondition
};

enum omt_FFEffectSubType
{
	omffstc_Null,

	// Standard constant or ramp force

	omffstc_Standard,

	// Periodic

	omffstc_Square,
	omffstc_Sine,			
	omffstc_Triangle,
	omffstc_SawtoothUp,
	omffstc_SawtoothDown,

	// Condition

	omffstc_Friction,
	omffstc_Damper,	
	omffstc_Inertia,	
	omffstc_Spring,

	// Custom (predefined custom force)

	omffstc_Custom
};


//------------------------------------------
// Constant force

class OMediaFFConstant
{
	public:
	
	float magnitude;		// -1.0 to 1.0
};

//------------------------------------------
// Ramp force

class OMediaFFRamp
{
	public:
	
	float start,end;		// -1.0 to 1.0
};

//------------------------------------------
// Periodic effect

class OMediaFFPeriodic
{
	public:
    float			magnitude;		// 0.0 to 1.0
    float			offset;

	float			phase;			// 0.0 to 1.0
	unsigned long	period;			// microsecs

};

//------------------------------------------
// Condition effect

class OMediaFFCondition
{
	public:
    float offset;				// -1.0 to 1.0

	float positive_coefficient;	// -1.0 to 1.0
	float negative_coefficient;	// -1.0 to 1.0
	
	float positive_saturation;	// 0.0 to 1.0
	float negative_saturation;	// 0.0 to 1.0

	float dead_band;
};

//------------------------------------------
// Envelope

class OMediaFFEnvelope
{
	public:

	float			attack_level;	// Envelope attack level: 0.0 to 1.0
	unsigned long	attack_time;	// Envelope attack time. (microsecs)
	float			fade_level;		// Fade level: 0.0 to 1.0
	unsigned long	fade_time;		// Fade time. (microsecs)
};


//------------------------------------------
// Internal stuff

typedef unsigned short omt_FFEffectDirtyFlags;
const omt_FFEffectDirtyFlags omffedf_Duration		= (1<<0);
const omt_FFEffectDirtyFlags omffedf_SamplePeriod	= (1<<1);
const omt_FFEffectDirtyFlags omffedf_Gain			= (1<<2);
const omt_FFEffectDirtyFlags omffedf_Envelope		= (1<<3);
const omt_FFEffectDirtyFlags omffedf_ExtType		= (1<<4);
const omt_FFEffectDirtyFlags omffedf_Axes			= (1<<5);
const omt_FFEffectDirtyFlags omffedf_Direction		= (1<<6);

//------------------------------------------
// Effect

// Do not create effect object yourself, use OMediaFFEffectType::create_effect instead

class OMediaFFEffectType;
class OMediaHIDElement;

class OMediaFFEffect : public OMediaListener
{
	public:

	// * Playback

	omtshared virtual void play(bool solo);
	omtshared virtual void stop(void);

	// * Changes have no effect until update method is called

	omtshared virtual void update(void);

	// * Specify axes

	inline void set_axes(OMediaHIDElement *x,	// Pass NULL for unwanted axis.
						 OMediaHIDElement *y,	// Typically you pass only x and y.
						 OMediaHIDElement *z) 
						{axes[0]=x;axes[1]=y;axes[2]=z;dirty_flags|=omffedf_Axes;}

	inline OMediaHIDElement *get_axis_x(void) const {return axes[0];} 
	inline OMediaHIDElement *get_axis_y(void) const {return axes[1];} 
	inline OMediaHIDElement *get_axis_z(void) const {return axes[2];} 

	// * Direction

	// Use a normalized vector

	inline void set_direction(const OMedia3DVector &v) {direction = v; dirty_flags|=omffedf_Direction;}
	inline void get_direction(OMedia3DVector &v) const {v = direction;}


	// * Force duration. omd_FFINFINITE for infinite (microsecs)

	inline void set_duration(const unsigned long d) {duration = d;dirty_flags|=omffedf_Duration;}
	inline unsigned long get_duration(void) const {return duration;}

	// * The period at which the device should play back the effect. Zero for default. (microsecs)

	inline void set_sample_period(const unsigned long sp) {sample_period = sp;dirty_flags|=omffedf_SamplePeriod;}
	inline unsigned long get_sample_period(void) const {return sample_period;}

	// * Scaling factor. 0.0 to 1.0

	inline void set_gain(const float g) {gain = g; dirty_flags|=omffedf_Gain;}
	inline float get_gain(void) const {return gain;}

	// * Envelope

	inline void set_envelope(bool on) {apply_envelope = on; dirty_flags|=omffedf_Envelope;}
	inline bool get_envelope(void) const {return apply_envelope;}

	inline void set_envelope_data(const OMediaFFEnvelope &e) {envelope  = e; dirty_flags|=omffedf_Envelope;}
	inline void get_envelope_data(OMediaFFEnvelope &e) const {e = envelope;}

	// * Specific data

	// Constant force

	inline void set_constant(const OMediaFFConstant &c) {constant = c; dirty_flags|=omffedf_ExtType;} 
	inline void get_constant(OMediaFFConstant &c) const {c = constant;}

	// Ramp force

	inline void set_ramp(const OMediaFFRamp &r) {ramp = r; dirty_flags|=omffedf_ExtType;} 
	inline void get_ramp(OMediaFFRamp &r) const {r = ramp;}

	// Periodic effect

	inline void set_periodic(const OMediaFFPeriodic &p) {periodic = p; dirty_flags|=omffedf_ExtType;} 
	inline void get_periodic(OMediaFFPeriodic &p) const {p = periodic;}

	// Condition

	inline void set_condition(const OMediaFFCondition &c) {condition = c; dirty_flags|=omffedf_ExtType;} 
	inline void get_condition(OMediaFFCondition &c) const {c = condition;}


	protected:

	omtshared OMediaFFEffect(OMediaFFEffectType *type);
	omtshared virtual ~OMediaFFEffect();

	OMediaFFEffectType	*its_type;

	unsigned long	duration;		
	unsigned long	sample_period;	
									
	float			gain;			

	OMediaHIDElement	*axes[3];
	OMedia3DVector		direction;

	// Envelope

	bool				apply_envelope;		
	OMediaFFEnvelope	envelope;


	omt_FFEffectDirtyFlags	dirty_flags;

	// Effect type specific data

	union
	{
		OMediaFFConstant	constant;	// Constant force
		OMediaFFRamp		ramp;		// Ramp force
		OMediaFFPeriodic	periodic;	// Periodic effect
		OMediaFFCondition	condition;	// Condition
	};

};



//------------------------------------------

typedef unsigned short omt_FFEffectTypeFlags;
const omt_FFEffectTypeFlags omfftf_Duration		= (1<<0);
const omt_FFEffectTypeFlags omfftf_Envelope		= (1<<1);	
const omt_FFEffectTypeFlags omfftf_Gain			= (1<<2);
const omt_FFEffectTypeFlags omfftf_SamplePeriod	= (1<<3);	
const omt_FFEffectTypeFlags omfftf_EnvAttack	= (1<<4);
const omt_FFEffectTypeFlags omfftf_EnvFade		= (1<<5);
const omt_FFEffectTypeFlags omfftf_PosNegCoef	= (1<<6);	
const omt_FFEffectTypeFlags omfftf_PosNegSat	= (1<<7);
const omt_FFEffectTypeFlags omfftf_Saturation	= (1<<8);
const omt_FFEffectTypeFlags omfftf_DeadBand		= (1<<9);

//------------------------------------------

class OMediaFFEffectType
{
	public:

	// * Construction

	omtshared OMediaFFEffectType(OMediaHumanInterfaceDevice *hid);
	omtshared virtual ~OMediaFFEffectType();

	inline OMediaHumanInterfaceDevice *get_hid(void) const {return its_hid;}

	// * Effect type and name

	inline omt_FFEffectType get_type(void) const {return type;}
	inline omt_FFEffectSubType get_subtype(void) const {return sub_type;}

	inline void get_name(string &n) const {n = name;}

	// * Supported flags (dynamic denotes effect that can be changed while effect is playing)

	inline omt_FFEffectTypeFlags get_static_flags(void) const {return static_flags;}
	inline omt_FFEffectTypeFlags get_dynamic_flags(void) const {return dynamic_flags;}

	// * Create new effect

	omtshared virtual OMediaFFEffect *create_effect(void);

	// Use C++ delete operator to delete effect. Effects are deleted for you
	// when HID is unlocked.

	protected:

	string						name;
	OMediaHumanInterfaceDevice	*its_hid;
	OMediaBroadcaster			working_effects;
	omt_FFEffectType			type;
	omt_FFEffectSubType			sub_type;

	omt_FFEffectTypeFlags		dynamic_flags,static_flags;
};

typedef vector<OMediaFFEffectType*> omt_FFEffectTypeList;


#endif

