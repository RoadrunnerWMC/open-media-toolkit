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
#ifndef OMEDIA_WorldPosition_H
#define OMEDIA_WorldPosition_H

#include "OMediaTypes.h"
#include "OMediaWorldUnits.h"
#include "OMedia3DPoint.h"
#include "OMedia3DVector.h"

class OMediaWorldPosition
{	
	public:
	
	// * Constructor/Destructor

	inline OMediaWorldPosition() {x=y=z=0;}

	// * Position in world

	inline omt_WUnit getx(void) const {return x;}
	inline omt_WUnit gety(void) const {return y;}
	inline omt_WUnit getz(void) const {return z;}

	inline omt_WUnit get_x(void) const {return x;}	// Same as getx
	inline omt_WUnit get_y(void) const {return y;}
	inline omt_WUnit get_z(void) const {return z;}

	inline void setx(const omt_WUnit v) {x = v;}
	inline void sety(const omt_WUnit v) {y = v;}
	inline void setz(const omt_WUnit v) {z = v;}

	inline void set_x(const omt_WUnit v) {x = v;}	// Same as setx
	inline void set_y(const omt_WUnit v) {y = v;}
	inline void set_z(const omt_WUnit v) {z = v;}

	inline void get_position(OMedia3DPoint &p) const {p.x=x; p.y=y; p.z=z;}
	inline void get_position(float &px, float &py, float &pz) const {px=x; py=y; pz=z;}
	

	inline long gettruncx(void) const {return omt_WUnitToLong(x);}
	inline long gettruncy(void) const {return omt_WUnitToLong(y);}
	inline long gettruncz(void) const {return omt_WUnitToLong(z);}

	inline void place(OMedia3DPoint &p) {x = p.x; y = p.y; z = p.z;}
	inline void move(OMedia3DVector &v) {x += v.x; y += v.y; z += v.z;}

	inline void place(omt_WUnit nx,omt_WUnit ny, omt_WUnit nz) {x = nx; y = ny; z = nz;}
	inline void move(omt_WUnit vx,omt_WUnit vy, omt_WUnit vz) {x += vx; y += vy; z += vz;}

	inline void place(omt_WUnit nx,omt_WUnit ny) {x = nx; y = ny;}
	inline void move(omt_WUnit vx,omt_WUnit vy) {x += vx; y += vy;}

	protected:

	omt_WUnit				x, y, z;
};



#endif

