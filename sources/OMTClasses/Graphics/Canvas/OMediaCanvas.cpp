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

#include "OMediaCanvas.h"
#include "OMediaCanvasConverter.h"
#include "OMediaError.h"
#include "OMediaBlitter.h"
#include "OMediaMemStream.h"
#include "OMediaCRC.h"
#include "OMediaCompression.h"

OMediaCanvas::OMediaCanvas(void) 
{
	width = height = 0;
	pixdata = NULL;
	internal_pixel_format = ompixfc_Best;
	compression_level = omclc_DefaultCompression;
	min_filtering = mag_filtering = omtfc_Nearest;
	memory_purged = false;
	subdiv_w = subdiv_h = -1;
}

OMediaCanvas::~OMediaCanvas(void)
{
	db_update();
	purge();
}

unsigned long OMediaCanvas::db_get_type(void) const
{
	return OMediaCanvas::db_type;
}

void OMediaCanvas::create(	long 				w,
							long 				h,
							OMediaCanvas		*source)
{
	purge();

	// Width/height
	
	width = w;
	height = h;	

	// Allocate pixels

	pixels.setsize((width*height)<<2L);

	if (source)
	{
		OMediaRect	rsrc,rdest;
		rsrc.set(0,0,source->get_width(),source->get_height());
		rdest.set(0,0,width,height);
		draw(source,&rsrc,&rdest);	
	}
}


void OMediaCanvas::purge(void)
{
	delete_imp_slaves();
	pixels.setsize(0);
	width = height = 0;
	pixdata = NULL;
	memory_purged = false;
}

void OMediaCanvas::lock(omt_LockFlags flags)
{
	OMediaEngineImpMaster::lock(flags);
	if (lock_count==1) 
	{
		if (memory_purged)
		{
			pixels.setsize((width*height)<<2L);
			memory_purged = false;
		}

		pixdata = (omt_RGBAPixel*)pixels.lock();
	}
}

void OMediaCanvas::unlock(void)
{
	OMediaEngineImpMaster::unlock();
	if (lock_count==0) 
	{
		pixdata = NULL;
		pixels.unlock();
	}
}

//----------------------------------------------------------

void OMediaCanvas::set_internal_pixel_format(const omt_PixelFormat pixformat)
{
	if (pixformat!=internal_pixel_format)
	{
		internal_pixel_format = pixformat;
		delete_imp_slaves();
	}
}

//----------------------------------------------------------


void OMediaCanvas::draw_full(OMediaCanvas *wsrc, long x, long y,
								omt_BlendFunc	blend_src, 
								omt_BlendFunc	blend_dest,
								omt_RGBAPixelMask	mask)
{
	wsrc->lock(omlf_Read);
	lock(omlf_Write);
	
	OMediaBlitter::draw_full(wsrc->get_pixels(), wsrc->get_width(), wsrc->get_height(),
							 get_pixels(), get_width(), get_height(),
							 x, y, blend_src, blend_dest, mask);

	wsrc->unlock();
	unlock();
}

void OMediaCanvas::draw(OMediaCanvas *wsrc, OMediaRect *src, long x, long y,
								omt_BlendFunc	blend_src, 
								omt_BlendFunc	blend_dest,
								omt_RGBAPixelMask	mask)
{
	wsrc->lock(omlf_Read);
	lock(omlf_Write);
	
	OMediaBlitter::draw(wsrc->get_pixels(), wsrc->get_width(), wsrc->get_height(),
						get_pixels(), get_width(), get_height(),
						src, x, y, blend_src, blend_dest,mask);

	wsrc->unlock();
	unlock();
}

void OMediaCanvas::draw(OMediaCanvas *wsrc, OMediaRect *src, OMediaRect *dest,
								omt_BlendFunc	blend_src, 
								omt_BlendFunc	blend_dest,
								omt_RGBAPixelMask	mask)
{
	wsrc->lock(omlf_Read);
	lock(omlf_Write);
	
	OMediaBlitter::draw(wsrc->get_pixels(), wsrc->get_width(), wsrc->get_height(),
						get_pixels(), get_width(), get_height(),
						src, dest, blend_src, blend_dest,mask);

	wsrc->unlock();
	unlock();
}

void OMediaCanvas::draw(OMediaCanvas *wsrc, OMediaRect *dest,
								omt_BlendFunc	blend_src, 
								omt_BlendFunc	blend_dest,
								omt_RGBAPixelMask	mask)
{
	wsrc->lock(omlf_Read);
	lock(omlf_Write);
	
	OMediaBlitter::draw(wsrc->get_pixels(), wsrc->get_width(), wsrc->get_height(),
						get_pixels(), get_width(), get_height(),
						dest, blend_src, blend_dest, mask);

	wsrc->unlock();
	unlock();
}

void OMediaCanvas::fill(OMediaARGBColor &argb, OMediaRect *dest,
								omt_BlendFunc	blend_src, 
								omt_BlendFunc	blend_dest,
								omt_RGBAPixelMask	mask)
{
	omt_RGBAPixel	rgba;
	omt_RGBAPixel	*pix;
	
	rgba = omd_IfLittleEndianReverseLong(argb.get_rgba());
	
	lock(omlf_Write);
	pix = get_pixels();

	OMediaBlitter::fill(rgba, pix, width,height,dest,blend_src,blend_dest,mask);

	unlock();
}


void OMediaCanvas::fill_alpha(unsigned char pixalpha, OMediaRect *dest)
{
	omt_RGBAPixel	*pix;

	lock(omlf_Write);
	pix = get_pixels();

	OMediaBlitter::fill_alpha(pixalpha, pix, width,height,dest);

	unlock();
}

void OMediaCanvas::draw_line(OMediaARGBColor 	&argb,
									long x1, 		long y1,
    	                 			long x2, 		long y2,
        	             			omt_BlendFunc	blend_src, 
									omt_BlendFunc	blend_dest)
{
	OMediaShortRect		srect;
	omt_RGBAPixel		*pix,rgba = argb.get_rgba();


	rgba = omd_IfLittleEndianReverseLong(rgba);
	lock(omlf_Write);
	pix = get_pixels();
	
	srect.set(0,0,get_width(),get_height());
	
	OMediaBlitter::draw_line(pix,get_width(),x1,y1,x2,y2,rgba,&srect,blend_src,blend_dest);

	unlock();
}

void OMediaCanvas::frame_rect(OMediaARGBColor &argb,  OMediaRect &prect, 
							omt_BlendFunc blend_src,  
							omt_BlendFunc blend_dest)
{
	OMediaRect			rect;
	OMediaShortRect		srect;
	omt_RGBAPixel		*pix,rgba = argb.get_rgba();



	rgba = omd_IfLittleEndianReverseLong(rgba);

	
	rect = prect;
	rect.right--;
	rect.bottom--;
	
	lock(omlf_Write);
	pix = get_pixels();
	srect.set(0,0,get_width(),get_height());

	OMediaBlitter::draw_line(pix,get_width(),rect.left,	rect.top,	rect.right,	rect.top,	rgba,&srect,blend_src,blend_dest);	
	OMediaBlitter::draw_line(pix,get_width(),rect.right,rect.top,	rect.right,	rect.bottom,rgba,&srect,blend_src,blend_dest);	
	OMediaBlitter::draw_line(pix,get_width(),rect.right,rect.bottom,rect.left,	rect.bottom,rgba,&srect,blend_src,blend_dest);	
	OMediaBlitter::draw_line(pix,get_width(),rect.left,	rect.bottom,rect.left,	rect.top,	rgba,&srect,blend_src,blend_dest);	

	unlock();
}

void OMediaCanvas::paint_emboss(OMediaRect &rect, bool out, 
							OMediaFARGBColor &dark, 
							OMediaFARGBColor &shine, 
							OMediaFARGBColor &fill_color,
							omt_BlendFunc blend_src,
							omt_BlendFunc blend_dest)
{
	OMediaRect	irect = rect;

	frame_emboss(rect, out, dark, shine,blend_src,blend_dest);

	irect.left++;	irect.top++;
	irect.right--;	irect.bottom--;

	fill(fill_color,&irect,blend_src,blend_dest);
}

void OMediaCanvas::frame_emboss(OMediaRect &prect, bool out, 
							OMediaFARGBColor &dark, 
							OMediaFARGBColor &shine, 
							omt_BlendFunc blend_src,
							omt_BlendFunc blend_dest)
{
	OMediaRect				rect;
	OMediaFARGBColor		*d,*s;
	
	rect = prect;
	rect.right--;
	rect.bottom--;

	if (out)
	{
		s = &shine;
		d = &dark;
	}
	else
	{
		d = &shine;
		s = &dark;	
	}

	draw_line(*s,	rect.left,rect.bottom, rect.left,rect.top,	blend_src,blend_dest);
	draw_line(*s,	rect.left,rect.top,	rect.right,rect.top, 	blend_src,blend_dest);

	draw_line(*d,	rect.right,rect.top, 	rect.right,rect.bottom,		blend_src,blend_dest);
	draw_line(*d,	rect.right,rect.bottom,	rect.left,rect.bottom, 		blend_src,blend_dest);
}

void OMediaCanvas::paint_emboss(OMediaRect &rect, bool out, 
							OMediaARGBColor &dark, 
							OMediaARGBColor &shine, 
							OMediaARGBColor &fill_color,
							omt_BlendFunc blend_src,
							omt_BlendFunc blend_dest)
{
	OMediaRect	irect = rect;

	frame_emboss(rect, out, dark, shine,blend_src,blend_dest);

	irect.left++;	irect.top++;
	irect.right--;	irect.bottom--;

	fill(fill_color,&irect,blend_src,blend_dest);	
}

void OMediaCanvas::frame_emboss(OMediaRect &prect, bool out, 
							OMediaARGBColor &dark, 
							OMediaARGBColor &shine, 
							omt_BlendFunc blend_src,
							omt_BlendFunc blend_dest)
{
	OMediaRect				rect;
	OMediaARGBColor		*d,*s;
	
	rect = prect;
	rect.right--;
	rect.bottom--;

	if (out)
	{
		s = &shine;
		d = &dark;
	}
	else
	{
		d = &shine;
		s = &dark;	
	}

	draw_line(*s,	rect.left,rect.bottom, rect.left,rect.top,	blend_src,blend_dest);
	draw_line(*s,	rect.left,rect.top,	rect.right,rect.top, 	blend_src,blend_dest);

	draw_line(*d,	rect.right,rect.top, 	rect.right,rect.bottom,		blend_src,blend_dest);
	draw_line(*d,	rect.right,rect.bottom,	rect.left,rect.bottom, 		blend_src,blend_dest);
}

void OMediaCanvas::free_master_memory(void)
{
	pixels.setsize(0);
	memory_purged = true;
}

//----------------------------------------------------------

void OMediaCanvas::png_export(OMediaStreamOperators &stream)
{
	char	c;
	OMediaMemStream		chunk;
	unsigned long		crc,chunk_type,chunk_size,t;
	void				*chunk_ptr;

	// File header

	c = char(0x89); stream<<c;
	c = 'P';		stream<<c;
	c = 'N';		stream<<c;
	c = 'G';		stream<<c;

	c = char(0x0D); stream<<c;
	c = char(0x0A);	stream<<c;
	c = char(0x1A);	stream<<c;
	c = char(0x0A);	stream<<c;

	// PNG header

	unsigned char	depth,color_type,compression,filter,interlace;
	long			width,height;

	width = get_width();
	height = get_height();
	depth = 8;
	color_type = 6;
	compression = 0;
	filter = 0;
	interlace = 0;

	chunk_type = 'IHDR';

	chunk.setsize(0);
	chunk<<chunk_type;
	chunk<<width;
	chunk<<height;
	chunk<<depth;
	chunk<<color_type;
	chunk<<compression;
	chunk<<filter;
	chunk<<interlace;

	chunk_ptr = chunk.get_memory_block()->lock();
	chunk_size = chunk.get_memory_block()->getsize();
	crc = OMediaCRC::crc((unsigned char*)chunk_ptr,chunk_size);
	chunk_size-=4;
	stream<<chunk_size;
	stream.write(chunk_ptr,chunk_size+4);
	stream<<crc;
	chunk.get_memory_block()->unlock();

	// Image data

	long dest_len,pixmap_size;

	lock(omlf_Read);

	pixmap_size = (get_width()*get_height()*4L) + get_height();
	dest_len = pixmap_size + 100;

	chunk_type = 'IDAT';
	chunk.setsize(dest_len+4);
	chunk.setposition(0);
	chunk<<chunk_type;

	chunk_ptr = chunk.get_memory_block()->lock();

	unsigned char	*buffer,*ps,*pd;
	short			x,y;

	buffer = new unsigned char[pixmap_size];

	ps = (unsigned char*)get_pixels();
	pd = (unsigned char*)buffer;
	

	for(y=0;y<get_height();y++)
	{
		*(pd++) = 0;

		for(x=0;x<get_width();x++)
		{
			*(pd++) = *(ps++);
			*(pd++) = *(ps++);
			*(pd++) = *(ps++);
			*(pd++) = *(ps++);
		}
	}
	
	OMediaCompression::compress(buffer, pixmap_size, 
								((char*)chunk_ptr)+4, dest_len);

	delete buffer;

	chunk_size = dest_len+4;
	crc = OMediaCRC::crc((unsigned char*)chunk_ptr,chunk_size);

	chunk_size-=4;
	stream<<chunk_size;
	stream.write(chunk_ptr,chunk_size+4);
	stream<<crc;
	chunk.get_memory_block()->unlock();

	unlock();

	// End of file
	
	chunk_size = 0;
	chunk_type = 'IEND';
	t = omd_IfLittleEndianReverseLong(chunk_type);
	crc = OMediaCRC::crc((unsigned char*)&t,chunk_size+4);

	stream<<chunk_size;
	stream<<chunk_type;
	stream<<crc;
}


//----------------------------------------------------------

#define omd_CANVASHDR2	'omc2'

void OMediaCanvas::read_class(OMediaStreamOperators &stream)
{
	OMediaDBObject::read_class(stream);

	unsigned long	hdr;

	stream>>hdr;
	
	if (hdr!=omd_CANVASHDR2)
	{
		purge();
		stream.setposition(-4,omcfr_Current);

		OMediaCanvasConverter *converter;
		converter = OMediaCanvasConverter::create_best_converter(&stream, this);
		if (!converter) omd_EXCEPTION(omcerr_BadFormat);
		converter->convert();
		delete converter;
		
		compression_level = omclc_DefaultCompression;
	}
	else
	{
		short			s;
		unsigned long	version;
		void			*rawpix;

		stream>>version;
		stream>>width;
		stream>>height;	
		
		if (width==0 || height==0) purge();
		else
		{		
			create(width,height);
			rawpix = pixels.lock();
			stream.read(rawpix,pixels.getsize());	
			pixels.unlock();
		}

		if (version>0)
		{
			stream>>internal_pixel_format;
			stream>>s;	min_filtering = (omt_CanvasFiltering)s;	
			stream>>s;	mag_filtering = (omt_CanvasFiltering)s;
			
			if (version>1)
			{
				stream>>subdiv_w;
				stream>>subdiv_h;
			}
		}
	}
}

void OMediaCanvas::write_class(OMediaStreamOperators &stream)
{
	OMediaDBObject::write_class(stream);

	unsigned long	version = 2;
	unsigned long	hdr = omd_CANVASHDR2;
	short			s;
	void			*rawpix;

	stream<<hdr;
	stream<<version;
	stream<<width;
	stream<<height;

	if (width && height)
	{
		rawpix = pixels.lock();
		stream.write(rawpix,pixels.getsize());	
		pixels.unlock();
	}

	stream<<internal_pixel_format;
	s = (short)min_filtering;	stream<<s;
	s = (short)mag_filtering;	stream<<s;
	
	stream<<subdiv_w;
	stream<<subdiv_h;
}

unsigned long OMediaCanvas::get_approximate_size(void)
{
	long	siz = sizeof(*this);
	siz +=pixels.getsize();
	
	for(omt_EngineImpSlaveList::iterator i = slaves.begin();
		i!=slaves.end();
		i++)
	{
		siz += (*i)->get_approximate_size();
	}
	
	return siz;
}

OMediaDBObject *OMediaCanvas::db_builder(void)
{
	return new OMediaCanvas;
}

void OMediaCanvas::set_2Dsubdivision(long w, long h)
{
	lock(omlf_Write);
	subdiv_w = w;
	subdiv_h = h;
	unlock();
}


