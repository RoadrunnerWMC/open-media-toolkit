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
#ifndef OMEDIA_3DMaterial_H
#define OMEDIA_3DMaterial_H

#include "OMediaRGBColor.h"
#include "OMediaTypes.h"
#include "OMediaDBObject.h"
#include "OMediaCanvas.h"
#include "OMediaRendererInterface.h"

enum omt_MaterialLightMode
{
	ommlmc_Color,		// Use material diffuse component as a fixed color (default)
	ommlmc_VertexColor,	// Use colors defined in the 3D shape and indexed in the polygons.
	ommlmc_Light		// Real time light
};


typedef unsigned short omt_MaterialFlags;
const omt_MaterialFlags ommatf_SecondPass 	= (1<<0);	// Objects with this material are drawn
														// during the second rendering pass.
														// You should set this flag if this
														// material is transparent and you
														// plan to render it to a ZBuffered
														// context.

const omt_MaterialFlags ommatf_DisableFog 			= (1<<1);	//+++ Not implemened yet
const omt_MaterialFlags ommatf_DisableCollision 	= (1<<2);	// No collision is detected on this
																// material



// * 3D Material

class OMedia3DMaterial : public OMediaDBObject
{
	public:
	
	// * Construction
	
	omtshared OMedia3DMaterial(void);
	omtshared OMedia3DMaterial(const OMedia3DMaterial &material);
	omtshared virtual ~OMedia3DMaterial(void);	
	
	omtshared virtual void purge(void);
	
	

	// * Flags
	
	inline void set_flags(const omt_MaterialFlags f) {flags = f;}
	inline omt_MaterialFlags get_flags(void) const {return flags;}


	// * Material render mode

	inline void set_shade_mode(const omt_ShadeMode mode) {shade_mode = mode;}
	inline omt_ShadeMode get_shade_mode(void) const {return shade_mode;}

	inline void set_fill_mode(const omt_FillMode mode) {fill_mode = mode;}
	inline omt_FillMode get_fill_mode(void) const {return fill_mode;}
	
	inline void set_light_mode(const omt_MaterialLightMode mode) {light_mode = mode;}
	inline omt_MaterialLightMode get_light_mode(void) const {return light_mode;}

	inline void set_blend(const omt_BlendFunc src_func, omt_BlendFunc dest_func) {src_blend = src_func; dest_blend = dest_func;}
	inline omt_BlendFunc get_blend_src(void) const {return src_blend;}
	inline omt_BlendFunc get_blend_dest(void) const {return dest_blend;}
	
	
	// * Material color

	omtshared void set_color(const OMediaFRGBColor &color,
			 float emission,			// 0.0f - 1.0f
			 float ambient,
			 float diffuse,
			 float specular);


	inline void set_emission(const OMediaFARGBColor &rgb) {emission = rgb;}
	inline void get_emission(OMediaFARGBColor &rgb) const {rgb = emission;}
	inline void set_emission_a(float a) {emission.alpha = a;}
	inline void set_emission_r(float r) {emission.red 	= r;}
	inline void set_emission_g(float g) {emission.green = g;}
	inline void set_emission_b(float b) {emission.blue 	= b;}
	inline float get_emission_a(void) const {return emission.alpha;}
	inline float get_emission_r(void) const {return emission.red;}
	inline float get_emission_g(void) const {return emission.green;}
	inline float get_emission_b(void) const {return emission.blue;}
	inline OMediaFARGBColor &get_emission_ref(void) {return emission;}


	inline void set_diffuse(const OMediaFARGBColor &rgb) {diffuse = rgb;}
	inline void get_diffuse(OMediaFARGBColor &rgb) const {rgb = diffuse;}
	inline void set_diffuse_a(float a) {diffuse.alpha 	= a;}
	inline void set_diffuse_r(float r) {diffuse.red 	= r;}
	inline void set_diffuse_g(float g) {diffuse.green 	= g;}
	inline void set_diffuse_b(float b) {diffuse.blue 	= b;}
	inline float get_diffuse_a(void) const {return diffuse.alpha;}
	inline float get_diffuse_r(void) const {return diffuse.red;}
	inline float get_diffuse_g(void) const {return diffuse.green;}
	inline float get_diffuse_b(void) const {return diffuse.blue;}
	inline OMediaFARGBColor &get_diffuse_ref(void) {return diffuse;}

	inline void set_specular(const OMediaFARGBColor &rgb) {specular = rgb;}
	inline void get_specular(OMediaFARGBColor &rgb) const {rgb = specular;}
	inline void set_specular_a(float a) {specular.alpha = a;}
	inline void set_specular_r(float r) {specular.red 	= r;}
	inline void set_specular_g(float g) {specular.green = g;}
	inline void set_specular_b(float b) {specular.blue 	= b;}
	inline float get_specular_a(void) const {return specular.alpha;}
	inline float get_specular_r(void) const {return specular.red;}
	inline float get_specular_g(void) const {return specular.green;}
	inline float get_specular_b(void) const {return specular.blue;}
	inline OMediaFARGBColor &get_specular_ref(void) {return specular;}

	inline void set_ambient(const OMediaFARGBColor &rgb) {ambient = rgb;}
	inline void get_ambient(OMediaFARGBColor &rgb) const {rgb = ambient;}
	inline void set_ambient_a(float a) {ambient.alpha 	= a;}
	inline void set_ambient_r(float r) {ambient.red 	= r;}
	inline void set_ambient_g(float g) {ambient.green 	= g;}
	inline void set_ambient_b(float b) {ambient.blue 	= b;}
	inline float get_ambient_a(void) const {return ambient.alpha;}
	inline float get_ambient_r(void) const {return ambient.red;}
	inline float get_ambient_g(void) const {return ambient.green;}
	inline float get_ambient_b(void) const {return ambient.blue;}
	inline OMediaFARGBColor &get_ambient_ref(void) {return ambient;}

	inline void set_shininess(const float s) {shininess = s;}		// Power 0-128
	inline float get_shininess(void) const {return shininess;}

	
	// * Texture
	
	omtshared virtual void set_texture(OMediaCanvas *canvas);
	inline OMediaCanvas *get_texture(void) const {return texture;}

	inline void set_texture_address_mode(const omt_TextureAddressMode am)
		{texture_address_mode = am;}

	inline omt_TextureAddressMode get_texture_address_mode(void) const
		{return texture_address_mode;}

	inline void set_texture_color_operation(const omt_TextureColorOperation am)
		{texture_color_operation = am;}

	inline omt_TextureColorOperation get_texture_color_operation(void) const
		{return texture_color_operation;}


	// * Assign
		
	omtshared void assign(const OMedia3DMaterial &m);	
	OMedia3DMaterial &operator=(const OMedia3DMaterial &m) { assign(m); return *this;}
	

	// * Compare

	inline bool operator==(const OMedia3DMaterial &m) const
	{
		return (fill_mode==m.fill_mode &&
				shade_mode==m.shade_mode &&
				light_mode==m.light_mode &&
				src_blend==m.src_blend &&
				dest_blend==m.dest_blend &&
				flags==m.flags &&
				emission==m.emission &&
				diffuse==m.diffuse &&
				specular==m.specular &&
				ambient==m.ambient &&
				shininess==m.shininess &&
				texture==m.texture &&
				texture_address_mode==m.texture_address_mode &&
				texture_color_operation==m.texture_color_operation);
	}

	inline bool operator!=(const OMedia3DMaterial &m) const
	{
		return !((*this)==m);
	}

	// * Database/streamer support
	
	enum { db_type = '3DMa' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void) const;


	protected:
	
	omt_FillMode			fill_mode;
	omt_ShadeMode			shade_mode;
	omt_MaterialLightMode	light_mode;
	omt_BlendFunc			src_blend,dest_blend;
	omt_MaterialFlags		flags;

	OMediaFARGBColor	emission,			// Emission
						diffuse,			// Diffusion
						specular,			// Specular
						ambient;			// Ambiant factor
	
	float				shininess;

	OMediaCanvas				*texture;
	omt_TextureAddressMode		texture_address_mode;
	omt_TextureColorOperation	texture_color_operation;
};



#endif

