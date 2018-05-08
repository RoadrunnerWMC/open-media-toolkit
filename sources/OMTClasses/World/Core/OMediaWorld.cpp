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
 
#include "OMediaWorld.h"
#include "OMediaAnimPeriodical.h"
#include "OMediaElement.h"
#include "OMediaEngineFactory.h"
#include "OMediaSupervisor.h"
#include "OMediaWindow.h"

OMediaWorld::OMediaWorld()
{
	pause_count = 0;
	input_engine = NULL;
	OMediaAnimPeriodical::get_anim_periocial()->logic_broadcaster.addlistener(this);
}

OMediaWorld::~OMediaWorld()
{
	while(layers.size()) delete *(layers.begin());
	while(elements.size()) delete *(elements.begin());

	delete input_engine;
}
			
void OMediaWorld::update_logic(void)
{
	if (pause_count==0)
	{	
		if (rate_manager.begin_update())
		{	
			update_elements(elements.begin(), elements.end(),(float)rate_manager.getelapsed());
			if (mouse_tracking.getlisteners()->size()) mouse_track();

			rate_manager.end_update();
		}
	}
}

void OMediaWorld::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_UpdateWorldLogic:
		update_logic();
		break;	
		
		case omsg_EngineDeleted:
		if ((OMediaEngine*)param == input_engine) input_engine = NULL;
		break;
	}
}

void OMediaWorld::update_elements(omt_ElementList::iterator begin,
								  omt_ElementList::iterator end,
								  float elapsed)
{
	OMediaElement	*element;
	omt_ElementList::iterator e,ne;

	for (e = begin; e != end; e=ne) 
	{
		ne = e;
		ne++;
		
		element = *e;
	
		element->update_logic(elapsed);
		
		if (element->is_destroy_on()) delete element;
		else if (element->get_element_list()->size())
			update_elements(element->get_element_list()->begin(),
							element->get_element_list()->end(),elapsed);
	}
}

void OMediaWorld::pause(bool p)
{
	if (p)
	{
		pause_count++;
	
		if (pause_count==1)	
		{
			rate_manager.reset_timer();
			pause_elements(p,elements.begin(),elements.end());
		}
	}
	else
	{
		if (pause_count==1)
		{
			rate_manager.reset_timer();
			pause_elements(p,elements.begin(),elements.end());
		}
		
		pause_count--;
	}	
}

void OMediaWorld::pause_elements(bool p,
								 omt_ElementList::iterator begin,
								 omt_ElementList::iterator end)
{
	OMediaElement	*element;

	for (omt_ElementList::iterator e = begin; e != end; e++) 
	{
		element = *e;
	
		element->pause(p);
		
		if (element->get_element_list()->size())
			pause_elements(p,element->get_element_list()->begin(),
						     element->get_element_list()->end());
	}
}

void OMediaWorld::set_input_engine(OMediaInputEngine *ie)
{
	delete input_engine;
	input_engine = ie;
}

OMediaInputEngine *OMediaWorld::get_input_engine(void)
{
	if (!input_engine) 
	{
		OMediaWindow	*master;
		
		if (OMediaWindow::get_window_list()->size())
		{
			master = (*OMediaWindow::get_window_list()->begin());
	
			input_engine = OMediaEngineFactory::get_factory()->create_input_engine(ommeic_OS, master);
			input_engine->addlistener(this);
		}
	}
	
	return input_engine;
}

void OMediaWorld::mouse_track(void)
{
	OMediaInputEngine		*engine = get_input_engine();	
	OMediaSupervisor 		*sup;
	OMediaMouseTrackPick	mtrackpick;
		
	sup = OMediaSupervisor::get_main_supervisor();
	if (sup && engine)
	{
		engine->get_mouse_position(mtrackpick.desktop_mousex,mtrackpick.desktop_mousey);
		mtrackpick.mouse_down = engine->mouse_down();
		mtrackpick.validated = false;
		sup->listen_to_message(omsg_VPCheckMouseTrack, &mtrackpick);
		if (mtrackpick.validated)
			mouse_tracking.broadcast_message(omsg_MouseTrack,&mtrackpick);
	}
}

void OMediaWorld::container_link_element(OMediaElement *e)
{
	e->world_link(this);
}

