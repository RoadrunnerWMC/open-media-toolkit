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
#ifndef OMEDIA_WorldAngle_H
#define OMEDIA_WorldAngle_H

#include "OMediaTypes.h"
#include "OMediaWorldUnits.h"
#include "OMedia3DVector.h"
#include "OMedia3DAxis.h"

class OMediaWorldAngle
{
	public:
	
	// * Constructor/Destructor

	inline OMediaWorldAngle() {anglex=angley=anglez=0;}

	// * Angle

	inline void set_anglex(omt_WAngle x) {anglex=x;}
	inline void set_angley(omt_WAngle y) {angley=y;}
	inline void set_anglez(omt_WAngle z) {anglez=z;}



	inline void set_angle(omt_WAngle nx,omt_WAngle ny, omt_WAngle nz) {anglex=nx;angley=ny;anglez=nz;}

	inline void set_angle(OMedia3DVector &v, omt_WAngle zang) 
	{
		v.angles(anglex,angley);
		anglez=zang;
	}
	
	inline void set_angle(OMedia3DAxis &axis, bool invert =false)
	{
		if (invert)
			axis.inv_convert(anglex,angley,anglez);
		else
			axis.convert(anglex,angley,anglez);
	}	
		
	inline void add_angle(omt_WAngle vx,omt_WAngle vy, omt_WAngle vz) {anglex+=vx;angley+=vy;anglez+=vz;}


	inline omt_WAngle get_anglex(void) const {return anglex;}
	inline omt_WAngle get_angley(void) const {return angley;}
	inline omt_WAngle get_anglez(void) const {return anglez;}

	inline void get_angle(omt_WAngle &ax, omt_WAngle &ay, omt_WAngle &az) const 
				{ax = anglex; ay = angley; az = anglez;}

	protected:

	omt_WAngle				anglex,angley,anglez;
};



#endif

