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
 

#include "OMediaViewPort.h"
#include "OMediaTypes.h"
#include "OMediaElement.h"
#include "OMediaRendererInterface.h"
#include "OMediaWindow.h"
#include "OMediaMatrix.h"
#include "OMediaAnimPeriodical.h"
#include "OMediaPickRequest.h"
#include "OMediaEventManager.h"
#include "OMediaFocus.h"

OMediaViewPort::OMediaViewPort(OMediaSupervisor *super) : OMediaSupervisor(super)
{
	bounds.set(0,0,64,64);
	rel_bounds = bounds;
	layer_key = 0xFFFFFFFFL;
	window = NULL;
	bounds_mode[omcpbc_Left] = bounds_mode[omcpbc_Right] = omvpbmc_LeftRelative;
	bounds_mode[omcpbc_Top] = bounds_mode[omcpbc_Bottom] = omvpbmc_TopRelative;
	picking = NULL;
	flags = 0;

	if (OMediaEventManager::get_event_manager())
		OMediaEventManager::get_event_manager()->getclickbroadcaster()->addlistener(this);
}

OMediaViewPort::~OMediaViewPort()
{
	unlink_window();
}

void OMediaViewPort::setbounds(OMediaRect *b)
{
	rel_bounds = *b;

	if (window)
	{
		if (bounds_mode[omcpbc_Left]==omvpbmc_RightRelative) bounds.left = window->get_width()+b->left;
		else bounds.left = b->left;

		if (bounds_mode[omcpbc_Right]==omvpbmc_RightRelative) bounds.right = window->get_width()+b->right;
		else bounds.right = b->right;

		if (bounds_mode[omcpbc_Top]==omvpbmc_BottomRelative) bounds.top = window->get_height()+b->top;
		else bounds.top = b->top;

		if (bounds_mode[omcpbc_Bottom]==omvpbmc_BottomRelative) bounds.bottom = window->get_height()+b->bottom;
		else bounds.bottom = b->bottom;
	}
	else 
        {
            bounds = *b;
	}

	for(omt_RenderPortList::iterator rpi = render_ports.begin();
		rpi != render_ports.end();
		rpi++)
	{
		(*rpi)->set_bounds(bounds);
	}
}


void OMediaViewPort::listen_to_message(omt_Message msg, void *param)
{
	OMediaEvent	*event;

	switch(msg)
	{
		case omsg_WindowDeleted:
		if (window==(OMediaWindow*)param) unlink_window();
		break;
		
		case omsg_WindowRendererUpdated:
		if (window==(OMediaWindow*)param) update_render_ports();
		break;
		
		case omsg_RenderPortDeleted:
		{
			for(omt_RenderPortList::iterator rpi = render_ports.begin();
				rpi != render_ports.end();
				rpi++)
			{
				if ((*rpi)==(OMediaRenderPort*)param)
				{
					render_ports.erase(rpi);
					break;
				}
			}
		}	
		break;
		
		case omsg_WindowBoundsChanged:
		setbounds(&rel_bounds);
		break;
		
		case omsg_RenderPort:
		render_viewport((OMediaRendererInterface*)param);
		break;

		case omsg_MouseClick:
		event = (OMediaEvent*)param;
		if (event->type==omtet_LMouseDown)
		{
			if (flags&omcvpf_ClickActivateSupervisor) check_mouse_activate(event);
		}
		break;

		case omsg_Event:
		event = (OMediaEvent*)param;
		if (event->type==omtet_LMouseDown)
		{
			if (flags&omcvpf_EnableMouseClick) handle_mouse_hit(event,omsg_ViewPortClicked);
		}
		else if (event->type==omtet_LMouseUp)
		{
			if (flags&omcvpf_EnableMouseClick_Up) handle_mouse_hit(event,omsg_ViewPortClicked_Up);
		}
				
		OMediaSupervisor::listen_to_message(msg,param);
		break;
		
		case omsg_VPCheckMouseTrack:
		mouse_track((OMediaMouseTrackPick*)param);
		break;

		default:
		OMediaSupervisor::listen_to_message(msg,param);
		break;
	}
	
	if (OMediaFocus::get_focus()) OMediaFocus::get_focus()->listen_to_message(msg,param);
}

void OMediaViewPort::update_render_ports(void)
{
	if (!window) return;

	while(render_ports.size()) delete (*render_ports.begin());

	omt_Win2EngList	*win_eng = window->get_win2engines();
	omt_Win2EngList::iterator wi;
	
	for(wi=win_eng->begin();
		wi!=win_eng->end();
		wi++)
	{
		if ((*wi).target)
		{
			OMediaRenderPort *port = (*wi).target->new_port();
			port->addlistener(this);
			port->set_bounds(bounds);
			render_ports.push_back(port);
		}
	}
}

void OMediaViewPort::link_window(OMediaWindow *win)
{
	unlink_window();

	window = win;

	if (window)
	{
		window->addlistener(this);
	
		omt_Win2EngList	*win_eng = window->get_win2engines();
		omt_Win2EngList::iterator wi;
		
		for(wi=win_eng->begin();
			wi!=win_eng->end();
			wi++)
		{
			if ((*wi).target)
			{
				OMediaRenderPort *port = (*wi).target->new_port();
				port->addlistener(this);
				port->set_bounds(bounds);
				render_ports.push_back(port);
			}
		}
	}
	
	setbounds(&rel_bounds);
}

void OMediaViewPort::unlink_window(void)
{	
	while(render_ports.size()) delete (*render_ports.begin());

	if (window)
	{
		window->removelistener(this);
		window = NULL;
	}
}

void OMediaViewPort::render_viewport(OMediaRendererInterface *rdr_i)
{
	list<OMediaPickResult>::iterator	hits_i;

	if (!its_world) return;
	if (!picking && rdr_i->get_pick_mode()) return;

	OMediaMatrix_4x4	viewmatrix;
	long				picking_skip;
	float				px,py,pz;
	short				ax,ay,az;

	if (picking) picking_skip = picking->hits.size();

	if (!its_superelement)
	{
		get_dynamic_offset(px,py,pz);
		px += getx();
		py += gety();
		pz += getz();

		if (!px && !py && !pz &&
			!get_anglex() && !get_angley() && !get_anglez()) viewmatrix.set_identity();
	
		else
			viewmatrix.set_world_transform(px, py, pz, get_anglex(),get_angley(),get_anglez());
	}
	else
	{
		get_absolute_info(px, py, pz, ax, ay, az);
		viewmatrix.set_world_transform(px, py, pz, ax, ay, az);
	}

	for(omt_LayerList::iterator li = its_world->get_layers()->begin();
		li!=its_world->get_layers()->end();
		li++)
	{
		if ((*li)->get_viewport_key()&layer_key)
		{		
			(*li)->render(rdr_i, bounds, viewmatrix);
		}
	}

	if (picking)
	{
		hits_i = picking->hits.begin();
		while(picking_skip--) hits_i++;
		for(;hits_i!=picking->hits.end();hits_i++)
		{
			(*hits_i).viewport = this;
		}
	}
}

void OMediaViewPort::pick(OMediaPickRequest &request)
{
	picking = &request;	
	OMediaAnimPeriodical::get_anim_periocial()->enable_picking_mode(picking);
	OMediaAnimPeriodical::get_anim_periocial()->spend_time();
	OMediaAnimPeriodical::get_anim_periocial()->disable_picking_mode();
	picking = NULL;
        
        analyze_picking_result(request);
}

void OMediaViewPort::analyze_picking_result(OMediaPickRequest &request)
{
	request.closer_hit.type = omptc_Null;

	// Find closer hit

	if (request.hits.size())
	{
		vector<OMediaPickSubResult>::iterator	closer_sub;
		OMediaPickResult						*closer_result = NULL;

		for(list<OMediaPickResult>::iterator pri = request.hits.begin();
			pri != request.hits.end();
			pri++)
		{
			OMediaPickResult	*res = &(*pri);

			for(vector<OMediaPickSubResult>::iterator psi = res->sub_info.begin();
				psi!=res->sub_info.end();
				psi++)
			{
				if (!closer_result || (*psi).minz < (*closer_sub).minz)
				{
					if ((request.level!=omplc_Surface ||
						!(request.flags&ompickf_CloserHit_SurfaceHitOnly) ||
						(*psi).surface_hit) &&
						(*pri).element->validate_closer_hit((*pri),(*psi)))
					{
						closer_result = &(*pri);
						closer_sub = psi;
					}
				}
			}
		}

		if (closer_result)
		{		
			request.closer_hit.type		= closer_result->type;
			request.closer_hit.viewport	= closer_result->viewport;
			request.closer_hit.element  = closer_result->element;
			request.closer_hit.shape	= closer_result->shape;
			request.closer_hit.canvas	= closer_result->canvas;

			request.closer_hit.sub_info.push_back((*closer_sub));
			
		}
	}
}

void OMediaViewPort::check_mouse_activate(OMediaEvent *event)
{
	if (bounds.is_pointin(event->win_mouse_x,event->win_mouse_y))
	{
		OMediaSupervisor::set_main_supervisor(this);
	}	
}

void OMediaViewPort::handle_mouse_hit(OMediaEvent *event,omt_Message msg)
{
	if (bounds.is_pointin(event->win_mouse_x,event->win_mouse_y))
	{
		OMediaPickRequest	request;

		request.level	= omplc_Surface;
		request.flags	= ompickf_CloserHit_SurfaceHitOnly;
		request.pickx	= float(event->win_mouse_x - bounds.left);
		request.picky	= float(event->win_mouse_y - bounds.top);
		request.pickw	= 0.5f;
		request.pickh	= 0.5f;
		
		pick(request);

		if (request.closer_hit.type!=omptc_Null &&
			request.closer_hit.element)
		{
			request.closer_hit.element->clicked(&request.closer_hit,msg==omsg_ViewPortClicked);

			bool	has_focus = false;
			request.closer_hit.element->listen_to_message(omsg_HasFocus,&has_focus);
			if (!has_focus) OMediaFocus::remove_focus();
		}
		else OMediaFocus::remove_focus();

		if (OMediaSupervisor::get_main_supervisor())
			OMediaSupervisor::get_main_supervisor()->listen_to_message(msg,&request);
	}
}

void OMediaViewPort::mouse_track(OMediaMouseTrackPick *request)
{
	if (!window) return;

	request->level	= omplc_Surface;
	request->flags	= ompickf_CloserHit_SurfaceHitOnly;
	request->pickx	= float(request->desktop_mousex - (bounds.left + window->get_x()));
	request->picky	= float(request->desktop_mousey - (bounds.top + window->get_y()));
	request->pickw	= 0.5f;
	request->pickh	= 0.5f;
	request->validated = true;
		
	pick(*request);
}

void OMediaViewPort::capture_frame(OMediaCanvas &canvas, bool capture_alpha)
{
	canvas.create(getbwidth(),getbheight());

	for(omt_RenderPortList::iterator ri=render_ports.begin();
		ri!=render_ports.end();
		ri++)
	{
		if (*ri) (*ri)->capture_frame(canvas);
	}

	if (!capture_alpha) canvas.fill_alpha(0xFF);
}



