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
#ifndef OMEDIA_RastBlitter_H
#define OMEDIA_RastBlitter_H

#include "OMediaTypes.h"
#include "OMediaRendererInterface.h"
#include "OMediaPixelFormat.h"
#include "OMediaRect.h"
#include "OMediaBlendTable.h"


class OMediaRastBlitter : public OMediaBlendTable
{
	public:

	// * ZBuffer generic test

	inline static bool do_ztest(	unsigned short 		z,
						 	unsigned short 		zc,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest)
	{
		if (ztest==omzbtc_Enabled)
		{
			switch(zfunc)
			{
				case omzbfc_Never: 		return false;		
				case omzbfc_Always:		return true;
				case omzbfc_Less: 		return z<zc;
				case omzbfc_LEqual: 	return z<=zc;
				case omzbfc_Equal: 		return z==zc;
				case omzbfc_GEqual: 	return z>=zc;
				case omzbfc_Greater:	return z>z;
				case omzbfc_NotEqual: 	return z!=zc;
			}	
			
			return false;	
		}
		
		return true;
	}	
		

	// * Block

	omtshared static void draw_full_32(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha);

	omtshared static void draw_32(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha);

	omtshared static void draw_32(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha);


	omtshared static void draw_full_32zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				z,
							unsigned short		*zbuffer,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite);

	omtshared static void draw_32zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				z,
							unsigned short		*zbuffer,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite);

	omtshared static void draw_32zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				z,
							unsigned short		*zbuffer,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite);

	omtshared static void draw_full_16(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha);

	omtshared static void draw_16(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha);

	omtshared static void draw_16(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha);


	omtshared static void draw_full_16zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				z,
							unsigned short		*zbuffer,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite);

	omtshared static void draw_16zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				z,
							unsigned short		*zbuffer,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite);

	omtshared static void draw_16zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				z,
							unsigned short		*zbuffer,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite);

};



#endif

