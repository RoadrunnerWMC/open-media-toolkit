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

#include "OMediaSegmentRasterizer.h"
#include "OMediaPixelFormat.h"



void OMediaSegmentRasterizer::fill_color_zbuffer32(unsigned long		*dest,
										unsigned long		color,
										unsigned short		*zbuffer,	
										unsigned long		zl,
										long		   		zi,					
										unsigned short		count,
										omt_ZBufferFunc		zfunc,
										omt_ZBufferTest		ztest,
										omt_ZBufferWrite	zwrite)
{
	unsigned short		*end,z;

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled && zwrite==omzbwc_Enabled)
	{
		end = zbuffer + (count&(~7));
	
		// Optimized path	
	     	
       	while(zbuffer!=end)
       	{ 
    		z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[1]) {zbuffer[1] = z; dest[1] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[2]) {zbuffer[2] = z; dest[2] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[3]) {zbuffer[3] = z; dest[3] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[4]) {zbuffer[4] = z; dest[4] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[5]) {zbuffer[5] = z; dest[5] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[6]) {zbuffer[6] = z; dest[6] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[7]) {zbuffer[7] = z; dest[7] = color;} zl += zi;
			
			dest+=8;
			zbuffer+=8;
        } 

		switch(count&7)
		{
			case 7: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++;
			case 6: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++;
			case 5: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++; 
			case 4: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++;
			case 3: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++; 
			case 2: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++;
			case 1: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++; 
		}	
	}
	else
	{
		end = zbuffer + count;
	
		// Generic version
	
		while(zbuffer!=end)
		{
			z = zl>>16L; 
			
			if (do_ztest(z, zbuffer[0], zfunc,ztest))
			{
				if (zwrite==omzbwc_Enabled) zbuffer[0] = z; 
				dest[0] = color;
			}
		
			zbuffer++;
			dest++;
			zl +=zi;
		}
	}
}


void OMediaSegmentRasterizer::fill_color_interpolation32(unsigned char *dest,
								omt_PixInterValue r,
								omt_PixInterValue g,
								omt_PixInterValue b,		
								omt_PixInterValue ir,
								omt_PixInterValue ig,
								omt_PixInterValue ib,
							    unsigned short count)
{
	unsigned short count4;
	unsigned long p1,p2,p3,p4;
  	
	count4 = count>>2;
	count &= 3;
	
 	while(count4--)
	{
		p1 = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
		r += ir;
		g += ig;
		b += ib;

		p2 = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
		r += ir;
		g += ig;
		b += ib;

		p3 = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
		r += ir;
		g += ig;
		b += ib;

		p4 = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
		r += ir;
		g += ig;
		b += ib;

    	((unsigned long*)dest)[0] = p1;
    	((unsigned long*)dest)[1] = p2;
    	((unsigned long*)dest)[2] = p3,
    	((unsigned long*)dest)[3] = p4;
    		 
		dest+=16;
	}    	
     	
	while(count--)
	{
		((unsigned long*)dest)[0] = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
		r += ir;
		g += ig;
		b += ib;    	      		
		dest+=4;
	} 
}

void OMediaSegmentRasterizer::fill_color_interpolation_zb32(unsigned char *dest,
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
							    unsigned short 		count)
{	
	unsigned short		*end,z;

	// Optimized path

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled && zwrite==omzbwc_Enabled)
	{
		unsigned short count4;
	  	
		count4 = count>>2;
		count &= 3;
		
	 	while(count4--)
		{
			z = zl>>16L;
		
			if (z<zbuffer[0]) 
			{
				zbuffer[0] = z; 		
				((unsigned long*)dest)[0] = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
			}
	
			r += ir;
			g += ig;
			b += ib;
			zl +=zi;
		
			z = zl>>16L;	
			
			if (z<zbuffer[1]) 
			{
				zbuffer[1] = z; 		
				((unsigned long*)dest)[1] = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
			}
	
			r += ir;
			g += ig;
			b += ib;
			zl +=zi;
	
			z = zl>>16L;
			
			if (z<zbuffer[2]) 
			{
				zbuffer[2] = z; 		
				((unsigned long*)dest)[2] = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
			}

			r += ir;
			g += ig;
			b += ib;
			zl +=zi;	

			z = zl>>16L;

			if (z<zbuffer[3]) 
			{
				zbuffer[3] = z; 		
				((unsigned long*)dest)[3] = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
			}

			r += ir;
			g += ig;
			b += ib;
			zl +=zi;	    		 
			dest+=16;
			zbuffer+=4;
		}    	
	     	
		while(count--)
		{
			z = zl>>16L;

			if (z<zbuffer[0]) 
			{
				zbuffer[0] = z; 		
				((unsigned long*)dest)[0] = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
			}
	
			r += ir;
			g += ig;
			b += ib;  	      		
			zbuffer++;
			zl +=zi;
			dest+=4;
		} 
	}
	else
	{
		end = zbuffer + count;
	
		// Generic version
	
		while(zbuffer!=end)
		{
			z = zl>>16L; 
			
			if (do_ztest(z, zbuffer[0], zfunc,ztest))
			{
				if (zwrite==omzbwc_Enabled) zbuffer[0] = z; 
				
				((unsigned long*)dest)[0] = omd_ARGBPackPixel32(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
    		}
		
			r += ir;
			g += ig;
			b += ib;    	
 			zbuffer++;
			dest+=4;
			zl +=zi;
		}
	}
}

#define omd_DOBLEND(res,gun) 	{res = (src_argb[gun] + dest_argb[gun]);		\
								if (res<0) res = 0; else if (res>0xFF) res = 0xFF;}

void OMediaSegmentRasterizer::fill_color_blend32(unsigned char		*dest,
												unsigned long		color,				
												unsigned short		count,
												omt_BlendFunc		blend_src, 
												omt_BlendFunc		blend_dest)
{
	if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		// Optimized path

		unsigned short sa,sr,sg,sb,sna;
		unsigned short dr,dg,db;
		unsigned long dcolor;
		unsigned char r,g,b;
		
		omd_ARGBUnPackPixelA32(color,sa,sr,sg,sb);
		sna = 0xFF-sa;

		sr *= sa;
		sg *= sa;
		sb *= sa;

		while(count--)
		{
			dcolor = *((unsigned long*)dest);
			omd_ARGBUnPackPixel32(dcolor,dr,dg,db);
		
			r = (sr + sna * dr) >> 8;			
			g = (sg + sna * dg) >> 8;			
			b = (sb + sna * db) >> 8;			

			dcolor = omd_ARGBPackPixel32(r,g,b);
		
			*((unsigned long*)dest) = dcolor;
			dest+=4;
		}	
	}
	else
	{
		// Generic version
		
		short src_argb[4];
		short dest_argb[4],a,r,g,b;
	
		omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
		omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);
		
		while(count--)
		{
			bsrc_func(&color, &color, ((omt_BlendPixel*)dest), src_argb);
			bdest_func(((omt_BlendPixel*)dest), &color, ((omt_BlendPixel*)dest), dest_argb);
		
	    	omd_DOBLEND(a,0);
	    	omd_DOBLEND(r,1);
	    	omd_DOBLEND(g,2);
	    	omd_DOBLEND(b,3);
	 
			((unsigned long*)dest)[0] = omd_ARGBPackPixelA32(a,r,g,b);     	
			dest+=4;
		} 
	}
}


void OMediaSegmentRasterizer::fill_color_blend_zbuffer32(unsigned long		*dest,
									unsigned long		color,
									unsigned short		*zbuffer,	
									unsigned long		zl,
									long		   		zi,					
									unsigned short		count,
									omt_ZBufferFunc		zfunc,
									omt_ZBufferTest		ztest,
									omt_ZBufferWrite	zwrite,
									omt_BlendFunc		blend_src, 
									omt_BlendFunc		blend_dest)
{
	unsigned short		*end,z;

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled && 
		blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		bool dowritez = (zwrite==omzbwc_Enabled);
		
		// Optimized path

		unsigned short sa,sr,sg,sb,sna;
		unsigned short dr,dg,db;
		unsigned long dcolor;
		unsigned char r,g,b;
		
		omd_ARGBUnPackPixelA32(color,sa,sr,sg,sb);
		sna = 0xFF-sa;

		sr *= sa;
		sg *= sa;
		sb *= sa;

		end = zbuffer + count;

		while(zbuffer!=end)
		{
			z = zl>>16L;
		
			if (z<zbuffer[0]) 
			{
				if (dowritez) zbuffer[0] = z;
				 		
				dcolor = *((unsigned long*)dest);
				omd_ARGBUnPackPixel32(dcolor,dr,dg,db);
			
				r = (sr + sna * dr) >> 8;			
				g = (sg + sna * dg) >> 8;			
				b = (sb + sna * db) >> 8;			
	
				dcolor = omd_ARGBPackPixel32(r,g,b);
			
				*((unsigned long*)dest) = dcolor;
			}		
			
			dest++;
			zl +=zi;						
			zbuffer++;	
		}	
	}
	else 
	{

		// Generic version
		end = zbuffer + count;

		short src_argb[4];
		short dest_argb[4],a,r,g,b;
	
		omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
		omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);
	
	
		while(zbuffer!=end)
		{
			z = zl>>16L; 
			
			if (do_ztest(z, zbuffer[0], zfunc,ztest))
			{
				if (zwrite==omzbwc_Enabled) zbuffer[0] = z; 

				bsrc_func(&color, &color, ((omt_BlendPixel*)dest), src_argb);
				bdest_func(((omt_BlendPixel*)dest), &color, ((omt_BlendPixel*)dest), dest_argb);
			
		    	omd_DOBLEND(a,0);
		    	omd_DOBLEND(r,1);
		    	omd_DOBLEND(g,2);
		    	omd_DOBLEND(b,3);
		 
				((unsigned long*)dest)[0] = omd_ARGBPackPixelA32(a,r,g,b);     	
			}
		
			zbuffer++;
			dest++;
			zl +=zi;
		}
	}
}


void OMediaSegmentRasterizer::fill_color_blend_interpolation32(unsigned char *dest,		
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
								omt_BlendFunc		blend_dest)
{
	if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		// Optimized path

		unsigned short sa,sna;
		unsigned short dr,dg,db;
		unsigned long dcolor;
		unsigned char r,g,b;
		
		while(count--)
		{
			dcolor = *((unsigned long*)dest);
			omd_ARGBUnPackPixel32(dcolor,dr,dg,db);
		
			sa = omd_PixInterToInt(ga);
			sna = 0xFF-sa;
			
			r = (((unsigned short)omd_PixInterToInt(gr)) * sa + sna * dr) >> 8;			
			g = (((unsigned short)omd_PixInterToInt(gg)) * sa + sna * dg) >> 8;			
			b = (((unsigned short)omd_PixInterToInt(gb)) * sa + sna * db) >> 8;

			dcolor = omd_ARGBPackPixel32(r,g,b);
		
			*((unsigned long*)dest) = dcolor;
			dest+=4;
			ga += ia;
			gr += ir;
			gg += ig;
			gb += ib;    	      		
		}	
	}
	else
	{
		// Generic version
		
		short src_argb[4];
		short dest_argb[4],a,r,g,b;
		unsigned long color;
	
		omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
		omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);
		
		while(count--)
		{
			color = omd_ARGBPackPixelA32(omd_PixInterToInt(ga),omd_PixInterToInt(gr),omd_PixInterToInt(gg),omd_PixInterToInt(gb));
		
			bsrc_func(&color, &color, ((omt_BlendPixel*)dest), src_argb);
			bdest_func(((omt_BlendPixel*)dest), &color, ((omt_BlendPixel*)dest), dest_argb);
		
	    	omd_DOBLEND(a,0);
	    	omd_DOBLEND(r,1);
	    	omd_DOBLEND(g,2);
	    	omd_DOBLEND(b,3);
	 
			((unsigned long*)dest)[0] = omd_ARGBPackPixelA32(a,r,g,b);     	
			dest+=4;
			ga += ia;
			gr += ir;
			gg += ig;
			gb += ib;    	      		
		} 
	}
}

void OMediaSegmentRasterizer::fill_color_blend_interpolation_zb32(unsigned char *dest,
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
								omt_BlendFunc		blend_dest)
{
	unsigned short		*end,z;

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled && 
		blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		bool dowritez = (zwrite==omzbwc_Enabled);
		
		// Optimized path

		unsigned short sa,sna;
		unsigned short dr,dg,db;
		unsigned long dcolor;
		unsigned char r,g,b;

		end = zbuffer + count;

		while(zbuffer!=end)
		{
			z = zl>>16L;
		
			if (z<zbuffer[0]) 
			{
				if (dowritez) zbuffer[0] = z;
				 		
				dcolor = *((unsigned long*)dest);
				omd_ARGBUnPackPixel32(dcolor,dr,dg,db);
			
				sa = omd_PixInterToInt(ga);
				sna = 0xFF-sa;
				
				r = (((unsigned short)omd_PixInterToInt(gr)) * sa + sna * dr) >> 8;			
				g = (((unsigned short)omd_PixInterToInt(gg)) * sa + sna * dg) >> 8;			
				b = (((unsigned short)omd_PixInterToInt(gb)) * sa + sna * db) >> 8;
	
				dcolor = omd_ARGBPackPixel32(r,g,b);
			
				*((unsigned long*)dest) = dcolor;

			}		
			
			dest+=4;
			zl +=zi;						
			zbuffer++;	
			ga += ia;
			gr += ir;
			gg += ig;
			gb += ib;    	      		
		}	
	}
	else 
	{

		// Generic version
		end = zbuffer + count;

		short src_argb[4];
		short dest_argb[4],a,r,g,b;
		unsigned long color;
	
		omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
		omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);
	
	
		while(zbuffer!=end)
		{
			z = zl>>16L; 
			
			if (do_ztest(z, zbuffer[0], zfunc,ztest))
			{
				if (zwrite==omzbwc_Enabled) zbuffer[0] = z; 

				color = omd_ARGBPackPixelA32(omd_PixInterToInt(ga),omd_PixInterToInt(gr),omd_PixInterToInt(gg),omd_PixInterToInt(gb));

				bsrc_func(&color, &color, ((omt_BlendPixel*)dest), src_argb);
				bdest_func(((omt_BlendPixel*)dest), &color, ((omt_BlendPixel*)dest), dest_argb);
			
		    	omd_DOBLEND(a,0);
		    	omd_DOBLEND(r,1);
		    	omd_DOBLEND(g,2);
		    	omd_DOBLEND(b,3);
		 
				((unsigned long*)dest)[0] = omd_ARGBPackPixelA32(a,r,g,b);     	
			}
		
			zbuffer++;
			dest+=4;
			zl +=zi;
			ga += ia;
			gr += ir;
			gg += ig;
			gb += ib;    	      		
		}
	}
}

// 16 bits

void OMediaSegmentRasterizer::fill_color_zbuffer16(unsigned short		*dest,
										unsigned long		color32,
										unsigned short		*zbuffer,	
										unsigned long		zl,
										long		   		zi,					
										unsigned short		count,
										omt_ZBufferFunc		zfunc,
										omt_ZBufferTest		ztest,
										omt_ZBufferWrite	zwrite)
{
	unsigned short		*end,z;
	unsigned short		color = omd_ARGB32to16(color32);

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled && zwrite==omzbwc_Enabled)
	{
		end = zbuffer + (count&(~7));
	
		// Optimized path	
	     	
       	while(zbuffer!=end)
       	{ 
    		z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[1]) {zbuffer[1] = z; dest[1] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[2]) {zbuffer[2] = z; dest[2] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[3]) {zbuffer[3] = z; dest[3] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[4]) {zbuffer[4] = z; dest[4] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[5]) {zbuffer[5] = z; dest[5] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[6]) {zbuffer[6] = z; dest[6] = color;} zl += zi;
    		z = zl>>16L; if (z<zbuffer[7]) {zbuffer[7] = z; dest[7] = color;} zl += zi;
			
			dest+=8;
			zbuffer+=8;
        } 

		switch(count&7)
		{
			case 7: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++;
			case 6: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++;
			case 5: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++; 
			case 4: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++;
			case 3: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++; 
			case 2: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++;
			case 1: z = zl>>16L; if (z<zbuffer[0]) {zbuffer[0] = z; dest[0] = color;} zl += zi;dest++;zbuffer++; 
		}	
	}
	else
	{
		end = zbuffer + count;
	
		// Generic version
	
		while(zbuffer!=end)
		{
			z = zl>>16L; 
			
			if (do_ztest(z, zbuffer[0], zfunc,ztest))
			{
				if (zwrite==omzbwc_Enabled) zbuffer[0] = z; 
				dest[0] = color;
			}
		
			zbuffer++;
			dest++;
			zl +=zi;
		}
	}
}


void OMediaSegmentRasterizer::fill_color_interpolation16(unsigned char *dest,
								omt_PixInterValue r,
								omt_PixInterValue g,
								omt_PixInterValue b,		
								omt_PixInterValue ir,
								omt_PixInterValue ig,
								omt_PixInterValue ib,
							    unsigned short count)
{
	unsigned short count4;
	unsigned short p1,p2,p3,p4;
  	
	count4 = count>>2;
	count &= 3;
	
 	while(count4--)
	{
		p1 = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
		r += ir;
		g += ig;
		b += ib;

		p2 = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
		r += ir;
		g += ig;
		b += ib;

		p3 = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
		r += ir;
		g += ig;
		b += ib;

		p4 = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
		r += ir;
		g += ig;
		b += ib;

    	((unsigned short*)dest)[0] = p1;
    	((unsigned short*)dest)[1] = p2;
    	((unsigned short*)dest)[2] = p3,
    	((unsigned short*)dest)[3] = p4;
    		 
		dest+=8;
	}    	
     	
	while(count--)
	{
		((unsigned short*)dest)[0] = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
		r += ir;
		g += ig;
		b += ib;    	      		
		dest+=2;
	} 
}

void OMediaSegmentRasterizer::fill_color_interpolation_zb16(unsigned char *dest,
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
							    unsigned short 		count)
{	
	unsigned short		*end,z;

	// Optimized path

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled && zwrite==omzbwc_Enabled)
	{
		unsigned short count4;
	  	
		count4 = count>>2;
		count &= 3;
		
	 	while(count4--)
		{
			z = zl>>16L;
		
			if (z<zbuffer[0]) 
			{
				zbuffer[0] = z; 		
				((unsigned short*)dest)[0] = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
			}
	
			r += ir;
			g += ig;
			b += ib;
			zl +=zi;
		
			z = zl>>16L;	
			
			if (z<zbuffer[1]) 
			{
				zbuffer[1] = z; 		
				((unsigned short*)dest)[1] = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
			}
	
			r += ir;
			g += ig;
			b += ib;
			zl +=zi;
	
			z = zl>>16L;
			
			if (z<zbuffer[2]) 
			{
				zbuffer[2] = z; 		
				((unsigned short*)dest)[2] = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
			}

			r += ir;
			g += ig;
			b += ib;
			zl +=zi;	

			z = zl>>16L;

			if (z<zbuffer[3]) 
			{
				zbuffer[3] = z; 		
				((unsigned short*)dest)[3] = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
			}

			r += ir;
			g += ig;
			b += ib;
			zl +=zi;	    		 
			dest+=8;
			zbuffer+=4;
		}    	
	     	
		while(count--)
		{
			z = zl>>16L;

			if (z<zbuffer[0]) 
			{
				zbuffer[0] = z; 		
				((unsigned short*)dest)[0] = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
			}
	
			r += ir;
			g += ig;
			b += ib;  	      		
			zbuffer++;
			zl +=zi;
			dest+=2;
		} 
	}
	else
	{
		end = zbuffer + count;
	
		// Generic version
	
		while(zbuffer!=end)
		{
			z = zl>>16L; 
			
			if (do_ztest(z, zbuffer[0], zfunc,ztest))
			{
				if (zwrite==omzbwc_Enabled) zbuffer[0] = z; 
				
				((unsigned short*)dest)[0] = omd_ARGBPackPixel16(omd_PixInterToInt(r),omd_PixInterToInt(g),omd_PixInterToInt(b));
    		}
		
			r += ir;
			g += ig;
			b += ib;    	
 			zbuffer++;
			dest+=2;
			zl +=zi;
		}
	}
}

#define omd_DOBLEND(res,gun) 	{res = (src_argb[gun] + dest_argb[gun]);		\
								if (res<0) res = 0; else if (res>0xFF) res = 0xFF;}

void OMediaSegmentRasterizer::fill_color_blend16(unsigned char		*dest,
												unsigned long		color,				
												unsigned short		count,
												omt_BlendFunc		blend_src, 
												omt_BlendFunc		blend_dest)
{
	if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		// Optimized path

		unsigned short sa,sr,sg,sb,sna;
		unsigned short dr,dg,db;
		unsigned short dcolor;
		unsigned char r,g,b;
		
		omd_ARGBUnPackPixelA32(color,sa,sr,sg,sb);
		sna = 0xFF-sa;

		sr *= sa;
		sg *= sa;
		sb *= sa;

		while(count--)
		{
			dcolor = *((unsigned short*)dest);
			omd_ARGBUnPackPixel16(dcolor,dr,dg,db);
		
			r = (sr + sna * dr) >> 8;			
			g = (sg + sna * dg) >> 8;			
			b = (sb + sna * db) >> 8;			
		
			*((unsigned short*)dest) = omd_ARGBPackPixel16(r,g,b);
			dest+=2;
		}	
	}
	else
	{
		// Generic version
		
		short src_argb[4];
		short dest_argb[4],a,r,g,b;
		unsigned short dcolor;
		unsigned long dcolor32;
	
		omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
		omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);
				
		while(count--)
		{
			dcolor = *((unsigned short*)dest);
			dcolor32 = omd_ARGB16toA32(dcolor);
		
			bsrc_func(&color, &color, &dcolor32, src_argb);
			bdest_func(&dcolor32, &color, &dcolor32, dest_argb);
		
	    	omd_DOBLEND(a,0);
	    	omd_DOBLEND(r,1);
	    	omd_DOBLEND(g,2);
	    	omd_DOBLEND(b,3);
	 
			((unsigned short*)dest)[0] = omd_ARGBPackPixelA16(a,r,g,b);   
			dest+=2;
		} 
	}
}


void OMediaSegmentRasterizer::fill_color_blend_zbuffer16(unsigned short		*dest,
									unsigned long		color,
									unsigned short		*zbuffer,	
									unsigned long		zl,
									long		   		zi,					
									unsigned short		count,
									omt_ZBufferFunc		zfunc,
									omt_ZBufferTest		ztest,
									omt_ZBufferWrite	zwrite,
									omt_BlendFunc		blend_src, 
									omt_BlendFunc		blend_dest)
{
	unsigned short		*end,z;

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled && 
		blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		bool dowritez = (zwrite==omzbwc_Enabled);
		
		// Optimized path

		unsigned short sa,sr,sg,sb,sna;
		unsigned short dr,dg,db;
		unsigned short dcolor;
		unsigned char r,g,b;
		
		omd_ARGBUnPackPixelA32(color,sa,sr,sg,sb);
		sna = 0xFF-sa;

		sr *= sa;
		sg *= sa;
		sb *= sa;

		end = zbuffer + count;

		while(zbuffer!=end)
		{
			z = zl>>16L;
		
			if (z<zbuffer[0]) 
			{
				if (dowritez) zbuffer[0] = z;
				 		
				dcolor = *((unsigned short*)dest);
				omd_ARGBUnPackPixel16(dcolor,dr,dg,db);
			
				r = (sr + sna * dr) >> 8;			
				g = (sg + sna * dg) >> 8;			
				b = (sb + sna * db) >> 8;			
	
				dcolor = omd_ARGBPackPixel16(r,g,b);
			
				*((unsigned short*)dest) = dcolor;
			}		
			
			dest++;
			zl +=zi;						
			zbuffer++;	
		}	
	}
	else 
	{

		// Generic version
		end = zbuffer + count;

		short src_argb[4];
		short dest_argb[4],a,r,g,b;
		unsigned short dcolor;
		unsigned long dcolor32;
	
		omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
		omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);
	
	
		while(zbuffer!=end)
		{
			z = zl>>16L; 
			
			if (do_ztest(z, zbuffer[0], zfunc,ztest))
			{
				if (zwrite==omzbwc_Enabled) zbuffer[0] = z; 

				dcolor = *((unsigned short*)dest);
				dcolor32 = omd_ARGB16toA32(dcolor);

				bsrc_func(&color, &color, &dcolor32, src_argb);
				bdest_func(&dcolor32, &color, &dcolor32, dest_argb);
			
		    	omd_DOBLEND(a,0);
		    	omd_DOBLEND(r,1);
		    	omd_DOBLEND(g,2);
		    	omd_DOBLEND(b,3);
		 
				((unsigned short*)dest)[0] = omd_ARGBPackPixelA16(a,r,g,b);     	
			}
		
			zbuffer++;
			dest++;
			zl +=zi;
		}
	}
}


void OMediaSegmentRasterizer::fill_color_blend_interpolation16(unsigned char *dest,		
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
								omt_BlendFunc		blend_dest)
{
	if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		// Optimized path

		unsigned short sa,sna;
		unsigned short dr,dg,db;
		unsigned short dcolor;
		unsigned char r,g,b;
		
		while(count--)
		{
			dcolor = *((unsigned short*)dest);
			omd_ARGBUnPackPixel16(dcolor,dr,dg,db);
		
			sa = omd_PixInterToInt(ga);
			sna = 0xFF-sa;
			
			r = (((unsigned short)omd_PixInterToInt(gr)) * sa + sna * dr) >> 8;			
			g = (((unsigned short)omd_PixInterToInt(gg)) * sa + sna * dg) >> 8;			
			b = (((unsigned short)omd_PixInterToInt(gb)) * sa + sna * db) >> 8;

			dcolor = omd_ARGBPackPixel16(r,g,b);
		
			*((unsigned short*)dest) = dcolor;
			dest+=2;
			ga += ia;
			gr += ir;
			gg += ig;
			gb += ib;    	      		
		}	
	}
	else
	{
		// Generic version
		
		short src_argb[4];
		short dest_argb[4],a,r,g,b,dcolor;
		unsigned long color,dcolor32;
	
		omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
		omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);
		
		while(count--)
		{
			color = omd_ARGBPackPixelA32(omd_PixInterToInt(ga),omd_PixInterToInt(gr),omd_PixInterToInt(gg),omd_PixInterToInt(gb));
			dcolor = *((unsigned short*)dest);
			dcolor32 = omd_ARGB16toA32(dcolor);
		
			bsrc_func(&color, &color, &dcolor32, src_argb);
			bdest_func(&dcolor32, &color, &dcolor32, dest_argb);
		
	    	omd_DOBLEND(a,0);
	    	omd_DOBLEND(r,1);
	    	omd_DOBLEND(g,2);
	    	omd_DOBLEND(b,3);
	 
			((unsigned short*)dest)[0] = omd_ARGBPackPixelA16(a,r,g,b);     	
			dest+=2;
			ga += ia;
			gr += ir;
			gg += ig;
			gb += ib;    	      		
		} 
	}
}

void OMediaSegmentRasterizer::fill_color_blend_interpolation_zb16(unsigned char *dest,
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
								omt_BlendFunc		blend_dest)
{
	unsigned short		*end,z;

	if (zfunc==omzbfc_Less && ztest==omzbtc_Enabled && 
		blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)
	{
		bool dowritez = (zwrite==omzbwc_Enabled);
		
		// Optimized path

		unsigned short sa,sna;
		unsigned short dr,dg,db;
		unsigned short dcolor;
		unsigned char r,g,b;

		end = zbuffer + count;

		while(zbuffer!=end)
		{
			z = zl>>16L;
		
			if (z<zbuffer[0]) 
			{
				if (dowritez) zbuffer[0] = z;
				 		
				dcolor = *((unsigned short*)dest);
				omd_ARGBUnPackPixel16(dcolor,dr,dg,db);
			
				sa = omd_PixInterToInt(ga);
				sna = 0xFF-sa;
				
				r = (((unsigned short)omd_PixInterToInt(gr)) * sa + sna * dr) >> 8;			
				g = (((unsigned short)omd_PixInterToInt(gg)) * sa + sna * dg) >> 8;			
				b = (((unsigned short)omd_PixInterToInt(gb)) * sa + sna * db) >> 8;
	
				dcolor = omd_ARGBPackPixel16(r,g,b);
			
				*((unsigned short*)dest) = dcolor;

			}		
			
			dest+=2;
			zl +=zi;						
			zbuffer++;	
			ga += ia;
			gr += ir;
			gg += ig;
			gb += ib;    	      		
		}	
	}
	else 
	{

		// Generic version
		end = zbuffer + count;

		short src_argb[4];
		short dest_argb[4],a,r,g,b,dcolor;
		unsigned long color,dcolor32;
	
		omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
		omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);
		
		while(zbuffer!=end)
		{
			z = zl>>16L; 
			
			if (do_ztest(z, zbuffer[0], zfunc,ztest))
			{
				if (zwrite==omzbwc_Enabled) zbuffer[0] = z; 

				color = omd_ARGBPackPixelA32(omd_PixInterToInt(ga),omd_PixInterToInt(gr),omd_PixInterToInt(gg),omd_PixInterToInt(gb));
				dcolor = *((unsigned short*)dest);
				dcolor32 = omd_ARGB16toA32(dcolor);

				bsrc_func(&color, &color, &dcolor32, src_argb);
				bdest_func(&dcolor32, &color, &dcolor32, dest_argb);
			
		    	omd_DOBLEND(a,0);
		    	omd_DOBLEND(r,1);
		    	omd_DOBLEND(g,2);
		    	omd_DOBLEND(b,3);
		 
				((unsigned short*)dest)[0] = omd_ARGBPackPixelA16(a,r,g,b);     	
			}
		
			zbuffer++;
			dest+=2;
			zl +=zi;
			ga += ia;
			gr += ir;
			gg += ig;
			gb += ib;    	      		
		}
	}
}


//--------------------------------------------------------------------------------------

#define omd_BLITBLEND(gun) 	{r = (src_rgba[gun] + dest_rgba[gun]);					\
							if (r<0) r = 0; else if (r>0xFF) r = 0xFF;			\
							((unsigned char*)(dest_pix))[gun] = (unsigned char)r;}

void OMediaSegmentRasterizer::draw_line_32(unsigned long 	*dest_buffer,
							 			long 				buffer_rowbytes,
							 			short				x1,
							 			short				y1,
							 			short				x2,
							 			short				y2,
                		     			unsigned long 		color,
		                     			omt_BlendFunc		blend_src, 
										omt_BlendFunc		blend_dest)
{
	short denominator, frac_x,frac_y,i,run,rise,sign_x,sign_y;
	bool	no_blend = (blend_src==omblendfc_One && blend_dest==omblendfc_Zero);
	short src_argb[4];
	short dest_argb[4],a,r,g,b;
	

	omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
	omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);


	dest_buffer = ((unsigned long*)((char*)dest_buffer + (y1*buffer_rowbytes))) + x1;

	run = x2-x1;
	if (run > 0) sign_x = 1;
	else
	{
		sign_x = -1;
		run = -run;
	}
	
	rise = y2-y1;
	if (rise>0) sign_y = buffer_rowbytes;
	else
	{
		sign_y = -buffer_rowbytes;
		rise = -rise;
	}
	
	denominator = (rise>run)?rise:run;
	frac_y = frac_x = denominator>>1;
	
	for (i=denominator;i;--i)
	{
		if (no_blend) *dest_buffer = color;
		else
		{		
			bsrc_func(&color, &color, ((omt_BlendPixel*)dest_buffer), src_argb);
			bdest_func(((omt_BlendPixel*)dest_buffer), &color, ((omt_BlendPixel*)dest_buffer), dest_argb);
		
	    	omd_DOBLEND(a,0);
	    	omd_DOBLEND(r,1);
	    	omd_DOBLEND(g,2);
	    	omd_DOBLEND(b,3);
	    	
	    	*dest_buffer = omd_ARGBPackPixelA32(a,r,g,b);
		}
	
		if ( (frac_x+=run) > denominator)
		{
			frac_x -=denominator;
			dest_buffer += sign_x;
		}
		
		if ( (frac_y+=rise) > denominator)
		{
			frac_y -=denominator;
			dest_buffer = (unsigned long*) ((char*)dest_buffer + sign_y);
		}
	}

	if (no_blend) *dest_buffer = color;
	else
	{		
		bsrc_func(&color, &color, ((omt_BlendPixel*)dest_buffer), src_argb);
		bdest_func(((omt_BlendPixel*)dest_buffer), &color, ((omt_BlendPixel*)dest_buffer), dest_argb);
	
    	omd_DOBLEND(a,0);
    	omd_DOBLEND(r,1);
    	omd_DOBLEND(g,2);
    	omd_DOBLEND(b,3);
    	
    	*dest_buffer = omd_ARGBPackPixelA32(a,r,g,b);
	}
}
								
void OMediaSegmentRasterizer::draw_line_16(unsigned short 	*dest_buffer,
							 			long 			buffer_rowbytes,
							 			short			x1,
							 			short			y1,
							 			short			x2,
							 			short			y2,
                		     			unsigned long 	color32,
		                     			omt_BlendFunc	blend_src, 
										omt_BlendFunc	blend_dest)
{
	short denominator, frac_x,frac_y,i,run,rise,sign_x,sign_y;
	unsigned short	color = omd_ARGB32to16(color32);
	bool	no_blend = (blend_src==omblendfc_One && blend_dest==omblendfc_Zero);
	short src_argb[4];
	short dest_argb[4],a,r,g,b;

	omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
	omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);


	dest_buffer = ((unsigned short*)((char*)dest_buffer + (y1*buffer_rowbytes))) + x1;

	run = x2-x1;
	if (run > 0) sign_x = 1;
	else
	{
		sign_x = -1;
		run = -run;
	}
	
	rise = y2-y1;
	if (rise>0) sign_y = buffer_rowbytes;
	else
	{
		sign_y = -buffer_rowbytes;
		rise = -rise;
	}
	
	denominator = (rise>run)?rise:run;
	frac_y = frac_x = denominator>>1;
	
	for (i=denominator;i;--i)
	{
		if (no_blend) *dest_buffer = color;
		else
		{
			unsigned long dp;
			
			dp = *dest_buffer;
			dp = omd_ARGB16to32(dp);
			
			bsrc_func(&color32, &color32, ((omt_BlendPixel*)&dp), src_argb);
			bdest_func(((omt_BlendPixel*)&dp), &color32, ((omt_BlendPixel*)&dp), dest_argb);
		
	    	omd_DOBLEND(a,0);
	    	omd_DOBLEND(r,1);
	    	omd_DOBLEND(g,2);
	    	omd_DOBLEND(b,3);
	    	
	    	*dest_buffer = omd_ARGBPackPixelA16(a,r,g,b);
		}

		if ( (frac_x+=run) > denominator)
		{
			frac_x -=denominator;
			dest_buffer += sign_x;
		
		}
		
		if ( (frac_y+=rise) > denominator)
		{
			frac_y -=denominator;
			dest_buffer = (unsigned short*) ((char*)dest_buffer + sign_y);
		}
	}

	if (no_blend) *dest_buffer = color;
	else
	{
		unsigned long dp;
		
		dp = *dest_buffer;
		dp = omd_ARGB16to32(dp);
		
		bsrc_func(&color32, &color32, ((omt_BlendPixel*)&dp), src_argb);
		bdest_func(((omt_BlendPixel*)&dp), &color32, ((omt_BlendPixel*)&dp), dest_argb);
	
    	omd_DOBLEND(a,0);
    	omd_DOBLEND(r,1);
    	omd_DOBLEND(g,2);
    	omd_DOBLEND(b,3);
    	
    	*dest_buffer = omd_ARGBPackPixelA16(a,r,g,b);
	}
}

void OMediaSegmentRasterizer::draw_line_zb32(unsigned long 	*dest_buffer,
							 			long 				buffer_rowbytes,
							 			short				x1,
							 			short				y1,
							 			float				fz1,
							 			short				x2,
							 			short				y2,
							 			float				fz2,
                		     			unsigned long 		color,
		                     			omt_BlendFunc		blend_src, 
										omt_BlendFunc		blend_dest,
										unsigned short		*dest_zbuffer,
										unsigned long		zb_modulo,	
										omt_ZBufferFunc		zfunc,
										omt_ZBufferTest		ztest,
										omt_ZBufferWrite	zwrite)
{
	short denominator, frac_x,frac_y,i,run,rise,sign_x,sign_y;

	long 			z, x_dz, y_dz,zparam;
	unsigned short	zs;
	
	bool	no_blend = (blend_src==omblendfc_One && blend_dest==omblendfc_Zero);
	short src_argb[4];
	short dest_argb[4],a,r,g,b;
	

	omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
	omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);


	z = long(fz1*float(0x7FFFFFFF));
	zparam = long((fz2-fz1)*float(0x7FFFFFFF));

	run = x2-x1;
	rise = y2-y1;

	if (omd_Abs(run)>omd_Abs(rise))
	{
		if (x2!=x1) x_dz = zparam / long(run);
		else x_dz = 0;
		y_dz = 0;
	}
	else
	{	
		if (y2!=y1) y_dz = zparam / long(rise);
		else y_dz = 0;
		x_dz = 0;
	}

	dest_buffer = ((unsigned long*)((char*)dest_buffer + (y1*buffer_rowbytes))) + x1;
	dest_zbuffer = ((unsigned short*)(((char*)dest_zbuffer) + (y1*zb_modulo))) + x1;

	if (run > 0) sign_x = 1;
	else
	{
		sign_x = -1;
		run = -run;
	}
	
	if (rise>0) sign_y = buffer_rowbytes;
	else
	{
		sign_y = -buffer_rowbytes;
		rise = -rise;
		zb_modulo = -zb_modulo;
	}
	
	denominator = (rise>run)?rise:run;
	frac_y = frac_x = denominator>>1;
	
	for (i=denominator;i;--i)
	{
		zs = (unsigned short)( ((unsigned long)z)>>16L);
		if (do_ztest(zs,*dest_zbuffer,zfunc,ztest)) 
		{
			if (zwrite==omzbwc_Enabled) *dest_zbuffer = zs;
			if (no_blend) *dest_buffer = color;
			else
			{		
				bsrc_func(&color, &color, ((omt_BlendPixel*)dest_buffer), src_argb);
				bdest_func(((omt_BlendPixel*)dest_buffer), &color, ((omt_BlendPixel*)dest_buffer), dest_argb);
			
		    	omd_DOBLEND(a,0);
		    	omd_DOBLEND(r,1);
		    	omd_DOBLEND(g,2);
		    	omd_DOBLEND(b,3);
		    	
		    	*dest_buffer = omd_ARGBPackPixelA32(a,r,g,b);
			}
		}

		if ( (frac_x+=run) > denominator)
		{
			frac_x -=denominator;
			dest_buffer += sign_x;
			dest_zbuffer += sign_x;
			z += x_dz;			
		}
		
		if ( (frac_y+=rise) > denominator)
		{
			frac_y -=denominator;
			dest_buffer = (unsigned long*) ((char*)dest_buffer + sign_y);
			dest_zbuffer = (unsigned short*)((char*)dest_zbuffer + zb_modulo);
			z += y_dz;
		}
	}

	zs = (unsigned short)( ((unsigned long)z)>>16L);
	if (do_ztest(zs,*dest_zbuffer,zfunc,ztest)) 
	{		
		if (zwrite==omzbwc_Enabled) *dest_zbuffer = zs;	
		if (no_blend) *dest_buffer = color;
		else
		{		
			bsrc_func(&color, &color, ((omt_BlendPixel*)dest_buffer), src_argb);
			bdest_func(((omt_BlendPixel*)dest_buffer), &color, ((omt_BlendPixel*)dest_buffer), dest_argb);
		
	    	omd_DOBLEND(a,0);
	    	omd_DOBLEND(r,1);
	    	omd_DOBLEND(g,2);
	    	omd_DOBLEND(b,3);
	    	
	    	*dest_buffer = omd_ARGBPackPixelA32(a,r,g,b);
		}
	}
}
								
void OMediaSegmentRasterizer::draw_line_zb16(unsigned short 	*dest_buffer,
							 			long 			buffer_rowbytes,
							 			short			x1,
							 			short			y1,
							 			float			fz1,
							 			short			x2,
							 			short			y2,
							 			float			fz2,
                		     			unsigned long 	color32,
		                     			omt_BlendFunc		blend_src, 
										omt_BlendFunc		blend_dest,
										unsigned short		*dest_zbuffer,
										unsigned long		zb_modulo,										
										omt_ZBufferFunc		zfunc,
										omt_ZBufferTest		ztest,
										omt_ZBufferWrite	zwrite)
{
	short denominator, frac_x,frac_y,i,run,rise,sign_x,sign_y;

	long 			z, x_dz, y_dz,zparam;
	unsigned short	zs, color = omd_ARGB32to16(color32);

	bool	no_blend = (blend_src==omblendfc_One && blend_dest==omblendfc_Zero);
	short src_argb[4];
	short dest_argb[4],a,r,g,b;

	omt_BlendFunctionPtr bsrc_func = OMediaBlitter::find_blend_func_argb(blend_src);
	omt_BlendFunctionPtr bdest_func = OMediaBlitter::find_blend_func_argb(blend_dest);
	

	z = long(fz1*float(0x7FFFFFFF));
	zparam = long((fz2-fz1)*float(0x7FFFFFFF));

	run = x2-x1;
	rise = y2-y1;

	if (omd_Abs(run)>omd_Abs(rise))
	{
		if (x2!=x1) x_dz = zparam / long(run);
		else x_dz = 0;
		y_dz = 0;
	}
	else
	{	
		if (y2!=y1) y_dz = zparam / long(rise);
		else y_dz = 0;
		x_dz = 0;
	}

	dest_buffer = ((unsigned short*)((char*)dest_buffer + (y1*buffer_rowbytes))) + x1;
	dest_zbuffer = ((unsigned short*)(((char*)dest_zbuffer) + (y1*zb_modulo))) + x1;

	if (run > 0) sign_x = 1;
	else
	{
		sign_x = -1;
		run = -run;
	}
	
	if (rise>0) sign_y = buffer_rowbytes;
	else
	{
		sign_y = -buffer_rowbytes;
		rise = -rise;
		zb_modulo = -zb_modulo;
	}
	
	denominator = (rise>run)?rise:run;
	frac_y = frac_x = denominator>>1;
	
	for (i=denominator;i;--i)
	{
		zs = (unsigned short)( ((unsigned long)z)>>16L);
		if (do_ztest(zs,*dest_zbuffer,zfunc,ztest)) 
		{
			if (zwrite==omzbwc_Enabled) *dest_zbuffer = zs;
			if (no_blend) *dest_buffer = color;
			else
			{
				unsigned long dp;
				
				dp = *dest_buffer;
				dp = omd_ARGB16to32(dp);
				
				bsrc_func(&color32, &color32, ((omt_BlendPixel*)&dp), src_argb);
				bdest_func(((omt_BlendPixel*)&dp), &color32, ((omt_BlendPixel*)&dp), dest_argb);
			
		    	omd_DOBLEND(a,0);
		    	omd_DOBLEND(r,1);
		    	omd_DOBLEND(g,2);
		    	omd_DOBLEND(b,3);
		    	
		    	*dest_buffer = omd_ARGBPackPixelA16(a,r,g,b);
			}
		}

		if ( (frac_x+=run) > denominator)
		{
			frac_x -=denominator;
			dest_buffer += sign_x;
			dest_zbuffer += sign_x;
			z += x_dz;			
		}
		
		if ( (frac_y+=rise) > denominator)
		{
			frac_y -=denominator;
			dest_buffer = (unsigned short*) ((char*)dest_buffer + sign_y);
			dest_zbuffer = (unsigned short*)((char*)dest_zbuffer + zb_modulo);
			z += y_dz;
		}
	}

	zs = (unsigned short)( ((unsigned long)z)>>16L);
	if (do_ztest(zs,*dest_zbuffer,zfunc,ztest)) 
	{		
		if (zwrite==omzbwc_Enabled) *dest_zbuffer = zs;	
		if (no_blend) *dest_buffer = color;
		else
		{
			unsigned long dp;
			
			dp = *dest_buffer;
			dp = omd_ARGB16to32(dp);
			
			bsrc_func(&color32, &color32, ((omt_BlendPixel*)&dp), src_argb);
			bdest_func(((omt_BlendPixel*)&dp), &color32, ((omt_BlendPixel*)&dp), dest_argb);
		
	    	omd_DOBLEND(a,0);
	    	omd_DOBLEND(r,1);
	    	omd_DOBLEND(g,2);
	    	omd_DOBLEND(b,3);
	    	
	    	*dest_buffer = omd_ARGBPackPixelA16(a,r,g,b);
		}
	}
}


#endif

