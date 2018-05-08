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

#pragma once
#ifndef OMEDIA_SurfaceElement_H
#define OMEDIA_SurfaceElement_H

#include "OMediaCanvasElement.h"
#include "OMediaPeriodical.h"
#include "OMediaTimer.h"

typedef unsigned short omt_SurfaceElementFlags;
const omt_SurfaceElementFlags omsef_Purgeable = (1<<0);			// Automatically purge the surface if the element has not 
																// been rendered for a specified amount of time.


class OMediaSurfaceElement : 	public OMediaCanvasElement
{
	public:

	// * Construction

	omtshared OMediaSurfaceElement();
	omtshared virtual ~OMediaSurfaceElement();
	
	omtshared virtual void purge_surface(void);
	
	// * Canvas
	
	inline OMediaCanvas *get_surface(void) {return &surf_buffer;}

	
	// * Flags
	
	inline void set_surface_flags(const omt_SurfaceElementFlags f) {surf_flags = f;}
	inline omt_SurfaceElementFlags get_surface_flags(void) const {return surf_flags;}

	// * Update logic
	
	omtshared virtual void update_logic(float millisecs_elapsed);

	// * Render

	omtshared virtual void render_geometry(	OMediaRendererInterface *rdr_i, 
											OMediaMatrix_4x4 &modelmatrix, 
											OMediaMatrix_4x4 &viewmatrix, 
											OMediaMatrix_4x4 &projectmatrix,
											omt_LightList	 *lights, 
											omt_RenderModeElementFlags render_flags);

	omtshared virtual bool render_reject(OMediaMatrix_4x4 &modelmatrix, OMediaMatrix_4x4 &projectmatrix,
								  			OMediaRendererInterface *rdr_i);


	// * Auto-purge (only if omsef_Purgeable flag is set)

		// Purge if element has not been rendered during the specified time. Default is 5000 (5 secs)
	inline void set_purge_time(const long time_ms) {surf_purge_time = time_ms;}
	inline long get_purge_time(long time_ms) const {return surf_purge_time;}


	omtshared virtual void rebuild_surface(void);	// Called when a purged surface needs to be rebuilt

	protected:

	OMediaCanvas				surf_buffer;
	long						surf_purge_time;
	OMediaTimer					surf_render_timer;
	omt_SurfaceElementFlags		surf_flags;
};

#endif

