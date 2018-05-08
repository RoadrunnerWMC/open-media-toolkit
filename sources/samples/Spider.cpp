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

// Flying around the famous Spider 

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
#include "OMediaMouseCursor.h"
#include "OMediaInputEngine.h"

//---------------------------------------------
// Definitions

	// Override standard OMT application class

class MouseObject;

class Spider :	public OMediaApplication,
				public OMediaPeriodical
{
public:

	Spider();
	virtual ~Spider();

	static Spider	*app;

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

	MouseObject				*spider;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;

	OMedia3DVector			move_v;		// Move vector
	float					ax,ay,az;	// Rotate camera

	OMedia3DAxis			camera_axis;	// Use a 3D axis object to maintain camera orientation.
											// It resolves trigo problem.

	OMediaTimer				timer;
};

Spider	*Spider::app;


class MouseObject : public OMedia3DShapeElement
{
	public:
	
	MouseObject() {process_mouse = false;map_x_to = -1; map_y_to = -1; mul_x =1; mul_y = 1;move_object = false;}
	virtual ~MouseObject() {}

	bool		process_mouse;
	short		mouse_ix,mouse_iy;
	short		map_x_to;
	short		map_y_to;
	short		mul_x,mul_y;
	bool		move_object;

	virtual void clicked(OMediaPickResult *res, bool mouse_down)
	{
		if (mouse_down && !process_mouse)
		{
			process_mouse = true;
			its_world->get_input_engine()->get_mouse_position(mouse_ix,mouse_iy);
			OMediaMouseCursor::hide();
		}
		else if (process_mouse)
		{
			process_mouse = false;
			OMediaMouseCursor::force_show();
		}
	}

	virtual void update_logic(float ms)
	{
		if (process_mouse && !its_world->get_input_engine()->mouse_down())
		{
			process_mouse = false;
			OMediaMouseCursor::force_show();
		}

		if (process_mouse)
		{
			short mx,my;
			omt_Angle	ax,ay,az;
			its_world->get_input_engine()->get_mouse_position(mx,my);

			if (!move_object || its_world->get_input_engine()->get_command_keys_status()&omtck_Shift)
			{
				ax = 0;
				ay = 0;
				az = 0;

				switch(map_x_to)
				{
					case 0: ax = (mx-mouse_ix)*10*mul_x; break;
					case 1: ay = (mx-mouse_ix)*10*mul_x; break;
					case 2: az = (mx-mouse_ix)*10*mul_x; break;
				}

				switch(map_y_to)
				{
					case 0: ax = (my-mouse_iy)*10*mul_y; break;
					case 1: ay = (my-mouse_iy)*10*mul_y; break;
					case 2: az = (my-mouse_iy)*10*mul_y; break;
				}

				add_angle(ax,ay,az);
			}
			else
			{
				OMedia3DVector	v;

				v.set(0,0,(my-mouse_iy)*-2);
				v.rotate(0,Spider::app->viewport->get_angley(),0);
				move(v);

				v.set((mx-mouse_ix)*2,0,0);
				v.rotate(0,Spider::app->viewport->get_angley(),0);
				move(v);	
			}

			mouse_ix = mx;
			mouse_iy = my;		
		}
	}



};


//---------------------------------------------
// Implementations

Spider::Spider()
{
	move_v.set(0,0,0);
	ax = ay = az = 0.0f;
	app = this;

	init_database();
	init_display();
	init_animation();
}

Spider::~Spider()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete monitors;

	close_database();
}

void Spider::init_database(void)
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

void Spider::close_database(void)
{
	delete database;
	delete dbfile;
}

void Spider::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(512,400);
	window->place(40,60);
	window->show();

	// * Get the first available video engine

	omt_EngineID video_engine = *(OMediaEngineFactory::get_factory()->video_engines.begin());
	//video_engine = ommeic_OS;	// GL +++ FIXME CLICK BUG

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

void Spider::init_animation(void)
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
	viewport->set_flags(omcvpf_EnableMouseClick|omcvpf_ClickActivateSupervisor);
	viewport->place(100,100,-100);


	// * Layer

	argb.set(1.0f,0.0f,0.0f,0.0f);

	layer = new OMediaLayer;
	layer->link(world);
	layer->add_flags(	omlayerf_ClearColor|omlayerf_ClearZBuffer|
						omlayerf_EnableZBufferWrite|omlayerf_EnableZBufferTest);

	layer->set_clear_color(argb);


	// * Light

	OMediaLight	*light;
	light = new OMediaLight;
	light->link(world);
	light->set_light_type(omclt_Point);
//	light->set_angle(omd_Deg2Angle(315),omd_Deg2Angle(25),0);
	light->place(0,80,-100);
	light->set_range(1000);
	light->set_constant_attenuation(2.0);
	
	// * Set filtering
	
	omd_GETOBJECT(database,OMediaCanvas,"spider")->set_filtering(omtfc_Linear,omtfc_Linear);
	omd_GETOBJECT(database,OMediaCanvas,"spidereye")->set_filtering(omtfc_Linear,omtfc_Linear);
	omd_GETOBJECT(database,OMediaCanvas,"ground")->set_filtering(omtfc_Linear,omtfc_Linear_Mipmap_Linear);

	// * Build the Spider

	OMedia3DShapeElement *e;
	MouseObject			*el;
	OMedia3DMaterial	*mat;


	el= new MouseObject;
	el->link(world);
	el->set_shape(omd_GETOBJECT(database,OMedia3DShape,"sofa"));
	el->place(0,0,0);
	el->move_object = true;
	el->map_x_to = 1;
//	el->map_y_to = 2;


	mat = omd_GETOBJECT(database,OMedia3DMaterial,"sofa");
	mat->set_shininess(10);


	spider = new MouseObject;
	spider->link(el);
	spider->set_shape(omd_GETOBJECT(database,OMedia3DShape,"spider"));
	spider->place(0,0,-40);
	spider->map_x_to = 1;

	mat = omd_GETOBJECT(database,OMedia3DMaterial,"spider");
	mat->set_shininess(10);
	
//	return;
	//+++


	el = new MouseObject;
	el->link(spider);
	el->set_shape(omd_GETOBJECT(database,OMedia3DShape,"spidereye"));
	el->place(-10,0,-85);
	el->map_x_to = 1;
	el->map_y_to = 0;

	el = new MouseObject;
	el->link(spider);
	el->set_shape(omd_GETOBJECT(database,OMedia3DShape,"spidereye"));
	el->place(10,0,-85);
	el->map_x_to = 1;
	el->map_y_to = 0;

	float z = -20;
	short i;

	MouseObject	*legs[6];
	MouseObject	*legs2[6];

	for(i = 0; i<3 ; i++)
	{
		legs[i] = new MouseObject;
		legs[i]->link(spider);
		legs[i]->set_shape(omd_GETOBJECT(database,OMedia3DShape,"spiderleg"));
		legs[i]->place(-30,-10,z);
		legs[i]->set_angle(omd_Deg2Angle(10),0,omd_Deg2Angle(45));
		legs[i]->map_x_to = 1;
		legs[i]->map_y_to = 2;
		legs[i]->mul_y = -1;

		z += 40;
	}

	legs[1]->move(-5,0,0);
	legs[0]->add_angle(0,omd_Deg2Angle(25),0);
	legs[2]->add_angle(0,-omd_Deg2Angle(10),0);


	z = -20;
	for(i = 3; i<6 ; i++)
	{
		legs[i] = new MouseObject;
		legs[i]->link(spider);
		legs[i]->set_shape(omd_GETOBJECT(database,OMedia3DShape,"spiderleg"));
		legs[i]->place(30,-10,z);
		legs[i]->set_angle(-omd_Deg2Angle(10),omd_Deg2Angle(180),omd_Deg2Angle(45));
		legs[i]->map_x_to = 1;
		legs[i]->map_y_to = 2;
		legs[i]->mul_y = -1;

		z += 40;
	}

	legs[4]->move(5,0,0);
	legs[3]->add_angle(0,-omd_Deg2Angle(25),0);
	legs[5]->add_angle(0,omd_Deg2Angle(10),0);

	for(i = 0; i<3 ; i++)
	{
		legs2[i] = new MouseObject;
		legs2[i]->link(legs[i]);
		legs2[i]->set_shape(omd_GETOBJECT(database,OMedia3DShape,"spiderleg"));
		legs2[i]->place(-85,0,0);
		legs2[i]->set_angle(0,0,-omd_Deg2Angle(110));
		legs2[i]->map_x_to = 0;
		legs2[i]->map_y_to = 2;
		legs2[i]->mul_x = -1;
		legs2[i]->mul_y = -1;
	}

	for(i = 3; i<6 ; i++)
	{
		legs2[i] = new MouseObject;
		legs2[i]->link(legs[i]);
		legs2[i]->set_shape(omd_GETOBJECT(database,OMedia3DShape,"spiderleg"));
		legs2[i]->place(-85,0,0);
		legs2[i]->set_angle(0,0,-omd_Deg2Angle(110));
		legs2[i]->map_x_to = 0;
		legs2[i]->map_y_to = 2;
		legs2[i]->mul_x = -1;
		legs2[i]->mul_y = -1;
	}

	

	e = new OMedia3DShapeElement;
	e->link(world);
	e->set_shape(omd_GETOBJECT(database,OMedia3DShape,"ground"));
	e->place(0,-150,0);



/*	e = new OMedia3DShapeElement;
	e->link(world);
	e->set_shape(omd_GETOBJECT(database,OMedia3DShape,"table"));
	e->place(0,-150,-200);
*/
}

// * Listen to message

void Spider::listen_to_message(omt_Message msg, void *param)
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
					else move_v.z = 300/1000.0f;					
					break;

					case omtsk_ArrowBottom:
					if (event->command_key&omtck_Shift)
						move_v.y = -(300/1000.0f);
					else 
						move_v.z = -(300/1000.0f);
					break;
					
					case omtsk_ArrowLeft:
						move_v.x = -(300/1000.0f);
					break;

					case omtsk_ArrowRight:
						move_v.x = (300/1000.0f);
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

void Spider::spend_time(void)
{
	omt_Angle			angx,angy; //,angz;
	OMedia3DVector		motion_dir;
	float				elapsed;

	elapsed = (float)timer.getelapsed();

/*	// Rotate camera

	camera_axis.rotate(omc3daxis_Pitch, omt_Angle(ax*elapsed));	
	camera_axis.rotate(omc3daxis_Yaw,	omt_Angle(ay*elapsed));
	camera_axis.rotate(omc3daxis_Roll,	omt_Angle(az*elapsed));

	// Convert camera axis to absolute angles

	camera_axis.inv_convert(angx,angy,angz);
*/
	motion_dir = move_v;				// Get direction vector
	motion_dir.x *= elapsed;			// Time based
	motion_dir.y *= elapsed;
	motion_dir.z *= elapsed;
/*	motion_dir.rotate(angx,angy,angz);	// Transform to camera orientation
*/
	// Set up viewport

	viewport->move(motion_dir);

	// Look at spider

	OMedia3DVector	v;
	OMedia3DPoint	p1,p2;
	float			x,y,z;

//	spider->get_absolute_info(x,y,z,angx,angy,angz);
	x = y = z =0;

	p1.set(viewport->getx(),viewport->gety(),viewport->getz());
	p2.set(x,y,z);
	v.set(p1,p2);
	v.angles(angx,angy);

	viewport->set_angle(angx,angy,0);




	timer.start();
}


//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	Spider	*app;

	app = new Spider;		// Create and start application
	app->start();
	delete app;  
}

