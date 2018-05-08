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
 

#include "OMediaBlitter.h"
#include "OMediaRect.h"
#include "OMediaFixedPoint.h"



#define omd_BLITBLEND(gun) 	{r = (src_rgba[gun] + dest_rgba[gun]);					\
							if (r<0) r = 0; else if (r>0xFF) r = 0xFF;			\
							((unsigned char*)dest_pix)[gun] = (unsigned char)r;}


#define omd_BLITTRANSBLEND(gun) ((unsigned char*)dest_pix)[(gun)] = (unsigned char) 			\
									  (((a * (long)((unsigned char*)src_pix)[(gun)]) +		\
									  (na * (long)((unsigned char*)dest_pix)[(gun)]))/255L)

#define omd_BLITTRANSBLEND_FILL(s,gun) ((unsigned char*)p)[(gun)] = (unsigned char) 			\
									  ((s +		\
									  (na * (long)((unsigned char*)p)[(gun)]))/255L)


void OMediaBlitter::draw_full(	omt_RGBAPixel *src_pix, long src_width, long src_height,
								omt_RGBAPixel *dest_pix, long dest_width, long dest_height,
								long x, long y,
								omt_BlendFunc	blend_src, 
								omt_BlendFunc	blend_dest,

								omt_RGBAPixelMask mask)
{
	OMediaRect		src,dest,res;

	src.set(x,y,x+src_width,y+src_height);
	dest.set(0,0,dest_width,dest_height);

	if (!src.find_intersection(&dest,&res)) return;

	if (src.is_equal(&res))
	{
		long			dest_modulo;
		omt_RGBAPixel	*last_pix;
	
		dest_pix += (y*dest_width)+x;
		dest_modulo = dest_width - src_width;

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero &&

			mask==ompixmc_Full)	// No blending
		{
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix) *(dest_pix++) = *(src_pix++);
				dest_pix += dest_modulo;
			}
		}
		else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha &&

				 mask==ompixmc_Full)	// Optimize transparency
		{
			long a,na;
		
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix)     
				{
					a = ((unsigned char*)src_pix)[omd_CGUN_A];
					na = 0xFFL-a;
				
					if (a==0) {dest_pix++; src_pix++;}
					else if (a==0xFF) *(dest_pix++) = *(src_pix++);
					else
					{
						omd_BLITTRANSBLEND(omd_CGUN_R);
						omd_BLITTRANSBLEND(omd_CGUN_G);
						omd_BLITTRANSBLEND(omd_CGUN_B);
						omd_BLITTRANSBLEND(omd_CGUN_A);		
						dest_pix++; src_pix++;					
					}
				}
				dest_pix += dest_modulo;
			}
		
		}		
		else		// General blending
		{
			short src_rgba[4];
			short dest_rgba[4],r;

			omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
			omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);
		
			while(src_height--)
			{
				last_pix = src_pix + src_width;
				while(src_pix!=last_pix) 
				{
					bsrc_func(src_pix, src_pix, dest_pix, src_rgba);
					bdest_func(dest_pix, src_pix, dest_pix, dest_rgba);
				
					if (mask&ompixmc_Red)   omd_BLITBLEND(omd_CGUN_R);
					if (mask&ompixmc_Green) omd_BLITBLEND(omd_CGUN_G);
					if (mask&ompixmc_Blue)  omd_BLITBLEND(omd_CGUN_B);
					if (mask&ompixmc_Alpha) omd_BLITBLEND(omd_CGUN_A);
				
					src_pix++;
					dest_pix++;
				}
				dest_pix += dest_modulo;
			}		
		}
	}
	else
	{
		res.left -= x;	res.right -= x; res.top -=y; res.bottom -=y;	
		draw(src_pix, src_width, src_height, dest_pix, dest_width, dest_height,
			 &res, x+res.left, y+res.top, blend_src, blend_dest,mask);
	}
}

void OMediaBlitter::draw(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							omt_RGBAPixel *dest_pix, long dest_width, long dest_height,
							OMediaRect *src, long x, long y,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest, 

							omt_RGBAPixelMask mask)
{
	OMediaRect	bounds;
	OMediaRect	dest;

	if (src->left>=src->right || src->top>=src->bottom) return;

	bounds.set(0,0,dest_width,dest_height);

	dest.left = x;
	dest.top  = y;

	if (!block_preparecliprects(*src, dest, bounds, src_width, src_height)) return;

	dest_pix += dest.left+(dest.top*dest_width);
	src_pix += src->left+(src->top*src_width);

	long	dest_modulo,src_modulo,w,h;
	omt_RGBAPixel	*last_pix;

	w = dest.get_width();
	h = dest.get_height();
	src_modulo = src_width - w;
	dest_modulo = dest_width - w;

	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && mask==ompixmc_Full)	// No blending
	{
		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix) *(dest_pix++) = *(src_pix++);
			src_pix += src_modulo;
			dest_pix += dest_modulo;
		}
	}
	else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha

		&& mask==ompixmc_Full)	// Optimize transparency
	{
		long a,na;

		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix) 
			{
				a = ((unsigned char*)src_pix)[omd_CGUN_A];
				na = 0xFFL-a;
			
				if (a==0) {dest_pix++; src_pix++;}
				else if (a==0xFF) *(dest_pix++) = *(src_pix++);
				else
				{
					omd_BLITTRANSBLEND(omd_CGUN_R);
					omd_BLITTRANSBLEND(omd_CGUN_G);
					omd_BLITTRANSBLEND(omd_CGUN_B);
					omd_BLITTRANSBLEND(omd_CGUN_A);		
					dest_pix++; src_pix++;					
				}
			}
			src_pix += src_modulo;
			dest_pix += dest_modulo;
		}		
	}
	else		// General blending
	{
		short src_rgba[4];
		short dest_rgba[4],r;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);

		while(h--)
		{
			last_pix = src_pix + w;
			while(src_pix!=last_pix)
			{
				bsrc_func(src_pix, src_pix, dest_pix, src_rgba);
				bdest_func(dest_pix, src_pix, dest_pix, dest_rgba);
			
				if (mask&ompixmc_Red)   omd_BLITBLEND(omd_CGUN_R);

				if (mask&ompixmc_Green) omd_BLITBLEND(omd_CGUN_G);

				if (mask&ompixmc_Blue)  omd_BLITBLEND(omd_CGUN_B);

				if (mask&ompixmc_Alpha) omd_BLITBLEND(omd_CGUN_A);



				
				src_pix++;
				dest_pix++;			
			}
			
			src_pix += src_modulo;
			dest_pix += dest_modulo;
		}
	}	
}


void OMediaBlitter::draw(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							omt_RGBAPixel *dest_pix, long dest_width, long dest_height,
							OMediaRect *src, 
							OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,

							omt_RGBAPixelMask mask)
{
	if (src->get_width()==dest->get_width() && src->get_height()==dest->get_height()) 
	{
		draw(src_pix, src_width, src_height, dest_pix,  dest_width,  dest_height, src, dest->left, dest->top,
				blend_src,blend_dest,mask);
				
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
	dest_pix 	+= dest_bounds.left + (dest_bounds.top * dest_width);

	long			w,h,dest_modulo;
	omt_RGBAPixel	*last_pix,*xpix;

	w = dest_bounds.get_width();
	h = dest_bounds.get_height();

	dest_modulo = dest_width - w;

	px &= 0xFFFFL;
	py &= 0xFFFFL;
	
	dx = omd_FloatToFixed16_16(scalex);
	dy = omd_FloatToFixed16_16(scaley);


	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero && mask==ompixmc_Full)	// No blending
	{
		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				*(dest_pix++) = *(xpix);
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
			
			dest_pix += dest_modulo;
		}
	}
	else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha && 

				mask==ompixmc_Full)	// Optimize transparency
	{
		long a,na;
	
		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				a = ((unsigned char*)xpix)[omd_CGUN_A];
				na = 0xFFL-a;
			
				if (a==0) {dest_pix++;}
				else if (a==0xFF) *(dest_pix++) = *(xpix);
				else
				{
					#define src_pix xpix
					omd_BLITTRANSBLEND(omd_CGUN_R);
					omd_BLITTRANSBLEND(omd_CGUN_G);
					omd_BLITTRANSBLEND(omd_CGUN_B);
					omd_BLITTRANSBLEND(omd_CGUN_A);
					#undef src_pix	
					dest_pix++;				
				}			
				
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
			
			dest_pix += dest_modulo;
		}	
	}
	else		// General
	{
		short src_rgba[4];
		short dest_rgba[4],r;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);

		while(h--)
		{
			xpix = src_pix;
			last_pix = dest_pix + w;
			ix = px;
			while(dest_pix!=last_pix) 
			{
				#define src_pix xpix
				bsrc_func(src_pix, src_pix, dest_pix, src_rgba);
				bdest_func(dest_pix, src_pix, dest_pix, dest_rgba);


				if (mask&ompixmc_Red)   omd_BLITBLEND(omd_CGUN_R);

				if (mask&ompixmc_Green) omd_BLITBLEND(omd_CGUN_G);

				if (mask&ompixmc_Blue)  omd_BLITBLEND(omd_CGUN_B);

				if (mask&ompixmc_Alpha) omd_BLITBLEND(omd_CGUN_A);


				#undef src_pix

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
			
			dest_pix += dest_modulo;
		}	
	}
}

void OMediaBlitter::fill_alpha(	unsigned char alpha,
						 		omt_RGBAPixel *pix, long width, long height,
						 		OMediaRect 	*dest)
{
	OMediaRect		full,bounds;
	omt_RGBAPixel	*p,*pe;
	short			w,h;
	
	full.set(0,0,width,height);

	if (!dest) bounds = full;
	else bounds = *dest;

	if (full.find_intersection(&bounds,&bounds))
	{
		pix += (bounds.top * width) + bounds.left;

		w = bounds.get_width();
		h = bounds.get_height();
		
		p = pix;
		long modulo = width-w;

		while(h--)
		{
			pe = p + w;

			for(;p!=pe;p++) 
			{			
				((unsigned char*)p)[omd_CGUN_A] = alpha;
			}

			p += modulo;
		}

	}
}

void OMediaBlitter::fill(omt_RGBAPixel rgba,
						 omt_RGBAPixel *pix, long width, long height,
						 OMediaRect 		*dest,
						 omt_BlendFunc		blend_src, 
						 omt_BlendFunc		blend_dest,

						 omt_RGBAPixelMask	mask)
{
	OMediaRect		full,bounds;
	omt_RGBAPixel	*p,*pe;
	short			w,h;
	
	full.set(0,0,width,height);

	if (!dest) bounds = full;
	else bounds = *dest;

	if (full.find_intersection(&bounds,&bounds))
	{
		pix += (bounds.top * width) + bounds.left;

		w = bounds.get_width();
		h = bounds.get_height();
		
		p = pix;
		long modulo = width-w;

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero &&

			mask==ompixmc_Full)	// No blending
		{
			while(h--)
			{
				pe = p + w;
	
				for(;p!=pe;p++) 
				{				
					*p = rgba;
				}
	
				p += modulo;
			}
		}
		else if (blend_src==omblendfc_Src_Alpha && blend_dest==omblendfc_Inv_Src_Alpha &&

				mask==ompixmc_Full)	// Optimize transparency
		{
			long a,na,sr,sg,sb,sa;

			a = ((unsigned char*)&rgba)[omd_CGUN_A];
			na = 0xFFL-a;
			sr = a * ((unsigned char*)&rgba)[omd_CGUN_R];
			sg = a * ((unsigned char*)&rgba)[omd_CGUN_G];
			sb = a * ((unsigned char*)&rgba)[omd_CGUN_B];
			sa = a * ((unsigned char*)&rgba)[omd_CGUN_A];
		
			while(h--)
			{
				pe = p + w;
	
				for(;p!=pe;p++) 
				{				
					if (a==0) continue;
					else if (a==0xFF) *p = rgba;
					else
					{
						omd_BLITTRANSBLEND_FILL(sr,omd_CGUN_R);
						omd_BLITTRANSBLEND_FILL(sg,omd_CGUN_G);
						omd_BLITTRANSBLEND_FILL(sb,omd_CGUN_B);
						omd_BLITTRANSBLEND_FILL(sa,omd_CGUN_A);		
					}					
				}
	
				p += modulo;
			}
		}
		else		// General case
		{
			short src_rgba[4];
			short dest_rgba[4],r;

			omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
			omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);

			#define dest_pix ((unsigned char*)p)

			while(h--)
			{
				pe = p + w;
	
				for(;p!=pe;p++) 
				{				
				
					bsrc_func(&rgba, &rgba, p, src_rgba);
					bdest_func(p, &rgba, p, dest_rgba);


					if (mask&ompixmc_Red)   omd_BLITBLEND(omd_CGUN_R);

					if (mask&ompixmc_Green) omd_BLITBLEND(omd_CGUN_G);

					if (mask&ompixmc_Blue)  omd_BLITBLEND(omd_CGUN_B);

					if (mask&ompixmc_Alpha) omd_BLITBLEND(omd_CGUN_A);

				}
	
				p += modulo;
			}		

			#undef dest_pix
		}
	}
}

void OMediaBlitter::draw(	omt_RGBAPixel *src_pix, long src_width, long src_height,
							omt_RGBAPixel *dest_pix, long dest_width, long dest_height,
							OMediaRect *dest,
							omt_BlendFunc	blend_src, omt_BlendFunc	blend_dest,

							omt_RGBAPixelMask	mask)
{
	OMediaRect	src;
	
	src.set(0,0,src_width,src_height);
	draw(src_pix,src_width,src_height,dest_pix,dest_width,dest_height,&src,dest,blend_src,blend_dest,mask);
}


bool OMediaBlitter::block_preparecliprects(	OMediaRect &src, 
										OMediaRect &dest, 
										OMediaRect &bounds,
										long w, long h)
{
	if (src.right<=0 || src.bottom<=0 || src.left>=w || src.top>=h ||
		src.left>=src.right || src.top>=src.bottom) return false;

	if (src.left<0) {dest.left-=src.left;src.left = 0;}
	if (src.top<0) {dest.top-=src.top;src.top = 0;}
	if (src.right>w) src.right = w;
	if (src.bottom>h) src.bottom = h;

	dest.right = dest.left+(src.right - src.left);
	dest.bottom = dest.top+(src.bottom - src.top);

	return block_clip(src, dest, bounds);
}

bool OMediaBlitter::block_clip(OMediaRect &blitsource, 
						 		 		OMediaRect &blitdest, 
						  				OMediaRect &bounds)
{
	if (blitdest.left < bounds.left)
	{
		blitsource.left += bounds.left - blitdest.left;
		blitdest.left = bounds.left;
	}

	if (blitdest.right > bounds.right)
	{
		blitdest.right = bounds.right;
	}

	if (blitdest.top < bounds.top)
	{
		blitsource.top += bounds.top - blitdest.top;
		blitdest.top = bounds.top;
	}

	if (blitdest.bottom > bounds.bottom)
	{
		blitdest.bottom = bounds.bottom;
	}

	if (blitdest.left >= blitdest.right || blitdest.right <= blitdest.left ||
		blitdest.top >= blitdest.bottom || blitdest.bottom <= blitdest.top) return false;
	
	return true;

}

//----------------------------------------------------

// Clip line using Cohen-Sutherland algorithm
const unsigned short omcclip_Left 	=	0x1;
const unsigned short omcclip_Right 	=	0x2;
const unsigned short omcclip_Bottom =	0x4;
const unsigned short omcclip_Top	=	0x8;

static inline unsigned short omfclip_find_code(short x, short y,  OMediaShortRect *clip)
{
	unsigned short code;
	
	if (x<clip->left) code = omcclip_Left;
	else if (x>=clip->right)  code = omcclip_Right;
	else code = 0;
	
	if (y>=clip->bottom) code |= omcclip_Bottom;
	else if (y<clip->top) code |= omcclip_Top;

	return code;
}

bool OMediaBlitter::clip_line(short &x1, short &y1, short &x2, short &y2, OMediaShortRect *clip)
{
	unsigned short code1;
	unsigned short code2, code;
	short x,y,t;

	if (x1>x2)
	{
		t = x1;	x1 = x2; x2 = t;
		t = y1;	y1 = y2; y2 = t;	
	}

	if (!clip) return true;


	code1 = omfclip_find_code(x1,y1,clip);
	code2 = omfclip_find_code(x2,y2,clip);
		
	while(code1 | code2)
	{
		if (code1 & code2) return false;
		
		if (code1) code = code1;
		else code = code2;
	
		if (code&omcclip_Left)
		{
			x = clip->left;
			y = y1 + short(((float)(y2-y1)/(x2-x1)*(clip->left-x1)));
		}
		else if (code&omcclip_Right)
		{
			x = clip->right-1;
			y = y1 + short(((float)(y2-y1)/(x2-x1)*(x-x1)));
		}
		else if (code&omcclip_Bottom)
		{
			y = clip->bottom-1;
			x = x1 + short(((float)(x2-x1)/(y2-y1)*(y-y1)));
		}
		else if (code&omcclip_Top)
		{
			y = clip->top;
			x = x1 + short(((float)(x2-x1)/(y2-y1)*(clip->top-y1)));
		}
	
		if (code==code1) 
		{
			x1 = x;
			y1 = y;
		
			code1 = omfclip_find_code(x1,y1,clip);	
		}
		else
		{
			x2 = x;
			y2 = y;
			
			code2 = omfclip_find_code(x2,y2,clip);
		}
	}
	
	return true;
}

void OMediaBlitter::draw_line(omt_RGBAPixel 	*dest_buffer,
					 			long 			buffer_width,
					 			short x1, 		short y1,
                     			short x2, 		short y2,
                     			omt_RGBAPixel 	color,
                     			OMediaShortRect *clip,
                     			omt_BlendFunc	blend_src, 
								omt_BlendFunc	blend_dest)
{

	short denominator, frac_x,frac_y,i,run,rise,sign_x,sign_y;

	if (!clip_line(x1, y1, x2, y2, clip)) return;

	dest_buffer += (y1*buffer_width) + x1;

	run = x2-x1;
	if (run > 0) sign_x = 1;
	else
	{
		sign_x = -1;
		run = -run;
	}
	
	rise = y2-y1;
	if (rise>0) sign_y = buffer_width;
	else
	{
		sign_y = -buffer_width;
		rise = -rise;
	}
	
	denominator = (rise>run)?rise:run;
	frac_y = frac_x = denominator>>1;

	if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)	// No blending
	{	
		for (i=denominator;i;--i)
		{
			*dest_buffer = color;
			if ( (frac_x+=run) > denominator)
			{
				frac_x -=denominator;
				dest_buffer += sign_x;
			
			}
			
			if ( (frac_y+=rise) > denominator)
			{
				frac_y -=denominator;
				dest_buffer = dest_buffer + sign_y;
			}
		}

		*dest_buffer = color;	
	}
	else		// General blending
	{
		short src_rgba[4];
		short dest_rgba[4],r;

		omt_BlendFunctionPtr bsrc_func = find_blend_func(blend_src);
		omt_BlendFunctionPtr bdest_func = find_blend_func(blend_dest);

		for (i=denominator;i;--i)
		{
			bsrc_func(&color, &color, dest_buffer, src_rgba);
			bdest_func(dest_buffer, &color, dest_buffer, dest_rgba);
			
			#define dest_pix dest_buffer
			omd_BLITBLEND(omd_CGUN_R);
			omd_BLITBLEND(omd_CGUN_G);
			omd_BLITBLEND(omd_CGUN_B);
			omd_BLITBLEND(omd_CGUN_A);
			#undef dest_pix

			if ( (frac_x+=run) > denominator)
			{
				frac_x -=denominator;
				dest_buffer += sign_x;
			}
			
			if ( (frac_y+=rise) > denominator)
			{
				frac_y -=denominator;
				dest_buffer = dest_buffer + sign_y;
			}
		}

		bsrc_func(&color, &color, dest_buffer, src_rgba);
		bdest_func(dest_buffer, &color, dest_buffer, dest_rgba);
		
		#define dest_pix dest_buffer
		omd_BLITBLEND(omd_CGUN_R);
		omd_BLITBLEND(omd_CGUN_G);
		omd_BLITBLEND(omd_CGUN_B);
		omd_BLITBLEND(omd_CGUN_A);
		#undef dest_pix
	}
}

//---------------------------------------------------------------------------
// Raw blitter

void OMediaBlitter::raw_fill(unsigned char *dest, unsigned long lgdata, short n)
{
	unsigned long ldata = lgdata;
	unsigned long *ldest;
	

	short ln;
	short lq;

	if (n>=8)
	{
		if ( ((unsigned long)dest)&3)
		{
			unsigned char cdata1 = ((unsigned char*)(&lgdata))[0];
			unsigned char cdata2 = ((unsigned char*)(&lgdata))[1];
			unsigned char cdata3 = ((unsigned char*)(&lgdata))[2];
			unsigned char cdata4 = ((unsigned char*)(&lgdata))[3];
		
			// Not long aligned

			ln = n>>3;	
			while(ln)
			{
				ln--;
				dest[0] = cdata1;
				dest[1] = cdata2;
				dest[2] = cdata3;
				dest[3] = cdata4;
				dest[4] = cdata1;
				dest[5] = cdata2;
				dest[6] = cdata3;
				dest[7] = cdata4;
				dest+=8;
			}
			
			n  = n&7;		
			if (!n) return;
		}
		else			
		{
			// Long aligned

			ldest = (unsigned long *) dest;

			ln = n>>7;
			lq = (n&0x7F)>>2;

			/*	 NOTE: Same as...
			ln = n/(movesize*32);
			lq = (n%(movesize*32))/movesize;
			n = n%movesize;
			... but faster, movesize is 4.
			*/

			while(ln--)
			{
				ldest[0] = ldata;  ldest[1] = ldata;  ldest[2] = ldata; 
				ldest[3] = ldata;  ldest[4] = ldata;  ldest[5] = ldata;
				ldest[6] = ldata;  ldest[7] = ldata;  ldest[8] = ldata;
				ldest[9] = ldata;  ldest[10] = ldata; ldest[11] = ldata;
				ldest[12] = ldata; ldest[13] = ldata; ldest[14] = ldata;
				ldest[15] = ldata; ldest[16] = ldata; ldest[17] = ldata;
				ldest[18] = ldata; ldest[19] = ldata; ldest[20] = ldata;
				ldest[21] = ldata; ldest[22] = ldata; ldest[23] = ldata;
				ldest[24] = ldata; ldest[25] = ldata; ldest[26] = ldata;
				ldest[27] = ldata; ldest[28] = ldata; ldest[29] = ldata;
				ldest[30] = ldata; ldest[31] = ldata;

				ldest+=32;
			}


			switch(lq)
			{
				case 32: ldest[31] = ldata;	case 31: ldest[30] = ldata;
				case 30: ldest[29] = ldata;	case 29: ldest[28] = ldata;
				case 28: ldest[27] = ldata;	case 27: ldest[26] = ldata;
				case 26: ldest[25] = ldata;	case 25: ldest[24] = ldata;
				case 24: ldest[23] = ldata;	case 23: ldest[22] = ldata;
				case 22: ldest[21] = ldata;	case 21: ldest[20] = ldata;
				case 20: ldest[19] = ldata;	case 19: ldest[18] = ldata;			
				case 18: ldest[17] = ldata;	case 17: ldest[16] = ldata;
				case 16: ldest[15] = ldata;	case 15: ldest[14] = ldata;
				case 14: ldest[13] = ldata;	case 13: ldest[12] = ldata;
				case 12: ldest[11] = ldata;	case 11: ldest[10] = ldata;
				case 10: ldest[9] = ldata;	case 9:	 ldest[8] = ldata;
				case 8:	 ldest[7] = ldata;	case 7:  ldest[6] = ldata;			
				case 6:	 ldest[5] = ldata;	case 5:	 ldest[4] = ldata;
				case 4:  ldest[3] = ldata;	case 3:	 ldest[2] = ldata;
				case 2:	 ldest[1] = ldata;	case 1:  ldest[0] = ldata;
			}
		
			ldest += lq;

			n  = n&0x3;				
			if (!n) return;
			dest = (unsigned char *) ldest;
		}
	}

	for(short i=0; i<n; i++)
	{
		*(dest++) = ((unsigned char*)(&lgdata))[i&3];
	}	
}

//---------------------------------------------------------------------------
// Remap

void OMediaBlitter::remap_to_A1555(	omt_RGBAPixel	*src,
									unsigned short	*dest,
									long			length)
{
	unsigned short	dp;
	unsigned char	*srcp = (unsigned char	*)src;

	while(length--)
	{
		dp = ((*(srcp++))>>3)<<10;
		dp |= ((*(srcp++))>>3)<<5;
		dp |= (*(srcp++))>>3;
		if (*(srcp++)) dp |= 1<<15;

		*(dest++) = dp;
	}
}


