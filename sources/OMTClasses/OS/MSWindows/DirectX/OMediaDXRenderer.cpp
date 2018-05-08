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
#include "OMediaDXRenderer.h"
#include "OMediaDXCanvas.h"
#include "OMediaRendererInterface.h"
#include "OMediaDXVideoEngine.h"
#include "OMediaError.h"
#include "OMediaWindow.h"
#include "OMedia3DMaterial.h"
#include "OMedia3DShape.h"
#include "OMediaWinRtgWindow.h"
#include "OMediaMemTools.h"
#include "OMediaEndianSupport.h"
#include "OMediaDX3DShape.h"
#include "OMediaAnimPeriodical.h"
#include "OMediaMathTools.h"

static D3DBLEND omc_BlendTable[] =
{
	D3DBLEND_ZERO,
	D3DBLEND_ONE,
	D3DBLEND_DESTCOLOR,
	D3DBLEND_SRCCOLOR,
	D3DBLEND_INVDESTCOLOR,
	D3DBLEND_INVSRCCOLOR,
	D3DBLEND_SRCALPHA,
	D3DBLEND_INVSRCALPHA,
	D3DBLEND_DESTALPHA,
	D3DBLEND_INVDESTALPHA,
	D3DBLEND_SRCALPHASAT
};

vector<OMediaDXVertex>		OMediaDXRenderPort::vertex_buffer;	
vector<OMediaDXPickVert>	OMediaDXRenderPort::vpick_buffer;	
OMediaDXPickVert			**OMediaDXRenderPort::pick_tab_buffer;
unsigned long				OMediaDXRenderPort::pick_tab_buffer_size;


OMediaDXRenderPort::OMediaDXRenderPort(OMediaRenderTarget *target):
					  OMediaRenderPort(target) 
{
	fillmode = omfillmc_Solid;
	shademode = omshademc_Gouraud;
	light_enabled = false;
	override_mat_fillmode = false;
	blend_enabled = false;
	has_zbuffer = false;
	picking_mode = false;
	max_lights = 0;
	lights = NULL;
	nrenderers++;
	texture_address_mode = omtamc_Wrap;
	texture_color_mode = omtcoc_Modulate;

	cliprect_enabled = false;
	clip_rect.set(0,0,32,32);

	if (pick_tab_buffer_size==0)
	{
		pick_tab_buffer_size = 32;
		pick_tab_buffer		= new OMediaDXPickVert*[pick_tab_buffer_size];
	}			  
}

OMediaDXRenderPort::~OMediaDXRenderPort() 
{
	delete [] lights;

	nrenderers--;
	if (nrenderers==0)
	{
		vertex_buffer.erase(vertex_buffer.begin(),vertex_buffer.end());
		delete [] pick_tab_buffer;
		pick_tab_buffer = NULL;
		pick_tab_buffer_size = 0;
	}
}

void OMediaDXRenderPort::capture_frame(OMediaCanvas &canv)
{
	omt_RGBAPixel				*pix;
	OMediaDXVideoEngine			*dxengine;
	OMediaDXOffscreenBuffer		*dxbuffer;
	HRESULT						ddrval;
	HDC							hdc = NULL;

	((OMediaDXRenderTarget*)target)->prepare_context(target->get_window());

	if ( !((OMediaDXRenderTarget*)target)->d3d_device ) return;

	dxengine = (OMediaDXVideoEngine*)target->get_renderer()->get_video_engine();
	dxbuffer = (OMediaDXOffscreenBuffer*)dxengine->create_offscreen_buffer(bounds.get_width(), bounds.get_height(),
											 omobfc_ARGB8888);


	canv.lock(omlf_Write);
	pix = canv.get_pixels();

	long ww = target->get_window()->get_width(),wh = target->get_window()->get_height();
	OMediaRect		videorect,winrect;

	get_target()->get_renderer()->get_video_engine()->get_bounds(videorect);

	winrect.left = target->get_window()->get_x();
	winrect.top = target->get_window()->get_y();
	winrect.right = winrect.left + ww;
	winrect.bottom = winrect.top + wh;

	if (winrect.touch(&videorect))
	{
		for(;;)
		{
			if (dxengine->page_flipping)
				ddrval = dxengine->dx_back_page_surf->GetDC(&hdc);
			else
				ddrval = ((OMediaDXRenderTarget*)target)->d3d_back_buffer->GetDC(&hdc);
			
			if (ddrval==DD_OK)
			{
				BitBlt(dxbuffer->win_device_context,
						bounds.left, bounds.top,
						dxbuffer->win_bitmap_info.bmWidth,
						dxbuffer->win_bitmap_info.bmHeight,
						hdc,
						0,0,
						SRCCOPY);

				if (dxengine->page_flipping)
					dxengine->dx_back_page_surf->ReleaseDC(hdc);
				else
					((OMediaDXRenderTarget*)target)->d3d_back_buffer->ReleaseDC(hdc);
				break;
			}
			
			if(ddrval == DDERR_SURFACELOST)
			{
				dxengine->dx_primary_surf->Restore();
				if (dxengine->page_flipping) dxengine->dx_back_page_surf->Restore();
				else ((OMediaDXRenderTarget*)target)->d3d_back_buffer->Restore();
				break;
			}
			else if (ddrval != DDERR_WASSTILLDRAWING) break;
		}

		// Remap pixels

		char	*destp, *srcp,*srclp;
		short		x,y;

		srclp = (char*)dxbuffer->win_bitmap_info.bmBits;
		destp = (char*)pix;

		for(y=0;y<dxbuffer->win_bitmap_info.bmHeight;y++)
		{
			srcp = srclp;

			for(x=0;x<dxbuffer->win_bitmap_info.bmWidth;x++)
			{
				destp[3] = srcp[3];
				destp[0] = srcp[2];
				destp[1] = srcp[1];
				destp[2] = srcp[0];

				destp+=4;	srcp+=4;
			}

			srclp += dxbuffer->win_bitmap_info.bmWidthBytes;
		}
	}

	canv.unlock();

	delete dxbuffer;
}

void OMediaDXRenderPort::prepare_dx_context(void)
{
	D3DVIEWPORT7	vp;
	HRESULT			res;

	vp.dwX = bounds.left;
	vp.dwY = bounds.top;
	vp.dwWidth = bounds.get_width();
	vp.dwHeight = bounds.get_height();

	vp.dvMinZ = 0.0f;
	vp.dvMaxZ = 1.0f;

	res = d3d_device->SetViewport(&vp);
	if (res!=DD_OK) omd_OSEXCEPTION(res);
}


void OMediaDXRenderPort::render(void)
{
	if (begin_render())
	{
		OMediaRendererInterface	*iface = this; 
		broadcast_message(omsg_RenderPort,iface);
		end_render();
	}
}

bool OMediaDXRenderPort::begin_render(void)
{
	long ww = target->get_window()->get_width(),wh = target->get_window()->get_height();
	short	i;
	OMediaRect		videorect,winrect;

	d3d_device = ((OMediaDXRenderTarget*)get_target())->d3d_device;
	if (!d3d_device) return false;

	get_target()->get_renderer()->get_video_engine()->get_bounds(videorect);

	winrect.left = target->get_window()->get_x();
	winrect.top = target->get_window()->get_y();
	winrect.right = winrect.left + ww;
	winrect.bottom = winrect.top + wh;

	if (!winrect.touch(&videorect)) return false;

	prepare_dx_context();
	
	if (!lights) 
	{
		D3DDEVICEDESC7		ddesc;
		HRESULT				res;

		ddesc.dwDevCaps = sizeof(D3DDEVICEDESC7);
		res = d3d_device->GetCaps(&ddesc);

		max_lights = ddesc.dwMaxActiveLights;
		if (max_lights<0) max_lights = 8;

		lights = new OMediaDXLight[max_lights];
		start_light_edit();
		end_light_edit();
	}

	texture_address_mode = omtamc_Wrap;
	texture_color_mode = omtcoc_Modulate;

	d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
	d3d_device->SetRenderState(D3DRENDERSTATE_LOCALVIEWER, TRUE);
	d3d_device->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
	d3d_device->SetTextureStageState (0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);

	d3d_device->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
	d3d_device->SetTextureStageState(0,D3DTSS_COLORARG1,D3DTA_TEXTURE);  
	d3d_device->SetTextureStageState(0,D3DTSS_COLORARG2,D3DTA_DIFFUSE);
	d3d_device->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
	d3d_device->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	d3d_device->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	for(i=0;i<((OMediaDXRenderTarget*)target)->max_texture_units;i++)
	    d3d_device->SetTextureStageState( i, D3DTSS_TEXCOORDINDEX, i );


	has_zbuffer = ((OMediaDXRenderTarget*)get_target())->d3d_zbuffer != NULL;

	light_enabled = false;
	culling_enabled = true;
	cliprect_enabled = false;
	texture = NULL;
	current_texture_pass = NULL;
	d3d_device->SetTexture( 0, NULL );

	HRESULT	hres;

	hres = d3d_device->BeginScene();


	if ( FAILED(hres) ) return false;
	return true;
}

void OMediaDXRenderPort::end_render(void)
{
	d3d_device->EndScene();	
}

void OMediaDXRenderPort::enable_faceculling(void)
{
	if (!culling_enabled)
	{
		culling_enabled = true;
		d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW );
	}
}

void OMediaDXRenderPort::disable_faceculling(void)
{
	if (culling_enabled)
	{
		culling_enabled = false;
		d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
	}
}


void OMediaDXRenderPort::dx_clear(DWORD flags, unsigned long argb, OMediaRect *src_area)
{
	OMediaRect	area,dest;
	D3DRECT		drect;

	if (src_area)
	{
		area = *src_area;
		area.offset(bounds.left,bounds.top);
		if (!area.find_intersection(&bounds,&dest)) return;
	}
	else
	{
		dest = bounds;
	}

	drect.x1 = dest.left;
	drect.y1 = dest.top;
	drect.x2 = dest.right;
	drect.y2 = dest.bottom;

	if (!has_zbuffer) flags &= ~D3DCLEAR_ZBUFFER;
	d3d_device->Clear(1,&drect,flags, argb, 1.0f, 0);
}
 
//----------------------------------
// Renderer interface

omt_RendererRequirementFlags OMediaDXRenderPort::get_requirement_flags(void)
{
	return omrrf_ProjectionZRange_0_1|omrrf_ExactPixelCorrectEven;
}

void OMediaDXRenderPort::get_view_bounds(OMediaRect &r)
{
	r = bounds;
}

void OMediaDXRenderPort::clear_colorbuffer(OMediaFARGBColor &rgb, OMediaRect *src_area)
{
	dx_clear(D3DCLEAR_TARGET , rgb.get_argb(), src_area);
}

void OMediaDXRenderPort::clear_zbuffer(OMediaRect *src_area)
{
	dx_clear(D3DCLEAR_ZBUFFER  , 0, src_area);
}

void OMediaDXRenderPort::clear_all_buffers(OMediaFARGBColor &rgb)
{
	dx_clear(D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, rgb.get_argb(), NULL);
}

void OMediaDXRenderPort::set_zbuffer_write(omt_ZBufferWrite zb)
{
	if (!has_zbuffer) return;

	if (zb==omzbwc_Enabled)
	{
		d3d_device->SetRenderState(	D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	}
	else
	{
		d3d_device->SetRenderState(	D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	}
}

void OMediaDXRenderPort::set_zbuffer_test(omt_ZBufferTest zb)
{
	if (!has_zbuffer) return;

	if (zb==omzbtc_Enabled) d3d_device->SetRenderState(	D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
	else d3d_device->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
}

void OMediaDXRenderPort::set_zbuffer_func(omt_ZBufferFunc zb)
{
	D3DCMPFUNC	func;

	if (!has_zbuffer) return;

	switch(zb)
	{
		case omzbfc_Never:
		func = D3DCMP_NEVER;
		break;
		
		case omzbfc_Always:
		func = D3DCMP_ALWAYS;
		break;
		
		case omzbfc_Less:
		func = D3DCMP_LESS;
		break;

		case omzbfc_LEqual:
		func = D3DCMP_LESSEQUAL;
		break;

		case omzbfc_Equal:
		func = D3DCMP_EQUAL;
		break;

		case omzbfc_GEqual:
		func = D3DCMP_GREATEREQUAL;
		break;

		case omzbfc_Greater:
		func = D3DCMP_GREATER;
		break;
		
		case omzbfc_NotEqual:
		func = D3DCMP_NOTEQUAL;
		break;
		
		default:
		return;
	}

	d3d_device->SetRenderState(D3DRENDERSTATE_ZFUNC, func);		 
}
	
	// * Blending
	
void OMediaDXRenderPort::set_blend(omt_Blend blend)
{
	if (blend==omblendc_Disabled) 
	{	
		if (blend_enabled)
		{
			d3d_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE); 
			d3d_device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);		 
			d3d_device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);		 
			blend_enabled = false;
		}
	}
	else  
	{
		if (!blend_enabled)
		{
			d3d_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE); 
			blend_enabled = true;
		}
	}
}


void OMediaDXRenderPort::set_blend_func(omt_BlendFunc src_func, omt_BlendFunc dest_func)
{
	d3d_device->SetRenderState(D3DRENDERSTATE_SRCBLEND, omc_BlendTable[(long)src_func]);		 
	d3d_device->SetRenderState(D3DRENDERSTATE_DESTBLEND, omc_BlendTable[(long)dest_func]); 
}
	
	// * Matrix
	
void OMediaDXRenderPort::set_model_view(OMediaMatrix_4x4 &m)
{
	d3d_device->SetTransform( D3DTRANSFORMSTATE_WORLD , (D3DMATRIX*)&m.m[0][0]); 
}

void OMediaDXRenderPort::set_projection(OMediaMatrix_4x4 &m)
{
	d3d_device->SetTransform( D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX*)&m.m[0][0]); 
}

	// * Light

void OMediaDXRenderPort::enable_lighting(void) 
{
	if (!light_enabled)
	{
		light_enabled = true;
		d3d_device->SetRenderState( D3DRENDERSTATE_LIGHTING, TRUE); 
	}
}

void OMediaDXRenderPort::disable_lighting(void) 
{
	if (light_enabled)
	{
		light_enabled = false;
		d3d_device->SetRenderState( D3DRENDERSTATE_LIGHTING, FALSE); 
	}
}

long OMediaDXRenderPort::get_max_lights(void) 
{
	return max_lights;
}

void OMediaDXRenderPort::enable_light(long index) 
{
	d3d_device->LightEnable(index, TRUE); 
}

void OMediaDXRenderPort::disable_light(long index) 
{
	d3d_device->LightEnable(index, FALSE);
	d3d_device->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
}

void OMediaDXRenderPort::set_light_type(long index, omt_LightType type) 
{
	OMediaDXLight	*l = &lights[index];
		
	switch(type)
	{
		case omclt_Directional:	
		if (l->dlight.dltType!=D3DLIGHT_DIRECTIONAL)
		{
			l->dlight.dltType=D3DLIGHT_DIRECTIONAL;
			l->dirty = true;
		}
		break;

		case omclt_Point:
		if (l->dlight.dltType!=D3DLIGHT_POINT)
		{
			l->dlight.dltType=D3DLIGHT_POINT;
			l->dirty = true;
		}
		break;

		case omclt_Spot:
		if (l->dlight.dltType!=D3DLIGHT_SPOT)
		{
			l->dlight.dltType=D3DLIGHT_SPOT;
			l->dirty = true;
		}
		break;
	}
}

void OMediaDXRenderPort::start_light_edit(void)
{	         
}

void OMediaDXRenderPort::end_light_edit(void)
{
	OMediaMatrix_4x4	identity;
	bool				one_dirty = false;
	OMediaDXLight		*l;

	l = lights;
	for(long i=0;i<max_lights;i++,l++)
	{
		if (l->dirty)
		{
			if (!one_dirty && l->dlight.dltType == D3DLIGHT_DIRECTIONAL)
			{
				one_dirty = true;
				d3d_device->GetTransform(D3DTRANSFORMSTATE_VIEW, &light_save_matrix);

				identity.set_identity();
				d3d_device->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX*)&identity.m[0][0]);
			}

			d3d_device->SetLight(i,&l->dlight);
			l->dirty = false;
		}
	}

	if (one_dirty) d3d_device->SetTransform(D3DTRANSFORMSTATE_VIEW, &light_save_matrix);
}

void OMediaDXRenderPort::set_light_pos(long index, OMedia3DPoint &p)
{
	OMediaDXLight	*l = &lights[index];
	l->dirty = true;
	l->dlight.dvPosition.x = p.x;
	l->dlight.dvPosition.y = p.y;
	l->dlight.dvPosition.z = p.z;
}

void OMediaDXRenderPort::set_light_dir(long index, OMedia3DVector &v)
{
	OMediaDXLight	*l = &lights[index];

	l->dirty = true;
	l->dlight.dvDirection.x = v.x;
	l->dlight.dvDirection.y = v.y;
	l->dlight.dvDirection.z = v.z;
}

void OMediaDXRenderPort::set_light_ambient(long index, OMediaFARGBColor &argb) 
{
	OMediaDXLight	*l = &lights[index];

	l->dirty = true;
	l->dlight.dcvAmbient.r	= argb.red;
	l->dlight.dcvAmbient.g	= argb.green;
	l->dlight.dcvAmbient.b	= argb.blue;
	l->dlight.dcvAmbient.a	= argb.alpha;
}

void OMediaDXRenderPort::set_light_diffuse(long index, OMediaFARGBColor &argb)
{
	OMediaDXLight	*l = &lights[index];

	l->dirty = true;
	l->dlight.dcvDiffuse.r	= argb.red;
	l->dlight.dcvDiffuse.g	= argb.green;
	l->dlight.dcvDiffuse.b	= argb.blue;
	l->dlight.dcvDiffuse.a	= argb.alpha;
}

void OMediaDXRenderPort::set_light_specular(long index, OMediaFARGBColor &argb)
{
	OMediaDXLight	*l = &lights[index];

	l->dirty = true;
	l->dlight.dcvSpecular.r	= argb.red;
	l->dlight.dcvSpecular.g	= argb.green;
	l->dlight.dcvSpecular.b	= argb.blue;
	l->dlight.dcvSpecular.a	= argb.alpha;
}

void OMediaDXRenderPort::set_light_attenuation(long index, float range, float constant, float linear, float quadratic)
{
	OMediaDXLight	*l = &lights[index];
	
	range = 1.0f/range;		// The range does not work with DirectX7, so use the same range calculation than
							// with OpenGL

	l->dirty = true;
	l->dlight.dvAttenuation0 = constant;
	l->dlight.dvAttenuation1 = linear*range;
	l->dlight.dvAttenuation2 = quadratic*range;
}

void OMediaDXRenderPort::set_light_spot_cutoff(long index, float cutoff) 
{
	OMediaDXLight	*l = &lights[index];

	l->dirty = true;
	l->dlight.dvPhi = omd_pi * cutoff * (1.0f/180.0f);
}

void OMediaDXRenderPort::set_light_spot_exponent(long index, float expo) 
{
	OMediaDXLight	*l = &lights[index];

	l->dirty = true;
	l->dlight.dvFalloff = expo;
}

void OMediaDXRenderPort::set_light_global_ambient(OMediaFARGBColor &argb) 
{
	D3DCOLOR	c;

	c = D3DRGBA(argb.red,argb.green,argb.blue,argb.alpha);
	d3d_device->SetRenderState(D3DRENDERSTATE_AMBIENT, c);
}

	// * Material

void OMediaDXRenderPort::set_material(	OMediaFARGBColor &emission,
										OMediaFARGBColor &diffuse,
										OMediaFARGBColor &specular,
										OMediaFARGBColor &ambient,
										float			 shininess)
{
	D3DMATERIAL7	dmat;

	dmat.emissive.r = emission.red;	dmat.emissive.g = emission.green;	dmat.emissive.b = emission.blue;	dmat.emissive.a = emission.alpha;
	dmat.diffuse.r = diffuse.red;	dmat.diffuse.g = diffuse.green;		dmat.diffuse.b = diffuse.blue;		dmat.diffuse.a = diffuse.alpha;
	dmat.specular.r = specular.red;	dmat.specular.g = specular.green;	dmat.specular.b = specular.blue;	dmat.specular.a = specular.alpha;
	dmat.ambient.r = ambient.red;	dmat.ambient.g = ambient.green;		dmat.ambient.b = ambient.blue;		dmat.ambient.a = ambient.alpha;
	dmat.power = shininess;

	if (dmat.specular.r==0 &&
		dmat.specular.g==0 &&
		dmat.specular.b==0) 
	{
		dmat.power = 0;
		d3d_device->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	}
	else
		d3d_device->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, TRUE);

	d3d_device->SetMaterial(&dmat);
}

	// * Modes

void OMediaDXRenderPort::set_fill_mode(omt_FillMode mode) 
{
	if (!override_mat_fillmode) fillmode = mode;
}

void OMediaDXRenderPort::set_override_material_fill_mode(bool mfmode)
{
	override_mat_fillmode = mfmode;
}

void OMediaDXRenderPort::set_shade_mode(omt_ShadeMode mode) 
{
	shademode = mode;
	d3d_device->SetRenderState(D3DRENDERSTATE_SHADEMODE , 
								(shademode==omshademc_Flat)?D3DSHADE_FLAT:D3DSHADE_GOURAUD);
}

	// * Texture

void OMediaDXRenderPort::set_texture(OMediaCanvas *canvas)
{
	if (canvas)
	{
		OMediaDXCanvas		*dxcanv;
		OMediaRenderTarget	*rtarget = target;

		dxcanv = (OMediaDXCanvas*)canvas->find_implementation(target, omcskf_Scaled|omcskf_Exact, 0, true);
		
		if (!dxcanv) dxcanv = new OMediaDXCanvas((OMediaDXRenderTarget*)rtarget,canvas,omcskf_Scaled|omcskf_Exact);
		else dxcanv->render_prepare();
		
		d3d_device->SetTexture( 0, dxcanv->texture);

		set_d3d_filtering(canvas->get_filtering_mag(),canvas->get_filtering_min());

		texture = canvas;
	}
	else
	{
		d3d_device->SetTexture( 0, NULL);
		texture = NULL;
	}
}
	
void OMediaDXRenderPort::set_extra_texture_passes(omt_ExtraTexturePassList *pass_list)
{
	if (!pass_list)
	{
		current_texture_pass = NULL;
		d3d_device->SetTextureStageState(1,D3DTSS_COLOROP,D3DTOP_DISABLE);
		d3d_device->SetTextureStageState(1,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
		return;
	}
	else
	{
		OMediaCanvas		*canvas;
		OMediaDXCanvas		*dxcanv;
		OMediaRenderTarget	*rtarget = target;

		if (pass_list!=current_texture_pass)
		{
			current_texture_pass=pass_list;

			long	p,n=((OMediaDXRenderTarget*)target)->max_texture_units;
			omt_ExtraTexturePassList::iterator ei; 

			for(p=1,ei=current_texture_pass->begin();
				(p<n && ei!=current_texture_pass->end());
				p++,ei++)
			{
				// Texture

				canvas = (*ei).get_texture();
				if (!canvas) break;

				dxcanv = (OMediaDXCanvas*)canvas->find_implementation(target, omcskf_Scaled|omcskf_Exact, 0, true);
				
				if (!dxcanv) dxcanv = new OMediaDXCanvas((OMediaDXRenderTarget*)rtarget,canvas,omcskf_Scaled|omcskf_Exact);
				else dxcanv->render_prepare();
				
				d3d_device->SetTexture( p, dxcanv->texture);

				// Filtering

				set_d3d_filtering(canvas->get_filtering_mag(),canvas->get_filtering_min(),p);

				// Address mode

				d3d_device->SetTextureStageState (p,D3DTSS_ADDRESS,
					((*ei).get_texture_address_mode()==omtamc_Clamp)?D3DTADDRESS_CLAMP:D3DTADDRESS_WRAP);

				// Color operation

				if ((*ei).get_texture_color_operation()==omtcoc_Modulate)
				{
					d3d_device->SetTextureStageState(p, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			        d3d_device->SetTextureStageState(p, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				    d3d_device->SetTextureStageState(p, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
					d3d_device->SetTextureStageState(p, D3DTSS_ALPHAOP,	  D3DTOP_MODULATE);
					d3d_device->SetTextureStageState(p, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
					d3d_device->SetTextureStageState(p, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
				}
				else
				{
					d3d_device->SetTextureStageState(p,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
					d3d_device->SetTextureStageState(p,D3DTSS_COLORARG1,D3DTA_TEXTURE);  
					d3d_device->SetTextureStageState(p,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
					d3d_device->SetTextureStageState(p, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				}
			}

			if (p<n)
			{
				d3d_device->SetTextureStageState(p,D3DTSS_COLOROP,D3DTOP_DISABLE);
				d3d_device->SetTextureStageState(p,D3DTSS_ALPHAOP,D3DTOP_DISABLE);
			}
		}
	}
}

long OMediaDXRenderPort::get_max_texture_passes(void)
{
	return 0;	//+++
}
	
void OMediaDXRenderPort::set_texture_address_mode(const omt_TextureAddressMode am)
{
	if (am!=texture_address_mode)
	{
		texture_address_mode = am;
		if (am==omtamc_Wrap)
		{
			d3d_device->SetTextureStageState (0,D3DTSS_ADDRESS,D3DTADDRESS_WRAP);
		}
		else
		{
			d3d_device->SetTextureStageState (0,D3DTSS_ADDRESS,D3DTADDRESS_CLAMP);
		}
	}
}

void OMediaDXRenderPort::set_texture_color_operation(const omt_TextureColorOperation cm)
{
	if (cm!=texture_color_mode)
	{
		texture_color_mode = cm;
		if (cm==omtcoc_Modulate)
		{
			d3d_device->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_MODULATE);
			d3d_device->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
		}
		else
		{
			d3d_device->SetTextureStageState(0,D3DTSS_COLOROP,D3DTOP_SELECTARG1);
			d3d_device->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
		}
	}
}


	// * Geometry

static D3DPRIMITIVETYPE omc_PrimitiveTable[] =
{
	D3DPT_POINTLIST,D3DPT_LINELIST,D3DPT_LINESTRIP,
	D3DPT_LINESTRIP,D3DPT_TRIANGLELIST, D3DPT_TRIANGLESTRIP, D3DPT_TRIANGLEFAN,D3DPT_TRIANGLEFAN
};

void OMediaDXRenderPort::draw(omt_RenderVertexList &vertices, omt_RenderDrawMode mode)
{
	if (fillmode==omfillmc_Point) mode = omrdmc_Points;
	else if (fillmode==omfillmc_Line && mode>=omrdmc_Triangles) mode = omrdmc_LineLoop;

	D3DPRIMITIVETYPE					dprim = omc_PrimitiveTable[(long)mode];
	long								nprim = vertices.size();
	omt_RenderVertexList::iterator		rvi;
	vector<OMediaDXVertex>::iterator	dvi;

	if (mode==omrdmc_LineLoop) nprim++;

	check_vbuffer_size(nprim);

	for(rvi = vertices.begin(), dvi = vertex_buffer.begin();
		rvi!= vertices.end();
		rvi++, dvi++)
	{
		(*dvi).x = (*rvi).x;
		(*dvi).y = (*rvi).y;
		(*dvi).z = (*rvi).z;
		(*dvi).nx = (*rvi).normal.x;
		(*dvi).ny = (*rvi).normal.y;
		(*dvi).nz = (*rvi).normal.z;
		(*dvi).diffuse  = (*rvi).diffuse.get_argb();
		(*dvi).uv[0].u	= (*rvi).u;
		(*dvi).uv[0].v	= (*rvi).v;

		if (current_texture_pass)
		{
			short	uvi,maxuvi = (*rvi).extra_passes_uv.size();
			if (maxuvi>(omc_DXMaxTexturePass-1)) maxuvi = (omc_DXMaxTexturePass-1);

			for(uvi=0; uvi<maxuvi; uvi++)
				(*dvi).uv[uvi+1] = (*rvi).extra_passes_uv[uvi];
		}
	}

	if (mode==omrdmc_LineLoop) (*dvi) = (*vertex_buffer.begin());

	if (picking_mode)
		pick_dx_primitive(dprim, &(*vertex_buffer.begin()), nprim);
	else
		d3d_device->DrawPrimitive(dprim,
							D3DFVF_XYZ  | D3DFVF_NORMAL | D3DFVF_DIFFUSE  |
							omd_FVF_TEX,
							&(*vertex_buffer.begin()),
							nprim,
							0L);
}

void OMediaDXRenderPort::draw_shape(OMedia3DShape	*shape,
									bool			&inout_second_pass,
									OMediaRendererOverrideVertexList	*ovlist)
{
	shape->lock(omlf_Read);

	omt_PolygonList *polygons = shape->get_polygons();
	omt_VertexList *vlist  = shape->get_vertices(); 
	omt_NormalList *nlist = shape->get_normals();
	omt_ColorList	*clist = shape->get_colors();

	OMedia3DMaterial					*mat = NULL,*newmat;
	omt_PolygonList::iterator			poly_i;
	omt_3DPolygonVertexList::iterator	i,vend;
	bool								second_pass_found = false;

	D3DPRIMITIVETYPE					dprim;
	long								nprim;
	vector<OMediaDXVertex>::iterator	dvi;

	if (ovlist)
	{
		if (ovlist->vertex_list) vlist = ovlist->vertex_list;
		if (ovlist->normal_list) nlist = ovlist->normal_list;
		if (ovlist->color_list) clist = ovlist->color_list;
	}
	else if (!picking_mode)
	{
		// Cached shape

		if ((shape->get_flags()&omshfc_CompileStatic) && !OMediaAnimPeriodical::get_quick_refresh())
		{
			OMediaDX3DShape	*dx_shape;
			dx_shape = (OMediaDX3DShape*)shape->find_implementation(target, 0, 0);
		
			if (!dx_shape) 
			{
				dx_shape = new OMediaDX3DShape(	(OMediaDXRenderTarget*)target,
												shape,
												((OMediaDXRenderTarget*)target)->d3d,
												d3d_device);
			}

			short render_pass;
			render_pass = (!inout_second_pass) ? 0:1;
		
			dx_shape->prepare(render_pass);

			if (!inout_second_pass)
				inout_second_pass = dx_shape->second_pass_found;
			mat = NULL;
			
			for(omt_D3DVBufferList::iterator	dbi=dx_shape->d3d_buffers[render_pass].begin();
				dbi!=dx_shape->d3d_buffers[render_pass].end();
				dbi++)
			{
				if ((*dbi).material!=mat)
				{
					mat = (*dbi).material;
					draw_shape_update_mat(mat);
				}

				if ((*dbi).pass_list!=current_texture_pass)
					set_extra_texture_passes((*dbi).pass_list);

				if ( (*dbi).two_sided && culling_enabled) 		
				{
					d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
					culling_enabled = false;
				}
				else if ( !(*dbi).two_sided && !culling_enabled)
				{
					d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
					culling_enabled = true;		
				}		

				switch(fillmode)
				{
					case omfillmc_Point:
					d3d_device->DrawPrimitiveVB(D3DPT_POINTLIST,(*dbi).d3d_buffer,0,(*dbi).d3d_buffer_size,0);
					break;
					
					case omfillmc_Line:
					{
						unsigned short	close_indexes[2];
						for(vector<unsigned short>::iterator	ti = (*dbi).triangles.begin();
							ti!=(*dbi).triangles.end();
							ti+=3)
						{
							d3d_device->DrawIndexedPrimitiveVB(D3DPT_LINESTRIP,
															(*dbi).d3d_buffer,
															0,
															(*dbi).d3d_buffer_size,
															&(*ti),
															3,
															0);
							close_indexes[0] = ti[2];
							close_indexes[1] = ti[0];

							d3d_device->DrawIndexedPrimitiveVB(D3DPT_LINESTRIP,
															(*dbi).d3d_buffer,
															0,
															(*dbi).d3d_buffer_size,
															close_indexes,
															2,
															0);
						}
					}
					break;

					case omfillmc_Solid:
					d3d_device->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST,(*dbi).d3d_buffer,
														0,(*dbi).d3d_buffer_size,
														&(*dbi).triangles.front(),
														(*dbi).triangles.size(),
														0);
					break;
				}
				
			}

			shape->unlock();
			return;
		}
	}

	// Not cached

	for(poly_i=polygons->begin();
		poly_i!=polygons->end();
		poly_i++)
	{
		if (picking_mode) set_picking_id(poly_i-polygons->begin());
			
		newmat = (*poly_i).get_material();
		if (!newmat || ((*poly_i).get_flags()&om3pf_Hide)) continue;
	
		if (newmat!=mat)
		{
			if (!inout_second_pass)
			{
				if (newmat->get_flags()&ommatf_SecondPass)
				{
					second_pass_found = true;
					continue;
				}
			}
			else
			{
				if (!(newmat->get_flags()&ommatf_SecondPass)) continue;
			}
		
			mat = newmat;
			draw_shape_update_mat(mat);
		}

		if ((*poly_i).get_extra_texture_pass_index()==-1)
		{
			if (current_texture_pass) set_extra_texture_passes(NULL);
		}
		else
		{
			omt_ExtraTexturePassList	*p;
			p = &((*shape->get_extra_texture_pass_sets())[(*poly_i).get_extra_texture_pass_index()].pass_list);
			if (p!=current_texture_pass) set_extra_texture_passes(p);
		}
		
		if ( ((*poly_i).get_flags()&om3pf_TwoSided) && culling_enabled) 		
		{
			d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
			culling_enabled = false;
		}
		else if ( !((*poly_i).get_flags()&om3pf_TwoSided) && !culling_enabled)
		{
			d3d_device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
			culling_enabled = true;		
		}		

		nprim = (*poly_i).get_vertices().size();

		switch(fillmode)
		{
			case omfillmc_Point:
			dprim = D3DPT_POINTLIST;
			break;
			
			case omfillmc_Line:
			dprim = D3DPT_LINESTRIP;
			nprim++;
			break;

			case omfillmc_Solid:
			dprim = D3DPT_TRIANGLEFAN;
			break;
			
			default:
			continue;
		}

		check_vbuffer_size(nprim);
		vend = (*poly_i).get_vertices().end();

		OMedia3DVector		*n,*nbase = (nlist->size() && shademode!=omshademc_Flat)?nlist->begin():NULL;
		OMedia3DPoint		*p,*pbase = vlist->begin();
		unsigned long		mat_diffuse = mat->get_diffuse_ref().get_argb();

		for(i = (*poly_i).get_vertices().begin(), dvi = vertex_buffer.begin();
			i!= vend;
			i++, dvi++)
		{
			p = pbase + (*i).vertex_index;

			(*dvi).x = p->x;
			(*dvi).y = p->y;
			(*dvi).z = p->z;
			(*dvi).uv[0].u = (*i).u;
			(*dvi).uv[0].v = (*i).v;

			if (current_texture_pass)
			{
				short	uvi,maxuvi = (*i).extra_passes_uv.size();
				if (maxuvi>(omc_DXMaxTexturePass-1)) maxuvi = (omc_DXMaxTexturePass-1);

				for(uvi=0; uvi<maxuvi; uvi++)
					(*dvi).uv[uvi+1] = (*i).extra_passes_uv[uvi];
			}

			switch(mat->get_light_mode())
			{
				case ommlmc_Color:
				(*dvi).diffuse  = mat_diffuse;
				break;

				case ommlmc_VertexColor:
				{
					OMediaFARGBColor	*argb = clist->begin() + (*i).color_index;	
					(*dvi).diffuse  = argb->get_argb();
				}
				break;

				case ommlmc_Light:
				(*dvi).diffuse  = mat_diffuse;
				if (nbase)
				{
					n = nbase + (*i).normal_index;
					(*dvi).nx = n->x;
					(*dvi).ny = n->y;
					(*dvi).nz = n->z;
				}
				else
				{
					(*dvi).nx = (*poly_i).get_normal().x;
					(*dvi).ny = (*poly_i).get_normal().y;
					(*dvi).nz = (*poly_i).get_normal().z;
				}
				break;
			}
		}

		if (fillmode==omfillmc_Line) (*dvi) = (*vertex_buffer.begin());

		if (picking_mode)
			pick_dx_primitive(dprim, &(*vertex_buffer.begin()), nprim);
		else
			d3d_device->DrawPrimitive(dprim,
							D3DFVF_XYZ  | D3DFVF_NORMAL | D3DFVF_DIFFUSE | 
							omd_FVF_TEX,
							&(*vertex_buffer.begin()),
							nprim,
							0L);
	}
	
	inout_second_pass = second_pass_found;

	shape->unlock();
}

void OMediaDXRenderPort::draw_shape_update_mat(OMedia3DMaterial *mat)
{
	if (mat->get_texture())
	{
		if (texture!=mat->get_texture()) set_texture(mat->get_texture());
	}
	else if (texture)
	{
		d3d_device->SetTexture(0,NULL);
		texture = NULL;
	}

	set_texture_address_mode(mat->get_texture_address_mode());
	set_texture_color_operation(mat->get_texture_color_operation());

	if (mat->get_blend_src()==omblendfc_One &&
		mat->get_blend_dest()==omblendfc_Zero) 
	{
		if (blend_enabled)
		{
			d3d_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,FALSE); 
			d3d_device->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);		 
			d3d_device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);		 
			blend_enabled = false;
		}
	}
	else
	{
		if (!blend_enabled)
		{
			d3d_device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,TRUE); 
			blend_enabled = true;
		}

		d3d_device->SetRenderState(D3DRENDERSTATE_SRCBLEND, omc_BlendTable[(long)mat->get_blend_src()]);		 
		d3d_device->SetRenderState(D3DRENDERSTATE_DESTBLEND, omc_BlendTable[(long)mat->get_blend_dest()]); 
	}
	
	if (mat->get_light_mode()==ommlmc_Light)
	{
		if (!light_enabled)
		{
			light_enabled = true;
			d3d_device->SetRenderState( D3DRENDERSTATE_LIGHTING, TRUE); 
		}

		set_material(	mat->get_emission_ref(),
						mat->get_diffuse_ref(),
						mat->get_specular_ref(),
						mat->get_ambient_ref(),
						mat->get_shininess());
	}
	else
	{
		if (light_enabled)
		{
			light_enabled = false;
			d3d_device->SetRenderState( D3DRENDERSTATE_LIGHTING, FALSE); 
		}
	}

	if (!override_mat_fillmode) fillmode = mat->get_fill_mode();

	if (shademode!=mat->get_shade_mode())
	{
		shademode = mat->get_shade_mode();
		d3d_device->SetRenderState(D3DRENDERSTATE_SHADEMODE , 
						(shademode==omshademc_Flat)?D3DSHADE_FLAT:D3DSHADE_GOURAUD);
	}
}

void OMediaDXRenderPort::draw_surface(OMediaCanvas *canv, 
										float x, 	float y, 	float z, 
										float width,	float height,
										OMediaFARGBColor	&diffuse)
{
	OMediaDXCanvas						*dxcanv;
	omt_CanvasSlaveKeyFlags				kflags;
	OMediaRenderTarget					*rtarget = target;
	vector<OMediaDXVertex>::iterator	dvi;
	long								nprim;
	D3DPRIMITIVETYPE					dprim;

	switch(fillmode)
	{
		case omfillmc_Point:
		dprim = D3DPT_POINTLIST;
		break;

		case omfillmc_Line:
		dprim = D3DPT_LINESTRIP;
		break;

		case omfillmc_Solid:
		dprim = D3DPT_TRIANGLEFAN;		
		break;
	}

	kflags = omcskf_SubDivided|omcskf_Exact;

	dxcanv = (OMediaDXCanvas*) canv->find_implementation(target, kflags, 0, true);
	if (!dxcanv) dxcanv = new OMediaDXCanvas((OMediaDXRenderTarget*)rtarget,canv,omcskf_SubDivided);
	else dxcanv->render_prepare();

	nprim = (fillmode==omfillmc_Line)?5:4;
	check_vbuffer_size(nprim);

	unsigned long argb_dif = diffuse.get_argb();

	set_texture_address_mode(omtamc_Clamp);
	set_texture_color_operation(omtcoc_Modulate);
	set_extra_texture_passes(NULL);

	set_d3d_filtering(canv->get_filtering_mag(),canv->get_filtering_min());


	if (dxcanv->get_slave_key_long()&omcskf_Exact)
	{
		d3d_device->SetTexture(0,dxcanv->texture);

		dvi = vertex_buffer.begin();

		(*dvi).x = x;		(*dvi).y = y;		(*dvi).z = z;
		(*dvi).nx = 0.0f;	(*dvi).ny = 0.0f;	(*dvi).nz = -1.0f;
		(*dvi).diffuse  = argb_dif;			
		(*dvi).uv[0].u = 0.0f;	(*dvi).uv[0].v = 0.9999f;
		dvi++;

		(*dvi).x = x+width;	(*dvi).y = y;		(*dvi).z = z;
		(*dvi).nx = 0.0f;	(*dvi).ny = 0.0f;	(*dvi).nz = -1.0f;
		(*dvi).diffuse  = argb_dif;				
		(*dvi).uv[0].u = 0.9999f;	(*dvi).uv[0].v = 0.9999f;
		dvi++;

		(*dvi).x = x+width;	(*dvi).y = y+height;(*dvi).z = z;
		(*dvi).nx = 0.0f;	(*dvi).ny = 0.0f;	(*dvi).nz = -1.0f;
		(*dvi).diffuse  = argb_dif;				
		(*dvi).uv[0].u = 0.9999f;	(*dvi).uv[0].v = 0.0f;
		dvi++;

		(*dvi).x = x;		(*dvi).y = y+height;(*dvi).z = z;
		(*dvi).nx = 0.0f;	(*dvi).ny = 0.0f;	(*dvi).nz = -1.0f;
		(*dvi).diffuse  = argb_dif;				
		(*dvi).uv[0].u = 0.0f;	(*dvi).uv[0].v = 0;
		

		if (nprim==5) { dvi++; (*dvi) = *(vertex_buffer.begin());}

		if (picking_mode)
			pick_dx_primitive(dprim, &(*vertex_buffer.begin()), nprim);
		else
			d3d_device->DrawPrimitive(dprim,
							D3DFVF_XYZ  | D3DFVF_NORMAL | D3DFVF_DIFFUSE |
							omd_FVF_TEX,
							&(*vertex_buffer.begin()),
							nprim,
							0L);
	}
	else
	{
		long						tx,ty;
		LPDIRECTDRAWSURFACE7		*textid = dxcanv->texturegrid;

		float		canv_w = float(canv->get_width()), canv_h = float(canv->get_height());
		float		scalex = width  / canv_w,
					scaley = height / canv_h;

		float		incx = dxcanv->subdiv_w * scalex,
					incy = dxcanv->subdiv_h * scaley;
		
		float		ix,iy,x2,y2,ih = canv_h * scaley;
		
		float		u2,v2;

		for(ty =0, iy = 0; ty < dxcanv->n_texth; ty++, iy +=incy)
		{
			v2 = (ty==dxcanv->n_texth-1)?dxcanv->last_v:0.9999f;
		
			for(tx =0, ix = 0; tx < dxcanv->n_textw; tx++,textid++,ix+=incx)
			{
				u2 = (tx==dxcanv->n_textw-1)?dxcanv->last_u:0.9999f;

				d3d_device->SetTexture(0,*textid);

				dvi = vertex_buffer.begin();

				x2 = ix+incx;	if (x2>width) x2 = width;
				y2 = iy+incy;	if (y2>height) y2 = height;

				(*dvi).x = x+ix;	(*dvi).y = y+(ih-iy);	(*dvi).z = z;
				(*dvi).nx = 0.0f;	(*dvi).ny = 0.0f;		(*dvi).nz = -1.0f;
				(*dvi).diffuse  = argb_dif;				
				(*dvi).uv[0].u = 0.0f;	(*dvi).uv[0].v = 0.0f;
				dvi++;

				(*dvi).x = x+ix;	(*dvi).y = y+(ih-y2);	(*dvi).z = z;
				(*dvi).nx = 0.0f;	(*dvi).ny = 0.0f;		(*dvi).nz = -1.0f;
				(*dvi).diffuse  = argb_dif;				
				(*dvi).uv[0].u = 0.0f;	(*dvi).uv[0].v = v2;
				dvi++;

				(*dvi).x = x+x2;	(*dvi).y = y+(ih-y2);	(*dvi).z = z;
				(*dvi).nx = 0.0f;	(*dvi).ny = 0.0f;		(*dvi).nz = -1.0f;
				(*dvi).diffuse  = argb_dif;				
				(*dvi).uv[0].u = u2;		(*dvi).uv[0].v = v2;
				dvi++;

				(*dvi).x = x+x2;	(*dvi).y = y+(ih-iy);	(*dvi).z = z;
				(*dvi).nx = 0.0f;	(*dvi).ny = 0.0f;		(*dvi).nz = -1.0f;
				(*dvi).diffuse  = argb_dif;				
				(*dvi).uv[0].u = u2;		(*dvi).uv[0].v = 0;
				

				if (nprim==5) { dvi++; (*dvi) = *(vertex_buffer.begin());}


				if (picking_mode)
					pick_dx_primitive(dprim, &(*vertex_buffer.begin()), nprim);
				else
					d3d_device->DrawPrimitive(dprim,
									D3DFVF_XYZ  | D3DFVF_NORMAL | D3DFVF_DIFFUSE |
									omd_FVF_TEX,
									&(*vertex_buffer.begin()),
									nprim,
									0L);
			}	
		}
	}

	d3d_device->SetTexture(0,NULL);
	current_texture_pass = NULL;
	texture = NULL;
}

	
void OMediaDXRenderPort::flush_pipeline(void) {}


// * Picking



bool OMediaDXRenderPort::pick_clip_polygon(OMediaDXPickVert *&poly, 
										long &npoints, 
										omt_DXPickClippingPlane plane,
										bool	closed_poly)
{
	OMediaDXPickVert		*ptr1,*ptr2,*last_p,*ptr1nv;
	bool					clip_current, clip_next,one_clipped;
    float 					para,d,dw;
	OMediaDXPickVert	 	**ptab;
	long					np = 0;

    ptr1 = poly;
	ptr2 = poly + npoints;
	if (ptr2<pick_temp_floor_buffer) ptr2 = pick_temp_floor_buffer;
	last_p = ptr1 + npoints;

	if ((npoints<<2L)>pick_tab_buffer_size) enlarge_pick_tab_buffer(npoints<<2L);

	ptab = pick_tab_buffer;
    one_clipped = false;

	for (;ptr1!=last_p;ptr1++)
    {
		ptr1nv = ptr1;
		ptr1nv++;
		if (ptr1nv==last_p) 
		{
			if (!closed_poly) break;
			ptr1nv = poly;  
		}

#define x xyzw[0]
#define y xyzw[1]
#define z xyzw[2]
#define w xyzw[3]
	
		switch(plane)
		{
			case omdxclippc_Left:
       		clip_current = (ptr1->x < -ptr1->w);
        	clip_next    = (ptr1nv->x < -ptr1nv->w);
			break;

			case omdxclippc_Right:
       		clip_current = (ptr1->x > ptr1->w);
        	clip_next    = (ptr1nv->x > ptr1nv->w);
			break;

			case omdxclippc_Top:
       		clip_current = (ptr1->y > ptr1->w);
        	clip_next    = (ptr1nv->y > ptr1nv->w);
			break;

			case omdxclippc_Bottom:
       		clip_current = (ptr1->y < -ptr1->w);
        	clip_next    = (ptr1nv->y < -ptr1nv->w);
			break;

			case omdxclippc_Near:
       		clip_current = (ptr1->z < -ptr1->w);
        	clip_next    = (ptr1nv->z < -ptr1nv->w);
			break;

			case omdxclippc_Far:
       		clip_current = (ptr1->z > ptr1->w);
        	clip_next    = (ptr1nv->z > ptr1nv->w);
			break;
		}


         if (!clip_current)
         {
			*ptab = ptr1;
			ptab++;
			np++;
			ptr2++;
         }
         
		 if (clip_current != clip_next)
         {
       		one_clipped = true;


			switch(plane)
			{
				case omdxclippc_Left:
				d = (ptr1nv->x -  ptr1->x);
				dw = (ptr1nv->w -  ptr1->w);
				para = -((ptr1->x + ptr1->w) / (dw+d));
				
				ptr2->x = ptr1->x + para * d;
				ptr2->y = ptr1->y + para * (ptr1nv->y - ptr1->y);
				ptr2->z = ptr1->z + para * (ptr1nv->z - ptr1->z);
				ptr2->w = ptr1->w + para * dw;
				break;
	
				case omdxclippc_Right:
				d = (ptr1nv->x -  ptr1->x);
				dw = (ptr1nv->w -  ptr1->w);
				para = (ptr1->x - ptr1->w) / (dw-d);
				
				ptr2->x = ptr1->x + para * d;
				ptr2->y = ptr1->y + para * (ptr1nv->y - ptr1->y);
				ptr2->z = ptr1->z + para * (ptr1nv->z - ptr1->z);
				ptr2->w = ptr1->w + para * dw;
				break;
	
				case omdxclippc_Top:
				d = (ptr1nv->y -  ptr1->y);
				dw = (ptr1nv->w -  ptr1->w);
				para = ((ptr1->y - ptr1->w) / (dw-d));
				
				ptr2->x = ptr1->x + para * (ptr1nv->x - ptr1->x);
				ptr2->y = ptr1->y + para * d;
				ptr2->z = ptr1->z + para * (ptr1nv->z - ptr1->z);
				ptr2->w = ptr1->w + para * dw;
				break;
	
				case omdxclippc_Bottom:
				d = (ptr1nv->y -  ptr1->y);
				dw = (ptr1nv->w -  ptr1->w);
				para = -((ptr1->y + ptr1->w) / (dw+d));
				
				ptr2->x = ptr1->x + para * (ptr1nv->x - ptr1->x);
				ptr2->y = ptr1->y + para * d;
				ptr2->z = ptr1->z + para * (ptr1nv->z - ptr1->z);
				ptr2->w = ptr1->w + para * dw;
				break;
	
				case omdxclippc_Near:
				d = (ptr1nv->z -  ptr1->z);
				dw = (ptr1nv->w -  ptr1->w);
				para = -((ptr1->z + ptr1->w) / (dw+d));
				
				ptr2->x = ptr1->x + para * (ptr1nv->x - ptr1->x);
				ptr2->y = ptr1->y + para * (ptr1nv->y - ptr1->y);
				ptr2->z = ptr1->z + para * d;
				ptr2->w = ptr1->w + para * dw;
				break;
	
				case omdxclippc_Far:
				d = (ptr1nv->z -  ptr1->z);
				dw = (ptr1nv->w -  ptr1->w);
				para = ((ptr1->z - ptr1->w) / (dw-d));
				
				ptr2->x = ptr1->x + para * (ptr1nv->x - ptr1->x);
				ptr2->y = ptr1->y + para * (ptr1nv->y - ptr1->y);
				ptr2->z = ptr1->z + para * d;
				ptr2->w = ptr1->w + para * dw;
				break;
			}		
			
			*ptab = omc_NULL;
			ptab++;
			ptr2++; 
			np++;
        }
	}

#undef x
#undef y
#undef z
#undef w
    
        
    if (np==0) return true;
        
    if (one_clipped) pick_move_clipped_points(np,poly,npoints,pick_tab_buffer); 

  	return false;
}

void OMediaDXRenderPort::pick_move_clipped_points(long np, 
								OMediaDXPickVert *&poly,
								long			  &npoints,
								OMediaDXPickVert **ptab)
{
	OMediaDXPickVert	*ptr1,*ptr2;

	ptr2 = poly + npoints;
	if (ptr2<pick_temp_floor_buffer) ptr2 = pick_temp_floor_buffer;
	poly = ptr2;
	npoints = np;
	while(np--)
	{
		ptr1 = *ptab;
		if (ptr1)
		{
			ptr2->xyzw[0] = ptr1->xyzw[0];
			ptr2->xyzw[1] = ptr1->xyzw[1];
			ptr2->xyzw[2] = ptr1->xyzw[2];
			ptr2->xyzw[3] = ptr1->xyzw[3];
		}

		ptr2++;
		ptab++;
	}
}

void OMediaDXRenderPort::pick_find_zrange(OMediaDXPickVert *poly, 
										long npoints, bool &hit, 
										float &min_z, float &max_z,
										bool	check_w)
{
	long i;
	float	x,y,z,w;

	for(i=0;i<npoints;i++)
	{		
		x = poly[i].xyzw[0];
		y = poly[i].xyzw[1];
		z = poly[i].xyzw[2];
		w = poly[i].xyzw[3];

		if (!check_w || !(	x < -w || x > w ||
							y < -w || y > w ||
							z < -w || z > w))
		{
			z /= w;

			if (!hit) {min_z = z; max_z = z; hit = true;}
			else
			{
				if (z<min_z) min_z = z;
				if (z>max_z) max_z = z;
			}
		}
	}
}

void OMediaDXRenderPort::pick_dx_primitive(D3DPRIMITIVETYPE ptype, OMediaDXVertex *vertices, long nv)
{
	OMediaMatrix_4x4	view_matrix,proj_matrix;
	long				i;
	bool				hit = false;
	float				min_z,max_z;

	check_vpick_size(nv<<4);

	// DX immediate mode has no picking support, so I need to emulate the pipeline	
	
	// Get current matrixes
	d3d_device->GetTransform( D3DTRANSFORMSTATE_WORLD , (D3DMATRIX*)&view_matrix.m[0][0]); 
	d3d_device->GetTransform( D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX*)&proj_matrix.m[0][0]); 

	// Transform the vertices

	for(i=0;i<nv;i++)
	{	
		vpick_buffer[i].xyzw[0] = vertices[i].x;
		vpick_buffer[i].xyzw[1] = vertices[i].y;
		vpick_buffer[i].xyzw[2] = vertices[i].z;
		vpick_buffer[i].xyzw[3] = 1.0f;

		view_matrix.multiply(vpick_buffer[i].xyzw);
		proj_matrix.multiply(vpick_buffer[i].xyzw);	
	}

	// Back face culling (in screen space)

	if (culling_enabled && nv>=3)
	{
		OMedia3DPoint	p1,p2,p3;
		OMedia3DVector	u,v,n,er;

		pick_set_nhcoord(vpick_buffer[0].xyzw, p1);
		pick_set_nhcoord(vpick_buffer[1].xyzw, p2);
		pick_set_nhcoord(vpick_buffer[2].xyzw, p3);
			
		u.set(p1,p2);
		v.set(p1,p3);

		v.cross_product(u,n);

		if (n.z>=0) return;
	}


	OMediaDXPickVert	*poly;
	long				npoints;

	pick_temp_floor_buffer = &vpick_buffer[nv];

	switch(ptype)
	{
		case D3DPT_POINTLIST:
		poly = &vpick_buffer[0];
		npoints = nv;
		pick_find_zrange(poly, npoints, hit, min_z, max_z);		
		break;

		case D3DPT_LINELIST:
		for(i=0;i<nv>>1;i++)
		{
			poly = &vpick_buffer[i<<1];
			npoints = 2;

			if (!pick_clip_polygon(poly, npoints, omdxclippc_Left,		false) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Right,		false) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Top,		false) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Bottom,	false)	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Near,		false) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Far,		false))
			{
				pick_find_zrange(poly, npoints, hit, min_z, max_z,true);	
			}
		}
		break;

		case D3DPT_LINESTRIP:
		poly = &vpick_buffer[0];
		npoints = nv;

		if		(!pick_clip_polygon(poly, npoints, omdxclippc_Left,	false) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Right,	false) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Top,		false) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Bottom,	false)  &&
				!pick_clip_polygon(poly, npoints, omdxclippc_Near,	false) 	&&
				!pick_clip_polygon(poly, npoints,  omdxclippc_Far,	false))
		{
			pick_find_zrange(poly, npoints, hit, min_z, max_z);	
		}
		break;

		case D3DPT_TRIANGLELIST:
		if (nv<3) break;
		for(i=0;i<nv;i+=3)
		{
			poly = &vpick_buffer[i];
			npoints = 3;

			if (!pick_clip_polygon(poly, npoints, omdxclippc_Left,	true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Right,	true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Top,		true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Bottom,	true)	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Near,	true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Far,		true))
			{
				pick_find_zrange(poly, npoints, hit, min_z, max_z);	
			}
		}
		break;

		case D3DPT_TRIANGLESTRIP:
		for(i=0;i<nv-2;i++)
		{
			poly = &vpick_buffer[i];
			npoints = 3;

			// I don't care about the clockwise orientation for clipping test
			if (!pick_clip_polygon(poly, npoints, omdxclippc_Left,	true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Right,	true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Top,		true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Bottom,	true)	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Near,	true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Far,		true))
			{
				pick_find_zrange(poly, npoints, hit, min_z, max_z);	
			}
		}
		break;

		case D3DPT_TRIANGLEFAN:
		poly = &vpick_buffer[0];
		npoints = nv;

		if (!pick_clip_polygon(poly, npoints, omdxclippc_Left,		true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Right,	true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Top,		true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Bottom,	true) &&
				!pick_clip_polygon(poly, npoints, omdxclippc_Near,	true) 	&&
				!pick_clip_polygon(poly, npoints, omdxclippc_Far,		true))
		{
			pick_find_zrange(poly, npoints, hit, min_z, max_z);	
		}
		break;
	}


	if (hit)
	{
		if (hit_list.size()==0 ||
			hit_list.back().id!=pick_newhit.id)
		{
			hit_list.push_back(pick_newhit);
		}

		hit_list.back().maxz = max_z;
		hit_list.back().minz = min_z;
	}
}


void OMediaDXRenderPort::start_picking(unsigned long max_ids)
{
	picking_mode = true;
	hit_list.erase(hit_list.begin(),hit_list.end());
}

vector<OMediaPickHit> *OMediaDXRenderPort::end_picking(void)
{
	picking_mode = false;	
	return &hit_list;
}

void OMediaDXRenderPort::set_picking_id(unsigned long id)
{
	pick_newhit.id = id;
}


OMediaPickRequest *OMediaDXRenderPort::get_pick_mode(void)
{
	return get_target()->get_renderer()->get_picking_mode();
} 

//---------------------------------------

// Use clipping planes, more a hack than anything else (waiting for scissor)...

void OMediaDXRenderPort::enable_clipping_rect(bool enable)
{
	cliprect_enabled = enable;

	if (cliprect_enabled)
		d3d_device->SetRenderState( D3DRENDERSTATE_CLIPPLANEENABLE, 0xf );
	else
		d3d_device->SetRenderState( D3DRENDERSTATE_CLIPPLANEENABLE, 0 );
}

void OMediaDXRenderPort::set_clipping_rect(const OMediaRect &rect)
{
	float			plane[4];
	OMedia3DPoint	p;
	OMedia3DVector	v;
	int				i;

	clip_rect=rect;
	clip_rect.top = bounds.get_height()-rect.bottom;
	clip_rect.bottom = bounds.get_height()-rect.top;
	clip_rect.offset(-(bounds.get_width()>>1),-(bounds.get_height()>>1));

	clip_rect.top = -clip_rect.top;
	clip_rect.bottom = -clip_rect.bottom;

	for(i=0;i<4;i++)
	{
		switch(i)
		{
			case 0:	v.set(0,1,0);	p.set(0,clip_rect.bottom,0);	break;	// Bottom
			case 1:	v.set(1,0,0);	p.set(clip_rect.left,0,0);		break;	// Left
			case 2:	v.set(0,-1,0);	p.set(0,clip_rect.top,0);		break;	// top
			case 3:	v.set(-1,0,0);	p.set(clip_rect.right,0,0);		break;	// Right
		}

		OMediaMathTools::find_plane(v.xyzw(),p.xyzw(),plane);
	    d3d_device->SetClipPlane(i, plane);
	}

}

void OMediaDXRenderPort::enable_fog(bool enable)
{
	//+++ NOT IMPLEMENTED
}

void OMediaDXRenderPort::set_fog_density(float d)
{
	//+++ NOT IMPLEMENTED
}

void OMediaDXRenderPort::set_fog_color(const OMediaFARGBColor &argb)
{
	//+++ NOT IMPLEMENTED
}

void OMediaDXRenderPort::set_fog_range(float start, float end)
{
	//+++ NOT IMPLEMENTED
}
	
void OMediaDXRenderPort::enable_texture_persp(bool enabled)
{
	//+++ NOT IMPLEMENTED
}

//---------------------------------------


long			OMediaDXRenderPort::nrenderers;


//----------------------------------
// Render target

OMediaDXRenderTarget::OMediaDXRenderTarget(OMediaRenderer *renderer) :
						OMediaRenderTarget(renderer,
											ommeic_DirectX,
											renderer->get_video_engine()->get_supervisor_window())
{
	DDSURFACEDESC2	dxdesc;

	d3d_device = NULL;
	d3d = ((OMediaDXVideoEngine*)renderer->get_video_engine())->dx_d3d;
	dx_draw = ((OMediaDXVideoEngine*)renderer->get_video_engine())->dx_draw;
	dx_primary = ((OMediaDXVideoEngine*)renderer->get_video_engine())->dx_primary_surf;
	d3d_zbuffer = NULL;
	square_texture = false;
	page_flipping = ((OMediaDXVideoEngine*)renderer->get_video_engine())->page_flipping;
	if (page_flipping)
		d3d_back_buffer = ((OMediaDXVideoEngine*)renderer->get_video_engine())->dx_back_page_surf;

	dxdesc.dwSize = sizeof(DDSURFACEDESC2);
	dx_draw->GetDisplayMode(&dxdesc);
	screen_depth = dxdesc.ddpfPixelFormat.dwRGBBitCount;
	max_texture_width = 128;
	max_texture_height = 128;

	find_zbuffer_format();						
}

OMediaDXRenderTarget::~OMediaDXRenderTarget()
{
	purge();
}
	
OMediaRenderPort *OMediaDXRenderTarget::new_port(void)
{
	return new OMediaDXRenderPort(this);
}

void OMediaDXRenderTarget::render(void)
{
	prepare_context(window);
	if (d3d_device) 
	{
		OMediaRenderTarget::render();
		if (!get_renderer()->get_picking_mode()) flip_buffers();
	}
}

void OMediaDXRenderTarget::purge(void)
{
	if (d3d_device)
	{
		delete_all_implementations();

		d3d_device->SetTexture( 0, NULL);
		d3d_device->Release();
		if (!page_flipping) d3d_back_buffer->Release();
		if (d3d_zbuffer) d3d_zbuffer->Release();

		d3d_device = NULL;
		d3d_zbuffer = NULL;
	}
}



static unsigned long getnearestpow(unsigned long n)
{
	long bit;

	for(bit= 32;;)
	{
		bit--;
		
		if (n&(1<<bit)) return (1<<bit);
		if (bit==0) break;
	}
	
	return 0;
}

void OMediaDXRenderTarget::check_texture_size(long &w, long &h)
{
	short	nw,nh;

	if (w>max_texture_width) w = max_texture_width;
	if (h>max_texture_height) h = max_texture_height;

	nw = getnearestpow(w);
	nh = getnearestpow(h);
	
	if (w!=nw) nw <<=1;
	if (h!=nh) nh <<=1;

	if (nw>max_texture_width) nw = max_texture_width;
	if (nh>max_texture_height) nh = max_texture_height;
	
	w = nw;
	h = nh;
}

void OMediaDXRenderTarget::prepare_context(OMediaWindow *awindow)
{
	bool	rebuild_me = false;
	HRESULT	hr;
	DDSURFACEDESC2 ddsd;

	if (!awindow) return;

	omt_RTGDefineLocalTypeObj(OMediaWinRtgWindow,win_rtg,awindow);

	if (IsIconic(win_rtg->hwnd) || !IsWindowVisible(win_rtg->hwnd))
	{
		purge();
		return;
	}

	if (d3d_device)
	{
		if (buffer_width != awindow->get_width() ||
			buffer_height != awindow->get_height()) rebuild_me = true;
	}
	else rebuild_me = true;

	if (rebuild_me)
	{
		purge();

		window = awindow;
		buffer_width = window->get_width();
		buffer_height = window->get_height();
		
		zbuffer_flags = ((OMediaDXRenderer*)get_renderer())->get_zbuffer_bitdepth();


		// Create back buffer

		if (!page_flipping)
		{
			ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
			ddsd.dwSize         = sizeof(DDSURFACEDESC2);
			ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE; 
			ddsd.dwWidth		= buffer_width;
			ddsd.dwHeight		= buffer_height;
			d3d_back_buffer		= NULL;
 
			hr = dx_draw->CreateSurface( &ddsd, &d3d_back_buffer, NULL );
			if( FAILED( hr ) ) omd_OSEXCEPTION(hr);
		}

		// Create window clipper

	    LPDIRECTDRAWCLIPPER pcClipper;
		hr = dx_draw->CreateClipper( 0, &pcClipper, NULL );
		if( FAILED( hr ) ) omd_OSEXCEPTION(hr);
		pcClipper->SetHWnd( 0, win_rtg->hwnd);
		dx_primary->SetClipper( pcClipper );
		pcClipper->Release();


		// Create ZBuffer if required

		if (d3d_zbuffer_format.dwSize)
		{
			ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
			ddsd.dwSize         = sizeof(DDSURFACEDESC2);
			ddsd.dwFlags        = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
			ddsd.dwWidth        = buffer_width;
			ddsd.dwHeight       = buffer_height;
			ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
			OMediaMemTools::copy(&d3d_zbuffer_format, &ddsd.ddpfPixelFormat, sizeof(DDPIXELFORMAT) ); 

			if( ((OMediaDXRenderer*)get_renderer())->accelerated )
			{
				ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;    
			}
			else
				ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY; 

			hr = dx_draw->CreateSurface( &ddsd, &d3d_zbuffer, NULL ) ;
			if( FAILED( hr ) ) omd_OSEXCEPTION(hr);

		    // Attach the z-buffer to the back buffer.

		    hr = d3d_back_buffer->AddAttachedSurface( d3d_zbuffer );
			if( FAILED( hr ) ) omd_OSEXCEPTION(hr);
		}	



		// Create the device

		hr = d3d->CreateDevice( ((OMediaDXRenderer*)get_renderer())->device_guid, 
									d3d_back_buffer,
								   &d3d_device );

		if( FAILED( hr ) )
		{
			// If this GUID doesn't work, try a software device.
			hr = d3d->CreateDevice( IID_IDirect3DRGBDevice, d3d_back_buffer,
									   &d3d_device );

			if( FAILED( hr ) ) omd_OSEXCEPTION(hr);
		}


		D3DDEVICEDESC7	d3ddesc;
		ZeroMemory( &d3ddesc, sizeof(D3DDEVICEDESC7) );

		d3d_device->GetCaps(&d3ddesc);
		square_texture = 
			((d3ddesc.dpcTriCaps.dwTextureCaps&D3DPTEXTURECAPS_SQUAREONLY )!=0);

		max_texture_units = d3ddesc.wMaxTextureBlendStages;
		if (max_texture_units>omc_DXMaxTexturePass) max_texture_units = omc_DXMaxTexturePass;
		max_texture_width = d3ddesc.dwMaxTextureWidth;
		max_texture_height = d3ddesc.dwMaxTextureHeight;
	}
}

void OMediaDXRenderTarget::flip_buffers(void)
{
	RECT	dest,src;
	HRESULT ddrval; 

	if (!window) return;

	if (!page_flipping)
	{
		for(;;)
		{
			src.left = 0;
			src.top = 0;
			src.right = buffer_width;
			src.bottom = buffer_height;

			dest.left = window->get_x();
			dest.top = window->get_y();
			dest.right = dest.left + buffer_width;
			dest.bottom = dest.top + buffer_height;
 
			ddrval = dx_primary->Blt( &dest, d3d_back_buffer, &src, DDBLT_WAIT, NULL );
			if (ddrval==DD_OK) break;
			else if(ddrval == DDERR_SURFACELOST)
			{
				dx_primary->Restore();
				d3d_back_buffer->Restore();
				if (d3d_zbuffer_format.dwSize) d3d_zbuffer->Restore();
				break;
			}
			else if (ddrval != DDERR_WASSTILLDRAWING) break;
		}
	}
	else
	{
		for(;;)
		{
			ddrval = dx_primary->Flip(NULL, 0); 
			if(ddrval == DD_OK) break;
			else if(ddrval == DDERR_SURFACELOST) 
			{ 
				ddrval = dx_primary->Restore();
				if (d3d_zbuffer_format.dwSize) d3d_zbuffer->Restore();
				break;
			} 
			else if(ddrval != DDERR_WASSTILLDRAWING) break;
		}
	}
}

void OMediaDXRenderTarget::find_zbuffer_format(void)
{
	omt_ZBufferBitDepthFlags	wanted_flags = ((OMediaDXRenderer*)get_renderer())->zbuffer;

	d3d_zbuffer_format.dwSize = 0;

	if (wanted_flags&omfzbc_16Bits) 
	{
		zbuffer_flags = omfzbc_16Bits;
		d3d->EnumZBufferFormats(((OMediaDXRenderer*)get_renderer())->device_guid, 
                                 EnumZBufferCallback, (VOID*)this );

		if (d3d_zbuffer_format.dwSize!=0) return;
	}

	if (wanted_flags&omfzbc_32Bits) 
	{
		zbuffer_flags = omfzbc_32Bits;
		d3d->EnumZBufferFormats(((OMediaDXRenderer*)get_renderer())->device_guid, 
                                 EnumZBufferCallback, (VOID*)this );

		if (d3d_zbuffer_format.dwSize!=0) return;
	}

	if (wanted_flags&omfzbc_24Bits) 
	{
		zbuffer_flags = omfzbc_24Bits;
		d3d->EnumZBufferFormats(((OMediaDXRenderer*)get_renderer())->device_guid, 
                                 EnumZBufferCallback, (VOID*)this );

		if (d3d_zbuffer_format.dwSize!=0) return;
	}

	if (wanted_flags&omfzbc_64Bits) 
	{
		zbuffer_flags = omfzbc_64Bits;
		d3d->EnumZBufferFormats(((OMediaDXRenderer*)get_renderer())->device_guid, 
                                 EnumZBufferCallback, (VOID*)this );

		if (d3d_zbuffer_format.dwSize!=0) return;
	}

	if (wanted_flags&omfzbc_8Bits) 
	{
		zbuffer_flags = omfzbc_8Bits;
		d3d->EnumZBufferFormats(((OMediaDXRenderer*)get_renderer())->device_guid, 
                                 EnumZBufferCallback, (VOID*)this );

		if (d3d_zbuffer_format.dwSize!=0) return;
	}
}

HRESULT WINAPI OMediaDXRenderTarget::EnumZBufferCallback(	DDPIXELFORMAT* pddpf, 
															VOID* user )
{
	OMediaDXRenderTarget	*target = (OMediaDXRenderTarget*)user;

	if( pddpf->dwFlags == DDPF_ZBUFFER )
	{
		short	bits;
		omt_ZBufferBitDepthFlags	zb_bits = target->zbuffer_flags;

		if (zb_bits&omfzbc_16Bits) bits = 16;
		else if (zb_bits&omfzbc_32Bits) bits = 32;
		else if (zb_bits&omfzbc_24Bits) bits = 24;
		else if (zb_bits&omfzbc_64Bits) bits = 64;
		else if (zb_bits&omfzbc_8Bits) bits = 8;
		else bits = 0;

		if (pddpf->dwZBufferBitDepth==bits)
		{
			OMediaMemTools::copy(pddpf,&target->d3d_zbuffer_format,sizeof(DDPIXELFORMAT));
	        return D3DENUMRET_CANCEL; 
		}
	} 

    return D3DENUMRET_OK;
}


//----------------------------------
// Renderer

OMediaDXRenderer::OMediaDXRenderer(OMediaVideoEngine *video,OMediaRendererDef *def, omt_ZBufferBitDepthFlags zbuffer_flags,
								bool depth_supported):
					OMediaRenderer(video,def,zbuffer)
{
	this->depth_supported = depth_supported;
	zbuffer = zbuffer_flags;
	accelerated = ((def->attributes&omcrdattr_Accelerated)!=0 && depth_supported);
	OMediaMemTools::copy(def->private_data,&device_guid,16);					
}

OMediaDXRenderer::~OMediaDXRenderer() 
{
}


OMediaRenderTarget *OMediaDXRenderer::new_target(void)
{
	return new OMediaDXRenderTarget(this);
}


#endif

