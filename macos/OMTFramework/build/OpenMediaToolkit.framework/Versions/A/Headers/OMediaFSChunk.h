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
#ifndef OMEDIA_FSChunk_H
#define OMEDIA_FSChunk_H

#include "OMediaTypes.h"
#include "OMediaCompression.h"
#include "OMediaMemStream.h"

#include <vector>
#include <list>
#include <string>


typedef long omt_FSChunkID;

class OMediaFSBufferedChunk : public OMediaMemStream
{
	public:
	
	omtshared OMediaFSBufferedChunk();

	omtshared virtual void write(void *buffer, unsigned long nbytes);
	omtshared virtual void setsize(unsigned long newsize);
	
	bool			dirty;
};


class OMediaFSBlock
{
	public:
	
	unsigned long 	offset;
	unsigned long 	size;

	bool operator<(const OMediaFSBlock& x) const
	{
		return (offset < x.offset);
	}

};


class OMediaFSChunk : public OMediaFSBlock
{
	public:
	
	omt_FSChunkID 					ckid;	
	void						*extern_link;
	string						name;
	omt_CompressionLevel				compression;		// 0 = No compression
                                                                                // 9 = Maximum compression

	OMediaFSBufferedChunk				*buffered;
	long								context_stack_count;

	bool operator<(const OMediaFSChunk& x) const
	{
		return (ckid < x.ckid);
	}
};

typedef vector<OMediaFSBlock> 	omt_BlockList;
typedef list<OMediaFSChunk> 	omt_ChunkList;


#endif

