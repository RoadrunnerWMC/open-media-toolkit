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


OMediaCollisionCache::OMediaCollisionCache()
{
	cache_limit = 20;
}

OMediaCollisionCache::~OMediaCollisionCache()
{
	flush();
}

void OMediaCollisionCache::flush(OMediaElement *e)
{
	if (e->coll_cache_linked)
	{
		delete (*e->coll_cache_iterator);
		cache.erase(e->coll_cache_iterator);
		e->coll_cache_linked = false;
	}
}


void OMediaCollisionCache::flush(void)
{
	omt_CollCacheBlockList::iterator i,ni;

	for(i=cache.begin();i!=cache.end();)
	{
		if ((*i)->its_element) 
		{
			if ((*i)->its_element->get_flags()& omelf_CollCacheStatic)
			{
				i++;
				continue;
			}

			(*i)->its_element->coll_cache_linked = false; 
		}

		delete (*i);

		ni = i;	ni++;
		cache.erase(i);
		i = ni;
	}

}

OMediaCollCacheBlock *OMediaCollisionCache::get_cache_block(OMediaElement *e)
{
	OMediaCollCacheBlock				*block;
	omt_CollCacheBlockList::iterator	i;

	if (e->coll_cache_linked)
	{
		block = (*e->coll_cache_iterator);

		i = cache.begin();
		if (e->coll_cache_iterator==i) return block;
		i++;
		if (e->coll_cache_iterator==i) return block;

		cache.erase(e->coll_cache_iterator);
		cache.push_front(block);
	}
	else
	{
		block = new OMediaCollCacheBlock;

		block->its_element = e;
		block->init_flags = 0;
		cache.push_front(block);

		if (cache_limit>0 && (long)cache.size()>cache_limit)
		{
			i = cache.end();	i--;
			if ((*i)->its_element) (*i)->its_element->coll_cache_linked = false; 
			delete (*i);
			cache.erase(i);
		}
	}

	e->coll_cache_iterator = cache.begin();
	return block;
}

void OMediaCollCacheBlock::add_tri_ext(const float	*V0,	const float	*V1,	const float	*V2)
{
	OMediaCollTriExt		ext;

#define	min_ext(p0,p1,p2,res)				\
	if (p0<p1)								\
	{										\
		if (p2<p0) res = p2;				\
		else res = p0;						\
	}										\
	else									\
	{										\
		if (p2<p1) res = p2;				\
		else res = p1;						\
	}

#define	max_ext(p0,p1,p2,res)				\
	if (p0>p1)								\
	{										\
		if (p2>p0) res = p2;				\
		else res = p0;						\
	}										\
	else									\
	{										\
		if (p2>p1) res = p2;				\
		else res = p1;						\
	}

	min_ext(V0[0],V1[0],V2[0],ext.min_x);		// x
	max_ext(V0[0],V1[0],V2[0],ext.max_x);		

	min_ext(V0[1],V1[1],V2[1],ext.min_y);		// y
	max_ext(V0[1],V1[1],V2[1],ext.max_y);		
	
	min_ext(V0[2],V1[2],V2[2],ext.min_z);		// z
	max_ext(V0[2],V1[2],V2[2],ext.max_z);		

	tri_exts.push_back(ext);
}


void OMediaCollCacheBlock::compute_tri_ext(void)
{
	omt_3DPolygonVertexList::iterator	p1,p2,p3;
	omt_PolygonList::iterator			pi;
	long								nvertex;

	tri_exts.reserve(polygons->size());

	for(pi = polygons->begin();
		pi!= polygons->end();
		pi++)
	{
		nvertex = (*pi).get_num_points()-2;
		if (nvertex<1) continue;

		p1 = (*pi).get_vertices().begin();
		p2 = p1+1;
		p3 = p2+1;

		while(nvertex)
		{
			OMedia3DPoint		*a1,*a2,*a3;

			a1 = &vertices[(*p1).vertex_index];
			a2 = &vertices[(*p2).vertex_index];
			a3 = &vertices[(*p3).vertex_index];

			add_tri_ext(a1->xyzw(), a2->xyzw(), a3->xyzw());

			nvertex--;
			p2++;
			p3++;
		}
	}
}


