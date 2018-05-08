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


 

#ifndef OMEDIA_DBObject_H
#define OMEDIA_DBObject_H

#include "OMediaTypes.h"
#include "OMediaFormattedStream.h"
#include "OMediaClassStreamer.h"

#include <list>

class OMediaDataBase;

// -- Database object

class OMediaDBObject : public virtual OMediaClassStreamer
{
	public:

	// * Construction

	omtshared OMediaDBObject(void);
	omtshared virtual ~OMediaDBObject(void);

	inline OMediaDataBase *get_database(void) {return database;}

	omtshared virtual void db_destruct(void);		// Called when the object is deleted from its
													// database.

	omtshared virtual unsigned long db_get_type(void) const;


	// * Lock/unlock

	inline void db_lock(void) {db_locked_count++;}		// When locked the object
	inline void db_unlock(void) {if (db_locked_count>0) db_locked_count--;}		// can't be purged.

	inline bool db_is_locked(void) {return (db_locked_count!=0);}


	// * Size in memory (should be overrided)

	omtshared virtual unsigned long get_approximate_size(void);	// Should return the approximate
														// size used by this object. It
														// should represent the number
														// of bytes that can be get when
														// the object is deleted.

	omtshared virtual void approximate_size_changed(void);	// You must call this method when the approximate
											// size of your object has been mofidied. To obtain
											// the best speed result, the database does not
											// recalculate its size as long as no new object
											// is created and no object changes its size.

	// * Chunk

	inline void get_chunk_info(omt_ChunkType &type, omt_FSChunkID &ckid) const {type = chunk_type; ckid = (chunk)?chunk->ckid:0;}
	inline void get_chunk_info(omt_ChunkType &type, omt_FSChunkID &ckid, string &name) const
	{
		type = chunk_type; 
		if (chunk)
		{
			ckid = chunk->ckid;
			name = chunk->name;
		}
		else
		{
			ckid = 0;
			name = "";
		}
	}

	inline omt_FSChunkID get_chunk_ID(void) const {return (chunk)?chunk->ckid:0;}

	omtshared virtual void set_chunk_name(string newname);
	inline void get_chunk_name(string &name) const {if (chunk) name = chunk->name; else name = "";}

		// Modified flag must be set if you want the compression level to be updated
	inline void set_chunk_compression(const omt_CompressionLevel level) {compression_level = level;}
	inline omt_CompressionLevel get_chunk_compression(void) const {if (chunk) return chunk->compression; else return 0;}


	// * Modified

	inline void set_modified(bool m = true) {modified = m;}		// If set the object will
	inline bool get_modified(void) const {return modified;}		// be saved when the database
																// is deleted or when
																// it is purged. Please note
																// that it is *not* saved
																// if you delete it yourself
																// using the "delete"
																// operator.

	// * Update
	
	omtshared virtual void db_update(void);			// If modified flag is set, update object
											// to stream.
						
						// *!* If you override the "write_class" method, you must call
						// "db_update" in your destructor. Or else the object will
						// no be properly saved when purged. This is because higher inheritage 
						// levels already have been deleted when the OMediaDBObject destructor
						// is called.


	// * Read/Write the class
	
	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	// * Disable link with database for read/write operation. Should be used carefully!

	inline void set_db_link_disabled(bool d) {db_disable_link = d;}
	inline bool get_db_link_disabled(void) const {return db_disable_link;}

	// * Is it a mirror object
	
	inline bool get_db_mirror(void) const {return db_mirror;}


	// * Low-level stuff
	
	inline unsigned long get_created_count(void) const {return created_count;}

	omtshared virtual void db_link(OMediaDataBase	*database, 
									OMediaFSChunk *chunk, 
									omt_ChunkType	chunk_type,
									bool			isMirror =false);


	protected:
	
	friend class OMediaDataBase;
	
	bool					modified,db_disable_link,db_mirror;
	OMediaFSChunk			*chunk;
	omt_ChunkType			chunk_type;
	long					db_locked_count;
	unsigned long			created_count;
	OMediaDataBase			*database;

	omt_CompressionLevel				compression_level;
	list<OMediaDBObject*>::iterator		mirror_iterator;	
};


class OMediaDBObjectStreamLink : public OMediaClassStreamer
{
	public:

	omtshared OMediaDBObjectStreamLink();
	omtshared virtual ~OMediaDBObjectStreamLink();

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	inline void set_object(OMediaDBObject *o) 
	{
		if (object) object->db_unlock();
		object = o;
		if (object) object->db_lock();
	}
	
	inline OMediaDBObject *get_object(void) {return object;}

	protected:

	OMediaDBObject	*object;	
};

typedef OMediaDBObject * (*omt_DBObjectBuilder)(void);

#endif

