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
#ifndef OMEDIA_3DAxis_H
#define OMEDIA_3DAxis_H

#include "OMediaTypes.h"
#include "OMedia3DPoint.h"
#include "OMediaTrigo.h"

#include <math.h>

enum omt_3DAxis
{
	omc3daxis_X,
	omc3daxis_Y,
	omc3daxis_Z,
	
	omc3daxis_Pitch = omc3daxis_X,
	omc3daxis_Yaw = omc3daxis_Y,
	omc3daxis_Roll = omc3daxis_Z	
};

/** 3D axis support.

This is a usefull class to resolve trigonometric problem in 3D space. 
Orientating a camera in 3D with euclidian angles is not always easy,
especially if you need to use free pitch/yaw/roll rotations like
in a flight simulator. The OMedia3DAxis class allows you to define
an arbitrary set of 3D axises that can be oriented the way you want. Then
you can translate your 3D axises to absolute euclidian angles that can
passed to a viewport or an element to orientate it.


The three axises are defined as three vectors. By default, the axis
are oriented in the default OMT 3D space left-hand implementation:

  X vector 1,0,0
  Y vector 0,1,0
  Z vector 0,0,1

*/

class OMedia3DAxis
{
	public:

		
	inline OMedia3DAxis()
	{
		reset_axis();
	}	

	/** Returns an axis. It is returned by reference, so you can use this method to modify the
		axis. Axis is identified by omc3daxis_X/Y/Z or omc3daxis_Pitch/Yaw/Roll that are synonimous
		of the X/Y/Z constants. */

	inline OMedia3DPoint &get_axis(omt_3DAxis a) {return axis[a];}

	/** Returns transformed absolute angles. These are inversed angles. Use this method to convert
		angles to camera orientation. */

	void inv_convert(omt_Angle &anglex, omt_Angle &angley, omt_Angle &anglez)
	{	
		float x = 	omd_Abs( axis[omc3daxis_Z].y );
		omt_Angle rx,ry,rz;

		if ( x > 1.0 ) axis[omc3daxis_Z].y /= x;

		rx = omd_ASin(-axis[omc3daxis_Z].y); 

		if( omd_Cos(rx) != 0 )  
		{
			rz = omd_ATan2( axis[omc3daxis_X].y, axis[omc3daxis_Y].y );
			ry = omd_ATan2( axis[omc3daxis_Z].x, axis[omc3daxis_Z].z );
		}
		else 
		{
			ry= -omd_ATan2( axis[omc3daxis_X].z, axis[omc3daxis_X].x );
			rz = 0;
		}

		anglex = rx; 
		angley = ry; 
		anglez = -rz;
	}

	/** Returns transformed absolute angles.*/

	void convert(omt_Angle &anglex, omt_Angle &angley, omt_Angle &anglez)
	{	
		float x = 	omd_Abs( axis[omc3daxis_Z].y );
		omt_Angle rx,ry,rz;

		if ( x > 1.0 ) axis[omc3daxis_Z].y /= x;

		rx = omd_ASin(-axis[omc3daxis_Z].y); 

		if( omd_Cos(rx) != 0)  
		{
			rz = omd_ATan2( -axis[omc3daxis_X].y, axis[omc3daxis_Y].y );
			ry = omd_ATan2( axis[omc3daxis_Z].x, axis[omc3daxis_Z].z );
		}
		else 
		{
			ry= -omd_ATan2( axis[omc3daxis_X].z, axis[omc3daxis_X].x );
			rz = 0;
		}

		anglex = -rx; 
		angley = -ry; 
		anglez = rz;
	}
	
	/** Rotate around one axis. You can specify the axis using one of the omc3daxis_X/Y/Z constant.
		The rotation is relative to the specified axis. The passed angle is in OMT custom angles.*/

	void rotate(omt_3DAxis a, omt_Angle angle)
	{
		float		*sin_tab, *cos_tab;
		float 	x,y,z;
		float 	c, s;

		if (!angle) return;
		angle&=omc_MaxAngleMask;

		cos_tab = OMediaCosSinTable::global.get_costab();
		sin_tab = OMediaCosSinTable::global.get_sintab();

		c  = cos_tab[angle];
		s = sin_tab[angle];
	
		switch(a)
		{
			case	omc3daxis_X:			// Pitch
			x = (axis[omc3daxis_Z].x * c) - (axis[omc3daxis_Y].x * s);
			y = (axis[omc3daxis_Z].y * c) - (axis[omc3daxis_Y].y * s);
			z = (axis[omc3daxis_Z].z * c) - (axis[omc3daxis_Y].z * s);

			axis[omc3daxis_Y].x = (axis[omc3daxis_Z].x * s) + (axis[omc3daxis_Y].x * c);
			axis[omc3daxis_Y].y = (axis[omc3daxis_Z].y * s) + (axis[omc3daxis_Y].y * c);
			axis[omc3daxis_Y].z = (axis[omc3daxis_Z].z * s) + (axis[omc3daxis_Y].z * c);

			axis[omc3daxis_Z].x = x;
			axis[omc3daxis_Z].y = y;
			axis[omc3daxis_Z].z = z;
			break;


			case	omc3daxis_Z:			// Roll
			x = (axis[omc3daxis_X].x * c) + (axis[omc3daxis_Y].x * s);
			y = (axis[omc3daxis_X].y * c) + (axis[omc3daxis_Y].y * s);
			z = (axis[omc3daxis_X].z * c) + (axis[omc3daxis_Y].z * s);

			axis[omc3daxis_Y].x = (axis[omc3daxis_Y].x * c) - (axis[omc3daxis_X].x * s);
			axis[omc3daxis_Y].y = (axis[omc3daxis_Y].y * c) - (axis[omc3daxis_X].y * s);
			axis[omc3daxis_Y].z = (axis[omc3daxis_Y].z * c) - (axis[omc3daxis_X].z * s);
			
			axis[omc3daxis_X].x = x;
			axis[omc3daxis_X].y = y;
			axis[omc3daxis_X].z = z;
			break;


			case	omc3daxis_Y:			// Yaw
			x = (axis[omc3daxis_X].x * c) - (axis[omc3daxis_Z].x * s);
			y = (axis[omc3daxis_X].y * c) - (axis[omc3daxis_Z].y * s);
			z = (axis[omc3daxis_X].z * c) - (axis[omc3daxis_Z].z * s);

			axis[omc3daxis_Z].x = (axis[omc3daxis_X].x * s) + (axis[omc3daxis_Z].x * c);
			axis[omc3daxis_Z].y = (axis[omc3daxis_X].y * s) + (axis[omc3daxis_Z].y * c);
			axis[omc3daxis_Z].z = (axis[omc3daxis_X].z * s) + (axis[omc3daxis_Z].z * c);

			axis[omc3daxis_X].x = x;
			axis[omc3daxis_X].y = y;
			axis[omc3daxis_X].z = z;
			break;
		}
	}

	
	/** Reset axises to default orientation. 
	
		The default orientation is:

				X vector 1,0,0
				Y vector 0,1,0
				Z vector 0,0,1
	*/

	inline void reset_axis(void)
	{
		axis[omc3daxis_X].x = 1;	axis[omc3daxis_X].y = 0; axis[omc3daxis_X].z = 0;
		axis[omc3daxis_Y].x = 0;	axis[omc3daxis_Y].y = 1; axis[omc3daxis_Y].z = 0;
		axis[omc3daxis_Z].x = 0;	axis[omc3daxis_Z].y = 0; axis[omc3daxis_Z].z = 1;
	}
	

	protected:


	OMedia3DPoint		axis[3];
};



#endif

