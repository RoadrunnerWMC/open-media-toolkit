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

// Doing a viewport shot and saving it as a PNG file

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

// Menu

const omt_Message		msg_Shot = 10;

OMediaMenu	menu_def[] = 
{
	{ommcmdc_Menu,		"File",				omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"New",				omsg_New, 			'N', 	0},
	{ommcmdc_Item,		"Open...",			omsg_Open, 			'O', 	0},
	{ommcmdc_Item,		"Close",			omsg_Close, 		'W', 	0},
	{ommcmdc_Separator,	NULL,			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Save",				omsg_Save, 			'S', 	0},
	{ommcmdc_Item,		"Save As...",		omsg_SaveAs, 		0, 		0},
	{ommcmdc_Item,		"Revert",			omsg_Revert, 		0, 		0},
	{ommcmdc_Separator,	NULL,			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Page Setup...",	omsg_PageSetup, 	0, 		0},
	{ommcmdc_Item,		"Print...",			omsg_Print, 		'P', 	0},
	{ommcmdc_Separator,	NULL,			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Quit",				omsg_Quit, 			'Q', 	0},

	{ommcmdc_Menu,		"Edit",				omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Undo",				omsg_Undo, 			'Z', 	0},
	{ommcmdc_Separator,	NULL,			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Copy",				omsg_Copy, 			'C', 	0},
	{ommcmdc_Item,		"Cut",				omsg_Cut, 			'X', 	0},
	{ommcmdc_Item,		"Paste",			omsg_Paste, 		'V', 	0},
	{ommcmdc_Item,		"Clear",			omsg_Clear, 		0, 		0},
	{ommcmdc_Separator,	NULL,			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Select All",		omsg_SelectAll, 	'A', 	0},

	{ommcmdc_Menu,		"Snapshot",			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Shot...",			msg_Shot, 			'K', 	0},


	{ommcmdc_AboutMenu,	"Help",				omsg_NULL, 			0, 		0},
	{ommcmdc_AboutItem,	"About...",			omsg_About, 		0, 		0},

	{ommcmdc_End,		NULL,			omsg_NULL, 			0, 		0}
};

// Override standard OMT application class

class Screenshot :	public OMediaApplication,
					public OMediaPeriodical
{
public:

	Screenshot();
	virtual ~Screenshot();

	void init_display(void);
	void init_font(void);
	void init_world(void);
	void init_demo(void);

	void init_database(void);
	void close_database(void);

	virtual void listen_to_message(omt_Message msg, void *param);
	virtual bool update_message_state(omt_Message msg, 
												bool &enabled,
												bool &mark);

	void vpshot(void);

	void spend_time(void);

	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;

	OMedia3DShape			*shape;
	OMedia3DMaterial		*material;

	bool					shot;
};


//---------------------------------------------
// Implementations

Screenshot::Screenshot()
{
	shot = false;

	init_database();
	init_display();
	init_world();
	init_demo();
}

Screenshot::~Screenshot()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete monitors;

	delete shape;
	delete material;
	close_database();
}

void Screenshot::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(menu_def);
	window->set_size(512,512);
	window->place(40,60);
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
			if ( (*ri).engine_id==ommeic_OMT
				/*(*ri).engine_id==ommeic_OpenGL &&*/ 
				/*((*ri).attributes & omcrdattr_Accelerated )*/ ) def = &(*ri);	// Accelerated ?
		}

		// No accelerated device. Take the first one.
		if (!def && (*vi)->get_renderer_list()->size()) def =  &(*(*vi)->get_renderer_list()->begin());


		// Once I found a renderer I need to select it. This will automatically build the renderer for me.
		if (def) (*vi)->select_renderer(def,def->zbuffer_depth);
	}
}

void Screenshot::init_world(void)
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

void Screenshot::init_database(void)
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

void Screenshot::close_database(void)
{
	delete database;
	delete dbfile;
}

void Screenshot::init_demo(void)
{
	// I create the sphere at runtime and map the OMT logos:

	// Get the canvas

	OMediaCanvas	*canvas;
	canvas = omd_GETOBJECT(database,OMediaCanvas,"planet1");

	// I need a 3D material:

	OMediaFRGBColor	rgb(1.0f,1.0f,1.0f);

	material = new OMedia3DMaterial;
	material->set_shade_mode(omshademc_Gouraud);
	material->set_light_mode(ommlmc_Light);
	material->set_texture(canvas);
	material->set_color(rgb,0.0f,0.6f,1.0f,0.0f);


	// Create the shape

	shape = new OMedia3DShape;
	shape->make_sphere(80,16,24);
	shape->set_material(material);


	// Create element

	OMedia3DShapeElement *e = new OMedia3DShapeElement;
	e->link(world);
	e->place(0,0,300);
	e->set_shape(shape);
	e->set_angle(0,omd_Deg2Angle(90),0);
}

void Screenshot::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case msg_Shot:
		shot = true;
		break;

		default:
		OMediaApplication::listen_to_message(msg, param);
		break;
	}
}

bool Screenshot::update_message_state(omt_Message msg, 
												bool &enabled,
												bool &mark)
{
	switch(msg)
	{
		case msg_Shot:
		enabled = true;
		return true;

		default:
		return OMediaApplication::update_message_state(msg, enabled, mark);
	}

}

void Screenshot::spend_time(void)
{
	if (shot)
	{
		shot = false;
		vpshot();
	}
}


void Screenshot::vpshot(void)
{
	OMediaFilePath		path;
	OMediaFileStream	file;
	OMediaAskUserFilters	filter_list;
	OMediaAskUserFilter		filter;
	
	filter.file_type = 'IPNG';
	filter.descriptor = "PNG (*.PNG)";
	filter.pattern = "*.PNG";
	filter_list.filters.push_back(filter);

	OMediaCanvas	canvas;
	viewport->capture_frame(canvas);

	if (path.ask_user(window, filter_list, true, "Save shot...", "Save shot :", "shot.png"))
	{
		file.setpath(&path);
		file.open(omcfp_Write,true,true);
		canvas.png_export(file);
		file.close();
	}
}

//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	Screenshot	*app;

	app = new Screenshot;		// Create and start application
	app->start();
	delete app;  
}

