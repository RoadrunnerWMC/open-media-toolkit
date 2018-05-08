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
 
#include "OMediaOffscreenBuffer.h"
#include "OMediaVideoEngine.h"
#include "OMediaBlitter.h"

OMediaOffscreenBuffer::OMediaOffscreenBuffer(OMediaVideoEngine *engine, 
											long w, long h, 
											omt_OffscreenBufferPixelFormat pixformat)
{
	width = w;
	height = h;
	pixel_format = pixformat;
	its_engine = engine;

	its_engine->get_offscreen_buffers()->push_back(this);
	container_node = its_engine->get_offscreen_buffers()->end();
	container_node--;
}

OMediaOffscreenBuffer::~OMediaOffscreenBuffer()
{
	broadcast_message(omsg_OffscreenBufferDeleted);
	its_engine->get_offscreen_buffers()->erase(container_node);

}

void OMediaOffscreenBuffer::fill(	OMediaRGBColor 	&color, 
									OMediaRect 		*dest)
{
	OMediaRect		full,bounds;
	char 			*bits;
	void			*vb;
	short 			w,h,depth;
	long 			rowbytes;
	unsigned long	direct_color;

	lock();
	
	get_pixmap(vb, rowbytes);
	bits = (char*)vb;

	unsigned short	r = color.red, g = color.green, b = color.blue;
	
	switch(pixel_format)
	{	
		case omobfc_ARGB1555:
		r >>=16-5;	g>>=16-5;	b>>=16-5;
		direct_color = (r<<10)|(g<<5)|b;
		direct_color = (direct_color<<16)|direct_color;
		depth = 1;
		break;
		
		case omobfc_ARGB8888:
		r >>=16-8;	g>>=16-8;	b>>=16-8;
		direct_color = (r<<16)|(g<<8)|b;
		depth = 2;
		break;
	}

	full.set(0,0,width,height);

	if (!dest) bounds = full;
	else bounds = *dest;

	if (full.find_intersection(&bounds,&bounds))
	{
		bits += (bounds.top* rowbytes) + (bounds.left<<depth);

		w = bounds.get_width()<<depth;
		h = bounds.get_height();
		
		while(h--) {OMediaBlitter::raw_fill((unsigned char*)bits, direct_color,w); bits+=rowbytes;}
	}

	unlock();
}
