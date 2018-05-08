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

#include "OMediaMouseCursor.h"

omt_MouseCursorShape	OMediaMouseCursor::shape;
long 					OMediaMouseCursor::hide_count;


void OMediaMouseCursor::set_shape(omt_MouseCursorShape ashape)
{
	HCURSOR		hcursor;

	switch(ashape)
	{
		case omcmct_Arrow:
		hcursor = LoadCursor(NULL,IDC_ARROW);
		break;
		
		case omcmct_Clock:
		hcursor = LoadCursor(NULL,IDC_WAIT);
		break;
		
		default:
		return;
	}
	
	SetCursor(hcursor);
}

void OMediaMouseCursor::hide(void)
{
	if (hide_count==0) ShowCursor(false);
	hide_count++;
}

void OMediaMouseCursor::show(void)
{
	if (hide_count<=0) {hide_count=0;return;}
	hide_count--;
	if (hide_count==0) ShowCursor(true);
}

void OMediaMouseCursor::force_show(void)
{
	hide_count = 1;
	show();
}

