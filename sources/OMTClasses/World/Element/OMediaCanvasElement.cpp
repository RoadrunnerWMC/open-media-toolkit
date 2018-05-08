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

#include "OMediaCanvasElement.h"
#include "OMediaCanvas.h"
#include "OMediaPickRequest.h"
#include "OMediaMathTools.h"



OMediaCanvasElement::OMediaCanvasElement()
{
	diffuse.set(1.0f,1.0f,1.0f,1.0f);
	src_blend = omblendfc_One;
	dest_blend = omblendfc_Zero;
	canv_x = canv_y = canv_z = 0.0f;
	canv_w = canv_h = 64.0f;
	canvas = NULL;
	align_h = omaac_None;
	align_v = omaac_None;
	canvas_flags = 0;
	
	set_flags(omelf_ExactTextelToPixel);
}

OMediaCanvasElement::~OMediaCanvasElement()
{
	db_update();
	if (canvas) canvas->db_unlock();
}

void OMediaCanvasElement::reset(void)
{
	OMediaElement::reset();

	diffuse.set(1.0f,1.0f,1.0f,1.0f);
	src_blend = omblendfc_One;
	dest_blend = omblendfc_Zero;
	canv_x = canv_y = canv_z = 0.0f;
	canv_w = canv_h = 64.0f;
	align_h = omaac_None;
	align_v = omaac_None;
	canvas_flags = 0;
	
	set_flags(omelf_ExactTextelToPixel);
	set_canvas(NULL);
}

	
void OMediaCanvasElement::set_canvas(OMediaCanvas *canv)
{
	if (canvas) canvas->db_unlock();
	canvas = canv;
	if (canvas) canvas->db_lock();
}
	
bool OMediaCanvasElement::render_reject(OMediaMatrix_4x4 &modelmatrix, 
											OMediaMatrix_4x4 &projectmatrix,
								  			OMediaRendererInterface *rdr_i)
{
	if (!getvisible()) return true;
	if (!canvas) return true;
	if (canvas->get_width()==0 || canvas->get_height()==0) return true;
	if (rdr_i->get_pick_mode() && (get_flags()&omelf_DisablePicking)) return true;

	return false;
}


void OMediaCanvasElement::render_geometry(OMediaRendererInterface *rdr_i, OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &viewmatrix, OMediaMatrix_4x4 &projectmatrix, omt_LightList	 *lights, omt_RenderModeElementFlags render_flags)
{
	OMediaPickRequest	*pick_request;
	float				canv_w,canv_h;
	
	get_canv_wordsize(canv_w,canv_h);
	
	pick_request = rdr_i->get_pick_mode();
	if (pick_request) 
	{
		rdr_i->start_picking(1);
		rdr_i->set_picking_id(1);
	}


	rdr_i->set_model_view(modelmatrix);
	rdr_i->disable_lighting();

	if (src_blend==omblendfc_One && dest_blend==omblendfc_Zero) rdr_i->set_blend(omblendc_Disabled);
	else
	{
		rdr_i->set_blend(omblendc_Enabled);
		rdr_i->set_blend_func(src_blend, dest_blend);
	}

	rdr_i->set_shade_mode(omshademc_Flat);
	rdr_i->set_fill_mode(omfillmc_Solid);
	rdr_i->disable_faceculling();

	rdr_i->draw_surface(canvas, canv_x, canv_y, canv_z, canv_w, canv_h, diffuse);

	if (pick_request) process_canvas_picking(pick_request,rdr_i->end_picking(),modelmatrix,projectmatrix);
}


void OMediaCanvasElement::process_canvas_picking(OMediaPickRequest *pick_request, 
												 vector<OMediaPickHit> *hit_list,
												 OMediaMatrix_4x4 &modelmatrix,
												 OMediaMatrix_4x4 &projectmatrix)
{
	OMediaPickResult	result;
	OMediaPickSubResult	sub_result;


	result.type = omptc_Element;
	result.viewport = omc_NULL;	// Should be filled later
	result.element = this;
	result.shape = omc_NULL;
	result.canvas = canvas;

	if (hit_list->size())
	{
		sub_result.minz = (*hit_list)[0].minz;
		sub_result.maxz = (*hit_list)[0].maxz;
		sub_result.polygon = -1;
		sub_result.surface_hit = false;
		result.sub_info.push_back(sub_result);

		if (pick_request->level==omplc_Surface) process_canvas_surface_picking(pick_request,result,modelmatrix,projectmatrix);
		pick_request->hits.push_back(result);
		
	}
}

void OMediaCanvasElement::process_canvas_surface_picking(	OMediaPickRequest *pick_request,
															OMediaPickResult	&result,
															OMediaMatrix_4x4 &modelmatrix,
															OMediaMatrix_4x4 &projectmatrix)
{
	OMediaProjectPointUV	pp[4];
	float					outz;
	OMediaPickSubResult		*sub_res;
	float					canv_w,canv_h;
	
	get_canv_wordsize(canv_w,canv_h);

	sub_res = &(*result.sub_info.begin());
	
	// Define the surface

	pp[0].x = canv_x;
	pp[0].y = canv_y;
	pp[0].z = canv_z;
	pp[0].w = 1.0f;
	pp[0].u = 0.0f;
	pp[0].v = 1.0;

	pp[1].x = canv_x+canv_w;
	pp[1].y = canv_y;
	pp[1].z = canv_z;
	pp[1].w = 1.0f;
	pp[1].u = 1.0f;
	pp[1].v = 1.0f;

	pp[2].x = canv_x+canv_w;
	pp[2].y = canv_y+canv_h;
	pp[2].z = canv_z;
	pp[2].w = 1.0f;
	pp[2].u = 1.0f;
	pp[2].v = 0.0f;

	pp[3].x = canv_x;
	pp[3].y = canv_y+canv_h;
	pp[3].z = canv_z;
	pp[3].w = 1.0f;
	pp[3].u = 0.0f;
	pp[3].v = 0.0f;

	if (!OMediaMathTools::project_point_uv(pp,4,pick_request->normalized_px,
											pick_request->normalized_py,
											modelmatrix,
											projectmatrix,
											outz,
											sub_res->u,
											sub_res->v,
											sub_res->inv_w))
	{
		sub_res->u = 0.0f;
		sub_res->v = 0.0f;
		sub_res->inv_w = 1.0f;
		sub_res->surface_hit = false;
	}
	else
		sub_res->surface_hit = true;
								
}

void OMediaCanvasElement::update_logic(float millisecs_elapsed)
{
	OMediaElement::update_logic(millisecs_elapsed);

	compute_auto_align();
}

void OMediaCanvasElement::compute_auto_align(void)
{
	float	canv_w, canv_h;

	get_canv_wordsize(canv_w,canv_h);

	if (align_h != omaac_None)
	{
		switch(align_h)
		{
			case omaac_Left:
			canv_x = 0;
			break;

			case omaac_Right:
			canv_x = -canv_w;
			break;

			case omaac_Center:
			canv_x = -(canv_w * 0.5f);
			break;
                        
                        default: break;
		}
	}

	if (align_v != omaac_None)
	{
		switch(align_v)
		{
			case omaac_Top:
			canv_y = -canv_h;
			break;

			case omaac_Bottom:
			canv_y = 0;
			break;

			case omaac_Center:
			canv_y = -(canv_h * 0.5f);
			break;
                        
                        default: break;
		}
	}
}

void OMediaCanvasElement::get_canv_wordsize(float &w, float &h)
{
	if (canvas_flags&omcanef_FreeWorldSize)
	{
		w =  OMediaCanvasElement::canv_w;
		h =  OMediaCanvasElement::canv_h;
	}
	else
	{
		if (canvas)
		{
			w = float(canvas->get_width());
			h = float(canvas->get_height());
		}
		else
		{
			w = 0;
			h = 0;
		}
	}
}

bool OMediaCanvasElement::validate_closer_hit(OMediaPickResult &hit,
												OMediaPickSubResult &closer)
{
	if (get_flags()&omelf_CloserHitCheckAlpha)
	{
		if (!hit.canvas) return false;
	
		long	px,py;
		long	cw = canvas->get_width(), ch = canvas->get_height();
		omt_RGBAPixel	pix;
			
		px = long(closer.u * float(cw));
		py = long(closer.v * float(ch));
			
		px %= cw;
		py %= ch;
		if (px<0) px += cw;
		if (py<0) py += ch;

		hit.canvas->lock(omlf_Read);
		hit.canvas->read_pixel(pix, px, py);
		hit.canvas->unlock();

		return ( ((char*)&pix)[omd_CGUN_A])!=0;
	}	
	
	return true;
}

OMediaDBObject *OMediaCanvasElement::db_builder(void)
{
	return new OMediaCanvasElement;
}

void OMediaCanvasElement::read_class(OMediaStreamOperators &stream)
{
	short	s;
	OMediaDBObjectStreamLink	slink;

	OMediaElement::read_class(stream);
		
	stream>>slink;
	set_canvas((OMediaCanvas*)slink.get_object());

	stream>>diffuse.alpha;
	stream>>diffuse.red;
	stream>>diffuse.green;
	stream>>diffuse.blue;

	stream>>canv_x;
	stream>>canv_y;
	stream>>canv_z;
	stream>>canv_w;
	stream>>canv_h;
	
	stream>>s;	src_blend = omt_BlendFunc(s);
	stream>>s;	dest_blend = omt_BlendFunc(s);

	stream>>s;	align_h = omt_AutoAlign(s);
	stream>>s;	align_v = omt_AutoAlign(s);

	stream>>canvas_flags;
}

void OMediaCanvasElement::write_class(OMediaStreamOperators &stream)
{
	short	s;
	OMediaDBObjectStreamLink	slink;

	OMediaElement::write_class(stream);

	slink.set_object(canvas);
	stream<<slink;

	stream<<diffuse.alpha;
	stream<<diffuse.red;
	stream<<diffuse.green;
	stream<<diffuse.blue;

	stream<<canv_x;
	stream<<canv_y;
	stream<<canv_z;
	stream<<canv_w;
	stream<<canv_h;
	
	s = short(src_blend);	stream<<s;
	s = short(dest_blend);	stream<<s;

	s = short(align_h);		stream<<s;
	s = short(align_v);		stream<<s;

	stream<<canvas_flags;
}

unsigned long OMediaCanvasElement::get_approximate_size(void)
{
	return OMediaElement::get_approximate_size();
}

unsigned long OMediaCanvasElement::db_get_type(void)
{
	return OMediaCanvasElement::db_type;
}
