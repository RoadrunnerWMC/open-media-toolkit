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
 
 
#include "OMedia3DVector.h"
#include "OMediaTrigo.h"
#include "OMediaError.h"

void OMedia3DVector::angles(omt_Angle &ax, omt_Angle &ay)
{
	OMedia3DVector v(x,y,z);

	ay = (-omd_ATan2(v.x,v.z))&omc_MaxAngleMask;
	v.rotate(0,-ay,0);
	ax = ((-omd_ATan2(v.z,v.y))+omd_Deg2Angle(90))&omc_MaxAngleMask;	
	if (ax>omc_MaxAngle>>1) ax = (short)-(omc_MaxAngle-ax);
}


float OMedia3DVector::quick_magnitude(void) const
{
	float	adj,hyp,cx,cy,cz;
	float	swap;
	omt_Angle	angle,s;
	omt_Angle  *acos_tab = OMediaCosSinTable::global.get_acostab();
	float	*sin_tab = OMediaCosSinTable::global.get_sintab();
	
	cx = x;
	cy = y;
	cz = z;
	
	if (cx<0) cx=-cx;	if (cy<0) cy=-cy;	if (cz<0) cz=-cz;	// Requires abs values

	if (cx>cy)
	{
		if (cx>cz) {swap=cz; cz=cx; cx=swap;}
	}
	else
	{
		if (cy>cz) {swap=cz; cz=cy; cy=swap;}
	}

	adj = (cx*cx) + (cy*cy);
	hyp = adj+(cz*cz);
	
	s = (short)(adj*acos_table_size/hyp);
	s &= acos_table_size-1;

#ifdef _DEBUG
	if (s>acos_table_size) omd_EXCEPTION(omcerr_MathError);
#endif
	
	angle = acos_tab[s];

#ifdef _DEBUG
	if (angle>omc_MaxAngle) omd_EXCEPTION(omcerr_MathError);
#endif
	
	return cz/sin_tab[angle&omc_MaxAngleMask];
}

short OMedia3DVector::angle_between(const OMedia3DVector &v) const
{
	omt_Angle  	*acos_tab = OMediaCosSinTable::global.get_acosntab();
	float		m1,m2,nm,dp;
	long		l;
	
	m1 = quick_magnitude();
	m2 = v.quick_magnitude();
	
	nm = m1*m2;

	dp = dot_product(v);
	
	l = long(((dp/nm)+1) * (acos_table_size>>1));
	if(l==(long)acos_table_size) return 0;	
	l &= acos_table_size-1;
	
	return acos_tab[l];
}




