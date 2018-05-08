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
#ifndef OMEDIA_3DMorphAnim_H
#define OMEDIA_3DMorphAnim_H

#include "OMedia3DShapeElement.h"
#include "OMediaAnim.h"
#include "OMediaRendererInterface.h"


// Morphing flags

typedef unsigned short omt_3DMorphFlags;
const omt_3DMorphFlags		ommorf_InterpolateVertices  = (1<<0);	// Interpolate vertex positions.
const omt_3DMorphFlags		ommorf_InterpolateVNormals	= (1<<1);	// Interpolate vertex normals.
const omt_3DMorphFlags		ommorf_InterpolateVColors	= (1<<2);	// Interpolate vertex colors.


//-----------------------------------------------------
// Definition

class OMedia3DMorphAnimDef : public OMediaAnimDef
{
	public:
	
	// * Constructor/Destructor

	omtshared OMedia3DMorphAnimDef();
	omtshared virtual ~OMedia3DMorphAnimDef();


	// Create and link a new frame
	omtshared virtual OMediaAnimFrame *create_frame(long sequence);

	// * Database/streamer support
	
	enum { db_type = 'A3dm' }; 

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void) const;
};


//-----------------------------------------------------
// Element

class OMedia3DMorphAnim :	public OMedia3DShapeElement,
							public OMediaAnim
{
	public:
	
	// * Constructor/Destructor

	omtshared OMedia3DMorphAnim();
	omtshared virtual ~OMedia3DMorphAnim();

	// * Anim definition

	omtshared virtual void set_anim_def(OMedia3DMorphAnimDef *def);

	// * Update logic
	
	omtshared virtual void update_logic(float elapsed);

	// * Override
	
	omtshared virtual void pause(bool p);
	omtshared virtual void get_dynamic_offset(float &x, float &y, float &z);

	// * Morphing flags

	inline void set_morph_flags(omt_3DMorphFlags f) {morph_flags = f;}
	inline omt_3DMorphFlags get_morph_flags(void) const {return morph_flags;}


	// * Database/streamer support
	
	enum { db_type = 'E3ma' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void);

	// * Dependencies:
	
	omtshared virtual void unlink_animdef_from_structure(OMediaAnimDef *animdef);


	protected:

	omtshared virtual void render_draw_shape(OMediaRendererInterface *rdr_i, OMedia3DShape	*shape, bool &pass2);


	omt_3DMorphFlags		morph_flags;

	static omt_VertexList	ivertex_buffer;
	static omt_NormalList	inormal_buffer;
	static omt_ColorList	icolor_buffer;

	float					morph_param;
};

#endif

