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
#ifndef OMEDIA_3DShape_H
#define OMEDIA_3DShape_H

#include "OMedia3DPoint.h"
#include "OMedia3DPolygon.h"
#include "OMedia3DVector.h"
#include "OMediaDBObject.h"
#include "OMediaSphere.h"
#include "OMedia3DMaterial.h"
#include "OMediaExtraTexturePass.h"

#include <vector>
#include <list>

class OMedia3DShape;

//--------------------------------------------------------
// Prepare flags

typedef unsigned short omt_PrepareFlags;

const omt_PrepareFlags ompfc_ComputeShapeSphere				= (1<<0);
const omt_PrepareFlags ompfc_ComputeVertexNormals			= (1<<1);
const omt_PrepareFlags ompfc_SetDefaultTextureOrientation	= (1<<2);
const omt_PrepareFlags ompfc_QuickMagnitude					= (1<<3);
const omt_PrepareFlags ompfc_ComputeBoundingSphere			= (1<<4);
const omt_PrepareFlags ompfc_ComputePolygonNormals			= (1<<5);
const omt_PrepareFlags ompfc_OptimizePolygonOrder			= (1<<6);

const omt_PrepareFlags ompfc_PrepareAll			=	ompfc_ComputeShapeSphere|
												ompfc_ComputeVertexNormals|
												ompfc_SetDefaultTextureOrientation|
												ompfc_ComputeBoundingSphere|
												ompfc_ComputePolygonNormals|
												ompfc_OptimizePolygonOrder;


//--------------------------------------------------------
// Shape flags

typedef unsigned short omt_3DShapeFlags;

const omt_3DShapeFlags	omshfc_CompileStatic	= (1<<0);	// Reformat the shape for optimal rendering speed and try
															// to copy it to the 3D card memory
															// Use only this flag for static shape. 


//--------------------------------------------------------
// Definition of a set of extra texture passes

class OMediaExtraTexturePassSet
{
public:

	omt_ExtraTexturePassList		pass_list;		// One object per pass
};

typedef vector<OMediaExtraTexturePassSet> omt_ExtraTexturePassSetList;


//--------------------------------------------------------
// The shape


class OMedia3DShape : 	public OMediaDBObject,
						public OMediaEngineImpMaster
{	
	public:
	
	omtshared OMedia3DShape();
	omtshared virtual ~OMedia3DShape();

	omtshared virtual void purge(void);

	// * Flags

	inline void set_flags(const omt_3DShapeFlags f) {flags = f;}
	inline omt_3DShapeFlags get_flags(void) const {return flags;}


	// * Vertices and polygons

		// WARNING: Shape must be locked while you access any of the following lists!
		//			Use lock/unlock methods.

	inline omt_VertexList *get_vertices(void) {return &vertices;}
	inline omt_NormalList *get_normals(void) {return &normals;}
	inline omt_PolygonList *get_polygons(void)  {return &polygons;}
	inline omt_ColorList *get_colors(void)  {return &colors;}
	inline omt_SphereList *get_bounding_spheres(void) {return &bounding_spheres;}
	inline omt_ExtraTexturePassSetList *get_extra_texture_pass_sets(void) {return &textpass_sets;}

	omtshared virtual unsigned long add_vertex(OMedia3DPoint &point);		// Add new vertex. If the
																			// vertex already exists, the
																			// old one is returned.	

	omtshared virtual void delete_unused_vertices(void);

	
	// * Shape methods
		
	omtshared virtual void prepare(omt_PrepareFlags f =ompfc_PrepareAll);	
									// You should call this method after you modified
									// the shape. It allows OMT to update all the
									// internal fields required for a proper rendering.
									// If you set new polygons or vertices without calling
									// this method, you'll have rendering errors.
									// Turning on Gouraud shading requires a "prepare"
									// call too (or a prepare_gouraud()).
									// Scale, transform and rotate does not require a "prepare" call.								
	
	omtshared virtual void center(void);		// Center the object at (0,0,0) on its local
												// coordinates.
	
	omtshared virtual void translate(float vx, float vy, float vz);					// Translate
	omtshared virtual void scale(float sx, float sy, float sz);						// Scale
	omtshared virtual void rotate(short angle_x, short angle_y, short angle_z);		// Rotate
	omtshared virtual void inv_rotate(short angle_x, short angle_y, short angle_z);	// Inverse Rotate

	omtshared virtual void find_sphere(OMediaSphere &sphere, bool only_selected =false);
	omtshared virtual void find_cube(float &minx, float &miny, float &minz,float &maxx, float &maxy, float &maxz);

	omtshared virtual void translate_top(void);				// (0,0,0) is top of object	 (-y)
	omtshared virtual void translate_bottom(void);		 	//        "       bottom "		   (y)
	omtshared virtual void translate_left(void);			//        "       left       "			(x)
	omtshared virtual void translate_right(void);			//        "       right     "		 (-x)
	omtshared virtual void translate_near(void);			//        "       nearest point (z)
	omtshared virtual void translate_far(void);				//        "       far  poinst		(-z)

	omtshared virtual void merge_points(float distance, bool only_selected =false);	  // Merge points

	omtshared virtual void unshare_selected_vertices(void);	
	omtshared virtual void flip_polygons(bool only_selected =true);	

	omtshared virtual void triangles_to_quads(float breaking_angle=0.01, bool only_selected=false);

	// * Attributes

	omtshared virtual void set_material(OMedia3DMaterial *m);	// Set a material for all polygons in the
														// shape. Shape doesn't require to be "reprepared".
														// after this call.

	omtshared virtual void set_two_sided(bool ts);		// (Un)set two sided flag for all polygons.




	// * The "prepare" method generates the following shape constants

	// Visual sphere

	inline float get_radius(void) const {return visual_sphere.radius;}
	inline float get_vcenterx(void) const {return visual_sphere.x;}
	inline float get_vcentery(void) const {return visual_sphere.y;}
	inline float get_vcenterz(void) const {return visual_sphere.z;}

	inline void get_global_sphere(OMediaSphere &s) const {s = visual_sphere;}
	inline void set_global_sphere(const OMediaSphere &s) {visual_sphere = s;}

	// Bounding sphere
	inline void get_global_bsphere(OMediaSphere &s) const {s=global_bounding_sphere;}
	inline void set_global_bsphere(const OMediaSphere &s) {global_bounding_sphere=s;}


	// * Normals
 
	omtshared virtual void compute_polygon_normals(bool only_selected =false);  // Polygon normals
	omtshared virtual void compute_normals(bool only_selected =false);		 // Points normals	

	omtshared virtual void compute_normals(float breaking_angle, bool only_selected =false);	
																	// Points normals with breaking angles
																	// May have to unshare some vertices	
																	// breaking_angle = 0-1.0
	omtshared virtual void delete_unused_normals(void);
	omtshared virtual void merge_normals(bool only_selected);

	// * Selection (modeler mode)
	
	omtshared virtual void select_all(void);
	omtshared virtual void deselect_all(void);


	// * Database/streamer support
	
	enum { db_type = '3DSh' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void) const;

	// * Operators
	
	omtshared OMedia3DShape& operator=(const OMedia3DShape& x);
	
	
	// * Texture coordinates
	
	omtshared virtual void map_cubic(float offsetx=0, float offsety=0,
									 float scalex=1.0, float scaley=1.0,   
									  bool only_selected = false);

	omtshared virtual void map_projection( short 			angle_x,
								   		   short 			angle_y,
										   short 			angle_z,
										   float			offset_x =0, 
										   float			offset_y =0,
										   float			scale_x =1.0, 
										   float			scale_y =1.0,
										   float		  	breaking_angle =0.3, // 0.0-1.0 (0-90 degrees)
										   bool 	  	only_selected =false,
										   bool			invert_angles = true);

	

	omtshared virtual void rotate_textures(short angle, float centerx, float centery, bool only_selected = false);
	omtshared virtual void scale_textures(float centerx, float centery,
										  float scalex, float scaley,  
										  bool only_selected = false);
	
	omtshared virtual void offset_textures(float offsetx, float offsety,bool only_selected = false);
	
	
	// * Primitives
	
	omtshared virtual void make_cube(float size, bool inverted =false);
	omtshared virtual void make_sphere(float radius, short nrings=8, short nrays=12);

	 

	protected:

	omtshared virtual void find_center_offset(float &ox, float &oy, float &oz);
	omtshared virtual void find_center_offset(OMedia3DPoint &o, bool only_selected);

	omtshared virtual void compute_global_bsphere(void);
	
	OMediaSphere				visual_sphere;
	OMediaSphere				global_bounding_sphere;	

	omt_VertexList					vertices;
	omt_NormalList					normals;
	omt_PolygonList					polygons;
	omt_SphereList					bounding_spheres;
	omt_ColorList					colors;
	omt_ExtraTexturePassSetList		textpass_sets;

	omt_3DShapeFlags		flags;

};



#endif

