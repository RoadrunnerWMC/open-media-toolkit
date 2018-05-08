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
#ifndef OMEDIA_Matrix_H
#define OMEDIA_Matrix_H

#include <math.h>

#include "OMediaTypes.h"
#include "OMedia3DVector.h"
#include "OMedia3DPoint.h"

// By default a matrix is standard. Using hint can speed up transformation.
// If you change the matrix contain by hand you may have to reset the hint.


enum omt_MatrixHint
{	
	ommc_Standard,	
	ommc_Identity,
	ommc_OrthoProjection, 
	ommc_Projection,
	ommc_Translate,
	ommc_Rotate,
	ommc_RotateTranslate
};


/** 4x4 matrix support.

OMT includes only supports for 4x4 matrixes that is the most used ones in 
2D/3D animation. The matrix is defined as 16 32bits floating point values,
plus a hint value.

The value order is the same than OpenGL: matrix.m[x][y]. (That is the inverse
of a typical C array where the x value is the second component of the array).

The hint value can be used to speed up matrix calculation. If the hint value
is ommc_Standard, full matrix calculation is applied. However by specifying
another hint only part of the matrix is used to compute the final result. For
example, if a matrix has the ommc_Identity hint, multiplying a point by the matrix
will have no cost.

The hint is used by the "multiply(float xyzw[])" method that is used everywhere
in OMT for real time point transformation.

Most of the methods that set up a matrix (such as set_identity), automatically set 
for you the required hint. When you multiply two matrixes, the resulting matrix
has the ommc_Standard hint, resulting in an unoptimized matrix. If you know that the
result of the multiply fits in a matrix hint, you can set the hint yourself.

However be carefull not to use a wrong hint.
*/


class OMediaMatrix_4x4
{
	public:
	
	float 	m[4][4];		// 4x4 matrix [x][y]

	omt_MatrixHint	hint;

	// * Construction

	inline OMediaMatrix_4x4(void) {hint = ommc_Standard;} 
	inline OMediaMatrix_4x4(const OMediaMatrix_4x4 &v) { *this = v;} 


	// * Core

	/** Assign matrix. The hint is copied. */
	inline void assign(const OMediaMatrix_4x4 &v)
	{
		m[0][0] = v.m[0][0];	m[0][1] = v.m[0][1];	m[0][2] = v.m[0][2];	m[0][3] = v.m[0][3];
		m[1][0] = v.m[1][0];	m[1][1] = v.m[1][1];	m[1][2] = v.m[1][2];	m[1][3] = v.m[1][3];
		m[2][0] = v.m[2][0];	m[2][1] = v.m[2][1];	m[2][2] = v.m[2][2];	m[2][3] = v.m[2][3];
		m[3][0] = v.m[3][0];	m[3][1] = v.m[3][1];	m[3][2] = v.m[3][2];	m[3][3] = v.m[3][3];	
		
		hint = v.hint;
	}

	/** Set the identity matrix. The hint is set to ommc_Identity. */
	inline void set_identity(void)
	{
		m[0][0] = 1.0f;	m[0][1] = 0.0f;	m[0][2] = 0.0f;	m[0][3] = 0.0f;
		m[1][0] = 0.0f;	m[1][1] = 1.0f;	m[1][2] = 0.0f;	m[1][3] = 0.0f;
		m[2][0] = 0.0f;	m[2][1] = 0.0f;	m[2][2] = 1.0f;	m[2][3] = 0.0f;
		m[3][0] = 0.0f;	m[3][1] = 0.0f;	m[3][2] = 0.0f;	m[3][3] = 1.0f;
		
		hint = 	ommc_Identity;
	}


	/** Clear matrix. The hint is set to ommc_Standard. */
	inline void set_zero(void)
	{
		m[0][0] = 0.0f;	m[0][1] = 0.0f;	m[0][2] = 0.0f;	m[0][3] = 0.0f;
		m[1][0] = 0.0f;	m[1][1] = 0.0f;	m[1][2] = 0.0f;	m[1][3] = 0.0f;
		m[2][0] = 0.0f;	m[2][1] = 0.0f;	m[2][2] = 0.0f;	m[2][3] = 0.0f;
		m[3][0] = 0.0f;	m[3][1] = 0.0f;	m[3][2] = 0.0f;	m[3][3] = 0.0f;	

		hint = 	ommc_Standard;
	}

	
	/** Set up translate matrix using a 3D point. The hint is set to ommc_Translate. */
	inline void set_translate(const OMedia3DPoint &p) {set_translate(p.x,p.y,p.z);}

	/** Set up translate matrix using a 3D vector. The hint is set to ommc_Translate. */
	inline void set_translate(const OMedia3DVector &v) {set_translate(v.x,v.y,v.z);}

	/** Set up translate matrix using direct values. The hint is set to ommc_Translate. */
	inline void set_translate(float x, float y, float z)
	{
		set_identity();
		m[3][0] = x;
		m[3][1] = y;
		m[3][2] = z;

		hint = ommc_Translate;
	}
	
	// * Rotate

	/** Produces a rotate matrix of "angle" around the passed vector. The passed vector must be normalized. */
	inline void set_rotate(omt_Angle angle,
					const OMedia3DVector &v)
	{
		set_rotate(angle,v.x,v.y,v.z);
	}


	/** Produces a rotate matrix of "angle" around the passed vector. The passed vector must be normalized. */
	inline void set_rotate(omt_Angle angle, float x, float y, float z)
	{
		const float s = omd_Sin( angle );
		const float c = omd_Cos( angle );
		
		const float neg_c = 1.0f - c;
		const float xy = x*y,
					xz = x*z,
					yz = y*z,
					sz = s*z,
					sy = s*y,
					sx = s*x;		
		
	
	    m[0][0] = ( x*x ) 	* ( neg_c ) + c;
	    m[0][1] = ( xy ) 	* ( neg_c ) - (sz);
	    m[0][2] = ( xz ) * ( neg_c ) + (sy);
	
	    m[1][0] = ( xy ) * ( neg_c ) + (sz);
	    m[1][1] = ( y*y ) * ( neg_c ) + c ;
	    m[1][2] = ( yz ) * ( neg_c ) - (sx);
	
	    m[2][0] = ( xz ) * ( neg_c ) - (sy);
	    m[2][1] = ( yz ) * ( neg_c ) + (sx);
	    m[2][2] = ( z*z ) * ( neg_c ) + c;
	    
	    m[0][3] = m[1][3] = m[2][3] = m[3][0] = m[3][1] = m[3][2] = 0.0f;
	    m[3][3] = 1.0f;

		hint = ommc_Rotate;
	}

	/** Produces a rotate matrix using absolute angles. OMT uses a left-hand orientation. Default rotation order is z, x, y*/
	inline void set_rotate(omt_Angle x, omt_Angle y, omt_Angle z)
	{
		OMediaMatrix_4x4 matrix_x, matrix_y, matrix_z, temp_matrix;

		matrix_x.set_rotate_x(x);
		matrix_y.set_rotate_y(y);
		matrix_z.set_rotate_z(z);
	
		matrix_y.multiply(matrix_x ,temp_matrix);
		temp_matrix.multiply(matrix_z ,*this);	

		hint = ommc_Rotate;
	}

	/** Produces an inverse rotate matrix using absolute angles. OMT uses a left-hand orientation. Inverse rotation order is y, x, z*/
	inline void set_inv_rotate(omt_Angle x, omt_Angle y, omt_Angle z)
	{
		OMediaMatrix_4x4 matrix_x, matrix_y, matrix_z, temp_matrix;

		matrix_x.set_rotate_x(x);
		matrix_y.set_rotate_y(y);
		matrix_z.set_rotate_z(z);
	
		matrix_z.multiply(matrix_x ,temp_matrix);
		temp_matrix.multiply(matrix_y ,*this);	

		hint = ommc_Rotate;
	}

	
	/** Produces a rotate matrix of "angle" around x axis*/
	inline void set_rotate_x(omt_Angle angle)
	{
		set_identity();
		
		float cos_a = omd_Cos( angle ), sin_a = omd_Sin(angle);

	    m[1][1] =  cos_a;
    	m[1][2] =  -sin_a;
    	m[2][1] =  sin_a;
    	m[2][2] =  cos_a;	

		hint = ommc_Rotate;
	}

	/** Produces a rotate matrix of "angle" around y axis*/
	inline void set_rotate_y(omt_Angle angle)
	{
		set_identity();

		float cos_a = omd_Cos( angle ), sin_a = omd_Sin(angle);

	    m[0][0] =  cos_a;
    	m[0][2] =  sin_a;
    	m[2][0] = -sin_a;
    	m[2][2] =  cos_a;

		hint = ommc_Rotate;
	}

	/** Produces a rotate matrix of "angle" around z axis*/
	inline void set_rotate_z(omt_Angle angle)
	{
		set_identity();

		float cos_a = omd_Cos( angle ), sin_a = omd_Sin(angle);

	    m[0][0] = cos_a;
    	m[0][1] = -sin_a;
    	m[1][0] = sin_a;
    	m[1][1] = cos_a;

		hint = ommc_Rotate;
	}
	
	
	// * Scale
	
	/** Produces a scale matrix of "angle" around x axis. The hint is set to ommc_Standard.*/
	inline void set_scale(float sx, float sy, float sz)
	{
		set_identity();
	
	    m[0][0] = sx;
    	m[1][1] = sy;
    	m[2][2] = sz;

		hint = ommc_Standard;
	}

	// * Multiply

	/** Multiply two matrixes. Result is stored into "r"*/
	inline void multiply(const OMediaMatrix_4x4 &b, OMediaMatrix_4x4 &r) const
	{
   		for (short i = 0; i < 4; i++) 
   		{
    		float mi0= m[0][i],  mi1=m[1][i],  mi2=m[2][i],  mi3=m[3][i];
    		
    		r.m[0][i] = mi0 * b.m[0][0] + mi1 * b.m[0][1] + mi2 * b.m[0][2] + mi3 * b.m[0][3];
	      	r.m[1][i] = mi0 * b.m[1][0] + mi1 * b.m[1][1] + mi2 * b.m[1][2] + mi3 * b.m[1][3];
      		r.m[2][i] = mi0 * b.m[2][0] + mi1 * b.m[2][1] + mi2 * b.m[2][2] + mi3 * b.m[2][3];
      		r.m[3][i] = mi0 * b.m[3][0] + mi1 * b.m[3][1] + mi2 * b.m[3][2] + mi3 * b.m[3][3];
   		}

		r.hint = ommc_Standard;
	}

	/** Multiply a point by the matrix. */
	inline void multiply(OMedia3DPoint &p) const
	{
		multiply(p.xyzw());
	}

	/** Multiply a vector by the matrix. */
	inline void multiply(OMedia3DVector &v) const
	{
		multiply(v.xyzw());
	}

	/** Multiply a point that includes an homogenous coordinated. The result replaces the passed
		array values. This is the main method used by OMT to transform 3D points. This method uses
		the hint value to speed up the transformation. */

	inline void multiply(float xyzw[]) const
	{
		float x,y,z,w;
		
		switch(hint)
		{
			case ommc_Standard:
			x = xyzw[0] * m[0][0];
			x += xyzw[1] * m[1][0];
			x += xyzw[2] * m[2][0];
			x += xyzw[3] * m[3][0];

			y = xyzw[0] * m[0][1];
			y += xyzw[1] * m[1][1];
			y += xyzw[2] * m[2][1];
			y += xyzw[3] * m[3][1];
	
			z = xyzw[0] * m[0][2];
			z += xyzw[1] * m[1][2];
			z += xyzw[2] * m[2][2];
			z += xyzw[3] * m[3][2];

			w = xyzw[0] * m[0][3];
			w += xyzw[1] * m[1][3];
			w += xyzw[2] * m[2][3];
			w += xyzw[3] * m[3][3];
			
			xyzw[3] = w;
			break;

			case ommc_Identity:
			return;

			case ommc_OrthoProjection:
			x = xyzw[0] * m[0][0];
			x += m[3][0];

			y = xyzw[1] * m[1][1];
			y += m[3][1];

			z = xyzw[2] * m[2][2];
			z += m[3][2];
			break;

			case ommc_Projection:			
			x = xyzw[0] * m[0][0];
			y = xyzw[1] * m[1][1];
	
			z = xyzw[2] * m[2][2];
			z += m[3][2];

			w = xyzw[2] * m[2][3];
			
			xyzw[3] = w;
			break;

			case ommc_Translate:
			x = xyzw[0] + m[3][0];
			y = xyzw[1] + m[3][1];
			z = xyzw[2] + m[3][2];
			break;
			
			case ommc_Rotate:
			x = xyzw[0] * m[0][0];
			x += xyzw[1] * m[1][0];
			x += xyzw[2] * m[2][0];

			y = xyzw[0] * m[0][1];
			y += xyzw[1] * m[1][1];
			y += xyzw[2] * m[2][1];
	
			z = xyzw[0] * m[0][2];
			z += xyzw[1] * m[1][2];
			z += xyzw[2] * m[2][2];			
			break;
			
			case ommc_RotateTranslate:
			x = xyzw[0] * m[0][0];
			x += xyzw[1] * m[1][0];
			x += xyzw[2] * m[2][0];
			x += m[3][0];

			y = xyzw[0] * m[0][1];
			y += xyzw[1] * m[1][1];
			y += xyzw[2] * m[2][1];
			y += m[3][1];
	
			z = xyzw[0] * m[0][2];
			z += xyzw[1] * m[1][2];
			z += xyzw[2] * m[2][2];
			z += m[3][2];			
			break;
		}
	
		xyzw[0] = x;
		xyzw[1] = y;
		xyzw[2] = z;		
	}
	
	// * Projection
                                     
	/** Generate a perspective projection matrix based on the field of view.
		Generate z coords going from -1.0 to 1.0 */

	inline void set_perspective_fov_zn1_1(omt_Angle fov,
									float near_plane, float far_plane,
									float aspect =1.0f)
	{
		float h = ( omd_Cos(fov>>1) / omd_Sin(fov>>1) );
		float w = h*aspect;
	
		m[0][0] = w;
		m[1][1] = h;
		m[2][2] = (far_plane+near_plane) / (far_plane-near_plane);
		m[2][3] = 1.0f;
		m[3][2] = -(2.0f*far_plane*near_plane) / (far_plane-near_plane); 

						m[0][1] = 0.0f;	m[0][2] = 0.0f;	m[0][3] = 0.0f;
		m[1][0] = 0.0f;					m[1][2] = 0.0f;	m[1][3] = 0.0f;
		m[2][0] = 0.0f;	m[2][1] = 0.0f;					
		m[3][0] = 0.0f;	m[3][1] = 0.0f;					m[3][3] = 1.0f;

		hint = ommc_Projection;	
	}

	/** Generate a perspective projection matrix based on the field of view.
		Generate z coords going from 0 to 1.0 */

	inline void set_perspective_fov_z0_1(omt_Angle fov,
									float near_plane, float far_plane,
									float aspect =1.0f)
	{
		float h = ( omd_Cos(fov>>1) / omd_Sin(fov>>1) );
		float w = h*aspect;
	
		m[0][0] = w;
		m[1][1] = h;
		m[2][2] = far_plane / (far_plane-near_plane);
		m[2][3] = 1.0f;
		m[3][2] = -m[2][2]*near_plane; 

						m[0][1] = 0.0f;	m[0][2] = 0.0f;	m[0][3] = 0.0f;
		m[1][0] = 0.0f;					m[1][2] = 0.0f;	m[1][3] = 0.0f;
		m[2][0] = 0.0f;	m[2][1] = 0.0f;					
		m[3][0] = 0.0f;	m[3][1] = 0.0f;					m[3][3] = 1.0f;

		hint = ommc_Projection;	
	}


	/** Generate a perspective projection matrix based on the viewport width and height. 
		Generate z coords going from -1.0 to 1.0*/
	inline void set_perspective_frustum_zn1_1(float vp_width, float vp_height,
										float near_plane, float far_plane,
										float aspect =1.0f)
	{
		float nearp2x = (2.0f * near_plane);
		float w = aspect * (nearp2x / vp_width),
			  h = nearp2x / vp_height;
	
		m[0][0] = w;
		m[1][1] = h;
		m[2][2] = (far_plane+near_plane) / (far_plane-near_plane);
		m[2][3] = 1.0f;
		m[3][2] = -(2.0f*far_plane*near_plane) / (far_plane-near_plane); 

						m[0][1] = 0.0f;	m[0][2] = 0.0f;	m[0][3] = 0.0f;
		m[1][0] = 0.0f;					m[1][2] = 0.0f;	m[1][3] = 0.0f;
		m[2][0] = 0.0f;	m[2][1] = 0.0f;					
		m[3][0] = 0.0f;	m[3][1] = 0.0f;					m[3][3] = 1.0f;

		hint = ommc_Projection;	
	}

	/** Generate a perspective projection matrix based on the viewport width and height. 
		Generate z coords going from 0.0 to 1.0*/

	inline void set_perspective_frustum_z0_1(float vp_width, float vp_height,
										float near_plane, float far_plane,
										float aspect =1.0f)
	{
		float nearp2x = (2.0f * near_plane);
		float w = aspect * (nearp2x / vp_width),
			  h = nearp2x / vp_height;
	
		m[0][0] = w;
		m[1][1] = h;
		m[2][2] = (far_plane) / (far_plane-near_plane);
		m[2][3] = 1.0f;
		m[3][2] = -m[2][2] * near_plane; 

						m[0][1] = 0.0f;	m[0][2] = 0.0f;	m[0][3] = 0.0f;
		m[1][0] = 0.0f;					m[1][2] = 0.0f;	m[1][3] = 0.0f;
		m[2][0] = 0.0f;	m[2][1] = 0.0f;					
		m[3][0] = 0.0f;	m[3][1] = 0.0f;					m[3][3] = 1.0f;

		hint = ommc_Projection;	
	}
	
	/** Generate an orthographic projection matrix. Generate z coords going from -1.0 to 1.0*/
	inline void set_ortho_zn1_1(float left, float top, float right, float bottom, float near_plane, float far_plane)
	{
		m[0][0] = 2.0f/(right-left);
		m[1][1] = 2.0f/(bottom-top);
		m[2][2] = 2.0f/(far_plane-near_plane);

	   	m[3][0] = -(right+left)/(right-left);
	   	m[3][1] = -(bottom+top)/(bottom-top);
	   	m[3][2] = -(far_plane + near_plane )/(far_plane-near_plane);

						m[0][1] = 0.0f;	m[0][2] = 0.0f;	m[0][3] = 0.0f;
		m[1][0] = 0.0f;					m[1][2] = 0.0f;	m[1][3] = 0.0f;
		m[2][0] = 0.0f;	m[2][1] = 0.0f;					m[2][3] = 0.0f;
														m[3][3] = 1.0f;

		hint = ommc_OrthoProjection;
	}

	/** Generate an orthographic projection matrix. Generate z coords going from 0 to 1.0*/
	inline void set_ortho_z0_1(float left, float top, float right, float bottom, float near_plane, float far_plane)
	{
		m[0][0] = 2.0f/(right-left);
		m[1][1] = 2.0f/(bottom-top);
		m[2][2] = 1.0f/(far_plane-near_plane);

	   	m[3][0] = -(right+left)/(right-left);
	   	m[3][1] = -(bottom+top)/(bottom-top);
	   	m[3][2] = -(near_plane)/(far_plane-near_plane);

						m[0][1] = 0.0f;	m[0][2] = 0.0f;	m[0][3] = 0.0f;
		m[1][0] = 0.0f;					m[1][2] = 0.0f;	m[1][3] = 0.0f;
		m[2][0] = 0.0f;	m[2][1] = 0.0f;					m[2][3] = 0.0f;
														m[3][3] = 1.0f;

		hint = ommc_OrthoProjection;
	}
	

	// * Picking

	/** Generate a picking matrix. Can also be used to scale an area of the display.*/
	inline void set_pick(float x, float y, float width, float height,
						 float vpx, float vpy, float vpw, float vph)
	{
		float sx, sy;
		float tx, ty;

		sx = vpw / width;
	   	sy = vph / height;
	   	tx = (vpw + 2.0f * (vpx - x)) / width;
	   	ty = (vph + 2.0f * (vpy - y)) / height;

		#define MAT(row,col)  m[col][row]
	   	MAT(0,0) = sx;   MAT(0,1) = 0.0;  MAT(0,2) = 0.0;  MAT(0,3) = tx;
	   	MAT(1,0) = 0.0;  MAT(1,1) = sy;   MAT(1,2) = 0.0;  MAT(1,3) = ty;
	   	MAT(2,0) = 0.0;  MAT(2,1) = 0.0;  MAT(2,2) = 1.0;  MAT(2,3) = 0.0;
	   	MAT(3,0) = 0.0;  MAT(3,1) = 0.0;  MAT(3,2) = 0.0;  MAT(3,3) = 1.0;
		#undef MAT	

		hint = ommc_Standard;
	}
	


	
	// * World transform
	
	/** Generate a world transform matrix. This matrix can be used to transform a point in absolute world position to viewer position and orientation*/
	void set_world_transform(float viewx, float viewy, float viewz, omt_Angle angle_x, omt_Angle angle_y, omt_Angle angle_z)
	{
		OMediaMatrix_4x4 rotate_matrix,trans_matrix;
		
		rotate_matrix.set_inv_rotate(-angle_x,-angle_y,-angle_z);
		trans_matrix.set_translate(-viewx,-viewy,-viewz);

		rotate_matrix.multiply(trans_matrix,*this);

		hint = ommc_RotateTranslate;
	}

	// * Normal transform

	/** Use this matrix to transform a normal vector.*/
	void normal_transform(OMedia3DVector &n, OMedia3DVector &out)
	{
		float nx,ny,nz;
		#define MAT (&m[0][0])
		nx = n.x * MAT[0] + n.y * MAT[1] + n.z * MAT[2];
		ny = n.x * MAT[4] + n.y * MAT[5] + n.z * MAT[6];
		nz = n.x * MAT[8] + n.y * MAT[9] + n.z * MAT[10];
		#undef MAT

		out.x = nx;
		out.y = ny;
		out.z = nz;
	}
	
	
	
	// * Invert

	/** Produce an invert matrix.*/
	bool invert(OMediaMatrix_4x4 &out) const
	{
		#define SWAP_ROWS(a, b) { float *_tmp = a; (a)=(b); (b)=_tmp; }
		#define MAT(mat,r,c) mat[c][r]
		
		 float wtmp[4][8];
		 float m0, m1, m2, m3, s;
		 float *r0, *r1, *r2, *r3;
		
		 r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];
		
		 r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
		 r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
		 r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
		
		 r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
		 r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
		 r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
		
		 r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
		 r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
		 r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
		
		 r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
		 r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
		 r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;
		
		 if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
		 if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
		 if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
		 if (0.0 == r0[0])  return false;
		
		 m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
		 s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
		 s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
		 s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
		 s = r0[4];
		 if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
		 s = r0[5];
		 if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
		 s = r0[6];
		 if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
		 s = r0[7];
		 if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }
		
		 if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
		 if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
		 if (0.0 == r1[1])  return false;
		
		 m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
		 r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
		 r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
		 s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
		 s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
		 s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
		 s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }
		
		 if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
		 if (0.0 == r2[2])  return false;
		
		 m3 = r3[2]/r2[2];
		 r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
		 r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
		 r3[7] -= m3 * r2[7];
		
		 if (0.0f == r3[3]) return false;
		
		 s = 1.0f/r3[3];              
		 r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;
		
		 m2 = r2[3];                 
		 s  = 1.0f/r2[2];
		 r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
		 r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
		 m1 = r1[3];
		 r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
		 r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
		 m0 = r0[3];
		 r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
		 r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;
		
		 m1 = r1[2];                 
		 s  = 1.0f/r1[1];
		 r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
		 r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
		 m0 = r0[2];
		 r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
		 r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;
		
		 m0 = r0[1];                 
		 s  = 1.0f/r0[0];
		 r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
		 r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

		 out.hint =  ommc_Standard;
		 MAT(out.m,0,0) = r0[4]; MAT(out.m,0,1) = r0[5],
		 MAT(out.m,0,2) = r0[6]; MAT(out.m,0,3) = r0[7],
		 MAT(out.m,1,0) = r1[4]; MAT(out.m,1,1) = r1[5],
		 MAT(out.m,1,2) = r1[6]; MAT(out.m,1,3) = r1[7],
		 MAT(out.m,2,0) = r2[4]; MAT(out.m,2,1) = r2[5],
		 MAT(out.m,2,2) = r2[6]; MAT(out.m,2,3) = r2[7],
		 MAT(out.m,3,0) = r3[4]; MAT(out.m,3,1) = r3[5],
		 MAT(out.m,3,2) = r3[6]; MAT(out.m,3,3) = r3[7]; 
		
		 return true;
		
		#undef MAT
		#undef SWAP_ROWS
	}
		
	

	// * Operators

	inline OMediaMatrix_4x4 &operator=(const OMediaMatrix_4x4 &v) 
	{ 
		m[0][0] = v.m[0][0];	m[0][1] = v.m[0][1];	m[0][2] = v.m[0][2];	m[0][3] = v.m[0][3];
		m[1][0] = v.m[1][0];	m[1][1] = v.m[1][1];	m[1][2] = v.m[1][2];	m[1][3] = v.m[1][3];
		m[2][0] = v.m[2][0];	m[2][1] = v.m[2][1];	m[2][2] = v.m[2][2];	m[2][3] = v.m[2][3];
		m[3][0] = v.m[3][0];	m[3][1] = v.m[3][1];	m[3][2] = v.m[3][2];	m[3][3] = v.m[3][3];	
		
		hint = v.hint;
		
		return *this;
	}


};




#endif

