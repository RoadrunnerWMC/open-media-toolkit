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

#include "OMediaParticleEmitter.h"
#include "OMediaPickRequest.h"

OMediaParticleEmitter::OMediaParticleEmitter()
{
	emitter_radius = 10000.0f;
	particle_struct_size = 0;
}

OMediaParticleEmitter::~OMediaParticleEmitter()
{
	for(vector<char	*>::iterator pi = particles_storage.begin();
		pi!=particles_storage.end();
		pi++)
	{
		delete [] (*pi);
	}
}

void OMediaParticleEmitter::reset(void)
{
	emitter_radius = 10000.0f;
	particle_struct_size = 0;

	for(vector<char	*>::iterator pi = particles_storage.begin();
		pi!=particles_storage.end();
		pi++)
	{
		delete [] (*pi);
	}
	
	particles_storage.erase(particles_storage.begin(),particles_storage.end());	
	alive_particles.erase(alive_particles.begin(),alive_particles.end());	
	dead_particles.erase(dead_particles.begin(),dead_particles.end());	
}

	
long OMediaParticleEmitter::get_sizeof_particle(void)
{
	return sizeof(OMediaAbstractParticle);
}

OMediaAbstractParticle *OMediaParticleEmitter::allocate_particle(void)
{
	long										l;
	OMediaAbstractParticle						*p;
	char										*c,*c2;
	vector<OMediaAbstractParticle*>::iterator	i;
	const long 									particles_pblock = 128;

	if (particle_struct_size==0) particle_struct_size = get_sizeof_particle();
	
	if (dead_particles.size())
	{
		i = dead_particles.end();	i--;
		p = (*i);	
		dead_particles.erase(i);

		alive_particles.push_back(p);
		
		p->alive_iterator = alive_particles.end();
		p->alive_iterator--;
		
		return (OMediaAbstractParticle*)p;
	}
	
	c = new char[particle_struct_size * particles_pblock];	
	particles_storage.push_back(c);
	for(l=0, c2=c; l<particles_pblock; l++,c2+=particle_struct_size)
	{
		dead_particles.push_back((OMediaAbstractParticle*)c2);
	}
	
	return allocate_particle();
}

void OMediaParticleEmitter::free_particle(OMediaAbstractParticle *p)
{
	alive_particles.erase(p->alive_iterator);
	dead_particles.push_back(p);
}

bool OMediaParticleEmitter::render_reject(OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &projectmatrix,
								  			OMediaRendererInterface *rdr_i)
{
	OMediaSphere		sphere;

	if (!getvisible()) return true;
	if (rdr_i->get_pick_mode()) return true;

	sphere.set(0,0,0);
	sphere.radius = emitter_radius;

	modelmatrix.multiply(sphere);

	return clip_sphere(sphere,modelmatrix,projectmatrix);
}

void OMediaParticleEmitter::render_geometry(OMediaRendererInterface *rdr_i, 
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

	// Default values:

	rdr_i->disable_faceculling();
	rdr_i->disable_lighting();
	rdr_i->set_blend(omblendc_Disabled);
	rdr_i->set_shade_mode(omshademc_Flat);
	rdr_i->set_fill_mode(omfillmc_Solid);
	rdr_i->set_texture(NULL);

	render_particles(rdr_i);

	if (pick_request) 
		process_particle_picking(pick_request,rdr_i->end_picking());
}

void OMediaParticleEmitter::process_particle_picking(OMediaPickRequest *pick_request, vector<OMediaPickHit> *hit_list)
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
		sub_result.surface_hit = false;
		result.sub_info.push_back(sub_result);
		pick_request->hits.push_back(result);
	}
} 

void OMediaParticleEmitter::render_particles(OMediaRendererInterface *rdr_i)
{
	// abstract
}


