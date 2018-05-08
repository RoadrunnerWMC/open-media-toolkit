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
#ifndef OMEDIA_StringField_H
#define OMEDIA_StringField_H

#include "OMediaSurfaceElement.h"
#include "OMediaString.h"
#include "OMediaFont.h"
#include "OMediaFocus.h"

class OMediaEvent;

#include <string>

enum omt_KeyFilter
{
	omckf_UnsignedInteger,
	omckf_Integer,
	omckf_Double,
	omckf_AlphaNumeric,
	omckf_AlphaNumericExtended,
	omckf_NoFilter
};


class OMediaStringField : 	public OMediaSurfaceElement,
				 			public OMediaBroadcaster,
				 			public OMediaFocus
{
	public:

	// * Construction

	omtshared OMediaStringField();	  
	omtshared virtual ~OMediaStringField();
	
	// * Attributes

	omtshared virtual void set_string(const string str);
	inline void get_string(string &str) const {str = its_string;}
	
	inline long get_long(void) const {return OMediaStringTools::string2long(its_string);}
	inline float get_float(void) const {return (float)OMediaStringTools::string2double(its_string);}

	inline void set_long(const long l) {set_string(OMediaStringTools::long2string(l));}
	inline void set_float(const float f) {set_string(OMediaStringTools::double2string(f));}


	// * Element Size
	
	inline void set_size(short w, short h) {width = w; height = h; purge_surface();}
	inline void get_size(short &w, short &h) const {w = width; h = height;}
 

	// * Message to send to listener when text modified

	inline void set_message(omt_Message m) {msg = m;}
	inline omt_Message get_message(void) {return msg;}

	inline void set_simple_click_message(omt_Message m) {sclick_msg = m;}
	inline omt_Message get_simple_click_message(void) {return sclick_msg;}

	inline void set_validate_message(omt_Message m) {valide_msg = m;}
	inline omt_Message get_validate_message(void) {return valide_msg;}


	// * Font

	inline void set_font(OMediaFont *qf) {font = qf; purge_surface();}
	inline OMediaFont *get_font(void) const {return font;}
	

	// * Messages

	omtshared virtual void listen_to_message(omt_Message msg, void *param =omc_NULL);
	omtshared virtual bool update_message_state(omt_Message msg, 
												bool &enabled,
												bool &mark);


	// * Border size
	
	inline void set_border_size(short bs) {border_size = bs; purge_surface();}
	inline short get_border_size(void) const {return border_size; }

	// * Double-click for selection
	
	inline void set_dclick_selection(bool b) {dclick_select = b;}
	inline bool get_dclick_selection(void) const {return dclick_select;}

	// * Color used by the default shape

	inline void set_shine_color(const OMediaFARGBColor &r) 	{shine = r; purge_surface();}
	inline void set_dark_color(const OMediaFARGBColor &r) 	{dark = r; purge_surface();}
	inline void set_back_color(const OMediaFARGBColor &r) 	{back = r; purge_surface();}
	inline void set_selection_color(const OMediaFARGBColor &r) {selection = r; purge_surface();}
	inline void set_cursor_color(const OMediaFARGBColor &r) {cursor = r; purge_surface();}

	inline void get_shine_color(OMediaFARGBColor &r) const {r = shine;}
	inline void get_dark_color(OMediaFARGBColor &r) const {r = dark;}
	inline void get_back_color(OMediaFARGBColor &r) const {r = back;}
	inline void get_cursor_color(OMediaFARGBColor &r) const {r = cursor;}


	// * Called when the element is clicked

	omtshared virtual void clicked(OMediaPickResult *res, bool mouse_down);


	// * Key filter
	
	inline void set_default_filter(omt_KeyFilter kf) {keyfilter = kf;}
	inline omt_KeyFilter get_default_filter(void) const {return keyfilter;}
	
	// * Transparent when deselected
	
	inline void set_deselected_transp(bool t) {deselected_transp = t; purge_surface();}
	inline bool get_deselected_transp(void) {return deselected_transp;}

	// * Auto-resize the object
	
	inline void set_auto_resize(bool a, short maxsize) {auto_resize = a; auto_maxsize = maxsize; purge_surface();}
	inline bool get_auto_resize(void) const {return auto_resize;}

	// * Custom parameter message (if null message parameter points to this object)
	
	inline void set_custom_param(void *p) {custom_param = p;}
	inline void *get_custom_param(void) const {return custom_param;}


	// * Delay before focus

	inline void set_focus_delay(float millisecs) {select_delay = millisecs;}
	inline float get_focus_delay(void) const {return select_delay;}
	inline void abort_focus_delay(void) 
	{
		select_delay_started = false; 
		if (select_waiting_field==this) select_waiting_field =omc_NULL;
	}

	// * Override 

	omtshared virtual void update_logic(float millisecs_elapsed);
	omtshared virtual void rebuild_surface(void);


	// * Scroll offset (in pixels)

	inline void set_scroll_offset(const long o) {scroll_offset = o; purge_surface();}
	inline long get_scroll_offset() const {return scroll_offset;}


	// * Selection (in characteres)
	
	inline void set_selection(long start, long end) {selection_start = start; selection_end = end; check_selection_bounds();}
	inline void get_selection(long &start, long &end) const {start = selection_start; end = selection_end;}

	omtshared virtual void scroll_to_selection(bool scroll_to_sel_end =false);
	
	inline void select_all(void) {selection_start = 0; selection_end = its_string.size(); purge_surface();}

	inline bool all_selected(void) const {return (selection_start==0 && selection_end==(long)its_string.size() && its_string.size());}
	
	inline bool has_selection(void) const {return selection_start<selection_end;}

	omtshared virtual void erase_selection(void);


	// * Select all when focus (default is true)
	
	inline void set_select_all_focus(bool s) {select_all_focus = s;}
	inline bool get_select_all_focus(void) const {return select_all_focus;}

	// * Maximum string length (0 for unlimited)

	inline void set_max_length(long m) {max_length = m; check_length();}
	inline long get_max_length(void) const {return max_length;}
	
	// * Clipboard
	
	omtshared virtual void clip_copy(void);
	omtshared virtual void clip_cut(void);
	omtshared virtual void clip_paste(void);


	//.....................................

	protected:

	omtshared virtual void update_focus(bool on);
	
	omtshared virtual void draw_text(OMediaCanvas &dp);
	omtshared virtual void draw_selection(OMediaCanvas &dp);
	
	omtshared virtual void recalc_size(void);

	omtshared virtual bool process_keyfilter(const OMediaEvent *event);

	omtshared virtual void start_selecting(long x, long y);
	omtshared virtual void process_selecting(float elapsed);

	omtshared virtual void get_canv_wordsize(float &w, float &h);

	inline void check_selection_bounds(void)
	{	
		if (selection_start>(long)its_string.size()) selection_start = its_string.size();
		if (selection_start<0) selection_start = 0;

		if (selection_end>(long)its_string.size()) selection_end = its_string.size();
		if (selection_end<0) selection_end = 0;	
		
		if (selection_end<selection_start) selection_end = selection_start;
	}

	inline void check_length(void)
	{
		if (max_length!=0)
		{
			if ((long)its_string.size()>max_length)
			{
				its_string.resize(max_length);
				purge_surface();
				check_selection_bounds();
			}
		}
	}

	long str2pix(long stroff);
	long pix2str(long pix);
	
	inline void show_cursor(void)
	{
		millisec_count = OMediaTimeCounter::get_millisecs();
		display_cursor = true;
	}

	omt_Message			msg,sclick_msg,valide_msg;
	short				width,height;
	short				border_size;
	OMediaFARGBColor	shine,dark,back,selection,cursor;
	string				its_string;
	OMediaFont		 	*font;
	bool				display_cursor;
	float				millisec_count;
	omt_KeyFilter		keyfilter;
	bool				deselected_transp,auto_resize,dclick_select, select_all_focus;
	short				auto_maxsize,cursor_width;
	unsigned long		lastclick_millisec;
	void				*custom_param;
	OMediaTimer			select_timer;
	float				select_delay;
	bool				select_delay_started;
	
	long				scroll_offset;
	long				selection_start, selection_end;
	
	bool				m_selecting;
	short				m_selecting_x,m_selection_y;
	long				m_selecting_bx,m_selecting_by;
	float				m_selection_scroll_time;
	
	long				last_x,last_y;
	long				max_length;

	omtshared static OMediaStringField	*select_waiting_field;
};


#endif

