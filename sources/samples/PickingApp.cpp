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

// Picking and mouse click sample
//
// This example shows how to receive mouse click informations and how to
// do picking in a viewport. Picking is used to get objects that are
// enclosed by a viewport rectangle. Picking is also used by mouse click
// to determinate what objects are under the mouse pointer.
// This example handles multiple and single selection of polygons. A
// rectangle is drawn to a second layer to display the active selection in
// front of the 3D scene.


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
#include "OMediaLight.h"
#include "OMediaPickRequest.h"
#include "OMediaPrimitiveElement.h"
#include "OMediaPeriodical.h"
#include "OMediaEngineFactory.h"

//******************************************************


class PickingApp :	public OMediaApplication,
					public OMediaPeriodical
{
public:

	PickingApp();
	virtual ~PickingApp();

	void init_display(void);
	void init_world(void);
	void init_elements(void);

	void listen_to_message(omt_Message msg, void *param);
	void handle_click(OMediaPickRequest *pick);

	void spend_time(void);


	// OMT makes a difference between geometry coordinates (canvas, shape, line, etc.) and 
	// system coordinates (window, viewport, desktop, buffers, canvases, etc.). 
	// Transformation matrixes are always applied to any element or geometry that is rendered
	// to the viewport. For this reason, all elements and canvases coordinates are dependant
	// of the layer matrix. When you want to do 2D animation with OMT you should use an
	// orthographic matrix. The default orthographic matrix transforms coordinates 
	// so the (0/0) is at the center of the viewport and the y goes up.
	// However when you are working with system coordinates, the (0/0) is always at the left/top
	// of the container with y going down. For these reasons, when you receive system coordinates
	// like the mouse position and want to use it to move a world 2D element you need to transform
	// the coordinates. The following method takes desktop coordinates and transforms them to
	// viewport orthographic coordinates.

	inline void desktop_to_world_coords(float &x, float &y) const
	{
		OMediaRect	bounds;

		viewport->getbounds(&bounds);

		x-=(float)window->get_x();
		y-=(float)window->get_y();

		x = (x-float(bounds.left)) - (float(bounds.get_width()) * 0.5f);
		y = (y-float(bounds.top)) - (float(bounds.get_height()) * 0.5f);
		y = -y;
	}

	void id_to_materials(void);
	void materials_to_id(void);


	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaInputEngine		*input;

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer,*sel_layer;

	OMedia3DMaterial		*selection_mat, *blue_mat;
	OMedia3DShape			*sphere_shape;

	OMediaPrimitiveElement	*selection_el;
	bool					selection_mode;
	short					selection_origin_x,selection_origin_y;
};

PickingApp::PickingApp()
{
	selection_mode = false;
																							// the mouse button.
	init_display();

	input = OMediaEngineFactory::get_factory()->create_input_engine(ommeic_OS,window);	// Create an input engine to track

	init_world();
	init_elements();

}

PickingApp::~PickingApp()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete blue_mat;
	delete selection_mat;
	delete sphere_shape;

	delete monitors;

	delete input;
}

void PickingApp::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(512,512);
	window->place(16,64);
	window->show();

	omt_EngineID video_engine = ommeic_OS; //*(OMediaEngineFactory::get_factory()->video_engines.begin());

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

void PickingApp::init_world(void)
{
	OMediaRect			r;
	OMediaFARGBColor	argb;

	// * World

	world = new OMediaWorld;	// Create the root class

	// * Viewport

	viewport = new OMediaViewPort(window);		// Supervisor is window
	viewport->link(world);
	viewport->link_window(window);				// Output is window

	// I need to set the following flag to get the viewport handling mouse click.

	viewport->set_flags(omcvpf_EnableMouseClick|omcvpf_ClickActivateSupervisor);


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

	// Selection layer

	// This layer is a 2D layer. I set the default projection matrix to orthographic and
	// set the near clip to zero.

	sel_layer = new OMediaLayer;
	sel_layer->link(world);
	sel_layer->set_flags(omlayerf_DisablePicking|omlayerf_Visible);
	sel_layer->set_projection_type(omlptc_Ortho);
	sel_layer->set_near_clip(0.0f);		
}

void PickingApp::init_elements(void)
{
	OMediaFRGBColor			rgb;
	OMedia3DShapeElement	*e3d;

	// * Light

	OMediaLight	*light;
	light = new OMediaLight;
	light->link(world);
	light->set_light_type(omclt_Directional);
	light->set_angle(omd_Deg2Angle(315),omd_Deg2Angle(25),0);

	// * Sphere

	rgb.set(0,0,1.0f);
	blue_mat = new OMedia3DMaterial;
	blue_mat->set_light_mode(ommlmc_Light);
	blue_mat->set_shade_mode(omshademc_Gouraud);
	blue_mat->set_color(rgb, 0.0f,1.0f,1.0f,0.3f);

	rgb.set(1.0f,0,0.0f);
	selection_mat = new OMedia3DMaterial;
	selection_mat->set_light_mode(ommlmc_Light);
	selection_mat->set_shade_mode(omshademc_Gouraud);
	selection_mat->set_color(rgb, 0.0f,1.0f,1.0f,0.3f);

	sphere_shape = new OMedia3DShape;
	sphere_shape->make_sphere(120,16,20);
	sphere_shape->set_material(blue_mat);

	e3d = new OMedia3DShapeElement;
	e3d->link(world);
	e3d->link_layer(layer);
	e3d->place(0,0,400);
	e3d->set_shape(sphere_shape);

	// Selection rectangle

	OMediaRenderVertex	rv;
	selection_el = new OMediaPrimitiveElement;
	selection_el->link(world);
	selection_el->link_layer(sel_layer);
	selection_el->set_draw_mode(omrdmc_LineLoop);
	selection_el->hide();

	rv.diffuse.set(1.0f,1.0f,1.0f,1.0f);
	rv.specular.set(0,0,0);
	rv.set(0,0,0);

	// Push empty points. It is filled later when the user moves the mouse.

	selection_el->get_vertices()->push_back(rv);
	selection_el->get_vertices()->push_back(rv);
	selection_el->get_vertices()->push_back(rv);
	selection_el->get_vertices()->push_back(rv);
}

void PickingApp::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_ViewPortClicked:
		// This is sent by the viewport when the omcvpf_EnableMouseClick flag is set.
		// The viewport must be the main supervisor in order to receive mouse clicks.
		// A pick request is associated with this message. It contains informations regarding
		// the mouse click.
		handle_click((OMediaPickRequest*)param);
		break;

		default:
		OMediaApplication::listen_to_message(msg,param);
	}
}

void PickingApp::handle_click(OMediaPickRequest *pick)
{
	// I check the closer hit (the front most element)
	// if it is not NULL the mouse click hitted an element.

	if (pick->closer_hit.type!=omptc_Null &&
		pick->closer_hit.shape &&
		pick->closer_hit.sub_info.size() &&
		pick->closer_hit.sub_info[0].polygon!=-1)
	{
		OMedia3DPolygon	*poly;

		sphere_shape->lock(omlf_Read);
		poly = &(*sphere_shape->get_polygons())[pick->closer_hit.sub_info[0].polygon];
		sphere_shape->unlock();

		if (poly->get_material()==blue_mat)			// For single selection, I just inverse
			poly->set_material(selection_mat);		// the material.
		else
			poly->set_material(blue_mat);
	}

	// Now I start multiple selection if required:

	selection_mode = true;
	input->get_mouse_position(selection_origin_x,selection_origin_y);

	materials_to_id();		// Save the current selection
}

// I use the polygon user's identifier to mark the temporary selection

void PickingApp::id_to_materials(void)	// Restore selection
{
	for(omt_PolygonList::iterator pi = sphere_shape->get_polygons()->begin();
		pi != sphere_shape->get_polygons()->end();
		pi++)
	{
		if ((*pi).get_id()) (*pi).set_material(selection_mat);
		else (*pi).set_material(blue_mat);
	}
}

void PickingApp::materials_to_id(void)	// Save selection
{
	for(omt_PolygonList::iterator pi = sphere_shape->get_polygons()->begin();
		pi != sphere_shape->get_polygons()->end();
		pi++)
	{
		if ((*pi).get_material()==blue_mat) (*pi).set_id(0);
		else (*pi).set_id(1);
	}
}

void PickingApp::spend_time(void)
{
	if (selection_mode)	// Selection mode ?
	{
		if (input->mouse_down())		// Mouse still down?
		{
			short	mx,my;

			input->get_mouse_position(mx,my);		// Get mouse position (desktop coordinates)

			if (mx==selection_origin_x || my==selection_origin_y)
			{
				// The selection rectangle is empty

				id_to_materials();		// Restore selection
				selection_el->hide();
			}
			else
			{
				selection_el->force_show();
				id_to_materials();		// Restore selection

				// Now prepare the selection element

				float		ox,oy,dx,dy;

				// Transform desktop coordinates to viewport orthographic coordinates

				ox = selection_origin_x;
				oy = selection_origin_y;
				dx = mx;
				dy = my;

				desktop_to_world_coords(ox,oy);
				desktop_to_world_coords(dx,dy);

				// Set up the rectangle

				(*selection_el->get_vertices())[0].x = ox;
				(*selection_el->get_vertices())[0].y = oy;
				(*selection_el->get_vertices())[1].x = ox;
				(*selection_el->get_vertices())[1].y = dy;
				(*selection_el->get_vertices())[2].x = dx;
				(*selection_el->get_vertices())[2].y = dy;
				(*selection_el->get_vertices())[3].x = dx;
				(*selection_el->get_vertices())[3].y = oy;

				// Pick the selected polygons

				OMediaPickRequest		pickrequest;

				pickrequest.level = omplc_Geometry;
				pickrequest.pickx = (selection_origin_x - window->get_x()) + ((mx-selection_origin_x)/2);
				pickrequest.picky = (selection_origin_y - window->get_y()) + ((my-selection_origin_y)/2);
				pickrequest.pickw = abs(mx-selection_origin_x);
				pickrequest.pickh = abs(my-selection_origin_y);

				viewport->pick(pickrequest);

				// Scan the pick results and select the required polygons

				sphere_shape->lock(omlf_Read);

				for(list<OMediaPickResult>::iterator pi = pickrequest.hits.begin();
					pi!=pickrequest.hits.end();
					pi++)
				{
					for(vector<OMediaPickSubResult>::iterator psi = (*pi).sub_info.begin();
						psi!=(*pi).sub_info.end();
						psi++)
					{
						if ((*psi).polygon!=-1) 
						{
							OMedia3DPolygon	*p;
							
							p = &(*sphere_shape->get_polygons())[(*psi).polygon];
							p->set_material(selection_mat);
						}
					}
				}

				sphere_shape->unlock();			
			}
		}
		else
		{
			selection_el->hide();
			selection_mode = false;
		}		
	}
}


//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	PickingApp	*app;

	app = new PickingApp;		// Create and start application
	app->start();
	delete app;  
}

