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


 

#ifndef OMEDIA_FixedPoint_H
#define OMEDIA_FixedPoint_H


// Macro version of the 32bits fixed point.

typedef long omt_FixedPoint16_16;

#define omd_FloatToFixed16_16(fl) ((long)((fl)*65536.0f))
#define omd_FixedToFloat16_16(fi) (((float)(fi))/65536.0f)

#define omd_IntToFixed16_16(i)  ((i)<<16L)
#define omd_FixedToInt16_16(fi) ((fi)>>16L)


typedef long omt_FixedPoint8_24;

#define omd_FloatToFixed8_24(fl) ((long)((fl)* float(1<<24L) ))
#define omd_FixedToFloat8_24(fi) ((float)((fi)/ float(1<<24L) ))

#define omd_IntToFixed8_24(i)  ((i)<<24L)
#define omd_FixedToInt8_24(fi) ((fi)>>24L)


#endif

