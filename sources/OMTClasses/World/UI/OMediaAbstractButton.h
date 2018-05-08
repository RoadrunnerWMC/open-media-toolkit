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
#ifndef OMEDIA_AbstractButton_H
#define OMEDIA_AbstractButton_H

#include "OMediaBroadcaster.h"
#include "OMediaCanvas.h"
#include "OMediaPeriodical.h"
#include "OMediaSurfaceElement.h"
#include "OMediaListener.h"


// * Radio button support (internal use)

class OMediaAbstractButton;
class OMediaMouseTrackPick;


class OMediaRadioBroadcaster : public OMediaBroadcaster	
{
	public:

	OMediaAbstractButton *its_button;
};


// * The abstract button class

class OMediaAbstractButton : public OMediaBroadcaster
{
	public:

	// * Construction

	omtshared OMediaAbstractButton();	  
	omtshared virtual ~OMediaAbstractButton();

	
	// * State

	inline bool isdown(void) const {return is_down;}


	// * Toggle mode
	
	inline void set_toggle_mode(bool t) {toggle_mode =t; }
	inline bool get_toggle_mode(void) const {return toggle_mode;}
	
	omtshared virtual void select(void);
	omtshared virtual void deselect(void);

	// * Message to send to listener

	inline void set_message(omt_Message m) {msg = m;}
	inline omt_Message get_message(void) {return msg;}

	inline void set_continuous_msg(omt_Message m) {cont_msg =m;}
	inline omt_Message get_continuous_msg(void) const {return cont_msg;}

	// * Get radio group broadcaster
	
	inline OMediaRadioBroadcaster *get_radio_broadcaster(void) {return &radio_broadcaster;}

	// * Mouse click/track
	
	omtshared virtual void clicked(	OMediaPickResult 	*res, 
									bool 				mouse_down,
									OMediaBroadcaster 	*tracking_broadcaster,
								 	OMediaElement		*element);

	omtshared virtual void track(OMediaMouseTrackPick 	*mtrackpick,
								 OMediaBroadcaster 		*tracking_broadcaster,
								 OMediaElement			*element);


	protected:
	
	bool					is_down, toggle_mode;
	omt_Message				msg,cont_msg;
	OMediaRadioBroadcaster	radio_broadcaster;
	short					clickox,clickoy,clickx,clicky;
};


#endif

