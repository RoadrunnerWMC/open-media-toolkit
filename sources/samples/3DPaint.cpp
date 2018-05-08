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

// Paint3D 
//
// This example shows how to extract pixel level information from a world.
// It allows you to paint over several 3D surfaces linked to the same
// canvas using the mouse cursor.


#include "OMediaApplication.h"
#include "OMediaSingleWindow.h"
#include "OMediaMonitorMap.h"
#include "OMediaRendererDef.h"
#include "OMediaWorld.h"
#include "OMediaViewPort.h"
#include "OMediaLayer.h"
#include "OMedia3DMaterial.h"
#include "OMedia3DShape.h"
#include "OMedia3DShapeElement.h"
#include "OMediaCanvasElement.h"
#include "OMediaLight.h"
#include "OMediaPickRequest.h"
#include "OMediaPrimitiveElement.h"
#include "OMediaPeriodical.h"
#include "OMediaEngineFactory.h"
#include "OMediaEndianSupport.h"

//******************************************************


class Paint3D :	public OMediaApplication,
					public OMediaPeriodical
{
public:

	Paint3D();
	virtual ~Paint3D();

	void init_display(void);
	void init_world(void);
	void init_elements(void);
	void init_canvas(void);

	void spend_time(void);
	
	inline void desktop_to_viewport_coords(float &x, float &y) const
	{
		OMediaRect	bounds;

		viewport->getbounds(&bounds);

		x-=(float)(window->get_x() + bounds.left) ;
		y-=(float)(window->get_y() + bounds.top) ;
	}

	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaInputEngine		*input;

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;
	
	OMediaCanvas			*canvas;
	OMedia3DMaterial		*canvas_mat;
	
	OMedia3DShape			*shape;	
	OMedia3DShapeElement	*sphere;
	
	OMediaTimer				timer;
};

Paint3D::Paint3D()
{
	init_display();
	init_world();
	init_canvas();
	init_elements();

}

Paint3D::~Paint3D()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete shape;

	delete canvas;
	delete canvas_mat;	

	delete monitors;

	delete input;
}

void Paint3D::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(512,512);
	window->place(16,64);
	window->show();

	input = OMediaEngineFactory::get_factory()->create_input_engine(ommeic_OS,window);	// Create an input engine to track
																							// the mouse button.


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

void Paint3D::init_world(void)
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


	// * Layers

	argb.set(1.0f,0.7f,0.7f,0.7f);

	layer = new OMediaLayer;
	layer->link(world);
	layer->add_flags(omlayerf_ClearColor);
	layer->set_clear_color(argb);
}

void Paint3D::init_canvas(void)
{
	OMediaFARGBColor	fargb(1.0f,1.0f,1.0f,1.0f);

	canvas = new OMediaCanvas;
	canvas->create(128,128);
	canvas->fill(fargb);
		
	canvas_mat = new OMedia3DMaterial;
	canvas_mat->set_light_mode(ommlmc_Light);
	canvas_mat->set_shade_mode(omshademc_Gouraud);
	canvas_mat->set_color(fargb, 0.0f,1.0f,1.0f,0.0f);
	canvas_mat->set_texture(canvas);
}


void Paint3D::init_elements(void)
{
	OMediaCanvasElement		*ecanv;

	// * Light

	OMediaLight	*light;
	light = new OMediaLight;
	light->link(world);
	light->set_light_type(omclt_Directional);
	light->set_angle(omd_Deg2Angle(315),omd_Deg2Angle(25),0);

	// * Canvas element
	
	ecanv = new OMediaCanvasElement;
	ecanv->link(world);
	ecanv->place(-80,0,400);
	ecanv->set_size(256,256);
	ecanv->set_auto_align(omaac_Center,omaac_Center);
	ecanv->set_canvas(canvas);
	ecanv->set_angley(omd_Deg2Angle(45));
	
	// * Sphere
	
	shape = new OMedia3DShape;
	shape->make_sphere(64,16,20);
	shape->set_material(canvas_mat);

	sphere = new OMedia3DShapeElement;
	sphere->link(world);
	sphere->place(90,0,400);
	sphere->set_shape(shape);
}


void Paint3D::spend_time(void)
{
	// Rotate the sphere

	if (sphere)
	{
		float elapsed = (float)timer.getelapsed();	
		sphere->add_angle(0,(omt_Angle)(omd_Deg2Angle(45)*(elapsed/1000.0f)),0);
		timer.start();
	}
	
	
	// Drawing

	if (input->mouse_down())		// Mouse still down?
	{
		short	mx,my;
		input->get_mouse_position(mx,my);		// Get mouse position (desktop coordinates)


		float		ox,oy;

		// Transform desktop coordinates to viewport orthographic coordinates
		
		ox = float(mx);
		oy = float(my);
		desktop_to_viewport_coords(ox,oy);

		// Picking...

		OMediaPickRequest		pickrequest;

		pickrequest.level = omplc_Surface;
		pickrequest.flags = ompickf_CloserHit_SurfaceHitOnly;
		pickrequest.pickx = ox;
		pickrequest.picky = oy;
		pickrequest.pickw = 0.5;
		pickrequest.pickh = 0.5;

		viewport->pick(pickrequest);

		// Scan the pick results
		
		if (pickrequest.closer_hit.type!=omptc_Null &&
			pickrequest.closer_hit.sub_info.size())
		{
			long	px,py;
			
			px = long((*pickrequest.closer_hit.sub_info.begin()).u * float(canvas->get_width()));
			py = long((*pickrequest.closer_hit.sub_info.begin()).v * float(canvas->get_height()));
			
			px %= canvas->get_width();
			py %= canvas->get_height();
			if (px<0) px += canvas->get_width();
			if (py<0) py += canvas->get_height();
		
			static bool	t;
			t =!t;

			omt_RGBAPixel	pixel;
			
			if (t) pixel = omd_IfLittleEndianReverseLong(0xFF0000FF);
			else pixel = omd_IfLittleEndianReverseLong(0x00FF00FF);

			canvas->lock(omlf_Write);
			canvas->write_pixel(pixel, px, py);
			canvas->unlock();
		}
	}	
}


//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	Paint3D	*app;

	app = new Paint3D;		// Create and start application
	app->start();
	delete app;  
}

