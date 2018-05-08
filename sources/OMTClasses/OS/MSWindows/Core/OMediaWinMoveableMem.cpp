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

#include "OMediaWinRtgMoveMem.h"
#include "OMediaMoveableMem.h"
#include "OMediaError.h"

OMediaMoveableMem::OMediaMoveableMem(long blocksize)
{
	HLOCAL	hdl;

	lock_count = 0;
	init_retarget();

	hdl = LocalAlloc(LMEM_MOVEABLE,blocksize);
	((OMediaWinRtgMoveMem*)retarget)->memblock = hdl;
	
	if (!hdl) omd_EXCEPTION(omcerr_OutOfMemory);
	
	real_size = blocksize;
}

OMediaMoveableMem::~OMediaMoveableMem(void)
{
	HLOCAL hdl = ((OMediaWinRtgMoveMem*)retarget)->memblock;

	if (lock_count) 
	{
		lock_count = 1;
		unlock();
	}

	if (hdl) LocalFree(hdl);
	
	delete retarget;
}

long OMediaMoveableMem::getsize(void)
{
	return real_size;
}

void OMediaMoveableMem::setsize(long newsize)
{
	HLOCAL hdl = ((OMediaWinRtgMoveMem*)retarget)->memblock;

	hdl = LocalReAlloc(hdl,newsize,LMEM_MOVEABLE);
	if (!hdl) omd_EXCEPTION(omcerr_OutOfMemory);

	real_size = newsize;
}
	
void *OMediaMoveableMem::lock(void)
{
	HLOCAL hdl = ((OMediaWinRtgMoveMem*)retarget)->memblock;

	lock_count++;
	
	return LocalLock(hdl);
}

void OMediaMoveableMem::unlock(void)
{
	lock_count--;
	LocalUnlock(((OMediaWinRtgMoveMem*)retarget)->memblock);
}

void OMediaMoveableMem::init_retarget(void)
{
	retarget = new OMediaWinRtgMoveMem;
	if (!retarget) omd_EXCEPTION(omcerr_OutOfMemory);
}

