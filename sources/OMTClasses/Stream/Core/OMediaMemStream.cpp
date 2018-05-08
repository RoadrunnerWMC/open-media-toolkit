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
 
 
 #include "OMediaMemStream.h"
 #include "OMediaMemTools.h"

//---------------------------------------------------

OMediaMemStream::OMediaMemStream() : memblock(0)
{
	position = 0;
}
 
//---------------------------------------------------

OMediaMemStream::~OMediaMemStream()
{
}

//---------------------------------------------------

unsigned long OMediaMemStream::do_getsize(void)
{
	return memblock.getsize();
}

//---------------------------------------------------

void OMediaMemStream::do_setsize(unsigned long newsize)
{
	memblock.setsize(newsize);
}
							  

//---------------------------------------------------

void OMediaMemStream::do_read(void *buffer, unsigned long nbytes)
{
	unsigned long siz = getsize();
	char *data;

	if (position+nbytes>siz) nbytes -= (position+nbytes)-siz;
	
	data = (char*)memblock.lock();
	OMediaMemTools::copy(data+position,buffer,nbytes);
	memblock.unlock();
		
	position += nbytes;	
}

//---------------------------------------------------

void OMediaMemStream::do_write(void *buffer, unsigned long nbytes)
{
	unsigned long siz = getsize();
	char *data;

	if (position+nbytes>siz) 
	{
		setsize(position+nbytes);
		siz = getsize();
		if (position+nbytes>siz) nbytes -= (position+nbytes)-siz;
	}
	
	data = (char*)memblock.lock();
	OMediaMemTools::copy(buffer,data+position,nbytes);
	memblock.unlock();
		
	position += nbytes;			
}


//---------------------------------------------------

void OMediaMemStream::do_setposition(long offset, short relative)
{
	long siz = getsize();

	switch(relative)
	{
		case omcfr_Start:		position = offset; break;
		case omcfr_End:			position = siz-offset; break;
		case omcfr_Current:		position += offset; break;
	}

	if (position<0) position = 0;
	else if (position>siz) position = siz;	
}

//---------------------------------------------------

long OMediaMemStream::do_getposition(void)
{
	return position;
}

//---------------------------------------------------
 