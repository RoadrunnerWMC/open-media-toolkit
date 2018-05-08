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
#ifndef OMEDIA_WinVideoEngine_H
#define OMEDIA_WinVideoEngine_H

#include "OMediaVideoEngine.h"
#include "OMediaBuildSwitch.h"


//---------------------------------------------------
// * Video engine

class OMediaWinVideoEngine : public OMediaVideoEngine
{
	public:

	// * Constructor

	omtshared OMediaWinVideoEngine(OMediaWindow *master_window);
	omtshared virtual ~OMediaWinVideoEngine();

	omtshared virtual OMediaVideoMode *get_current_video_mode(void);


	// * Link to card

	omtshared virtual void link(OMediaVideoCard *vcard);	// Link to card
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

	// * Offscreen buffer

	omtshared virtual OMediaOffscreenBuffer *create_offscreen_buffer(	long width, long height, 
															 			omt_OffscreenBufferPixelFormat	pixel_format);


	protected:

	OMediaVideoMode	*default_video_mode;
	OMediaVideoMode *current_video_mode;
	OMediaVideoCard *linked_card;	


	OMediaVideoMode *find_current_vmode(void);

	void create_video_cards(void);
	void scan_modes_dm(OMediaVideoCard &driver);
	unsigned long find_refresh_rate_dm(void);

	void update_renderer_defs(void);


	public:

	HDC		win_screenhdc;


#ifdef omd_ENABLE_OPENGL
	void scan_opengl(void);

	static bool opengl_accelerated;
	static string opengl_card_vendor;
	static string opengl_card_renderer;
	static omt_ZBufferBitDepthFlags	opengl_card_depth;
	static omt_PixelFormat	opengl_card_color;

	static omt_ZBufferBitDepthFlags	opengl_generic_depth;
	static omt_PixelFormat	opengl_generic_color;



#endif

};



#endif

