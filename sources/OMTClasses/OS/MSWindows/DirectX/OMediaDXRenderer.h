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
#ifndef OMEDIA_DXRenderer_H
#define OMEDIA_DXRenderer_H

#include "OMediaSysDefs.h"
#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_DIRECT3D
#include "OMediaRenderer.h"
#include "OMediaRendererInterface.h"
#include "OMediaRetarget.h"

#include <d3d.h>

class OMediaVideoMode;

#ifndef pi
	#ifndef PI
	#define omd_pi 3.1415926535f
	#else
	#define omd_pi PI
	#endif
#else
#define omd_pi pi
#endif

class OMediaDXLight
{
	public:
	
	OMediaDXLight()
	{
		dlight.dltType = D3DLIGHT_DIRECTIONAL;
		dlight.dcvDiffuse.r = dlight.dcvDiffuse.g = dlight.dcvDiffuse.b = dlight.dcvDiffuse.a = 1.0f;
		dlight.dcvSpecular.r = dlight.dcvSpecular.g = dlight.dcvSpecular.b = dlight.dcvSpecular.a = 1.0f;
		dlight.dcvAmbient.r = dlight.dcvAmbient.g = dlight.dcvAmbient.b = 0.0f; dlight.dcvAmbient.a = 1.0f;
		dlight.dvRange = D3DLIGHT_RANGE_MAX;
		dlight.dvFalloff = 1.0f;
		dlight.dvAttenuation0 = 1.0f;
		dlight.dvAttenuation1 = 0.0f;
		dlight.dvAttenuation2 = 0.0f;
		dlight.dvTheta = 0.0f;
		dlight.dvPhi = omd_pi/2.0f;
		dlight.dvAttenuation2 = 0.0f;
		dirty = true;
	}

	D3DLIGHT7			dlight;
	bool				dirty;
};


#define omd_FVF_TEX		D3DFVF_TEX3			
const unsigned long omc_DXMaxTexturePass = 3;	
			// Warn: Update OMediaDXVertex::operator < and
			// clear_uv when changing
			// pass number.

class OMediaDXVertex
{
	public:
	
	float						x,y,z;		// position
	float						nx,ny,nz;	// normal
	DWORD						diffuse;	// color
	OMediaExtraTexturePassUV	uv[omc_DXMaxTexturePass];	// texture

	inline void clear_uv(void)
	{
		uv[2].u = 0.0f;		uv[2].v = 0.0f;
		uv[1].u = 0.0f;		uv[1].v = 0.0f;
		uv[0].u = 0.0f;		uv[0].v = 0.0f;
	}

	inline bool operator <(const OMediaDXVertex	&vx) const
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
								if (diffuse==vx.diffuse)
								{
									for (short i=0;i<omc_DXMaxTexturePass; i++)
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
								else return diffuse<vx.diffuse;
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

class OMediaDXPickVert
{
public:
	float	xyzw[4];

};

enum omt_DXPickClippingPlane
{
	omdxclippc_Left,
	omdxclippc_Right,
	omdxclippc_Top,
	omdxclippc_Bottom,
	omdxclippc_Near,
	omdxclippc_Far
};


//----------------------------------
// Render port

class OMediaDXRenderPort : 	public OMediaRenderPort,
							public OMediaRendererInterface
{
	public:
	
	omtshared OMediaDXRenderPort(OMediaRenderTarget *target);
	omtshared virtual ~OMediaDXRenderPort();

	omtshared virtual void capture_frame(OMediaCanvas &canv);

	omtshared virtual void render(void);

	// * Begin/End
	
	omtshared virtual bool begin_render(void);
	omtshared virtual void end_render(void);

	// * Renderer interface

	omtshared virtual omt_RendererRequirementFlags get_requirement_flags(void);

	// * View bounds
	
	omtshared virtual void get_view_bounds(OMediaRect &r);

	// * Buffer

	omtshared virtual void clear_all_buffers(OMediaFARGBColor &rgb);	// Color and ZBuffer

	omtshared virtual void clear_colorbuffer(OMediaFARGBColor &rgb, OMediaRect *area =NULL);

	omtshared virtual void clear_zbuffer(OMediaRect *area =NULL);
	omtshared virtual void set_zbuffer_write(omt_ZBufferWrite zb);
	omtshared virtual void set_zbuffer_test(omt_ZBufferTest zb);
	omtshared virtual void set_zbuffer_func(omt_ZBufferFunc zb);
	
	
	// * Matrix
	
	omtshared virtual void set_model_view(OMediaMatrix_4x4 &m);
	omtshared virtual void set_projection(OMediaMatrix_4x4 &m);

	// * Light

	omtshared virtual void enable_lighting(void);
	omtshared virtual void disable_lighting(void);

	omtshared virtual long get_max_lights(void);


	omtshared virtual void start_light_edit(void);
	omtshared virtual void end_light_edit(void);

	omtshared virtual void enable_light(long index);
	omtshared virtual void disable_light(long index);

	omtshared virtual void set_light_pos(long index, OMedia3DPoint &p);
	omtshared virtual void set_light_dir(long index, OMedia3DVector &v);

	omtshared virtual void set_light_type(long index, omt_LightType type);

	omtshared virtual void set_light_ambient(long index, OMediaFARGBColor &argb);
	omtshared virtual void set_light_diffuse(long index, OMediaFARGBColor &argb);
	omtshared virtual void set_light_specular(long index, OMediaFARGBColor &argb);

	omtshared virtual void set_light_attenuation(long index, float range, float constant, float linear, float quadratic);

	omtshared virtual void set_light_spot_cutoff(long index, float cutoff);
	omtshared virtual void set_light_spot_exponent(long index, float expo);

	omtshared virtual void set_light_global_ambient(OMediaFARGBColor &argb);

	// * Culling

	omtshared virtual void enable_faceculling(void);
	omtshared virtual void disable_faceculling(void);

	// * Material

	omtshared virtual void set_material(OMediaFARGBColor &emission,
										OMediaFARGBColor &diffuse,
										OMediaFARGBColor &specular,
										OMediaFARGBColor &ambient,
										float			 shininess);

	// * Blending
	
	omtshared virtual void set_blend(omt_Blend blend);
	omtshared virtual void set_blend_func(omt_BlendFunc src_func, omt_BlendFunc dest_func);


	// * Fill mode

	omtshared virtual void set_override_material_fill_mode(bool override_mat_mode);

	omtshared virtual void set_fill_mode(omt_FillMode mode);
	omtshared virtual void set_shade_mode(omt_ShadeMode mode);
	
	// * Draw geometry

	omtshared virtual void draw(omt_RenderVertexList &vertices, omt_RenderDrawMode mode);

	omtshared virtual void draw_shape(OMedia3DShape *shape, bool &inout_second_pass,
										OMediaRendererOverrideVertexList	*ovlist =NULL);


	omtshared virtual void draw_surface(OMediaCanvas *canv, 
										float x, 	float y, 	float z, 
										float width,	float height,
										OMediaFARGBColor	&diffuse);

	// * Texture
	
	omtshared virtual void set_texture(OMediaCanvas *canvas);
	
	omtshared virtual void set_extra_texture_passes(omt_ExtraTexturePassList *pass_list);
	omtshared virtual long get_max_texture_passes(void);
	
	omtshared virtual void set_texture_address_mode(const omt_TextureAddressMode am);
	omtshared virtual void set_texture_color_operation(const omt_TextureColorOperation cm);

	// * Pipeline
	
	// You have no guarantee that polygons have been drawn as long as the following
	// method has not been called.
	
	omtshared virtual void flush_pipeline(void);


	// * Picking
	
	omtshared virtual void start_picking(unsigned long max_ids);
	omtshared virtual vector<OMediaPickHit> *end_picking(void);

	omtshared virtual void set_picking_id(unsigned long id);
	
	omtshared virtual OMediaPickRequest *get_pick_mode(void);	// Returns NULL if not in picking mode

	// * Fog (only linear for now)
	
	omtshared virtual void enable_fog(bool enable);
	omtshared virtual void set_fog_density(float d);
	omtshared virtual void set_fog_color(const OMediaFARGBColor &argb);
	omtshared virtual void set_fog_range(float start, float end);

	// * Texture perspective 
	
	omtshared virtual void enable_texture_persp(bool enabled);


	// * Clipping rect (scissor)

	omtshared virtual void enable_clipping_rect(bool enable);	
	omtshared virtual void set_clipping_rect(const OMediaRect &rect);


	protected:
	
	void dx_clear(DWORD flags, unsigned long argb, OMediaRect *src_area);

	void prepare_dx_context(void);

	void pick_dx_primitive(D3DPRIMITIVETYPE ptype, OMediaDXVertex *vertices, long nv);
	bool pick_clip_polygon(OMediaDXPickVert *&poly, long &npoints, omt_DXPickClippingPlane plane, bool	closed_poly);
	void pick_move_clipped_points(long np, OMediaDXPickVert *&poly, long &npoints, OMediaDXPickVert **ptab);
	void pick_find_zrange(OMediaDXPickVert *poly, 
										long npoints, bool &hit, 
										float &min_z, float &max_z,
										bool check_w =false);


	inline void set_d3d_filtering(	omt_CanvasFiltering mag_f,
									omt_CanvasFiltering min_f,
									long	pass =0)
	{
		switch(mag_f)
		{
			case omtfc_Nearest:
			d3d_device->SetTextureStageState(pass,D3DTSS_MAGFILTER,D3DTFG_POINT);
			break;

			case omtfc_Linear:
			d3d_device->SetTextureStageState(pass,D3DTSS_MAGFILTER,D3DTFG_LINEAR);
			break;
		}

		switch(min_f)
		{
 
			case omtfc_Nearest:
			d3d_device->SetTextureStageState(pass,D3DTSS_MINFILTER,D3DTFG_POINT);
			d3d_device->SetTextureStageState(pass,D3DTSS_MIPFILTER,D3DTFP_NONE );
			break;

			case omtfc_Linear:
			d3d_device->SetTextureStageState(pass,D3DTSS_MINFILTER,D3DTFG_LINEAR);
			d3d_device->SetTextureStageState(pass,D3DTSS_MIPFILTER,D3DTFP_NONE );
			break;

			case omtfc_Nearest_Mipmap_Nearest:
			d3d_device->SetTextureStageState(pass,D3DTSS_MINFILTER,D3DTFG_POINT);
			d3d_device->SetTextureStageState(pass,D3DTSS_MIPFILTER,D3DTFP_POINT );
			break;

			case omtfc_Nearest_Mipmap_Linear:
			d3d_device->SetTextureStageState(pass,D3DTSS_MINFILTER,D3DTFG_POINT);
			d3d_device->SetTextureStageState(pass,D3DTSS_MIPFILTER,D3DTFP_LINEAR );
			break;

			case omtfc_Linear_Mipmap_Nearest:
			d3d_device->SetTextureStageState(pass,D3DTSS_MINFILTER,D3DTFG_LINEAR);
			d3d_device->SetTextureStageState(pass,D3DTSS_MIPFILTER,D3DTFP_POINT );
			break;

			case omtfc_Linear_Mipmap_Linear:
			d3d_device->SetTextureStageState(pass,D3DTSS_MINFILTER,D3DTFG_LINEAR);
			d3d_device->SetTextureStageState(pass,D3DTSS_MIPFILTER,D3DTFP_LINEAR );
			break;

		}	

	}

	inline void pick_set_nhcoord(float *xyzw, OMedia3DPoint	&p)
	{
		float inv_w = 1.0f/xyzw[3];
		p.x = xyzw[0] * inv_w;
		p.y = xyzw[1] * inv_w;
		p.z = xyzw[2] * inv_w;
	}

	bool						cliprect_enabled;
	OMediaRect					clip_rect;

	omt_FillMode				fillmode;
	omt_ShadeMode				shademode;
	bool						light_enabled;
	bool						override_mat_fillmode;
	bool						blend_enabled;
	bool						culling_enabled;
	bool						picking_mode;
	bool						has_zbuffer;
	OMediaDXLight				*lights;
	OMediaCanvas				*texture;
	omt_ExtraTexturePassList	*current_texture_pass;

	omt_TextureAddressMode		texture_address_mode;
	omt_TextureColorOperation	texture_color_mode;
	
	OMediaPickHit				pick_newhit;

	static long						nrenderers;

	static vector<OMediaDXVertex>	vertex_buffer;	
	static vector<OMediaDXPickVert>	vpick_buffer;

	omtshared static OMediaDXPickVert			**pick_tab_buffer;
	omtshared static unsigned long				pick_tab_buffer_size;
	OMediaDXPickVert							*pick_temp_floor_buffer;

	inline void enlarge_pick_tab_buffer(long n)
	{
		delete [] pick_tab_buffer;

		pick_tab_buffer_size += n;
		pick_tab_buffer		= new OMediaDXPickVert*[pick_tab_buffer_size];
	}


	inline void check_vbuffer_size(long n)
	{
		if (n>vertex_buffer.size()) 
		{
			OMediaDXVertex	new_v;
			n -= vertex_buffer.size();	if (n<64) n=64;
			vertex_buffer.insert(vertex_buffer.end(),n, new_v);
		}
	}

	inline void check_vpick_size(long n)
	{
		if (n>vpick_buffer.size()) 
		{
			OMediaDXPickVert	new_v;
			n -= vpick_buffer.size();	if (n<64) n=64;
			vpick_buffer.insert(vpick_buffer.end(),n, new_v);
		}
	}

	void draw_shape_update_mat(OMedia3DMaterial *mat);

	long						max_lights;

	vector<OMediaPickHit> 		hit_list;

	LPDIRECT3DDEVICE7			d3d_device;
	D3DMATRIX					light_save_matrix;
};



//----------------------------------
// Render target

class OMediaDXRenderTarget : public OMediaRenderTarget
{
	public:
	
	OMediaDXRenderTarget(OMediaRenderer *renderer);
	virtual ~OMediaDXRenderTarget();
	
	virtual void purge(void);


	omtshared virtual OMediaRenderPort *new_port(void);

	omtshared virtual void render(void);

	omtshared virtual void prepare_context(OMediaWindow *window);


	// * Check texture size

	omtshared virtual void check_texture_size(long &w, long &h);


	omtshared virtual void flip_buffers(void);

	omtshared virtual void find_zbuffer_format(void);


	static HRESULT WINAPI EnumZBufferCallback( DDPIXELFORMAT* pddpf,
	                                           VOID* user );
 
	LPDIRECT3D7					d3d;
	LPDIRECT3DDEVICE7			d3d_device;
	LPDIRECTDRAWSURFACE7		d3d_back_buffer,d3d_zbuffer,dx_primary;
	LPDIRECTDRAW7				dx_draw;
	short						buffer_width,buffer_height;
	DDPIXELFORMAT				d3d_zbuffer_format;
	omt_ZBufferBitDepthFlags	zbuffer_flags;
	short						screen_depth;
	bool						square_texture;
	bool						page_flipping;
	long						max_texture_units;
	long						max_texture_width,max_texture_height;

};


//----------------------------------
// Renderer

class OMediaDXRenderer : 	public OMediaRenderer
{
	public:
	
	// * Construction
	
	omtshared OMediaDXRenderer(OMediaVideoEngine *video, 
								OMediaRendererDef *def, 
								omt_ZBufferBitDepthFlags zbuffer,
								bool depth_supported);

	omtshared virtual ~OMediaDXRenderer();


	// * Targets

	omtshared virtual OMediaRenderTarget *new_target(void);
	

	// * ZBuffer bit depth

	inline omt_ZBufferBitDepthFlags get_zbuffer_bitdepth(void) {return zbuffer;}

	
	omt_ZBufferBitDepthFlags	zbuffer;
	GUID						device_guid;
	bool						accelerated;
	bool						depth_supported;
};



#endif
#endif

