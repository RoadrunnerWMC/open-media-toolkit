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
 

#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_OMTRASTERIZER

#include "OMediaTextSegmentRasterizer.h"

#define omd_DOBLEND(res,gun) 	{res = (src_argb[gun] + dest_argb[gun]);		\
								if (short(res)<0) res = 0; else if (short(res)>0xFF) res = 0xFF;}

// Init texture interpolant

#define omd_INITTEXT_INTER			\
	if (count==0) return;			\
	du 		= ((u2-u) / count);		\
	dv 		= ((v2-v) / count);		\
	dinv_w	= ((inv_w2-inv_w) / count);	\
	du<<=3L;							\
	dv<<=3L;							\
	dinv_w<<=3L;						\
	cx2 = (u/(inv_w>>omd_INVW_SHIFTBIT))<<(omd_LINEAR_FP-omd_COORD_FPEX);	\
	cy2 = (v/(inv_w>>omd_INVW_SHIFTBIT))<<(omd_LINEAR_FP-omd_COORD_FPEX);	\
	dest_end = dest + (count&(~7));

#define omd_TEXT_INTERNEXT8			\
		u+=du;						\
		v+=dv;						\
		inv_w+=dinv_w;				\
		cx  = cx2;					\
		cy  = cy2;					\
		cx2 = (u/(inv_w>>omd_INVW_SHIFTBIT))<<(omd_LINEAR_FP-omd_COORD_FPEX);	\
		cy2 = (v/(inv_w>>omd_INVW_SHIFTBIT))<<(omd_LINEAR_FP-omd_COORD_FPEX);	\
		dcx = (cx2-cx)>>3L;				\
		dcy = (cy2-cy)>>3L;

#define omd_TEXT_INTERREMAIN			\
		du=(du>>3L)*count;				\
		dv=(dv>>3L)*count;				\
		dinv_w=(dinv_w>>3L)*count;		\
		u+=du;							\
		v+=dv;							\
		inv_w+=dinv_w;					\
		cx  = cx2;						\
		cy  = cy2;						\
		cx2 = (u/(inv_w>>omd_INVW_SHIFTBIT))<<(omd_LINEAR_FP-omd_COORD_FPEX);	\
		cy2 = (v/(inv_w>>omd_INVW_SHIFTBIT))<<(omd_LINEAR_FP-omd_COORD_FPEX);	\
		dcx = (cx2-cx)/count;		\
		dcy = (cy2-cy)/count;		\


//-------------------------------------------------------------------------
// Generic zbuffer 16 bits macro 	

#define omd_GENZB16T32PIX(npix)													\
		z = zl>>16L; 															\
																				\
		if (do_ztest(z, zbuffer[npix], zfunc,ztest))								\
		{																		\
			if (zwrite==omzbwc_Enabled) zbuffer[npix] = z; 						\
																				\
																				\
			p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
											  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																													\
			omd_ARGBUnPackPixelA32(p,a,r,g,b);							\
																		\
																\
			a = ((a * omd_PixInterToInt(da) )>>8);									\
			r = ((r * omd_PixInterToInt(dr))>>8) + omd_PixInterToInt(sr);	if (r>0xFF) r = 0xFF;		\
			g = ((g * omd_PixInterToInt(dg))>>8) + omd_PixInterToInt(sg);	if (g>0xFF) g = 0xFF;		\
			b = ((b * omd_PixInterToInt(db))>>8) + omd_PixInterToInt(sb);	if (b>0xFF) b = 0xFF;		\
																\
																\
			color = omd_ARGBPackPixelA32(a,r,g,b);				\
			dcolor = dest[npix]; dcolor = omd_ARGB16to32(dcolor);		\
																										\
			bsrc_func(&color, &color, &dcolor, src_argb);								\
			bdest_func(&dcolor, &color, &dcolor, dest_argb);			\
									\
	    	omd_DOBLEND(a,0);		\
	    	omd_DOBLEND(r,1);		\
	    	omd_DOBLEND(g,2);		\
	    	omd_DOBLEND(b,3);		\
	 								\
			dest[npix] = omd_ARGBPackPixelA16(a,r,g,b);     		\
		}						\
								\
		zl +=zi;				\
		da += ida;				\
		dr += idr;				\
		dg += idg;				\
		db += idb;				\
		sr += isr;  sg += isg; sb += isb;  	      	\
		cx+=dcx; cy+=dcy;   	
								
// Generic zbuffer 16 bits macro, optimized path, blend 	

#define omd_GENZB16T32PIX_OPT_BLEND(npix)										\
		z = zl>>16L; 															\
																				\
		if (z<zbuffer[npix]) 												\
		{																		\
			if (dowritez) zbuffer[npix] = z; 									\
																				\
																				\
			p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
											  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																													\
			omd_ARGBUnPackPixelA32(p,a,r,g,b);							\
																		\
			if (a)												\
			{													\
																\
				a = ((a * omd_PixInterToInt(da) )>>8);									\
				r = ((r * omd_PixInterToInt(dr))>>8) + omd_PixInterToInt(sr);	if (r>0xFF) r = 0xFF;		\
				g = ((g * omd_PixInterToInt(dg))>>8) + omd_PixInterToInt(sg);	if (g>0xFF) g = 0xFF;		\
				b = ((b * omd_PixInterToInt(db))>>8) + omd_PixInterToInt(sb);	if (b>0xFF) b = 0xFF;		\
																	\
				dcolor = dest[npix];								\
				omd_ARGBUnPackPixel16(dcolor,dstr,dstg,dstb);		\
																\
				sna = 0xFF-a;										\
																	\
				r = (r * a + sna * dstr) >> 8;		\
				g = (g * a + sna * dstg) >> 8;		\
				b = (b * a + sna * dstb) >> 8;		\
													 								\
				dest[npix] = omd_ARGBPackPixel16(r,g,b);  	 			  				\
			}																			\
		}						\
								\
		zl +=zi;				\
		da += ida;				\
		dr += idr;				\
		dg += idg;				\
		db += idb;				\
		sr += isr;  sg += isg; sb += isb;  	      	\
		cx+=dcx; cy+=dcy;   	

// Generic zbuffer 16 bits macro, optimized path, no blend 	

#define omd_GENZB16T32PIX_OPT(npix)										\
		z = zl>>16L; 															\
																				\
		if (z<zbuffer[npix]) 												\
		{																		\
			zbuffer[npix] = z; 									\
																				\
																				\
			p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
											  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																													\
			omd_ARGBUnPackPixel32(p,r,g,b);							\
																\
			r = ((r * omd_PixInterToInt(dr))>>8) + omd_PixInterToInt(sr);	if (r>0xFF) r = 0xFF;		\
			g = ((g * omd_PixInterToInt(dg))>>8) + omd_PixInterToInt(sg);	if (g>0xFF) g = 0xFF;		\
			b = ((b * omd_PixInterToInt(db))>>8) + omd_PixInterToInt(sb);	if (b>0xFF) b = 0xFF;		\
																\
														 								\
			dest[npix] = omd_ARGBPackPixel16(r,g,b);  	 			  				\
		}						\
								\
		zl +=zi;				\
		dr += idr;				\
		dg += idg;				\
		db += idb;				\
		sr += isr;  sg += isg; sb += isb;  	      	\
		cx+=dcx; cy+=dcy;   	
								
								


void OMediaTextSegmentRasterizer::fill_text_generic_zb16(
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
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest)
{
	unsigned short		z,*dest_end;
	unsigned long		p,dcolor;
	long 				cx,cy,cx2,cy2,dcx,dcy;
	long				du,dv,dinv_w;

		// Generic version

	short src_argb[4];
	short dest_argb[4];
	unsigned short sna,a,r,g,b,dstr,dstg,dstb;
	unsigned long color;


	omd_INITTEXT_INTER;

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled)
	{
		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && zwrite==omzbwc_Enabled)
		{
			// Optimized path
		
			while(dest!=dest_end)
			{
				omd_TEXT_INTERNEXT8;
			
				omd_GENZB16T32PIX_OPT(0);
				omd_GENZB16T32PIX_OPT(1);
				omd_GENZB16T32PIX_OPT(2);
				omd_GENZB16T32PIX_OPT(3);
				omd_GENZB16T32PIX_OPT(4);
				omd_GENZB16T32PIX_OPT(5);
				omd_GENZB16T32PIX_OPT(6);
				omd_GENZB16T32PIX_OPT(7);
				
				zbuffer += 8;
				dest +=8;
			}
		
			count = count&7;
			if (count)
			{
				omd_TEXT_INTERREMAIN;
				
				switch(count)
				{											 
					case 7:	omd_GENZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 6:	omd_GENZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 5:	omd_GENZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 4:	omd_GENZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 3:	omd_GENZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 2:	omd_GENZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 1:	omd_GENZB16T32PIX_OPT(0);
				}
			}
			
			return;		
		}
	
		if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
		{
			// Optimized path for blending
	
			bool dowritez = (zwrite==omzbwc_Enabled);
	
			while(dest!=dest_end)
			{
				omd_TEXT_INTERNEXT8;
			
				omd_GENZB16T32PIX_OPT_BLEND(0);
				omd_GENZB16T32PIX_OPT_BLEND(1);
				omd_GENZB16T32PIX_OPT_BLEND(2);
				omd_GENZB16T32PIX_OPT_BLEND(3);
				omd_GENZB16T32PIX_OPT_BLEND(4);
				omd_GENZB16T32PIX_OPT_BLEND(5);
				omd_GENZB16T32PIX_OPT_BLEND(6);
				omd_GENZB16T32PIX_OPT_BLEND(7);
				
				zbuffer += 8;
				dest +=8;
			}
		
			count = count&7;
			if (count)
			{
				omd_TEXT_INTERREMAIN;
				
				switch(count)
				{											 
					case 7:	omd_GENZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 6:	omd_GENZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 5:	omd_GENZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 4:	omd_GENZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 3:	omd_GENZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 2:	omd_GENZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 1:	omd_GENZB16T32PIX_OPT_BLEND(0);
				}
			}
			
			return;	
		}
	}

	omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
	omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);

	// Generic path

	while(dest!=dest_end)
	{
		omd_TEXT_INTERNEXT8;
	
		omd_GENZB16T32PIX(0);
		omd_GENZB16T32PIX(1);
		omd_GENZB16T32PIX(2);
		omd_GENZB16T32PIX(3);
		omd_GENZB16T32PIX(4);
		omd_GENZB16T32PIX(5);
		omd_GENZB16T32PIX(6);
		omd_GENZB16T32PIX(7);
		
		zbuffer += 8;
		dest +=8;
	}

	count = count&7;
	if (count)
	{
		omd_TEXT_INTERREMAIN;
		
		switch(count)
		{											 
			case 7:	omd_GENZB16T32PIX(0);	zbuffer++;	dest++;
			case 6:	omd_GENZB16T32PIX(0);	zbuffer++;	dest++;
			case 5:	omd_GENZB16T32PIX(0);	zbuffer++;	dest++;
			case 4:	omd_GENZB16T32PIX(0);	zbuffer++;	dest++;
			case 3:	omd_GENZB16T32PIX(0);	zbuffer++;	dest++;
			case 2:	omd_GENZB16T32PIX(0);	zbuffer++;	dest++;
			case 1:	omd_GENZB16T32PIX(0);		
		}
	}
}

//------------------------------------------------------------------
// Flat zbuffer 16 bits macro 	

#define omd_FLATZB16T32PIX(npix)													\
		z = zl>>16L; 															\
																				\
		if (do_ztest(z, zbuffer[npix], zfunc,ztest))								\
		{																		\
			if (zwrite==omzbwc_Enabled) zbuffer[npix] = z; 						\
																				\
																				\
			p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
											  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																													\
			omd_ARGBUnPackPixelA32(p,a,r,g,b);							\
																		\
																\
			a = ((a * (da))>>8);									\
			r = ((r * (dr))>>8) + (sr);	if (r>0xFF) r = 0xFF;		\
			g = ((g * (dg))>>8) + (sg);	if (g>0xFF) g = 0xFF;		\
			b = ((b * (db))>>8) + (sb);	if (b>0xFF) b = 0xFF;		\
																\
																\
			color = omd_ARGBPackPixelA32(a,r,g,b);				\
			dcolor = dest[npix];	dcolor = omd_ARGB16to32(dcolor);	\
																										\
			bsrc_func(&color, &color, &dcolor, src_argb);								\
			bdest_func(&dcolor, &color, &dcolor, dest_argb);			\
									\
	    	omd_DOBLEND(a,0);		\
	    	omd_DOBLEND(r,1);		\
	    	omd_DOBLEND(g,2);		\
	    	omd_DOBLEND(b,3);		\
	 								\
			dest[npix] = omd_ARGBPackPixelA16(a,r,g,b);     		\
		}						\
								\
		zl +=zi;				\
		cx+=dcx; cy+=dcy;   	
								
// Flat zbuffer 16 bits macro, optimized path, blend 	

#define omd_FLATZB16T32PIX_OPT_BLEND(npix)										\
		z = zl>>16L; 															\
																				\
		if (z<zbuffer[npix]) 												\
		{																		\
			if (dowritez) zbuffer[npix] = z; 									\
																				\
																				\
			p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
											  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																													\
			omd_ARGBUnPackPixelA32(p,a,r,g,b);							\
																		\
																\
			if (a)													\
			{														\
																\
				a = ((a * (da) )>>8);									\
				r = ((r * (dr))>>8) + (sr);	if (r>0xFF) r = 0xFF;		\
				g = ((g * (dg))>>8) + (sg);	if (g>0xFF) g = 0xFF;		\
				b = ((b * (db))>>8) + (sb);	if (b>0xFF) b = 0xFF;		\
																	\
				dcolor = dest[npix];								\
				omd_ARGBUnPackPixel16(dcolor,dstr,dstg,dstb);		\
				sna = 0xFF-a;										\
																	\
				r = (r * a + sna * dstr) >> 8;		\
				g = (g * a + sna * dstg) >> 8;		\
				b = (b * a + sna * dstb) >> 8;		\
													 								\
				dest[npix] = omd_ARGBPackPixel16(r,g,b);  	 			  				\
			}					\
		}						\
								\
		zl +=zi;				\
		cx+=dcx; cy+=dcy;   	

// Flat zbuffer 16 bits macro, optimized path, no blend 	

#define omd_FLATZB16T32PIX_OPT(npix)										\
		z = zl>>16L; 															\
																				\
		if (z<zbuffer[npix]) 												\
		{																		\
			zbuffer[npix] = z; 									\
																				\
																				\
			p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
											  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																													\
			omd_ARGBUnPackPixel32(p,r,g,b);							\
																\
			r = ((r * (dr))>>8) + (sr);	if (r>0xFF) r = 0xFF;		\
			g = ((g * (dg))>>8) + (sg);	if (g>0xFF) g = 0xFF;		\
			b = ((b * (db))>>8) + (sb);	if (b>0xFF) b = 0xFF;		\
																\
														 								\
			dest[npix] = omd_ARGBPackPixel16(r,g,b);  	 			  				\
		}						\
								\
		zl +=zi;				\
		cx+=dcx; cy+=dcy;   	
								
								

void OMediaTextSegmentRasterizer::fill_text_flat_zb16(
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
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest)
{
	unsigned short		z,*dest_end;
	unsigned long		p,dcolor;
	long 				cx,cy,cx2,cy2,dcx,dcy;
	long				du,dv,dinv_w;

		// Generic version

	short src_argb[4];
	short dest_argb[4];
	unsigned short sna,a,r,g,b,dstr,dstg,dstb;
	unsigned long color;


	omd_INITTEXT_INTER;

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled)
	{
		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && zwrite==omzbwc_Enabled)
		{
			// Optimized path
		
			while(dest!=dest_end)
			{
				omd_TEXT_INTERNEXT8;
			
				omd_FLATZB16T32PIX_OPT(0);
				omd_FLATZB16T32PIX_OPT(1);
				omd_FLATZB16T32PIX_OPT(2);
				omd_FLATZB16T32PIX_OPT(3);
				omd_FLATZB16T32PIX_OPT(4);
				omd_FLATZB16T32PIX_OPT(5);
				omd_FLATZB16T32PIX_OPT(6);
				omd_FLATZB16T32PIX_OPT(7);
				
				zbuffer += 8;
				dest +=8;
			}
		
			count = count&7;
			if (count)
			{
				omd_TEXT_INTERREMAIN;
				
				switch(count)
				{											 
					case 7:	omd_FLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 6:	omd_FLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 5:	omd_FLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 4:	omd_FLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 3:	omd_FLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 2:	omd_FLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 1:	omd_FLATZB16T32PIX_OPT(0);
				}
			}
			
			return;		
		}
	
		if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
		{
			// Optimized path for blending
	
			bool dowritez = (zwrite==omzbwc_Enabled);
	
			while(dest!=dest_end)
			{
				omd_TEXT_INTERNEXT8;
			
				omd_FLATZB16T32PIX_OPT_BLEND(0);
				omd_FLATZB16T32PIX_OPT_BLEND(1);
				omd_FLATZB16T32PIX_OPT_BLEND(2);
				omd_FLATZB16T32PIX_OPT_BLEND(3);
				omd_FLATZB16T32PIX_OPT_BLEND(4);
				omd_FLATZB16T32PIX_OPT_BLEND(5);
				omd_FLATZB16T32PIX_OPT_BLEND(6);
				omd_FLATZB16T32PIX_OPT_BLEND(7);
				
				zbuffer += 8;
				dest +=8;
			}
		
			count = count&7;
			if (count)
			{
				omd_TEXT_INTERREMAIN;
				
				switch(count)
				{											 
					case 7:	omd_FLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 6:	omd_FLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 5:	omd_FLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 4:	omd_FLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 3:	omd_FLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 2:	omd_FLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 1:	omd_FLATZB16T32PIX_OPT_BLEND(0);
				}
			}
			
			return;	
		}
	}

	omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
	omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);

	// Generic path

	while(dest!=dest_end)
	{
		omd_TEXT_INTERNEXT8;
	
		omd_FLATZB16T32PIX(0);
		omd_FLATZB16T32PIX(1);
		omd_FLATZB16T32PIX(2);
		omd_FLATZB16T32PIX(3);
		omd_FLATZB16T32PIX(4);
		omd_FLATZB16T32PIX(5);
		omd_FLATZB16T32PIX(6);
		omd_FLATZB16T32PIX(7);
		
		zbuffer += 8;
		dest +=8;
	}

	count = count&7;
	if (count)
	{
		omd_TEXT_INTERREMAIN;
		
		switch(count)
		{											 
			case 7:	omd_FLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 6:	omd_FLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 5:	omd_FLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 4:	omd_FLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 3:	omd_FLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 2:	omd_FLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 1:	omd_FLATZB16T32PIX(0);		
		}
	}
}



//------------------------------------------------------------------
// Flat zbuffer 16 bits macro, no modulation 	

#define omd_NOMODFLATZB16T32PIX(npix)													\
		z = zl>>16L; 															\
																				\
		if (do_ztest(z, zbuffer[npix], zfunc,ztest))								\
		{																		\
			if (zwrite==omzbwc_Enabled) zbuffer[npix] = z; 						\
																				\
																				\
			color = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
											  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																													\
																										\
			dcolor = dest[npix];	dcolor = omd_ARGB16to32(dcolor);	\
																										\
			bsrc_func(&color, &color, &dcolor, src_argb);								\
			bdest_func(&dcolor, &color, &dcolor, dest_argb);			\
									\
	    	omd_DOBLEND(a,0);		\
	    	omd_DOBLEND(r,1);		\
	    	omd_DOBLEND(g,2);		\
	    	omd_DOBLEND(b,3);		\
	 								\
			dest[npix] = omd_ARGBPackPixelA16(a,r,g,b);     		\
		}						\
								\
		zl +=zi;				\
		cx+=dcx; cy+=dcy;   	
								
// Flat zbuffer 16 bits macro, optimized path, blend, no modulation  	

#define omd_NOMODFLATZB16T32PIX_OPT_BLEND(npix)										\
		z = zl>>16L; 															\
																				\
		if (z<zbuffer[npix]) 												\
		{																		\
			if (dowritez) zbuffer[npix] = z; 									\
																				\
																				\
			p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
											  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																													\
			omd_ARGBUnPackPixelA32(p,a,r,g,b);							\
																		\
			if (a)													\
			{														\
				dcolor = dest[npix];								\
				omd_ARGBUnPackPixel16(dcolor,dstr,dstg,dstb);		\
																\
				sna = 0xFF-a;										\
																	\
				r = (r * a + sna * dstr) >> 8;		\
				g = (g * a + sna * dstg) >> 8;		\
				b = (b * a + sna * dstb) >> 8;		\
																						\
													 									\
				dest[npix] = omd_ARGBPackPixel16(r,g,b);  	 			  				\
			}					\
		}						\
								\
		zl +=zi;				\
		cx+=dcx; cy+=dcy;   	

// Flat zbuffer 16 bits macro, optimized path, no blend, no modulation   	

#define omd_NOMODFLATZB16T32PIX_OPT(npix)										\
		z = zl>>16L; 															\
																				\
		if (z<zbuffer[npix]) 												\
		{																		\
			zbuffer[npix] = z; 									\
																				\
																				\
			dcolor = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
											  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
			dest[npix] = omd_ARGB32to16(dcolor);														\
		}						\
								\
		zl +=zi;				\
		cx+=dcx; cy+=dcy;   	
								
void OMediaTextSegmentRasterizer::fill_text_flat_nomod_zb16(unsigned short *dest,				// Buffer
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
								unsigned short		*zbuffer,		// Zbuffer
								unsigned long		zl,
								long		   		zi,					
								omt_ZBufferFunc		zfunc,
								omt_ZBufferTest		ztest,
								omt_ZBufferWrite	zwrite,						
							    long 				count,			// Count
								omt_BlendFunc		blend_src, 		// Blending
								omt_BlendFunc		blend_dest)
{
	unsigned short		z,*dest_end;
	unsigned long		p,dcolor;
	long 				cx,cy,cx2,cy2,dcx,dcy;
	long				du,dv,dinv_w;

		// Generic version

	short src_argb[4];
	short dest_argb[4];
	unsigned short sna,a,r,g,b,dstr,dstg,dstb;
	unsigned long color;


	omd_INITTEXT_INTER;

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled)
	{
		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && zwrite==omzbwc_Enabled)
		{
			// Optimized path
		
			while(dest!=dest_end)
			{
				omd_TEXT_INTERNEXT8;
			
				omd_NOMODFLATZB16T32PIX_OPT(0);
				omd_NOMODFLATZB16T32PIX_OPT(1);
				omd_NOMODFLATZB16T32PIX_OPT(2);
				omd_NOMODFLATZB16T32PIX_OPT(3);
				omd_NOMODFLATZB16T32PIX_OPT(4);
				omd_NOMODFLATZB16T32PIX_OPT(5);
				omd_NOMODFLATZB16T32PIX_OPT(6);
				omd_NOMODFLATZB16T32PIX_OPT(7);
				
				zbuffer += 8;
				dest +=8;
			}
		
			count = count&7;
			if (count)
			{
				omd_TEXT_INTERREMAIN;
				
				switch(count)
				{											 
					case 7:	omd_NOMODFLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 6:	omd_NOMODFLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 5:	omd_NOMODFLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 4:	omd_NOMODFLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 3:	omd_NOMODFLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 2:	omd_NOMODFLATZB16T32PIX_OPT(0);	zbuffer++;	dest++;
					case 1:	omd_NOMODFLATZB16T32PIX_OPT(0);
				}
			}
			
			return;		
		}
	
		if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
		{
			// Optimized path for blending
	
			bool dowritez = (zwrite==omzbwc_Enabled);
	
			while(dest!=dest_end)
			{
				omd_TEXT_INTERNEXT8;
			
				omd_NOMODFLATZB16T32PIX_OPT_BLEND(0);
				omd_NOMODFLATZB16T32PIX_OPT_BLEND(1);
				omd_NOMODFLATZB16T32PIX_OPT_BLEND(2);
				omd_NOMODFLATZB16T32PIX_OPT_BLEND(3);
				omd_NOMODFLATZB16T32PIX_OPT_BLEND(4);
				omd_NOMODFLATZB16T32PIX_OPT_BLEND(5);
				omd_NOMODFLATZB16T32PIX_OPT_BLEND(6);
				omd_NOMODFLATZB16T32PIX_OPT_BLEND(7);
				
				zbuffer += 8;
				dest +=8;
			}
		
			count = count&7;
			if (count)
			{
				omd_TEXT_INTERREMAIN;
				
				switch(count)
				{											 
					case 7:	omd_NOMODFLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 6:	omd_NOMODFLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 5:	omd_NOMODFLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 4:	omd_NOMODFLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 3:	omd_NOMODFLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 2:	omd_NOMODFLATZB16T32PIX_OPT_BLEND(0);	zbuffer++;	dest++;
					case 1:	omd_NOMODFLATZB16T32PIX_OPT_BLEND(0);
				}
			}
			
			return;	
		}
	}

	omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
	omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);

	// Generic path

	while(dest!=dest_end)
	{
		omd_TEXT_INTERNEXT8;
	
		omd_NOMODFLATZB16T32PIX(0);
		omd_NOMODFLATZB16T32PIX(1);
		omd_NOMODFLATZB16T32PIX(2);
		omd_NOMODFLATZB16T32PIX(3);
		omd_NOMODFLATZB16T32PIX(4);
		omd_NOMODFLATZB16T32PIX(5);
		omd_NOMODFLATZB16T32PIX(6);
		omd_NOMODFLATZB16T32PIX(7);
		
		zbuffer += 8;
		dest +=8;
	}

	count = count&7;
	if (count)
	{
		omd_TEXT_INTERREMAIN;
		
		switch(count)
		{											 
			case 7:	omd_NOMODFLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 6:	omd_NOMODFLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 5:	omd_NOMODFLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 4:	omd_NOMODFLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 3:	omd_NOMODFLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 2:	omd_NOMODFLATZB16T32PIX(0);	zbuffer++;	dest++;
			case 1:	omd_NOMODFLATZB16T32PIX(0);		
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Generic 16 bits macro 	

#define omd_GEN16T32PIX(npix)													\
																				\
		p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
										  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																												\
		omd_ARGBUnPackPixelA32(p,a,r,g,b);										\
																				\
																				\
		a = ((a * omd_PixInterToInt(da))>>8);									\
		r = ((r * omd_PixInterToInt(dr))>>8) + omd_PixInterToInt(sr);	if (r>0xFF) r = 0xFF;		\
		g = ((g * omd_PixInterToInt(dg))>>8) + omd_PixInterToInt(sg);	if (g>0xFF) g = 0xFF;		\
		b = ((b * omd_PixInterToInt(db))>>8) + omd_PixInterToInt(sb);	if (b>0xFF) b = 0xFF;		\
																	\
																	\
		color = omd_ARGBPackPixelA32(a,r,g,b);						\
		dcolor = dest[npix];	dcolor = omd_ARGB16to32(dcolor);	\
																	\
		bsrc_func(&color, &color, &dcolor, src_argb);				\
		bdest_func(&dcolor, &color, &dcolor, dest_argb);			\
								\
    	omd_DOBLEND(a,0);		\
    	omd_DOBLEND(r,1);		\
    	omd_DOBLEND(g,2);		\
    	omd_DOBLEND(b,3);		\
 								\
		dest[npix] = omd_ARGBPackPixelA16(a,r,g,b);     		\
								\
		da += ida;				\
		dr += idr;				\
		dg += idg;				\
		db += idb;				\
		sr += isr;  sg += isg; sb += isb;  	      	\
		cx+=dcx; cy+=dcy;   	
								
// Generic 16 bits macro, optimized path, blend 	

#define omd_GEN16T32PIX_OPT_BLEND(npix)										\
																				\
																				\
		p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
										  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																												\
		omd_ARGBUnPackPixelA32(p,a,r,g,b);							\
																	\
		if (a)													\
		{														\
															\
			a = ((a * omd_PixInterToInt(da) )>>8);									\
			r = ((r * omd_PixInterToInt(dr))>>8) + omd_PixInterToInt(sr);	if (r>0xFF) r = 0xFF;		\
			g = ((g * omd_PixInterToInt(dg))>>8) + omd_PixInterToInt(sg);	if (g>0xFF) g = 0xFF;		\
			b = ((b * omd_PixInterToInt(db))>>8) + omd_PixInterToInt(sb);	if (b>0xFF) b = 0xFF;		\
															\
			dcolor = dest[npix];								\
			omd_ARGBUnPackPixel16(dcolor,dstr,dstg,dstb);		\
															\
			sna = 0xFF-a;										\
																\
			r = (r * a + sna * dstr) >> 8;		\
			g = (g * a + sna * dstg) >> 8;		\
			b = (b * a + sna * dstb) >> 8;		\
																					\
												 									\
			dest[npix] = omd_ARGBPackPixel16(r,g,b);  	 			  				\
		}								\
								\
		da += ida;				\
		dr += idr;				\
		dg += idg;				\
		db += idb;				\
		sr += isr;  sg += isg; sb += isb;  	      	\
		cx+=dcx; cy+=dcy;   	

// Generic 16 bits macro, optimized path, no blend 	

#define omd_GEN16T32PIX_OPT(npix)										\
																				\
																				\
		p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
										  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																												\
		omd_ARGBUnPackPixel32(p,r,g,b);							\
															\
		r = ((r * omd_PixInterToInt(dr))>>8) + omd_PixInterToInt(sr);	if (r>0xFF) r = 0xFF;		\
		g = ((g * omd_PixInterToInt(dg))>>8) + omd_PixInterToInt(sg);	if (g>0xFF) g = 0xFF;		\
		b = ((b * omd_PixInterToInt(db))>>8) + omd_PixInterToInt(sb);	if (b>0xFF) b = 0xFF;		\
															\
													 								\
		dest[npix] = omd_ARGBPackPixel16(r,g,b);  	 			  				\
								\
		dr += idr;				\
		dg += idg;				\
		db += idb;				\
		sr += isr;  sg += isg; sb += isb;  	      	\
		cx+=dcx; cy+=dcy;   	
								
								


void OMediaTextSegmentRasterizer::fill_text_generic_16(
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
								omt_BlendFunc		blend_dest)
{
	unsigned short		*dest_end;
	unsigned long		p,dcolor;
	long 				cx,cy,cx2,cy2,dcx,dcy;
	long				du,dv,dinv_w;

		// Generic version

	short src_argb[4];
	short dest_argb[4];
	unsigned short sna,a,r,g,b,dstr,dstg,dstb;
	unsigned long color;


	omd_INITTEXT_INTER;

	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
	{
		// Optimized path
	
		while(dest!=dest_end)
		{
			omd_TEXT_INTERNEXT8;
		
			omd_GEN16T32PIX_OPT(0);
			omd_GEN16T32PIX_OPT(1);
			omd_GEN16T32PIX_OPT(2);
			omd_GEN16T32PIX_OPT(3);
			omd_GEN16T32PIX_OPT(4);
			omd_GEN16T32PIX_OPT(5);
			omd_GEN16T32PIX_OPT(6);
			omd_GEN16T32PIX_OPT(7);
			
			dest +=8;
		}
	
		count = count&7;
		if (count)
		{
			omd_TEXT_INTERREMAIN;
			
			switch(count)
			{											 
				case 7:	omd_GEN16T32PIX_OPT(0);		dest++;
				case 6:	omd_GEN16T32PIX_OPT(0);		dest++;
				case 5:	omd_GEN16T32PIX_OPT(0);		dest++;
				case 4:	omd_GEN16T32PIX_OPT(0);		dest++;
				case 3:	omd_GEN16T32PIX_OPT(0);		dest++;
				case 2:	omd_GEN16T32PIX_OPT(0);		dest++;
				case 1:	omd_GEN16T32PIX_OPT(0);
			}
		}
		
		return;		
	}

	if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		// Optimized path for blending

		while(dest!=dest_end)
		{
			omd_TEXT_INTERNEXT8;
		
			omd_GEN16T32PIX_OPT_BLEND(0);
			omd_GEN16T32PIX_OPT_BLEND(1);
			omd_GEN16T32PIX_OPT_BLEND(2);
			omd_GEN16T32PIX_OPT_BLEND(3);
			omd_GEN16T32PIX_OPT_BLEND(4);
			omd_GEN16T32PIX_OPT_BLEND(5);
			omd_GEN16T32PIX_OPT_BLEND(6);
			omd_GEN16T32PIX_OPT_BLEND(7);
			
			dest +=8;
		}
	
		count = count&7;
		if (count)
		{
			omd_TEXT_INTERREMAIN;
			
			switch(count)
			{											 
				case 7:	omd_GEN16T32PIX_OPT_BLEND(0);		dest++;
				case 6:	omd_GEN16T32PIX_OPT_BLEND(0);		dest++;
				case 5:	omd_GEN16T32PIX_OPT_BLEND(0);		dest++;
				case 4:	omd_GEN16T32PIX_OPT_BLEND(0);		dest++;
				case 3:	omd_GEN16T32PIX_OPT_BLEND(0);		dest++;
				case 2:	omd_GEN16T32PIX_OPT_BLEND(0);		dest++;
				case 1:	omd_GEN16T32PIX_OPT_BLEND(0);
			}
		}
		
		return;	
	}

	omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
	omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);

	// Generic path

	while(dest!=dest_end)
	{
		omd_TEXT_INTERNEXT8;
	
		omd_GEN16T32PIX(0);
		omd_GEN16T32PIX(1);
		omd_GEN16T32PIX(2);
		omd_GEN16T32PIX(3);
		omd_GEN16T32PIX(4);
		omd_GEN16T32PIX(5);
		omd_GEN16T32PIX(6);
		omd_GEN16T32PIX(7);
		
		dest +=8;
	}

	count = count&7;
	if (count)
	{
		omd_TEXT_INTERREMAIN;
		
		switch(count)
		{											 
			case 7:	omd_GEN16T32PIX(0);	dest++;
			case 6:	omd_GEN16T32PIX(0);	dest++;
			case 5:	omd_GEN16T32PIX(0);	dest++;
			case 4:	omd_GEN16T32PIX(0);	dest++;
			case 3:	omd_GEN16T32PIX(0);	dest++;
			case 2:	omd_GEN16T32PIX(0);	dest++;
			case 1:	omd_GEN16T32PIX(0);		
		}
	}
}

//------------------------------------------------------------------
// Flat 16 bits macro 	

#define omd_FLAT16T32PIX(npix)													\
																				\
																				\
		p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
										  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																												\
		omd_ARGBUnPackPixelA32(p,a,r,g,b);							\
																	\
																\
		a = ((a * (da))>>8);									\
		r = ((r * (dr))>>8) + (sr);	if (r>0xFF) r = 0xFF;		\
		g = ((g * (dg))>>8) + (sg);	if (g>0xFF) g = 0xFF;		\
		b = ((b * (db))>>8) + (sb);	if (b>0xFF) b = 0xFF;		\
																\
																\
		color = omd_ARGBPackPixelA32(a,r,g,b);					\
		dcolor = dest[npix];	dcolor = omd_ARGB16to32(dcolor);	\
																	\
		bsrc_func(&color, &color, &dcolor, src_argb);				\
		bdest_func(&dcolor, &color, &dcolor, dest_argb);			\
								\
    	omd_DOBLEND(a,0);		\
    	omd_DOBLEND(r,1);		\
    	omd_DOBLEND(g,2);		\
    	omd_DOBLEND(b,3);		\
 								\
		dest[npix] = omd_ARGBPackPixelA16(a,r,g,b);     		\
								\
		cx+=dcx; cy+=dcy;   	
								
// Flat 16 bits macro, optimized path, blend 	

#define omd_FLAT16T32PIX_OPT_BLEND(npix)										\
																				\
																			\
		p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
										  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																												\
		omd_ARGBUnPackPixelA32(p,a,r,g,b);							\
																	\
		if (a)														\
		{														\
															\
			a = ((a * (da) )>>8);									\
			r = ((r * (dr))>>8) + (sr);	if (r>0xFF) r = 0xFF;		\
			g = ((g * (dg))>>8) + (sg);	if (g>0xFF) g = 0xFF;		\
			b = ((b * (db))>>8) + (sb);	if (b>0xFF) b = 0xFF;		\
																\
			dcolor = dest[npix];								\
			omd_ARGBUnPackPixel16(dcolor,dstr,dstg,dstb);		\
																\
			sna = 0xFF-a;										\
																\
			r = (r * a + sna * dstr) >> 8;		\
			g = (g * a + sna * dstg) >> 8;		\
			b = (b * a + sna * dstb) >> 8;		\
																					\
												 									\
			dest[npix] = omd_ARGBPackPixel16(r,g,b);  	 			  				\
		}																			\
		cx+=dcx; cy+=dcy;   	

// Flat 16 bits macro, optimized path, no blend 	

#define omd_FLAT16T32PIX_OPT(npix)										\
																				\
																				\
		p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
										  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																												\
		omd_ARGBUnPackPixel32(p,r,g,b);							\
															\
		r = ((r * (dr))>>8) + (sr);	if (r>0xFF) r = 0xFF;		\
		g = ((g * (dg))>>8) + (sg);	if (g>0xFF) g = 0xFF;		\
		b = ((b * (db))>>8) + (sb);	if (b>0xFF) b = 0xFF;		\
															\
													 								\
		dest[npix] = omd_ARGBPackPixel16(r,g,b);  	 			  				\
								\
		cx+=dcx; cy+=dcy;   	
								
								

void OMediaTextSegmentRasterizer::fill_text_flat_16(
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
								omt_BlendFunc		blend_dest)
{
	unsigned short		*dest_end;
	unsigned long		p,dcolor;
	long 				cx,cy,cx2,cy2,dcx,dcy;
	long				du,dv,dinv_w;

		// Generic version

	short src_argb[4];
	short dest_argb[4];
	unsigned short sna,a,r,g,b,dstr,dstg,dstb;
	unsigned long color;


	omd_INITTEXT_INTER;

	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
	{
		// Optimized path
	
		while(dest!=dest_end)
		{
			omd_TEXT_INTERNEXT8;
		
			omd_FLAT16T32PIX_OPT(0);
			omd_FLAT16T32PIX_OPT(1);
			omd_FLAT16T32PIX_OPT(2);
			omd_FLAT16T32PIX_OPT(3);
			omd_FLAT16T32PIX_OPT(4);
			omd_FLAT16T32PIX_OPT(5);
			omd_FLAT16T32PIX_OPT(6);
			omd_FLAT16T32PIX_OPT(7);
			
			dest +=8;
		}
	
		count = count&7;
		if (count)
		{
			omd_TEXT_INTERREMAIN;
			
			switch(count)
			{											 
				case 7:	omd_FLAT16T32PIX_OPT(0);		dest++;
				case 6:	omd_FLAT16T32PIX_OPT(0);		dest++;
				case 5:	omd_FLAT16T32PIX_OPT(0);		dest++;
				case 4:	omd_FLAT16T32PIX_OPT(0);		dest++;
				case 3:	omd_FLAT16T32PIX_OPT(0);		dest++;
				case 2:	omd_FLAT16T32PIX_OPT(0);		dest++;
				case 1:	omd_FLAT16T32PIX_OPT(0);
			}
		}
		
		return;		
	}

	if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		// Optimized path for blending

		while(dest!=dest_end)
		{
			omd_TEXT_INTERNEXT8;
		
			omd_FLAT16T32PIX_OPT_BLEND(0);
			omd_FLAT16T32PIX_OPT_BLEND(1);
			omd_FLAT16T32PIX_OPT_BLEND(2);
			omd_FLAT16T32PIX_OPT_BLEND(3);
			omd_FLAT16T32PIX_OPT_BLEND(4);
			omd_FLAT16T32PIX_OPT_BLEND(5);
			omd_FLAT16T32PIX_OPT_BLEND(6);
			omd_FLAT16T32PIX_OPT_BLEND(7);
			
			dest +=8;
		}
	
		count = count&7;
		if (count)
		{
			omd_TEXT_INTERREMAIN;
			
			switch(count)
			{											 
				case 7:	omd_FLAT16T32PIX_OPT_BLEND(0);	dest++;
				case 6:	omd_FLAT16T32PIX_OPT_BLEND(0);	dest++;
				case 5:	omd_FLAT16T32PIX_OPT_BLEND(0);	dest++;
				case 4:	omd_FLAT16T32PIX_OPT_BLEND(0);	dest++;
				case 3:	omd_FLAT16T32PIX_OPT_BLEND(0);	dest++;
				case 2:	omd_FLAT16T32PIX_OPT_BLEND(0);	dest++;
				case 1:	omd_FLAT16T32PIX_OPT_BLEND(0);
			}
		}
		
		return;	
	}

	omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
	omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);

	// Generic path

	while(dest!=dest_end)
	{
		omd_TEXT_INTERNEXT8;
	
		omd_FLAT16T32PIX(0);
		omd_FLAT16T32PIX(1);
		omd_FLAT16T32PIX(2);
		omd_FLAT16T32PIX(3);
		omd_FLAT16T32PIX(4);
		omd_FLAT16T32PIX(5);
		omd_FLAT16T32PIX(6);
		omd_FLAT16T32PIX(7);
		
		dest +=8;
	}

	count = count&7;
	if (count)
	{
		omd_TEXT_INTERREMAIN;
		
		switch(count)
		{											 
			case 7:	omd_FLAT16T32PIX(0);		dest++;
			case 6:	omd_FLAT16T32PIX(0);		dest++;
			case 5:	omd_FLAT16T32PIX(0);		dest++;
			case 4:	omd_FLAT16T32PIX(0);		dest++;
			case 3:	omd_FLAT16T32PIX(0);		dest++;
			case 2:	omd_FLAT16T32PIX(0);		dest++;
			case 1:	omd_FLAT16T32PIX(0);		
		}
	}
}



//------------------------------------------------------------------
// Flat  16 bits macro, no modulation 	

#define omd_NOMODFLAT16T32PIX(npix)													\
																				\
		color = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
										  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																												\
		dcolor = dest[npix];	dcolor = omd_ARGB16to32(dcolor);	\
																	\
		bsrc_func(&color, &color, &dcolor, src_argb);				\
		bdest_func(&dcolor, &color, &dcolor, dest_argb);			\
		bsrc_func(&color, &color, ((omt_BlendPixel*)&dest[npix]), src_argb);								\
		bdest_func(((omt_BlendPixel*)&dest[npix]), &color, ((omt_BlendPixel*)&dest[npix]), dest_argb);			\
								\
    	omd_DOBLEND(a,0);		\
    	omd_DOBLEND(r,1);		\
    	omd_DOBLEND(g,2);		\
    	omd_DOBLEND(b,3);		\
 								\
		dest[npix] = omd_ARGBPackPixelA16(a,r,g,b);     		\
								\
		cx+=dcx; cy+=dcy;   	
								
// Flat 16 bits macro, optimized path, blend, no modulation  	

#define omd_NOMODFLAT16T32PIX_OPT_BLEND(npix)										\
																				\
																				\
		p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
										  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																												\
		omd_ARGBUnPackPixelA32(p,a,r,g,b);							\
																	\
		if (a)												\
		{													\
			dcolor = dest[npix];								\
			omd_ARGBUnPackPixel16(dcolor,dstr,dstg,dstb);		\
															\
															\
			sna = 0xFF-a;									\
			r = (r * a + sna * dstr) >> 8;		\
			g = (g * a + sna * dstg) >> 8;		\
			b = (b * a + sna * dstb) >> 8;		\
																				\
												 								\
			dest[npix] = omd_ARGBPackPixel16(r,g,b);  	 			  				\
		}						\
																		\
		cx+=dcx; cy+=dcy;   	

// Flat 16 bits macro, optimized path, no blend, no modulation   	

#define omd_NOMODFLAT16T32PIX_OPT(npix)										\
																				\
																				\
		p = *((unsigned long*)(texture + ((((cx>>omd_LINEAR_FP)&text_maskx)<<2) |				\
											  (((cy>>omd_LINEAR_FP)&text_masky)<<text_rowbytes_shifter))));			\
																													\
		dest[npix] = omd_ARGB32to16(p);						\
		cx+=dcx; cy+=dcy;   	
								
void OMediaTextSegmentRasterizer::fill_text_flat_nomod_16(unsigned short *dest,				// Buffer
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
								omt_BlendFunc		blend_dest)
{
	unsigned long		p,dcolor;
	unsigned short		*dest_end;
	long 				cx,cy,cx2,cy2,dcx,dcy;
	long				du,dv,dinv_w;

		// Generic version

	short src_argb[4];
	short dest_argb[4];
	unsigned short sna,a,r,g,b,dstr,dstg,dstb;
	unsigned long color;


	omd_INITTEXT_INTER;

	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
	{
		// Optimized path
	
		while(dest!=dest_end)
		{
			omd_TEXT_INTERNEXT8;
		
			omd_NOMODFLAT16T32PIX_OPT(0);
			omd_NOMODFLAT16T32PIX_OPT(1);
			omd_NOMODFLAT16T32PIX_OPT(2);
			omd_NOMODFLAT16T32PIX_OPT(3);
			omd_NOMODFLAT16T32PIX_OPT(4);
			omd_NOMODFLAT16T32PIX_OPT(5);
			omd_NOMODFLAT16T32PIX_OPT(6);
			omd_NOMODFLAT16T32PIX_OPT(7);
			
			dest +=8;
		}
	
		count = count&7;
		if (count)
		{
			omd_TEXT_INTERREMAIN;
			
			switch(count)
			{											 
				case 7:	omd_NOMODFLAT16T32PIX_OPT(0);	dest++;
				case 6:	omd_NOMODFLAT16T32PIX_OPT(0);	dest++;
				case 5:	omd_NOMODFLAT16T32PIX_OPT(0);	dest++;
				case 4:	omd_NOMODFLAT16T32PIX_OPT(0);	dest++;
				case 3:	omd_NOMODFLAT16T32PIX_OPT(0);	dest++;
				case 2:	omd_NOMODFLAT16T32PIX_OPT(0);	dest++;
				case 1:	omd_NOMODFLAT16T32PIX_OPT(0);
			}
		}
		
		return;		
	}

	if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		// Optimized path for blending

		while(dest!=dest_end)
		{
			omd_TEXT_INTERNEXT8;
		
			omd_NOMODFLAT16T32PIX_OPT_BLEND(0);
			omd_NOMODFLAT16T32PIX_OPT_BLEND(1);
			omd_NOMODFLAT16T32PIX_OPT_BLEND(2);
			omd_NOMODFLAT16T32PIX_OPT_BLEND(3);
			omd_NOMODFLAT16T32PIX_OPT_BLEND(4);
			omd_NOMODFLAT16T32PIX_OPT_BLEND(5);
			omd_NOMODFLAT16T32PIX_OPT_BLEND(6);
			omd_NOMODFLAT16T32PIX_OPT_BLEND(7);
			
			dest +=8;
		}
	
		count = count&7;
		if (count)
		{
			omd_TEXT_INTERREMAIN;
			
			switch(count)
			{											 
				case 7:	omd_NOMODFLAT16T32PIX_OPT_BLEND(0);		dest++;
				case 6:	omd_NOMODFLAT16T32PIX_OPT_BLEND(0);		dest++;
				case 5:	omd_NOMODFLAT16T32PIX_OPT_BLEND(0);		dest++;
				case 4:	omd_NOMODFLAT16T32PIX_OPT_BLEND(0);		dest++;
				case 3:	omd_NOMODFLAT16T32PIX_OPT_BLEND(0);		dest++;
				case 2:	omd_NOMODFLAT16T32PIX_OPT_BLEND(0);		dest++;
				case 1:	omd_NOMODFLAT16T32PIX_OPT_BLEND(0);
			}
		}
		
		return;	
	}

	omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
	omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);

	// Generic path

	while(dest!=dest_end)
	{
		omd_TEXT_INTERNEXT8;
	
		omd_NOMODFLAT16T32PIX(0);
		omd_NOMODFLAT16T32PIX(1);
		omd_NOMODFLAT16T32PIX(2);
		omd_NOMODFLAT16T32PIX(3);
		omd_NOMODFLAT16T32PIX(4);
		omd_NOMODFLAT16T32PIX(5);
		omd_NOMODFLAT16T32PIX(6);
		omd_NOMODFLAT16T32PIX(7);
		
		dest +=8;
	}

	count = count&7;
	if (count)
	{
		omd_TEXT_INTERREMAIN;
		
		switch(count)
		{											 
			case 7:	omd_NOMODFLAT16T32PIX(0);	dest++;
			case 6:	omd_NOMODFLAT16T32PIX(0);	dest++;
			case 5:	omd_NOMODFLAT16T32PIX(0);	dest++;
			case 4:	omd_NOMODFLAT16T32PIX(0);	dest++;
			case 3:	omd_NOMODFLAT16T32PIX(0);	dest++;
			case 2:	omd_NOMODFLAT16T32PIX(0);	dest++;
			case 1:	omd_NOMODFLAT16T32PIX(0);		
		}
	}
}



#endif

