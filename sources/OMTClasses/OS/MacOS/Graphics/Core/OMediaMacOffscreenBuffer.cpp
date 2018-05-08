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
 
#include "OMediaMacOffscreenBuffer.h"
#include "OMediaMacVideoEngine.h"
#include "OMediaMacRtgWindow.h"
#include "OMediaError.h"
#include "OMediaWindow.h"

OMediaMacOffscreenBuffer::OMediaMacOffscreenBuffer(OMediaVideoEngine *engine, 
										long width, long height, 
										omt_OffscreenBufferPixelFormat pixformat):
										OMediaOffscreenBuffer(engine,width,height,pixformat)
{
	Rect				picRect;
	OSErr				err;
	short				depth;

	picRect.bottom 	= height;
	picRect.right 	= width;
	picRect.top 	= 0;
	picRect.left 	= 0;
	

	switch(pixformat)
	{
		case omobfc_ARGB1555:
		depth = 16;
		break;

		default:
		depth = 32;
		break;
	
	}
	
	err = NewGWorld(&gworld,depth,&picRect,NULL,NULL,0);
	
	if (err!=noErr) omd_OSEXCEPTION(err);
	if (gworld==NULL) omd_EXCEPTION(omcerr_OutOfMemory);
	
	lock_count = 0;
}
										
OMediaMacOffscreenBuffer::~OMediaMacOffscreenBuffer()
{
	DisposeGWorld(gworld);
}

void OMediaMacOffscreenBuffer::get_pixmap(void *&pixels, long &rowbytes)
{
	if (lock_count)
	{
		PixMapHandle 	macpixmap;
		BitMap			*macbitmap;
		
		macpixmap = GetGWorldPixMap(gworld);
		macbitmap = (BitMap*) *(macpixmap);

		pixels = GetPixBaseAddr(macpixmap);

		rowbytes = (long) ((unsigned short)macbitmap->rowBytes & (~ ((1<<15)|(1<<14)|(1<<13)) ));
	}
	else
	{
		pixels = NULL;
		rowbytes = 0;	
	}
}

void OMediaMacOffscreenBuffer::lock(void)
{
	lock_count++;
	
	if (lock_count==1)
	{
		PixMapHandle 	macpixmap;

		macpixmap = GetGWorldPixMap(gworld);
		LockPixels(macpixmap);
	}
}

void OMediaMacOffscreenBuffer::unlock(void)
{
	if (lock_count==1)
	{
		PixMapHandle 	macpixmap;

		macpixmap = GetGWorldPixMap(gworld);
		UnlockPixels(macpixmap);
	}
	
	lock_count--;
}

void OMediaMacOffscreenBuffer::draw(OMediaWindow *window, long x, long y)
{
	Rect		 		s,d;
	GrafPtr				old_port = NULL,win_port;
	omt_RTGDefineLocalTypeObj(OMediaMacRtgWindow,win_rtg,window);
	PixMapHandle 		macpixmap;
	BitMap				*macbitmap;
		
	lock();

	GetPort(&old_port);
	win_port = GetWindowPort(win_rtg->windowptr);
	SetPort(win_port);

	s.left = 0;
	s.top  = 0;
	s.right = width;
	s.bottom = height;

	d.left = x;
	d.top  = y;
	d.right = x + width;
	d.bottom = y + height;

	macpixmap = GetGWorldPixMap(gworld);
	macbitmap = (BitMap*) *(macpixmap);

	CopyBits(macbitmap, (BitMap *) *::GetPortPixMap(win_port),&s,&d,srcCopy,NULL);


	RgnHandle	rgn = NewRgn ();
	SetRectRgn (rgn,x, y, x + width, y + height);

	QDFlushPortBuffer(win_port,rgn);
	DisposeRgn(rgn);

	SetPort(old_port);		

	unlock();
}

