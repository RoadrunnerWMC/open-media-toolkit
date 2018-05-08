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
#ifndef OMEDIA_3DPolygon_H
#define OMEDIA_3DPolygon_H

#include "OMediaRGBColor.h"
#include "OMediaTypes.h"
#include "OMedia3DVector.h"
#include "OMediaClassStreamer.h"
#include "OMediaExtraTexturePass.h"

#include <vector>

class OMediaDataBase;
class OMediaDBObject;
class OMedia3DMaterial;

typedef unsigned short omt_3DPolygonFlags;
const omt_3DPolygonFlags om3pf_Selected = (1<<0);
const omt_3DPolygonFlags om3pf_TwoSided = (1<<1);
const omt_3DPolygonFlags om3pf_Hide 	= (1<<2);

const long			omc_NoExtraTexturePasses = -1;

//--------------------------------------------------------

class OMedia3DPolygonVertex
{
	public:

	unsigned long				vertex_index;
	unsigned long				normal_index;
	unsigned long				color_index;
	
	float						u,v;	// Texture coordinates (first pass)
	
	omt_ExtraTexturePassUV		extra_passes_uv;	// Texture coordinates for extra passes
};

typedef vector<OMedia3DPolygonVertex>	omt_3DPolygonVertexList;

//--------------------------------------------------------


// Since OMT V2.0, a polygon can have more than four vertices as long as it is convex.


class OMedia3DPolygon : public OMediaClassStreamer
{	
	public:

	omtshared OMedia3DPolygon(const OMedia3DPolygon &x);
	omtshared OMedia3DPolygon();
	omtshared ~OMedia3DPolygon();

	// * Vertices
	
	inline omt_3DPolygonVertexList &get_vertices(void) {return poly_vertices;}
	
	inline const omt_3DPolygonVertexList &get_vertices_const(void) const {return poly_vertices;};


		// Shortcut
	inline long get_num_points(void) const {return poly_vertices.size();}
	inline unsigned long get_point(short p) const {return poly_vertices[p].vertex_index;}
	omtshared void set_point(short p, unsigned long v, bool set_normal_index =true);

	omtshared void set_vertex_normal(short p, unsigned long n); 
	inline unsigned long get_vertex_normal(short p) const {return poly_vertices[p].normal_index;}

	omtshared void set_vertex_color(short p, unsigned long n); 
	inline unsigned long get_vertex_color(short p) const {return poly_vertices[p].color_index;}


	omtshared void set_quad(void);
	omtshared void set_triangle(void);



	// * Material

	omtshared void set_material(OMedia3DMaterial *m) ;						// If material is
	inline OMedia3DMaterial *get_material(void) const {return material;}	// NULL, polygon is not drawn.																		
																				
	
	// * Stream support

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	// * User ID
	
	inline unsigned long get_id(void) const {return user_id;}
	inline void set_id(unsigned long id) {user_id = id;}

	// * Face normal

	inline OMedia3DVector &get_normal(void) {return normal;}
	inline void get_normal_const(OMedia3DVector &n) const {n=normal;}


	// * Operators
	
	omtshared OMedia3DPolygon& operator=(const OMedia3DPolygon& x);
	omtshared bool operator<(const OMedia3DPolygon& x) const;

	// * Misc.

	omtshared short num_point_shared(OMedia3DPolygon &cp);


	// * Polygon info

	inline void set_flags(const omt_3DPolygonFlags f) {flags = f;}
	inline omt_3DPolygonFlags get_flags(void) const {return flags;}
	

		// Shortcut
	inline bool get_selected(void) const {return (flags&om3pf_Selected)!=0;}
	inline void set_selected(bool s) {if (s) flags|=om3pf_Selected;else flags&=~om3pf_Selected; }

	inline bool get_two_sided(void) const {return (flags&om3pf_TwoSided)!=0;}
	inline void set_two_sided(bool s) {if (s) flags|=om3pf_TwoSided;else flags&=~om3pf_TwoSided; }

	inline void set_text_coord_u(short p, float u) {poly_vertices[p].u = u;}
	inline void set_text_coord_v(short p, float v) {poly_vertices[p].v = v;}

	inline float get_text_coord_u(short p) const {return poly_vertices[p].u;}
	inline float get_text_coord_v(short p) const {return poly_vertices[p].v;}


	// * Index of an extra texture pass set.  Use omc_NoExtraTexturePasses to disable extra texture passes.

	inline void set_extra_texture_pass_index(long index) {textextrapass_index = index;}
	inline long get_extra_texture_pass_index(void) const {return textextrapass_index;}


	// * Low-level

	static inline void set_stream_dbase(OMediaDataBase *db, OMediaDBObject *obj) {stream_db = db; stream_obj=obj;}
	static inline OMediaDataBase *get_stream_dbase(void) {return stream_db;}
	static inline OMediaDBObject *get_stream_dobj(void) {return stream_obj;}
	static inline void set_reading_version(long v) {reading_version = v;}	

	protected:

	static OMediaDataBase	*stream_db;	
	static OMediaDBObject	*stream_obj;
	static long				reading_version;

	OMedia3DMaterial					*material;
	OMedia3DVector						normal;
	unsigned long						user_id;
	omt_3DPolygonFlags					flags;

	long								textextrapass_index;

	omt_3DPolygonVertexList				poly_vertices;
};

#endif

