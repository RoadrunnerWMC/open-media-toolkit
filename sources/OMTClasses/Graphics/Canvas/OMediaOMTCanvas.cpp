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

#ifdef omd_ENABLE_OMTRENDERER

#include "OMediaCanvas.h"
#include "OMediaOMTCanvas.h"
#include "OMediaOMTRenderer.h"
#include "OMediaEngineImplementation.h"
#include "OMediaVideoEngine.h"
#include "OMediaBlitter.h"

OMediaOMTCanvas::OMediaOMTCanvas(	OMediaOMTRenderTarget *atarget,
							    OMediaCanvas *master,
							    omt_CanvasSlaveKeyFlags key_flags)
								:OMediaCanvasSlave(atarget,master)
{
	OMediaVideoEngine *vengine = atarget->get_renderer()->get_video_engine();

	texture_grid = NULL;
	pixel_format = 0;
	slave_key_long = 0;
	slave_key_ptr = 0;

	target_depth = vengine->get_current_video_mode()->depth;
	if (target_depth==32) target_depth = 32;
	else target_depth = 16;

	create_texture(true, key_flags);
	dirty = false;
}

OMediaOMTCanvas::~OMediaOMTCanvas()
{
	if (texture_grid)
	{
		OMediaOMTCanvasText	*tp = texture_grid;
	
		while(n_texture--) delete [] (tp++)->texture;	
		delete [] texture_grid;
	}
}

static unsigned long getpowbit(unsigned long n)
{
	unsigned long bit;

	for(bit=32;;)
	{
		bit--;
		
		if (n&(1<<bit)) return bit;
		if (bit==0) break;
	}
	
	return 0;
}

static unsigned long getnearestpow(unsigned long n)
{
	unsigned long bit;

	for(bit= 32;;)
	{
		bit--;
		
		if (n&(1<<bit)) return (1<<bit);
		if (bit==0) break;
	}
	
	return 0;
}

static unsigned long getnearestpow_shift(unsigned long n)
{
	unsigned long bit;

	for(bit= 32;;)
	{
		bit--;
		
		if (n&(1<<bit)) return bit;
		if (bit==0) break;
	}
	
	return 0;
}

void OMediaOMTCanvas::check_texture_size(long &w, long &h)
{
	short	nw,nh;

	if (w>256) w = 256;
	if (h>256) h = 256;

	nw = getnearestpow(w);
	nh = getnearestpow(h);
	
	if (w!=nw) nw <<=1;
	if (h!=nh) nh <<=1;

	if (nw>256) nw = 256;
	if (nh>256) nh = 256;
	
	w = nw;
	h = nh;
}

void OMediaOMTCanvas::prepare_depth(void)
{
	omt_PixelFormat	master_pixformat;
	OMediaCanvas	*master_canv;

	master_canv = (OMediaCanvas*)master;
	master_pixformat = master_canv->get_internal_pixel_format();

	if ((master_pixformat&ompixfc_Best)==ompixfc_Best || master_pixformat==0) pixel_format = ompixfc_ARGB8888;
	else if (	(master_pixformat&ompixfc_ResBest)==ompixfc_ResBest || 
				(master_pixformat&ompixfc_ResBestAlpha)==ompixfc_ResBestAlpha ||
				(master_pixformat&ompixfc_ResBestAlpha1bit)==ompixfc_ResBestAlpha1bit)
	{
		if (target_depth==16) pixel_format = ompixfc_ARGB1555;
		else pixel_format = ompixfc_ARGB8888;
	}
	else if (master_pixformat&(ompixfc_RGB555|ompixfc_RGB565|ompixfc_ARGB1555))
	{
		pixel_format = ompixfc_ARGB1555;
	}
	else
		pixel_format = ompixfc_ARGB8888;
}

void OMediaOMTCanvas::create_texture(bool new_texture, omt_CanvasSlaveKeyFlags key_flags)
{
	long					w,nw,h,nh;
	OMediaCanvas			*master_canv;	

	master_canv = (OMediaCanvas*)master;

	nw = w = master_canv->get_width();
	nh = h = master_canv->get_height();

	prepare_depth();
	check_texture_size(nw,nh);

	if (w==nw && h==nh)		// Exact size
	{
		n_texture = 1;
		slave_key_long = omcskf_Exact;

		master_canv->lock(omlf_Read);

		if (new_texture)
		{
			texture_grid = new OMediaOMTCanvasText[1];
			create_texture_block(texture_grid,w,h);
		}

		if (pixel_format==ompixfc_ARGB1555)
		{
			OMediaBlitter::remap_to_A1555(master_canv->get_pixels(),
										  (unsigned short*)texture_grid->texture,
										  w*h);
		}
		else
		{		
			OMediaBlitter::draw_full(master_canv->get_pixels(), w, h, (omt_RGBAPixel*)texture_grid->texture,w,h, 0,0, 
								omblendfc_One,omblendfc_Zero,ompixmc_Full);

			flip_alpha(texture_grid->texture,nw,nh);
		}
			
		master_canv->unlock();
	}
	else if (key_flags&omcskf_SubDivided)	// Sub divised
	{
		create_subimage(master_canv,w,h,new_texture,nw,nh);	
	}
	else
	{
		OMediaRect	src,dest;
	
		slave_key_long = omcskf_Scaled;	
		n_texture = 1;
	
		master_canv->lock(omlf_Read);

		if (new_texture)
		{
			texture_grid = new OMediaOMTCanvasText[1];
			create_texture_block(texture_grid,nw,nh);
		}
		
		src.set(0,0,w,h);
		dest.set(0,0,nw,nh);

		if (pixel_format==ompixfc_ARGB1555)
		{
			omt_RGBAPixel	*t_buffer;
			t_buffer=new omt_RGBAPixel[nw*nh];
			OMediaBlitter::draw(master_canv->get_pixels(), w, h, t_buffer,nw,nh,
							&src,&dest,omblendfc_One,omblendfc_Zero,ompixmc_Full);

			OMediaBlitter::remap_to_A1555(t_buffer,
										  (unsigned short*)texture_grid->texture,
										  nw*nh);

			delete [] t_buffer;
		}
		else
		{

			OMediaBlitter::draw(master_canv->get_pixels(), w, h, (omt_RGBAPixel*)texture_grid->texture,nw,nh,
							&src,&dest,omblendfc_One,omblendfc_Zero,ompixmc_Full);

			flip_alpha(texture_grid->texture,nw,nh);
		}
			
		master_canv->unlock();
	}
}

void OMediaOMTCanvas::master_modified(void)
{
	dirty = true;
}

void OMediaOMTCanvas::create_subimage(OMediaCanvas *canv, long w, long h, bool new_texture, long nw, long nh)
{
	OMediaRect	r;
	
	slave_key_long = omcskf_SubDivided;	
	
	subdiv_shift_w = getnearestpow_shift((nw>w)?nw>>1:nw);
	subdiv_shift_h = getnearestpow_shift((nh>h)?nh>>1:nh);

	subdiv_w = float(1<<subdiv_shift_w);
	subdiv_h = float(1<<subdiv_shift_h);

	subdiv_w_mask = (1<<subdiv_shift_w)-1;
	subdiv_h_mask = (1<<subdiv_shift_h)-1;

	n_textw = w>>subdiv_shift_w;	if (w&subdiv_w_mask) n_textw++;
	n_texth = h>>subdiv_shift_h;	if (h&subdiv_h_mask) n_texth++;

	n_texture = n_textw * n_texth;

	if (new_texture)
	{
		texture_grid = new OMediaOMTCanvasText[n_texture];
	}

	OMediaOMTCanvasText			*tptr = texture_grid;
	
	long			tw,th;	
	long			last_th,
					last_tw;

	if ((w&subdiv_w_mask)!=0)
	{
		last_tw = getnearestpow(w&subdiv_w_mask);
		if (last_tw!=(w&subdiv_w_mask)) last_tw<<=1L;
		last_u = float(w&subdiv_w_mask)/float(last_tw);
	}
	else last_u = 0.9999f;

	if ((h&subdiv_h_mask)!=0)
	{
		last_th = getnearestpow(h&subdiv_h_mask);
		if (last_th!=(h&subdiv_h_mask)) last_th<<=1L;
		last_v = float(h&subdiv_h_mask)/float(last_th);
	}
	else last_v = 0.9999f;

	canv->lock(omlf_Read);

	for(long ty = 0; ty<n_texth; ty++)
	{
		if ((ty==n_texth-1 && (h&subdiv_h_mask)!=0))
		{
			th = last_th;			
		}
		else 
		{
			th = subdiv_h_mask+1;
		}
	
		for(long tx = 0; tx<n_textw; tx++,tptr++)
		{
			if (tx==n_textw-1 && (w&subdiv_w_mask)!=0)
			{
				tw = last_tw;
			}
			else				
			{
				tw = (subdiv_w_mask+1);
			}

			if (new_texture) 
			{
				create_texture_block(tptr,tw,th);
			}

			r.left = (tx<<subdiv_shift_w);
			r.top  = (ty<<subdiv_shift_h);
			r.right = r.left + tw;
			r.bottom = r.top + th;

			if (pixel_format==ompixfc_ARGB1555)
			{
				omt_RGBAPixel	*t_buffer;

				t_buffer = new omt_RGBAPixel[tw*th];

				OMediaBlitter::draw(canv->get_pixels(), canv->get_width(), canv->get_height(),
								t_buffer, tw, th, &r, 0,0, omblendfc_One,omblendfc_Zero,ompixmc_Full);

				OMediaBlitter::remap_to_A1555(t_buffer,
											  (unsigned short*)tptr->texture,
											  tw*th);

				delete [] t_buffer;
			}
			else
			{
				OMediaBlitter::draw(canv->get_pixels(), canv->get_width(), canv->get_height(),
								(omt_RGBAPixel*)tptr->texture, tw, th, &r, 0,0, omblendfc_One,omblendfc_Zero,ompixmc_Full);

				flip_alpha(tptr->texture,tw,th);
			}
		}
	}

	canv->unlock();
}

void OMediaOMTCanvas::flip_alpha(char *pixels, short width, short height)
{
	long			n= width * height;
	unsigned long	*pix = (unsigned long*)pixels,tp,dp;

	while(n--)
	{
		tp = *pix;
		tp = omd_IfLittleEndianReverseLong(tp);
		dp =	((tp&0x000000FFUL)<<24L);		// alpha
		dp |= 	((tp&0x0000FF00UL)>>8L);		// blue
		dp |=	((tp&0x00FF0000UL)>>8L);		// green
		dp |=	((tp&0xFF000000UL)>>8L);		// red

		(*pix++) = dp;
	}
}

void OMediaOMTCanvas::prepare_omttext(OMediaOMTCanvasText *text,short w, short h, long rowbytes)
{
	text->rowbytes_shift = getpowbit(rowbytes);	
	text->xmod_mask = 0xFFFF>>(16-getpowbit(w));
	text->ymod_mask = 0xFFFF>>(16-getpowbit(h));
	text->fwidth = float(w);
	text->fheight = float(h);
	text->argb1555 = (pixel_format==ompixfc_ARGB1555);
}

void OMediaOMTCanvas::create_texture_block(	OMediaOMTCanvasText *text,
											short w, 
											short h)
{
	if (pixel_format==ompixfc_ARGB1555)
	{
		text->texture = new char[(w*h)<<1L];
		prepare_omttext(text,w,h,w<<1L);
	}
	else
	{
		text->texture = new char[(w*h)<<2L];
		prepare_omttext(text,w,h,w<<2L);
	}
}



#endif

