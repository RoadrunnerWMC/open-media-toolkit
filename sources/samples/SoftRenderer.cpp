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

// Software renderer demo

#define WINDOWED

#ifdef PROFILE
#include "profiler.h"	//P+++
#endif

#include "OMediaApplication.h"
#include "OMediaSingleWindow.h"
#include "OMediaMonitorMap.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasFont.h"
#include "OMediaCaption.h"
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
#include "OMediaString.h"
#include "OMediaStdButton.h"
#include "OMediaError.h"

#ifndef WINDOWED
#include "OMediaFullscreenWindow.h"
#else
#include "OMediaSingleWindow.h"
#endif


//---------------------------------------------
// Definitions

const omt_Message	msg_Light	= 100;
const omt_Message	msg_Gouraud	= 101;
const omt_Message	msg_ZBuffer	= 102;



	// Override standard OMT application class

class SoftRenderer :	public OMediaApplication,
						public OMediaPeriodical,
						public OMediaListener
{
public:

	SoftRenderer();
	virtual ~SoftRenderer();

	void init_display(void);
	void init_font(void);
	void init_world(void);
	void init_demo(void);

	void init_database(void);
	void close_database(void);

	void listen_to_message(omt_Message msg, void *param);


	void spend_time(void);

	OMediaMonitorMap		*monitors;

#ifdef WINDOWED
	OMediaSingleWindow		*window;
#else
	OMediaFullscreenWindow	*window;
#endif

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer,*layer2;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;

	OMediaCanvas			*font_canvas;
	OMediaCanvasFont		*font;

	OMediaCaption			*frate_caption;

	OMediaTimer				timer;

	OMedia3DMaterial		*planet_material;

	long					npolygons;

};

	// Override OMT shape element

class PlanetElement : public OMedia3DShapeElement
{
public:
	
	PlanetElement();
	virtual ~PlanetElement();

	// Following method is called for each element before rendering

	virtual void update_logic(float millisecs_elapsed);

};

//---------------------------------------------
// Implementations

SoftRenderer::SoftRenderer()
{
	init_database();
	init_display();
	init_font();
	init_world();
	init_demo();

#ifdef PROFILE

	ProfilerInit(collectDetailed, bestTimeBase, 1000, 128);
	ProfilerSetStatus(true);	 	
#endif

}

SoftRenderer::~SoftRenderer()
{

#ifdef PROFILE
	ProfilerDump("\psoftrend.prof");
	ProfilerTerm();
#endif

	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)
#ifdef WINDOWED
	delete monitors;
#endif

	delete font;
	delete font_canvas;
	
	close_database();
}

void SoftRenderer::init_display(void)
{
	// * Create window

#ifdef WINDOWED
	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(640,480);
	window->place(40,60);
#else
	window = new OMediaFullscreenWindow(this);
#endif	


	window->show();

	omt_EngineID video_engine = *(OMediaEngineFactory::get_factory()->video_engines.begin());

#ifdef WINDOWED

	// WINDOWED MULTIPLE MONITORS

	monitors = new OMediaMonitorMap(video_engine,window);
	window->link_monitor_map(monitors);

	// Find the software renderer (windowed)

	for(omt_VideoEngineList::iterator vi=monitors->engines.begin();
		vi!=monitors->engines.end();
		vi++)
	{
		OMediaRendererDef	*def = omc_NULL;

		for(omt_RendererDefList::iterator ri = (*vi)->get_renderer_list()->begin();
			ri!=(*vi)->get_renderer_list()->end();
			ri++)
		{
			// Is it the OMT renderer?
			if ((*ri).engine_id== ommeic_OMT) def = &(*ri);
		}

		// No software renderer. Take the first one.
		if (!def && (*vi)->get_renderer_list()->size()) def =  &(*(*vi)->get_renderer_list()->begin());


		// Once I found a renderer I need to select it. This will automatically build the renderer for me.
		if (def) (*vi)->select_renderer(def,def->zbuffer_depth);
	}

#else

	// FULL SCREEN

	omt_VideoCardList::iterator vci;
	omt_VideoModeList::iterator vmi;

	OMediaVideoEngine *veng = OMediaEngineFactory::get_factory()->create_video_engine(video_engine,window);	
	window->link_video_engine(veng);	

	for(vci = veng->get_video_cards()->begin();
		vci != veng->get_video_cards()->end();
		vci++)
	{
		if ( (*vci).flags&omcvdf_MainMonitor) break;	
	}
	
	if (vci==veng->get_video_cards()->end())
	{
		// Can't find main monitor...? Just leave...
		omd_STREXCEPTION("Can't find main monitor");
	}
	
	// * Let's link the engine to the card
	
	veng->link(&(*vci));

	// * Okay, now let's try to change the resolution to 800x600x32 or lower

	for(vmi= (*vci).modes.begin();
		vmi!=(*vci).modes.end();
		vmi++)
	{
		if ((*vmi).width==640 &&
			(*vmi).height==480 &&
			(*vmi).depth==16 &&
			
			// I want to use the same refresh rate than the current mode.
			
			(*vmi).refresh_rate == veng->get_current_video_mode()->refresh_rate)
		{
			// Video mode found, set it.
			
			veng->set_mode(&(*vmi));
			break;
		}		
	}	

	// Find the software renderer (full screen)

	OMediaRendererDef	*def = omc_NULL;

	for(omt_RendererDefList::iterator ri = veng->get_renderer_list()->begin();
		ri!=veng->get_renderer_list()->end();
		ri++)
	{
		// Is it the OMT renderer?
		if ((*ri).engine_id== ommeic_OMT) def = &(*ri);
	}

	// No software renderer. Take the first one.
	if (!def && veng->get_renderer_list()->size()) def =  &(*veng->get_renderer_list()->begin());


	// Once I found a renderer I need to select it. This will automatically build the renderer for me.
	if (def) veng->select_renderer(def,def->zbuffer_depth);

#endif

}

void SoftRenderer::init_world(void)
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


	// * Layer

	argb.set(1.0f,0.8f,0.8f,0.8f);

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

void SoftRenderer::init_database(void)
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

void SoftRenderer::close_database(void)
{
	delete database;
	delete dbfile;
}

void SoftRenderer::init_demo(void)
{

	long	polypershape;

	npolygons = 0;

	// Get planet shape

	OMedia3DShape		*planet_shape = omd_GETOBJECT(database,OMedia3DShape,"torus");

	// Extract material

	planet_shape->lock(omlf_Read);
	planet_material = (*planet_shape->get_polygons()->begin()).get_material();
	planet_material->set_light_mode(ommlmc_Color);
	planet_material->set_shade_mode(omshademc_Flat);
	planet_material->set_blend(omblendfc_One,omblendfc_Zero);
	polypershape = planet_shape->get_polygons()->size();
	planet_shape->unlock();

	// I want the canvas rendering pixel format to be 16bits 1555.
	// It speeds up software renderer.

	if (planet_material->get_texture()) planet_material->get_texture()->set_internal_pixel_format(ompixfc_ARGB1555);

//	for(short x=0;x<2;x++)
//	for(short y=0;y<2;y++)
	{
		PlanetElement *planet = new PlanetElement;
		planet->link(world);
//		planet->place(-20+(x*60),-30+(y*60),110);
		planet->place(0,0,80);//200);
		planet->set_shape(planet_shape);
		npolygons +=polypershape;
	}


	// User-interface

	layer2 = new OMediaLayer;
	layer2->link(world);
	layer2->set_flags(omlayerf_Visible);
	layer2->set_projection_type(omlptc_Ortho);	
	layer2->set_near_clip(0.0f);		

	frate_caption = new OMediaCaption;
	frate_caption->link(world);
	frate_caption->link_layer(layer2);
	frate_caption->set_font(font);
	frate_caption->place(-300,200,0);

	OMediaStdButton	*stdbtn;
	stdbtn = new OMediaStdButton;
	stdbtn->link(world);
	stdbtn->link_layer(layer2);
	stdbtn->place(-300,100);
	stdbtn->set_string("Light");	
	stdbtn->set_style(ombtnc_Checkbox);
	stdbtn->set_font(font);
	stdbtn->set_message(msg_Light);
	stdbtn->addlistener(this);

	stdbtn = new OMediaStdButton;
	stdbtn->link(world);
	stdbtn->link_layer(layer2);
	stdbtn->place(-300,80);
	stdbtn->set_string("Gouraud");	
	stdbtn->set_style(ombtnc_Checkbox);
	stdbtn->set_font(font);
	stdbtn->set_message(msg_Gouraud);
	stdbtn->addlistener(this);

	stdbtn = new OMediaStdButton;
	stdbtn->link(world);
	stdbtn->link_layer(layer2);
	stdbtn->place(-300,60);
	stdbtn->set_string("ZBuffer");	
	stdbtn->set_style(ombtnc_Checkbox);
	stdbtn->set_font(font);
	stdbtn->set_message(msg_ZBuffer);
	stdbtn->addlistener(this);

}

void SoftRenderer::listen_to_message(omt_Message msg, void *param)
{
	OMediaStdButton	*stdbtn = (OMediaStdButton	*)param;

	switch(msg)
	{
		case msg_Light:
		if (stdbtn->isdown())
			planet_material->set_light_mode(ommlmc_Light);
		else
			planet_material->set_light_mode(ommlmc_Color);
		break;

		case msg_Gouraud:
		if (stdbtn->isdown())
			planet_material->set_shade_mode(omshademc_Gouraud);
		else
			planet_material->set_shade_mode(omshademc_Flat);
		break;

		case msg_ZBuffer:
		if (stdbtn->isdown())
		{
			layer->add_flags(omlayerf_ClearZBuffer|omlayerf_EnableZBufferWrite|omlayerf_EnableZBufferTest);

		}
		else
		{
			layer->remove_flags(omlayerf_ClearZBuffer|omlayerf_EnableZBufferWrite|omlayerf_EnableZBufferTest);
		}
		break;

		case omsg_Event:
		{
			// In fullscreen, there is no window close button. Just
			// test escape key.

			OMediaEvent	*event = (OMediaEvent*)param;
			if (event->type==omtet_KeyDown)
			{
				if (event->special_key==omtsk_Escape) quit();	
			}		
		}
		break;	


		default:
		OMediaApplication::listen_to_message(msg, param);
		break;
	}
}

void SoftRenderer::init_font(void)
{
	// Normally the font canvas should be stored in an OMT database. However since
	// the MediaMeister V2.0 has not been released there is no way to edit an image with an
	// alpha-channel at this time. We used the alpha-channel to generate anti-aliased fonts.
	// Now for the DR1, I stored the font in a PNG image.

	// OMT automatically recognizes the PNG format.

	// Open the file:

	OMediaFilePath		path("medias/fontsmall.png");	// path
	OMediaFileStream	file;					// file

	file.setpath(&path);
	file.open(omcfp_Read,false,false);

	font_canvas = new OMediaCanvas;
	file>>font_canvas;				// read the image


	// Now I need to build the font:

	font = new OMediaCanvasFont;
	font->set_font_canvas(font_canvas, font_canvas->get_width(), 10, 10);
	font->set_proportional(true);
	font->create_proportional_tab();
	font->set_space_size(3);
	font->set_char_space(1);
}

void SoftRenderer::spend_time(void)
{
	long			elapsed = timer.getelapsed();
	string			str;

	if (elapsed)
	{
		str = "Software Renderer: fps = ";
		str += omd_L2STR(1000 / elapsed);
		str += " / Polygons = ";
		str += omd_L2STR(npolygons);

		frate_caption->set_string(str);
	}

	timer.start();
}

//---------------------------------------------
// Animated element

PlanetElement::PlanetElement() {}
PlanetElement::~PlanetElement() {}

void PlanetElement::update_logic(float millisecs_elapsed)
{
	OMedia3DShapeElement::update_logic(millisecs_elapsed);

	add_angle(0,(omt_Angle)(millisecs_elapsed* (float(omd_Deg2Angle(90))/1000.0f)), 0);
}

//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	SoftRenderer	*app;

	app = new SoftRenderer;		// Create and start application
	app->start();
	delete app;  
}

