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
#ifndef OMEDIA_MipmapGenerator_H
#define OMEDIA_MipmapGenerator_H

#include "OMediaTypes.h"
#include "OMediaMemTools.h"
#include "OMediaBlitter.h"
#include "OMediaEndianSupport.h"

class OMediaMipmapGenerator
{
	public:
	
	// Canvas must be "read" locked
	
	OMediaMipmapGenerator(OMediaCanvas *master)
	{
		long siz;

		full_width = cur_width = master->get_width();
		cur_height = master->get_height();

		siz = (cur_width * cur_height) << 2L;
		pixels = new unsigned char[siz];
		OMediaMemTools::copy(master->get_pixels(),pixels,siz);
		level = 0;
	}

	OMediaMipmapGenerator(OMediaCanvas *master,
							short x, short y,
							short w, short h)
	{
		long siz;
		OMediaRect	r(x,y,x+w,y+h);

		full_width = cur_width = w;
		cur_height = h;

		siz = (cur_width * cur_height) << 2L;
		pixels = new unsigned char[siz];
		OMediaBlitter::draw(master->get_pixels(), master->get_width(), master->get_height(),
							(omt_RGBAPixel*)pixels,cur_width,cur_height, &r, 0,0, omblendfc_One, omblendfc_Zero,
							ompixmc_Full);						
		
		level = 0;	
	}

	OMediaMipmapGenerator(OMediaCanvas *master, OMediaRect &src_rect, OMediaRect &dest_rect)
	{
		long siz;

		full_width = cur_width = dest_rect.get_width();
		cur_height = dest_rect.get_height();

		siz = (cur_width * cur_height) << 2L;
		pixels = new unsigned char[siz];
		OMediaBlitter::draw(master->get_pixels(), master->get_width(), master->get_height(),
							(omt_RGBAPixel*)pixels,cur_width,cur_height, &src_rect, &dest_rect, 
							omblendfc_One, omblendfc_Zero,
							ompixmc_Full);
		
		level = 0;	
	}
	

	~OMediaMipmapGenerator()
	{
		delete [] pixels;
	}
	
	bool generate_next_mipmap(void)
	{
		short			nl,nr;
		unsigned char 	*s, *s2, *d;
		unsigned short 	v;
		long			d_mod,s_mod;
	
		if (cur_width==1 && cur_height==1) return false;

		if (cur_width>1)
		{
			s_mod = (full_width - cur_width) << 2L;
			cur_width 	>>= 1L;
			d_mod = (full_width - cur_width) << 2L;
		
			s = d = pixels;
			nl = cur_height;
			while(nl)
			{
				nr = cur_width;
				
				s2 = s+4;
				while(nr)
				{			
					v = s[0];	v+= s2[0];	v >>= 1;	d[0] = v;
					v = s[1];	v+= s2[1];	v >>= 1;	d[1] = v;
					v = s[2];	v+= s2[2];	v >>= 1;	d[2] = v;
					v = s[3];	v+= s2[3];	v >>= 1;	d[3] = v;
					s += 8;
					s2 += 8;
					d += 4;
					nr--;
				}
			
				s += s_mod;
				d += d_mod;			
				nl--;
			}
		}
		
		if (cur_height>1) 	
		{
			cur_height 	>>= 1L;	
			s = d = pixels;

			s_mod = d_mod = (full_width - cur_width) << 2L;
			s_mod += full_width<<2;

			nl = cur_height;
			while(nl)
			{
				nr = cur_width;
				s2 = s + (full_width<<2);
				while(nr)
				{
					v = s[0];	v+= s2[0];	v >>= 1;	d[0] = v;
					v = s[1];	v+= s2[1];	v >>= 1;	d[1] = v;
					v = s[2];	v+= s2[2];	v >>= 1;	d[2] = v;
					v = s[3];	v+= s2[3];	v >>= 1;	d[3] = v;
					s +=4;
					s2 +=4;
					d +=4;
					nr--;
				}
			
				s += s_mod;
				d += d_mod;			
				nl--;
			}
		}
		
		level++;
		return true;
	}

	void flip_alpha(void)
	{
		long			n= cur_width * cur_height;
		unsigned long	*pix = (unsigned long*)pixels,tp,dp;

		while(n--)
		{
			tp = *pix;
			tp = omd_IfLittleEndianReverseLong(tp);
			dp =	((tp&0x000000FFUL)<<24L);		// alpha
			dp |= 	((tp&0x0000FF00UL)>>8L);		// red
			dp |=	((tp&0x00FF0000UL)>>8L);		// green
			dp |=	((tp&0xFF000000UL)>>8L);		// blue

			(*pix++) = dp;
		}
	}

	short		cur_width,cur_height, full_width;	
	short		level;
	unsigned char	*pixels;
};

#endif

