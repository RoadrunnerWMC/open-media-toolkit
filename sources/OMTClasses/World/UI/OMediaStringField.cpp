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
 
#include "OMediaStringField.h"
#include "OMediaEventManager.h"
#include "OMediaPickRequest.h"
#include "OMediaWorld.h"
#include "OMediaInputEngine.h"
#include "OMediaClipboard.h"
#include "OMediaMemTools.h"

OMediaStringField	*OMediaStringField::select_waiting_field;

OMediaStringField::OMediaStringField()
{
	set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);

	sclick_msg = msg = valide_msg = omsg_NULL;
		
	width = height = 0;
	border_size = 3;
	cursor_width = 5;
	auto_resize = true;
	dclick_select = false;
	select_delay = 0;
	select_delay_started = false;
	select_all_focus = false;
	scroll_offset = 0;
	selection_start = selection_end = 0;
	m_selecting = false;
	max_length = 0;

	back.alpha = 1.0f; 			back.red = back.green = back.blue = float(0xEEEE)/float(0xFFFF);	
	dark.alpha = 1.0f; 			dark.red = dark.green = dark.blue = float(0x6666)/float(0xFFFF);
	shine.alpha = 1.0f; 		shine.red = shine.green = shine.blue = float(0xFFFF)/float(0xFFFF);	
	selection.alpha = 1.0f; 	selection.red = selection.green = selection.blue = float(0xAAAA)/float(0xFFFF);
	cursor.alpha = 1.0f; 		cursor.red = cursor.green = cursor.blue = float(0)/float(0xFFFF);

	font = NULL;
	display_cursor = false;
	millisec_count = -1;
	keyfilter = omckf_AlphaNumericExtended;
	set_deselected_transp(true);
	
	auto_maxsize = 640;
	lastclick_millisec = 0;
	
	custom_param = NULL;
}
	  
OMediaStringField::~OMediaStringField()
{
	if (select_waiting_field==this) select_waiting_field = NULL;
}
	
void OMediaStringField::set_string(const string str)
{
	its_string = str;
	check_length();
	purge_surface();
}

void OMediaStringField::clicked(OMediaPickResult *res, bool mouse_down)
{
	unsigned long cur_millisec = OMediaTimeCounter::get_millisecs();
	long x = long(res->sub_info[0].u * float(surf_buffer.get_width())),
		 y = long(res->sub_info[0].v * float(surf_buffer.get_height()));

	if (dclick_select)
	{				
		if (lastclick_millisec)
		{
			if (cur_millisec - lastclick_millisec <= 600) 
			{
				if (!(get_focus_flags()&omfof_DisableFocus)) set_focus(true);
				else broadcast_message(OMediaStringField::msg,(custom_param)?custom_param:this);
			}
			else if (sclick_msg != omsg_NULL) broadcast_message(sclick_msg,(custom_param)?custom_param:this);
		}
		else if (sclick_msg != omsg_NULL) broadcast_message(sclick_msg,(custom_param)?custom_param:this);
				
	}
	else 
	{
		if (!(get_focus_flags()&omfof_DisableFocus)) 
		{
			if (select_delay!=0)
			{
				if (!is_focus())
				{
					OMediaFocus::remove_focus();

					if (select_waiting_field!=this)
					{
						if (select_waiting_field) select_waiting_field->abort_focus_delay();
						select_waiting_field = this;
						select_delay_started = true;
						select_timer.start();
					}
				}
				else
				{
					if (lastclick_millisec && cur_millisec - lastclick_millisec <= 600 &&
						!all_selected() && abs(last_x-x)<2 && abs(last_y-y)<2) 	select_all();
					else start_selecting(x,y);
				}
			}
			else 
			{
				set_focus(true);
				if (lastclick_millisec && cur_millisec - lastclick_millisec <= 600 &&
						!all_selected() && abs(last_x-x)<2 && abs(last_y-y)<2) select_all();

				else start_selecting(x,y);
			}
		}
		else broadcast_message(OMediaStringField::msg,(custom_param)?custom_param:this);
	}

	lastclick_millisec = cur_millisec;
	last_x = x;
	last_y = y;
}

 
void OMediaStringField::listen_to_message(omt_Message msg, void *param)
{
	OMediaEvent *event =  (OMediaEvent *)param;

	if (getvisible_extend())
	{
		switch(msg)
		{
			case omsg_HasFocus:
			{
				bool	*p = (bool*)param;
				*p = is_focus();
			}
			break;
			
			case omsg_Cut:
			clip_cut();
			break;

			case omsg_Copy:
			clip_copy();
			break;

			case omsg_Paste:
			clip_paste();
			break;

			case omsg_Clear:
			erase_selection();
			break;

			case omsg_SelectAll:
			select_all();
			break;
		
			case omsg_Event:
			if (OMediaFocus::is_focus() && event->type==omtet_KeyDown)
			{
				if (event->special_key!=omtsk_Null)
				{
					switch (event->special_key)
					{
						case omtsk_Enter:
						case omtsk_Return:
						set_focus(false);
						break;
						
						case omtsk_Backspace:
						if (its_string.length())
						{
							if (selection_start>=selection_end)
							{
								if (selection_start>0)
								{
									its_string.erase(selection_start-1,1);
									selection_start--;
									selection_end = selection_start;
								}
							}
							else
							{
								if (selection_start>=0 && selection_end<=(long)its_string.length())
									its_string.erase(selection_start, (selection_end-selection_start));	
									
									selection_end = selection_start;
							}

							check_selection_bounds();
							
							show_cursor();
							purge_surface();
							scroll_to_selection();
						}
						break;
						
						case omtsk_Delete:
						if (its_string.length())
						{
							if (selection_start>=selection_end)
							{
								if (selection_start<(long)its_string.length())
									its_string.erase(selection_start,1);
							}
							else
							{
								if (selection_start>=0 && selection_end<=(long)its_string.length())
									its_string.erase(selection_start, (selection_end-selection_start));	
									
									selection_end = selection_start;
							}

							check_selection_bounds();
						
							show_cursor();
							purge_surface();
							scroll_to_selection();
						}
						break;
						
						case omtsk_ArrowLeft:
						if (!(event->command_key&omtck_Shift))
						{
							if (selection_start>=selection_end)
							{
								selection_start--;
								if (selection_start<0) selection_start = 0;
							}
							selection_end = selection_start;					
						}
						else
						{
							selection_start--;
							if (selection_start<0) selection_start = 0;
						}
						show_cursor();
						purge_surface();
						scroll_to_selection();
						break;

						case omtsk_ArrowRight:
						if (!(event->command_key&omtck_Shift))
						{
							if (selection_start<selection_end)
							{
								selection_start = selection_end;
							}
							else selection_start++;
							
							if (selection_start>(long)its_string.length()) selection_start = its_string.length();
							selection_end = selection_start;
							scroll_to_selection();
						}
						else
						{
							selection_end++;
							if (selection_end>(long)its_string.length()) selection_end = its_string.length();						
							scroll_to_selection(true);
						}							
							
						show_cursor();
						purge_surface();
						break;
						
						case omtsk_ArrowTop:
						selection_start = 0;
						if (!(event->command_key&omtck_Shift)) selection_end = selection_start;
						show_cursor();
						purge_surface();
						scroll_to_selection();
						break;
						
						case omtsk_ArrowBottom:
						if (!(event->command_key&omtck_Shift))
						{
							selection_start=its_string.length();
							selection_end = selection_start;
							scroll_to_selection();
						}
						else
						{
							selection_end=its_string.length();
							scroll_to_selection(true);
						}
							
						show_cursor();
						purge_surface();
						
						break;
                                                
                                                default: break;
					}
				}
				else if ( (event->command_key&(omtck_Command|omtck_Control)) ==0 )
				{
					if (process_keyfilter(event))
					{
						char	tstr[2];
						
						tstr[0] = event->ascii_key;
						tstr[1] = 0;
						
						if (selection_start<selection_end)
						{
							if (selection_start>=0 && selection_end<=(long)its_string.length())
									its_string.erase(selection_start, (selection_end-selection_start));	
		
							selection_end = selection_start;
							check_selection_bounds();
						}
						
						its_string.insert(selection_start,tstr);
						selection_start++;
						selection_end = selection_start;
						check_selection_bounds();

						show_cursor();
						purge_surface();
						scroll_to_selection();
					}
				}
			}
			
			break;
		}

		check_length();
	}
	else if (is_focus()) set_focus(false);
}

void OMediaStringField::update_logic(float millisecs_elapsed)
{
	unsigned long	cur_millisec;

	if (select_delay_started)
	{
		if (select_timer.getelapsed()>=select_delay)
		{
			select_waiting_field = NULL;
			select_delay_started = false;
			set_focus(true);
		}
	}

	if (is_focus())
	{	
		if (getvisible())
		{
			cur_millisec = OMediaTimeCounter::get_millisecs();
		
			if (millisec_count==-1) millisec_count = cur_millisec;
			else
			{
				if (cur_millisec - millisec_count>=600) 
				{
					millisec_count = cur_millisec;
					display_cursor = !display_cursor;
					purge_surface();
				}
			}
			
			process_selecting(millisecs_elapsed);
		}
		else set_focus(false);
	}
}

void OMediaStringField::update_focus(bool on)
{
	if (on)
	{
		display_cursor = true;
		millisec_count=-1;
		if (select_all_focus)
		{
			selection_start = 0;
			selection_end = its_string.size();
		}

		purge_surface();		
	}
	else
	{
		if (auto_resize) scroll_offset = 0;
		display_cursor = false;
		purge_surface();	
		if (valide_msg) broadcast_message(valide_msg,(custom_param)?custom_param:this);
	}
}

long OMediaStringField::str2pix(long stroff)
{
	if (!font) return 0;

	if (stroff>=(long)its_string.length())
	{
		return font->get_text_length(its_string);
	}
	else if (stroff<=0) return 0;
	
	string	str = its_string;
	str.resize(stroff);

	return font->get_text_length(str);	
}

long OMediaStringField::pix2str(long pix)
{
	if (!font) return 0;

	long	l;
	
	string	str;
	for(l=0;l<(long)its_string.size(); l++)
	{
		str += its_string[l];
		if (pix<font->get_text_length(str)) return l;
	}
	
	return l;
}

void OMediaStringField::draw_selection(OMediaCanvas &dp)
{
	if (!is_focus()) return;

	if (selection_start>=selection_end)
	{
		// No selection, simple cursor

		if (display_cursor) 
		{
			long	pix;
			pix = (str2pix(selection_start) - scroll_offset) + border_size;
		
			dp.draw_line(cursor,pix,0,pix,height);
		}
	}
	else
	{
		OMediaRect	r;
		
		if (selection_start==0 && selection_end==(long)its_string.size() && selection_start<selection_end)
		{
			r.set(0,0,width,height);
		}
		else
		{
			r.set(str2pix(selection_start), 0, str2pix(selection_end), height);
			r.offset(-scroll_offset + border_size,0);		
		}
	
		dp.paint_rect(selection,r);	
	}
}

void OMediaStringField::draw_text(OMediaCanvas &dp)
{
	if (font)
	{
		draw_selection(dp);
		dp.draw_string(its_string, (border_size)-scroll_offset, border_size,font,omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
	}
}

void OMediaStringField::rebuild_surface(void)
{
	OMediaRect		bounds;

	if (!font) purge_surface();
	else
	{
		bool	do_draw_frame;
	
		if (auto_resize) recalc_size();

		surf_buffer.create(width,height);
		
		bounds.set(0,0,width,height);
		
		do_draw_frame = (!deselected_transp || is_focus() || its_string.length()==0);
		
		if (do_draw_frame) surf_buffer.fill(back);
		else surf_buffer.fill_alpha(0);
		
		draw_text(surf_buffer);
		
		if (do_draw_frame) surf_buffer.frame_emboss(bounds,false,dark,shine);
	}
}

void OMediaStringField::recalc_size(void)
{
	if (font) 
	{
		width = font->get_text_length(its_string) + (border_size<<1) + cursor_width;
		if (width>auto_maxsize) width = auto_maxsize;
	}
}

void OMediaStringField::scroll_to_selection(bool scroll_to_sel_end)
{	
	long pix;
	
	pix = (str2pix((scroll_to_sel_end)?selection_end:selection_start)) - scroll_offset;
	if (pix<0) //border_size)
	{
		scroll_offset += pix;	// - (border_size<<1); 
		purge_surface();
	}
	else if (pix> (width - (border_size<<1)))
	{
		scroll_offset += (pix-width) + (border_size<<1);
		purge_surface();
	}
	
	if (scroll_offset<0) scroll_offset = 0;
}

void OMediaStringField::erase_selection(void)
{
	if (its_string.length() && selection_start<selection_end)
	{
		if (selection_start>=0 && selection_end<=(long)its_string.length())
			its_string.erase(selection_start, (selection_end-selection_start));	
			
		selection_end = selection_start;

		check_selection_bounds();
		
		show_cursor();
		purge_surface();
		scroll_to_selection();				
	}
}

void OMediaStringField::start_selecting(long x, long y)
{
	OMediaInputEngine	*input = its_world->get_input_engine();

	if (!its_world || !input) return;

	m_selecting = true;	
	input->get_mouse_position(m_selecting_x,m_selection_y);

	m_selecting_bx = x;
	m_selecting_by = y;
	m_selection_scroll_time = 0;

	selection_start = selection_end = pix2str((m_selecting_bx-border_size)+scroll_offset);
	
	purge_surface();
}

void OMediaStringField::process_selecting(float elapsed)
{
	OMediaInputEngine	*input = its_world->get_input_engine();

	if (!its_world || !m_selecting || !input) return;

	if (!input->mouse_down())
	{
		m_selecting = false;
		return;
	}

	short	dx,dy;
	long	old_scroll;

	input->get_mouse_position(dx,dy);
	dx -= m_selecting_x;

	long	nx;
	
	nx = m_selecting_bx + dx;
	old_scroll = scroll_offset;

	if (nx<0) 
	{
		m_selection_scroll_time += elapsed;
		if (m_selection_scroll_time<60) return;
		
		selection_start--;
		if (selection_start<0) selection_start = 0;
		scroll_to_selection();
		m_selection_scroll_time = 0;


	}
	else if (nx>width) 
	{
		m_selection_scroll_time += elapsed;
		if (m_selection_scroll_time<60) return;

		selection_end++;
		if (selection_end>(long)its_string.size()) selection_end = its_string.size();
		scroll_to_selection(true);
		m_selection_scroll_time = 0;
	}

	long	p;
		
	p = pix2str((nx-border_size)+scroll_offset);
	if (dx<=0) selection_start = p;
	else if (dx>0) selection_end = p;

	m_selecting_bx -= scroll_offset - old_scroll;
	m_selecting_x -= scroll_offset - old_scroll;
		
	check_selection_bounds();
	purge_surface();
}

bool OMediaStringField::process_keyfilter(const OMediaEvent *event)
{
	char		keychar = event->ascii_key;
	
	if (keyfilter&omckf_UnsignedInteger)
	{
		if (keychar>= '0' && keychar<= '9') return true;
		return false;
	}

	if (keyfilter== omckf_Integer || keyfilter==omckf_Double)
	{
		if ((keychar>= '0' && keychar<= '9') || (keychar=='.' && keyfilter==omckf_Double))
		{
			if (keychar=='.' && its_string.find('.')!=string::npos) return false;			
			return true; 
		}
		else if (keychar=='-')
		{
			return (its_string.length()==0);
		}
	
		return false;
	}	
	
	
	if (keyfilter==omckf_AlphaNumeric || keyfilter==omckf_AlphaNumericExtended)
	{
		if (keychar>= '0' && keychar<= '9') return true;
		if (keychar>= 'a' && keychar<= 'z') return true;
		if (keychar>= 'A' && keychar<= 'Z') return true;
		if (keychar=='Ž' || 
			keychar=='ˆ' ||
			keychar=='' ||
			keychar=='Ÿ' ||
			keychar=='Š' ||
			keychar=='š' ||
			keychar=='•' ||
			keychar=='' ||
			keychar=='™' ||
			keychar=='' ||
			keychar=='‰' ||
			keychar=='”' ||
			keychar=='ž' ||
			keychar=='“' ||
			keychar=='‹' ||
			keychar=='›' ||
			keychar=='˜' ||
			keychar=='' ||
			keychar==' ') return true;		

		if (keyfilter==omckf_AlphaNumericExtended)
		{
			if (keychar=='.' ||
				keychar=='-' ||
				keychar=='(' ||
				keychar==')' ||
				keychar=='@' ||
				keychar==';' ||
				keychar==',' ||
				keychar=='#' ||
				keychar=='"' ||
				keychar=='%' ||
				keychar=='$' ||
				keychar=='*' ||
				keychar=='+' ||
				keychar=='[' ||
				keychar==']' ||
				keychar=='{' ||
				keychar=='}' ||
				keychar=='!' ||
				keychar=='?' ||
				keychar=='/' ||
				keychar=='\\' ||
				keychar=='"' ||
				keychar=='\'' ||
				keychar=='$' ||
				keychar=='_' ||
				keychar=='-' ||
				keychar=='>' ||
				keychar=='<' ||
				keychar=='~')	return true;		
		}

		return false;
	}
	
	return true;
}

void OMediaStringField::clip_copy(void)
{
	if (has_selection() && its_string.size())
	{
		OMediaMoveableMem	mem;
		
		mem.setsize(selection_end-selection_start);
		OMediaMemTools::copy(its_string.c_str()+selection_start,mem.lock(),selection_end-selection_start);
		mem.unlock();
	
		OMediaClipboard::open();
		OMediaClipboard::clear();
		OMediaClipboard::add_clip(omclip_Text,mem);		
		OMediaClipboard::close();
	}
}

void OMediaStringField::clip_cut(void)
{
	clip_copy();
	erase_selection();
}

void OMediaStringField::clip_paste(void)
{
	OMediaClipboard::open();
	if (OMediaClipboard::clip_exist(omclip_Text))
	{
		OMediaMoveableMem	mem;
		OMediaClipboard::get_clip(omclip_Text, mem); 
		
		erase_selection();

		its_string.insert(selection_start,(char*)mem.lock(),mem.getsize());
		mem.unlock();

		selection_start += mem.getsize();
		selection_end = selection_start;

		check_length();
		check_selection_bounds();
		scroll_to_selection();
		show_cursor();
		purge_surface();
	}

	OMediaClipboard::close();	
}

bool OMediaStringField::update_message_state(omt_Message msg, 
												bool &enabled,
												bool &mark)
{
	switch(msg)
	{
		case omsg_Cut:
		case omsg_Copy:
		case omsg_Clear:
		enabled = has_selection();
		break;

		case omsg_Paste:
		OMediaClipboard::open();
		enabled = OMediaClipboard::clip_exist(omclip_Text);
		OMediaClipboard::close();
		break;

		case omsg_SelectAll:
		enabled = its_string.size() && !all_selected();
		break;
	
		default:
		return false;
	}

	return true;
}

void OMediaStringField::get_canv_wordsize(float &w, float &h)
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