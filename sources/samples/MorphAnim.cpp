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

// Show how to import an animation from an external file format and
// how to play linear-interpolation between keyframes.


#include "OMediaApplication.h"
#include "OMediaSingleWindow.h"
#include "OMediaMonitorMap.h"
#include "OMediaMemStream.h"
#include "OMediaRendererDef.h"
#include "OMediaWorld.h"
#include "OMediaViewPort.h"
#include "OMediaLayer.h"
#include "OMediaDataBase.h"
#include "OMedia3DMorphAnim.h"
#include "OMediaLight.h"
#include "OMediaPeriodical.h"
#include "OMedia3DAxis.h"
#include "OMediaTimer.h"
#include "OMediaFileStream.h"
#include "OMediaFilePath.h"
#include "OMedia3DShapeConverter.h"
#include "OMediaError.h"

//---------------------------------------------
// Definitions

	// Override standard OMT application class

class MorphAnim :	public OMediaApplication,
					public OMediaPeriodical
{
public:

	MorphAnim();
	virtual ~MorphAnim();

	void init_display(void);
	void init_world(void);
	void init_animation(void);

	void listen_to_message(omt_Message msg, void *param);
	void spend_time(void);

	OMediaMemStream			*memstream;
	OMediaDataBase			*database;

	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;

	OMedia3DVector			move_v;		// Move vector
	float					ax,ay,az;	// Rotate camera

	OMedia3DAxis			camera_axis;	// Use a 3D axis object to maintain camera orientation.

	OMediaTimer				timer;
};


//---------------------------------------------
// Implementations

MorphAnim::MorphAnim()
{
	move_v.set(0,0,0);
	ax = ay = az = 0.0f;

	init_display();
	init_world();
	init_animation();
}

MorphAnim::~MorphAnim()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete database;
	delete memstream;
	
	delete monitors;
}

void MorphAnim::init_display(void)
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

void MorphAnim::init_world(void)
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

	argb.set(1.0f,0.4f,0.4f,0.4f);

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


}

// * Listen to message

void MorphAnim::listen_to_message(omt_Message msg, void *param)
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

void MorphAnim::spend_time(void)
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

void MorphAnim::init_animation(void)
{
	OMedia3DShape	*shape;

	// Load a 3DS animation file. For this sample, I just create a temporary
	// database in memory and import the 3DS file to it. Then I get the
	// animation definition. 

	// First of all I need a database. I build it on the top of a memory stream:

	memstream = new OMediaMemStream;
	database = new OMediaDataBase(memstream);

	// Let's register streameable classes

	omd_REGISTERCLASS(OMediaCanvas);
	omd_REGISTERCLASS(OMedia3DShape);
	omd_REGISTERCLASS(OMedia3DMaterial);
	omd_REGISTERCLASS(OMedia3DMorphAnimDef);


	// Get an empty 3D shape

	shape = omd_GETOBJECT(database,OMedia3DShape,0);

	// Now open the 3DS file:

	OMediaFilePath		path("medias/MorphAnim.ase");
	OMediaFileStream	file;

	file.setpath(&path);
	file.open(omcfp_Read,false,false);

	// Ask importer to generate an animation definition. It will stored in the database
	// as an OMedia3DMorphAnimDef object.

	OMedia3DShapeConverter::set_anim_conv_mode_mode(omc3danconv_AlwaysCreateNewMorphAnimDef);

	// Import the ASE file to the shape object - materials and animation definition are generated as 
	// seperated database objects.

	database->generate_from_stream(file,shape);

	// You can get the created morph defs using the following list. This list is generated each time a new
	// shape is imported.

	if (!OMedia3DShapeConverter::related_morphdefs.size()) omd_EXCEPTION(omcerr_CantBuild);

	OMedia3DMorphAnimDef	*def = omd_GETOBJECT(database,OMedia3DMorphAnimDef,OMedia3DShapeConverter::related_morphdefs[0].morph_def);

	// Now create a morph element that uses the morph anim definition

	OMedia3DMorphAnim	*anim;

	anim = new OMedia3DMorphAnim;
	anim->link(world);
	anim->place(80,-100,400);
	anim->set_anim_def(def);
	anim->set_shape(shape);
	anim->set_morph_flags(ommorf_InterpolateVertices|ommorf_InterpolateVNormals);

	def->db_unlock();
	shape->db_unlock();
}

//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	MorphAnim	*app;

	app = new MorphAnim;		// Create and start application
	app->start();
	delete app;  
}

