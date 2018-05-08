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
#ifndef OMEDIA_Thread_H
#define OMEDIA_Thread_H

#include "OMediaTypes.h"

enum omt_TaskPriority
{
	omtpc_Normal,
	omtpc_TimeCritical
};

class OMediaThread
{
public:


	omtshared static void set_current_process_priority(omt_TaskPriority p);
	omtshared static omt_TaskPriority get_current_process_priority(void);

	omtshared static void set_current_thread_priority(omt_TaskPriority p);
	omtshared static omt_TaskPriority get_current_thread_priority(void);
};


#endif
