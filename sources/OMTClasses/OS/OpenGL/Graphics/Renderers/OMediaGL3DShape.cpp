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

#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_OPENGL

#include "OMediaTypes.h"
#include "OMediaGL3DShape.h"

#ifdef omd_WINDOWS
#include "glext.h"
#ifdef GL_NV_vertex_array_range
#undef GL_NV_vertex_array_range
#endif
#include "glh_genext.h"
#endif

#ifndef GL_ARB_multitexture
#ifdef GL_VERSION_1_3
#define glMultiTexCoord2fARB glMultiTexCoord2f
#define GL_TEXTURE0_ARB GL_TEXTURE0
#endif
#endif

#include <map>
#include <set>
#include <algorithm>


void OMediaGLBVertex::send_vertex(void)
{
	glColor4f(r,g,b,a);
	
	if (nuv_passes<=1)
	{
		glTexCoord2f(uv[0].u, uv[0].v);
	}
	else
	{
		for(unsigned short i = 0; i<nuv_passes;i++)
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i,uv[i].u, uv[i].v);
	}

	glNormal3f(nx,ny,nz);
	glVertex3f(x,y,z);
}



OMediaGL3DShape::OMediaGL3DShape(OMediaGLRenderTarget *target, 
								 OMedia3DShape *master):
				OMediaEngineImpSlave(target,master)	
{
	gltarget = target;
	dirty = true;
	second_pass_found = false;
	shape = master;
}

OMediaGL3DShape::~OMediaGL3DShape()
{
	purge();
}

void OMediaGL3DShape::purge(void)
{
	for(short i=0;i<2;i++)
	{
		for(omt_GLVBufferList::iterator gvi = gl_buffers[i].begin();
			gvi!=gl_buffers[i].end();
			gvi++)
		{
			glDeleteLists((*gvi).display_list_id,1);
		}	

		gl_buffers[i].erase(gl_buffers[i].begin(),gl_buffers[i].end());
	}

	dirty = true;
}

void OMediaGL3DShape::master_modified(void)
{
	dirty = true;
}

void OMediaGL3DShape::prepare(short render_pass,OMediaGLRenderPort	*port)
{
	if (!dirty) return;
	purge();

	glworking_port = port;

	omt_GLBPolyList		sorted_plist;
	OMediaGLBPoly		newpoly;

	for(omt_PolygonList::iterator pi = shape->get_polygons()->begin();
		pi != shape->get_polygons()->end();
		pi++)
	{
		newpoly.poly = &(*pi);
		sorted_plist.push_back(newpoly);
	}

	sort(sorted_plist.begin(),sorted_plist.end());

	if (sorted_plist.size()) generate_vertex_buffer(sorted_plist.begin(),sorted_plist.end(),render_pass);

	dirty = false;
}

void OMediaGL3DShape::generate_vertex_buffer(omt_GLBPolyList::iterator	poly_i,
											omt_GLBPolyList::iterator	poly_end,
											short render_pass)
{
	typedef	less<OMediaGLBVertex>	less_GLBVertex;
	typedef vector<OMediaGLVBTriangle*>	omt_GLVBTPtrVec;

	set<OMediaGLBVertex,less_GLBVertex>					v_set;
	set<OMediaGLBVertex,less_GLBVertex>::iterator		vs_i;
	list<omt_GLVBTPtrVec>								triangle_links;	

	OMediaGLVBuffer										new_buf,*cur_buf;
	OMedia3DMaterial									*mat;
	OMediaGLBVertex										gldv;
	long												v_index;
	short												max_text_units = gltarget->max_texture_units;
	omt_ExtraTexturePassList							*pass_list;
	omt_GLVBTriangleList								triangle_list;
	bool												force_flat,cur_two_sided;

	omt_VertexList *vlist  = shape->get_vertices(); 
	omt_NormalList *nlist = shape->get_normals();
	omt_ColorList	*clist = shape->get_colors();

	for(;;)
	{
		v_index = 0;
		mat = NULL;
		for(;poly_i!=poly_end;poly_i++)
		{
			if ((*poly_i).poly->get_flags()&om3pf_Hide) continue;

			// Look for material and extra passes
			mat = (*poly_i).poly->get_material();
			if (!mat) continue;
			if ((mat->get_flags()&ommatf_SecondPass) && render_pass==0) 
			{
				second_pass_found = true;
				continue;
			}
			if (!(mat->get_flags()&ommatf_SecondPass) && render_pass==1) continue;		
			break; 
		}

		if (!mat || poly_i==poly_end) return;
		
		force_flat = (mat->get_light_mode()==ommlmc_Light && mat->get_shade_mode()==omshademc_Flat);

		gl_buffers[render_pass].push_back(new_buf);
		cur_buf = &(gl_buffers[render_pass].back());

		OMedia3DVector		*n,*nbase = (nlist->size())?(&(*(nlist->begin()))):NULL;
		OMedia3DPoint		*p,*pbase = &(*(vlist->begin()));

		cur_buf->material = mat;
		cur_buf->pass_list = ((*poly_i).poly->get_extra_texture_pass_index()!=-1)?
			&(*shape->get_extra_texture_pass_sets())[(*poly_i).poly->get_extra_texture_pass_index()].pass_list:NULL;
		cur_buf->two_sided = ((*poly_i).poly->get_flags()&om3pf_TwoSided)!=0;

		for(;;poly_i++)
		{
			if (poly_i!=poly_end)
			{
				mat = (*poly_i).poly->get_material();
				if (!mat) continue;
				if ((*poly_i).poly->get_flags()&om3pf_Hide) continue;

				pass_list = ((*poly_i).poly->get_extra_texture_pass_index()!=-1)?
				&(*shape->get_extra_texture_pass_sets())[(*poly_i).poly->get_extra_texture_pass_index()].pass_list:NULL;

				cur_two_sided = ((*poly_i).poly->get_flags()&om3pf_TwoSided)!=0;	
			}

			if (poly_i==poly_end || 
				((*poly_i).poly->get_vertices().size()+v_set.size()>omc_MaxGLBufSize) ||
				(mat!=cur_buf->material) || 
				(pass_list!=cur_buf->pass_list) || 
				(cur_two_sided!=cur_buf->two_sided) ||
				((triangle_list.size() +  
				((*poly_i).poly->get_vertices().size()-2))*3)>omc_MaxGLBufSize )
			{
				OMediaGLBVertex		**vertex_array;

				// Generate linear vertex_array

				vertex_array = new OMediaGLBVertex*[v_set.size()];

				for(vs_i = v_set.begin();
					vs_i!= v_set.end();
					vs_i++)
				{
					vertex_array[(*vs_i).index] = (OMediaGLBVertex*)&(*vs_i);
				}

				// Generate new GL display list

				generate_display_list(vertex_array,v_set.size(),triangle_list, cur_buf);

				delete [] vertex_array;

				// Continue if required
				if (poly_i!=poly_end)
				{
					triangle_links.erase(triangle_links.begin(),triangle_links.end());
					triangle_list.erase(triangle_list.begin(),triangle_list.end());
					v_set.erase(v_set.begin(),v_set.end());
					break;
				}
				
				// Exit
				return;
			}

			long				vtab[3],nverts;

			nverts = (*poly_i).poly->get_vertices().size()-2;
			if (nverts<0) nverts = 0;

			// Add new triangle

			vtab[0] = 0;	
			vtab[1] = 1; 
			vtab[2] = 2;

			while(nverts--)
			{
				OMediaGLVBTriangle		newtriangle,*cur_triangle;
				triangle_list.push_back(newtriangle);
				cur_triangle = &triangle_list.back();
				cur_triangle->compiled = false;

				for(short ti=0;ti<3;ti++)
				{
					OMedia3DPolygonVertex	*i;

					i =  &((*poly_i).poly->get_vertices())[vtab[ti]];
					p = pbase + (*i).vertex_index;

					// New vertex

					// Coordinate

					gldv.x = p->x;
					gldv.y = p->y;
					gldv.z = p->z;
					
					// uv

					gldv.clear_uv();
					gldv.uv[0].u = (*i).u;
					gldv.uv[0].v = (*i).v;
					gldv.nuv_passes = 1;

					if (max_text_units>1)
					{
						short	uvi,maxuvi = (*i).extra_passes_uv.size();
						if (maxuvi>(short)(omc_GLMaxTexturePass-1)) maxuvi = (omc_GLMaxTexturePass-1);

						for(uvi=0; uvi<maxuvi; uvi++)
							gldv.uv[uvi+1] = (*i).extra_passes_uv[uvi];

						gldv.nuv_passes += (*i).extra_passes_uv.size();
						if (gldv.nuv_passes>omc_GLMaxTexturePass) gldv.nuv_passes = omc_GLMaxTexturePass;
					}

					// Color

					switch(mat->get_light_mode())
					{
						case ommlmc_Color:
						gldv.a  = mat->get_diffuse_ref().alpha;
						gldv.r  = mat->get_diffuse_ref().red;
						gldv.g  = mat->get_diffuse_ref().green;
						gldv.b  = mat->get_diffuse_ref().blue;
						gldv.nx = 0;
						gldv.ny = 0;
						gldv.nz = 1.0f;
						break;

						case ommlmc_VertexColor:
						{
							OMediaFARGBColor	*argb = &(*(clist->begin() + (*i).color_index));	
							gldv.a  = argb->alpha;
							gldv.r  = argb->red;
							gldv.g  = argb->green;
							gldv.b  = argb->blue;
							gldv.nx = 0;
							gldv.ny = 0;
							gldv.nz = 1.0f;
						}
						break;

						case ommlmc_Light:
						gldv.a  = mat->get_diffuse_ref().alpha;
						gldv.r  = mat->get_diffuse_ref().red;
						gldv.g  = mat->get_diffuse_ref().green;
						gldv.b  = mat->get_diffuse_ref().blue;
						if (nbase && mat->get_shade_mode()==omshademc_Gouraud && !force_flat)
						{
							n = nbase + (*i).normal_index;
							gldv.nx = n->x;
							gldv.ny = n->y;
							gldv.nz = n->z;
						}
						else
						{
							gldv.nx = (*poly_i).poly->get_normal().x;
							gldv.ny = (*poly_i).poly->get_normal().y;
							gldv.nz = (*poly_i).poly->get_normal().z;
						}
						break;
					}

					// Search for an identical vertex

					vs_i = v_set.find(gldv);
					if (vs_i!=v_set.end())
					{
						cur_triangle->indexes[ti] = (*vs_i).index;
						(*vs_i).triangles->push_back(cur_triangle);
					}
					else
					{
						vector<OMediaGLVBTriangle*>		empty_tl_list;
						triangle_links.push_back(empty_tl_list);

						gldv.index = v_index;
						gldv.triangles = &(triangle_links.back());
						gldv.triangles->push_back(cur_triangle);
						v_set.insert(v_set.begin(),gldv);
						cur_triangle->indexes[ti] = v_index ;
						v_index++;
					}
				}

				vtab[1]++; 
				vtab[2]++;
			}
		}
	}
}



void OMediaGL3DShape::generate_display_list(OMediaGLBVertex		**vertex_array,
											long					vertex_array_length,
											omt_GLVBTriangleList	&triangle_list, 
											OMediaGLVBuffer			*cur_buf)
{
	bool					force_flat,even;
	omt_FillMode			fillmode;
	short					v,v2,vn;
	OMediaGLBVertex			*tri[3];
	omt_GLVBTriangleList::iterator ti;

#define INC_V(v)	{v++;	if (v>2) v = 0;}
#define DEC_V(v)	{v--;	if (v<0) v = 2;}

	// Material
	glworking_port->draw_shape_prepare_mat(cur_buf->material, force_flat, fillmode);

	// Generate list

	cur_buf->display_list_id = glGenLists(1);
	glNewList(cur_buf->display_list_id,GL_COMPILE);

	if (fillmode==omfillmc_Point)
	{
		glBegin(GL_POINTS);
		while(vertex_array_length--)
		{
			(*vertex_array)->send_vertex();
			vertex_array++;
		}

		glEnd();
		glEndList();
		return;
	}
	else if (fillmode==omfillmc_Line)
	{
		for(ti = triangle_list.begin();
			ti!=triangle_list.end();
			ti++)
		{
			glBegin(GL_LINE_LOOP);
			vertex_array[(*ti).indexes[0]]->send_vertex();
			vertex_array[(*ti).indexes[1]]->send_vertex();
			vertex_array[(*ti).indexes[2]]->send_vertex();
			glEnd();
		}
		glEndList();
		return;
	}


	// Generate triangle strips

	for(ti = triangle_list.begin();
		ti!=triangle_list.end();
		ti++)
	{
		if ((*ti).compiled) continue;
		(*ti).compiled = true;

		// Seach best starting point

		tri[0] = vertex_array[(*ti).indexes[0]];
		tri[1] = vertex_array[(*ti).indexes[1]];
		tri[2] = vertex_array[(*ti).indexes[2]];

		if (tri[0]->triangles->size()<tri[1]->triangles->size())
		{
			if (tri[0]->triangles->size()<tri[2]->triangles->size()) v = 0;
			else v = 2;
		}
		else
		{
			if (tri[1]->triangles->size()<tri[2]->triangles->size()) v = 1;
			else v = 2;
		}

		glBegin(GL_TRIANGLE_STRIP);

		// Send first triangle

		tri[v]->send_vertex();	INC_V(v);
		tri[v]->send_vertex();	INC_V(v);
		tri[v]->send_vertex();

		

		// Seach next triangle if any

		v2 = v;
		DEC_V(v);
		even = false;

		for(;;)
		{
			OMediaGLVBTriangle	*co_tri;
			bool				co_found;
			
			co_found = false;

			for(vector<OMediaGLVBTriangle*>::iterator ti = tri[v]->triangles->begin();
				ti!= tri[v]->triangles->end();
				ti++)
			{
				co_tri = (*ti);
				if (co_tri->compiled) continue;

				for(vn=0;vn<3;vn++)
				{
					if (co_tri->indexes[vn]==tri[v]->index)
					{
						short	t = vn;

						if (even) INC_V(vn)
						else DEC_V(vn);

						if (co_tri->indexes[vn]==tri[v2]->index)
						{
							co_found = true;
							break;
						}

						vn = t;
					}
				}

				if (co_found) break;
			}

			if (co_found)
			{
				co_tri->compiled = true;
				tri[0] = vertex_array[co_tri->indexes[0]];
				tri[1] = vertex_array[co_tri->indexes[1]];
				tri[2] = vertex_array[co_tri->indexes[2]];

				v = vn;
				v2 = vn;	
				if (even) INC_V(v2)
				else DEC_V(v2);

				tri[v2]->send_vertex();
				even = !even;
			}
			else break;
		}

		glEnd();
	}

	glEndList();
}


#endif

