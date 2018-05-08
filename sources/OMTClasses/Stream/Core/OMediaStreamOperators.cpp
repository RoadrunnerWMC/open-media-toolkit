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
 

#include "OMediaStreamOperators.h"
#include "OMediaMemTools.h"
#include "OMediaEndianSupport.h"
#include "OMediaClassStreamer.h"

#include <string.h>



OMediaStreamOperators::OMediaStreamOperators(void) 
{
	#ifdef omd_LITTLE_ENDIAN
	reverse_bytes =  true;
	#else
	reverse_bytes =  false;
	#endif

	db_streaming_object = NULL;
}	

OMediaStreamOperators::~OMediaStreamOperators(void) {}

void OMediaStreamOperators::read(void *buffer, unsigned long nbytes) {}
void OMediaStreamOperators::write(void *buffer, unsigned long nbytes) {}

OMediaDataBase *OMediaStreamOperators::get_database(void)
{
	return NULL;
}

bool OMediaStreamOperators::getpath(OMediaFilePath *) const
{
	return false;
}

//----------------------------------------------------------

// ** Input:

OMediaStreamOperators& OMediaStreamOperators::operator>>(string&p)
{
    unsigned char	c1,c2;

    *this>>c1;
    *this>>c2;
    if (c1==0xFF && c2==0xFF)
    {
        long 	length;
        char	*buf;
        
        *this>>length;
        
        buf = new char[length+1];
        read(buf,length);
        buf[length] = 0;

        p = buf;
        delete [] buf;
    }
    else // Backward compatibility:
    {
        char	cstr[256];
        
        cstr[0] = c2;
    
	read(cstr+1,c1-1);
	cstr[c1] = 0;
        
	p = cstr;
    }
		
    return *this;
}


OMediaStreamOperators& OMediaStreamOperators::operator>>(string*p)
{
	return *this>>*p;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(char *p)
{
	unsigned char	siz;

	read(&siz,1);
	read(p,siz);
	p[siz] = 0;
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(unsigned char *p)
{
	return *this>>(char*)p;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(char&p) {read(&p,sizeof(p)); return *this;}
OMediaStreamOperators& OMediaStreamOperators::operator>>(unsigned char& p)  {read(&p,sizeof(p)); return *this;}

OMediaStreamOperators& OMediaStreamOperators::operator>>(short&p)  
{
	read(&p,sizeof(p)); 
	if (reverse_bytes) p = omd_ReverseShort(p);
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(unsigned short&p)  
{
	read(&p,sizeof(p)); 
	if (reverse_bytes) p = omd_ReverseShort(p);
	return *this;
}


OMediaStreamOperators& OMediaStreamOperators::operator>>(int&p) 
{
	read(&p,sizeof(p)); 
	if (reverse_bytes) omf_ReverseBuffer(&p,sizeof(p));
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(unsigned int&p) 
{
	read(&p,sizeof(p)); 
	if (reverse_bytes) omf_ReverseBuffer(&p,sizeof(p));
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(long&p) 
{
	read(&p,sizeof(p)); 
	if (reverse_bytes) p = omd_ReverseLong(p);
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(unsigned long&p) 
{
	read(&p,sizeof(p)); 
	if (reverse_bytes) p = omd_ReverseLong(p);
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(float&p)  
{
	read(&p,sizeof(p)); 
	if (reverse_bytes) omf_ReverseBuffer(&p,sizeof(p));
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(double&p)
{
	read(&p,sizeof(p)); 
	if (reverse_bytes) omf_ReverseBuffer(&p,sizeof(p));
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(bool&p)  
{
	short s;	// For historical reason, bool type is saved as a short.

	read(&s,2); 
	if (reverse_bytes) s = omd_ReverseShort(s);
	
	
	p = (s)?true:false;
	
	return *this;
}


// ** Output:

OMediaStreamOperators& OMediaStreamOperators::operator<<(string *p)
{
	return *this<<*p;	
}

OMediaStreamOperators& OMediaStreamOperators::operator<<(string &p)
{
    unsigned char c = 0xFF;
    
    (*this)<<c;	// Code to recognize long string
    (*this)<<c;

    long 	len = p.length();
    char	*buf;
        
    (*this)<<len;
        
    buf = new char[len];
    memcpy(buf,p.c_str(),len);
    write(buf,len);
    delete [] buf;
    	
    return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator<<(char *p)
{
	unsigned char	siz = strlen(p);

	write(&siz,1);
	write(p,siz);
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator<<(unsigned char *p)
{
	return *this<<(char*)p;
}

OMediaStreamOperators& OMediaStreamOperators::operator<<(char&p) {write(&p,sizeof(p)); return *this;}
OMediaStreamOperators& OMediaStreamOperators::operator<<(unsigned char&p)  {write(&p,sizeof(p)); return *this;}

OMediaStreamOperators& OMediaStreamOperators::operator<<(short&p)
{
	short t;
	t = (reverse_bytes)?omd_ReverseShort(p):p;

	write(&t,sizeof(p)); 
	return *this;
}


OMediaStreamOperators& OMediaStreamOperators::operator<<(unsigned short&p)  
{
	unsigned short t;
	t = (reverse_bytes)?omd_ReverseShort(p):p;

	write(&t,sizeof(p)); 
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator<<(int&p)  
{
	int		t = p;

	if (reverse_bytes) omf_ReverseBuffer(&t,sizeof(p));
	write(&t,sizeof(p)); 
	return *this;
}


OMediaStreamOperators& OMediaStreamOperators::operator<<(unsigned int&p)
{
	unsigned int		t = p;

	if (reverse_bytes) omf_ReverseBuffer(&t,sizeof(p));
	write(&t,sizeof(p)); 
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator<<(long&p)  
{
	long t;
	t = (reverse_bytes)?omd_ReverseLong(p):p;

	write(&t,sizeof(p)); 
	return *this;
}


OMediaStreamOperators& OMediaStreamOperators::operator<<(unsigned long&p)
{
	unsigned long t;
	t = (reverse_bytes)?omd_ReverseLong(p):p;

	write(&t,sizeof(p)); 
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator<<(float&p)
{
	float		t = p;

	if (reverse_bytes) omf_ReverseBuffer(&t,sizeof(p));
	write(&t,sizeof(p)); 
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator<<(double&p)
{
	double		t = p;

	if (reverse_bytes) omf_ReverseBuffer(&t,sizeof(p));
	write(&t,sizeof(p)); 
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator<<(bool&p)  
{
	short s;	// For historical reason, bool type is saved as a short.

	s = (short)p;

	if (reverse_bytes) s = omd_ReverseShort(s);
	write(&s,2); 
	
	return *this;
}

// Class streamer

OMediaStreamOperators& OMediaStreamOperators::operator<<(OMediaClassStreamer *p)
{
	p->write_class(*this);
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator<<(OMediaClassStreamer &p)
{
	p.write_class(*this);
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(OMediaClassStreamer *p)
{
	p->read_class(*this);
	return *this;
}

OMediaStreamOperators& OMediaStreamOperators::operator>>(OMediaClassStreamer &p)
{
	p.read_class(*this);
	return *this;
}

void OMediaStreamOperators::setposition(long offset, short relative) {}
long OMediaStreamOperators::getposition(void) {return 0;}


unsigned long OMediaStreamOperators::getsize(void) {return 0;}
void OMediaStreamOperators::setsize(unsigned long newsize) {}
