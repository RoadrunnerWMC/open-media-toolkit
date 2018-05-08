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

 

#include "OMediaApplication.h"
#include "OMediaPeriodical.h"
#include "OMediaWindow.h"

OMediaApplication::OMediaApplication(omt_InitAppHints 		ainithints,
									 bool				ainit_retarget)
{
	retarget = NULL;
	do_quit = false;
	event_manager = new OMediaEventManager;

	if (ainit_retarget) init_retarget(ainithints);
	
	set_main_supervisor(this);
}

OMediaApplication::~OMediaApplication()
{
	delete event_manager;
	delete retarget;
}

void OMediaApplication::start(void)
{
	while(!do_quit) OMediaPeriodical::devote_time();
}


void OMediaApplication::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_Quit: 
		do_quit = true; 
		break;
		
		case omsg_Event:
		{
			OMediaEvent	*event;
			event = (OMediaEvent*)param;
			if (event->window)
			{
				switch(event->type)
				{
					case omtet_WindowRefresh:		
					event->window->refresh();
					break;
			
					case omtet_WindowCloseButton:		
					event->window->close();
					break;
					
					case omtet_WindowActivated:
					event->window->activated();
					break;

					case omtet_WindowDeactivated:
					event->window->deactivated();
					break;
                                        
                                        default:
                                        break;
				}
			}
		}		
		break;
		
		default:
		OMediaSupervisor::listen_to_message(msg,param);
		break;
	}
}

bool OMediaApplication::update_message_state(omt_Message msg, 
												bool &enabled,
												bool &mark)
{
	switch(msg)
	{
		case omsg_Quit:
		enabled = true;
		break;
	
		default:
		return OMediaSupervisor::update_message_state(msg, enabled, mark);
	}
	
	return true;
}



