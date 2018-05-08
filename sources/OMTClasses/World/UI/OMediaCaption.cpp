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


#include "OMediaCaption.h"

OMediaCaption::OMediaCaption()
{
	font = NULL;
	dirty = false;
	set_surface_flags(omsef_Purgeable);
	set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
}

OMediaCaption::~OMediaCaption() {}

void OMediaCaption::update_logic(float millisecs_elapsed)
{
	if (dirty)
	{
		dirty = false;
		rebuild_surface();
	}

	OMediaSurfaceElement::update_logic(millisecs_elapsed);
}

void OMediaCaption::rebuild_surface(void)
{
	if (!font) purge_surface();
	else
	{
		long	w,h,vm,l;
		font->get_font_info(w, h, vm);
		l = font->get_text_length(text);
		
		surf_buffer.create(l,h);
		surf_buffer.fill_alpha(0);
		surf_buffer.draw_string(text,0,0,font);
	}
}

void OMediaCaption::get_canv_wordsize(float &ow, float &oh)
{
	if ((canvas_flags&omcanef_FreeWorldSize) ||
		(surf_buffer.get_width() && surf_buffer.get_height())) 
	{
		OMediaSurfaceElement::get_canv_wordsize(ow,oh);
	}
	else
	{
		long	w,h,vm,l;
		
		if (font)
		{		
			font->get_font_info(w, h, vm);
			l = font->get_text_length(text);
			ow = (float) l;
			oh = (float) h;
		}
		else {ow=oh=0;}
	}
}


