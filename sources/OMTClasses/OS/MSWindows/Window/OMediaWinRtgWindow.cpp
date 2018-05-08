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

#include "OMediaWinRtgWindow.h"
#include "OMediaSupervisor.h"
#include "OMediaWindow.h"
#include "OMediaRenderer.h"
#include "OMediaMemTools.h"
#include "OMediaFocus.h"

OMediaWinRtgWindow::OMediaWinRtgWindow() : OMediaRetarget(omcrtg_Window)
{
	hwnd = NULL;
	accel = NULL;
	has_menu = false;
}


OMediaWinRtgWindow::~OMediaWinRtgWindow()
{
	delete_menus();	
}

void OMediaWinRtgWindow::process_update_region(void)
{
	if (!hwnd) return;

	PAINTSTRUCT		paintstruct;
	RECT			r;


	for(omt_Win2EngList::iterator wi = window->get_win2engines()->begin();
		wi!=window->get_win2engines()->end();
		wi++)
	{
		OMediaRenderTarget	*target = (*wi).target;
		if (!target) return;
		
		for(omt_RenderPortList::iterator ti = target->get_ports()->begin();
			ti!=target->get_ports()->end();
			ti++)
		{
			OMediaRect	orect;
		
			(*ti)->get_bounds(orect);

			r.left		= orect.left;
			r.top		= orect.top;
			r.right		= orect.right;
			r.bottom	= orect.bottom;

			ValidateRect(hwnd,&r);	
		}
	}
	
	OMediaRGBColor	c;
	window->get_backcolor(c);

	BeginPaint(hwnd,&paintstruct);

	HGDIOBJ oldobj;
	HDC		hdc = GetDC(hwnd);
	HBRUSH brush = CreateSolidBrush(c.getbgr32bits());

	oldobj = SelectObject(hdc,brush);
  
	r.left = 0;	r.top = 0;
	r.right = window->get_width();
	r.bottom = window->get_height();
	FillRect(hdc,&r,brush); 

	SelectObject(hdc,oldobj);
	DeleteObject(brush);
	ReleaseDC(hwnd,hdc);	

	EndPaint(hwnd,&paintstruct);
}


void OMediaWinRtgWindow::delete_menus(void)
{
	HMENU menu = GetMenu(hwnd);
	if (menu) 
	{
		SetMenu(hwnd,NULL);
		DestroyMenu(menu);
	}

	if (accel) 
	{
		DestroyAcceleratorTable(accel);
		accel = NULL;
	}
	
	DrawMenuBar(hwnd);
	
	msg2items.erase(msg2items.begin(),msg2items.end());	
	has_menu = false;
}

void OMediaWinRtgWindow::create_menus(OMediaMenu *menu)
{
	HMENU hmenu = GetMenu(hwnd);
	if (hmenu) 
	{
		SetMenu(hwnd,NULL);
		DestroyMenu(hmenu);
	}

	if (accel) 
	{
		DestroyAcceleratorTable(accel);
		accel = NULL;
	}

	has_menu = true;

	msg2items.erase(msg2items.begin(),msg2items.end());

	hmenu = CreateMenu();

	short		item_id=0,menu_id=0;

	OMediaWinMessage2Item	newmsg2it;
	MENUITEMINFO			info;
	HMENU					hpopup = NULL;
	bool					create_about = false,end=false;
	char					*help_name = "Help", *help_about;
	omt_Message				about_msg;
	char					str[256];
	vector<ACCEL>			accel_table;
	ACCEL					newaccel;


	OMediaMemTools::zero(&info,sizeof(MENUITEMINFO));
 
	for(;!end;menu++)
	{
		switch(menu->cmd)
		{
			case ommcmdc_Menu:
			hpopup = CreateMenu();
		    InsertMenu (hmenu, menu_id++, MF_STRING | MF_POPUP | MF_BYPOSITION, (DWORD) hpopup,  menu->name) ; 
			break;

			case ommcmdc_Item:			
			info.cbSize = sizeof(MENUITEMINFO);
			info.fMask =  MIIM_ID | MIIM_TYPE | MIIM_STATE;
			info.fState = MFS_ENABLED | MFS_DEFAULT;
			info.fType =  MFT_STRING ;
			info.wID = item_id;

			if (menu->shortcut)
			{
				short l;
				strcpy(str,menu->name);
				strcat(str,"\tCtrl+");
				l = strlen(str);
				str[l] = menu->shortcut;
				str[l+1] = 0;
				info.dwTypeData = str;
				info.cch = l+1;

				newaccel.fVirt = FCONTROL|FVIRTKEY;
				newaccel.key = menu->shortcut;
				newaccel.cmd = item_id;
				accel_table.push_back(newaccel);
			}
			else 
			{		
				info.dwTypeData = menu->name;
				info.cch = strlen(menu->name);
			}

			if (menu->flags&ommenf_Mark) info.fState = MFS_CHECKED;
			else info.fState = MFS_UNCHECKED;

			newmsg2it.set_values(menu->msg,item_id);	
			msg2items.push_back(newmsg2it);
			InsertMenuItem(hpopup,item_id++,true,&info);
			break;

			case ommcmdc_Separator:
			info.cbSize = sizeof(MENUITEMINFO);
			info.fMask =  MIIM_ID | MIIM_TYPE | MIIM_STATE;
			info.fType = MFT_SEPARATOR;
			info.wID = item_id;
			info.fState = MFS_DISABLED | MFS_DEFAULT;
			info.dwTypeData = 0;
			info.cch = 0;
			InsertMenuItem(hpopup,item_id++,true,&info);
			break;

			case ommcmdc_AboutItem:
			create_about = true;
			help_about = menu->name;
			about_msg = menu->msg;
			break;

			case ommcmdc_AboutMenu:
			help_name = menu->name;
			break;

			case ommcmdc_End:
			SetMenu(hwnd,hmenu);
			DrawMenuBar(hwnd);
			end = true;
			break;
		}		
	}

	if (create_about)
	{
		hpopup = CreateMenu();
	    InsertMenu (hmenu, menu_id++, MF_STRING | MF_POPUP | MF_BYPOSITION, (DWORD) hpopup,  help_name);

		info.cbSize = sizeof(MENUITEMINFO);
		info.fMask =  MIIM_ID | MIIM_TYPE | MIIM_STATE;
		info.fState = MFS_ENABLED | MFS_DEFAULT;
		info.fType =  MFT_STRING ;
		info.wID = item_id;
		info.dwTypeData = help_about;
		info.cch = strlen(help_about);
		info.fState = MFS_UNCHECKED;

		newmsg2it.set_values(about_msg,item_id);
		msg2items.push_back(newmsg2it);
		InsertMenuItem(hpopup,item_id++,true,&info);
	}

	if (accel_table.size())
	{	
		accel = CreateAcceleratorTable(&(*accel_table.begin()),accel_table.size());
	}
}

bool OMediaWinRtgWindow::menu_to_message(short item, omt_Message &msg)
{
	msg = omsg_NULL;

	for(vector<OMediaWinMessage2Item>::iterator i=msg2items.begin();
		i!=msg2items.end();
		i++)
	{
		if (item==(*i).item_id)
		{
			msg = (*i).msg;
		
			return true;
		}
	}
	
	return false;
}

void OMediaWinRtgWindow::update_menu_states(void)
{
	OMediaSupervisor	*sup = OMediaSupervisor::get_main_supervisor();
	if (!sup) return;

	MENUITEMINFO	info;
	HMENU			menu = GetMenu(hwnd);

	if (!menu) return;

	info.cbSize = sizeof(MENUITEMINFO);
	info.fMask = MIIM_STATE;

	for(vector<OMediaWinMessage2Item>::iterator i=msg2items.begin();
		i!=msg2items.end();
		i++)
	{
		bool	enabled,mark;
		mark = false;
		enabled = false;

		if (!OMediaFocus::get_focus() || 
			!OMediaFocus::get_focus()->update_message_state((*i).msg, enabled,mark))
		{
			sup->update_message_state((*i).msg, enabled,mark);
		}
	
		if (!mark) info.fState = MFS_UNCHECKED;
		else info.fState = MFS_CHECKED;

		if (!enabled) info.fState |= MFS_GRAYED;
		else info.fState |= MFS_ENABLED;

		SetMenuItemInfo(menu, (*i).item_id, false, &info);
	}
}

