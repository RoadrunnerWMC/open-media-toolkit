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
#ifndef OMEDIA_StreamOperators_H
#define OMEDIA_StreamOperators_H

#include "OMediaTypes.h"
#include <string>

enum
{
	omcfr_Start,
	omcfr_End,
	omcfr_Current
};

class OMediaClassStreamer;
class OMediaDataBase;
class OMediaDBObject;
class OMediaFilePath;

class OMediaStreamOperators
{
	public:


	omtshared OMediaStreamOperators(void);
	omtshared virtual ~OMediaStreamOperators(void);


	// * In/out
	
	omtshared virtual void read(void *buffer, unsigned long nbytes);
	omtshared virtual void write(void *buffer, unsigned long nbytes);


	// * C++ streaming operators (binary oriented)
	
	omtshared OMediaStreamOperators& operator>>(string*);
	omtshared OMediaStreamOperators& operator>>(string&);
	omtshared OMediaStreamOperators& operator>>(char *);
	omtshared OMediaStreamOperators& operator>>(unsigned char *);
	omtshared OMediaStreamOperators& operator>>(char&);
	omtshared OMediaStreamOperators& operator>>(unsigned char& );
	omtshared OMediaStreamOperators& operator>>(short&);
	omtshared OMediaStreamOperators& operator>>(unsigned short&);
	omtshared OMediaStreamOperators& operator>>(int&);
	omtshared OMediaStreamOperators& operator>>(unsigned int&);
	omtshared OMediaStreamOperators& operator>>(long&);
	omtshared OMediaStreamOperators& operator>>(unsigned long&);
	omtshared OMediaStreamOperators& operator>>(float&);
	omtshared OMediaStreamOperators& operator>>(double&);
	omtshared OMediaStreamOperators& operator>>(bool&);
	omtshared OMediaStreamOperators& operator>>(OMediaClassStreamer&);
	omtshared OMediaStreamOperators& operator>>(OMediaClassStreamer*);

	omtshared OMediaStreamOperators& operator<<(string*);
	omtshared OMediaStreamOperators& operator<<(string&);
	omtshared OMediaStreamOperators& operator<<(char *);
	omtshared OMediaStreamOperators& operator<<(unsigned char *);
	omtshared OMediaStreamOperators& operator<<(char&);
	omtshared OMediaStreamOperators& operator<<(unsigned char&);
	omtshared OMediaStreamOperators& operator<<(short&);
	omtshared OMediaStreamOperators& operator<<(unsigned short&);
	omtshared OMediaStreamOperators& operator<<(int&);
	omtshared OMediaStreamOperators& operator<<(unsigned int&);
	omtshared OMediaStreamOperators& operator<<(long&);
	omtshared OMediaStreamOperators& operator<<(unsigned long&);
	omtshared OMediaStreamOperators& operator<<(float&);
	omtshared OMediaStreamOperators& operator<<(double&);
	omtshared OMediaStreamOperators& operator<<(bool&);
	omtshared OMediaStreamOperators& operator<<(OMediaClassStreamer&);
	omtshared OMediaStreamOperators& operator<<(OMediaClassStreamer*);


	// * Reverse byte order (Default: True if Intel processor)
	inline void set_reverse_bytes_order(bool i) {reverse_bytes = i;}
	inline bool get_reverse_bytes_order(void) const {return reverse_bytes;}


	// * Position

	omtshared virtual void setposition(long offset, short relative = omcfr_Start);
	omtshared virtual long getposition(void);

	inline void skip(unsigned long nbytes) {setposition(nbytes,omcfr_Current);}

	// * Size

	omtshared virtual unsigned long getsize(void);
	omtshared virtual void setsize(unsigned long newsize);

	// * Return database pointer if this stream is a database or NULL

	omtshared virtual OMediaDataBase *get_database(void);
	
	// * Return path if it has a path, else return false if not
	
	omtshared virtual bool getpath(OMediaFilePath *) const;

	// * A pointer to the object that is current being streamed

	inline OMediaDBObject *get_db_streaming_object(void)
	{
		return db_streaming_object;
	}
        
	protected:

	OMediaDBObject			*db_streaming_object;
	bool				reverse_bytes;
};



#endif

