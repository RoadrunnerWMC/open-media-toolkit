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

#include "OMediaSingleWindow.h"
#include "OMediaMacRtgWindow.h"

extern OMediaMenu	omc_DefaultMenus[];


OMediaSingleWindow::OMediaSingleWindow(OMediaSupervisor *supervisor,
										bool initially_visible,
										bool dont_initialize):
					OMediaWindow(supervisor)
{
	retarget = new OMediaMacRtgWindow;
	((OMediaMacRtgWindow*)retarget)->window = this;

	min_width = 100;
	min_height = 80;

	max_width = 30000;
	max_height = 30000;

	visible = initially_visible;
	if (!dont_initialize) init_window(initially_visible);
}

OMediaSingleWindow::~OMediaSingleWindow()
{
	destroy_dependencies();
	cleanup_window();
}

void OMediaSingleWindow::close(void)
{
	OMediaWindow::close();

	if (window_list.size()==1) listen_to_message(omsg_Quit);
}


omt_WindowStyle OMediaSingleWindow::get_style(void)
{
	return omwstyle_Single;
}

void OMediaSingleWindow::place(short x, short y)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	MoveWindow (rtg->windowptr, x,y, false);
	bounds_changed();
}

void OMediaSingleWindow::set_size(short w, short h)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	SizeWindow(rtg->windowptr, w, h, false);

	Rect	r;

	r.left = 0;
	r.top = 0;
	r.right = w;
	r.bottom = h;
	InvalWindowRect(rtg->windowptr,&r);

	update_contexts();
	bounds_changed();
}

void OMediaSingleWindow::bounds_changed(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);
	
	Rect	portRect;

	GetWindowBounds (rtg->windowptr,kWindowContentRgn,&portRect);

	x = portRect.left;
	y = portRect.top;
	width = portRect.right-portRect.left;
	height = portRect.bottom-portRect.top;

	OMediaWindow::bounds_changed();
}


void OMediaSingleWindow::set_max_size(short w, short h) 
{
	max_width = w;
	max_height = h;
}

void OMediaSingleWindow::set_min_size(short w, short h) 
{
	min_width = w;
	min_height = h;
}

void OMediaSingleWindow::get_max_size(short &w, short &h) 
{
	w=max_width;
	h=max_height;
}

void OMediaSingleWindow::get_min_size(short &w, short &h) 
{
	w=min_width;
	h=min_height;
}



void OMediaSingleWindow::set_title(string str)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	title = str;

	Str255	ptitle;

	ptitle[0] = title.length();
	BlockMove(title.c_str(),ptitle+1, title.length());

	SetWTitle (rtg->windowptr,ptitle);
}

void OMediaSingleWindow::init_window(bool initially_visible)
{	
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);
	Rect 	winbounds;
	Str255	ptitle;

	ptitle[0] = title.length();
	BlockMove(title.c_str(),ptitle+1, title.length());
	
	winbounds.left 		= 16;
	winbounds.top 		= 50;
	winbounds.right 	= winbounds.left+320;
	winbounds.bottom 	= winbounds.top+320;

	rtg->windowptr = NewCWindow(NULL, &winbounds,ptitle,initially_visible,zoomDocProc, 
										(WindowPtr)-1L,
					 					true,
					 					(long)this);
	x = winbounds.left;
	y = winbounds.top;
	width = winbounds.right-winbounds.left;
	height = winbounds.bottom-winbounds.top;

	set_backcolor(back_color);
}

void OMediaSingleWindow::cleanup_window(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	DisposeWindow(rtg->windowptr);
}

void OMediaSingleWindow::activate(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	if (!is_active())
	{
		SelectWindow(rtg->windowptr);
	}
}

void OMediaSingleWindow::activated(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	rtg->relink_menu();
	OMediaWindow::activated();
}

omt_Bool OMediaSingleWindow::is_active(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	return (FrontWindow()==rtg->windowptr);
}

void OMediaSingleWindow::show(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	ShowWindow(rtg->windowptr);
	visible = true;
}

void OMediaSingleWindow::hide(void)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	HideWindow(rtg->windowptr);
	visible = false;
}

void OMediaSingleWindow::set_menu(OMediaMenu *menu_array)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	if (menu_array==omd_DefaultMenu)
		rtg->create_menus(omc_DefaultMenus);	
	else
		rtg->create_menus(menu_array);
}


bool OMediaSingleWindow::update_message_state(omt_Message msg, 
												bool &enabled,
												bool &mark)
{
	switch(msg)
	{
		case omsg_Close:
		enabled = true;
		break;
	
		default:
		return OMediaSupervisor::update_message_state(msg, enabled, mark);
	}

	return true;
}

void OMediaSingleWindow::set_backcolor(const OMediaRGBColor &rgb)
{
	omt_RTGDefineLocalType(OMediaMacRtgWindow,rtg);

	OMediaWindow::set_backcolor(rgb);

	Rect	r;
	SetRect(&r,0,0,get_width(),get_height());

	InvalWindowRect(rtg->windowptr,&r);

}


OMediaMenu	omc_DefaultMenus[] = 
{
	{ommcmdc_Menu,		"File",				omsg_NULL, 			0, 		0} ,
	{ommcmdc_Item,		"New",				omsg_New, 			'N', 	0},
	{ommcmdc_Item,		"Open...",			omsg_Open, 			'O', 	0},
	{ommcmdc_Item,		"Close",			omsg_Close, 		'W', 	0},
	{ommcmdc_Separator,	NULL,			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Save",				omsg_Save, 			'S', 	0},
	{ommcmdc_Item,		"Save As...",		omsg_SaveAs, 		0, 		0},
	{ommcmdc_Item,		"Revert",			omsg_Revert, 		0, 		0},
	{ommcmdc_Separator,	NULL,			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Page Setup...",	omsg_PageSetup, 	0, 		0},
	{ommcmdc_Item,		"Print...",			omsg_Print, 		'P', 	0},
	{ommcmdc_Separator,	NULL,			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Quit",				omsg_Quit, 			'Q', 	0},

	{ommcmdc_Menu,		"Edit",				omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Undo",				omsg_Undo, 			'Z', 	0},
	{ommcmdc_Separator,	NULL,			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Copy",				omsg_Copy, 			'C', 	0},
	{ommcmdc_Item,		"Cut",				omsg_Cut, 			'X', 	0},
	{ommcmdc_Item,		"Paste",			omsg_Paste, 		'V', 	0},
	{ommcmdc_Item,		"Clear",			omsg_Clear, 		0, 		0},
	{ommcmdc_Separator,	NULL,			omsg_NULL, 			0, 		0},
	{ommcmdc_Item,		"Select All",		omsg_SelectAll, 	'A', 	0},

	{ommcmdc_AboutMenu,	"Help",				omsg_NULL, 			0, 		0},
	{ommcmdc_AboutItem,	"About...",			omsg_About, 		0, 		0},

	{ommcmdc_End,		NULL,			omsg_NULL, 			0, 		0}
};

