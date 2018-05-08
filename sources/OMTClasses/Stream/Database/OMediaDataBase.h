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
#ifndef OMEDIA_DataBase_H
#define OMEDIA_DataBase_H

#include "OMediaTypes.h"
#include "OMediaFormattedStream.h"
#include "OMediaClassStreamer.h"
#include "OMediaDBObject.h"
#include "OMediaBroadcaster.h"
#include "OMediaSupervisor.h"

#include <vector>


//**********************************************************
// ** DB object identity

class OMediaDBObjId
{
	public:

	OMediaDBObjId() {}
	OMediaDBObjId(omt_ChunkType t, omt_FSChunkID i) {type = t; ckid = i;}

	omt_ChunkType type;
	omt_FSChunkID ckid;
};


//**********************************************************
// ** Internal stuff

// Chunk type  registration structure


class OMediaDBCtorRegistration
{
	public:

	bool operator<(const OMediaDBCtorRegistration& x) const
	{
		return (x.delete_priority < delete_priority);
	}

	omt_DBObjectBuilder		builder;
	omt_ChunkType 			type;
	unsigned long			delete_priority;
};

// Object iterator (should be used as a black box)

class omt_DBIterator
{
	public:	
	OMediaFSChunkTypeNode	*type;
	omt_ChunkList::iterator	iterator;
};

//**********************************************************
// ** Database class

class OMediaDataBase :  public OMediaListener, public OMediaFormattedStream
{
	public:

	// * Construction

	omtshared OMediaDataBase(OMediaStream *stream);
	omtshared virtual ~OMediaDataBase(void);
	
	
	// * Registration
	
	omtshared static void register_object(omt_ChunkType type, omt_DBObjectBuilder builder,
										  long delete_priority =0);
		// To allow databases to build the correct object for a specified chunk type
		// you should register one class builder per type using this method.
		// This is a static method and registred classes are recognized by all
		// database objects.
		// The "delete_priority" is used when database is deleted to know what object should
		// type should be deleted first. This can be to resolve object dependencies (for
		// example a 3D texture material must be deleted before its linked canvas).

	omtshared static 	vector<OMediaDBCtorRegistration> *get_registred_object_list(void);
	omtshared static omt_DBObjectBuilder find_builder(omt_ChunkType type);
	
	
	// * Manage objects

	omtshared virtual OMediaDBObject *get_object(omt_ChunkType type, omt_FSChunkID id);
		// Get an object from database. If the object does not exist it is
		// created. Object is empty and will not be saved if you don't set
		// the modified flag. The object is automatically db_locked when you call this
		// method (in other words, each time you call this method, the lock count
		// of the object is incremented once). You should call db_unlock when you don't
		// need the object anymore.

	omtshared virtual OMediaDBObject *get_object(omt_ChunkType type, string name);
		// Same than precedent method but works with chunk name. Please note that several chunks
		// can have the same name (only IDs are unique). So this method returns the first object
		// found with specified name. If object does not exist, it is created with a new
		// unique id. The object is automatically db_locked when you call this
		// method (in other words, each time you call this method, the lock count
		// of the object is incremented once). You should call db_unlock when you don't
		// need the object anymore.


	omtshared virtual void set_object(OMediaDBObject *, omt_ChunkType type, omt_FSChunkID id);
		// Link the following object to the database. If there was a previous version of the object, it
		// is replaced by the new one. If the previous object was in memory and locked the function
		// will fail. The passed object is marked "modified" and will be saved to the stream when
		// it is deleted.
	
	omtshared virtual bool object_exists(omt_ChunkType type, omt_FSChunkID id);
		// Returns true if the specified object exists in the database.

	omtshared virtual bool object_exists(omt_ChunkType type, string name);
		// Returns true if at least one object with specified name exists.

	
	omtshared virtual void delete_object(omt_ChunkType type, omt_FSChunkID id);
		// Object is removed from the database. If object is in memory it is
		// purged. Do nothing if object is locked.

	omtshared virtual bool object_loaded(omt_ChunkType type, omt_FSChunkID id);	
		// Returns true if the specified object is loaded

	omtshared virtual void generate_from_stream(OMediaStream &stream,
												OMediaDBObject *object);
		// This method (re)build object using data from a stream.
	

	// * Write all modified objets

	omtshared virtual void write_objects_all(void);

	
	// * Iterate objects of a specified type (return NULL when last object reached).
	// Objects are db_locked at each iteration. So don't forget to call db_unlock after
	// each get_first_object and get_next_object calls.
	
	omtshared virtual  OMediaDBObject *get_first_object(omt_ChunkType type, omt_DBIterator &db_iterator);
	omtshared virtual  OMediaDBObject *get_next_object(omt_DBIterator &db_iterator);
	

	// * Purge object(s) from memory
	
	omtshared virtual void purge_object(OMediaDBObject *dbo,bool break_lock =false);
															// Purge one object. If "break_lock"
															// is true, object is deleted
															// even if it's locked. If the
															// object is marked as modified
															// it is saved. Unlike
															// the "delete" operator this
															// method takes care of
															// of locked state.
	
	omtshared virtual long purge_objects(unsigned long size);	// Purge unlocked objects until
													// required size has been reached.
													// If size is zero, all unlocked
													// objects are purged. Returns the
													// number of objects deleted. Last
													// loaded objects are the last
													// deleted.


	omtshared virtual void full_purge(void);		// All objects are purged, locked or unlocked.
	omtshared virtual void full_purge_type(omt_ChunkType ctype); // Same but only for a specific chunk type
												

	omtshared static long purge_objects_all(unsigned long size);	// Same as "purge_objects" but for all opened databases
	
	// * Cache
	
	omtshared static void set_cache_size(unsigned long cache_size);				// The cache size represents the
	static inline unsigned long get_cache_size( void)  {return cache_size;}		// amount of memory that can be
																// used before databases start
																// to purge objects. If set to zero,
																// database never purges objects itself.
																// If not zero, database checks each
																// time a new object is loaded
																// that the cache has not been filled.
																// Please note that the cache is
																// not defined for one database but
																// for *all* database objects.
																// Default is 500K.

	omtshared static void check_cache_full(void);		// This methods verify that the cache is not full. If it's full, it starts
									// to purge objects from all opened databases.
	

	// * Memory
	
	omtshared virtual unsigned long get_memory_used(void);
		// Memory used by the objects in this database

	omtshared static unsigned long get_memory_used_all(void);
		// Memory used by all databases

	omtshared virtual void set_dbsize_dirty(void);
		// Force database to recalculate its size and check if objects should be purged
	
	
	// * All created databases are listener of the following static broadcaster object:

	omtshared static OMediaBroadcaster	*get_database_broadcaster(void);

	
	// * Return this

	omtshared virtual OMediaDataBase *get_database(void);


	// * Mirror

	omtshared virtual OMediaDBObject *mirror_object(omt_ChunkType type, string name);
	omtshared virtual OMediaDBObject *mirror_object(omt_ChunkType type, omt_FSChunkID id);
		// These methods return a clone of the specified object. The returned object has the same ID 
		// than the original, but it is not an object of the database (it is not saved, removed from cache, etc.). 
		// However all mirror objects are deleted when the database is closed or when the free_all_mirrors method
		// is called. You can also directly free a mirror object using the standard "delete" operator.
		// Mirror objects are useful when you want to use several copies of the same object without having to duplicate it.

	omtshared virtual void unlink_mirror(OMediaDBObject *clone);	// Unlink the clone object from the database. It becomes an
																	// independant object.
		

	omtshared virtual void free_all_mirrors(void);


	protected:
	
	friend class OMediaDBObject;
	
	omtshared static unsigned long						cache_size;
	unsigned long									memory_used;
	bool										memory_used_dirty;
	
	list<OMediaDBObject*>						mirrors;

	omtshared virtual OMediaDBObject *mirror_object_ck(omt_ChunkType type, OMediaFSChunk *chunk);
	
	omtshared virtual void do_purge_object(OMediaFSChunk	*chunk);
	
	omtshared virtual OMediaDBObject *get_object_ck(omt_ChunkType type, OMediaFSChunk *chunk);

	
	omtshared static vector<OMediaDBCtorRegistration>	*registred_builders;
	omtshared static OMediaBroadcaster					*database_broadcaster;
};

#define omd_MIRROROBJECT(db,cl,id) ((cl*)db->mirror_object(cl::db_type,id))
#define omd_GETOBJECT(db,cl,id) ((cl*)db->get_object(cl::db_type,id))
#define omd_REGISTERCLASS(cl) OMediaDataBase::register_object(cl::db_type, cl::db_builder)


#endif

