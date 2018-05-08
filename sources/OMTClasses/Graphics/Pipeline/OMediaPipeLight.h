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
#ifndef OMEDIA_PipeLight_H
#define OMEDIA_PipeLight_H

#include "OMediaTypes.h"
#include "OMedia3DPoint.h"
#include "OMedia3DVector.h"
#include "OMediaRGBColor.h"
#include "OMediaRendererInterface.h"


class OMediaPipeLight
{
	public:
	
	OMediaPipeLight()
	{
		enabled = false;
		type = omclt_Directional;
		pos.set(0.0f,0.0f,0.0f);
		vect.set(0.0f,0.0f,1.0f);
		ambient.set(1.0f,0.0f,0.0f,0.0f);
		diffuse.set(1.0f,1.0f,1.0f,1.0f);
		specular.set(1.0f,1.0f,1.0f,1.0f);
		spot_exponent = 0.0f;
		spot_cutoff = omd_Deg2Angle(90);
		range = 200;
		constant_attenuation = 1.0f;
		linear_attenuation = 0.0f; 
		quadratic_attenuation = 0.0f;
	}
	
	bool				enabled;

	omt_LightType		type;

	OMedia3DPoint		pos;
	OMedia3DVector		vect;
	
	OMediaFARGBColor	ambient,diffuse,specular;
	float				constant_attenuation, linear_attenuation, quadratic_attenuation;
	float				range;
	
	float				spot_exponent;
	omt_Angle			spot_cutoff;

};


#endif
