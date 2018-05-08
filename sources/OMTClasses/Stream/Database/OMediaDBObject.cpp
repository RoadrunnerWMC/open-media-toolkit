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
 
#include "OMediaDataBase.h"
#include "OMediaError.h"

#include <vector>

OMediaDBObject::OMediaDBObject(void)
{
	static unsigned long count;

	modified = false;
	chunk = NULL;
	chunk_type = 'null';
	db_locked_count = 0;
	created_count = count++;
	database = NULL;
	db_disable_link = false;
	compression_level = 0;
	db_mirror = false;
}

OMediaDBObject::~OMediaDBObject(void)
{
	if (db_mirror)
	{
		if (database) database->unlink_mirror(this);
	}
	else
	{
		db_update();	// Be sure to add this call to your destructor if you
						// override the "write_class" method!
							
		db_lock();
		approximate_size_changed();
		db_unlock();

		if (chunk) chunk->extern_link = NULL;
	}	
}

void OMediaDBObject::db_destruct(void) 
{
}

unsigned long OMediaDBObject::db_get_type(void) const
{
	return 0;
}

unsigned long OMediaDBObject::get_approximate_size(void)
{
	// By default, I simply return the size of the structure
	
	return sizeof(OMediaDBObject);
}

void OMediaDBObject::approximate_size_changed(void)
{
	if (database) database->set_dbsize_dirty();
}


void OMediaDBObject::read_class(OMediaStreamOperators &stream)
{
	if (database && !db_disable_link && &stream==database)
	{
		omt_ChunkType type; 
		omt_FSChunkID id;
		
		
		get_chunk_info(type, id);
		database->open_chunk(type,id);
		if (database->getsize()==0) throw OMediaError(omcerr_EmptyDBChunk);	// This exception will be catched by DB
	}
}

void OMediaDBObject::write_class(OMediaStreamOperators &stream)
{
	if (database && !db_disable_link && &stream==database)
	{
		omt_ChunkType type; 
		omt_FSChunkID id;
		
		get_chunk_info(type, id);
		database->open_chunk(type,id);
		database->setsize(0);
		modified = false;
	}
}

void OMediaDBObject::db_update(void)
{
	if (database && modified && !db_mirror) 
	{
		if (chunk && compression_level!=chunk->compression)
		{
			database->set_chunk_compression(chunk_type,chunk->ckid,compression_level);
		}

		database->db_streaming_object = this;
		write_class(*database);
		database->db_streaming_object = NULL;
	}
}

void OMediaDBObject::db_link(OMediaDataBase	*db, OMediaFSChunk *c, omt_ChunkType ct,
									bool			isMirror)
{
	if (chunk && chunk->extern_link && !db_mirror) chunk->extern_link = NULL;

	db_mirror = isMirror;
	database = db;
	chunk = c;
	chunk_type = ct;
	compression_level = chunk->compression;
	if (!db_mirror) chunk->extern_link = this;
}

void OMediaDBObject::set_chunk_name(string newname)
{
	if (chunk && database && !db_mirror)
	{
		chunk->name = newname;
		database->set_header_dirty();
	}
}



// Stream link

OMediaDBObjectStreamLink::OMediaDBObjectStreamLink()
{
	object = NULL;
}

OMediaDBObjectStreamLink::~OMediaDBObjectStreamLink()
{
	if (object) object->db_unlock();
}

void OMediaDBObjectStreamLink::read_class(OMediaStreamOperators &stream)
{
	OMediaDataBase	*database;
	OMediaDBObject	*context_object;

	context_object = stream.get_db_streaming_object();
	if (context_object) database = context_object->get_database();

	bool	filled;

	set_object(NULL);

	stream>>filled;
	if (filled)
	{
		unsigned long lt;
		long li;
		omt_ChunkType type;
		omt_FSChunkID id;
		
		stream>>lt;
		stream>>li;
		type = omt_ChunkType(lt);
		id = omt_FSChunkID(li);

		if (database && database->object_exists(type,id))
		{
			database->store_stream_context();
			object = database->get_object(type,id);
			database->restore_stream_context();
		}
	}
}

void OMediaDBObjectStreamLink::write_class(OMediaStreamOperators &stream)
{
	bool	filled;
	OMediaDataBase	*database;
	OMediaDBObject	*context_object;

	context_object = stream.get_db_streaming_object();
	if (context_object) database = context_object->get_database();

	filled =  (object && database);
	if (filled && object->get_database()!=database) filled = false;

	stream<<filled;
	if (filled)
	{
		unsigned long lt;
		long li;
		omt_ChunkType type;
		omt_FSChunkID id;
		object->get_chunk_info(type, id);
		lt = long(type);
		li = long(id);
		stream<<lt;
		stream<<li;
	}
	
}
