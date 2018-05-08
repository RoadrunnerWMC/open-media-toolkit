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
 
 

#include "OMediaEngine.h"
#include "OMediaEngineImplementation.h"
#include "OMediaWindow.h"

OMediaEngine::OMediaEngine(omt_EngineID id, OMediaWindow *awindow) 
{
	engine_id = id;
        master_window = awindow;
        
        if (master_window)
        {
            awindow->get_supervised_engines()->push_back(this);
            win_elist_iterator = master_window->get_supervised_engines()->end();
            win_elist_iterator--;
        }
}

OMediaEngine::~OMediaEngine()
{
	broadcast_message(omsg_EngineDeleted,this);

	delete_all_implementations();
        
        if (master_window) master_window->get_supervised_engines()->erase(win_elist_iterator);
}
	
void OMediaEngine::delete_all_implementations(void)
{
	while(implementation_list.size()) delete *(implementation_list.begin());
}

