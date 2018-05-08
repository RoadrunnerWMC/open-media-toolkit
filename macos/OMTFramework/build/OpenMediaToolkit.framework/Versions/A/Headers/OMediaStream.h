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
#ifndef OMEDIA_Stream_H
#define OMEDIA_Stream_H

#include "OMediaTypes.h"
#include "OMediaStreamOperators.h"

class OMediaDataBase;

class OMediaStream : public OMediaStreamOperators
{
	public:



	omtshared OMediaStream(void);
	omtshared virtual ~OMediaStream(void);

	// * In/out

	omtshared virtual void read(void *buffer, unsigned long nbytes);
	omtshared virtual void write(void *buffer, unsigned long nbytes);

	// * Size

	omtshared virtual unsigned long getsize(void);
	omtshared virtual void setsize(unsigned long newsize);


	// * Position

	omtshared virtual void setposition(long offset, short relative = omcfr_Start);
	omtshared virtual long getposition(void);


	// * Buffered access (0 to disable it)
	
	omtshared virtual void set_buffer_size(long siz);
	inline long get_buffer_size(void) const {return buffer_size;}
	omtshared virtual void flush_buffer(void);


	protected:

	omtshared virtual void do_read(void *buffer, unsigned long nbytes);
	omtshared virtual void do_write(void *buffer, unsigned long nbytes);

	omtshared virtual void do_setposition(long offset, short relative = omcfr_Start);
	omtshared virtual long do_getposition(void);

	omtshared virtual unsigned long do_getsize(void);
	omtshared virtual void do_setsize(unsigned long newsize);

	omtshared virtual void stream2buffer(long pos, bool disable_read=false);

	friend class OMediaDataBase;


	char			*buffer;
	long			buffer_size,buffer_start,buffer_end,buffer_pointer;
	bool			buffer_dirty;
};



#endif

