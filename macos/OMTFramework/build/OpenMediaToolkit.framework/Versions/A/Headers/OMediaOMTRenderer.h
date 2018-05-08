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
#ifndef OMEDIA_OMTRenderer_H
#define OMEDIA_OMTRenderer_H

#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_OMTRENDERER
#include "OMediaRenderer.h"
#include "OMediaRendererInterface.h"
#include "OMediaPipeline.h"
#include "OMediaListener.h"

class OMediaPipePolygon;
class OMediaOffscreenBuffer;
class OMediaOMTCanvas;

//----------------------------------
// Render port

class OMediaOMTRenderPort : 	public OMediaRenderPort,
								public OMediaPipe2RendererInterface,
								public OMediaListener
{
	public:
	
	omtshared OMediaOMTRenderPort(OMediaRenderTarget *target);
	omtshared virtual ~OMediaOMTRenderPort();

	omtshared virtual void capture_frame(OMediaCanvas &canv);

	omtshared virtual void set_bounds(OMediaRect &rect);

	omtshared virtual void render(void);

	// * Listen to message
						
	omtshared virtual void listen_to_message(omt_Message msg, void *param =NULL);


	// * Begin/End
	
	omtshared virtual bool begin_render(void);
	omtshared virtual void end_render(void);

	omtshared void send_polygons(void);

	// * Pipeline interface
	
	omtshared virtual void flush_pipeline(void);
	
	omtshared virtual void get_view_bounds(OMediaRect &r);

	omtshared virtual void clear_colorbuffer(OMediaFARGBColor &rgb, OMediaRect *area =NULL);
		
	omtshared virtual void clear_zbuffer(OMediaRect *area);
	omtshared virtual void set_zbuffer_write(omt_ZBufferWrite zb);
	omtshared virtual void set_zbuffer_test(omt_ZBufferTest zb);
	omtshared virtual void set_zbuffer_func(omt_ZBufferFunc zb);

	omtshared virtual void get_zbuffer_info(omt_ZBufferTest &zt, omt_ZBufferWrite &zw, omt_ZBufferFunc &zf);

	omtshared virtual OMediaRenderTarget *get_target(void);

	inline OMediaOffscreenBuffer *get_color_buffer(void) {return cbuffer;}


	protected:

	
	omtshared void render_polygon_points(OMediaPipePolygon *poly);
	omtshared void render_polygon_lines(OMediaPipePolygon *poly);
	omtshared void render_polygon_surface(OMediaPipePolygon *poly);
	omtshared void render_polygon_shaded(OMediaPipePolygon *poly);
	omtshared void render_polygon_texture32(OMediaPipePolygon *poly);
	omtshared void render_polygon_texture16(OMediaPipePolygon *poly);

	omtshared unsigned long point_to_directcolor(OMediaPipePoint	*p);

	
	omtshared virtual void prepare_context(void);
	
	OMediaRect					bounds,current_bounds;
	
	OMediaPipeline						*pipeline;

	char					*cb_pixels;
	long					cb_rowbytes;

	
	omtshared static unsigned short		*zbuffer;
	omtshared static unsigned long		zbuffer_length;
	omtshared static unsigned long		port_counter;
	
	
	unsigned long						zmodulo; 
	
	omt_ZBufferWrite					zb_write;
	omt_ZBufferTest						zb_test;
	omt_ZBufferFunc						zb_func;
	
	OMediaOffscreenBuffer				*cbuffer;
	float								cbuff_fw, cbuff_fh;

	float								half_vpw,half_vph;

};



//----------------------------------
// Render target

class OMediaOMTRenderTarget : public OMediaRenderTarget
{
	public:
	
	OMediaOMTRenderTarget(OMediaRenderer *renderer);
	virtual ~OMediaOMTRenderTarget();
	
	omtshared virtual OMediaRenderPort *new_port(void);

	omtshared virtual void render(void);


	bool						rebuild_contexts;

};


//----------------------------------
// Renderer

class OMediaOMTRenderer : 	public OMediaRenderer
{
	public:
	
	// * Construction
	
	omtshared OMediaOMTRenderer(OMediaVideoEngine *video, OMediaRendererDef *def, omt_ZBufferBitDepthFlags zbuffer);
	omtshared virtual ~OMediaOMTRenderer();


	// * Targets

	omtshared virtual OMediaRenderTarget *new_target(void);
	
	bool		zbuffer;
};



#endif
#endif

