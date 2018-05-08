/*****************************************************************        O P E N      M E D I A     T O O L K I T              V2.5             Copyright Yves Schmid 1996-2003         See www.garagecube.com for more informations about this library.                Author(s): Yves Schmid         OMT is provided under LGPL:           This library is free software; you can redistribute it and/or          modify it under the terms of the GNU Lesser General Public          License as published by the Free Software Foundation; either          version 2.1 of the License, or (at your option) any later version.          This library is distributed in the hope that it will be useful,          but WITHOUT ANY WARRANTY; without even the implied warranty of          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          Lesser General Public License for more details.          You should have received a copy of the GNU Lesser General Public          License along with this library; if not, write to the Free Software          Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA          The full text of the license can be found in lgpl.txt          ******************************************************************/#include "LOMTWindow.h"#include "OMediaMacRtgWindow.h"LOMTWindow::LOMTWindow( LStream* inStream):LWindow(inStream),OMediaWindow(NULL){	monitors = NULL;	retarget = new OMediaMacRtgWindow;	((OMediaMacRtgWindow*)retarget)->window = this;	((OMediaMacRtgWindow*)retarget)->windowptr = mMacWindowP;	bounds_changed();}void LOMTWindow::ShowSelf(){	LWindow::ShowSelf();	bounds_changed();}void LOMTWindow::init_video_engine(){	// * Create a monitor maps		if (monitors) return;			monitors = new OMediaMonitorMap(ommeic_OS,this);	link_monitor_map(monitors);	// * Select renderers for all monitors	// Now we need to pick up a renderer for each monitors. Because each	// monitor can be linked to a different cards, they may have different	// renderers.	//	// The monitor map contains a list of video engines. For each monitors	// I scan the display list and look for an accerated renderer. If there	// is no accelered device, I pick the first renderer of the list.	for(omt_VideoEngineList::iterator vi=monitors->engines.begin();		vi!=monitors->engines.end();		vi++)	{		OMediaRendererDef	*def = omc_NULL;		for(omt_RendererDefList::iterator ri = (*vi)->get_renderer_list()->begin();			ri!=(*vi)->get_renderer_list()->end();			ri++)		{			if ((*ri).attributes & omcrdattr_Accelerated ) def = &(*ri);	// Accelerated ?		}		// No accelerated device. Take the first one.		if (!def && (*vi)->get_renderer_list()->size()) def =  &(*(*vi)->get_renderer_list()->begin());		// Once I found a renderer I need to select it. This will automatically build the renderer for me.		if (def) (*vi)->select_renderer(def,omfzbc_32Bits);	}}LOMTWindow::~LOMTWindow(){	delete monitors;	destroy_dependencies();}void LOMTWindow::close(void){	OMediaWindow::close();	LWindow::DoClose();}omt_WindowStyle LOMTWindow::get_style(void){	return omwstyle_Other;}void LOMTWindow::place(short x, short y){	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);	MoveWindow (rtg->windowptr, x,y, false);	bounds_changed();	AdjustUserBounds();}void LOMTWindow::set_size(short w, short h){	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);	SizeWindow(rtg->windowptr, w, h, false);	Rect	r;	r.left = 0;	r.top = 0;	r.right = w;	r.bottom = h;	InvalWindowRect(rtg->windowptr,&r);	bounds_changed();	AdjustUserBounds();}void LOMTWindow::bounds_changed(void){	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);		Rect	portRect;	GetWindowBounds (rtg->windowptr,kWindowContentRgn,&portRect);	x = portRect.left;	y = portRect.top;	width = portRect.right-portRect.left;	height = portRect.bottom-portRect.top;	OMediaWindow::update_contexts();	OMediaWindow::bounds_changed();}void LOMTWindow::AdjustUserBounds(){	LWindow::AdjustUserBounds();	bounds_changed();}void LOMTWindow::DoSetBounds(	const Rect&		inBounds){	LWindow::DoSetBounds(inBounds);	bounds_changed();}void LOMTWindow::ResizeFrameBy(								SInt16				inWidthDelta,								SInt16				inHeightDelta,								Boolean				inRefresh){	bounds_changed();	LWindow::ResizeFrameBy(inWidthDelta,inHeightDelta,inRefresh);}void LOMTWindow::set_title(string str){	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);	title = str;	Str255	ptitle;	ptitle[0] = title.length();	BlockMove(title.c_str(),ptitle+1, title.length());	SetWTitle (rtg->windowptr,ptitle);}void LOMTWindow::init_window(){		omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);	rtg->windowptr = mMacWindowP;		OMediaWindow::bounds_changed();	}void LOMTWindow::activate(void){	Select();}void LOMTWindow::activated(void){	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);	OMediaWindow::activated();}omt_Bool LOMTWindow::is_active(void){	return (mActive == triState_On);}void LOMTWindow::show(void){	Show();}void LOMTWindow::hide(void){	Hide();	}