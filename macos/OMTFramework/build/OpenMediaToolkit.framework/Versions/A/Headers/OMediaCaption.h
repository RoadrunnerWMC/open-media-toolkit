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
#ifndef OMEDIA_Caption_H
#define OMEDIA_Caption_H

#include "OMediaSurfaceElement.h"

#include <string>



class OMediaCaption : public OMediaSurfaceElement
{
	public:

	// * Construction

	omtshared OMediaCaption();	  
	omtshared virtual ~OMediaCaption();


	// * Logic

	omtshared virtual void update_logic(float millisecs_elapsed);
	
	// * Text

	inline void set_string(const string s) {text = s; set_dirty();}
	inline void get_string(string &s) const {s = text;}

	// * Font

	inline void set_font(OMediaFont *f) {font = f; set_dirty();}
	inline OMediaFont *get_font(void) {return font;}

	// * Dirty

	inline void set_dirty(void) {dirty =true;}

	// * Surface

	omtshared virtual void rebuild_surface(void);


	protected:
	
	omtshared virtual void get_canv_wordsize(float &w, float &h);
		
	string				text;
	OMediaFont			*font;
	long				text_w,text_h;
	bool				dirty;
	bool				true_size;
};


#endif

