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
#ifndef OMEDIA_CollisionCache_H
#define OMEDIA_CollisionCache_H

#include "OMediaTypes.h"
#include "OMediaMatrix.h"
#include "OMediaSphere.h"
#include "OMedia3DShape.h"

#include <list>

class OMediaElement;

typedef unsigned short			omt_CollCacheInitFlags;
const omt_CollCacheInitFlags	omccif_Matrix = (1<<0);
const omt_CollCacheInitFlags	omccif_GlobalSphere = (1<<1);
const omt_CollCacheInitFlags	omccif_BoundingSpheres = (1<<2);
const omt_CollCacheInitFlags	omccif_Triangles = (1<<3);
const omt_CollCacheInitFlags	omccif_TriExt = (1<<4);

class OMediaCollTriExt
{
public:

	float	max_x,min_x;
	float	max_y,min_y;
	float	max_z,min_z;

	inline bool	intersect(const OMediaCollTriExt	&u) const
	{
		if (max_x<u.min_x) return false;
		if (u.max_x<min_x) return false;

		if (max_y<u.min_y) return false;
		if (u.max_y<min_y) return false;
		
		if (max_z<u.min_z) return false;
		if (u.max_z<min_z) return false;
		return true;
	}
};

class OMediaCollCacheBlock
{
public:
	
	OMediaElement					*its_element;

	OMediaMatrix_4x4				xform_matrix;

	OMediaSphere					global_sphere;

	vector<OMedia3DPoint>			vertices;
	omt_PolygonList					*polygons;	// This is not a copy. Simple pointer.
	vector<float>					radius;
	omt_CollCacheInitFlags			init_flags;
	vector<OMediaCollTriExt>		tri_exts;

	void add_tri_ext(const float	*V0,	const float	*V1,	const float	*V2);
	void compute_tri_ext(void);
};

typedef list<OMediaCollCacheBlock*>	omt_CollCacheBlockList;


class OMediaCollisionCache
{
public:

	omtshared OMediaCollisionCache();
	omtshared virtual ~OMediaCollisionCache();

	// Be sure to flush the cache, when the state of the elements changes
	// (like position, frame, element deleted, etc.).

	omtshared virtual void flush(void);

	// Flush only one element:

	omtshared virtual void flush(OMediaElement *e);

	
	// Maximum number of element cached. Zero means no limitation (all elements
	// are cached). Default is 20.
	inline void set_cache_size(const long n) {cache_limit = n;}
	inline long get_cache_size(void) const {return cache_limit;}

	// Obtain a cache block for an element
	omtshared virtual OMediaCollCacheBlock *get_cache_block(OMediaElement *e);

	// Triangles hitted ( a,b = a->collide(b)), in transformed coordinates (world)

	inline OMedia3DPoint	*get_hit_triangle_a(void) {return triangle[0];}
	inline OMedia3DPoint	*get_hit_triangle_b(void) {return triangle[1];}

	inline OMedia3DPolygon	*get_hit_polygon_a(void) {return polygon_hits[0];}
	inline OMedia3DPolygon	*get_hit_polygon_b(void) {return polygon_hits[1];}

protected:

	friend class OMediaElement;

	omt_CollCacheBlockList		cache;
	long						cache_limit;

	OMedia3DPoint				triangle[2][3];
	OMedia3DPolygon				*polygon_hits[2];
};
 


#endif

