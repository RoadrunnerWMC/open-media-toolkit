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
#ifndef OMEDIA_Application_H
#define OMEDIA_Application_H

#include "OMediaTypes.h"
#include "OMediaSupervisor.h"
#include "OMediaRetarget.h"
#include "OMediaAppHints.h"
#include "OMediaEventManager.h"

// OMT application
 
class OMediaApplication : public OMediaSupervisor
{
	public:

	// * Construction

	omtshared OMediaApplication(omt_InitAppHints 		ainithints = omchint_BestInit,
					  bool				ainit_retarget = true);
					  
	omtshared virtual ~OMediaApplication(void);
	

	// * Process

	omtshared virtual void start(void);
	inline void quit(void) {do_quit = true;}

	inline bool is_quitting(void) const {return do_quit;}

	omtshared virtual void listen_to_message(omt_Message msg, void *param);

	// * Event manager
	
	inline void seteventmanager(OMediaEventManager *ev) {delete event_manager; event_manager = ev;}
	inline OMediaEventManager *geteventmanager(void) {return event_manager;}


	// * Retarget

	inline OMediaRetarget	*get_retarget(void) {return retarget;}


	// * Update message state

	omtshared virtual bool update_message_state(omt_Message msg, 
												bool &enabled,
												bool &mark);


	protected:

	omtshared virtual void init_retarget(omt_InitAppHints ainithints);

	bool		do_quit;

	OMediaRetarget		*retarget;
	OMediaEventManager	*event_manager;
};


#endif

