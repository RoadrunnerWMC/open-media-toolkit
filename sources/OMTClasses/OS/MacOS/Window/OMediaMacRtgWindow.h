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
#ifndef OMEDIA_MacRtgWindow_H
#define OMEDIA_MacRtgWindow_H

#include "OMediaRetarget.h"
#include "OMediaMenu.h"


#include <vector>

class OMediaWindow;

class OMediaMacMessage2Item
{
	public:
	
	inline void set_values(omt_Message msg, short menu, short item, MenuHandle menu_handle)
	{
		this->msg = msg;
		this->menu = menu;
		this->item = item;
		this->menu_handle = menu_handle;
	}
	
	omt_Message			msg;
	short				item;
	short				menu;
	MenuHandle			menu_handle;
};

class OMediaMacRtgWindow : public OMediaRetarget
{
	public:

	OMediaMacRtgWindow(void);
	virtual ~OMediaMacRtgWindow();

	WindowPtr						windowptr;
	CGrafPtr						render_port;
	vector<MenuHandle>				menus;
	vector<OMediaMacMessage2Item>	msg2items;
	
	OMediaWindow					*window;

	void process_update_region(void);

	void delete_menus(void);
	void create_menus(OMediaMenu *menu);	
	void relink_menu(void);

	bool menu_to_message(short menu, short item, omt_Message &msg);

	void update_menu_states(void);

};


#endif
