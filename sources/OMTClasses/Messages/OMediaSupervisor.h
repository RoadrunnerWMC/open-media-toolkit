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
#ifndef OMEDIA_Supervisor_H
#define OMEDIA_Supervisor_H

#include "OMediaTypes.h"
#include "OMediaMessagePort.h"

#include <list>

class OMediaSupervisor;

typedef list<OMediaSupervisor*> omt_SupervisorList;


class OMediaSupervisor : public OMediaMessagePort
{
	public:


	// * Constructor
	
	omtshared OMediaSupervisor();					
	omtshared OMediaSupervisor(OMediaSupervisor *its_supervisor);					
	omtshared virtual ~OMediaSupervisor();
	
	// * Main supervisor
	
	static inline void set_main_supervisor(OMediaSupervisor *s) {main_supervisor = s;}
	static inline OMediaSupervisor *get_main_supervisor(void) {return main_supervisor;}

	// * Its supervisor

	inline OMediaSupervisor *get_supervisor(void) {return its_supervisor;}
	omtshared virtual void set_supervisor(OMediaSupervisor *supervisor);


	// * Supevisors under its control
	
	inline omt_SupervisorList *get_sub_supervisors(void) {return &supervisor_list;}


	// * Listen to message
						
	omtshared virtual void listen_to_message(omt_Message msg, void *param =NULL);


	// * Update message state

	omtshared virtual bool update_message_state(omt_Message msg, 
												bool &enabled,
												bool &mark);


	protected:
	
	omtshared static OMediaSupervisor			*main_supervisor;

	OMediaSupervisor				*its_supervisor;
	omt_SupervisorList::iterator	node;

	omt_SupervisorList				supervisor_list;
};



#endif

