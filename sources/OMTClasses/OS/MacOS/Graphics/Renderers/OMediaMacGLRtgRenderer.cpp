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

#include "OMediaMacGLRtgRenderer.h"
#include "OMediaGLRenderer.h"
#include "OMediaVideoEngine.h"
#include "OMediaMacRtgWindow.h"
#include "OMediaWindow.h"
#include "OMediaError.h"

AGLContext	OMediaMacGLRtgRenderTarget::current_context;

//----------------------------------

OMediaMacGLRtgRenderer::OMediaMacGLRtgRenderer() : OMediaRetarget(omcrtg_GLRenderer)
{
	renderer_device = 0;
}


OMediaMacGLRtgRenderer::~OMediaMacGLRtgRenderer()
{
}

//---------

void OMediaGLRenderer::init_retarget(OMediaVideoMode *videomode, OMediaRendererDef *def)
{
	OMediaMacGLRtgRenderer	*rtg;

	retarget = rtg = new OMediaMacGLRtgRenderer;

	rtg->renderer_device = (GLint)def->private_id;
	rtg->gdevice = (GDHandle)videomode->its_card->private_id;
}

//----------------------------------

OMediaMacGLRtgRenderTarget::OMediaMacGLRtgRenderTarget() : OMediaRetarget(omcrtg_GLRenderTarget)
{
	context = NULL;
}

OMediaMacGLRtgRenderTarget::~OMediaMacGLRtgRenderTarget()
{
	if (context)
	{
		glFinish();
		aglDestroyContext (context);	
		if (current_context==context) current_context = NULL;
	}
}

//---------

void OMediaGLRenderTarget::init_retarget(void)
{
	OMediaMacGLRtgRenderer	*rtg;

	retarget = rtg = new OMediaMacGLRtgRenderer;
}

void OMediaGLRenderTarget::erase_context(void)
{
	omt_RTGDefineLocalTypeObj(OMediaMacGLRtgRenderTarget,rtg,this);

	if (rtg->context)
	{
		aglSetCurrentContext(rtg->context);

		delete_all_implementations();

		glFinish();
		aglDestroyContext (rtg->context);	
		if (rtg->current_context==rtg->context) rtg->current_context = NULL;
		rtg->context = NULL;
	}
}

void OMediaGLRenderTarget::update_context(void)
{
	omt_RTGDefineLocalTypeObj(OMediaMacGLRtgRenderTarget,rtg,this);

	if (rtg->context)
	{
		glFinish();
		aglUpdateContext(rtg->context);
	}
}

void OMediaGLRenderTarget::prepare_context(OMediaWindow *window)
{
	if (!window) return;

	omt_RTGDefineLocalTypeObj(OMediaMacGLRtgRenderTarget,rtg,this);

	if (!rtg->context)
	{
		vector<GLint>	attrib;

		omt_ZBufferBitDepthFlags zb_bits = ((OMediaGLRenderer*)get_renderer())->get_zbuffer_bitdepth();

		omt_RTGDefineLocalTypeObj(OMediaMacGLRtgRenderer,rend_rtg,((OMediaGLRenderer*)get_renderer()));
		omt_RTGDefineLocalTypeObj(OMediaMacRtgWindow,win_rtg,window);	
		
		// Renderer
	
		attrib.push_back(AGL_RENDERER_ID);
		attrib.push_back(rend_rtg->renderer_device);
		attrib.push_back(AGL_ALL_RENDERERS);
		attrib.push_back(AGL_SINGLE_RENDERER);
		attrib.push_back(AGL_DOUBLEBUFFER);
		attrib.push_back(AGL_NO_RECOVERY);
	
		// Pixel format
	
		if ((*(*rend_rtg->gdevice )->gdPMap)->pixelSize>16)
		{
			attrib.push_back(AGL_RED_SIZE);
			attrib.push_back(8);
			attrib.push_back(AGL_GREEN_SIZE);
			attrib.push_back(8);
			attrib.push_back(AGL_BLUE_SIZE);
			attrib.push_back(8);
			attrib.push_back(AGL_ALPHA_SIZE);
			attrib.push_back(8);	
		}
		else
		{
			attrib.push_back(AGL_RED_SIZE);
			attrib.push_back(5);
			attrib.push_back(AGL_GREEN_SIZE);
			attrib.push_back(5);
			attrib.push_back(AGL_BLUE_SIZE);
			attrib.push_back(5);
			attrib.push_back(AGL_ALPHA_SIZE);
			attrib.push_back(0);
		}

		attrib.push_back(AGL_DEPTH_SIZE);
		if (zb_bits&omfzbc_16Bits) attrib.push_back(16);
		else if (zb_bits&omfzbc_32Bits) attrib.push_back(32);
		else if (zb_bits&omfzbc_24Bits) attrib.push_back(24);
		else if (zb_bits&omfzbc_64Bits) attrib.push_back(64);
		else if (zb_bits&omfzbc_8Bits) attrib.push_back(8);
		else attrib.push_back(0);

		attrib.push_back(AGL_RGBA);
		//attrib.push_back(AGL_FULLSCREEN);
		attrib.push_back(AGL_NONE);
			
		AGLPixelFormat fmt = aglChoosePixelFormat(&rend_rtg->gdevice, 1, &(*(attrib.begin())));
		if (!fmt) omd_EXCEPTION(omcerr_GLCantChoosePixelFormat);
		
		rtg->context = aglCreateContext(fmt, NULL);
		aglDestroyPixelFormat(fmt);
	
		if (!rtg->context) omd_EXCEPTION(omcerr_GLCantCreateContext);
				
		if (!aglSetDrawable (rtg->context, 
							 
							 (win_rtg->render_port)?win_rtg->render_port:
							 						GetWindowPort(win_rtg->windowptr)
							 
							 )) omd_EXCEPTION(omcerr_GLCantAttachContext);
	}
	
		
	if (rtg->current_context!=rtg->context)
	{
		rtg->current_context = rtg->context;
		aglSetCurrentContext(rtg->context);
	}
}

void OMediaGLRenderTarget::set_context(void)
{
	omt_RTGDefineLocalTypeObj(OMediaMacGLRtgRenderTarget,rtg,this);

	if (rtg->current_context!=rtg->context)
	{
		rtg->current_context = rtg->context;
		aglSetCurrentContext(rtg->context);
	}
}

void OMediaGLRenderTarget::flip_buffers(void)
{
	omt_RTGDefineLocalTypeObj(OMediaMacGLRtgRenderTarget,rtg,this);

	if (rtg->context) aglSwapBuffers (rtg->context);
}

#endif


