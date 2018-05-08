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
#ifndef OMEDIA_PipePoint_H
#define OMEDIA_PipePoint_H

#include "OMediaTypes.h"
#include "OMedia3DPoint.h"
#include "OMediaRGBColor.h"
#include "OMedia3DVector.h"

#include <list>

typedef unsigned short omt_PipePointFlags;
const omt_PipePointFlags		omppf_Clipped 	= (1<<0);

class OMediaPipePoint
{
	public:
	
	inline OMediaPipePoint() {}
	inline OMediaPipePoint(const OMediaPipePoint &pp) {}
		
	float				xyzw[4];		// Coord.
	float				inv_w;			
	
	OMedia3DVector		n;				// Normal
		
	float				a,dr,dg,db;		// Diffuse
	float				sr,sg,sb;		// Specular
	float				u,v;			// Texture coord.
	
	omt_PipePointFlags	flags;
	
	OMediaPipePoint &operator=(const OMediaPipePoint &p) {return *this;}

};

#define omd_PipePointSegmentSize 512

class OMediaPipePointBufferSegment
{
	public:
	
	OMediaPipePoint	points[omd_PipePointSegmentSize];
};


class OMediaPipePointBuffer
{
	public:

	inline OMediaPipePointBuffer()
	{
		OMediaPipePointBufferSegment *emptyseg;
		emptyseg = new OMediaPipePointBufferSegment;
		segments.push_back(*emptyseg);
		delete emptyseg;
	}
	
	inline void reset_pointer(void)
	{
		current_seg = segments.begin();
		current_seg_offset = 0;
		current_point = (*current_seg).points;
	}
	
	OMediaPipePoint *get_points(long n);
	
	void new_segment(void)
	{
		OMediaPipePointBufferSegment *emptyseg;
		emptyseg = new OMediaPipePointBufferSegment;
		segments.push_back(*emptyseg);
		delete emptyseg;
		
		current_seg = segments.end();
		current_seg--;
		current_seg_offset = 0;
		current_point = (*current_seg).points;		
	}

	inline void mark_temporary_points(void)
	{
		mark_seg = current_seg;
		mark_seg_offset = current_seg_offset;
		mark_point = current_point;
	}
	
	inline void release_temporary_points(void)
	{
		current_seg = mark_seg;
		current_seg_offset = mark_seg_offset;
		current_point = mark_point;
	}
	

	inline OMediaPipePoint *reserve_points(long n)
	{
		if (current_seg_offset+n<=omd_PipePointSegmentSize) return current_point;
		return do_reserve_points(n);
	}


	list<OMediaPipePointBufferSegment>	segments;

	list<OMediaPipePointBufferSegment>::iterator current_seg,mark_seg;
	long current_seg_offset,mark_seg_offset;
	OMediaPipePoint	*current_point,*mark_point;

	protected:
	
	OMediaPipePoint *do_reserve_points(long n);

};



#endif
