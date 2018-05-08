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
#ifndef OMEDIA_MathTools_H
#define OMEDIA_MathTools_H

#include <math.h>

#include "OMediaTypes.h"
#include "OMedia3DPoint.h"
#include "OMediaMatrix.h"

class OMediaProjectPointUV
{
public:

	// In/out:
	float	x,y,z,w;
	float	u,v;
	
	// Out:
	float	inv_w;
	
	inline void transform_homogenous_coord(void)
	{
		if (w!=1.0f)
		{	
			float iw = 1.0f/w;
	
			inv_w = iw;	
			x *= iw;
			y *= iw;
			u *= iw;
			v *= iw;
		}
		else inv_w = 1.0f;
	}
};

class OMediaMathTools
{
	public:
	

	omtshared static bool line_intersection(float ax1, float ay1, 	// Returns false if no intersection point
								    float ax2, float ay2, 
								    float bx1, float by1, 
								    float bx2, float by2, 
								    float &xi, float &yi);

	omtshared static bool segment_intersection(float ax1, float ay1, 	
					 float ax2, float ay2, 
					 float bx1, float by1, 
					 float bx2, float by2, 
					 float &xi, float &yi);

	omtshared static bool segment_intersection_for(float ax1, float ay1, 	
					 float ax2, float ay2, 
					 float bx1, float by1, 
					 float bx2, float by2, 
					 float xi, float yi);


	omtshared static bool is_point_in_polygon(	OMedia3DPoint *poly_array,	// Returns true if the
												long poly_length,			// point is enclosed
												OMedia3DPoint *test_point,	// on x/y axis. 
												float &out_z);				// Then it returns
										  									// the test_point z projected
										  									// on the passed polygon.
										  									// If out_z==test_point.z
										  									// the point is on the polygon
										  									// in 3D space.

	omtshared static bool project_point_uv(		OMediaProjectPointUV	*b,			// vertex array
											long					poly_length,	// number of vertex
											float					px,				// projected point
											float					py,				//   it must be normalized to (-1<p<1)
											OMediaMatrix_4x4		&model_matrix,	// model matrix
											OMediaMatrix_4x4		&proj_matrix,	// projection matrix
											float					&out_z,			// z result
											float					&out_u,			// u/v result
											float					&out_v,
											float					&out_invw,		// inv_w result
											bool					clip_outside =false);

	omtshared static float tri_area(const float	*V0,	const float	*V1,	const float	*V2);
	omtshared static float tri_area_square(const float	*V0,	const float	*V1,	const float	*V2);


	omtshared static bool tri_tri_quick_reject(const float	*V0,	const float	*V1,	const float	*V2,
												const float	*U0,	const float	*U1,	const float	*U2);


	omtshared static bool tri_tri_intersect(const float	*V0,	const float	*V1,	const float	*V2,
											const float	*U0,	const float	*U1,	const float	*U2);

	omtshared static bool tri_sphere_intersect(const OMedia3DPoint	&V0,	
												const OMedia3DPoint	&V1,	
												const OMedia3DPoint	&V2,
												const OMedia3DPoint	&SP,	
												const float SR);

	omtshared static bool tri_ray_intersect(const float *ray_orig, const float *ray_dir,
											const float *vert0, const float *vert1, const float *vert2,
											float *out_distance, float *out_u, float *out_v);

	omtshared static bool sphere_ray_intersect (const float *ray_origin, 
											const float *ray_direction,
											const float *sphere_center, 
											const float sphere_radius, 
											short		&out_nhits, 
											float		*out_distances);

	omtshared static void find_plane(const float *normal_v,
									 const float *plane_pnt,
									 float *out_plane_v4);

	omtshared static bool plane_line_intersect(const float *l_org, 
											 const float *l_vec,
											 const float *plane_v4,
											 float *out_dest);


};


#endif

