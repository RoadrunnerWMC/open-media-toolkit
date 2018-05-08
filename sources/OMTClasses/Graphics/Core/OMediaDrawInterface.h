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
#ifndef OMEDIA_DrawInterface_H
#define OMEDIA_DrawInterface_H

#include "OMediaTypes.h"
#include "OMediaRendererInterface.h"
#include "OMediaEngineImplementation.h"
#include "OMediaFont.h"

class OMediaCanvas;
class OMediaRect;

class OMediaDrawInterface
{
	public:

	// * Lock support

	omtshared virtual void lock(omt_LockFlags flags = (omlf_Read|omlf_Write)) =0L;
	omtshared virtual void unlock(void) =0L;


	// * Draw canvas

	omtshared virtual void draw_full(OMediaCanvas *csrc, long destx, long desty, omt_BlendFunc blend_src =omblendfc_One,  omt_BlendFunc blend_dest =omblendfc_Zero, omt_RGBAPixelMask pixmask = ompixmc_Full) =0L;
	omtshared virtual void draw(OMediaCanvas *csrc, OMediaRect *src, long x, long y, omt_BlendFunc blend_src=omblendfc_One,  omt_BlendFunc blend_dest=omblendfc_Zero, omt_RGBAPixelMask pixmask = ompixmc_Full) =0L;
	omtshared virtual void draw(OMediaCanvas *csrc, OMediaRect *src, OMediaRect *dest, omt_BlendFunc blend_src=omblendfc_One,  omt_BlendFunc blend_dest=omblendfc_Zero, omt_RGBAPixelMask pixmask = ompixmc_Full) =0L;
	omtshared virtual void draw(OMediaCanvas *csrc, OMediaRect *dest, omt_BlendFunc blend_src=omblendfc_One,  omt_BlendFunc blend_dest=omblendfc_Zero, omt_RGBAPixelMask pixmask = ompixmc_Full) =0L;


	// * Fill

	omtshared virtual void fill(OMediaARGBColor &argb, OMediaRect *dest =NULL, omt_BlendFunc blend_src =omblendfc_One,  omt_BlendFunc	blend_dest =omblendfc_Zero, omt_RGBAPixelMask pixmask = ompixmc_Full) =0L;
	omtshared virtual void fill_alpha(unsigned char pixalpha, OMediaRect *dest =NULL) =0L;	// alpha is 0-0xFF

	inline void fill(OMediaFARGBColor &argb_f, OMediaRect *dest =NULL, omt_BlendFunc blend_src =omblendfc_One,  omt_BlendFunc	blend_dest =omblendfc_Zero, omt_RGBAPixelMask pixmask = ompixmc_Full)
	{
		OMediaARGBColor	argb;	argb.fset(argb_f.alpha,argb_f.red,argb_f.green,argb_f.blue);	
		fill(argb, dest, blend_src,blend_dest, pixmask);
	}

	// * Draw string

	inline void draw_string(string str, long dx, long dy,
										OMediaFont		*font,
										omt_BlendFunc 	blend_src =omblendfc_One,  
										omt_BlendFunc 	blend_dest =omblendfc_Zero)
	{
		font->draw_string(str, this, dx,dy,  blend_src, blend_dest);
	}
	
	
};



#endif

