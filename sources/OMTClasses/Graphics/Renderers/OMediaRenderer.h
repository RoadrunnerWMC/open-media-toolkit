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
#ifndef OMEDIA_Renderer_H
#define OMEDIA_Renderer_H

#include "OMediaTypes.h"
#include "OMedia3DPoint.h"
#include "OMediaRGBColor.h"
#include "OMediaBroadcaster.h"
#include "OMediaListener.h"
#include "OMediaRect.h"
#include "OMediaRendererDef.h"
#include "OMediaEngine.h"

#include <list>
 
class OMediaVideoEngine;
class OMediaRenderer;
class OMediaWindow;
class OMediaRenderPort;
class OMediaRenderTarget;
class OMediaPickRequest;
class OMediaCanvas;

typedef list<OMediaRenderPort*> omt_RenderPortList;
typedef list<OMediaRenderTarget*> omt_RenderTargetList;

typedef unsigned short omt_CanvasImplementationFlags;
const omt_CanvasImplementationFlags 	omcif_Image 	= (1<<0);
const omt_CanvasImplementationFlags 	omcif_Texture 	= (1<<1);
const omt_CanvasImplementationFlags 	omcif_Scaled 	= (1<<2);
const omt_CanvasImplementationFlags 	omcif_Remapped 	= (1<<3);


//----------------------------------
// Render port

class OMediaRenderPort : public OMediaBroadcaster
{
	public:
	
	omtshared OMediaRenderPort(OMediaRenderTarget *target);
	omtshared virtual ~OMediaRenderPort();

	inline OMediaRenderTarget *get_target(void) {return target;}

	omtshared virtual void set_bounds(OMediaRect &rect);
	inline void get_bounds(OMediaRect &rect) const {rect = bounds;}

	omtshared virtual void render(void);

	omtshared virtual void capture_frame(OMediaCanvas &canv);

	protected:
	
	OMediaRect						bounds;
	OMediaRenderTarget				*target;
	omt_RenderPortList::iterator	plist_iterator;
};



//----------------------------------
// Render target

class OMediaRenderTarget : public OMediaEngine
{
	public:
	
	OMediaRenderTarget(OMediaRenderer *renderer, omt_EngineID id, OMediaWindow *master_window);
	virtual ~OMediaRenderTarget();
	
	inline OMediaRenderer *get_renderer(void) {return renderer;}

	inline omt_RenderPortList *get_ports(void) {return &ports;}

	omtshared virtual OMediaRenderPort *new_port(void);
	
	omtshared virtual void render(void);

	inline void set_window(OMediaWindow *w) {window = w;}
	inline OMediaWindow *get_window(void) {return window;}

	omtshared virtual void erase_context(void);
	omtshared virtual void update_context(void);

	static omt_RenderPortList		render_only_these_ports;

	protected:

	OMediaRenderTarget(omt_EngineID id);

	OMediaWindow					*window;
	omt_RenderPortList				ports;
	OMediaRenderer					*renderer;
	omt_RenderTargetList::iterator 	tlist_iterator;
};


//----------------------------------
// Renderer

class OMediaRenderer : 	public OMediaBroadcaster,
						public OMediaListener
{
	public:
	
	// * Construction
	 
	omtshared OMediaRenderer(OMediaVideoEngine *video, OMediaRendererDef *def, omt_ZBufferBitDepthFlags zbuffer);
	omtshared virtual ~OMediaRenderer();

	// * Video
	
	inline OMediaVideoEngine *get_video_engine(void) {return video;}
			// NULL for offscreen renderer

	// * Targets

	omtshared virtual OMediaRenderTarget *new_target(void);
	
	inline omt_RenderTargetList *get_targets(void) {return &targets;}


	// * Listen to message
						
	omtshared virtual void listen_to_message(omt_Message msg, void *param =NULL);


	// * Render
	
	omtshared virtual void render(void);


	// * Picking
	
	inline OMediaPickRequest *get_picking_mode(void) {return picking_mode;}


	protected:

	OMediaVideoEngine		*video;	
	omt_RenderTargetList	targets;
	OMediaPickRequest		*picking_mode;
};




#endif

