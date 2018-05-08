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
#ifndef OMEDIA_Blitter_H
#define OMEDIA_Blitter_H

#include "OMediaTypes.h"
#include "OMediaRendererInterface.h"
#include "OMediaPixelFormat.h"
#include "OMediaRect.h"
#include "OMediaBlendTable.h"


class OMediaBlitter : public OMediaBlendTable
{
	public:
	
	// * Block

	omtshared static void draw_full(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							omt_RGBAPixel *dest_pix, long dest_width, long dest_height,
							long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,

							omt_RGBAPixelMask mask);

	omtshared static void draw(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							omt_RGBAPixel *dest_pix, long dest_width, long dest_height,
							OMediaRect *src, long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,

							omt_RGBAPixelMask mask);

	omtshared static void draw(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							omt_RGBAPixel *dest_pix, long dest_width, long dest_height,
							OMediaRect *src, OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,

							omt_RGBAPixelMask mask);

	omtshared static void draw(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							omt_RGBAPixel *dest_pix, long dest_width, long dest_height,
							OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,

							omt_RGBAPixelMask mask);

	omtshared static void fill(omt_RGBAPixel src_pix,
								omt_RGBAPixel *dest_pix, long dest_width, long dest_height,
								OMediaRect *fill_rect,
								omt_BlendFunc	blend_src, 
								omt_BlendFunc	blend_dest,

								omt_RGBAPixelMask mask);

	omtshared static void fill_alpha(	unsigned char alpha,
						 		omt_RGBAPixel *pix, long width, long height,
						 		OMediaRect 	*dest);


	omtshared static bool block_preparecliprects(	OMediaRect &src, 
										OMediaRect &dest, 
										OMediaRect &bounds,
										long w, long h);

	omtshared static bool block_clip(OMediaRect &blitsource, 
						 		 		OMediaRect &blitdest, 
						  				OMediaRect &bounds);

	// * Line

	omtshared static void draw_line(omt_RGBAPixel 	*dest_buffer,
					 			long 			buffer_width,
					 			short x1, 		short y1,
                     			short x2, 		short y2,
                     			omt_RGBAPixel 	color,
                     			OMediaShortRect *clip,
                     			omt_BlendFunc	blend_src, 
								omt_BlendFunc	blend_dest);
								
	omtshared static bool clip_line(short &x1, short &y1, short &x2, short &y2, OMediaShortRect *clip);





	// * Raw blitter

	omtshared static void raw_fill(unsigned char *dest, unsigned long lgdata, short n);

	// * Remap

	omtshared static void remap_to_A1555(	omt_RGBAPixel	*src,	
											unsigned short	*dest,
											long			length);

};



#endif

