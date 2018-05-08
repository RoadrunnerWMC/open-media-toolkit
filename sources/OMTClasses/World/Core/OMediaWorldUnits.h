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
#ifndef OMEDIA_WorldUnits_H
#define OMEDIA_WorldUnits_H

#include "OMediaRect.h"
#include "OMediaTrigo.h"

typedef float 		omt_WUnit;			// World unit
typedef float 		omt_WUnitPerSec;	// Units per second (time based animation)
typedef float 		omt_WFramePerSec;	// Frames per second (time based animation)
typedef float 		omt_WMilliSec;		// Used to calculate millisecs
typedef omt_Angle 	omt_WAngle;			// World angle (0-360)
typedef float 		omt_WAnglePerSec;	// Angles per second (time based animation)


#define omt_WUnitToLong(u) long(u)



class OMedia2DWorldRect : public OMediaRectTemplate<omt_WUnit>
{
	public:

	inline OMedia2DWorldRect(void) {}
	inline OMedia2DWorldRect(OMedia2DWorldRect	*r) : OMediaRectTemplate<omt_WUnit> (r) {}
	
	inline OMedia2DWorldRect(omt_WUnit aleft, omt_WUnit atop, omt_WUnit aright,omt_WUnit abottom) : 
									OMediaRectTemplate<omt_WUnit>(aleft, atop, aright, abottom) {}
};


#endif

