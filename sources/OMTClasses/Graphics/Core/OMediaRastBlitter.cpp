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
 

#include "OMediaRastBlitter.h"
#include "OMediaBlitter.h"
#include "OMediaFixedPoint.h"

#define omd_DOBLEND(res,gun) 	{res = (src_rgba[gun] + dest_rgba[gun]); if (short(res)<0) res = 0; else if (short(res)>0xFF) res = 0xFF;}


//----------------------------------------------------------------
// Blit 32 no zb

#define omd_DIRECTBLIT(src)									\
					pix = *(src);							\
					*(dest_pix) = omd_RGBAtoARGB_32(pix);	


#define omd_BLENDBLIT(src)										\
	scolor = *src;											\
	omd_RGBAUnPackPixelA32(scolor,a,r,g,b);								\
													\
	if (a==0xFF && flat_alpha==0xFF) *dest_pix = omd_RGBAtoARGB_32(scolor);				\
	else if (a!=0)											\
	{												\
		a = (a*flat_alpha)>>8;									\
                                                                                                        \
		dcolor = *dest_pix;									\
		omd_ARGBUnPackPixel32(dcolor,dstr,dstg,dstb);						\
													\
													\
		sna = 0xFF-a;										\
                                                                                                        \
		r = (r * a + sna * dstr) >> 8;							\
		g = (g * a + sna * dstg) >> 8;							\
		b = (b * a + sna * dstb) >> 8;							\
												\
		*dest_pix = omd_ARGBPackPixel32(r,g,b);						\
	}


#define omd_GENERICBLIT(src)									\
	dcolor = *dest_pix;									\
	dcolor = omd_ARGBtoRGBA_A32(dcolor);							\
												\
	scolor = *src_pix;									\
	scolor = omd_RGBAMULALPHA_32(scolor,flat_alpha);					\
												\
	bsrc_func(&scolor, &scolor, &dcolor, src_rgba);						\
	bdest_func(&dcolor, &scolor, &dcolor, dest_rgba);					\
                                                                                                \
        omd_DOBLEND(a,0);                                                               	\
	omd_DOBLEND(r,1);									\
        omd_DOBLEND(g,2);									\
	omd_DOBLEND(b,3);									\
												\
	*dest_pix = omd_ARGBPackPixelA32(a,r,g,b);




void OMediaRastBlitter::draw_full_32(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha)
{
	OMediaRect		src,dest,res;

	src.set(x,y,x+src_width,y+src_height);
	dest.set(0,0,dest_width,dest_height);

	if (!src.find_intersection(&dest,&res)) return;

	if (src.is_equal(&res))
	{
		long			dest_modulo;
		unsigned long	pix;
		omt_RGBAPixel	*last_pix;

		dest_pix = (unsigned long *)(((char*)dest_pix) + (x<<2) + (y*dest_rowbytes));	
		dest_modulo = dest_rowbytes - (src_width<<2L);

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
		{
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix) 
				{
					omd_DIRECTBLIT(src_pix);
					src_pix++;
					dest_pix++;
				}

				dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
			}
		}
		else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)	// Optimize transparency
		{
			unsigned short sna,a,r,g,b,dstr,dstg,dstb;
			unsigned long scolor,dcolor;
		
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix)     
				{
					omd_BLENDBLIT(src_pix);

					src_pix++;
					dest_pix++;
				}

				dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
			}
		
		}		
		else		// General blending
		{

			short src_rgba[4];
			short dest_rgba[4];
			unsigned long	scolor,dcolor;
			unsigned short a,r,g,b;

			omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
			omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);
		
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix) 
				{
					omd_GENERICBLIT(src_pix);
				
					src_pix++;
					dest_pix++;
				}
				dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
			}			
		}
	}
	else
	{
		res.left -= x;	res.right -= x; res.top -=y; res.bottom -=y;	
		draw_32(src_pix, src_width, src_height, dest_pix, dest_width, dest_height,
			dest_rowbytes,
			 &res, x+res.left, y+res.top, blend_src, blend_dest,flat_alpha);
	}
}

void OMediaRastBlitter::draw_32(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha)
{
	OMediaRect	bounds;
	OMediaRect	dest;
	unsigned long pix;

	if (src->left>=src->right || src->top>=src->bottom) return;

	bounds.set(0,0,dest_width,dest_height);

	dest.left = x;
	dest.top  = y;

	if (!OMediaBlitter::block_preparecliprects(*src, dest, bounds, src_width, src_height)) return;

	dest_pix = (unsigned long *)(((char*)dest_pix) + (dest.left<<2) + (dest.top*dest_rowbytes));
	src_pix += src->left+(src->top*src_width);

	long	dest_modulo,src_modulo,w,h;
	omt_RGBAPixel	*last_pix;

	w = dest.get_width();
	h = dest.get_height();
	src_modulo = src_width - w;
	dest_modulo = dest_rowbytes - (w<<2); 

	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)	// No blending
	{
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix) 
			{
				omd_DIRECTBLIT(src_pix);
				src_pix++;
				dest_pix++;
			}
			
			src_pix += src_modulo;
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
		}
	}
	else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)	// Optimize transparency
	{
		unsigned short sna,a,r,g,b,dstr,dstg,dstb;
		unsigned long scolor,dcolor;
		
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix)     
			{
				omd_BLENDBLIT(src_pix);

				src_pix++;
				dest_pix++;
			}

			src_pix += src_modulo;
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
		}		
	}
	else		// General blending
	{
		short src_rgba[4];
		short dest_rgba[4];
		unsigned long	dcolor,scolor;
		unsigned short a,r,g,b;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);
	
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix) 
			{
				omd_GENERICBLIT(src_pix);
			
				src_pix++;
				dest_pix++;
			}

			src_pix += src_modulo;
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
		}
	}	
} 

void OMediaRastBlitter::draw_32(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha)
{
	if (src->get_width()==dest->get_width() && src->get_height()==dest->get_height()) 
	{
		draw_32(src_pix, src_width, src_height, dest_pix,  dest_width,  dest_height, dest_rowbytes, src, dest->left, dest->top,
				blend_src,blend_dest,flat_alpha);
				
		return;
	}

	// Scale

	OMediaRect	src_bounds,dest_bounds;
	src_bounds.set(0,0,src_width,src_height);
	dest_bounds.set(0,0,dest_width,dest_height);

	if (!src_bounds.find_intersection(src, &src_bounds) || dest->empty()) return;
	if (!dest_bounds.find_intersection(dest, &dest_bounds)) return;
	
	omt_FixedPoint16_16		px,py,ix,dx,dy;
	float					scalex,scaley,sx,sy;
	
	scalex = float(src_bounds.get_width()) / float(dest->get_width());
	scaley = float(src_bounds.get_height()) / float(dest->get_height());

	sx = (dest->left<0)	? float(-dest->left)*scalex:0.0f;
	sy = (dest->top<0)	? float(-dest->top)*scaley:0.0f;

	px = omd_FloatToFixed16_16(sx);
	py = omd_FloatToFixed16_16(sy);

	src_pix 	+= src->left+omd_FixedToInt16_16(px) + ((src->top+omd_FixedToInt16_16(py)) * src_width);
	dest_pix = (unsigned long *)(((char*)dest_pix) + (dest_bounds.left<<2) + (dest_bounds.top*dest_rowbytes));

	long			w,h,dest_modulo;
	omt_RGBAPixel	*last_pix,*xpix;

	w = dest_bounds.get_width();
	h = dest_bounds.get_height();

	dest_modulo = dest_rowbytes - (w<<2);

	px &= 0xFFFFL;
	py &= 0xFFFFL;
	
	dx = omd_FloatToFixed16_16(scalex);
	dy = omd_FloatToFixed16_16(scaley);


	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)	// No blending
	{
		unsigned long pix;

		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_DIRECTBLIT(xpix);
				dest_pix++;

				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
		}
	}
	else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)	// Optimize transparency
	{
		unsigned short sna,a,r,g,b,dstr,dstg,dstb;
		unsigned long scolor,dcolor;
	
		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_BLENDBLIT(xpix);
				dest_pix++;
				
				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
		}	
	}
	else		// General
	{
		short src_rgba[4];
		short dest_rgba[4];
		unsigned long	dcolor,scolor;
		unsigned short a,r,g,b;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);

		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_GENERICBLIT(xpix);

				dest_pix++;
				
				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
		}	
	}
}

#undef omd_DIRECTBLIT
#undef omd_BLENDBLIT
#undef omd_GENERICBLIT

//----------------------------------------------------------------
// Blit 32 zb

#define omd_DIRECTBLIT(src)						\
	if (z<(*dest_z)) 							\
	{											\
		if (dowritez) (*dest_z) = z; 			\
		pix = *(src);							\
		*(dest_pix) = omd_RGBAtoARGB_32(pix);	\
	}

#define omd_BLENDBLIT(src)									\
	if (z<(*dest_z)) 										\
	{														\
		if (dowritez) (*dest_z) = z; 						\
		scolor = *src;											\
		omd_RGBAUnPackPixelA32(scolor,a,r,g,b);					\
																\
		if (a==0xFF && flat_alpha==0xFF) *dest_pix = omd_RGBAtoARGB_32(scolor);	\
		else if (a!=0)												\
		{															\
			a = (a*flat_alpha)>>8;									\
																	\
			dcolor = *dest_pix;										\
			omd_ARGBUnPackPixel32(dcolor,dstr,dstg,dstb);			\
																	\
																	\
			sna = 0xFF-a;											\
																	\
			r = (r * a + sna * dstr) >> 8;							\
			g = (g * a + sna * dstg) >> 8;							\
			b = (b * a + sna * dstb) >> 8;							\
																	\
			*dest_pix = omd_ARGBPackPixel32(r,g,b);					\
		}															\
	}

#define omd_GENERICBLIT(src)									\
	if (do_ztest(z,*dest_z,zfunc,ztest)) 					\
	{														\
		if (dowritez) (*dest_z) = z; 						\
		dcolor = *dest_pix;										\
		dcolor = omd_ARGBtoRGBA_A32(dcolor);					\
															\
		scolor = *src_pix;									\
		scolor = omd_RGBAMULALPHA_32(scolor,flat_alpha);	\
															\
		bsrc_func(&scolor, &scolor, &dcolor, src_rgba);		\
		bdest_func(&dcolor, &scolor, &dcolor, dest_rgba);	\
															\
		omd_DOBLEND(a,0);									\
		omd_DOBLEND(r,1);									\
		omd_DOBLEND(g,2);									\
		omd_DOBLEND(b,3);									\
															\
		*dest_pix = omd_ARGBPackPixelA32(a,r,g,b);			\
	}



void OMediaRastBlitter::draw_full_32zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				fz,
							unsigned short		*dest_z,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite)
{
	OMediaRect		src,dest,res;

	src.set(x,y,x+src_width,y+src_height);
	dest.set(0,0,dest_width,dest_height);

	if (!src.find_intersection(&dest,&res)) return;

	if (src.is_equal(&res))
	{
		long			dest_modulo,zmodulo;
		unsigned long	pix;
		unsigned short	z;
		omt_RGBAPixel	*last_pix;
		bool			fast_z = (zfunc==omzbfc_Less && ztest==omzbtc_Enabled);
		bool dowritez = (zwrite==omzbwc_Enabled);
	
		z = long(fz*float(0x7FFFFFFF));

		dest_pix = (unsigned long *)(((char*)dest_pix) + (x<<2) + (y*dest_rowbytes));
		dest_modulo = dest_rowbytes - (src_width<<2L);

		dest_z	 =	(unsigned short*)(((char*)dest_z) + (y*zrowbytes) + (x<<1));
		zmodulo = zrowbytes - (src_width<<1);

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && fast_z)
		{
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix) 
				{
					omd_DIRECTBLIT(src_pix);
					src_pix++;
					dest_pix++;
					dest_z++;
				}

				dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
				dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
			}
		}
		else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha
					&& fast_z)	// Optimize transparency
		{
			unsigned short sna,a,r,g,b,dstr,dstg,dstb;
			unsigned long scolor,dcolor;
		
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix)     
				{
					omd_BLENDBLIT(src_pix);

					src_pix++;
					dest_pix++;
					dest_z++;
				}

				dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
				dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
			}
		
		}		
		else		// General blending
		{

			short src_rgba[4];
			short dest_rgba[4];
			unsigned long	scolor,dcolor;
			unsigned short a,r,g,b;

			omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
			omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);
		
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix) 
				{
					omd_GENERICBLIT(src_pix);
				
					src_pix++;
					dest_pix++;
					dest_z++;
				}
				dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
				dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
			}			
		}
	}
	else
	{
		res.left -= x;	res.right -= x; res.top -=y; res.bottom -=y;	
		draw_32zb(src_pix, src_width, src_height, dest_pix, dest_width, dest_height,
			dest_rowbytes,
			 &res, x+res.left, y+res.top, blend_src, blend_dest,flat_alpha,
			 fz,dest_z,zrowbytes,zfunc,ztest,zwrite);
	}
}

void OMediaRastBlitter::draw_32zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				fz,
							unsigned short		*dest_z,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite)
{
	OMediaRect	bounds;
	OMediaRect	dest;
	unsigned long pix;

	if (src->left>=src->right || src->top>=src->bottom) return;

	bounds.set(0,0,dest_width,dest_height);

	dest.left = x;
	dest.top  = y;

	if (!OMediaBlitter::block_preparecliprects(*src, dest, bounds, src_width, src_height)) return;

	dest_pix = (unsigned long *)(((char*)dest_pix) + (dest.left<<2) + (dest.top*dest_rowbytes));
	src_pix += src->left+(src->top*src_width);
	dest_z = (unsigned short*)(((char*)dest_z) + (dest.top*zrowbytes) + (dest.left<<1));

	long	dest_modulo,src_modulo,w,h,zmodulo;
	omt_RGBAPixel	*last_pix;
	unsigned short	z;
	bool			fast_z = (zfunc==omzbfc_Less && ztest==omzbtc_Enabled);
	bool dowritez = (zwrite==omzbwc_Enabled);

	z = long(fz*float(0x7FFFFFFF));

	w = dest.get_width();
	h = dest.get_height();
	src_modulo = src_width - w;
	dest_modulo = dest_rowbytes - (w<<2); 
	zmodulo = zrowbytes - (w<<1);


	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && fast_z)	// No blending
	{
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix) 
			{
				omd_DIRECTBLIT(src_pix);
				src_pix++;
				dest_pix++;
				dest_z++;
			}
			
			src_pix += src_modulo;
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}
	}
	else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha && fast_z)	// Optimize transparency
	{
		unsigned short sna,a,r,g,b,dstr,dstg,dstb;
		unsigned long scolor,dcolor;
		
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix)     
			{
				omd_BLENDBLIT(src_pix);

				src_pix++;
				dest_pix++;
				dest_z++;
			}

			src_pix += src_modulo;
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}		
	}
	else		// General blending
	{
		short src_rgba[4];
		short dest_rgba[4];
		unsigned long	dcolor,scolor;
		unsigned short a,r,g,b;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);
	
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix) 
			{
				omd_GENERICBLIT(src_pix);
			
				src_pix++;
				dest_pix++;
				dest_z++;
			}

			src_pix += src_modulo;
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}
	}	
} 

void OMediaRastBlitter::draw_32zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned long *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				fz,
							unsigned short		*dest_z,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite)
{
	if (src->get_width()==dest->get_width() && src->get_height()==dest->get_height()) 
	{
		draw_32zb(src_pix, src_width, src_height, dest_pix,  dest_width,  dest_height, dest_rowbytes, src, dest->left, dest->top,
				blend_src,blend_dest,flat_alpha,fz,dest_z,zrowbytes,zfunc,ztest,zwrite);
				
		return;
	}

	// Scale

	OMediaRect	src_bounds,dest_bounds;
	src_bounds.set(0,0,src_width,src_height);
	dest_bounds.set(0,0,dest_width,dest_height);

	if (!src_bounds.find_intersection(src, &src_bounds) || dest->empty()) return;
	if (!dest_bounds.find_intersection(dest, &dest_bounds)) return;
	
	omt_FixedPoint16_16		px,py,ix,dx,dy;
	float					scalex,scaley,sx,sy;
	
	scalex = float(src_bounds.get_width()) / float(dest->get_width());
	scaley = float(src_bounds.get_height()) / float(dest->get_height());

	sx = (dest->left<0)	? float(-dest->left)*scalex:0.0f;
	sy = (dest->top<0)	? float(-dest->top)*scaley:0.0f;

	px = omd_FloatToFixed16_16(sx);
	py = omd_FloatToFixed16_16(sy);

	src_pix 	+= src->left+omd_FixedToInt16_16(px) + ((src->top+omd_FixedToInt16_16(py)) * src_width);
	dest_pix = (unsigned long *)(((char*)dest_pix) + (dest_bounds.left<<2) + (dest_bounds.top*dest_rowbytes));
	dest_z = (unsigned short*)(((char*)dest_z) + (dest_bounds.top*zrowbytes) + (dest_bounds.left<<1));

	long			w,h,dest_modulo,zmodulo;
	omt_RGBAPixel	*last_pix,*xpix;
	unsigned short	z;
	bool			fast_z = (zfunc==omzbfc_Less && ztest==omzbtc_Enabled);
	bool			dowritez = (zwrite==omzbwc_Enabled);


	z = long(fz*float(0x7FFFFFFF));

	w = dest_bounds.get_width();
	h = dest_bounds.get_height();

	dest_modulo = dest_rowbytes - (w<<2);
	zmodulo = zrowbytes - (w<<1);

	px &= 0xFFFFL;
	py &= 0xFFFFL;
	
	dx = omd_FloatToFixed16_16(scalex);
	dy = omd_FloatToFixed16_16(scaley);


	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && fast_z)	// No blending
	{
		unsigned long pix;

		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_DIRECTBLIT(xpix);
				dest_pix++;
				dest_z++;

				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}
	}
	else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha && fast_z)	// Optimize transparency
	{
		unsigned short sna,a,r,g,b,dstr,dstg,dstb;
		unsigned long scolor,dcolor;
	
		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_BLENDBLIT(xpix);
				dest_pix++;
				dest_z++;
				
				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}	
	}
	else		// General
	{
		short src_rgba[4];
		short dest_rgba[4];
		unsigned long	dcolor,scolor;
		unsigned short a,r,g,b;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);

		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_GENERICBLIT(xpix);

				dest_pix++;
				dest_z++;
				
				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned long*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}	
	}
}

#undef omd_DIRECTBLIT
#undef omd_BLENDBLIT
#undef omd_GENERICBLIT


//----------------------------------------------------------------
// Blit 16 no zb

#define omd_DIRECTBLIT(src)									\
					pix = *src;								\
					pix = omd_RGBAtoARGB_32(pix);			\
					*dest_pix = omd_ARGB32to16(pix);	

#define omd_BLENDBLIT(src)		\
							\
	scolor = *src;											\
	omd_RGBAUnPackPixelA32(scolor,a,r,g,b);					\
															\
	if (a==0xFF && flat_alpha==0xFF)							\
	{															\
		scolor = omd_RGBAtoARGB_32(scolor);						\
		*dest_pix = omd_ARGB32to16(scolor);				\
	}															\
	else if (a!=0)												\
	{															\
		a = (a*flat_alpha)>>8;									\
																\
		dcolor = *dest_pix;										\
		omd_ARGBUnPackPixel16(dcolor,dstr,dstg,dstb);			\
																\
																\
		sna = 0xFF-a;											\
																\
		r = (r * a + sna * dstr) >> 8;							\
		g = (g * a + sna * dstg) >> 8;							\
		b = (b * a + sna * dstb) >> 8;							\
																\
		*dest_pix = omd_ARGBPackPixel16(r,g,b);					\
	}											


#define omd_GENERICBLIT(src)							\
	dcolor = *dest_pix;									\
	dcolor = omd_ARGB16to32(dcolor);					\
	dcolor = omd_ARGBtoRGBA_A32(dcolor);				\
														\
	scolor = *src_pix;									\
	scolor = omd_RGBAMULALPHA_32(scolor,flat_alpha);	\
														\
	bsrc_func(&scolor, &scolor, &dcolor, src_rgba);		\
	bdest_func(&dcolor, &scolor, &dcolor, dest_rgba);	\
														\
	omd_DOBLEND(a,0);									\
	omd_DOBLEND(r,1);									\
    omd_DOBLEND(g,2);									\
	omd_DOBLEND(b,3);									\
														\
	*dest_pix = omd_ARGBPackPixelA16(a,r,g,b);




void OMediaRastBlitter::draw_full_16(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha)
{
	OMediaRect		src,dest,res;

	src.set(x,y,x+src_width,y+src_height);
	dest.set(0,0,dest_width,dest_height);

	if (!src.find_intersection(&dest,&res)) return;

	if (src.is_equal(&res))
	{
		long			dest_modulo;
		unsigned long	pix;
		omt_RGBAPixel	*last_pix;

		dest_pix = (unsigned short *)(((char*)dest_pix) + (x<<1) + (y*dest_rowbytes));
		dest_modulo = dest_rowbytes - (src_width<<1L);

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
		{
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix) 
				{
					omd_DIRECTBLIT(src_pix);
					src_pix++;
					dest_pix++;
				}

				dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
			}
		}
		else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)	// Optimize transparency
		{
			unsigned short sna,a,r,g,b,dstr,dstg,dstb;
			unsigned long scolor,dcolor;
		
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix)     
				{
					omd_BLENDBLIT(src_pix);

					src_pix++;
					dest_pix++;
				}

				dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
			}
		
		}		
		else		// General blending
		{

			short src_rgba[4];
			short dest_rgba[4];
			unsigned long	scolor,dcolor;
			unsigned short a,r,g,b;

			omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
			omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);
		
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix) 
				{
					omd_GENERICBLIT(src_pix);
				
					src_pix++;
					dest_pix++;
				}
				dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
			}			
		}
	}
	else
	{
		res.left -= x;	res.right -= x; res.top -=y; res.bottom -=y;	
		draw_16(src_pix, src_width, src_height, dest_pix, dest_width, dest_height,
			dest_rowbytes,
			 &res, x+res.left, y+res.top, blend_src, blend_dest,flat_alpha);
	}
}

void OMediaRastBlitter::draw_16(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha)
{
	OMediaRect	bounds;
	OMediaRect	dest;
	unsigned long pix;

	if (src->left>=src->right || src->top>=src->bottom) return;

	bounds.set(0,0,dest_width,dest_height);

	dest.left = x;
	dest.top  = y;

	if (!OMediaBlitter::block_preparecliprects(*src, dest, bounds, src_width, src_height)) return;

	dest_pix = (unsigned short *)(((char*)dest_pix) + (dest.left<<1) + (dest.top*dest_rowbytes));
	src_pix += src->left+(src->top*src_width);

	long	dest_modulo,src_modulo,w,h;
	omt_RGBAPixel	*last_pix;

	w = dest.get_width();
	h = dest.get_height();
	src_modulo = src_width - w;
	dest_modulo = dest_rowbytes - (w<<1); 

	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)	// No blending
	{
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix) 
			{
				omd_DIRECTBLIT(src_pix);
				src_pix++;
				dest_pix++;
			}
			
			src_pix += src_modulo;
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
		}
	}
	else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)	// Optimize transparency
	{
		unsigned short sna,a,r,g,b,dstr,dstg,dstb;
		unsigned long scolor,dcolor;
		
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix)     
			{
				omd_BLENDBLIT(src_pix);

				src_pix++;
				dest_pix++;
			}

			src_pix += src_modulo;
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
		}		
	}
	else		// General blending
	{
		short src_rgba[4];
		short dest_rgba[4];
		unsigned long	dcolor,scolor;
		unsigned short a,r,g,b;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);
	
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix) 
			{
				omd_GENERICBLIT(src_pix);
			
				src_pix++;
				dest_pix++;
			}
			src_pix += src_modulo;
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
		}
	}	
} 

void OMediaRastBlitter::draw_16(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha)
{
	if (src->get_width()==dest->get_width() && src->get_height()==dest->get_height()) 
	{
		draw_16(src_pix, src_width, src_height, dest_pix,  dest_width,  dest_height, dest_rowbytes, src, dest->left, dest->top,
				blend_src,blend_dest,flat_alpha);
				
		return;
	}

	// Scale

	OMediaRect	src_bounds,dest_bounds;
	src_bounds.set(0,0,src_width,src_height);
	dest_bounds.set(0,0,dest_width,dest_height);

	if (!src_bounds.find_intersection(src, &src_bounds) || dest->empty()) return;
	if (!dest_bounds.find_intersection(dest, &dest_bounds)) return;
	
	omt_FixedPoint16_16		px,py,ix,dx,dy;
	float					scalex,scaley,sx,sy;
	
	scalex = float(src_bounds.get_width()) / float(dest->get_width());
	scaley = float(src_bounds.get_height()) / float(dest->get_height());

	sx = (dest->left<0)	? float(-dest->left)*scalex:0.0f;
	sy = (dest->top<0)	? float(-dest->top)*scaley:0.0f;

	px = omd_FloatToFixed16_16(sx);
	py = omd_FloatToFixed16_16(sy);

	src_pix 	+= src->left+omd_FixedToInt16_16(px) + ((src->top+omd_FixedToInt16_16(py)) * src_width);
	dest_pix = (unsigned short *)(((char*)dest_pix) + (dest_bounds.left<<1) + (dest_bounds.top*dest_rowbytes));

	long			w,h,dest_modulo;
	omt_RGBAPixel	*xpix;
	unsigned short	*last_pix;

	w = dest_bounds.get_width();
	h = dest_bounds.get_height();

	dest_modulo = dest_rowbytes - (w<<1);

	px &= 0xFFFFL;
	py &= 0xFFFFL;
	
	dx = omd_FloatToFixed16_16(scalex);
	dy = omd_FloatToFixed16_16(scaley);


	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)	// No blending
	{
		unsigned long pix;

		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_DIRECTBLIT(xpix);
				dest_pix++;

				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
		}
	}
	else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha)	// Optimize transparency
	{
		unsigned short sna,a,r,g,b,dstr,dstg,dstb;
		unsigned long scolor,dcolor;
	
		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_BLENDBLIT(xpix);
				dest_pix++;
				
				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
		}	
	}
	else		// General
	{
		short src_rgba[4];
		short dest_rgba[4];
		unsigned long	dcolor,scolor;
		unsigned short a,r,g,b;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);

		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_GENERICBLIT(xpix);

				dest_pix++;
				
				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
		}	
	}
}

#undef omd_DIRECTBLIT
#undef omd_BLENDBLIT
#undef omd_GENERICBLIT

//----------------------------------------------------------------
// Blit 16 zb

#define omd_DIRECTBLIT(src)						\
	if (z<(*dest_z)) 							\
	{											\
		pix = *src;								\
		pix = omd_RGBAtoARGB_32(pix);			\
		*dest_pix = omd_ARGB32to16(pix);		\
	}

#define omd_BLENDBLIT(src)									\
	if (z<(*dest_z)) 										\
	{														\
		if (dowritez) (*dest_z) = z; 						\
		scolor = *src;											\
		omd_RGBAUnPackPixelA32(scolor,a,r,g,b);					\
																\
		if (a==0xFF && flat_alpha==0xFF)							\
		{															\
			scolor = omd_RGBAtoARGB_32(scolor);						\
			*dest_pix = omd_ARGB32to16(scolor);				\
		}															\
		else if (a!=0)												\
		{															\
			a = (a*flat_alpha)>>8;									\
																	\
			dcolor = *dest_pix;										\
			omd_ARGBUnPackPixel16(dcolor,dstr,dstg,dstb);			\
																	\
																	\
			sna = 0xFF-a;											\
																	\
			r = (r * a + sna * dstr) >> 8;							\
			g = (g * a + sna * dstg) >> 8;							\
			b = (b * a + sna * dstb) >> 8;							\
																	\
			*dest_pix = omd_ARGBPackPixel16(r,g,b);					\
		}															\
	}

#define omd_GENERICBLIT(src)									\
	if (do_ztest(z,*dest_z,zfunc,ztest)) 					\
	{														\
		if (dowritez) (*dest_z) = z; 						\
		dcolor = *dest_pix;									\
		dcolor = omd_ARGB16to32(dcolor);					\
		dcolor = omd_ARGBtoRGBA_A32(dcolor);				\
															\
		scolor = *src_pix;									\
		scolor = omd_RGBAMULALPHA_32(scolor,flat_alpha);	\
															\
		bsrc_func(&scolor, &scolor, &dcolor, src_rgba);		\
		bdest_func(&dcolor, &scolor, &dcolor, dest_rgba);	\
															\
		omd_DOBLEND(a,0);									\
		omd_DOBLEND(r,1);									\
		omd_DOBLEND(g,2);									\
		omd_DOBLEND(b,3);									\
															\
		*dest_pix = omd_ARGBPackPixelA16(a,r,g,b);			\
	}



void OMediaRastBlitter::draw_full_16zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							long x, long y,
							omt_BlendFunc		blend_src, omt_BlendFunc	blend_dest,
							unsigned long		flat_alpha,
							float				fz,
							unsigned short		*dest_z,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite)
{
	OMediaRect		src,dest,res;

	src.set(x,y,x+src_width,y+src_height);
	dest.set(0,0,dest_width,dest_height);

	if (!src.find_intersection(&dest,&res)) return;

	if (src.is_equal(&res))
	{
		long			dest_modulo,zmodulo;
		unsigned long	pix;
		unsigned short	z;
		unsigned long	*last_pix;
		bool			fast_z = (zfunc==omzbfc_Less && ztest==omzbtc_Enabled);
		bool dowritez = (zwrite==omzbwc_Enabled);
	
		z = long(fz*float(0x7FFFFFFF));

		dest_pix = (unsigned short *)(((char*)dest_pix) + (x<<1) + (y*dest_rowbytes));
		dest_modulo = dest_rowbytes - (src_width<<1L);

		dest_z	 =	(unsigned short*)(((char*)dest_z) + (y*zrowbytes) + (x<<1));
		zmodulo = zrowbytes - (src_width<<1);

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && fast_z)
		{
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix) 
				{
					omd_DIRECTBLIT(src_pix);
					src_pix++;
					dest_pix++;
					dest_z++;
				}

				dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
				dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
			}
		}
		else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha
					&& fast_z)	// Optimize transparency
		{
			unsigned short sna,a,r,g,b,dstr,dstg,dstb;
			unsigned long scolor,dcolor;
		
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix)     
				{
					omd_BLENDBLIT(src_pix);

					src_pix++;
					dest_pix++;
					dest_z++;
				}

				dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
				dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
			}
		
		}		
		else		// General blending
		{

			short src_rgba[4];
			short dest_rgba[4];
			unsigned long	scolor,dcolor;
			unsigned short a,r,g,b;

			omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
			omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);
		
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix) 
				{
					omd_GENERICBLIT(src_pix);
				
					src_pix++;
					dest_pix++;
					dest_z++;
				}
				dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
				dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
			}			
		}
	}
	else
	{
		res.left -= x;	res.right -= x; res.top -=y; res.bottom -=y;	
		draw_16zb(src_pix, src_width, src_height, dest_pix, dest_width, dest_height,
			dest_rowbytes,
			 &res, x+res.left, y+res.top, blend_src, blend_dest,flat_alpha,
			 fz,dest_z,	zrowbytes,zfunc,ztest,zwrite);
	}
}

void OMediaRastBlitter::draw_16zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				fz,
							unsigned short		*dest_z,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite)
{
	OMediaRect	bounds;
	OMediaRect	dest;
	unsigned long pix;

	if (src->left>=src->right || src->top>=src->bottom) return;

	bounds.set(0,0,dest_width,dest_height);

	dest.left = x;
	dest.top  = y;

	if (!OMediaBlitter::block_preparecliprects(*src, dest, bounds, src_width, src_height)) return;

	dest_pix = (unsigned short *)(((char*)dest_pix) + (dest.left<<1) + (dest.top*dest_rowbytes));
	src_pix += src->left+(src->top*src_width);
	dest_z = (unsigned short*)(((char*)dest_z) + (dest.top*zrowbytes) + (dest.left<<1));

	long	dest_modulo,src_modulo,w,h,zmodulo;
	unsigned long	*last_pix;
	unsigned short	z;
	bool			fast_z = (zfunc==omzbfc_Less && ztest==omzbtc_Enabled);
	bool dowritez = (zwrite==omzbwc_Enabled);

	z = long(fz*float(0x7FFFFFFF));

	w = dest.get_width();
	h = dest.get_height();
	src_modulo = src_width - w;
	dest_modulo = dest_rowbytes - (w<<1); 
	zmodulo = zrowbytes - (w<<1);


	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && fast_z)	// No blending
	{
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix) 
			{
				omd_DIRECTBLIT(src_pix);
				src_pix++;
				dest_pix++;
				dest_z++;
			}
			
			src_pix += src_modulo;
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}
	}
	else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha && fast_z)	// Optimize transparency
	{
		unsigned short sna,a,r,g,b,dstr,dstg,dstb;
		unsigned long scolor,dcolor;
		
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix)     
			{
				omd_BLENDBLIT(src_pix);

				src_pix++;
				dest_pix++;
				dest_z++;
			}

			src_pix += src_modulo;
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}		
	}
	else		// General blending
	{
		short src_rgba[4];
		short dest_rgba[4];
		unsigned long	dcolor,scolor;
		unsigned short a,r,g,b;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);
	
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix) 
			{
				omd_GENERICBLIT(src_pix);
			
				src_pix++;
				dest_pix++;
				dest_z++;
			}
			src_pix += src_modulo;
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}
	}	
} 

void OMediaRastBlitter::draw_16zb(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							unsigned short *dest_pix, long dest_width, long dest_height, long dest_rowbytes,
							OMediaRect *src, OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,
							unsigned long flat_alpha,
							float				fz,
							unsigned short		*dest_z,	
							unsigned long		zrowbytes,
							omt_ZBufferFunc		zfunc,
							omt_ZBufferTest		ztest,
							omt_ZBufferWrite	zwrite)
{
	if (src->get_width()==dest->get_width() && src->get_height()==dest->get_height()) 
	{
		draw_16zb(src_pix, src_width, src_height, dest_pix,  dest_width,  dest_height, dest_rowbytes, src, dest->left, dest->top,
				blend_src,blend_dest,flat_alpha,fz,dest_z,zrowbytes,zfunc,ztest,zwrite);
				
		return;
	}

	// Scale

	OMediaRect	src_bounds,dest_bounds;
	src_bounds.set(0,0,src_width,src_height);
	dest_bounds.set(0,0,dest_width,dest_height);

	if (!src_bounds.find_intersection(src, &src_bounds) || dest->empty()) return;
	if (!dest_bounds.find_intersection(dest, &dest_bounds)) return;
	
	omt_FixedPoint16_16		px,py,ix,dx,dy;
	float					scalex,scaley,sx,sy;
	
	scalex = float(src_bounds.get_width()) / float(dest->get_width());
	scaley = float(src_bounds.get_height()) / float(dest->get_height());

	sx = (dest->left<0)	? float(-dest->left)*scalex:0.0f;
	sy = (dest->top<0)	? float(-dest->top)*scaley:0.0f;

	px = omd_FloatToFixed16_16(sx);
	py = omd_FloatToFixed16_16(sy);

	src_pix 	+= src->left+omd_FixedToInt16_16(px) + ((src->top+omd_FixedToInt16_16(py)) * src_width);
	dest_pix = (unsigned short *)(((char*)dest_pix) + (dest_bounds.left<<1) + (dest_bounds.top*dest_rowbytes));
	dest_z = (unsigned short*)(((char*)dest_z) + (dest_bounds.top*zrowbytes) + (dest_bounds.left<<1));

	long			w,h,dest_modulo,zmodulo;
	omt_RGBAPixel	*xpix;
	unsigned short	*last_pix;
	unsigned short	z;
	bool			fast_z = (zfunc==omzbfc_Less && ztest==omzbtc_Enabled);
	bool			dowritez = (zwrite==omzbwc_Enabled);


	z = long(fz*float(0x7FFFFFFF));

	w = dest_bounds.get_width();
	h = dest_bounds.get_height();

	dest_modulo = dest_rowbytes - (w<<1);
	zmodulo = zrowbytes - (w<<1);

	px &= 0xFFFFL;
	py &= 0xFFFFL;
	
	dx = omd_FloatToFixed16_16(scalex);
	dy = omd_FloatToFixed16_16(scaley);


	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && fast_z)	// No blending
	{
		unsigned long pix;

		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_DIRECTBLIT(xpix);
				dest_pix++;
				dest_z++;

				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}
	}
	else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha && fast_z)	// Optimize transparency
	{
		unsigned short sna,a,r,g,b,dstr,dstg,dstb;
		unsigned long scolor,dcolor;
	
		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_BLENDBLIT(xpix);
				dest_pix++;
				dest_z++;
				
				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}	
	}
	else		// General
	{
		short src_rgba[4];
		short dest_rgba[4];
		unsigned long	dcolor,scolor;
		unsigned short a,r,g,b;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);

		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				omd_GENERICBLIT(xpix);

				dest_pix++;
				dest_z++;
				
				ix += dx;
				if (ix>0xFFFFL)
				{
					xpix += omd_FixedToInt16_16(ix);
					ix &= 0xFFFFL;
				}
			}
			
			py += dy;
			if (py>0xFFFFL)
			{
				src_pix += src_width * omd_FixedToInt16_16(py);
				py &= 0xFFFFL;
			}
			
			dest_pix = (unsigned short*)(((unsigned char*)dest_pix) + dest_modulo);
			dest_z = (unsigned short*)(((unsigned char*)dest_z) + zmodulo);
		}	
	}
}

#undef omd_DIRECTBLIT
#undef omd_BLENDBLIT
#undef omd_GENERICBLIT
