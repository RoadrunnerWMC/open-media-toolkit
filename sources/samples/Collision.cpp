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

// Collision

#include "OMediaApplication.h"
#include "OMediaSingleWindow.h"
#include "OMediaMonitorMap.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasFont.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaRendererDef.h"
#include "OMedia3DShapeElement.h"
#include "OMediaWorld.h"
#include "OMediaViewPort.h"
#include "OMediaLayer.h"
#include "OMediaDatabase.h"
#include "OMedia3DMaterial.h"
#include "OMediaLight.h"
#include "OMedia3DShape.h"
#include "OMediaPeriodical.h"
#include "OMedia3DVector.h"
#include "OMediaRandomNumber.h"
#include "OMedia3DAxis.h"


//---------------------------------------------
// Definitions


	// Override standard OMT application class

class Collision :	public OMediaApplication,
					public OMediaPeriodical

{
public:

	Collision();
	virtual ~Collision();

	void init_display(void);
	void init_font(void);
	void init_world(void);
	void init_demo(void);

	void init_database(void);
	void close_database(void);

	void listen_to_message(omt_Message msg, void *param);

	void spend_time(void);

	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;

	OMediaCollisionCache	*coll_cache;

	OMediaTimer				timer;

	OMedia3DShapeElement	*room;
	OMedia3DShapeElement	*sphere;

	OMedia3DVector			sphere_vect;
	OMedia3DAxis			sphere_axis;
	float					sphere_rotation_speed;
	float					sphere_rotation_angle;

	OMediaRandomNumber		random;
};

//---------------------------------------------
// Implementations

Collision::Collision()
{
	sphere_rotation_speed = 0.0f;
	sphere_rotation_angle = 0.0f;

	init_database();
	init_display();
	init_world();
	init_demo();
}

Collision::~Collision()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)
	delete coll_cache;
	
	delete monitors;
	close_database();
}

void Collision::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(700,480);
	window->place(40,80);
	window->show();

	// * Get the first available video engine

	omt_EngineID video_engine = *(OMediaEngineFactory::get_factory()->video_engines.begin());

	// * Create a monitor maps
	
	// A monitor map automatically creates for you one video engine
	// per card installed in your machine. It allows OMT to take
	// care of multiple monitors automatically for you.

	monitors = new OMediaMonitorMap(video_engine,window);
	window->link_monitor_map(monitors);


	// * Select renderers for all monitors

	// Now we need to pick up a renderer for each monitors. Because each
	// monitor can be linked to a different cards, they may have different
	// renderers.
	//
	// The monitor map contains a list of video engines. For each monitors
	// I scan the display list and look for an accerated renderer. If there
	// is no accelered device, I pick the first renderer of the list.

	for(omt_VideoEngineList::iterator vi=monitors->engines.begin();
		vi!=monitors->engines.end();
		vi++)
	{
		OMediaRendererDef	*def = omc_NULL;

		for(omt_RendererDefList::iterator ri = (*vi)->get_renderer_list()->begin();
			ri!=(*vi)->get_renderer_list()->end();
			ri++)
		{		
			if ((*ri).attributes & omcrdattr_Accelerated ) def = &(*ri);	// Accelerated ?
		}

		// No accelerated device. Take the first one.
		if (!def && (*vi)->get_renderer_list()->size()) def =  &(*(*vi)->get_renderer_list()->begin());


		// Once I found a renderer I need to select it. This will automatically build the renderer for me.
		if (def) (*vi)->select_renderer(def,def->zbuffer_depth);
	}
}

void Collision::init_world(void)
{
	OMediaRect			r;
	OMediaFARGBColor	argb;

	// * World

	world = new OMediaWorld;	// Create the root class


	// * Viewport


	viewport = new OMediaViewPort(window);		// Supervisor is window
	viewport->link(world);
	viewport->link_window(window);				// Output is window


	r.set(0,0,0,0);		// Viewport bounds are right/bottom relative, so
						// when the window is resized, the viewport is resized
						// too.

	viewport->setbounds(&r);
	viewport->set_bounds_mode(omcpbc_Right,omvpbmc_RightRelative);
	viewport->set_bounds_mode(omcpbc_Bottom,omvpbmc_BottomRelative);
	viewport->place(0,180,0);

	// * Layer

	argb.set(1.0f,0,0,0);

	layer = new OMediaLayer;
	layer->link(world);
	layer->add_flags(omlayerf_ClearColor|omlayerf_ClearZBuffer|omlayerf_EnableZBufferWrite|omlayerf_EnableZBufferTest);
	layer->set_clear_color(argb);
	layer->set_light_global_ambient(OMediaFARGBColor(1.0f,0.5f,0.5f,0.5f));

	// * Light

	OMediaLight	*light;
	light = new OMediaLight;
	light->link(world);
	light->set_light_type(omclt_Directional);
	light->set_angle(omd_Deg2Angle(35),omd_Deg2Angle(25),0);
}

void Collision::init_database(void)
{
	OMediaFilePath			path("medias/samples.omt");

	// Set the cache to zero. In this mode, OMT database never removes objects
	// from memory (until you delete the database).

	OMediaDataBase::set_cache_size(0);

	// We need to register the OMT classes we'll get from the database: 

	omd_REGISTERCLASS(OMediaCanvas);
	omd_REGISTERCLASS(OMedia3DMaterial);
	omd_REGISTERCLASS(OMedia3DShape);

	dbfile = new OMediaFileStream;
	dbfile->setpath(&path);
	dbfile->open(omcfp_Read);			// Read only

	database = new OMediaDataBase(dbfile);	// Open database
}

void Collision::close_database(void)
{
	delete database;
	delete dbfile;
}

void Collision::init_demo(void)
{
	OMediaCanvas			*canvas;

	// Create

	room = new OMedia3DShapeElement;
	room->link(world);
	room->set_shape(omd_GETOBJECT(database,OMedia3DShape,"room"));

	// The room is static so I can set the omelf_CollCacheStatic flag. This
	// way the room stay in the collision cache and it is not flushed at each 
	// new frame.
	room->set_flags(omelf_CollCacheStatic);

	// Set the collision level. For the room, I need triangles test:
	room->set_collision_level(omcol_Triangles);

	// Create spheres

	sphere = new OMedia3DShapeElement;
	sphere->link(world);
	sphere->set_shape(omd_GETOBJECT(database,OMedia3DShape,"omtsphere"));
	sphere->set_collision_level(omcol_GlobalSphere);
	sphere->place(10,60,100);
	sphere_vect.set(0,0,300.0f);
	sphere_vect.rotate(omd_Deg2Angle(41),omd_Deg2Angle(60),0);

	// Set filtering
	canvas = omd_GETOBJECT(database,OMediaCanvas,"metalground1");
	canvas->set_filtering(omtfc_Linear,omtfc_Linear_Mipmap_Linear);
	canvas = omd_GETOBJECT(database,OMediaCanvas,"metalground2");
	canvas->set_filtering(omtfc_Linear,omtfc_Linear_Mipmap_Linear);
	canvas = omd_GETOBJECT(database,OMediaCanvas,"metalground3");
	canvas->set_filtering(omtfc_Linear,omtfc_Linear_Mipmap_Linear);
	canvas = omd_GETOBJECT(database,OMediaCanvas,"OMT2");
	canvas->set_filtering(omtfc_Linear,omtfc_Linear_Mipmap_Linear);

	// The collision cache is used to speed up collision testing. It contains
	// a version of the latest tested elements in a transformed state. By default,
	// the cache stores only 20 elements at the same time. This can be changed
	// by using the OMediaCollisionCache::set_cache_size method.

	coll_cache = new OMediaCollisionCache;
}

void Collision::spend_time(void)
{
	float	elapsed = timer.getelapsed();
	timer.start();
	OMedia3DPoint	p;
	OMedia3DVector	v,v2,normal,ov;
	omt_Angle		ax,ay,az;

	if (elapsed>100) elapsed = 100;

	// Main direction

	float	m = elapsed / 1000.0f,f,mg;

	// Animate the sphere

	sphere_vect.y -= 300.0f * elapsed/1000.0f;

	v = sphere_vect;
	v.normalize();
	v.x *= 30.0f * m;
	v.y *= 30.0f * m;
	v.z *= 30.0f * m;

	sphere_vect.x -= v.x; 
	sphere_vect.y -= v.y;
	sphere_vect.z -= v.z;

	sphere->get_position(p);
	sphere->move(sphere_vect.x*m,sphere_vect.y*m,sphere_vect.z*m);

	mg = sphere_vect.quick_magnitude();
	f = (mg*mg)/(300.0f * 300.0f);

	if (mg<0.2)
	{
		sphere_vect.x = random.range(0,800)-400; 
		sphere_vect.y = random.range(250,400);
		sphere_vect.z = random.range(0,800)-400;
	}

	sphere_rotation_angle += sphere_rotation_speed * m;
	sphere_axis.rotate(omc3daxis_Pitch, (int)(sphere_rotation_speed * m));

	// I need to remove the sphere from the cache each times it is moved.

	coll_cache->flush(sphere);

	// Test collision against the room

	if (sphere->collide(room,coll_cache))
	{
		OMedia3DPoint	*t = coll_cache->get_hit_triangle_b();

		// Inverse vector direction

		v.set(t[0],t[1]);
		v2.set(t[0],t[2]);
		v.cross_product(v2,normal);
		normal.normalize();

		ov = sphere_vect;
		v = ov;
		v.x *=-1.0f;
		v.y *=-1.0f;
		v.z *=-1.0f;
		sphere_vect.reflect(normal,sphere_vect);

		// Roll effect

		sphere_rotation_speed = v.angle_between(sphere_vect) * (sphere_vect.quick_magnitude()/100.0f);
		if (sphere_rotation_speed>0.5f)
		{
			sphere_axis.convert(ax,ay,az);

			// I modify directly the shape. Would be better to use a matrix instead.
			sphere->get_shape()->rotate(ax,ay,az);

			v = sphere_vect;
			v.normalize();

			v.angles(ax,ay);
			sphere_axis.reset_axis();
			sphere_axis.get_axis(omc3daxis_Pitch).rotate(ax,ay,0);
			sphere_axis.get_axis(omc3daxis_Yaw).rotate(ax,ay,0);
			sphere_axis.get_axis(omc3daxis_Roll).rotate(ax,ay,0);

			sphere_axis.inv_convert(ax,ay,az);
			sphere->get_shape()->inv_rotate(ax,ay,-az);
		}


		// Try to move it now (would be more efficient to do it at next frame)
		sphere->place(p);
		coll_cache->flush(sphere);

		sphere->move(sphere_vect.x*m,sphere_vect.y*m,sphere_vect.z*m);

		coll_cache->flush(sphere);
		if (sphere->collide(room,coll_cache))
		{
			// Still colliding, simply reverse vector

			sphere->place(p);
			coll_cache->flush(sphere);
			sphere_vect = ov;
			sphere_vect.x *= -1.0f;
			sphere_vect.y *= -1.0f;
			sphere_vect.z *= -1.0f;
		}
	}

	// Set sphere angles
	sphere_axis.convert(ax,ay,az);
	sphere->set_angle(ax,ay,az);

	// Viewport is looking the sphere zero

	v.x = sphere->getx() - viewport->getx();
	v.y = sphere->gety() - viewport->gety();
	v.z = sphere->getz() - viewport->getz();

	v.angles(ax,ay);
	viewport->set_angle(ax,ay,0);

}

void Collision::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_Event:
		{
			OMediaEvent	*event = (OMediaEvent*)param;
			if (event->type==omtet_KeyDown)
			{
				switch(event->special_key)
				{
					case omtsk_Escape:
					quit();
					break;
                                        
                                        default:
                                        break;
				}
				
			}		
		}
		break;	
	}

	OMediaApplication::listen_to_message(msg, param);
}

//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	Collision	*app;

	app = new Collision;		// Create and start application
	app->start();
	delete app;  
}

