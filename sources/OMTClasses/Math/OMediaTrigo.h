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
#ifndef OMEDIA_Trigo_H
#define OMEDIA_Trigo_H

#include "OMediaTypes.h"

const unsigned long acos_table_size = 0x4000;
const unsigned long asin_table_size = 0x4000;

/** Angle.

	OMT uses 14bits angle precision for fast trigonometric calculation. The
	omt_Angle defines a standard OMT angle. If you need to translate degrees
	to OMT angles you should use the omd_Deg2Angle macro. */

typedef short omt_Angle;

const unsigned long omc_MaxAngleBit = 14;
const long omc_MaxAngle = 1<<omc_MaxAngleBit;
const unsigned short omc_MaxAngleMask = omc_MaxAngle-1;

/** Convert degree to OMT angle format. */
#define omd_Deg2Angle(x) (omt_Angle((((long(x))<<omc_MaxAngleBit)/360L)))
#define omd_Deg2AngleF(x) (float((((long(x))<<omc_MaxAngleBit)/360.0f)))

/** Convert OMT angle format to degree. */
#define omd_Angle2Deg(x) (omt_Angle((((long(x))*360L)>>omc_MaxAngleBit)))
#define omd_Angle2DegF(x) (float((((long(x))*360L)/ float(1<<omc_MaxAngleBit) )))


/** Support for fast trigonometric calculations.

	The OMediaCosSinTable is a global class. You should not have to build it.
	You can access the object using OMediaCosSinTable::global static field.

	OMT uses precomputed tables to optimize trigonometric calculation.

	Most of the time you should use the following global defines for trigonometric
	operation: omd_Cos, omd_Sin, omd_ASin and omd_ATan2.
*/

class OMediaCosSinTable
{
	public:
	
	omtshared OMediaCosSinTable();

	omtshared void init_table(void);

	//** Compute a cos using 14bits precision table. Same as using the omd_Cos macro*/
	inline float cos(const omt_Angle angle) const {return cos_tab[angle&omc_MaxAngleMask];}
	
	//** Compute a sin using 14bits precision table. Same as using the omd_Sin macro*/
	inline float sin(const omt_Angle angle) const {return sin_tab[angle&omc_MaxAngleMask];}

	//** Compute an asin using a precomputed table. Same as using the omd_ASin macro*/
	omt_Angle asin(float i)
	{
		if (i>1.0 || i<-1.0) return 0;
		
		if (i<0) return  -asin_tab[ long(-i*asin_table_size)];
		else return asin_tab[ long(i*asin_table_size)];
	}

	//** Compute an atan2 using a precomputed table. Same as using the omd_ATan2 macro*/
	omt_Angle atan2(float opp, float adj)
	{
		float		adj2,hyp;
		long		t;
		omt_Angle	angle;
		
		if (opp==0.0 && adj==0.0) return 0;
	
		adj2=adj*adj;
		hyp=adj2+opp*opp;
		t=long(adj2*acos_table_size/hyp);	
	
		angle = acos_tab[t];

		if (adj<0)
		{
			if (opp<0)	return omd_Deg2Angle(180)+angle;	
			else return omd_Deg2Angle(180)-angle;
		}
		else
		{
			if (opp<0)	return omd_Deg2Angle(360)-angle;
			else	return angle;	
		}
	}



	omtshared static OMediaCosSinTable	global;

	inline float *get_costab(void) {return cos_tab;}
	inline float *get_sintab(void) {return sin_tab;}
	inline omt_Angle *get_acostab(void) {return acos_tab;}
	inline omt_Angle *get_acosntab(void) {return acos_ntab;}

	protected:
	float sin_tab[omc_MaxAngle+1];
	float cos_tab[omc_MaxAngle+1];
	omt_Angle acos_tab[acos_table_size+1];
	omt_Angle acos_ntab[acos_table_size+1];
	omt_Angle asin_tab[asin_table_size+1];
	
};

//** Compute a cos using a precomputed table.*/
#define omd_Cos(x) OMediaCosSinTable::global.cos(x)

//** Compute a cos using a precomputed table.*/
#define omd_Sin(x) OMediaCosSinTable::global.sin(x)

//** Compute an asin using a precomputed table.*/
#define omd_ASin(x) OMediaCosSinTable::global.asin(x)

//** Compute an atan2 using a precomputed table.*/
#define omd_ATan2(x,y) OMediaCosSinTable::global.atan2(x,y)

#endif

