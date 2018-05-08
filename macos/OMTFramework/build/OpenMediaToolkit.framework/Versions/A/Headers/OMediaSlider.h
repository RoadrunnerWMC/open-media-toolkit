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
#ifndef OMEDIA_Slider_H
#define OMEDIA_Slider_H

#include "OMediaSurfaceElement.h"


enum omt_SliderMode
{
	omsmc_Horizontal,		// Default
	omsmc_Vertical
};

class OMediaSlider : 	public OMediaSurfaceElement,
		 				public OMediaBroadcaster
{
	public:

	// * Construction

	omtshared OMediaSlider();	  
	omtshared virtual ~OMediaSlider();
	
	
	// * Attributes

	inline void set_slider_mode(omt_SliderMode m) {smode = m; purge_surface();}
	inline omt_SliderMode get_slider_mode(void) const {return smode;}

	inline void set_maxvalue(float t) {maxvalue = t; purge_surface();}
	inline float get_maxvalue(void) const {return maxvalue;}

	inline void set_value(float v) {value = v; purge_surface();}
	inline float get_value(void) const {return value;}

	
	// * Element Size
	
	inline void set_size(short w, short h) {width = w; height = h; purge_surface();}
	inline void get_size(long &w, long &h) const {w=width;h=height;}
 

	// * Message sent when value changed

	inline void set_message(omt_Message m) {msg = m;}
	inline omt_Message get_message(void) {return msg;}


	// * Called when the element is clicked

	omtshared virtual void clicked(OMediaPickResult *res, bool mouse_down);


	// * Color used by the default shape

	inline void set_shine_color(const OMediaFARGBColor &r) {shine = r; purge_surface();}
	inline void set_dark_color(const OMediaFARGBColor &r) {dark = r; purge_surface();}
	inline void set_fill_color(const OMediaFARGBColor &r) {fill = r; purge_surface();}
	inline void set_back_fill_color(const OMediaFARGBColor &r) {back_fill = r; purge_surface();}

	inline void get_shine_color(OMediaFARGBColor &r) const {r = shine;}
	inline void get_dark_color(OMediaFARGBColor &r) const {r = dark;}
	inline void get_fill_color(OMediaFARGBColor &r) const {r = fill;}
	inline void get_back_fill_color(OMediaFARGBColor &r) const {r = back_fill;}

	// * Override 

	omtshared virtual void update_logic(float millisecs_elapsed);
	omtshared virtual void rebuild_surface(void);


	//.....................................

	protected:
		
	omtshared virtual void draw_knob(OMediaCanvas &dp, OMediaRect &knob_r, OMediaRect &bounds);
	omtshared virtual void draw_back(OMediaCanvas &dp, OMediaRect &bounds);
		
	omtshared virtual bool isinknob(long px, long py);

	omtshared virtual void move_knob_to(short x, short y, short px, short py);
	omtshared virtual void stop_move(void);

	omtshared virtual void compute_knob_rect(OMediaRect &bounds, OMediaRect &r);

	omtshared virtual void get_canv_wordsize(float &w, float &h);


	omt_SliderMode  	smode;
	omt_Message			msg;
	short				width,height;
	float 				maxvalue,value,base_value;
	short				delta;
	OMediaFARGBColor	shine,dark,fill,back_fill;
	short				clicko;
	bool				selected;
        int				knob_length;
};


#endif

