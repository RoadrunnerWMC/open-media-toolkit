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

#include "OMediaThread.h"

void OMediaThread::set_current_process_priority(omt_TaskPriority p)
{
	switch(p)
	{
		case omtpc_Normal:
		SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
		break;

		case omtpc_TimeCritical:
		SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
		break;
	}
}

omt_TaskPriority OMediaThread::get_current_process_priority(void)
{
	if (GetPriorityClass(GetCurrentProcess())!=HIGH_PRIORITY_CLASS)
		return omtpc_Normal;

	return omtpc_TimeCritical;
}

void OMediaThread::set_current_thread_priority(omt_TaskPriority p)
{
	switch(p)
	{
		case omtpc_Normal:
		SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
		break;

		case omtpc_TimeCritical:
		SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);
		break;
	}

}

omt_TaskPriority OMediaThread::get_current_thread_priority(void)
{
	if (GetThreadPriority(GetCurrentThread())!=THREAD_PRIORITY_TIME_CRITICAL)
		return omtpc_Normal;

	return omtpc_TimeCritical;
}

