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
#ifndef OMEDIA_EndianSupport_H
#define OMEDIA_EndianSupport_H

#include "OMediaTypes.h"
#include "OMediaSysDefs.h"

#define omd_ReverseLong(l) ((((l)&0xFF)<<24L) | (((l)&0xFF00)<<8L) | (((l)&0xFF0000)>>8L) | (((l)&0xFF000000)>>24L))
#define omd_ReverseShort(l) ((((l)&0xFF00)>>8) | (((l)&0xFF)<<8))

#ifdef omd_LITTLE_ENDIAN
#define omd_IfLittleEndianReverseLong(l) omd_ReverseLong(l)
#define omd_IfLittleEndianReverseShort(l) omd_ReverseShort(l)
#define omd_IfLittleEndianReverseLongProc(l) l = omd_ReverseLong(l)
#define omd_IfLittleEndianReverseShortProc(l) l = omd_ReverseShort(l)
#else
#define omd_IfLittleEndianReverseLong(l) l
#define omd_IfLittleEndianReverseShort(l) l
#define omd_IfLittleEndianReverseLongProc(l)
#define omd_IfLittleEndianReverseShortProc(l)
#endif


inline void omf_ReverseBuffer(void *buf, unsigned long buflen)
{
	char *s = (char*)buf,*e = (char*)buf + (buflen-1) ,c;
	buflen>>=1; while(buflen--) {c = *s; *s = *e; *e = c; s++; e--;}
}

#endif

