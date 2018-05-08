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
 
 
#include "OMediaMacResStream.h"
#include "OMediaError.h"

OMediaMacResStream::OMediaMacResStream(void)
{
	isopenflag = false;
}

OMediaMacResStream::~OMediaMacResStream(void)
{
	close();
}

unsigned long OMediaMacResStream::getsize(void)
{
	long siz;

	if (isopenflag) 
	{
		siz = GetHandleSize(reshdl);
		
		if (MemError()!=noErr) omd_OSEXCEPTION(MemError());
	}
	else siz = 0;
	
	return siz;
}

void OMediaMacResStream::setsize(unsigned long newsize)
{
	if (isopenflag) 
	{
		SetHandleSize(reshdl,newsize);
		
		if (MemError()!=noErr) omd_OSEXCEPTION(MemError());
	}
}
							  
void OMediaMacResStream::create(ResType type, short resid, const string *name, bool overwrite)
{
	Handle newhdl;
	Str255 pname;
	short l;

	close();
	
	if (overwrite) kill(type, resid);
	
	if (GetResource(type,resid)) return;	

	if (!name) pname[0] = 0;
	else 
	{
		l = name->length();
		BlockMove(name->c_str(),pname+1,l);
		pname[0] = l;
	}

	newhdl = NewHandle(0);
	AddResource(newhdl,type,resid,pname);

	if (ResError()!=noErr) omd_OSEXCEPTION(ResError());
}

void OMediaMacResStream::open(ResType type, short resid)
{
	OSErr err;

	close();

	reshdl = GetResource(type,resid);
	if (!reshdl) omd_OSEXCEPTION(resNotFound);

	err = ResError();
	if (err==noErr) 
	{
		isopenflag = true;
		HNoPurge(reshdl);
		dirty = false;
		position = 0;
		
		opentype = type;
		openresid = resid;
	}
	else if (ResError()!=noErr) omd_OSEXCEPTION(err);
}

void OMediaMacResStream::close(void)
{
	if (isopenflag)
	{
		if (dirty)
		{
			ChangedResource(reshdl);			
			if (ResError()==noErr) 
			{
				WriteResource(reshdl);				
				if (ResError()!=noErr) omd_OSEXCEPTION(ResError());
			}
			else omd_OSEXCEPTION(ResError());
		}
		
		HPurge(reshdl);
		ReleaseResource(reshdl);		
		isopenflag = false;
	}
}

void OMediaMacResStream::kill(ResType type, short resid)
{
	Handle hdl;
	OSErr  err;

	if (isopenflag && opentype==type && openresid==resid) close();

	hdl = GetResource(type,resid);
	if (!hdl) omd_OSEXCEPTION(resNotFound);
	err = ResError();
	if (err!=noErr) omd_OSEXCEPTION(err);
	
	RemoveResource(hdl);
	err = ResError();
	if (err!=noErr) omd_OSEXCEPTION(err);

	DisposeHandle(hdl);
	
}

long OMediaMacResStream::getposition(void)
{
	if (isopenflag) return position;
	else return 0;
}

void OMediaMacResStream::setposition(long offset, short relative)
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

void OMediaMacResStream::read(void *buffer, unsigned long nbytes)
{
	if (isopenflag)
	{
		unsigned long siz = getsize();

		if (position+nbytes>siz) nbytes -= (position+nbytes)-siz;
	
		HLock(reshdl);
		BlockMove(((char*)*reshdl)+position,buffer,nbytes);
		HUnlock(reshdl);
		
		position += nbytes;
	}
}

void OMediaMacResStream::write(void *buffer, unsigned long nbytes)
{
	if (isopenflag)
	{
		unsigned long siz = getsize();

		if (position+nbytes>siz) 
		{
			setsize(position+nbytes);
			siz = getsize();
			if (position+nbytes>siz) nbytes -= (position+nbytes)-siz;
		}
	
		HLock(reshdl);
		BlockMove(buffer,((char*)*reshdl)+position,nbytes);
		HUnlock(reshdl);
		
		position += nbytes;
		
		dirty = true;
	}	
}



