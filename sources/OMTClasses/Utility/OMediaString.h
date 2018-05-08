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
#ifndef OMEDIA_String_H
#define OMEDIA_String_H

#include "OMediaTypes.h"
#include "OMediaClassStreamer.h"
#include "OMediaStreamOperators.h"

#include <string>
#define OMediaString string

#ifndef unichar
typedef unsigned short unichar;
#endif

// Basic Unicode support

class OMediaUniString : public OMediaClassStreamer
{
    public:
    
    // Constructor
    
    OMediaUniString()
    {
        length = 0;
        ustring = NULL;
    }

    OMediaUniString(const OMediaUniString &str)
    {
        length = 0;
        ustring = NULL;
        assign(str);
    }

    OMediaUniString(const char *cstr)
    {
        length = 0;
        ustring = NULL;
        assign(cstr);
    }

    OMediaUniString(const string cstr)
    {
        length = 0;
        ustring = NULL;
        assign(cstr);
    }
    
    virtual ~OMediaUniString()
    {
        delete [] ustring;
    }

    // Stream

    virtual void read_class(OMediaStreamOperators &stream)
    {
        delete [] ustring;
        ustring = NULL;
        
        stream>>length;
        
        if (length) 
        {
            ustring = new unichar[length];
            stream.read(ustring,length*sizeof(unichar));
        }
    }
    
    virtual void write_class(OMediaStreamOperators &stream)
    {
        stream<<length;
        if (length) stream.write(ustring,length*sizeof(unichar));
    }
    
    // Assign
    
    void assign(const OMediaUniString &str)
    {
        if (length!=str.get_length())
        {
            delete []ustring;
            ustring = NULL;
            length = str.get_length();
            if (length) ustring = new unichar[length];
        }
        
        memcpy(ustring,str.get_char_constptr(),length*sizeof(unichar));
    }

    void assign(const string cstr)
    {
        assign(cstr.c_str());
    }

    void assign(const char *cstr)
    {
        int i,l=strlen(cstr);
        set_length(l);
        for(i=0;i<l;i++) ustring[i] = (unichar)cstr[i];
    }

    
    OMediaUniString &operator =(const OMediaUniString&str)
    {
        assign(str);
        return *this;
    }

    OMediaUniString &operator =(const char *str)
    {
        assign(str);
        return *this;
    }

    OMediaUniString &operator =(const string &str)
    {
        assign(str);
        return *this;
    }
    
    // Smaller
    
    bool isSmaller(const OMediaUniString &ustr) const
    {
        int minl = min(get_length(),ustr.get_length());
        const unichar *a = get_char_constptr();
        const unichar *b = ustr.get_char_constptr();

        for(int i=0;i<minl;i++,a++,b++)
        {
            if (*a<*b) return true;
            else if (*a>*b) return false;
        }
        
        return (get_length()<ustr.get_length());
    }

    bool operator<(const OMediaUniString &ustr) const
    {
        return isSmaller(ustr);
    }
    
    // Equal
    
    bool isEqual(const OMediaUniString &ustr) const
    {
        if (length!=ustr.get_length()) return false;
        for(int i=0;i<length;i++) if (ustring[i]!=(ustr.get_char_constptr())[i]) return false;
        return true;
    }

    bool operator ==(const OMediaUniString &ustr) const
    {
        return isEqual(ustr);
    }

    bool operator !=(const OMediaUniString &ustr) const
    {
        return !isEqual(ustr);
    }    
    
    // Length
    
    void set_length(const int nlength)
    {
        if (nlength==0)
        {
            delete [] ustring;
            ustring = NULL;
            length = 0;
            return;
        }
    
        unichar	*np = new unichar[nlength];
        
        if (length!=0)
        {
            memcpy(np,ustring,((length>nlength)?nlength:length)*sizeof(unichar));
            delete [] ustring;
        }
        
        length = nlength;
        ustring = np;
    }
    
    int get_length(void) const {return length;}
    
    // Access
    
    unichar *get_char_ptr() {return ustring;} 
    const unichar *get_char_constptr() const {return ustring;} 

    
    protected:
    
    int		length;
    unichar	*ustring;
};

// Basic String tools

class OMediaStringTools
{
	public:

	omtshared static long string2long(const string &str);
	omtshared static double string2double(const string &str);

	omtshared static const string &long2string(const long l);
	omtshared static const string &double2string(const double d);
};

#define omd_L2STR(x) OMediaStringTools::long2string(x)
#define omd_D2STR(x) OMediaStringTools::double2string(x)
#define omd_STR2L(x) OMediaStringTools::string2long(x)
#define omd_STR2D(x) OMediaStringTools::string2double(x)

#endif

