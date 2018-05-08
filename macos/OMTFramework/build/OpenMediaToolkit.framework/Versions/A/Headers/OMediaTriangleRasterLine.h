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
#ifndef OMEDIA_TriangleRasterLine_H
#define OMEDIA_TriangleRasterLine_H

#include "OMediaBlitter.h"
#include "OMediaTriangleSegment.h"
#include "OMediaTextSegmentRasterizer.h"

//---------------------------------------
// * Abstract flat raster line

class OMediaTriangleRasterLine		// Abstract, no depth defined
{
	public:
	
	inline OMediaTriangleRasterLine(char *adest_buffer, 
									   long arowbytes,
									   omt_BlendFunc blend_src, omt_BlendFunc blend_dest)
	{
		buffer_start = adest_buffer;
		rowbytes = arowbytes;
		this->blend_src = blend_src;
		this->blend_dest = blend_dest;
	}
	
	inline void start(short line){dest_buffer = buffer_start + (line*rowbytes);}	
	inline void step(void) {dest_buffer += rowbytes;}

	char			*dest_buffer, *buffer_start;
	long			rowbytes;
	
	omt_BlendFunc	blend_src, blend_dest;
};

//---------------------------------------
// * Abstract ZBuffer raster line

class OMediaTriangleRasterLineZB : public OMediaTriangleRasterLine	
{
	public:
	
	inline OMediaTriangleRasterLineZB(char *adest_buffer,
										    long arowbytes,
										    omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
											unsigned short *zb,
											unsigned long zbmod,
											omt_ZBufferFunc		zfunc,
											omt_ZBufferTest		ztest,
											omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLine(adest_buffer,
																		arowbytes,
																		blend_src,
																		blend_dest)
	{
		zbuffer_start = (unsigned char*)zb;
		zbmodulo = zbmod;
		this->zfunc = zfunc;
		this->ztest = ztest;
		this->zwrite = zwrite;
	}
	
	inline void start(short line)
	{
		OMediaTriangleRasterLine::start(line);
		zbuffer = (unsigned short*)(zbuffer_start + (line*zbmodulo));
	}	
	
	inline void step(void) 
	{
		OMediaTriangleRasterLine::step();
		zbuffer = (unsigned short*)(((char*)zbuffer)+zbmodulo);
	}

	unsigned char		*zbuffer_start;
	unsigned short		*zbuffer;
	unsigned long		zbmodulo;

	omt_ZBufferFunc		zfunc;
	omt_ZBufferTest		ztest;
	omt_ZBufferWrite	zwrite;
};


//---------------------------------------
// * Abstract Gouraud raster line

class OMediaTriangleRasterLine_gouraud : public OMediaTriangleRasterLine 
{
	public:
	
	inline OMediaTriangleRasterLine_gouraud(char *adest_buffer, 
										    long arowbytes,
										    omt_BlendFunc blend_src, omt_BlendFunc blend_dest):
										    OMediaTriangleRasterLine(adest_buffer,
																	 arowbytes,
																	 blend_src,blend_dest) {}
	
	inline void compute_intrgb(omt_PixInterValue delta,
							   const OMediaTriangleGouraudSegment *seg1, 
						   	   const OMediaTriangleGouraudSegment *seg2,
						   	   omt_PixInterValue &ia,
							   omt_PixInterValue &ir, 
							   omt_PixInterValue &ig,
							   omt_PixInterValue &ib) const
	{
		if (delta!=omd_PixInterCast(0)) 
		{
			#ifdef omd_PixelInterpolationFixedPoint
			ia = ((seg2->a-seg1->a)/delta);
			ir = ((seg2->r-seg1->r)/delta);
			ig = ((seg2->g-seg1->g)/delta);
			ib = ((seg2->b-seg1->b)/delta);
			#else
			delta = 1.0/delta;
			ia = ((seg2->a-seg1->a)*delta);
			ir = ((seg2->r-seg1->r)*delta);
			ig = ((seg2->g-seg1->g)*delta);
			ib = ((seg2->b-seg1->b)*delta);
			#endif

		}
		else {ia=ir=ig=ib=omd_PixInterCast(0);}	
	}
};

//---------------------------------------
// * Abstract ZBuffer+Gouraud raster line

class OMediaTriangleRasterLineZB_gouraud : public OMediaTriangleRasterLineZB
{
	public:
	
	inline OMediaTriangleRasterLineZB_gouraud(char 				*adest_buffer, 
										    long 				arowbytes,
										    omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
											unsigned short 		*zb,
											unsigned long 		zbmod,											
											omt_ZBufferFunc		zfunc,
											omt_ZBufferTest		ztest,
											omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB(adest_buffer,
																	 arowbytes,
																	 blend_src, blend_dest,
																	 zb,zbmod,zfunc,ztest,zwrite) {}
	
	inline void compute_intrgb(omt_PixInterValue delta,
							   const OMediaTriangleGouraudSegment *seg1, 
						   	   const OMediaTriangleGouraudSegment *seg2,
						   	   omt_PixInterValue &ia,
							   omt_PixInterValue &ir, 
							   omt_PixInterValue &ig,
							   omt_PixInterValue &ib) const
	{
		if (delta!=omd_PixInterCast(0)) 
		{
			#ifdef omd_PixelInterpolationFixedPoint
			ia = ((seg2->a-seg1->a)/delta);
			ir = ((seg2->r-seg1->r)/delta);
			ig = ((seg2->g-seg1->g)/delta);
			ib = ((seg2->b-seg1->b)/delta);
			#else
			delta = 1.0/delta;
			ia = ((seg2->a-seg1->a)*delta);
			ir = ((seg2->r-seg1->r)*delta);
			ig = ((seg2->g-seg1->g)*delta);
			ib = ((seg2->b-seg1->b)*delta);
			#endif

		}
		else {ia=ir=ig=ib=omd_PixInterCast(0);}	
	}
};


//-----------------------------------------------------------------------------------
// 32 bits

//---------------------------------------
// * Painter, flat, rgb copy

class OMediaTriangleRasterLine_flatcopy_32 : public OMediaTriangleRasterLine
{
	public:
	
	inline OMediaTriangleRasterLine_flatcopy_32(char *adest_buffer, 
										    	long arowbytes,
										    	omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    	unsigned long apattern):
										    	OMediaTriangleRasterLine(adest_buffer,arowbytes,blend_src, blend_dest)
	{
		pattern = apattern;
	}

	inline void rasterline(OMediaTriangleSegment *seg1, OMediaTriangleSegment *seg2, short y)
	{
		long x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);
		
		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
			OMediaBlitter::raw_fill((unsigned char*)dest_buffer+(x1<<2L),pattern,(x2-x1)<<2L);	
		else
			OMediaSegmentRasterizer::fill_color_blend32((unsigned char*)dest_buffer+(x1<<2),
										pattern,
										(x2-x1),blend_src,blend_dest);
    }

	unsigned long pattern;
};

//---------------------------------------
// * Painter, gouraud, rgb copy

class OMediaTriangleRasterLine_gouraudcopy_32 : public OMediaTriangleRasterLine_gouraud
{
	public:
	
	inline OMediaTriangleRasterLine_gouraudcopy_32(char *adest_buffer, 
										    long arowbytes,omt_BlendFunc blend_src, omt_BlendFunc blend_dest):
										    OMediaTriangleRasterLine_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest){}

	inline void rasterline(OMediaTriangleGouraudSegment *seg1, 
						   OMediaTriangleGouraudSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib);

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
			OMediaSegmentRasterizer::fill_color_interpolation32((unsigned char*)dest_buffer+(x1<<2),		
										seg1->r,seg1->g,seg1->b,
										ir,ig,ib,
										(x2-x1));
		else
			OMediaSegmentRasterizer::fill_color_blend_interpolation32((unsigned char*)dest_buffer+(x1<<2),		
										seg1->a,seg1->r,seg1->g,seg1->b,
										ia,ir,ig,ib,
										(x2-x1),blend_src,blend_dest);

    }
};


//---------------------------------------
// * ZBuffer, flat, rgb copy

class OMediaTriangleRasterLineZB_flatcopy_32 : public OMediaTriangleRasterLineZB
{
	public:
	
	inline OMediaTriangleRasterLineZB_flatcopy_32(char *adest_buffer, 
										    	long arowbytes,
										    	omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    	unsigned short *zb,
												unsigned long zbmod,
												omt_ZBufferFunc		zfunc,
												omt_ZBufferTest		ztest,
												omt_ZBufferWrite	zwrite,
										    	unsigned long apattern):
										    	OMediaTriangleRasterLineZB(adest_buffer,
																	 	arowbytes,
																	 	blend_src, blend_dest,
																	 	zb,zbmod,
																	 	zfunc,ztest,zwrite)
	{
		pattern = apattern;
	}

	inline void rasterline(OMediaTriangleZSegment *seg1, OMediaTriangleZSegment *seg2, short y)
	{
		long x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
			OMediaSegmentRasterizer::fill_color_zbuffer32((unsigned long*)dest_buffer+x1,
													    pattern,
														zbuffer+x1,
														seg1->z,
														(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,
														(x2-x1),
														zfunc,
														ztest,
														zwrite);
		else
			OMediaSegmentRasterizer::fill_color_blend_zbuffer32((unsigned long*)dest_buffer+x1,
													    pattern,
														zbuffer+x1,
														seg1->z,
														(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,
														(x2-x1),
														zfunc,
														ztest,
														zwrite,
														blend_src,blend_dest);
    }

	unsigned long pattern;
};


//---------------------------------------
// * ZBuffer, gouraud, rgb copy

class OMediaTriangleRasterLineZB_gouraudcopy_32 : public OMediaTriangleRasterLineZB_gouraud
{
	public:
	
	inline OMediaTriangleRasterLineZB_gouraudcopy_32(char *adest_buffer, 
										    		 long arowbytes,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 unsigned short *zb,
													 unsigned long zbmod,
													omt_ZBufferFunc		zfunc,
													omt_ZBufferTest		ztest,
													omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest,
																	 		 zb,zbmod,zfunc,ztest,zwrite){}

	inline void rasterline(OMediaTriangleZGouraudSegment *seg1, 
						   OMediaTriangleZGouraudSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib);

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
			OMediaSegmentRasterizer::fill_color_interpolation_zb32((unsigned char*)dest_buffer+(x1<<2),		
										seg1->r,seg1->g,seg1->b,		
										ir,ig,ib,
										zbuffer+x1,
										seg1->z,
										(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
										zfunc,
										ztest,
										zwrite,						
									    (x2-x1));		
		else
			OMediaSegmentRasterizer::fill_color_blend_interpolation_zb32((unsigned char*)dest_buffer+(x1<<2),		
										seg1->a, seg1->r,seg1->g,seg1->b,		
										ia,ir,ig,ib,
										zbuffer+x1,
										seg1->z,
										(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
										zfunc,
										ztest,
										zwrite,						
									    (x2-x1),
									    blend_src, blend_dest);
    }
};


//-----------------------------------------------------------------------------------
// 16 bits

//---------------------------------------
// * Painter, flat, rgb copy

class OMediaTriangleRasterLine_flatcopy_16 : public OMediaTriangleRasterLine
{
	public:
	
	inline OMediaTriangleRasterLine_flatcopy_16(char *adest_buffer, 
										    	long arowbytes,
										    	omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    	unsigned long apattern):
										    	OMediaTriangleRasterLine(adest_buffer,arowbytes,blend_src, blend_dest)
	{
		pattern = apattern;
		pattern16 = omd_ARGB32toA16(apattern);
		pattern16 |= pattern16<<16UL;
	}

	inline void rasterline(OMediaTriangleSegment *seg1, OMediaTriangleSegment *seg2, short y)
	{
		long x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);
		
		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
			OMediaBlitter::raw_fill((unsigned char*)dest_buffer+(x1<<1L),pattern16,(x2-x1)<<1L);	
		else
			OMediaSegmentRasterizer::fill_color_blend16((unsigned char*)dest_buffer+(x1<<1),
										pattern,
										(x2-x1),blend_src,blend_dest);
    }

	unsigned long pattern,pattern16;
};

//---------------------------------------
// * Painter, gouraud, rgb copy

class OMediaTriangleRasterLine_gouraudcopy_16 : public OMediaTriangleRasterLine_gouraud
{
	public:
	
	inline OMediaTriangleRasterLine_gouraudcopy_16(char *adest_buffer, 
										    long arowbytes,omt_BlendFunc blend_src, omt_BlendFunc blend_dest):
										    OMediaTriangleRasterLine_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest){}

	inline void rasterline(OMediaTriangleGouraudSegment *seg1, 
						   OMediaTriangleGouraudSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib);

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
			OMediaSegmentRasterizer::fill_color_interpolation16((unsigned char*)dest_buffer+(x1<<1),		
										seg1->r,seg1->g,seg1->b,
										ir,ig,ib,
										(x2-x1));
		else
			OMediaSegmentRasterizer::fill_color_blend_interpolation16((unsigned char*)dest_buffer+(x1<<1),		
										seg1->a,seg1->r,seg1->g,seg1->b,
										ia,ir,ig,ib,
										(x2-x1),blend_src,blend_dest);

    }
};


//---------------------------------------
// * ZBuffer, flat, rgb copy

class OMediaTriangleRasterLineZB_flatcopy_16 : public OMediaTriangleRasterLineZB
{
	public:
	
	inline OMediaTriangleRasterLineZB_flatcopy_16(char *adest_buffer, 
										    	long arowbytes,
										    	omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    	unsigned short *zb,
												unsigned long zbmod,
												omt_ZBufferFunc		zfunc,
												omt_ZBufferTest		ztest,
												omt_ZBufferWrite	zwrite,
										    	unsigned long apattern):
										    	OMediaTriangleRasterLineZB(adest_buffer,
																	 	arowbytes,
																	 	blend_src, blend_dest,
																	 	zb,zbmod,
																	 	zfunc,ztest,zwrite)
	{
		pattern = apattern;
	}

	inline void rasterline(OMediaTriangleZSegment *seg1, OMediaTriangleZSegment *seg2, short y)
	{
		long x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
			OMediaSegmentRasterizer::fill_color_zbuffer16((unsigned short*)dest_buffer+x1,
													    pattern,
														zbuffer+x1,
														seg1->z,
														(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,
														(x2-x1),
														zfunc,
														ztest,
														zwrite);
		else
			OMediaSegmentRasterizer::fill_color_blend_zbuffer16((unsigned short*)dest_buffer+x1,
													    pattern,
														zbuffer+x1,
														seg1->z,
														(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,
														(x2-x1),
														zfunc,
														ztest,
														zwrite,
														blend_src,blend_dest);
    }

	unsigned long pattern;
};


//---------------------------------------
// * ZBuffer, gouraud, rgb copy

class OMediaTriangleRasterLineZB_gouraudcopy_16 : public OMediaTriangleRasterLineZB_gouraud
{
	public:
	
	inline OMediaTriangleRasterLineZB_gouraudcopy_16(char *adest_buffer, 
										    		 long arowbytes,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 unsigned short *zb,
													 unsigned long zbmod,
													omt_ZBufferFunc		zfunc,
													omt_ZBufferTest		ztest,
													omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest,
																	 		 zb,zbmod,zfunc,ztest,zwrite){}

	inline void rasterline(OMediaTriangleZGouraudSegment *seg1, 
						   OMediaTriangleZGouraudSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib);

		if (blend_src==omblendfc_One && blend_dest==omblendfc_Zero)
			OMediaSegmentRasterizer::fill_color_interpolation_zb16((unsigned char*)dest_buffer+(x1<<1),		
										seg1->r,seg1->g,seg1->b,		
										ir,ig,ib,
										zbuffer+x1,
										seg1->z,
										(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
										zfunc,
										ztest,
										zwrite,						
									    (x2-x1));		
		else
			OMediaSegmentRasterizer::fill_color_blend_interpolation_zb16((unsigned char*)dest_buffer+(x1<<1),		
										seg1->a, seg1->r,seg1->g,seg1->b,		
										ia,ir,ig,ib,
										zbuffer+x1,
										seg1->z,
										(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
										zfunc,
										ztest,
										zwrite,						
									    (x2-x1),
									    blend_src, blend_dest);
    }
};



//----------------------------------------------------------------------------------
// TEXTURE 32bits

//---------------------------------------
// * Generic texture, ZBuffer, 32 bits

class OMediaTriangleRasterLineZB_TextGen32 : public OMediaTriangleRasterLineZB_gouraud
{
	public:

	inline void compute_intrgb(omt_PixInterValue delta,
							   const OMediaTriangleGouraudTextureSegment *seg1, 
						   	   const OMediaTriangleGouraudTextureSegment *seg2,
						   	   omt_PixInterValue &ia,
							   omt_PixInterValue &ir, 
							   omt_PixInterValue &ig,
							   omt_PixInterValue &ib,
							   omt_PixInterValue &isr, 
							   omt_PixInterValue &isg,
							   omt_PixInterValue &isb) const
	{
		if (delta!=omd_PixInterCast(0)) 
		{
			#ifdef omd_PixelInterpolationFixedPoint
			ia = ((seg2->a_light-seg1->a_light)/delta);
			ir = ((seg2->r_light-seg1->r_light)/delta);
			ig = ((seg2->g_light-seg1->g_light)/delta);
			ib = ((seg2->b_light-seg1->b_light)/delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)/delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)/delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)/delta);
			
			#else
			
			delta = 1.0/delta;
			ia = ((seg2->a_light-seg1->a_light)*delta);
			ir = ((seg2->r_light-seg1->r_light)*delta);
			ig = ((seg2->g_light-seg1->g_light)*delta);
			ib = ((seg2->b_light-seg1->b_light)*delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)*delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)*delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)*delta);
			#endif
		}
		else {ia=ir=ig=ib=isr=isg=isb=omd_PixInterCast(0);}	
	}

	
	inline OMediaTriangleRasterLineZB_TextGen32(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 unsigned short *zb,
													 unsigned long zbmod,
													omt_ZBufferFunc		zfunc,
													omt_ZBufferTest		ztest,
													omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest,
																	 		 zb,zbmod,zfunc,ztest,zwrite)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
	}

	inline void rasterline(OMediaTriangleZGouraudTextureSegment *seg1, 
						   OMediaTriangleZGouraudTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib,isr,isg,isb;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib,isr,isg,isb);

		OMediaTextSegmentRasterizer::fill_text_generic_zb32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															seg1->a_light, seg1->r_light,seg1->g_light,seg1->b_light,		
															ia,ir,ig,ib,
															seg1->r_hilspec,seg1->g_hilspec,seg1->b_hilspec,		
															isr,isg,isb,
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
};


// Flat shading

class OMediaTriangleRasterLineZB_TextFlat32 : public OMediaTriangleRasterLineZB
{
	public:

	inline OMediaTriangleRasterLineZB_TextFlat32(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 float			da,
										    		 float			dr,
										    		 float			dg,
										    		 float			db,
										    		 float			sr,
										    		 float			sg,
										    		 float			sb,
										    		 unsigned short *zb,
													 unsigned long zbmod,
													omt_ZBufferFunc		zfunc,
													omt_ZBufferTest		ztest,
													omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest,
																	 		 zb,zbmod,zfunc,ztest,zwrite)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
		this->da = (unsigned short)(da * float(0xFF));
		this->dr = (unsigned short)(dr * float(0xFF));
		this->dg = (unsigned short)(dg * float(0xFF));
		this->db = (unsigned short)(db * float(0xFF));
		this->sr = (unsigned short)(sr * float(0xFF));
		this->sg = (unsigned short)(sg * float(0xFF));
		this->sb = (unsigned short)(sb * float(0xFF));		
		
		no_modulation = this->da==0xFF && this->dr==0xFF && this->dg==0xFF && this->db==0xFF &&
						this->sr==0x00 && this->sg==0x00 && this->sb==0x00;
	}

	inline void rasterline(OMediaTriangleZTextureSegment *seg1, 
						   OMediaTriangleZTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		if (no_modulation)
			OMediaTextSegmentRasterizer::fill_text_flat_nomod_zb32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);		
		else
			OMediaTextSegmentRasterizer::fill_text_flat_zb32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															da, dr,dg,db,sr,sg,sb,	
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
	 unsigned short da,dr,dg,db,sr,sg,sb;
	 bool			no_modulation;

};

//-----------------------------------------------------------
// No ZBuffer

class OMediaTriangleRasterLine_TextGen32 : public OMediaTriangleRasterLine_gouraud
{
	public:

	inline void compute_intrgb(omt_PixInterValue delta,
							   const OMediaTriangleGouraudTextureSegment *seg1, 
						   	   const OMediaTriangleGouraudTextureSegment *seg2,
						   	   omt_PixInterValue &ia,
							   omt_PixInterValue &ir, 
							   omt_PixInterValue &ig,
							   omt_PixInterValue &ib,
							   omt_PixInterValue &isr, 
							   omt_PixInterValue &isg,
							   omt_PixInterValue &isb) const
	{
		if (delta!=omd_PixInterCast(0)) 
		{
			#ifdef omd_PixelInterpolationFixedPoint
			ia = ((seg2->a_light-seg1->a_light)/delta);
			ir = ((seg2->r_light-seg1->r_light)/delta);
			ig = ((seg2->g_light-seg1->g_light)/delta);
			ib = ((seg2->b_light-seg1->b_light)/delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)/delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)/delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)/delta);
			
			#else
			
			delta = 1.0/delta;
			ia = ((seg2->a_light-seg1->a_light)*delta);
			ir = ((seg2->r_light-seg1->r_light)*delta);
			ig = ((seg2->g_light-seg1->g_light)*delta);
			ib = ((seg2->b_light-seg1->b_light)*delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)*delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)*delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)*delta);
			#endif
		}
		else {ia=ir=ig=ib=isr=isg=isb=omd_PixInterCast(0);}	
	}

	
	inline OMediaTriangleRasterLine_TextGen32(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest):
										    OMediaTriangleRasterLine_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
	}

	inline void rasterline(OMediaTriangleGouraudTextureSegment *seg1, 
						   OMediaTriangleGouraudTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib,isr,isg,isb;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib,isr,isg,isb);

		OMediaTextSegmentRasterizer::fill_text_generic_32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															seg1->a_light, seg1->r_light,seg1->g_light,seg1->b_light,		
															ia,ir,ig,ib,
															seg1->r_hilspec,seg1->g_hilspec,seg1->b_hilspec,		
															isr,isg,isb,
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
};


// Flat shading

class OMediaTriangleRasterLine_TextFlat32 : public OMediaTriangleRasterLine
{
	public:

	inline OMediaTriangleRasterLine_TextFlat32(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 float			da,
										    		 float			dr,
										    		 float			dg,
										    		 float			db,
										    		 float			sr,
										    		 float			sg,
										    		 float			sb):
										    OMediaTriangleRasterLine(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
		this->da = (unsigned short)(da * float(0xFF));
		this->dr = (unsigned short)(dr * float(0xFF));
		this->dg = (unsigned short)(dg * float(0xFF));
		this->db = (unsigned short)(db * float(0xFF));
		this->sr = (unsigned short)(sr * float(0xFF));
		this->sg = (unsigned short)(sg * float(0xFF));
		this->sb = (unsigned short)(sb * float(0xFF));		
		
		no_modulation = this->da==0xFF && this->dr==0xFF && this->dg==0xFF && this->db==0xFF &&
						this->sr==0x00 && this->sg==0x00 && this->sb==0x00;
	}

	inline void rasterline(OMediaTriangleTextureSegment *seg1, 
						   OMediaTriangleTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		if (no_modulation)
			OMediaTextSegmentRasterizer::fill_text_flat_nomod_32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
														    (x2-x1),
														    blend_src, blend_dest);		
		else
			OMediaTextSegmentRasterizer::fill_text_flat_32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															da, dr,dg,db,sr,sg,sb,	
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
	 unsigned short da,dr,dg,db,sr,sg,sb;
	 bool			no_modulation;

};



//---------------------------------------
// * Generic texture, ZBuffer, 16 bits

class OMediaTriangleRasterLineZB_TextGen16 : public OMediaTriangleRasterLineZB_gouraud
{
	public:

	inline void compute_intrgb(omt_PixInterValue delta,
							   const OMediaTriangleGouraudTextureSegment *seg1, 
						   	   const OMediaTriangleGouraudTextureSegment *seg2,
						   	   omt_PixInterValue &ia,
							   omt_PixInterValue &ir, 
							   omt_PixInterValue &ig,
							   omt_PixInterValue &ib,
							   omt_PixInterValue &isr, 
							   omt_PixInterValue &isg,
							   omt_PixInterValue &isb) const
	{
		if (delta!=omd_PixInterCast(0)) 
		{
			#ifdef omd_PixelInterpolationFixedPoint
			ia = ((seg2->a_light-seg1->a_light)/delta);
			ir = ((seg2->r_light-seg1->r_light)/delta);
			ig = ((seg2->g_light-seg1->g_light)/delta);
			ib = ((seg2->b_light-seg1->b_light)/delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)/delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)/delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)/delta);
			
			#else
			
			delta = 1.0/delta;
			ia = ((seg2->a_light-seg1->a_light)*delta);
			ir = ((seg2->r_light-seg1->r_light)*delta);
			ig = ((seg2->g_light-seg1->g_light)*delta);
			ib = ((seg2->b_light-seg1->b_light)*delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)*delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)*delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)*delta);
			#endif
		}
		else {ia=ir=ig=ib=isr=isg=isb=omd_PixInterCast(0);}	
	}

	
	inline OMediaTriangleRasterLineZB_TextGen16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 unsigned short *zb,
													 unsigned long zbmod,
													omt_ZBufferFunc		zfunc,
													omt_ZBufferTest		ztest,
													omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest,
																	 		 zb,zbmod,zfunc,ztest,zwrite)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
	}

	inline void rasterline(OMediaTriangleZGouraudTextureSegment *seg1, 
						   OMediaTriangleZGouraudTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib,isr,isg,isb;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib,isr,isg,isb);

		OMediaTextSegmentRasterizer::fill_text_generic_zb16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															seg1->a_light, seg1->r_light,seg1->g_light,seg1->b_light,		
															ia,ir,ig,ib,
															seg1->r_hilspec,seg1->g_hilspec,seg1->b_hilspec,		
															isr,isg,isb,
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
};


// Flat shading

class OMediaTriangleRasterLineZB_TextFlat16 : public OMediaTriangleRasterLineZB
{
	public:

	inline OMediaTriangleRasterLineZB_TextFlat16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 float			da,
										    		 float			dr,
										    		 float			dg,
										    		 float			db,
										    		 float			sr,
										    		 float			sg,
										    		 float			sb,
										    		 unsigned short *zb,
													 unsigned long zbmod,
													omt_ZBufferFunc		zfunc,
													omt_ZBufferTest		ztest,
													omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest,
																	 		 zb,zbmod,zfunc,ztest,zwrite)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
		this->da = (unsigned short)(da * float(0xFF));
		this->dr = (unsigned short)(dr * float(0xFF));
		this->dg = (unsigned short)(dg * float(0xFF));
		this->db = (unsigned short)(db * float(0xFF));
		this->sr = (unsigned short)(sr * float(0xFF));
		this->sg = (unsigned short)(sg * float(0xFF));
		this->sb = (unsigned short)(sb * float(0xFF));		
		
		no_modulation = this->da==0xFF && this->dr==0xFF && this->dg==0xFF && this->db==0xFF &&
						this->sr==0x00 && this->sg==0x00 && this->sb==0x00;
	}

	inline void rasterline(OMediaTriangleZTextureSegment *seg1, 
						   OMediaTriangleZTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		if (no_modulation)
			OMediaTextSegmentRasterizer::fill_text_flat_nomod_zb16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);		
		else
			OMediaTextSegmentRasterizer::fill_text_flat_zb16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															da, dr,dg,db,sr,sg,sb,	
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
	 unsigned short da,dr,dg,db,sr,sg,sb;
	 bool			no_modulation;

};

//-----------------------------------------------------------
// No ZBuffer

class OMediaTriangleRasterLine_TextGen16 : public OMediaTriangleRasterLine_gouraud
{
	public:

	inline void compute_intrgb(omt_PixInterValue delta,
							   const OMediaTriangleGouraudTextureSegment *seg1, 
						   	   const OMediaTriangleGouraudTextureSegment *seg2,
						   	   omt_PixInterValue &ia,
							   omt_PixInterValue &ir, 
							   omt_PixInterValue &ig,
							   omt_PixInterValue &ib,
							   omt_PixInterValue &isr, 
							   omt_PixInterValue &isg,
							   omt_PixInterValue &isb) const
	{
		if (delta!=omd_PixInterCast(0)) 
		{
			#ifdef omd_PixelInterpolationFixedPoint
			ia = ((seg2->a_light-seg1->a_light)/delta);
			ir = ((seg2->r_light-seg1->r_light)/delta);
			ig = ((seg2->g_light-seg1->g_light)/delta);
			ib = ((seg2->b_light-seg1->b_light)/delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)/delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)/delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)/delta);
			
			#else
			
			delta = 1.0/delta;
			ia = ((seg2->a_light-seg1->a_light)*delta);
			ir = ((seg2->r_light-seg1->r_light)*delta);
			ig = ((seg2->g_light-seg1->g_light)*delta);
			ib = ((seg2->b_light-seg1->b_light)*delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)*delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)*delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)*delta);
			#endif
		}
		else {ia=ir=ig=ib=isr=isg=isb=omd_PixInterCast(0);}	
	}

	
	inline OMediaTriangleRasterLine_TextGen16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest):
										    OMediaTriangleRasterLine_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
	}

	inline void rasterline(OMediaTriangleGouraudTextureSegment *seg1, 
						   OMediaTriangleGouraudTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib,isr,isg,isb;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib,isr,isg,isb);

		OMediaTextSegmentRasterizer::fill_text_generic_16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															seg1->a_light, seg1->r_light,seg1->g_light,seg1->b_light,		
															ia,ir,ig,ib,
															seg1->r_hilspec,seg1->g_hilspec,seg1->b_hilspec,		
															isr,isg,isb,
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
};


// Flat shading

class OMediaTriangleRasterLine_TextFlat16 : public OMediaTriangleRasterLine
{
	public:

	inline OMediaTriangleRasterLine_TextFlat16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 float			da,
										    		 float			dr,
										    		 float			dg,
										    		 float			db,
										    		 float			sr,
										    		 float			sg,
										    		 float			sb):
										    OMediaTriangleRasterLine(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
		this->da = (unsigned short)(da * float(0xFF));
		this->dr = (unsigned short)(dr * float(0xFF));
		this->dg = (unsigned short)(dg * float(0xFF));
		this->db = (unsigned short)(db * float(0xFF));
		this->sr = (unsigned short)(sr * float(0xFF));
		this->sg = (unsigned short)(sg * float(0xFF));
		this->sb = (unsigned short)(sb * float(0xFF));		
		
		no_modulation = this->da==0xFF && this->dr==0xFF && this->dg==0xFF && this->db==0xFF &&
						this->sr==0x00 && this->sg==0x00 && this->sb==0x00;
	}

	inline void rasterline(OMediaTriangleTextureSegment *seg1, 
						   OMediaTriangleTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		if (no_modulation)
			OMediaTextSegmentRasterizer::fill_text_flat_nomod_16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
														    (x2-x1),
														    blend_src, blend_dest);		
		else
			OMediaTextSegmentRasterizer::fill_text_flat_16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															da, dr,dg,db,sr,sg,sb,	
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
	 unsigned short da,dr,dg,db,sr,sg,sb;
	 bool			no_modulation;

};



//----------------------------------------------------------------------------------
// TEXTURE 16bits

//---------------------------------------
// * Generic texture, ZBuffer, 32 bits

class OMediaTriangleRasterLineZB_TextGen32T16 : public OMediaTriangleRasterLineZB_gouraud
{
	public:

	inline void compute_intrgb(omt_PixInterValue delta,
							   const OMediaTriangleGouraudTextureSegment *seg1, 
						   	   const OMediaTriangleGouraudTextureSegment *seg2,
						   	   omt_PixInterValue &ia,
							   omt_PixInterValue &ir, 
							   omt_PixInterValue &ig,
							   omt_PixInterValue &ib,
							   omt_PixInterValue &isr, 
							   omt_PixInterValue &isg,
							   omt_PixInterValue &isb) const
	{
		if (delta!=omd_PixInterCast(0)) 
		{
			#ifdef omd_PixelInterpolationFixedPoint
			ia = ((seg2->a_light-seg1->a_light)/delta);
			ir = ((seg2->r_light-seg1->r_light)/delta);
			ig = ((seg2->g_light-seg1->g_light)/delta);
			ib = ((seg2->b_light-seg1->b_light)/delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)/delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)/delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)/delta);
			
			#else
			
			delta = 1.0/delta;
			ia = ((seg2->a_light-seg1->a_light)*delta);
			ir = ((seg2->r_light-seg1->r_light)*delta);
			ig = ((seg2->g_light-seg1->g_light)*delta);
			ib = ((seg2->b_light-seg1->b_light)*delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)*delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)*delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)*delta);
			#endif
		}
		else {ia=ir=ig=ib=isr=isg=isb=omd_PixInterCast(0);}	
	}

	
	inline OMediaTriangleRasterLineZB_TextGen32T16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 unsigned short *zb,
													 unsigned long zbmod,
													omt_ZBufferFunc		zfunc,
													omt_ZBufferTest		ztest,
													omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest,
																	 		 zb,zbmod,zfunc,ztest,zwrite)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
	}

	inline void rasterline(OMediaTriangleZGouraudTextureSegment *seg1, 
						   OMediaTriangleZGouraudTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib,isr,isg,isb;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib,isr,isg,isb);

		OMediaTextSegmentRasterizer::t16_fill_text_generic_zb32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															seg1->a_light, seg1->r_light,seg1->g_light,seg1->b_light,		
															ia,ir,ig,ib,
															seg1->r_hilspec,seg1->g_hilspec,seg1->b_hilspec,		
															isr,isg,isb,
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
};


// Flat shading

class OMediaTriangleRasterLineZB_TextFlat32T16 : public OMediaTriangleRasterLineZB
{
	public:

	inline OMediaTriangleRasterLineZB_TextFlat32T16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 float			da,
										    		 float			dr,
										    		 float			dg,
										    		 float			db,
										    		 float			sr,
										    		 float			sg,
										    		 float			sb,
										    		 unsigned short *zb,
													 unsigned long zbmod,
													omt_ZBufferFunc		zfunc,
													omt_ZBufferTest		ztest,
													omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest,
																	 		 zb,zbmod,zfunc,ztest,zwrite)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
		this->da = (unsigned short)(da * float(0xFF));
		this->dr = (unsigned short)(dr * float(0xFF));
		this->dg = (unsigned short)(dg * float(0xFF));
		this->db = (unsigned short)(db * float(0xFF));
		this->sr = (unsigned short)(sr * float(0xFF));
		this->sg = (unsigned short)(sg * float(0xFF));
		this->sb = (unsigned short)(sb * float(0xFF));		
		
		no_modulation = this->da==0xFF && this->dr==0xFF && this->dg==0xFF && this->db==0xFF &&
						this->sr==0x00 && this->sg==0x00 && this->sb==0x00;
	}

	inline void rasterline(OMediaTriangleZTextureSegment *seg1, 
						   OMediaTriangleZTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		if (no_modulation)
			OMediaTextSegmentRasterizer::t16_fill_text_flat_nomod_zb32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);		
		else
			OMediaTextSegmentRasterizer::t16_fill_text_flat_zb32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															da, dr,dg,db,sr,sg,sb,	
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
	 unsigned short da,dr,dg,db,sr,sg,sb;
	 bool			no_modulation;

};

//-----------------------------------------------------------
// No ZBuffer

class OMediaTriangleRasterLine_TextGen32T16 : public OMediaTriangleRasterLine_gouraud
{
	public:

	inline void compute_intrgb(omt_PixInterValue delta,
							   const OMediaTriangleGouraudTextureSegment *seg1, 
						   	   const OMediaTriangleGouraudTextureSegment *seg2,
						   	   omt_PixInterValue &ia,
							   omt_PixInterValue &ir, 
							   omt_PixInterValue &ig,
							   omt_PixInterValue &ib,
							   omt_PixInterValue &isr, 
							   omt_PixInterValue &isg,
							   omt_PixInterValue &isb) const
	{
		if (delta!=omd_PixInterCast(0)) 
		{
			#ifdef omd_PixelInterpolationFixedPoint
			ia = ((seg2->a_light-seg1->a_light)/delta);
			ir = ((seg2->r_light-seg1->r_light)/delta);
			ig = ((seg2->g_light-seg1->g_light)/delta);
			ib = ((seg2->b_light-seg1->b_light)/delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)/delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)/delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)/delta);
			
			#else
			
			delta = 1.0/delta;
			ia = ((seg2->a_light-seg1->a_light)*delta);
			ir = ((seg2->r_light-seg1->r_light)*delta);
			ig = ((seg2->g_light-seg1->g_light)*delta);
			ib = ((seg2->b_light-seg1->b_light)*delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)*delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)*delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)*delta);
			#endif
		}
		else {ia=ir=ig=ib=isr=isg=isb=omd_PixInterCast(0);}	
	}

	
	inline OMediaTriangleRasterLine_TextGen32T16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest):
										    OMediaTriangleRasterLine_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
	}

	inline void rasterline(OMediaTriangleGouraudTextureSegment *seg1, 
						   OMediaTriangleGouraudTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib,isr,isg,isb;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib,isr,isg,isb);

		OMediaTextSegmentRasterizer::t16_fill_text_generic_32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															seg1->a_light, seg1->r_light,seg1->g_light,seg1->b_light,		
															ia,ir,ig,ib,
															seg1->r_hilspec,seg1->g_hilspec,seg1->b_hilspec,		
															isr,isg,isb,
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
};


// Flat shading

class OMediaTriangleRasterLine_TextFlat32T16 : public OMediaTriangleRasterLine
{
	public:

	inline OMediaTriangleRasterLine_TextFlat32T16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 float			da,
										    		 float			dr,
										    		 float			dg,
										    		 float			db,
										    		 float			sr,
										    		 float			sg,
										    		 float			sb):
										    OMediaTriangleRasterLine(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
		this->da = (unsigned short)(da * float(0xFF));
		this->dr = (unsigned short)(dr * float(0xFF));
		this->dg = (unsigned short)(dg * float(0xFF));
		this->db = (unsigned short)(db * float(0xFF));
		this->sr = (unsigned short)(sr * float(0xFF));
		this->sg = (unsigned short)(sg * float(0xFF));
		this->sb = (unsigned short)(sb * float(0xFF));		
		
		no_modulation = this->da==0xFF && this->dr==0xFF && this->dg==0xFF && this->db==0xFF &&
						this->sr==0x00 && this->sg==0x00 && this->sb==0x00;
	}

	inline void rasterline(OMediaTriangleTextureSegment *seg1, 
						   OMediaTriangleTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		if (no_modulation)
			OMediaTextSegmentRasterizer::t16_fill_text_flat_nomod_32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
														    (x2-x1),
														    blend_src, blend_dest);		
		else
			OMediaTextSegmentRasterizer::t16_fill_text_flat_32(((unsigned long*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															da, dr,dg,db,sr,sg,sb,	
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
	 unsigned short da,dr,dg,db,sr,sg,sb;
	 bool			no_modulation;

};



//---------------------------------------
// * Generic texture, ZBuffer, 16 bits

class OMediaTriangleRasterLineZB_TextGen16T16 : public OMediaTriangleRasterLineZB_gouraud
{
	public:

	inline void compute_intrgb(omt_PixInterValue delta,
							   const OMediaTriangleGouraudTextureSegment *seg1, 
						   	   const OMediaTriangleGouraudTextureSegment *seg2,
						   	   omt_PixInterValue &ia,
							   omt_PixInterValue &ir, 
							   omt_PixInterValue &ig,
							   omt_PixInterValue &ib,
							   omt_PixInterValue &isr, 
							   omt_PixInterValue &isg,
							   omt_PixInterValue &isb) const
	{
		if (delta!=omd_PixInterCast(0)) 
		{
			#ifdef omd_PixelInterpolationFixedPoint
			ia = ((seg2->a_light-seg1->a_light)/delta);
			ir = ((seg2->r_light-seg1->r_light)/delta);
			ig = ((seg2->g_light-seg1->g_light)/delta);
			ib = ((seg2->b_light-seg1->b_light)/delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)/delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)/delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)/delta);
			
			#else
			
			delta = 1.0/delta;
			ia = ((seg2->a_light-seg1->a_light)*delta);
			ir = ((seg2->r_light-seg1->r_light)*delta);
			ig = ((seg2->g_light-seg1->g_light)*delta);
			ib = ((seg2->b_light-seg1->b_light)*delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)*delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)*delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)*delta);
			#endif
		}
		else {ia=ir=ig=ib=isr=isg=isb=omd_PixInterCast(0);}	
	}

	
	inline OMediaTriangleRasterLineZB_TextGen16T16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 unsigned short *zb,
													 unsigned long zbmod,
													omt_ZBufferFunc		zfunc,
													omt_ZBufferTest		ztest,
													omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest,
																	 		 zb,zbmod,zfunc,ztest,zwrite)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
	}

	inline void rasterline(OMediaTriangleZGouraudTextureSegment *seg1, 
						   OMediaTriangleZGouraudTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib,isr,isg,isb;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib,isr,isg,isb);

		OMediaTextSegmentRasterizer::t16_fill_text_generic_zb16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															seg1->a_light, seg1->r_light,seg1->g_light,seg1->b_light,		
															ia,ir,ig,ib,
															seg1->r_hilspec,seg1->g_hilspec,seg1->b_hilspec,		
															isr,isg,isb,
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
};


// Flat shading

class OMediaTriangleRasterLineZB_TextFlat16T16 : public OMediaTriangleRasterLineZB
{
	public:

	inline OMediaTriangleRasterLineZB_TextFlat16T16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 float			da,
										    		 float			dr,
										    		 float			dg,
										    		 float			db,
										    		 float			sr,
										    		 float			sg,
										    		 float			sb,
										    		 unsigned short *zb,
													 unsigned long zbmod,
													omt_ZBufferFunc		zfunc,
													omt_ZBufferTest		ztest,
													omt_ZBufferWrite	zwrite):
										    OMediaTriangleRasterLineZB(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest,
																	 		 zb,zbmod,zfunc,ztest,zwrite)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
		this->da = (unsigned short)(da * float(0xFF));
		this->dr = (unsigned short)(dr * float(0xFF));
		this->dg = (unsigned short)(dg * float(0xFF));
		this->db = (unsigned short)(db * float(0xFF));
		this->sr = (unsigned short)(sr * float(0xFF));
		this->sg = (unsigned short)(sg * float(0xFF));
		this->sb = (unsigned short)(sb * float(0xFF));		
		
		no_modulation = this->da==0xFF && this->dr==0xFF && this->dg==0xFF && this->db==0xFF &&
						this->sr==0x00 && this->sg==0x00 && this->sb==0x00;
	}

	inline void rasterline(OMediaTriangleZTextureSegment *seg1, 
						   OMediaTriangleZTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		if (no_modulation)
			OMediaTextSegmentRasterizer::t16_fill_text_flat_nomod_zb16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);		
		else
			OMediaTextSegmentRasterizer::t16_fill_text_flat_zb16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															da, dr,dg,db,sr,sg,sb,	
															zbuffer+x1,
															seg1->z,
															(x1!=x2)?((seg2->z)-(seg1->z))/(x2-x1):0,					
															zfunc,
															ztest,
															zwrite,						
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
	 unsigned short da,dr,dg,db,sr,sg,sb;
	 bool			no_modulation;

};

//-----------------------------------------------------------
// No ZBuffer

class OMediaTriangleRasterLine_TextGen16T16 : public OMediaTriangleRasterLine_gouraud
{
	public:

	inline void compute_intrgb(omt_PixInterValue delta,
							   const OMediaTriangleGouraudTextureSegment *seg1, 
						   	   const OMediaTriangleGouraudTextureSegment *seg2,
						   	   omt_PixInterValue &ia,
							   omt_PixInterValue &ir, 
							   omt_PixInterValue &ig,
							   omt_PixInterValue &ib,
							   omt_PixInterValue &isr, 
							   omt_PixInterValue &isg,
							   omt_PixInterValue &isb) const
	{
		if (delta!=omd_PixInterCast(0)) 
		{
			#ifdef omd_PixelInterpolationFixedPoint
			ia = ((seg2->a_light-seg1->a_light)/delta);
			ir = ((seg2->r_light-seg1->r_light)/delta);
			ig = ((seg2->g_light-seg1->g_light)/delta);
			ib = ((seg2->b_light-seg1->b_light)/delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)/delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)/delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)/delta);
			
			#else
			
			delta = 1.0/delta;
			ia = ((seg2->a_light-seg1->a_light)*delta);
			ir = ((seg2->r_light-seg1->r_light)*delta);
			ig = ((seg2->g_light-seg1->g_light)*delta);
			ib = ((seg2->b_light-seg1->b_light)*delta);
			isr = ((seg2->r_hilspec-seg1->r_hilspec)*delta);
			isg = ((seg2->g_hilspec-seg1->g_hilspec)*delta);
			isb = ((seg2->b_hilspec-seg1->b_hilspec)*delta);
			#endif
		}
		else {ia=ir=ig=ib=isr=isg=isb=omd_PixInterCast(0);}	
	}

	
	inline OMediaTriangleRasterLine_TextGen16T16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest):
										    OMediaTriangleRasterLine_gouraud(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
	}

	inline void rasterline(OMediaTriangleGouraudTextureSegment *seg1, 
						   OMediaTriangleGouraudTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;
		omt_PixInterValue	delta,ia,ir,ig,ib,isr,isg,isb;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		delta = omd_PixInterCast(x2 - x1);
		compute_intrgb(delta,seg1,seg2,ia,ir,ig,ib,isr,isg,isb);

		OMediaTextSegmentRasterizer::t16_fill_text_generic_16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															seg1->a_light, seg1->r_light,seg1->g_light,seg1->b_light,		
															ia,ir,ig,ib,
															seg1->r_hilspec,seg1->g_hilspec,seg1->b_hilspec,		
															isr,isg,isb,
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
};


// Flat shading

class OMediaTriangleRasterLine_TextFlat16T16 : public OMediaTriangleRasterLine
{
	public:

	inline OMediaTriangleRasterLine_TextFlat16T16(char *adest_buffer, 
										    		 long arowbytes,
										    		 unsigned char *text,
													 unsigned long text_maskx,
													 unsigned long text_masky,
						 							 unsigned long text_rowbytes_shifter,
										    		 omt_BlendFunc blend_src, omt_BlendFunc blend_dest,
										    		 float			da,
										    		 float			dr,
										    		 float			dg,
										    		 float			db,
										    		 float			sr,
										    		 float			sg,
										    		 float			sb):
										    OMediaTriangleRasterLine(adest_buffer,
																	 		 arowbytes,
																	 		 blend_src, blend_dest)
	{
		this->text = text;
		this->text_maskx = text_maskx;
		this->text_masky = text_masky;
		this->text_rowbytes_shifter = text_rowbytes_shifter;
		this->da = (unsigned short)(da * float(0xFF));
		this->dr = (unsigned short)(dr * float(0xFF));
		this->dg = (unsigned short)(dg * float(0xFF));
		this->db = (unsigned short)(db * float(0xFF));
		this->sr = (unsigned short)(sr * float(0xFF));
		this->sg = (unsigned short)(sg * float(0xFF));
		this->sb = (unsigned short)(sb * float(0xFF));		
		
		no_modulation = this->da==0xFF && this->dr==0xFF && this->dg==0xFF && this->db==0xFF &&
						this->sr==0x00 && this->sg==0x00 && this->sb==0x00;
	}

	inline void rasterline(OMediaTriangleTextureSegment *seg1, 
						   OMediaTriangleTextureSegment *seg2, 
						   short y)
	{
		long 				x1,x2;

		x1 = long(seg1->x);
		x2 = long(seg2->x);

		if (no_modulation)
			OMediaTextSegmentRasterizer::t16_fill_text_flat_nomod_16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
														    (x2-x1),
														    blend_src, blend_dest);		
		else
			OMediaTextSegmentRasterizer::t16_fill_text_flat_16(((unsigned short*)dest_buffer)+x1,
															text,text_maskx,text_masky,text_rowbytes_shifter,
															seg1->u,seg1->v,seg1->inv_w,
															seg2->u,seg2->v,seg2->inv_w,
															da, dr,dg,db,sr,sg,sb,	
														    (x2-x1),
														    blend_src, blend_dest);															
    }

	 unsigned char *text;
	 unsigned long text_maskx;
	 unsigned long text_masky;
	 unsigned long text_rowbytes_shifter;
	 unsigned short da,dr,dg,db,sr,sg,sb;
	 bool			no_modulation;

};



#endif

