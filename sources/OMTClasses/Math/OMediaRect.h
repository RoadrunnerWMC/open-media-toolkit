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
#ifndef OMEDIA_Rect_H
#define OMEDIA_Rect_H

#include "OMediaTypes.h"


/** Rectangle Template

Defines a mathematical rectangle template. This is the base class for every rectangle classes. 

OMT implements various rectangles. The default rectangle class is OMediaRect that uses long word
integers. The OMediaFloatRect class is used for floating point rectangles.

Right and bottom coordinates are not included in the rectangle. 

So the width of the rectangle can be computed this way:

	width = myrect.right - myrect.left


*/

template <class T>
class OMediaRectTemplate
{
	public:

	T left,top,right,bottom;

	inline T *coords(void) {return (&left);}


	inline OMediaRectTemplate(void) {}
	inline OMediaRectTemplate(OMediaRectTemplate	*r)
					{left = r->left; top = r->top; right = r->right; bottom = r->bottom;}

	inline OMediaRectTemplate(T aleft, T atop, T aright,T abottom)
					{left = aleft; top = atop; right = aright; bottom = abottom;}

	inline T get_width(void) const {return right-left;}
	inline T get_height(void) const {return bottom-top;}

	inline void set(T aleft, T atop, T aright,T abottom)
					{left = aleft; top = atop; right = aright; bottom = abottom;}


	inline void copy_to(OMediaRectTemplate	*r) const
					{r->left = left; r->top = top; r->right = right; r->bottom = bottom;}

	inline void offset(T h, T v)
					{{T ox=h,oy=v; left += ox; top += oy; right += ox; bottom += oy;}}


	/** Return true if two rectangles intersect. */
	bool touch(const OMediaRectTemplate *r) const 
						   {return !(r->right<=left || r->left>=right ||
						   			 r->bottom<=top || r->top>=bottom);}


	/** Find the intersection between two rectangles. Returns false if no intersection is found.
		The "common" rectangle contains the intersection area if any. */

	inline bool find_intersection(const OMediaRectTemplate *r, OMediaRectTemplate *common) const
	{
		if (r->top  >= bottom ||
			r->left >= right || 
			top  >= r->bottom || 
			left >= r->right) return false;
	
		common->left = (r->left <= left)? left:r->left;
		common->top = (r->top <= top)? top:r->top;
	
		common->right = (r->right <= right)? r->right:right;
		common->bottom = (r->bottom <= bottom)? r->bottom:bottom;

		return !common->empty();
	}

	/** Returns true if rectangles are identical*/
	inline bool is_equal(const OMediaRectTemplate *r) const
	{
		return (left==r->left && top==r->top &&
				right==r->right && bottom==r->bottom);
	}

	/** Find union of the two rectangles */
	inline void find_union(const OMediaRectTemplate *r, OMediaRectTemplate *unionr) const
	{
		unionr->left = (left< r->left) ? left:r->left;
		unionr->top = (top< r->top) ? top:r->top;
		unionr->bottom = (bottom> r->bottom) ? bottom:r->bottom;
		unionr->right = (right> r->right) ? right:r->right;
	}

	/** Returns true if rectangle is empty. A rectangle is empty if left is greater or equal to right,
		or if top is greater or equal to bottom */
	inline bool empty(void) const
	{
		return (left>=right || top>=bottom);
	}

	/** Returns true if the rectangle is enclosed by the passed rectangle. */
	inline bool is_inrect(const OMediaRectTemplate *r) const
	{
		return (left>=r->left && top>=r->top && right<=r->right && bottom<=r->bottom);
	}

	/** Returns true if the passed coordinate is insided the rectangle. */
	inline bool is_pointin(const T x, const T y) const
	{
		return (x>=left && y>=top && x<right && y<bottom);
	}

};

// Long rectangle

class OMediaShortRect;


/** 32 bits integer rectangle 



Implements a long word based rectangle. See OMediaRectTemplate class for more general 

informations about rectangles.



*/


class OMediaRect : public OMediaRectTemplate<long>
{
	public:

	OMediaRect(void) {}

	inline OMediaRect(OMediaRect	*r)
        {
            left = r->left; top = r->top; right = r->right; bottom = r->bottom;
        }
	
	inline OMediaRect(long aleft, long atop, long aright,long abottom)
        {
            left = aleft; top = atop; right = aright; bottom = abottom;
        }

	OMediaRect(OMediaShortRect &);

	void copyto(OMediaShortRect *r) const;

};

/** 16 bits integer rectangle 

Implements a word based rectangle. See OMediaRectTemplate class for more general 
informations about rectangles.

*/


class OMediaShortRect : public OMediaRectTemplate<short>
{
	public:

	inline OMediaShortRect(void) {}
	inline OMediaShortRect(OMediaShortRect	*r)
        {
            left = r->left; top = r->top; right = r->right; bottom = r->bottom;
        }
	
	inline OMediaShortRect(short aleft, short atop, short aright,short abottom) 
        {
            left = aleft; top = atop; right = aright; bottom = abottom;
        }
        
        inline OMediaShortRect(OMediaRect &);
	
	inline void copyto(OMediaRect *r) const;
};

/** 32 bits floating point rectangle 



Implements a float based rectangle. See OMediaRectTemplate class for more general 

informations about rectangles.



*/


class OMediaFloatRect : public OMediaRectTemplate<float>
{
	public:

	inline OMediaFloatRect(void) {}
	inline OMediaFloatRect(OMediaFloatRect	*r) : OMediaRectTemplate<float> (r) {}
	
	inline OMediaFloatRect(float aleft, float atop, float aright,float abottom) : 
									OMediaRectTemplate<float>(aleft, atop, aright, abottom) {}

	inline OMediaFloatRect(OMediaRect &);
	
	inline void copyto(OMediaRect *r) const;
};


// Special implementation

inline OMediaRect::OMediaRect(OMediaShortRect &sr) : 
			OMediaRectTemplate<long>(sr.left, sr.top, sr.right, sr.bottom) {}

inline void OMediaRect::copyto(OMediaShortRect *r) const
						{r->left = (short)left; r->top = (short)top; r->right = (short)right; r->bottom = (short)bottom;}


inline 	OMediaShortRect::OMediaShortRect(OMediaRect &lr) : 
			OMediaRectTemplate<short>((short)lr.left, (short)lr.top, (short)lr.right, (short)lr.bottom) {}

inline void OMediaShortRect::copyto(OMediaRect *r) const
						{r->left = left; r->top = top; r->right = right; r->bottom = bottom;}



inline 	OMediaFloatRect::OMediaFloatRect(OMediaRect &lr) : 
			OMediaRectTemplate<float>((float)lr.left, (float)lr.top, (float)lr.right, (float)lr.bottom) {}

inline void OMediaFloatRect::copyto(OMediaRect *r) const
						{r->left = (long)left; r->top = (long)top; r->right = (long)right; r->bottom = (long)bottom;}

#endif

