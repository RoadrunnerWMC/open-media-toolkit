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
// MagicHat - magical OMT user-interface


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
#include "OMediaInputEngine.h"
#include "OMediaPickRequest.h"
#include "OMediaCanvasButton.h"
#include "OMediaStdButton.h"
#include "OMediaRadioGroup.h"
#include "OMediaScroller.h"
#include "OMediaSlider.h"
#include "OMediaStringField.h"
#include "OMediaMouseCursor.h"
#include "OMediaFont.h"
#include "OMedia3DShape.h"
#include "OMedia3DShapeElement.h"
#include "OMedia3DMaterial.h"
#include "OMediaPrimitiveElement.h"

//---------------------------------------------
// Definitions

	// Override standard OMT application class

class MagicHat :	public OMediaApplication,
					public OMediaPeriodical,
					public OMediaListener
{
public:

	MagicHat();
	virtual ~MagicHat();

	void init_display(void);
	void init_font(void);
	void init_world(void);
	void init_demo(void);

	void init_database(void);
	void close_database(void);

	void spend_time(void);

	OMediaSlider *create_slider(	string text, 
										float x, float y, float z,
										OMediaElement *super_e,
										omt_Message	msg,
										float		value);

	OMediaStdButton *create_button(string text, 
										float x, float y, float z,
										omt_Message	msg);


	virtual void listen_to_message(omt_Message msg, void *param);

	virtual void update_pict_shadow(OMediaCanvasElement *e);


	static MagicHat *app;


	OMediaCanvas *load_image(string filename);



	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaCanvas			*brush, *alpha_brush;

	OMediaCanvas			*font_canvas;
	OMediaCanvasFont		*font;

	OMediaCanvas			*small_font_canvas;
	OMediaCanvasFont		*small_font;


	OMediaWorld				*world;
	OMediaViewPort			*viewport;

	OMediaLayer				*pict_layer, *front_layer;

	OMediaCanvasElement		*canv_element[2];
	OMediaElement			*super_element;

	OMediaDataBase			*database;
	OMediaFileStream		*dbfile;	

	OMediaRadioGroup		rgroup;
	omt_Message				current_mode;

	OMedia3DShape			*sphere_shape;
	OMedia3DShapeElement	*sphere_element;
	OMedia3DMaterial		*sphere_material;

	OMediaPrimitiveElement	*pict_shadow;

	OMediaTimer				timer;

};

//---------------------------------------------

const omt_Message	omsg_Red		= 10;
const omt_Message	omsg_Green		= 11;
const omt_Message	omsg_Blue		= 12;
const omt_Message	omsg_Alpha		= 13;
const omt_Message	omsg_Z			= 14;
const omt_Message	omsg_Viewrotate	= 15;
const omt_Message	omsg_Viewzoom	= 16;

const omt_Message	omsg_ModeDrag		= 17;
const omt_Message	omsg_ModeDrawRed	= 18;
const omt_Message	omsg_ModeDrawBlue	= 19;
const omt_Message	omsg_ModeDrawGreen	= 20;
const omt_Message	omsg_ModeDrawAlpha	= 21;

class MagicHatTarget
{
	public:

	MagicHatTarget() {dragging = false;}

	bool	dragging;
	short	mouse_ix,mouse_iy;

	virtual void clicked(OMediaElement *pe, OMediaWorld *its_world, OMediaPickResult *res, bool mouse_down)
	{
		if (mouse_down)
		{

			if (MagicHat::app->current_mode==omsg_ModeDrag)
			{
				OMediaMouseCursor::hide();
				dragging = true;
				its_world->get_input_engine()->get_mouse_position(mouse_ix,mouse_iy);

			}
			else
			{
				its_world->get_mouse_tracking_broadcaster().addlistener(pe);
				draw_brush(pe,res);
			}		
		}
		else
		{
			its_world->get_mouse_tracking_broadcaster().removelistener(pe);
		}
	}

	void draw_brush(OMediaElement *pe, OMediaPickResult *result)
	{
		if (result->type!=omptc_Null &&
			result->element==pe &&
			result->sub_info.size())
		{
			long	px,py;
			omt_RGBAPixelMask	pmask;
			OMediaCanvas		*canvas = result->canvas;
			if (!canvas && result->shape)
			{
				result->shape->lock(omlf_Read);
				OMedia3DPolygon *poly = &((*result->shape->get_polygons())[result->sub_info[0].polygon]);

				if (poly->get_material()) 
				{
					canvas = poly->get_material()->get_texture();
				}

				result->shape->unlock();
			}

			if (!canvas) return;
			
			px = long((*result->sub_info.begin()).u * float(canvas->get_width()));
			py = long((*result->sub_info.begin()).v * float(canvas->get_height()));
			
			px %= canvas->get_width();
			py %= canvas->get_height();
			if (px<0) px += canvas->get_width();
			if (py<0) py += canvas->get_height();


			if (MagicHat::app->current_mode==omsg_ModeDrawAlpha)
			{
				canvas->draw_full(MagicHat::app->alpha_brush, 
								px - MagicHat::app->brush->get_width()/2, 
								py - MagicHat::app->brush->get_height()/2, 
								omblendfc_Zero, omblendfc_Src_Alpha, ompixmc_Alpha);
			}
			else
			{
				switch(MagicHat::app->current_mode)
				{
					case omsg_ModeDrawRed:		pmask = ompixmc_Red;	break;
					case omsg_ModeDrawBlue:		pmask = ompixmc_Blue;	break;
					case omsg_ModeDrawGreen:	pmask = ompixmc_Green;	break;
				}

				canvas->draw_full(MagicHat::app->brush, 
								px - MagicHat::app->brush->get_width()/2, 
								py - MagicHat::app->brush->get_height()/2, 
								omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha, pmask);
			}
		}
	}

	virtual void update_logic(OMediaElement *pe, OMediaWorld *its_world, float ms)
	{
		if (!its_world->get_input_engine()->mouse_down())
		{
			dragging = false;
			its_world->get_mouse_tracking_broadcaster().removelistener(pe);
			OMediaMouseCursor::show();
		}

		if (dragging)
		{
			short mx,my;
			its_world->get_input_engine()->get_mouse_position(mx,my);
			
			pe->move(float(mx-mouse_ix),-float(my-mouse_iy));
			mouse_ix = mx;
			mouse_iy = my;		
		}
	}

};



class MagicHatDragPic : public OMediaCanvasElement,
						public MagicHatTarget
{
public:

	MagicHatDragPic() 
	{
		set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
	}
	
	virtual ~MagicHatDragPic() {}

	virtual void listen_to_message(omt_Message msg, void *param)
	{
		bool diffuse_changed = false;

		switch(msg)
		{
			case omsg_Red:
			diffuse.red = float(((OMediaSlider*)param)->get_value()) / 100.0f;
			diffuse_changed = true;
			break;

			case omsg_Green:
			diffuse.green = float(((OMediaSlider*)param)->get_value()) / 100.0f;
			diffuse_changed = true;
			break;

			case omsg_Blue:
			diffuse.blue = float(((OMediaSlider*)param)->get_value()) / 100.0f;
			diffuse_changed = true;
			break;

			case omsg_Alpha:
			diffuse.alpha = float(((OMediaSlider*)param)->get_value()) / 100.0f;
			diffuse_changed = true;
			break;

			case omsg_Z:
			set_anglez( (omt_Angle)((float(((OMediaSlider*)param)->get_value()) / 100.0f) * omc_MaxAngle ));
			break;

			case omsg_MouseTrack:
			draw_brush(this,&((OMediaPickRequest*)param)->closer_hit);
			break;
		}

		if (diffuse_changed && MagicHat::app->sphere_material->get_texture()==canvas)
		{
			MagicHat::app->sphere_material->set_diffuse(diffuse);
			MagicHat::app->update_pict_shadow(this);
		}

	}

	virtual void clicked(OMediaPickResult *res, bool mouse_down)
	{
		OMediaElement *e;

		MagicHatTarget::clicked(this, its_world, res, mouse_down);

		if (mouse_down)
		{
			MagicHat::app->sphere_material->set_texture(canvas);
			MagicHat::app->sphere_material->set_diffuse(diffuse);
			MagicHat::app->update_pict_shadow(this);

			if (this==MagicHat::app->canv_element[0]) e = MagicHat::app->canv_element[1];
			else  e = MagicHat::app->canv_element[0];
					
			place(getx(),gety(),0);
			e->place(e->getx(),e->gety(),20);
		}
	}

	virtual void update_logic(float ms)
	{
		MagicHatTarget::update_logic(this, its_world, ms);
		OMediaCanvasElement::update_logic(ms);		

		if (dragging)
		{
			MagicHat::app->pict_shadow->place(getx()+10,gety()-10,10);
		}
	}

};

class MagicHatDragShape : public OMedia3DShapeElement,
							public MagicHatTarget
{
public:

	MagicHatDragShape() {}
	
	virtual ~MagicHatDragShape() {}

	virtual void listen_to_message(omt_Message msg, void *param)
	{
		switch(msg)
		{
			case omsg_MouseTrack:
			draw_brush(this,&((OMediaPickRequest*)param)->closer_hit);
			break;

			default:
			OMedia3DShapeElement::listen_to_message(msg, param);
		}
	}

	virtual void clicked(OMediaPickResult *res, bool mouse_down)
	{
		MagicHatTarget::clicked(this, its_world, res, mouse_down);
	}

	virtual void update_logic(float ms)
	{
		MagicHatTarget::update_logic(this, its_world, ms);
		OMedia3DShapeElement::update_logic(ms);		
	}

};

//---------------------------------------------
// Implementations

MagicHat *MagicHat::app;


MagicHat::MagicHat()
{
	world = NULL;
	app = this;

	init_database();
	init_display();
	init_font();
	init_world();
	init_demo();
}

MagicHat::~MagicHat()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete font;
	delete font_canvas;
	delete small_font;
	delete small_font_canvas;
	delete brush;
	delete alpha_brush;
	delete sphere_shape;
	delete sphere_material;

	delete monitors;

	close_database();

	OMediaMouseCursor::show();
}

void MagicHat::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(800,600);
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
		if (def) (*vi)->select_renderer(def,0);
	}
}

OMediaCanvas *MagicHat::load_image(string filename)
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

void MagicHat::init_font(void)
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

void MagicHat::init_world(void)
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


	// * Layers

	argb.set(1.0f,0.9f,0.9f,0.9f);

	pict_layer = new OMediaLayer;
	pict_layer->link(world);
	pict_layer->set_flags(	omlayerf_Visible|omlayerf_ClearColor);

	pict_layer->set_clear_color(argb);

	front_layer = new OMediaLayer;
	front_layer->link(world);
	front_layer->set_flags(	omlayerf_Visible);
	front_layer->set_projection_type(omlptc_Ortho);
	front_layer->set_near_clip(0);
}

void MagicHat::listen_to_message(omt_Message msg, void *param)
{
	if (msg>=omsg_ModeDrag && msg<=omsg_ModeDrawAlpha)
	{
		current_mode = msg;
		return;
	}

	switch(msg)
	{
		case omsg_Viewrotate:
		super_element->set_angley((omt_Angle)( (float(((OMediaSlider*)param)->get_value()) / 100.0f) * omc_MaxAngle) );
		break;

		case omsg_Viewzoom:
		super_element->place(0,0, 200 + ((float(((OMediaSlider*)param)->get_value()) / 100.0f) * 400 ));
		break;

		default:
		OMediaApplication::listen_to_message(msg, param);
		break;
	}
}

OMediaSlider *MagicHat::create_slider(	string text, 
										float x, float y, float z,
										OMediaElement *super_e,
										omt_Message	msg,
										float		value)
{
	OMediaSlider	*slider;
	OMediaCaption	*caption;

	slider = new OMediaSlider;
	if (super_e) 
	{
		slider->link(super_e);
		slider->addlistener(super_e);
	}
	slider->place(x,y,z);
	slider->set_size(100,16);
	slider->set_slider_mode(omsmc_Horizontal);
	slider->set_maxvalue(100);
	slider->set_value(value);
	slider->set_message(msg);

	if (text.size())
	{
		caption = new OMediaCaption;
		caption->link(slider);
		caption->set_string(text);
		caption->set_font(small_font);
		caption->place( -(small_font->get_text_length(text)+4),0);
	}


	return slider;
}

OMediaStdButton *MagicHat::create_button(string text, 
										float x, float y, float z,
										omt_Message	msg)
{
	OMediaStdButton	*button;
	button = new OMediaStdButton;
	button->link(world);
	button->link_layer(front_layer);
	button->place(x,y,z);
	button->set_font(small_font);
	button->set_string(text);
	button->set_toggle_mode(true);
	button->set_message(msg);
	button->addlistener(this);
	button->set_button_size(100,0);

	rgroup.add_button(button);

	return button;
}

void MagicHat::init_demo(void)
{
	float	y,x;

	super_element = new OMediaElement;
	super_element->link(world);
	super_element->link_layer(pict_layer);
	super_element->place(0,0,400);

	for(short i=0; i<2; i++)
	{
		canv_element[i] = new MagicHatDragPic;
		canv_element[i]->link(super_element);
		canv_element[i]->set_canvas(omd_GETOBJECT(	database,
													OMediaCanvas, 
													(i==0)?"TanakaShot1":"TanakaShot2"));


		canv_element[i]->set_auto_align(omaac_Center,omaac_Center);
		canv_element[i]->set_flags(omelf_RenderSubElementAfter|omelf_CloserHitCheckAlpha);



		y = (canv_element[i]->get_canvas()->get_height() * 0.5) - 10;
		x = canv_element[i]->get_canvas()->get_width() * -0.5;

		create_slider("Red", x,y,-1, canv_element[i], omsg_Red,100); y -= 20;
		create_slider("Green",x ,y,-1, canv_element[i], omsg_Green,100); y -= 20;
		create_slider("Blue", x,y,-1, canv_element[i], omsg_Blue,100); y -= 20;
		create_slider("Alpha", x,y,-1, canv_element[i], omsg_Alpha,100); y -= 20;
		create_slider("Z", x,y,-1, canv_element[i], omsg_Z,0); y -= 20;

		OMediaStringField	*field;

		y = (canv_element[i]->get_canvas()->get_height() * -0.5) + 10;
		x = (canv_element[i]->get_canvas()->get_width() * -0.5) + 20;
	
		field = new OMediaStringField;
		field->link(canv_element[i]);
		field->place(x,y,-15);
		field->set_size(100,16);
		field->set_string("Image name");
		field->set_deselected_transp(false);
		field->set_font(small_font);	
		field->set_auto_resize(false,200);
	}

	canv_element[0]->place(-100,-100,0);
	canv_element[1]->place(100,100,20);

	OMediaSlider *slider = create_slider("Rotate view", 200,-160,0, NULL, omsg_Viewrotate,0);
	slider->link(world);
	slider->link_layer(front_layer);
	slider->addlistener(this);

	slider = create_slider("Zoom view", 200,-180,0, NULL, omsg_Viewzoom,0);
	slider->link(world);
	slider->link_layer(front_layer);
	slider->addlistener(this);
	slider->set_value(50);

	brush = load_image("medias/gradient.png");
	alpha_brush = load_image("medias/inv_gradient.png");

	create_button("Drag", 200, -40, 0, omsg_ModeDrag)->select();
	create_button("Draw red", 200, -60, 0, omsg_ModeDrawRed);
	create_button("Draw blue", 200, -80, 0, omsg_ModeDrawBlue);
	create_button("Draw green", 200, -100, 0, omsg_ModeDrawGreen);
	create_button("Draw alpha", 200, -120, 0, omsg_ModeDrawAlpha);

	current_mode = omsg_ModeDrag;

	sphere_material = new OMedia3DMaterial;
	sphere_material->set_texture(canv_element[0]->get_canvas());
	sphere_material->set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);

	sphere_shape = new OMedia3DShape;
	sphere_shape->make_sphere(60,10,20);
	sphere_shape->set_material(sphere_material);

	sphere_element = new MagicHatDragShape;
	sphere_element->link(world);
	sphere_element->link_layer(front_layer);
	sphere_element->place(-200,130,200);
	sphere_element->set_shape(sphere_shape);
	sphere_element->set_flags(omelf_CloserHitCheckAlpha);

	pict_shadow = new OMediaPrimitiveElement;
	pict_shadow->link(world);
	pict_shadow->link_layer(pict_layer);
	pict_shadow->set_draw_mode(omrdmc_Polygon);
	pict_shadow->set_blend(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);
	pict_shadow->place(canv_element[0]->getx()+10,canv_element[0]->gety()-10,10);
	pict_shadow->set_flags(omelf_DisablePicking);
	pict_shadow->hide();

	update_pict_shadow(canv_element[0]);
}

void MagicHat::update_pict_shadow(OMediaCanvasElement *e)
{
	OMediaRenderVertex		v;
	float					w,h;
	

	pict_shadow->get_vertices()->erase(	pict_shadow->get_vertices()->begin(),
										pict_shadow->get_vertices()->end());

	w = e->get_canvas()->get_width()*0.5;
	h = e->get_canvas()->get_height()*0.5;

	v.z = 0;
	e->get_diffuse(v.diffuse);
	v.diffuse.alpha *=0.5f;

	v.x  = -w;	v.y  = -h;	
	pict_shadow->get_vertices()->push_back(v);
	v.x  = -w;	v.y  = h;	
	pict_shadow->get_vertices()->push_back(v);
	v.x  = w;	v.y  = h;	
	pict_shadow->get_vertices()->push_back(v);
	v.x  = w;	v.y  = -h;	
	pict_shadow->get_vertices()->push_back(v);
}

void MagicHat::spend_time(void)
{
	// Simply sort the element by hand:
 
	canv_element[0]->unlink();
	canv_element[1]->unlink();
	pict_shadow->unlink();

	bool zero_first;

	zero_first = (canv_element[0]->getz()>canv_element[1]->getz());

	if (super_element->get_angley() > omd_Deg2Angle(90) && 
		super_element->get_angley() < omd_Deg2Angle(270)) zero_first = !zero_first;

	if (zero_first) 
	{
		canv_element[0]->link(super_element);
		pict_shadow->link(super_element);
		canv_element[1]->link(super_element);
	}
	else
	{
		canv_element[1]->link(super_element);
		pict_shadow->link(super_element);
		canv_element[0]->link(super_element);
	}

	float elapsed = timer.getelapsed();

	sphere_element->add_angle(0,omt_Angle(elapsed * omd_Deg2Angle(40)/1000.0f),0);

	timer.start();

}

void MagicHat::init_database(void)
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

void MagicHat::close_database(void)
{
	delete database;
	delete dbfile;
}


//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	MagicHat	*app;

	app = new MagicHat;		// Create and start application
	app->start();
	delete app;  
}

