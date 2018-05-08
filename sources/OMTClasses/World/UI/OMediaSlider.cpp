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
 

#include "OMediaSlider.h"
#include "OMediaPickRequest.h"
#include "OMediaWorld.h"
#include "OMediaInputEngine.h"


const short omc_BaseKnobLength = 8;

OMediaSlider::OMediaSlider()
{
	msg = omsg_NULL;
	selected = false;
        knob_length = omc_BaseKnobLength;
	
	width = height = 0;

	maxvalue = 100;
	value = 0;
	delta = 0;
	smode = omsmc_Horizontal;

	fill.alpha = 1.0f; 		fill.red = fill.green = fill.blue = float(0xCCCC)/float(0xFFFF);	
	dark.alpha = 1.0f; 		dark.red = dark.green = dark.blue = float(0x6666)/float(0xFFFF);
	shine.alpha = 1.0f; 	shine.red = shine.green = shine.blue = float(0xFFFF)/float(0xFFFF);	
	back_fill.alpha = 1.0f; back_fill.red = back_fill.green = back_fill.blue = float(0xAAAA)/float(0xFFFF);

	set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
}

OMediaSlider::~OMediaSlider() {}
	
void OMediaSlider::clicked(OMediaPickResult *res, bool mouse_down)
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
		short	clickox,clickoy;
	
		input->get_mouse_position(clickox,clickoy);
			
		clicko = (smode == omsmc_Horizontal)?clickox:clickoy;
		base_value = value;
		selected = true;
	}
	else
	{
		OMediaRect	bounds(0,0,width,height);
		OMediaRect	r;
		long		px = (long)x, py = (long)y;

		compute_knob_rect(bounds, r);

		if (smode == omsmc_Horizontal)
		{
			if (px<r.left) value -= maxvalue * (1.0f/10.0f);
			else if (px>r.right) value += maxvalue * (1.0f/10.0f);
		}
		else
		{
			if (py<r.top) value -= maxvalue * (1.0f/10.0f);
			else if (py>r.bottom) value += maxvalue * (1.0f/10.0f);
		}

		if (value > maxvalue) value = maxvalue;
		if (value < 0.0f) value = 0.0f;

		if (OMediaSlider::msg != omsg_NULL) broadcast_message(OMediaSlider::msg,this);

		purge_surface();
	}
}	 

void OMediaSlider::stop_move(void)
{
	if (!selected) return;

	float pixtotal = (smode == omsmc_Horizontal)?width:height;
	pixtotal -= knob_length;

	if (delta && pixtotal>0)
	{
		value = base_value + ((delta*maxvalue)/pixtotal);
		
		if (value<0) value = 0;
		if (value>maxvalue) value = maxvalue;
			
		delta = 0;
	}

	selected = false;
	purge_surface();
}

void OMediaSlider::update_logic(float elapsed)
{
	short x,y;
	float old_value = value;
	float pixtotal = (smode == omsmc_Horizontal)?width:height;
	pixtotal -= knob_length;
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
		
			delta = (smode == omsmc_Horizontal)?x:y;
			delta -= clicko;
		
			if (pixtotal>0)
			{
				value = base_value + ((delta*maxvalue)/pixtotal);		
				if (value<0) value = 0;
				if (value>maxvalue) value = maxvalue;
			}
		
			if (old_value!=value)
			{
				if (OMediaSlider::msg != omsg_NULL) broadcast_message(OMediaSlider::msg,this);
				purge_surface();
			}
		}
	}

	OMediaSurfaceElement::update_logic(elapsed);
}

void OMediaSlider::move_knob_to(short x, short y, short px, short py)
{
	float pixtotal = (smode == omsmc_Horizontal)?width:height;
	pixtotal -= knob_length;

	short		p = (smode == omsmc_Horizontal)?px:py;
	short		v = (smode == omsmc_Horizontal)?x:y;
	
	p = (p - (knob_length>>1)) - v;

	if (pixtotal>0)
	{
		value = (float(p)*maxvalue)/pixtotal;		
		if (value<0) value = 0;
		if (value>maxvalue) value = maxvalue;
	}
}

void OMediaSlider::draw_knob(OMediaCanvas &canv, OMediaRect &knob_r, OMediaRect &bounds)
{	
	canv.paint_emboss(knob_r,true,dark,shine,fill);

	short p = (knob_length/2);

	if (smode == omsmc_Horizontal)
	{
		canv.draw_line(dark,knob_r.left + (p-1), knob_r.top+3,
						knob_r.left + (p-1), knob_r.bottom-4);
	
		canv.draw_line(shine,knob_r.left + p, knob_r.top+3,
				knob_r.left + p, knob_r.bottom-4);
	}
	else
	{
		canv.draw_line(dark,knob_r.left+3 , knob_r.top + (p-1),
				knob_r.right-4, knob_r.top + (p-1));
	
		canv.draw_line(shine,knob_r.left+3 , knob_r.top + p,
				knob_r.right-4, knob_r.top + p);
	}
}

void OMediaSlider::draw_back(OMediaCanvas &dp, OMediaRect &r)
{
	dp.paint_emboss(r,false,dark,shine,back_fill);
}

void OMediaSlider::compute_knob_rect(OMediaRect &bounds, OMediaRect &r)
{
	float pixtotal = (smode == omsmc_Horizontal)?width:height;
	pixtotal -= knob_length;

	r.set(0,0,0,0);

	if (bounds.empty() || pixtotal<=0 || maxvalue<=0) return;
	if (smode==omsmc_Horizontal && bounds.get_width()<knob_length) return;
	if (smode==omsmc_Vertical && bounds.get_height()<knob_length) return;

	short pixpos;

	pixpos = short((value*pixtotal)/maxvalue);	

	if (pixpos<0) pixpos=0;
	if (pixpos>=pixtotal) pixpos = (short)pixtotal;

	if (smode == omsmc_Horizontal)
	{
		r.left = bounds.left + pixpos;
		r.right = r.left + knob_length;
		r.top = bounds.top;
		r.bottom = bounds.bottom;
	}
	else
	{
		r.top = bounds.top + pixpos;
		r.bottom = r.top + knob_length;
		r.left = bounds.left;
		r.right = bounds.right;
	}
}


void OMediaSlider::rebuild_surface(void)
{
	OMediaRect		r,knob_r,bounds,backbounds;

	surf_buffer.create(width,height);
	surf_buffer.fill_alpha(0);

	bounds.set(0,0,width,height);
	backbounds = bounds;

	if (smode == omsmc_Horizontal)
	{
		backbounds.top += 3;
		backbounds.bottom -= 3;
	}
	else
	{
		backbounds.left += 3;
		backbounds.right -= 3;
	}

	if (!backbounds.empty()) draw_back(surf_buffer,backbounds);

	r.set(0,0,width,height);
	compute_knob_rect(r,knob_r);
	if (!knob_r.empty()) draw_knob(surf_buffer, knob_r,r);
	
}

bool OMediaSlider::isinknob(long px, long py)
{
	OMediaRect		r, knob_r;
	
	r.set(0,0,width,height);
	compute_knob_rect(r,knob_r);

	return (knob_r.is_pointin(px,py));
}

void OMediaSlider::get_canv_wordsize(float &w, float &h)
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
