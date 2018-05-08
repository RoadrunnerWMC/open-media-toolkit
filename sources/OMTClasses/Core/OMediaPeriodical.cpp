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
 
 

#include "OMediaPeriodical.h"
#include "OMediaError.h"

bool OMediaPeriodical::periodical_aborted;

omt_PeriodicalList	*OMediaPeriodical::periodical_list;

OMediaPeriodical::OMediaPeriodical() 
{
	if (!periodical_list) 
	{	
		periodical_list = new omt_PeriodicalList;
		if (!periodical_list) omd_EXCEPTION(omcerr_OutOfMemory);
	}

	periodical_list->push_back(this);
	node = --(periodical_list->end());
	
	paused = false;
}					

OMediaPeriodical::~OMediaPeriodical() 
{
	periodical_list->erase(node);

	if (periodical_list->size()==0)
	{
		delete periodical_list;
		periodical_list = NULL;
	}
	
	abort_devote_time();
}


void OMediaPeriodical::spend_time(void) {}

void OMediaPeriodical::devote_time(void)
{
	if (!periodical_list) return;
	
	periodical_aborted = false;

	for(omt_PeriodicalList::iterator i = periodical_list->begin();
		i != periodical_list->end();
		i++)
	{
		if (!(*i)->is_paused()) (*i)->spend_time();
		
		if (periodical_aborted) break;
	}
}

