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
#ifndef OMEDIA_Light_H
#define OMEDIA_Light_H

#include "OMediaElement.h"
#include "OMedia3DVector.h"
#include "OMedia3DPoint.h"
#include "OMediaRGBColor.h"
#include "OMediaRendererInterface.h"


class OMediaLight : public OMediaElement
{
	public:
	
	// * Constructor/Destructor

	omtshared OMediaLight();
	omtshared virtual ~OMediaLight();	

	omtshared virtual void reset(void);
	

	// * Light type
	
	inline omt_LightType get_light_type(void) const {return light_type;}
	inline void set_light_type(omt_LightType lt) {light_type = lt;}

	// * Layer key

	inline void set_layer_key(unsigned long k) {layer_key = k;}
	inline unsigned long get_layer_key(void) const {return layer_key;}

	// * Color

	inline void set_ambient(const OMediaFARGBColor &argb) {ambient = argb;}
	inline void get_ambient(OMediaFARGBColor &argb) const {argb = ambient;}

	inline void set_diffuse(const OMediaFARGBColor &argb) {diffuse = argb;}
	inline void get_diffuse(OMediaFARGBColor &argb) const {argb = diffuse;}

	inline void set_specular(const OMediaFARGBColor &argb) {specular = argb;}
	inline void get_specular(OMediaFARGBColor &argb) const {argb = specular;}



	// * Attenuation
	
	inline void set_constant_attenuation(const float ca) { const_attenuation = ca;}
	inline float get_constant_attenuation(void) const { return const_attenuation;}

	inline void set_linear_attenuation(const float la) { linear_attenuation = la;}
	inline float get_linear_attenuation(void) const { return linear_attenuation;}

	inline void set_quadratic_attenuation(const float qa) { quadratic_attenuation = qa;}
	inline float get_quadratic_attenuation(void) const { return quadratic_attenuation;}

	inline void set_range(const float r) { range = r;}
	inline float get_range(void) const { return range;}
	

	// * Spot support

	inline void set_spot_cutoff(const omt_Angle cutoff) {spot_cutoff = cutoff;}
	inline omt_Angle get_spot_cutoff(void) const {return spot_cutoff;}

	inline void set_spot_exponent(const float sp_exp) {spot_exponent = sp_exp;}
	inline float get_spot_exponent(void) const {return spot_exponent;}


	// * Unlink (override)
	
	omtshared virtual void unlink(void);


	// * Database/streamer support
	
	enum { db_type = 'Elig' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void);


	protected:

	omtshared virtual void update_world_ptr(OMediaWorld *);

	omt_LightList::iterator					light_node;
	unsigned long							layer_key;
	
	omt_LightType							light_type;
	float									const_attenuation,linear_attenuation,
											quadratic_attenuation,range;

	omt_Angle								spot_cutoff;
	float									spot_exponent;

	OMediaFARGBColor						ambient,diffuse,specular;


	public:
											
	void render_compute_light(OMediaMatrix_4x4 &viewmatrix);
	void render_light(	long index, OMediaRendererInterface *rdr_i);


	unsigned short 							render_count;

	static unsigned short					current_render_count;
	static unsigned long					render_layer_key;

	OMedia3DVector							render_direction;
	OMedia3DPoint							render_position;
};



#endif

