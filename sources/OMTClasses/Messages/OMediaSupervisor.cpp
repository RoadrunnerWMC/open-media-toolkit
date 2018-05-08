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

#include "OMediaSupervisor.h"


OMediaSupervisor *OMediaSupervisor::main_supervisor;

OMediaSupervisor::OMediaSupervisor()
{
	its_supervisor = NULL;
}
					
OMediaSupervisor::OMediaSupervisor(OMediaSupervisor *s)
{
	its_supervisor = NULL;
	set_supervisor(s);
}
			
OMediaSupervisor::~OMediaSupervisor()
{
	while(supervisor_list.size()) delete *(supervisor_list.begin());
	
	if (its_supervisor) its_supervisor->get_sub_supervisors()->erase(node);

	if (main_supervisor == this) main_supervisor = its_supervisor;
}

void OMediaSupervisor::set_supervisor(OMediaSupervisor *s)
{
	if (its_supervisor) its_supervisor->get_sub_supervisors()->erase(node);
	
	its_supervisor = s;
	
	if (its_supervisor) 
	{
		its_supervisor->get_sub_supervisors()->push_back(this);
		node = --(its_supervisor->get_sub_supervisors()->end());
	}
}

void OMediaSupervisor::listen_to_message(omt_Message msg, void *param)
{
	if (its_supervisor) its_supervisor->listen_to_message(msg,param);
}

bool OMediaSupervisor::update_message_state(omt_Message msg, 
												bool &enabled,
												bool &mark)
{
	if (its_supervisor) return its_supervisor->update_message_state(msg,
															 enabled,
															 mark);
	return false;
}


