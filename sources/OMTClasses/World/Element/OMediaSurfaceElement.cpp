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

#include "OMediaSurfaceElement.h"

OMediaSurfaceElement::OMediaSurfaceElement()
{
	surf_flags = 0;
	surf_purge_time = 5000;
	set_canvas(&surf_buffer);
}

OMediaSurfaceElement::~OMediaSurfaceElement()
{
	set_canvas(NULL);
}

void OMediaSurfaceElement::purge_surface(void)
{
	surf_render_timer.stop();
	surf_buffer.purge();
}

void OMediaSurfaceElement::update_logic(float millisecs_elapsed)
{
	if ((surf_flags&omsef_Purgeable) && surf_render_timer.is_running())
	{
		if ((long)surf_render_timer.getelapsed()>=surf_purge_time && surf_buffer.get_width())
		{
			purge_surface();
		}
	}


	OMediaCanvasElement::update_logic(millisecs_elapsed);

}

bool OMediaSurfaceElement::render_reject(OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &projectmatrix,
								  			OMediaRendererInterface *rdr_i)
{
	OMediaSphere		sphere;
	OMedia3DVector		v;
	float				canv_w,canv_h;

	if (!getvisible()) return true;

	get_canv_wordsize(canv_w, canv_h);

	v.set(canv_w,canv_h,0);
	sphere.set(canv_x,canv_y,canv_z);
	sphere.radius = v.quick_magnitude();
	modelmatrix.multiply(sphere);

	if (clip_sphere(sphere,modelmatrix,projectmatrix)) 
	{
		return true;
	}

	if (surf_buffer.get_width()==0 || surf_buffer.get_height()==0) 
	{
		rebuild_surface();
		if (surf_buffer.get_width()==0 || surf_buffer.get_height()==0) return true;
	}

	if (rdr_i->get_pick_mode() && (get_flags()&omelf_DisablePicking)) return true;

	return false;
}

void OMediaSurfaceElement::rebuild_surface(void) {}


void OMediaSurfaceElement::render_geometry(	OMediaRendererInterface *rdr_i, 
											OMediaMatrix_4x4 		&modelmatrix, 
											OMediaMatrix_4x4 		&viewmatrix,
											OMediaMatrix_4x4 		&projectmatrix,
											omt_LightList	 		*lights, 
											omt_RenderModeElementFlags render_flags)
{
	OMediaCanvasElement::render_geometry(rdr_i, modelmatrix, viewmatrix, projectmatrix, lights, render_flags);
	surf_render_timer.start();
}

