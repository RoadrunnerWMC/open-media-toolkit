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

// Full screen example

//#define WINDOWED

#include "OMediaEngineID.h"
#include "OMediaApplication.h"
#include "OMediaMonitorMap.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasFont.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaRendererDef.h"
#include "OMedia3DShapeElement.h"
#include "OMediaCanvasElement.h"
#include "OMediaWorld.h"
#include "OMediaViewPort.h"
#include "OMediaLayer.h"
#include "OMediaDatabase.h"
#include "OMedia3DMaterial.h"
#include "OMediaLight.h"
#include "OMedia3DShape.h"
#include "OMediaError.h"
#include "OMediaRandomNumber.h"
#include "OMediaThread.h"
#include "OMediaMouseCursor.h"
#include "OMediaCaption.h"


#ifndef WINDOWED
#include "OMediaFullscreenWindow.h"
#else
#include "OMediaSingleWindow.h"
#endif

//---------------------------------------------
// Definitions

class AutoRotateElement;

	// Override standard OMT application class

class Fullscreen : 	public OMediaApplication,
					public OMediaPeriodical
{
public:

	Fullscreen();
	virtual ~Fullscreen();

	void init_display(void);
	void init_font(void);
	void init_world(void);
	void init_demo(void);
	void init_back_layer(void);

	void init_database(void);
	void close_database(void);

	OMediaCanvas *load_image(string filename);

	virtual void listen_to_message(omt_Message msg, void *param);

	virtual void spend_time(void);

	void select_planet(OMediaPickRequest *pick);
	void deselect_planet(void);


#ifdef WINDOWED
	OMediaSingleWindow		*window;
#else
	OMediaFullscreenWindow	*window;
#endif

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer, *sky_layer, *sky_layer2;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;

	
	OMediaCanvas			*sun_canvas;
	OMediaCanvasElement		*sun;
	
	OMedia3DShape			*near_sky_shape;
	OMedia3DMaterial		*near_sky_mat;
	OMediaCanvas			*near_sky_canvas;
	
	#define					nspace_backs 3
	OMediaCanvas			*space_backs[nspace_backs];

	OMediaCanvas			*redlight_canv;
	OMediaCanvasElement		*redlight_el;
	OMediaLayer				*redlight_layer;
	OMediaLight				*redlight_light;
	
	OMediaCanvasFont		*font;
	OMediaCanvas			*font_canvas;
	
	OMediaCanvasElement		*cursor_element;
	OMediaCanvas			*cursor_canvas;
	OMediaCaption			*caption;
	OMediaLayer				*info_layer;

	OMediaElement			*selected_element;
	OMedia3DShape			*selected_shape;
		
	OMediaTimer				timer;

	AutoRotateElement		*planet, *alien, *planet2,*planet3,*planet4;

	float					alien_time,redlight_time;

	static bool					paused;	
};

bool	Fullscreen::paused;	


class AutoRotateElement : public OMedia3DShapeElement
{
public:
	
	AutoRotateElement() {ang_speed = 90;}
	virtual ~AutoRotateElement() {}

	float ang_speed;

	void update_logic(float millisecs_elapsed)
	{
		if (Fullscreen::paused) return;
	
		OMedia3DShapeElement::update_logic(millisecs_elapsed);
	
                float	angy;
                
                angy = millisecs_elapsed;
                angy *= omd_Deg2Angle(this->ang_speed)/1000.0f;

		add_angle(	0,
                                (omt_Angle)angy, 
                                0);
	}

};


//---------------------------------------------
// Implementations

Fullscreen::Fullscreen()
{
	init_database();
	init_display();
	init_world();
	init_font();
	init_demo();

	selected_element = NULL;
	selected_shape = NULL;
	paused = false;

#ifndef WINDOWED
	OMediaMouseCursor::hide();
#endif
}

Fullscreen::~Fullscreen()
{
#ifndef WINDOWED
	OMediaMouseCursor::show();
#endif

	delete world;
	delete sun_canvas;
	delete near_sky_shape;
	delete near_sky_mat;
	delete near_sky_canvas;
	delete redlight_canv;
	for(short i=0;i<nspace_backs;i++) delete space_backs[i];

	delete font;
	delete font_canvas;
	delete cursor_canvas;

	close_database();
}

void Fullscreen::init_display(void)
{
	omt_VideoCardList::iterator vci;
	omt_VideoModeList::iterator vmi;

	// * Create fullscreen window

#ifdef WINDOWED
	window = new OMediaSingleWindow(this);
	window->set_size(640,480);
#else
	window = new OMediaFullscreenWindow(this);
#endif	

	window->show();

	// * Get the first available video engine, the first of the list is always considered
	// as the best one.

	omt_EngineID video_engine = *(OMediaEngineFactory::get_factory()->video_engines.begin());

	// * In full screen mode, we don't create a monitor map, because a full screen window can
	// be linked only to one video card. So first of all, let's create a video engine.

	OMediaVideoEngine *veng = OMediaEngineFactory::get_factory()->create_video_engine(video_engine,window);

	// When I created the video engine, I passed a pointer to the window. The passed window is considered
	// as the master window of the engine. Any engine that you create must have a master window.
	// When the master window is deleted, the engines are deleted too. The rule of the master window is
	// too supervise the engines. Several API (such as DirectX) are very window centric, that's why
	// engines require a supervisor window. Most of the time it is simply the main window of your
	// application. Don't forget that if you delete this window, you'll have to rebuild engines.
	
	// Now you still have to tell the window what video engine should be used for its drawings.
	
	window->link_video_engine(veng);	
	
	
	// * Now I need to find a video card. For this example, I'm just looking for the main
	// one.	
	
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
		if ((*vmi).width==800 &&
			(*vmi).height==600 &&
			(*vmi).depth==16  &&
			
			// I want to use the same refresh rate than the current mode.
			
			(*vmi).refresh_rate == veng->get_current_video_mode()->refresh_rate )
		{
			// Video mode found, set it.
			
#ifndef WINDOWED
			veng->set_mode(&(*vmi));
#endif
			break;
		}		
	}	

	// * Select renderers

	OMediaRendererDef	*def = omc_NULL;

	for(omt_RendererDefList::iterator ri = veng->get_renderer_list()->begin();
		ri!=veng->get_renderer_list()->end();
		ri++)
	{
		if ((*ri).attributes & omcrdattr_Accelerated ) def = &(*ri);	// Accelerated ?
	}

	// No accelerated device. Take the first one.
	if (!def && veng->get_renderer_list()->size()) def =  &(*veng->get_renderer_list()->begin());

	// Once I found a renderer I need to select it. This will automatically build the renderer for me.
	if (def) veng->select_renderer(def,def->zbuffer_depth);

}

void Fullscreen::init_world(void)
{
	OMediaRect			r;
	OMediaFARGBColor	argb;

	// * World

	world = new OMediaWorld;	// Create the root class


	// * Layers

	// The sky layer
	
	// I don't need to clear the zbuffer and the back color because the sky entirely
	// filled the background viewport. It is a closed environment.
	// Also I use a custom view transform matrix, so the sky always stay at (0,0).
	// It rotates with the viewport but it does not move.
	
	sky_layer = new OMediaLayer;
	sky_layer->link(world);
	sky_layer->set_flags(omlayerf_Visible|omlayerf_CustomViewMatrix|omlayerf_ClearZBuffer|omlayerf_DisablePicking);

	sky_layer2 = new OMediaLayer;
	sky_layer2->link(world);
	sky_layer2->set_flags(omlayerf_Visible|omlayerf_CustomViewMatrix|omlayerf_DisablePicking);

	// Planet layer

	argb.set(1.0f,0.0f,0.2f,0.4f);
	layer = new OMediaLayer;
	layer->link(world);
	layer->add_flags(omlayerf_EnableZBufferWrite|omlayerf_EnableZBufferTest);
	layer->set_light_global_ambient(argb);

	// Light layer

	redlight_layer = new OMediaLayer;
	redlight_layer->link(world);
	redlight_layer->set_flags(omlayerf_Visible|omlayerf_EnableZBufferTest|omlayerf_DisablePicking);
	
	// Info layer

	info_layer = new OMediaLayer;
	info_layer->link(world);
	info_layer->set_flags(omlayerf_Visible|omlayerf_DisableViewportTransform|omlayerf_DisablePicking);
	info_layer->set_projection_type(omlptc_Ortho);
	info_layer->set_near_clip(0);

	// * Viewport


	viewport = new OMediaViewPort(window);		// Supervisor is window
	viewport->link(world);
	viewport->link_layer(layer);
	viewport->link_window(window);				// Output is window
	viewport->place(0,200,-700);
	viewport->set_flags(omcvpf_EnableMouseClick|omcvpf_ClickActivateSupervisor);

	r.set(0,0,0,0);		// Viewport bounds are right/bottom relative, so
						// when the window is resized, the viewport is resized
						// too.

	viewport->setbounds(&r);
	viewport->set_bounds_mode(omcpbc_Right,omvpbmc_RightRelative);
	viewport->set_bounds_mode(omcpbc_Bottom,omvpbmc_BottomRelative);


	// * Light

	OMediaLight	*light;
	light = new OMediaLight;
	light->link(world);
	light->set_light_type(omclt_Point);
	light->set_range(1000);
}

void Fullscreen::spend_time(void)
{
	OMediaMatrix_4x4	matrix;
	float				elapsed = timer.getelapsed();
	OMedia3DPoint		p,p2;
	OMedia3DVector		v;
	omt_Angle			ax,ay,az;

	if (!paused)
	{
		// Rotating the sun, rotates all the planets.
	
		sun->add_angle(0, omt_Angle((float(omd_Deg2Angle(15))/1000.0f) * elapsed), 0);
	
		// Place the viewport
	
		p.set(viewport->getx(),viewport->gety(),viewport->getz());
		p.rotate(0, omt_Angle((float(omd_Deg2Angle(10))/1000.0f) * elapsed), 0);
		viewport->place(p);
	
		planet->get_absolute_info(p2.x,p2.y,p2.z,ax,ay,az);
		v.set(p,p2);
		v.angles(ax,ay);
		viewport->set_angle(ax,ay,omd_Deg2Angle(15));
	
		// I use a periodical to update my custom view matrix. 
		// Now I simply want a world to camera transform
		// matrix without the translate. In other words, the sky sphere rotates with the
		// viewport but stays centred.
	
		matrix.set_world_transform(0,0,0, viewport->get_anglex(),viewport->get_angley(),viewport->get_anglez());
		sky_layer->set_custom_view_matrix(matrix);
	
		matrix.set_world_transform(0,0,0, (omt_Angle)(viewport->get_anglex()*1.5f),(omt_Angle)(viewport->get_angley()*1.5f),(omt_Angle)(viewport->get_anglez()*1.5f));
		sky_layer2->set_custom_view_matrix(matrix);
	
	
		// Animate the alien now
		
		const float				alien_max_time = 4000.0f;
		float					f;
		OMediaFARGBColor		argb;
		
		alien_time += elapsed;
		if (alien_time>alien_max_time) alien_time = 0;
	
		alien->place(10, ((omd_Sin((int)(omd_Deg2Angle(360) * (alien_time/alien_max_time)))) * 10) , 35);
		
	
		const float				redlight_max_time = 1000.0f;
			
		redlight_time += elapsed;
		if (redlight_time>redlight_max_time) redlight_time = 0;
		
		p.set(0,-4,0);
		alien->local_to_world(p);
		redlight_el->place(p);
	
		f = 1.0f-fabs(omd_Sin((int)(omd_Deg2Angle(90) * (redlight_time/redlight_max_time))));
	
		argb.set(f, 1.0f,1.0f,1.0f);
		redlight_el->set_diffuse(argb);
	
		argb.set(1.0f, f, 0.0f, 0.0f);
		redlight_light->set_diffuse(argb);
	}

	// Move the cursor

	// For this example I just use the input engine of the world object. For a more advanced full screen application
	// it would be better to create an input engine and use it to read mouse deltas.

	short mx,my;

	world->get_input_engine()->get_mouse_position(mx,my);

	// I need to convert desktop coordinates to orthogonal coordinates:
	
	p.x = mx-(window->get_x()+viewport->getbx());	// Most of the time window position is 0,0 for a full screen window.
	p.y = my-(window->get_y()+viewport->getby());	// However it mays not be the case in a multiple monitors context.
	p.z = 0;

	p.x = (p.x - (viewport->getbwidth()*0.5));
	p.y = (-p.y) + (viewport->getbheight()*0.5);

	cursor_element->place(p);
	cursor_element->add_angle(0,0, omt_Angle(elapsed * (omc_MaxAngle/1000)));
	
	
	// Update caption
	
	if (selected_element)
	{
		float	vpx,vpy;
		OMedia3DPoint	p;

		p.set(0,0,0);
		selected_element->local_to_world(p);
	
		if (layer->world_to_viewport_coords(viewport, p, vpx, vpy))
		{
			vpx +=(selected_element==alien)?0:32;
			vpy +=(selected_element==alien)?64:32;
		
			caption->place(vpx,vpy);
			caption->force_show();
		}
		else caption->hide();
	}
	
	// Restart counter
	
	timer.start();
}

OMediaCanvas *Fullscreen::load_image(string filename)
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

void Fullscreen::init_database(void)
{
	OMediaFilePath			path("medias/samples.omt");

	// Set the cache to zero. In this mode, OMT database never removes objects
	// from memory (until you delete the database).

	OMediaDataBase::set_cache_size(0);

	// We need to register the OMT classes we'll get from the database: 

	omd_REGISTERCLASS(OMediaCanvas);
	omd_REGISTERCLASS(OMedia3DShape);
	omd_REGISTERCLASS(OMedia3DMaterial);

	dbfile = new OMediaFileStream;
	dbfile->setpath(&path);
	dbfile->open(omcfp_Read);			// Read only

	database = new OMediaDataBase(dbfile);	// Open database
}

void Fullscreen::close_database(void)
{
	delete database;
	delete dbfile;
}

void Fullscreen::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_Event:
		{
			// In fullscreen, there is no window close button. Just
			// test escape key.

			OMediaEvent	*event = (OMediaEvent*)param;
			if (event->type==omtet_KeyDown)
			{
				if (event->special_key==omtsk_Escape) quit();	
				else if (event->ascii_key=='p') 
				{
					paused = !paused;
					timer.start();
				}
				else if (event->ascii_key=='b')
				{
					static long breakpoint;
					breakpoint++;
				}
			}		
		}
		break;	
		
		case omsg_ViewPortClicked:
		{
			OMediaPickRequest	*pick = (OMediaPickRequest*) param;
			select_planet(pick);
		}			
		break;
	}
	
	OMediaApplication::listen_to_message(msg, param);
}

void Fullscreen::select_planet(OMediaPickRequest *pick)
{
	OMediaFARGBColor	green(1.0f,0,1.0f,0);

	deselect_planet();

	if (pick->closer_hit.type==omptc_Element)
	{
		if (pick->closer_hit.shape)
		{
			caption->force_show();		
				
			pick->closer_hit.shape->lock(omlf_Write);
			
			for(omt_PolygonList::iterator pi = pick->closer_hit.shape->get_polygons()->begin();
				pi!=pick->closer_hit.shape->get_polygons()->end();
				pi++)
			{
				(*pi).get_material()->set_diffuse(green);
				(*pi).get_material()->set_light_mode(ommlmc_Color);
			}			
			
			pick->closer_hit.shape->unlock();
			
			selected_element = pick->closer_hit.element;
			selected_shape = pick->closer_hit.shape;

			if (selected_element==planet) caption->set_string("Planet Afkanan");
			else if (selected_element==planet2) caption->set_string("Planet Xenos");
			else if (selected_element==planet3) caption->set_string("Planet Yotak");
			else if (selected_element==planet4) caption->set_string("Planet Ios");
			else if (selected_element==alien) caption->set_string("Unknown");

			caption->force_show();	
			
		}
		else if (pick->closer_hit.canvas && pick->closer_hit.element == sun)
		{
			caption->force_show();	
			caption->set_string("Star #112a");
		
			selected_element = pick->closer_hit.element;
			selected_shape = NULL;
			
			((OMediaCanvasElement*)selected_element)->set_diffuse(green);
		}			
	}
}

void Fullscreen::deselect_planet(void)
{
	OMediaFARGBColor	full(1.0f,1.0f,1.0f,1.0f);

	if (!selected_element) return;

	caption->hide();


	if (selected_shape)
	{	
		selected_shape->lock(omlf_Write);
		for(omt_PolygonList::iterator pi = selected_shape->get_polygons()->begin();
			pi!=selected_shape->get_polygons()->end();
			pi++)
		{
			(*pi).get_material()->set_diffuse(full);
			(*pi).get_material()->set_light_mode(ommlmc_Light);
		}
		
		selected_shape->unlock();
	}
	else
		((OMediaCanvasElement*)selected_element)->set_diffuse(full);

	selected_element = NULL;
	selected_shape = NULL;
}

void Fullscreen::init_back_layer(void)
{
	OMedia3DShape			*shape;
	OMedia3DShapeElement	*sky;
	OMediaCanvas			*sky_canv;

	// Background sky

	sky_canv = omd_GETOBJECT(database,OMediaCanvas,"sky back");
	sky_canv->set_filtering(omtfc_Linear,omtfc_Nearest);
	shape = omd_GETOBJECT(database,OMedia3DShape,"sky back");
	sky = new OMedia3DShapeElement;
	sky->link(world);
	sky->link_layer(sky_layer);
	sky->set_shape(shape);	

	// Sky level 2

	near_sky_canvas = load_image("medias/space.png");

	near_sky_mat =new  OMedia3DMaterial;
	near_sky_mat->set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
	near_sky_mat->set_texture(near_sky_canvas);

	near_sky_shape = new OMedia3DShape;
	*near_sky_shape = *shape;
	near_sky_shape->set_material(near_sky_mat);

	sky = new OMedia3DShapeElement;
	sky->link(world);
	sky->link_layer(sky_layer2);
	sky->set_shape(near_sky_shape);	


	// Sky elements
	
	long i,s,c;
	string	str;
	
	// Load graphics
	
	for(i=0;i<nspace_backs;i++) 
	{
		str = "medias/spaceback";
		str += omd_L2STR(i+1);
		str += ".png";
		space_backs[i] = load_image(str);
		space_backs[i]->set_filtering(omtfc_Linear,omtfc_Linear);
	}

	// Build elements

	OMediaCanvasElement *el;
	OMediaRandomNumber	rnd;
	OMedia3DPoint		p;
	omt_Angle			ay,az;
	const short nelements = 14;

	c = 0;
	
	ay = 0;

	for(i=0; i<nelements; i++)
	{	
		p.set(0,0,rnd.range(400,600));
		
		ay += omc_MaxAngle/nelements;
		ay += rnd.range(0,omc_MaxAngle/20);	
		az = rnd.range(0,omc_MaxAngle);
	
		p.rotate(0,ay,az);
		p.y += rnd.range(0,400) - 200;
	
		s = rnd.range(100,200);
	
		el = new OMediaCanvasElement;
		el->link(world);
		el->link_layer(sky_layer);
		el->place(p.x,p.y,p.z);
		el->set_angle(0,ay,az);
		el->set_canvas(space_backs[c]);	
		el->set_flags(omelf_SecondPass); 
		el->set_canvas_flags(omcanef_FreeWorldSize);
		el->set_size(s,s);
		el->set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
		el->set_auto_align(omaac_Center,omaac_Center);
		
		c++;
		if (c>=nspace_backs) c =0;
	}
}

void Fullscreen::init_font(void)
{
	// OMT automatically recognizes the PNG format.

	font_canvas 		= load_image("medias/green_font.png");

	// Now I need to build the font:

	font = new OMediaCanvasFont;
	font->set_font_canvas(font_canvas, font_canvas->get_width(), 21, 22);
	font->set_proportional(true);
	font->create_proportional_tab();
	font->set_space_size(10);
	font->set_char_space(3);
}


void Fullscreen::init_demo(void)
{
	OMediaCanvas			*canv;
	OMedia3DPoint			p;

	// Cursor
	
	cursor_canvas = load_image("medias/green_cursor.png");

	cursor_element = new OMediaCanvasElement;
	cursor_element->link(world);
	cursor_element->link_layer(info_layer);
	cursor_element->set_canvas(cursor_canvas);
	cursor_element->set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
	cursor_element->set_auto_align(omaac_Center,omaac_Center);

	// Caption

	caption = new OMediaCaption;
	caption->link(world);
	caption->link_layer(info_layer);
	caption->hide();
	caption->set_font(font);


	// Build sun

	sun_canvas = load_image("medias/sun.png");
	sun_canvas->set_filtering(omtfc_Linear,omtfc_Linear);
	
	sun = new OMediaCanvasElement;
	sun->link(world);
	sun->link_layer(layer);
	sun->set_canvas(sun_canvas);	
	sun->set_flags(omelf_FaceViewport|omelf_SecondPass|omelf_DisableRotate|omelf_CloserHitCheckAlpha); 
	sun->set_canvas_flags(omcanef_FreeWorldSize);
	sun->set_size(130,130);
	sun->set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
	sun->set_auto_align(omaac_Center,omaac_Center);

	// Planets

	canv = omd_GETOBJECT(database,OMediaCanvas,"planet1");
	canv->set_filtering(omtfc_Linear,omtfc_Linear);

	planet = new AutoRotateElement;
	planet->link(sun);
	planet->link_layer(layer);
	planet->set_shape(omd_GETOBJECT(database,OMedia3DShape,"planet1"));
	planet->place(250,0,0);

	canv = omd_GETOBJECT(database,OMediaCanvas,"planet2");
	canv->set_filtering(omtfc_Linear,omtfc_Linear);

	planet2 = new AutoRotateElement;
	planet2->link(sun);
	planet2->link_layer(layer);
	planet2->set_shape(omd_GETOBJECT(database,OMedia3DShape,"planet2"));
	planet2->place(300,0,600);
	planet2->ang_speed = 25;

	canv = omd_GETOBJECT(database,OMediaCanvas,"planet3");
	canv->set_filtering(omtfc_Linear,omtfc_Linear);

	planet3 = new AutoRotateElement;
	planet3->link(sun);
	planet3->link_layer(layer);
	planet3->set_shape(omd_GETOBJECT(database,OMedia3DShape,"planet3"));
	planet3->place(150,0,-50);
	planet3->ang_speed = 45;


	canv = omd_GETOBJECT(database,OMediaCanvas,"planet4");
	canv->set_filtering(omtfc_Linear,omtfc_Linear);

	planet4 = new AutoRotateElement;
	planet4->link(sun);
	planet4->link_layer(layer);
	planet4->set_shape(omd_GETOBJECT(database,OMedia3DShape,"planet4"));
	planet4->place(-300,0,-300);
	planet4->ang_speed = 60;

	
	// Space ship
	
	alien = new AutoRotateElement;
	alien->link(viewport);
	alien->link_layer(layer);
	alien->place(10,10,35);
	alien->set_shape(omd_GETOBJECT(database,OMedia3DShape,"alien"));
	alien->ang_speed = 180;
	alien->set_angle(omd_Deg2Angle(10),0,0);
	
	
	redlight_canv = load_image("medias/redlight.png");

	redlight_el = new OMediaCanvasElement;
	redlight_el->link(world);
	redlight_el->link_layer(redlight_layer);
	redlight_el->set_canvas(redlight_canv);
	redlight_el->set_flags(omelf_FaceViewport|omelf_DisableRotate);
	redlight_el->set_canvas_flags(omcanef_FreeWorldSize);
	redlight_el->set_size(6,6);
	redlight_el->set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
	redlight_el->set_auto_align(omaac_Center,omaac_Center);

	p.set(0,-4,0);
	alien->local_to_world(p);
	redlight_el->place(p);

	OMediaFARGBColor	argb(0,0,0,0);

	redlight_light = new OMediaLight;
	redlight_light->link(redlight_el);
	redlight_light->set_light_type(omclt_Point);
	redlight_light->set_range(20);
	redlight_light->set_diffuse(argb);
	redlight_light->place(0,-4,0);
	redlight_light->set_constant_attenuation(0.5f);

	alien_time=redlight_time=0;

	init_back_layer();
}


//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	Fullscreen	*app;

	app = new Fullscreen;		// Create and start application
	app->start();
	delete app;  
}

