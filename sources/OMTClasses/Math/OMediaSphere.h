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
#ifndef OMEDIA_Sphere_H
#define OMEDIA_Sphere_H

#include "OMedia3DPoint.h"

#include <vector>

/** Sphere support.

  This method defines a 3D point plus a radius. It is a child class of 
  the OMedia3DPoint class, so you can use all the 3D point methods.

*/

class OMediaSphere : public OMedia3DPoint
 {
 	public:
 
 	OMediaSphere(void) {radius = 1.0;}
 	OMediaSphere(float r) {radius = r;}

	// * Radius

 	inline void set_radius(float r) {radius = r;}
 	inline float get_radius(void) const {return radius ;}
 
 
 	// * Operators

	OMedia3DPoint &operator=(const OMedia3DPoint &p) { x = p.x; y = p.y; z = p.z; return *this;}
	OMediaSphere &operator=(const OMediaSphere &p) { x = p.x; y = p.y; z = p.z; radius=p.get_radius(); return *this;}
	
	float radius;
 };

typedef vector<OMediaSphere> omt_SphereList;


#endif

