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
#ifndef OMEDIA_AbstractWindow_H
#define OMEDIA_AbstractWindow_H

#include <string>
#include <list>

#include "OMediaSupervisor.h"
#include "OMediaListener.h"
#include "OMediaRetarget.h"
#include "OMediaMenu.h"
#include "OMediaBroadcaster.h"
#include "OMediaRGBColor.h"
#include "OMediaMonitorMap.h"

class OMediaVideoEngine;
class OMediaRenderer;
class OMediaRenderTarget;
class OMediaWindow;

enum omt_WindowStyle
{
	omwstyle_Null,
	
	omwstyle_Fullscreen,		// Full screen
	omwstyle_Single,			// Single

	// multiple window mode

	omwstyle_Desktop,			// Parent window
	omwstyle_Child,				// Child window
	
	omwstyle_Other
};

typedef list<OMediaWindow*> omt_WindowList;


//-----------------
// Internal use

class OMediaWin2Engine
{
	public:
	
	OMediaVideoEngine	*engine;
	OMediaRenderer		*renderer;
	OMediaRenderTarget	*target;
};

typedef list<OMediaWin2Engine> omt_Win2EngList;


//-----------------
// Abstract window

class OMediaWindow : public OMediaListener,
					 public OMediaSupervisor,
					 public OMediaBroadcaster
{
	public:
	
	// * Construction

	omtshared OMediaWindow(OMediaSupervisor *supervisor);
	omtshared virtual ~OMediaWindow();
	
	omtshared virtual omt_WindowStyle get_style(void);

	omtshared virtual void close(void);		// This will delete the Windows asap. You don't have to delete
											// the window object when you call this method
	inline omt_Bool is_closing(void) {return closing;}

	// * Menu
	
	omtshared virtual void set_menu(OMediaMenu *menu_array);


	// * Messages

	omtshared virtual void listen_to_message(omt_Message msg, void *param =NULL);

	// * Size and position
	
	inline short get_x(void) const {return x;}
	inline short get_y(void) const {return y;}
	inline short get_width(void) const {return width;}
	inline short get_height(void) const {return height;}

	omtshared virtual void place(short x, short y);
	omtshared virtual void set_size(short w, short h);

	omtshared virtual void set_max_size(short w, short h);
	omtshared virtual void set_min_size(short w, short h);

	omtshared virtual void get_max_size(short &w, short &h);
	omtshared virtual void get_min_size(short &w, short &h);

	omtshared virtual void bounds_changed(void);


	// * Active

	static OMediaWindow *find_active(void);
	
	omtshared virtual void activate(void);
	omtshared virtual omt_Bool is_active(void);

	omtshared virtual void activated(void);		// Called when activated
	omtshared virtual void deactivated(void);	// or deactivated

	
	// * Refresh
	
	omtshared virtual void refresh(void);

	// * Name
	
	inline void get_title(string &str) const {str = title;}
	omtshared virtual void set_title(string str);

	// * Show/hide
	
	omtshared virtual void show(void);
	omtshared virtual void hide(void);
	inline bool is_visible(void) const {return visible;}
	

	// * Video engines

	omtshared virtual void link_video_engine(OMediaVideoEngine *engine);
	omtshared virtual void unlink_video_engine(OMediaVideoEngine *engine);

	inline omt_Win2EngList	*get_win2engines(void) {return &win2engines;}
	
	// Don't use the this method on a fullscreen window.
	inline void link_monitor_map(OMediaMonitorMap *monitors)
	{
		for(omt_VideoEngineList::iterator vi=monitors->engines.begin();
			vi!=monitors->engines.end();
			vi++)
		{
			link_video_engine(*vi);
		}
	}
	

	// * Retarget

	inline OMediaRetarget *get_retarget(void) {return retarget;}
	
	
	// * Window list
	
	static list<OMediaWindow*> *get_window_list(void) {return &window_list;}

	
	// * Background color
	
	omtshared virtual void set_backcolor(const OMediaRGBColor &rgb);
	inline void get_backcolor(OMediaRGBColor &rgb) const {rgb = back_color;}

	// * Engines supervised by this window

	inline omt_SupervisedEngineList *get_supervised_engines(void) {return &engines;}

	protected:

	omtshared virtual void update_renderer(OMediaWin2Engine *weng);

	omtshared virtual void destroy_dependencies(void);
	
	short 			x,y;
	short 			width,height;
	string 			title;
	bool			closing,visible;
	OMediaRGBColor	back_color;

	OMediaRetarget					*retarget;
	omt_WindowList::iterator		winlist_iterator;
	
	omt_Win2EngList					win2engines;
	omt_SupervisedEngineList		engines;

	static omt_WindowList			window_list;

	public:
	
	omtshared virtual void enter_sleep_mode(void);
	omtshared virtual void exit_sleep_mode(void);
	omtshared virtual void erase_contexts(void);
	omtshared virtual void update_contexts(void);

};


#endif

