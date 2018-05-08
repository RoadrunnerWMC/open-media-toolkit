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
 
#include "OMediaScroller.h"
#include "OMediaPickRequest.h"
#include "OMediaWorld.h"
#include "OMediaInputEngine.h"

OMediaScroller::OMediaScroller()
{
	msg = omsg_NULL;
		
	width = height = 0;
	
	min_knob_w = min_knob_h = 10;
	htotal = 100;
	hpos = 0;
	hnunits = 100;
	vtotal = 100; 
	vpos = 0;
	vnunits = 100;
	border_size = 1;
	delta_x = delta_y = 0;
	selected = false;

	fill.alpha = 1.0f; 		fill.red = fill.green = fill.blue = float(0xEEEE)/float(0xFFFF);	
	dark.alpha = 1.0f; 		dark.red = dark.green = dark.blue = float(0x6666)/float(0xFFFF);
	shine.alpha = 1.0f; 	shine.red = shine.green = shine.blue = float(0xFFFF)/float(0xFFFF);	
	shadow.alpha = 1.0f; 	shadow.red = shadow.green = shadow.blue = float(0x3333)/float(0xFFFF);	
	back_fill.alpha = 1.0f; back_fill.red = back_fill.green = back_fill.blue = float(0xAAAA)/float(0xFFFF);
}

OMediaScroller::~OMediaScroller()
{
}

void OMediaScroller::clicked(OMediaPickResult *res, bool mouse_down)
{
	float	x,y;

	OMediaInputEngine	*input = its_world->get_input_engine();

	
	if (!its_world || !input) return;
	if (!mouse_down)
	{
		stop_move();
		return;
	}
	
	x = res->sub_info[0].u * float(surf_buffer.get_width());
	y = res->sub_info[0].v * float(surf_buffer.get_height());

	if (isinknob((int)x,(int)y))
	{				
		input->get_mouse_position(clickox,clickoy);
		selected = true;
		purge_surface();
	}
	else 
	{
		OMediaRect	bounds(0,0,width,height);
		OMediaRect	r;
		long		px = (long)x, py = (long)y;

		compute_knob_rect(bounds, r);
		
		if (px<r.left) hpos -= hnunits;
		else if (px>r.right) hpos += hnunits;

		if (hpos + hnunits > htotal) hpos = htotal-hnunits;
		if (hpos<0) hpos = 0;

		if (py<r.top) vpos -= vnunits;
		else if (py>r.bottom) vpos += vnunits;

		if (vpos + vnunits > vtotal) vpos = vtotal-vnunits;
		if (vpos<0) vpos = 0;

		if (OMediaScroller::msg != omsg_NULL) broadcast_message(OMediaScroller::msg,this);

		purge_surface();
	}
}


void OMediaScroller::stop_move(void)
{
	if (!selected) return;

	if (delta_x || delta_y)
	{
		hpos += delta_x;
		vpos += delta_y;
		
		if (hpos<0) hpos = 0;
		if (vpos<0) vpos = 0;

		if (hpos + hnunits > htotal) hpos = htotal - hnunits;
		if (vpos + vnunits > vtotal) vpos = vtotal - vnunits;
			
		delta_x = 0;
		delta_y = 0;
	}

	selected = false;
	purge_surface();
}

void OMediaScroller::update_logic(float elapsed)
{
	short x,y;
	float	old_delta_x = delta_x, old_delta_y = delta_y;
	OMediaInputEngine	*input = its_world->get_input_engine();

	if (selected && its_world && input)
	{
		if (!input->mouse_down())
		{
			stop_move();
		}
		else
		{	
			input->get_mouse_position(x,y);
		
			x -= clickox;
			y -= clickoy;
			
			delta_x = short(float(x* htotal) / float(width-(border_size<<1)));
			delta_y = short(float(y* vtotal) / float(height-(border_size<<1)));
		
			if (old_delta_x!=delta_x || old_delta_y!=delta_y)
			{
				if (OMediaScroller::msg != omsg_NULL) broadcast_message(OMediaScroller::msg,this);
				purge_surface();
			}
		}
	}
	
	OMediaSurfaceElement::update_logic(elapsed);
}

void OMediaScroller::move_knob_to(short x, short y, short px, short py)
{
	short		kw,kh,bw,bh;

	bw = width-(border_size<<1);
	bh = height-(border_size<<1);

	kw = short((hnunits * bw) / htotal);
	kh  = short((vnunits * bh) / vtotal);
	
	px =  (px - (kw>>1)) - x;
	py =  (py - (kh>>1)) - y;
	
	hpos = (px* htotal)/bw;
	vpos = (py* vtotal)/bh;

	if (hpos<0) hpos = 0;
	if (vpos<0) vpos = 0;

	if (hpos + hnunits > htotal) hpos = htotal - hnunits;
	if (vpos + vnunits > vtotal) vpos = vtotal - vnunits;
}

void OMediaScroller::draw_knob(OMediaCanvas &canv, OMediaRect &knob_r, OMediaRect &bounds)
{
	OMediaRect		r,shad_r;
	
	canv.paint_emboss(knob_r, true, dark, shine, fill, omblendfc_One, omblendfc_Zero);
	
	shad_r.set(knob_r.left+1,knob_r.bottom, knob_r.right, knob_r.bottom+1);
	if (shad_r.find_intersection(&bounds,&r))
	{
		canv.paint_rect(shadow,r);
	}

	shad_r.set(knob_r.right, knob_r.top+1, knob_r.right+1, knob_r.bottom+1);
	if (shad_r.find_intersection(&bounds,&r))
	{
		canv.paint_rect(shadow,r);
	}
}

void OMediaScroller::draw_back(OMediaCanvas &canv, OMediaRect &r)
{
	canv.paint_emboss(r,false,dark,shine,back_fill);
}

void OMediaScroller::compute_knob_rect(OMediaRect &bounds, OMediaRect &r)
{
	float		length;

	if (htotal==0 || vtotal==0) 
	{
		r = bounds;
		return;
	}

	// Horizontal
	
	length = bounds.get_width();
	r.left = long((hpos * length) / htotal) + bounds.left;
	
	if (hpos + hnunits >= htotal) r.right = bounds.right;
	else r.right = r.left + long((hnunits * length) / htotal);

	if (r.get_width()<min_knob_w) r.right = r.left + min_knob_w;

	if (r.right>bounds.right)
	{
		r.offset(bounds.right - r.right,0);
		if (r.left<bounds.left) r.left = bounds.left;
	}

	// Vertical
	
	length = bounds.get_height();
	r.top = long((vpos * length) / vtotal) + bounds.top;
	
	if (vpos + vnunits >= vtotal) r.bottom = bounds.bottom;
	else r.bottom = r.top + long((vnunits * length) / vtotal);

	if (r.get_height()<min_knob_h) r.bottom = r.top + min_knob_h;

	if (r.bottom>bounds.bottom)
	{
		r.offset(0,bounds.bottom - r.bottom);
		if (r.top<bounds.top) r.top = bounds.top;
	}
}

float OMediaScroller::get_hpos(void) const  
{
	float r = hpos;

	if (delta_x)
	{
		r += delta_x;
	
		if (r<0) r = 0;
		if (r + hnunits > htotal) r = htotal - hnunits;
	}
	
	return r;
}

float OMediaScroller::get_vpos(void) const  
{
	float r = vpos;

	if (delta_y)
	{
		r += delta_y;
	
		if (r<0) r = 0;
		if (r + vnunits > vtotal) r = vtotal - vnunits;
	}
	
	return r;
}

void OMediaScroller::rebuild_surface(void)
{
	OMediaRect		r,knob_r,bounds;
	float			tx,ty;

	surf_buffer.create(width,height);

	tx = hpos;
	ty = vpos;

	if (delta_x || delta_y)
	{
		hpos += delta_x;
		vpos += delta_y;
	
		if (hpos<0) hpos = 0;
		if (vpos<0) vpos = 0;

		if (hpos + hnunits > htotal) hpos = htotal - hnunits;
		if (vpos + vnunits > vtotal) vpos = vtotal - vnunits;
	}
	
	bounds.set(0,0,width,height);
	draw_back(surf_buffer,bounds);

	r.set(border_size,border_size,width-border_size,height-border_size);
	compute_knob_rect(r,knob_r);
	draw_knob(surf_buffer, knob_r,r);
		
	hpos = tx;
	vpos = ty;
}

void OMediaScroller::get_canv_wordsize(float &w, float &h)
{
	if ((canvas_flags&omcanef_FreeWorldSize) ||
		(surf_buffer.get_width() && surf_buffer.get_height())) 
	{
		OMediaSurfaceElement::get_canv_wordsize(w,h);
	}
	else
	{
		w = width;
		h = height;
	}
}

bool OMediaScroller::isinknob(long px, long py)
{
	OMediaRect		r, knob_r;

	r.set(0,0,width,height);	
	
	r.left += border_size;
	r.top += border_size;
	r.bottom -=border_size;
	r.right -=border_size;

	compute_knob_rect(r,knob_r);

	return (knob_r.is_pointin(px,py));
}

	
