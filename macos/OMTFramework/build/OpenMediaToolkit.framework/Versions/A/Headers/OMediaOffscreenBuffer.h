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
#ifndef OMEDIA_OffscreenBuffer_H
#define OMEDIA_OffscreenBuffer_H

#include "OMediaTypes.h"
#include "OMediaBroadcaster.h"
#include "OMediaRGBColor.h"
#include "OMediaRect.h"

#include <list>

class OMediaVideoEngine;
class OMediaOffscreenBuffer;
class OMediaWindow;

typedef list<OMediaOffscreenBuffer*>	omt_OffscreenBufferList;

// OMT supports only true color is supported at this time

enum omt_OffscreenBufferPixelFormat
{
	omobfc_ARGB1555,		// 16 bits
	omobfc_ARGB8888			// 32 bits
};


// Offscreen buffers are allocated from a video engine, see the OMediaVideoEngine class

class OMediaOffscreenBuffer : public OMediaBroadcaster
{
	public:

	omtshared virtual ~OMediaOffscreenBuffer();


	omtshared virtual void lock(void) =0L;		// Returns a pointer to pixels
	omtshared virtual void unlock(void) =0L;

	omtshared virtual void get_pixmap(void *&pixels, long &rowbytes) =0L;	// Must be locked

	inline long get_width(void) const {return width;}
	inline long get_height(void) const {return height;}

	omtshared virtual void draw(OMediaWindow *window, long x, long y) =0L;

	inline omt_OffscreenBufferPixelFormat get_pixel_format(void) const {return pixel_format;}

	omtshared virtual void fill(OMediaRGBColor &color, OMediaRect *dest =NULL);


	protected:

	omtshared OMediaOffscreenBuffer(OMediaVideoEngine *engine, long width, long height, omt_OffscreenBufferPixelFormat pixformat);


	long	width,height;				
	omt_OffscreenBufferPixelFormat		pixel_format;
	omt_OffscreenBufferList::iterator	container_node;
	OMediaVideoEngine					*its_engine;

	
	friend class OMediaVideoEngine;
};


#endif

