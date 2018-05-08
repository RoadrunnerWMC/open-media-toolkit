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
 
#include "OMediaBuildSwitch.h"

#ifdef omd_ENABLE_DIRECTX
#include "OMediaDXOffscreenBuffer.h"
#include "OMediaDXVideoEngine.h"
#include "OMediaWinRtgWindow.h"
#include "OMediaError.h"
#include "OMediaWindow.h"
#include "OMediaMemTools.h"

OMediaDXOffscreenBuffer::OMediaDXOffscreenBuffer(OMediaVideoEngine *engine, 
										long width, long height, 
										omt_OffscreenBufferPixelFormat pixformat):
										OMediaWinOffscreenBuffer(engine,width,height,pixformat)
{	
}
										
OMediaDXOffscreenBuffer::~OMediaDXOffscreenBuffer()
{
}

void OMediaDXOffscreenBuffer::draw(OMediaWindow *window, long x, long y)
{
	if (((OMediaDXVideoEngine*)its_engine)->exclusive_mode)
	{
		HRESULT		ddrval;
		HDC			hdc;
		OMediaDXVideoEngine	*dxengine = (OMediaDXVideoEngine*)its_engine;

		for(;;)
		{


			if (dxengine->page_flipping)
				ddrval = dxengine->dx_back_page_surf->GetDC(&hdc);
			else
				ddrval = dxengine->dx_primary_surf->GetDC(&hdc);
			
			if (ddrval==DD_OK)
			{
				BitBlt(hdc,
						x,y,
						win_bitmap_info.bmWidth,
						win_bitmap_info.bmHeight,
						win_device_context,
						0,0,
						SRCCOPY);

				if (dxengine->page_flipping)
					dxengine->dx_back_page_surf->ReleaseDC(hdc);
				else
					dxengine->dx_primary_surf->ReleaseDC(hdc);
				break;
			}
			
			if(ddrval == DDERR_SURFACELOST)
			{
				dxengine->dx_primary_surf->Restore();
				if (dxengine->page_flipping) dxengine->dx_back_page_surf->Restore();
				break;
			}
			else if (ddrval != DDERR_WASSTILLDRAWING) break;
		}
	}
	else
	OMediaWinOffscreenBuffer::draw(window, x, y);
}

#endif


