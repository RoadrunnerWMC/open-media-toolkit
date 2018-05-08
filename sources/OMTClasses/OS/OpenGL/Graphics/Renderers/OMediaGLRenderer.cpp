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
#include "OMediaGLRenderer.h"
#include "OMediaGLCanvas.h"
#include "OMediaGL3DShape.h"
#include "OMediaRendererInterface.h"
#include "OMediaVideoEngine.h"
#include "OMediaError.h"
#include "OMediaWindow.h"
#include "OMedia3DMaterial.h"
#include "OMedia3DShape.h"
#include "OMediaAnimPeriodical.h"
#include "OMediaViewPort.h"

#ifdef omd_WINDOWS
#include "glext.h"
#define GLH_EXT_SINGLE_FILE
#ifdef GL_NV_vertex_array_range
#undef GL_NV_vertex_array_range
#endif
#include "glh_genext.h"
#endif

#ifndef GL_ARB_multitexture
#ifdef GL_VERSION_1_3
#define glMultiTexCoord2fARB glMultiTexCoord2f
#define GL_TEXTURE0_ARB GL_TEXTURE0
#define GL_TEXTURE1_ARB GL_TEXTURE1
#define glActiveTextureARB glActiveTexture
#define GL_MAX_TEXTURE_UNITS_ARB GL_MAX_TEXTURE_UNITS
#endif
#endif

#define omd_TEXTURE_UV_LIMIT 0.9999f

static GLenum omc_BlendTable[] =
{
	GL_ZERO,
	GL_ONE,
	GL_DST_COLOR,
	GL_SRC_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
	GL_SRC_ALPHA_SATURATE
};

static GLuint oms_filtering_tab[] = 
{
	GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST,
	GL_NEAREST_MIPMAP_LINEAR,GL_LINEAR_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_LINEAR
};

#define omd_SET_MAIN_TPASS										\
	if (((OMediaGLRenderTarget*)target)->max_texture_units>1)	\
	{															\
		if (current_text_arb != 0)								\
		{														\
			current_text_arb = 0;								\
			glActiveTextureARB(GL_TEXTURE0_ARB);				\
		}														\
	}

bool OMediaGLRenderPort::glVP_zerobased = false;

OMediaGLRenderPort::OMediaGLRenderPort(OMediaRenderTarget *target):
					  OMediaRenderPort(target) 
{
	fillmode = omfillmc_Solid;
	shademode = omshademc_Gouraud;
	light_enabled = false;
	override_mat_fillmode = false;
	user_scissor = false;
	blend_enabled = false;
	picking_mode = false;	
	zbuffer_test = omzbtc_Disabled;
	zbuffer_write = omzbwc_Disabled;
	texture_address_mode = omtamc_Wrap;
	texture_color_mode = omtcoc_Modulate;
	lights = NULL;
	nrenderers++;
}

OMediaGLRenderPort::~OMediaGLRenderPort() 
{
	delete [] lights;

	nrenderers--;
	if (nrenderers==0)
	{
		delete [] picking_buffer;
		picking_buffer = NULL;
		picking_buffer_size = 0;
	}
}

void OMediaGLRenderPort::prepare_gl_context(void)
{
	if (!glVP_zerobased)
		glViewport(bounds.left,bounds.top,bounds.get_width(),bounds.get_height());
	else
		glViewport(0,0,bounds.get_width(),bounds.get_height());
}


void OMediaGLRenderPort::render(void)
{
	if (begin_render())
	{
		OMediaRendererInterface	*iface = this; 
		broadcast_message(omsg_RenderPort,iface);
		end_render();
	}
}

void OMediaGLRenderPort::capture_frame(OMediaCanvas &canv)
{
	omt_RGBAPixel	*pix;
        bool		has_video_rect;

	((OMediaGLRenderTarget*)target)->prepare_context(target->get_window());

	canv.lock(omlf_Write);
	pix = canv.get_pixels();


	long ww,wh;
	OMediaRect		videorect,winrect;

	has_video_rect = ((OMediaGLRenderTarget*)target)->get_video_bounds(videorect);
        
        if (target->get_window())
        {
            ww = target->get_window()->get_width();
            wh = target->get_window()->get_height();
    
            winrect.left = target->get_window()->get_x();
            winrect.top = target->get_window()->get_y();
            winrect.right = winrect.left + ww;
            winrect.bottom = winrect.top + wh;
        }
        else has_video_rect = false;

	if (!has_video_rect || winrect.touch(&videorect))
	{
		glPixelStorei(GL_PACK_ROW_LENGTH,0);
		glPixelStorei(GL_PACK_SKIP_PIXELS,0);
		glPixelStorei(GL_PACK_ALIGNMENT,1);

		glReadPixels(bounds.left, wh-(bounds.top+bounds.get_height()),
					 bounds.get_width(),bounds.get_height(),
					 GL_RGBA,
					 GL_UNSIGNED_BYTE,
					 pix);
	}

	canv.unlock();
}


bool OMediaGLRenderPort::begin_render(void)
{
	long 			ww,wh;
	OMediaRect		videorect,winrect;
        bool			has_video_rect;

	has_video_rect = ((OMediaGLRenderTarget*)target)->get_video_bounds(videorect);

        if (target->get_window())
        {
            ww = target->get_window()->get_width();
            wh = target->get_window()->get_height();
    
            winrect.left = target->get_window()->get_x();
            winrect.top = target->get_window()->get_y();
            winrect.right = winrect.left + ww;
            winrect.bottom = winrect.top + wh;
        }
        else has_video_rect = false;

	if (has_video_rect && !winrect.touch(&videorect)) return false;

	prepare_gl_context();

        if (has_video_rect)
        {
            if (bounds.left!=0 		|| 
                    bounds.top!=0 		||
                    bounds.right!= ww	||
                    bounds.bottom!= wh)
            {
                    user_scissor = true;
                    scissor_rect.set(bounds.left,wh-(bounds.top+bounds.get_height()),bounds.get_width(),bounds.get_height());
                    glEnable(GL_SCISSOR_TEST);
                    glScissor(scissor_rect.left, scissor_rect.top, scissor_rect.right,scissor_rect.bottom);
            }
        }

	original_use_scissor = user_scissor;
	original_scissor_rect = scissor_rect;
		
	if (!lights) lights = new OMediaGLLight[get_max_lights()];

	texture_address_mode = omtamc_Wrap;
	texture_color_mode = omtcoc_Modulate;

	glEnable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);

	//glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
	
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glCullFace(GL_BACK);
	culling_enabled = true;
	texture = NULL;

	GLint max_text = ((OMediaGLRenderTarget*)target)->max_texture_units;

	if (max_text>1)
	{
		for(GLint i=0; i<max_text;i++)
		{
			glActiveTextureARB(GL_TEXTURE0_ARB+i);
			glDisable(GL_TEXTURE_2D);
		}

		glActiveTextureARB(GL_TEXTURE0_ARB);
	}
	else glDisable(GL_TEXTURE_2D);

	current_texture_pass = NULL;
	current_text_arb = 0;	

	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	
	glDisable(GL_FOG);
		
	return true;
}


void OMediaGLRenderPort::end_render(void)
{
	glFlush();
	
	if (user_scissor)
	{
		user_scissor = false;
		glDisable(GL_SCISSOR_TEST);
	}
}

void OMediaGLRenderPort::enable_faceculling(void)
{
	if (!culling_enabled)
	{
		culling_enabled = true;
		glEnable(GL_CULL_FACE);
	}
}

void OMediaGLRenderPort::disable_faceculling(void)
{
	if (culling_enabled)
	{
		culling_enabled = false;
		glDisable(GL_CULL_FACE);
	}
}


void OMediaGLRenderPort::gl_clear(GLbitfield buf, OMediaRect *src_area)
{
	OMediaRect	area,dest;

	if (src_area)
	{
		area = *src_area;
		area.offset(bounds.left,bounds.top);
		if (!area.find_intersection(&bounds,&dest)) return;
	}
	else if (user_scissor)
	{
		glClear(buf);
		return;
	}
	else
	{
		dest = bounds;
	}
	
	long y1;
        
        if (target->get_window())
            y1 = target->get_window()->get_height()-(dest.top+dest.get_height());
        else
            y1 = bounds.get_height()-(dest.top+dest.get_height());

	if (user_scissor)
	{
		glScissor(dest.left, y1, dest.get_width(),dest.get_height());
		glClear(buf);
		glScissor(scissor_rect.left, scissor_rect.top, scissor_rect.right,scissor_rect.bottom);
	}
	else
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(dest.left, y1, dest.get_width(),dest.get_height());
		glClear(buf);
	 	glDisable(GL_SCISSOR_TEST);
	}
}

//----------------------------------
// Renderer interface

omt_RendererRequirementFlags OMediaGLRenderPort::get_requirement_flags(void)
{
	return 0;
}

void OMediaGLRenderPort::get_view_bounds(OMediaRect &r)
{
	r = bounds;
}

void OMediaGLRenderPort::enable_clipping_rect(bool enable)
{
	if (enable)
	{ 
		glEnable(GL_SCISSOR_TEST);	
		user_scissor = true;
	}
	else
	{
		if (!original_use_scissor) 
		{
			glDisable(GL_SCISSOR_TEST);
			user_scissor = false;
		}
		else
		{
			glEnable(GL_SCISSOR_TEST);	
			user_scissor = true;
			scissor_rect = original_scissor_rect;
			glScissor(scissor_rect.left, scissor_rect.top, scissor_rect.right,scissor_rect.bottom);
		}
	}
}

void OMediaGLRenderPort::set_clipping_rect(const OMediaRect &rect)
{
	scissor_rect = rect;
	glScissor(scissor_rect.left, scissor_rect.top, 
				scissor_rect.right-scissor_rect.left,
				scissor_rect.bottom-scissor_rect.top);
}

void OMediaGLRenderPort::clear_colorbuffer(OMediaFARGBColor &rgb, OMediaRect *src_area)
{
	glClearColor(rgb.red,rgb.green,rgb.blue,rgb.alpha);

	gl_clear(GL_COLOR_BUFFER_BIT, src_area);
}

void OMediaGLRenderPort::clear_zbuffer(OMediaRect *area)
{
	gl_clear(GL_DEPTH_BUFFER_BIT, area);
}

void OMediaGLRenderPort::clear_all_buffers(OMediaFARGBColor &rgb)
{
	glClearColor(rgb.red,rgb.green,rgb.blue,rgb.alpha);
	gl_clear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT, NULL);
}	

void OMediaGLRenderPort::set_zbuffer_write(omt_ZBufferWrite zb)
{
	if (zbuffer_write!=zb)
	{
		glDepthMask((zb==omzbwc_Enabled)?GL_TRUE:GL_FALSE);
		zbuffer_write = zb;
	}
}

void OMediaGLRenderPort::set_zbuffer_test(omt_ZBufferTest zb)
{
	if (zb!=zbuffer_test)
	{
		if (zb==omzbtc_Enabled) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
	
		zbuffer_test = zb;
	}
}

void OMediaGLRenderPort::set_zbuffer_func(omt_ZBufferFunc zb)
{
	GLenum	func;

	switch(zb)
	{
		case omzbfc_Never:
		func = GL_NEVER;
		break;
		
		case omzbfc_Always:
		func = GL_ALWAYS;
		break;
		
		case omzbfc_Less:
		func = GL_LESS;
		break;

		case omzbfc_LEqual:
		func = GL_LEQUAL;
		break;

		case omzbfc_Equal:
		func = GL_EQUAL;
		break;

		case omzbfc_GEqual:
		func = GL_GEQUAL;
		break;

		case omzbfc_Greater:
		func = GL_GREATER;
		break;
		
		case omzbfc_NotEqual:
		func = GL_NOTEQUAL;
		break;
		
		default:
		return;
	}

	glDepthFunc(func);
}
	
	// * Blending
	
void OMediaGLRenderPort::set_blend(omt_Blend blend)
{
	if (blend==omblendc_Disabled) 
	{	
		if (blend_enabled)
		{
			glDisable(GL_BLEND);
			blend_enabled = false;
		}
	}
	else  
	{
		if (!blend_enabled)
		{
			glEnable(GL_BLEND);
			blend_enabled = true;
		}
	}
}

void OMediaGLRenderPort::set_blend_func(omt_BlendFunc src_func, omt_BlendFunc dest_func)
{
	glBlendFunc(omc_BlendTable[(long)src_func],omc_BlendTable[(long)dest_func]);
}
	
	// * Matrix
	
void OMediaGLRenderPort::set_model_view(OMediaMatrix_4x4 &m)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&m.m[0][0]);
}

void OMediaGLRenderPort::set_projection(OMediaMatrix_4x4 &m)
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(&m.m[0][0]);	
}

	// * Light

void OMediaGLRenderPort::enable_lighting(void) 
{
	if (!light_enabled)
	{
		light_enabled = true;
		glEnable(GL_LIGHTING);
	}
}

void OMediaGLRenderPort::disable_lighting(void) 
{
	if (light_enabled)
	{
		light_enabled = false;
		glDisable(GL_LIGHTING);
	}
}

long OMediaGLRenderPort::get_max_lights(void) 
{
	GLint n; 
	glGetIntegerv(GL_MAX_LIGHTS,&n);
	return n;
}

void OMediaGLRenderPort::enable_light(long index) 
{
	glEnable(GL_LIGHT0+index);
}

void OMediaGLRenderPort::disable_light(long index) 
{
	glDisable(GL_LIGHT0+index);
}

void OMediaGLRenderPort::set_light_type(long index, omt_LightType type) 
{
	OMediaGLLight	*l = &lights[index];
	
	if (l->type!=type)
	{
		l->type = type;
	
		switch(l->type)
		{
			case omclt_Directional:	
			l->pos[3] = 0;
			glLightfv(GL_LIGHT0+index,GL_POSITION,l->pos);
			glLightf(GL_LIGHT0+index,GL_SPOT_CUTOFF,180.0f);
			break;

			case omclt_Point:
			l->pos[3] = 1.0f;
			glLightfv(GL_LIGHT0+index,GL_POSITION,l->pos);
			glLightf(GL_LIGHT0+index,GL_SPOT_CUTOFF,180.0f);
			break;

			case omclt_Spot:
			l->pos[3] = 1.0f;
			glLightfv(GL_LIGHT0+index,GL_POSITION,l->pos);
			glLightf(GL_LIGHT0+index,GL_SPOT_CUTOFF,l->spot_cuttoff);
			break;
		}
	}
}

void OMediaGLRenderPort::start_light_edit(void)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void OMediaGLRenderPort::end_light_edit(void)
{
	glPopMatrix();
}

void OMediaGLRenderPort::set_light_pos(long index, OMedia3DPoint &p)
{
	OMediaGLLight	*l = &lights[index];
	l->pos[0] = p.x;
	l->pos[1] = p.y;
	l->pos[2] = p.z;
	glLightfv(GL_LIGHT0+index,GL_POSITION,l->pos);
}

void OMediaGLRenderPort::set_light_dir(long index, OMedia3DVector &v)
{
	OMediaGLLight	*l = &lights[index];

	if (l->type==omclt_Directional)
	{
		l->pos[0] = -v.x;
		l->pos[1] = -v.y;
		l->pos[2] = -v.z;
		glLightfv(GL_LIGHT0+index,GL_POSITION,l->pos);
	}
	else
	{
		l->dir[0] = -v.x;
		l->dir[1] = -v.y;
		l->dir[2] = -v.z;
		glLightfv(GL_LIGHT0+index,GL_SPOT_DIRECTION,l->dir);
	}
}

void OMediaGLRenderPort::set_light_ambient(long index, OMediaFARGBColor &argb) 
{
	float	rgba[4];
	rgba[0] = argb.red;	rgba[1] = argb.green;	rgba[2] = argb.blue;	rgba[3] = argb.alpha;
	glLightfv(GL_LIGHT0+index,GL_AMBIENT,rgba);
}

void OMediaGLRenderPort::set_light_diffuse(long index, OMediaFARGBColor &argb)
{
	float	rgba[4];
	rgba[0] = argb.red;	rgba[1] = argb.green;	rgba[2] = argb.blue;	rgba[3] = argb.alpha;
	glLightfv(GL_LIGHT0+index,GL_DIFFUSE,rgba);
}

void OMediaGLRenderPort::set_light_specular(long index, OMediaFARGBColor &argb)
{
	float	rgba[4];
	rgba[0] = argb.red;	rgba[1] = argb.green;	rgba[2] = argb.blue;	rgba[3] = argb.alpha;
	glLightfv(GL_LIGHT0+index,GL_SPECULAR,rgba);
}

void OMediaGLRenderPort::set_light_attenuation(long index, float range, float constant, float linear, float quadratic)
{
	GLenum	light_index = GL_LIGHT0+index;
	
	range = 1.0f/range;	

	glLightf(light_index,GL_CONSTANT_ATTENUATION,constant);
	glLightf(light_index,GL_LINEAR_ATTENUATION,linear*range);
	glLightf(light_index,GL_QUADRATIC_ATTENUATION,quadratic*range);
}

void OMediaGLRenderPort::set_light_spot_cutoff(long index, float cutoff) 
{
	OMediaGLLight	*l = &lights[index];
	l->spot_cuttoff = cutoff;
	glLightf(GL_LIGHT0+index,GL_SPOT_CUTOFF,cutoff);
}

void OMediaGLRenderPort::set_light_spot_exponent(long index, float expo) 
{
	glLightf(GL_LIGHT0+index,GL_SPOT_EXPONENT,expo);
}

void OMediaGLRenderPort::set_light_global_ambient(OMediaFARGBColor &argb) 
{
	float	rgba[4];
	rgba[0] = argb.red;	rgba[1] = argb.green;	rgba[2] = argb.blue;	rgba[3] = argb.alpha;

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,rgba);
}

	// * Material

void OMediaGLRenderPort::set_material(OMediaFARGBColor &emission,
										OMediaFARGBColor &diffuse,
										OMediaFARGBColor &specular,
										OMediaFARGBColor &ambient,
										float			 shininess)
{
	float	rgba[4];

	rgba[0] = emission.red;	rgba[1] = emission.green;	rgba[2] = emission.blue;	rgba[3] = emission.alpha;
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,rgba);

	rgba[0] = diffuse.red;	rgba[1] = diffuse.green;	rgba[2] = diffuse.blue;		rgba[3] = diffuse.alpha;
	glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,rgba);

	rgba[0] = specular.red;	rgba[1] = specular.green;	rgba[2] = specular.blue;	rgba[3] = specular.alpha;
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,rgba);

	rgba[0] = ambient.red;	rgba[1] = ambient.green;	rgba[2] = ambient.blue;		rgba[3] = ambient.alpha;
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,rgba);

	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shininess);
}

	// * Modes

void OMediaGLRenderPort::set_fill_mode(omt_FillMode mode) 
{
	fillmode = mode;
}

void OMediaGLRenderPort::set_override_material_fill_mode(bool mfmode)
{
	override_mat_fillmode = mfmode;
}

void OMediaGLRenderPort::set_shade_mode(omt_ShadeMode mode) 
{
	shademode = mode;
	glShadeModel((shademode==omshademc_Flat)?GL_FLAT:GL_SMOOTH);
}

	// * Texture

void OMediaGLRenderPort::set_texture(OMediaCanvas *canvas)
{
	omd_SET_MAIN_TPASS;

	if (canvas)
	{
		OMediaGLCanvas		*glcanv;
		OMediaRenderTarget	*rtarget = target;

		glEnable(GL_TEXTURE_2D);
		glcanv = (OMediaGLCanvas*)canvas->find_implementation(target, omcskf_Scaled|omcskf_Exact, 0, true);
		
		if (!glcanv) glcanv = new OMediaGLCanvas((OMediaGLRenderTarget*)rtarget,canvas,omcskf_Scaled|omcskf_Exact);
		else glcanv->render_prepare();
		
		glBindTexture(GL_TEXTURE_2D, glcanv->texture_id);
		texture = canvas;

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, oms_filtering_tab[(long)canvas->get_filtering_mag()]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, oms_filtering_tab[(long)canvas->get_filtering_min()]);

		if (texture_address_mode == omtamc_Wrap)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		}
	}
	else
	{
		glDisable(GL_TEXTURE_2D);
		texture = NULL;
	}
}

void OMediaGLRenderPort::set_texture_address_mode(const omt_TextureAddressMode am)
{
	omd_SET_MAIN_TPASS;

	if (am!=texture_address_mode)
	{
		texture_address_mode = am;
		if (am==omtamc_Wrap)
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
		}
	}
}

void OMediaGLRenderPort::set_texture_color_operation(const omt_TextureColorOperation cm)
{
	omd_SET_MAIN_TPASS;

	if (cm!=texture_color_mode)
	{
		texture_color_mode = cm;
		if (cm==omtcoc_Modulate)
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
		else
		{
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		}
	}
}

void OMediaGLRenderPort::set_extra_texture_passes(omt_ExtraTexturePassList *pass_list)
{
	GLint max_text = ((OMediaGLRenderTarget*)target)->max_texture_units;
	if (max_text<=1) return;

	if (!pass_list)
	{
		if (current_texture_pass)
		{
			GLint	i;

			current_texture_pass = NULL;
			for(i=1;i<max_text;i++)
			{
				glActiveTextureARB(GL_TEXTURE0_ARB+i);
				glDisable(GL_TEXTURE_2D);
			}

			glActiveTextureARB(GL_TEXTURE0_ARB);
			current_text_arb = 0;
		}
		return;
	}
	else
	{
		OMediaCanvas		*canvas;
		OMediaGLCanvas		*glcanv;
		OMediaRenderTarget	*rtarget = target;

		if (pass_list!=current_texture_pass)
		{
			current_texture_pass=pass_list;

			long	p;
			omt_ExtraTexturePassList::iterator ei; 

			for(p=1,ei=current_texture_pass->begin();
				p<max_text;
				p++)
			{
				glActiveTextureARB(GL_TEXTURE0_ARB+p);

				if (ei==current_texture_pass->end())
				{
					glDisable(GL_TEXTURE_2D);
					continue;
				}

				canvas = (*ei).get_texture();
				if (!canvas)
				{
					glDisable(GL_TEXTURE_2D);
					ei = current_texture_pass->end();
					continue;
				}

				// Texture

				glEnable(GL_TEXTURE_2D);
				glcanv = (OMediaGLCanvas*)canvas->find_implementation(target, omcskf_Scaled|omcskf_Exact, 0, true);
				
				if (!glcanv) glcanv = new OMediaGLCanvas((OMediaGLRenderTarget*)rtarget,canvas,omcskf_Scaled|omcskf_Exact);
				else glcanv->render_prepare();
				
				glBindTexture(GL_TEXTURE_2D, glcanv->texture_id);

				// Filtering

				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, oms_filtering_tab[(long)canvas->get_filtering_mag()]);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, oms_filtering_tab[(long)canvas->get_filtering_min()]);

				// Address mode

				if ((*ei).get_texture_address_mode()==omtamc_Wrap)
				{
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
					glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
				}

				// Color operation

				if ((*ei).get_texture_color_operation()==omtcoc_Modulate)
				{
					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				}
				else
				{
					glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				}

				ei++;
			}

			glActiveTextureARB(GL_TEXTURE0_ARB);
			current_text_arb = 0;
		}
	}
}

long OMediaGLRenderPort::get_max_texture_passes(void)
{
	return ((OMediaGLRenderTarget*)target)->max_texture_units;
}
	

	
	// * Geometry

void OMediaGLRenderPort::draw(omt_RenderVertexList &vertices, omt_RenderDrawMode mode)
{
	GLint max_text = ((OMediaGLRenderTarget*)target)->max_texture_units;

	if (fillmode==omfillmc_Point) mode = omrdmc_Points;
	else if (fillmode==omfillmc_Line && mode>=omrdmc_Triangles) mode = omrdmc_LineLoop;

	switch(mode)
	{
		case omrdmc_Points:
		glBegin(GL_POINTS);
		break;

		case omrdmc_Lines:
		glBegin(GL_LINES);
		break;

		case omrdmc_LineStrip:
		glBegin(GL_LINE_STRIP);
		break;

		case omrdmc_LineLoop:
		glBegin(GL_LINE_LOOP);
		break;

		case omrdmc_Triangles:
		glBegin(GL_TRIANGLES);
		break;

		case omrdmc_TriangleStrip:
		glBegin(GL_TRIANGLE_STRIP);
		break;

		case omrdmc_TriangleFan:
		glBegin(GL_TRIANGLE_FAN);
		break;

		case omrdmc_Polygon:
		glBegin(GL_POLYGON);
		break;
	}

	for(omt_RenderVertexList::iterator i=vertices.begin();
		i!=vertices.end();
		i++)
	{
		float	a,r,g,b;

		a = (*i).diffuse.alpha;
		r = (*i).diffuse.red + (*i).specular.red;
		g = (*i).diffuse.green + (*i).specular.green;
		b = (*i).diffuse.blue + (*i).specular.blue;
		if (r>1.0f) r = 1.0f;
		if (g>1.0f) g = 1.0f;
		if (b>1.0f) b = 1.0f;

		glColor4f(r,g,b,a);
		if (texture) 
		{
			glTexCoord2f((*i).u,  (*i).v);
			if (current_texture_pass)
			{
				short	uvi,maxuvi = (*i).extra_passes_uv.size();
				if (maxuvi>max_text-1) maxuvi = max_text-1;

				for(uvi=0; uvi<maxuvi; uvi++)
				{
					OMediaExtraTexturePassUV	*uvp = &(*i).extra_passes_uv[uvi];
					glMultiTexCoord2fARB(GL_TEXTURE1_ARB+uvi,uvp->u,uvp->v);
				}	
			}
		}

		if (light_enabled) glNormal3f((*i).normal.x,(*i).normal.y,(*i).normal.z);
		glVertex3f((*i).x,(*i).y,(*i).z);
	}

	glEnd();
}

void OMediaGLRenderPort::draw_shape(OMedia3DShape	*shape,
									bool			&inout_second_pass,
									OMediaRendererOverrideVertexList	*ovlist)
{
	GLint max_text = ((OMediaGLRenderTarget*)target)->max_texture_units;
	bool								force_flat;

	shape->lock(omlf_Read);

	omt_PolygonList *polygons = shape->get_polygons();
	omt_VertexList *vlist  = shape->get_vertices(); 
	omt_NormalList *nlist = shape->get_normals();
	omt_ColorList	*clist = shape->get_colors();
	
	if (clist->size()==0) clist = NULL;

	if (ovlist)
	{
		// Note: overrided vertex list can be bigger than the vertex list of the shape
		if (ovlist->vertex_list) vlist = ovlist->vertex_list;
		if (ovlist->normal_list) nlist = ovlist->normal_list;
		if (ovlist->color_list) clist = ovlist->color_list;
	}
	else if ((shape->get_flags()&omshfc_CompileStatic) && !picking_mode && !override_mat_fillmode &&
			!OMediaAnimPeriodical::get_quick_refresh())
	{	
		// Cached shape

		OMediaGL3DShape	*gl_shape;
		gl_shape = (OMediaGL3DShape*)shape->find_implementation(target, 0, 0);
	
		if (!gl_shape) gl_shape = new OMediaGL3DShape(	(OMediaGLRenderTarget*)target, shape);

		short render_pass;
		render_pass = (!inout_second_pass) ? 0:1;
	
		gl_shape->prepare(render_pass,this);

		if (!inout_second_pass)
			inout_second_pass = gl_shape->second_pass_found;
		
		for(omt_GLVBufferList::iterator	dbi=gl_shape->gl_buffers[render_pass].begin();
			dbi!=gl_shape->gl_buffers[render_pass].end();
			dbi++)
		{
			// Material
			draw_shape_prepare_mat((*dbi).material, force_flat, fillmode);

			// Extra texture passes
			set_extra_texture_passes((*dbi).pass_list);
		
			// Culling
			if ((*dbi).two_sided) disable_faceculling();
				else enable_faceculling();

			// Call list
			glCallList((*dbi).display_list_id);				
		}

		shape->unlock();
		return;	
	}	

	
	OMedia3DMaterial					*mat = NULL,*newmat;
	omt_PolygonList::iterator			poly_i;
	omt_3DPolygonVertexList::iterator	i,vend;
	bool								second_pass_found = false;

	for(poly_i=polygons->begin();
		poly_i!=polygons->end();
		poly_i++)
	{
		if (picking_mode) glLoadName(poly_i-polygons->begin());
	
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
	
			omt_FillMode	mat_fill_mode;
			draw_shape_prepare_mat(mat,force_flat,mat_fill_mode);	
			if (!override_mat_fillmode) fillmode = mat_fill_mode;
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
			glDisable(GL_CULL_FACE);
			culling_enabled = false;		
		}
		else if ( !((*poly_i).get_flags()&om3pf_TwoSided) && !culling_enabled)
		{
			glEnable(GL_CULL_FACE);
			culling_enabled = true;		
		}		


	
		switch(fillmode)
		{
			case omfillmc_Point:
			glBegin(GL_POINTS);
			break;
			
			case omfillmc_Line:
			glBegin(GL_LINE_LOOP);
			break;

			case omfillmc_Solid:
			glBegin(GL_POLYGON);
			break;
			
			default:
			continue;
		}
		
		vend = (*poly_i).get_vertices().end();
		bool do_uv = (texture!=NULL);

		switch(mat->get_light_mode())
		{
			case ommlmc_Color:
			glColor4f(mat->get_diffuse_r(),mat->get_diffuse_g(),mat->get_diffuse_b(),mat->get_diffuse_a());

			for(i=(*poly_i).get_vertices().begin();
				i!=vend;
				i++)
			{
				OMedia3DPoint	*p = &(*(vlist->begin() + (*i).vertex_index));		
				if (do_uv) 
				{
					glTexCoord2f((*i).u,  (*i).v);
					if (current_texture_pass)
					{
						short	uvi,maxuvi = (*i).extra_passes_uv.size();
						if (maxuvi>max_text-1) maxuvi = max_text-1;

						for(uvi=0; uvi<maxuvi; uvi++)
						{
							OMediaExtraTexturePassUV	*uvp = &(*i).extra_passes_uv[uvi];
							glMultiTexCoord2fARB(GL_TEXTURE1_ARB+uvi,uvp->u,uvp->v);
						}
					}					
				}

				glVertex3f(p->x,p->y,p->z);
			}
			break;

			case ommlmc_VertexColor:
			for(i=(*poly_i).get_vertices().begin();
				i!= vend;
				i++)
			{
				OMedia3DPoint		*p = &(*(vlist->begin() + (*i).vertex_index));	

				if (clist)
				{
					OMediaFARGBColor	*argb = &(*(clist->begin() + (*i).color_index));
					glColor4f(argb->red,argb->green,argb->blue,argb->alpha);
				}
				else
					glColor4f(1.0f,1.0f,1.0f,1.0f);
				
				if (do_uv) 
				{
					glTexCoord2f((*i).u,  (*i).v);
					if (current_texture_pass)
					{
						short	uvi,maxuvi = (*i).extra_passes_uv.size();
						if (maxuvi>max_text-1) maxuvi = max_text-1;

						for(uvi=0; uvi<maxuvi; uvi++)
						{
							OMediaExtraTexturePassUV	*uvp = &(*i).extra_passes_uv[uvi];
							glMultiTexCoord2fARB(GL_TEXTURE1_ARB+uvi,uvp->u,uvp->v);
						}
					}					
				}
				glVertex3f(p->x,p->y,p->z);
			}
			break;		

			case ommlmc_Light:
			if (shademode==omshademc_Flat || force_flat)
			{					
				glNormal3f((*poly_i).get_normal().x,(*poly_i).get_normal().y,(*poly_i).get_normal().z);
				for(i=(*poly_i).get_vertices().begin();
					i!=vend;
					i++)
				{
					OMedia3DPoint		*p = &(*(vlist->begin() + (*i).vertex_index));	
					if (do_uv) 
					{
						glTexCoord2f((*i).u,  (*i).v);
						if (current_texture_pass)
						{
							short	uvi,maxuvi = (*i).extra_passes_uv.size();
							if (maxuvi>max_text-1) maxuvi = max_text-1;

							for(uvi=0; uvi<maxuvi; uvi++)
							{
								OMediaExtraTexturePassUV	*uvp = &(*i).extra_passes_uv[uvi];
								glMultiTexCoord2fARB(GL_TEXTURE1_ARB+uvi,uvp->u,uvp->v);
							}
						}					
					}
					glVertex3f(p->x,p->y,p->z);
				}
			}
			else
			{
				for(i=(*poly_i).get_vertices().begin();
					i!=vend;
					i++)
				{
					OMedia3DPoint		*p = &(*(vlist->begin() + (*i).vertex_index));
					OMedia3DVector		*n = &(*(nlist->begin() + (*i).normal_index));
	
					glNormal3f(n->x,n->y,n->z);
					if (do_uv) 
					{
						glTexCoord2f((*i).u,  (*i).v);
						if (current_texture_pass)
						{
							short	uvi,maxuvi = (*i).extra_passes_uv.size();
							if (maxuvi>max_text-1) maxuvi = max_text-1;

							for(uvi=0; uvi<maxuvi; uvi++)
							{
								OMediaExtraTexturePassUV	*uvp = &(*i).extra_passes_uv[uvi];
								glMultiTexCoord2fARB(GL_TEXTURE1_ARB+uvi,uvp->u,uvp->v);
							}
						}					
					}
					glVertex3f(p->x,p->y,p->z);
				}
			}
			break;
		}
	
		glEnd();
	}

	inout_second_pass = second_pass_found;

	shape->unlock();	
}

void OMediaGLRenderPort::draw_shape_prepare_mat(OMedia3DMaterial *mat,
												bool &force_flat,
												omt_FillMode	&fillmode)
{
	force_flat = false;
	float								rgba[4];

	if (mat->get_texture())
	{
		if (texture!=mat->get_texture()) 
		{
			texture_address_mode = mat->get_texture_address_mode();
			texture_color_mode = mat->get_texture_color_operation();
			set_texture(mat->get_texture());
		}
		else
		{
			set_texture_address_mode(mat->get_texture_address_mode());
			set_texture_color_operation(mat->get_texture_color_operation());
		}
	}
	else if (texture) set_texture(NULL);

	if (mat->get_blend_src()==omblendfc_One &&
		mat->get_blend_dest()==omblendfc_Zero) 
	{
		if (blend_enabled)
		{
			glDisable(GL_BLEND);
			blend_enabled = false;
		}
	}
	else
	{
		if (!blend_enabled)
		{
			glEnable(GL_BLEND);
			blend_enabled = true;
		}

		glBlendFunc(omc_BlendTable[(long)mat->get_blend_src()],omc_BlendTable[(long)mat->get_blend_dest()]);	
	}
	
	if (mat->get_light_mode()==ommlmc_Light)
	{
		if (!light_enabled)
		{
			light_enabled = true;
			glEnable(GL_LIGHTING);
		}
		
		rgba[0] = mat->get_emission_r();	
		rgba[1] = mat->get_emission_g();		
		rgba[2] = mat->get_emission_b();		
		rgba[3] = mat->get_emission_a();
		glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,rgba);

		rgba[0] = mat->get_diffuse_r();	
		rgba[1] = mat->get_diffuse_g();		
		rgba[2] = mat->get_diffuse_b();		
		rgba[3] = mat->get_diffuse_a();
		glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,rgba);

		rgba[0] = mat->get_specular_r();	
		rgba[1] = mat->get_specular_g();		
		rgba[2] = mat->get_specular_b();		
		rgba[3] = mat->get_specular_a();
		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,rgba);

		rgba[0] = mat->get_ambient_r();	
		rgba[1] = mat->get_ambient_g();		
		rgba[2] = mat->get_ambient_b();		
		rgba[3] = mat->get_ambient_a();
		glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,rgba);

		glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,mat->get_shininess());
	}
	else	
	{
		if (light_enabled)
		{
			light_enabled = false;
			glDisable(GL_LIGHTING);				
		}
	
	}

	fillmode = mat->get_fill_mode();


	//FIXME? MacOS SMOOTHING problem
	if (mat->get_light_mode()==ommlmc_Light) 
	{
		if (shademode!=omshademc_Gouraud)
		{
			shademode = omshademc_Gouraud;
			glShadeModel(GL_SMOOTH);					
		}
		
		if (mat->get_shade_mode()==omshademc_Flat) force_flat = true;
	}	//---
	
	else if (shademode!=mat->get_shade_mode())
	{
		shademode = mat->get_shade_mode();
		glShadeModel((shademode==omshademc_Flat)?GL_FLAT:GL_SMOOTH);
	}
}

void OMediaGLRenderPort::draw_surface(OMediaCanvas *canv, 
										float x, 	float y, 	float z, 
										float width,	float height,
										OMediaFARGBColor	&diffuse)
{
	OMediaGLCanvas				*glcanv;
	omt_CanvasSlaveKeyFlags		kflags;
	OMediaRenderTarget			*rtarget = target;

	if (current_texture_pass) set_extra_texture_passes(NULL);

	omd_SET_MAIN_TPASS;
	glEnable(GL_TEXTURE_2D);

	kflags = omcskf_SubDivided|omcskf_Exact;

	glcanv = (OMediaGLCanvas*) canv->find_implementation(target, kflags, 0, true);
	if (!glcanv) glcanv = new OMediaGLCanvas((OMediaGLRenderTarget*)rtarget,canv,omcskf_SubDivided);
	else glcanv->render_prepare();

	if (glcanv->get_slave_key_long()&omcskf_Exact)
	{
		glBindTexture(GL_TEXTURE_2D, glcanv->texture_id);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, oms_filtering_tab[(long)canv->get_filtering_mag()]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, oms_filtering_tab[(long)canv->get_filtering_min()]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

		glNormal3f(0.0f,0.0f,-1.0f);
		glColor4f(diffuse.red,diffuse.green,diffuse.blue,diffuse.alpha);

		switch(fillmode)
		{
			case omfillmc_Point:
			glBegin(GL_POINTS);
			break;
			
			case omfillmc_Line:
			glBegin(GL_LINE_LOOP);
			break;

			case omfillmc_Solid:
			glBegin(GL_POLYGON);
			break;
		}

		glTexCoord2f(0.0f,   					omd_TEXTURE_UV_LIMIT);		glVertex3f(x,y,z);				
		glTexCoord2f(omd_TEXTURE_UV_LIMIT,  	omd_TEXTURE_UV_LIMIT);		glVertex3f(x+width,y,z);		
		glTexCoord2f(omd_TEXTURE_UV_LIMIT,  	0.0f);						glVertex3f(x+width,y+height,z);	
		glTexCoord2f(0.0f,   	0.0f);										glVertex3f(x,y+height,z);		

		glEnd();
	}
	else
	{
		long		tx,ty;
		GLuint		*textid = glcanv->texturegrid_id;
		float		canv_w = float(canv->get_width()), canv_h = float(canv->get_height());
		float		scalex = width  / canv_w,
					scaley = height / canv_h;

		float		incx = glcanv->subdiv_w * scalex,
					incy = glcanv->subdiv_h * scaley;
		
		float		ix,iy,x2,y2,ih = canv_h * scaley;
		
		float		u2,v2;

		glNormal3f(0.0f,0.0f,-1.0f);
		glColor4f(diffuse.red,diffuse.green,diffuse.blue,diffuse.alpha);

		for(ty =0, iy = 0; ty < glcanv->n_texth; ty++, iy +=incy)
		{
			v2 = (ty==glcanv->n_texth-1)?glcanv->last_v:omd_TEXTURE_UV_LIMIT;
		
			for(tx =0, ix = 0; tx < glcanv->n_textw; tx++,textid++,ix+=incx)
			{			
				u2 = (tx==glcanv->n_textw-1)?glcanv->last_u:omd_TEXTURE_UV_LIMIT;
							
				glBindTexture(GL_TEXTURE_2D, *textid);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, oms_filtering_tab[(long)canv->get_filtering_mag()]);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, oms_filtering_tab[(long)canv->get_filtering_min()]);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

	
				switch(fillmode)
				{
					case omfillmc_Point:
					glBegin(GL_POINTS);
					break;
					
					case omfillmc_Line:
					glBegin(GL_LINE_LOOP);
					break;
		
					case omfillmc_Solid:
					glBegin(GL_POLYGON);
					break;
				}
	
				x2 = ix+incx;	if (x2>width) x2 = width;
				y2 = iy+incy;	if (y2>height) y2 = height;
					
				glTexCoord2f(0.0f,  0.0f);	glVertex3f(x+ix,y+(ih-iy),z);
				glTexCoord2f(0.0f,  v2);	glVertex3f(x+ix,y+(ih-y2),z);
				glTexCoord2f(u2, 	v2);	glVertex3f(x+x2,y+(ih-y2),z);
				glTexCoord2f(u2, 	0.0f);	glVertex3f(x+x2,y+(ih-iy),z);
	
				glEnd();
			}	
		}
	}

	glDisable(GL_TEXTURE_2D);
	texture = NULL;
}

	
void OMediaGLRenderPort::flush_pipeline(void) 
{
	glFlush();
}


// * Picking

void OMediaGLRenderPort::start_picking(unsigned long max_ids)
{
	picking_mode = true;
	
	max_ids=max_ids<<4;

	if (max_ids>(unsigned long)picking_buffer_size)
	{
		delete [] picking_buffer;
		picking_buffer = new GLuint[max_ids];
		picking_buffer_size = max_ids;	
	}
		
	glSelectBuffer(picking_buffer_size,picking_buffer);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

}

vector<OMediaPickHit> *OMediaGLRenderPort::end_picking(void)
{
	GLuint	nhits, *pick,names;
	OMediaPickHit	newhit;
	
	picking_mode = false;

	hit_list.erase(hit_list.begin(),hit_list.end());

	nhits = glRenderMode(GL_RENDER);
	if (nhits==0xFFFFFFFFL) 
	{
		return &hit_list;
	}


	pick = picking_buffer;
	while(nhits--)
	{
		names = *(pick++);

		newhit.minz = ((float) (*(pick++)) / (unsigned long)0x7fffffffL)-1.0f;
		newhit.maxz = ((float) (*(pick++)) / (unsigned long)0x7fffffffL)-1.0f;

		newhit.id = *(pick++);	names--;

		while(names--) pick++;
		
		hit_list.push_back(newhit);
	}
	
	return &hit_list;
}

void OMediaGLRenderPort::set_picking_id(unsigned long id)
{
	glLoadName(id);
}


OMediaPickRequest *OMediaGLRenderPort::get_pick_mode(void)
{
    return ((OMediaGLRenderTarget*)target)->get_pick_mode();
}


GLuint			*OMediaGLRenderPort::picking_buffer;
long			OMediaGLRenderPort::picking_buffer_size;
long			OMediaGLRenderPort::nrenderers;


// * Fog

void OMediaGLRenderPort::enable_fog(bool enable)
{
	if (enable) 
	{
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE,GL_LINEAR);	// Only linear for now
		glHint(GL_FOG_HINT,GL_DONT_CARE);
	}
	else glDisable(GL_FOG);
}

void OMediaGLRenderPort::set_fog_density(float d)
{
	glFogf(GL_FOG_DENSITY,d);	// Not used for linear...
}

void OMediaGLRenderPort::set_fog_color(const OMediaFARGBColor &argb)
{
	GLfloat	fogColor[4];
	
	fogColor[0] = argb.red;
	fogColor[1] = argb.green;
	fogColor[2] = argb.blue;
	fogColor[3] = argb.alpha;

	glFogfv(GL_FOG_COLOR,fogColor);
}

void OMediaGLRenderPort::set_fog_range(float start, float end)
{
	glFogf(GL_FOG_START,start);
	glFogf(GL_FOG_END,end);
}

void OMediaGLRenderPort::enable_texture_persp(bool enabled)
{
	if (!enabled) 	
		glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_FASTEST);
	else 	
		glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
}

//----------------------------------
// Render target

OMediaGLRenderTarget::OMediaGLRenderTarget():OMediaRenderTarget(ommeic_OpenGL)
{
    first_render = true;
    retarget = NULL;
}

OMediaGLRenderTarget::OMediaGLRenderTarget(OMediaRenderer *renderer) :
						OMediaRenderTarget(renderer,ommeic_OpenGL,renderer->get_video_engine()->get_supervisor_window())
{
	init_retarget();
	first_render = true;
}

OMediaGLRenderTarget::~OMediaGLRenderTarget()
{
	if (retarget!=NULL) set_context();
	delete_all_implementations();
	delete retarget;
}
	
OMediaRenderPort *OMediaGLRenderTarget::new_port(void)
{
	return new OMediaGLRenderPort(this);
}

void OMediaGLRenderTarget::prepare_first_render()
{
    if (first_render)
    {
            first_render = false;

#ifdef omd_WINDOWS
            if (InitExtension("GL_ARB_multitexture"))
            {
                glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&max_texture_units);
            }
            else max_texture_units = 1;
#else
            glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&max_texture_units);
#endif 

            if (max_texture_units>(int)omc_GLMaxTexturePass) 
                    max_texture_units = omc_GLMaxTexturePass;
    }
}

void OMediaGLRenderTarget::render(void)
{
	prepare_context(window);
        prepare_first_render();

	OMediaRenderTarget::render();
	if (!get_pick_mode()) flip_buffers();
}

OMediaPickRequest *OMediaGLRenderTarget::get_pick_mode(void)
{
    return get_renderer()->get_picking_mode();
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

void OMediaGLRenderTarget::check_texture_size(long &w, long &h)
{
	short	nw,nh;
	GLint	max_size;

	set_context();

	glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_size);
	//if (max_size>512) max_size = 512;	//+++

	if (w>max_size) w = max_size;
	if (h>max_size) h = max_size;

	nw = getnearestpow(w);
	nh = getnearestpow(h);
	
	if (w!=nw) nw <<=1;
	if (h!=nh) nh <<=1;

	if (nw>max_size) nw = max_size;
	if (nh>max_size) nh = max_size;
	
	w = nw;
	h = nh;
}

bool OMediaGLRenderTarget::get_video_bounds(OMediaRect &videorect)
{
    if (!get_renderer()) return false;
    if (get_renderer()) get_renderer()->get_video_engine()->get_bounds(videorect);    
    return true;
}

#ifndef omd_MACOS
void OMediaGLRenderTarget::erase_context(void) {}	// Used on MacOS
void OMediaGLRenderTarget::update_context(void) {}
#endif


//----------------------------------

OMediaPureGLRenderTarget::OMediaPureGLRenderTarget()
{
    pick_request = NULL;
    gl_port = new OMediaGLRenderPort(this);
}

OMediaPureGLRenderTarget::~OMediaPureGLRenderTarget()
{
    set_context();
    delete_all_implementations();
}
	
void OMediaPureGLRenderTarget::prepare_context(OMediaWindow *window)
{
    set_context();
}
        
OMediaPickRequest *OMediaPureGLRenderTarget::get_pick_mode(void)
{
    return pick_request;
}

void OMediaPureGLRenderTarget::set_pick_mode(OMediaPickRequest *apr)
{
    pick_request = apr;
}

void OMediaPureGLRenderTarget::render_viewport(OMediaViewPort *viewport)
{
    OMediaRect	bounds;

    prepare_first_render();

    viewport->getbounds(&bounds);
    gl_port->set_bounds(bounds);

    set_pick_mode(NULL);
    
    if (gl_port->begin_render())
    {
        viewport->render_viewport(gl_port);
        gl_port->end_render();
    }
}

void OMediaPureGLRenderTarget::pick_viewport(OMediaViewPort *viewport,OMediaPickRequest *pick_mode)
{
    OMediaRect	bounds;

    prepare_first_render();

    viewport->getbounds(&bounds);
    gl_port->set_bounds(bounds);

    set_pick_mode(pick_mode);

    if (gl_port->begin_render())
    {
        viewport->render_viewport(gl_port);
        gl_port->end_render();
    }

    set_pick_mode(NULL);
    
    viewport->analyze_picking_result(*pick_mode);
}

void OMediaPureGLRenderTarget::set_context() {}

void OMediaPureGLRenderTarget::init_retarget(void) {}
void OMediaPureGLRenderTarget::flip_buffers(void) {}


//----------------------------------
// Renderer

OMediaGLRenderer::OMediaGLRenderer(OMediaVideoEngine *video,OMediaRendererDef *def, omt_ZBufferBitDepthFlags zbuffer_flags):
					OMediaRenderer(video,def,zbuffer)
{
	init_retarget(video->get_current_video_mode(), def);

	zbuffer = zbuffer_flags;
}

OMediaGLRenderer::~OMediaGLRenderer() 
{
	delete retarget;
}


OMediaRenderTarget *OMediaGLRenderer::new_target(void)
{
	return new OMediaGLRenderTarget(this);
}


	

#endif

