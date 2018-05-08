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
#ifndef OMEDIA_StdButton_H
#define OMEDIA_StdButton_H

#include "OMediaSurfaceElement.h"
#include "OMediaAbstractButton.h"

enum omt_StdButtonStyle
{
	ombtnc_Standard,
	ombtnc_Checkbox,
	ombtnc_Radio
};




class OMediaStdButton : 	public OMediaSurfaceElement,
							public OMediaAbstractButton
{
	public:

	// * Construction

	omtshared OMediaStdButton();
	omtshared virtual ~OMediaStdButton();
	
	// * Attributes

	inline void set_string(const string str) {text = str; purge_surface();}
	inline void get_string(string &str) const {str = text;}

	inline void set_style(omt_StdButtonStyle s) {style = s; purge_surface(); set_toggle_mode(style!=ombtnc_Standard);}
	inline omt_StdButtonStyle get_style(void) const {return style;}

		// (0,0) for auto-size
	inline void set_button_size(long width, long height)
	{
		btn_width = width;
		btn_height = height;
		purge_surface();
	}	
	
	inline long get_button_width(void) const {return btn_width;}
	inline long get_button_height(void) const {return btn_height;}


	inline void set_font(OMediaFont *f) {font = f;purge_surface();}
	inline OMediaFont *get_font(void) {return font;}


	// * Messages

	omtshared virtual void listen_to_message(omt_Message msg, void *param);

	// * Called when the element is clicked

	omtshared virtual void clicked(OMediaPickResult *res, bool mouse_down);

	// * Toggle mode ( see also OMediaAbstractButton)

	omtshared virtual void select(void);
	omtshared virtual void deselect(void);

	// * Color used by the default shape

	inline void set_shine_color(const OMediaFARGBColor &r) {shine = r; purge_surface();}
	inline void set_dark_color(const OMediaFARGBColor &r) {dark = r; purge_surface();}
	inline void set_fill_up_color(const OMediaFARGBColor &r) {fill_up = r; purge_surface();}
	inline void set_fill_down_color(const OMediaFARGBColor &r) {fill_down = r; purge_surface();}

	inline void get_shine_color(OMediaFARGBColor &r) const {r = shine;}
	inline void get_dark_color(OMediaFARGBColor &r) const {r = dark;}
	inline void get_fill_up_color(OMediaFARGBColor &r) const {r = fill_up;}
	inline void get_fill_down_color(OMediaFARGBColor &r) const {r = fill_down;}
	
	protected:

	omtshared virtual void rebuild_surface(void);	
	omtshared virtual void get_canv_wordsize(float &w, float &h);

	string						text;
	omt_StdButtonStyle			style;
	long						btn_width,btn_height;
	OMediaFont					*font;

	OMediaFARGBColor			dark,shine,fill_up,fill_down;
	

	enum omt_StdButtonInternalState
	{
		ombistc_Empty,
		ombistc_Down,
		ombistc_Up
	} 
	internal_state;
};


#endif

