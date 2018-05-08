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
#ifndef OMEDIA_GLRenderer_H
#define OMEDIA_GLRenderer_H

#include "OMediaSysDefs.h"
#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_OPENGL
#include "OMediaRenderer.h"
#include "OMediaRendererInterface.h"
#include "OMediaRetarget.h"
#include "OMediaExtraTexturePass.h"


class OMediaVideoMode;

class OMediaGLLight
{
	public:
	
	OMediaGLLight()
	{
		type = omclt_Directional;
		pos[0] = pos[1] = pos[3] = 0.0f; pos[2] = -1.0f;
		dir[0] = dir[1] = 0.0f; dir[2] = -1.0f;
		spot_cuttoff = 45.0f;
	}
	
	omt_LightType	type;
	float			spot_cuttoff;
	float			pos[4];
	float			dir[3];


};

const unsigned long	omc_GLMaxTexturePass	= 4;	// You need to update
													// the OMediaGL3DShape.h
													// file when changing
													// the number of passes.


//----------------------------------
// Render port

class OMediaGLRenderPort : 	public OMediaRenderPort,
							public OMediaRendererInterface
{
	public:
	
	omtshared OMediaGLRenderPort(OMediaRenderTarget *target);
	omtshared virtual ~OMediaGLRenderPort();

	omtshared virtual void capture_frame(OMediaCanvas &canv);

	omtshared virtual void render(void);

	// * Begin/End
	
	omtshared virtual bool begin_render(void);
	omtshared virtual void end_render(void);

	// * Renderer interface

	omtshared virtual omt_RendererRequirementFlags get_requirement_flags(void);

	// * View bounds
	
	omtshared virtual void get_view_bounds(OMediaRect &r);

	// * Buffer

	omtshared virtual void clear_all_buffers(OMediaFARGBColor &rgb);	// Color and ZBufer

	omtshared virtual void clear_colorbuffer(OMediaFARGBColor &rgb, OMediaRect *area =NULL);

	omtshared virtual void clear_zbuffer(OMediaRect *area =NULL);
	omtshared virtual void set_zbuffer_write(omt_ZBufferWrite zb);
	omtshared virtual void set_zbuffer_test(omt_ZBufferTest zb);
	omtshared virtual void set_zbuffer_func(omt_ZBufferFunc zb);

	omtshared virtual void enable_clipping_rect(bool enable);	
	omtshared virtual void set_clipping_rect(const OMediaRect &rect);	
	
	// * Matrix
	
	omtshared virtual void set_model_view(OMediaMatrix_4x4 &m);
	omtshared virtual void set_projection(OMediaMatrix_4x4 &m);

	// * Light

	omtshared virtual void enable_lighting(void);
	omtshared virtual void disable_lighting(void);

	omtshared virtual long get_max_lights(void);


	omtshared virtual void start_light_edit(void);
	omtshared virtual void end_light_edit(void);

	omtshared virtual void enable_light(long index);
	omtshared virtual void disable_light(long index);

	omtshared virtual void set_light_pos(long index, OMedia3DPoint &p);
	omtshared virtual void set_light_dir(long index, OMedia3DVector &v);

	omtshared virtual void set_light_type(long index, omt_LightType type);

	omtshared virtual void set_light_ambient(long index, OMediaFARGBColor &argb);
	omtshared virtual void set_light_diffuse(long index, OMediaFARGBColor &argb);
	omtshared virtual void set_light_specular(long index, OMediaFARGBColor &argb);

	omtshared virtual void set_light_attenuation(long index, float range, float constant, float linear, float quadratic);

	omtshared virtual void set_light_spot_cutoff(long index, float cutoff);
	omtshared virtual void set_light_spot_exponent(long index, float expo);

	omtshared virtual void set_light_global_ambient(OMediaFARGBColor &argb);

	// * Culling

	omtshared virtual void enable_faceculling(void);
	omtshared virtual void disable_faceculling(void);

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
	
	// * Draw geometry

	omtshared virtual void draw(omt_RenderVertexList &vertices, omt_RenderDrawMode mode);

	omtshared virtual void draw_shape(OMedia3DShape *shape, bool &inout_second_pass,
										OMediaRendererOverrideVertexList	*override_vlist);


	omtshared virtual void draw_surface(OMediaCanvas *canv, 
										float x, 	float y, 	float z, 
										float width,	float height,
										OMediaFARGBColor	&diffuse);

	// * Texture
	
	omtshared virtual void set_texture(OMediaCanvas *canvas);
	
	omtshared virtual void set_extra_texture_passes(omt_ExtraTexturePassList *pass_list);
	omtshared virtual long get_max_texture_passes(void);
	
	omtshared virtual void set_texture_address_mode(const omt_TextureAddressMode am);
	omtshared virtual void set_texture_color_operation(const omt_TextureColorOperation cm);


	// * Pipeline
	
	// You have no guarantee that polygons have been drawn as long as the following
	// method has not been called.
	
	omtshared virtual void flush_pipeline(void);


	// * Picking
	
	omtshared virtual void start_picking(unsigned long max_ids);
	omtshared virtual vector<OMediaPickHit> *end_picking(void);

	omtshared virtual void set_picking_id(unsigned long id);
	
	omtshared virtual OMediaPickRequest *get_pick_mode(void);	// Returns NULL if not in picking mode


	// * Fog (only linear for now)
	
	omtshared virtual void enable_fog(bool enable);
	omtshared virtual void set_fog_density(float d);
	omtshared virtual void set_fog_color(const OMediaFARGBColor &argb);
	omtshared virtual void set_fog_range(float start, float end);
	
	
	// * Enable texture perspective

	omtshared virtual void enable_texture_persp(bool enabled);

	//-------------------

	void draw_shape_prepare_mat(OMedia3DMaterial *mat,
												bool &force_flat,
												omt_FillMode	&fillmode);

	inline static void set_glVP_zerobased(bool s) {glVP_zerobased = s;}

	protected:
	
	void gl_clear(GLbitfield buf, OMediaRect *src_area);

	void prepare_gl_context(void);

	omt_FillMode				fillmode;
	omt_ShadeMode				shademode;
	bool						light_enabled;
	bool						override_mat_fillmode;
	bool						user_scissor,original_use_scissor;
	bool						blend_enabled;
	bool						culling_enabled;
	bool						picking_mode;
	OMediaRect					scissor_rect,original_scissor_rect;
	OMediaGLLight				*lights;
	OMediaCanvas				*texture;
	omt_TextureAddressMode		texture_address_mode;
	omt_TextureColorOperation	texture_color_mode;
	omt_ExtraTexturePassList	*current_texture_pass;
	long						max_texture_units;
	
	omt_ZBufferWrite			zbuffer_write;
	omt_ZBufferTest				zbuffer_test;

	GLint						current_text_arb;
	
	static GLuint				*picking_buffer;
	static long					picking_buffer_size;
	static long					nrenderers;
	
	vector<OMediaPickHit> 		hit_list;

	static bool					glVP_zerobased;
};



//----------------------------------
// Render target

class OMediaGLRenderTarget : public OMediaRenderTarget
{
	public:
	
	OMediaGLRenderTarget();
	OMediaGLRenderTarget(OMediaRenderer *renderer);
	virtual ~OMediaGLRenderTarget();
	
	omtshared virtual OMediaRenderPort *new_port(void);

	omtshared virtual void render(void);

	omtshared virtual void prepare_context(OMediaWindow *window);
	omtshared virtual void set_context(void);

	// * Retarget

	inline OMediaRetarget *get_retarget(void) {return retarget;}


	// * Check texture size

	omtshared virtual void check_texture_size(long &w, long &h);

        // * Picking

        omtshared virtual OMediaPickRequest *get_pick_mode(void);

	//------------------------------------------

	omtshared virtual void init_retarget(void);
	omtshared virtual void flip_buffers(void);

	omtshared virtual void erase_context(void);
	omtshared virtual void update_context(void);

        omtshared virtual bool get_video_bounds(OMediaRect &videorect);

        omtshared virtual void prepare_first_render();

	OMediaRetarget 				*retarget;
	GLint					max_texture_units;
	bool					first_render;
};

//----------------------------------
// Special GL target that can be used for pure GL context
// simply override the set_context method to set your GL context.
// Pure GL context does not use OMT windows, AGL, etc. It uses
// only pure GL functions.

class OMediaViewPort;

class OMediaPureGLRenderTarget : public OMediaGLRenderTarget
{
	public:
	
	OMediaPureGLRenderTarget();
	virtual ~OMediaPureGLRenderTarget();	// Call "set_context();delete_all_implementations()" 

	        
	omtshared virtual void set_context(void);	// Must be overrided - set your GL context

        // * Render - call the following method to render/pick a viewport

        omtshared virtual void render_viewport(OMediaViewPort *viewport);
        omtshared virtual void pick_viewport(OMediaViewPort *viewport,OMediaPickRequest *pick_mode);
        

       // * Picking - low level

        omtshared virtual OMediaPickRequest *get_pick_mode(void);
        omtshared virtual void set_pick_mode(OMediaPickRequest *pick_request);

	//------------------------------------------

	omtshared virtual void prepare_context(OMediaWindow *window);	// Do nothing, window is NULL in this context

        protected:

	omtshared virtual void init_retarget(void);
	omtshared virtual void flip_buffers(void);
        
        OMediaPickRequest 	*pick_request;
        OMediaGLRenderPort	*gl_port;
};


//----------------------------------
// Renderer

class OMediaGLRenderer : 	public OMediaRenderer
{
	public:
	
	// * Construction
	
	omtshared OMediaGLRenderer(OMediaVideoEngine *video, OMediaRendererDef *def, omt_ZBufferBitDepthFlags zbuffer);
	omtshared virtual ~OMediaGLRenderer();


	// * Targets

	omtshared virtual OMediaRenderTarget *new_target(void);
	

	// * Retarget

	inline OMediaRetarget *get_retarget(void) {return retarget;}

	// * ZBuffer bit depth

	inline omt_ZBufferBitDepthFlags get_zbuffer_bitdepth(void) {return zbuffer;}

	protected:
	
	omtshared virtual void init_retarget(OMediaVideoMode *videomode, OMediaRendererDef *def);
	
	OMediaRetarget 				*retarget;
	omt_ZBufferBitDepthFlags	zbuffer;
};



#endif
#endif

