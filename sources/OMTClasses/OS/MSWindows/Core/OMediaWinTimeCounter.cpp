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


#include "OMediaTimeCounter.h"

static bool millisecs_paused = false;
static long millisecs_delta = 0;
static unsigned long pause_millisecs=0;
static omt_MillisecFuncTrap millisectrap =NULL;
static omt_MilliSecsFlags millisecflags = 0;


void OMediaTimeCounter::set_flags_millisecs(omt_MilliSecsFlags flags)
{
	millisecflags = flags;
}

omt_MilliSecsFlags OMediaTimeCounter::get_flags_millisecs(void)
{
	return millisecflags;
}

void OMediaTimeCounter::add_millisecs(long delta_msecs)
{
	millisecs_delta -= delta_msecs;
}

void OMediaTimeCounter::set_millisecs_trap(omt_MillisecFuncTrap func)
{
	millisectrap = func;
}

omt_MillisecFuncTrap OMediaTimeCounter::get_millisecs_trap(void)
{
	return millisectrap;
}

/*
float  TimeElapsed;
LARGE_INTEGER LastCount, ThisCount, CounterFrequency; 

QueryPerformanceFrequency(&CounterFrequency);
QueryPerformanceCounter(&LastCount); 

while(Alive())
     {
                // work out timing?

         QueryPerformanceCounter(&ThisCount);
         TimeElapsed = float(ThisCount.QuadPart - LastCount.QuadPart) 
                            / CounterFrequency.QuadPart; 
*/


unsigned long OMediaTimeCounter::lowlevel_get_millisecs(void)
{	 
	return (unsigned long)(GetTickCount());

/*	static bool			initFreq;
	static __int64		CounterFrequency;

	__int64				ThisCount;


	if (!initFreq)
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&CounterFrequency);
		initFreq = true;
	}

	if (CounterFrequency==0) return (unsigned long)(GetTickCount());

    QueryPerformanceCounter((LARGE_INTEGER*)&ThisCount);

	float res = (float(ThisCount)/float(CounterFrequency)) * 1000.0f;

	return (unsigned long)res;*/
}

unsigned long OMediaTimeCounter::get_millisecs(void)
{
	if (millisecflags&ommsf_AlwaysUseLowCounter) return lowlevel_get_millisecs();
	if (millisectrap) return millisectrap();
	if (millisecs_paused) return ::pause_millisecs;

	return (unsigned long)(lowlevel_get_millisecs())-millisecs_delta;
}


void OMediaTimeCounter::pause_millisecs(bool pause)
{
	if (pause && !millisecs_paused)
	{
		::pause_millisecs = get_millisecs();
		millisecs_paused = true;
	}
	else if (!pause && millisecs_paused)
	{
		millisecs_paused = false;		
		millisecs_delta += get_millisecs() - ::pause_millisecs; 
	}
}

bool OMediaTimeCounter::is_paused_millisecs(void)
{
	return millisecs_paused;
}



