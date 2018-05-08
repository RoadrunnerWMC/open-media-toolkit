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
#ifndef OMEDIA_RateManager_H
#define OMEDIA_RateManager_H

#include "OMediaTimer.h"

enum omt_RateMode
{
	omcrm_Fixed,			// The rate is fixed
	omcrm_None,				// No frame rate. OMT updates are done as fast as possible  (default)
	omcrm_AutoRegulate		// OMT tries to smooth the rate in real time.
};

class OMediaRateManager : public OMediaTimer
{
	public:
	
	// * Construction
	
	inline OMediaRateManager() 
	{
		mode=omcrm_None; 
		rate_fixed = 40;
		rate_ceil = 500;
		rate_floor = 1;
	}
		
	// * Begin/end update
	
	bool begin_update(void)
	{
		lock(true);
		if (rate_ceil)
		{
			if (get_last_elapsed()>rate_ceil) setelapsed(rate_ceil);
		}

		if (rate_floor && is_running())
		{
			if (get_last_elapsed()<rate_floor)
			{
				lock(false);
				return false;
			}
		}

		unsigned long elapsed;
		bool res;

		if (!is_running()) start();	
			
		elapsed = get_last_elapsed();
	
		if (mode==omcrm_AutoRegulate) update_autoregulate();

		if (mode==omcrm_Fixed || mode==omcrm_AutoRegulate)
		{
			if (elapsed >= rate_fixed) 
			{
				res = true;
				start();	
			}
			else res = false;
		}
		else
		{
			rate_fixed = elapsed;
			res = true;
			start();
		}
		
		return res;
	}

	inline void end_update(void)
	{
		lock(false);
	}

	void update_autoregulate(void)
	{
		unsigned long elapsed = get_last_elapsed();
	
		if (elapsed == 0)
		{
			rate_auto_framecount = 0;
			rate_auto_ratetotal = 0; 
			rate_fixed = 40;
		}
		else
		{
			if (rate_auto_framecount==16)
			{
				rate_fixed = rate_auto_ratetotal>>4;
				rate_auto_ratetotal = 0;
				rate_auto_framecount = 0;
			}
			else if (elapsed >= 500 && rate_auto_framecount>=1)
			{
				rate_fixed = rate_auto_ratetotal/rate_auto_framecount;
				rate_auto_ratetotal = 0;
				rate_auto_framecount = 0;
			}
			else 
			{
				rate_auto_framecount++;
				rate_auto_ratetotal += elapsed;
			}
		}
	}

	// * Timer

	inline void reset_timer(void) {stop(); setelapsed(0);}

	// * Rate mode

	inline void set_mode(omt_RateMode mode)
	{
		reset_timer();
		mode = mode;
	}

	inline omt_RateMode get_mode(void) {return mode;}


	// * Rate

	void set_rate(unsigned long millisecs)
	{
		reset_timer();
		rate_fixed = millisecs;
	}

	inline unsigned long get_rate(void) {return rate_fixed;}
	inline float get_rate_fps(void) {return 1000.0f/float(rate_fixed);}


	// * Ceil

	inline void set_ceil(unsigned long millisecs) {rate_ceil = millisecs;}
	inline unsigned long get_ceil(void) const {return rate_ceil;}	

	// * Floor

	inline void set_floor(unsigned long millisecs) {rate_floor = millisecs;}
	inline unsigned long get_floor(void) const {return rate_floor;}

	
	protected:
	
	omt_RateMode	mode;
	unsigned long 	rate_fixed,rate_ceil,rate_floor,rate_auto_framecount,rate_auto_ratetotal;	
};



#endif

