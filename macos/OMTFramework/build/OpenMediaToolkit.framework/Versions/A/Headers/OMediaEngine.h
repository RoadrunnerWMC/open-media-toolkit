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
#ifndef OMEDIA_Engine_H
#define OMEDIA_Engine_H

#include "OMediaEngineID.h"
#include "OMediaBroadcaster.h"
#include "OMediaListener.h"

#include <list>


class OMediaEngineImpSlave;
class OMediaEngine;
class OMediaWindow;

typedef list<OMediaEngineImpSlave*> omt_EngineImpSlaveList;
typedef list<OMediaEngine*> omt_SupervisedEngineList;

class OMediaEngine : public OMediaBroadcaster,
					 public OMediaListener
{
	public:
	
	OMediaEngine(omt_EngineID id, OMediaWindow *window);
	virtual ~OMediaEngine();
	
	inline omt_EngineID get_engine_id(void) const {return engine_id;}

	inline omt_EngineImpSlaveList *get_implementation_list(void) 
													{return &implementation_list;}

	omtshared virtual void delete_all_implementations(void);
	
	inline OMediaWindow *get_supervisor_window(void) {return master_window;}
	
	protected:	
	
	omt_EngineID							engine_id;
	omt_EngineImpSlaveList					implementation_list;
	OMediaWindow 							*master_window;
	omt_SupervisedEngineList::iterator		win_elist_iterator;
};



#endif

