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

#include "OMediaCanvasButton.h"
#include "OMediaWorld.h"

OMediaCanvasButton::OMediaCanvasButton()
{
	canvas_up = canvas_down = NULL;
}

OMediaCanvasButton::~OMediaCanvasButton()
{
	if (canvas_up) canvas_up->db_unlock();
	if (canvas_down) canvas_down->db_unlock();
}


	
void OMediaCanvasButton::set_canvas_up(OMediaCanvas *canv)
{
	if (canvas_up) canvas_up->db_unlock();
	canvas_up = canv;
	if (canvas_up) canvas_up->db_lock();

	if (!isdown()) set_canvas(canvas_up);
}

void OMediaCanvasButton::set_canvas_down(OMediaCanvas *canv)
{
	if (canvas_down) canvas_down->db_unlock();
	canvas_down = canv;
	if (canvas_down) canvas_down->db_lock();

	if (isdown()) set_canvas(canvas_down);
}


void OMediaCanvasButton::clicked(OMediaPickResult *res, bool mouse_down)
{
	if (!its_world) return;

	OMediaAbstractButton::clicked(	res, mouse_down, &its_world->get_mouse_tracking_broadcaster(),this);

	if (isdown()) set_canvas(canvas_down);
	else set_canvas(canvas_up);
} 

void OMediaCanvasButton::select(void)
{
	OMediaAbstractButton::select();
	if (isdown()) set_canvas(canvas_down);
	else set_canvas(canvas_up);
}

void OMediaCanvasButton::deselect(void)
{
	OMediaAbstractButton::deselect();
	if (isdown()) set_canvas(canvas_down);
	else set_canvas(canvas_up);
}


void OMediaCanvasButton::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_MouseTrack:
		{
			OMediaMouseTrackPick	*mtrack = (OMediaMouseTrackPick*)param;
			
			if (!its_world) return;
			
			OMediaAbstractButton::track(mtrack,
									 	&its_world->get_mouse_tracking_broadcaster(),
								 		this);

			if (isdown()) set_canvas(canvas_down);
			else set_canvas(canvas_up);
		}
		break;
		
		default:
		OMediaElement::listen_to_message(msg, param);
		break;	
	}
}


