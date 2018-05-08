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
 


#include "OMediaRenderer.h"
#include "OMediaVideoEngine.h"
#include "OMediaAnimPeriodical.h"

//----------------------------------
// Render port

OMediaRenderPort::OMediaRenderPort(OMediaRenderTarget *target)
{
	this->target = target;
	bounds.set(0,0,0,0);
	
	target->get_ports()->push_back(this);
	plist_iterator = target->get_ports()->end();
	plist_iterator--;
}

OMediaRenderPort::~OMediaRenderPort()
{
	broadcast_message(omsg_RenderPortDeleted,this);

	target->get_ports()->erase(plist_iterator);
}

void OMediaRenderPort::set_bounds(OMediaRect &rect)
{
	bounds = rect;
}

void OMediaRenderPort::render(void) {}

void OMediaRenderPort::capture_frame(OMediaCanvas &canv) {}


//----------------------------------
// Render target

omt_RenderPortList OMediaRenderTarget::render_only_these_ports;

OMediaRenderTarget::OMediaRenderTarget(omt_EngineID id):OMediaEngine(id,NULL)
{
    this->renderer = NULL;
    window = NULL;
}

OMediaRenderTarget::OMediaRenderTarget(OMediaRenderer *renderer, omt_EngineID id,
										OMediaWindow *master_window) : OMediaEngine(id,master_window)
{
	this->renderer = renderer;

	renderer->get_targets()->push_back(this);
	tlist_iterator = renderer->get_targets()->end();
	tlist_iterator--;
	
	window = NULL;
}

OMediaRenderTarget::~OMediaRenderTarget()
{
	broadcast_message(omsg_RenderTargetDeleted,this);

	while(ports.size()) delete *(ports.begin());

        if (renderer) renderer->get_targets()->erase(tlist_iterator);
}

OMediaRenderPort *OMediaRenderTarget::new_port(void)
{
	return new OMediaRenderPort(this);
}

void OMediaRenderTarget::render(void)
{
	if (render_only_these_ports.size())
	{	
		for(omt_RenderPortList::iterator pi = ports.begin();
			pi!=ports.end();
			pi++)
		{
			omt_RenderPortList::iterator spi;
		
			for(spi = render_only_these_ports.begin();
				spi!=render_only_these_ports.end();
				spi++)
			{
				if ((*spi)==(*pi)) break;
			}
			
			if (spi==render_only_these_ports.end()) continue;
		
			(*pi)->render();
		}
	
		return;	
	}

	for(omt_RenderPortList::iterator pi = ports.begin();
		pi!=ports.end();
		pi++)
	{
		(*pi)->render();
	}


}

void OMediaRenderTarget::erase_context(void) {}
void OMediaRenderTarget::update_context(void) {}

//----------------------------------
// Renderer

OMediaRenderer::OMediaRenderer(OMediaVideoEngine *video,OMediaRendererDef *def, omt_ZBufferBitDepthFlags zbuffer)
{
	this->video = video;
	picking_mode = NULL;
	addlistener(video);

	OMediaAnimPeriodical::get_anim_periocial()->renderer_broadcaster.addlistener(this);
}

OMediaRenderer::~OMediaRenderer()
{
	broadcast_message(omsg_RendererDeleted,this);

	while(targets.size()) delete *(targets.begin());
}

OMediaRenderTarget *OMediaRenderer::new_target(void)
{
	return new OMediaRenderTarget(this,ommeic_Null,video->get_supervisor_window());
}

void OMediaRenderer::listen_to_message(omt_Message msg, void *param)
{
	switch(msg)
	{
		case omsg_RenderFrames:
		picking_mode = (OMediaPickRequest*)param;
		render();
		picking_mode = NULL;
		break;
	}
}

void OMediaRenderer::render(void)
{
	for(omt_RenderTargetList::iterator ti = targets.begin();
		ti!=targets.end();
		ti++)
	{
		(*ti)->render();
	}
}

