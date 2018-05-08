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
 

#include "OMediaMathTools.h"
#include "OMediaTrigo.h"

#define omd_FLT_EPSILON 1.19209290e-07f        

inline static float omf_Dot (OMedia3DPoint &p, OMedia3DPoint &q)
{
    return p.xyzw()[0]*q.xyzw()[0]+p.xyzw()[1]*q.xyzw()[1]+p.xyzw()[2]*q.xyzw()[2];
}


static bool pt_in_segment(float ax1, float ay1, 	
					 	float ax2, float ay2,
					 	float xi, float yi);

static short linter_special_case(float ax1, float ay1, 	
						          float ax2, float ay2, 
							  float bx1, float by1, 
							  float bx2, float by2, 
							  float &xi, float &yi);




static short linter_special_case(float ax1, float ay1, 	
						          float ax2, float ay2, 
							  float bx1, float by1, 
							  float bx2, float by2, 
							  float &xi, float &yi)
{
	if (ay2==ay1)
	{
		if (ax2==ax1) return 0;	// It's a point, return false
	
		if (by2==by1)
		{
			if (ay1==by1) 	// Lines are on the same y 
			{
				if (ax1>=bx1 && ax1<= bx2) xi = ax1;
				else if (ax2>=bx1 && ax2<= bx2) xi = ax2;
				else if (bx1>=ax1 && bx1<= ax2) xi = bx1;
				else xi = bx2;
				yi = ay1;
				return 1;
			}
			else return 0;
		}
	
		yi = ay1;
		xi = bx1 +  ((yi - by1) / (by2 - by1)) * (bx2- bx1);
		return 1;
	}
	
	return 2;
}


bool OMediaMathTools::line_intersection(float ax1, float ay1, 	
								    float ax2, float ay2, 
								    float bx1, float by1, 
								    float bx2, float by2, 
								    float &xi, float &yi)
{
	float		ma,mb;

	// I process special cases first:
	
	switch(linter_special_case(ax1, ay1, ax2, ay2, bx1, by1, bx2, by2, xi, yi))
	{
		case 0: return false;
		case 1: return true;
	}

	switch(linter_special_case(ay1, ax1, ay2, ax2, by1, bx1, by2, bx2, yi, xi))
	{
		case 0: return false;
		case 1: return true;
	}

	switch(linter_special_case(bx1, by1, bx2, by2, ax1, ay1, ax2, ay2, xi, yi))
	{
		case 0: return false;
		case 1: return true;
	}

	switch(linter_special_case(by1, bx1, by2, bx2, ay1, ax1, ay2, ax2, yi, xi))
	{
		case 0: return false;
		case 1: return true;
	}


	// No special case, just standard math:
	
	ma = (ay2-ay1)/(ax2-ax1);
	mb = (by2-by1)/(bx2-bx1);

	xi =((ma*ax1) - (mb*bx1) + (by1 - ay1)) / (ma - mb);	
	yi = ma * (xi - ax1) + ay1;
	
	return true;
}

static inline bool pt_in_segment(float ax1, float ay1, 	
					 	float ax2, float ay2,
					 	float xi, float yi)
{
	float	swap;

	if (ax1==xi)
	{
		if (ay1==yi) return true;
	
		if (ay1>ay2)
		{
			swap = ay1;
			ay1 = ay2;
			ay2 = swap;
		}
		
		return (yi>=ay1 && yi<=ay2);
	}
	else if (ay1==yi)
	{
		if (ax1>ax2)
		{
			swap = ax1;
			ax1 = ax2;
			ax2 = swap;
		}

		return (xi>=ax1 && xi<=ax2);
	}

	if (ax1>ax2)
	{
		swap = ax1;
		ax1 = ax2;
		ax2 = swap;
	}

	if (ay1>ay2)
	{
		swap = ay1;
		ay1 = ay2;
		ay2 = swap;
	}

	return (xi>=ax1 && xi<=ax2 && yi>=ay1 && yi<=ay2);
}					 	

bool OMediaMathTools::segment_intersection(float ax1, float ay1, 	
					 float ax2, float ay2, 
					 float bx1, float by1, 
					 float bx2, float by2, 
					 float &xi, float &yi)
{

	if (!line_intersection(ax1, ay1, ax2, ay2, bx1, by1, bx2, by2, xi, yi)) return false;

	return (pt_in_segment(ax1, ay1, ax2, ay2,xi, yi) && pt_in_segment(bx1, by1, bx2, by2,xi, yi));
}

bool OMediaMathTools::segment_intersection_for(float ax1, float ay1, 	
					 float ax2, float ay2, 
					 float bx1, float by1, 
					 float bx2, float by2, 
					 float xi, float yi)
{
	return (pt_in_segment(ax1, ay1, ax2, ay2,xi, yi) && pt_in_segment(bx1, by1, bx2, by2,xi, yi));
}

bool OMediaMathTools::is_point_in_polygon(OMedia3DPoint *b, 			
										  long poly_length,	
										  OMedia3DPoint *a,	
										  float &bz)
{
	float dx,dy;
	short seg1_1, seg1_2, seg2_1, seg2_2,b1,b2;
	float x1,z1,x2,z2,para,y1,y2;

	dx = a->x;
	dy = a->y;
	
	seg1_1 = -1;
	seg2_1 = -1;
	
	for (b1=0,b2=1; b1<poly_length;b1++,b2++)
	{	
		if (b2==poly_length) b2=0;

		if (b[b1].y==b[b2].y) continue;

		if (b[b1].y<b[b2].y) {y1 = b[b1].y; y2 = b[b2].y;}
		else { y1 = b[b2].y; y2 = b[b1].y;}

		if (dy>= y1 && dy<= y2)
		{
			if (seg1_1==-1) {seg1_1 = b1; seg1_2 = b2;}
			else {seg2_1 = b1; seg2_2 = b2; break;}
		}
	}
	
	if (seg1_1==-1 || seg2_1==-1) return false;	// Outside

	para = (dy - b[seg1_1].y) / (b[seg1_2].y - b[seg1_1].y);
	x1 = b[seg1_1].x + para * (b[seg1_2].x - b[seg1_1].x);
	z1 = b[seg1_1].z + para * (b[seg1_2].z - b[seg1_1].z);
   
   	para = (dy - b[seg2_1].y) / (b[seg2_2].y - b[seg2_1].y);
	x2 = b[seg2_1].x + para * (b[seg2_2].x - b[seg2_1].x);
	z2 = b[seg2_1].z + para * (b[seg2_2].z - b[seg2_1].z);
 
	if (x1<x2)
	{
		if (dx<x1 || dx>x2) return false;	// Ouside 	
	}
	else if (x1>x2)
	{
		if (dx<x2 || dx>x1) return false;	// Ouside
	}
	else 
	{
		if (x1==dx)
		{
			bz = z2;
			return true;	// Inside
		}
	
		return  false;	// Ouside
	}
	
	// Inside
	
	bz = ((x2-dx) * z1 + (dx-x1) * z2) / (x2-x1);
	
	return true;
}


bool OMediaMathTools::project_point_uv(		OMediaProjectPointUV	*b,			
											long					poly_length,
											float					px,			
											float					py,			
											OMediaMatrix_4x4		&view_matrix,
											OMediaMatrix_4x4		&proj_matrix,
											float					&bz,
											float					&bu,
											float					&bv,
											float					&binvw,
											bool					clip_outside)
{
	long seg1_1, seg1_2, seg2_1, seg2_2,b1,b2,p;
	float x1,z1,x2,z2,para,y1,y2,u1,u2,v1,v2,iw1,iw2;


	// Transform

	for(p=0;p<poly_length;p++)
	{
		view_matrix.multiply(&b[p].x);
		proj_matrix.multiply(&b[p].x);
		b[p].transform_homogenous_coord();
	}

	seg1_1 = -1;
	seg2_1 = -1;
	
	for (b1=0,b2=1; b1<poly_length;b1++,b2++)
	{	
		if (b2==poly_length) b2=0;

		if (b[b1].y==b[b2].y) continue;

		if (b[b1].y<b[b2].y) {y1 = b[b1].y; y2 = b[b2].y;}
		else { y1 = b[b2].y; y2 = b[b1].y;}

		if ((py>= y1 && py<= y2) || !clip_outside)
		{
			if (seg1_1==-1) {seg1_1 = b1; seg1_2 = b2;}
			else {seg2_1 = b1; seg2_2 = b2; break;}
		}
	}
	
	if (seg1_1==-1 || seg2_1==-1) return false;	// Outside


	para = (py - b[seg1_1].y) / (b[seg1_2].y - b[seg1_1].y);
	x1 = b[seg1_1].x + para * (b[seg1_2].x - b[seg1_1].x);
	z1 = b[seg1_1].z + para * (b[seg1_2].z - b[seg1_1].z);
	u1 = b[seg1_1].u + para * (b[seg1_2].u - b[seg1_1].u);
	v1 = b[seg1_1].v + para * (b[seg1_2].v - b[seg1_1].v);
	iw1 = b[seg1_1].inv_w + para * (b[seg1_2].inv_w - b[seg1_1].inv_w);
   
   	para = (py - b[seg2_1].y) / (b[seg2_2].y - b[seg2_1].y);
	x2 = b[seg2_1].x + para * (b[seg2_2].x - b[seg2_1].x);
	z2 = b[seg2_1].z + para * (b[seg2_2].z - b[seg2_1].z);
	u2 = b[seg2_1].u + para * (b[seg2_2].u - b[seg2_1].u);
	v2 = b[seg2_1].v + para * (b[seg2_2].v - b[seg2_1].v); 
	iw2 = b[seg2_1].inv_w + para * (b[seg2_2].inv_w - b[seg2_1].inv_w);
 
	if (x1<x2)
	{
		if ((px<x1 || px>x2) && clip_outside) return false;	// Ouside 	
	}
	else if (x1>x2)
	{
		if ((px<x2 || px>x1) && clip_outside) return false;	// Ouside
	}
	else 
	{
		if (x1==px)
		{
			bz = z2;
			return true;	// Inside
		}
	
		return  false;	// Ouside
	}
	
	// Inside
		
	binvw = ((x2-px) * iw1 + (px-x1) * iw2) / (x2-x1);
	
	bz = ((x2-px) * z1 + (px-x1) * z2) / (x2-x1);
	bu = ((x2-px) * u1 + (px-x1) * u2) / (x2-x1);
	bv = ((x2-px) * v1 + (px-x1) * v2) / (x2-x1);
	
	bz *= binvw;
	bu /= binvw;
	bv /= binvw;

	return true;
}

//----------------------------------------------------------------------

/* Triangle to triangle collision
 * Based on Tomas Moller's article:
 * "A Fast Triangle-Triangle Intersection Test"
 */

/* if omd_USE_omd_EPSILON_TEST is true then we do a check:
         if |dv|<omd_EPSILON then dv=0.0;
   else no check is done (which is less robust)
*/
#define omd_USE_EPSILON_TEST TRUE
#define omd_EPSILON 0.000001f

/* some macros */
#define omd_CROSS(dest,v1,v2)                      \
              dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
              dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
              dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define omd_DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define omd_SUB(dest,v1,v2)          \
            dest[0]=v1[0]-v2[0]; \
            dest[1]=v1[1]-v2[1]; \
            dest[2]=v1[2]-v2[2];

/* omd_SORT so that a<=b */
#define omd_SORT(a,b)       \
             if(a>b)    \
             {          \
               float c; \
               c=a;     \
               a=b;     \
               b=c;     \
             }

#define omd_ISECT(VV0,VV1,VV2,D0,D1,D2,omd_ISECT0,omd_ISECT1) \
              omd_ISECT0=VV0+(VV1-VV0)*D0/(D0-D1);    \
              omd_ISECT1=VV0+(VV2-VV0)*D0/(D0-D2);

#define omd_COMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,omd_ISECT0,omd_ISECT1) \
  if(D0D1>0.0f)                                         \
  {                                                     \
    /* here we know that D0D2<=0.0 */                   \
    /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
    omd_ISECT(VV2,VV0,VV1,D2,D0,D1,omd_ISECT0,omd_ISECT1);          \
  }                                                     \
  else if(D0D2>0.0f)                                    \
  {                                                     \
    /* here we know that d0d1<=0.0 */                   \
    omd_ISECT(VV1,VV0,VV2,D1,D0,D2,omd_ISECT0,omd_ISECT1);          \
  }                                                     \
  else if(D1*D2>0.0f || D0!=0.0f)                       \
  {                                                     \
    /* here we know that d0d1<=0.0 or that D0!=0.0 */   \
    omd_ISECT(VV0,VV1,VV2,D0,D1,D2,omd_ISECT0,omd_ISECT1);          \
  }                                                     \
  else if(D1!=0.0f)                                     \
  {                                                     \
    omd_ISECT(VV1,VV0,VV2,D1,D0,D2,omd_ISECT0,omd_ISECT1);          \
  }                                                     \
  else if(D2!=0.0f)                                     \
  {                                                     \
    omd_ISECT(VV2,VV0,VV1,D2,D0,D1,omd_ISECT0,omd_ISECT1);          \
  }                                                     \
  else                                                  \
  {                                                     \
    /* triangles are coplanar */                        \
    return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2);      \
  }

/* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */
#define omd_EDGE_EDGE_TEST(V0,U0,U1)                      \
  Bx=U0[i0]-U1[i0];                                   \
  By=U0[i1]-U1[i1];                                   \
  Cx=V0[i0]-U0[i0];                                   \
  Cy=V0[i1]-U0[i1];                                   \
  f=Ay*Bx-Ax*By;                                      \
  d=By*Cx-Bx*Cy;                                      \
  if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
  {                                                   \
    e=Ax*Cy-Ay*Cx;                                    \
    if(f>0)                                           \
    {                                                 \
      if(e>=0 && e<=f) return true;                   \
    }                                                 \
    else                                              \
    {                                                 \
      if(e<=0 && e>=f) return true;                   \
    }                                                 \
  }

#define omd_EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
{                                              \
  float Ax,Ay,Bx,By,Cx,Cy,e,d,f;               \
  Ax=V1[i0]-V0[i0];                            \
  Ay=V1[i1]-V0[i1];                            \
  /* test edge U0,U1 against V0,V1 */          \
  omd_EDGE_EDGE_TEST(V0,U0,U1);                    \
  /* test edge U1,U2 against V0,V1 */          \
  omd_EDGE_EDGE_TEST(V0,U1,U2);                    \
  /* test edge U2,U1 against V0,V1 */          \
  omd_EDGE_EDGE_TEST(V0,U2,U0);                    \
}

#define omd_POINT_IN_TRI(V0,U0,U1,U2)           \
{                                           \
  float a,b,c,d0,d1,d2;                     \
  /* is T1 completly inside T2? */          \
  /* check if V0 is inside tri(U0,U1,U2) */ \
  a=U1[i1]-U0[i1];                          \
  b=-(U1[i0]-U0[i0]);                       \
  c=-a*U0[i0]-b*U0[i1];                     \
  d0=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U2[i1]-U1[i1];                          \
  b=-(U2[i0]-U1[i0]);                       \
  c=-a*U1[i0]-b*U1[i1];                     \
  d1=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U0[i1]-U2[i1];                          \
  b=-(U0[i0]-U2[i0]);                       \
  c=-a*U2[i0]-b*U2[i1];                     \
  d2=a*V0[i0]+b*V0[i1]+c;                   \
  if(d0*d1>0.0)                             \
  {                                         \
    if(d0*d2>0.0) return true;              \
  }                                         \
}

static inline bool coplanar_tri_tri(const float *N, const float *V0, const float *V1, const float *V2,
									const float *U0, const float *U1, const float *U2)
{
   float A[3];
   short i0,i1;
   /* first project onto an axis-aligned plane, that maximizes the area */
   /* of the triangles, compute indices: i0,i1. */
   A[0]=(float)fabs(N[0]);
   A[1]=(float)fabs(N[1]);
   A[2]=(float)fabs(N[2]);
   if(A[0]>A[1])
   {
      if(A[0]>A[2])
      {
          i0=1;      /* A[0] is greatest */
          i1=2;
      }
      else
      {
          i0=0;      /* A[2] is greatest */
          i1=1;
      }
   }
   else   /* A[0]<=A[1] */
   {
      if(A[2]>A[1])
      {
          i0=0;      /* A[2] is greatest */
          i1=1;
      }
      else
      {
          i0=0;      /* A[1] is greatest */
          i1=2;
      }
    }

    /* test all edges of triangle 1 against the edges of triangle 2 */
    omd_EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2);
    omd_EDGE_AGAINST_TRI_EDGES(V1,V2,U0,U1,U2);
    omd_EDGE_AGAINST_TRI_EDGES(V2,V0,U0,U1,U2);

    /* finally, test if tri1 is totally contained in tri2 or vice versa */
    omd_POINT_IN_TRI(V0,U0,U1,U2);
    omd_POINT_IN_TRI(U0,V0,V1,V2);

    return false;
}

bool OMediaMathTools::tri_tri_intersect(const float	*V0,	const float	*V1,	const float	*V2,
						const float	*U0,	const float	*U1,	const float	*U2)

{
  float E1[3],E2[3];
  float N1[3],N2[3],d1,d2;
  float du0,du1,du2,dv0,dv1,dv2;
  float D[3];
  float omd_ISECT1[2], omd_ISECT2[2];
  float du0du1,du0du2,dv0dv1,dv0dv2;
  short index;
  float vp0,vp1,vp2;
  float up0,up1,up2;
  float b,c,max;

  /* compute plane equation of triangle(V0,V1,V2) */
  omd_SUB(E1,V1,V0);
  omd_SUB(E2,V2,V0);
  omd_CROSS(N1,E1,E2);
  d1=-omd_DOT(N1,V0);
  /* plane equation 1: N1.X+d1=0 */

  /* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
  du0=omd_DOT(N1,U0)+d1;
  du1=omd_DOT(N1,U1)+d1;
  du2=omd_DOT(N1,U2)+d1;

  /* coplanarity robustness check */
#if omd_USE_EPSILON_TEST==TRUE
  if(fabs(du0)<omd_EPSILON) du0=0.0;
  if(fabs(du1)<omd_EPSILON) du1=0.0;
  if(fabs(du2)<omd_EPSILON) du2=0.0;
#endif
  du0du1=du0*du1;
  du0du2=du0*du2;

  if(du0du1>0.0f && du0du2>0.0f) /* same sign on all of them + not equal 0 ? */
    return false;                /* no intersection occurs */

  /* compute plane of triangle (U0,U1,U2) */
  omd_SUB(E1,U1,U0);
  omd_SUB(E2,U2,U0);
  omd_CROSS(N2,E1,E2);
  d2=-omd_DOT(N2,U0);
  /* plane equation 2: N2.X+d2=0 */

  /* put V0,V1,V2 into plane equation 2 */
  dv0=omd_DOT(N2,V0)+d2;
  dv1=omd_DOT(N2,V1)+d2;
  dv2=omd_DOT(N2,V2)+d2;

#if omd_USE_EPSILON_TEST==TRUE
  if(fabs(dv0)<omd_EPSILON) dv0=0.0;
  if(fabs(dv1)<omd_EPSILON) dv1=0.0;
  if(fabs(dv2)<omd_EPSILON) dv2=0.0;
#endif

  dv0dv1=dv0*dv1;
  dv0dv2=dv0*dv2;

  if(dv0dv1>0.0f && dv0dv2>0.0f) /* same sign on all of them + not equal 0 ? */
    return false;                /* no intersection occurs */

  /* compute direction of intersection line */
  omd_CROSS(D,N1,N2);

  /* compute and index to the largest component of D */
  max=(float)fabs(D[0]);
  index=0;
  b=(float)fabs(D[1]);
  c=(float)fabs(D[2]);
  if(b>max) max=b,index=1;
  if(c>max) max=c,index=2;

        /* this is the simplified projection onto L*/
        vp0=V0[index];
        vp1=V1[index];
        vp2=V2[index];

        up0=U0[index];
        up1=U1[index];
        up2=U2[index];

  /* compute interval for triangle 1 */
  omd_COMPUTE_INTERVALS(vp0,vp1,vp2,dv0,dv1,dv2,dv0dv1,dv0dv2,omd_ISECT1[0],omd_ISECT1[1]);

  /* compute interval for triangle 2 */
  omd_COMPUTE_INTERVALS(up0,up1,up2,du0,du1,du2,du0du1,du0du2,omd_ISECT2[0],omd_ISECT2[1]);

  omd_SORT(omd_ISECT1[0],omd_ISECT1[1]);
  omd_SORT(omd_ISECT2[0],omd_ISECT2[1]);

  if(omd_ISECT1[1]<omd_ISECT2[0] || omd_ISECT2[1]<omd_ISECT1[0]) return false;
  return true;
}


//---------------------------------------------------------------------------
// triangle sphere interesect


bool OMediaMathTools::tri_sphere_intersect(const OMedia3DPoint	&v0,	
												const OMedia3DPoint	&v1,	
												const OMedia3DPoint	&v2,
												const OMedia3DPoint	&SC,	
												const float sr)
{
    OMedia3DPoint	diff;
    float			dist2;
    float			radius2 = sr*sr;
    
    // test if v2 is inside the sphere
    diff.x = v2.x - SC.x;
    diff.y = v2.y - SC.y;
    diff.z = v2.z - SC.z;
    dist2 = diff.x*diff.x+diff.y*diff.y+diff.z*diff.z;
    if ( dist2 <= radius2 ) return true;

    // test if v1 is inside the sphere
    diff.x = v1.x - SC.x;
    diff.y = v1.y - SC.y;
    diff.z = v1.z - SC.z;
    dist2 = diff.x*diff.x+diff.y*diff.y+diff.z*diff.z;
    if ( dist2 <= radius2 ) return true;

    // test if v0 is inside the sphere
    diff.x = v0.x - SC.x;
    diff.y = v0.y - SC.y;
    diff.z = v0.z - SC.z;
    dist2 = diff.x*diff.x+diff.y*diff.y+diff.z*diff.z;
    if ( dist2 <= radius2 ) return true;

    OMedia3DPoint edge0(v1.x-v0.x, v1.y-v0.y, v1.z-v0.z);
    OMedia3DPoint edge1(v2.x-v0.x, v2.y-v0.y, v2.z-v0.z);
    float A = omf_Dot(edge0,edge0);
    float B = omf_Dot(edge0,edge1);
    float C = omf_Dot(edge1,edge1);
    float D = omf_Dot(edge0,diff);
    float E = omf_Dot(edge1,diff);
    float F = omf_Dot(diff,diff);
    float det = A*C-B*B;  // assert:  det != 0 for triangles
    float invdet = 1.0f/det;
    float s = (B*E-C*D)*invdet;
    float t = (B*D-A*E)*invdet;

    if ( s+t <= 1.0f )
    {
        if ( s < 0.0f )
        {
            if ( t < 0.0f )  // region 4
            {
                if ( D < 0.0f )
                {
                    t = 0.0f;
                    s = -D/A;
                    if ( s > 1.0f ) s = 1.0f;
                }
                else if ( E < 0.0f )
                {
                    s = 0.0f;
                    t = -E/C;
                    if ( t > 1.0f ) t = 1.0f;
                }
                else
                {
                    s = 0.0f;
                    t = 0.0f;
                }
            }
            else  // region 3
            {
                s = 0.0f;
                t = -E/C;
                if ( t < 0.0f ) t = 0.0f; else if ( t > 1.0f ) t = 1.0f;
            }
        }
        else if ( t < 0.0f )  // region 5
        {
            t = 0.0f;
            s = -D/A;
            if ( s < 0.0f ) s = 0.0f; else if ( s > 1.0 ) s = 1.0f;
        }
        else  // region 0
        {
            // minimum at interior point
        }
    }
    else
    {
        if ( s < 0.0f )  // region 2
        {
            if ( B-C+D-E < 0.0f )
            {
                s = -(B-C+D-E)/(A-2*B+C);
                if ( s < 0.0f ) s = 0.0f; else if ( s > 1.0f ) s = 1.0f;
                t = 1.0f-s;
            }
            else if ( C+E > 0.0f )
            {
                s = 0.0f;
                t = -E/C;
                if ( t < 0.0f ) t = 0.0f; else if ( t > 1.0f ) t = 1.0f;
            }
            else
            {
                s = 0.0f;
                t = 1.0f;
            }
        }
        else if ( t < 0.0f )  // region 6
        {
            if ( A-B+D-E > 0.0f )
            {
                t = (A-B+D-E)/(A-2*B+C);
                if ( t < 0.0f ) t = 0.0; else if ( t > 1.0f ) t = 1.0f;
                s = 1.0f-t;
            }
            else if ( A+D > 0.0f )
            {
                t = 0.0f;
                s = -D/A;
                if ( s < 0.0f ) s = 0.0f; else if ( s > 1.0f ) s = 1.0f;
            }
            else
            {
                s = 1.0f;
                t = 0.0f;
            }
        }
        else  // region 1
        {
            s = -(B-C+D-E)/(A-2*B+C);
            if ( s < 0.0f ) s = 0.0f; else if ( s > 1.0f ) s = 1.0f;
            t = 1.0f-s;
        }
    }

    dist2 = s*(A*s+B*t+2*D)+t*(B*s+C*t+2*E)+F;

    return dist2 < radius2;
}

bool OMediaMathTools::tri_tri_quick_reject(const float	*V0,	const float	*V1,	const float	*V2,
											const float	*U0,	const float	*U1,	const float	*U2)
{
#define	min_ext(p0,p1,p2,res)				\
	if (p0<p1)								\
	{										\
		if (p2<p0) res = p2;				\
		else res = p0;						\
	}										\
	else									\
	{										\
		if (p2<p1) res = p2;				\
		else res = p1;						\
	}

#define	max_ext(p0,p1,p2,res)				\
	if (p0>p1)								\
	{										\
		if (p2>p0) res = p2;				\
		else res = p0;						\
	}										\
	else									\
	{										\
		if (p2>p1) res = p2;				\
		else res = p1;						\
	}

	float	min_res,max_res;

	min_ext(V0[0],V1[0],V2[0],min_res);		// x
	max_ext(U0[0],U1[0],U2[0],max_res);
	if (max_res<min_res) return true;

	min_ext(U0[0],U1[0],U2[0],min_res);
	max_ext(V0[0],V1[0],V2[0],max_res);
	if (max_res<min_res) return true;
	
	min_ext(V0[1],V1[1],V2[1],min_res);		// y
	max_ext(U0[1],U1[1],U2[1],max_res);
	if (max_res<min_res) return true;

	min_ext(U0[1],U1[1],U2[1],min_res);
	max_ext(V0[1],V1[1],V2[1],max_res);
	if (max_res<min_res) return true;

	min_ext(V0[2],V1[2],V2[2],min_res);		// z
	max_ext(U0[2],U1[2],U2[2],max_res);
	if (max_res<min_res) return true;

	min_ext(U0[2],U1[2],U2[2],min_res);
	max_ext(V0[2],V1[2],V2[2],max_res);
	if (max_res<min_res) return true;

	return false;
}


bool OMediaMathTools::tri_ray_intersect(const float *orig, const float *dir,
						const float *vert0, const float *vert1, const float *vert2,
						float *t, float *u, float *v)
{
   float edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   float det,inv_det;

   /* find vectors for two edges sharing vert0 */
   omd_SUB(edge1, vert1, vert0);
   omd_SUB(edge2, vert2, vert0);

   /* begin calculating determinant - also used to calculate U parameter */
   omd_CROSS(pvec, dir, edge2);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = omd_DOT(edge1, pvec);

   if (det > -omd_EPSILON && det < omd_EPSILON) return false;
   inv_det = 1.0f / det;

   /* calculate distance from vert0 to ray origin */
   omd_SUB(tvec, orig, vert0);

   /* calculate U parameter and test bounds */
   *u = omd_DOT(tvec, pvec) * inv_det;
   if (*u < 0.0f || *u > 1.0f) return false;

   /* prepare to test V parameter */
   omd_CROSS(qvec, tvec, edge1);

   /* calculate V parameter and test bounds */
   *v = omd_DOT(dir, qvec) * inv_det;
   if (*v < 0.0f || *u + *v > 1.0f) return false;

   /* calculate t, ray intersects triangle */
   *t = omd_DOT(edge2, qvec) * inv_det;

   return ((*t)>=0.0f);
}

bool OMediaMathTools::sphere_ray_intersect (const float *ray_origin, 
											const float *ray_direction,
											const float *sphere_center, 
											const float sphere_radius, 
											short		&out_nhits, 
											float		*out_distances)
{
    float kDiff[3];
    
    kDiff[0] = ray_origin[0] - sphere_center[0];
    kDiff[1] = ray_origin[1] - sphere_center[1];
    kDiff[2] = ray_origin[2] - sphere_center[2];    
    
    float fA = (ray_direction[0] * ray_direction[0] + ray_direction[1] * ray_direction[1] + ray_direction[2] * ray_direction[2]);
    float fB = kDiff[0] * ray_direction[0] + kDiff[1] * ray_direction[1] + kDiff[2] * ray_direction[2];
    float fC = (kDiff[0] * kDiff[0] + kDiff[1] * kDiff[1] + kDiff[2] * kDiff[2]) - sphere_radius*sphere_radius;

    float afT[2];
    float fDiscr = fB*fB - fA*fC;
    if ( fDiscr < 0.0f )
    {
        out_nhits = 0;
        return false;
    }
    else if ( fDiscr > 0.0f )
    {
	    float fRoot = (float)sqrt(fDiscr);
        float fInvA = 1.0f/fA;
        afT[0] = (-fB - fRoot)*fInvA;
        afT[1] = (-fB + fRoot)*fInvA;

        if ( afT[0] >= 0.0f )
        {
            out_nhits = 2;
			out_distances[0] = afT[0];
			out_distances[1] = afT[1];
            return true;
        }
        else if ( afT[1] >= 0.0f )
        {
            out_nhits = 1;
			out_distances[0] = afT[1];
            return true;
        }
        else
        {
            out_nhits = 0;
            return false;
        }
    }
    else
    {
        afT[0] = -fB/fA;
        if ( afT[0] >= 0.0f )
        {
            out_nhits = 1;
			out_distances[0] = afT[0];
            return true;
        }
        else
        {
            out_nhits = 0;
            return false;
        }
    }

}

float OMediaMathTools::tri_area(const float	*V0,	const float	*V1,	const float	*V2)
{
	return (float)sqrt(tri_area_square(V0,V1,V2));
}


float OMediaMathTools::tri_area_square(const float	*V0,	const float	*V1,	const float	*V2)
{
	OMedia3DVector		v;
	float				a,b,c,s;

	v.x = V0[0] - V1[0];
	v.y = V0[1] - V1[1];
	v.z = V0[2] - V1[2];
	a = v.quick_magnitude();

	v.x = V1[0] - V2[0];
	v.y = V1[1] - V2[1];
	v.z = V1[2] - V2[2];
	b = v.quick_magnitude();

	v.x = V2[0] - V0[0];
	v.y = V2[1] - V0[1];
	v.z = V2[2] - V0[2];
	c = v.quick_magnitude();

	s = (a + b + c) * 0.5f;

	return s * (s-a) * (s-b) * (s-c);
}

//----------------------------------------------

void OMediaMathTools::find_plane(const float *normal_v,
									 const float *plane_pnt,
									 float *out_plane_v4)
{
	out_plane_v4[0] = normal_v[0];
	out_plane_v4[1] = normal_v[1];
	out_plane_v4[2] = normal_v[2];
	out_plane_v4[3] = - omd_DOT(normal_v,plane_pnt);
}

bool OMediaMathTools::plane_line_intersect(const float *l_org, 
											 const float *l_vec,
											 const float *plane_v4,
											 float *out_dest)
{
	float tmp = omd_DOT ( l_vec, plane_v4 ),s ;

	/* Is line parallel to plane? */

	if ( fabs ( tmp ) < omd_FLT_EPSILON ) return false;

	s = -( omd_DOT ( l_org, plane_v4 ) + plane_v4[3] ) / tmp ;

	out_dest [ 0 ] = (l_vec[0] * s) + l_org[0];
	out_dest [ 1 ] = (l_vec[1] * s) + l_org[1];
	out_dest [ 2 ] = (l_vec[2] * s) + l_org[2];

	return true ;
}

