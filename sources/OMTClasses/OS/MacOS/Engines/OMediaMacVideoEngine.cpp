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
 
#include "OMediaMacVideoEngine.h"
#include "OMediaBuildSwitch.h"
#include "OMediaString.h"
#include "OMediaError.h"
#include "OMediaDefMessages.h"
#include "OMediaRenderer.h"
#include "OMediaWindow.h"
#include "OMediaMacHideMBar.h"
#include "OMediaMacOffscreenBuffer.h"

#ifdef omd_ENABLE_OPENGL
#include "OMediaGLRenderer.h"
#endif

#ifdef omd_ENABLE_OMTRENDERER
#include "OMediaOMTRenderer.h"
#endif



omt_GDHandle2RenderMap	OMediaMacVideoEngine::renderer_map;

OMediaMacVideoEngine::OMediaMacVideoEngine(OMediaWindow *master, omt_EngineID eng_id) : OMediaVideoEngine(eng_id, master)
{
	state = 0;
	current_video_mode = default_video_mode = NULL;	
	linked_card = NULL;	
	full_screen_entered = false;
    long		response;
    OSErr		err;

	display_mng = use_display_manager();
	use_qt = false; //use_quicktime();

	err = Gestalt(gestaltMenuMgrAttr, &response);
	macos_x = ((err == noErr) && (response & gestaltMenuMgrAquaLayoutMask));
	
	create_video_cards();
}

OMediaMacVideoEngine::~OMediaMacVideoEngine()
{
	unlink();
}

CGrafPtr OMediaMacVideoEngine::get_full_screen_port(void)
{
	return NULL;
}

bool OMediaMacVideoEngine::use_quicktime(void)
{
	OSErr	error;
	long 	result = 0;

	error = Gestalt(gestaltQuickTime,&result);
	if (error==noErr)
	{
		if ( (result>>16L)>=0x0250) return true;
	}
	
	return false;
}

bool OMediaMacVideoEngine::use_display_manager(void)
{
	long					value = 0;
	unsigned long			displayMgrVersion;
	long					displayMgrPresent;

	Gestalt(gestaltDisplayMgrAttr,&value);
	Gestalt(gestaltDisplayMgrVers, (long*)&displayMgrVersion);

	displayMgrPresent=value&(1<<gestaltDisplayMgrPresent);
	if (displayMgrPresent && displayMgrVersion >= 0x00020000) return true;

	return false;
}

OMediaVideoMode *OMediaMacVideoEngine::get_current_video_mode(void)
{
	if (!(state&omvesc_Linked)) omd_EXCEPTION(omcerr_NoVideoCardLinked);

	return current_video_mode;
}

void OMediaMacVideoEngine::link(OMediaVideoCard *vcard) 
{
	if (state&omvesc_Linked)
	{
		if (linked_card==vcard) return;
		unlink();
	}
	
	linked_card = vcard;
	state|=omvesc_Linked;

	current_video_mode = default_video_mode = find_current_vmode(linked_card);	

	update_renderer_defs();
	
	if (master_window->get_style()==omwstyle_Fullscreen) enter_fullscreen();
	broadcast_message(omsg_VideoEngineLinked,this);
}

void OMediaMacVideoEngine::link_quiet(OMediaVideoCard *vcard)
{
	if (master_window->get_style()!=omwstyle_Fullscreen)
	{
		link(vcard);
		return;
	}

	if (state&omvesc_Linked)
	{
		if (linked_card==vcard) return;
		unlink();
	}
	
	linked_card = vcard;
	state|=omvesc_Linked;

	current_video_mode = default_video_mode = find_current_vmode(linked_card);	

	update_renderer_defs();
	
	broadcast_message(omsg_VideoEngineLinked,this);
}

void OMediaMacVideoEngine::unlink(void) 
{
	if (!(state&omvesc_Linked)) return;
	if (full_screen_entered) exit_fullscreen();

	broadcast_message(omsg_VideoEngineUnlinked,this);

	delete_offscreen_buffers();

	if (state&omvesc_VideoModeSet) restore_default_mode();
	
	current_video_mode = default_video_mode = NULL;	
	linked_card = NULL;
	state = 0;

	deselect_renderer();
	
	update_renderer_defs();
}

void OMediaMacVideoEngine::enter_fullscreen(void)
{
	if (!full_screen_entered && (state&omvesc_Linked))
	{
		GDHandle	device = (GDHandle)linked_card->private_id;
		OSErr	error;

		if (GetMainDevice() == device)
		{
			full_screen_entered = true;

			if (use_qt)
			{
				qt_restoreState = NULL;
				EnterMovies();
				error = BeginFullScreen(&qt_restoreState,device,
									NULL,NULL,
									NULL,
									NULL,
									fullScreenAllowEvents);
				
				if (error!=noErr)
				{
					ExitMovies();
					use_qt = false;
				}	
			}
			
			if (!use_qt) OMediaMacHideMBar::hide();
		}
	}
}

void OMediaMacVideoEngine::exit_fullscreen(void)
{
	if (full_screen_entered)
	{
		full_screen_entered = false;
	
		if (use_qt)
		{
			EndFullScreen(qt_restoreState, 0);	
			ExitMovies();
		}
		else
		{
			OMediaMacHideMBar::show();
		}
	}
}

void OMediaMacVideoEngine::get_bounds(OMediaRect &rect) const
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

void OMediaMacVideoEngine::set_mode(OMediaVideoMode *vmode)
{
	short old_depth;

	if (!(state&omvesc_Linked)) omd_EXCEPTION(omcerr_NoVideoCardLinked);
	if (master_window->get_style()!=omwstyle_Fullscreen) omd_EXCEPTION(omcerr_CantChangeVideoMode);
	if (current_video_mode==vmode) return;

	deselect_renderer();

	if (!vmode) vmode = default_video_mode;	
	
	broadcast_message(omsg_ScreenModeWillChange,this);
	
	GDHandle	gdhandle = (GDHandle) linked_card->private_id;
	
	old_depth = current_video_mode->depth;
	
	if (display_mng)	// Use display manager
	{
		Handle	displayState;
		OSErr	err;
		unsigned long	tdepth = (unsigned long)vmode->private_id[1];

		if (DMBeginConfigureDisplays (&displayState)==noErr)
		{
			err = DMSetDisplayMode(	gdhandle,
							(unsigned long)vmode->private_id[0],
							&tdepth,
							0L,
							displayState);

			if (err!=noErr) omd_OSEXCEPTION(err);
	
			DMEndConfigureDisplays(displayState);
		}
	}
	else
	{
		// Cannot use display manager...
		SetDepth(gdhandle,vmode->depth,1,1);
	}
	
	current_video_mode = vmode;
	if (current_video_mode!=default_video_mode) state|=omvesc_VideoModeSet;
	else state&=~omvesc_VideoModeSet;

	renderer_map.erase(renderer_map.begin(),renderer_map.end());

	update_renderer_defs();
	
	broadcast_message(omsg_ScreenModeChanged,this);	
}

void OMediaMacVideoEngine::create_video_cards(void)
{
	OMediaVideoCard		empty_driver,*driver;
	OMediaVideoMode		mode;
	GDHandle			gdscan;
	short				monitor=0,i;
	const short			depth_tab_len=6;
	static short		depth_tab[depth_tab_len] = {1,2,4,8,16,32};	
	
	if (display_mng)
	{
		create_video_cards_dm();
	}
	else
	{		
		gdscan = GetDeviceList();
			
		while(gdscan)
		{
			if (TestDeviceAttribute(gdscan,screenDevice|screenActive))
			{
				monitor++;
				video_cards.push_back(empty_driver);
				omt_VideoCardList::iterator vdi=video_cards.end();
				vdi--;
				driver = &(*vdi);
				
				// Modes

				driver->name = "Monitor ";
				driver->name += omd_L2STR(monitor);
				driver->flags = 0;
				driver->positionx = (*(*gdscan)->gdPMap)->bounds.left;
				driver->positiony = (*(*gdscan)->gdPMap)->bounds.top;

				if (gdscan==GetMainDevice()) driver->flags |= omcvdf_MainMonitor; 
				driver->private_id = gdscan;

		
				for(i=0;i<depth_tab_len;i++)
				{
					if (HasDepth(gdscan,depth_tab[i],0,0)) 
					{
						mode.its_card = driver;
						mode.flags = omcvmf_Null;
						mode.width = (*(*gdscan)->gdPMap)->bounds.right - (*(*gdscan)->gdPMap)->bounds.left;
						mode.height = (*(*gdscan)->gdPMap)->bounds.bottom - (*(*gdscan)->gdPMap)->bounds.top;
						
						mode.depth = depth_tab[i];
						mode.refresh_rate = 0;		// Cannot get refresh rate without display manager

						if (mode.depth==16)
						{
							mode.rgbmask[0] = 0x1F<<10;
							mode.rgbmask[1] = 0x1F<<5;
							mode.rgbmask[2] = 0x1F;
						}
						else
						{
							mode.rgbmask[0] = 0xFF<<16;
							mode.rgbmask[1] = 0xFF<<8;
							mode.rgbmask[2] = 0xFF;							
						}
					
						driver->modes.push_back(mode);
					}
			
				}	
			}
			
			gdscan = GetNextDevice(gdscan);
		}
	}
	
	search_accelerated_cards();
}


void OMediaMacVideoEngine::search_accelerated_cards(void)
{
	//if (macos_x) return;		// Not supported on MacOS beta 1

#ifdef omd_ENABLE_OPENGL

	omt_VideoCardList::iterator 	i;
	vector<bool>::iterator			ci;
	static vector<bool>				accel_cache;

	if (accel_cache.size()!=video_cards.size())
	{
		accel_cache.erase(accel_cache.begin(),accel_cache.end());
	
		for(i=video_cards.begin();
			i!=video_cards.end();
			i++)
		{
			GDHandle	gdhandle;
			GLint 		inum, rv;
			AGLRendererInfo info, head_info;
	
			gdhandle = (GDHandle)(*i).private_id;
	
			head_info =  aglQueryRendererInfo(&gdhandle, 1);
			if(!head_info) continue;
			
			info = head_info;
			inum = 0;
			while(info)
			{
				aglDescribeRenderer(info, AGL_ACCELERATED, &rv);
				if (rv) (*i).flags|=omcvdf_3D;
	
				info = aglNextRendererInfo(info);
				inum++;
			}
	
			aglDestroyRendererInfo(head_info);
			
			accel_cache.push_back( ((*i).flags&omcvdf_3D)?true:false);
		}
	}
	else
	{
		for(i=video_cards.begin(),ci=accel_cache.begin();
			i!=video_cards.end();
			i++,ci++)
		{
			if (*ci) (*i).flags|=omcvdf_3D;
		}	
	}

#endif
}


//------ If display manager V2.0 is available, use it!

void OMediaMacVideoEngine::create_video_cards_dm(void)
{
	GDHandle				monitor;
	OMediaVideoCard			driver;
	long					n = 1;

	monitor = DMGetFirstScreenDevice (dmOnlyActiveDisplays);
	
	while(monitor)
	{
		driver.private_id = monitor;
		driver.name = "Monitor ";
		driver.name += omd_L2STR(n);
		driver.flags = 0;
		driver.positionx = (*(*monitor)->gdPMap)->bounds.left;
		driver.positiony = (*(*monitor)->gdPMap)->bounds.top;

		if (monitor==GetMainDevice()) driver.flags |= omcvdf_MainMonitor; 
		
		video_cards.push_back(driver);

		scan_modes_dm(*(--video_cards.end()));
	
		monitor = DMGetNextScreenDevice ( monitor, dmOnlyActiveDisplays );
		n++;
	}
}

void OMediaMacVideoEngine::scan_modes_dm(OMediaVideoCard &driver)
{
	DMListIndexType					theDisplayModeCount;
	DMListType						theDisplayModeList;
	DMDisplayModeListIteratorUPP	myModeIteratorProc = nil;
	OSErr							err;
	long							i;
	DisplayIDType					idtype;

	myModeIteratorProc = NewDMDisplayModeListIteratorUPP(mode_list_iterator_dm);
	if (!myModeIteratorProc) omd_EXCEPTION(omcerr_OutOfMemory);

	err = DMGetDisplayIDByGDevice( (GDHandle)driver.private_id, &idtype, false );
	if (err!=noErr) omd_OSEXCEPTION(err);	

	err =  DMNewDisplayModeList(idtype, 0, 0, &theDisplayModeCount, &theDisplayModeList);
	if (err!=noErr) omd_OSEXCEPTION(err);
		
	for (i=0; i<(long)theDisplayModeCount; i++)
	{
		DMGetIndexedDisplayModeFromList(theDisplayModeList, i, 0,myModeIteratorProc , (void*)&driver);
	}

	DMDisposeList(theDisplayModeList);
	DisposeDMDisplayModeListIteratorUPP(myModeIteratorProc);
}

pascal void OMediaMacVideoEngine::mode_list_iterator_dm(void *userData, DMListIndexType, DMDisplayModeListEntryPtr displaymodeInfo)
{
	unsigned long			depthCount, i;
	OMediaVideoCard			*driver		= (OMediaVideoCard*) userData;
	OMediaVideoMode			newmode;

	newmode.its_card = driver;
	newmode.width = displaymodeInfo->displayModeResolutionInfo->csHorizontalPixels;
	newmode.height = displaymodeInfo->displayModeResolutionInfo->csVerticalLines;
	newmode.refresh_rate = displaymodeInfo->displayModeResolutionInfo->csRefreshRate>>16L;
	newmode.rgbmask[0] = 0x1F<<10;
	newmode.rgbmask[1] = 0x1F<<5;
	newmode.rgbmask[2] = 0x1F;

	depthCount = displaymodeInfo->displayModeDepthBlockInfo->depthBlockCount;
	for (i=0; i < depthCount; i++)
	{		
		newmode.depth = displaymodeInfo->displayModeDepthBlockInfo->depthVPBlock[i].depthVPBlock->vpPixelSize;
		newmode.flags = omcvmf_Null;
		
		newmode.private_id[0] = (void*)displaymodeInfo->displayModeDepthBlockInfo->depthVPBlock[i].depthSwitchInfo->csData;
                
                unsigned long depthVPBlock = displaymodeInfo->displayModeDepthBlockInfo->depthVPBlock[i].depthSwitchInfo->csMode;
                
		newmode.private_id[1] = (void*)depthVPBlock;
 
		driver->modes.push_back(newmode);
	}
}


OMediaVideoMode *OMediaMacVideoEngine::find_current_vmode(OMediaVideoCard	*card)
{
	short			cwidth,cheight,cdepth;
	unsigned long	crefresh_rate;	
	GDHandle		gdhandle = (GDHandle)card->private_id;

	cwidth = (*(*gdhandle)->gdPMap)->bounds.right - (*(*gdhandle)->gdPMap)->bounds.left;
	cheight = (*(*gdhandle)->gdPMap)->bounds.bottom - (*(*gdhandle)->gdPMap)->bounds.top;
	cdepth = (*(*gdhandle)->gdPMap)->pixelSize;
	
	if (!display_mng) crefresh_rate = 0;
	else crefresh_rate = find_refresh_rate_dm((GDHandle)linked_card->private_id);
		
	for(omt_VideoModeList::iterator i = card->modes.begin();
		i!=card->modes.end();
		i++)
	{
		if (cwidth==(*i).width &&
			cheight==(*i).height &&
			cdepth==(*i).depth &&
			(long)crefresh_rate==(*i).refresh_rate) return &(*i);
	}

	omd_EXCEPTION(omcerr_CantFindCurrentVMode);
	return NULL;
}

//---------------------------------------------------------------------------
// Find refresh rate

class OMediaFindRefreshRate
{
	public:
	
	VDSwitchInfoRec		switchinfo;
	unsigned long		refresh_rate;
	
	bool				found;
};

static pascal void omf_DisplayMng_find_refresh_iterator(void *userData, DMListIndexType, DMDisplayModeListEntryPtr displaymodeInfo);


unsigned long OMediaMacVideoEngine::find_refresh_rate_dm(GDHandle	gdevice)
{
	OSErr					err;
	OMediaFindRefreshRate	frr;
	
	frr.found = false;
	frr.refresh_rate = 0;

	err = DMGetDisplayMode(gdevice, &frr.switchinfo);
	if (err!=noErr) omd_OSEXCEPTION(err);

	DMListIndexType					theDisplayModeCount;
	DMListType						theDisplayModeList;
	DMDisplayModeListIteratorUPP	myModeIteratorProc = nil;
	long							i;
	DisplayIDType					idtype;

	myModeIteratorProc = NewDMDisplayModeListIteratorUPP(omf_DisplayMng_find_refresh_iterator);
	if (!myModeIteratorProc) omd_EXCEPTION(omcerr_OutOfMemory);

	err = DMGetDisplayIDByGDevice( gdevice, &idtype, false );
	if (err!=noErr) omd_OSEXCEPTION(err);	

	err =  DMNewDisplayModeList(idtype, 0, 0, &theDisplayModeCount, &theDisplayModeList);
	if (err!=noErr) omd_OSEXCEPTION(err);

	for (i=0; i<(long)theDisplayModeCount; i++)
	{
		DMGetIndexedDisplayModeFromList(theDisplayModeList, i, 0,myModeIteratorProc , (void*)&frr);
		if (frr.found) break;
	}

	DMDisposeList(theDisplayModeList);
	DisposeDMDisplayModeListIteratorUPP(myModeIteratorProc);

	return frr.refresh_rate;
}

static pascal void omf_DisplayMng_find_refresh_iterator(void *userData, DMListIndexType, DMDisplayModeListEntryPtr displaymodeInfo)
{
	long	depthCount;

	OMediaFindRefreshRate *frr = (OMediaFindRefreshRate*)userData;
	
	if (frr->switchinfo.csData==displaymodeInfo->displayModeSwitchInfo->csData)
	{
		frr->refresh_rate = displaymodeInfo->displayModeResolutionInfo->csRefreshRate>>16L;
		frr->found = true; 
		return;
	}
	

	depthCount = displaymodeInfo->displayModeDepthBlockInfo->depthBlockCount;
	for (long i=0; i < depthCount; i++)
	{
		if (frr->switchinfo.csData==displaymodeInfo->displayModeDepthBlockInfo->depthVPBlock[i].depthSwitchInfo->csData)
		{
			frr->refresh_rate = displaymodeInfo->displayModeResolutionInfo->csRefreshRate>>16L;
			frr->found = true;		
		}		
	}
}

//---------------------------------------------------------------------------
// Renderers

void OMediaMacVideoEngine::update_renderer_defs(void)
{
	renderer_list.erase(renderer_list.begin(),renderer_list.end());
	if (!linked_card) return;

	// See if we already built this renderer defs
	
	omt_GDHandle2RenderMap::iterator rmi;
	
	GDHandle 			gdhandle = (GDHandle) linked_card->private_id;
	GDHandleKey			gdhkey;
	
	gdhkey.gdhandle = gdhandle;
	
	rmi = renderer_map.find(gdhkey);
	if (rmi!=renderer_map.end())
	{
		// It's cached, just use the copy	
		renderer_list = (*rmi).second;	
		return;
	}
	
	

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

	// OpenGL renderer


	GLint 				inum, rv,deviceid;
	long				acount=0,scount=0;
	AGLRendererInfo info, head_info;

	head_info =  aglQueryRendererInfo(&gdhandle, 1);
	if(head_info)
	{	
		info = head_info;
		inum = 0;
		while(info)
		{
			aglDescribeRenderer(info, AGL_RENDERER_ID, &deviceid);
		
			def.private_id = (void*)deviceid;
			def.engine_id = ommeic_OpenGL;
			def.attributes=omcrdattr_Blending|omcrdattr_TextureFiltering|omcrdattr_Gouraud|
									omcrdattr_TextureColor|omcrdattr_AntiAliasing|omcrdattr_MipMapping|
									omcrdattr_Fog|omcrdattr_Texture;
		
			aglDescribeRenderer(info, AGL_ACCELERATED, &rv);
			if (rv) def.attributes|=omcrdattr_Accelerated;
	
	
			aglDescribeRenderer(info, AGL_DEPTH_MODES, &rv);
	
			def.zbuffer_depth = 0;
	
			if (rv&AGL_8_BIT) def.zbuffer_depth|=omfzbc_8Bits;
			if (rv&AGL_16_BIT) def.zbuffer_depth|=omfzbc_16Bits;
			if (rv&AGL_24_BIT) def.zbuffer_depth|=omfzbc_24Bits;
			if (rv&AGL_32_BIT) def.zbuffer_depth|=omfzbc_32Bits;
			if (rv&AGL_64_BIT) def.zbuffer_depth|=omfzbc_64Bits;
	
			aglDescribeRenderer(info, AGL_COLOR_MODES, &rv);
	
			def.pixel_format = 0;
	
			if (rv & AGL_RGB555_BIT) def.pixel_format|=ompixfc_RGB555;
			if (rv & AGL_RGB565_BIT) def.pixel_format|=ompixfc_RGB565;
			if (rv & AGL_ARGB1555_BIT) def.pixel_format|=ompixfc_ARGB1555;
			if (rv & AGL_RGB888_BIT) def.pixel_format|=ompixfc_RGB888;
			if (rv & AGL_ARGB8888_BIT) def.pixel_format|=ompixfc_ARGB8888;

			if (def.attributes&omcrdattr_Accelerated)
			{
				acount++;
				def.name="OpenGL Accelerated Renderer";
				if (acount>1)
				{
					def.name += " ";
					def.name += omd_L2STR(acount);
				}
			}
			else
			{
				scount++;
				def.name="OpenGL Software Renderer";
				if (scount>1)
				{
					def.name += " ";
					def.name += omd_L2STR(scount);
				}
			}

			renderer_list.push_back(def);
	
	/*		This is the only way to get the renderer name. However it is really too slow. So I disabled it.
	
			AGLPixelFormat fmt;
			AGLContext ctx;
			GLint attrib[] = {AGL_RENDERER_ID,0, AGL_RGBA, AGL_ALL_RENDERERS,AGL_SINGLE_RENDERER,AGL_NONE };
	
			attrib[1] = deviceid;
	
			fmt = aglChoosePixelFormat(&gdhandle, 1, attrib);
			if (fmt)
			{
				Rect 		winbounds;
				WindowPtr	winptr;

				ctx = aglCreateContext(fmt, NULL);
				aglDestroyPixelFormat(fmt);
				if (ctx)
				{
					winbounds.left 		= (*(*gdhandle)->gdPMap)->bounds.left;
					winbounds.top 		= (*(*gdhandle)->gdPMap)->bounds.top;
					winbounds.right 	= winbounds.left+32;
					winbounds.bottom 	= winbounds.top+32;

					winptr = NewCWindow(NULL, &winbounds,"\p",false,zoomDocProc, 
										(WindowPtr)-1L,
					 					false,
					 					(long)this);
	
			
					if (aglSetDrawable (ctx, GetWindowPort(winptr))) 
					{
						aglSetCurrentContext(ctx);
					
						const GLubyte *byte;
						byte = glGetString (GL_RENDERER);
					
						def.name = (char*)byte;
					
						renderer_list.push_back(def);
					}
	
					aglDestroyContext ( ctx );
					
					DisposeWindow(winptr);
				}
			}*/
	
			info = aglNextRendererInfo(info);
			inum++;
		}
	
		aglDestroyRendererInfo(head_info);
	}
#endif

	// Cache it
		
	renderer_map[gdhkey] = renderer_list;

}

void OMediaMacVideoEngine::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_RendererDeleted:
		renderer = NULL;
		break;	
	}
}

void OMediaMacVideoEngine::select_renderer(OMediaRendererDef *def, omt_ZBufferBitDepthFlags zbuffer)
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
                
                default:
                break;
	}

	broadcast_message(omsg_RendererSelected,this);
}

void OMediaMacVideoEngine::deselect_renderer(void)
{
	delete renderer;
	renderer = NULL;
	
	broadcast_message(omsg_RendererSelected,this);
}

OMediaOffscreenBuffer *OMediaMacVideoEngine::create_offscreen_buffer(	long width, long height, 
															 		omt_OffscreenBufferPixelFormat	pixel_format)
{
	return new OMediaMacOffscreenBuffer(this, width, height, pixel_format);
}


