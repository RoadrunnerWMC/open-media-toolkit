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
#ifndef OMEDIA_ViewPort_H
#define OMEDIA_ViewPort_H

#include "OMediaWorldUnits.h"
#include "OMediaRect.h"
#include "OMediaWorld.h"
#include "OMediaWorldPosition.h"
#include "OMedia3DPoint.h"
#include "OMediaBroadcaster.h"
#include "OMediaRateManager.h"
#include "OMediaRenderer.h"
#include "OMediaElement.h"
#include "OMediaSupervisor.h"

class OMediaWorld;
class OMediaWindow;
class OMediaRendererInterface;
class OMediaEvent;
class OMediaCanvas;

enum omt_VPBoundsMode
{
	omvpbmc_LeftRelative,
	omvpbmc_RightRelative,

	omvpbmc_TopRelative = omvpbmc_LeftRelative,
	omvpbmc_BottomRelative = omvpbmc_RightRelative
};

enum omt_VPBounds
{
	omcpbc_Left,
	omcpbc_Top,
	omcpbc_Right,
	omcpbc_Bottom
};

typedef unsigned short omt_ViewPortFlags;
const omt_ViewPortFlags	omcvpf_EnableMouseClick = (1<<0);	// Enable mouse clicks. You must
															// set this flag if you want
															// your viewport to handle mouse clicks.

const omt_ViewPortFlags	omcvpf_EnableMouseClick_Up = (1<<1);	// Receives messages when the mouse button is
																// released

const omt_ViewPortFlags	omcvpf_ClickActivateSupervisor = (1<<2);	// When this flag is set, the viewport
																	// becomes the main supervisor when
																	// it receives a mouse click.


class OMediaViewPort : 	public OMediaElement, 
						public OMediaSupervisor
{
	public:
	
	// * Constructor/Destructor
	
	omtshared OMediaViewPort(OMediaSupervisor *super =NULL);
	omtshared virtual ~OMediaViewPort();
	
	// * Bounds
	
	inline void getbounds(OMediaRect *b) const {*b = bounds;}
	omtshared virtual void setbounds(OMediaRect *b);

	inline void get_relative_bounds(OMediaRect *b) const {*b = rel_bounds;}


	
	inline long getbx(void) {return bounds.left;}
	inline long getby(void) {return bounds.top;}
	inline long getbwidth(void) {return bounds.get_width();}
	inline long getbheight(void) {return bounds.get_height();}

	inline void bmove(long x, long y) {OMediaRect r = rel_bounds; r.offset(x,y); setbounds(&r);}
	inline void bplace(long x, long y) {OMediaRect r = rel_bounds; 
										r.offset(x-rel_bounds.left,y-rel_bounds.top); setbounds(&r);}

	inline void set_bounds_mode(omt_VPBounds b, omt_VPBoundsMode m) {bounds_mode[b] = m; setbounds(&rel_bounds);}
	inline omt_VPBoundsMode get_bounds_mode(omt_VPBounds b) const {return bounds_mode[b];}

	// * Output	
	
	omtshared virtual void link_window(OMediaWindow *win);
	omtshared virtual void unlink_window(void);
	
	inline OMediaWindow *get_window(void) {return window;}

	// * Listen to message
						
	omtshared virtual void listen_to_message(omt_Message msg, void *param =NULL);

	// * Rate manager 

	inline OMediaRateManager *get_rate_manager(void) {return &rate_manager;}

	// * Layer key

	inline void set_layer_key(unsigned long k) {layer_key = k;}
	inline unsigned long get_layer_key(void) const {return layer_key;}

		// Layer is drawn by viewport only if (layer->viewport_key&viewport->layer_key)!=0

	// * Render

	omtshared virtual void render_viewport(OMediaRendererInterface *rdr_interface);


	// * Pick

	omtshared virtual void pick(OMediaPickRequest &request);
        omtshared virtual void analyze_picking_result(OMediaPickRequest &request);

	// * Flags

	inline void set_flags(const omt_ViewPortFlags vpf) {flags = vpf;}
	inline omt_ViewPortFlags get_flags(void) const {return flags;}


	// * Capture frame

	omtshared virtual void capture_frame(OMediaCanvas &canvas, bool capture_alpha =false);


	// * Render ports

	inline omt_RenderPortList *get_render_ports(void) {return &render_ports;}


	protected:

	omtshared virtual void check_mouse_activate(OMediaEvent *event);
	omtshared virtual void handle_mouse_hit(OMediaEvent *event,omt_Message msg);

	omtshared virtual void update_render_ports(void);

	omtshared virtual void mouse_track(OMediaMouseTrackPick *mpick);

	OMediaWindow		*window;
	OMediaRateManager	rate_manager;
	unsigned long 		layer_key;
	OMediaRect			bounds,rel_bounds;
	omt_VPBoundsMode	bounds_mode[4];
	OMediaPickRequest	*picking;

	omt_RenderPortList	render_ports;

	omt_ViewPortFlags	flags;
};



#endif

