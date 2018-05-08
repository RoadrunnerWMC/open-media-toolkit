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


 
 
#ifndef OMEDIA_MacResStream_H
#define OMEDIA_MacResStream_H

#include "OMediaStream.h"
#include "OMediaTypes.h"

#include <string>

class OMediaMacResStream : public OMediaStream
{
	public:


	OMediaMacResStream(void);
	~OMediaMacResStream(void);


	// * Size

	virtual unsigned long getsize(void);
	virtual void setsize(unsigned long newsize);
							  

	// * Managing

	virtual void create(ResType type, short resid,  const string *name, bool overwrite =false);

	virtual void open(ResType type, short resid);
	virtual void close(void);


	inline Boolean isopen(void) const {return isopenflag;}
	
	virtual void kill(ResType type, short resid);


	// * Position (override)

	virtual long getposition(void);
	virtual void setposition(long offset, short relative =omcfr_Start);


	// * In/out (override)

	virtual void read(void *buffer, unsigned long nbytes);
	virtual void write(void *buffer, unsigned long nbytes);



	//***********************************************************
	// LOW LEVEL

	protected:
	
	bool		isopenflag, dirty;

	long			position;
	Handle			reshdl;

	ResType 		opentype;
	short		openresid;
};



#endif