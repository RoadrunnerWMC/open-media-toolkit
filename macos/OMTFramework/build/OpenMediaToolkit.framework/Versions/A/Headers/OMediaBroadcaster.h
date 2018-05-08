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


 

#ifndef OMEDIA_Broadcaster_H
#define OMEDIA_Broadcaster_H

#include "OMediaTypes.h"
#include "OMediaListener.h"


class OMediaBroadcaster
{
	public:

	// * Constructor/Destructor

	omtshared OMediaBroadcaster(void);
	omtshared virtual ~OMediaBroadcaster(void);

	// * Message

	omtshared virtual void broadcast_message(omt_Message msg, void *param = NULL);


	// * Listeners
	
	inline omt_ListenerList *getlisteners(void) {return &listeners;}
	
	omtshared virtual void addlistener(OMediaListener *l);
	omtshared virtual void removelistener(OMediaListener *l);

	protected:

	omt_ListenerList		listeners;
};


#endif

