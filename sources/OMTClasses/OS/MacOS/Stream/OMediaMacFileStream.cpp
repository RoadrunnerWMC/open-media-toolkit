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
 #include "OMediaMacRtgFilePath.h"
 #include "OMediaError.h"


//---------------------------------------------------

OMediaFileStream::OMediaFileStream(void)
{
	isopenflag = false;	
	set_buffer_size(2*1024);
}
 
//---------------------------------------------------

OMediaFileStream::~OMediaFileStream(void)
{
	close();
}

//---------------------------------------------------

unsigned long OMediaFileStream::do_getsize(void)
{
	OMediaMacRtgFilePath	*path = (OMediaMacRtgFilePath*) filepath.getretarget();
	long siz;

	if (isopenflag) 
	{
		OSErr err;
	
		err = GetEOF(path->refNum,&siz);
		if (err!=noErr) omd_OSEXCEPTION(err);
	}
	else siz = 0;
	
	return siz;
}

//---------------------------------------------------

void OMediaFileStream::do_setsize(unsigned long newsize)
{
	OMediaMacRtgFilePath	*path = (OMediaMacRtgFilePath*) filepath.getretarget();

	if (isopenflag) 
	{
		OSErr err;
	
		err = SetEOF(path->refNum,newsize);		
		if (err!=noErr) omd_OSEXCEPTION(err);
	}
}
			  

//---------------------------------------------------

void OMediaFileStream::open(omt_FilePermission perm,
							bool truncate, bool createifnofile)
{
	OSErr err = noErr;
	OMediaMacRtgFilePath	*path = (OMediaMacRtgFilePath*) filepath.getretarget();
	char osperm;

	if (fileexists())
	{
		if (truncate && (perm!=omcfp_Read)) 
		{
			kill();
			err = FSpCreate(&path->fsspec,filepath.getapptype(),filepath.getfiletype(),path->scriptcode);
		}
	}
	else if (createifnofile && (perm!=omcfp_Read)) 
		err = FSpCreate(&path->fsspec, filepath.getapptype(), filepath.getfiletype(),path->scriptcode);

	if (err!=noErr) omd_OSEXCEPTION(err);

	close();

        FSSpec	*fsspec = NULL;

        if (!fileexists())
        {
            if (path->tryOSXResource)
            {
                fsspec = &path->osxResFsspec;
            }
        }
        else fsspec = &path->fsspec;

        if (!fsspec) omd_EXCEPTION(omcerr_CantOpen);

	switch(perm)
	{
		case omcfp_Write:		osperm = fsWrPerm; break;
		case omcfp_Read:		osperm = fsRdPerm; break;
		case omcfp_ReadWrite:	osperm = fsRdWrPerm; break;
	}

	err = FSpOpenDF(fsspec,osperm,&path->refNum);

	if (err==noErr) isopenflag = true;
	else  
        {
            omd_OSEXCEPTION(err);
        }
}

//---------------------------------------------------

void OMediaFileStream::close(void)
{
	OMediaMacRtgFilePath	*path = (OMediaMacRtgFilePath*) filepath.getretarget();

	if (isopenflag)
	{
		OSErr	err;
	
		flush_buffer();
	
		err = FSClose(path->refNum);
		if (err!=noErr) omd_OSEXCEPTION(err);		
		isopenflag = false;
	}
}


//---------------------------------------------------

bool OMediaFileStream::fileexists(void)
{
	FInfo	info;
	OMediaMacRtgFilePath	*path = (OMediaMacRtgFilePath*) filepath.getretarget();

	if (isopenflag || FSpGetFInfo(&path->fsspec,&info)==noErr) return true;
	else return false;
}

//---------------------------------------------------

void OMediaFileStream::do_read(void *buffer, unsigned long nbytes)
{
	long total = nbytes;
	OMediaMacRtgFilePath	*path = (OMediaMacRtgFilePath*) filepath.getretarget();

	if (isopenflag) 
	{
		OSErr	err;
	
		err = FSRead(path->refNum,&total,buffer);
		if (err!=noErr) omd_OSEXCEPTION(err);	
	}	
}

//---------------------------------------------------

void OMediaFileStream::do_write(void *buffer, unsigned long nbytes)
{
	long total = nbytes;
	OMediaMacRtgFilePath	*path = (OMediaMacRtgFilePath*) filepath.getretarget();

	if (isopenflag) 
	{
		OSErr	err;
	
		err = FSWrite(path->refNum,&total,buffer);
		
		if (err!=noErr) omd_OSEXCEPTION(err);	
	}	
}

//---------------------------------------------------

void OMediaFileStream::kill(void)
{
	OSErr	err;
	OMediaMacRtgFilePath	*path = (OMediaMacRtgFilePath*) filepath.getretarget();

	close();
	err =  FSpDelete(&path->fsspec);
	if (err!=noErr) omd_OSEXCEPTION(err);	
}


//---------------------------------------------------

void OMediaFileStream::do_setposition(long offset, short relative)
{
	OSErr	err;
	OMediaMacRtgFilePath	*path = (OMediaMacRtgFilePath*) filepath.getretarget();

	switch(relative)
	{
		case omcfr_Start:		relative = fsFromStart; break;
		case omcfr_End:			relative = fsFromLEOF; break;
		case omcfr_Current:		relative = fsFromMark; break;
	}
	
	err = SetFPos(path->refNum,relative,offset);
	
	if (err!=noErr)  omd_OSEXCEPTION(err);
}

//---------------------------------------------------

long OMediaFileStream::do_getposition(void)
{
	long pos;
	OSErr err;
	OMediaMacRtgFilePath	*path = (OMediaMacRtgFilePath*) filepath.getretarget();

	if (isopenflag) 
	{
		err = GetFPos(path->refNum,&pos);		
		if (err!=noErr) omd_OSEXCEPTION(err);
	}
	else pos = 0;

	return pos;
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

