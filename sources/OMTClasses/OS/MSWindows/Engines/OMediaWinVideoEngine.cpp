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
 
#include "OMediaWinVideoEngine.h"
#include "OMediaWinOffscreenBuffer.h"
#include "OMediaBuildSwitch.h"
#include "OMediaString.h"
#include "OMediaError.h"
#include "OMediaDefMessages.h"
#include "OMediaRenderer.h"

#ifdef omd_ENABLE_OPENGL
#include "OMediaGLRenderer.h"
#endif

#ifdef omd_ENABLE_OMTRENDERER
#include "OMediaOMTRenderer.h"
#endif


#ifdef omd_ENABLE_OPENGL
#include <gl/gl.h>
#endif

OMediaWinVideoEngine::OMediaWinVideoEngine(OMediaWindow *master_window) : 
						OMediaVideoEngine(ommeic_OS, master_window)
{
	state = 0;
	current_video_mode = default_video_mode = NULL;	
	linked_card = NULL;	

	win_screenhdc = CreateDC("DISPLAY",omc_NULL,omc_NULL,omc_NULL);
	if (!win_screenhdc) omd_OSEXCEPTION(GetLastError());

#ifdef omd_ENABLE_OPENGL
	scan_opengl();
#endif

	create_video_cards();
}

OMediaWinVideoEngine::~OMediaWinVideoEngine()
{
	unlink();

	if (win_screenhdc) DeleteDC(win_screenhdc);
}

#ifdef omd_ENABLE_OPENGL

bool						OMediaWinVideoEngine::opengl_accelerated;
string						OMediaWinVideoEngine::opengl_card_vendor;
string						OMediaWinVideoEngine::opengl_card_renderer;
omt_ZBufferBitDepthFlags	OMediaWinVideoEngine::opengl_card_depth;
omt_PixelFormat				OMediaWinVideoEngine::opengl_card_color;

omt_ZBufferBitDepthFlags	OMediaWinVideoEngine::opengl_generic_depth;
omt_PixelFormat				OMediaWinVideoEngine::opengl_generic_color;


void OMediaWinVideoEngine::scan_opengl(void)
{
    HWND        hWnd;
	PIXELFORMATDESCRIPTOR	pixelformat;
	HDC		hdc;
	HGLRC	glcontext;
	int	n,i;
    WNDCLASS    wc;
    static HINSTANCE hInstance = 0;

    if (!hInstance) 
	{
        hInstance = GetModuleHandle(NULL);
        wc.style         = CS_OWNDC;
        wc.lpfnWndProc   = (WNDPROC)DefWindowProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = "OMT - OpenGL scan";

        if (!RegisterClass(&wc)) return;
    }

    hWnd = CreateWindow("OMT - OpenGL scan", "", 
						WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                        0, 0, 16, 16, NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) return;

	hdc = GetDC(hWnd);

	omt_ZBufferBitDepthFlags	*depthflags;
	omt_PixelFormat				*colorflags;

	opengl_accelerated = false;
	opengl_card_depth = 0;
	opengl_generic_depth = 0;
	opengl_card_color = 0;
	opengl_generic_color = 0;

	i = 0;
	do
	{
		pixelformat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		n = DescribePixelFormat(hdc,i+1,sizeof(PIXELFORMATDESCRIPTOR),&pixelformat); 
		if (n!=0)
		{
			if (!(pixelformat.dwFlags&PFD_GENERIC_FORMAT) ||
				(pixelformat.dwFlags&PFD_GENERIC_ACCELERATED))
			{
				// Accelerated

				if (!opengl_accelerated)	// Set pixel format
				{
					if (SetPixelFormat(hdc, i+1, &pixelformat))
					{
						opengl_accelerated = true;
					}
				}

				depthflags = &opengl_card_depth;
				colorflags = &opengl_card_color;

			}
			else
			{
				// Generic

				depthflags = &opengl_generic_depth;
				colorflags = &opengl_generic_color;
			}

			switch(pixelformat.cDepthBits)
			{
				case 8:
				*depthflags |= omfzbc_8Bits;
				break;

				case 16:
				*depthflags |= omfzbc_16Bits;
				break;

				case 24:
				*depthflags |= omfzbc_24Bits;
				break;

				case 32:
				*depthflags |= omfzbc_32Bits;
				break;

				case 64:
				*depthflags |= omfzbc_64Bits;
				break;
			}

			if (pixelformat.iPixelType==PFD_TYPE_RGBA)
			{
				if (pixelformat.cColorBits==16)
				{
					if ((pixelformat.cRedBits + 
						pixelformat.cGreenBits + 
						pixelformat.cBlueBits)==16) 

						*colorflags |= ompixfc_RGB565;
					else
						*colorflags |= ompixfc_RGB555|ompixfc_ARGB1555;

				}
				else if (pixelformat.cColorBits==24)
				{
					*colorflags |= ompixfc_RGB888;
				}
				else if (pixelformat.cColorBits==32)
				{
					*colorflags |= ompixfc_RGB888|ompixfc_ARGB8888;
				}
			}
		}
		else break;

		i++;
	}
	while(i<n);

	if (opengl_accelerated)
	{
		glcontext = wglCreateContext(hdc);
		wglMakeCurrent(hdc, glcontext);
		const GLubyte *byte = glGetString (GL_RENDERER);
		opengl_card_renderer = (char*)byte;

		const GLubyte *byte2 = glGetString (GL_VENDOR);
		opengl_card_vendor = (char*)byte2;
	
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(glcontext);
	}

    ReleaseDC(hWnd,hdc);
	DestroyWindow(hWnd);
}

#endif



OMediaVideoMode *OMediaWinVideoEngine::get_current_video_mode(void)
{
	if (!(state&omvesc_Linked)) omd_EXCEPTION(omcerr_NoVideoCardLinked);

	return current_video_mode;
}

void OMediaWinVideoEngine::link(OMediaVideoCard *vcard) 
{
	if (state&omvesc_Linked)
	{
		if (linked_card==vcard) return;
		unlink();
	}
	
	linked_card = vcard;
	state|=omvesc_Linked;

	current_video_mode = default_video_mode = find_current_vmode();	

	update_renderer_defs();

	broadcast_message(omsg_VideoEngineLinked,this);
}

void OMediaWinVideoEngine::unlink(void) 
{
	if (!(state&omvesc_Linked)) return;

	broadcast_message(omsg_VideoEngineUnlinked,this);

	delete_offscreen_buffers();

	if (state&omvesc_VideoModeSet) restore_default_mode();
	
	current_video_mode = default_video_mode = NULL;	
	linked_card = NULL;
	state = 0;

	deselect_renderer();
	
	update_renderer_defs();
}

void OMediaWinVideoEngine::get_bounds(OMediaRect &rect) const
{	
	if ((state&omvesc_Linked) && current_video_mode)
	{
		rect.left =  current_video_mode->its_card->positionx;
		rect.top =  current_video_mode->its_card->positiony;

		rect.right =  rect.left + current_video_mode->width;
		rect.bottom =  rect.top + current_video_mode->height;
	}
	else rect.set(0,0,0,0);
}

void OMediaWinVideoEngine::set_mode(OMediaVideoMode *vmode)
{
	if (!(state&omvesc_Linked)) omd_EXCEPTION(omcerr_NoVideoCardLinked);
	if (current_video_mode==vmode) return;

	deselect_renderer();

	if (!vmode) vmode = default_video_mode;	
	
	broadcast_message(omsg_ScreenModeWillChange,this);
	
	current_video_mode = vmode;
	if (current_video_mode!=default_video_mode) state|=omvesc_VideoModeSet;
	else state&=~omvesc_VideoModeSet;

	update_renderer_defs();
	
	broadcast_message(omsg_ScreenModeChanged,this);	
}

void OMediaWinVideoEngine::create_video_cards(void)
{
	OMediaVideoCard		empty_driver,*driver;
	OMediaVideoMode		mode;
	short				monitor=0;

	video_cards.push_back(empty_driver);
	omt_VideoCardList::iterator vdi=video_cards.end();
	vdi--;
	driver = &(*vdi);

	// Modes
 
	driver->name = "MS-Windows GUI";
	driver->flags = omcvdf_MainMonitor;
	driver->positionx = 0;
	driver->positiony = 0;

	driver->private_id = win_screenhdc;

	mode.its_card = driver;
	mode.flags = omcvmf_Null;
	mode.width = GetDeviceCaps(win_screenhdc,HORZRES);
	mode.height = GetDeviceCaps(win_screenhdc,VERTRES);
	mode.depth = GetDeviceCaps(win_screenhdc,BITSPIXEL);
	mode.refresh_rate = 0;

#ifdef omd_ENABLE_OPENGL
	if (opengl_accelerated) mode.flags |= omcvdf_3D;
#endif

	if (mode.depth==16)
	{
		mode.rgbmask[0] = 0xF800;
		mode.rgbmask[1] = 0x07E0;
		mode.rgbmask[2] = 0x001F;
	}
	else if (mode.depth==32)
	{
		mode.rgbmask[0] = 0xFF0000;
		mode.rgbmask[1] = 0x00FF00;
		mode.rgbmask[2] = 0x0000FF;
	}

	driver->modes.push_back(mode);
}

OMediaVideoMode *OMediaWinVideoEngine::find_current_vmode(void)
{
	return &(linked_card->modes.front());
}


//---------------------------------------------------------------------------
// Renderers

void OMediaWinVideoEngine::update_renderer_defs(void)
{
	renderer_list.erase(renderer_list.begin(),renderer_list.end());
	if (!linked_card) return;

	OMediaRendererDef	def;

#ifdef omd_ENABLE_OMTRENDERER

	def.engine_id = ommeic_OMT;
	def.name = "Open Media Toolkit";
	def.attributes = omcrdattr_Blending|omcrdattr_Texture|omcrdattr_TextureColor|
					 omcrdattr_Fog|omcrdattr_Gouraud;

	def.zbuffer_depth = omfzbc_16Bits;
	def.pixel_format = ompixfc_ARGB8888|ompixfc_RGB888|ompixfc_RGB555|ompixfc_ARGB1555;
	
	renderer_list.push_back(def);

#endif

#ifdef omd_ENABLE_OPENGL

	// OpenGL generic renderer

	def.private_id = (void*)0L;
	def.engine_id = ommeic_OpenGL;
	def.attributes=	omcrdattr_Blending|omcrdattr_TextureFiltering|omcrdattr_Gouraud|
					omcrdattr_TextureColor|omcrdattr_AntiAliasing|omcrdattr_MipMapping|
					omcrdattr_Fog|omcrdattr_Texture;

	def.zbuffer_depth = opengl_generic_depth;
	def.pixel_format = opengl_generic_color;
	def.name = "OpenGL Generic Software";
	renderer_list.push_back(def);

	if (opengl_accelerated)
	{
		def.private_id = (void*)1L;		// Accelerated
		def.engine_id = ommeic_OpenGL;
		def.attributes=	omcrdattr_Blending|omcrdattr_TextureFiltering|omcrdattr_Gouraud|
						omcrdattr_TextureColor|omcrdattr_AntiAliasing|omcrdattr_MipMapping|
						omcrdattr_Fog|omcrdattr_Texture|omcrdattr_Accelerated;

		def.zbuffer_depth = opengl_card_depth;
		def.pixel_format = opengl_card_color;
		def.name = opengl_card_vendor;
		def.name += " ";
		def.name += opengl_card_renderer;
		renderer_list.push_back(def);
	}

#endif
}

void OMediaWinVideoEngine::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_RendererDeleted:
		renderer = NULL;
		break;	
	}
}

void OMediaWinVideoEngine::select_renderer(OMediaRendererDef *def, omt_ZBufferBitDepthFlags zbuffer)
{
	deselect_renderer();

	switch(def->engine_id)
	{	
		#ifdef omd_ENABLE_OPENGL
		case ommeic_OpenGL:
		renderer = new OMediaGLRenderer(this,def,zbuffer);
		break;
		#endif

		#ifdef omd_ENABLE_OMTRENDERER
		case ommeic_OMT:
		renderer = new OMediaOMTRenderer(this,def,zbuffer);
		break;
		#endif		
	}

	broadcast_message(omsg_RendererSelected,this);
}

void OMediaWinVideoEngine::deselect_renderer(void)
{
	delete renderer;
	renderer = NULL;
	
	broadcast_message(omsg_RendererSelected,this);
}

OMediaOffscreenBuffer *OMediaWinVideoEngine::create_offscreen_buffer(long width, long height, 
															 			omt_OffscreenBufferPixelFormat	pixel_format)
{
	return new OMediaWinOffscreenBuffer(this, width, height, pixel_format);
}



