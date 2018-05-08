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
#ifndef OMEDIA_FullscreenWindow_H
#define OMEDIA_FullscreenWindow_H

#include "OMediaWindow.h"

/* Please note that a full screen window is very different than a standard window.
First of all you cannot link it to more than one video engine. The window is
visible only if it's video engine is linked to a video card. If it's not the
case it is inactive and invisible.
*/

class OMediaFullscreenWindow : public OMediaWindow
{
	public:
	
	// * Construction

	omtshared OMediaFullscreenWindow(OMediaSupervisor *supervisor, bool initially_visible =false);
	omtshared virtual ~OMediaFullscreenWindow();


	omtshared virtual omt_WindowStyle get_style(void);


	// * Size/Position, it is static for fullscreen window

	omtshared virtual void bounds_changed(void);


	// * Active
	
	omtshared virtual void activate(void);
	omtshared virtual omt_Bool is_active(void);

	omtshared virtual void activated(void);

	// * Messages

	omtshared virtual void listen_to_message(omt_Message msg, void *param =NULL);

	// * Video engines

	omtshared virtual void link_video_engine(OMediaVideoEngine *engine);
	omtshared virtual void unlink_video_engine(OMediaVideoEngine *engine);


	// * Name

	omtshared virtual void set_title(string str);
	
	// * Show/hide
	
	omtshared virtual void show(void);
	omtshared virtual void hide(void);


	// * Background color
	
	omtshared virtual void set_backcolor(const OMediaRGBColor &rgb);

	protected:	
	
	omtshared virtual void init_window(void);
	omtshared virtual void cleanup_window(void);
	
	omtshared virtual void update_video_engine_connection(void);
	omtshared virtual void exit_sleep_mode(void);
	omtshared virtual void enter_sleep_mode(void);
	omtshared virtual void update_window_posiz(void);

	
	bool	asleep;
	
	
};


#endif

