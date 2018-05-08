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
 
#include "OMediaError.h"
#include "OMediaMacRtgApplication.h"
#include "OMediaMacAppleEvent.h"


//................................

void OMediaApplication::init_retarget(omt_InitAppHints ainithints)
{
	retarget = new OMediaMacRtgApplication(this,ainithints);
	if (!retarget) omd_EXCEPTION(omcerr_OutOfMemory);
}



//................................

OMediaMacRtgApplication::OMediaMacRtgApplication(OMediaApplication *app, omt_InitAppHints	inithints):
						OMediaRetarget(omcrtg_Application)
{
	if (inithints!=omchint_NoInit)
	{
		FlushEvents(everyEvent,0);
		InitCursor();
		SetEventMask(everyEvent);
		
		if (inithints==omchint_BestInit)
		{
			its_appleevent = new OMediaAppleEvent;
			if (!its_appleevent) omd_EXCEPTION(omcerr_OutOfMemory);			
		}
		else its_appleevent = NULL;
	}
	else its_appleevent = NULL;
}


OMediaMacRtgApplication::~OMediaMacRtgApplication()
{
	delete its_appleevent;
}
