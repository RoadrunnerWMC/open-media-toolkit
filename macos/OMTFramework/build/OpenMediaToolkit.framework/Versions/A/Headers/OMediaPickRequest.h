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
#ifndef OMEDIA_PickRequest_H
#define OMEDIA_PickRequest_H

#include "OMediaTypes.h"

#include <list>

class OMediaViewPort;
class OMediaElement;
class OMedia3DPolygon;
class OMedia3DShape;
class OMediaCanvas;

enum omt_PickType
{

	omptc_Null,			// Null
	omptc_Element		// Element picked
};

enum omt_PickLevel
{
	omplc_Element,		// Returns only elements
	omplc_Geometry,		// Elements plus sub-elements primitive (like polygons)

	omplc_Surface		// Returns also u/v informations of the pickx/y projected to the
						// surface of the primitive. This is usefull if you need to read/write
						// pixels.
};


class OMediaPickSubResult
{
	public:

	float				minz,maxz;
	
	// only if PickLevel is Geometry or higher
	long				polygon;				// Polygon picked or -1
		
	// only if PickLevel is Surface:
	bool				surface_hit;			// True if pickx/y hit the surface. In this case you can read u/v/inc_w.
	float				u,v;					// u/v on the top of the polygon/surface. 
	float				inv_w;					// inv_w on the top of the polygon/surface.
};

class OMediaPickResult
{
	public:
	
	omt_PickType		type;					// Result
	OMediaViewPort		*viewport;				// Viewport or NULL
	OMediaElement		*element;				// Element or NULL
	OMedia3DShape		*shape;					// Shape or NULL
	OMediaCanvas		*canvas;				// Canvas or NULL

	vector<OMediaPickSubResult>	sub_info;
};

typedef unsigned short omt_PickRequestFlags;
const omt_PickRequestFlags	ompickf_CloserHit_SurfaceHitOnly = (1<<0);	// If set, the closer hit is filled only by a hit
																		// where pickx/y was on its surface 
																		// (in other words when the
																		// surface_hit bool is true). This flag has effect
																		// only if PickLevel is Surface.

class OMediaPickRequest
{
	public:

	OMediaPickRequest() {level = omplc_Element; flags = 0;}
	
	
	// In:
	
	omt_PickLevel			level;
	omt_PickRequestFlags	flags;

	float				pickx,picky;	// Position of the area
	float				pickw,pickh;	// Size of the area ( -pickw<x<pickw )
	
	// Out:
	
	list<OMediaPickResult>		hits;			// Hits
	OMediaPickResult			closer_hit;		// Hit closer from the viewer.
												// If type is omptc_Null,
												// there is no hit.


	// Internal
	
	float			normalized_px,
					normalized_py;
};


#endif

