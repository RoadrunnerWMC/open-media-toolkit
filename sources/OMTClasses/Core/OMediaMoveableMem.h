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


 

#ifndef OMEDIA_MoveableMem_H
#define OMEDIA_MoveableMem_H

#include "OMediaTypes.h"

class OMediaRetarget;

class OMediaMoveableMem
{
	public:
	
	omtshared OMediaMoveableMem(long	blocksize =0);
	omtshared virtual ~OMediaMoveableMem(void);
	
	omtshared virtual long getsize(void);
	omtshared virtual void setsize(long newsize);
	
	omtshared virtual void *lock(void);	// Locks and returns a pointer on the memory block
	omtshared virtual void unlock(void);

	inline bool is_locked(void) const {return (lock_count)?true:false;}

	inline OMediaRetarget	*get_retarget(void) {return retarget;}

	protected:

	long		lock_count;

	omtshared virtual void init_retarget(void);
	OMediaRetarget	*retarget;

	private:
	long		real_size;

};


#endif

