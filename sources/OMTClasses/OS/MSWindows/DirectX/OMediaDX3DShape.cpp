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

#ifdef omd_ENABLE_DIRECT3D

#include "OMediaDX3DShape.h"
#include <map>
#include <set>
#include <algorithm>

OMediaDX3DShape::OMediaDX3DShape(OMediaDXRenderTarget *target, 
								 OMedia3DShape *master,
								 LPDIRECT3D7	d3d_i,
								 LPDIRECT3DDEVICE7	d3d_d):
				OMediaEngineImpSlave(target,master)	
{
	dxtarget = target;
	dirty = true;
	shape = master;
	d3d_base = d3d_i;
	d3d_device = d3d_d;
	second_pass_found = false;
}

OMediaDX3DShape::~OMediaDX3DShape()
{
	purge();
}

void OMediaDX3DShape::purge(void)
{
	for(short i=0;i<2;i++)
	{
		for(omt_D3DVBufferList::iterator dvi = d3d_buffers[i].begin();
			dvi!=d3d_buffers[i].end();
			dvi++)
		{
			(*dvi).d3d_buffer->Release();
		}	

		d3d_buffers[i].erase(d3d_buffers[i].begin(),d3d_buffers[i].end());
	}
	dirty = true;
}

void OMediaDX3DShape::master_modified(void)
{
	dirty = true;
}

void OMediaDX3DShape::prepare(short render_pass)
{
	if (!dirty) return;
	purge();

	omt_DX3DBPolyList		sorted_plist;
	OMediaDX3DBPoly			newpoly;

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

void OMediaDX3DShape::generate_vertex_buffer(omt_DX3DBPolyList::iterator	poly_i,
											omt_DX3DBPolyList::iterator	poly_end,
											short render_pass)
{
	typedef	less<OMediaDX3DBVertex>	less_DX3DBVertex;

	set<OMediaDX3DBVertex,less_DX3DBVertex>				v_set;
	set<OMediaDX3DBVertex,less_DX3DBVertex>::iterator	vs_i;
	OMediaD3DVBuffer									new_buf,*cur_buf;
	OMedia3DMaterial									*mat;
	OMediaDX3DBVertex									d3dv;
	long												v_index = 0;
	short												max_text_units = dxtarget->max_texture_units;
	D3DVERTEXBUFFERDESC									d3d_vbdesc;
	LPVOID												vptr;
	omt_ExtraTexturePassList							*pass_list;
	bool												cur_two_sided;

	omt_PolygonList *polygons = shape->get_polygons();
	omt_VertexList *vlist  = shape->get_vertices(); 
	omt_NormalList *nlist = shape->get_normals();
	omt_ColorList	*clist = shape->get_colors();

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

	if (!mat) return;

	d3d_buffers[render_pass].push_back(new_buf);
	cur_buf = &(d3d_buffers[render_pass].back());

	OMedia3DVector		*n,*nbase = (nlist->size())?nlist->begin():NULL;
	OMedia3DPoint		*p,*pbase = vlist->begin();

	cur_buf->material = mat;
	cur_buf->pass_list = ((*poly_i).poly->get_extra_texture_pass_index()!=-1)?
		&(*shape->get_extra_texture_pass_sets())[(*poly_i).poly->get_extra_texture_pass_index()].pass_list:NULL;
	cur_buf->two_sided = ((*poly_i).poly->get_flags()&om3pf_TwoSided)!=0;

	for(;;poly_i++)
	{
		if (poly_i!=poly_end)
		{
			if ((*poly_i).poly->get_flags()&om3pf_Hide) continue;
			mat = (*poly_i).poly->get_material();
			if (!mat) continue;

			pass_list = ((*poly_i).poly->get_extra_texture_pass_index()!=-1)?
			&(*shape->get_extra_texture_pass_sets())[(*poly_i).poly->get_extra_texture_pass_index()].pass_list:NULL;

			cur_two_sided = ((*poly_i).poly->get_flags()&om3pf_TwoSided)!=0;	
		}

		if (poly_i==poly_end || ((*poly_i).poly->get_vertices().size()+v_set.size()>omc_MaxD3DBufSize) ||
			(mat!=cur_buf->material) || (pass_list!=cur_buf->pass_list) || (cur_two_sided!=cur_buf->two_sided) ||
			(cur_buf->triangles.size() +  (((*poly_i).poly->get_vertices().size()-2)*3))>omc_MaxD3DBufSize )
		{
			// Generate new D3D buffer

			d3d_vbdesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
			d3d_vbdesc.dwCaps = D3DVBCAPS_WRITEONLY ;
			d3d_vbdesc.dwFVF = D3DFVF_XYZ  | D3DFVF_NORMAL | D3DFVF_DIFFUSE  | omd_FVF_TEX;
			d3d_vbdesc.dwNumVertices = v_set.size();

			cur_buf->d3d_buffer_size = d3d_vbdesc.dwNumVertices;

			d3d_base->CreateVertexBuffer(&d3d_vbdesc,&cur_buf->d3d_buffer,0);
			cur_buf->d3d_buffer->Lock(DDLOCK_SURFACEMEMORYPTR|DDLOCK_WAIT|DDLOCK_WRITEONLY|DDLOCK_DISCARDCONTENTS,
										&vptr,NULL);

			OMediaDXVertex		*dx_vb = (OMediaDXVertex*)vptr;

			for(vs_i = v_set.begin();
				vs_i!= v_set.end();
				vs_i++)
			{
				dx_vb[(*vs_i).index] = (*vs_i);
			}

			cur_buf->d3d_buffer->Unlock();
			cur_buf->d3d_buffer->Optimize(d3d_device,0);

			// Continue if required
			if (poly_i!=poly_end)
			{
				v_set.erase(v_set.begin(),v_set.end());
				generate_vertex_buffer(poly_i,poly_end,render_pass);
			}
			break;
		}

		unsigned long		mat_diffuse = mat->get_diffuse_ref().get_argb();
		long				vtab[3],nverts;

		nverts = (*poly_i).poly->get_vertices().size()-2;
		if (nverts<0) nverts = 0;

		// Add new triangle

		vtab[0] = 0;	
		vtab[1] = 1; 
		vtab[2] = 2;

		while(nverts--)
		{
			for(short ti=0;ti<3;ti++)
			{
				OMedia3DPolygonVertex	*i;

				i =  &((*poly_i).poly->get_vertices())[vtab[ti]];
				p = pbase + (*i).vertex_index;

				// New vertex

				// Coordinate

				d3dv.x = p->x;
				d3dv.y = p->y;
				d3dv.z = p->z;
				
				// uv

				d3dv.clear_uv();
				d3dv.uv[0].u = (*i).u;
				d3dv.uv[0].v = (*i).v;

				if (max_text_units>1)
				{
					short	uvi,maxuvi = (*i).extra_passes_uv.size();
					if (maxuvi>(omc_DXMaxTexturePass-1)) maxuvi = (omc_DXMaxTexturePass-1);

					for(uvi=0; uvi<maxuvi; uvi++)
						d3dv.uv[uvi+1] = (*i).extra_passes_uv[uvi];
				}

				// Color

				switch(mat->get_light_mode())
				{
					case ommlmc_Color:
					d3dv.diffuse  = mat_diffuse;
					d3dv.nx = 0;
					d3dv.ny = 0;
					d3dv.nz = 1.0f;
					break;

					case ommlmc_VertexColor:
					{
						OMediaFARGBColor	*argb = clist->begin() + (*i).color_index;	
						d3dv.diffuse  = argb->get_argb();
						d3dv.nx = 0;
						d3dv.ny = 0;
						d3dv.nz = 1.0f;
					}
					break;

					case ommlmc_Light:
					d3dv.diffuse  = mat_diffuse;
					if (nbase && mat->get_shade_mode()==omshademc_Gouraud)
					{
						n = nbase + (*i).normal_index;
						d3dv.nx = n->x;
						d3dv.ny = n->y;
						d3dv.nz = n->z;
					}
					else
					{
						d3dv.nx = (*poly_i).poly->get_normal().x;
						d3dv.ny = (*poly_i).poly->get_normal().y;
						d3dv.nz = (*poly_i).poly->get_normal().z;
					}
					break;
				}

				// Search for an identical vertex

				vs_i = v_set.find(d3dv);
				if (vs_i!=v_set.end())
				{
					cur_buf->triangles.push_back( (*vs_i).index);
				}
				else
				{
					d3dv.index = v_index;
					v_set.insert(v_set.begin(),d3dv);
					cur_buf->triangles.push_back( v_index );
					v_index++;
				}
			}

			vtab[1]++; 
			vtab[2]++;
		}		
	}
}


#endif

