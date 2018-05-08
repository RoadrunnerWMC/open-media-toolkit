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

// Flying around the famous teapot

#include "OMediaApplication.h"
#include "OMediaSingleWindow.h"
#include "OMediaMonitorMap.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasFont.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaRendererDef.h"
#include "OMediaCaption.h"
#include "OMediaWorld.h"
#include "OMediaViewPort.h"
#include "OMediaLayer.h"
#include "OMediaDataBase.h"
#include "OMediaFileStream.h"
#include "OMedia3DShapeElement.h"
#include "OMedia3DShape.h"
#include "OMedia3DMaterial.h"
#include "OMediaLight.h"
#include "OMediaPeriodical.h"
#include "OMedia3DAxis.h"
#include "OMediaTimer.h"
#include "OMediaEngineID.h"

//---------------------------------------------
// Definitions

	// Override standard OMT application class

class Teapot :	public OMediaApplication,
				public OMediaPeriodical
{
public:

	Teapot();
	virtual ~Teapot();

	void init_display(void);
	void init_animation(void);

	void init_database(void);
	void close_database(void);

	void listen_to_message(omt_Message msg, void *param);
	void spend_time(void);

	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;

	OMedia3DShapeElement	*teapot;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;

	OMedia3DVector			move_v;		// Move vector
	float					ax,ay,az;	// Rotate camera

	OMedia3DAxis			camera_axis;	// Use a 3D axis object to maintain camera orientation.
											// It resolves trigo problem.

	OMediaTimer				timer;

};



//---------------------------------------------
// Implementations

Teapot::Teapot()
{
	move_v.set(0,0,0);
	ax = ay = az = 0.0f;

	init_database();
	init_display();
	init_animation();
}

Teapot::~Teapot()
{

	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete monitors;

	close_database();

}

void Teapot::init_database(void)
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

void Teapot::close_database(void)
{
	delete database;
	delete dbfile;
}

void Teapot::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(512,400);
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
			if ((*ri).attributes & omcrdattr_Accelerated ) def = &(*ri);	// Accelerated ?
		}

		// No accelerated device. Take the first one.
		if (!def && (*vi)->get_renderer_list()->size()) def =  &(*(*vi)->get_renderer_list()->begin());


		// Once I found a renderer I need to select it. This will automatically build the renderer for me.
		if (def) (*vi)->select_renderer(def,def->zbuffer_depth);
	}
}

void Teapot::init_animation(void)
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
	layer->add_flags(	omlayerf_ClearColor|omlayerf_ClearZBuffer|
						omlayerf_EnableZBufferWrite|omlayerf_EnableZBufferTest);

	layer->set_clear_color(argb);


	// * Light

	OMediaLight	*light;
	light = new OMediaLight;
	light->link(world);
	light->set_light_type(omclt_Directional);
	light->set_angle(omd_Deg2Angle(315),omd_Deg2Angle(25),0);


	// * Build the teapot

	teapot = new OMedia3DShapeElement;
	teapot->link(world);
	teapot->set_shape(omd_GETOBJECT(database,OMedia3DShape,"teapot"));
	teapot->place(0,0,600);
}

// * Listen to message

void Teapot::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		// Handle keys

		// Forward/backward: top/bottom arrows
		// Yaw: left/right arrows
		// Side move: left/right arrows + shift
		// Up/down move: top/bottom arrows + shift
		// Roll: left/right arrows + control
		// Pitch: top/bottom arrows + control

		case omsg_Event:
		{
			OMediaEvent	*event = (OMediaEvent*) param;
		
			if (event->type==omtet_KeyDown)
			{			
				switch(event->special_key)
				{
					case omtsk_ArrowTop:
					if (event->command_key&omtck_Shift)
						move_v.y = (300/1000.0f);					
					else if (event->command_key&omtck_Control)
						ax = omd_Deg2Angle(60)/1000.0f;					
					else move_v.z = 300/1000.0f;					
					break;

					case omtsk_ArrowBottom:
					if (event->command_key&omtck_Shift)
						move_v.y = -(300/1000.0f);
					else if (event->command_key&omtck_Control)
						ax = -omd_Deg2Angle(60)/1000.0f;					
					else 
						move_v.z = -(300/1000.0f);
					break;
					
					case omtsk_ArrowLeft:
					if (event->command_key&omtck_Shift)
						move_v.x = -(300/1000.0f);
					else if (event->command_key&omtck_Control)
						az = omd_Deg2Angle(60)/1000.0f;					
					else
						ay = omd_Deg2Angle(60)/1000.0f;
					break;

					case omtsk_ArrowRight:
					if (event->command_key&omtck_Shift)
						move_v.x = (300/1000.0f);
					else if (event->command_key&omtck_Control)
						az = -omd_Deg2Angle(60)/1000.0f;					
					else
						ay = -omd_Deg2Angle(60)/1000.0f;
					break;			
                                        
                                                default:
                                                break;	
				}
			}
			else if (event->type==omtet_KeyUp)
			{
				move_v.set(0,0,0);
				ax = ay = az = 0;
			}
			else OMediaApplication::listen_to_message(msg, param);	
		}
		break;
		
		default:
		OMediaApplication::listen_to_message(msg, param);	
	}	
}

void Teapot::spend_time(void)
{
	omt_Angle			angx,angy,angz;
	OMedia3DVector		motion_dir;
	float				elapsed;

	elapsed = (float)timer.getelapsed();

	// Rotate camera

	camera_axis.rotate(omc3daxis_Pitch, omt_Angle(ax*elapsed));	
	camera_axis.rotate(omc3daxis_Yaw,	omt_Angle(ay*elapsed));
	camera_axis.rotate(omc3daxis_Roll,	omt_Angle(az*elapsed));

	// Convert camera axis to absolute angles

	camera_axis.inv_convert(angx,angy,angz);

	motion_dir = move_v;				// Get direction vector
	motion_dir.x *= elapsed;			// Time based
	motion_dir.y *= elapsed;
	motion_dir.z *= elapsed;
	motion_dir.rotate(angx,angy,angz);	// Transform to camera orientation

	// Set up viewport

	viewport->move(motion_dir);
	viewport->set_angle(angx,angy,angz);


	timer.start();
}


//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	Teapot	*app;

	app = new Teapot;		// Create and start application
	app->start();
	delete app;  
}

