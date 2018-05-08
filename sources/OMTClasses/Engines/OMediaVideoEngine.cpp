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

#include "OMediaVideoEngine.h"

// Abstract

OMediaVideoEngine::OMediaVideoEngine(omt_EngineID id, OMediaWindow *master) : OMediaEngine(id,master)
{
	state = 0;
	renderer = NULL;
}

OMediaVideoEngine::~OMediaVideoEngine()
{
	unlink();
	
	broadcast_message(omsg_VideoEngineDeleted,this);
}


OMediaVideoMode *OMediaVideoEngine::get_current_video_mode(void)
{
	return NULL;
}

void OMediaVideoEngine::link(OMediaVideoCard *vcard) {}

void OMediaVideoEngine::link_quiet(OMediaVideoCard *vcard)
{
	// By default, use standard link:
	
	link(vcard);
}

void OMediaVideoEngine::unlink(void) 
{
	broadcast_message(omsg_VideoEngineUnlinked,this);
	deselect_renderer();
}

void OMediaVideoEngine::set_mode(OMediaVideoMode *mode) {}

void OMediaVideoEngine::get_bounds(OMediaRect &rect) const 
{
	rect.set(0,0,0,0);
}


void OMediaVideoEngine::select_renderer(OMediaRendererDef*,
										omt_ZBufferBitDepthFlags zbuffer)
{
}

void OMediaVideoEngine::deselect_renderer(void) {}

OMediaOffscreenBuffer *OMediaVideoEngine::create_offscreen_buffer(	long width, long height, 
															 		omt_OffscreenBufferPixelFormat	pixel_format)
{
	return NULL;
}

void OMediaVideoEngine::delete_offscreen_buffers(void)
{
	while(offscreen_buffers.size()) delete *(offscreen_buffers.begin());
}

void OMediaVideoEngine::flip_page(void) {}
