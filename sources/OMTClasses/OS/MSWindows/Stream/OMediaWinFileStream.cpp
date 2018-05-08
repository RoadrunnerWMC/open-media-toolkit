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

 
#include "OMediaFileStream.h"
#include "OMediaWinRtgFilePath.h"
#include "OMediaError.h"


//---------------------------------------------------

OMediaFileStream::OMediaFileStream(void)
{
	isopenflag = false;	
	position = 0;
}
 
//---------------------------------------------------

OMediaFileStream::~OMediaFileStream(void)
{
	close();
}

//---------------------------------------------------

unsigned long OMediaFileStream::do_getsize(void)
{
	OMediaWinRtgFilePath	*path = (OMediaWinRtgFilePath*) filepath.getretarget();
	long siz;

	if (isopenflag) 
	{
		siz =  GetFileSize(path->file,(LPDWORD)NULL);
	
		if (siz==0xFFFFFFFF) omd_OSEXCEPTION(GetLastError());
	}
	else siz = 0;
	
	return siz;
}

//---------------------------------------------------

void OMediaFileStream::do_setsize(unsigned long newsize)
{
	OMediaWinRtgFilePath	*path = (OMediaWinRtgFilePath*) filepath.getretarget();

	if (isopenflag) 
	{
		do_setposition(newsize,omcfr_Start);
		if (!SetEndOfFile(path->file)) omd_OSEXCEPTION(GetLastError());
	}
}
			  

//---------------------------------------------------

void OMediaFileStream::open(omt_FilePermission perm,
							bool truncate, bool createifnofile)
{
	OMediaWinRtgFilePath	*path = (OMediaWinRtgFilePath*) filepath.getretarget();
	DWORD				 	osperm;
	DWORD					createflag;

	close();

	switch(perm)
	{
		case omcfp_Write:		osperm = GENERIC_WRITE; break;
		case omcfp_Read:		osperm = GENERIC_READ; break;
		case omcfp_ReadWrite:	osperm = GENERIC_WRITE|GENERIC_READ; break;
	}
	
	if (truncate) createflag = CREATE_ALWAYS;
	else if (createifnofile) createflag = OPEN_ALWAYS;
	else createflag = OPEN_EXISTING;

	path->file = CreateFile(path->path.c_str(),
							osperm,
							FILE_SHARE_READ,
							(LPSECURITY_ATTRIBUTES)NULL,
							createflag,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
							
	if (path->file == INVALID_HANDLE_VALUE) omd_OSEXCEPTION(GetLastError());

	isopenflag = true;
	position = 0;

	set_buffer_size(2*1024);
}

//---------------------------------------------------

void OMediaFileStream::close(void)
{
	OMediaWinRtgFilePath	*path = (OMediaWinRtgFilePath*) filepath.getretarget();

	if (isopenflag)
	{
		flush_buffer();
	
		if (!CloseHandle(path->file))  omd_OSEXCEPTION(GetLastError());		
		isopenflag = false;
		
		set_buffer_size(0);
	}
}


//---------------------------------------------------

bool OMediaFileStream::fileexists(void)
{
	OMediaWinRtgFilePath	*path = (OMediaWinRtgFilePath*) filepath.getretarget();

	if (isopenflag || GetFileAttributes(path->path.c_str())!=0xFFFFFFFF) return true;
	else return false;
}

//---------------------------------------------------

void OMediaFileStream::do_read(void *buffer, unsigned long nbytes)
{
	unsigned long total = nbytes;
	OMediaWinRtgFilePath	*path = (OMediaWinRtgFilePath*) filepath.getretarget();

	if (isopenflag) 
	{
		if (!ReadFile(path->file,buffer,total,&total,(LPOVERLAPPED)NULL))  omd_OSEXCEPTION(GetLastError());	
		position += nbytes;	
	}	
}

//---------------------------------------------------

void OMediaFileStream::do_write(void *buffer, unsigned long nbytes)
{
	unsigned long total = nbytes;
	OMediaWinRtgFilePath	*path = (OMediaWinRtgFilePath*) filepath.getretarget();

	if (isopenflag) 
	{
		if (!WriteFile(path->file,buffer,total,&total,(LPOVERLAPPED)NULL))  omd_OSEXCEPTION(GetLastError());	
		position += nbytes;
	}	
}

//---------------------------------------------------

void OMediaFileStream::kill(void)
{
	OMediaWinRtgFilePath	*path = (OMediaWinRtgFilePath*) filepath.getretarget();

	close();

	if (!DeleteFile(path->path.c_str()))  omd_OSEXCEPTION(GetLastError());	
}


//---------------------------------------------------

void OMediaFileStream::do_setposition(long offset, short omtrelative)
{
	OMediaWinRtgFilePath	*path = (OMediaWinRtgFilePath*) filepath.getretarget();
	DWORD	relative;

	switch(omtrelative)
	{
		case omcfr_Start:		relative = FILE_BEGIN; break;
		case omcfr_End:			relative = FILE_END; break;
		case omcfr_Current:		relative = FILE_CURRENT; break;
	}

	position = SetFilePointer(path->file,offset,(PLONG)NULL,relative);
	if (position==0xFFFFFFFF) omd_OSEXCEPTION(GetLastError());	
}

//---------------------------------------------------

long OMediaFileStream::do_getposition(void)
{
	return position;
}

//---------------------------------------------------

void OMediaFileStream::setpath(const OMediaFilePath *p)
{
	filepath.setpath(p);
}

//---------------------------------------------------

bool OMediaFileStream::getpath(OMediaFilePath *p) const
{
	p->setpath(&filepath);
	return true;
}
