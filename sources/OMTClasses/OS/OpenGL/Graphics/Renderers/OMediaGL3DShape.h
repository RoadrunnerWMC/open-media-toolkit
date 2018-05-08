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
#ifndef OMEDIA_GL3DShape_H
#define OMEDIA_GL3DShape_H

#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_OPENGL

#include "OMedia3DShape.h"
#include "OMediaGLRenderer.h"

#include <list>
#include <vector>

const unsigned long	omc_MaxGLBufSize		= 0xFFFFUL;


class OMediaGLVBTriangle
{
public:

	unsigned short		indexes[3];
	bool				compiled;
};

typedef list<OMediaGLVBTriangle>	omt_GLVBTriangleList;


class OMediaGLBVertex
{
	public:
	
	GLfloat						x,y,z;		// position
	GLfloat						nx,ny,nz;	// normal
	GLfloat						r,g,b,a;	// color
	OMediaExtraTexturePassUV	uv[omc_GLMaxTexturePass];	// texture

	unsigned short				nuv_passes;
	unsigned short				index;

	vector<OMediaGLVBTriangle*>		*triangles;


	void send_vertex(void);

	inline void clear_uv(void)
	{
		uv[3].u = 0.0f;		uv[3].v = 0.0f;
		uv[2].u = 0.0f;		uv[2].v = 0.0f;
		uv[1].u = 0.0f;		uv[1].v = 0.0f;
		uv[0].u = 0.0f;		uv[0].v = 0.0f;
		nuv_passes = 0;
	}

	inline bool operator <(const OMediaGLBVertex	&vx) const
	{
		if (x==vx.x)
		{
			if (y==vx.y)
			{
				if (z==vx.z)
				{
					if (nx==vx.nx)
					{
						if (ny==vx.ny)
						{
							if (nz==vx.nz)
							{
								if (r==vx.r)
								{
									if (g==vx.g)
									{
										if (b==vx.b)
										{
											if (a==vx.a)
											{
												if (nuv_passes==vx.nuv_passes)
												{
													for (short i=0;i<(short)omc_GLMaxTexturePass; i++)
													{
														if (uv[i].u==vx.uv[i].u)
														{
															if (uv[i].v==vx.uv[i].v) continue;
															else return uv[i].v<vx.uv[i].v;
														}
														else return uv[i].u<vx.uv[i].u;
													}
													return false;
												}
												else return nuv_passes<vx.nuv_passes;
											}
											else return a<vx.a;
										}
										else return b<vx.b;
									}
									else return g<vx.g;
								}
								else return r<vx.r;
							}
							else return nz<vx.nz;
						}
						else return ny<vx.ny;
					}
					else return nx<vx.nx;
				}
				else return z<vx.z;
			}
			else return y<vx.y;
		}
		else return x<vx.x;
	}
};


class OMediaGLBPoly
{
public:

	OMedia3DPolygon	*poly;

	inline bool operator < (const OMediaGLBPoly &x) const
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

typedef vector<OMediaGLBPoly>	omt_GLBPolyList;




class OMediaGLVBuffer
{
public:

	OMedia3DMaterial					*material;
	omt_ExtraTexturePassList			*pass_list;
	bool								two_sided;

	GLuint								display_list_id;
};

typedef list<OMediaGLVBuffer>		omt_GLVBufferList;



class OMediaGL3DShape : public OMediaEngineImpSlave
{
	public:
	
	omtshared OMediaGL3DShape(OMediaGLRenderTarget *target, 
							 OMedia3DShape *master);

	omtshared virtual ~OMediaGL3DShape();

	omtshared virtual void purge(void);

	omtshared virtual void master_modified(void);

	omtshared void prepare(short render_pass,OMediaGLRenderPort	*port);

	omtshared void generate_vertex_buffer(	omt_GLBPolyList::iterator	poly_i,
											omt_GLBPolyList::iterator	poly_end,
											short render_pass);


	omtshared void generate_display_list(OMediaGLBVertex		**vertex_array,
											long					vertex_array_length,
											omt_GLVBTriangleList	&triangle_list, 
											OMediaGLVBuffer			*cur_buf);


	omt_GLVBufferList		gl_buffers[2];

	bool					dirty,second_pass_found;

	OMediaGLRenderTarget	*gltarget;
	OMediaGLRenderPort		*glworking_port;

	OMedia3DShape			*shape;
};


#endif
#endif

