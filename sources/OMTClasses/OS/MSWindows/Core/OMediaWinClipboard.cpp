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

#include "OMediaClipboard.h"
#include "OMediaMemTools.h"
#include "OMediaWinRtgWindow.h"
#include "OMediaWindow.h"

static HWND clip_window; 

void OMediaClipboard::open(void)
{
	OMediaWindow	*front_win;

	if (clip_window) return;

	front_win = OMediaWindow::find_active();
	if (!front_win) return;

	clip_window = ((OMediaWinRtgWindow*)front_win->get_retarget())->hwnd;

	if (!OpenClipboard(clip_window)) clip_window = NULL;
}

void OMediaClipboard::close()
{
	if (!clip_window) return;

	CloseClipboard();

	clip_window = NULL;
}
	
void OMediaClipboard::clear()
{
	if (!clip_window) return;
	
	EmptyClipboard();
}
	
bool OMediaClipboard::clip_exist(omt_ClipType t)
{
	UINT	fmt;

	if (!clip_window) return false;

	switch(t)
	{
		case omclip_Text:
		fmt = CF_TEXT;
		break;

		default:
		return false;
	}

	return IsClipboardFormatAvailable(fmt)?true:false;
}

bool OMediaClipboard::get_clip(omt_ClipType t, OMediaMoveableMem &mem)
{
	HGLOBAL		handle;
	UINT		format = 0;
	char		*str;

	if (!clip_window) return false;

	mem.setsize(0);

	for(;;)
	{
		format = EnumClipboardFormats(format);
		if (!format) break;

		switch(format)
		{
			case CF_TEXT:
			if (t==omclip_Text)
			{
				handle = GetClipboardData(CF_TEXT);
				str = (char*)GlobalLock(handle);             
				if (str != NULL)
				{
					long	n = strlen(str);
					mem.setsize(n);
					OMediaMemTools::copy(str,mem.lock(),n);
					mem.unlock();

					GlobalUnlock(handle);
				}
			}
			break;
		}
	}

	return false;
}

void OMediaClipboard::add_clip(omt_ClipType t, OMediaMoveableMem &mem)
{
	UINT		format;
	HGLOBAL		handle;
	char		*str;

	if (!clip_window) return;

	switch(t)
	{
		case omclip_Text:
		format = CF_TEXT;
		break;

		default:
		return;
	}

	long	n = mem.getsize();

	handle = GlobalAlloc(GMEM_DDESHARE, n+1);
	if (!handle) return;

	str = (char*)GlobalLock(handle);
	OMediaMemTools::copy(mem.lock(),str,n);
	str[n] = 0;

	mem.unlock();
	GlobalUnlock(handle);
	SetClipboardData(CF_TEXT, handle); 
}

