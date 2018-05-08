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
#ifndef OMEDIA_CanvasElement_H
#define OMEDIA_CanvasElement_H

#include "OMediaElement.h"
#include "OMediaCanvas.h"

enum omt_AutoAlign
{
	omaac_None,
	omaac_Left,
	omaac_Right,
	omaac_Center,
	omaac_Top = omaac_Left,
	omaac_Bottom = omaac_Right
};

typedef unsigned short omt_CanvasElementFlags;
const omt_CanvasElementFlags omcanef_FreeWorldSize = (1<<0);		// By default the world size is the same than the canvas
																	// size. If you set this flag, the world size can be changed
																	// using the OMediaCanvasElement::set_size.
																	// You need to set this flag if you want to do scaling effect.


class OMediaPickResult;
class OMediaCanvasElement;


/****************************************************/

class OMediaCanvasElement : 	public OMediaElement
{
	public:

	// * Construction

	omtshared OMediaCanvasElement();
	omtshared virtual ~OMediaCanvasElement();

	omtshared virtual void reset(void);


	// * Canvas
	
	omtshared virtual void set_canvas(OMediaCanvas *canv);
	inline OMediaCanvas *get_canvas(void) {return canvas;}

	// * World size and offset

	inline void set_offset(float ox, float oy, float oz) {canv_x = ox; canv_y = oy; canv_z=oz;}
	inline float get_offsetx(void) const {return canv_x;}
	inline float get_offsety(void) const {return canv_y;}
	inline float get_offsetz(void) const {return canv_y;}

		// Following size is used only if the omcanef_FreeWorldSize flag is set.
	inline void set_size(float w, float h) {canv_w = w; canv_h = h;}
	inline float get_width(void) const {return canv_w;}
	inline float get_height(void) const {return canv_h;}

	inline void set_auto_align(omt_AutoAlign horiz, omt_AutoAlign vert)
	{
		align_h = horiz;
		align_v = vert;
	}

	inline omt_AutoAlign get_auto_align_h(void) const {return align_h;}
	inline omt_AutoAlign get_auto_align_v(void) const {return align_v;}

	// * Diffuse color
	
	inline void set_diffuse(const OMediaFARGBColor &argb) {diffuse = argb;}
	inline void get_diffuse(OMediaFARGBColor &argb) const {argb = diffuse;}

	// * Blending

	inline void set_blend(omt_BlendFunc src, omt_BlendFunc dest) {src_blend = src; dest_blend = dest;}
	inline omt_BlendFunc get_blend_src(void) const {return src_blend;}
	inline omt_BlendFunc get_blend_dest(void) const {return dest_blend;}

	// * Render

	omtshared virtual void render_geometry(OMediaRendererInterface *rdr_i, OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &viewmatrix, OMediaMatrix_4x4 &projectmatrix, omt_LightList	 *lights, omt_RenderModeElementFlags render_flags);
	omtshared virtual bool render_reject(OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &projectmatrix,
										  	OMediaRendererInterface *rdr_i);

	// * Update logic

	omtshared virtual void update_logic(float millisecs_elapsed);
	omtshared virtual bool validate_closer_hit(OMediaPickResult &hit,
												OMediaPickSubResult &closer);

	// * Canvas element flags

	inline void set_canvas_flags(const omt_CanvasElementFlags f) {canvas_flags = f;}
	inline omt_CanvasElementFlags get_canvas_flags(void) const {return canvas_flags;}


	// * Database/streamer support
	
	enum { db_type = 'Ecan' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void);

	
	protected:


	omtshared virtual void process_canvas_picking(	OMediaPickRequest *pick_request, 
													vector<OMediaPickHit> *hit_list,
												 	OMediaMatrix_4x4 &modelmatrix,
												 	OMediaMatrix_4x4 &projectmatrix);

	omtshared virtual void process_canvas_surface_picking(	OMediaPickRequest *pick_request,
															OMediaPickResult	&result,
															OMediaMatrix_4x4 &modelmatrix,
															OMediaMatrix_4x4 &projectmatrix);



	omtshared virtual void compute_auto_align(void);

	omtshared virtual void get_canv_wordsize(float &w, float &h);

	OMediaCanvas		*canvas;
	OMediaFARGBColor	diffuse;
	
	float				canv_x,canv_y,canv_z,canv_w,canv_h;

	omt_BlendFunc 		src_blend, dest_blend;

	omt_AutoAlign			align_h,align_v;
	omt_CanvasElementFlags	canvas_flags;

};

#endif

