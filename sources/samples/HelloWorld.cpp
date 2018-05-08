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

// Simple hello world application

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


//---------------------------------------------
// Definitions

	// Override standard OMT application class

class HelloWorld : public OMediaApplication
{
public:

	HelloWorld();
	virtual ~HelloWorld();

	void init_display(void);
	void init_font(void);
	void init_animation(void);

	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaCanvas			*font_canvas;
	OMediaCanvasFont		*font;

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;
};

	// Override OMT caption

class HelloWorldCaption : public OMediaCaption
{
public:
	
	HelloWorldCaption();
	virtual ~HelloWorldCaption();

	// Following method is called for each element before rendering

	virtual void update_logic(float millisecs_elapsed);

};

//---------------------------------------------
// Implementations

HelloWorld::HelloWorld()
{
	init_display();
	init_font();
	init_animation();
}

HelloWorld::~HelloWorld()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete font;
	delete font_canvas;

	delete monitors;
}

void HelloWorld::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(512,512);
	window->place(40,80);
	window->show();

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
			if ((*ri).attributes & omcrdattr_Accelerated ) def = &(*ri);	// Accelerated ?
		}

		// No accelerated device. Take the first one.
		if (!def && (*vi)->get_renderer_list()->size()) def =  &(*(*vi)->get_renderer_list()->begin());


		// Once I found a renderer I need to select it. This will automatically build the renderer for me.
		if (def) (*vi)->select_renderer(def,omfzbc_NoZBuffer);
	}
}

void HelloWorld::init_font(void)
{
	// Normally the font canvas should be stored in an OMT database. However since
	// the MediaMeister V2.0 has not been released there is no way to edit an image with an
	// alpha-channel at this time. We used the alpha-channel to generate anti-aliased fonts.
	// Now for the DR1, I stored the font in a PNG image.

	// OMT automatically recognizes the PNG format.

	// Open the file:

	OMediaFilePath		path("medias/font.png");	// path
	OMediaFileStream	file;				// file

	file.setpath(&path);
	file.open(omcfp_Read,false,false);

	font_canvas = new OMediaCanvas;
	file>>font_canvas;				// read the image


	// Now I need to build the font:

	font = new OMediaCanvasFont;
	font->set_font_canvas(font_canvas, font_canvas->get_width(), 76, 77);
	font->set_proportional(true);
	font->create_proportional_tab();
	font->set_space_size(10);
	font->set_char_space(3);
}

void HelloWorld::init_animation(void)
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

	argb.set(1.0f,0.9f,0.9f,0.9f);

	layer = new OMediaLayer;
	layer->link(world);
	layer->add_flags(omlayerf_ClearColor);
	layer->set_clear_color(argb);


	// * Caption

	HelloWorldCaption *capt = new HelloWorldCaption;
	capt->link(world);
	capt->set_font(font);
	capt->set_string("Hello World!");
	capt->place(0,0,800);
	capt->set_auto_align(omaac_Center, omaac_Center);
}


//---------------------------------------------
// Animated caption

HelloWorldCaption::HelloWorldCaption() {}
HelloWorldCaption::~HelloWorldCaption() {}

void HelloWorldCaption::update_logic(float millisecs_elapsed)
{
	OMediaCaption::update_logic(millisecs_elapsed);

	add_angle(0,(omt_Angle)(millisecs_elapsed* (float(omd_Deg2Angle(90))/1000.0f)), 0);
}

//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	HelloWorld	*app;

	app = new HelloWorld;		// Create and start application
	app->start();
	delete app;  
}

