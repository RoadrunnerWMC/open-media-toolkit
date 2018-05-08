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

#include "OMediaWinGLRtgRenderer.h"
#include "OMediaGLRenderer.h"
#include "OMediaVideoEngine.h"
#include "OMediaWinRtgWindow.h"
#include "OMediaWindow.h"
#include "OMediaError.h"
#include "OMediaMemTools.h"

HGLRC	OMediaWinGLRtgRenderTarget::current_context;

//----------------------------------

OMediaWinGLRtgRenderer::OMediaWinGLRtgRenderer() : OMediaRetarget(omcrtg_GLRenderer)
{
	generic = 0;
}


OMediaWinGLRtgRenderer::~OMediaWinGLRtgRenderer()
{
}

//---------

void OMediaGLRenderer::init_retarget(OMediaVideoMode *videomode, OMediaRendererDef *def)
{
	OMediaWinGLRtgRenderer	*rtg;

	retarget = rtg = new OMediaWinGLRtgRenderer;

	rtg->generic = (long)def->private_id==0;
}

//----------------------------------

OMediaWinGLRtgRenderTarget::OMediaWinGLRtgRenderTarget() : 
							OMediaRetarget(omcrtg_GLRenderTarget)
{
	context = NULL;
	window = NULL;
	hdc = NULL;
}

OMediaWinGLRtgRenderTarget::~OMediaWinGLRtgRenderTarget()
{
	if (context)
	{
		glFinish();
		wglDeleteContext (context);	
		if (current_context==context) current_context = NULL;
		if (window) 
		{
			omt_RTGDefineLocalTypeObj(OMediaWinRtgWindow,win_rtg,window);

			ReleaseDC(win_rtg->hwnd,hdc);
			//+++FIXME window->rebuild();
		}
	}
}

//---------

void OMediaGLRenderTarget::init_retarget(void)
{
	retarget = new OMediaWinGLRtgRenderTarget;
}

void OMediaGLRenderTarget::prepare_context(OMediaWindow *awindow)
{
	if (!awindow) return;

	omt_RTGDefineLocalTypeObj(OMediaWinGLRtgRenderTarget,rtg,this);
	omt_RTGDefineLocalTypeObj(OMediaWinGLRtgRenderer,rend_rtg,((OMediaGLRenderer*)get_renderer()));
	omt_RTGDefineLocalTypeObj(OMediaWinRtgWindow,win_rtg,window);

	if (!rtg->context)
	{
		window = awindow;
		
		omt_ZBufferBitDepthFlags zb_bits = ((OMediaGLRenderer*)get_renderer())->get_zbuffer_bitdepth();


		PIXELFORMATDESCRIPTOR	pixformat;

		OMediaMemTools::zero(&pixformat,sizeof(pixformat));

		pixformat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pixformat.nVersion = 1;
		pixformat.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
		pixformat.iPixelType = PFD_TYPE_RGBA;
		pixformat.cColorBits = get_renderer()->
										get_video_engine()->
											get_current_video_mode()->depth;

		pixformat.iLayerType = PFD_MAIN_PLANE;

		if (zb_bits&omfzbc_16Bits) pixformat.cDepthBits = 16;
		else if (zb_bits&omfzbc_32Bits) pixformat.cDepthBits = 32;
		else if (zb_bits&omfzbc_24Bits) pixformat.cDepthBits = 24;
		else if (zb_bits&omfzbc_64Bits) pixformat.cDepthBits = 64;
		else if (zb_bits&omfzbc_8Bits) pixformat.cDepthBits = 8;
		else pixformat.cDepthBits = 0;

		
		rtg->hdc = GetDC(win_rtg->hwnd);
		long pixindex = ChoosePixelFormat(rtg->hdc,&pixformat); 
		if (pixindex==0)
		{
			ReleaseDC(win_rtg->hwnd,rtg->hdc);
			window = NULL;
			omd_EXCEPTION(omcerr_GLCantCreateContext);
		}

		if (!SetPixelFormat(rtg->hdc,pixindex,&pixformat))
		{
			ReleaseDC(win_rtg->hwnd,rtg->hdc);
			window = NULL;
			omd_EXCEPTION(omcerr_GLCantCreateContext);
		}

		rtg->context = wglCreateContext(rtg->hdc);
		if (!rtg->context)
		{
			ReleaseDC(win_rtg->hwnd,rtg->hdc);
			window = NULL;
			omd_EXCEPTION(omcerr_GLCantCreateContext);
		}
	}
	
	if (rtg->current_context!=rtg->context)
	{
		ReleaseDC(win_rtg->hwnd,rtg->hdc);
		rtg->hdc = GetDC(win_rtg->hwnd);

		rtg->current_context = rtg->context;
		wglMakeCurrent(rtg->hdc,rtg->context);
	}
}

void OMediaGLRenderTarget::set_context(void)
{
	omt_RTGDefineLocalTypeObj(OMediaWinGLRtgRenderTarget,rtg,this);
	omt_RTGDefineLocalTypeObj(OMediaWinRtgWindow,win_rtg,window);
  
	if (rtg->current_context!=rtg->context)
	{
		ReleaseDC(win_rtg->hwnd,rtg->hdc);
		rtg->hdc = GetDC(win_rtg->hwnd);

		rtg->current_context = rtg->context;
		if (rtg->context) wglMakeCurrent(rtg->hdc,rtg->context);
	}
}

void OMediaGLRenderTarget::flip_buffers(void)
{
	omt_RTGDefineLocalTypeObj(OMediaWinGLRtgRenderTarget,rtg,this);

	if (rtg->context) SwapBuffers (rtg->hdc);
}

#endif



