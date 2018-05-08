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
#ifndef OMEDIA_PrimitiveElement_H
#define OMEDIA_PrimitiveElement_H

#include "OMediaElement.h"
#include "OMediaRGBColor.h"
#include "OMediaSphere.h"
#include "OMediaRendererInterface.h"

class OMediaCanvas;

#include <vector>

typedef unsigned short omt_PrimitiveElementFlags;
const omt_PrimitiveElementFlags		ompref_FaceCulling = (1<<0);

class OMediaPrimitiveElement :   public OMediaElement
{
	public:
	
	// * Constructor/Destructor

	omtshared  OMediaPrimitiveElement();
	omtshared  virtual ~OMediaPrimitiveElement();	

	omtshared virtual void reset(void);


	// * Vertices

	inline omt_RenderVertexList *get_vertices(void) {return &vertex_list;}

	// * Primitive flags

	inline void set_primitive_flags(const omt_PrimitiveElementFlags f) {primitive_flags = f;}
	inline omt_PrimitiveElementFlags get_primitive_flags(void) const {return primitive_flags;}

	// * Render mode

	inline void set_draw_mode(const omt_RenderDrawMode dm) {draw_mode = dm;}
	inline omt_RenderDrawMode get_draw_mode(void) const {return draw_mode;}

	// * Shade mode

	inline void set_shade_mode(const omt_ShadeMode sm) {primitive_shade_mode = sm;}
	inline omt_ShadeMode get_shade_mode(void) const {return primitive_shade_mode;}

	// * Blending

	inline void set_blend(omt_BlendFunc src, omt_BlendFunc dest) {src_blend = src; dest_blend = dest;}
	inline omt_BlendFunc get_blend_src(void) const {return src_blend;}
	inline omt_BlendFunc get_blend_dest(void) const {return dest_blend;}

	// * Texture (if any)

	omtshared virtual void set_texture(OMediaCanvas *c);
	inline OMediaCanvas *get_texture(void) {return texture;}

	inline void set_texture_address_mode(const omt_TextureAddressMode am)
		{texture_address_mode = am;}

	inline omt_TextureAddressMode get_texture_address_mode(void) const
		{return texture_address_mode;}

	inline void set_texture_color_operation(const omt_TextureColorOperation am)
		{texture_color_operation = am;}

	inline omt_TextureColorOperation get_texture_color_operation(void) const
		{return texture_color_operation;}

	// * Multi-pass texturing

	omt_ExtraTexturePassList	*get_extra_texture_passes(void) {return &texture_passes;}
	omt_ExtraTexturePassUV		*get_extra_texture_passes_uv(void) {return &texture_passes_uv;}

	// * Rendering

	omtshared virtual void render_geometry(OMediaRendererInterface *rdr_i, 
										OMediaMatrix_4x4 &modelmatrix, 
										OMediaMatrix_4x4 &viewmatrix, 
										OMediaMatrix_4x4 &projectmatrix, 
										omt_LightList	 *lights, 
										omt_RenderModeElementFlags render_flags);

	// * Database/streamer support
	
	enum { db_type = 'Epri' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void);


	protected:

	omtshared virtual bool render_reject(OMediaMatrix_4x4 &modelmatrix, 
											OMediaMatrix_4x4 &projectmatrix,
								  			OMediaRendererInterface *rdr_i);

	omtshared virtual void process_primitive_picking(OMediaPickRequest *pick_request, vector<OMediaPickHit> *hit_list);


	omt_RenderVertexList		vertex_list;
	omt_RenderDrawMode			draw_mode;
	OMediaCanvas				*texture;
	omt_PrimitiveElementFlags	primitive_flags;
	omt_ShadeMode				primitive_shade_mode;
	omt_BlendFunc				src_blend,dest_blend;
	omt_TextureAddressMode		texture_address_mode;
	omt_TextureColorOperation	texture_color_operation;
	omt_ExtraTexturePassList	texture_passes;
	omt_ExtraTexturePassUV		texture_passes_uv;
};



#endif

