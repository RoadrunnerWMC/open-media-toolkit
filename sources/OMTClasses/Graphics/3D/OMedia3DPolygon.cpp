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
 
 

#include "OMedia3DPolygon.h"
#include "OMediaSysDefs.h"
#include "OMediaError.h"
#include "OMedia3DMaterial.h"

#include "OMediaDataBase.h"
#include "OMediaDBObject.h"

OMediaDataBase	*OMedia3DPolygon::stream_db;	
OMediaDBObject	*OMedia3DPolygon::stream_obj;

long OMedia3DPolygon::reading_version;

OMedia3DPolygon::OMedia3DPolygon(const OMedia3DPolygon &x)
{
	material = NULL;
	user_id = 0;
	textextrapass_index = -1;

	*this = x;
}

OMedia3DPolygon::OMedia3DPolygon()
{
	material = NULL;
	user_id = 0;
	flags = 0;
	textextrapass_index = -1;
}

OMedia3DPolygon::~OMedia3DPolygon()
{
	if (material) material->db_unlock();
}

void OMedia3DPolygon::read_class(OMediaStreamOperators &stream)
{	
	OMedia3DPolygonVertex		vertex;
	long						l,l2;
	OMediaDBObjectStreamLink	slink;

	poly_vertices.erase(poly_vertices.begin(),poly_vertices.end());

	if (reading_version>2)
	{
		stream>>flags;
		stream>>user_id;

		stream>>l;
		while(l--)
		{
			vertex.extra_passes_uv.erase(vertex.extra_passes_uv.begin(),
											vertex.extra_passes_uv.end());

			stream>>vertex.vertex_index;		
			stream>>vertex.normal_index;		
			stream>>vertex.color_index;		
			stream>>vertex.u;
			stream>>vertex.v;

			if (reading_version>4)
			{
				OMediaExtraTexturePassUV	epuv;

				stream>>l2;
				while(l2--)
				{
					stream>>epuv.u;
					stream>>epuv.v;
					vertex.extra_passes_uv.push_back(epuv);
				}				
			}

			poly_vertices.push_back(vertex);		
		}

		stream>>normal.x;
		stream>>normal.y;
		stream>>normal.z;	
		
		stream>>slink;
		set_material((OMedia3DMaterial*)slink.get_object());

		if (reading_version>4)
		{
			stream>>textextrapass_index;
		}
	}
	else
	{
		short s,i;
		bool two_sided,selected,load_material;
		short					priority_key;
		unsigned short			n;
		float					correct_sorting,back_correct_sorting;
		
		flags = 0;
	
		stream>>two_sided;	if (two_sided) flags|=om3pf_TwoSided;
		stream>>selected;	if (selected) flags|=om3pf_Selected;
		stream>>priority_key;
		stream>>user_id;
		
		if (reading_version>0)
		{
			stream>>correct_sorting;
			stream>>back_correct_sorting;
		}
	
		stream>>s;		//sort ref point
		
		stream>>n;
		for(i=0; i<n; i++) 
		{
			vertex.color_index = 0;
		
			stream>>vertex.vertex_index;
			stream>>vertex.u;
			stream>>vertex.v;
			
			if (reading_version>1) stream>>vertex.normal_index;
			else vertex.normal_index = vertex.vertex_index;

			poly_vertices.push_back(vertex);
		}
		
		stream>>normal.x;
		stream>>normal.y;
		stream>>normal.z;
	
		stream>>load_material;
		if (load_material)
		{
			OMediaDBObjectStreamLink	slink;
			
			stream>>slink;
			set_material((OMedia3DMaterial*)slink.get_object());
		}
		else set_material(NULL);
	}
}

void OMedia3DPolygon::write_class(OMediaStreamOperators &stream)
{
	long						l;
	OMediaDBObjectStreamLink	slink;

	stream<<flags;
	stream<<user_id;

	l = poly_vertices.size();
	stream<<l;
	for(omt_3DPolygonVertexList::iterator vi = poly_vertices.begin();
		vi!= poly_vertices.end();
		vi++)
	{
		stream<<(*vi).vertex_index;
		stream<<(*vi).normal_index;
		stream<<(*vi).color_index;
		stream<<(*vi).u;
		stream<<(*vi).v;

		l = (*vi).extra_passes_uv.size();
		stream<<l;
		for(omt_ExtraTexturePassUV::iterator eti = (*vi).extra_passes_uv.begin();
			eti!=(*vi).extra_passes_uv.end();
			eti++)
		{
			stream<<(*eti).u;
			stream<<(*eti).v;
		}
	}

	stream<<normal.x;
	stream<<normal.y;
	stream<<normal.z;	

	slink.set_object(material);
	stream<<slink;

	stream<<textextrapass_index;
}


void OMedia3DPolygon::set_material(OMedia3DMaterial *m)
{
 	if (material) material->db_unlock();
 	material = m;
 	if (material) material->db_lock();
}

 
OMedia3DPolygon& OMedia3DPolygon::operator=(const OMedia3DPolygon& x)
{
	set_material(x.get_material());
	
	flags = x.get_flags();
	
	poly_vertices = x.get_vertices_const();
	user_id = x.get_id();
	x.get_normal_const(normal);

	textextrapass_index = x.get_extra_texture_pass_index();

	return *this;
}

bool OMedia3DPolygon::operator<(const OMedia3DPolygon& x) const
{
	return (unsigned long)material<(unsigned long)x.get_material();
}


short OMedia3DPolygon::num_point_shared(OMedia3DPolygon &cp)
{
	short nshared = 0;

	omt_3DPolygonVertexList::iterator	p1,p2;
	
	for(p1 = get_vertices().begin();
		p1!= get_vertices().end();
		p1++)
	{
		for(p2 = cp.get_vertices().begin();
			p2!= cp.get_vertices().end();
			p2++)
		{
			if ((*p1).vertex_index == (*p2).vertex_index) nshared++;		
		}
	}	

	return nshared;
}

void OMedia3DPolygon::set_quad(void) 
{
	OMedia3DPolygonVertex v; 
	omt_3DPolygonVertexList::iterator vi;
	
	while(poly_vertices.size()<4) poly_vertices.push_back(v);
	while(poly_vertices.size()>4) {vi = poly_vertices.end(); vi--; poly_vertices.erase(vi);}
}

void OMedia3DPolygon::set_triangle(void) 
{
	OMedia3DPolygonVertex v; 
	omt_3DPolygonVertexList::iterator vi;

	while(poly_vertices.size()<3) poly_vertices.push_back(v);
	while(poly_vertices.size()>3) {vi = poly_vertices.end(); vi--; poly_vertices.erase(vi);}
}

void OMedia3DPolygon::set_point(short p, unsigned long v, bool set_normal_index) 
{
	if(p>=get_num_points()) omd_EXCEPTION(omcerr_OutOfRange); 
	
	poly_vertices[p].vertex_index = v; 
	if (set_normal_index) poly_vertices[p].normal_index = v;
}

void OMedia3DPolygon::set_vertex_normal(short p, unsigned long n)
{
	if(p>=get_num_points()) omd_EXCEPTION(omcerr_OutOfRange); 
	poly_vertices[p].normal_index = n;
}

void OMedia3DPolygon::set_vertex_color(short p, unsigned long n)
{
	if(p>=get_num_points()) omd_EXCEPTION(omcerr_OutOfRange); 
	poly_vertices[p].color_index = n;
}



