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
#ifndef OMEDIA_World_H
#define OMEDIA_World_H

#include "OMediaTypes.h"
#include "OMediaRect.h"
#include "OMediaListener.h"
#include "OMediaTimer.h"
#include "OMediaElementContainer.h"
#include "OMediaRateManager.h"
#include "OMediaLayer.h"
#include "OMediaElement.h"
#include "OMediaPickRequest.h"
#include "OMediaBroadcaster.h"

#include <list>

class OMediaInputEngine;

//***********************
// The World

class OMediaWorld : public OMediaElementContainer, 
					public OMediaListener 
{
	public:
	
	// * Constructor/Destructor
	
	omtshared OMediaWorld();
	omtshared virtual ~OMediaWorld();	
				// Destructor deletes all elements and viewports for you

	// * Pause
	
	omtshared virtual void pause(bool p);
	inline bool is_paused(void) {return pause_count!=0;}

	// * Update
	
	omtshared virtual void update_logic(void); 	// Update animation logic for this world

	// * Messages

	omtshared virtual void listen_to_message(omt_Message msg, void *param =NULL);

	// * Rate manager. This is only used for logic. Not for rendering. Each viewport has its 
	// own rate manager for rendering.

	inline OMediaRateManager *get_rate_manager(void) {return &rate_manager;}

	// * Layers
	
	inline omt_LayerList *get_layers(void) {return &layers;}

	// * Light sources

	inline omt_LightList *get_light_sources(void) {return &lights;}
	
	// * Mouse tracking
	
	// Elements that require mouse tracking should be listeners of the
	// mouse tracking broadcaster. As long as there is one element registred, OMT repeatly
	// picks the main viewport and sends the result to the listeners. As soon as
	// an element does not need mouse tracking information anymore it should remove itself
	// from the mouse tracking broadcaster. Mouse tracking should be used only when required
	// because it is time consuming.
	// OMT sends an omsg_MouseTrack message with a pointer to an OMediaPickResult object to
	// the listeners.
	// Please note that the viewport checked is the main supervisor. 
	// So if there is no active viewport, no mouse tracking is performed.
	
	inline OMediaBroadcaster &get_mouse_tracking_broadcaster(void) {return mouse_tracking;}
	
	
	// * Default input engine - Some element requires input (like buttons or string fields)
	// You can set your own input engine here. By default OMT initializes it with the default
	// system input engine.
	
	omtshared virtual void set_input_engine(OMediaInputEngine *ie);
	omtshared virtual OMediaInputEngine *get_input_engine(void);
	

	protected:
	
	omtshared virtual void update_elements(omt_ElementList::iterator begin,
								  omt_ElementList::iterator end,
								  float elapsed);

	omtshared virtual void pause_elements(bool p,
								 omt_ElementList::iterator begin,
								 omt_ElementList::iterator end);

	omtshared virtual void mouse_track(void);

	omtshared virtual void container_link_element(OMediaElement *e);


	long				pause_count;
	OMediaRateManager	rate_manager;
	omt_LayerList		layers;
	omt_LightList		lights;

	OMediaInputEngine	*input_engine;
	OMediaBroadcaster	mouse_tracking;
};


// Internal use

class OMediaMouseTrackPick : public OMediaPickRequest
{
	public:

	short		desktop_mousex,desktop_mousey;
	bool		mouse_down;
	bool		validated;
};


#endif

