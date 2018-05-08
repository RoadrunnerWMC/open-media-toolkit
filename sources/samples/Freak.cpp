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

// Freak.cpp


#include "OMediaApplication.h"
#include "OMediaSingleWindow.h"
#include "OMediaMonitorMap.h"
#include "OMediaCanvas.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaRendererDef.h"
#include "OMediaCaption.h" 
#include "OMediaWorld.h"
#include "OMediaPeriodical.h"
#include "OMediaTimer.h"
#include "OMediaDatabase.h"
#include "OMediaListener.h"
#include "OMediaCanvasAnim.h"
#include "OMediaCanvasAnimFrame.h"
#include "OMediaViewport.h"

//---------------------------------------------
// Definitions

enum freak_Scroll
{
	scroll_Left,
	scroll_Right,
	scroll_None
};


	// Override standard OMT application class

class Freak :	public OMediaApplication,
				public OMediaPeriodical,
				public OMediaListener				
{
public:

	Freak();
	virtual ~Freak();

	void init_display(void);
	void init_world(void);
	void init_freak(void);

	void listen_to_message(omt_Message msg, void *param);

	void init_database(void);
	void close_database(void);

	void update_worldsize(void);
	
	void spend_time(void);

	void process_key(OMediaEvent* event);


	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;
	
	OMediaCanvasElement		*background[2];
	
	OMediaTimer				timer;
	
	freak_Scroll			scroll;
	
	OMediaCanvas			*bg_canvas;

	OMediaCanvasAnim		*anim;
	OMediaCanvasAnimDef		*anim_def;

};


//---------------------------------------------
// Implementations

Freak::Freak()
{
	world = NULL;
	background[0] = background[1] = NULL;
	scroll = scroll_None;

	init_database();
	
	bg_canvas = omd_GETOBJECT(database,OMediaCanvas,"FreakBackground");
	
	init_display();
	init_world();
	init_freak();
}

Freak::~Freak()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete anim_def;
	delete monitors;

	close_database();
}

void Freak::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(bg_canvas->get_width(),bg_canvas->get_height());
	window->place(40,60);
	window->show();

	// I need to listen to window omsg_WindowBoundsChanged message. This way I can scale the
	// animation to the window size.

	window->addlistener(this);


	// * Create a monitor maps
	
	// A monitor map automatically creates for you one video engine
	// per card installed in your machine. It allows OMT to take
	// care of multiple monitors automatically for you.

	omt_EngineID video_engine = *(OMediaEngineFactory::get_factory()->video_engines.begin());

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
//			if ((*ri).attributes & omcrdattr_Accelerated ) def = &(*ri);	// Accelerated ?
			
			if ((*ri).engine_id==ommeic_OMT) def = &(*ri);


		}

		// No accelerated device. Take the first one.
		if (!def && (*vi)->get_renderer_list()->size()) def =  &(*(*vi)->get_renderer_list()->begin());


		// Once I found a renderer I need to select it. This will automatically bFreakld the renderer for me.
		if (def) (*vi)->select_renderer(def,omfzbc_NoZBuffer);
	}
}

void Freak::listen_to_message(omt_Message msg, void *param)
{
	// I override this method in order to receive window and event messages.

	switch(msg)
	{
		case omsg_WindowBoundsChanged:
		update_worldsize();
		break;
		
		case omsg_Event:
		{
			OMediaEvent	*event = (OMediaEvent*)param;
			if ((event->type==omtet_KeyDown) || (event->type==omtet_KeyUp))
			{					
				process_key(event);
			}
			else 
			{
				// The application handles other events (like window refresh, etc.). So let's pass
				// the message to it when it's not a key event.
			
				OMediaApplication::listen_to_message(msg, param);
			}
		}
		break;
		
		default:
		
		// Not a message for me, just pass it to the parent class
		
		OMediaApplication::listen_to_message(msg, param);
	}
}

void Freak::process_key(OMediaEvent* event)
{
	if (event->type==omtet_KeyUp) scroll = scroll_None;
	else if (event->special_key==omtsk_ArrowLeft)
	{
		scroll = scroll_Left;
		anim->set_angley(0);
	}
	else if (event->special_key==omtsk_ArrowRight)
	{
		scroll = scroll_Right;	
		anim->set_angley(omd_Deg2Angle(180));
	}
}

void Freak::init_world(void)
{
	OMediaRect			r;

	// * World

	world = new OMediaWorld;	// Create the root class


	// * Layer

	layer = new OMediaLayer;
	layer->link(world);

	// We don't need to clear the background
	layer->remove_flags(omlayerf_ClearColor);

	// The elements are sorted from the smaller to the bigger z value
	layer->add_flags(omlayerf_SortElementBack2Front);


	// I set a default ortho matrix for this layer. No perspective, pure 2D!
	layer->set_projection_type(omlptc_Ortho);

	// Because there is no perspective I can set the near clip plane to zero.
	// Objects with a z<near_clip are not drawn. So if you're doing 2D stuff and
	// don't want to worry about the z component, just set it to zero.
	// Never do that if you set a perspective matrix!
	
	layer->set_near_clip(0.0f);		

	// * Viewport

	// Please note that in this example, I create the viewport after the layer. This is because
	// I want the viewport to be linked to a layer (the link method always links to the last layer
	// any new created element). This is because I'm linking an element to the viewport and an element
	// is rendered only if the root supervisor is linked to a layer. As long as you don't link elements
	// to the viewport, the viewport does not have to be linked to a layer.
	

	viewport = new OMediaViewPort(window);		// Supervisor is window
	viewport->link(world);
	viewport->link_window(window);					// Output is window
	
	// We need to enable mouse click support
	
	viewport->set_flags(omcvpf_EnableMouseClick|omcvpf_ClickActivateSupervisor);
	
	r.set(0,0,0,0);		// Viewport bounds are right/bottom relative, so
						// when the window is resized, the viewport is resized
						// too.

	viewport->setbounds(&r);
	viewport->set_bounds_mode(omcpbc_Right,omvpbmc_RightRelative);
	viewport->set_bounds_mode(omcpbc_Bottom,omvpbmc_BottomRelative);

}

void Freak::init_freak(void)
{
	short i;

	// Background image

	for(i=0;i<2; i++)
	{
		background[i] = new OMediaCanvasElement;
		background[i]->link(world);		
		background[i]->set_canvas(bg_canvas);
		background[i]->set_auto_align(omaac_Center,omaac_Center);
		background[i]->set_canvas_flags(omcanef_FreeWorldSize);
		background[i]->place(0,0,10.0f);
	}
	

	// The creature

	// Create the anim definition

	OMediaCanvasAnimFrame	*frame;

	anim_def = new OMediaCanvasAnimDef;

	// The static sequence
	
	for(i=0;i<3;i++)
	{
		frame = new OMediaCanvasAnimFrame;
		frame->link(anim_def,0);
		frame->set_canvas(omd_GETOBJECT(database,OMediaCanvas,2+i));
		frame->setframespeed_tb(16);
	}

	// Walk sequence

	for(i=0;i<3;i++)
	{
		frame = new OMediaCanvasAnimFrame;
		frame->link(anim_def,1);
		frame->set_canvas(omd_GETOBJECT(database,OMediaCanvas,5+i));
		frame->setframespeed_tb(16);
	}


	// Create the anim object


	anim = new OMediaCanvasAnim;
	anim->link(viewport);				// Link to viewport, so it moves with it
	anim->setplay_loop(true);
	anim->setplay_pingpong(true);
	anim->setplay_timebased(true);
	anim->set_auto_align(omaac_Center,omaac_Center);
	anim->set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
	anim->place(0,-40);
	anim->set_anim_def(anim_def);
	

	update_worldsize();
}

void Freak::update_worldsize(void)
{
	if (!background[0]) return;

	// Well, I need to scale the elements to the window size. 

	float	w = (float)window->get_width(),
			h = (float)window->get_height();

	for(short i=0;i<2;i++)
	{
		background[i]->set_size(w,h);
	}

	background[1]->place(w-0.5f,0);

	float bw = background[0]->get_width();
	
	if (viewport->getx()<0)
	{
		viewport->move(bw,0);		
	}
	else if (viewport->getx()> bw)
	{
		viewport->move(-bw,0);
	} 

	// Replace also the creature (without scaling it, it's cooler)
	
	if (window->get_height() <= bg_canvas->get_height()) anim->place(0,-40);
	else
	{
		anim->place(0,-40 * float(window->get_height()) / float(bg_canvas->get_height()) * 1.5);
	}
}


void Freak::init_database(void)
{
	OMediaFilePath			path("medias/samples.omt");

	// Set the cache to zero. In this mode, the OMT database never removes objects
	// from memory (until you delete the database).

	OMediaDataBase::set_cache_size(0);

	// We need to register the OMT classes we'll get from the database: 

	omd_REGISTERCLASS(OMediaCanvas);

	dbfile = new OMediaFileStream;
	dbfile->setpath(&path);
	dbfile->open(omcfp_Read);			// Read only

	database = new OMediaDataBase(dbfile);	// Open database
}

void Freak::close_database(void)
{
	delete database;
	delete dbfile;
}

void Freak::spend_time(void)
{
	// This method overrides the OMediaPeriodical::spend_time method. It is called as fast as possible.

	if (!viewport) return;

	// Scroll the world if required


	if (scroll!=scroll_None)
	{
		float	elapsed = (float) timer.getelapsed();
		float	v,bw;
		
		v =  elapsed * 100.0f/1000.0f;	// 100 units per second
		if (scroll==scroll_Left) v = -v;
			
		viewport->move(v,0);
		
		bw = background[0]->get_width();
		
		if (viewport->getx()<0)
		{
			viewport->move(bw,0);		
		}
		else if (viewport->getx()> bw)
		{
			viewport->move(-bw,0);
		}
		
		anim->setcurrentsequence(1);
	}
	else anim->setcurrentsequence(0);

	timer.start();
}



//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	Freak	*app;

	app = new Freak;		// Create and start application
	app->start();
	delete app;  
}

