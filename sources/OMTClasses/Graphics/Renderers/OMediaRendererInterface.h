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
#ifndef OMEDIA_RendererInterface_H
#define OMEDIA_RendererInterface_H

#include "OMediaTypes.h"
#include "OMediaRGBColor.h"
#include "OMedia3DPoint.h"
#include "OMedia3DVector.h"
#include "OMediaMatrix.h"
#include "OMedia3DPolygon.h"
#include "OMediaRenderConstants.h"
#include "OMediaExtraTexturePass.h"

class OMediaRect;
class OMedia3DShape;
class OMediaPickRequest;
class OMediaCanvas;

typedef vector<OMedia3DPolygon> 			omt_PolygonList;
typedef vector<OMedia3DPoint> 				omt_VertexList;
typedef vector<OMedia3DVector>				omt_NormalList;
typedef vector<OMediaFARGBColor>			omt_ColorList;


class OMediaRenderVertex : public OMedia3DPoint
{
	public:
	
	OMediaFARGBColor			diffuse;
	OMediaFRGBColor				specular;
	
	float				u,v;
	OMedia3DVector		normal;

	omt_ExtraTexturePassUV		extra_passes_uv;


	OMediaRenderVertex &operator=(const OMediaRenderVertex &p) 
	{
		x = p.x; y = p.y; z = p.z; 
		diffuse = p.diffuse;
		specular = p.specular;
		u = p.u;
		v = p.v;
		normal = p.normal;
		extra_passes_uv = p.extra_passes_uv;
		return *this;
	}

};



class OMediaRendererOverrideVertexList
{
public:

	omt_VertexList		*vertex_list;
	omt_NormalList		*normal_list;				// NULL for shape normals
	omt_ColorList		*color_list;				// NULL for shape colors
};


typedef unsigned short omt_RendererRequirementFlags;
const omt_RendererRequirementFlags		omrrf_ProjectionZRange_0_1 = (1<<0);	// If this flag is set, the renderer
																			// expects a matrix that generate
																			// z values going from 0 to 1. By
																			// default range is -1 to 1.

const omt_RendererRequirementFlags		omrrf_ExactPixelCorrectEven = (1<<1);


typedef vector<OMediaRenderVertex>	omt_RenderVertexList;

class OMediaPickHit
{
	public:
	
	unsigned long		id;
	float				minz, maxz;
};

class OMediaLight;
class OMedia3DMaterial;
class OMediaCanvas;

class OMediaRendererInterface
{
	public:

	// * Requirements

	omtshared virtual omt_RendererRequirementFlags get_requirement_flags(void) =0;

	// * View bounds
	
	omtshared virtual void get_view_bounds(OMediaRect &r) =0;

	// * Buffers

	omtshared virtual void clear_all_buffers(OMediaFARGBColor &rgb) =0;	// Color and ZBufer

	omtshared virtual void clear_colorbuffer(OMediaFARGBColor &rgb, OMediaRect *area =NULL) = 0;
	
	omtshared virtual void clear_zbuffer(OMediaRect *area =NULL) = 0;
	omtshared virtual void set_zbuffer_write(omt_ZBufferWrite zb) =0;
	omtshared virtual void set_zbuffer_test(omt_ZBufferTest zb) =0;
	omtshared virtual void set_zbuffer_func(omt_ZBufferFunc zb) =0;

	// * Clipping rect (scissor)

	omtshared virtual void enable_clipping_rect(bool enable) = 0;	
	omtshared virtual void set_clipping_rect(const OMediaRect &rect) = 0;
	
	// * Matrix
	
	omtshared virtual void set_model_view(OMediaMatrix_4x4 &m) =0;
	omtshared virtual void set_projection(OMediaMatrix_4x4 &m) =0;

	// * Light

	omtshared virtual void enable_lighting(void) =0;
	omtshared virtual void disable_lighting(void) =0;

	omtshared virtual long get_max_lights(void) =0;

	// Any change to one or more lights needs to be enclosed by start/end_light_edit calls.

	omtshared virtual void start_light_edit(void) =0;
	omtshared virtual void end_light_edit(void) =0;

	omtshared virtual void enable_light(long index) =0;
	omtshared virtual void disable_light(long index) =0;

	omtshared virtual void set_light_type(long index, omt_LightType type) =0;
	omtshared virtual void set_light_pos(long index, OMedia3DPoint &p) =0;
	omtshared virtual void set_light_dir(long index, OMedia3DVector &v) =0;

	omtshared virtual void set_light_ambient(long index, OMediaFARGBColor &argb) =0;
	omtshared virtual void set_light_diffuse(long index, OMediaFARGBColor &argb) =0;
	omtshared virtual void set_light_specular(long index, OMediaFARGBColor &argb) =0;

	omtshared virtual void set_light_attenuation(long index, float range, float constant, float linear, float quadratic) =0;

	omtshared virtual void set_light_spot_cutoff(long index, float cutoff) = 0;
	omtshared virtual void set_light_spot_exponent(long index, float expo) = 0;

	omtshared virtual void set_light_global_ambient(OMediaFARGBColor &argb) = 0;


	// * Material

	omtshared virtual void set_material(OMediaFARGBColor &emission,
										OMediaFARGBColor &diffuse,
										OMediaFARGBColor &specular,
										OMediaFARGBColor &ambient,
										float			 shininess) = 0;

	// * Blending
	
	omtshared virtual void set_blend(omt_Blend blend) =0;
	omtshared virtual void set_blend_func(omt_BlendFunc src_func, omt_BlendFunc dest_func) =0;


	// * Fill mode

	omtshared virtual void set_override_material_fill_mode(bool override_mat_mode) =0;
	omtshared virtual void set_fill_mode(omt_FillMode mode) =0;
	omtshared virtual void set_shade_mode(omt_ShadeMode mode) =0;

	// * Culling

	omtshared virtual void enable_faceculling(void) =0;
	omtshared virtual void disable_faceculling(void) =0;

	// * Texture
	
	omtshared virtual void set_texture(OMediaCanvas *canvas) =0;

	omtshared virtual void set_extra_texture_passes(omt_ExtraTexturePassList *pass_list) =0;
	omtshared virtual long get_max_texture_passes(void) =0;

	omtshared virtual void set_texture_address_mode(const omt_TextureAddressMode am) = 0;
	omtshared virtual void set_texture_color_operation(const omt_TextureColorOperation am) = 0;

	
	// * Draw primitives (only convex polygons are supported)

	omtshared virtual void draw(omt_RenderVertexList &vertices, omt_RenderDrawMode mode) =0;


	// * Render flat surfaces. Size of the surface has no limitation (no power of 2, etc.)
	
	omtshared virtual void draw_surface(OMediaCanvas 		*canv, 
										float x, 			float y, 		float z, 
										float width,		float height,
										OMediaFARGBColor	&diffuse) = 0L;
	
	// * Render shape

		// In picking mode, one hit id is set per polygon (from 0 to last polygon)

	omtshared virtual void draw_shape(OMedia3DShape		*shape,
										bool			&inout_second_pass,
										OMediaRendererOverrideVertexList	*override_vlist =NULL) =0;
								
														// inout_second_pass:
														// In: If false, every material with the second pass
														// flag set are rejected.
														// If true, only material with the second pass flag
														// are drawn.
														// Out: True if one material with the second pass
														// flag set has been rejected.
											

	// * Picking
	
	omtshared virtual void start_picking(unsigned long max_ids) =0L;
	omtshared virtual vector<OMediaPickHit> *end_picking(void) =0L;

	omtshared virtual void set_picking_id(unsigned long id) =0L;

	omtshared virtual OMediaPickRequest *get_pick_mode(void) =0L;	// Returns NULL if not in picking mode
	
	// * Pipeline
	
	// You have no guarantee that polygons have been drawn as long as the following
	// method has not been called.
	
	omtshared virtual void flush_pipeline(void) =0;


	// * Fog (only linear for now)
	
	omtshared virtual void enable_fog(bool enable) =0;
	omtshared virtual void set_fog_density(float d) =0;
	omtshared virtual void set_fog_color(const OMediaFARGBColor &argb) =0;
	omtshared virtual void set_fog_range(float start, float end) =0;


	// * Texture perspective 
	
	omtshared virtual void enable_texture_persp(bool enabled) =0;
};


#endif

