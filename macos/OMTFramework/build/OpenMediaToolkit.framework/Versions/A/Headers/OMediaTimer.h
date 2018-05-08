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
#ifndef OMEDIA_OMediaTimer_H
#define OMEDIA_OMediaTimer_H

#include "OMediaTypes.h"
#include "OMediaTimeCounter.h"
#include "OMediaPeriodical.h"

//---------------------------------------------------------------------
// Timer template

class OMediaLowlevelTimerMSCounter
{
	public:

	static inline unsigned long get_millisecs(void) {return OMediaTimeCounter::lowlevel_get_millisecs();}
};

class OMediaTimerMSCounter
{
	public:

	static inline unsigned long get_millisecs(void) {return OMediaTimeCounter::get_millisecs();}
};

class OMediaTimerSysDevoteTime
{
	public:

	static inline void devote_time(void) {OMediaPeriodical::devote_time();}
};

class OMediaTimerSysDevoteTimeDisabled
{
	public:

	static inline void devote_time(void) {}
};


template<class MSCOUNTER, class SYSDEVOTE_TIME>
class OMediaTimerTemplate
{
	public:
	
	inline OMediaTimerTemplate() {stopped = true;elapsed=initial=0;locked=false;}
	
	inline void start(long start_at=0) {stop(); initial=start_at; last_count = MSCOUNTER::get_millisecs(); stopped=false;}
	inline unsigned long getelapsed(void) const {return (stopped || locked) ?elapsed:(MSCOUNTER::get_millisecs() - last_count)+initial;}
	inline void stop(void) {if (!stopped) {elapsed = getelapsed(); stopped = true;}}

	inline unsigned long get_last_elapsed(void) const {return elapsed;}


	inline bool is_running(void) const {return !stopped;}
	inline bool is_locked(void) const {return locked;}

	inline void wait(unsigned long ms) {start(); while(getelapsed()<ms) SYSDEVOTE_TIME::devote_time(); stop();}

	inline void setelapsed(unsigned long e) {elapsed = e;}

	inline void lock(bool l) {if (!locked) elapsed = getelapsed();  locked = l;}


	//----------- Low-level

	inline void set_last_mscount(const unsigned long lc) { last_count = lc;}
	inline unsigned long get_last_mscount(void) const {return last_count;}

	protected:
	
	unsigned long last_count, elapsed,initial;
	bool	     stopped,locked;
};

//---------------------------------------------------------------------
// Timer implementation

// Standard - use high level millisecs counter, devote time to periodicals during wait() calls.

class OMediaTimer : public OMediaTimerTemplate<OMediaTimerMSCounter,OMediaTimerSysDevoteTime> {};


// Use low-level millisecs counter

class OMediaLowTimer : public OMediaTimerTemplate<OMediaLowlevelTimerMSCounter,OMediaTimerSysDevoteTime> {};

// Busy wait, do not call periodicals during wait().

class OMediaTimerBWait : public OMediaTimerTemplate<OMediaTimerMSCounter,OMediaTimerSysDevoteTimeDisabled> {};


// Busy wait, use low-level millisecs counter

class OMediaLowTimerBWait : public OMediaTimerTemplate<OMediaLowlevelTimerMSCounter,OMediaTimerSysDevoteTimeDisabled> {};


#endif

