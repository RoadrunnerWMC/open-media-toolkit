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

#include "OMediaString.h"

#include <stdio.h>
 
#ifndef omd_OBSOLETE_STRING_CLASS

long OMediaStringTools::string2long(const string &str)
{
	long	l;
	sscanf(str.c_str(),"%ld",&l);
	return l;
}


double OMediaStringTools::string2double(const string &str)
{
	char	*p;

	return strtod(str.c_str(), &p);
}

const string &OMediaStringTools::long2string(const long l)
{	
	char	cstr[64];
	static string str; 

	sprintf(cstr,"%ld",l);

	str = cstr; 
	return str;
}

const string &OMediaStringTools::double2string(const double d) 
{
	char	cstr[64];
	static string str; 

	sprintf(cstr,"%g",d);
	
	str = cstr;
	return str;
}


#else

void OMediaString::reserve(short l) 
{
	if (!str)
	{
		if (l) 
		{
			str = new char[l];
			str[0] = 0;
			res = l;
		}
	}
	else if (l>res) 
	{
		char *t = new char[l]; 
		strcpy(t,str);
		delete str;
		str = t;
		res = l;
	}
}


int OMediaString::find(const char *fstr, long len) const
{	
	int res,s,i;
	
	if (!str) return -1;

	if (len==-1) len = strlen(fstr);

	for(s=0; s<length(); s++)
	{
		res = 0;
		for(i=0;  ((i+s)<length() && i<len ); i++,res++)
		{
			if (fstr[i] != str[s+i]) break;
		}
		
		if (res==len) return s;
	}
	
	return -1;
}

void OMediaString::assign(const long l)
{
	reserve(64);
	sprintf(str,"%ld",l);
	len = strlen(str);
}

void OMediaString::assign(const double d)
{
	reserve(64);
	sprintf(str,"%f",d);
	len = strlen(str);
}

void OMediaString::assign(const char c)
{
	reserve(2);
	str[0] = c;
	str[1] = 0;
	len = 1;
}

void OMediaString::append(const long l)
{
	char	cstr[64];

	sprintf(cstr,"%ld",l);
	append(cstr);
}

void OMediaString::append(const double d)
{
	char	cstr[64];

	sprintf(cstr,"%f",d);
	append(cstr);
}

void OMediaString::append(const char c)
{
	reserve(len+2);
	str[len] = c;
	str[len+1] = 0;
	len++;
}

long OMediaString::get_long(void) const
{
	long	l;

	if (len==0) return 0;
	
	sscanf(str,"%ld",&l);
	return l;
}

double OMediaString::get_double(void) const
{
	double	d;

	if (len==0) return 0;
	
	sscanf(str,"%f",&d);
	return d;
}

#endif

