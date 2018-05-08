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

// UI


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
#include "OMediaPeriodical.h"
#include "OMediaTimer.h"
#include "OMediaDatabase.h"

#include "OMediaCanvasButton.h"
#include "OMediaStdButton.h"
#include "OMediaRadioGroup.h"
#include "OMediaScroller.h"
#include "OMediaSlider.h"
#include "OMediaStringField.h"


//---------------------------------------------
// Definitions

	// Override standard OMT application class

class UI :	public OMediaApplication,
				public OMediaPeriodical
{
public:

	UI();
	virtual ~UI();

	void init_display(void);
	void init_font(void);
	void init_world(void);
	void init_UI(void);


	OMediaCanvas *load_image(string filename);



	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaCanvas			*font_canvas;
	OMediaCanvasFont		*font;

	OMediaCanvas			*small_font_canvas;
	OMediaCanvasFont		*small_font;


	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;

	OMediaFileStream		*dbfile;	
};


//---------------------------------------------
// Implementations

UI::UI()
{
	world = NULL;

	init_display();
	init_font();
	init_world();
	init_UI();
}

UI::~UI()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete font;
	delete font_canvas;
	delete small_font;
	delete small_font_canvas;

	delete monitors;

}

void UI::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(400,440);
	window->place(40,60);
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

OMediaCanvas *UI::load_image(string filename)
{
	// Normally the canvases should be stored in an OMT database. However since
	// the MediaMeister V2.0 has not been released there is no way to edit an image with an
	// alpha-channel at this time. We used the alpha-channel to generate transparency and
	// anti-aliasing.

	// Open the file:

	OMediaFilePath		path(filename);		// path
	OMediaFileStream	file;				// file
	OMediaCanvas		*canvas;

	file.setpath(&path);
	file.open(omcfp_Read,false,false);

	canvas = new OMediaCanvas;
	file>>canvas;				// read the image

	return canvas;
}

void UI::init_font(void)
{
	// OMT automatically recognizes the PNG format.

	font_canvas 		= load_image("medias/font.png");
	small_font_canvas 	= load_image("medias/fontsmall.png");

	// Now I need to build the font:

	font = new OMediaCanvasFont;
	font->set_font_canvas(font_canvas, font_canvas->get_width(), 76, 77);
	font->set_proportional(true);
	font->create_proportional_tab();
	font->set_space_size(10);
	font->set_char_space(3);

	small_font = new OMediaCanvasFont;
	small_font->set_font_canvas(small_font_canvas, small_font_canvas->get_width(), 10, 10);
	small_font->set_proportional(true);
	small_font->create_proportional_tab();
	small_font->set_space_size(3);
	small_font->set_char_space(1);
}

void UI::init_world(void)
{
	OMediaRect			r;
	OMediaFARGBColor	argb;

	// * World

	world = new OMediaWorld;	// Create the root class


	// * Viewport


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


	// * Layer

	argb.set(1.0f,0.9f,0.9f,0.9f);

	layer = new OMediaLayer;
	layer->link(world);
	layer->add_flags(omlayerf_ClearColor);
	layer->set_clear_color(argb);

	// I set a default ortho matrix for this layer. No perspective, pure 2D!
	layer->set_projection_type(omlptc_Ortho);

	// Because there is no perspective I can set the near clip plane to zero.
	// Objects with a z<near_clip are not drawn. So if you're doing 2D stuff and
	// don't want to worry about the z component, just set it to zero.
	// Never do that if you set a perspective matrix!
	
	layer->set_near_clip(0.0f);		
}

void UI::init_UI(void)
{
	OMediaStdButton		*stdbtn;
	OMediaScroller		*scroller;
	OMediaSlider		*slider;
	OMediaStringField	*field;

	stdbtn = new OMediaStdButton;
	stdbtn->link(world);
	stdbtn->place(40,40);
	stdbtn->set_string("Click!");	
	stdbtn->set_style(ombtnc_Standard);
	stdbtn->set_font(small_font);
	
	scroller = new OMediaScroller;
	scroller->link(world);
	scroller->place(-100,0);
	scroller->set_vtotal(100);
	scroller->set_vrange(0,40);
	scroller->set_size(16,100);
	
	slider = new OMediaSlider;
	slider->link(world);
	slider->place(-100,-100);
	slider->set_size(100,16);
	slider->set_slider_mode(omsmc_Horizontal);
	
	field = new OMediaStringField;
	field->link(world);
	field->place(0,140);
	field->set_auto_resize(true,150);
	field->set_size(32,16);
	field->set_string("String field 1");
	field->set_font(small_font);	

	field = new OMediaStringField;
	field->link(world);
	field->place(0,120);
	field->set_size(100,16);
	field->set_string("String field 2");
	field->set_deselected_transp(false);
	field->set_font(small_font);	
	field->set_auto_resize(false,200);
}


//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	UI	*app;

	app = new UI;		// Create and start application
	app->start();
	delete app;  
}

