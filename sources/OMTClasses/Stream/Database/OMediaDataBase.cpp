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

#include <algorithm>

vector<OMediaDBCtorRegistration> *OMediaDataBase::registred_builders;
unsigned long					OMediaDataBase::cache_size = 0xFFFFFFFF;
OMediaBroadcaster				*OMediaDataBase::database_broadcaster;


OMediaDataBase::OMediaDataBase(OMediaStream *stream) : OMediaFormattedStream(stream)
{
	get_database_broadcaster()->addlistener(this);
	memory_used_dirty = true;
	if (cache_size==0xFFFFFFFF) cache_size = 2000000;	// 2megs for default cache size
}

OMediaDataBase::~OMediaDataBase(void)
{
	free_all_mirrors();
	full_purge();
}

void OMediaDataBase::full_purge(void)
{
	OMediaFSChunkTypeNode *chunktypenode;

	// First try to purge them using standard method

	while(purge_objects(0)) {}

	// If some objects are still locked, I
	// purge them from memory in the delete priority order of their types

	vector<OMediaDBCtorRegistration> *list = get_registred_object_list();

	for(vector<OMediaDBCtorRegistration>::iterator di = list->begin();
		di != list->end();
		di++)
	{
		chunktypenode=get_chunk_type_node((*di).type);
		if (chunktypenode)
		{
			for(omt_ChunkList::iterator ci = chunktypenode->get_chunk_list()->begin();
				ci != chunktypenode->get_chunk_list()->end();
				ci++)
			{
				do_purge_object(&(*ci));
			}
		}
	}
}

void OMediaDataBase::full_purge_type(omt_ChunkType ctype)
{
	OMediaFSChunkTypeNode *chunktypenode;

	chunktypenode=get_chunk_type_node(ctype);
	if (chunktypenode)
	{
		for(omt_ChunkList::iterator ci = chunktypenode->get_chunk_list()->begin();
				ci != chunktypenode->get_chunk_list()->end();
				ci++)
		{		
			do_purge_object(&(*ci));
		}
	}
}


void OMediaDataBase::set_object(OMediaDBObject *new_dbo, omt_ChunkType type, omt_FSChunkID id)
{
	OMediaFSChunk 		*chunk;
	OMediaDBObject 		*dbo;
	
	chunk = get_chunk(type, id);
	if (chunk)
	{
		if (chunk->extern_link)
		{
			// Already in memory
			dbo =  (OMediaDBObject *) chunk->extern_link;
			
			if (dbo->db_is_locked()) omd_EXCEPTION(omcerr_DBCantSetLockedObject);
			
			dbo->set_modified(false);
			delete dbo;
		}
	}
	else
	{
		open_chunk(type,id);
		chunk = get_chunk(type, id);
	}

	new_dbo->db_lock();
	new_dbo->db_link(this,chunk,type);
	new_dbo->db_unlock();
	
	new_dbo->set_modified(true);
}

OMediaDBObject *OMediaDataBase::get_object(omt_ChunkType type, string name)
{
	OMediaFSChunk *chunk = get_chunk(type, name);
	OMediaDBObject 		*dbo;

	if (chunk) 
	{
		dbo = get_object_ck(type,chunk);
		dbo->db_lock();
		return dbo;
	}
	else
	{
		// Chunk does not exist, just build a new one

		omt_DBObjectBuilder	builder;
		omt_FSChunkID 		id=0;
	
		open_chunk(type,name,&id);
		chunk = get_chunk(type, id);
		builder = find_builder(type);
		dbo = builder();
		if (!dbo) omd_EXCEPTION(omcerr_OutOfMemory);
		dbo->db_lock();
		dbo->db_link(this,chunk,type);	
		check_cache_full();
		close_chunk();
		
		return dbo;
	}
}

OMediaDBObject *OMediaDataBase::get_object(omt_ChunkType type, omt_FSChunkID id)
{
	OMediaFSChunk *chunk = get_chunk(type, id);
	OMediaDBObject 			*dbo;

	if (chunk) 
	{
		dbo = get_object_ck(type,chunk);
		dbo->db_lock();
		return dbo;
	}
	else
	{
		// Chunk does not exist, just build a new one

		omt_DBObjectBuilder		builder;
		omt_CompressionLevel	clevel;
	
		open_chunk(type,id);
		chunk = get_chunk(type, id);
		builder = find_builder(type);
		dbo = builder();
		if (!dbo) omd_EXCEPTION(omcerr_OutOfMemory);

		dbo->db_lock();

		clevel = dbo->get_chunk_compression();		
		dbo->db_link(this,chunk,type);	
		dbo->set_chunk_compression(clevel);
		
		check_cache_full();
	
		close_chunk();	
		
		return dbo;
	}
}

OMediaDBObject *OMediaDataBase::get_object_ck(omt_ChunkType type, OMediaFSChunk *chunk)
{
	OMediaDBObject 		*dbo;
	omt_DBObjectBuilder	builder;
	
	if (chunk)
	{	
		if (chunk->extern_link)
		{
			// Already in memory
			dbo =  (OMediaDBObject *) chunk->extern_link;
		}
		else
		{
			// I need to load it from stream
			builder = find_builder(type);
			dbo = builder();
			if (!dbo) omd_EXCEPTION(omcerr_OutOfMemory);
			
			dbo->db_lock();
			dbo->db_link(this,chunk,type);


			try
			{
				db_streaming_object = dbo;
				dbo->read_class(*this);
				db_streaming_object = NULL;
			}

			catch(OMediaError err)
			{
				db_streaming_object = NULL;
				if (err.errorcode!=omcerr_EmptyDBChunk) throw err;
			}

			close_chunk();
			
			check_cache_full();
			dbo->db_unlock();
		}
	}
	
	return dbo;
}

void OMediaDataBase::generate_from_stream(OMediaStream &stream, OMediaDBObject *object)
{
	stream.db_streaming_object = object;
	stream>>object;
	stream.db_streaming_object = NULL;
}
	 
void OMediaDataBase::delete_object(omt_ChunkType type, omt_FSChunkID id)
{
	OMediaFSChunk 	*chunk;
	OMediaDBObject 	*dbo;
	
	chunk = get_chunk(type, id);
	if (chunk)
	{
		dbo = (OMediaDBObject *) chunk->extern_link; 

		if (dbo)
		{
			if (dbo->db_is_locked()) return;
			dbo->db_destruct();
			do_purge_object(chunk);
		}
		
		delete_chunk(type, id);
	}
}

void OMediaDataBase::purge_object(OMediaDBObject *dbo, bool break_lock)
{
	if (dbo)
	{
		omt_ChunkType 	type;
		omt_FSChunkID 	id;
		
		if (!dbo->db_is_locked() || break_lock) 
		{
			dbo->get_chunk_info(type, id);
			do_purge_object(get_chunk(type, id));
		}
	}
}

void OMediaDataBase::do_purge_object(OMediaFSChunk	*chunk)
{
	OMediaDBObject *dbo = (OMediaDBObject *) chunk->extern_link;
	if (dbo) delete dbo;
}

static bool purge_order_compare(const OMediaFSChunk *c1, 
								const OMediaFSChunk *c2)
{
	const OMediaDBObject 	*dbo1 = (OMediaDBObject *) c1->extern_link, 
			   				*dbo2 = (OMediaDBObject *) c2->extern_link;

	return (dbo1->get_created_count() < dbo2->get_created_count());
}

void OMediaDataBase::write_objects_all(void)
{
	OMediaFSChunkTypeNode 	*chunktypenode;
	OMediaFSChunk 			*chunk;

	for(omt_ChunkTypeList::iterator cti = chunk_type_list.begin();
		cti != chunk_type_list.end();
		cti++)
	{
		chunktypenode = &(*cti);
	
		for(omt_ChunkList::iterator ci = chunktypenode->get_chunk_list()->begin();
			ci != chunktypenode->get_chunk_list()->end();
			ci++)
		{
			chunk = &(*ci);
			OMediaDBObject *dbo = (OMediaDBObject *) chunk->extern_link;

			if (dbo) dbo->db_update();
		}
	}
}
	
long OMediaDataBase::purge_objects(unsigned long mem_size)
{
	typedef OMediaFSChunk				*OMediaFSChunkPtr;
	typedef vector<OMediaFSChunkPtr>	OMediaFSChunkPtrList;

	OMediaFSChunkPtrList				chunk_list;

	OMediaFSChunkTypeNode 	*chunktypenode;
	OMediaFSChunk 			*chunk;
	OMediaDBObject 			*dbo;
	long					npurged_object = 0;
	unsigned long			mem_count = 0;

	for(omt_ChunkTypeList::iterator cti = chunk_type_list.begin();
		cti != chunk_type_list.end();
		cti++)
	{
		chunktypenode = &(*cti);
	
		for(omt_ChunkList::iterator ci = chunktypenode->get_chunk_list()->begin();
			ci != chunktypenode->get_chunk_list()->end();
			ci++)
		{
			chunk = &(*ci);
			OMediaDBObject *dbo = (OMediaDBObject *) chunk->extern_link;

			if (dbo && !dbo->db_is_locked())
			{
				if (mem_size==0) 
				{
					do_purge_object(chunk);
					npurged_object++;
				}
				else 
				{
					chunk_list.push_back(chunk);
				}
			}
		}
	}
	
	if (mem_size!=0 && chunk_list.size())	
	{
		// I want to purge objects using the creation order. I need to sort the temporary
		// list.
		
		sort(chunk_list.begin(), chunk_list.end(), purge_order_compare);
		
		for(OMediaFSChunkPtrList::iterator i = chunk_list.begin();
			i !=chunk_list.end();
			i++)
		{
			dbo = (OMediaDBObject *) (*i)->extern_link;
			mem_count += dbo->get_approximate_size();
			do_purge_object((*i));
			npurged_object++;
			
			if (mem_count>=mem_size) break;	// It's enough, break the loop
		}
	}
	
	return npurged_object;
}

unsigned long OMediaDataBase::get_memory_used(void)
{
	OMediaFSChunkTypeNode 	*chunktypenode;
	OMediaFSChunk 			*chunk;
	unsigned long			mem_count = 0;

	if (memory_used_dirty)
	{
		for(omt_ChunkTypeList::iterator cti = chunk_type_list.begin();
			cti != chunk_type_list.end();
			cti++)
		{
			chunktypenode = &(*cti);
		
			for(omt_ChunkList::iterator ci = chunktypenode->get_chunk_list()->begin();
				ci != chunktypenode->get_chunk_list()->end();
				ci++)
			{
				chunk = &(*ci);
				OMediaDBObject *dbo = (OMediaDBObject *) chunk->extern_link;

				if (dbo) mem_count += dbo->get_approximate_size();
			}
		}
		
		memory_used_dirty = false;
		memory_used = mem_count;
	}
	else mem_count = memory_used;
	
	return mem_count;
}

unsigned long OMediaDataBase::get_memory_used_all(void)
{
	OMediaBroadcaster	*bc;
	omt_ListenerList	*llist;
	OMediaDataBase	*db;
	unsigned long 		mem_count = 0;

	bc = get_database_broadcaster();
	llist = bc->getlisteners();
	
	for(omt_ListenerList::iterator i = llist->begin();
		i!=llist->end();
		i++)
	{
		db = (OMediaDataBase*) (*i);
		mem_count += db->get_memory_used();
	}
	
	return mem_count;
}

bool OMediaDataBase::object_loaded(omt_ChunkType type, omt_FSChunkID id)
{
	OMediaFSChunk *chunk = get_chunk(type,id);
	return (chunk && chunk->extern_link);
}


void OMediaDataBase::register_object(omt_ChunkType type, omt_DBObjectBuilder builder,
									long delete_priority)
{
	vector<OMediaDBCtorRegistration> *list = get_registred_object_list();

	// Scan the list to see if the type is already registred
	
	for(vector<OMediaDBCtorRegistration>::iterator i = list->begin();
		i!=list->end();
		i++)
	{
		if ((*i).type==type)
		{
			// If already registred, I just update the structure and resort
		
			(*i).builder = builder;
			(*i).delete_priority = delete_priority;
			sort(list->begin(),list->end());	

			return;
		}
	}
	
	// Ok let's build a new entry
	
	OMediaDBCtorRegistration		reg;
	
	reg.type = type;
	reg.builder = builder;
	reg.delete_priority = delete_priority;
	
	list->push_back(reg);
	sort(list->begin(),list->end());	
}

omt_DBObjectBuilder OMediaDataBase::find_builder(omt_ChunkType type)
{
	vector<OMediaDBCtorRegistration> *list = get_registred_object_list();

	for(vector<OMediaDBCtorRegistration>::iterator i = list->begin();
		i!=list->end();
		i++)
	{
		if ((*i).type==type)
		{		
			return (*i).builder;
		}
	}	

	// Required type has not been registred! 
	omd_EXCEPTION(omcerr_DBUnregistredChunkType);
	
	return NULL;
}

static bool quiet_get_list;

vector<OMediaDBCtorRegistration> *OMediaDataBase::get_registred_object_list(void)
{
	if (!registred_builders && !quiet_get_list)
	{
		registred_builders = new vector<OMediaDBCtorRegistration>;	
		if (!registred_builders) omd_EXCEPTION(omcerr_OutOfMemory);
	}
	
	return registred_builders;
}

OMediaBroadcaster	*OMediaDataBase::get_database_broadcaster(void)
{
	if (!database_broadcaster && !quiet_get_list)
	{
		database_broadcaster = new OMediaBroadcaster;
		if (!database_broadcaster) omd_EXCEPTION(omcerr_OutOfMemory);
	}

	return database_broadcaster;
}


void OMediaDataBase::set_dbsize_dirty(void) 
{
	memory_used_dirty = true;
	check_cache_full();

}


long OMediaDataBase::purge_objects_all(unsigned long mem_size)
{
	omt_ListenerList					*llist;
	OMediaDataBase					*db;
	omt_ListenerList::iterator 			i;
	vector<OMediaFSChunk *>				chunk_list;
	OMediaFSChunkTypeNode 				*chunktypenode;
	OMediaFSChunk 					*chunk;
	OMediaDBObject 					*dbo;
	unsigned long						mem_count = 0, obj_count=0;

	llist = get_database_broadcaster()->getlisteners();
	
	if (!mem_size)
	{
		// All objects should be purged
	
		for(i= llist->begin(); i!=llist->end(); i++)
		{
			db = (OMediaDataBase*) (*i);
			obj_count += db->purge_objects(0);
		}
	}
	else
	{
		// Create a temporaty list of all object stored in databases
	
		for(i= llist->begin(); i!=llist->end(); i++)
		{
			db = (OMediaDataBase*) (*i);

			for(omt_ChunkTypeList::iterator cti = db->get_chunk_type_list()->begin();
				cti != db->get_chunk_type_list()->end();
				cti++)
			{
				chunktypenode = &(*cti);

				for(omt_ChunkList::iterator ci = chunktypenode->get_chunk_list()->begin();
					ci != chunktypenode->get_chunk_list()->end();
					ci++)
				{
					chunk = &(*ci);
					OMediaDBObject *dbo = (OMediaDBObject *) chunk->extern_link;

					if (dbo && !dbo->db_is_locked()) chunk_list.push_back(chunk);
				}
			}
		}

		if (chunk_list.size())	
		{
			// I want to purge objects using the creation order. I need to sort the temporary
			// list.
			
			sort(chunk_list.begin(), chunk_list.end(), purge_order_compare);
			
			for(vector<OMediaFSChunk *>::iterator i = chunk_list.begin();
				i !=chunk_list.end();
				i++)
			{
				dbo = (OMediaDBObject *) (*i)->extern_link;
				mem_count += dbo->get_approximate_size();
				delete dbo;
				obj_count++;
				
				if (mem_count>=mem_size) break;	// It's enough, break the loop
			}
		}

	}

	return obj_count;
}

void OMediaDataBase::check_cache_full(void)
{
	static bool		already_checking;

	unsigned long mem;

	if (!cache_size || already_checking) return;
	
	already_checking = true;
	
	mem = get_memory_used_all();
	
	if (cache_size<mem) purge_objects_all(mem-cache_size);
	
	already_checking = false;
}

void OMediaDataBase::set_cache_size(unsigned long cs)
{
	cache_size = cs;
	check_cache_full();
}

bool OMediaDataBase::object_exists(omt_ChunkType type, omt_FSChunkID id)
{
	return (get_chunk(type,id)!=NULL);
}

bool OMediaDataBase::object_exists(omt_ChunkType type, string name)
{
	return (get_chunk(type,name)!=NULL);
}

OMediaDBObject *OMediaDataBase::get_first_object(omt_ChunkType type, omt_DBIterator &db_iterator)
{
	OMediaDBObject	*dbo;

	OMediaFSChunkTypeNode *type_node = get_chunk_type_node(type);

	if (!type_node) return NULL;
	if (type_node->get_chunk_list()->size()==0) return NULL;

	db_iterator.type = type_node;
	db_iterator.iterator = type_node->get_chunk_list()->begin();
	
	dbo = get_object_ck(type, &(*(db_iterator.iterator)));
	dbo->db_lock();
	return dbo;
}

OMediaDBObject *OMediaDataBase::get_next_object(omt_DBIterator &db_iterator)
{
	OMediaDBObject	*dbo;

	db_iterator.iterator++;

	if (db_iterator.iterator == db_iterator.type->get_chunk_list()->end()) return NULL;
	
	dbo =  get_object_ck( db_iterator.type->get_chunk_type(),  &(*(db_iterator.iterator)));
	dbo->db_lock();
	return dbo;
}

OMediaDataBase *OMediaDataBase::get_database(void)
{
	return this;
}

//------ Mirror objects

OMediaDBObject *OMediaDataBase::mirror_object_ck(omt_ChunkType type, OMediaFSChunk *chunk)
{
	OMediaDBObject 		*dbo = NULL,*src_dbo = NULL;
	omt_DBObjectBuilder	builder;
	
	if (chunk)
	{	
		if (chunk->extern_link)
		{
			// If the original object is modified, I have to stream it first
			src_dbo =  (OMediaDBObject *) chunk->extern_link;
			if (src_dbo->get_modified()) src_dbo->db_update();
		}
		
		// I need to load it from stream
		builder = find_builder(type);
		dbo = builder();
		if (!dbo) omd_EXCEPTION(omcerr_OutOfMemory);
			
		dbo->db_lock();
		dbo->db_link(this,chunk,type,true);

		try
		{
			db_streaming_object = dbo;
			dbo->read_class(*this);
			db_streaming_object = NULL;
		}

		catch(OMediaError err)
		{
			db_streaming_object = NULL;
			if (err.errorcode!=omcerr_EmptyDBChunk) throw err;
		}

		close_chunk();
		
		mirrors.push_back(dbo);
		dbo->mirror_iterator = --(mirrors.end());
	}
	
	return dbo;
}

OMediaDBObject *OMediaDataBase::mirror_object(omt_ChunkType type, string name)
{
	return mirror_object_ck(type, get_chunk(type, name));
}

OMediaDBObject *OMediaDataBase::mirror_object(omt_ChunkType type, omt_FSChunkID id)
{
	return mirror_object_ck(type, get_chunk(type, id));
}

void OMediaDataBase::unlink_mirror(OMediaDBObject *dbo)
{
	if (!dbo->get_db_mirror() || dbo->get_database()!=this) return;
	
	mirrors.erase(dbo->mirror_iterator);
	dbo->database = NULL;
	dbo->chunk = NULL;
	dbo->db_mirror = false;
}
		
void OMediaDataBase::free_all_mirrors(void)
{
	while(mirrors.size()) delete *(mirrors.begin());
}


//------ Registration list auto-destructor

class OMediaDBRegListAutoDtor
{
	public:

	OMediaDBRegListAutoDtor();
	virtual ~OMediaDBRegListAutoDtor();

	static OMediaDBRegListAutoDtor destroy;
};

OMediaDBRegListAutoDtor OMediaDBRegListAutoDtor::destroy;
OMediaDBRegListAutoDtor::OMediaDBRegListAutoDtor() {}
OMediaDBRegListAutoDtor::~OMediaDBRegListAutoDtor() 
{
	quiet_get_list = true;
	delete OMediaDataBase::get_registred_object_list();	
	delete OMediaDataBase::get_database_broadcaster();
}
