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

#include "OMedia3DShape.h"
#include "OMediaError.h"
#include "OMediaTrigo.h"
#include "OMediaStreamOperators.h"
#include "OMedia3DAxis.h" 
#include "OMediaMemStream.h"
#include "OMedia3DShapeConverter.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <set>
#include <map>
#include <math.h>


#ifdef omd_WINDOWS
#include <algorithm>
#else
#ifdef __MWERKS__
#include <algorithm>
#else
#include <algo.h>
#endif
#endif

#ifndef pi

	#ifndef PI
	#define omt_pi 3.1415926535
	#else
	#define omt_pi PI
	#endif

#else
#define omt_pi pi
#endif

OMedia3DShape::OMedia3DShape()
{
	visual_sphere.set(0,0,0);
	visual_sphere.radius=0;	
	global_bounding_sphere.set(0,0,0);
	global_bounding_sphere.radius=0;
	compression_level = omclc_DefaultCompression;
	flags = 0;
}

OMedia3DShape::~OMedia3DShape()
{
	db_update();
}

void OMedia3DShape::purge(void)
{
	delete_imp_slaves();

	vertices.erase(vertices.begin(),vertices.end());
	polygons.erase(polygons.begin(),polygons.end());
	bounding_spheres.erase(bounding_spheres.begin(),bounding_spheres.end());
	normals.erase(normals.begin(),normals.end());
	colors.erase(colors.begin(), colors.end());
	textpass_sets.erase(textpass_sets.begin(),textpass_sets.end());

	flags = 0;
}

unsigned long OMedia3DShape::db_get_type(void) const
{
	return OMedia3DShape::db_type;
}

void OMedia3DShape::find_center_offset(float &ox, float &oy, float &oz)
{
	omt_VertexList::iterator i;
	float	maxx,maxy,maxz,minx,miny,minz;
	
	lock(omlf_Read);
	
	for (i=vertices.begin(); i!=vertices.end();i++)
	{
		if (i==vertices.begin()) 
		{
			minx = maxx = i->x;
			miny = maxy = i->y;
			minz  =maxz = i->z;
		}
		else
		{
			if (i->x<minx) minx=i->x;
			else if (i->x>maxx) maxx=i->x;

			if (i->y<miny) miny=i->y;
			else if (i->y>maxy) maxy=i->y;		

			if (i->z<minz) minz=i->z;
			else if (i->z>maxz) maxz=i->z;
		}
	}

	unlock();

	ox = (((maxx - minx)/2) + minx);
	oy = (((maxy - miny)/2) + miny);
	oz = (((maxz - minz)/2) + minz);
}

void OMedia3DShape::find_center_offset(OMedia3DPoint &o, bool only_selected)
{
	float					  maxx,maxy,maxz,minx,miny,minz;
	bool				  first = true;

	lock(omlf_Read);
	
	for(omt_PolygonList::iterator ip = polygons.begin();
		ip!=polygons.end();
		ip++)
	{
		if ( (*ip).get_selected() || !only_selected)
		{
			for( short p=0;p<(*ip).get_num_points();p++)
			{
				OMedia3DPoint *i;
				i = &vertices[(*ip).get_point(p)];

				if (first) 
				{
					minx = maxx = i->x;
					miny = maxy = i->y;
					minz  =maxz = i->z;
					first = false;
				}
				else
				{
					if (i->x<minx) minx=i->x;
					else if (i->x>maxx) maxx=i->x;

					if (i->y<miny) miny=i->y;
					else if (i->y>maxy) maxy=i->y;		

					if (i->z<minz) minz=i->z;
					else if (i->z>maxz) maxz=i->z;
				}
			}
		}
	}

	unlock();

	o.x = (((maxx - minx)/2) + minx);
	o.y = (((maxy - miny)/2) + miny);
	o.z = (((maxz - minz)/2) + minz);
}

void OMedia3DShape::find_sphere(OMediaSphere &sphere, bool only_selected)
{
	lock(omlf_Read);

	// Find center

	find_center_offset(sphere, only_selected);

	// Compute radius

	float new_radius;
	sphere.radius = 0;

	for(omt_PolygonList::iterator ip = polygons.begin();
		ip!=polygons.end();
		ip++)
	{
		if ( (*ip).get_selected() || !only_selected)
		{
			for(short p=0;p<(*ip).get_num_points();p++)
			{
				OMedia3DVector	v;
				OMedia3DPoint 	*i;
				i = &vertices[(*ip).get_point(p)];

				v.set(i->x-sphere.x,i->y-sphere.y,i->z-sphere.z);

				new_radius = v.quick_magnitude();
		
		    	if (new_radius > sphere.radius) sphere.radius = new_radius;
		    }
		}
    }
    
  	unlock();
}


void OMedia3DShape::center(void)
{
	float	ox,oy,oz;
	
	find_center_offset(ox, oy, oz);
	translate(-ox,-oy,-oz);
}

void OMedia3DShape::translate(float vx, float vy, float vz)
{
	lock(omlf_Write);

	for (omt_VertexList::iterator i=vertices.begin(); i!=vertices.end();i++)
	{
		i->x += vx;
		i->y += vy;
		i->z += vz;
	}
	
	for(omt_SphereList::iterator bs=bounding_spheres.begin();bs!=bounding_spheres.end();bs++)
	{
		(*bs).x +=vx;
		(*bs).y +=vy;
		(*bs).z +=vz;
	}

	visual_sphere.x +=vx;
	visual_sphere.y +=vy;
	visual_sphere.z +=vz;

	global_bounding_sphere.x +=vx;
	global_bounding_sphere.y +=vy;
	global_bounding_sphere.z +=vz;

	unlock();
}

void OMedia3DShape::scale(float sx, float sy, float sz)
{
	lock(omlf_Write);

	float smax;

	if (sx > sy)
	{
		if (sx > sz ) smax= sx;
		else smax= sz;
	}
	else
	{
		if (sy > sz) smax= sy;
		else smax= sz;
	}
		
	for (omt_VertexList::iterator i=vertices.begin(); i!=vertices.end();i++)
	{
		i->x *= sx;
		i->y *= sy;
		i->z *= sz;
	}

	for(omt_SphereList::iterator bs=bounding_spheres.begin();bs!=bounding_spheres.end();bs++)
	{
		(*bs).x *=sx;
		(*bs).y *=sy;
		(*bs).z *=sz;
		(*bs).radius *=smax;
	}

	visual_sphere.x *=sx;
	visual_sphere.y *=sy;
	visual_sphere.z *=sz;
	visual_sphere.radius *=smax;

	global_bounding_sphere.x *=sx;
	global_bounding_sphere.y *=sy;
	global_bounding_sphere.z *=sz;
	global_bounding_sphere.radius *=smax;
	
	unlock();
}

void OMedia3DShape::rotate(short angle_x, short angle_y, short angle_z)
{
	lock(omlf_Write);

	for (omt_VertexList::iterator i=vertices.begin(); i!=vertices.end();i++)
	{
		(*i).rotate(angle_x, angle_y, angle_z);
    }
    
 	for(omt_SphereList::iterator bs=bounding_spheres.begin();bs!=bounding_spheres.end();bs++)
	{
		(*bs).rotate(angle_x, angle_y, angle_z);
	}

 	for(omt_NormalList::iterator n=normals.begin();n!=normals.end();n++)
	{
		(*n).rotate(angle_x, angle_y, angle_z);
	}

	for(omt_PolygonList::iterator p=polygons.begin();
		p!=polygons.end();
		p++)
	{
		(*p).get_normal().rotate(angle_x, angle_y, angle_z);
	}

	visual_sphere.rotate(angle_x, angle_y, angle_z);

	global_bounding_sphere.rotate(angle_x, angle_y, angle_z);
	
	unlock();

}

void OMedia3DShape::inv_rotate(short angle_x, short angle_y, short angle_z)
{
	lock(omlf_Write);

	for (omt_VertexList::iterator i=vertices.begin(); i!=vertices.end();i++)
	{
		(*i).inv_rotate(angle_x, angle_y, angle_z);
    }
    
 	for(omt_SphereList::iterator bs=bounding_spheres.begin();bs!=bounding_spheres.end();bs++)
	{
		(*bs).inv_rotate(angle_x, angle_y, angle_z);
	}

 	for(omt_NormalList::iterator n=normals.begin();n!=normals.end();n++)
	{
		(*n).inv_rotate(angle_x, angle_y, angle_z);
	}

	for(omt_PolygonList::iterator p=polygons.begin();
		p!=polygons.end();
		p++)
	{
		(*p).get_normal().inv_rotate(angle_x, angle_y, angle_z);
	}

	visual_sphere.inv_rotate(angle_x, angle_y, angle_z);

	global_bounding_sphere.inv_rotate(angle_x, angle_y, angle_z);

	unlock();
}

void OMedia3DShape::compute_polygon_normals(bool only_selected)
{
	OMedia3DVector	u,v;
	OMedia3DPoint	*p1,*p2,*p3;

	lock(omlf_Write);

	for(omt_PolygonList::iterator i=polygons.begin();
		i!=polygons.end();
		i++)
	{
		if (only_selected && !(*i).get_selected()) continue;

		p1 = (&vertices[(*i).get_point(0)]);
		p2 = (&vertices[(*i).get_point(1)]);
		p3 = (&vertices[(*i).get_point(2)]);
			
		u.set(*p1,*p2);
		v.set(*p1,*p3);
			
		v.cross_product(u,(*i).get_normal());
		(*i).get_normal().normalize();			
	}

	unlock();
}


void OMedia3DShape::prepare(omt_PrepareFlags flags)
{
	float new_radius; 
	OMedia3DVector	v;

	lock(omlf_Read|omlf_Write);
	
	if (flags&ompfc_OptimizePolygonOrder) sort(polygons.begin(),polygons.end());

	if (flags&ompfc_ComputePolygonNormals) compute_polygon_normals();
	if (flags&ompfc_ComputeVertexNormals) compute_normals();
	
	if (flags&ompfc_ComputeShapeSphere)
	{
		// Calculate center offset
		find_center_offset(visual_sphere.x, visual_sphere.y, visual_sphere.z);

		// Calculate maximum radius
		visual_sphere.radius = 0;
		for (omt_VertexList::iterator i=vertices.begin(); i!=vertices.end();i++)
		{		
			v.set(i->x-visual_sphere.x,i->y-visual_sphere.y,i->z-visual_sphere.z);

			if (flags&ompfc_QuickMagnitude) new_radius = v.quick_magnitude();
			else new_radius = v.magnitude();
		
	    		if (new_radius > visual_sphere.radius) visual_sphere.radius = new_radius;
    		}    
	}
	
	if (flags&ompfc_ComputeBoundingSphere)  compute_global_bsphere();

	if (flags&ompfc_SetDefaultTextureOrientation)
	{
		for (omt_PolygonList::iterator	i=polygons.begin();
			 i!=polygons.end();
			 i++)
		{
			if ((*i).get_num_points()>=1)
			{				
				(*i).set_text_coord_u(0,0.0);
				(*i).set_text_coord_v(0,0.0);

				if ((*i).get_num_points()>=2)
				{				
					(*i).set_text_coord_u(1,0.0);
					(*i).set_text_coord_v(1,1.0);

					if ((*i).get_num_points()>=3)
					{				
						(*i).set_text_coord_u(2,1.0);
						(*i).set_text_coord_v(2,1.0);
		
						if ((*i).get_num_points()>=4)
						{						
							(*i).set_text_coord_u(3,1.0);
							(*i).set_text_coord_v(3,0.0);
						}
					}
				}
			}
		}
	}
	
	unlock();
}

void OMedia3DShape::select_all(void)
{
	lock(omlf_Write);

	for(omt_PolygonList::iterator ip = polygons.begin();
		ip!=polygons.end();
		ip++)
	{
		(*ip).set_selected(true);
	}

	unlock();
}

void OMedia3DShape::deselect_all(void)
{
	lock(omlf_Write);

	for(omt_PolygonList::iterator ip = polygons.begin();
		ip!=polygons.end();
		ip++)
	{
		(*ip).set_selected(false);
	}

	unlock();
}


void OMedia3DShape::compute_normals(bool only_selected)
{
	vector<long>				normal_count;
	OMedia3DVector				empty_normal(0,0,1);
	omt_NormalList				save_normals;
	vector<long>				trans_n;
	omt_PolygonList::iterator	i;

	lock(omlf_Read|omlf_Write);

	if (only_selected) save_normals = normals;

	normals.erase(normals.begin(),normals.end());
	normals.insert(normals.begin(),vertices.size(),empty_normal);	
	normal_count.insert(normal_count.begin(),vertices.size(),0);

	for (i=polygons.begin();
		 i!=polygons.end();
		 i++)
	{
		if (only_selected && !(*i).get_selected()) continue;

		for(short p=0; p<(*i).get_num_points();p++)
		{
			long					index = (*i).get_point(p);

			(*i).set_vertex_normal(p,index);
			
			if (normal_count[index]==0) normals[index].set(0,0,0);
			
			normals[index] += (*i).get_normal();
			normal_count[index]++;
		}
	}

	omt_NormalList::iterator ni;
	vector<long>::iterator ci;

	for(ni=normals.begin(),ci=normal_count.begin();
		   ni!=normals.end();
		   ni++,ci++)
	{
		if ((*ci)==0) continue;
	
		float count = 1.0f/float(*ci);

		(*ni).x *= count;
		(*ni).y *= count;
		(*ni).z *= count;
	}
	
	if (only_selected)
	{
		trans_n.insert(trans_n.begin(),save_normals.size(),-1);
		
		for (i=polygons.begin();
			 i!=polygons.end();
			 i++)
		{
			if ((*i).get_selected()) continue;

			for(short p=0; p<(*i).get_num_points();p++)
			{
				long					vnorm = (*i).get_vertex_normal(p);
				
				if (trans_n[vnorm]==-1)
				{
					if (vnorm<(long)normal_count.size() && normal_count[vnorm]==0)
					{
						trans_n[vnorm] = vnorm;		
						normals[vnorm] = save_normals[vnorm];			
					}
					else
					{
						normals.push_back(save_normals[vnorm]);
						trans_n[vnorm] = normals.size()-1;
						(*i).set_vertex_normal(p,normals.size()-1);
					}
				}
				else
				{
					(*i).set_vertex_normal(p,trans_n[vnorm]);
				}
			}
		}
		
		delete_unused_normals();
	}
	
	unlock();
}

class OMediaNormBlock
{
	public:
	
	OMediaNormBlock() {poly=NULL;normal.set(0,0,0);normal_count=0;unshare_me=false;}
	
	OMedia3DPolygon	*poly;
	OMedia3DVector	normal;
	long			normal_count;
	bool		unshare_me;

};

class OMediaNormInfo
{
	public:
	vector<OMediaNormBlock>	poly_list;
};
 
void OMedia3DShape::compute_normals(float breaking_angle, bool only_selected)
{
	vector<OMediaNormInfo>	normal_info;
	OMediaNormInfo			empty_norminfo;
	vector<OMediaNormInfo>::iterator	ni;
	
	lock(omlf_Read|omlf_Write);

	compute_normals(only_selected);
	
	// Create normals to poly list
	
	normal_info.insert(normal_info.begin(),vertices.size(),empty_norminfo);

	for (omt_PolygonList::iterator	i=polygons.begin();
		 i!=polygons.end();
		 i++)
	{
		if (only_selected && !(*i).get_selected()) continue;
	
		for(short p=0; p<(*i).get_num_points();p++)
		{
			long					index = (*i).get_point(p);
			OMediaNormBlock			ninfo;
			
			ninfo.poly = &(*i);
	
			normal_info[index].poly_list.push_back(ninfo);			
		}
	}
	
	// Compute normals	

	for(ni = normal_info.begin();
		ni!= normal_info.end();
		ni++)
	{
		vector<OMediaNormBlock>	*nlist;
		unsigned long			p1,p2;
		OMedia3DVector	v1,v2;
		
		nlist = &(*ni).poly_list;

		if (nlist->size())
		{

			// Base normal
			for(p1=0;p1<nlist->size();p1++)
			{
				v1 = (*nlist)[p1].poly->get_normal();
				(*nlist)[p1].normal_count=1;
				(*nlist)[p1].normal = v1;
			}

			// Compare normal
			for(p1=0;p1<nlist->size()-1;p1++)
			{
				v1 = (*nlist)[p1].poly->get_normal();
		
				for(p2=p1+1;p2<nlist->size();p2++)
				{
					float bangle;
					bool	add_normal;
							
					v2 = (*nlist)[p2].poly->get_normal();
				
					if (p1==p2) add_normal = true;
					else
					{				
						// Check breaking angle
						bangle =  v1.dot_product(v2);
						if ((1.0-bangle)>=breaking_angle || bangle<0.0)
						{
							(*nlist)[p2].unshare_me = true;
							add_normal = false;
						}
						else add_normal = true;
					}
				
					if (add_normal)
					{
						(*nlist)[p1].normal_count++;
						(*nlist)[p1].normal += v2;
					}			
				}			
			}
		
			long index = ni - normal_info.begin();
			const float inv_count = 1.0f/((float)(*nlist)[0].normal_count);
			normals[index] = (*nlist)[0].normal;
			normals[index].x *= inv_count;
			normals[index].y *= inv_count;
			normals[index].z *= inv_count;
		}	
	}
	
	// Unshare required normals
	
	for(ni = normal_info.begin();
		ni!= normal_info.end();
		ni++)
	{
		for(vector<OMediaNormBlock>::iterator bi = (*ni).poly_list.begin();
			bi!= (*ni).poly_list.end();
			bi++)
		{
			if ((*bi).unshare_me)
			{
				unsigned long	index = ni - normal_info.begin();
				for(unsigned short pi=0;pi<(*bi).poly->get_num_points();pi++)
				{
					if (index==(*bi).poly->get_point(pi))
					{						
						float count = (float)(*bi).normal_count;
						(*bi).normal.x /= count;
						(*bi).normal.y /= count;
						(*bi).normal.z /= count;
						normals.push_back((*bi).normal);
						
						(*bi).poly->set_vertex_normal(pi,normals.size()-1);
						break;
					}
				}
			}		
		}	
	}
	
	unlock();
}


unsigned long OMedia3DShape::add_vertex(OMedia3DPoint &point)
{
	lock(omlf_Read|omlf_Write);

	unsigned long index;
	omt_VertexList::iterator vertex = vertices.begin();

	for (index=0;index<vertices.size();index++)
	{
		if (point.x==vertex[index].x &&
			point.y==vertex[index].y &&
			point.z==vertex[index].z) 
		{
			unlock();
			return index;
		}
	}
	
	vertices.push_back(point);
	long res = vertices.size()-1;
	
	unlock();
	return res;
}


void OMedia3DShape::set_material(OMedia3DMaterial *m)
{
	lock(omlf_Read|omlf_Write);

	for (omt_PolygonList::iterator	i=polygons.begin();
		 i!=polygons.end();
		 i++)
	{
		i->set_material(m);
	}
	
	unlock();
}

void OMedia3DShape::set_two_sided(bool ts)
{
	lock(omlf_Read|omlf_Write);

	for (omt_PolygonList::iterator	i=polygons.begin();
		 i!=polygons.end();
		 i++)
	{
		i->set_two_sided(ts);
	}
	
	unlock();
}


OMedia3DShape& OMedia3DShape::operator=(const OMedia3DShape& x)
{
	lock(omlf_Read|omlf_Write);

	purge();

	vertices = x.vertices;
	polygons = x.polygons;
	normals = x.normals;
	colors = x.colors;
	bounding_spheres = x.bounding_spheres;
	textpass_sets = x.textpass_sets;
	flags = x.flags;

	visual_sphere.set(x.get_vcenterx(),x.get_vcentery(),x.get_vcenterz());
	visual_sphere.radius = x.get_radius();
	x.get_global_bsphere(global_bounding_sphere);
	
	unlock();
	
	return *this;
}


// * OMT shape support

const unsigned long omt_ShapeHeader = '3DSP';

inline float omf_ReverseL2Float(unsigned long l)
{
	l = omd_IfLittleEndianReverseLong(l);
	return *((float*)(&l));
}

void OMedia3DShape::write_class(OMediaStreamOperators &stream)
{
	omt_VertexList::iterator v;
	unsigned long l = omt_ShapeHeader,version=5;
	OMediaDBObjectStreamLink	slink;
	short			es;

	#define write_enum(x) es = short(x); stream<<es


	lock(omlf_Read);

	OMediaDBObject::write_class(stream);

	stream<<l;
	stream<<version;
	stream<<visual_sphere.x;			// Save local infos
	stream<<visual_sphere.y;
	stream<<visual_sphere.z;
	stream<<visual_sphere.radius;
	stream<<global_bounding_sphere.x;
	stream<<global_bounding_sphere.y;
	stream<<global_bounding_sphere.z;
	stream<<global_bounding_sphere.radius;
	
	// Vertices
	
	l = vertices.size();
	stream<<l;
	
	for(v = vertices.begin();
		v!=vertices.end();
		v++)
	{
		stream<<v->x;
		stream<<v->y;
		stream<<v->z;
	}

	// Polygons

	l = polygons.size();
	stream<<l;

	OMedia3DPolygon::set_stream_dbase(database,this);

	for(omt_PolygonList::iterator p = polygons.begin();
		p!=polygons.end();
		p++)
	{
		stream<<(*p);
	}

	OMedia3DPolygon::set_stream_dbase(NULL,NULL);
	
	// Bounding spheres

	l = bounding_spheres.size();
	stream<<l;
	
	for(omt_SphereList::iterator s = bounding_spheres.begin();
		s!=bounding_spheres.end();
		s++)
	{
		stream<<(*s).x;
		stream<<(*s).y;
		stream<<(*s).z;
		stream<<(*s).radius;
	}

	// Normals

	l = normals.size();
	stream<<l;
	for(omt_NormalList::iterator n=normals.begin();n!=normals.end();n++)
	{
		stream<<(*n).x;
		stream<<(*n).y;
		stream<<(*n).z;
	}

	// Colors
	l = colors.size();
	stream<<l;
	for(omt_ColorList::iterator c=colors.begin();c!=colors.end();c++)
	{
		stream<<(*c).red;
		stream<<(*c).green;
		stream<<(*c).blue;
		stream<<(*c).alpha;
	}

	// Flags
	stream<<flags;

	// Extra texture passes
	l = textpass_sets.size();
	stream<<l;
	for(omt_ExtraTexturePassSetList::iterator m=textpass_sets.begin();m!=textpass_sets.end();m++)
	{
		l = (*m).pass_list.size();
		stream<<l;
		for(omt_ExtraTexturePassList::iterator p=(*m).pass_list.begin();
			p!=(*m).pass_list.end();
			p++)
		{
			slink.set_object((*p).get_texture());
			stream<<slink;

			write_enum((*p).get_texture_address_mode());
			write_enum((*p).get_texture_color_operation());
		}
	}

	unlock();
}

void OMedia3DShape::read_class(OMediaStreamOperators &stream)
{
	unsigned long 				l,l2,version;
	OMedia3DPoint				v;
	OMedia3DPolygon				p;
	OMediaSphere				s;
	OMedia3DVector				n;
	short						es;
	OMediaDBObjectStreamLink	slink;
	unsigned long				*base_buffer,*buffer;

	#define read_enum(t,f) stream>>es; f(t(es))
	#define	buffer_pop(x) x = omf_ReverseL2Float(*buffer);	buffer++

	OMedia3DShapeConverter::converter_used = false;

	lock(omlf_Read|omlf_Write);	

	OMediaDBObject::read_class(stream);
	
	stream>>l;
	
	if (l==omt_ShapeHeader)
	{
		purge();

		stream>>version;
		if (version>5) omd_EXCEPTION(omcerr_BadFormat);	
		OMedia3DPolygon::set_reading_version(version);

		stream>>visual_sphere.x;		
		stream>>visual_sphere.y;
		stream>>visual_sphere.z;
		stream>>visual_sphere.radius;
		stream>>global_bounding_sphere.x;
		stream>>global_bounding_sphere.y;
		stream>>global_bounding_sphere.z;
		stream>>global_bounding_sphere.radius;

		if (version>0 && version<3)
		{
			float					custom_farzclip;
			bool					do_custom_farzclip;

			stream>>custom_farzclip;
			stream>>do_custom_farzclip;
		}
	
		// Vertices
		
		stream>>l;
		if (l)
		{
			base_buffer = buffer = new unsigned long[l*3];
			stream.read(buffer,l*3*4);
			while(l--)
			{
				buffer_pop(v.x);
				buffer_pop(v.y);
				buffer_pop(v.z);
				vertices.push_back(v);
			}

			delete base_buffer;
		}

		// Polygons

		OMedia3DPolygon::set_stream_dbase(database,this);

		stream>>l;

		while(l--)
		{
			stream>>p;
			polygons.push_back(p);
		}

		OMedia3DPolygon::set_stream_dbase(NULL,NULL);
		
		// Bounding spheres
		
		stream>>l;
		
		while(l--)
		{
			stream>>s.x;
			stream>>s.y;
			stream>>s.z;
			stream>>s.radius;
			bounding_spheres.push_back(s);
		}
	
		// Normals

		
		stream>>l;
		if (l)
		{
			base_buffer = buffer = new unsigned long[l*3];
			stream.read(buffer,l*3*4);
			while(l--)
			{
				buffer_pop(n.x);
				buffer_pop(n.y);
				buffer_pop(n.z);
				normals.push_back(n);
			}

			delete base_buffer;
		}
		
		if (version>2)
		{
			OMediaFARGBColor	col;
		
			// Colors
			stream>>l;
			while(l--)
			{
				stream>>col.red;
				stream>>col.green;
				stream>>col.blue;
				stream>>col.alpha;
				colors.push_back(col);
			}
		}

		if (version>3)
		{
			stream>>flags; 
		}

		if (version>4)
		{
			OMediaExtraTexturePassSet		pass_set;	
			OMediaExtraTexturePass			pass;

			stream>>l;
			while(l--)
			{
				pass_set.pass_list.erase(pass_set.pass_list.begin(),pass_set.pass_list.end());

				stream>>l2;
				while(l2--)
				{
					stream>>slink;
					pass.set_texture((OMediaCanvas*)slink.get_object());

					read_enum(omt_TextureAddressMode,pass.set_texture_address_mode);
					read_enum(omt_TextureColorOperation,pass.set_texture_color_operation);
					pass_set.pass_list.push_back(pass);
				}

				textpass_sets.push_back(pass_set);
			}
		}
	}
	else
	{
		purge();

		stream.setposition(-4, omcfr_Current);
	
		OMedia3DShapeConverter::converter_used = true;

		OMedia3DShapeConverter *converter = 
			OMedia3DShapeConverter::create_best_converter(&stream,this);
		
		if (!converter) 
		{
			unlock();
			omd_EXCEPTION(omcerr_BadFormat);
		}

		converter->convert();
		delete converter;

		compression_level = omclc_DefaultCompression;
	}
	
	unlock();
}

unsigned long OMedia3DShape::get_approximate_size(void)
{
	long	res = 0;

	lock(omlf_Read);

	res += sizeof(*this) + (sizeof(OMedia3DPolygon)*polygons.size()) + 
					   (sizeof(OMedia3DPoint)*vertices.size())+
					   (sizeof(OMediaSphere)*bounding_spheres.size())+
					   (sizeof(OMedia3DVector)*normals.size()) +
					   (sizeof(OMediaFARGBColor)*colors.size());

	unlock();
	return res;
}

OMediaDBObject *OMedia3DShape::db_builder(void)
{
	return new OMedia3DShape;
}

void OMedia3DShape::compute_global_bsphere(void)
{
	omt_SphereList::iterator 		i;
	OMedia3DVector			v;

	lock(omlf_Read);
	
	global_bounding_sphere.set(0,0,0);
	if (bounding_spheres.size()==0) 
	{
		unlock();
		return;
	}
	
	for(i=bounding_spheres.begin(); i!=bounding_spheres.end();i++)
	{
		global_bounding_sphere.x += (*i).x;	
		global_bounding_sphere.y += (*i).y;	
		global_bounding_sphere.z += (*i).z;		
	}

	global_bounding_sphere.x /= bounding_spheres.size();	
	global_bounding_sphere.y /= bounding_spheres.size();	
	global_bounding_sphere.z /= bounding_spheres.size();	
	global_bounding_sphere.set_radius(0.0);	

	for(i=bounding_spheres.begin(); i!=bounding_spheres.end();i++)
	{
		v.set(global_bounding_sphere,(*i));
		float len = v.quick_magnitude()+(*i).get_radius();	
		if (len>global_bounding_sphere.get_radius()) global_bounding_sphere.set_radius(len);
	}
	
	unlock();
}

void OMedia3DShape::rotate_textures(short angle, float centerx, float centery, bool only_selected)
{
	OMedia3DPoint		p;

	lock(omlf_Write);

	for(omt_PolygonList::iterator ip = polygons.begin();
		ip!=polygons.end();
		ip++)
	{
		if ( (*ip).get_selected() || !only_selected)
		{
			for(short i=0; i<(*ip).get_num_points();i++)
			{
				p.x = (*ip).get_text_coord_u(i);
				p.y = (*ip).get_text_coord_v(i);

				p.x -= centerx;
				p.y -= centery;
				
				p.rotate(0,0,angle);
				
				p.x += centerx;
				p.y += centery;
	
				(*ip).set_text_coord_u(i,p.x);
				(*ip).set_text_coord_v(i,p.y);
			}							
		}
	}
	
	unlock();
}

void OMedia3DShape::scale_textures(float centerx, float centery, float scalex, float scaley,
														   		 bool only_selected)
{
	float	u,v;

	lock(omlf_Write);

	for(omt_PolygonList::iterator ip = polygons.begin();
		ip!=polygons.end();
		ip++)
	{
		if ( (*ip).get_selected() || !only_selected)
		{
			for(short i=0; i<(*ip).get_num_points();i++)
			{
				u = (*ip).get_text_coord_u(i);
				v = (*ip).get_text_coord_v(i);

				u-=centerx;
				v-=centery;
				u*=scalex;
				v*=scaley;
				u+=centerx*scalex;
				u+=centery*scaley;
	
				(*ip).set_text_coord_u(i,u);
				(*ip).set_text_coord_v(i,v);
			}							
		}
	}
	
	unlock();
}

void OMedia3DShape::offset_textures(float offsetx, float offsety,bool only_selected)
{
	float	u,v;

	lock(omlf_Write);

	for(omt_PolygonList::iterator ip = polygons.begin();
		ip!=polygons.end();
		ip++)
	{
		if ( (*ip).get_selected() || !only_selected)
		{
			for(short i=0; i<(*ip).get_num_points();i++)
			{
				u = (*ip).get_text_coord_u(i);
				v = (*ip).get_text_coord_v(i);

				u+=offsetx;
				v+=offsety;
	
				(*ip).set_text_coord_u(i,u);
				(*ip).set_text_coord_v(i,v);
			}							
		}
	}
	
	unlock();
}

void OMedia3DShape::map_projection(short 			angle_x,
								   short 			angle_y,
								   short 			angle_z,
								   float			offsetx, 
								   float			offsety,
								   float			scalex, 
								   float			scaley,
								   float		  	breaking_angle,	// 0.0-1.0 (0-90 degrees)
								   bool 	  	only_selected,
								   bool			invert_angles)
{
	float			x_start,y_start;
	float			x_end,y_end, vangle;	
	OMedia3DVector	vz(0,0,-1),n;
	vector<long>	polys;
	unsigned long	p;
	long			i;
	OMedia3DPolygon	*poly;
	bool		init;
	OMedia3DPoint	pt;

	lock(omlf_Write);
	
	// Reject invalid polygons
	
	for(p=0;p<polygons.size();p++)
	{
		if ( polygons[p].get_selected() || !only_selected)
		{
			// Check breaking angle
			
			n = polygons[p].get_normal();
			if (invert_angles) n.inv_rotate(-angle_x,-angle_y, -angle_z); else n.rotate(angle_x,angle_y,angle_z);
			vangle = n.dot_product(vz);
			if (vangle>=1.0-breaking_angle && vangle>0.0) polys.push_back(p);	
		}
	}
	
	// Compute projection rectangle
	
	init = true;

	for(p=0;p<polys.size();p++)
	{
		poly = &polygons[polys[p]];
		
		for(i=0;i<poly->get_num_points();i++)
		{
			pt = vertices[poly->get_point(short(i))];
			if (invert_angles) pt.inv_rotate(-angle_x,-angle_y, -angle_z); else pt.rotate(angle_x,angle_y,angle_z);
			
			if (init)
			{
				x_end = x_start = pt.x;
				y_end = y_start = pt.y;
				init = false;
			}
			else
			{
				if (pt.x<x_start) x_start = pt.x;
				if (pt.x>x_end) x_end = pt.x;
				if (pt.y<y_start) y_start = pt.y;
				if (pt.y>y_end) y_end = pt.y;
			}	
		}
	}

	if (x_start==x_end || y_start==y_end || polys.size()==0) 
	{
		unlock();
		return;
	}

	float	u_start=0.0,u_end=1.0,v_start=0.0,v_end=1.0;

	u_start *= scalex;
	u_end 	*= scalex;	
	v_start *= scaley;	
	v_end 	*= scaley;

	u_start += offsetx;
	u_end 	+= offsetx;
	v_start += offsety;
	v_end 	+= offsety;

	float	iu,iv;
	
	iu = (u_end-u_start)/(x_end-x_start);
	iv = (v_end-v_start)/(y_end-y_start);
	
	for(p=0;p<polys.size();p++)
	{
		poly = &polygons[polys[p]];
		
		for(i=0;i<poly->get_num_points();i++)
		{
			pt = vertices[poly->get_point(short(i))];
			if (invert_angles) pt.inv_rotate(-angle_x,-angle_y, -angle_z); else pt.rotate(angle_x,angle_y,angle_z);
					
			poly->set_text_coord_u(short(i),u_start+((pt.x-x_start)*iu));
			poly->set_text_coord_v(short(i),v_end-(v_start+((pt.y-y_start)*iv)));
		}		
	}
	
	unlock();
}

void OMedia3DShape::map_cubic(float offsetx, float offsety,
							  float scalex, float scaley,   
							  bool only_selected)
{
	map_projection(0,0,0,offsetx,offsety,scalex,scaley,0.5,only_selected);
	map_projection(0,omd_Deg2Angle(90),0,offsetx,offsety,scalex,scaley,0.5, only_selected);
	map_projection(0,omd_Deg2Angle(180),0,offsetx,offsety,scalex,scaley,0.5, only_selected);
	map_projection(0,omd_Deg2Angle(270),0,offsetx,offsety,scalex,scaley,0.5, only_selected);
	map_projection(omd_Deg2Angle(90),0,0,offsetx,offsety,scalex,scaley,0.5, only_selected);
	map_projection(omd_Deg2Angle(360-90),0,0,offsetx,offsety,scalex,scaley,0.5, only_selected);
}


void OMedia3DShape::make_cube(float size, bool inverted)
{
	OMedia3DPoint		p;
	OMedia3DPolygon		poly;

	lock(omlf_Write);
	
	purge();

	size/=2;
	
	p.set(size,-size,-size);
	vertices.push_back(p);	
	p.set(size,-size,size);
	vertices.push_back(p);	
	p.set(size,size,size);
	vertices.push_back(p);	
	p.set(size,size,-size);
	vertices.push_back(p);	
	p.set(-size,-size,-size);
	vertices.push_back(p);	
	p.set(-size,-size,size);
	vertices.push_back(p);	
	p.set(-size,+size,+size);
	vertices.push_back(p);	
	p.set(-size,size,-size);
	vertices.push_back(p);	

	if (inverted)
	{	
		poly.set_quad();
		poly.set_point(0,7);	poly.set_point(3,4);	poly.set_point(2,0);	poly.set_point(1,3);
		polygons.push_back(poly);
		poly.set_quad();
		poly.set_point(0,6);	poly.set_point(3,2);	poly.set_point(2,1);	poly.set_point(1,5);
		polygons.push_back(poly);
		poly.set_quad();
		poly.set_point(0,7);	poly.set_point(3,3);	poly.set_point(2,2);	poly.set_point(1,6);
		polygons.push_back(poly);
		poly.set_quad();
		poly.set_point(0,5);	poly.set_point(3,1);	poly.set_point(2,0);	poly.set_point(1,4);
		polygons.push_back(poly);
		poly.set_quad();
		poly.set_point(0,0);	poly.set_point(3,1);	poly.set_point(2,2);	poly.set_point(1,3);
		polygons.push_back(poly);
		poly.set_quad();
		poly.set_point(0,7);	poly.set_point(3,6);	poly.set_point(2,5);	poly.set_point(1,4);
		polygons.push_back(poly);
	}
	else
	{
		poly.set_quad();
		poly.set_point(0,7);	poly.set_point(1,4);	poly.set_point(2,0);	poly.set_point(3,3);
		polygons.push_back(poly);
		poly.set_quad();
		poly.set_point(0,6);	poly.set_point(1,2);	poly.set_point(2,1);	poly.set_point(3,5);
		polygons.push_back(poly);
		poly.set_quad();
		poly.set_point(0,7);	poly.set_point(1,3);	poly.set_point(2,2);	poly.set_point(3,6);
		polygons.push_back(poly);
		poly.set_quad();
		poly.set_point(0,5);	poly.set_point(1,1);	poly.set_point(2,0);	poly.set_point(3,4);
		polygons.push_back(poly);
		poly.set_quad();
		poly.set_point(0,0);	poly.set_point(1,1);	poly.set_point(2,2);	poly.set_point(3,3);
		polygons.push_back(poly);
		poly.set_quad();
		poly.set_point(0,7);	poly.set_point(1,6);	poly.set_point(2,5);	poly.set_point(3,4);
		polygons.push_back(poly);	
	}
	
	prepare(ompfc_PrepareAll&(~(ompfc_SetDefaultTextureOrientation|ompfc_ComputeVertexNormals)));
	compute_normals(0.3f);
	map_cubic();
	
	unlock();
}

void OMedia3DShape::make_sphere(float radius, short nrings, short nrays)
{
	lock(omlf_Write);

	purge();

	if (nrings<1) nrings = 1;
	if (nrays<3) nrays = 3;
	if (radius<=0) radius = 1.0;
	
	OMedia3DPolygon	poly;
	OMedia3DPoint	p1,p2,p3,p4,pt,pb;
	double			ring_i,ray_i,r;
	long			i1,i2,i3,i4,i;
	double			iu,iv,u1,v1,u2,v2,u3,v3,u4,v4,u,v;
	
	ring_i = (double(omc_MaxAngle>>1))/(nrings+1);
	ray_i = double(omc_MaxAngle)/nrays;
	iu = (1.0-0.0)/double(omc_MaxAngle);
	iv = (1.0-0.0)/double(omc_MaxAngle>>1);
	u = 0;
	v = 0;
	

	// Create top/bottom of the sphere
	
	for (i=0;i<2;i++)
	{
		if (i==0)
		{
			p1.set(0,radius,0);
			p2.set(0,radius,0);
			p2.rotate(short(ring_i),0,0);
			u1 = u;
			v1 = v;
			u2 = u;
			v2 = v + (ring_i*iv);
		}
		else
		{
			p1.set(0,-radius,0);
			p2.set(0,radius,0);
			p2.rotate(short(ring_i*nrings),0,0);

			u1 = u;
			v1 = v + (double(omc_MaxAngle>>1)*iv);
			u2 = u;
			v2 = v + ((ring_i*nrings)*iv);
		}

		p4 = p3 = p2;
		p3.rotate(0,short(ray_i),0);
		
		u3 = u + (ray_i*iu);
		v3 = v2;
		
		for(r=0;;)
		{
			i1 = add_vertex(p1);
			i2 = add_vertex(p2);
			i3 = add_vertex(p3);
			poly.set_triangle();
			poly.set_point(0,i1);
			poly.set_text_coord_u(0,float(u1));
			poly.set_text_coord_v(0,float(v1));

			if (i==0)
			{
				poly.set_point(1,i2);
				poly.set_point(2,i3);
				poly.set_text_coord_u(1,float(u2));
				poly.set_text_coord_v(1,float(v2));
				poly.set_text_coord_u(2,float(u3));
				poly.set_text_coord_v(2,float(v3));
			}
			else
			{
				poly.set_point(2,i2);
				poly.set_point(1,i3);
				poly.set_text_coord_u(2,float(u2));
				poly.set_text_coord_v(2,float(v2));
				poly.set_text_coord_u(1,float(u3));
				poly.set_text_coord_v(1,float(v3));
			}
			

			polygons.push_back(poly);
		
			r++;	
			if (r>=nrays) break;
			
			p2 = p3;
			p3 = p4;
			
			u2 = u3;
			u3 = u + ((ray_i*double(r+1))*iu);
			
			if (r!=nrays-1) p3.rotate(0,short(ray_i*double(r+1)),0);
		}		
	}

	// Fill inside

	for(i=1;i<nrings;i++)
	{
		p1.set(0,radius,0);
		p1.rotate(short(ring_i*double(i)),0,0);
		p2.set(0,radius,0);
		p2.rotate(short(ring_i*double(i+1)),0,0);
		p3 = p2;
		p3.rotate(0,short(ray_i),0);
		p4 = p1;
		p4.rotate(0,short(ray_i),0);
		pt = p1;
		pb = p2;
		
		u1 = u2 = u;
		u3 = u4 = u + (ray_i*iu);
		v1 = v4 = v + ((ring_i*double(i))*iv);
		v2 = v3 = v + ((ring_i*double(i+1))*iv);

		for(r=0;;)
		{
			i1 = add_vertex(p1);
			i2 = add_vertex(p2);
			i3 = add_vertex(p3);
			i4 = add_vertex(p4);
			poly.set_quad();
			poly.set_point(0,i1);
			poly.set_point(1,i2);
			poly.set_point(2,i3);
			poly.set_point(3,i4);

			poly.set_text_coord_u(0,float(u1));
			poly.set_text_coord_v(0,float(v1));
			poly.set_text_coord_u(1,float(u2));
			poly.set_text_coord_v(1,float(v2));
			poly.set_text_coord_u(2,float(u3));
			poly.set_text_coord_v(2,float(v3));
			poly.set_text_coord_u(3,float(u4));
			poly.set_text_coord_v(3,float(v4));

			polygons.push_back(poly);
		
			r++;	
			if (r>=nrays) break;
	
			p1 = p4;
			p2 = p3;
			p4 = pt;
			p3 = pb;
			u1 = u4;
			u2 = u3;
			u3 = u4 = u + ((ray_i*double(r+1))*iu);
			
			if (r!=nrays-1)
			{
				p3.rotate(0,short(ray_i*double(r+1)),0);
				p4.rotate(0,short(ray_i*double(r+1)),0);
			}
		}
	}
	
	prepare(ompfc_PrepareAll&(~ompfc_SetDefaultTextureOrientation));
	
	unlock();
}

void OMedia3DShape::find_cube(float &minx, float &miny, float &minz,float &maxx, float &maxy, float &maxz)
{
	minx=miny=minz=maxx=maxy=maxz=0;

	lock(omlf_Read);

	if (vertices.size()==0) 
	{
		unlock();
		return;
	}

	omt_VertexList::iterator i=vertices.begin();

	maxx = minx = (*i).x;
	maxy = miny = (*i).y;
	maxz = minz = (*i).z;

	i++;

	for(;i!=vertices.end();i++)
	{
		float x,y,z;

		x = (*i).x;
		y = (*i).y;
		z = (*i).z;

		if ( x < minx) minx = x;
		if ( y < miny) miny = y;
		if ( z < minz) minz = z;
		if ( x > maxx) maxx = x;
		if ( y > maxy) maxy = y;
		if ( z > maxz) maxz = z;
	}
	
	unlock();
}

void OMedia3DShape::translate_top(void)
{
	float minx,miny,minz,maxx,maxy,maxz;
	float	ox,oy,oz;

	lock(omlf_Read|omlf_Write);
	
	find_center_offset(ox, oy, oz);
	find_cube(minx,miny,minz,maxx,maxy,maxz);
	translate(0.0f, (-((maxy-miny)/2.0f))-oy,0.0f);

	unlock();
}

void OMedia3DShape::translate_bottom(void)
{
	float minx,miny,minz,maxx,maxy,maxz;
	float	ox,oy,oz;
	
	lock(omlf_Read|omlf_Write);
	find_center_offset(ox, oy, oz);
	find_cube(minx,miny,minz,maxx,maxy,maxz);
	translate(0.0f, (((maxy-miny)/2.0f))-oy,0.0f);
	unlock();
}

void OMedia3DShape::translate_left(void)
{
	float minx,miny,minz,maxx,maxy,maxz;
	float	ox,oy,oz;
	
	lock(omlf_Read|omlf_Write);
	find_center_offset(ox, oy, oz);
	find_cube(minx,miny,minz,maxx,maxy,maxz);
	translate((((maxx-minx)/2.0f))-ox, 0.0f ,0.0f);
	unlock();
}

void OMedia3DShape::translate_right(void)
{
	float minx,miny,minz,maxx,maxy,maxz;
	float	ox,oy,oz;
	
	lock(omlf_Read|omlf_Write);
	find_center_offset(ox, oy, oz);
	find_cube(minx,miny,minz,maxx,maxy,maxz);
	translate((-((maxx-minx)/2.0f))-ox, 0.0f ,0.0f);
	unlock();
}

void OMedia3DShape::translate_near(void)
{
	float minx,miny,minz,maxx,maxy,maxz;
	float	ox,oy,oz;
	
	lock(omlf_Read|omlf_Write);
	find_center_offset(ox, oy, oz);
	find_cube(minx,miny,minz,maxx,maxy,maxz);
	translate(0.0f, 0.0f, (((maxz-minz)/2.0f))-oz);
	unlock();

}

void OMedia3DShape::translate_far(void)
{
	float minx,miny,minz,maxx,maxy,maxz;
	float	ox,oy,oz;
	
	lock(omlf_Read|omlf_Write);
	find_center_offset(ox, oy, oz);
	find_cube(minx,miny,minz,maxx,maxy,maxz);
	translate(0.0f, 0.0f, (-((maxz-minz)/2.0f))-oz);
	unlock();
}

class OMediaTempMerge
{
	public:
	vector<OMedia3DPolygon*>	poly_used;
};

void OMedia3DShape::merge_normals(bool only_selected)
{
	vector<OMediaTempMerge>	ntopoly_list;
	OMediaTempMerge				empty_pou;
	unsigned long					i,i2;
	omt_PolygonList::iterator 		pi;
	short							p;
	vector<OMedia3DPolygon*>::iterator pui;

	lock(omlf_Read|omlf_Write);

	if (normals.size()<=1) 
	{
		unlock();
		return;
	}

	ntopoly_list.insert(ntopoly_list.begin(),normals.size(),empty_pou);

	for(pi=polygons.begin();pi<polygons.end();pi++)
	{
		if (!only_selected || (*pi).get_selected()) 
		{
			for(p=0;p<(*pi).get_num_points();p++)
			{
				ntopoly_list[(*pi).get_vertex_normal(p)].poly_used.push_back(&(*pi));
			}
		}	
	}

	for(i=0;i<normals.size()-1;i++)
	{
		for(i2=i+1;i2<normals.size();i2++)
		{
			if (normals[i]==normals[i2])
			{
				OMediaTempMerge	*tmn;
				
				tmn = &ntopoly_list[i2];
			
				for(pui = tmn->poly_used.begin();
				  	pui!= tmn->poly_used.end();
				  	pui++)
				{
					for(p=0;p<(*pui)->get_num_points();p++)
					{
						if ((*pui)->get_vertex_normal(p)==i2)
						{
							(*pui)->set_vertex_normal(p,i);
						}
					}
				}	
			}
		}
	}
	
	delete_unused_normals();
	
	unlock();
}


void OMedia3DShape::merge_points(float distance, bool only_selected)
{
	OMedia3DPoint						*p1,*p2;
	vector<OMediaTempMerge>				vtopoly_list;
	OMediaTempMerge						empty_pou;
	unsigned long						i,i2;
	omt_PolygonList::iterator 			pi;
	short								p;
	vector<OMedia3DPolygon*>::iterator pui;
	
	lock(omlf_Read|omlf_Write);

	if (vertices.size()<=1) 
	{
		unlock();
		return;
	}

	vtopoly_list.insert(vtopoly_list.begin(),vertices.size(),empty_pou);

	for(pi=polygons.begin();pi<polygons.end();pi++)
	{
		if (!only_selected || (*pi).get_selected()) 
		{
			for(p=0;p<(*pi).get_num_points();p++)
			{
				vtopoly_list[(*pi).get_point(p)].poly_used.push_back(&(*pi));
			}
		}	
	}


	for(i=0;i<vertices.size()-1;i++)
	{
		for(i2=i+1;i2<vertices.size();i2++)
		{
			p1 = &vertices[i];
			p2 = &vertices[i2];

			if (omd_Abs(p1->x-p2->x)<=distance && 
				omd_Abs(p1->y-p2->y)<=distance &&
				omd_Abs(p1->z-p2->z)<=distance)
			{
				OMediaTempMerge	*tmn;
				
				tmn = &vtopoly_list[i2];
			
				for(pui = tmn->poly_used.begin();
				  	pui!= tmn->poly_used.end();
				  	pui++)
				{
					for(p=0;p<(*pui)->get_num_points();p++)
					{
						if ((*pui)->get_point(p)==i2)
						{
							(*pui)->set_point(p,i,false);
						}
					}
				}	
			}
		}
	}
	
	delete_unused_vertices();
	
	unlock();
}

class OMediaVToPolyBlock
{
    public:
    int			index;
    OMedia3DPolygon	*poly;
};

class OMediaVToPoly
{
    public:

    int	newpos;
    vector<OMediaVToPolyBlock>	poly_used;

};

void OMedia3DShape::delete_unused_vertices(void)
{
	long							l,p;
	omt_PolygonList::iterator				pi;
	vector<OMediaVToPoly>					vtopoly_list;
        OMediaVToPoly						empty_vtop;
        OMediaVToPolyBlock					vpb;
	        
	lock(omlf_Read|omlf_Write);

	vtopoly_list.insert(vtopoly_list.begin(),vertices.size(),empty_vtop);

	for(pi=polygons.begin();pi<polygons.end();pi++)
	{
		for(p=0;p<(*pi).get_num_points();p++)
		{
                    vpb.index = p;
                    vpb.poly = &(*pi);
                
                    vtopoly_list[(*pi).get_point(p)].poly_used.push_back(vpb);
		}
	}
        
        omt_VertexList	newVList;
        
        for(l=0;l<(long)vtopoly_list.size();l++)
        {
            if (vtopoly_list[l].poly_used.size()!=0)
            {
                vtopoly_list[l].newpos = newVList.size();
                newVList.push_back(vertices[l]);
            }
        }

        for(l=0;l<(long)vtopoly_list.size();l++)
        {
            vector<OMediaVToPolyBlock>	*poly_used = &vtopoly_list[l].poly_used;
            
            int	newpos = vtopoly_list[l].newpos;
            
            if (newpos!=l)
            {
                for(vector<OMediaVToPolyBlock>::iterator vpi=poly_used->begin();
                    vpi!=poly_used->end();
                    vpi++)
                {                 
                    ((*vpi).poly->get_vertices())[(*vpi).index].vertex_index=newpos;
                }
            }
        }

	
        vertices = newVList;
	
	unlock();
}


void OMedia3DShape::delete_unused_normals(void)
{
	vector<long>						delete_list;
	long								l,p;
	omt_PolygonList::iterator			pi;
	vector<OMediaTempMerge>				ntopoly_list;
	vector<OMediaTempMerge>::iterator 	piu;
	OMediaTempMerge						empty_pou;

	lock(omlf_Read|omlf_Write);

	ntopoly_list.insert(ntopoly_list.begin(),normals.size(),empty_pou);

	for(pi=polygons.begin();pi<polygons.end();pi++)
	{
		for(p=0;p<(*pi).get_num_points();p++)
		{
			ntopoly_list[(*pi).get_vertex_normal(p)].poly_used.push_back(&(*pi));
		}
	}
	
	for(piu=ntopoly_list.begin(),l=0;
		piu!=ntopoly_list.end();
		piu++,l++)
	{
		if ((*piu).poly_used.size()==0) delete_list.push_back(l);
	}

	long delete_offset = 0;
	for(vector<long>::iterator di = delete_list.begin();
		 di!=delete_list.end();
		 di++)
	{
		long dv = ((*di)-delete_offset);

		// erase vertex

		normals.erase(normals.begin()+dv);
		delete_offset++;

		// update polygons

		for(pi = polygons.begin();
			 pi!=polygons.end();
			 pi++)
		{
			for(short pti=0;pti<(*pi).get_num_points();pti++)
			{
				long cvertex = (*pi).get_vertex_normal(pti);
				if (cvertex>dv) (*pi).set_vertex_normal(pti,cvertex-1);
			}
		}
	}
	
	unlock();
}


void OMedia3DShape::unshare_selected_vertices(void)
{
	typedef less<long> compare_long;
	typedef set<long,compare_long> long_set;
	typedef map<long,long,compare_long> long_map;

	lock(omlf_Read|omlf_Write);

	long_set	vert_selected, vert_not_selected;
	long_map	vert_map;
	long_set::iterator is;
	unsigned long			index_base = vertices.size();
	omt_PolygonList::iterator pi;
	short i;

	// Sort selected and not selected vertices

	for(pi = polygons.begin();
		  pi!=polygons.end();
		  pi++)
	{
		OMedia3DPolygon *poly = &(*pi);
		bool				selected = poly->get_selected();

		for(i=0;i<poly->get_num_points();i++)
		{
			if (selected) vert_selected.insert(vert_selected.begin(),(long)poly->get_point(i));
			else vert_not_selected.insert(vert_not_selected.begin(),(long)poly->get_point(i));
		}
	}

	// Create a map of shared vertices

	for(is=vert_not_selected.begin();
				is!=vert_not_selected.end();
				is++)
	{
		if (vert_selected.count(*is))
		{
			vert_map[(*is)] = index_base++;
		}
	}

	if (index_base==vertices.size()) 
	{
		unlock();
		return;
	}

	// Add new vertices

	OMedia3DPoint	newpt;
	vertices.insert(vertices.end(),index_base-vertices.size(),newpt);

	// Duplicate vertices

	for(long_map::iterator im=vert_map.begin();
		  im!=vert_map.end();
		  im++)
	{
		long source,dest;
		source = (*im).first;
		dest = (*im).second;

		vertices[dest] = vertices[source];
	}

	// Update polygons

	for(pi = polygons.begin();
		  pi!=polygons.end();
		  pi++)
	{
		OMedia3DPolygon *poly = &(*pi);

		if (poly->get_selected())
		{
			for(i=0;i<poly->get_num_points();i++)
			{
				long_map::iterator im = vert_map.find(poly->get_point(i));
				if (im!=vert_map.end())
				{
					poly->set_point(i,(*im).second,false);
				}
			}
		}
	}
	
	unlock();
}

void OMedia3DShape::flip_polygons(bool only_selected)
{
	lock(omlf_Read|omlf_Write);

	for(omt_PolygonList::iterator pi = polygons.begin();
		  pi!=polygons.end();
		  pi++)
	{
		OMedia3DPolygon *poly = &(*pi);

		if (!only_selected || poly->get_selected())
		{
			vector<long>	plist,nlist,clist;
			unsigned long	p,np = poly->get_num_points();
			
			for(p=0; p<np; p++)
			{
				plist.push_back(poly->get_point(p));
				nlist.push_back(poly->get_vertex_normal(p));
				clist.push_back(poly->get_vertex_color(p));
			}
			
			for(p=0;p<np;p++)
			{
				poly->set_point(p,plist[(np-1)-p],false);
				poly->set_vertex_normal(p,nlist[(np-1)-p]);
				poly->set_vertex_color(p,clist[(np-1)-p]);
			}		
		}
	}
	
	compute_polygon_normals(only_selected);
	
	unlock();
}

void OMedia3DShape::triangles_to_quads(float normal_tolerance, bool only_selected)
{
	omt_PolygonList::iterator	pa,pb,paend,pbend;
	vector<long>				pdelete_list;
	OMedia3DVector				na,nb;
	float						ab_angle;
	
	lock(omlf_Read|omlf_Write);

	if (polygons.size()<2) 
	{
		unlock();
		return;
	}

	paend = polygons.end();	paend--;
	pbend = polygons.end();

	for(pa=polygons.begin();pa!=paend;pa++)
	{
		if (only_selected && (!(*pa).get_selected())) continue;
		if ((*pa).get_num_points()!=3) continue;

		na = (*pa).get_normal();
	
		pb = pa;
		pb++;
		for(;pb!=pbend;pb++)
		{
			if (only_selected && (!(*pb).get_selected())) continue;
			if ((*pb).get_num_points()!=3) continue;
	
			nb = (*pb).get_normal();		
			ab_angle = na.dot_product(nb);
			if (fabs(1.0-ab_angle)<=normal_tolerance && ab_angle>0.0)
			{
				OMedia3DPolygon	*a,*b;
				short nshared;
				short b2a_shared[3];
				
				a = &(*pa);
				b = &(*pb);	
				nshared = 0;

				b2a_shared[0] = b2a_shared[1] =  b2a_shared[2] = -1;

				for(short ia=0;ia<3;ia++)
				{
					for(short ib=0;ib<3;ib++)
					{
						if (a->get_point(ia)==b->get_point(ib)) 
						{
							nshared++;
							b2a_shared[ib] = ia;
						}
					}
				}
	
				if (nshared==2)
				{
					// There is a quad to do here. I keep a and delete b
					
					short ia_insert,i,ib_insert;
					
					for(i=0;i<3;i++)
					{
						if (b2a_shared[i]==-1)
						{
							short ti = i;

							ti++;
							if (ti>2) ti=0;
							
							ia_insert = b2a_shared[ti];
							ib_insert = i;
							break;
						}
					}
					
					a->set_quad();
					for(i=2;;i--)
					{
						a->set_point(i+1,a->get_point(i));
						a->set_vertex_normal(i+1,a->get_vertex_normal(i));
						if (ia_insert==i)
						{
							a->set_point(i,b->get_point(ib_insert));
							a->set_vertex_normal(i,b->get_vertex_normal(ib_insert));
							a->set_text_coord_u(i, b->get_text_coord_u(ib_insert));
							a->set_text_coord_v(i, b->get_text_coord_v(ib_insert));
							break;
						}			
					}
					
					b->set_quad();	// Mark it as a quad, so it will not be checked anymore
					pdelete_list.push_back(pb-polygons.begin());
					break;
				}							
			}			
		}	
	}
	
	// Delete unused triangles
	
	while(pdelete_list.size())
	{
		long	index;
		index = *(pdelete_list.begin());
		polygons.erase(polygons.begin() + index);
		pdelete_list.erase(pdelete_list.begin());

		for(vector<long>::iterator pi = pdelete_list.begin();
			pi!=pdelete_list.end();
			pi++)
		{
			if ((*pi)>index) (*pi)--;	// Scale indexes	
		}
	}
	
	unlock();
}

