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
#ifndef OMEDIA_3DShapeAnim_H
#define OMEDIA_3DShapeAnim_H

#include "OMedia3DShapeElement.h" 
#include "OMediaAnim.h"

class OMedia3DShape;
class OMedia3DShapeAnim;

//-----------------------------------------------------
// Definition

class OMedia3DShapeAnimDef : public OMediaAnimDef
{
	public:
	
	// * Constructor/Destructor

	omtshared OMedia3DShapeAnimDef();
	omtshared virtual ~OMedia3DShapeAnimDef();

	// Create and link new frame
	omtshared virtual OMediaAnimFrame *create_frame(long sequence);

	// * Database/streamer support
	
	enum { db_type = 'A3ds' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void) const;
};



//-----------------------------------------------------
// Element

class OMedia3DShapeAnim :	public OMedia3DShapeElement,
							public OMediaAnim
{
	public:
	
	// * Constructor/Destructor

	omtshared OMedia3DShapeAnim();
	omtshared virtual ~OMedia3DShapeAnim();
	

	// * Anim definition

	omtshared virtual void set_anim_def(OMedia3DShapeAnimDef *def);


	// * Update logic
	
	omtshared virtual void update_logic(float elapsed);

	// * Override
	
	omtshared virtual void pause(bool p);
	omtshared virtual void get_dynamic_offset(float &x, float &y, float &z);

	// * Dependencies:
	
	omtshared virtual void unlink_shape_from_structure(OMedia3DShape *shape);
	omtshared virtual void unlink_animdef_from_structure(OMediaAnimDef *animdef);

	protected:

	omtshared virtual void render_draw_shape(OMediaRendererInterface *rdr_i, OMedia3DShape	*shape, bool &pass2);

};

#endif

 