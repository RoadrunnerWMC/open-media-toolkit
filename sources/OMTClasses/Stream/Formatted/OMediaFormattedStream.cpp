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
 
#include "OMediaFormattedStream.h"
#include "OMediaTimeCounter.h"
#include "OMediaStream.h"
#include "OMediaError.h"
#include "OMediaMoveableMem.h"
#include "OMediaMemStream.h"


#include <algorithm>

//---------------------------------------------------------------------

OMediaFormattedStream::OMediaFormattedStream(OMediaStream *astream)
{
	max_comp_time = 4;
	stream = astream;
	chunk_open = NULL;
	update_header = false;
	buffered_chunk = NULL;
	open_formatstream();
}

OMediaFormattedStream::~OMediaFormattedStream()
{
	flush_buffer_chunk();
	flush_buffer();

	if (update_header) 
	{	
		chunk_open = NULL;		
	
		// Compress stream
		compress();
		
		// Save header
		save_header();
	}
}

void OMediaFormattedStream::open_formatstream(void)
{
	unsigned long hdrid, hdroff;

	if (stream->getsize()==0)	// new stream, build an empty structure
	{
		hdroff = 0;
		hdrid = omcid_fmtcurrent_version;
		stream->setposition(0);
		*stream<<hdrid;		// Push id
		*stream<<hdroff;	// Push chunk offset. Will be updated by "save_header"
		update_header = true;
		version = 3;
	}
	else 
	{
		// Load formatted stream structure
		stream->setposition(0);
		*stream>>hdrid;
		if (hdrid==omcid_fmtstream) version = 1;
		else if (hdrid==omcid_fmtstream_v2) version = 2;
		else if (hdrid==omcid_fmtstream_v3) version = 3;
		else omd_EXCEPTION(omcerr_BadFormattedStream);
		
		load_header();
	}
}

//---------------------------------------------------------------------

void OMediaFormattedStream::init_buffer_chunk(void)
{
	if (chunk_open)
	{
		long	save_pos = getposition(),src_size,dest_size;
		OMediaMoveableMem			*compressed;
		OMediaFSBufferedChunk		*dest;
		void						*dest_data, *comp_data;
	
		if (chunk_open->buffered)
		{
			buffered_chunk = chunk_open->buffered;
			setposition(save_pos);
			return;
		}

		if (buffered_chunk) flush_buffer_chunk();

		if (getsize()==0) 
		{
			buffered_chunk = new OMediaFSBufferedChunk;
			return;
		}

		dest = new OMediaFSBufferedChunk;
		
		setposition(0);
		
		if (chunk_open->compression==0)
		{
			src_size = getsize();		
			dest->setsize(src_size);

			dest_data = dest->get_memory_block()->lock();
			read(dest_data,src_size);
			dest->get_memory_block()->unlock();
		}
		else
		{
			src_size = getsize();
			*this>>dest_size;
			
			dest->setsize(dest_size);	src_size-=4;
			
			compressed = new OMediaMoveableMem;
			compressed->setsize(src_size);
			
			comp_data = compressed->lock();
			dest_data = dest->get_memory_block()->lock();
		
			read(comp_data,src_size);

			OMediaCompression::uncompress (comp_data, src_size, dest_data, dest_size);			

			compressed->unlock();
			dest->get_memory_block()->unlock();
			
			delete compressed;
		}
		
		buffered_chunk = dest;
		buffered_chunk->dirty = false;
		setposition(save_pos);
	}
	else omd_EXCEPTION(omcerr_FormattedStreamAccessFault);
}

void OMediaFormattedStream::flush_buffer_chunk(void)
{
	if (chunk_open && buffered_chunk)
	{
		if (chunk_open->buffered)
		{
			buffered_chunk = NULL;
			return;	
		}
	
		OMediaMoveableMem	*compressed;
		void				*comp_data,*src_data;
		long				src_size = buffered_chunk->getsize();
		long				comp_len;

		if (!buffered_chunk->dirty)
		{
			delete buffered_chunk;
			buffered_chunk = NULL;
			return;
		}

		if (src_size==0)
		{		
			delete buffered_chunk;
			buffered_chunk = NULL;
			setsize(0);
			return;
		}
		
		compressed = new OMediaMoveableMem;
		compressed->setsize(src_size+100);

		comp_data = compressed->lock();
		src_data = buffered_chunk->get_memory_block()->lock();
		
		comp_len = src_size+100;
		OMediaCompression::compress(src_data, src_size, comp_data, comp_len, chunk_open->compression);

		buffered_chunk->get_memory_block()->unlock();

		delete buffered_chunk;
		buffered_chunk = NULL;

		setsize(comp_len+4);
		setposition(0);

		*this<<src_size;
		write(comp_data,comp_len);

		compressed->unlock();

		delete compressed;
	}
}

void OMediaFormattedStream::set_chunk_compression(omt_ChunkType type, 
												  omt_FSChunkID id,
												  omt_CompressionLevel level)
{
	OMediaFSChunk	*chunk;

	store_stream_context();

	chunk = get_chunk(type, id);
	if (!chunk) return;

	if (chunk_open!=chunk) open_chunk(type,id);

	if (!chunk->compression && level)
	{
		init_buffer_chunk();
	}
	else if (chunk->compression && !level)
	{
		flush_buffer_chunk();
	}
	else if (chunk->compression == level)
	{
		restore_stream_context();
		return;
	}

	chunk->compression = level;

	restore_stream_context();
	set_header_dirty();
}

//---------------------------------------------------------------------

void OMediaFormattedStream::store_stream_context(void)
{
	OMediaFMTStreamContext	context;

	context.chunk = chunk_open;

	if (chunk_open)
	{
		context.position = getposition();
		context.db_streaming_object = db_streaming_object;
		context.chunk->buffered = buffered_chunk;
		context.chunk->context_stack_count++;
	}
	
	context_stack.push_back(context);
}

void OMediaFormattedStream::restore_stream_context(void)
{
	omt_FMTStreamContextContainer::iterator	sc = context_stack.end();
	sc--;

	if (chunk_open==(*sc).chunk)
	{
		if (chunk_open) setposition((*sc).position);
	}
	else
	{
		if (chunk_open) close_chunk();
		chunk_open = (*sc).chunk;
		if (chunk_open)
		{
			buffered_chunk = (*sc).chunk->buffered;
			setposition((*sc).position);
		}
	}

	if ((*sc).chunk)
	{
		(*sc).chunk->context_stack_count--;
		if ((*sc).chunk->context_stack_count==0) (*sc).chunk->buffered = NULL;
	}

	db_streaming_object = (*sc).db_streaming_object;

	context_stack.erase(sc);
}
 


//---------------------------------------------------------------------

void OMediaFormattedStream::open_chunk(omt_ChunkType type, omt_FSChunkID id)
{
	OMediaFSChunkTypeNode *chunktype_open;

	close_chunk();

	chunktype_open = get_chunk_type_node(type);
	if (!chunktype_open) chunktype_open = create_chunktype(type);
	
	chunk_open = get_chunk(chunktype_open, id);
	if (!chunk_open) chunk_open = create_chunk(chunktype_open, id);
	
	if (chunk_open)
	{
		if (chunk_open->compression) 
		{
			init_buffer_chunk();
			setposition(0);
		}
		
		// If not empty place the stream marker
		if (chunk_open->size) stream->setposition(chunk_open->offset);
	}
}

void OMediaFormattedStream::open_chunk(omt_ChunkType type, string name, omt_FSChunkID *id)
{
	OMediaFSChunkTypeNode *chunktype_open;

	close_chunk();

	chunktype_open = get_chunk_type_node(type);
	if (!chunktype_open) chunktype_open = create_chunktype(type);
	
	chunk_open = get_chunk(chunktype_open, name);
	if (!chunk_open) 
	{
		chunk_open = create_chunk(chunktype_open, get_unique_chunkid(type));
		chunk_open->name = name;
	}
	
	if (chunk_open)
	{
		if (chunk_open->compression)
		{
			init_buffer_chunk();
			setposition(0);
		}

		// If not empty place the stream marker
		if (chunk_open->size) stream->setposition(chunk_open->offset);
		
		if (id) *id = chunk_open->ckid;
	}
	else if (id) *id = -1;
}

void OMediaFormattedStream::close_chunk(void)
{
	flush_buffer_chunk();
	flush_buffer();
	chunk_open = NULL;
}

void OMediaFormattedStream::duplicate_chunk(omt_ChunkType src_type, omt_FSChunkID src_id,
								    omt_ChunkType dest_type, omt_FSChunkID dest_id)
{
	OMediaFSChunk	*chunk;
	const unsigned long buf_size = 512*4;
	unsigned long	size,nblock,offset;
	char			*buffer;

	chunk = get_chunk(src_type,src_id);
	if (chunk->compression)
	{
		open_chunk(src_type, src_id);
		size = getsize();
		if (size)
		{
			buffer = new char[size];
			read(buffer,size);
		}

		open_chunk(dest_type, dest_id);
		setsize(0);
		set_chunk_compression(dest_type,dest_id,chunk->compression);
		if (size)
		{
			write(buffer,size);
			delete [] buffer;
		}
		
		return;
	}


	open_chunk(src_type, src_id);
	size = getsize();
	open_chunk(dest_type, dest_id);
	set_chunk_compression(dest_type,dest_id,0);
	setsize(size);

	if (!size) return;
	
	buffer = new char[buf_size];
	if (!buffer) omd_EXCEPTION(omcerr_OutOfMemory);

	nblock = size/buf_size;
	offset=0;
	while(nblock--)
	{
		open_chunk(src_type, src_id);
		setposition(offset);
		read(buffer,buf_size);
		open_chunk(dest_type, dest_id);
		setposition(offset);
		write(buffer,buf_size);		
		offset += buf_size;
	}
	
	size -=offset;
	if (size)
	{
		open_chunk(src_type, src_id);
		setposition(offset);
		read(buffer,size);
		open_chunk(dest_type, dest_id);
		setposition(offset);
		write(buffer,size);			
	}
	
	delete buffer;
}

void OMediaFormattedStream::export_chunk(omt_ChunkType src_type, omt_FSChunkID src_id,
								     OMediaStream *dest_stream)
{
	const unsigned long buf_size = 512*4;
	unsigned long	size,nblock,offset;
	char			*buffer;

	if (dest_stream==this) omd_EXCEPTION(omcerr_CantBuild);

	open_chunk(src_type, src_id);
	size = getsize();

	if (!size) return;
	
	buffer = new char[buf_size];
	if (!buffer) omd_EXCEPTION(omcerr_OutOfMemory);

	nblock = size/buf_size;
	offset=0;
	while(nblock--)
	{
		read(buffer,buf_size);
		dest_stream->write(buffer,buf_size);		
		offset += buf_size;
	}
	
	size -=offset;
	if (size)
	{
		read(buffer,size);
		dest_stream->write(buffer,size);			
	}
	
	delete buffer;
}

void OMediaFormattedStream::import_chunk(OMediaStream *src_stream, long src_size,
								    omt_ChunkType dest_type, omt_FSChunkID dest_id)
{
	const unsigned long buf_size = 512*4;
	unsigned long	size,nblock,offset;
	char			*buffer;

	if (src_stream==this) omd_EXCEPTION(omcerr_CantBuild);

	open_chunk(dest_type, dest_id);
	size = src_size;

	if (!size) return;
	
	buffer = new char[buf_size];
	if (!buffer) omd_EXCEPTION(omcerr_OutOfMemory);

	nblock = size/buf_size;
	offset=0;
	while(nblock--)
	{
		src_stream->read(buffer,buf_size);
		write(buffer,buf_size);		
		offset += buf_size;
	}
	
	size -=offset;
	if (size)
	{
		src_stream->read(buffer,size);
		write(buffer,size);			
	}
	
	delete buffer;
}


void OMediaFormattedStream::truncate_chunk(OMediaFSChunk *chunk, unsigned long newsize)
{
	OMediaFSBlock	block;

	block.offset = chunk->offset+newsize;
	block.size = chunk->size-newsize;
	chunk_freelist.push_back(block);
	
	// Empty blocks should be placed at the start of the flat list
	
	if (newsize==0)
	{
		chunk->size = 0;
		chunk->offset = 0;
		sort(chunk_flatlist.begin(), chunk_flatlist.end(), compare_flatchunk_offset());
	}
	
	sort(chunk_freelist.begin(), chunk_freelist.end());
	union_free_block(block);

	update_header = true;
}

void OMediaFormattedStream::create_free_block(unsigned long offset, unsigned long size)
{
	OMediaFSBlock	block;
	vector<OMediaFSChunk*>::iterator flati;

	if (!chunk_flatlist.size()) return;
	
	flati = chunk_flatlist.end(); flati--;

	if (!size || !offset || offset>=(*flati)->offset) return;

	block.offset = offset;
	block.size = size;
	chunk_freelist.push_back(block);
	
	sort(chunk_freelist.begin(), chunk_freelist.end());
	union_free_block(block);

	update_header = true;
}


OMediaFSChunkTypeNode *OMediaFormattedStream::create_chunktype(omt_ChunkType chunktype)
{
	OMediaFSChunkTypeNode	emptychunktype, *newchunktype;

	chunk_type_list.push_back(emptychunktype);
	newchunktype = &(*(--chunk_type_list.end()));
	newchunktype->set_chunk_type(chunktype);
	
	update_header = true;
	
	return newchunktype;
}

OMediaFSChunk *OMediaFormattedStream::create_chunk(OMediaFSChunkTypeNode *chunktypenode, omt_FSChunkID id)
{
	OMediaFSChunk				newchunk;
	omt_ChunkList::iterator		chunk_i;

	// Create a new chunk. An empty chunk does not have a position.
	
	newchunk.ckid = id;
	newchunk.offset = 0;
	newchunk.size = 0;
	newchunk.compression = 0;
	newchunk.buffered = NULL;
	newchunk.context_stack_count = 0;
	newchunk.extern_link = NULL;
	chunktypenode->get_chunk_list()->push_back(newchunk);

	// Add to flat list. An empty chunk is always at the start of the flat list
	chunk_flatlist.insert(chunk_flatlist.begin(),&(*(--chunktypenode->get_chunk_list()->end())));

	// Update indexes

	chunk_i = (chunktypenode->get_chunk_list()->end());
	chunk_i--;

	(*(chunktypenode->get_id_indexes()))[id] = chunk_i;
	(*(chunktypenode->get_name_indexes()))[newchunk.name] = chunk_i;

	// Update header

	update_header = true;
	
	return &(*chunk_i);
}

void OMediaFormattedStream::delete_chunktype(OMediaFSChunkTypeNode *chunktypenode)
{
	for(omt_ChunkTypeList::iterator cti = chunk_type_list.begin();
		cti != chunk_type_list.end();
		cti++)
	{
		if (&(*cti) == chunktypenode) 
		{
			chunk_type_list.erase(cti); 
			return;
		}
	}
}


void OMediaFormattedStream::delete_chunk(omt_ChunkType type, omt_FSChunkID id)
{
	OMediaFSChunkTypeNode	*typenode;
	omt_ChunkList::iterator	ci;
	OMediaFSBlock	block;
	OMediaFSChunk *chunk;
	vector<OMediaFSChunk*>::iterator flati;
	bool		build_free_block;

	flush_buffer();

	// Find chunk
	typenode = get_chunk_type_node(type);
	chunk = get_chunk_iterator(typenode, id, ci);

	// Check if we can delete chunk
	if (!chunk) return;
	if (chunk_open==chunk) 
	{
		chunk_open = NULL;
		delete buffered_chunk;
		buffered_chunk = NULL;
	}

	// Create a free block?
	flati = chunk_flatlist.end(); flati--;
	build_free_block = (chunk->size != 0);

	// Push chunk to free block list
	if (build_free_block)
	{
		block.offset = chunk->offset;
		block.size = chunk->size;
		chunk_freelist.push_back(block);
	}

	// Delete flat iterator
	flati = find_flatiterator(chunk);	// Should not return NULL
	if (flati!=chunk_flatlist.end()) chunk_flatlist.erase(flati);

	// Delete ID index

	omt_FMIDIndexes	*id_indexes = typenode->get_id_indexes();
	omt_FMIDIndexes::iterator		id_ind_i;

	id_ind_i = id_indexes->find((*ci).ckid);
	if (id_ind_i!=id_indexes->end()) id_indexes->erase(id_ind_i);

	// Delete name index

	omt_FMNameIndexes *name_indexes = typenode->get_name_indexes();
	omt_FMNameIndexes::iterator		name_ind_i;
	string							name_deleted;

	name_deleted = (*ci).name;

	name_ind_i = name_indexes->find((*ci).name);
	if (name_ind_i!=name_indexes->end()) name_indexes->erase(name_ind_i);


	// Delete chunk
	typenode->get_chunk_list()->erase(ci);

	// Update name index

	for(ci=typenode->get_chunk_list()->begin();
		ci!=typenode->get_chunk_list()->end();
		ci++)
	{
		if ((*ci).name==name_deleted)
		{
			(*name_indexes)[name_deleted] = ci;
			break;
		}
	}

	// Delete chunk type if empty

	if (typenode->get_chunk_list()->size()==0) delete_chunktype(typenode);


	if (build_free_block)
	{
		// Sort free block list
		sort(chunk_freelist.begin(), chunk_freelist.end());
		
		// Try to union free block
		union_free_block(block);
	}


	update_header = true;
}

void OMediaFormattedStream::union_free_block(OMediaFSBlock	block)
{
	omt_BlockList::iterator	bi,bi2;
	vector<OMediaFSChunk*>::iterator flati;

	bi = search_freeblock_offset(block.offset);	// Before
	if (bi != chunk_freelist.begin())
	{
		bi2 = bi; bi2--;
		if ((*bi).offset == (*bi2).offset + (*bi2).size)
		{
			(*bi2).size += (*bi).size;
			block = (*bi2);
			chunk_freelist.erase(bi);
			bi = search_freeblock_offset(block.offset);
		}
	}

	bi2 = bi; bi2++;											// After
	if (bi2 != chunk_freelist.end())
	{
		if ((*bi).offset + (*bi).size == (*bi2).offset)
		{
			(*bi).size += (*bi2).size;
			block = (*bi);
			chunk_freelist.erase(bi2);
			bi = search_freeblock_offset(block.offset);
		}
	}
	
	// Delete if it's at the end of stream
	
	if (chunk_flatlist.size()==0) chunk_freelist.erase(bi);
	else
	{
		flati = chunk_flatlist.end(); flati--;
		if ((*flati)->offset < block.offset) 
			chunk_freelist.erase(bi);
	}	
}


omt_BlockList::iterator OMediaFormattedStream::search_freeblock_offset(unsigned long offset)
{
	omt_BlockList::iterator i;

	for(i = chunk_freelist.begin();
		i != chunk_freelist.end();
		i++)
	{
		if ((*i).offset == offset) return i;
	}
	
	return i;
}

omt_BlockList::iterator OMediaFormattedStream::search_freeblock_minsize(unsigned long minimal_size)
{
	omt_BlockList::iterator i, besti = chunk_freelist.end();


	for(i = chunk_freelist.begin();
		i != chunk_freelist.end();
		i++)
	{
		if ((*i).size >= minimal_size) 
		{
			// This method returns the biggest block which fits in the minimal size
			if (besti==chunk_freelist.end() ||
				(*besti).size < (*i).size) besti = i;
		}
	}
	
	return besti;
}

omt_BlockList::iterator OMediaFormattedStream::search_freeblock_minsize_before(unsigned long before_offset,
																				unsigned long minimal_size)
{
	omt_BlockList::iterator i, besti = chunk_freelist.end();


	for(i = chunk_freelist.begin();
		i != chunk_freelist.end();
		i++)
	{
		if ((*i).size >= minimal_size && (*i).offset < before_offset && (*besti).size) 
		{
			// This method returns the smallest offset possible
			if (besti==chunk_freelist.end() ||
				(*besti).offset > (*i).offset) besti = i;
		}
	}
	
	return besti;
}


vector<OMediaFSChunk*>::iterator OMediaFormattedStream::find_flatiterator(OMediaFSChunk *chunk)
{
	for(vector<OMediaFSChunk*>::iterator i = chunk_flatlist.begin();
		i != chunk_flatlist.end();
		i++)
	{
		if (*i == chunk) return i;
	}
	
	return chunk_flatlist.end();
}


OMediaFSChunk *OMediaFormattedStream::get_chunk(OMediaFSChunkTypeNode *chunktypenode, omt_FSChunkID id)
{
	if (chunktypenode)
	{
		omt_ChunkList::iterator		ci;
		omt_FMIDIndexes::iterator	i;
		omt_FMIDIndexes				*ilist;

		ilist = chunktypenode->get_id_indexes();	
		i = ilist->find(id);
		if (i==ilist->end()) return NULL;

		ci = (*i).second;
		return &(*ci);
	}
	
	return NULL;
}

void OMediaFormattedStream::remove_chkname_FMNames(OMediaFSChunkTypeNode *typenode,
													omt_ChunkList::iterator ci)
{
	// Delete name index

	omt_FMNameIndexes *name_indexes = typenode->get_name_indexes();
	omt_FMNameIndexes::iterator		name_ind_i;
	omt_ChunkList::iterator			i;

	name_ind_i = name_indexes->find((*ci).name);
	if (name_ind_i!=name_indexes->end()) 
	{	
		if ((*name_ind_i).second!=ci) return;
		name_indexes->erase(name_ind_i);
	}

	// Now I need to update the name map list
	
	for(i=typenode->get_chunk_list()->begin();
		i!=typenode->get_chunk_list()->end();
		i++)
	{
		if (ci==i) continue;

		name_ind_i = name_indexes->find((*i).name);
		if (name_ind_i==name_indexes->end())
		{
			(*name_indexes)[(*i).name]	= i;		
		}			
	}
}

void OMediaFormattedStream::update_chkname_FMNames(OMediaFSChunkTypeNode *typenode,
													omt_ChunkList::iterator ci, 
													string &newname)
{
	omt_FMNameIndexes *name_indexes = typenode->get_name_indexes();

	remove_chkname_FMNames(typenode,ci);
	
	(*name_indexes)[newname]= ci;		
}


OMediaFSChunk *OMediaFormattedStream::get_chunk(OMediaFSChunkTypeNode *chunktypenode, string name)
{
	if (chunktypenode)
	{
		omt_FMNameIndexes::iterator		i;
		omt_FMNameIndexes				*ilist;
		omt_ChunkList::iterator			ci;

		ilist = chunktypenode->get_name_indexes();	
		i = ilist->find(name);
		if (i==ilist->end()) return NULL;

		ci = (*i).second;
		return &(*ci);
	}
	
	return NULL;
}


OMediaFSChunkTypeNode *OMediaFormattedStream::get_chunk_type_node(omt_ChunkType type)
{
	for(omt_ChunkTypeList::iterator cti = chunk_type_list.begin();
		cti != chunk_type_list.end();
		cti++)
	{
		if ((*cti).get_chunk_type() == type) return &(*cti);
	}
	
	return NULL;
}

OMediaFSChunk *OMediaFormattedStream::get_chunk_iterator(OMediaFSChunkTypeNode *chunktypenode, 
														 omt_FSChunkID id,
														 omt_ChunkList::iterator &ci)
{
	if (chunktypenode)
	{
		omt_FMIDIndexes::iterator	i;
		omt_FMIDIndexes				*ilist;

		ilist = chunktypenode->get_id_indexes();	
		i = ilist->find(id);
		if (i==ilist->end()) 
		{
			ci = chunktypenode->get_chunk_list()->end();
			return NULL;
		}

		ci = (*i).second;
		return &(*ci);
	}
	
	return NULL;
}


void OMediaFormattedStream::move_chunk_to_end(OMediaFSChunk *chunk)
{
	unsigned long free_offset = chunk->offset;
	vector<OMediaFSChunk*>::iterator flati;

	// Restructure	 
	if (chunk_flatlist.size()==0) chunk->offset = omc_free_region_start;
	else
	{
		flati = chunk_flatlist.end(); flati--;
		if ((*flati)->offset == free_offset) 
		{
			if (chunk_flatlist.size()==1 && !chunk->size)
				 chunk->offset = omc_free_region_start;
		
			return;
		}	
		else chunk->offset = (*flati)->offset  + (*flati)->size; 
	}	
	
	if (chunk->offset == free_offset) return;

	sort(chunk_flatlist.begin(), chunk_flatlist.end(), compare_flatchunk_offset());
	create_free_block(free_offset, chunk->size);
	
	// Let's move data
	move_block(free_offset, chunk->offset, chunk->size);
}

void OMediaFormattedStream::move_chunk_to_free_block(OMediaFSChunk *chunk,
													 omt_BlockList::iterator bi,
													 unsigned long size_required)
{
	unsigned long old_offset = chunk->offset, old_size = chunk->size;
	vector<OMediaFSChunk*>::iterator flati;

	// Restructure	 
	chunk->offset = (*bi).offset;

	sort(chunk_flatlist.begin(), chunk_flatlist.end(), compare_flatchunk_offset());
	
	if ((*bi).size == size_required)
	{
		chunk_freelist.erase(bi);
	}
	else
	{
		(*bi).offset += size_required;
		(*bi).size -= size_required;
	}

	if (chunk_freelist.size())
	{
		flati = chunk_flatlist.end(); flati--;
		
		bi = chunk_freelist.end(); bi--;
		if ((*bi).offset > (*flati)->offset) chunk_freelist.erase(bi);
	}

	if (old_offset && old_size) create_free_block(old_offset, old_size);

	// Let's move data
	move_block(old_offset, chunk->offset, chunk->size);
}


omt_FSChunkID OMediaFormattedStream::get_unique_chunkid(omt_ChunkType type)
{
	omt_FSChunkID				chunkid = 0;
	OMediaFSChunkTypeNode		*chunktypenode = get_chunk_type_node(type);
	omt_FMIDIndexes				*ilist;
  
	if (!chunktypenode) return 0;

	ilist = chunktypenode->get_id_indexes();
	
	for(chunkid = 0; ;chunkid++)
	{
		omt_FMIDIndexes::iterator	i;

		i = ilist->find(chunkid);
		if (i==ilist->end()) return chunkid;
	}
}

omt_FSChunkID OMediaFormattedStream::get_unique_chunkid(omt_ChunkType type, omt_FSChunkID greater_equal_id)
{
	omt_FSChunkID				chunkid = 0;
	OMediaFSChunkTypeNode		*chunktypenode = get_chunk_type_node(type);
	omt_FMIDIndexes				*ilist;
  
	if (!chunktypenode) return greater_equal_id;

	ilist = chunktypenode->get_id_indexes();
	
	for(chunkid = greater_equal_id; ;chunkid++)
	{
		omt_FMIDIndexes::iterator	i;

		i = ilist->find(chunkid);
		if (i==ilist->end()) return chunkid;
	}
}



//---------------------------------------------------------------------

void OMediaFormattedStream::goto_header(void)
{
	unsigned long 	hdr;

	stream->setposition(omc_static_hdr_offset);	// Go to header offset
	*stream>>hdr;
	stream->setposition(hdr);
}

void OMediaFormattedStream::save_header_data(void)
{
	unsigned long 			nchunktype, nchunk, hdr_chunk_size_pos;
	omt_ChunkType			chunktype;
	OMediaFSChunkTypeNode	*chunktypenode;
	OMediaFSChunk			*chunk;
	OMediaFSBlock			*block;

	// Start to write header now:
	nchunktype = chunk_type_list.size();
	*this<<nchunktype;	// Number of chunk type
	
	for(omt_ChunkTypeList::iterator cti = chunk_type_list.begin();
		cti != chunk_type_list.end();
		cti++)
	{
		chunktypenode = &(*cti);
		chunktype = chunktypenode->get_chunk_type();
		*this<<chunktype;
		
		nchunk 	= chunktypenode->get_chunk_list()->size();
		*this<<nchunk;	// Number of chunk
		
		for(omt_ChunkList::iterator ci = chunktypenode->get_chunk_list()->begin();
			ci != chunktypenode->get_chunk_list()->end();
			ci++)
		{
			chunk = &(*ci);
		
			// Write chunk info
			*this<<chunk->ckid;
			*this<<chunk->offset;
			if (chunk==chunk_open) hdr_chunk_size_pos = stream->getposition();
			*this<<chunk->size;
			*this<<chunk->name;
			*this<<chunk->compression;
		}
	}


	// Save free blocks
	
	nchunk 	= chunk_freelist.size();
	*this<<nchunk;	// Number of free chunk
	
	for(omt_BlockList::iterator ci = chunk_freelist.begin();
		ci != chunk_freelist.end();
		ci++)
	{
		block = &(*ci);
	
		// Write free chunk info
		*this<<block->offset;
		*this<<block->size;
	}
	
	// Update header chunk size
	stream->setposition(hdr_chunk_size_pos);
	*this<<chunk_open->size;
}



void OMediaFormattedStream::save_header(void)
{
	// Header is placed in a special chunk:
	open_chunk(omcchunk_header, omcchunkid_header);
		
	// Header should be placed at the end of stream during save
	move_chunk_to_end(chunk_open);
	setposition(0);

	// Save it!
	save_header_data();

	// Always keep the header offset at an absolute position in stream, so we can
	// find it later even if the chunk tree is not in memory.
	stream->setposition(omc_static_hdr_offset);
	*stream<<chunk_open->offset;
	
	// Check if version type needs to be updated
	if (version!=omc_fmtcurrent_version)
	{
		unsigned long	current_versid = omcid_fmtcurrent_version;
	
		stream->setposition(0);
		*stream<<current_versid;
	}

	chunk_open = NULL;
	update_header = false;
}

void OMediaFormattedStream::write_header(void)
{
	flush_buffer_chunk();
	flush_buffer();	

	save_header();
	delete_chunk(omcchunk_header, omcchunkid_header);
	update_header = false;

	stream->flush_buffer();	
	
	chunk_open = NULL;
}

void OMediaFormattedStream::load_header(void)
{
	unsigned long 			nchunktype, nchunk;
	omt_ChunkType			chunktype;
	OMediaFSChunkTypeNode	emptychunktype, *newchunktype;
	OMediaFSChunk			newchunk;
	OMediaFSBlock			newblock;
	omt_ChunkList::iterator	ci;

	purge_loaded_header();
	
	goto_header();	// Read directly from the stream. We can't use the
					// chunk tree because it's not in memory at this time.
	
	// Start to read header now:
	*stream>>nchunktype;	// Number of chunk type

	newchunk.buffered = NULL;
	newchunk.context_stack_count = 0;
	
	while(nchunktype--)
	{
		*stream>>chunktype;	// new chunk type
		chunk_type_list.push_back(emptychunktype);
		newchunktype = &(*(--(chunk_type_list.end())));
		newchunktype->set_chunk_type(chunktype);
		
		*stream>>nchunk;	// Number of chunk
		while(nchunk--)
		{
			// Read chunk info
			*stream>>newchunk.ckid;
			*stream>>newchunk.offset;
			*stream>>newchunk.size;			
			if (version>=2) *stream>>newchunk.name;
			if (version>=3) *stream>>newchunk.compression;
			else newchunk.compression = 0;
			
			newchunk.extern_link = NULL;
			newchunktype->get_chunk_list()->push_back(newchunk);

			ci = newchunktype->get_chunk_list()->end();	ci--;
			
			// Update to indexes

			(*newchunktype->get_id_indexes())[(*ci).ckid] = ci;
			(*newchunktype->get_name_indexes())[(*ci).name] = ci;

			// Add to flat list:
			chunk_flatlist.push_back(&(*ci));
		}
	}
	
	// Sort flat list
	sort(chunk_flatlist.begin(), chunk_flatlist.end(), compare_flatchunk_offset());

	// Load free chunks
	*stream>>nchunk; // Number of free chunks
	while(nchunk--)
	{
		// Read free chunk info
		*stream>>newblock.offset;
		*stream>>newblock.size;
		chunk_freelist.push_back(newblock);
	}
	
	// Don't keep in memory the header chunk
	delete_chunk(omcchunk_header, omcchunkid_header);
	update_header = false;
}

void OMediaFormattedStream::purge_loaded_header(void)
{
	chunk_type_list.erase(chunk_type_list.begin(), chunk_type_list.end());
	chunk_freelist.erase(chunk_freelist.begin(), chunk_freelist.end());
	chunk_flatlist.erase(chunk_flatlist.begin(), chunk_flatlist.end());
}

void OMediaFormattedStream::rebuild_flat_list(void)
{
	chunk_flatlist.erase(chunk_flatlist.begin(), chunk_flatlist.end());

	for(omt_ChunkTypeList::iterator cti = chunk_type_list.begin();
		cti != chunk_type_list.end();
		cti++)
	{
		for(omt_ChunkList::iterator ci = (*cti).get_chunk_list()->begin();
			ci != (*cti).get_chunk_list()->end();
			ci++)
		{
			chunk_flatlist.push_back(&(*(ci)));		
		}
	}

	sort(chunk_flatlist.begin(), chunk_flatlist.end(), compare_flatchunk_offset());
	
	update_header = true;
}


//---------------------------------------------------------------------

void OMediaFormattedStream::do_read(void *buffer, unsigned long nbytes)
{
	if (buffered_chunk)
	{
		buffered_chunk->read(buffer,nbytes);
	}
	else if (chunk_open && 
		(stream->getposition()+nbytes) <=
		(chunk_open->offset + chunk_open->size))
	{
		stream->read(buffer,nbytes);
	}
	else omd_EXCEPTION(omcerr_FormattedStreamAccessFault);
}

void OMediaFormattedStream::do_write(void *buffer, unsigned long nbytes)
{
	if (buffered_chunk)
	{
		buffered_chunk->write(buffer,nbytes);
	}
	else if (chunk_open)
	{
		// Need to be resized?
		if (!chunk_open->size ||
			(stream->getposition()+nbytes > chunk_open->offset + chunk_open->size))
		{
			setsize(chunk_open->size + nbytes);
		}
	
		stream->write(buffer,nbytes);
	}
	else omd_EXCEPTION(omcerr_FormattedStreamAccessFault);
}


unsigned long OMediaFormattedStream::do_getsize(void)
{
	if (buffered_chunk)
	{
		return buffered_chunk->getsize();
	}
	else if (chunk_open)
	{
		return chunk_open->size;
	
	} else  omd_EXCEPTION(omcerr_FormattedStreamAccessFault);

	return 0;
}

void OMediaFormattedStream::do_setsize(unsigned long newsize)
{
	if (buffered_chunk)
	{
		buffered_chunk->setsize(newsize);
	}
	else if (chunk_open)
	{
		omt_BlockList::iterator	bi;
		unsigned long curposition;
		vector<OMediaFSChunk*>::iterator flati;
	
		if (newsize == chunk_open->size) return;
	
		curposition = (chunk_open->size)?getposition():0;
	
		if (newsize<chunk_open->size)
		{
			// Create a free block
			truncate_chunk(chunk_open, newsize);
		}
		else
		{			
			// Is there a free block at the end of the chunk
			bi = (chunk_open->size==0)?chunk_freelist.end():search_freeblock_offset(chunk_open->offset + chunk_open->size);
			if (bi!=chunk_freelist.end() &&
				chunk_open->size + (*bi).size >= newsize)
			{
				(*bi).size -= newsize - chunk_open->size;
				(*bi).offset += newsize - chunk_open->size;
				if ((*bi).size == 0) chunk_freelist.erase(bi);
			}
			else
			{
				// If not the last chunk
				flati = chunk_flatlist.end(); flati--;
				if (chunk_open->offset != (*flati)->offset)
				{
					// Is there a free block
					bi = search_freeblock_minsize(newsize);
					if (bi!=chunk_freelist.end())
					{
						// Move chunk to free block
						move_chunk_to_free_block(chunk_open,bi,newsize);
					}
					else
					{
						// No room. Push the chunk at the end of stream
						move_chunk_to_end(chunk_open);
					}
				}
			}
		}
		
		// Check if it was an empty block
		if (chunk_open->offset==0 && newsize) 
		{
			flati = chunk_flatlist.end(); flati--;
			if ((*flati)->offset != chunk_open->offset) 
				chunk_open->offset = (*flati)->offset + (*flati)->size;
			else
				chunk_open->offset = omc_free_region_start;	
			
			sort(chunk_flatlist.begin(), chunk_flatlist.end(), compare_flatchunk_offset());	
		}

		// Update size
		chunk_open->size = newsize;
		
		if (newsize)
		{
			// If it's the last block, enlarge the stream
			flati = chunk_flatlist.end(); flati--;
			if (chunk_open->offset == (*flati)->offset) 
				stream->setsize(chunk_open->offset+chunk_open->size);
			
			// Update stream position
			setposition(curposition);
		}

		// Header need to be updated
		update_header = true;
	} 
	else  
	omd_EXCEPTION(omcerr_FormattedStreamAccessFault);
}


void OMediaFormattedStream::do_setposition(long offset, short relative)
{
	if (buffered_chunk)
	{
		buffered_chunk->setposition(offset,relative);
	}
	else if (chunk_open)
	{
		unsigned long siz = getsize();
		unsigned long position = getposition();

		switch(relative)
		{
			case omcfr_Start:		position = offset; break;
			case omcfr_End:			position = siz-offset; break;
			case omcfr_Current:		position += offset; break;
		}
	
		if (position<0) position = 0;
		else if (position>siz) position = siz;	
		
		stream->setposition(position+chunk_open->offset);

	} 
	else  omd_EXCEPTION(omcerr_FormattedStreamAccessFault);
}

long OMediaFormattedStream::do_getposition(void)
{
	if (buffered_chunk)
	{
		return buffered_chunk->getposition();
	}
	else if (chunk_open)
	{
		return stream->getposition() - chunk_open->offset;	
	} 
	else  omd_EXCEPTION(omcerr_FormattedStreamAccessFault);

	return 0;
}


void OMediaFormattedStream::move_block(unsigned long src, unsigned long dest, unsigned long size)
{
	#define BUFSIZE (1024L*16L)	// 16K

	static char		buffer[BUFSIZE];
	long			nloop,l;
	long			lastnbytes;

	if (src==dest || !size) return;
			
	nloop = size/BUFSIZE;
	lastnbytes = (size%BUFSIZE);
	if (lastnbytes) nloop++;
	else lastnbytes = BUFSIZE;

	for(l=0;l<nloop;l++)
	{
		stream->setposition(src);
		size = (l!=nloop-1) ? BUFSIZE:lastnbytes;
		
		stream->read(buffer,size);
	
		stream->setposition(dest);
		
		stream->write(buffer,size);
		
		src += BUFSIZE;
		dest += BUFSIZE;
	}
}

//---------------------------------------------------------------------

void OMediaFormattedStream::compress(void)
{
	unsigned long maxtime_ms, last_ms;
	vector<OMediaFSChunk*>::iterator flati;
	omt_BlockList::iterator freeblocki;
	
	if (!max_comp_time || 
		chunk_flatlist.size()==0 ||
		chunk_freelist.size()==0) return;

	flush_buffer();	

	flati = chunk_flatlist.end();
	last_ms = OMediaTimeCounter::get_millisecs();
	maxtime_ms = max_comp_time*1000;

	for(;;)
	{
		flati--;

		if ((OMediaTimeCounter::get_millisecs() - last_ms) >= maxtime_ms) break;

		freeblocki = search_freeblock_minsize_before((*flati)->offset, (*flati)->size);
		if (freeblocki!=chunk_freelist.end())
		{
			move_chunk_to_free_block((*flati), freeblocki, (*flati)->size);
			flati = chunk_flatlist.end();
		}

		if (flati==chunk_flatlist.begin()) break;
	}	
}

//---------------------------------------------------------------------

OMediaFSBufferedChunk::OMediaFSBufferedChunk()
{
	dirty = false;
}

void OMediaFSBufferedChunk::setsize(unsigned long newsize)
{
	dirty = true;
	OMediaMemStream::setsize(newsize);
}

void OMediaFSBufferedChunk::write(void *buffer, unsigned long nbytes)
{
	dirty = true;
	OMediaMemStream::write(buffer,nbytes);
}
	
//---------------------------------------------------------------------


/*
*/
/*
void OMediaFormattedStream::dump_structure(void)
{
	

	char str[256];
	omt_ChunkType 	chunktype;

	SIOUXSettings.columns = 90;
	SIOUXSettings.rows = 16;
	SIOUXSettings.toppixel = 380;
	SIOUXSettings.leftpixel = 64;
	SIOUXSettings.setupmenus = false;
	SIOUXSettings.initializeTB = false;
	SIOUXSettings.autocloseonquit = true;
	SIOUXSettings.asktosaveonclose = false;	

	cout<<"** Dump current formatted stream structure **\n\n";

	cout<<"* Chunks:\n";

	for(omt_ChunkTypeList::iterator cti = chunk_type_list.begin();
		cti != chunk_type_list.end();
		cti++)
	{
		chunktype = (*cti).get_chunk_type();
	
		str[0] = ((char*)&chunktype)[0];
		str[1] = ((char*)&chunktype)[1];
		str[2] = ((char*)&chunktype)[2];
		str[3] = ((char*)&chunktype)[3];
		str[4] = 0;

		cout<<"Chunk type: "<<str<<"\n";

		for(omt_ChunkList::iterator ci = (*cti).get_chunk_list()->begin();
			ci !=  (*cti).get_chunk_list()->end();
			ci++)
		{
			cout<<"Chunk: id="<<(*ci).id<<", size="<<(*ci).size<<", offset="<<(*ci).offset<<"\n";
		}
	}


	cout<<"\n* Flat chunk list:\n";

	for(vector<OMediaFSChunk*>::iterator ci = chunk_flatlist.begin();
		ci !=  chunk_flatlist.end();
		ci++)
	{
		cout<<"Chunk: id="<<(*ci)->id<<", bounds =("<<(*ci)->offset<<","<<((*ci)->offset+(*ci)->size)<<")\n";
	}

	cout<<"\n* Free block list:\n";

	for(omt_BlockList::iterator ci = chunk_freelist.begin();
		ci !=  chunk_freelist.end();
		ci++)
	{
		cout<<"Block: bounds =("<<(*ci).offset<<","<<((*ci).offset+(*ci).size)<<")\n";
	}

	cout<<"\n";

	
}
*/
