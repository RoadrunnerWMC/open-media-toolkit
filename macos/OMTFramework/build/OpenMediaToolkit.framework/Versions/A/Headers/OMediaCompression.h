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
#ifndef OMEDIA_Compression_H
#define OMEDIA_Compression_H

#include "OMediaTypes.h"

// Compression level

typedef unsigned short omt_CompressionLevel;				// 1-9
const omt_CompressionLevel	omclc_BestSpeed = 1;
const omt_CompressionLevel	omclc_BestCompression = 9;
const omt_CompressionLevel	omclc_DefaultCompression = 6;


class OMediaCompression
{
	public:

	omtshared static void compress(void *source, long source_len, void *dest, long &dest_len,  omt_CompressionLevel level =omclc_DefaultCompression);
	omtshared static void uncompress (void *source, long source_len, void *dest, long &dest_len);

	
	// Adler-32 checksum
	
	static unsigned long adler32(unsigned long adler, const unsigned char *buf, unsigned long len);

};


#endif
