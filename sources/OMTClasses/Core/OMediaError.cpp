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
 
 
#include "OMediaError.h"
 
  
char *OMediaError::error_strings[] =
{
	"Null",
	"Out of memory",
	"Can't open",
	"Out of range",
	"Bad format",
	"OS Error",
	"Can't build",
	"Null pointer",
	"No list",
	"List error",
	"Can't find",
	"Can't unlink",
	"Can't link",
	"Can't remove port",
	"Bad message",
	"OS error",
	"GfxWorld draw error",
	"Bad GfxWorld for DrawPort",
	"Draw to unlocked port",
	"Bad pool entry deleted",
	"Bad pixmap source for blitter compiler",
	"Bad mask for blitter compiler",
	"Bad destination for compiled image",
	"Can't link element to this world",
	"Bad sequence",
	"Can't set an unlinked frame",
	"Bad formatted stream",
	"Can't find formatted stream header",
	"Formatted stream access fault",
	"Formatted stream write error",
	"Can't delete open chunk",
	"Bad world for distorted draw",
	"OMediaWinStoreStartInfo not initialized",
	"Unregistred chunk type required by database",
	"Can't replace locked database object",
	"Bad pixmap destination",
	"Math error",
	"Bad DXF format",
	"Can't open WinSock",
	"Not closed",
	"Bad configuration",
	"Socket error",
	"Point buffer full",
	"Empty DB Chunk",
	"*Check String Field!*",
	"Invalid parameter",
	"No video card linked",
	"Can't find current video mode",
	"OpenGL cannot choose a pixel format",
	"OpenGL cannot create a context",
	"OpenGL cannot attach context",
	"Compression Error",
	"Internet Error",
	"No texture format available",
	"Can't change video mode"
};


char *OMediaError::errortype_to_string(omt_ErrorId errtype)
{
	return error_strings[errtype];
}


