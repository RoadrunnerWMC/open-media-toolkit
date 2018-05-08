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

#include "OMediaStdButton.h"
#include "OMediaWorld.h"
#include "OMediaBlitter.h"
#include "OMediaEndianSupport.h"

const long omc_CheckBoxCanvWidth 	= 14;
const long omc_CheckBoxCanvHeight 	= 14;
const long omc_RadioCanvWidth 		= 13;
const long omc_RadioCanvHeight 		= 13;

extern unsigned long oms_CheckboxUpData[]; 
extern unsigned long oms_CheckboxDownData[]; 
extern unsigned long oms_RadioUpData[]; 
extern unsigned long oms_RadioDownData[]; 

#ifdef omd_LITTLE_ENDIAN
extern bool omg_butdata_ordered;
extern void omf_order_bufdata(void);
#endif


OMediaStdButton::OMediaStdButton()
{
	set_surface_flags(omsef_Purgeable);
	internal_state = ombistc_Empty;
	btn_width = btn_height = 0;
	font = 0;
	style = ombtnc_Standard;

	dark.set(1.0f,float(0x4444)/float(0xFFFF),float(0x4444)/float(0xFFFF),float(0x4444)/float(0xFFFF));
	shine.set(1.0f,float(0xEEEE)/float(0xFFFF),float(0xEEEE)/float(0xFFFF),float(0xEEEE)/float(0xFFFF));
	fill_up.set(1.0f,float(0xCCCC)/float(0xFFFF),float(0xCCCC)/float(0xFFFF),float(0xCCCC)/float(0xFFFF));
	fill_down.set(1.0f,float(0xAAAA)/float(0xFFFF),float(0xAAAA)/float(0xFFFF),float(0xAAAA)/float(0xFFFF));


#ifdef omd_LITTLE_ENDIAN
	if (!omg_butdata_ordered) omf_order_bufdata();
#endif
}

OMediaStdButton::~OMediaStdButton()
{
}

void OMediaStdButton::clicked(OMediaPickResult *res, bool mouse_down)
{
	if (!its_world) return;

	OMediaAbstractButton::clicked(	res, 
									mouse_down, 
									&its_world->get_mouse_tracking_broadcaster(),
									this);

	if (isdown()) 
	{
		if (internal_state!=ombistc_Down) purge_surface();
	}
	else
	{
		if (internal_state!=ombistc_Up) purge_surface();	
	}
}

void OMediaStdButton::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_MouseTrack:
		{
			OMediaMouseTrackPick	*mtrack = (OMediaMouseTrackPick*)param;
			
			if (!its_world) return;
			
			OMediaAbstractButton::track(mtrack,
									 	&its_world->get_mouse_tracking_broadcaster(),
								 		this);

			if (isdown()) 
			{
				if (internal_state!=ombistc_Down) purge_surface();
			}
			else
			{
				if (internal_state!=ombistc_Up) purge_surface();	
			}
		}
		break;
		
		default:
		OMediaElement::listen_to_message(msg, param);
		break;	
	}
}

void OMediaStdButton::rebuild_surface(void)	
{
	OMediaRect				rect;
	OMediaARGBColor			black(0xFFFF,0,0,0);

	if (!font) 
	{
		internal_state=ombistc_Empty;
		purge_surface();
		return;
	}

	if (isdown()) internal_state=ombistc_Down;
	else internal_state=ombistc_Up;

	float	w,h;
	long	tw,th,vm;

	font->get_font_info(tw, th,vm);	
	tw = font->get_text_length(text);

	get_canv_wordsize(w,h);
	
	switch(style)
	{
		case ombtnc_Standard:
		{
			set_blend(omblendfc_One, omblendfc_Zero);
					
			surf_buffer.create((int)w,(int)h);
			surf_buffer.frame_rect(black,0,0,(int)w,(int)h);
			
			rect.set(1,1,(int)w-1,(int)h-1);
			surf_buffer.paint_emboss(rect, !isdown(), 
									dark,shine,isdown()?fill_down:fill_up);
			
			surf_buffer.draw_string(text,(long(w)>>1)-(tw>>1),(long(h)>>1)-(th>>1),
									font,
									omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
		}
		break;

		case ombtnc_Checkbox:
		{
			set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);

			surf_buffer.create((int)w,(int)h);
			surf_buffer.lock(omlf_Write);
			surf_buffer.fill_alpha(0x00000000L);
			OMediaBlitter::draw_full(isdown()?oms_CheckboxDownData:oms_CheckboxUpData, 
										omc_CheckBoxCanvWidth, omc_CheckBoxCanvHeight,
										surf_buffer.get_pixels(),
										surf_buffer.get_width(),surf_buffer.get_height(),
										0, (surf_buffer.get_height()>>1)-(omc_CheckBoxCanvHeight>>1),
										omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha,ompixmc_Full);

			surf_buffer.draw_string(text,omc_CheckBoxCanvWidth+2,
									(long(h)>>1)-(th>>1),
									font,
									omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);

			surf_buffer.unlock();
		}
		break;

		case ombtnc_Radio:
		{
			set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);

			surf_buffer.create((int)w,(int)h);
			surf_buffer.lock(omlf_Write);
			surf_buffer.fill_alpha(0x00000000L);
			OMediaBlitter::draw_full(isdown()?oms_RadioDownData:oms_RadioUpData, 
										omc_RadioCanvWidth, omc_RadioCanvHeight,
										surf_buffer.get_pixels(),
										surf_buffer.get_width(),surf_buffer.get_height(),
										0, (surf_buffer.get_height()>>1)-(omc_RadioCanvHeight>>1),
										omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha,ompixmc_Full);

			surf_buffer.draw_string(text,omc_RadioCanvWidth+2,
									(long(h)>>1)-(th>>1),
									font,
									omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);

			surf_buffer.unlock();
		}
		break;
	}
}

void OMediaStdButton::get_canv_wordsize(float &ow, float &oh)
{
	if ((canvas_flags&omcanef_FreeWorldSize) ||
		(surf_buffer.get_width() && surf_buffer.get_height())) 
	{
		OMediaSurfaceElement::get_canv_wordsize(ow,oh);
	}
	else
	{
		long	w,h,vm,space_size;
		
		if (font)
		{
			if (!btn_width || !btn_height)
			{
				font->get_font_info(w, h, vm);	
				w = font->get_text_length(text);
			}

			space_size = font->get_text_length(" ");
			
			switch(style)
			{
				case ombtnc_Standard:
				{
					if (!btn_width) w += 24;
					else w = btn_width;
		
					if (!btn_height) h += 7;
					else h = btn_height;
					
					ow = w;
					oh = h;
				}
				break;

				case ombtnc_Checkbox:
				{
					ow = w+space_size+omc_CheckBoxCanvWidth;
					oh = ((h+2)<omc_CheckBoxCanvHeight)?omc_CheckBoxCanvHeight:h+2;
				}
				break;

				case ombtnc_Radio:
				{
					ow = w+space_size+omc_RadioCanvWidth;
					oh = ((h+2)<omc_RadioCanvHeight)?omc_RadioCanvHeight:h+2;
				}
				break;
			}
		}
		else {ow=oh=0;}
	}
}

void OMediaStdButton::select(void)
{
	OMediaAbstractButton::select();
	purge_surface();
}

void OMediaStdButton::deselect(void)
{
	OMediaAbstractButton::deselect();
	purge_surface();
}

//---- Image data

#ifdef omd_LITTLE_ENDIAN
static bool omg_butdata_ordered;

static void omf_order_bufdata(void)
{
	omg_butdata_ordered = true;

	long	n;
	unsigned long *ptr;

	n = omc_CheckBoxCanvWidth * omc_CheckBoxCanvHeight;
	ptr = oms_CheckboxUpData;
	while(n--) 
	{
		*ptr = omd_ReverseLong(*ptr);
		ptr++;
	}

	n = omc_CheckBoxCanvWidth * omc_CheckBoxCanvHeight;
	ptr = oms_CheckboxDownData;
	while(n--) 
	{
		*ptr = omd_ReverseLong(*ptr);
		ptr++;
	}

	n = omc_RadioCanvWidth * omc_RadioCanvHeight;
	ptr = oms_RadioUpData;
	while(n--) 
	{
		*ptr = omd_ReverseLong(*ptr);
		ptr++;
	}

	n = omc_RadioCanvWidth * omc_RadioCanvHeight;
	ptr = oms_RadioDownData;
	while(n--) 
	{
		*ptr = omd_ReverseLong(*ptr);
		ptr++;
	}
}

#endif

unsigned long oms_CheckboxUpData[] = 
{
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0xffffffff, 
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
	0xffffffff, 0xffffffff, 0xffffffff, 0x292929ff, 
	0x292929ff, 0xffffffff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0x6b6b6bff, 0x292929ff, 0x292929ff, 0xffffffff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0x6b6b6bff, 0x292929ff, 
	0x292929ff, 0xffffffff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0x6b6b6bff, 0x292929ff, 0x292929ff, 0xffffffff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0x6b6b6bff, 0x292929ff, 
	0x292929ff, 0xffffffff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0x6b6b6bff, 0x292929ff, 0x292929ff, 0xffffffff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0x6b6b6bff, 0x292929ff, 
	0x292929ff, 0xffffffff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0x6b6b6bff, 0x292929ff, 0x292929ff, 0xffffffff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0x6b6b6bff, 0x292929ff, 
	0x292929ff, 0xffffffff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0x6b6b6bff, 0x292929ff, 0x292929ff, 0xffffffff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0x6b6b6bff, 0x292929ff, 
	0x292929ff, 0xffffffff, 0x6b6b6bff, 0x6b6b6bff, 
	0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bff, 
	0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bff, 
	0x6b6b6bff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff
};

unsigned long oms_CheckboxDownData[] = 
{
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x6b6b6bff, 
	0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bff, 
	0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bff, 
	0x6b6b6bff, 0x6b6b6bff, 0xffffffff, 0x292929ff, 
	0x292929ff, 0x6b6b6bff, 0xff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0xff, 
	0xffffffff, 0x292929ff, 0x292929ff, 0x6b6b6bff, 
	0x9b9b9bff, 0xff, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 
	0xff, 0x9b9b9bff, 0xffffffff, 0x292929ff, 
	0x292929ff, 0x6b6b6bff, 0x9b9b9bff, 0x9b9b9bff, 
	0xff, 0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0xff, 0x9b9b9bff, 0x9b9b9bff, 
	0xffffffff, 0x292929ff, 0x292929ff, 0x6b6b6bff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0xff, 
	0x9b9b9bff, 0x9b9b9bff, 0xff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0xffffffff, 0x292929ff, 
	0x292929ff, 0x6b6b6bff, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0xff, 0xff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 
	0xffffffff, 0x292929ff, 0x292929ff, 0x6b6b6bff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 
	0xff, 0xff, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0xffffffff, 0x292929ff, 
	0x292929ff, 0x6b6b6bff, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0xff, 0x9b9b9bff, 0x9b9b9bff, 
	0xff, 0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 
	0xffffffff, 0x292929ff, 0x292929ff, 0x6b6b6bff, 
	0x9b9b9bff, 0x9b9b9bff, 0xff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0xff, 
	0x9b9b9bff, 0x9b9b9bff, 0xffffffff, 0x292929ff, 
	0x292929ff, 0x6b6b6bff, 0x9b9b9bff, 0xff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0xff, 0x9b9b9bff, 
	0xffffffff, 0x292929ff, 0x292929ff, 0x6b6b6bff, 
	0xff, 0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0xff, 0xffffffff, 0x292929ff, 
	0x292929ff, 0xffffffff, 0xffffffff, 0xffffffff, 
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
	0xffffffff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff, 
	0x292929ff, 0x292929ff, 0x292929ff, 0x292929ff
};

unsigned long oms_RadioUpData[] = 
{
	0xe7180000, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0xff, 0xff, 0xff, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xe7180000, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0xffffffff, 0xffffffff, 0xffffffff, 
	0xffffffff, 0xffffffff, 0xfd, 0xe7180000, 
	0xe7180000, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0xffffffff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xffffffff, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0xffffffff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xffffffff, 0xfd, 0xe7180000, 
	0xfd, 0xffffffff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0x6b6b6bfd, 
	0xfd, 0xff, 0xffffffff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0x6b6b6bff, 0xff, 0xff, 0xffffffff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0x6b6b6bff, 0xff, 0xff, 
	0xffffffff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0x6b6b6bff, 0xff, 
	0xfd, 0xffffffff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0x6b6b6bfd, 
	0xfd, 0xe7180000, 0xfd, 0x6b6b6bfd, 
	0xcececeff, 0xcececeff, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0x6b6b6bfd, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0x6b6b6bfd, 0xcececeff, 0xcececeff, 
	0xcececeff, 0xcececeff, 0xcececeff, 0x6b6b6bfd, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xe7180000, 0xe7180000, 0xfd, 0x6b6b6bfd, 
	0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bfd, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xe7180000, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0xff, 0xff, 0xff, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xe7180000	
};

unsigned long oms_RadioDownData[] = 
{
	0xe7180000, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0xff, 0xff, 0xff, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xe7180000, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0x6b6b6bff, 0x6b6b6bff, 0x6b6b6bff, 
	0x6b6b6bff, 0x6b6b6bff, 0xfd, 0xe7180000, 
	0xe7180000, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0x6b6b6bff, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0x6b6b6bff, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0x6b6b6bff, 0x9b9b9bff, 0x9b9b9bff, 
	0xff, 0xff, 0xff, 0x9b9b9bff, 
	0x9b9b9bff, 0x6b6b6bff, 0xfd, 0xe7180000, 
	0xfd, 0x6b6b6bff, 0x9b9b9bff, 0x9b9b9bff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0x9b9b9bff, 0x9b9b9bff, 0xfffffffd, 
	0xfd, 0xff, 0x6b6b6bff, 0x9b9b9bff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0x9b9b9bff, 
	0xffffffff, 0xff, 0xff, 0x6b6b6bff, 
	0x9b9b9bff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0x9b9b9bff, 0xffffffff, 0xff, 0xff, 
	0x6b6b6bff, 0x9b9b9bff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0x9b9b9bff, 0xffffffff, 0xff, 
	0xfd, 0x6b6b6bff, 0x9b9b9bff, 0x9b9b9bff, 
	0xff, 0xff, 0xff, 0xff, 
	0xff, 0x9b9b9bff, 0x9b9b9bff, 0xfffffffd, 
	0xfd, 0xe7180000, 0xfd, 0xfffffffd, 
	0x9b9b9bff, 0x9b9b9bff, 0xff, 0xff, 
	0xff, 0x9b9b9bff, 0x9b9b9bff, 0xfffffffd, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0xfffffffd, 0x9b9b9bff, 0x9b9b9bff, 
	0x9b9b9bff, 0x9b9b9bff, 0x9b9b9bff, 0xfffffffd, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xe7180000, 0xe7180000, 0xfd, 0xfffffffd, 
	0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffd, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xe7180000, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xfd, 0xff, 0xff, 0xff, 
	0xfd, 0xe7180000, 0xe7180000, 0xe7180000, 
	0xe7180000	
};

