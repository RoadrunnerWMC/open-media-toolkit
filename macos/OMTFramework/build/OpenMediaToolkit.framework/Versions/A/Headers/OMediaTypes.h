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
#ifndef OMEDIA_Types_H
#define OMEDIA_Types_H

#include "OMediaDLLSupport.h"

#ifndef NULL
#define NULL 0L
#endif

#define omc_NULL NULL


#define omd_Abs(x) ((x)<0?-(x):(x))

extern void omt_main(void);

#ifndef omd_NAMESPACE_DISABLED
namespace std {}
using namespace std;			// By default use "std" name space
#endif



//.................................................
// omt_Bool is only defined for compatibility reason.
// Use standard C++ bool type instead.

#ifndef omd_OBSOLETE_BOOL_FORMAT
#define omt_Bool  bool		// Boolean
#define omc_True  true		// Boolean values
#define omc_False false
#else
typedef short omt_Bool;					// Boolean
const omt_Bool omc_True  = 1;			// Boolean values
const omt_Bool omc_False = 0;
#endif

//.................................................


#endif

