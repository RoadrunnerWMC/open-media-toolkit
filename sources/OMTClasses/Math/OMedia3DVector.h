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
#ifndef OMEDIA_3DVector_H
#define OMEDIA_3DVector_H

#include <math.h>

#include "OMedia3DPoint.h"

/** Define a 3D vector.

The OMT  implementation of a 3D vector uses 32bits floating point values defined as x,y, z and w.

w is initialized to 0.0f by constructor.
*/


class OMedia3DVector
{
	public:
	
	float x,y,z,w;

	inline float *xyzw(void) {return (&x);}


	// * Construction

	OMedia3DVector(void) {w=0.0f;} 
	OMedia3DVector(const OMedia3DVector &v) { x = v.x; y = v.y; z = v.z; w=v.w;} 
	OMedia3DVector(float vx, float vy, float vz, float vw=0.0f) { x = vx; y = vy; z = vz; w = vw;} 
	OMedia3DVector(const OMedia3DPoint &p1, const OMedia3DPoint &p2) 
	{
		x = p2.x - p1.x;	
		y = p2.y - p1.y;	
		z = p2.z - p1.z;
		w = 0.0f;
	}

	// * Methods

	void set(float vx, float vy, float vz) { x = vx; y = vy; z = vz;} 
	void set(float vx, float vy, float vz, float vw) { x = vx; y = vy; z = vz; w = vw;} 
	void set(const OMedia3DPoint &p1, const OMedia3DPoint &p2)
	{
		x = p2.x - p1.x;	
		y = p2.y - p1.y;	
		z = p2.z - p1.z;
	}

	void set(OMedia3DPoint &p1) {x = p1.x;	y = p1.y;	z = p1.z;}	

	/** Returns magnitude of the vector. It is computed using a square root so it is slow but accurate.
	If you need a quick way to compute a magnitude use the quick_magnitude method instead. */
	float magnitude(void) const { return float(sqrt(x*x + y*y + z*z)); }	// Length


	/** Add a vector. */
	
	void add(const OMedia3DVector &v) {x+=v.x;y+=v.y;z+=v.z;}
	void add(float vx, float vy, float vz) {x+=vx;y+=vy;z+=vz;}
	

	/** Compute magnitude without using a square root. Faster than magnitude but less accurate. */
	omtshared float quick_magnitude(void) const;
										

	/** Returns the dot product. */
	float dot_product(const OMedia3DVector &v) const {return (x*v.x) + (y*v.y) + (z*v.z);}
	
	/** Compute the cross product and store it in normal parameter. It is a left-hand cross product.*/
	void cross_product(const OMedia3DVector &v, OMedia3DVector &normal) const
	{
		normal.x =  (y * v.z - z * v.y);
		normal.y = -(x * v.z - z * v.x);
		normal.z =  (x * v.y - y * v.x);
	}
	
	/** Generate a unit vector. Please note that quick_magnitude is used to normalize the vector.*/
	void normalize(void)
	{
		float m = 1.0f/quick_magnitude();
		x *=m;	y *=m;	z *=m;
	}
	
	/** Rotate vector. OMT uses a left-hand orientation. Default rotation order is z, x, y.*/
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
	inline void inv_rotate(omt_Angle angle_x, omt_Angle angle_y, omt_Angle angle_z)
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

	/** Compute the normal of a plane. The plane is defined by three 3D points. */
	inline void compute_normal(const OMedia3DPoint &p1, const OMedia3DPoint &p2, const OMedia3DPoint &p3)
	{
		float	x1,y1,z1;
		float	x2,y2,z2;
		
		x1 = p2.x-p1.x;	y1 = p2.y-p1.y;	z1 = p2.z-p1.z;
		x2 = p3.x-p1.x;	y2 = p3.y-p1.y;	z2 = p3.z-p1.z;
	
		x =  (y2 * z1 - z2 * y1);
		y = -(x2 * z1 - z2 * x1);
		z =  (x2 * y1 - y2 * x1);
		
		normalize();
	}
	
	/** Returns angle between two vectors. Maximum angle is 180 degrees.*/
	omtshared omt_Angle angle_between(const OMedia3DVector &v) const;
	
	/** Find the orientation of the vector in absolute angles. */
	omtshared void angles(omt_Angle &x, omt_Angle &y);

	/** Find the reflected vector based on a normal vector. The normal
	vector must be normalized */

	inline void reflect(const OMedia3DVector &normal_v, OMedia3DVector &result)
	{
		float	rx,ry,rz,d;
		d = -normal_v.dot_product(*this);
		rx = 2.0f*normal_v.x*d;
		ry = 2.0f*normal_v.y*d;
		rz = 2.0f*normal_v.z*d;
		result.x = x + rx;
		result.y = y + ry;
		result.z = z + rz;
	}

	// * Operators

	OMedia3DVector &operator=(const OMedia3DVector &v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this;}
	OMedia3DVector &operator=(const OMedia3DPoint &p) { x = p.x; y = p.y; z = p.z; return *this;}

	OMedia3DVector operator+(const OMedia3DVector &v) const { OMedia3DVector r(x+v.x, y+v.y, z+v.z); return r;}
	OMedia3DVector operator-(const OMedia3DVector &v) const { OMedia3DVector r(x-v.x, y-v.y, z-v.z); return r;}

	OMedia3DVector &operator+=(const OMedia3DVector &v) {  x += v.x; y += v.y; z += v.z; return *this;}
	OMedia3DVector &operator-=(const OMedia3DVector &v) {  x -= v.x; y -= v.y; z -= v.z; return *this;}

	OMedia3DVector operator*(const OMedia3DVector &v) const // cross-product
	{ OMedia3DVector r; cross_product(v,r); return r;}

	inline int operator==(const OMedia3DVector& p) const {return (p.x==x && p.y==y && p.z==z && p.w==w);}
	inline int operator!=(const OMedia3DVector& p) const {return !(p==*this);}

};


#endif

