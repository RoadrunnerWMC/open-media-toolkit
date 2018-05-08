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
 

#include "OMediaTrigo.h"

#include <math.h>


#ifndef pi

	#ifndef PI
	#define omt_pi 3.1415926535
	#else
	#define omt_pi PI
	#endif

#else
#define omt_pi pi
#endif


OMediaCosSinTable	OMediaCosSinTable::global;

OMediaCosSinTable::OMediaCosSinTable()
{
	init_table();
}


void OMediaCosSinTable::init_table(void)
{
	omt_Angle	angle,l;
	double	rad,acc,actsize = acos_table_size;

	for(angle = 0; angle <=omc_MaxAngle; angle++)
	{
		rad =  omt_pi * double(angle) / (omc_MaxAngle/2);

		sin_tab[angle] = (float)::sin(rad);
		cos_tab[angle] = (float)::cos(rad);
	}

	sin_tab[omd_Deg2Angle(90)] = 1.0;	// Polish results
	cos_tab[omd_Deg2Angle(90)] = 0;
	sin_tab[omd_Deg2Angle(180)] = 0.0;
	cos_tab[omd_Deg2Angle(180)] = -1.0;
	sin_tab[omd_Deg2Angle(270)] = -1.0;
	cos_tab[omd_Deg2Angle(270)] = 0;
	sin_tab[omd_Deg2Angle(360)] = sin_tab[0];
	cos_tab[omd_Deg2Angle(360)] = cos_tab[0];

	
	for(l=0;l<(long)acos_table_size;l++)
	{
		acc = ::acos((l/(actsize/2))-1) * ((float)omc_MaxAngle/2) / omt_pi;
		acos_tab[l] = short(acc/2);
		acos_ntab[l] = short(acc);
	}
	
	for(l=0;l<(long)asin_table_size;l++)
	{
		asin_tab[l] = short(::asin(l/actsize) * ((float)omc_MaxAngle/2) / omt_pi);
	}

	acos_tab[l] = 0;
	acos_ntab[l] = 0;	
	asin_tab[l] = omd_Deg2Angle(90);	
}

