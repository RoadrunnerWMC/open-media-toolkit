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
#ifndef OMEDIA_3DShapeElement_H
#define OMEDIA_3DShapeElement_H

#include "OMediaElement.h"

class OMedia3DShape;
class OMediaPickRequest;
class OMediaPickHit;
class OMediaPickSubResult;
class OMedia3DShapeElement;


/****************************************************/

class  OMedia3DShapeElement :	public OMediaElement
{
	public:
	
	// * Constructor/Destructor

	omtshared OMedia3DShapeElement();
	omtshared virtual ~OMedia3DShapeElement();	

	omtshared virtual void reset(void);
	

	// * Current shape (if any)

	omtshared virtual void set_shape(OMedia3DShape *shape);		// Please note that the element does not
	omtshared virtual OMedia3DShape *get_shape(void);			// own the shape. So it will never delete


	// * Render

	omtshared virtual void render_geometry(OMediaRendererInterface *rdr_i, OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &viewmatrix, OMediaMatrix_4x4 &projectmatrix, omt_LightList	 *lights, omt_RenderModeElementFlags render_flags);
	omtshared virtual bool render_reject(OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &projectmatrix,
								  			OMediaRendererInterface *rdr_i);



	// * Database/streamer support
	
	enum { db_type = 'E3ds' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void);

	// * Dependencies:
	
	omtshared virtual void unlink_shape_from_structure(OMedia3DShape *shape);

	protected:

	omtshared virtual bool fill_coll_bounding_spheres(OMediaCollCacheBlock *block);
	omtshared virtual bool fill_coll_triangles(OMediaCollCacheBlock *block);
	omtshared virtual bool find_global_sphere(OMediaSphere &sphere);

	omtshared virtual void process_shape_picking(OMediaPickRequest *pick_request, vector<OMediaPickHit> *hit_list,
															OMediaMatrix_4x4 		&modelmatrix,
															OMediaMatrix_4x4 		&projectmatrix);
	
	omtshared virtual void process_polygon_surface_picking(	OMediaPickRequest 		*pick_request,
															OMediaPickSubResult		*sub_res,
															OMediaMatrix_4x4 		&modelmatrix,
															OMediaMatrix_4x4 		&projectmatrix);

	
	omtshared virtual bool validate_closer_hit(OMediaPickResult &hit,
												OMediaPickSubResult &closer);


	omtshared virtual void render_draw_shape(OMediaRendererInterface *rdr_i, OMedia3DShape	*shape, bool &pass2);

	OMedia3DShape 	*shape;

};



#endif

