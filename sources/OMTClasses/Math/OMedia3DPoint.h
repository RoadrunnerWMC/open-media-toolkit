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
#ifndef OMEDIA_3DPoint_H
#define OMEDIA_3DPoint_H

#include <math.h>

#include "OMediaTypes.h"
#include "OMediaTrigo.h"

class OMedia3DVector;

/** Define a 3D point.

This is the main class used to identify a point in 3D space. The OMT 
implementation of 3D point uses 32bits floating point values defined as x,y,z and w.

w is set to 1.0f by the constructor.

*/

class OMedia3DPoint
{
	public:
	
	float x,y,z,w;

	inline float *xyzw(void) {return (&x);}

	// * Construction

	OMedia3DPoint(void) {w=1.0f;} 
	OMedia3DPoint(const OMedia3DPoint &p) { x = p.x; y = p.y; z = p.z; w = p.w;} 
	OMedia3DPoint(float px, float py, float pz, float pw =1.0f) 
					{ x = px; y = py; z = pz; w = pw;}


	// * Methods

	void set(float px, float py, float pz) { x = px; y = py; z = pz;} 
	void set(float px, float py, float pz, float pw) { x = px; y = py; z = pz; w = pw;} 


	// * Transformation
	
	/** Translate point. Passed values are added to the point component. */
	inline void translate(float x, float y, float z)
	{
		this->x+=x;
		this->y+=y;
		this->z+=z;	
	}
	

	/** Scale point. Point components are multiplied by the passed values. */
	inline void scale(float x, float y, float z)
	{
		this->x*=x;
		this->y*=y;
		this->z*=z;	
	}
	
	/** Rotate point. OMT uses a left-hand orientation. Default rotation order is z, x, y.*/
	inline void rotate(omt_Angle angle_x, omt_Angle angle_y, omt_Angle angle_z)
	{
		float		x1,y1,z1,x2,y2,z2;
		float		sin_x, cos_x, sin_y, cos_y,sin_z,cos_z;
		float		*sin_tab, *cos_tab;

		cos_tab = OMediaCosSinTable::global.get_costab();
		sin_tab = OMediaCosSinTable::global.get_sintab();

		sin_x = sin_tab[angle_x&omc_MaxAngleMask];
		cos_x = cos_tab[angle_x&omc_MaxAngleMask];

		sin_y = sin_tab[angle_y&omc_MaxAngleMask];
		cos_y = cos_tab[angle_y&omc_MaxAngleMask];

		sin_z = sin_tab[angle_z&omc_MaxAngleMask];
		cos_z = cos_tab[angle_z&omc_MaxAngleMask];

		x1 = (x*cos_z + y*sin_z);
		z1 = z;
		y1 = (y*cos_z - x*sin_z) ;
	
		x2 = x1;
		z2 = (z1*cos_x - y1*sin_x) ;
		y2 = (z1*sin_x + y1*cos_x) ;
	
		x = (x2*cos_y - z2*sin_y) ;
		z = (x2*sin_y + z2*cos_y);
		y = y2;	
	}
	
	/** Inverse rotate point. OMT uses a left-hand orientation. Inverse rotation order is y, x, z.*/
	inline void inv_rotate(omt_Angle angle_x, omt_Angle angle_y, omt_Angle angle_z) // Order: y-x-z
	{
		float		x1,y1,z1,x2,y2,z2;
		float		sin_x, cos_x, sin_y, cos_y,sin_z,cos_z;
		float		*sin_tab, *cos_tab;

		cos_tab = OMediaCosSinTable::global.get_costab();
		sin_tab = OMediaCosSinTable::global.get_sintab();

		sin_x = sin_tab[angle_x&omc_MaxAngleMask];
		cos_x = cos_tab[angle_x&omc_MaxAngleMask];

		sin_y = sin_tab[angle_y&omc_MaxAngleMask];
		cos_y = cos_tab[angle_y&omc_MaxAngleMask];

		sin_z = sin_tab[angle_z&omc_MaxAngleMask];
		cos_z = cos_tab[angle_z&omc_MaxAngleMask];

		x1 = (x*cos_y - z*sin_y) ;
		z1 = (x*sin_y + z*cos_y);
		y1 = y;		
	
		x2 = x1;
		z2 = (z1*cos_x - y1*sin_x) ;
		y2 = (z1*sin_x + y1*cos_x) ;

		x = (x2*cos_z + y2*sin_z);
		z = z2;
		y = (y2*cos_z - x2*sin_z) ;
	}

	/** World transform. Transform a point in absolute world position to viewer position and orientation.*/
	
	inline void world_transform(float viewx, float viewy, float viewz, omt_Angle angle_x, omt_Angle angle_y, omt_Angle angle_z)
	{
		translate(-viewx,-viewy, -viewz);		
		inv_rotate(-angle_x,-angle_y,-angle_z);
	}




	// * Operators

	OMedia3DPoint &operator=(const OMedia3DPoint &p) { x = p.x; y = p.y; z = p.z; w = p.w; return *this;}

	OMedia3DPoint &operator+=(const OMedia3DPoint &p) {  x += p.x; y += p.y; z += p.z; return *this;}
	OMedia3DPoint &operator-=(const OMedia3DPoint &p) {  x -= p.x; y -= p.y; z -= p.z; return *this;}

	inline int operator==(const OMedia3DPoint& p) const {return (p.x==x && p.y==y && p.z==z && p.w==w);}
	inline int operator!=(const OMedia3DPoint& p) const {return !(p==*this);}	
};



#endif

