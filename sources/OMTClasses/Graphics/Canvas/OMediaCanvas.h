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
#ifndef OMEDIA_Canvas_H
#define OMEDIA_Canvas_H

#include "OMediaTypes.h"
#include "OMediaRect.h"
#include "OMediaDBObject.h"
#include "OMediaPixelFormat.h"
#include "OMediaEngineImplementation.h"
#include "OMediaMoveableMem.h"
#include "OMediaRGBColor.h"
#include "OMediaRendererInterface.h"
#include "OMediaDrawInterface.h"


class OMediaCanvas : public OMediaDBObject,
					 public OMediaEngineImpMaster,
					 public OMediaDrawInterface
{
	public:

	// * Constructor/Destructor

	omtshared OMediaCanvas();
	omtshared virtual ~OMediaCanvas();


	// * Create canvas

	omtshared virtual void create(long 				width,
								  long 				height,
								  OMediaCanvas		*source = NULL);	// Source is scaled if required

	omtshared virtual void purge(void);
	
	
	// * Canvas size

	inline long get_width(void) const {return width;}
	inline long get_height(void) const {return height;}


	// * Canvas to canvas copy

		// Please note that in OMT 2.x, "this" is the destination of the draw: dest->draw(src...)

	omtshared virtual void draw_full(OMediaCanvas *csrc, long destx, long desty, omt_BlendFunc blend_src =omblendfc_One,  omt_BlendFunc blend_dest =omblendfc_Zero, omt_RGBAPixelMask	pixmask = ompixmc_Full);
	omtshared virtual void draw(OMediaCanvas *csrc, OMediaRect *src, long x, long y, omt_BlendFunc blend_src=omblendfc_One,  omt_BlendFunc blend_dest=omblendfc_Zero, omt_RGBAPixelMask	pixmask = ompixmc_Full);
	omtshared virtual void draw(OMediaCanvas *csrc, OMediaRect *src, OMediaRect *dest, omt_BlendFunc blend_src=omblendfc_One,  omt_BlendFunc blend_dest=omblendfc_Zero, omt_RGBAPixelMask	pixmask = ompixmc_Full);
	omtshared virtual void draw(OMediaCanvas *csrc, OMediaRect *dest, omt_BlendFunc blend_src=omblendfc_One,  omt_BlendFunc blend_dest=omblendfc_Zero, omt_RGBAPixelMask pixmask = ompixmc_Full);

	// * Fill canvas

	omtshared virtual void fill(OMediaARGBColor &argb, OMediaRect *dest =NULL, omt_BlendFunc blend_src =omblendfc_One,  omt_BlendFunc	blend_dest =omblendfc_Zero, omt_RGBAPixelMask	pixmask = ompixmc_Full);
	omtshared virtual void fill_alpha(unsigned char pixalpha, OMediaRect *dest =NULL);	// alpha is 0-0xFF

	inline void fill(OMediaFARGBColor &argb_f, OMediaRect *dest =NULL, omt_BlendFunc blend_src =omblendfc_One,  omt_BlendFunc	blend_dest =omblendfc_Zero, omt_RGBAPixelMask	pixmask = ompixmc_Full)
	{
		OMediaARGBColor	argb;	argb.fset(argb_f.alpha,argb_f.red,argb_f.green,argb_f.blue);	
		fill(argb, dest, blend_src,blend_dest,pixmask);
	}

	// * Draw line

	omtshared virtual void draw_line(OMediaARGBColor 	&argb,
									long x1, 		long y1,
    	                 			long x2, 		long y2,
        	             			omt_BlendFunc	blend_src =omblendfc_One, 
									omt_BlendFunc	blend_dest =omblendfc_Zero);

	inline void draw_line(OMediaFARGBColor 	&argb_f,
								long x1, 		long y1,
                     			long x2, 		long y2,
                     			omt_BlendFunc	blend_src =omblendfc_One, 
								omt_BlendFunc	blend_dest =omblendfc_Zero)
	{
		OMediaARGBColor	argb;	argb.fset(argb_f.alpha,argb_f.red,argb_f.green,argb_f.blue);	
		draw_line(argb, x1, y1, x2, y2, blend_src, blend_dest);
	}	
	
	// * Draw frame/box
	
	
		// Filled rectangle (ARGB floating points)
		
	inline void paint_rect(OMediaFARGBColor &fargb, long x1, long y1, long x2, long y2,
							omt_BlendFunc blend_src =omblendfc_One,  
							omt_BlendFunc blend_dest =omblendfc_Zero)
	{
		OMediaRect dest(x1,y1,x2,y2);
		fill(fargb, &dest, blend_src,  blend_dest);
	}
	
	inline void paint_rect(OMediaFARGBColor &fargb, OMediaRect &rect, 
							omt_BlendFunc blend_src =omblendfc_One,  
							omt_BlendFunc blend_dest =omblendfc_Zero) 
	{
		fill(fargb, &rect, blend_src,  blend_dest,ompixmc_Full);
	}
	
		// Filled rectangle (ARGB integers)

	inline void paint_rect(OMediaARGBColor &fargb, long x1, long y1, long x2, long y2,
							omt_BlendFunc blend_src =omblendfc_One,  
							omt_BlendFunc blend_dest =omblendfc_Zero)
	{
		OMediaRect dest(x1,y1,x2,y2);
		fill(fargb, &dest, blend_src,  blend_dest,ompixmc_Full);
	}
	
	inline void paint_rect(OMediaARGBColor &fargb,OMediaRect &rect, 
							omt_BlendFunc blend_src =omblendfc_One,  
							omt_BlendFunc blend_dest =omblendfc_Zero) 
	{
		fill(fargb, &rect, blend_src,  blend_dest);
	}

	
	
		// Frame (ARGB floating points)

	inline void frame_rect(OMediaFARGBColor &fargb, OMediaRect &rect, 
							omt_BlendFunc blend_src =omblendfc_One,  
							omt_BlendFunc blend_dest =omblendfc_Zero)
	{
		OMediaARGBColor	argb;	argb.fset(fargb.alpha,fargb.red,fargb.green,fargb.blue);	
		frame_rect(argb, rect, blend_src, blend_dest);
	}
		
	inline void frame_rect(OMediaFARGBColor &fargb, long x1, long y1, long x2, long y2,
							omt_BlendFunc blend_src =omblendfc_One,
							omt_BlendFunc blend_dest =omblendfc_Zero)
	{
		OMediaRect	rect(x1,y1,x2,y2);
		frame_rect(fargb, rect, blend_src,blend_dest);
	}
		// Frame (ARGB integer)

	omtshared virtual void frame_rect(OMediaARGBColor &fargb,  OMediaRect &rect, 
							omt_BlendFunc blend_src =omblendfc_One,  
							omt_BlendFunc blend_dest =omblendfc_Zero);
	
	inline void frame_rect(OMediaARGBColor &fargb, long x1, long y1, long x2, long y2,
							omt_BlendFunc blend_src =omblendfc_One,
							omt_BlendFunc blend_dest =omblendfc_Zero)
	{
		OMediaRect	rect(x1,y1,x2,y2);
		frame_rect(fargb, rect, blend_src, blend_dest);
	}
	
		// Emboss (ARGB floating points)

	omtshared virtual void paint_emboss(OMediaRect &rect, bool out, 
							OMediaFARGBColor &dark, 
							OMediaFARGBColor &shine, 
							OMediaFARGBColor &fill,
							omt_BlendFunc blend_src =omblendfc_One,
							omt_BlendFunc blend_dest =omblendfc_Zero);

	omtshared virtual void frame_emboss(OMediaRect &rect, bool out, 
							OMediaFARGBColor &dark, 
							OMediaFARGBColor &shine, 
							omt_BlendFunc blend_src =omblendfc_One,
							omt_BlendFunc blend_dest =omblendfc_Zero);

		// Emboss (ARGB integers)

	omtshared virtual void paint_emboss(OMediaRect &rect, bool out, 
							OMediaARGBColor &dark, 
							OMediaARGBColor &shine, 
							OMediaARGBColor &fill,
							omt_BlendFunc blend_src =omblendfc_One,
							omt_BlendFunc blend_dest =omblendfc_Zero);

	omtshared virtual void frame_emboss(OMediaRect &rect, bool out, 
							OMediaARGBColor &dark, 
							OMediaARGBColor &shine, 
							omt_BlendFunc blend_src =omblendfc_One,
							omt_BlendFunc blend_dest =omblendfc_Zero);
	

	// * Draw string
	
	inline void draw_string(string str, long dx, long dy,
										OMediaFont		*font,
										omt_BlendFunc 	blend_src =omblendfc_One,  
										omt_BlendFunc 	blend_dest =omblendfc_Zero)
	{
		font->draw_string(str, this, dx,dy,  blend_src, blend_dest);
	}
	


	// * Read write pixels (canvas must be locked)
	
	inline void read_pixel(omt_RGBAPixel &pix, long x, long y) const {pix = pixdata[(y*width) + x];}
	inline void write_pixel(omt_RGBAPixel pix, long x, long y) {pixdata[(y*width) + x] = pix;}

	inline void read_pixel(OMediaARGBColor &pix, long x, long y) const 
	{

		unsigned long p = pixdata[(y*width) + x];
		pix.set_rgba(omd_IfLittleEndianReverseLong(p));
	}

	inline void write_pixel(OMediaARGBColor pix, long x, long y)
	{
		unsigned long p = pix.get_rgba();

		pixdata[(y*width) + x] = omd_IfLittleEndianReverseLong(p);
	}

	inline void read_pixel(OMediaFARGBColor &pix, long x, long y) const 
	{
		unsigned long p = pixdata[(y*width) + x];

		pix.set_rgba(omd_IfLittleEndianReverseLong(p));
	}

	inline void write_pixel(OMediaFARGBColor pix, long x, long y)
	{

		unsigned long p = pix.get_rgba();
		pixdata[(y*width) + x] = omd_IfLittleEndianReverseLong(p);
	}


	// * Lock the pixels

	omtshared virtual void lock(omt_LockFlags flags);
	omtshared virtual void unlock(void);

	inline omt_RGBAPixel *get_pixels(void) {return pixdata;} // Returns zero if it's not locked
	
	// * Best internal pixel format
	
	omtshared virtual void set_internal_pixel_format(const omt_PixelFormat pixformat);
	inline omt_PixelFormat get_internal_pixel_format(void) const {return internal_pixel_format;}

	// * Filtering
	
	inline void set_filtering(	omt_CanvasFiltering mag_f,
								omt_CanvasFiltering min_f) {min_filtering = min_f; mag_filtering = mag_f; delete_imp_slaves();} 

	inline omt_CanvasFiltering get_filtering_mag(void) const {return mag_filtering; } 
	inline omt_CanvasFiltering get_filtering_min(void) const {return min_filtering; }


	// * Database/streamer support
	
	enum { db_type = 'Canv' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void) const;


	// * PNG exporting

	omtshared virtual void png_export(OMediaStreamOperators &stream);



	// * Free image memory. Memory is reallocated
	// when it is locked (but the contain is lost). Implementations
	// are not deleted.

	omtshared virtual void free_master_memory(void);
	
	// * VRam subdivision for 2D surface

	// By default OMT uses the biggest texture size to subdivise 2D surface.
	// This can be changed by using the following values (must be a power of 2)
	// Set to -1 to reset default values.
	
	omtshared virtual void set_2Dsubdivision(long w, long h);
	inline long get_2Dsubdivision_width(void) const {return subdiv_w;}
	inline long get_2Dsubdivision_height(void) const {return subdiv_h;}
	
	

	//****************

	protected:

	omtshared void init_retarget(void);

	long					width,height;
	long					subdiv_w,subdiv_h;
	OMediaMoveableMem		pixels;
	omt_RGBAPixel			*pixdata;
	omt_PixelFormat			internal_pixel_format;
	omt_CanvasFiltering		min_filtering,mag_filtering;
	bool					memory_purged;
};


#endif

