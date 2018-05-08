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
 
 
#include "OMediaMacAppleEvent.h"
#include "OMediaError.h"
#include "OMediaSupervisor.h"
#include "OMediaMacRtgFilePath.h"
#include "OMediaFilePath.h"


OMediaAppleEvent *OMediaAppleEvent::globalptr;
 

//..............................
// AppleEvent handlers:


pascal OSErr OMediaAppleEvent::AEOpenHandler(const AppleEvent *messagein, AppleEvent *reply, omt_AppleEventRefVal refIn)
{
	OMediaAppleEvent::globalptr->ae_openapplication(messagein,reply, refIn);
	return noErr;
}
	
pascal OSErr OMediaAppleEvent::AEOpenDocHandler(const AppleEvent *messagein, AppleEvent *reply, omt_AppleEventRefVal refIn)
{
	OMediaAppleEvent::globalptr->ae_opendocument(messagein,reply, refIn);
	return noErr;
}

pascal OSErr OMediaAppleEvent::AEPrintHandler(const AppleEvent *messagein, AppleEvent *reply, omt_AppleEventRefVal refIn)
{                                                           /* no printing handler in yet, so we'll ignore this */
	OMediaAppleEvent::globalptr->ae_print(messagein,reply, refIn);
	return noErr;
}

pascal OSErr OMediaAppleEvent::AEQuitHandler(const AppleEvent *messagein, AppleEvent *reply, omt_AppleEventRefVal refIn)
{
	OMediaAppleEvent::globalptr->ae_quit(messagein,reply, refIn);
	return noErr;
}

// AppleEvent methods, OMT uses only the quit message

OSErr OMediaAppleEvent::ae_openapplication(const AppleEvent *messagein, AppleEvent *reply, long refIn)
{
	return noErr;
}
	
OSErr OMediaAppleEvent::ae_opendocument(const AppleEvent *messagein, AppleEvent *reply, long refIn)
{
 	OSErr err;
 	AEDescList doclist; 
 	AEKeyword keywd; 
 	DescType rttype;
 	long i, ndocs, size;
 	FSSpec fss;
 	vector<OMediaFilePath*>	filePathes;
	OMediaFilePath			*fpath;
 	
 	err = AEGetParamDesc(messagein, keyDirectObject, typeAEList, &doclist);
 	if (err) return noErr;
 	
 	err = AECountItems(&doclist, &ndocs);
	if (err) return noErr; 
	
	for(i = 1; i <= ndocs; i++) 
	{
		err = AEGetNthPtr(&doclist, i, typeFSS, &keywd, &rttype, &fss, sizeof(fss), &size); 
		if (err) break; 
		
		fpath = new OMediaFilePath;
		((OMediaMacRtgFilePath*)fpath->getretarget())->fsspec = fss;
		filePathes.push_back(fpath);
	}
	
	if (filePathes.size())
	{
		if (OMediaSupervisor::get_main_supervisor()) 
			OMediaSupervisor::get_main_supervisor()->listen_to_message(omsg_OpenFileList,&filePathes);
	
		for(vector<OMediaFilePath*>::iterator fi=filePathes.begin();
			fi!=filePathes.end();
			fi++)
		{
			delete (*fi);
		}
	}

	return noErr;
}

OSErr OMediaAppleEvent::ae_print(const AppleEvent *messagein, AppleEvent *reply, long refIn)
{                                            
	return noErr;
}

OSErr OMediaAppleEvent::ae_quit(const AppleEvent *messagein, AppleEvent *reply, long refIn)
{
	if (OMediaSupervisor::get_main_supervisor()) 
		OMediaSupervisor::get_main_supervisor()->listen_to_message(omsg_Quit);
	
	return noErr;
}

const short omediatotal_Handlers = 4;

struct omedia_AEinstalls 
{
	AEEventClass 			theClass;
  	AEEventID 				theEvent;
  	AEEventHandlerProcPtr	theProc;
};

static omedia_AEinstalls omedia_HandlersToInstall[omediatotal_Handlers] =  
{
    {
        kCoreEventClass, kAEOpenApplication, OMediaAppleEvent::AEOpenHandler
    },  
    
    {
        kCoreEventClass, kAEOpenDocuments, OMediaAppleEvent::AEOpenDocHandler
    },  
    
    {
        kCoreEventClass, kAEQuitApplication, OMediaAppleEvent::AEQuitHandler
    },  
    
    {
        kCoreEventClass, kAEPrintDocuments, OMediaAppleEvent::AEPrintHandler
    }
};

static AEEventHandlerUPP  omedia_ProcToInstall[omediatotal_Handlers];

//....................................

OMediaAppleEvent::OMediaAppleEvent()
{

   OSErr err = noErr;
   long l = 0;

	if (globalptr==this) return;	// Should not be installed more than once
	delete globalptr;
	globalptr = NULL;

 
    if (Gestalt(gestaltAppleEventsAttr, &l) == noErr)	// Support AppleEvent?
    {
        for (l = 0; l < omediatotal_Handlers; l++) 
        {
 			// Build EventHandler procs
 
        	omedia_ProcToInstall[l] = NewAEEventHandlerUPP(omedia_HandlersToInstall[l].theProc);
        
        	// Install the procs
        
            err = AEInstallEventHandler(omedia_HandlersToInstall[l].theClass, 
            							omedia_HandlersToInstall[l].theEvent,
                                        omedia_ProcToInstall[l], 0, false);

			// Error?

            if (err!=noErr)  omd_OSEXCEPTION(err);
        }
    }
    
    globalptr = this;	// Ok, installed
}


OMediaAppleEvent::~OMediaAppleEvent()
{
	long	l;
 	OSErr 	err;
 
	if (globalptr && globalptr==this)
	{
       for (l = 0; l < omediatotal_Handlers ; l++) 
       {
        
            err = AERemoveEventHandler(omedia_HandlersToInstall[l].theClass, 
            						   omedia_HandlersToInstall[l].theEvent,
                                       omedia_ProcToInstall[l],
                                       false);


            if (err!=noErr)  omd_OSEXCEPTION(err);
        }
        
        globalptr = NULL;
	}
}

