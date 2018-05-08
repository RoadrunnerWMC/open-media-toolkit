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
#ifndef OMEDIA_DX3DShape_H
#define OMEDIA_DX3DShape_H

#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_DIRECT3D

#include "OMedia3DShape.h"
#include "OMediaDXRenderer.h"

#include <vector>
#include <list>

const unsigned long	omc_MaxD3DBufSize		= 0xFFFFUL;

class OMediaDX3DBVertex : public OMediaDXVertex
{
public:
	unsigned short		index;
};

class OMediaDX3DBPoly
{
public:

	OMedia3DPolygon	*poly;

	inline bool operator < (const OMediaDX3DBPoly &x) const
	{
		if (poly->get_material()==x.poly->get_material())
		{
			if (poly->get_extra_texture_pass_index()==x.poly->get_extra_texture_pass_index())
			{
				omt_3DPolygonFlags	f1,f2;
				f1 = (poly->get_flags()&om3pf_TwoSided);
				f2 = (x.poly->get_flags()&om3pf_TwoSided);
				return f1<f2;

			}
			else return (poly->get_extra_texture_pass_index()<x.poly->get_extra_texture_pass_index());
		}
		else return ((long)poly->get_material()<(long)x.poly->get_material());
	}
};

typedef vector<OMediaDX3DBPoly>	omt_DX3DBPolyList;

class OMediaD3DVBuffer
{
public:

	OMedia3DMaterial					*material;
	omt_ExtraTexturePassList			*pass_list;
	bool								two_sided;

	vector<unsigned short>				triangles;

	LPDIRECT3DVERTEXBUFFER7				d3d_buffer;
	long								d3d_buffer_size;
};

typedef list<OMediaD3DVBuffer>		omt_D3DVBufferList;


class OMediaDX3DShape : public OMediaEngineImpSlave
{
	public:
	
	omtshared OMediaDX3DShape(OMediaDXRenderTarget *target, 
							 OMedia3DShape *master,
							 LPDIRECT3D7	d3d_i,
							 LPDIRECT3DDEVICE7	d3d_d);

	omtshared virtual ~OMediaDX3DShape();

	omtshared virtual void purge(void);


	omtshared virtual void master_modified(void);

	omtshared void prepare(short render_pass);

	omtshared void generate_vertex_buffer(	omt_DX3DBPolyList::iterator	poly_i,
											omt_DX3DBPolyList::iterator	poly_end,
											short render_pass);


	bool								dirty,second_pass_found;

	OMediaDXRenderTarget				*dxtarget;
	OMedia3DShape						*shape;

	LPDIRECT3D7							d3d_base;
	LPDIRECT3DDEVICE7					d3d_device;

	omt_D3DVBufferList					d3d_buffers[2];
};


#endif
#endif

