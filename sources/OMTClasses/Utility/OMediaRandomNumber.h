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
#ifndef OMEDIA_RandomNumber_H
#define OMEDIA_RandomNumber_H

#include "OMediaTypes.h"

class OMediaRandomNumber
{
	public:
    
	OMediaRandomNumber() : seed(314159265) {}
    OMediaRandomNumber(unsigned long s) : seed(s) {}
        
    inline unsigned short random() { return rand() >> 16;}
    inline bool coin(){ return (rand() >> 30) != 0;}

	inline unsigned short range(unsigned short min, unsigned short max)
	{
		return (( (long(max-min)+1)*random())>>16L)+min;
	}

	inline void set_seed(unsigned long l) {seed = l;}
	inline unsigned long get_seed(void) const {return seed;}

	protected:

    unsigned long seed;
    inline unsigned long rand() { return seed = 1664525*seed + 32767;}

};

#endif
