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
#ifndef OMEDIA_ParticleEmitter_H
#define OMEDIA_ParticleEmitter_H

#include "OMediaTypes.h"
#include "OMediaElement.h"
#include "OMediaRGBColor.h"
#include "OMediaRendererInterface.h"

#include <list.h>

class OMediaParticleEmitter;

class OMediaAbstractParticle
{
	public:
	
	OMedia3DPoint		position;
	
	protected:
	
	friend class OMediaParticleEmitter;
	
	list<OMediaAbstractParticle*>::iterator	alive_iterator;
};

class OMediaParticleEmitter : public OMediaElement
{
	public: 
	
	// * Constructor/Destructor

	omtshared OMediaParticleEmitter();
	omtshared virtual ~OMediaParticleEmitter();	
	
	omtshared virtual void reset(void);
	
	// * Max distance between particles and element position. This is used
	// to clip the emitter when it is outside the view.

	inline void set_emitter_radius(const float er) {emitter_radius = er;}
	inline float get_emitter_radius(void) const {return emitter_radius;}
	
		
	// * Override this function if you need to extend the OMediaAbstractParticle
	// structure. By default returns sizeof(OMediaAbstractParticle)
	
	omtshared virtual long get_sizeof_particle(void);

	// * Allocate/free a particle
	
	omtshared OMediaAbstractParticle *allocate_particle(void);
	omtshared void free_particle(OMediaAbstractParticle *p);
	
	// * Get the active particle list

	inline list<OMediaAbstractParticle*> *get_active_particles(void) {return &alive_particles;}

	// * Render particle

	omtshared virtual void render_particles(OMediaRendererInterface *rdr_i);	// Override this function
																				// to render particles

	// * Element rendering

	omtshared void render_geometry(OMediaRendererInterface *rdr_i, 
										OMediaMatrix_4x4 &modelmatrix, 
										OMediaMatrix_4x4 &viewmatrix, 
										OMediaMatrix_4x4 &projectmatrix, 
										omt_LightList	 *lights, 
										omt_RenderModeElementFlags render_flags);


	protected:

	omtshared virtual bool render_reject(OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &projectmatrix,
								  			OMediaRendererInterface *rdr_i);


	omtshared virtual void process_particle_picking(OMediaPickRequest *pick_request, vector<OMediaPickHit> *hit_list);


	vector<char	*>		particles_storage;
	long				particle_struct_size;
	
	list<OMediaAbstractParticle*>		alive_particles;
	vector<OMediaAbstractParticle*>		dead_particles;

	float				emitter_radius;

};



#endif

