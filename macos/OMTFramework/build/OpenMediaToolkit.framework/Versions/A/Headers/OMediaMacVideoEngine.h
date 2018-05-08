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
#ifndef OMEDIA_MacVideoEngine_H
#define OMEDIA_MacVideoEngine_H

#include "OMediaVideoEngine.h"

#include <map>

class GDHandleKey
{
	public:

	GDHandle	gdhandle;

	bool operator<(const GDHandleKey &x) const
	{
		return (long)gdhandle < (long)x.gdhandle;
	
	}
};

typedef less<GDHandleKey> omt_LessGDHandleKey;
typedef map<GDHandleKey,omt_RendererDefList,omt_LessGDHandleKey> omt_GDHandle2RenderMap;


//---------------------------------------------------
// * Video engine

class OMediaMacVideoEngine : public OMediaVideoEngine
{
	public:

	// * Constructor

	omtshared OMediaMacVideoEngine(OMediaWindow *master, omt_EngineID id =ommeic_OS);
	omtshared virtual ~OMediaMacVideoEngine();

	omtshared virtual OMediaVideoMode *get_current_video_mode(void);


	// * Link to card

	omtshared virtual void link(OMediaVideoCard *vcard);	// Link to card
	omtshared virtual void link_quiet(OMediaVideoCard *vcard);

	omtshared virtual void unlink(void);					// Unlinking resets old video mode if it has
															// been changed.

	// * Bounds

	omtshared virtual void get_bounds(OMediaRect &rect) const;


	// * Change mode

	omtshared virtual void set_mode(OMediaVideoMode *mode);	// Set video mode. Video engine must be
															// linked to the video card first.


	// * Renderer

	omtshared virtual void select_renderer(OMediaRendererDef*, omt_ZBufferBitDepthFlags zbuffer =omfzbc_16Bits);
	omtshared virtual void deselect_renderer(void);

	// * Listen to message
						
	omtshared virtual void listen_to_message(omt_Message msg, void *param =NULL);


	// * GDHandle
	
	inline GDHandle get_current_card_gdhandle(void) {return (GDHandle)linked_card->private_id;}


	// * Offscreen buffer

	omtshared virtual OMediaOffscreenBuffer *create_offscreen_buffer(	long width, long height, 
															 			omt_OffscreenBufferPixelFormat	pixel_format);

	// * Fullscreen retarget port
	
	virtual CGrafPtr get_full_screen_port(void);

	protected:

	OMediaVideoMode	*default_video_mode;
	OMediaVideoMode *current_video_mode;
	OMediaVideoCard *linked_card;	

	static omt_GDHandle2RenderMap	renderer_map;

	bool			display_mng;


	Ptr 			qt_restoreState;
	bool			full_screen_entered;
	bool			use_qt;
	
	bool			macos_x;

	bool use_display_manager(void);
	bool use_quicktime(void);

	omtshared virtual void enter_fullscreen(void);
	omtshared virtual void exit_fullscreen(void);
	

	omtshared virtual OMediaVideoMode *find_current_vmode(OMediaVideoCard	*card);

	omtshared virtual void search_accelerated_cards(void);
	void create_video_cards_dm(void);

	omtshared virtual void create_video_cards(void);
	void scan_modes_dm(OMediaVideoCard &driver);
	unsigned long find_refresh_rate_dm(GDHandle	gdevice);

	omtshared virtual void update_renderer_defs(void);

	static pascal void mode_list_iterator_dm(void *userData, DMListIndexType, DMDisplayModeListEntryPtr displaymodeInfo);
};



#endif

