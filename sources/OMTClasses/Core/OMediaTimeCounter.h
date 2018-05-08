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
#ifndef OMEDIA_TimeCounter_H
#define OMEDIA_TimeCounter_H

#include "OMediaTypes.h"
#include "OMediaSysDefs.h"

typedef unsigned long (*omt_MillisecFuncTrap)(void);

typedef unsigned short omt_MilliSecsFlags;
const omt_MilliSecsFlags ommsf_AlwaysUseLowCounter = (1<<0);

class OMediaTimeCounter
{
	public:	
	
	// Global millisecs counter
	
	omtshared static unsigned long get_millisecs(void);
	
	omtshared static void pause_millisecs(bool pause);
	omtshared static bool is_paused_millisecs(void);
	
	omtshared static void add_millisecs(long delta_msecs);
	
	omtshared static void set_flags_millisecs(omt_MilliSecsFlags flags);
	omtshared static omt_MilliSecsFlags get_flags_millisecs(void);

	omtshared static void set_millisecs_trap(omt_MillisecFuncTrap func);
	omtshared static omt_MillisecFuncTrap get_millisecs_trap(void);

	
	omtshared static unsigned long lowlevel_get_millisecs(void);	
};

#endif

