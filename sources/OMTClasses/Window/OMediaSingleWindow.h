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
#ifndef OMEDIA_SingleWindow_H
#define OMEDIA_SingleWindow_H

#include "OMediaWindow.h"

class OMediaSingleWindow : public OMediaWindow
{
	public:
	
	// * Construction

	omtshared OMediaSingleWindow(OMediaSupervisor *supervisor, bool initially_visible =false, bool dont_initialize =false);

	omtshared virtual ~OMediaSingleWindow();

	omtshared virtual void close(void);


	omtshared virtual omt_WindowStyle get_style(void);

	// * Menu
	
	omtshared virtual void set_menu(OMediaMenu *menu_array);


	// * Size/Position

	omtshared virtual void place(short x, short y);
	omtshared virtual void set_size(short x, short y);

	omtshared virtual void set_max_size(short w, short h);
	omtshared virtual void set_min_size(short w, short h);

	omtshared virtual void get_max_size(short &w, short &h);
	omtshared virtual void get_min_size(short &w, short &h);

	omtshared virtual void bounds_changed(void);


	// * Active
	
	omtshared virtual void activate(void);
	omtshared virtual omt_Bool is_active(void);

	omtshared virtual void activated(void);


	// * Name

	omtshared virtual void set_title(string str);
	
	// * Show/hide
	
	omtshared virtual void show(void);
	omtshared virtual void hide(void);

	// * Update message state

	omtshared virtual bool update_message_state(omt_Message msg, 
												bool &enabled,
												bool &mark);


	// * Background color
	
	omtshared virtual void set_backcolor(const OMediaRGBColor &rgb);

	protected:	
	
	omtshared virtual void init_window(bool initially_visible);
	omtshared virtual void cleanup_window(void);

	short	max_width,max_height;
	short	min_width,min_height;
};


#endif

