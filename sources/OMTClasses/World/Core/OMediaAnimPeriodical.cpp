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
 
#include "OMediaAnimPeriodical.h"

OMediaAnimPeriodical 	*OMediaAnimPeriodical::anim_periodical;
OMediaPickRequest		*OMediaAnimPeriodical::picking_mode;
bool					OMediaAnimPeriodical::quick_refresh = false;

OMediaAnimPeriodical::OMediaAnimPeriodical()
{
	min_time_per_frame = 0;
	elapsed = 0;
}

OMediaAnimPeriodical::~OMediaAnimPeriodical()
{
}

void OMediaAnimPeriodical::spend_time(void)
{
	bool render = false,direct;

	direct = quick_refresh || min_time_per_frame==0 || picking_mode;

	if (direct) render = true;
	else if (timer.is_running())
	{
		if (min_time_per_frame>0)
		{
			elapsed += timer.getelapsed();
			if (elapsed>min_time_per_frame) render = true;
			elapsed = elapsed%min_time_per_frame;
		}
	}
	else render = true;

	if (render)
	{
		if (!picking_mode) logic_broadcaster.broadcast_message(omsg_UpdateWorldLogic);
		renderer_broadcaster.broadcast_message(omsg_RenderFrames,picking_mode);
	}

	if (!direct) timer.start();
}

OMediaAnimPeriodical *OMediaAnimPeriodical::get_anim_periocial(void)
{
	if (!anim_periodical)
	{
		anim_periodical = new OMediaAnimPeriodical;
	}
	
	return anim_periodical;
}
	
void OMediaAnimPeriodical::replace_anim_periocial(OMediaAnimPeriodical *p)
{
	if (!anim_periodical)
	{
		anim_periodical = p;
		return;
	}
	
	if (p)
	{
		omt_ListenerList::iterator i;
	
		for(i=anim_periodical->logic_broadcaster.getlisteners()->begin();
			i!=anim_periodical->logic_broadcaster.getlisteners()->end();
			i++)
		{	
			p->logic_broadcaster.addlistener(*i);
		}

		for(i=anim_periodical->renderer_broadcaster.getlisteners()->begin();
			i!=anim_periodical->renderer_broadcaster.getlisteners()->end();
			i++)
		{	
			p->renderer_broadcaster.addlistener(*i);
		}
	}
	
	delete anim_periodical;
	anim_periodical = p;
}

//------ Engine factory auto-destructor

class OMediaAnimPeriodicalAutoDtor
{
	public:

	OMediaAnimPeriodicalAutoDtor();
	virtual ~OMediaAnimPeriodicalAutoDtor();

	static OMediaAnimPeriodicalAutoDtor destroy;
};

OMediaAnimPeriodicalAutoDtor OMediaAnimPeriodicalAutoDtor::destroy;
OMediaAnimPeriodicalAutoDtor::OMediaAnimPeriodicalAutoDtor() {}
OMediaAnimPeriodicalAutoDtor::~OMediaAnimPeriodicalAutoDtor() 
{
	OMediaAnimPeriodical::replace_anim_periocial(NULL);
}
