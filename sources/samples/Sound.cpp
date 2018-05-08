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

// Sound example


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
#include "OMediaStdButton.h"
#include "OMediaString.h"
#include "OMediaSound.h"
#include "OMediaSoundChannel.h"
#include "OMediaEngineFactory.h"
#include "OMediaSlider.h"

//---------------------------------------------
// Definitions

const omt_Message msg_PlaySound1 = 1;
const omt_Message msg_PlaySound2 = 2;
const omt_Message msg_PlaySound3 = 3;
const omt_Message msg_StopSound1 = 4;
const omt_Message msg_StopSound2 = 5;
const omt_Message msg_StopSound3 = 6;
const omt_Message msg_ChangeVolume1 = 7;
const omt_Message msg_ChangeVolume2 = 8;
const omt_Message msg_ChangeVolume3 = 9;



	// Override standard OMT application class

class Sound :	public OMediaApplication,
				public OMediaPeriodical,
				public OMediaListener
{
public:

	Sound();
	virtual ~Sound();

	void init_display(void);
	void init_font(void);
	void init_world(void);
	void init_sound(void);

	void init_database(void);
	void close_database(void);

	void listen_to_message(omt_Message msg, void *param);

	OMediaCanvas *load_image(string filename);


	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaCanvas			*small_font_canvas;
	OMediaCanvasFont		*small_font;


	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;	

	OMediaSound				*sounds[3];
	OMediaSoundEngine		*sound_engine;

};


//---------------------------------------------
// Implementations

Sound::Sound()
{
	world = NULL;

	init_database();
	init_display();
	init_font();
	init_world();
	init_sound();
}

Sound::~Sound()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete small_font;
	delete small_font_canvas;

	delete monitors;

	close_database();

	delete sound_engine;
}

void Sound::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(300,160);
	window->place(40,60);
	window->show();

	// * Create a monitor maps
	
	// A monitor map automatically creates for you one video engine
	// per card installed in your machine. It allows OMT to take
	// care of multiple monitors automatically for you.

	omt_EngineID video_engine = *(OMediaEngineFactory::get_factory()->video_engines.begin());
//	video_engine = ommeic_OS;	//+++

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

OMediaCanvas *Sound::load_image(string filename)
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

void Sound::init_font(void)
{
	// OMT automatically recognizes the PNG format.

	small_font_canvas 	= load_image("medias/fontsmall.png");

	// Now I need to build the font:

	small_font = new OMediaCanvasFont;
	small_font->set_font_canvas(small_font_canvas, small_font_canvas->get_width(), 10, 10);
	small_font->set_proportional(true);
	small_font->create_proportional_tab();
	small_font->set_space_size(3);
	small_font->set_char_space(1);
}

void Sound::init_world(void)
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

void Sound::init_sound(void)
{
	OMediaStdButton	*stdbtn;
	OMediaCaption	*caption;
	OMediaSlider	*slider;
	float			y;
	string			str;
	long			i;

	// Init user interface

	y = 10;

	for(i=0; i<3; i++, y-=20)
	{
		str = "Channel ";
		str += omd_L2STR(i+1);
		str += ":";

		caption = new OMediaCaption;
		caption->link(world);
		caption->place(-130,y+2);
		caption->set_string(str);
		caption->set_font(small_font);
		caption->set_flags(0);

		stdbtn = new OMediaStdButton;
		stdbtn->link(world);
		stdbtn->place(-70,y-2);
		stdbtn->set_string("Play");	
		stdbtn->set_style(ombtnc_Standard);
		stdbtn->set_font(small_font);
		stdbtn->addlistener(this);
		stdbtn->set_message(msg_PlaySound1+i);

		stdbtn = new OMediaStdButton;
		stdbtn->link(world);
		stdbtn->place(-10,y-2);
		stdbtn->set_string("Stop");	
		stdbtn->set_style(ombtnc_Standard);
		stdbtn->set_font(small_font);	
		stdbtn->addlistener(this);
		stdbtn->set_message(msg_StopSound1+i);

		slider = new OMediaSlider;
		slider->link(world);
		slider->place(50,y);
		slider->set_slider_mode(omsmc_Horizontal);
		slider->set_size(70,16);
		slider->set_maxvalue(255);
		slider->set_value(255);
		slider->set_message(msg_ChangeVolume1+i);
		slider->addlistener(this);
	}

	// Init sound engine
	
	OMediaEngineFactory	*factory = OMediaEngineFactory::get_factory();
	omt_EngineList	*sound_engines = &factory->sound_engines;
	omt_EngineID	engine_id = ommeic_Null;		
        omt_EngineList::iterator sei;

	for(sei=sound_engines->begin();
		sei!=sound_engines->end();
		sei++)
	{
		// Try to find an engine that supports multiple channels

		if (factory->get_sound_engine_attr((*sei))&omseaf_NoChannelLimitation)
		{
			engine_id = (*sei);
		}
	}

	if (engine_id==ommeic_Null)
	{
		// No multiple channels engine, just take the first one
		engine_id =  (*sei);
	}

	// Create sound engine
	sound_engine = factory->create_sound_engine(engine_id,window);

	// Create 3 channels
	sound_engine->set_nchannels(3);	

	// Load sounds
	for(i=0;i<3;i++)
	{
		sounds[i] = omd_GETOBJECT(database, OMediaSound, i);
	}
}


void Sound::init_database(void)
{
	OMediaFilePath			path("medias/samples.omt");

	// Set the cache to zero. In this mode, OMT database never removes objects
	// from memory (until you delete the database).

	OMediaDataBase::set_cache_size(0);

	// We need to register the OMT classes we'll get from the database: 

	omd_REGISTERCLASS(OMediaCanvas);
	omd_REGISTERCLASS(OMediaSound);

	dbfile = new OMediaFileStream;
	dbfile->setpath(&path);
	dbfile->open(omcfp_Read);			// Read only

	database = new OMediaDataBase(dbfile);	// Open database
}

void Sound::close_database(void)
{
	delete database;
	delete dbfile;
}

void Sound::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case msg_PlaySound1:
		if (sound_engine->get_nchannels()>0) sound_engine->play(0, sounds[0],false,true); 
		break;

		case msg_PlaySound2:
		if (sound_engine->get_nchannels()>1) sound_engine->play(1, sounds[1],false,true); 
		break;

		case msg_PlaySound3:
		if (sound_engine->get_nchannels()>2) sound_engine->play(2, sounds[2],false,true); 
		break;

		case msg_StopSound1:
		if (sound_engine->get_nchannels()>0) sound_engine->get_channel(0)->stop();
		break;

		case msg_StopSound2:
		if (sound_engine->get_nchannels()>1) sound_engine->get_channel(1)->stop();
		break;

		case msg_StopSound3:
		if (sound_engine->get_nchannels()>2) sound_engine->get_channel(2)->stop();
		break;

		case msg_ChangeVolume1:
		if (sound_engine->get_nchannels()>0) 
			sound_engine->get_channel(0)->set_volume((int) ((OMediaSlider*)param)->get_value());
		break;

		case msg_ChangeVolume2:
		if (sound_engine->get_nchannels()>1)
			sound_engine->get_channel(1)->set_volume((int) ((OMediaSlider*)param)->get_value());

		break;

		case msg_ChangeVolume3:
		if (sound_engine->get_nchannels()>2)
			sound_engine->get_channel(2)->set_volume((int) ((OMediaSlider*)param)->get_value());
		break;


		default:
		OMediaApplication::listen_to_message(msg, param);
		break;
	}
}

//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	Sound	*app;

	app = new Sound;		// Create and start application
	app->start();
	delete app;  
}

