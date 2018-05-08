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
#ifndef OMEDIA_Menu_H
#define OMEDIA_Menu_H

#include "OMediaTypes.h"
#include "OMediaDefMessages.h"

#include <string>

typedef unsigned short omt_MenuFlags;
const omt_MenuFlags ommenf_Mark = (1<<0);

enum omt_MenuCmd
{
	ommcmdc_Menu,
	ommcmdc_Item,
	ommcmdc_Separator,
	
	ommcmdc_AboutMenu,		// Special command to create a standard about menu.
	ommcmdc_AboutItem,		// The position in the array is not important.

	ommcmdc_End
};

class OMediaMenu
{
	public:

	omt_MenuCmd		cmd;
	char			*name;
	omt_Message		msg;


	char			shortcut;	
	omt_MenuFlags	flags;
};

#define omd_DefaultMenu NULL

#endif

