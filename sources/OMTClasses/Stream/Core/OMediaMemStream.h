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


 
#ifndef OMediaMemStream_H
#define OMediaMemStream_H

#include "OMediaStream.h"
#include "OMediaMoveableMem.h"


class OMediaMemStream : public OMediaStream
{
	public:


	omtshared OMediaMemStream(void);
	omtshared ~OMediaMemStream(void);

	// * Memory block (low level). Please that if stream is buffered the memblock
	// could not contain last changes. So call flush_buffer before trying to read
	// memory block.
	// By the way it does not make a lot of sens to buffer a memory block.
	
	inline OMediaMoveableMem *get_memory_block(void) {return &memblock;}

	protected:

	OMediaMoveableMem		memblock;
	long					position;

	omtshared virtual unsigned long do_getsize(void);
	omtshared virtual void do_setsize(unsigned long newsize);

	omtshared virtual long do_getposition(void);
	omtshared virtual void do_setposition(long offset, short relative);

	omtshared virtual void do_read(void *buffer, unsigned long nbytes);
	omtshared virtual void do_write(void *buffer, unsigned long nbytes);
};


#endif
