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
#ifndef OMEDIA_AnimPeriodical_H
#define OMEDIA_AnimPeriodical_H

#include "OMediaPeriodical.h"
#include "OMediaBroadcaster.h"
#include "OMediaTimer.h"

class OMediaPickRequest;

class OMediaAnimPeriodical : public OMediaPeriodical
{
	public:
	
	omtshared OMediaAnimPeriodical();
	omtshared virtual ~OMediaAnimPeriodical();

	omtshared virtual void spend_time(void);

	omtshared static OMediaAnimPeriodical *get_anim_periocial(void);
	omtshared static void replace_anim_periocial(OMediaAnimPeriodical *p);

	OMediaBroadcaster		logic_broadcaster;
	OMediaBroadcaster		renderer_broadcaster;

	inline void enable_picking_mode(OMediaPickRequest *p) {picking_mode = p;}
	inline void disable_picking_mode(void) {picking_mode = omc_NULL;}

	static inline void set_quick_refresh(bool b) {quick_refresh = b;}
	static inline bool get_quick_refresh(void) {return quick_refresh;}

	inline void set_min_time_per_frame(int msecs) {min_time_per_frame = msecs;}
	inline float get_min_time_per_frame() {return min_time_per_frame;}

	protected:

	omtshared static OMediaAnimPeriodical	*anim_periodical;
	omtshared static OMediaPickRequest		*picking_mode;

	omtshared static bool					quick_refresh;

	int										min_time_per_frame;
	int										elapsed;
	OMediaTimer								timer;
};

#endif

