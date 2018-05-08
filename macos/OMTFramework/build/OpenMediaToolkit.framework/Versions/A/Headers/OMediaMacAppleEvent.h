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
#ifndef OMEDIA_AppleEvent_H
#define OMEDIA_AppleEvent_H

#include "OMediaTypes.h"

#ifdef __MWERKS__
//typedef unsigned long omt_AppleEventRefVal;
typedef long omt_AppleEventRefVal;
#else
typedef long omt_AppleEventRefVal;
#endif


class OMediaAppleEvent
{
	public:	


	OMediaAppleEvent();
	virtual ~OMediaAppleEvent();


	virtual OSErr ae_openapplication(const AppleEvent *messagein, AppleEvent *reply, long refIn);
	virtual OSErr ae_opendocument(const AppleEvent *messagein, AppleEvent *reply, long refIn);
	virtual OSErr ae_print(const AppleEvent *messagein, AppleEvent *reply, long refIn);
	virtual OSErr ae_quit(const AppleEvent *messagein, AppleEvent *reply, long refIn);



	// *************
	// * Low-level *
	
	// Low-level AppleEvent handler
	pascal static OSErr AEOpenHandler(const AppleEvent *messagein, AppleEvent *reply, omt_AppleEventRefVal refIn);
	pascal static OSErr AEOpenDocHandler(const AppleEvent *messagein, AppleEvent *reply, omt_AppleEventRefVal refIn);
	pascal static OSErr AEPrintHandler(const AppleEvent *messagein, AppleEvent *reply, omt_AppleEventRefVal refIn);
	pascal static OSErr AEQuitHandler(const AppleEvent *messagein, AppleEvent *reply, omt_AppleEventRefVal refIn);

	protected:
	
	static	OMediaAppleEvent	*globalptr;
};


#endif

