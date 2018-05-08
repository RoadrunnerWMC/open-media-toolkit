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
 

#ifndef OMEDIA_Error_H
#define OMEDIA_Error_H

//#define omd_EXCEPTION_DISABLED	// Can be used to disable OMT exceptions


#include "OMediaTypes.h"
#include "OMediaString.h"

#include <string>

enum omt_ErrorId {		omcerr_Null,	
						omcerr_OutOfMemory,
						omcerr_CantOpen,
						omcerr_OutOfRange,
						omcerr_BadFormat,
						omcerr_OsError,
						omcerr_CantBuild,
						omcerr_NullPointer,
						omcerr_NoList,
						omcerr_ListError,
						omcerr_CantFind,
						omcerr_CantUnlink,
						omcerr_CantLink,
						omcerr_CantRemovePort,
						omcerr_BadMessage,
						omcerr_OSError,				// errordata = os error
						omcerr_GfxWorldDrawError,
						omcerr_DrawPortBadWorld,
						omcerr_DrawToUnlockedPort,
						omcerr_BadPoolEntryDeleted,
						omcerr_BlitCompilerBadSource,
						omcerr_BlitCompilerBadMask,
						omcerr_BadDestCompiledWorld,
						omcerr_BadWorldForElement,
						omcerr_BadSequence,
						omcerr_SetUnlinkedFrame,
						omcerr_BadFormattedStream,
						omcerr_CantFindFormattedStreamHeader,
						omcerr_FormattedStreamAccessFault,
						omcerr_FormattedStreamWriteError,
						omcerr_CantDeleteOpenChunk,
						omcerr_BadWorldForDistortedDraw,
						omcerr_StoreStartInfoNotInitialized,
						omcerr_DBUnregistredChunkType,
						omcerr_DBCantSetLockedObject,
						omcerr_BadPixmapDestination,
						omcerr_MathError,
						omcerr_BadDXFFormat,
						omcerr_CantOpenWinsock,
						omcerr_NotClosed,
						omcerr_BadConfiguration,
						omcerr_SocketError,
						omcerr_PointBufferFull,
						omcerr_EmptyDBChunk,
						omcerr_String,
						omcerr_InvalidParamater,
						omcerr_NoVideoCardLinked,
						omcerr_CantFindCurrentVMode,
						omcerr_GLCantChoosePixelFormat,
						omcerr_GLCantCreateContext,
						omcerr_GLCantAttachContext,
						omcerr_CompressionError,
						omcerr_InetErr,
						omcerr_NoTextureFormatAvailable,
						omcerr_CantChangeVideoMode
						};
 

typedef long 	omt_ErrorData;


// * OpenMedia throws this structure when an error occurs:

class OMediaError
{
	public:
	
	omt_ErrorId				errorcode;	// A standard error code
	omt_ErrorData			errordata;

	string					error_string;	// If errorcode is omcerr_String

	string					file;			// Code information
	int						line;

	
	inline OMediaError(omt_ErrorId errtype, 
						  omt_ErrorData errdata = 0,
						  string afile = "",
						  int aline = 0,
						  string errstring ="")
	{
		errorcode = errtype;
		errordata = errdata;
		error_string = errstring;
		file = afile;
		line = aline;
	}
	
	inline void get_error_string(string &error)
	{
		error = "OMT Exception: ";
		
		if (errorcode==omcerr_String)
		{
			error += error_string;
		}
		else
		{	
			error += errortype_to_string(errorcode);
			error += " (";
			error += omd_L2STR(errordata);
			error += "). ";
		}
		
		error += "File: ";
		error += file;
		error += ". ";
		error += "Line: ";
		error += omd_L2STR(line); 
		error += "."; 
	}
	
	omtshared static char *errortype_to_string(omt_ErrorId errtype);


	protected:
	
	static char *error_strings[];
};


#ifndef omd_EXCEPTION_DISABLED
#define omd_EXCEPTION(type) {char*f=__FILE__; int l=__LINE__; throw OMediaError(type,0,f,l);}
#define omd_OSEXCEPTION(data) {char*f=__FILE__; int l=__LINE__; throw OMediaError(omcerr_OSError,data,f,l);}
#define omd_STREXCEPTION(str) {char*f=__FILE__; int l=__LINE__; throw OMediaError(omcerr_String,0,f,l,str);}
#else
#define omd_EXCEPTION(type) {}
#define omd_OSEXCEPTION(data) {}
#define omd_STREXCEPTION(data) {}
#endif

#endif

