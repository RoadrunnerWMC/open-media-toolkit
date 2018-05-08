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

// Particle demo


#include "OMediaApplication.h"
#include "OMediaSingleWindow.h"
#include "OMediaMonitorMap.h"
#include "OMediaCanvas.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"
#include "OMediaRendererDef.h"
#include "OMedia3DShapeElement.h"
#include "OMediaWorld.h"
#include "OMediaViewPort.h"
#include "OMediaLayer.h"
#include "OMediaDatabase.h"
#include "OMedia3DShape.h" 
#include "OMediaParticleEmitter.h"
#include "OMediaRandomNumber.h"

//---------------------------------------------
// Definitions
 
	// Override standard OMT application class

class ParticleDemo : public OMediaApplication
{  
public:

	ParticleDemo();
	virtual ~ParticleDemo();

	void init_display(void);
	void init_font(void); 
	void init_world(void);
	void init_demo(void);

	OMediaCanvas *load_image(string filename);

	OMediaMonitorMap		*monitors;
	OMediaSingleWindow		*window;

	OMediaWorld				*world;
	OMediaViewPort			*viewport;
	OMediaLayer				*layer;

	OMediaCanvas			*canv_particle,*canv_particle2;
};

	// Override particle element

class Particle : public OMediaAbstractParticle
{
public:

	float				life_time,size;
	OMedia3DVector		direction;
};

class Emitter : public OMediaParticleEmitter
{
public:
	
	Emitter(OMediaCanvas *image,float lev_speed);
	virtual ~Emitter();

	// Following method is called for each element before rendering

	virtual void update_logic(float millisecs_elapsed);

	virtual long get_sizeof_particle(void);
	virtual void render_particles(OMediaRendererInterface *rdr_i);

	OMediaCanvas	*image;
	float			gen_timecount,global_angle,lev_y;

	float	wave_ms,lev_speed;
	bool	generate;

};

//---------------------------------------------
// Implementations

ParticleDemo::ParticleDemo()
{
	init_display();
	init_world();
	init_demo();
}

ParticleDemo::~ParticleDemo()
{
	delete world;		// Deleting the world, deletes also sub-elements
						// (viewport, captions, etc.)

	delete canv_particle;
	delete canv_particle2;
	delete monitors;
}

void ParticleDemo::init_display(void)
{
	// * Create window

	window = new OMediaSingleWindow(this);
	window->set_menu(omd_DefaultMenu);
	window->set_size(512,512);
	window->place(40,60);
	window->show();

	// * Create a monitor maps
	
	// A monitor map automatically creates for you one video engine
	// per card installed in your machine. It allows OMT to take
	// care of multiple monitors automatically for you.

	monitors = new OMediaMonitorMap(ommeic_Best,window);
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
		if (def) (*vi)->select_renderer(def,def->zbuffer_depth);
	}
}

void ParticleDemo::init_world(void)
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

	argb.set(1.0f,0,0,0);

	layer = new OMediaLayer;
	layer->link(world);
	layer->add_flags(omlayerf_ClearColor);
	layer->set_clear_color(argb);
}

void ParticleDemo::init_demo(void)
{
	Emitter	*e;

	canv_particle = load_image("medias/particle.png");
	canv_particle2 = load_image("medias/particle2.png");

	e = new Emitter(canv_particle,(30000.0f/1000.0f));
	e->link(world);
	e->place(-100,0,400);

	e = new Emitter(canv_particle2,(20000.0f/1000.0f));
	e->link(world);
	e->place(100,0,400);
}

OMediaCanvas *ParticleDemo::load_image(string filename)
{
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


//---------------------------------------------
// Particles

#define LIFE_TIME	3000

static OMediaRandomNumber	rnd;

Emitter::Emitter(OMediaCanvas *image, float lev_speed)
{
	this->image = image;
	gen_timecount = global_angle = 0;
	set_flags(omelf_FaceViewport);

	wave_ms = rnd.range(800,1200);
	lev_y = 0;
	this->lev_speed = lev_speed;
	generate =true;
}

Emitter::~Emitter()
{
}

void Emitter::update_logic(float millisecs_elapsed)
{
	list<OMediaAbstractParticle*>::iterator pi;

	//float	generate_interval_ms = rnd.range(100,300);
	
	wave_ms -= millisecs_elapsed;
	if (wave_ms<0)
	{
		generate = !generate;
		if (generate) wave_ms = rnd.range(800,1500);
		else wave_ms = rnd.range(200,500);
	}
	

	gen_timecount += millisecs_elapsed;

	// Generate new particles

	if (gen_timecount>=millisecs_elapsed && generate)
	{
		gen_timecount = 0;

		// Generate a new particule

		Particle *p = (Particle*) allocate_particle();

		p->position.set(rnd.range(0,50)-5,rnd.range(0,50)-5,rnd.range(0,50)-5);
		p->direction.set(0.0f,1.0f,0.0f);
		p->direction.rotate(0,0,(omt_Angle)(rnd.range(0,omc_MaxAngle>>1)));
		p->life_time = LIFE_TIME;
		p->size = 3.0f;
	}

	// Process active particles

	for(pi = get_active_particles()->begin();
		pi!= get_active_particles()->end();)
	{
		Particle	*p;
		p = (Particle*)(*pi);
		pi++;		// pi may be deleted by free_particle. So we need to advance it
					// right now.

		p->life_time -= millisecs_elapsed;

		if (p->life_time<=0)	// Test life time.		
		{
			// Delete the particle

			free_particle(p);	
			continue;
		}

		// Apply forces

		float speed = (millisecs_elapsed * (200.0f/1000.0f));

		p->position.x += p->direction.x * speed;
		p->position.y += p->direction.y * speed;
		p->position.z += (millisecs_elapsed * (30.0f/1000.0f));

		p->position.rotate(0,0,omt_Angle(millisecs_elapsed*(5000.0f/1000.0f)));

		p->size += (millisecs_elapsed * (7.0f/1000.0f));
	}

	global_angle +=millisecs_elapsed*(1000.0f/1000.0f);
	if (global_angle>omc_MaxAngle) global_angle -= omc_MaxAngle;

	add_angle(0,0,(omt_Angle)(millisecs_elapsed*(100.0f/1000.0f)));

	// Levitation

	float	res;
	
	lev_y+= millisecs_elapsed*lev_speed;

	res = omd_Sin(omt_Angle(lev_y));
	place(getx(),(res*20)-5,getz());
}

long Emitter::get_sizeof_particle(void)
{
	return sizeof(Particle);
}



void Emitter::render_particles(OMediaRendererInterface *rdr_i)
{
	OMediaRenderVertex				v;
	omt_RenderVertexList			vertices;
	omt_RenderVertexList::iterator	vi;
	float							a,r,g,b;
	Particle						*p;
	OMedia3DPoint					pt;

	v.specular.set(0.0f,0.0f,0.0f);

	vertices.insert(vertices.begin(),4,v);
	vi = vertices.begin();

	rdr_i->set_texture(image);
	rdr_i->set_blend(omblendc_Enabled);
	rdr_i->set_blend_func(omblendfc_Src_Alpha,omblendfc_Inv_Src_Alpha);

	// Render particles

	for(list<OMediaAbstractParticle*>::iterator pi = get_active_particles()->begin();
		pi!=get_active_particles()->end();
		pi++)
	{
		p = (Particle*)(*pi);

		float inv_life_time = LIFE_TIME - p->life_time;

		if (inv_life_time > ((LIFE_TIME/3.0f)*1.0f) )
		{
			a = 1.0f - ((inv_life_time - ((LIFE_TIME/3.0f)*1.0f)) / ((LIFE_TIME/3.0f)*2.0f));
		}
		else a = 1.0f;

		r = g = b = 1.0f;

		if (inv_life_time > (LIFE_TIME/3.0f) )
		{
			g = b = 1.0f - ((inv_life_time - (LIFE_TIME/3.0f)) / ((LIFE_TIME/3.0f)*2.0f));
		}
		
		pt = (*pi)->position;
		pt.rotate(omt_Angle(global_angle),0,0);		

		vi[0].x = pt.x-p->size;
		vi[0].y = pt.y-p->size;
		vi[0].z = pt.z;
		vi[0].u = 0.0f;
		vi[0].v = 0.0f;
		vi[0].diffuse.set(a,r,g,b);

		vi[1].x = pt.x-p->size;
		vi[1].y = pt.y+p->size;
		vi[1].z = pt.z;
		vi[1].u = 0.0f;
		vi[1].v = -1.0f;
		vi[1].diffuse.set(a,r,g,b);

		vi[2].x = pt.x+p->size;
		vi[2].y = pt.y+p->size;
		vi[2].z = pt.z;
		vi[2].u = 1.0f;
		vi[2].v = -1.0f;
		vi[2].diffuse.set(a,r,g,b);

		vi[3].x = pt.x+p->size;
		vi[3].y = pt.y-p->size;
		vi[3].z = pt.z;
		vi[3].u = 1.0f;
		vi[3].v = 0.0f;
		vi[3].diffuse.set(a,r,g,b);

		rdr_i->draw(vertices, omrdmc_Polygon);
	}
}

//---------------------------------------------
// OMT main entry point

void omt_main(void)
{
	ParticleDemo	*app;

	app = new ParticleDemo;		// Create and start application
	app->start();
	delete app;  
}

