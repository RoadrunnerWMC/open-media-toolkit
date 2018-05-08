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
#ifndef OMEDIA_MonitorMap_H
#define OMEDIA_MonitorMap_H

#include "OMediaEngineFactory.h"

#include <vector>

class OMediaWindow;

typedef vector<OMediaVideoEngine*> omt_VideoEngineList;


class OMediaMonitorMap
{
	public:

	OMediaMonitorMap(omt_EngineID id, OMediaWindow *master_window)
	{
		OMediaEngineFactory	*f = OMediaEngineFactory::get_factory();
	
		OMediaVideoEngine	*info_engine,*eng;
		long				i,c;
		omt_VideoCardList::iterator i2;
		long				ncards;

		info_engine = f->create_video_engine(id,master_window);
		ncards = info_engine->get_video_cards()->size();
		delete info_engine;

		for(i=0;i<ncards; i++)
		{
			eng = f->create_video_engine(id,master_window);
		
			i2 = eng->get_video_cards()->begin();
		
			c = i;
			while(c--) i2++;
			
			eng->link(&(*i2));
			engines.push_back(eng);
		}		
	}
	
	~OMediaMonitorMap() 
	{
		for(omt_VideoEngineList::iterator i=engines.begin();
			i!=engines.end();
			i++)
			delete *(i);
	}

	omt_VideoEngineList	engines;
};



#endif

