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
#ifndef OMEDIA_FormattedStream_H
#define OMEDIA_FormattedStream_H


#include "OMediaTypes.h"

#include "OMediaFSChunkTypeNode.h"
#include "OMediaStreamOperators.h"
#include "OMediaStream.h"

class OMediaFMTStreamContext
{
	public:
	OMediaFSChunk	*chunk;
	long			position;
	OMediaDBObject	*db_streaming_object;
};

typedef vector<OMediaFMTStreamContext>							omt_FMTStreamContextContainer;



typedef list<OMediaFSChunkTypeNode>	omt_ChunkTypeList;

class OMediaFormattedStream : public OMediaStream
{
	public:

	// * Constructor/Destructor

	omtshared OMediaFormattedStream(OMediaStream *stream);
	omtshared virtual ~OMediaFormattedStream();

	// * Chunk
	
	omtshared virtual void open_chunk(omt_ChunkType type, omt_FSChunkID id);
	omtshared virtual void open_chunk(omt_ChunkType type, string name, omt_FSChunkID *id =NULL);

	omtshared virtual void close_chunk(void);

	// Be sure to open a chunk before using stream methods.

	omtshared virtual void delete_chunk(omt_ChunkType type, omt_FSChunkID id);

	omtshared virtual void duplicate_chunk(omt_ChunkType src_type, omt_FSChunkID src_id,
								    omt_ChunkType dest_type, omt_FSChunkID dest_id);
	
	omtshared virtual void import_chunk(OMediaStream *src_stream, long src_size,
								    omt_ChunkType dest_type, omt_FSChunkID dest_id);

	omtshared virtual void export_chunk(omt_ChunkType src_type, omt_FSChunkID src_id,
								     OMediaStream *dest_stream);
	
	// * Chunk name
	
	inline void set_chunk_name(omt_ChunkType type, omt_FSChunkID ckid, string newname)
	{
		set_chunk_name(get_chunk_type_node(type),ckid,newname);
	}

	inline void set_chunk_name(OMediaFSChunkTypeNode *typenode, omt_FSChunkID ckid, string newname)
	{
		omt_ChunkList::iterator ci;
		OMediaFSChunk *chk = get_chunk_iterator(typenode, ckid,ci);
		if (chk)
		{
			update_chkname_FMNames(typenode,ci,newname);
			chk->name = newname;
			set_header_dirty();
		}
	}

	inline void get_chunk_name(omt_ChunkType type, omt_FSChunkID ckid, string &name)
	{
		OMediaFSChunk *chk = get_chunk(type, ckid);
		if (chk) name = chk->name;
		else name = "";	
	
	}

	inline void get_chunk_name(OMediaFSChunkTypeNode *typenode, omt_FSChunkID ckid, string &name)
	{
		OMediaFSChunk *chk = get_chunk(typenode, ckid);
		if (chk) name = chk->name;
		else name = "";	
	}
	
	// * Chunk Compression

	omtshared virtual void set_chunk_compression(omt_ChunkType type, 
												  omt_FSChunkID ckid,
												  omt_CompressionLevel level);

	
	// * Store context
	
	omtshared virtual void store_stream_context(void);
	omtshared virtual void restore_stream_context(void);


	// * Structure

	inline omt_ChunkTypeList *get_chunk_type_list(void) {return &chunk_type_list;}

	omtshared virtual OMediaFSChunkTypeNode *get_chunk_type_node(omt_ChunkType type);
	omtshared virtual OMediaFSChunk *get_chunk(OMediaFSChunkTypeNode *typenode, omt_FSChunkID ckid);
	omtshared virtual OMediaFSChunk *get_chunk(OMediaFSChunkTypeNode *typenode, string name);


	inline OMediaFSChunk *get_chunk(omt_ChunkType type, omt_FSChunkID ckid) {return get_chunk(get_chunk_type_node(type),ckid);}
	inline OMediaFSChunk *get_chunk(omt_ChunkType type, string name) {return get_chunk(get_chunk_type_node(type),name);}


	omtshared virtual omt_FSChunkID get_unique_chunkid(omt_ChunkType type);
	omtshared virtual omt_FSChunkID get_unique_chunkid(omt_ChunkType type, omt_FSChunkID greater_equal_id);

	omtshared OMediaFSChunk *get_chunk_iterator(OMediaFSChunkTypeNode *chunktypenode, 
														 omt_FSChunkID ckid,
														 omt_ChunkList::iterator &ci);

	inline void set_header_dirty(void) {update_header = true;}


	omtshared virtual void write_header(void);	// OMT does it for you when the object is destructed.
										// You can force an update yourself using this method.
										// Please note that if you don't destruct the object,
										// the header will not be updated and your stream
										// can be corrupted! 

	omtshared virtual void rebuild_flat_list(void);

	// * Compression
	
	omtshared virtual void compress(void);	// Compress the stream by trying to remove
									// chunk fragmentation. This method is called
									// by the destructor.

	// Following methods allow you to specify the maximum compression time.
	// For example if you set 5, compression will stop after 5 seconds even if
	// the stream is not completely defragmented. Default is four seconds.
	// Set to zero if you don't want compression.
	inline void set_max_compression_time(long seconds) {max_comp_time = seconds;}
	inline long get_max_compression_time(void) {return max_comp_time;}

	// * Version

	short get_fmtstream_version(void) {return version;}

	protected:
	
	#define omcid_fmtstream 		'0MFS'	// Version 1
	#define omcid_fmtstream_v2 		'0MF2'	// Version 2
	#define omcid_fmtstream_v3 		'0MF3'	// Version 3

	#define omc_fmtcurrent_version	 3
	#define omcid_fmtcurrent_version omcid_fmtstream_v3

	#define omc_static_hdr_offset 	4L
	#define omc_free_region_start 	8L

	#define omcchunk_header		'0HDR'
	#define omcchunkid_header 	'0HID'


	OMediaFSChunk 			*chunk_open;
	OMediaStream			*stream;
	
	OMediaFSBufferedChunk	*buffered_chunk;

	bool					update_header;
	omt_ChunkTypeList		chunk_type_list;
	vector<OMediaFSChunk*>	chunk_flatlist;
	omt_BlockList			chunk_freelist;
	long					max_comp_time;
	short					version;
	
	omt_FMTStreamContextContainer	context_stack;
	
	omtshared virtual	void open_formatstream(void);
	omtshared virtual void save_header(void);
	omtshared virtual void save_header_data(void);

	omtshared virtual void load_header(void);
	omtshared virtual void purge_loaded_header(void);
	omtshared virtual void goto_header(void);

	omtshared virtual void delete_chunktype(OMediaFSChunkTypeNode *chunktypenode);
	omtshared virtual void truncate_chunk(OMediaFSChunk *chunk, unsigned long chunk_offset);
	omtshared virtual void move_chunk_to_end(OMediaFSChunk *chunk);
	omtshared virtual void create_free_block(unsigned long offset, unsigned long size);
	omtshared virtual void move_chunk_to_free_block(OMediaFSChunk *chunk,
										  omt_BlockList::iterator bi,
										  unsigned long size_required);



	// * Stream methods (Override)
	
	omtshared virtual void do_read(void *buffer, unsigned long nbytes);
	omtshared virtual void do_write(void *buffer, unsigned long nbytes);
	omtshared virtual unsigned long do_getsize(void);
	omtshared virtual void do_setsize(unsigned long newsize);
	omtshared virtual void do_setposition(long offset, short relative = omcfr_Start);
	omtshared virtual long do_getposition(void);

	// Low-level stuff

	omtshared virtual void flush_buffer_chunk(void);
	omtshared virtual void init_buffer_chunk(void);

	omtshared virtual OMediaFSChunkTypeNode *create_chunktype(omt_ChunkType type);
	omtshared virtual OMediaFSChunk *create_chunk(OMediaFSChunkTypeNode *typenode, omt_FSChunkID id);

	omtshared virtual vector<OMediaFSChunk*>::iterator find_flatiterator(OMediaFSChunk *chunk);

	omtshared virtual omt_BlockList::iterator search_freeblock_minsize(unsigned long minimal_size);
	omtshared virtual omt_BlockList::iterator search_freeblock_minsize_before(unsigned long offset, 
																	unsigned long minimal_size);

	omtshared virtual omt_BlockList::iterator search_freeblock_offset(unsigned long offset);
	omtshared virtual void union_free_block(OMediaFSBlock	block);

	class compare_flatchunk_offset {public: bool operator() (const OMediaFSChunk *c1, const OMediaFSChunk *c2) const {return c1->offset < c2->offset;}};

	omtshared virtual void move_block(unsigned long src, unsigned long dest, unsigned long size);


	omtshared virtual void update_chkname_FMNames(OMediaFSChunkTypeNode *typenode,
													omt_ChunkList::iterator ci, 
													string &newname);
													
	omtshared virtual void remove_chkname_FMNames(OMediaFSChunkTypeNode *typenode,
													omt_ChunkList::iterator ci);

};


#endif

