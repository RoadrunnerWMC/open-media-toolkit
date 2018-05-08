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
 
#include "OMediaRetarget.h"
#include "OMediaFilePath.h"
#include "OMediaMacRtgFilePath.h"
#include "OMediaError.h"
#include "OMediaTypes.h"

static bool makeFSSpecFromPath(FSSpec *myFSSpecPtr, const char *pstr)
{
#ifdef __MWERKS__

	unsigned char	upstr[256];
	
	c2pstrcpy(upstr,(char*)pstr);

  	return FSMakeFSSpec(0, 0, upstr,myFSSpecPtr) == noErr;

#else

    FSRef myFSRef;
    OSStatus status = FSPathMakeRef ((const UInt8*)pstr,
                                        &myFSRef, 
                                        NULL);            
    if (status == noErr)
        status = FSGetCatalogInfo (&myFSRef, 
                                kFSCatInfoNone, 
                                NULL, 
                                NULL,
                                myFSSpecPtr, 
                                NULL);     
    return status == noErr;

#endif 
}


OMediaFilePath::OMediaFilePath(void)
{
	retarget = new OMediaMacRtgFilePath;
	if (!retarget) omd_EXCEPTION(omcerr_OutOfMemory);
}

OMediaFilePath::OMediaFilePath(string name, omt_AppType appt, omt_FileType filet)
{
	retarget = new OMediaMacRtgFilePath;
	if (!retarget) omd_EXCEPTION(omcerr_OutOfMemory);

	setpath(name, appt, filet);
}

OMediaFilePath::OMediaFilePath(string name, const OMediaFilePath *base_path,omt_AppType appt, omt_FileType filet)
{
	retarget = new OMediaMacRtgFilePath;
	if (!retarget) omd_EXCEPTION(omcerr_OutOfMemory);

	setpath(name, base_path, appt, filet);
}


OMediaFilePath::~OMediaFilePath(void)
{
	delete retarget;
}



void OMediaFilePath::setpath(string filename, omt_AppType appt, omt_FileType filet)
{
    #define N_MAXPATH 1024
    static bool applicationPathFound;
    static char	applicationPath[N_MAXPATH];

    app_type = appt;
    file_type =filet;

#ifndef __MWERKS__
    if (!applicationPathFound)
    {
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        CFURLRef bundleURL = CFBundleCopyBundleURL( mainBundle );
        FSRef bundleFSRef;
            
        CFURLGetFSRef( bundleURL, &bundleFSRef );
        FSRefMakePath( &bundleFSRef, (unsigned char*)applicationPath, N_MAXPATH );
        applicationPathFound = true;  
    }
    
    if (filename.size()>0 && filename[0]!=':'
        && filename[0]!='/')    
    {
        string str;

        ((OMediaMacRtgFilePath*)retarget)->tryOSXResource = true;
        
        str = applicationPath;
        str += "/Contents/Resources/";
        str += filename;
        makeFSSpecFromPath(&(((OMediaMacRtgFilePath*)retarget)->osxResFsspec),str.c_str());
    
        str = applicationPath;
        str += "/../";
        str += filename;
        filename = str;
    }
    
#endif
        
    makeFSSpecFromPath(&(((OMediaMacRtgFilePath*)retarget)->fsspec),filename.c_str());
}

void OMediaFilePath::set_default_preference_path(string filename, omt_AppType appt, omt_FileType filet)
{
        ((OMediaMacRtgFilePath*)retarget)->tryOSXResource = false;
        
	app_type = appt;
	file_type =filet;

	short	refNum;
	long	dirID;
	OSErr	err;

	err = ::FindFolder(kOnSystemDisk,
					 kPreferencesFolderType,
					 kCreateFolder,
					 &refNum,
					 &dirID);

	if (err!=noErr) omd_OSEXCEPTION(err);

	unsigned char	pstr[256];
	
	c2pstrcpy(pstr,(char*)filename.c_str());

	FSMakeFSSpec(refNum, dirID, pstr,
  						&(((OMediaMacRtgFilePath*)retarget)->fsspec));
}


void OMediaFilePath::setpath(const OMediaFilePath *newpath)
{
	*((OMediaMacRtgFilePath*)retarget) = *((OMediaMacRtgFilePath*)(newpath->getretarget()));

	app_type = newpath->getapptype();
	file_type = newpath->getfiletype();
}

void OMediaFilePath::setpath(string filename, const OMediaFilePath *base_path,omt_AppType appt, omt_FileType filet)
{
	long	l;

        ((OMediaMacRtgFilePath*)retarget)->tryOSXResource = false;

	app_type = appt;
	file_type =filet;

	*((OMediaMacRtgFilePath*)retarget) = *((OMediaMacRtgFilePath*)(base_path->getretarget()));

	l = filename.length();
	if (l>63) l = 63;
	BlockMove(filename.c_str(),((OMediaMacRtgFilePath*)retarget)->fsspec.name+1,l);
	((OMediaMacRtgFilePath*)retarget)->fsspec.name[0] = l;	
}

void OMediaFilePath::get_filename(string &str)
{
	char	cstr[256];

	BlockMove(((OMediaMacRtgFilePath*)retarget)->fsspec.name+1,
				cstr,
				((OMediaMacRtgFilePath*)retarget)->fsspec.name[0]);

	cstr[((OMediaMacRtgFilePath*)retarget)->fsspec.name[0]] = 0;

	str = cstr;
}

bool OMediaFilePath::ask_user(OMediaWindow *win, OMediaAskUserFilters &filterobj, bool save_mode, string title, string prompt, string defaultname)
{
    NavDialogOptions    dialogOptions;
    OSErr               anErr = noErr;
    bool				result = false;
    
    anErr = NavGetDefaultDialogOptions(&dialogOptions);
    if (anErr == noErr)
    {
    	NavTypeListHandle		type_handle;
    	NavTypeList				*type_list;
    	
    	if (filterobj.filters.size()!=0 && !save_mode)
    	{
	    	type_handle = (NavTypeListHandle) 
	    		NewHandle(sizeof(NavTypeList)+ (sizeof(OSType)*filterobj.filters.size()) );

		    HLock((Handle)type_handle);
	    	type_list = *type_handle;
	    	type_list->componentSignature = kNavGenericSignature; 
	    	type_list->osTypeCount = filterobj.filters.size();
	    	for(long i=0;i<(long)filterobj.filters.size();i++)
	    	{
	    		type_list->osType[i] = filterobj.filters[i].file_type;	    	
	    	}	    
	    	
		    HUnlock((Handle)type_handle);
		}
	    else
	    	type_handle = NULL;
	    	
	 	dialogOptions.dialogOptionFlags &=~(kNavAllowMultipleFiles);
	 	dialogOptions.dialogOptionFlags |= kNavNoTypePopup;
	 	if (save_mode) c2pstrcpy(dialogOptions.savedFileName,defaultname.c_str());
	 	
	 	c2pstrcpy(dialogOptions.windowTitle,title.c_str());
   
        NavReplyRecord 		reply;
       
        if (save_mode)
        	anErr = NavPutFile (NULL,&reply,&dialogOptions,NULL,file_type,app_type,NULL);
        
        else
	        anErr = NavGetFile (NULL, &reply, &dialogOptions,
                           NULL, nil, NULL,
                           type_handle, nil);
          
                           
       if (anErr == noErr && reply.validRecord)
       {
          
			long    count;
           
           	anErr = AECountItems(&(reply.selection), &count);
           
           	// Set up index for file list
           	if (anErr == noErr)
           	{
               	long index = 1;
               
              	AEKeyword   theKeyword;
              	DescType    actualType;
              	Size        actualSize;
              	FSSpec      documentFSSpec;
              
              	// Get a pointer to selected file
              	anErr = AEGetNthPtr(&(reply.selection), index,
                	                  typeFSS, &theKeyword,
                    	              &actualType,&documentFSSpec,
                        	          sizeof(documentFSSpec),
                            	      &actualSize);
              	if (anErr == noErr)
              	{
                        ((OMediaMacRtgFilePath*)retarget)->tryOSXResource = false;
              		((OMediaMacRtgFilePath*)retarget)->fsspec = documentFSSpec;
                  	result = true;
              	}
           }
           
           //  Dispose of NavReplyRecord, resources, descriptors
           anErr = NavDisposeReply(&reply);
       }
                  
 		if (type_handle != NULL) DisposeHandle((Handle)type_handle);
 
    }

    return result;
}

