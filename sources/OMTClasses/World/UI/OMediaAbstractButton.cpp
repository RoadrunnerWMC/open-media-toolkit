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


#include "OMediaAbstractButton.h"
#include "OMediaWorld.h"

OMediaAbstractButton::OMediaAbstractButton() 
{
	is_down = false;
	msg = cont_msg = omsg_NULL;
	toggle_mode = false;
	radio_broadcaster.its_button = this;
}
 
OMediaAbstractButton::~OMediaAbstractButton()
{
}

void OMediaAbstractButton::select(void) 
{
	if (toggle_mode && !is_down) 
	{ 
		if (radio_broadcaster.getlisteners()->size())
		{
			radio_broadcaster.broadcast_message(omsg_RadioButtonDown, this);
			is_down = true;
		}
		else
		{
			is_down = true;
		}
	}
}

void OMediaAbstractButton::deselect(void) 
{
	if (toggle_mode) 
	{
		is_down = false; 
	}
}


void OMediaAbstractButton::clicked(	OMediaPickResult 	*res, 
									bool 				mouse_down,
									OMediaBroadcaster 	*tracking_broadcaster,
								 	OMediaElement		*element)
{
	if (!mouse_down) return;
	
	if (toggle_mode)
	{
		if (radio_broadcaster.getlisteners()->size())
		{
			if (!is_down)
			{
				radio_broadcaster.broadcast_message(omsg_RadioButtonDown, this);
				is_down = true;
				if (OMediaAbstractButton::msg != omsg_NULL) broadcast_message(OMediaAbstractButton::msg,element);
			}
		}
		else
		{
			is_down = !is_down;
			if (OMediaAbstractButton::msg != omsg_NULL) broadcast_message(OMediaAbstractButton::msg,element);
		}
	}
	else if (!is_down)
	{
		is_down = true;
		tracking_broadcaster->addlistener(element);		
		if (cont_msg!=omsg_NULL) broadcast_message(cont_msg,element);
	}
}


void OMediaAbstractButton::track(OMediaMouseTrackPick 	*mtrackpick,
								 OMediaBroadcaster 		*tracking_broadcaster,
								 OMediaElement			*element)
{
	is_down = (mtrackpick->closer_hit.type!=omptc_Null && mtrackpick->closer_hit.element==element);
	if (is_down && cont_msg!=omsg_NULL) broadcast_message(cont_msg,element);

	if (!mtrackpick->mouse_down)
	{
		if (OMediaAbstractButton::msg != omsg_NULL && is_down) broadcast_message(OMediaAbstractButton::msg,element);
		tracking_broadcaster->removelistener(element);
		is_down = false;
	}
}

