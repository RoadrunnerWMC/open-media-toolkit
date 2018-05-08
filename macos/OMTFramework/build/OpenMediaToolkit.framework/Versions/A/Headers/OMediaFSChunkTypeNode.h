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
#ifndef OMEDIA_FSChunkTypeNode_H
#define OMEDIA_FSChunkTypeNode_H

#include "OMediaTypes.h"
#include "OMediaFSChunk.h"

#include <list>
#include <map>


typedef less<string>	omt_FMLessString;
typedef less<omt_FSChunkID>	omt_FMLessID;
typedef map<string,omt_ChunkList::iterator,omt_FMLessString>		omt_FMNameIndexes;
typedef map<omt_FSChunkID,omt_ChunkList::iterator,omt_FMLessID>		omt_FMIDIndexes;


typedef unsigned long 	omt_ChunkType;

class OMediaFSChunkTypeNode
{
	public:

	// * Constructor/Destructor

	inline OMediaFSChunkTypeNode() {chunk_list = NULL;name_indexes=NULL;id_indexes=NULL;}
	inline ~OMediaFSChunkTypeNode() {delete chunk_list;delete name_indexes; delete id_indexes;}
	
	inline omt_ChunkType get_chunk_type(void) const {return chunk_type;}
	inline omt_ChunkList *get_chunk_list(void) {if (!chunk_list) chunk_list = new omt_ChunkList; return chunk_list;}

	inline void set_chunk_type(omt_ChunkType id) {chunk_type = id;}

	inline omt_FMNameIndexes *get_name_indexes(void) {if (!name_indexes) name_indexes = new omt_FMNameIndexes; return name_indexes;}
	inline omt_FMIDIndexes *get_id_indexes(void)  {if (!id_indexes) id_indexes = new omt_FMIDIndexes; return id_indexes;}

	protected:

	omt_FMNameIndexes		*name_indexes;
	omt_FMIDIndexes			*id_indexes;

	omt_ChunkType 			chunk_type;
	omt_ChunkList			*chunk_list;
};


#endif

