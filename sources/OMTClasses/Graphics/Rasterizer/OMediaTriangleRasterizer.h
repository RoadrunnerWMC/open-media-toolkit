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
#ifndef OMEDIA_TriangleRasterizer_H
#define OMEDIA_TriangleRasterizer_H

template <class TPOINT, class TSEGMENT, class TRASTERLINE>
class OMediaTriangleRasterizer
{
	public:

	inline OMediaTriangleRasterizer(TRASTERLINE *arasterline)
	{
		rasterline = arasterline;
	}

	inline void swap(TPOINT *&a, TPOINT *&b) {TPOINT *s; s=a; a=b; b=s;} 

	void draw(TPOINT 		*p1, 
			  TPOINT 		*p2,
			  TPOINT 		*p3)
	{
		if (p2->xyzw[1]<p1->xyzw[1]) swap(p1,p2);
		if (p3->xyzw[1]<p1->xyzw[1]) swap(p1,p3);
		if (p3->xyzw[1]<p2->xyzw[1]) swap(p2,p3); 

		TSEGMENT	bigseg,smallseg;
		TSEGMENT	*leftseg,*rightseg;
		short		y,yend;

		bigseg.set(p1,p3);
		smallseg.set(p1,p2);

		if (short(p1->xyzw[0]+0.5) != short(p2->xyzw[0]+0.5))
		{
			if (p1->xyzw[0]+((p2->xyzw[1]-p1->xyzw[1])*bigseg.dx)<p2->xyzw[0]) {leftseg = &bigseg; rightseg = &smallseg;}
			else {leftseg = &smallseg; rightseg = &bigseg;}
		}
		else
		{
			if (p3->xyzw[0]<p2->xyzw[0]) {leftseg = &bigseg; rightseg = &smallseg;}
			else {leftseg = &smallseg; rightseg = &bigseg;}
		}

		rasterline->start((int)p1->xyzw[1]);
		y = bigseg.y1;
		yend = smallseg.y2;

		while(y<yend)
		{
			if (leftseg->x<rightseg->x) rasterline->rasterline(leftseg, rightseg,y);

			rasterline->step();
			bigseg.step();
			smallseg.step();
			y++;
		}

		smallseg.set(p2,p3);
		yend = bigseg.y2-1;

		while(y<=yend)
		{
			if (leftseg->x<rightseg->x) rasterline->rasterline(leftseg, rightseg, y);
		
			rasterline->step();
			bigseg.step();
			smallseg.step();
			y++;	
		}
	}
	
	protected:
	
	TRASTERLINE		*rasterline;
};


#endif

