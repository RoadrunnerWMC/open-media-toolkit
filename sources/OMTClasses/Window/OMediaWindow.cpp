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

#include "OMediaWindow.h"
#include "OMediaPeriodical.h"
#include "OMediaRenderer.h"
#include "OMediaVideoEngine.h"

omt_WindowList	OMediaWindow::window_list;
OMediaWindow 	*active_window;


OMediaWindow::OMediaWindow(OMediaSupervisor *supervisor):
						OMediaSupervisor(supervisor)
{
	back_color.set(0,0,0);

	title = "OMT Output";
	x=y=width=height=0;
	retarget = NULL;
	set_main_supervisor(this);
	
	window_list.push_back(this);
	winlist_iterator = window_list.end();
	winlist_iterator--;
	
	closing = false;
	visible = true;	
}

OMediaWindow::~OMediaWindow()
{
	broadcast_message(omsg_WindowDeleted,this);

	destroy_dependencies();

	window_list.erase(winlist_iterator);
	delete retarget;
}

void OMediaWindow::destroy_dependencies(void)
{
	for(omt_Win2EngList::iterator wi = win2engines.begin();
		wi!= win2engines.end();
		wi++)
	{
		delete (*wi).target;
	}
	
	win2engines.erase(win2engines.begin(),win2engines.end());
	while(engines.size()) delete *(engines.begin());
}

void OMediaWindow::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_Close:
		close();
		break;
		
		case omsg_VideoEngineDeleted:
		{
			for(omt_Win2EngList::iterator ei = win2engines.begin();
				ei!= win2engines.end();
				ei++)
			{
				if ((*ei).engine==(OMediaVideoEngine*)param)
				{
					win2engines.erase(ei);
					return;
				}			
			}
		}
		break;
		
		case omsg_RendererSelected:
		{
			for(omt_Win2EngList::iterator ei = win2engines.begin();
				ei!= win2engines.end();
				ei++)
			{
				if ((*ei).engine==(OMediaVideoEngine*)param)
				{
					update_renderer(&(*ei));
					break;
				}
			}
		}		
		
		break;
		
		case omsg_RendererDeleted:
		{
			for(omt_Win2EngList::iterator ei = win2engines.begin();
				ei!= win2engines.end();
				ei++)
			{
				if ((*ei).renderer==(OMediaRenderer*)param) (*ei).renderer=NULL;
			}
		}		
		break;
		
		case omsg_RenderTargetDeleted:
		{
			for(omt_Win2EngList::iterator ei = win2engines.begin();
				ei!= win2engines.end();
				ei++)
			{
				if ((*ei).target==(OMediaRenderTarget*)param) (*ei).target = NULL;
			}
		}		
		break;
	
		default:
		OMediaSupervisor::listen_to_message(msg, param);
		break;
	}
}

void OMediaWindow::set_menu(OMediaMenu *menu_array) {}


void OMediaWindow::activated(void)
{
	set_main_supervisor(this);

}

void OMediaWindow::deactivated(void) {}


void OMediaWindow::place(short x, short y) {}
void OMediaWindow::set_size(short w, short h) {}
void OMediaWindow::set_title(string str) {}

omt_WindowStyle OMediaWindow::get_style(void) {return omwstyle_Null;}

void OMediaWindow::activate(void) {}

omt_Bool OMediaWindow::is_active(void) {return false;}

void OMediaWindow::close(void)
{
	closing = true;
}

void OMediaWindow::refresh(void) {}

void OMediaWindow::show(void) {}
void OMediaWindow::hide(void) {}

void OMediaWindow::set_max_size(short w, short h) {}
void OMediaWindow::set_min_size(short w, short h) {}

void OMediaWindow::get_max_size(short &w, short &h) {w=h=0;}
void OMediaWindow::get_min_size(short &w, short &h) {w=h=0;}

OMediaWindow *OMediaWindow::find_active(void)
{
	for(omt_WindowList::iterator i=window_list.begin();
		i!=window_list.end();
		i++)
	{
		if ((*i)->is_active()) return (*i);	
	}
	
	return NULL;
}

void OMediaWindow::bounds_changed(void) 
{
	broadcast_message(omsg_WindowBoundsChanged);
}

void OMediaWindow::link_video_engine(OMediaVideoEngine *engine)
{
	// Check if already linked
	
	for(omt_Win2EngList::iterator wi = win2engines.begin();
		wi!= win2engines.end();
		wi++)
	{
		if ((*wi).engine==engine) return;
	}
	
	engine->addlistener(this);
	
	OMediaWin2Engine	weng;
	weng.engine = engine;
	weng.target = NULL;
	weng.renderer = NULL;
	win2engines.push_back(weng);

	update_renderer(&win2engines.back());
}

void OMediaWindow::update_renderer(OMediaWin2Engine *weng)
{
	weng->renderer = weng->engine->get_selected_renderer();
	if (weng->renderer)
	{	
		weng->target = weng->renderer->new_target();
		weng->target->set_window(this);
		weng->target->addlistener(this);
		weng->renderer->addlistener(this);
	}
	else weng->target = NULL;
	
	broadcast_message(omsg_WindowRendererUpdated,this);
}

void OMediaWindow::unlink_video_engine(OMediaVideoEngine *engine)
{
	omt_Win2EngList::iterator wi;

	for(wi = win2engines.begin();
		wi!= win2engines.end();
		wi++)
	{
		if ((*wi).engine==engine) break;
	}
	
	if (wi==win2engines.end()) return;
	
	delete (*wi).target;
	
	win2engines.erase(wi);	
}

void OMediaWindow::set_backcolor(const OMediaRGBColor &rgb)
{
	back_color = rgb;
}


void OMediaWindow::enter_sleep_mode(void) {}
void OMediaWindow::exit_sleep_mode(void) {}
void OMediaWindow::erase_contexts(void) 
{
	for(omt_Win2EngList::iterator ei = win2engines.begin();
		ei!= win2engines.end();
		ei++)
	{
		if ((*ei).target!=NULL) (*ei).target->erase_context();
	}
}

void OMediaWindow::update_contexts(void) 
{
	for(omt_Win2EngList::iterator ei = win2engines.begin();
		ei!= win2engines.end();
		ei++)
	{
		if ((*ei).target!=NULL) (*ei).target->update_context();
	}
}


//--------------------------------------
// Auto-close window manager

class OMediaAutoCloseWindows : public OMediaPeriodical
{
	public:

	OMediaAutoCloseWindows() {}
	~OMediaAutoCloseWindows() {}

	virtual void spend_time(void)
	{
		omt_WindowList::iterator wi,nwi;
	
		for(wi = OMediaWindow::get_window_list()->begin();
			wi!=OMediaWindow::get_window_list()->end();)
		{
			nwi = wi; nwi++;
		
			if ( (*wi)->is_closing()) delete (*wi);
			
			wi = nwi;
		}	
	}

	static OMediaAutoCloseWindows auto_close;
};

OMediaAutoCloseWindows OMediaAutoCloseWindows::auto_close;



