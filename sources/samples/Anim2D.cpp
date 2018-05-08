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

// Playing with 2D


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

enum AnimStep
{
	step_Initializing,
	step_PMWPresent,
	step_PMWPresentFadeout,
	step_OMTLogoFadein
};


//---------------------------------------------
// Definitions

	// Override standard OMT application class

class Anim2D :	public OMediaApplication,
				public OMediaPeriodical
{
public:

	Anim2D();
	virtual ~Anim2D();

	void init_display(void);
	void init_font(void);
	void init_world(void);

	virtual void spend_time(void);	// Give me time

	void init_database(void);
	void close_database(void);

	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaCanvas			*font_canvas;
	OMediaCanvasFont		*font;


	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;

	AnimStep				step;

	OMediaTimer				timer;

	OMediaCaption			*pmw_caption;

	OMediaCanvasElement		*logo_element;
	OMediaCanvas			*logo;

	float					scale_pmw,fade;

};


//---------------------------------------------
// Implementations

Anim2D::Anim2D()
{
	step = step_Initializing;
	world = NULL;

	init_database();
	init_display();
	init_font();
	init_world();

	logo = omd_GETOBJECT(database,OMediaCanvas,"OMT2");
}

Anim2D::~Anim2D()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete font;
	delete font_canvas;

	delete monitors;

	close_database();
}

void Anim2D::init_display(void)
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
		if (def) (*vi)->select_renderer(def,omfzbc_NoZBuffer);
	}
}

void Anim2D::init_font(void)
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

void Anim2D::init_world(void)
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

	argb.set(1.0f,1.0f,1.0f,1.0f);

	layer = new OMediaLayer;
	layer->link(world);
	layer->add_flags(omlayerf_ClearColor);
	layer->set_clear_color(argb);

	// I set a default ortho matrix for this layer. No perspective, pure 2D!
	layer->set_projection_type(omlptc_Ortho);

	// Because there is no perspective I can set the near clip plane to zero.
	// Objects with a z<near_clip are not drawn. So if you're doing 2D stuff and
	// don't want to worry about the z component, just set it to zero.
	// Never do that if you have a perspective matrix!
	
	layer->set_near_clip(0.0f);		
}

void Anim2D::init_database(void)
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

void Anim2D::close_database(void)
{
	delete database;
	delete dbfile;
}


//---------------------------------------------
// Called periodically by OMT (see OMediaPeriodical)

void Anim2D::spend_time(void)
{
	if (!world) return;

	OMediaFARGBColor	argb(0.0f,1.0,1.0,1.0);

	float		elapsed;
	elapsed = float(timer.getelapsed());	// millisecs ellapsed since
											// last frame


	timer.start();

	if (step==step_Initializing && world)
	{
		// Ok, let's create the GarageCube Presents caption

		pmw_caption = new OMediaCaption;
		pmw_caption->link(world);
		pmw_caption->set_font(font);
		pmw_caption->set_string("GarageCube presents");
		pmw_caption->place(0,0);
		pmw_caption->set_auto_align(omaac_Center, omaac_Center);
		pmw_caption->set_canvas_flags(omcanef_FreeWorldSize);

		scale_pmw = 3.0f;
		step = step_PMWPresent;

	}
	else if (step==step_PMWPresent)
	{
		scale_pmw -= (1.0f/1000.0f) * elapsed;
		if (scale_pmw<=0.3) 
		{
			step=step_PMWPresentFadeout;
			fade = 1.0;
		}
		else
		{	
			long	w,h,vm,l;

			font->get_font_info(w, h, vm);
			l = font->get_text_length("GarageCube presents");

			pmw_caption->set_size(l*scale_pmw,h*scale_pmw);
		}
	}
	else if (step==step_PMWPresentFadeout)
	{
		fade -= (0.8f/1000.0f) * elapsed;
		if (fade<0) 
		{
			fade = 0;
			step=step_OMTLogoFadein;

			delete pmw_caption;
			pmw_caption = NULL;

			logo_element = new OMediaCanvasElement;
			logo_element->link(world);
			logo_element->set_canvas(logo);
			logo_element->set_diffuse(argb);			
			logo_element->set_auto_align(omaac_Center, omaac_Center);
			logo_element->set_size(logo->get_width(),logo->get_height());

			// Turn on alpha-blending. I don't have to do it for the caption because
			// the OMediaCaptionElement class turns it on by default. It is not the case
			// for simple canvas element.
			logo_element->set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
	
		}
		else
		{
			argb.alpha = fade;
			pmw_caption->set_diffuse(argb);
		}
	}
	else if (step==step_OMTLogoFadein)
	{
		fade += (0.3f/1000.0f) * elapsed;
		if (fade>1.0f) {fade = 1.0f;}
		
		argb.alpha = fade;
		logo_element->set_diffuse(argb);
		logo_element->add_angle(0,0, (int)((float(omd_Deg2Angle(45))/1000.0f)*elapsed)  );		// 45 degrees per second
	}



}


//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	Anim2D	*app;

	app = new Anim2D;		// Create and start application
	app->start();
	delete app;  
}

