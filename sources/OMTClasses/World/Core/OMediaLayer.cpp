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


#include "OMediaLayer.h"
#include "OMediaElement.h"
#include "OMediaLight.h"
#include "OMediaWorld.h"
#include "OMediaRendererInterface.h"
#include "OMediaPickRequest.h"
#include "OMediaViewPort.h"

#include <algorithm>

OMediaLayer::OMediaLayer()
{
	flags = omlayerf_Visible;
	flags_pass2 = omlayerf_Visible|omlayerf_EnableZBufferTest|omlayerf_SortElementBack2Front;
	proj_type = omlptc_PerspectiveFOV;

	viewport_key = light_key = 0xFFFFFFFFL;

	its_world = NULL;

	custom_view_matrix.set_identity();
	proj_matrix.set_identity();
	proj_persp_fov = omd_Deg2Angle(60);

	far_clip			= 3200;
	near_clip	 		= 20;
	
	override_fmode		= false;
	fillmode			= omfillmc_Solid;

	clear_zbuffer_area = false;
	zbuffer_func = omzbfc_Less;
	
	clear_color_area = false;
	clear_color.set(1.0f,0.0f,0.0f,0.0f);

	glob_ambient.set(1.0f,0.2f,0.2f,0.2f);

	clip_rect_enabled = false;
	clip_rect.set(0,0,16,16);

	fog_enabled = false;
	fog_density = 0.3f;
	fog_color.set(1,1,1,1);
	fog_start = 100.0f;
	fog_end = 1000.0f;
	
	text_pcorrect = true;
}

OMediaLayer::~OMediaLayer()
{
	while(elements.size()) (*elements.begin())->unlink_layer();

	unlink();
}

void OMediaLayer::link(OMediaWorld *world)
{
	unlink();

	its_world = world;
	if (its_world)
	{
		its_world->get_layers()->push_back(this);
		layer_iterator = its_world->get_layers()->end();
		layer_iterator--;
	}
}

void OMediaLayer::unlink(void)
{
	if (its_world)
	{
		its_world->get_layers()->erase(layer_iterator);
		its_world = NULL;
	}
}

void OMediaLayer::prepare_projection_matrix(omt_RendererRequirementFlags flags, 
											OMediaMatrix_4x4 &proj_matrix,
											float	vp_width,
											float	vp_height)
{
	float	hw,hh;

	hw = vp_width*0.5f;
	hh = vp_height*0.5f;

	switch(proj_type)
	{
		case omlptc_None:
		proj_matrix.set_identity();
		break;
	
		case omlptc_Ortho:
		if (flags&omrrf_ProjectionZRange_0_1)
			proj_matrix.set_ortho_z0_1(-hw, -hh, hw, hh, near_clip, far_clip);
		else
			proj_matrix.set_ortho_zn1_1(-hw, -hh, hw, hh, near_clip, far_clip);
		break;

		case omlptc_PerspectiveFrustum:
		if (flags&omrrf_ProjectionZRange_0_1)
			proj_matrix.set_perspective_frustum_z0_1(vp_width,vp_height,
											near_clip,far_clip,
											1.0f);
		else
			proj_matrix.set_perspective_frustum_zn1_1(vp_width,vp_height,
											near_clip,far_clip,
											1.0f);
		break;

		case omlptc_PerspectiveFOV:
		if (flags&omrrf_ProjectionZRange_0_1)
			proj_matrix.set_perspective_fov_z0_1(proj_persp_fov,
											near_clip,far_clip,
											vp_height/vp_width);
		else
			proj_matrix.set_perspective_fov_zn1_1(proj_persp_fov,
											near_clip,far_clip,
											vp_height/vp_width);
		break;

		default:
		proj_matrix.set_identity();
		break;
	}
}

void OMediaLayer::render(OMediaRendererInterface *rdr_i, 
						 const OMediaRect &dest_bounds,
						 OMediaMatrix_4x4 &aviewmatrix)
{
	if (!(flags&omlayerf_Visible)) return;

	// Set projection matrix

	OMediaMatrix_4x4	viewmatrix;
	float	vpw = float(dest_bounds.get_width());
	float	vph = float(dest_bounds.get_height());
	float	hw = vpw*0.5f;
	float	hh = vph*0.5f;
	
	OMediaPickRequest	*pickrequest = rdr_i->get_pick_mode();
	OMediaMatrix_4x4	pick_matrix,temp_matrix;
	
	if (pickrequest && (flags&omlayerf_DisablePicking)) return;

	if (flags&omlayerf_CustomViewMatrix)
	{
		viewmatrix = custom_view_matrix;
	}
	else
	{
		if (flags&omlayerf_DisableViewportTransform) viewmatrix.set_identity();
		else viewmatrix = aviewmatrix;
	}

	prepare_projection_matrix(rdr_i->get_requirement_flags(), proj_matrix, vpw,vph); 

	if (pickrequest)
	{
		pick_matrix.set_pick(pickrequest->pickx, hh-(pickrequest->picky-hh), 
							 pickrequest->pickw, pickrequest->pickh,
						 	 0,0,dest_bounds.get_width(),dest_bounds.get_height());
		pick_matrix.multiply(proj_matrix,temp_matrix);	

		rdr_i->set_projection(temp_matrix);

		pickrequest->normalized_px = (pickrequest->pickx - hw) / hw;
		pickrequest->normalized_py = -((pickrequest->picky - hh) / hh);
		
	}
	else rdr_i->set_projection(proj_matrix);
	

	if (!pickrequest)
	{
		// Clear all buffers
	
		if ((flags&(omlayerf_ClearColor|omlayerf_ClearZBuffer))==(omlayerf_ClearColor|omlayerf_ClearZBuffer) &&
			!clear_color_area && !clear_zbuffer_area)
		{
			rdr_i->set_zbuffer_write(omzbwc_Enabled);
			rdr_i->clear_all_buffers(clear_color);
		}	
		else
		{
			// Color
		
			if (flags&omlayerf_ClearColor)
			{
				rdr_i->clear_colorbuffer(clear_color, clear_color_area? &color_area: NULL);
			}
			
			// ZBuffer
		
			if (flags&omlayerf_ClearZBuffer)
			{
				rdr_i->set_zbuffer_write(omzbwc_Enabled);
				rdr_i->clear_zbuffer(clear_zbuffer_area? &zbuffer_area: NULL);
			}
		}
	}
	
	// Clipping
	
	rdr_i->enable_clipping_rect(clip_rect_enabled);
	if (clip_rect_enabled) rdr_i->set_clipping_rect(clip_rect);
	

	// ZBuffer options

	rdr_i->set_zbuffer_write((flags&omlayerf_EnableZBufferWrite)?omzbwc_Enabled:omzbwc_Disabled);
	rdr_i->set_zbuffer_test((flags&omlayerf_EnableZBufferTest)?omzbtc_Enabled:omzbtc_Disabled);
	rdr_i->set_zbuffer_func(zbuffer_func);	
	
	// Options
	
	if (override_fmode) 
	{
		rdr_i->set_fill_mode(fillmode);
		rdr_i->set_override_material_fill_mode(true);
	}
	else rdr_i->set_override_material_fill_mode(false);
	
	// Fog

	rdr_i->enable_fog(fog_enabled);
	rdr_i->set_fog_density(fog_density);
	rdr_i->set_fog_color(fog_color);
	rdr_i->set_fog_range(fog_start,fog_end);
	
	// Texture correction
	
	rdr_i->enable_texture_persp(text_pcorrect);


	// Prepare light

	OMediaLight::current_render_count++;
	OMediaLight::render_layer_key = light_key;

	rdr_i->set_light_global_ambient(glob_ambient);


	// Render elements

	OMediaRenderHTransform	hxform,base_hxform;
	
	base_hxform.trans_x = base_hxform.trans_y = base_hxform.trans_z = 0;
	bool							require_second_pass = false;
	omt_RenderPreSortedElementList	*presort;
	omt_RenderPreSortedElementList	presort_list;

	// Pass 1

	if (flags&(omlayerf_SortElementBack2Front|omlayerf_SortElementFront2Back)) presort = &presort_list;
	else presort = NULL;

	for(omt_ElementList::iterator ei=elements.begin();
		ei!=elements.end();
		ei++)
	{
		(*ei)->render_mark_element_flags = 0;
		(*ei)->compute_hxform(base_hxform,hxform);
		(*ei)->render(rdr_i,hxform,viewmatrix,proj_matrix,its_world->get_light_sources(),presort,0);
		if ((*ei)->render_mark_element_flags&(omermf_ScanSecondPassMark|omermf_SecondPassMark)) require_second_pass = true;
	}
	
	if (presort) render_presorted_element(rdr_i,presort,viewmatrix,0,(flags_pass2&omlayerf_SortElementBack2Front)!=0);

	// Pass 2
	
	if (require_second_pass && (flags_pass2&omlayerf_Visible))
	{
		// ZBuffer options pass 2

		rdr_i->set_zbuffer_write((flags_pass2&omlayerf_EnableZBufferWrite)?omzbwc_Enabled:omzbwc_Disabled);
		rdr_i->set_zbuffer_test((flags_pass2&omlayerf_EnableZBufferTest)?omzbtc_Enabled:omzbtc_Disabled);

		if (flags_pass2&(omlayerf_SortElementBack2Front|omlayerf_SortElementFront2Back)) 
		{
			presort = &presort_list;
			presort_list.erase(presort_list.begin(),presort_list.end());
		}
		else presort = NULL;

		base_hxform.trans_x = base_hxform.trans_y = base_hxform.trans_z = 0;
	
		for(omt_ElementList::iterator ei=elements.begin();
			ei!=elements.end();
			ei++)
		{
			if ((*ei)->render_mark_element_flags&(omermf_SecondPassMark|omermf_ScanSecondPassMark))
			{
				(*ei)->compute_hxform(base_hxform,hxform);
				(*ei)->render(rdr_i,hxform,viewmatrix,proj_matrix,its_world->get_light_sources(),presort,omref_SecondPass);
			}
		}

		if (presort) render_presorted_element(rdr_i,presort,viewmatrix,omref_SecondPass,(flags_pass2&omlayerf_SortElementBack2Front)!=0);
	}
	
	
	// Force polygons to be sent to the video
	
	rdr_i->flush_pipeline();
}

class OMediaPreSortElementPtr
{
	public:
	
	OMediaRenderPreSortedElement	*psortelementptr;
	
	inline bool operator < (const OMediaPreSortElementPtr &x) const
	{
		return psortelementptr->sort_value < x.psortelementptr->sort_value;
	}
};

void OMediaLayer::render_presorted_element(OMediaRendererInterface *rdr_i,
											omt_RenderPreSortedElementList *presort,
											OMediaMatrix_4x4 &viewmatrix,
											omt_RenderModeElementFlags render_flags,
											bool back2frontsort)
{
	OMediaPreSortElementPtr		ptr;
	vector<OMediaPreSortElementPtr>	sorter;
	
	sorter.reserve(presort->size());
	
	for(omt_RenderPreSortedElementList::iterator i=presort->begin();
		i!=presort->end();
		i++)
	{
		ptr.psortelementptr = &(*i);
		if (flags&omlayerf_SortByUntransformedZ)
		{
			if (back2frontsort) ptr.psortelementptr->sort_value = -ptr.psortelementptr->element->getz();
			else ptr.psortelementptr->sort_value = ptr.psortelementptr->element->getz();
		}
		else
		{
			if (back2frontsort) ptr.psortelementptr->sort_value = -(*i).model_matrix.m[3][2];
			else ptr.psortelementptr->sort_value = (*i).model_matrix.m[3][2];
		}
		
		sorter.push_back(ptr);
	}
	
	sort(sorter.begin(),sorter.end());
	for(vector<OMediaPreSortElementPtr>::iterator si = sorter.begin();
		si!=sorter.end();
		si++)
	{
		(*si).psortelementptr->element->render_geometry(rdr_i, (*si).psortelementptr->model_matrix,
														 viewmatrix,
														 proj_matrix,
														 its_world->get_light_sources(),
														 render_flags);
	}
}

bool OMediaLayer::world_to_viewport_coords(OMediaViewPort *vp, const OMedia3DPoint &pts, float &vpx, float &vpy)
{
	float		vp_width = (float)vp->getbwidth(), 
				vp_height = (float)vp->getbheight();

	float		hw=float(vp_width)*0.5f;
	float		hh=float(vp_height)*0.5f;
	
	OMediaMatrix_4x4	view_matrix,proj_matrix;

	// View matrix

	view_matrix.set_world_transform(	vp->getx(),vp->gety(), vp->getz(), 
									vp->get_anglex(),vp->get_angley(),vp->get_anglez());

	// Compute projection matrix

	prepare_projection_matrix(0, proj_matrix, vp_width,vp_height); 
	


	float	xyzw[4];

	xyzw[0] = pts.x;	
	xyzw[1] = pts.y;
	xyzw[2] = pts.z; 
	xyzw[3] = 1.0f;

	view_matrix.multiply(xyzw);
	proj_matrix.multiply(xyzw);

	// clipped?

	if (xyzw[0]<-xyzw[3] || xyzw[0]>xyzw[3]) return false;
	if (xyzw[1]<-xyzw[3] || xyzw[1]>xyzw[3]) return false;
	if (xyzw[2]<-xyzw[3] || xyzw[2]>xyzw[3]) return false;

	// homogenous to non-homogenous coordinates

	xyzw[0] /= xyzw[3];
	xyzw[1] /= xyzw[3];

	// clamp

	if (xyzw[0]<-1.0f) xyzw[0] = -1.0f;
	else if (xyzw[0]>1.0f) xyzw[0] = 1.0f;

	if (xyzw[1]<-1.0f) xyzw[1] = -1.0f;
	else if (xyzw[1]>1.0f) xyzw[1] = 1.0f;

	// viewport coordinates

	vpx = xyzw[0]*hw;
	vpy = xyzw[1]*hh;

	return true;
}


											
