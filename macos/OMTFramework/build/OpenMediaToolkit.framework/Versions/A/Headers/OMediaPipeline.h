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
#ifndef OMEDIA_Pipeline_H
#define OMEDIA_Pipeline_H

#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_OMTRENDERER

#include "OMediaTypes.h"
#include "OMediaPipePoint.h"
#include "OMediaPipePolygon.h"
#include "OMediaMatrix.h"
#include "OMediaRendererInterface.h"
#include "OMediaPipeLight.h"
#include "OMediaPipeMaterial.h"
#include "OMediaBuildSwitch.h"

class OMediaOMTCanvas;
class OMediaOMTCanvasText;
class OMediaRenderTarget;

typedef unsigned short omt_PipelineFlags;
const omt_PipelineFlags omplf_Front2BackSort 		= (1<<0);
const omt_PipelineFlags omplf_EyeDistSort			= (1<<1);
const omt_PipelineFlags omplf_ZSort			 		= (1<<2);

enum omt_RasterizerFillMode
{
	omrfmc_PointList,
	omrfmc_LineLoop,
	omrfmc_TriangleFan
};

class OMediaPipe2RendererInterface
{
	public:
	
	omtshared virtual void flush_pipeline(void) =0L;
	
	omtshared virtual void get_view_bounds(OMediaRect &r) =0L;

	omtshared virtual void clear_colorbuffer(OMediaFARGBColor &rgb, OMediaRect *area =NULL) =0L;
		
	omtshared virtual void clear_zbuffer(OMediaRect *area =NULL) =0L;
	omtshared virtual void set_zbuffer_write(omt_ZBufferWrite zb) =0L;
	omtshared virtual void set_zbuffer_test(omt_ZBufferTest zb) =0L;
	omtshared virtual void set_zbuffer_func(omt_ZBufferFunc zb) =0L;

	omtshared virtual OMediaRenderTarget *get_target(void) =0L;

	omtshared virtual void get_zbuffer_info(omt_ZBufferTest &zt, omt_ZBufferWrite &zw, omt_ZBufferFunc &zf) =0;

};

enum omt_ClippingPlane
{
	omclippc_Left,
	omclippc_Right,
	omclippc_Top,
	omclippc_Bottom,
	omclippc_Near,
	omclippc_Far
};




class OMediaPipeline : public OMediaRendererInterface
{
	public:
	
	// * Construction
	
	omtshared OMediaPipeline(OMediaPipe2RendererInterface *p2rdr_i);
	omtshared virtual ~OMediaPipeline();

	// * Begin/end work

	omtshared virtual void begin(omt_PipelineFlags flags_pass);
	omtshared virtual void end();

	// * Buffer

	inline OMediaPipePolygonBuffer *get_polygons(void) {return poly_buffer;}


	// * Interface support

		// * Requirements
	
		omtshared virtual omt_RendererRequirementFlags get_requirement_flags(void);
	
		// * View bounds
		
		omtshared virtual void get_view_bounds(OMediaRect &r);

		// * Clipping rect (scissor) not implemented yet

		omtshared virtual void enable_clipping_rect(bool enable);	
		omtshared virtual void set_clipping_rect(const OMediaRect &rect);

	
		// * Buffers
	
		omtshared virtual void clear_all_buffers(OMediaFARGBColor &rgb);	// Color and ZBufer
	
		omtshared virtual void clear_colorbuffer(OMediaFARGBColor &rgb, OMediaRect *area =NULL);
		
		omtshared virtual void clear_zbuffer(OMediaRect *area =NULL);
		omtshared virtual void set_zbuffer_write(omt_ZBufferWrite zb);
		omtshared virtual void set_zbuffer_test(omt_ZBufferTest zb);
		omtshared virtual void set_zbuffer_func(omt_ZBufferFunc zb);
	
		
		// * Matrix
		
		omtshared virtual void set_model_view(OMediaMatrix_4x4 &m);
		omtshared virtual void set_projection(OMediaMatrix_4x4 &m);
	
		// * Light
	
		omtshared virtual void enable_lighting(void);
		omtshared virtual void disable_lighting(void);
	
		omtshared virtual long get_max_lights(void);
	
		// Any change to one or more lights needs to be enclosed by start/end_light_edit calls.
	
		omtshared virtual void start_light_edit(void);
		omtshared virtual void end_light_edit(void);
	
		omtshared virtual void enable_light(long index);
		omtshared virtual void disable_light(long index);
	
		omtshared virtual void set_light_type(long index, omt_LightType type);
		omtshared virtual void set_light_pos(long index, OMedia3DPoint &p);
		omtshared virtual void set_light_dir(long index, OMedia3DVector &v);
	
		omtshared virtual void set_light_ambient(long index, OMediaFARGBColor &argb);
		omtshared virtual void set_light_diffuse(long index, OMediaFARGBColor &argb);
		omtshared virtual void set_light_specular(long index, OMediaFARGBColor &argb);
	
		omtshared virtual void set_light_attenuation(long index, float range, float constant, float linear, float quadratic);
	
		omtshared virtual void set_light_spot_cutoff(long index, float cutoff);
		omtshared virtual void set_light_spot_exponent(long index, float expo);
	
		omtshared virtual void set_light_global_ambient(OMediaFARGBColor &argb);
	
	
		// * Material
	
		omtshared virtual void set_material(OMediaFARGBColor &emission,
											OMediaFARGBColor &diffuse,
											OMediaFARGBColor &specular,
											OMediaFARGBColor &ambient,
											float			 shininess);
	
		// * Blending
		
		omtshared virtual void set_blend(omt_Blend blend);
		omtshared virtual void set_blend_func(omt_BlendFunc src_func, omt_BlendFunc dest_func);
	
	
		// * Fill mode
	
		omtshared virtual void set_override_material_fill_mode(bool override_mat_mode);
		omtshared virtual void set_fill_mode(omt_FillMode mode);
		omtshared virtual void set_shade_mode(omt_ShadeMode mode);
	
		// * Culling
	
		omtshared virtual void enable_faceculling(void);
		omtshared virtual void disable_faceculling(void);
	
		// * Texture
		
		omtshared virtual void set_texture(OMediaCanvas *canvas);
	
		omtshared virtual void set_extra_texture_passes(omt_ExtraTexturePassList *pass_list);
		omtshared virtual long get_max_texture_passes(void);
	
		omtshared virtual void set_texture_address_mode(const omt_TextureAddressMode am);
		omtshared virtual void set_texture_color_operation(const omt_TextureColorOperation cm);
	
		
		// * Draw primitives (only convex polygons are supported)
	
		omtshared virtual void draw(omt_RenderVertexList &vertices, omt_RenderDrawMode mode);
	
	
		// * Render flat surfaces. Size of the surface has no limitation (no power of 2, etc.)
		
		omtshared virtual void draw_surface(OMediaCanvas 		*canv, 
											float x, 			float y, 		float z, 
											float width,		float height,
											OMediaFARGBColor	&diffuse);
		
		// * Render shape
	
			// In picking mode, one hit id is set per polygon (from 0 to last polygon)
	
		omtshared virtual void draw_shape(OMedia3DShape		*shape,
											bool			&inout_second_pass,
											OMediaRendererOverrideVertexList	*override_vlist =NULL);
									
															// inout_second_pass:
															// In: If false, every material with the second pass
															// flag set are rejected.
															// If true, only material with the second pass flag
															// are drawn.
															// Out: True if one material with the second pass
															// flag set has been rejected.
												
	
		// * Picking
		
		omtshared virtual void start_picking(unsigned long max_ids);
		omtshared virtual vector<OMediaPickHit> *end_picking(void);
	
		omtshared virtual void set_picking_id(unsigned long id);
	
		omtshared virtual OMediaPickRequest *get_pick_mode(void);	// Returns NULL if not in picking mode
		
		// * Pipeline
		
		// You have no guarantee that polygons have been drawn as long as the following
		// method has not been called.
		
		omtshared virtual void flush_pipeline(void);
	

		// * Fog (only linear for now)
		
		omtshared virtual void enable_fog(bool enable);
		omtshared virtual void set_fog_density(float d);
		omtshared virtual void set_fog_color(const OMediaFARGBColor &argb);
		omtshared virtual void set_fog_range(float start, float end);


		// * Texture perspective 
		
		omtshared virtual void enable_texture_persp(bool enabled);


	// * Misc.

	inline omt_ShadeMode get_shade_mode(void) const {return shade_mode;}
	inline bool get_blend_enabled(void) const {return blend_enabled;}
	inline omt_BlendFunc get_src_blend(void) const {return blend_src;}
	inline omt_BlendFunc get_dest_blend(void) const {return blend_dest;}

	inline bool is_picking_on(void) const {return picking_mode;}
	

	protected:

	bool clip_polygon(OMediaPipePolygon *poly, omt_ClippingPlane plane);
	void move_clipped_points(short np, OMediaPipePolygon *poly, OMediaPipePoint **ptab);

	void compute_light(OMediaPipePoint *p);

	void gen_surface(OMediaCanvas *canv,float *xyzw, float *xyzw2,float alpha);

	inline void begin_gen_polygon(omt_RasterizerFillMode fmode, long	nvertices);
		
	inline void gen_vertex(OMedia3DPoint		&p,	
					OMedia3DVector		*normal,
					OMediaFARGBColor	*color,
					float u,
					float v);
	
	inline void end_gen_polygon(void);

	inline void clamp_nonhomogenous_coord(OMediaPipePoint *pp)
	{		
		if (pp->xyzw[0]<-1.0f) pp->xyzw[0] = -1.0f;
		else if (pp->xyzw[0]>1.0f) pp->xyzw[0] = 1.0f;

		if (pp->xyzw[1]<-1.0f) pp->xyzw[1] = -1.0f;
		else if (pp->xyzw[1]>1.0f) pp->xyzw[1] = 1.0f;

		if (pp->xyzw[2]<-1.0f) pp->xyzw[2] = -1.0f;
		else if (pp->xyzw[2]>1.0f) pp->xyzw[2] = 1.0f;
	}
	
	inline float transform_homogenous_coord(float *xyzw)
	{
		if (xyzw[3]!=1.0f)
		{	
			float iw = 1.0f/xyzw[3];
	
			xyzw[0] *= iw;
			xyzw[1] *= iw;
			xyzw[2] *= iw;	
			
			return iw;	
		}
		
		return 1.0f;
	}
	
	inline void transform_xyzw(float *xyzw, float *txyzw, float *nh_txyzw)
	{
		model_view.multiply(xyzw);
		copy_xyzw(xyzw,txyzw);	
		projection.multiply(txyzw);	
		copy_xyzw(txyzw,nh_txyzw);	
		transform_homogenous_coord(nh_txyzw);
	}
	
	inline void copy_xyzw(const float *xyzw, float *d_xyzw)
	{
		d_xyzw[0] = xyzw[0];
		d_xyzw[1] = xyzw[1];
		d_xyzw[2] = xyzw[2];
		d_xyzw[3] = xyzw[3];
	}
	
	inline bool backface_culled(const float *p1, 
								 const float *p2,
								 const float *p3)
	{
		OMedia3DVector	u,v,n;

		u.x = p2[0]-p1[0];	u.y = p2[1]-p1[1];	u.z = p2[2]-p1[2];
		v.x = p3[0]-p1[0];	v.y = p3[1]-p1[1];	v.z = p3[2]-p1[2];
			
		v.cross_product(u,n);

		return (n.z>=0);
	}

	inline void enlarge_clip_tab_buffer(long n)
	{
		delete [] clip_tab_buffer;

		clip_tab_buffer_size += n;
		clip_tab_buffer		= new OMediaPipePoint*[clip_tab_buffer_size];
	}

	void pick_find_zrange(	OMediaPipePolygon *poly, 
											float &min_z, float &max_z,
											bool	newhit);

	bool flat_draw_surface(OMediaCanvas 		*canv, 
									float x, 			float y, 		float z, 
									float width,		float height,
									OMediaFARGBColor	&diffuse);


	OMediaPickHit				pick_newhit;
	vector<OMediaPickHit> 		hit_list;

	OMediaMatrix_4x4						model_view,projection,inv_model_view;
	omt_PipelineFlags						flags;
	OMediaPipe2RendererInterface			*pipe2rnd_interface;

	OMediaPipeLight							lights[omd_MAX_PIPELINE_LIGHTS];
	OMediaPipeMaterial						material;
	bool									light_enabled,blend_enabled,culling_enabled,override_mat_fillmode;
	OMediaFARGBColor						global_ambient;

	omt_BlendFunc							blend_src, blend_dest;
	
	OMediaOMTCanvasText						*texture;
	omt_TextureColorOperation				texture_color_mode;

	omt_ShadeMode							shade_mode;
	omt_FillMode							fill_mode;

	long									gen_base_npoints;
	OMediaPipePoint							*gen_base_p, *gen_cur_p;
	omt_RasterizerFillMode					gen_fill_mode;
	
	bool 									picking_mode;

	omtshared static OMediaPipePointBuffer 		*point_buffer;
	omtshared static OMediaPipePolygonBuffer 	*poly_buffer;

	omtshared static long buffer_user;
	
	omtshared static OMediaPipePoint			**clip_tab_buffer;
	omtshared static unsigned long				clip_tab_buffer_size;
};



#endif
#endif

