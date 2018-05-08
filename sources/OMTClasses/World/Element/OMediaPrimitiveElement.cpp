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
 

#include "OMediaPrimitiveElement.h"
#include "OMediaCanvas.h"
#include "OMediaPickRequest.h"

OMediaPrimitiveElement::OMediaPrimitiveElement()
{
	src_blend = omblendfc_One;
	dest_blend = omblendfc_Zero;
	draw_mode = omrdmc_Points;
	primitive_flags = 0;
	primitive_shade_mode = omshademc_Flat;
	texture = NULL;
	texture_address_mode = omtamc_Wrap;
	texture_color_operation = omtcoc_Modulate;
}

OMediaPrimitiveElement::~OMediaPrimitiveElement()
{
	db_update();
	if (texture) texture->db_unlock();
}

void OMediaPrimitiveElement::reset(void)
{
	OMediaElement::reset();
	src_blend = omblendfc_One;
	dest_blend = omblendfc_Zero;
	draw_mode = omrdmc_Points;
	primitive_flags = 0;
	primitive_shade_mode = omshademc_Flat;
	set_texture(NULL);
	vertex_list.erase(vertex_list.begin(),vertex_list.end());
}

void OMediaPrimitiveElement::set_texture(OMediaCanvas *c)
{
	if (texture) texture->db_unlock();
	texture = c;
	if (texture) texture->db_lock();
}

bool OMediaPrimitiveElement::render_reject(OMediaMatrix_4x4 &modelmatrix, 
											OMediaMatrix_4x4 &projectmatrix,
								  			OMediaRendererInterface *rdr_i)
{
	if (!getvisible()) return true;
	if (rdr_i->get_pick_mode() && (get_flags()&omelf_DisablePicking)) return true;
	return false;
}

void OMediaPrimitiveElement::render_geometry(OMediaRendererInterface *rdr_i, 
										OMediaMatrix_4x4 &modelmatrix, 
										OMediaMatrix_4x4 &viewmatrix, 
										OMediaMatrix_4x4 &projectmatrix, 
										omt_LightList	 *lights, 
										omt_RenderModeElementFlags render_flags)
{
	OMediaPickRequest	*pick_request;
	
	pick_request = rdr_i->get_pick_mode();
	if (pick_request)
	{
		rdr_i->start_picking(1);
		rdr_i->set_picking_id(0);
	}

	rdr_i->set_model_view(modelmatrix);

	if (primitive_flags&ompref_FaceCulling)  rdr_i->enable_faceculling();
	else rdr_i->disable_faceculling();

	rdr_i->disable_lighting();

	if (src_blend==omblendfc_One && dest_blend==omblendfc_Zero) rdr_i->set_blend(omblendc_Disabled);
	else
	{
		rdr_i->set_blend(omblendc_Enabled);
		rdr_i->set_blend_func(src_blend, dest_blend);
	}

	rdr_i->set_shade_mode(primitive_shade_mode);
	rdr_i->set_fill_mode(omfillmc_Solid);
	rdr_i->set_texture(texture);
	rdr_i->set_texture_address_mode(texture_address_mode);
	rdr_i->set_texture_color_operation(texture_color_operation);
	rdr_i->set_extra_texture_passes(&texture_passes);

	rdr_i->draw(vertex_list,draw_mode);

	if (pick_request) 
		process_primitive_picking(pick_request,rdr_i->end_picking());
}


void OMediaPrimitiveElement::process_primitive_picking(OMediaPickRequest *pick_request, vector<OMediaPickHit> *hit_list)
{
	OMediaPickResult	result;
	OMediaPickSubResult	sub_result;


	result.type = omptc_Element;
	result.viewport = omc_NULL;	// Should be filled later
	result.element = this;
	result.shape = omc_NULL;
	result.canvas = omc_NULL;

	if (hit_list->size())
	{
		sub_result.minz = (*hit_list)[0].minz;
		sub_result.maxz = (*hit_list)[0].maxz;
		sub_result.polygon = -1;
		sub_result.surface_hit = true;	// Don't test the surface hit... Always true...
		sub_result.u = 0.0f;
		sub_result.v = 0.0f;
		sub_result.inv_w = 1.0f;

		result.sub_info.push_back(sub_result);
		pick_request->hits.push_back(result);
	}
} 

OMediaDBObject *OMediaPrimitiveElement::db_builder(void)
{
	return new OMediaPrimitiveElement;
}

void OMediaPrimitiveElement::read_class(OMediaStreamOperators &stream)
{
	short						s;
	long						n;
	OMediaDBObjectStreamLink	slink;
	OMediaRenderVertex			rnd_v;

	OMediaElement::read_class(stream);

	stream>>n;

	while(n--)
	{
		stream>>rnd_v.x;
		stream>>rnd_v.y;
		stream>>rnd_v.z;
		stream>>rnd_v.diffuse.alpha;
		stream>>rnd_v.diffuse.red;
		stream>>rnd_v.diffuse.green;
		stream>>rnd_v.diffuse.blue;
		stream>>rnd_v.specular.red;
		stream>>rnd_v.specular.green;
		stream>>rnd_v.specular.blue;
		stream>>rnd_v.u;
		stream>>rnd_v.v;
		stream>>rnd_v.normal.x;
		stream>>rnd_v.normal.y;
		stream>>rnd_v.normal.z;

		vertex_list.push_back(rnd_v);
	}

	stream>>s;	draw_mode = omt_RenderDrawMode(s);
	stream>>s;	primitive_shade_mode = omt_ShadeMode(s);
	stream>>s;	src_blend = omt_BlendFunc(s);	
	stream>>s;	dest_blend = omt_BlendFunc(s);				

	stream>>primitive_flags;

	stream>>slink;
	set_texture((OMediaCanvas*)slink.get_object());
}

void OMediaPrimitiveElement::write_class(OMediaStreamOperators &stream)
{
	long		n;
	short		s;
	OMediaDBObjectStreamLink	slink;

	OMediaElement::write_class(stream);

	n = vertex_list.size();
	stream<<n;

	for(omt_RenderVertexList::iterator	vi = vertex_list.begin();
		vi != vertex_list.end();
		vi++)
	{
		stream<<(*vi).x;
		stream<<(*vi).y;
		stream<<(*vi).z;
		stream<<(*vi).diffuse.alpha;
		stream<<(*vi).diffuse.red;
		stream<<(*vi).diffuse.green;
		stream<<(*vi).diffuse.blue;
		stream<<(*vi).specular.red;
		stream<<(*vi).specular.green;
		stream<<(*vi).specular.blue;
		stream<<(*vi).u;
		stream<<(*vi).v;
		stream<<(*vi).normal.x;
		stream<<(*vi).normal.y;
		stream<<(*vi).normal.z;
	}

	s = short(draw_mode);				stream<<s;
	s = short(primitive_shade_mode);	stream<<s;
	s = short(src_blend);				stream<<s;
	s = short(dest_blend);				stream<<s;

	stream<<primitive_flags;

	slink.set_object(texture);
	stream<<slink;
}

unsigned long OMediaPrimitiveElement::get_approximate_size(void)
{
	return OMediaElement::get_approximate_size();
}
  
unsigned long OMediaPrimitiveElement::db_get_type(void)
{
	return OMediaPrimitiveElement::db_type;
}


