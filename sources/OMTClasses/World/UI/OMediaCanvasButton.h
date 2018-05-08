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
#ifndef OMEDIA_CanvasButton_H
#define OMEDIA_CanvasButton_H

#include "OMediaCanvasElement.h"
#include "OMediaAbstractButton.h"


class OMediaCanvasButton : 	public OMediaCanvasElement,
				public OMediaAbstractButton
{
	public:

	// * Construction

	omtshared OMediaCanvasButton();
	omtshared virtual ~OMediaCanvasButton();
	
	// * Messages

	omtshared virtual void listen_to_message(omt_Message msg, void *param);

	// * Canvases
	
	omtshared virtual void set_canvas_up(OMediaCanvas *canv);
	inline OMediaCanvas *get_canvas_up(void) {return canvas_up;}

	omtshared virtual void set_canvas_down(OMediaCanvas *canv);
	inline OMediaCanvas *get_canvas_down(void) {return canvas_down;}

	// * Called when the element is clicked

	omtshared virtual void clicked(OMediaPickResult *res, bool mouse_down);

	// * Toggle mode ( see also OMediaAbstractButton)

	omtshared virtual void select(void);
	omtshared virtual void deselect(void);

	
	protected:

	OMediaCanvas		*canvas_up,
						*canvas_down;
};

#endif

