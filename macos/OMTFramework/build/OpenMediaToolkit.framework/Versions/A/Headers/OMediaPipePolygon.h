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
#ifndef OMEDIA_PipePolygon_H
#define OMEDIA_PipePolygon_H

#include "OMediaTypes.h"
#include "OMediaRendererInterface.h"

#include <list>

class OMediaPipePoint;
class OMediaOMTCanvasText;
class OMediaCanvas;

typedef unsigned short omt_PipePolygonFlags;
const omt_PipePolygonFlags		omppf_Gouraud 		= (1<<0);
const omt_PipePolygonFlags		omppf_Lines 		= (1<<1);
const omt_PipePolygonFlags		omppf_Points 		= (1<<2);
const omt_PipePolygonFlags		omppf_ZWrite 		= (1<<3);
const omt_PipePolygonFlags		omppf_ZTest 		= (1<<4);
const omt_PipePolygonFlags		omppf_FlatSurface 	= (1<<5);

class OMediaPipePolygon
{
	public:
	
	inline OMediaPipePolygon() {}
	inline OMediaPipePolygon(const OMediaPipePolygon &pp) {}

	OMediaPipePoint			*points;
	long					npoints;

	union
	{
		OMediaOMTCanvasText		*texture;
		OMediaCanvas			*surface;
	};

	omt_PipePolygonFlags	flags;

	omt_ZBufferFunc			zfunc;
	omt_BlendFunc			src_blend,dest_blend;
	
	
	OMediaPipePolygon &operator=(const OMediaPipePolygon &p) {return *this;}
};

#define omd_PipePolygonSegmentSize 1024

class OMediaPipePolygonBufferSegment
{
	public:

	OMediaPipePolygon	polygons[omd_PipePolygonSegmentSize];
};


class OMediaPipePolygonBuffer
{
	public:

	inline OMediaPipePolygonBuffer()
	{
		const OMediaPipePolygonBufferSegment emptyseg;
		segments.push_back(emptyseg);
	}
	
	inline void reset_pointer(void)
	{
		current_seg = segments.begin();
		current_seg_offset = 0;
		current_polygon = (*current_seg).polygons;
	}
	
	OMediaPipePolygon *get_polygon(void);
	
	void new_segment(void)
	{
		OMediaPipePolygonBufferSegment emptyseg;
		segments.push_back(emptyseg);
		
		current_seg = segments.end();
		current_seg--;
		current_seg_offset = 0;
		current_polygon = (*current_seg).polygons;
	}

	void release_polygon(void)
	{
		if (current_seg_offset) {current_seg_offset--; current_polygon--;}
		else
		{
			current_seg--;
			current_seg_offset = omd_PipePolygonSegmentSize-1;
			current_polygon = (*current_seg).polygons + (omd_PipePolygonSegmentSize-1);
		}
	}

	inline void scan_start(void)
	{
		scan_seg_offset = 0;
		scan_seg		= segments.begin();
		current_polygon = (*scan_seg).polygons;
	}
	
	inline bool scan_end(void)
	{
		return (scan_seg==current_seg && scan_seg_offset==current_seg_offset);
	}
	
	inline void scan_next(void)
	{
		scan_seg_offset++;
		current_polygon++;

		if (scan_seg_offset>=omd_PipePolygonSegmentSize &&
			scan_seg!=current_seg)
		{
			scan_seg++;
			scan_seg_offset = 0;
			current_polygon = (*scan_seg).polygons;
		}
	}
	
	inline OMediaPipePolygon *scan_polygon(void) 
	{
		return current_polygon;
	}
	
	
	long scan_seg_offset;
	list<OMediaPipePolygonBufferSegment>::iterator scan_seg;


	list<OMediaPipePolygonBufferSegment>	segments;

	list<OMediaPipePolygonBufferSegment>::iterator current_seg;
	long current_seg_offset;
	OMediaPipePolygon	*current_polygon;

};



#endif
