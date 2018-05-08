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
 

#include "OMediaElement.h"
#include "OMediaWorld.h"
#include "OMediaLayer.h"
#include "OMediaRendererInterface.h"
#include "OMediaLight.h"
#include "OMediaError.h"
#include "OMediaDataBase.h"
#include "OMediaMathTools.h"

OMediaElement::OMediaElement()
{
	its_world = NULL;
	its_layer = NULL;
	its_superelement = NULL;
	its_container = NULL;
	hide_count = 0;
	pause_count = 0;
	its_id = 0;		
	element_flags = 0;
	collision_level = omcol_None;
	coll_cache_linked = false;
}

OMediaElement::~OMediaElement()
{
	if (coll_cache_linked) (*coll_cache_iterator)->its_element = NULL;
	db_update();
	unlink();
}

void OMediaElement::reset(void)
{
	OMediaElementContainer::reset();

	place(0,0,0);
	set_angle(0,0,0);
	hide_count = 0;
	pause_count = 0;
	its_id = 0;		
	element_flags = 0;
}


void OMediaElement::container_link_element(OMediaElement *e)
{
	e->element_link(this);
}

void OMediaElement::link(OMediaElementContainer *c)
{
	c->container_link_element(this);
}

void OMediaElement::world_link(OMediaWorld *world)
{
	unlink();

	its_container = NULL;
	container_link(world);
	its_superelement = NULL;
	update_world_ptr(world);

	if (world->get_layers()->size())
	{
		link_layer(world->get_layers()->back());
	}	
}

void OMediaElement::element_link(OMediaElement *element)
{
	unlink();

	its_container = NULL;
	container_link(element);
	its_superelement = element;
	update_world_ptr(element->getworld());
}

void OMediaElement::unlink(void)
{
	unlink_layer();

	if (its_container)
	{
		container_unlink(its_container);
		return;
	}

	if (its_superelement) container_unlink(its_superelement);
	else if (its_world) container_unlink(its_world);

	update_world_ptr(NULL);

	its_superelement = NULL;
}

void OMediaElement::link_layer(OMediaLayer *layer)
{
	unlink_layer();

	if (!its_superelement)
	{
		its_layer = layer;

		if (its_layer)
		{
			its_layer->get_element_list()->push_back(this);
			layer_node = its_layer->get_element_list()->end();
			layer_node--;
		}
	}
}

void OMediaElement::unlink_layer(void)
{
	if (its_layer)
	{
		its_layer->get_element_list()->erase(layer_node);
		its_layer = NULL;
	}
}

void OMediaElement::container_link(OMediaElementContainer *container)
{
	container->get_element_list()->push_back(this);	
	container_node = --(container->get_element_list()->end());
}

void OMediaElement::container_unlink(OMediaElementContainer *container)
{
	container->get_element_list()->erase(container_node);
}


void OMediaElement::update_world_ptr(OMediaWorld *world)
{
	its_world = world;
	
	for(omt_ElementList::iterator i = elements.begin();
		i!=elements.end();
		i++) (*i)->update_world_ptr(world);
}

void OMediaElement::update_logic(float millisecs_elapsed)
{
}

void OMediaElement::hide(bool h) {hide_count += (h)?1:-1;}

void OMediaElement::pause(bool p)
{
	if (p) pause_count++;	
	else pause_count--;
}

bool OMediaElement::getvisible_extend(void)
{
	OMediaElement *superelement;

	if (!getvisible()) return false;
	
	superelement = this;
	
	for(;;)
	{
		superelement = superelement->getsuperelement();
		if (!superelement) break;
		
		if (!(superelement->getvisible())) return false;
	}
	
	return true;
}

void OMediaElement::unlink_animdef_from_structure(OMediaAnimDef *animdef)
{
	for(omt_ElementList::iterator ei=get_element_list()->begin();
		ei!=get_element_list()->end();
		ei++)
	{
		(*ei)->unlink_animdef_from_structure(animdef);
	}
}

void OMediaElement::unlink_shape_from_structure(OMedia3DShape *shape)
{
	for(omt_ElementList::iterator ei=get_element_list()->begin();
		ei!=get_element_list()->end();
		ei++)
	{
		(*ei)->unlink_shape_from_structure(shape);
	}
}

void OMediaElement::listen_to_message(omt_Message msg, void *param) 
{

}

void OMediaElement::render_sub_elements(OMediaRendererInterface 		*rdr_i, 
										OMediaRenderHTransform 			&super_hxform,
										OMediaMatrix_4x4 				&viewmatrix,
										OMediaMatrix_4x4 				&projectmatrix,
										omt_LightList	 				*lights,
						  				omt_RenderPreSortedElementList	*presort,
										omt_RenderModeElementFlags		render_flags)
{
	OMediaRenderHTransform	hxform;

	if (!(render_flags&omref_SecondPass))
	{
		// Render sub-elements
	
		for(omt_ElementList::iterator ei=elements.begin();
			ei!=elements.end();
			ei++)
		{
			(*ei)->render_mark_element_flags = 0;		
			(*ei)->compute_hxform(super_hxform,hxform);
			(*ei)->render(rdr_i, hxform,viewmatrix,projectmatrix,lights,presort,false);
			if ((*ei)->render_mark_element_flags&omermf_SecondPassMark) render_mark_element_flags|=omermf_ScanSecondPassMark;
		}		
	}
	else
	{
		// Render sub-elements

		if (render_mark_element_flags&omermf_ScanSecondPassMark)
		{
			for(omt_ElementList::iterator ei=elements.begin();
				ei!=elements.end();
				ei++)
			{
				if ((*ei)->render_mark_element_flags&(omermf_SecondPassMark|omermf_ScanSecondPassMark))
				{
					(*ei)->compute_hxform(super_hxform,hxform);
					(*ei)->render(rdr_i, hxform,viewmatrix,projectmatrix,lights,presort,render_flags);
				}
			}
		}		
	}
}

void OMediaElement::render(OMediaRendererInterface 			*rdr_i, 
							OMediaRenderHTransform 			&super_hxform,
							OMediaMatrix_4x4 				&viewmatrix,
							OMediaMatrix_4x4 				&projectmatrix,
							omt_LightList	 				*lights,
						  	omt_RenderPreSortedElementList	*presort,
							omt_RenderModeElementFlags		render_flags)
{
	bool					render_me = true;

	if ((!(element_flags&omelf_DontHideSub) && !getvisible())) return;

	if (!(render_flags&omref_SecondPass))
	{		
		if (element_flags&omelf_SecondPass) 
		{
			render_mark_element_flags |=omermf_SecondPassMark;
			render_me = false;
		}
	}
	else
	{
		if (!(render_mark_element_flags&omermf_SecondPassMark)) render_me = false;
	}

	if (!(element_flags&omelf_RenderSubElementAfter))
	{
		render_sub_elements(rdr_i,super_hxform,viewmatrix,projectmatrix,lights,presort,render_flags);
	}

	if (render_me)
	{
		OMediaMatrix_4x4	model_matrix;

		float	save_trans_x,save_trans_y;
		bool	exact_textel = (element_flags&omelf_ExactTextelToPixel) &&
								projectmatrix.hint==ommc_OrthoProjection;

		if (exact_textel)
		{	
			OMediaRect	r;
			save_trans_x = super_hxform.trans_x;
			save_trans_y = super_hxform.trans_y;
			
			super_hxform.trans_x = ceil(super_hxform.trans_x);	
			super_hxform.trans_y = ceil(super_hxform.trans_y);
			rdr_i->get_view_bounds(r);

			if (!(rdr_i->get_requirement_flags()&omrrf_ExactPixelCorrectEven))
			{
				if (r.get_width()&1) super_hxform.trans_x -= 0.5f;
				if (r.get_height()&1) super_hxform.trans_y -= 0.5f;
			}
			else
			{
				if (!(r.get_width()&1)) super_hxform.trans_x -= 0.5f;
				if (!(r.get_height()&1)) super_hxform.trans_y -= 0.5f;
			}
		}

		compute_model_matrix(super_hxform,viewmatrix,model_matrix);

		if (!render_reject(model_matrix,projectmatrix, rdr_i))
		{
			if (presort)
			{
				OMediaRenderPreSortedElement				prsoel;
				omt_RenderPreSortedElementList::iterator	prsi;
				presort->push_back(prsoel);
				prsi = presort->end();	prsi--;
				(*prsi).element = this;
				(*prsi).model_matrix = model_matrix;
			}
			else
				render_geometry(rdr_i,model_matrix,viewmatrix, projectmatrix, lights,render_flags);
		}
		
		if (exact_textel)
		{
			super_hxform.trans_x = save_trans_x;
			super_hxform.trans_y = save_trans_y;	
		}

	}	

	if (element_flags&omelf_RenderSubElementAfter)
	{
		render_sub_elements(rdr_i,super_hxform,viewmatrix,projectmatrix,lights,presort,render_flags);
	}

}

bool OMediaElement::render_reject(	OMediaMatrix_4x4 &modelmatrix, 
									OMediaMatrix_4x4 &projectmatrix,
								  	OMediaRendererInterface *rdr_i)
{
	if (rdr_i->get_pick_mode()) return true;	// By default reject element in picking mode
	return !getvisible();
}

void OMediaElement::render_geometry(OMediaRendererInterface *rdr_i, OMediaMatrix_4x4 &modelmatrix, 
																	OMediaMatrix_4x4 &viewmatrix, 
																	OMediaMatrix_4x4 &projectmatrix, 
																	omt_LightList *lights, 
																	omt_RenderModeElementFlags render_flags) {}

void OMediaElement::prepare_lights(	OMediaRendererInterface *rdr_i, 
									OMediaSphere			&world_gravity,
									omt_LightList 			*lights,
									OMediaMatrix_4x4		&viewmatrix)
{
	if (lights->size())
	{
		long	index = 0, max_light = rdr_i->get_max_lights();
		OMedia3DVector	v;
		
		rdr_i->start_light_edit();
	
		for(omt_LightList::iterator li=lights->begin();
			li!=lights->end();
			li++)
		{
			OMediaLight	*l = (*li);
		
			if (l->get_layer_key()&OMediaLight::render_layer_key)
			{
			
				l->render_compute_light(viewmatrix);

				v.set(world_gravity,l->render_position);
					
				if (l->get_light_type()==omclt_Directional ||
					(v.quick_magnitude()<=l->get_range()+world_gravity.radius))
				{
					rdr_i->enable_light(index);
					l->render_light(index, rdr_i);
					index++;
					if (index>=max_light) break;
				}
			}
		}

		while(index<max_light) rdr_i->disable_light(index++);

		rdr_i->end_light_edit();
	}
}

void OMediaElement::compute_hxform(	OMediaRenderHTransform &super_hxform,
									OMediaRenderHTransform &hxform)
{
	float	x,y,z;

	get_dynamic_offset(x,y,z);
	x += OMediaWorldPosition::x;
	y += OMediaWorldPosition::y;
	z += OMediaWorldPosition::z;

	hxform = super_hxform;

	hxform.trans_x += hxform.get_axis(omc3daxis_X).x * x;	
	hxform.trans_y += hxform.get_axis(omc3daxis_X).y * x;	
	hxform.trans_z += hxform.get_axis(omc3daxis_X).z * x;

	hxform.trans_x += hxform.get_axis(omc3daxis_Y).x * y;	
	hxform.trans_y += hxform.get_axis(omc3daxis_Y).y * y;	
	hxform.trans_z += hxform.get_axis(omc3daxis_Y).z * y;
	
	hxform.trans_x += hxform.get_axis(omc3daxis_Z).x * z;	
	hxform.trans_y += hxform.get_axis(omc3daxis_Z).y * z;	
	hxform.trans_z += hxform.get_axis(omc3daxis_Z).z * z;

	hxform.rotate(omc3daxis_Y,-angley);
	hxform.rotate(omc3daxis_X,-anglex);
	hxform.rotate(omc3daxis_Z,-anglez);
}

void OMediaElement::compute_model_matrix(OMediaRenderHTransform &hxform, 
										 OMediaMatrix_4x4 		&matrix)
{
	OMediaMatrix_4x4	m1,m2;
	omt_Angle			ax,ay,az;

	hxform.convert(ax,ay,az);

	if (element_flags&omelf_DisableRotate) m1.set_identity();
	else m1.set_rotate(ax,ay,az);

	if (element_flags&omelf_DisableTranslate) m2.set_identity();
	else m2.set_translate(hxform.trans_x,hxform.trans_y,hxform.trans_z);

	m2.multiply(m1,matrix);

	matrix.hint = ommc_RotateTranslate;
}

void OMediaElement::compute_model_matrix(OMediaRenderHTransform &hxform, 
										 OMediaMatrix_4x4 		&viewmatrix,
										 OMediaMatrix_4x4 		&matrix)
{
	OMediaMatrix_4x4	m1,m2,m3;
	omt_Angle			ax,ay,az;

	hxform.convert(ax,ay,az);

	if (element_flags&omelf_DisableRotate) m1.set_identity();
	else m1.set_rotate(ax,ay,az);

	if (element_flags&omelf_FaceViewport)
	{
		float	xyzw[4];
		xyzw[0] = hxform.trans_x;
		xyzw[1] = hxform.trans_y;
		xyzw[2] = hxform.trans_z;
		xyzw[3] = 1.0f;	
		
		viewmatrix.multiply(xyzw);
		m2.set_translate(xyzw[0],xyzw[1],xyzw[2]);
	}
	else
	{
		if (element_flags&omelf_DisableTranslate) m2.set_identity();
		else m2.set_translate(hxform.trans_x,hxform.trans_y,hxform.trans_z);
	}

	if (viewmatrix.hint==ommc_Identity || 
		(element_flags&(omelf_DisableViewportTransform|omelf_FaceViewport)))
	{
		m2.multiply(m1,matrix);
	}
	else
	{
		m2.multiply(m1,m3);
		viewmatrix.multiply(m3,matrix);
	}

	matrix.hint = ommc_RotateTranslate;
}

void OMediaElement::compute_hxform_hierarchy(OMediaRenderHTransform &hxform)
{
	if (its_superelement) its_superelement->compute_hxform_hierarchy(hxform);
	compute_hxform(hxform,hxform);
}

void OMediaElement::get_absolute_info(omt_WUnit &px, omt_WUnit &py, omt_WUnit &pz,
									 omt_WAngle &ax, omt_WAngle &ay, omt_WAngle &az)
{
	OMediaRenderHTransform hxform;

	hxform.trans_x = hxform.trans_y = hxform.trans_z = 0;

	compute_hxform_hierarchy(hxform);

	px = hxform.trans_x;
	py = hxform.trans_y;
	pz = hxform.trans_z;

	hxform.convert(ax,ay,az);
}

void OMediaElement::local_to_world_matrix(OMediaMatrix_4x4 &xform, OMediaMatrix_4x4 *view_m)
{
	OMediaRenderHTransform hxform;
	OMediaMatrix_4x4		view;

	if (!view_m) view.set_identity();

	hxform.trans_x = hxform.trans_y = hxform.trans_z = 0;
	compute_hxform_hierarchy(hxform);

	compute_model_matrix(hxform, view_m ? *view_m : view, xform);
}

void OMediaElement::local_to_world(OMedia3DPoint	&p, OMediaMatrix_4x4 *view_m)
{
	OMediaMatrix_4x4 xform;

	local_to_world_matrix(xform, view_m);

	xform.multiply(p);
}

bool OMediaElement::clip_sphere(OMediaSphere		&gravity,
								OMediaMatrix_4x4	&xform_view_matrix,
								OMediaMatrix_4x4	&xform_project_matrix)
{
	float		v,w,p;
	float 		radius = gravity.radius;

	w = (gravity.z+radius) * xform_project_matrix.m[2][3];	// w is constant now
	w += xform_project_matrix.m[3][3];

	// left

	v = (gravity.x+radius) * xform_project_matrix.m[0][0];
	v+= xform_project_matrix.m[3][0];
	if (v<-w) 
	{
		return true;
	}
	
	// right

	v = (gravity.x-radius) * xform_project_matrix.m[0][0];
	v+= xform_project_matrix.m[3][0];
	if (v>w) 
	{
		return true;
	}
	
	// top

	v = (gravity.y-radius) * xform_project_matrix.m[1][1];
	v+= xform_project_matrix.m[3][1];
	if (v>w) 
	{
		return true;
	}

	// bottom

	v = (gravity.y+radius) * xform_project_matrix.m[1][1];
	v+= xform_project_matrix.m[3][1];
	if (v<-w) 
	{
		return true;
	}

	// near

	p = (gravity.z+radius);
	v = (p * xform_project_matrix.m[2][2]) +  xform_project_matrix.m[3][2];
	w = p * xform_project_matrix.m[2][3];
	w += xform_project_matrix.m[3][3];

	if (v<-w) 
	{
		return true;
	}

	// far
	
	p = (gravity.z-radius);
	v = (p * xform_project_matrix.m[2][2]) +  xform_project_matrix.m[3][2];
	w = p * xform_project_matrix.m[2][3];
	w += xform_project_matrix.m[3][3];

	if (v>w) 
	{
		return true;
	}

	return false;
}

void OMediaElement::clicked(OMediaPickResult *res, bool mouse_down) {}	// Abstract

bool OMediaElement::validate_closer_hit(OMediaPickResult &hit,
												OMediaPickSubResult &closer)
{
	return true;		// By default always accept!
}

void OMediaElement::get_dynamic_offset(float &x, float &y, float &z)
{
	x=y=z=0;
}


OMediaDBObject *OMediaElement::db_builder(void)
{
	return new OMediaElement;
}

unsigned long OMediaElement::get_approximate_size(void)
{
	return OMediaElementContainer::get_approximate_size();
}

unsigned long OMediaElement::db_get_type(void)
{
	return OMediaElement::db_type;
}

void OMediaElement::read_class(OMediaStreamOperators &stream)
{
	long	layer;

	OMediaElementContainer::read_class(stream);

	stream>>x;
	stream>>y;
	stream>>z;
	stream>>anglex;
	stream>>angley;
	stream>>anglez;
	stream>>hide_count;
	stream>>its_id;
	stream>>element_flags;
	stream>>element_desc;

	stream>>layer;

	unlink_layer();
	if (layer!=-1 && its_world)
	{
		for(omt_LayerList::iterator li = its_world->get_layers()->begin();
			li!=its_world->get_layers()->end();
			li++)
		{
			if (layer==0)
			{
				link_layer(*li);
				break;
			}

			layer--;
		}
	}
}

void OMediaElement::write_class(OMediaStreamOperators &stream)
{
	long					layer;
	omt_LayerList::iterator li;

	OMediaElementContainer::write_class(stream);

	stream<<x;
	stream<<y;
	stream<<z;
	stream<<anglex;
	stream<<angley;
	stream<<anglez;
	stream<<hide_count;
	stream<<its_id;
	stream<<element_flags;
	stream<<element_desc;

	if (its_layer && its_world)
	{	
		layer = 0;
		for(li = its_world->get_layers()->begin();
			li!=its_world->get_layers()->end();
			li++)
		{
			if (its_layer==(*li)) break;
			layer++;
		}

		if (li==its_world->get_layers()->end()) layer = -1;
	}
	else layer = -1;

	stream<<layer;
}

bool OMediaElement::ray_intersect(OMedia3DPoint	&ray_origin,
										 OMedia3DVector	&ray_direction,
										 OMediaCollisionCache *cache,
										 float			&out_distance)
{
	OMediaCollCacheBlock	*block_a;


	bool					result = false;

	if (get_collision_level()==omcol_None) return false;

	block_a = cache->get_cache_block(this);

	// Compute matrix

	if (!(block_a->init_flags&omccif_Matrix))
	{
		OMediaRenderHTransform	hxform;
		hxform.trans_x=hxform.trans_y=hxform.trans_z=0.0f;
		compute_hxform_hierarchy(hxform);
		compute_model_matrix(hxform,block_a->xform_matrix);
		block_a->init_flags|=omccif_Matrix;
	}

	// Global sphere first

	if (!(block_a->init_flags&omccif_GlobalSphere))
	{
		if (find_global_sphere(block_a->global_sphere))
		{
			block_a->xform_matrix.multiply(block_a->global_sphere);
			block_a->init_flags|=omccif_GlobalSphere;
		}
	}

	if ((block_a->init_flags&omccif_GlobalSphere))
	{
		short	hits;
		float	dist[2];

		// Test global sphere against ray

		if (!OMediaMathTools::sphere_ray_intersect (ray_origin.xyzw(), 
									ray_direction.xyzw(),
									block_a->global_sphere.xyzw(),
									block_a->global_sphere.radius,
									hits,dist))
									return false;
		
		if (hits==1) out_distance = dist[0];
		else out_distance = (dist[0]<dist[1])?dist[0]:dist[1];

		result = true;
	}

	if (get_collision_level()==omcol_GlobalSphere) return result;

	// More precision required

	collision_fill_cacheblock(block_a);

	if (block_a->init_flags&omccif_BoundingSpheres)
	{
		// Ray against bounding spheres
		return colltest_ray_sphe(ray_origin,ray_direction,block_a,out_distance);
	}
	else if (block_a->init_flags&omccif_Triangles)
	{
		// Ray against triangles
		return colltest_ray_tri(ray_origin,ray_direction,block_a,cache->triangle[0],cache->polygon_hits[0],out_distance);
	}

	return false;
}

bool OMediaElement::colltest_ray_sphe( OMedia3DPoint	&ray_origin,
									  OMedia3DVector	&ray_direction,
									OMediaCollCacheBlock *block_a,
								   float				&out_distance)
{
	short			hits;
	float			dist[2];

	vector<OMedia3DPoint>::iterator		a_pi;
	vector<float>::iterator				a_ri;

	for(a_pi = block_a->vertices.begin(),
		a_ri = block_a->radius.begin();
		a_pi!= block_a->vertices.end();
		a_pi++,a_ri++)
	{
		if (OMediaMathTools::sphere_ray_intersect(ray_origin.xyzw(), 
									ray_direction.xyzw(),
									(*a_pi).xyzw(),
									(*a_ri),
									hits,dist))
		{
			if (hits==1) out_distance = dist[0];
			else out_distance = (dist[0]<dist[1])?dist[0]:dist[1];
			return true;
		}
	}

	return false;
}

bool OMediaElement::colltest_ray_tri( OMedia3DPoint	&ray_origin,
									  OMedia3DVector	&ray_direction,
									 OMediaCollCacheBlock	*block_a,
									 OMedia3DPoint			*triangle,
									 OMedia3DPolygon		*&polygon_hit,
									 float					&out_distance)
{
	omt_3DPolygonVertexList::iterator	a_p1,a_p2,a_p3;
	omt_PolygonList::iterator			a_pi;
	long								a_nvertex;
	float								u,v;

	for(a_pi = block_a->polygons->begin();
		a_pi!= block_a->polygons->end();
		a_pi++)
	{
		a_nvertex = (*a_pi).get_num_points()-2;
		if (a_nvertex<1) continue;

		a_p1 = (*a_pi).get_vertices().begin();
		a_p2 = a_p1+1;
		a_p3 = a_p2+1;

		while(a_nvertex)
		{
			if (OMediaMathTools::tri_ray_intersect(ray_origin.xyzw(), 
												ray_direction.xyzw(),
												block_a->vertices[(*a_p1).vertex_index].xyzw(),	
												block_a->vertices[(*a_p2).vertex_index].xyzw(),
												block_a->vertices[(*a_p3).vertex_index].xyzw(),
												&out_distance,&u,&v))
			{
				triangle[0] = block_a->vertices[(*a_p1).vertex_index];
				triangle[1] = block_a->vertices[(*a_p2).vertex_index];
				triangle[2] = block_a->vertices[(*a_p3).vertex_index];
				polygon_hit = &(*a_pi);				
				return true;
			}

			a_nvertex--;
			a_p2++;
			a_p3++;
		}
	}

	return false;
}


bool OMediaElement::collide(OMediaElement *b, OMediaCollisionCache *cache) 
{
	OMediaCollCacheBlock	*block_a;
	OMediaCollCacheBlock	*block_b;
	OMedia3DVector			v;
	float					dist;

	bool					result = false;

	if (b->get_collision_level()==omcol_None || get_collision_level()==omcol_None) return false;

	block_a = cache->get_cache_block(this);
	block_b = cache->get_cache_block(b);

	// Compute matrix

	if (!(block_a->init_flags&omccif_Matrix))
	{
		OMediaRenderHTransform	hxform;
		hxform.trans_x=hxform.trans_y=hxform.trans_z=0.0f;
		compute_hxform_hierarchy(hxform);
		compute_model_matrix(hxform,block_a->xform_matrix);
		block_a->init_flags|=omccif_Matrix;
	}

	if (!(block_b->init_flags&omccif_Matrix))
	{
		OMediaRenderHTransform	hxform;
		hxform.trans_x=hxform.trans_y=hxform.trans_z=0.0f;
		b->compute_hxform_hierarchy(hxform);
		b->compute_model_matrix(hxform,block_b->xform_matrix);
		block_b->init_flags|=omccif_Matrix;
	}

	// Global sphere first

	if (!(block_a->init_flags&omccif_GlobalSphere))
	{
		if (find_global_sphere(block_a->global_sphere))
		{
			block_a->xform_matrix.multiply(block_a->global_sphere);
			block_a->init_flags|=omccif_GlobalSphere;
		}
	}

	if (!(block_b->init_flags&omccif_GlobalSphere))
	{
		if (b->find_global_sphere(block_b->global_sphere))
		{
			block_b->xform_matrix.multiply(block_b->global_sphere);
			block_b->init_flags|=omccif_GlobalSphere;
		}
	}

	if ((block_a->init_flags&omccif_GlobalSphere) && 
		(block_b->init_flags&omccif_GlobalSphere))
	{
		// Test global spheres

		v.set(block_a->global_sphere,block_b->global_sphere);
		dist = v.quick_magnitude();
		if (dist>(block_a->global_sphere.radius+
				  block_b->global_sphere.radius)) return false;

		result = true;
	}

	if (b->get_collision_level()==omcol_GlobalSphere && 
		get_collision_level()==omcol_GlobalSphere) return result;

	// More precision required

	collision_fill_cacheblock(block_a);
	b->collision_fill_cacheblock(block_b);

	if ((block_a->init_flags&omccif_BoundingSpheres) &&
		(block_b->init_flags&omccif_BoundingSpheres))
	{
		// Bounding spheres against bounding spheres
		return colltest_sphe_sphe(block_a,block_b);
	}
	else if ((block_a->init_flags&omccif_Triangles) &&
			(block_b->init_flags&omccif_Triangles))
	{
		// Triangles against triangles

		return colltest_tri_tri(block_a,block_b,
								cache->triangle[0],cache->triangle[1],
								cache->polygon_hits[0],
								cache->polygon_hits[1]);

	}
	else
	{
		// Sphere against triangles

		if ((block_a->init_flags&omccif_Triangles) &&
			(block_b->init_flags&omccif_BoundingSpheres))
		{
			return colltest_tri_sphe(block_a,block_b,cache->triangle[0],cache->polygon_hits[0]);	
		}
		else if ((block_b->init_flags&omccif_Triangles) &&
				 (block_a->init_flags&omccif_BoundingSpheres))
		{
			return colltest_tri_sphe(block_b,block_a,cache->triangle[1],cache->polygon_hits[1]);	
		}
		else if ((block_a->init_flags&omccif_Triangles) &&
				 (block_b->init_flags&omcol_GlobalSphere))
		{
			return colltest_tri_gsphe(block_a,block_b,cache->triangle[0],cache->polygon_hits[0]);	
		}
		else if ((block_b->init_flags&omccif_Triangles) &&
				 (block_a->init_flags&omcol_GlobalSphere))
		{
			return colltest_tri_gsphe(block_b,block_a,cache->triangle[1],cache->polygon_hits[1]);	
		}
	}

	return false;
}

bool OMediaElement::colltest_tri_gsphe(OMediaCollCacheBlock *block_a,
								       OMediaCollCacheBlock *block_b,
									   OMedia3DPoint		*triangle,
									   OMedia3DPolygon		*&polygon_hit)
{
	omt_3DPolygonVertexList::iterator	a_p1,a_p2,a_p3;
	omt_PolygonList::iterator			a_pi;
	long								a_nvertex;

	for(a_pi = block_a->polygons->begin();
		a_pi!= block_a->polygons->end();
		a_pi++)
	{
		a_nvertex = (*a_pi).get_num_points()-2;
		if (a_nvertex<1) continue;

		a_p1 = (*a_pi).get_vertices().begin();
		a_p2 = a_p1+1;
		a_p3 = a_p2+1;

		while(a_nvertex)
		{
			if (OMediaMathTools::tri_sphere_intersect(	block_a->vertices[(*a_p1).vertex_index],	
														block_a->vertices[(*a_p2).vertex_index],
														block_a->vertices[(*a_p3).vertex_index],
														block_b->global_sphere,	
														block_b->global_sphere.radius))
			{
				triangle[0] = block_a->vertices[(*a_p1).vertex_index];
				triangle[1] = block_a->vertices[(*a_p2).vertex_index];
				triangle[2] = block_a->vertices[(*a_p3).vertex_index];
				return true;
			}

			a_nvertex--;
			a_p2++;
			a_p3++;
		}
	}

	return false;


}

bool OMediaElement::colltest_tri_sphe(OMediaCollCacheBlock *block_a,
								       OMediaCollCacheBlock *block_b,
									   OMedia3DPoint		*triangle,
									   OMedia3DPolygon		*&polygon_hit)
{
	omt_3DPolygonVertexList::iterator	a_p1,a_p2,a_p3;
	omt_PolygonList::iterator			a_pi;
	long								a_nvertex;
	vector<OMedia3DPoint>::iterator		b_pi;
	vector<float>::iterator				b_ri;

	for(a_pi = block_a->polygons->begin();
		a_pi!= block_a->polygons->end();
		a_pi++)
	{
		a_nvertex = (*a_pi).get_num_points()-2;
		if (a_nvertex<1) continue;

		a_p1 = (*a_pi).get_vertices().begin();
		a_p2 = a_p1+1;
		a_p3 = a_p2+1;

		while(a_nvertex)
		{
			for(b_pi = block_b->vertices.begin(),
				b_ri = block_b->radius.begin();
				b_pi!= block_b->vertices.end();
				b_pi++,b_ri++)
			{
				if (OMediaMathTools::tri_sphere_intersect(	block_a->vertices[(*a_p1).vertex_index],	
															block_a->vertices[(*a_p2).vertex_index],
															block_a->vertices[(*a_p3).vertex_index],
															(*b_pi),	
															(*b_ri))) 
				{														
					triangle[0] = block_a->vertices[(*a_p1).vertex_index];
					triangle[1] = block_a->vertices[(*a_p2).vertex_index];
					triangle[2] = block_a->vertices[(*a_p3).vertex_index];
					polygon_hit = &(*a_pi);
					return true;
				}
			}

			a_nvertex--;
			a_p2++;
			a_p3++;
		}
	}

	return false;


}

bool OMediaElement::colltest_tri_tri(OMediaCollCacheBlock *block_a,
								     OMediaCollCacheBlock *block_b,
									 OMedia3DPoint			*triangle_a,
									 OMedia3DPoint			*triangle_b,
								     OMedia3DPolygon		*&polygon_ahit,
								     OMedia3DPolygon		*&polygon_bhit)
{
	omt_3DPolygonVertexList::iterator	a_p1,a_p2,a_p3;
	omt_3DPolygonVertexList::iterator	b_p1,b_p2,b_p3;
	omt_PolygonList::iterator			a_pi,b_pi;
	long								a_nvertex, b_nvertex;
	vector<OMediaCollTriExt>::iterator	a_texi,b_texi;
	OMedia3DMaterial *mat;

	if (!(block_a->init_flags&omccif_TriExt))
	{
		block_a->init_flags|=omccif_TriExt;
		block_a->compute_tri_ext();
	}

	if (!(block_b->init_flags&omccif_TriExt))
	{
		block_b->init_flags|=omccif_TriExt;
		block_b->compute_tri_ext();
	}

	for(a_pi = block_a->polygons->begin(),
		a_texi = block_a->tri_exts.begin();
		a_pi!= block_a->polygons->end();
		a_pi++)
	{
		a_nvertex = (*a_pi).get_num_points()-2;
		if (a_nvertex<1) continue;

		mat = (*a_pi).get_material();		
		if (mat && (mat->get_flags()&ommatf_DisableCollision)!=0) continue;

		a_p1 = (*a_pi).get_vertices().begin();
		a_p2 = a_p1+1;
		a_p3 = a_p2+1;

		while(a_nvertex)
		{
			OMedia3DPoint		*a1,*a2,*a3;

			a1 = &block_a->vertices[(*a_p1).vertex_index];
			a2 = &block_a->vertices[(*a_p2).vertex_index];
			a3 = &block_a->vertices[(*a_p3).vertex_index];

			for(b_pi = block_b->polygons->begin(),
				b_texi = block_b->tri_exts.begin();
				b_pi!= block_b->polygons->end();
				b_pi++)
			{
				b_nvertex = (*b_pi).get_num_points()-2;
				if (b_nvertex<1) continue;

				mat = (*b_pi).get_material();		
				if (mat && (mat->get_flags()&ommatf_DisableCollision)!=0) continue;


				b_p1 = (*b_pi).get_vertices().begin();
				b_p2 = b_p1+1;
				b_p3 = b_p2+1;

				while(b_nvertex)
				{
					OMedia3DPoint		*b1,*b2,*b3;

					b1 = &block_b->vertices[(*b_p1).vertex_index];	
					b2 = &block_b->vertices[(*b_p2).vertex_index];
					b3 = &block_b->vertices[(*b_p3).vertex_index];

					if ((*a_texi).intersect(*b_texi) &&
						OMediaMathTools::tri_tri_intersect(a1->xyzw(),a2->xyzw(),a3->xyzw(),b1->xyzw(),b2->xyzw(),b3->xyzw()))
					{
						triangle_a[0] = *a1;
						triangle_a[1] = *a2;
						triangle_a[2] = *a3;
						triangle_b[0] = *b1;
						triangle_b[1] = *b2;
						triangle_b[2] = *b3;
						polygon_ahit = &(*a_pi);
						polygon_bhit = &(*b_pi);
						return true;
					}

					b_nvertex--;
					b_p2++;
					b_p3++;
					b_texi++;
				}
			}

			a_nvertex--;
			a_p2++;
			a_p3++;
			a_texi++;
		}
	}

	return false;
}

bool OMediaElement::colltest_sphe_sphe(OMediaCollCacheBlock *block_a,
									   OMediaCollCacheBlock *block_b)
{
	OMedia3DVector	v;
	float			dist;

	// Bounding spheres against bounding spheres
	vector<OMedia3DPoint>::iterator		a_pi,b_pi;
	vector<float>::iterator				a_ri,b_ri;

	for(a_pi = block_a->vertices.begin(),
		a_ri = block_a->radius.begin();
		a_pi!= block_a->vertices.end();
		a_pi++,a_ri++)
	{
		for(b_pi = block_b->vertices.begin(),
			b_ri = block_b->radius.begin();
			b_pi!= block_b->vertices.end();
			b_pi++,b_ri++)
		{
			v.set(*a_pi,*b_pi);
			dist = v.quick_magnitude();
			if (dist<=((*a_ri) + (*b_ri)) ) return true;
		}
	}

	return false;
}

void OMediaElement::collision_fill_cacheblock(OMediaCollCacheBlock *block)
{
	if (get_collision_level()==omcol_Spheres)
	{
		if (!(block->init_flags&omccif_BoundingSpheres))
		{
			if (fill_coll_bounding_spheres(block))
			{
				block->init_flags |= omccif_BoundingSpheres;
			}
		}
	}
	else if (get_collision_level()==omcol_Triangles)
	{
		if (!(block->init_flags&omccif_Triangles))
		{
			if (fill_coll_triangles(block))
			{
				block->init_flags |= omccif_Triangles;
			}
		}
	}
}

bool OMediaElement::fill_coll_bounding_spheres(OMediaCollCacheBlock *block)
{
	return false;	// Abstract
}

bool OMediaElement::fill_coll_triangles(OMediaCollCacheBlock *block)
{
	return false;	// Abstract
}

bool OMediaElement::find_global_sphere(OMediaSphere &sphere)
{
	return false;	// Abstract
}

