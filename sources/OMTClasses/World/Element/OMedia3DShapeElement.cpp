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
 

#include "OMedia3DShapeElement.h"
#include "OMedia3DShape.h"
#include "OMediaRendererInterface.h"
#include "OMediaPickRequest.h"
#include "OMediaMathTools.h"


OMedia3DShapeElement::OMedia3DShapeElement()
{
	shape = NULL;
}

OMedia3DShapeElement::~OMedia3DShapeElement()
{
	db_update();
	if (shape) shape->db_unlock(); 
}

void OMedia3DShapeElement::reset(void)
{
	OMediaElement::reset();

	if (shape) shape->db_unlock();
	shape = NULL;
}

bool OMedia3DShapeElement::render_reject(OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &projectmatrix,
								  			OMediaRendererInterface *rdr_i)
{
	OMediaSphere		sphere;

	if (!getvisible()) return true;
	if (!shape) return true;
	if (rdr_i->get_pick_mode() && (get_flags()&omelf_DisablePicking)) return true;

	shape->get_global_sphere(sphere);
	modelmatrix.multiply(sphere);

	return clip_sphere(sphere,modelmatrix,projectmatrix);
}


void OMedia3DShapeElement::render_geometry(OMediaRendererInterface *rdr_i, OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &viewmatrix, 
											OMediaMatrix_4x4 &projectmatrix, 
											omt_LightList	 *lights, 
											omt_RenderModeElementFlags render_flags)
{
	bool				pass2;
	OMediaSphere		sphere;
	OMediaPickRequest	*pick_request;
	
	pick_request = rdr_i->get_pick_mode();
	if (pick_request) 
	{
		rdr_i->start_picking(shape->get_polygons()->size());
	}

	shape->get_global_sphere(sphere);
	modelmatrix.multiply(sphere);

	prepare_lights(rdr_i,sphere,lights,viewmatrix);

	rdr_i->set_model_view(modelmatrix);

	pass2 = (render_flags&omref_SecondPass)!=0;
	render_draw_shape(rdr_i,shape, pass2);
	if (pass2) render_mark_element_flags |= omermf_SecondPassMark;

	if (pick_request) process_shape_picking(pick_request,rdr_i->end_picking(),modelmatrix,projectmatrix);
}

void OMedia3DShapeElement::render_draw_shape(OMediaRendererInterface *rdr_i,OMedia3DShape	*shape, bool &pass2)
{
	rdr_i->draw_shape(shape,pass2);	
}

void OMedia3DShapeElement::process_shape_picking(OMediaPickRequest *pick_request, vector<OMediaPickHit> *hit_list,
															OMediaMatrix_4x4 		&modelmatrix,
															OMediaMatrix_4x4 		&projectmatrix)
{
	OMediaPickResult	result,*lres;
	OMediaPickSubResult	sub_result;

	result.type = omptc_Element;
	result.viewport = omc_NULL;	// Should be filled later
	result.element = this;
	result.shape = shape;
	result.canvas = omc_NULL;

	if (pick_request->level==omplc_Element)
	{
		if (hit_list->size())
		{
			sub_result.minz = (*hit_list)[0].minz;
			sub_result.maxz = (*hit_list)[0].maxz;
			sub_result.polygon = -1;
			sub_result.surface_hit = false;
			result.sub_info.push_back(sub_result);
			pick_request->hits.push_back(result);
		}
		
		return;
	}

	for(vector<OMediaPickHit>::iterator i=hit_list->begin();
		i!=hit_list->end();
		i++)
	{
		pick_request->hits.push_back(result);
		lres = &(pick_request->hits.back());

		sub_result.polygon = (*i).id;
		sub_result.minz = (*i).minz;
		sub_result.maxz = (*i).maxz;
		sub_result.surface_hit = false;

		if (pick_request->level==omplc_Surface)
		{
			process_polygon_surface_picking(pick_request,&sub_result,modelmatrix,projectmatrix);		
		}

		lres->sub_info.push_back(sub_result);
	}
}

void OMedia3DShapeElement::process_polygon_surface_picking(	OMediaPickRequest 		*pick_request,
															OMediaPickSubResult		*sub_res,
															OMediaMatrix_4x4 		&modelmatrix,
															OMediaMatrix_4x4 		&projectmatrix)
{
	OMediaProjectPointUV	*pp,*ppi;
	OMediaProjectPointUV	pp_s[4];
	OMediaProjectPointUV	*temp_pp_bug;
	float					outz;
	OMedia3DPolygon			*polygon;

	shape->lock(omlf_Read);
	
	polygon = &(*shape->get_polygons())[sub_res->polygon];
	
	if (polygon->get_num_points()>4)
	{
		temp_pp_bug = new OMediaProjectPointUV[polygon->get_num_points()];
		pp = temp_pp_bug;
	}
	else 
	{
		pp = pp_s;
		temp_pp_bug = NULL;
	}
	
	// Define the surface

	ppi = pp;
	
	for(omt_3DPolygonVertexList::iterator pi = polygon->get_vertices().begin();
		pi!=polygon->get_vertices().end();
		pi++,ppi++)
	{
		OMedia3DPoint	*v;
		
		v = &(*shape->get_vertices())[(*pi).vertex_index];

		ppi->x = v->x;
		ppi->y = v->y;
		ppi->z = v->z;
		ppi->w = 1.0f;
		ppi->u = (*pi).u;
		ppi->v = (*pi).v;
	}

	if (!OMediaMathTools::project_point_uv(pp,polygon->get_num_points(),
											pick_request->normalized_px,
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
						
	
	delete [] temp_pp_bug;			

	shape->unlock();
}

bool OMedia3DShapeElement::validate_closer_hit(OMediaPickResult &hit,
												OMediaPickSubResult &closer)
{
	if (get_flags()&omelf_CloserHitCheckAlpha)
	{
		if (!shape || hit.shape!=shape) return false;

		shape->lock(omlf_Read);

		for(vector<OMediaPickSubResult>::iterator poly_i = hit.sub_info.begin();
			poly_i!=hit.sub_info.end();
			poly_i++)
		{	
				
			OMedia3DMaterial	*mat;
			OMediaCanvas		*canvas;

			if ((*poly_i).polygon==-1) continue;

			
			mat = (*shape->get_polygons())[(*poly_i).polygon].get_material();
			if (!mat) continue;

			canvas = mat->get_texture();

			if (!canvas)
			{
				shape->unlock();
				return true;
			}
		
			long	px,py;
			long	cw = canvas->get_width(), ch = canvas->get_height();
			omt_RGBAPixel	pix;
				
			px = long((*poly_i).u * float(cw));
			py = long((*poly_i).v * float(ch));
				
			px %= cw;
			py %= ch;
			if (px<0) px += cw;
			if (py<0) py += ch;

			canvas->lock(omlf_Read);
			canvas->read_pixel(pix, px, py);
			canvas->unlock();

			if (( ((char*)&pix)[omd_CGUN_A])!=0 )
			{
				shape->unlock();
				return true;
			}
		}

		shape->unlock();
		return false;
	}	
	
	return true;
}

OMediaDBObject *OMedia3DShapeElement::db_builder(void)
{
	return new OMedia3DShapeElement;
}

void OMedia3DShapeElement::read_class(OMediaStreamOperators &stream)
{
	OMediaDBObjectStreamLink	slink;

	OMediaElement::read_class(stream);
		
	stream>>slink;
	set_shape((OMedia3DShape*)slink.get_object());
}

void OMedia3DShapeElement::write_class(OMediaStreamOperators &stream)
{
	OMediaDBObjectStreamLink	slink;

	OMediaElement::write_class(stream);

	slink.set_object(shape);
	stream<<slink;	
}

unsigned long OMedia3DShapeElement::get_approximate_size(void)
{
	return OMediaElement::get_approximate_size();
}

unsigned long OMedia3DShapeElement::db_get_type(void)
{
	return OMedia3DShapeElement::db_type;
}


void OMedia3DShapeElement::set_shape(OMedia3DShape *s)
{
	if (shape) shape->db_unlock();
	shape = s;
	if (shape) shape->db_lock();
}

OMedia3DShape *OMedia3DShapeElement::get_shape(void)
{
	return shape;
}

bool OMedia3DShapeElement::fill_coll_bounding_spheres(OMediaCollCacheBlock *block)
{
	if (!shape) return false;

	omt_SphereList	*sl = shape->get_bounding_spheres();
	
	block->vertices.erase(block->vertices.begin(),block->vertices.end());
	block->radius.erase(block->radius.begin(),block->radius.end());

	for(omt_SphereList::iterator si = sl->begin();
		si!=sl->end();
		si++)
	{
		block->vertices.push_back((*si));
		block->xform_matrix.multiply(block->vertices.back().xyzw());
		block->radius.push_back((*si).radius);
	}

	return true;
}

bool OMedia3DShapeElement::fill_coll_triangles(OMediaCollCacheBlock *block)
{
	if (!shape) return false;

	omt_VertexList	*vl = shape->get_vertices();
	
	block->vertices.erase(block->vertices.begin(),block->vertices.end());

	for(omt_VertexList::iterator vi = vl->begin();
		vi!=vl->end();
		vi++)
	{
		block->vertices.push_back((*vi));
		block->xform_matrix.multiply(block->vertices.back().xyzw());
	}

	block->polygons = shape->get_polygons();

	return true;
}

bool OMedia3DShapeElement::find_global_sphere(OMediaSphere &sphere)
{
	if (!shape) return false;
	
	shape->get_global_sphere(sphere);

	return true;
}

void OMedia3DShapeElement::unlink_shape_from_structure(OMedia3DShape *shape)
{
	if (shape==this->shape) set_shape(NULL);
	OMediaElement::unlink_shape_from_structure(shape);
}

