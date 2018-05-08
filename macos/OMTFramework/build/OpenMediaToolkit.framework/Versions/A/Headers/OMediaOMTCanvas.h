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
#ifndef OMEDIA_OMTCanvas_H
#define OMEDIA_OMTCanvas_H

#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_OMTRENDERER

#include "OMediaOMTRenderer.h"
#include "OMediaCanvasSlave.h"
#include "OMediaPixelFormat.h"

class OMediaCanvas;

class OMediaOMTCanvasText
{
	public:

	unsigned long				rowbytes_shift;
	unsigned short				xmod_mask,ymod_mask;
	char						*texture;
	float						fwidth,fheight;
	bool						argb1555;
};

class OMediaOMTCanvas : public OMediaCanvasSlave
{
	public:
	
	omtshared OMediaOMTCanvas(OMediaOMTRenderTarget *target, 
							 OMediaCanvas *master,
							 omt_CanvasSlaveKeyFlags key_flags);

	omtshared virtual ~OMediaOMTCanvas();

	omtshared virtual void create_subimage(OMediaCanvas *canv, long w, long h, bool new_texture, long nw, long nh);

	omtshared virtual void master_modified(void);

	omtshared virtual void create_texture(bool new_texture, omt_CanvasSlaveKeyFlags key_flags);

	inline void render_prepare(void) 
	{
		if (dirty)
		{
			create_texture(false, slave_key_long);
			dirty = false;
		}
	}

	void flip_alpha(char *pixels, short width, short height);
	void check_texture_size(long &w, long &h);

	void prepare_depth(void);

	void prepare_omttext(OMediaOMTCanvasText *text,short w, short h, long rowbytes);

	void create_texture_block(	OMediaOMTCanvasText *text,
											short w, 
											short h);

	OMediaOMTCanvasText		*texture_grid;
	omt_PixelFormat			pixel_format;
	short					target_depth;

	long					n_texture,n_textw,n_texth;
	long					subdiv_shift_w, subdiv_shift_h;
	long					subdiv_w_mask,subdiv_h_mask;
	float					subdiv_w,subdiv_h;
	float					last_u,last_v;
	bool					dirty;
	
};


#endif
#endif

