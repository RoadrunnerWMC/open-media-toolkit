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
#ifndef OMEDIA_Element_H
#define OMEDIA_Element_H

#include "OMediaTypes.h"
#include "OMediaElementContainer.h"
#include "OMediaWorldUnits.h"
#include "OMediaWorldPosition.h"
#include "OMediaWorldAngle.h"
#include "OMediaListener.h"
#include "OMedia3DAxis.h"
#include "OMediaSphere.h"
#include "OMediaMatrix.h"
#include "OMediaCollisionCache.h"

#include <list>
#include <vector>

class OMediaWorld;
class OMediaLayer;
class OMediaRendererInterface;
class OMediaLight;
class OMediaRenderPreSortedElement;
class OMediaRenderHTransform;
class OMediaElement;
class OMediaPickResult;
class OMediaPickSubResult;
class OMediaDataBase;
class OMedia3DShape;
class OMediaAnimDef;

typedef list<OMediaLight*>	omt_LightList;

// Flags

typedef unsigned long omt_ElementFlags;
const omt_ElementFlags omelf_DontHideSub 			= (1<<0);	// Sub-element visible flag is not supervisor dependant
const omt_ElementFlags omelf_Destroy 				= (1<<1);	// Same as calling destroy
const omt_ElementFlags omelf_DisableZBuffer			= (1<<2);	// ZBuffer is disabled
const omt_ElementFlags omelf_SecondPass				= (1<<3);	// Draw this element during the second pass.
const omt_ElementFlags omelf_DisablePicking			= (1<<4);	// Does not receive picking hits
const omt_ElementFlags omelf_CloserHitCheckAlpha	= (1<<5);	// When this flag is set, OMT does not set the closer hit for
																// the elements with an alpha of zero. You should
																// set this flag if you want the mouse clicks or the closer
																// hit to take care of transparency.

const omt_ElementFlags omelf_DisableRotate				= (1<<6);	// Do not rotate the element
const omt_ElementFlags omelf_DisableTranslate			= (1<<7);	// Do not translate element
const omt_ElementFlags omelf_DisableViewportTransform	= (1<<8);	// Do not apply viewport transform
const omt_ElementFlags omelf_FaceViewport				= (1<<9);	// Do not apply viewport rotate. Element
																	// is always facing the viewport.
const omt_ElementFlags omelf_ExactTextelToPixel			= (1<<10);	// This flag should be set when you
																	// want to be sure that textels are exactly
																	// mapped to pixel. It is tested only if
																	// the projection matrix is orthogonal.
																	// The canvas element set this flag by
																	// default.

const omt_ElementFlags omelf_RenderSubElementAfter		= (1<<11);	// By default, OMT renders first the child-elements.
																	// When this flag is set, the element is rendered first
																	// and then the child elements.

const omt_ElementFlags omelf_CollCacheStatic			= (1<<12);	// This element is not removed from the collision cache
																	// when a OMediaCollCache::flush method is called. Use only
																	// this flag with completely static elements 
																	// (no animation, no motion, etc.).


// Collision level

enum omt_CollisionLevel
{
	omcol_None,			// No collision
	omcol_GlobalSphere,	// Global bounding sphere
	omcol_Spheres,		// Bounding spheres
	omcol_Triangles		// Polygons
};


//****************************************************
// Rendering classes

class OMediaRenderHTransform : public OMedia3DAxis
{
	public:

	float		trans_x,trans_y,trans_z;
};


class OMediaRenderPreSortedElement
{
	public:
	
	inline OMediaRenderPreSortedElement() {}
	inline OMediaRenderPreSortedElement(const OMediaRenderPreSortedElement &ctr) {}
	OMediaRenderPreSortedElement &operator=(const OMediaRenderPreSortedElement &v) {return *this;}
	
	OMediaElement			*element;
	OMediaMatrix_4x4		model_matrix;
	float					sort_value;
};

typedef list<OMediaRenderPreSortedElement> omt_RenderPreSortedElementList;

typedef unsigned short omt_RenderModeElementFlags;
const omt_RenderModeElementFlags	omref_SecondPass  = (1<<0);


//****************************************************
// Element
 
class OMediaElement : public OMediaWorldPosition,
					  public OMediaWorldAngle,
					  public OMediaElementContainer,
					  public OMediaListener
{
	public: 
	
	// * Constructor/Destructor

	omtshared OMediaElement();
	omtshared virtual ~OMediaElement();	

	omtshared virtual void reset(void);


	inline OMediaWorld 	*getworld(void) {return its_world;}
	inline OMediaElement *getsuperelement(void) {return its_superelement;}


	// * Flags
	
	inline omt_ElementFlags get_flags(void) const {return element_flags;}
	inline void set_flags(const omt_ElementFlags flags) {element_flags = flags;}

	// * Link/unlink 

	omtshared virtual void link(OMediaElementContainer *c);	// link to the last world layer


	// Unlink from world and layer
	omtshared virtual void unlink(void);
	
	inline bool islinked(void) {return (its_world!=NULL);}


	// * Layer (only root element should be linked to a layer, child elements are
	//			always drawn in the same layer than the root)

	omtshared virtual void link_layer(OMediaLayer *layer);
	omtshared virtual void unlink_layer(void);
	
	inline OMediaLayer *get_layer(void) {return its_layer;}	// Always NULL for child elements


	// * Update logic
	
	omtshared virtual void update_logic(float millisecs_elapsed);


	// * Render

	omtshared virtual void render(OMediaRendererInterface 			*rdr_i,
								  OMediaRenderHTransform 			&super_hxform,
								  OMediaMatrix_4x4 					&viewmatrix,
								  OMediaMatrix_4x4	 				&projectmatrix,
								  omt_LightList	 					*lights,
								  omt_RenderPreSortedElementList	*presort,
								  omt_RenderModeElementFlags		render_flags);

	omtshared virtual bool render_reject(OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &projectmatrix, OMediaRendererInterface *rdr_i);
	omtshared virtual void render_geometry(	OMediaRendererInterface *rdr_i, 
											OMediaMatrix_4x4 &modelmatrix, 
											OMediaMatrix_4x4 &viewmatrix, 
											OMediaMatrix_4x4 &projectmatrix,
											omt_LightList	 *lights, 
											omt_RenderModeElementFlags	render_flags);


	// * Find absolute position and orientation in world coordinates
	
	omtshared virtual void get_absolute_info(	omt_WUnit &x, omt_WUnit &y, omt_WUnit &z,
												omt_WAngle &ax, omt_WAngle &ay, omt_WAngle &az);

	
	omtshared virtual void local_to_world(OMedia3DPoint	&p, OMediaMatrix_4x4 *view_m =NULL);

	omtshared virtual void local_to_world_matrix(OMediaMatrix_4x4 &xform, OMediaMatrix_4x4 *view_m =NULL);



	// * Show/hide
	
	omtshared virtual void hide(bool h = true);
	inline void show(bool s = true) {hide(!s);}
	inline bool getvisible(void) const {return (hide_count==0);}

	omtshared virtual bool getvisible_extend(void);	// Returns true only if all supervisors
													// are visible too.

	inline void force_show(void) {if (hide_count) {hide_count=1; show();}}


	// * Pause
	
	omtshared virtual void pause(bool p);
	inline bool is_paused(void) {return pause_count!=0;}


	// * Container node (use this carefully!)

	inline omt_ElementList::iterator get_container_node(void) {return container_node;}
	inline void set_container_node(omt_ElementList::iterator i) {container_node = i;}

	// * User ID

	inline void set_id(long id) {its_id = id;}
	inline long get_id() const {return its_id;}

	// * Descriptor
	
	inline void set_descriptor(const string desc) {element_desc = desc;}
	inline string get_descriptor(void) const {return element_desc;}


	// * Element is deleted at next update (before being drawed, can be called
	// from inside update() method.)

	inline void destroy(void) {element_flags|=omelf_Destroy;}
	inline void undestroy(void) {element_flags&=~omelf_Destroy;}
	inline bool	is_destroy_on(void) const {return (element_flags&omelf_Destroy)!=0;}

	// * Listen to message
						
	omtshared virtual void listen_to_message(omt_Message msg, void *param =NULL);
	

	// * Hierarchical transformation

	omtshared virtual void compute_hxform(	OMediaRenderHTransform &super_hxform,
											OMediaRenderHTransform &hxform);

	omtshared virtual void compute_hxform_hierarchy(OMediaRenderHTransform &hxform);

	omtshared virtual void compute_model_matrix(OMediaRenderHTransform &hxform, 
												OMediaMatrix_4x4 &viewmatrix,
												OMediaMatrix_4x4 &matrix);

			// Model matrix without viewport transformation:
	omtshared virtual void compute_model_matrix(OMediaRenderHTransform &hxform, 
												 OMediaMatrix_4x4 		&matrix);

	// * Mouse click and picking
	
	omtshared virtual void clicked(OMediaPickResult *res, bool mouse_down);
	omtshared virtual bool validate_closer_hit(OMediaPickResult &hit,
												OMediaPickSubResult &closer);	// Returns true if this element accepts
																		// to be the closer hit.
	

	// * Abstract method to add dynamically an offset to the element position

	omtshared virtual void get_dynamic_offset(float &x, float &y, float &z);

	// * Database/streamer support
	
	enum { db_type = 'Elem' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void);

	// * Collision / intersection

	omtshared virtual bool collide(OMediaElement *element, OMediaCollisionCache *cache);

	omtshared virtual bool ray_intersect(OMedia3DPoint	&ray_origin,
										 OMedia3DVector	&ray_direction, 
										 OMediaCollisionCache	*cache,
										 float			&out_distance);


	inline void set_collision_level(const omt_CollisionLevel l) {collision_level = l;}
	inline omt_CollisionLevel get_collision_level(void) const {return collision_level;}


	// * Dependencies:
	
	omtshared virtual void unlink_shape_from_structure(OMedia3DShape *shape);
	omtshared virtual void unlink_animdef_from_structure(OMediaAnimDef *animdef);

	
	protected:	

	friend class OMediaWorld;


	omtshared virtual void world_link(OMediaWorld *world);				// link to the last world layer
	omtshared virtual void element_link(OMediaElement *element);		// link to the supervisor layer

	omtshared virtual void container_link_element(OMediaElement *e);


	omtshared virtual void update_world_ptr(OMediaWorld *);

	
	virtual omtshared void render_sub_elements(OMediaRendererInterface 		*rdr_i, 
										OMediaRenderHTransform 			&super_hxform,
										OMediaMatrix_4x4 				&viewmatrix,
										OMediaMatrix_4x4 				&projectmatrix,
										omt_LightList	 				*lights,
						  				omt_RenderPreSortedElementList	*presort,
										omt_RenderModeElementFlags		render_flags);


	omtshared virtual void container_link(OMediaElementContainer *container);
	omtshared virtual void container_unlink(OMediaElementContainer *container);
	
	omtshared bool clip_sphere(	OMediaSphere		&abs_world_gravity,
						OMediaMatrix_4x4	&xform_view_matrix,
						OMediaMatrix_4x4	&xform_project_matrix);

	omtshared void prepare_lights(			OMediaRendererInterface *rdr_i, 
									OMediaSphere			&world_gravity,
									omt_LightList 			*lights,
									OMediaMatrix_4x4		&viewmatrix);


	omtshared virtual bool fill_coll_bounding_spheres(OMediaCollCacheBlock *block);
	omtshared virtual bool fill_coll_triangles(OMediaCollCacheBlock *block);
	omtshared virtual bool find_global_sphere(OMediaSphere &sphere);
	
	omtshared bool colltest_tri_gsphe(OMediaCollCacheBlock *block_a,
								       OMediaCollCacheBlock *block_b, 
								       OMedia3DPoint		*triangle,
								       OMedia3DPolygon		*&polygon_hit);
	
	omtshared bool colltest_tri_tri(OMediaCollCacheBlock *block_a,
								     OMediaCollCacheBlock *block_b, 
								     OMedia3DPoint *ta, OMedia3DPoint *tb,
								     OMedia3DPolygon		*&polygon_ahit,
								     OMedia3DPolygon		*&polygon_bhit);

	omtshared bool colltest_tri_sphe(OMediaCollCacheBlock *block_a,
								       OMediaCollCacheBlock *block_b, 
								       OMedia3DPoint		*triangle,
								       OMedia3DPolygon		*&polygon_hit);
	
	omtshared bool colltest_sphe_sphe(OMediaCollCacheBlock *block_a,
									   OMediaCollCacheBlock *block_b);

	omtshared bool colltest_ray_tri(OMedia3DPoint	&ray_origin,
									 OMedia3DVector	&ray_direction,
									 OMediaCollCacheBlock	*block_a,
									 OMedia3DPoint			*triangle,
 									 OMedia3DPolygon		*&polygon_hit,
									 float					&out_distance);

	omtshared bool colltest_ray_sphe(OMedia3DPoint	&ray_origin,
									 OMedia3DVector	&ray_direction,
									OMediaCollCacheBlock *block_a,
								   float				&out_distance);




	omtshared void collision_fill_cacheblock(OMediaCollCacheBlock *block);


	OMediaWorld					*its_world;
	OMediaElement				*its_superelement;
	OMediaElementContainer		*its_container;
	
	OMediaLayer					*its_layer;
	omt_ElementList::iterator	layer_node;

	omt_ElementList::iterator	container_node;

	long								hide_count;
	long								pause_count;
	long								its_id;	
	omt_ElementFlags					element_flags;
	omt_CollisionLevel					collision_level;
	omt_CollCacheBlockList::iterator	coll_cache_iterator;
	bool								coll_cache_linked;
	string								element_desc;

	friend class OMediaCollisionCache;

	public:

	#define omermf_SecondPassMark 		(1<<0)	// Needs to be rendered during the second pass
	#define omermf_ScanSecondPassMark 	(1<<1)	// Needs to be scanned during the second pass
	short render_mark_element_flags;
};



#endif

