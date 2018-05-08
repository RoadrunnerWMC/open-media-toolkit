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
#ifndef OMEDIA_Scroller_H
#define OMEDIA_Scroller_H

#include "OMediaSurfaceElement.h"


class OMediaScroller : 	public OMediaSurfaceElement,
		 				public OMediaBroadcaster
{
	public:

	// * Construction

	omtshared OMediaScroller();	  
	omtshared virtual ~OMediaScroller();
	
	
	// * Attributes
	
		// Horizontal
	
	inline void set_htotal(float htot) {htotal = htot; purge_surface();}
	inline void set_hrange(float pos, float nunits)  {hpos = pos; hnunits = nunits ;purge_surface();}

	inline float get_htotal(void) const {return htotal;}
	omtshared virtual float get_hpos(void) const;
	inline float get_hnunits(void) const  {return hnunits; }


		// Vertical

	inline void set_vtotal(float vtot) {vtotal = vtot; purge_surface();}
	inline void set_vrange(float pos, float nunits)  {vpos = pos; vnunits = nunits, purge_surface();}

	inline float get_vtotal(void) const {return vtotal;}
	omtshared virtual float get_vpos(void) const;
	inline float get_vnunits(void) const  {return vnunits; }

	
	// * Element Size
	
	inline void set_size(short w, short h) {width = w; height = h; purge_surface();}
	inline void get_size(long &out_width, long &out_height) const {out_width = width; out_height = height;}
 
 	inline void set_min_knob_size(short mknob_w, short mknob_h) {min_knob_w =  mknob_w; min_knob_h =  mknob_h;	purge_surface();}
  	inline short get_min_knob_width(void) {return min_knob_w;}
  	inline short get_min_knob_height(void) {return min_knob_h;}
 

	// * Message to send to listener

	inline void set_message(omt_Message m) {msg = m;}
	inline omt_Message get_message(void) {return msg;}


	// * Called when the element is clicked

	omtshared virtual void clicked(OMediaPickResult *res, bool mouse_down);


	// * Border size
	
	inline void set_border_size(short bs) {border_size = bs; purge_surface();}
	inline short get_border_size(void) const {return border_size; }


	// * Color used by the default shape

	inline void set_shine_color(const OMediaFARGBColor &r) {shine = r; purge_surface();}
	inline void set_dark_color(const OMediaFARGBColor &r) {dark = r; purge_surface();}
	inline void set_fill_color(const OMediaFARGBColor &r) {fill = r; purge_surface();}
	inline void set_shadow_color(const OMediaFARGBColor &r) {shadow = r; purge_surface();}
	inline void set_back_fill_color(const OMediaFARGBColor &r) {back_fill = r; purge_surface();}

	inline void get_shine_color(OMediaFARGBColor &r) const {r = shine;}
	inline void get_dark_color(OMediaFARGBColor &r) const {r = dark;}
	inline void get_fill_color(OMediaFARGBColor &r) const {r = fill;}
	inline void get_shadow_color(OMediaFARGBColor &r) const {r = shadow;}
	inline void get_back_fill_color(OMediaFARGBColor &r) const {r = back_fill;}

	// * Override 

	omtshared virtual void update_logic(float millisecs_elapsed);
	omtshared virtual void rebuild_surface(void);


	//.....................................

	protected:
	
	omtshared virtual void compute_knob_rect(OMediaRect &bounds, OMediaRect &r);
	
	omtshared virtual void draw_knob(OMediaCanvas &dp, OMediaRect &knob_r, OMediaRect &bounds);
	omtshared virtual void draw_back(OMediaCanvas &dp, OMediaRect &bounds);
	
	omtshared virtual void move_knob_to(short x, short y, short px, short py);
	omtshared virtual void stop_move(void);

	omtshared virtual void get_canv_wordsize(float &w, float &h);

	omtshared virtual bool isinknob(long px, long py);


	omt_Message			msg;
	short				width,height;
	short				min_knob_w, min_knob_h;
	float 				htotal, hpos, hnunits, vtotal, vpos, vnunits;
	short				border_size;
	short				delta_x,  delta_y;
	OMediaFARGBColor	shine,dark,fill,shadow,back_fill;
	short				clickox,clickoy;
	
	bool				selected;

};


#endif

