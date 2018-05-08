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
#ifdef omd_ENABLE_DIRECTDRAW

#include "OMediaDXVideoEngine.h"
#include "OMediaDXRenderer.h"
#include "OMediaWinRtgWindow.h"
#include "OMediaWindow.h"
#include "OMediaString.h"
#include "OMediaError.h"
#include "OMediaDefMessages.h"
#include "OMediaRenderer.h"
#include "OMediaMemTools.h"



#define COMPILE_MULTIMON_STUBS
#include "multimon.h"    

OMediaDXVideoEngine::OMediaDXVideoEngine(OMediaWindow *master_window) : 
					 OMediaVideoEngine(ommeic_DirectX, master_window)
{
    DynaDirectDrawCreateEx = NULL;
	dx_d3d = NULL;
	dx_primary_surf = NULL;

	win_screenhdc = CreateDC("DISPLAY",omc_NULL,omc_NULL,omc_NULL);
	if (!win_screenhdc) omd_OSEXCEPTION(GetLastError());

	state = 0;
	current_video_mode = default_video_mode = NULL;	
	linked_card = NULL;	

	// Init DirectDraw

    ddhinst = LoadLibrary( "DDRAW.DLL" );
    if( ddhinst == NULL ) omd_STREXCEPTION("Can't load DirectDraw library");

    DynaDirectDrawCreateEx = (DIRECTDRAWCREATEEX)GetProcAddress( ddhinst,
                                                       "DirectDrawCreateEx" );
    if( NULL == DynaDirectDrawCreateEx )
		omd_STREXCEPTION("Can't access DirectDrawCreateEx");

	DynaDirectDrawEnumerateEx = (LPDIRECTDRAWENUMERATEEX) GetProcAddress(ddhinst,"DirectDrawEnumerateExA");

	// Init video cards

	create_video_cards();
}

OMediaDXVideoEngine::~OMediaDXVideoEngine()
{
	unlink();

	FreeLibrary(ddhinst);

	if (win_screenhdc) DeleteDC(win_screenhdc);
}

OMediaVideoMode *OMediaDXVideoEngine::get_current_video_mode(void)
{
	if (!(state&omvesc_Linked)) omd_EXCEPTION(omcerr_NoVideoCardLinked);

	return current_video_mode;
}

void OMediaDXVideoEngine::link(OMediaVideoCard *vcard) 
{
	HRESULT	ddrval;

	if (state&omvesc_Linked)
	{
		if (linked_card==vcard) return;
		unlink();
	}
	
	linked_card = vcard;
	state|=omvesc_Linked;


	// Init default mode

	ddrval = DynaDirectDrawCreateEx(	vcard->private_id==NULL?NULL:(GUID*)vcard->private_data,
									(VOID**)&dx_draw, 
									IID_IDirectDraw7, 
									NULL);
	
	if(ddrval != DD_OK) omd_OSEXCEPTION(ddrval);

	// Set cooperative mode

#ifdef	omd_ENABLE_DIRECTDRAW_EXCLUSIVE_MODE
	if (master_window->get_style()==omwstyle_Fullscreen)
	{
		omt_RTGDefineLocalTypeObj(OMediaWinRtgWindow,winrtg,master_window);	

		ddrval = dx_draw->SetCooperativeLevel(winrtg->hwnd, DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE); 
		exclusive_mode = true;
		page_flipping = true;
	}
	else
#endif
	{
		ddrval = dx_draw->SetCooperativeLevel(NULL, DDSCL_NORMAL); 
		exclusive_mode = false;
		page_flipping = false;
	}

	current_video_mode = default_video_mode = find_current_vmode();	

	init_primary_surfaces();
	update_renderer_defs();

	broadcast_message(omsg_VideoEngineLinked,this);
}

void OMediaDXVideoEngine::unlink(void) 
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

	free_primary_surfaces();
	dx_draw->Release();
}

void OMediaDXVideoEngine::free_primary_surfaces(void)
{
	if (dx_d3d) dx_d3d->Release();
	if (dx_primary_surf) dx_primary_surf->Release();

	dx_d3d = NULL;
	dx_primary_surf = NULL;
}

void OMediaDXVideoEngine::init_primary_surfaces(void)
{

	DDSURFACEDESC2	ddsd;
	DDSCAPS2		ddscaps;
	HRESULT			ddrval;

	// Page flipping

	if (page_flipping)
	{
	    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
	    ddsd.dwSize         = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags        = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.dwBackBufferCount = 1;

		ddrval = dx_draw->CreateSurface( &ddsd, &dx_primary_surf, NULL );
  		if(ddrval != DD_OK) page_flipping = false;
		else
		{
		    ZeroMemory( &ddscaps, sizeof(DDSCAPS2) );
			ddscaps.dwCaps = DDSCAPS_BACKBUFFER; 
			ddrval = dx_primary_surf->GetAttachedSurface(&ddscaps, &dx_back_page_surf); 
			if(ddrval != DD_OK) 
			{
				dx_draw->Release();
				page_flipping = false;
			}
		}
	}

	// Page copy

	if (!page_flipping)
	{    

	    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
	    ddsd.dwSize         = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags        = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		ddrval = dx_draw->CreateSurface( &ddsd, &dx_primary_surf, NULL );
	  	if(ddrval != DD_OK) omd_OSEXCEPTION(ddrval);
	}


#ifdef omd_ENABLE_DIRECT3D
	// Query DirectDraw for access to Direct3D
    ddrval = dx_draw->QueryInterface( IID_IDirect3D7, (VOID**)&dx_d3d );
  	if(ddrval != DD_OK) omd_OSEXCEPTION(ddrval);
#endif

}

void OMediaDXVideoEngine::get_bounds(OMediaRect &rect) const
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

void OMediaDXVideoEngine::set_mode(OMediaVideoMode *vmode)
{
	HRESULT	res;

	if (!(state&omvesc_Linked)) omd_EXCEPTION(omcerr_NoVideoCardLinked);
	if (master_window->get_style()!=omwstyle_Fullscreen) omd_EXCEPTION(omcerr_CantChangeVideoMode);
	if (current_video_mode==vmode) return;

	deselect_renderer();

	if (!vmode) vmode = default_video_mode;	
	
	broadcast_message(omsg_ScreenModeWillChange,this);


	free_primary_surfaces();

	res = dx_draw->SetDisplayMode(vmode->width,vmode->height,vmode->depth,
									vmode->refresh_rate,0);

	init_primary_surfaces();
	
	current_video_mode = vmode;
	if (current_video_mode!=default_video_mode) state|=omvesc_VideoModeSet;
	else state&=~omvesc_VideoModeSet;

	update_renderer_defs();
	
	broadcast_message(omsg_ScreenModeChanged,this);
}

//-------------------------------------------
// Create video cards and modes

BOOL FAR PASCAL omfcallback_DDEnum(GUID FAR* lpGUID, LPSTR lpDriverDesc,
                                      LPSTR lpDriverName, LPVOID lpContext);

BOOL WINAPI omfcallback_DDEnumEx(GUID FAR *lpGUID,   LPSTR     lpDriverDescription, 
								 LPSTR     lpDriverName,        LPVOID    lpContext,           
								 HMONITOR  hm);


HRESULT CALLBACK omfcallback_DModesEnum(LPDDSURFACEDESC2 pddsd, LPVOID lpContext);

HRESULT CALLBACK omfcallback_DModesEnum(LPDDSURFACEDESC2 pddsd, LPVOID lpContext)
{
	OMediaVideoCard		*driver = (OMediaVideoCard*)lpContext;
	OMediaVideoMode		mode;

	mode.its_card = driver;

	// New mode	

	mode.width = pddsd->dwWidth;
	mode.height = pddsd->dwHeight;
	mode.depth = pddsd->ddpfPixelFormat.dwRGBBitCount;
	mode.refresh_rate = pddsd->dwRefreshRate;
	mode.rgbmask[0] = pddsd->ddpfPixelFormat.dwRBitMask;
	mode.rgbmask[1] = pddsd->ddpfPixelFormat.dwGBitMask;
	mode.rgbmask[2] = pddsd->ddpfPixelFormat.dwBBitMask;
	mode.flags = omcvmf_Null;

	driver->modes.push_back(mode);

	return DDENUMRET_OK;
}

BOOL WINAPI DDEnumCallbackEx(
  GUID FAR *lpGUID,    
  LPSTR     lpDriverDescription, 
  LPSTR     lpDriverName,        
  LPVOID    lpContext,           
  HMONITOR  hm        
);


BOOL WINAPI omfcallback_DDEnumEx(
  GUID FAR *lpGUID,    
  LPSTR     lpDriverDesc, 
  LPSTR     lpDriverName,        
  LPVOID    lpContext,           
  HMONITOR  hm)
{
	OMediaDXVideoEngine		*engine = (OMediaDXVideoEngine	*)lpContext;
	omt_VideoCardList		*video_drivers;
	LPDIRECTDRAW7			lpDD;
    DDCAPS					DriverCaps, HELCaps;
	HRESULT					res;
	OMediaVideoCard			emptydriver,*driver;
 
	video_drivers = engine->get_video_cards();

    if (engine->DynaDirectDrawCreateEx(lpGUID, (VOID**)&lpDD, IID_IDirectDraw7, NULL)!=DD_OK) 
	{
		return DDENUMRET_OK;
	}

    memset(&DriverCaps, 0, sizeof(DDCAPS));
    memset(&HELCaps, 0, sizeof(DDCAPS));
    DriverCaps.dwSize = sizeof(DDCAPS);
    HELCaps.dwSize = sizeof(DDCAPS);

    if (lpDD->GetCaps(&DriverCaps, &HELCaps)!=DD_OK) 
	{
		lpDD->Release();
        return DDENUMRET_OK;
    }

	// Let's build a new driver desc.

	video_drivers->push_back(emptydriver);
	omt_VideoCardList::iterator vdi=video_drivers->end();
	vdi--;
	driver = &(*vdi);

	driver->name		= lpDriverDesc;

	if (!hm)
	{
		driver->positionx	= 0;
		driver->positiony	= 0;
	}
	else
	{
		MONITORINFO	minfo;
		if (GetMonitorInfo(hm,&minfo))
		{
			driver->positionx	= minfo.rcWork.left;
			driver->positiony	= minfo.rcWork.right;
		}
	}

	if (lpGUID)
	{
		OMediaMemTools::copy(lpGUID,driver->private_data, sizeof(GUID));
		driver->flags = 0;
		driver->private_id = (void*)0xFFFFFFFF;
	}
	else
	{
		OMediaMemTools::zero(driver->private_data, sizeof(GUID));
		driver->flags = omcvdf_MainMonitor;
		driver->private_id = (void*)NULL;
	}

	if (DriverCaps.dwCaps & DDCAPS_3D) driver->flags |= omcvdf_3D;
	if (DriverCaps.dwCaps & DDCAPS_BLT) driver->flags |= omcvdf_Blitter;

	// Get video mode desc.

	res = lpDD->EnumDisplayModes(0 , NULL,driver, omfcallback_DModesEnum);
	
	if(res != DD_OK ) omd_OSEXCEPTION(res);

	lpDD->Release();

    return DDENUMRET_OK;
}

BOOL FAR PASCAL omfcallback_DDEnum(GUID FAR* lpGUID, LPSTR lpDriverDesc,
                                      LPSTR lpDriverName, LPVOID lpContext)
{
	return omfcallback_DDEnumEx(lpGUID,lpDriverDesc,lpDriverName,lpContext,NULL);
}

void OMediaDXVideoEngine::create_video_cards(void)
{
	HRESULT		 res;

	if (DynaDirectDrawEnumerateEx==NULL)
	{
	    res = DirectDrawEnumerate(omfcallback_DDEnum, this);
		if (res != DD_OK) omd_OSEXCEPTION(res);
	}
	else
	{
		res = DynaDirectDrawEnumerateEx(omfcallback_DDEnumEx, this, 
											DDENUM_ATTACHEDSECONDARYDEVICES|
											DDENUM_DETACHEDSECONDARYDEVICES|
											DDENUM_NONDISPLAYDEVICES);
		if (res != DD_OK) omd_OSEXCEPTION(res);
	}
}

//-------------------------------------------

OMediaVideoMode *OMediaDXVideoEngine::find_current_vmode(void)
{
	DDSURFACEDESC2	dxdesc;

	dxdesc.dwSize = sizeof(DDSURFACEDESC2);
	dx_draw->GetDisplayMode(&dxdesc);

	for(omt_VideoModeList::iterator	vi = linked_card->modes.begin();
		vi!=linked_card->modes.end();
		vi++)
	{
		if ((*vi).width==dxdesc.dwWidth && (*vi).height==dxdesc.dwHeight &&
			(*vi).depth==dxdesc.ddpfPixelFormat.dwRGBBitCount &&
			((*vi).refresh_rate == dxdesc.dwRefreshRate || 
			(*vi).refresh_rate==0) ) return &(*vi);
	}

	omd_EXCEPTION(omcerr_CantFindCurrentVMode);
	return NULL;
}

//---------------------------------------------------------------------------
// Renderers

#ifdef omd_ENABLE_DIRECT3D

HRESULT CALLBACK OMediaDXVideoEngine::enum_render_devices(  LPSTR				lpDeviceDescription,
															LPSTR				lpDeviceName,
															LPD3DDEVICEDESC7	desc, 
															LPVOID				lpContext)
{
	OMediaDXVideoEngine	 *me = (OMediaDXVideoEngine*)lpContext;
	omt_RendererDefList	*rndr_list = &me->renderer_list;

	OMediaRendererDef	def;

	def.engine_id = ommeic_DirectX;
	def.private_id = NULL;
	OMediaMemTools::copy(&desc->deviceGUID,def.private_data,16);

	def.name = lpDeviceName;
	def.attributes = omcrdattr_Texture;

	if (desc->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) 
			def.attributes |= omcrdattr_Accelerated;

	if (desc->dwDevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) 
			def.attributes |= omcrdattr_Accelerated|omcrdattr_TLAccelerated;


	if (desc->dpcTriCaps.dwSrcBlendCaps&D3DPBLENDCAPS_INVSRCALPHA &&
		desc->dpcTriCaps.dwDestBlendCaps&D3DPBLENDCAPS_SRCALPHA) def.attributes |= omcrdattr_Blending;
	
	if (desc->dpcTriCaps.dwRasterCaps&(D3DPRASTERCAPS_ANTIALIASEDGES)) def.attributes |= omcrdattr_AntiAliasing; 
	if (desc->dpcTriCaps.dwRasterCaps&(D3DPRASTERCAPS_DITHER)) def.attributes |= omcrdattr_AntiAliasing; 
	if (desc->dpcTriCaps.dwRasterCaps&(D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT)) def.attributes |= omcrdattr_AntiAliasing; 
	if (desc->dpcTriCaps.dwRasterCaps&(D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT)) def.attributes |= omcrdattr_AntiAliasing; 
	if (desc->dwTextureOpCaps&D3DTOP_MODULATE) def.attributes |= omcrdattr_TextureColor;
	if (desc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR ) def.attributes |= omcrdattr_TextureFiltering;
	if (desc->dpcTriCaps.dwTextureFilterCaps & (D3DPTFILTERCAPS_MIPLINEAR|D3DPTFILTERCAPS_LINEARMIPLINEAR|D3DPTFILTERCAPS_MIPNEAREST) ) def.attributes |= omcrdattr_MipMapping;
	if (desc->dpcTriCaps.dwShadeCaps&D3DPSHADECAPS_COLORGOURAUDRGB) def.attributes |= omcrdattr_Gouraud;
	if (desc->dpcTriCaps.dwShadeCaps&(D3DPSHADECAPS_FOGFLAT|D3DPSHADECAPS_FOGGOURAUD)) def.attributes |= omcrdattr_Fog;

	def.zbuffer_depth = 0;
	if (desc->dwDeviceZBufferBitDepth&DDBD_16) def.zbuffer_depth|=omfzbc_16Bits; 
	if (desc->dwDeviceZBufferBitDepth&DDBD_24) def.zbuffer_depth|=omfzbc_24Bits; 
	if (desc->dwDeviceZBufferBitDepth&DDBD_32) def.zbuffer_depth|=omfzbc_32Bits; 
	if (desc->dwDeviceZBufferBitDepth&DDBD_8)  def.zbuffer_depth|=omfzbc_8Bits; 

	def.pixel_format = 0;
	if (desc->dwDeviceRenderBitDepth&DDBD_16) def.pixel_format |=ompixfc_RGB555|ompixfc_RGB565|ompixfc_ARGB1555;
	if (desc->dwDeviceRenderBitDepth&DDBD_24) def.pixel_format |=ompixfc_RGB888;
	if (desc->dwDeviceRenderBitDepth&DDBD_32) def.pixel_format |=ompixfc_ARGB8888|ompixfc_RGB888;

	if ((desc->dwDeviceRenderBitDepth&(DDBD_16|DDBD_24|DDBD_32))!=0)
	{
		rndr_list->push_back(def);
	}


	return D3DENUMRET_OK;
}

#endif


void OMediaDXVideoEngine::update_renderer_defs(void)
{
	OMediaRendererDef	def;

	renderer_list.erase(renderer_list.begin(),renderer_list.end());
	if (!linked_card) return;

#ifdef omd_ENABLE_OMTRENDERER

	def.engine_id = ommeic_OMT;
	def.name = "Open Media Toolkit";
	def.attributes = omcrdattr_Blending|omcrdattr_Texture|omcrdattr_TextureColor|
					 omcrdattr_Fog|omcrdattr_Gouraud;

	def.zbuffer_depth = omfzbc_16Bits;
	def.pixel_format = ompixfc_ARGB8888|ompixfc_RGB888|ompixfc_RGB555|ompixfc_ARGB1555;
	
	renderer_list.push_back(def);

#endif

#ifdef omd_ENABLE_DIRECT3D

	HRESULT	res;

	BOOL fDeviceFound = FALSE; 
	res = dx_d3d->EnumDevices(enum_render_devices, this);  
	if (FAILED(res)) omd_OSEXCEPTION(res);

#endif
}

void OMediaDXVideoEngine::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_RendererDeleted:
		renderer = NULL;
		break;	
	}
}

void OMediaDXVideoEngine::select_renderer(OMediaRendererDef *def, omt_ZBufferBitDepthFlags zbuffer)
{
	deselect_renderer();

	switch(def->engine_id)
	{	
		#ifdef omd_ENABLE_DIRECT3D
		case ommeic_DirectX:
		{
			if (current_video_mode)
			{
				bool	can_render = false;

				switch(current_video_mode->depth)
				{
					case 16:
					if (def->pixel_format&(ompixfc_RGB555|ompixfc_RGB565|
											ompixfc_ARGB1555|ompixfc_ARGB4444) )
					{
						can_render = true;
					}
					break;

					case 24:
					case 32:
					if (def->pixel_format&(ompixfc_RGB888|ompixfc_ARGB8888) )
					{
						can_render = true;
					}
					break;

					default:
					can_render = false;
					break;
				}
				
				renderer = new OMediaDXRenderer(this,def,zbuffer,can_render);
			}
		}
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

void OMediaDXVideoEngine::deselect_renderer(void)
{
	delete renderer;
	renderer = NULL;
	
	broadcast_message(omsg_RendererSelected,this);
}

OMediaOffscreenBuffer *OMediaDXVideoEngine::create_offscreen_buffer(long width, long height, 
															 			omt_OffscreenBufferPixelFormat	pixel_format)
{
	return new OMediaDXOffscreenBuffer(this, width, height, pixel_format);
}

void OMediaDXVideoEngine::flip_page(void)
{
	if (page_flipping)
	{
		HRESULT	ddrval;

		for(;;)
		{
			ddrval = dx_primary_surf->Flip(NULL, 0); 
			if(ddrval == DD_OK) break;
			else if(ddrval == DDERR_SURFACELOST) 
			{ 
				ddrval = dx_primary_surf->Restore();
				break;
			} 
			else if(ddrval != DDERR_WASSTILLDRAWING) break;
		}
	}

}


#endif

