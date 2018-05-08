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
#ifndef OMEDIA_VideoEngine_H
#define OMEDIA_VideoEngine_H

#include "OMediaTypes.h"
#include "OMediaEngine.h"
#include "OMediaRendererDef.h"
#include "OMediaRect.h"
#include "OMediaOffscreenBuffer.h"


#include <vector>
#include <string>
#include <list>

class OMediaRenderer;
class OMediaOffscreenBuffer;


//---------------------------------------------------
// * Video engine attributes

typedef unsigned short omt_VideoEngineAttr;
const omt_VideoEngineAttr omveaf_CanChangeDepth 		= (1<<0);
const omt_VideoEngineAttr omveaf_CanChangeResolution 	= (1<<1);

//---------------------------------------------------
// * Video card flags

typedef unsigned short omt_VideoCardFlags;
const omt_VideoCardFlags omcvdf_Null					= 0;
const omt_VideoCardFlags omcvdf_3D					= (1<<0);
const omt_VideoCardFlags omcvdf_Blitter				= (1<<1);
const omt_VideoCardFlags omcvdf_MainMonitor			= (1<<2);


//---------------------------------------------------
// * Video mode flags

typedef unsigned short omt_VideoModeFlags;
const omt_VideoModeFlags omcvmf_Null				= 0;


//---------------------------------------------------
// * Mode and card classes

class OMediaVideoCard;

class OMediaVideoMode
{
	public:

	omt_VideoModeFlags	flags;

	short 				width,height,depth;
	unsigned long 		rgbmask[3];
	long				refresh_rate;				// 0 if not available
	
	OMediaVideoCard		*its_card;

	void *private_id[2];							// Internal
};

typedef vector<OMediaVideoMode>		omt_VideoModeList;


class OMediaVideoCard
{
	public:

	string						name;
	omt_VideoCardFlags			flags;
	omt_VideoModeList			modes;

	long						positionx,positiony;	// Monitor position, (0,0) if not available

	void						*private_id; 		// Internal
	char						private_data[16];
};

typedef list<OMediaVideoCard>	omt_VideoCardList;


//---------------------------------------------------
// * State flags

typedef unsigned short omt_VideoEngineStateFlags;
const omt_VideoEngineStateFlags omvesc_Linked 			= (1<<0);	// Engine is linked to a video card.
const omt_VideoEngineStateFlags omvesc_VideoModeSet 	= (1<<1);	// Video mode has been changed.


	// Please note that changing video mode is optional. The video engine is ready as soon as a 
	// video driver has been set.	


//---------------------------------------------------
// * Video engine

class OMediaVideoEngine : public OMediaEngine
{
	public:

	// * Constructor

	omtshared OMediaVideoEngine(omt_EngineID id, OMediaWindow *master);
	omtshared virtual ~OMediaVideoEngine();

	// * State

	inline omt_VideoEngineStateFlags get_state(void) const {return state;}


	// * Video cards and modes

	inline omt_VideoCardList *get_video_cards(void) {return &video_cards;}

	omtshared virtual OMediaVideoMode *get_current_video_mode(void);


	// * Link to card

	omtshared virtual void link(OMediaVideoCard *vcard);	// Link to card
	omtshared virtual void unlink(void);					// Unlinking resets old video mode if it has
															// been changed.

	omtshared virtual void link_quiet(OMediaVideoCard *vcard);		// Same than link, but wait for a set_mode before
																	// Initializing screen context. Only use it when you
																	// change the video mode just after.

	// * Video mode

	omtshared virtual void set_mode(OMediaVideoMode *mode);	// Set video mode. Video engine must be
															// linked to the video card first. The master
															// window must be full screen. The current
															// renderer is deleted when you change the
															// video mode.

	inline void restore_default_mode(void) {set_mode(NULL);}


	// * Bounds

	omtshared virtual void get_bounds(OMediaRect &rect) const;


	// * Renderers

	inline omt_RendererDefList *get_renderer_list(void)	{return &renderer_list;}
															// The renderer list is available
															// only when the engine is linked
															// to a video card.
															
															// The renderer list is rebuilt
															// each time the video mode
															// is changed or the video engine
															// is relinked.
															
															// This is list is read only

	
	omtshared virtual void select_renderer(OMediaRendererDef*,	omt_ZBufferBitDepthFlags zbuffer);
																// No rendering to a window can be
																// done as long as no
																// renderer has been selected.
																//
																// There can be only one
																// active renderer per video
																// engine (unlike offscreen
																// renderers). 
																//
																// Changing the video mode
																// or calling link/unlink method
																// deselects the renderer.
																//
																// Deleting the renderer is
																// the same as calling
																// deselect_renderer.
	
	omtshared virtual void deselect_renderer(void);
	
	inline OMediaRenderer *get_selected_renderer(void) {return renderer;}


	// * Offscreen buffers

	// Offscreen buffers are deleted automatically as soon as the card is unlinked. Of course you can also
	// delete the buffer yourself using the "delete" operator.

	omtshared virtual OMediaOffscreenBuffer *create_offscreen_buffer(long width, long height, 
															 omt_OffscreenBufferPixelFormat	pixel_format);

	inline omt_OffscreenBufferList *get_offscreen_buffers(void) {return &offscreen_buffers;}

	// * Page-flipping

	omtshared virtual void flip_page(void);

	protected:
	
	omtshared virtual void delete_offscreen_buffers(void);
	
	omt_VideoEngineStateFlags		state;
	omt_VideoCardList				video_cards;
	omt_RendererDefList				renderer_list;
	OMediaRenderer					*renderer;
	
	omt_OffscreenBufferList			offscreen_buffers;
};


#endif

