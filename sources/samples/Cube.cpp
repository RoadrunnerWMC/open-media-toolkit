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

// Simple cube application

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


//---------------------------------------------
// Definitions

	// Override standard OMT application class

class Cube : public OMediaApplication
{
public:

	Cube();
	virtual ~Cube();

	void init_display(void);
	void init_font(void);
	void init_world(void);
	void init_cube(void);

	void init_database(void);
	void close_database(void);

	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;

	OMedia3DShape			*cube_shape;
	OMedia3DMaterial		*cube_material;
};

	// Override OMT shape element

class CubeElement : public OMedia3DShapeElement
{
public:
	
	CubeElement();
	virtual ~CubeElement();

	// Following method is called for each element before rendering

	virtual void update_logic(float millisecs_elapsed);

};

//---------------------------------------------
// Implementations

Cube::Cube()
{
	init_database();
	init_display();
	init_world();
	init_cube();
}

Cube::~Cube()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete monitors;

	delete cube_shape;
	delete cube_material;
	close_database();
}

void Cube::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(512,512);
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

void Cube::init_world(void)
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


	// * Layer

	argb.set(1.0f,0,0,0);

	layer = new OMediaLayer;
	layer->link(world);
	layer->add_flags(omlayerf_ClearColor);
	layer->set_clear_color(argb);


	// * Light

	OMediaLight	*light;
	light = new OMediaLight;
	light->link(world);
	light->set_light_type(omclt_Directional);
	light->set_angle(omd_Deg2Angle(45),omd_Deg2Angle(25),0);
}

void Cube::init_database(void)
{
	OMediaFilePath			path("medias/samples.omt");

	// Set the cache to zero. In this mode, OMT database never removes objects
	// from memory (until you delete the database).

	OMediaDataBase::set_cache_size(0);

	// We need to register the OMT classes we'll get from the database: 

	omd_REGISTERCLASS(OMediaCanvas);

	dbfile = new OMediaFileStream;
	dbfile->setpath(&path);
	dbfile->open(omcfp_Read);			// Read only

	database = new OMediaDataBase(dbfile);	// Open database
}

void Cube::close_database(void)
{
	delete database;
	delete dbfile;
}

void Cube::init_cube(void)
{
	// I create the cube at runtime and map the OMT logos:

	// Get the canvas

	OMediaCanvas	*canvas;
	canvas = omd_GETOBJECT(database,OMediaCanvas,0);

	// I need a 3D material:

	OMediaFRGBColor	rgb(1.0f,1.0f,1.0f);

	cube_material = new OMedia3DMaterial;
	cube_material->set_shade_mode(omshademc_Flat);
	cube_material->set_light_mode(ommlmc_Light);
	cube_material->set_texture(canvas);
	cube_material->set_color(rgb,0.0f,0.6f,1.0f,0.0f);


	// Create the shape

	cube_shape = new OMedia3DShape;
	cube_shape->make_cube(80);
	cube_shape->set_material(cube_material);


	// Create element

	CubeElement *cube = new CubeElement;
	cube->link(world);
	cube->place(0,0,300);
	cube->set_shape(cube_shape);

	// Play a bit with hierarchy

	CubeElement *cube2 = new CubeElement;
	cube2->link(cube);
	cube2->place(0,120,120);
	cube2->set_shape(cube_shape);

	CubeElement *cube3 = new CubeElement;
	cube3->link(cube);
	cube3->place(0,-120,-120);
	cube3->set_shape(cube_shape);
}


//---------------------------------------------
// Animated element

CubeElement::CubeElement() {}
CubeElement::~CubeElement() {}

void CubeElement::update_logic(float millisecs_elapsed)
{
	OMedia3DShapeElement::update_logic(millisecs_elapsed);

	add_angle(0,(int)(millisecs_elapsed* (float(omd_Deg2Angle(90))/1000.0f)), 0);
}

//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	Cube	*app;

	app = new Cube;		// Create and start application
	app->start();
	delete app;  
}

