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
#ifndef OMEDIA_TextSegmentRasterizer_H
#define OMEDIA_TextSegmentRasterizer_H

#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_OMTRASTERIZER

#include "OMediaSegmentRasterizer.h"
#include "OMediaTriangleSegment.h"

#define omd_LINEAR_FP 20L
#define omd_COORD_FPEX 2L
#define omd_INVW_SHIFTBIT ((omd_InvWBits-omd_UVBits)+omd_COORD_FPEX)



class OMediaTextSegmentRasterizer : public OMediaSegmentRasterizer
{
	public:
	
	// T32 to 32 bits

	omtshared static void fill_text_generic_zb32(
								unsigned long *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								omt_PixInterValue 	da,				// Diffuse
								omt_PixInterValue 	dr,
								omt_PixInterValue 	dg,
								omt_PixInterValue 	db,		
								omt_PixInterValue 	ida,
								omt_PixInterValue 	idr,
								omt_PixInterValue 	idg,
								omt_PixInterValue 	idb,
								omt_PixInterValue 	sr,				// Specular
								omt_PixInterValue 	sg,
								omt_PixInterValue 	sb,		
								omt_PixInterValue 	isr,
								omt_PixInterValue 	isg,
								omt_PixInterValue 	isb,
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	omtshared static void fill_text_flat_zb32(
								unsigned long *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short	 	da,				// Diffuse
								unsigned short	 	dr,
								unsigned short	 	dg,
								unsigned short	 	db,		
								unsigned short	 	sr,				// Specular
								unsigned short	 	sg,
								unsigned short	 	sb,		
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	omtshared static void fill_text_flat_nomod_zb32(
								unsigned long *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void fill_text_generic_32(
								unsigned long *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
								omt_PixInterValue 	da,				// Diffuse
								omt_PixInterValue 	dr,
								omt_PixInterValue 	dg,
								omt_PixInterValue 	db,		
								omt_PixInterValue 	ida,
								omt_PixInterValue 	idr,
								omt_PixInterValue 	idg,
								omt_PixInterValue 	idb,
								omt_PixInterValue 	sr,				// Specular
								omt_PixInterValue 	sg,
								omt_PixInterValue 	sb,		
								omt_PixInterValue 	isr,
								omt_PixInterValue 	isg,
								omt_PixInterValue 	isb,
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void fill_text_flat_32(
								unsigned long *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short	 	da,				// Diffuse
								unsigned short	 	dr,
								unsigned short	 	dg,
								unsigned short	 	db,		
								unsigned short	 	sr,				// Specular
								unsigned short	 	sg,
								unsigned short	 	sb,		
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void fill_text_flat_nomod_32(unsigned long *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	// T32 to 16 bits

	omtshared static void fill_text_generic_zb16(
								unsigned short *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								omt_PixInterValue 	da,				// Diffuse
								omt_PixInterValue 	dr,
								omt_PixInterValue 	dg,
								omt_PixInterValue 	db,		
								omt_PixInterValue 	ida,
								omt_PixInterValue 	idr,
								omt_PixInterValue 	idg,
								omt_PixInterValue 	idb,
								omt_PixInterValue 	sr,				// Specular
								omt_PixInterValue 	sg,
								omt_PixInterValue 	sb,		
								omt_PixInterValue 	isr,
								omt_PixInterValue 	isg,
								omt_PixInterValue 	isb,
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	omtshared static void fill_text_flat_zb16(
								unsigned short *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short	 	da,				// Diffuse
								unsigned short	 	dr,
								unsigned short	 	dg,
								unsigned short	 	db,		
								unsigned short	 	sr,				// Specular
								unsigned short	 	sg,
								unsigned short	 	sb,		
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	omtshared static void fill_text_flat_nomod_zb16(
								unsigned short *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void fill_text_generic_16(
								unsigned short *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
								omt_PixInterValue 	da,				// Diffuse
								omt_PixInterValue 	dr,
								omt_PixInterValue 	dg,
								omt_PixInterValue 	db,		
								omt_PixInterValue 	ida,
								omt_PixInterValue 	idr,
								omt_PixInterValue 	idg,
								omt_PixInterValue 	idb,
								omt_PixInterValue 	sr,				// Specular
								omt_PixInterValue 	sg,
								omt_PixInterValue 	sb,		
								omt_PixInterValue 	isr,
								omt_PixInterValue 	isg,
								omt_PixInterValue 	isb,
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void fill_text_flat_16(
								unsigned short *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short	 	da,				// Diffuse
								unsigned short	 	dr,
								unsigned short	 	dg,
								unsigned short	 	db,		
								unsigned short	 	sr,				// Specular
								unsigned short	 	sg,
								unsigned short	 	sb,		
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void fill_text_flat_nomod_16(unsigned short *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	// T16 to 32 bits

	omtshared static void t16_fill_text_generic_zb32(
								unsigned long *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								omt_PixInterValue 	da,				// Diffuse
								omt_PixInterValue 	dr,
								omt_PixInterValue 	dg,
								omt_PixInterValue 	db,		
								omt_PixInterValue 	ida,
								omt_PixInterValue 	idr,
								omt_PixInterValue 	idg,
								omt_PixInterValue 	idb,
								omt_PixInterValue 	sr,				// Specular
								omt_PixInterValue 	sg,
								omt_PixInterValue 	sb,		
								omt_PixInterValue 	isr,
								omt_PixInterValue 	isg,
								omt_PixInterValue 	isb,
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	omtshared static void t16_fill_text_flat_zb32(
								unsigned long *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short	 	da,				// Diffuse
								unsigned short	 	dr,
								unsigned short	 	dg,
								unsigned short	 	db,		
								unsigned short	 	sr,				// Specular
								unsigned short	 	sg,
								unsigned short	 	sb,		
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	omtshared static void t16_fill_text_flat_nomod_zb32(
								unsigned long *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void t16_fill_text_generic_32(
								unsigned long *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
								omt_PixInterValue 	da,				// Diffuse
								omt_PixInterValue 	dr,
								omt_PixInterValue 	dg,
								omt_PixInterValue 	db,		
								omt_PixInterValue 	ida,
								omt_PixInterValue 	idr,
								omt_PixInterValue 	idg,
								omt_PixInterValue 	idb,
								omt_PixInterValue 	sr,				// Specular
								omt_PixInterValue 	sg,
								omt_PixInterValue 	sb,		
								omt_PixInterValue 	isr,
								omt_PixInterValue 	isg,
								omt_PixInterValue 	isb,
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void t16_fill_text_flat_32(
								unsigned long *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short	 	da,				// Diffuse
								unsigned short	 	dr,
								unsigned short	 	dg,
								unsigned short	 	db,		
								unsigned short	 	sr,				// Specular
								unsigned short	 	sg,
								unsigned short	 	sb,		
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void t16_fill_text_flat_nomod_32(unsigned long *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	// T16 to 16 bits

	omtshared static void t16_fill_text_generic_zb16(
								unsigned short *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								omt_PixInterValue 	da,				// Diffuse
								omt_PixInterValue 	dr,
								omt_PixInterValue 	dg,
								omt_PixInterValue 	db,		
								omt_PixInterValue 	ida,
								omt_PixInterValue 	idr,
								omt_PixInterValue 	idg,
								omt_PixInterValue 	idb,
								omt_PixInterValue 	sr,				// Specular
								omt_PixInterValue 	sg,
								omt_PixInterValue 	sb,		
								omt_PixInterValue 	isr,
								omt_PixInterValue 	isg,
								omt_PixInterValue 	isb,
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	omtshared static void t16_fill_text_flat_zb16(
								unsigned short *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short	 	da,				// Diffuse
								unsigned short	 	dr,
								unsigned short	 	dg,
								unsigned short	 	db,		
								unsigned short	 	sr,				// Specular
								unsigned short	 	sg,
								unsigned short	 	sb,		
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);


	omtshared static void t16_fill_text_flat_nomod_zb16(
								unsigned short *dest,				// Buffer
								unsigned char *text,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u1,
								long				v1,
								long				inv_w1,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void t16_fill_text_generic_16(
								unsigned short *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
								omt_PixInterValue 	da,				// Diffuse
								omt_PixInterValue 	dr,
								omt_PixInterValue 	dg,
								omt_PixInterValue 	db,		
								omt_PixInterValue 	ida,
								omt_PixInterValue 	idr,
								omt_PixInterValue 	idg,
								omt_PixInterValue 	idb,
								omt_PixInterValue 	sr,				// Specular
								omt_PixInterValue 	sg,
								omt_PixInterValue 	sb,		
								omt_PixInterValue 	isr,
								omt_PixInterValue 	isg,
								omt_PixInterValue 	isb,
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void t16_fill_text_flat_16(
								unsigned short *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
								unsigned short	 	da,				// Diffuse
								unsigned short	 	dr,
								unsigned short	 	dg,
								unsigned short	 	db,		
								unsigned short	 	sr,				// Specular
								unsigned short	 	sg,
								unsigned short	 	sb,		
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);

	omtshared static void t16_fill_text_flat_nomod_16(unsigned short *dest,				// Buffer
								unsigned char *texture,				// Texture
								unsigned long text_maskx,
								unsigned long text_masky,
						 		unsigned long text_rowbytes_shifter,
								long				u,
								long				v,
								long				inv_w,
								long				u2,
								long				v2,
								long				inv_w2,								
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest);
};

#endif
#endif

