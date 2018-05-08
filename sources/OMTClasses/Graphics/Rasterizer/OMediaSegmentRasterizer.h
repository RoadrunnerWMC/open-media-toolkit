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
#ifndef OMEDIA_SegmentRasterizer_H
#define OMEDIA_SegmentRasterizer_H

#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_OMTRASTERIZER

#include "OMediaTypes.h"
#include "OMediaBlitter.h"
#include "OMediaFixedPoint.h"
#include "OMediaRendererInterface.h"
#include "OMediaBlendTable.h"

#define omd_PixelInterpolationFixedPoint


#ifdef omd_PixelInterpolationFixedPoint
typedef omt_FixedPoint16_16 omt_PixInterValue;

#define omd_FloatToPixInter(v) omd_FloatToFixed16_16(v)
#define omd_PixInterToFloat(v) omd_FixedToFloat16_16(v)

#define omd_IntToPixInter(v) omd_IntToFixed16_16(v) 
#define omd_PixInterToInt(v) omd_FixedToInt16_16(v)

#define omd_ScaleDownPixInter(v) omd_FixedToInt16_16(v)
#define omd_PixInterCast(v) ((long)(v))

#else

typedef float omt_PixInterValue;

#define omd_FloatToPixInter(v) (v)
#define omd_PixInterToFloat(v) (v)

#define omd_IntToPixInter(v) ((float)(v))
#define omd_PixInterToInt(v) ((long)(v))

#define omd_ScaleDownPixInter(v) (v)
#define omd_PixInterCast(v) ((float)(v))
#endif


class OMediaSegmentRasterizer : public OMediaBlendTable
{
	public:
	
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

	omtshared static void fill_color_zbuffer32(unsigned long		*dest,
									unsigned long		color,
									unsigned short		*zbuffer,	
									unsigned long		zl,
									long		   		zi,					
									unsigned short		count,
									omt_ZBufferFunc		zfunc,
									omt_ZBufferTest		ztest,
									omt_ZBufferWrite	zwrite);


	omtshared static void fill_color_interpolation32(unsigned char *dest,		
								omt_PixInterValue r,		// Start color, 8 bits per gun
								omt_PixInterValue g,
								omt_PixInterValue b,		
								omt_PixInterValue ir,		// Linear interpolation value
								omt_PixInterValue ig,
								omt_PixInterValue ib,
							        unsigned short count);	// Count in pixel (not bytes!)

	omtshared static void fill_color_interpolation_zb32(unsigned char *dest,
								omt_PixInterValue 	r,
								omt_PixInterValue 	g,
								omt_PixInterValue 	b,		
								omt_PixInterValue 	ir,
								omt_PixInterValue 	ig,
								omt_PixInterValue 	ib,
								unsigned short		*zbuffer,	
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    unsigned short 		count);


	omtshared static void fill_color_blend32(unsigned char		*dest,
											unsigned long		color,				
											unsigned short		count,
											omt_BlendFunc		blend_src, 
											omt_BlendFunc		blend_dest);


	omtshared static void fill_color_blend_zbuffer32(unsigned long		*dest,
									unsigned long		color,
									unsigned short		*zbuffer,	
									unsigned long		zl,
									long		   		zi,					
									unsigned short		count,
									omt_ZBufferFunc		zfunc,
									omt_ZBufferTest		ztest,
									omt_ZBufferWrite	zwrite,
									omt_BlendFunc		blend_src, 
									omt_BlendFunc		blend_dest);


	omtshared static void fill_color_blend_interpolation32(unsigned char *dest,		
								omt_PixInterValue ga,
								omt_PixInterValue gr,
								omt_PixInterValue gg,
								omt_PixInterValue gb,		
								omt_PixInterValue ia,		
								omt_PixInterValue ir,		
								omt_PixInterValue ig,
								omt_PixInterValue ib,
						        unsigned short count,
								omt_BlendFunc		blend_src, 
								omt_BlendFunc		blend_dest);

	omtshared static void fill_color_blend_interpolation_zb32(unsigned char *dest,
								omt_PixInterValue 	ga,
								omt_PixInterValue 	gr,
								omt_PixInterValue 	gg,
								omt_PixInterValue 	gb,		
								omt_PixInterValue 	ia,
								omt_PixInterValue 	ir,
								omt_PixInterValue 	ig,
								omt_PixInterValue 	ib,
								unsigned short		*zbuffer,	
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    unsigned short 		count,
								omt_BlendFunc		blend_src, 
								omt_BlendFunc		blend_dest);


	// 16bits

	omtshared static void fill_color_zbuffer16(unsigned short		*dest,
									unsigned long		color,
									unsigned short		*zbuffer,	
									unsigned long		zl,
									long		   		zi,					
									unsigned short		count,
									omt_ZBufferFunc		zfunc,
									omt_ZBufferTest		ztest,
									omt_ZBufferWrite	zwrite);


	omtshared static void fill_color_interpolation16(unsigned char *dest,		
								omt_PixInterValue r,	
								omt_PixInterValue g,
								omt_PixInterValue b,		
								omt_PixInterValue ir,	
								omt_PixInterValue ig,
								omt_PixInterValue ib,
							        unsigned short count);	

	omtshared static void fill_color_interpolation_zb16(unsigned char *dest,
								omt_PixInterValue 	r,
								omt_PixInterValue 	g,
								omt_PixInterValue 	b,		
								omt_PixInterValue 	ir,
								omt_PixInterValue 	ig,
								omt_PixInterValue 	ib,
								unsigned short		*zbuffer,	
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    unsigned short 		count);


	omtshared static void fill_color_blend16(unsigned char		*dest,
											unsigned long		color,				
											unsigned short		count,
											omt_BlendFunc		blend_src, 
											omt_BlendFunc		blend_dest);


	omtshared static void fill_color_blend_zbuffer16(unsigned short		*dest,
									unsigned long		color,
									unsigned short		*zbuffer,	
									unsigned long		zl,
									long		   		zi,					
									unsigned short		count,
									omt_ZBufferFunc		zfunc,
									omt_ZBufferTest		ztest,
									omt_ZBufferWrite	zwrite,
									omt_BlendFunc		blend_src, 
									omt_BlendFunc		blend_dest);


	omtshared static void fill_color_blend_interpolation16(unsigned char *dest,		
								omt_PixInterValue ga,
								omt_PixInterValue gr,
								omt_PixInterValue gg,
								omt_PixInterValue gb,		
								omt_PixInterValue ia,		
								omt_PixInterValue ir,		
								omt_PixInterValue ig,
								omt_PixInterValue ib,
						        unsigned short count,
								omt_BlendFunc		blend_src, 
								omt_BlendFunc		blend_dest);

	omtshared static void fill_color_blend_interpolation_zb16(unsigned char *dest,
								omt_PixInterValue 	ga,
								omt_PixInterValue 	gr,
								omt_PixInterValue 	gg,
								omt_PixInterValue 	gb,		
								omt_PixInterValue 	ia,
								omt_PixInterValue 	ir,
								omt_PixInterValue 	ig,
								omt_PixInterValue 	ib,
								unsigned short		*zbuffer,	
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    unsigned short 		count,
								omt_BlendFunc		blend_src, 
								omt_BlendFunc		blend_dest);


//------------------------------------------------
// Line support

	omtshared static void draw_line_32(unsigned long 	*dest_buffer,
							 			long 				buffer_rowbytes,
							 			short				x1,
							 			short				y1,
							 			short				x2,
							 			short				y2,
                		     			unsigned long 		color,
		                     			omt_BlendFunc		blend_src, 
										omt_BlendFunc		blend_dest);
								
	omtshared static void draw_line_16(unsigned short 	*dest_buffer,
							 			long 			buffer_rowbytes,
							 			short			x1,
							 			short			y1,
							 			short			x2,
							 			short			y2,
                		     			unsigned long 	argb,
		                     			omt_BlendFunc	blend_src, 
										omt_BlendFunc	blend_dest);

	omtshared static void draw_line_zb32(unsigned long 	*dest_buffer,
							 			long 			buffer_rowbytes,
							 			short			x1,
							 			short			y1,
							 			float			z1,
							 			short			x2,
							 			short			y2,
							 			float			z2,
                		     			unsigned long 	argb,
		                     			omt_BlendFunc	blend_src, 
										omt_BlendFunc	blend_dest,
										unsigned short	*zbuffer,
										unsigned long	zbuffer_rowbytes,	
										omt_ZBufferFunc		zfunc,
										omt_ZBufferTest		ztest,
										omt_ZBufferWrite	zwrite);					
								
	omtshared static void draw_line_zb16(unsigned short 	*dest_buffer,
							 			long 			buffer_rowbytes,
							 			short			x1,
							 			short			y1,
							 			float			z1,
							 			short			x2,
							 			short			y2,
							 			float			z2,
                		     			unsigned long 	argb,
		                     			omt_BlendFunc	blend_src, 
										omt_BlendFunc	blend_dest,
										unsigned short	*zbuffer,
										unsigned long	zbuffer_rowbytes,	
										omt_ZBufferFunc		zfunc,
										omt_ZBufferTest		ztest,
										omt_ZBufferWrite	zwrite);



};

#endif
#endif

