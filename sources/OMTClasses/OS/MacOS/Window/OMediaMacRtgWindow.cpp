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

#include "OMediaMacRtgWindow.h"
#include "OMediaSupervisor.h"
#include "OMediaWindow.h"
#include "OMediaRenderer.h"
#include "OMediaFocus.h"


OMediaMacRtgWindow::OMediaMacRtgWindow() : OMediaRetarget(omcrtg_Window)
{
	windowptr = NULL;
	window = NULL;
	render_port = NULL;
}


OMediaMacRtgWindow::~OMediaMacRtgWindow()
{
	delete_menus();	
}

void OMediaMacRtgWindow::process_update_region(void)
{
	if (!window || !windowptr) return;

	SetPort(GetWindowPort(windowptr));

	Rect		r;

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
			
			SetRect(&r,orect.left,orect.top,orect.right,orect.bottom);
			ValidWindowRect(windowptr,&r);		
		}
	}
	
	OMediaRGBColor	c;
	RGBColor		mc;
	window->get_backcolor(c);
	mc.red = c.red;
	mc.green = c.green;
	mc.blue = c.blue;
	SetRect(&r,0,0,window->get_width(),window->get_height());
	
	
	BeginUpdate(windowptr);
	RGBForeColor(&mc);
	PaintRect(&r);
	EndUpdate(windowptr);
}


void OMediaMacRtgWindow::delete_menus(void)
{
	if (!menus.size()) return;

	ClearMenuBar();
	for(vector<MenuHandle>::iterator i = menus.begin();
		i!=menus.end();
		i++)
	{
		DisposeMenu((*i));
	}
	
	menus.erase(menus.begin(),menus.end());	
	msg2items.erase(msg2items.begin(),msg2items.end());	
}

void OMediaMacRtgWindow::create_menus(OMediaMenu *basemenu)
{
	delete_menus();

	MenuHandle	current_m,about_m;
	short		menuid=0;
	short		item=1,about_item=1;
	Str255		pstr;
	OMediaMacMessage2Item	newmsg2it;
    bool		aqua_menu;
    long		response;
    OSErr		err;
	OMediaMenu	*menu;

	current_m = about_m = NewMenu(++menuid, (unsigned char*)"\001\024");
	
	//------------------------
	// About

	menu = basemenu;

	for(;;menu++)
	{
		switch(menu->cmd)
		{
			case ommcmdc_AboutItem:
			pstr[0] = strlen(menu->name);
			BlockMove(menu->name,pstr+1,pstr[0]);
			AppendMenu(about_m, pstr);

			if (menu->shortcut) SetItemCmd(about_m, about_item, menu->shortcut);
			if (menu->flags&ommenf_Mark) SetItemMark(about_m, about_item, checkMark);

			newmsg2it.set_values(menu->msg,1,about_item,about_m);			
			msg2items.push_back(newmsg2it);

			about_item++;
			break;

			case ommcmdc_End:
			menu = NULL;
			break;
                        
            default:
            continue;
		}
		
		if (menu==NULL) break;
	}

	menus.push_back(about_m);
	InsertMenu(about_m,0);

	//-----------

	menu = basemenu;	

    err = Gestalt(gestaltMenuMgrAttr, &response);
	aqua_menu = ((err == noErr) && (response & gestaltMenuMgrAquaLayoutMask));

	for(;;menu++)
	{
		switch(menu->cmd)
		{
			case ommcmdc_Menu:
			pstr[0] = strlen(menu->name);
			BlockMove(menu->name,pstr+1,pstr[0]);
			current_m = NewMenu(++menuid, pstr);
			menus.push_back(current_m);
			InsertMenu(current_m,0);
			item=1;
			break;

			case ommcmdc_Item:
            if (aqua_menu && menu->msg==omsg_Quit) continue;
			pstr[0] = strlen(menu->name);
			BlockMove(menu->name,pstr+1,pstr[0]);
			AppendMenu(current_m, pstr);

			if (menu->shortcut) SetItemCmd(current_m, item, menu->shortcut);
			if (menu->flags&ommenf_Mark) SetItemMark(current_m, item, checkMark);

			newmsg2it.set_values(menu->msg,menuid,item,current_m);			
			msg2items.push_back(newmsg2it);

			item++;
			break;

			case ommcmdc_Separator:
            if (aqua_menu && menu[1].msg==omsg_Quit) continue;
			AppendMenu(current_m, "\p(-");
			item++;
			break;

			case ommcmdc_End:
			return;
                        
            default:
            continue;
		}	
	}
}

void OMediaMacRtgWindow::relink_menu(void)
{
	ClearMenuBar();

	for(vector<MenuHandle>::iterator i = menus.begin();
		i!=menus.end();
		i++)
	{
		InsertMenu((*i),0);		
	}

	DrawMenuBar();
}

bool OMediaMacRtgWindow::menu_to_message(	short menu, 
											short item, 
											omt_Message &msg)
{
	msg = omsg_NULL;

	for(vector<OMediaMacMessage2Item>::iterator i=msg2items.begin();
		i!=msg2items.end();
		i++)
	{
		if (menu==(*i).menu && item==(*i).item)
		{
			msg = (*i).msg;
		
			return true;
		}
	}
	
	return false;
}

void OMediaMacRtgWindow::update_menu_states(void)
{
	OMediaSupervisor	*sup = OMediaSupervisor::get_main_supervisor();
	if (!sup) return;

	for(vector<OMediaMacMessage2Item>::iterator i=msg2items.begin();
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
		
                if (mark)
                    SetItemMark((*i).menu_handle, (*i).item, checkMark);
                else
                    SetItemMark((*i).menu_handle, (*i).item, noMark);
                    
		if (!enabled) DisableMenuItem((*i).menu_handle, (*i).item);
		else EnableMenuItem((*i).menu_handle, (*i).item);
	}
}

