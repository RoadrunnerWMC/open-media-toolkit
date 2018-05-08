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
 
#include "OMediaWinOffscreenBuffer.h"
#include "OMediaWinRtgWindow.h"
#include "OMediaError.h"
#include "OMediaWindow.h"

OMediaWinOffscreenBuffer::OMediaWinOffscreenBuffer(OMediaVideoEngine *engine, 
										long width, long height, 
										omt_OffscreenBufferPixelFormat pixformat):
										OMediaOffscreenBuffer(engine,width,height,pixformat)
{
	BITMAPINFO	*bitmapinfo;
	VOID		*bitsptr;
	short		ncolors;
	UINT		usage;
	DWORD		compression;
	short		alwidth;
	short		depth;
	
	switch(pixformat)
	{
		case omobfc_ARGB1555:
		depth = 16;
		break;

		default:
		depth = 32;
		break;
	
	}

	win_device_context = CreateCompatibleDC(NULL);
	if (!win_device_context) omd_OSEXCEPTION(GetLastError());

	bitmapinfo = (BITMAPINFO*) new char[sizeof(BITMAPINFOHEADER)+(3L*sizeof(DWORD))];
	ncolors = 0;
	compression = BI_BITFIELDS;
	usage = DIB_RGB_COLORS; 
	
	DWORD	*rgbmask = (DWORD*)(((char*)bitmapinfo)+sizeof(BITMAPINFOHEADER));
	
	if (depth==16)
	{
		rgbmask[0] = 0x7C00;
		rgbmask[1] = 0x03E0;
		rgbmask[2] = 0x001F;
	}
	else
	{
		rgbmask[0] = 0xFF0000;
		rgbmask[1] = 0x00FF00;
		rgbmask[2] = 0x0000FF;	
	}
	

	// Pre-align for NT
	
	short	div = 8/(depth/8),mod;
	mod = width%div;
	if (mod) alwidth = width + (div-mod);
	else alwidth = width;	

	bitmapinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfo->bmiHeader.biWidth = alwidth;
	bitmapinfo->bmiHeader.biHeight = -height;
	bitmapinfo->bmiHeader.biPlanes = 1;
	bitmapinfo->bmiHeader.biBitCount = depth;
	bitmapinfo->bmiHeader.biCompression = compression;
	bitmapinfo->bmiHeader.biSizeImage = 0;
	bitmapinfo->bmiHeader.biClrUsed = ncolors;
	bitmapinfo->bmiHeader.biClrImportant = 0;

	win_bitmap = CreateDIBSection(	win_device_context, 
									bitmapinfo,
									usage,
									&bitsptr,
									omc_NULL,
									0);

	delete bitmapinfo;

	if (!win_bitmap) 
	{
		DeleteDC(win_device_context);
		omd_OSEXCEPTION(GetLastError());
	}
	
	win_old_selected = SelectObject(win_device_context, win_bitmap);
	if (!win_old_selected)
	{
		DeleteDC(win_device_context);
		DeleteObject(win_bitmap);
		omd_OSEXCEPTION(GetLastError());
	}
	
	if (GetObject(win_bitmap,sizeof(BITMAP),&win_bitmap_info)==0)
	{
		omd_OSEXCEPTION(GetLastError());
	}
	
	lock_count = 0;
}
										
OMediaWinOffscreenBuffer::~OMediaWinOffscreenBuffer()
{
	if (win_device_context)
	{
		SelectObject(win_device_context,win_old_selected);
		DeleteObject(win_bitmap);
		DeleteDC(win_device_context);	
	}
}

void OMediaWinOffscreenBuffer::get_pixmap(void *&pixels, long &rowbytes)
{
	if (lock_count)
	{
		pixels = win_bitmap_info.bmBits;
		rowbytes = win_bitmap_info.bmWidthBytes;
	}
	else
	{
		pixels = NULL;
		rowbytes = 0;	
	}
}

void OMediaWinOffscreenBuffer::lock(void)
{
	lock_count++;
	
	if (lock_count==1) GdiFlush();
}

void OMediaWinOffscreenBuffer::unlock(void)
{	
	lock_count--;
}

void OMediaWinOffscreenBuffer::draw(OMediaWindow *window, long x, long y)
{
	omt_RTGDefineLocalTypeObj(OMediaWinRtgWindow,win_rtg,window);
		
	BOOL					res;
	HDC						hdc;

	hdc = GetDC(win_rtg->hwnd);

	res = BitBlt(hdc,
					x,y,
					win_bitmap_info.bmWidth,
					win_bitmap_info.bmHeight,
					win_device_context,
					0,0,
					SRCCOPY);

	ReleaseDC(win_rtg->hwnd,hdc);

	if (!res) omd_OSEXCEPTION(GetLastError());		
}

