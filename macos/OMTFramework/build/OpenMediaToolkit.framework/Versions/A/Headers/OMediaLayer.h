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
#ifndef OMEDIA_Layer_H
#define OMEDIA_Layer_H

#include "OMediaTypes.h"
#include "OMediaElementContainer.h"
#include "OMediaRect.h"
#include "OMediaMatrix.h"
#include "OMediaRendererInterface.h"
#include "OMediaElement.h"

#include <list>

class OMediaWorld;
class OMediaLayer; 
class OMediaRenderHTransform;
class OMediaRenderVTransform;
class OMediaViewPort;
 
enum omt_LayerProjectionType
{
	omlptc_None,					// No projection. Identity matrix.
	omlptc_Ortho,
	omlptc_PerspectiveFrustum,		// Perspective is based on the frustrum matrix
	omlptc_PerspectiveFOV,			// Perspective is based on the field-of-view matrix (default)
	
	omlptc_Matrix
};



typedef unsigned long omt_LayerFlags;
const omt_LayerFlags omlayerf_ClearZBuffer 				= (1<<0);	// Clear ZBuffer before rendering.
const omt_LayerFlags omlayerf_ClearColor 				= (1<<1);	// Clear color before rendering.
const omt_LayerFlags omlayerf_Visible		 			= (1<<2);	// Visible
const omt_LayerFlags omlayerf_EnableZBufferWrite		= (1<<3);	// ZBuffer write is enabled
const omt_LayerFlags omlayerf_EnableZBufferTest			= (1<<4);	// ZBuffer test is enabled
const omt_LayerFlags omlayerf_SortElementBack2Front		= (1<<5);	// Elements are pre-sorted back to front
const omt_LayerFlags omlayerf_SortElementFront2Back		= (1<<6);	// Elements are pre-sorted front to back
const omt_LayerFlags omlayerf_DisablePicking			= (1<<7);	// Disable picking
const omt_LayerFlags omlayerf_DisableViewportTransform	= (1<<8);	// Disable viewport transformation for this
																	// layer.

const omt_LayerFlags omlayerf_CustomViewMatrix			= (1<<9);	// Use a custom view matrix for this layer.
const omt_LayerFlags omlayerf_SortByUntransformedZ		= (1<<10);	// When set elements are sorted using the
																	// z value as it is defined by the place
																	// method (no rotation or translate is applied).
																	// This flag has effect only with omlayerf_SortElementBack2Front
																	// or omlayerf_SortElementBack2Front flags.
																	


typedef list<OMediaLayer*> omt_LayerList;

class OMediaLayer
{
	public:
	
	// * Constructor/Destructor

	omtshared OMediaLayer();
	omtshared virtual ~OMediaLayer();

	// * Link to world

	omtshared virtual void link(OMediaWorld *world);
	omtshared virtual void unlink(void);

	inline OMediaWorld *get_world(void) const {return its_world;}

	// * Clear color

	inline void set_clear_color(const OMediaFARGBColor	&rgb) {clear_color = rgb;}
	inline void get_clear_color(OMediaFARGBColor &rgb) const {rgb = clear_color;}


		// By default the whole color buffer is cleared when the "omlayerf_ClearZBuffer" is set.
		// You can clear only an area if required. Please note that the area coordinates are projected
		// using the layer projection matrix.

	inline void enable_color_area(const OMediaRect &area)
	{
		clear_color_area = true;
		color_area = area;
	}
	
	inline void disable_color_area(void)
	{
		clear_color_area = false;
	}
	
	inline void get_color_area(OMediaRect &r) const {r=color_area;}



	// * ZBuffer

	inline void set_zbuffer_func(const omt_ZBufferFunc func) {zbuffer_func = func;}
	inline omt_ZBufferFunc set_zbuffer_func(void) const {return zbuffer_func;}
	
		// By default the whole ZBuffer is cleared when the "omlayerf_ClearColor" is set.
		// You can clear only an area if required. Please note that the area coordinates are projected
		// using the layer projection matrix.

	inline void enable_zbuffer_area(const OMediaRect &area)
	{
		clear_zbuffer_area = true;
		zbuffer_area = area;
	}
	
	inline void disable_zbuffer_area(void)
	{
		clear_zbuffer_area = false;
	}
	
	inline void get_zbuffer_area(OMediaRect &r) const {r=zbuffer_area;}

	
	// * Clipping planes

	inline void set_far_clip(float f) {far_clip = f;}
	inline float get_far_clip(void) const {return far_clip;}

	inline void set_near_clip(float n) {near_clip = n;}
	inline float get_near_clip(void) const {return near_clip;}
	
	
	// * Projection type
	
		// This set the default projection matrix for this layer. Of course you can
		// change the matrix at any time using the OMediaRendererInterface::set_projection method.
		// Default is "omlpmc_Perspective"
	
	inline void set_projection_type(omt_LayerProjectionType lpt) {proj_type = lpt;}
	inline omt_LayerProjectionType get_projection_type(void) const {return proj_type;}
	
		// Field of view for the FOV perspective projection
		
	inline void set_proj_perspective_fov(omt_Angle fov)
	{
		proj_persp_fov = fov;
	}

	inline omt_Angle get_proj_perspective_fov(void) const
	{
		return proj_persp_fov;
	}

		// If you set the "omlptc_Matrix" type, you should override the following methods to fill
		// the projection matrix with your custom matrix. This method is called before the 
		// frame is rendered. When you set a custom projection matrix don't forget to take care
		// of the required Z range of the renderer. You can find this range in the passed flags.

	omtshared virtual void prepare_projection_matrix(omt_RendererRequirementFlags flags, OMediaMatrix_4x4 &m,
													float	vp_width,
													float	vp_height);


	// * Custom view matrix (used when the omlayerf_CustomViewMatrix flag is set)

	inline void set_custom_view_matrix(const OMediaMatrix_4x4 &m) {custom_view_matrix = m; } 
	inline void get_custom_view_matrix(OMediaMatrix_4x4 &m) const {m = custom_view_matrix; } 

	// * Flags

		// First pass flags.
		// Default is omlayerf_Visible
	
	inline void set_flags(omt_LayerFlags fl) {flags = fl;}
	inline omt_LayerFlags get_flags(void) const {return flags;}

	inline void add_flags(const omt_LayerFlags fl) {flags |= fl;}
	inline void remove_flags(const omt_LayerFlags fl) {flags &= ~fl;}

		// Second pass flags. The second pass is typically used to render transparent
		// object when zbuffering is enabled. If the visible flag is not set for the
		// second pass, it is not rendered. Using omlayerf_ClearXXX flags on the second
		// pass has no effect.
		//
		// Default is omlayerf_Visible|omlayerf_EnableZBufferTest|omlayerf_SortElementBack2Front

	inline void set_flags_pass2(omt_LayerFlags fl) {flags_pass2 = fl;}
	inline omt_LayerFlags get_flags_pass2(void) const {return flags_pass2;}

	inline void add_flags_pass2(const omt_LayerFlags fl) {flags_pass2 |= fl;}
	inline void remove_flags_pass2(const omt_LayerFlags fl) {flags_pass2 &= ~fl;}

		

	// * Override material fill mode

	inline void enable_override_fill_mode(omt_FillMode mode) {override_fmode = true; fillmode = mode;}
	inline void disable_override_fill_mode(void) {override_fmode = false;}
	inline omt_FillMode get_override_fill_mode(void) const {return fillmode;}	

	// * Viewport key

	inline void set_viewport_key(unsigned long k) {viewport_key = k;}
	inline unsigned long get_viewport_key(void) const {return viewport_key;}
	
		// Layer is drawn by viewport only if (layer->viewport_key&viewport->layer_key)!=0

	// * Light

	inline void set_light_key(unsigned long k) {light_key = k;}
	inline unsigned long get_light_key(void) const {return light_key;}
	
		// Light is computed by this layer if (layer->light_key&light->layer_key)!=0


	inline void set_light_global_ambient(const OMediaFARGBColor &argb) {glob_ambient = argb;}
	inline void get_light_global_ambient(OMediaFARGBColor &argb) const {argb = glob_ambient;}
	


	// * Elements

	inline omt_ElementList *get_element_list(void) {return &elements;}


	// * Render

	omtshared virtual void render(OMediaRendererInterface *rdr_i, 
								  const OMediaRect &dest_bounds,
								  OMediaMatrix_4x4 &viewmatrix);

	// * Clipping rect. (only GL for now)

	inline void enable_clipping_rect(bool enable) {clip_rect_enabled = enable;}	
	inline void set_clipping_rect(const OMediaRect &rect) {clip_rect = rect;}
	inline void get_clipping_rect(OMediaRect &rect) const {rect = clip_rect;}


	// * Fog (only linear for now and GL for now)
	
	inline void enable_fog(bool enable) {fog_enabled = true;}
	inline void set_fog_density(float d) {fog_density = d;}
	inline void set_fog_color(const OMediaFARGBColor &argb) {fog_color = argb;}
	inline void set_fog_range(float start, float end) {fog_start = start; fog_end = end;}
	
	// * Texture perspective correction (true by default)
	
	inline void set_texture_pcorrect(const bool pc) {text_pcorrect = pc;}
	inline bool get_texture_pcorrect(void) const {return text_pcorrect;}


	// * Misc.

	omtshared virtual bool world_to_viewport_coords(OMediaViewPort *vp, const OMedia3DPoint &src, float &vpx, float &vpy);
				// Returns FALSE if outside viewport
				// Matrix of this layer is used
				// 
				// Coordinates returned are based on the center of the viewport
				// with y going up.


	protected:

	omtshared virtual void render_presorted_element(OMediaRendererInterface *rdr_i,
											omt_RenderPreSortedElementList *presort,
											OMediaMatrix_4x4 &viewmatrix,
											omt_RenderModeElementFlags render_flags,
											bool back2frontsort);


	omt_LayerFlags			flags,flags_pass2;
	OMediaWorld				*its_world;
	omt_ElementList			elements;
	omt_LayerList::iterator	layer_iterator;
	unsigned long			viewport_key,light_key;
	
	bool					override_fmode;
	omt_FillMode			fillmode;

	omt_LayerProjectionType	proj_type;
	OMediaMatrix_4x4		proj_matrix;
	omt_Angle				proj_persp_fov;

	OMediaMatrix_4x4		custom_view_matrix;

	float					far_clip,near_clip;

	bool 					clip_rect_enabled;
	OMediaRect				clip_rect;
	
	bool					clear_zbuffer_area;
	OMediaRect				zbuffer_area;
	omt_ZBufferFunc			zbuffer_func;

	bool					clear_color_area;
	OMediaRect				color_area;
	OMediaFARGBColor		clear_color,glob_ambient;

	bool					fog_enabled;
	float					fog_density;
	OMediaFARGBColor		fog_color;
	float					fog_start,fog_end;
	
	bool					text_pcorrect;
};


#endif

