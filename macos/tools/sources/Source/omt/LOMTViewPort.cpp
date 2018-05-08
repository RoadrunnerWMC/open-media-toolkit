/*****************************************************************        O P E N      M E D I A     T O O L K I T              V2.5             Copyright Yves Schmid 1996-2003         See www.garagecube.com for more informations about this library.                Author(s): Yves Schmid         OMT is provided under LGPL:           This library is free software; you can redistribute it and/or          modify it under the terms of the GNU Lesser General Public          License as published by the Free Software Foundation; either          version 2.1 of the License, or (at your option) any later version.          This library is distributed in the hope that it will be useful,          but WITHOUT ANY WARRANTY; without even the implied warranty of          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          Lesser General Public License for more details.          You should have received a copy of the GNU Lesser General Public          License along with this library; if not, write to the Free Software          Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA          The full text of the license can be found in lgpl.txt          ******************************************************************/#include "AppConstants.h"#include "LOMTViewPort.h"#include "LOMTWindow.h"#include "OMediaAnimPeriodical.h"#include "OMediaGLRenderer.h"#include "OMediaMacGLRtgRenderer.h"#include "OMediaPickRequest.h"LOMTViewPort::LOMTViewPort(LStream*		inStream):LPane(inStream){	viewport = NULL;	pickingOn = false;	OMediaGLRenderPort::set_glVP_zerobased(true);}LOMTViewPort::~LOMTViewPort(){}void LOMTViewPort::prepare(LOMTWindow *window){	this->window = window;	window->init_video_engine();	viewport = new OMediaViewPort(window);	viewport->link_window(window);		bounds_changed();}void LOMTViewPort::ResizeFrameBy(SInt16		inWidthDelta,								  SInt16		inHeightDelta,								  Boolean		inRefresh){	LPane::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);	bounds_changed();}								  void LOMTViewPort::MoveBy(SInt32		inHorizDelta,						   SInt32		inVertDelta,						   Boolean		inRefresh){	LPane::MoveBy(inHorizDelta, inVertDelta, inRefresh);	bounds_changed();}void LOMTViewPort::ClickSelf(const SMouseDownEvent&	inMouseDown ){	if (pickingOn)	{		OMediaPickRequest	request;		request.level	= omplc_Surface;		request.flags	= ompickf_CloserHit_SurfaceHitOnly;		request.pickx	= float(inMouseDown.whereLocal.h - mFrameLocation.h);		request.picky	= float(inMouseDown.whereLocal.v - mFrameLocation.v);		request.pickw	= 0.5f;		request.pickh	= 0.5f;				viewport->pick(request);				BroadcastMessage(msgViewPortPicked,&request);	}	Point    theOldMouseLoc;	Point    theNewMouseLoc;	FocusDraw();	theOldMouseLoc = inMouseDown.whereLocal;	MouseDown(theOldMouseLoc);	theNewMouseLoc = theOldMouseLoc;	while (::StillDown()) 	{		::GetMouse(&theNewMouseLoc);		if (::EqualPt(theNewMouseLoc, theOldMouseLoc) == false) 		{			Point v;						v.h = theNewMouseLoc.h - theOldMouseLoc.h;			v.v = theNewMouseLoc.v - theOldMouseLoc.v;					MouseDrag(v);			theOldMouseLoc = theNewMouseLoc;		}	}}						  void LOMTViewPort::MouseDown(const Point& p){}void LOMTViewPort::MouseDrag(const Point& p){}	void LOMTViewPort::DrawSelf(){	if (!viewport) return;	OMediaRenderTarget::render_only_these_ports = *(viewport->get_render_ports());		OMediaAnimPeriodical::get_anim_periocial()->renderer_broadcaster.broadcast_message(omsg_RenderFrames,NULL);			OMediaRenderTarget::render_only_these_ports.erase(OMediaRenderTarget::render_only_these_ports.begin(),															OMediaRenderTarget::render_only_these_ports.end());}void LOMTViewPort::bounds_changed(){	OMediaRect	bounds;		if (!viewport) return;	bounds.set(0,0, mFrameSize.width,mFrameSize.height);	bounds.offset(0,(window->get_height()-mFrameSize.height));		viewport->setbounds(&bounds);			set_gl_clip();}void LOMTViewPort::set_gl_clip(){	OMediaRect	bounds;	GLint		coords[4];	omt_Win2EngList::iterator wi;	if (!viewport) return;	bounds.set(mFrameLocation.h,mFrameLocation.v, mFrameLocation.h+mFrameSize.width,mFrameLocation.v+mFrameSize.height);	coords[0] = bounds.left;	coords[1] = window->get_height()-(bounds.top+bounds.get_height());	coords[2] = bounds.right-bounds.left;	coords[3] = bounds.bottom-bounds.top;	for(wi=window->get_win2engines()->begin();		wi!=window->get_win2engines()->end();		wi++)	{		OMediaGLRenderTarget		*target;		OMediaMacGLRtgRenderTarget	*rtg;				target = (OMediaGLRenderTarget*)(*wi).target;		rtg = (OMediaMacGLRtgRenderTarget*)target->get_retarget();			if (!rtg->context) target->prepare_context(window);		if (!rtg->context) continue;		aglEnable(rtg->context,AGL_BUFFER_RECT);		aglSetInteger(rtg->context,AGL_BUFFER_RECT,coords); 	}}