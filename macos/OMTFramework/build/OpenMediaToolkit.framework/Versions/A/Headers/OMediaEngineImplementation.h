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
#ifndef OMEDIA_EngineImplementation_H
#define OMEDIA_EngineImplementation_H

#include "OMediaEngine.h"

class OMediaEngineImpMaster;

typedef unsigned short omt_LockFlags;
const omt_LockFlags omlf_Read  = (1<<0);
const omt_LockFlags omlf_Write = (1<<1);


// * Abstract implementation

class OMediaEngineImplementation
{
	public:

	// * Construction

	OMediaEngineImplementation();
	virtual ~OMediaEngineImplementation();

	
	// * Lock data

	omtshared virtual void lock(omt_LockFlags flags);
	omtshared virtual void unlock(void);
	
	inline bool is_locked(void) const {return lock_count!=0;}
	inline omt_LockFlags get_locked_flags(void) const {return locked_flags;}

	// * DB support

	omtshared virtual unsigned long get_approximate_size(void);

	protected:
	
	omt_LockFlags							locked_flags;
	long									lock_count;
};


// * Slave implementation

enum omt_EngineSlaveImpState
{
	omeisc_Empty,
	omeisc_Copy
};

class OMediaEngineImpSlave : public OMediaEngineImplementation
{
	public:
	
	// * Construction
	
	OMediaEngineImpSlave(OMediaEngine *engine, OMediaEngineImpMaster *master);
	virtual ~OMediaEngineImpSlave();	

	// * Attributes

	inline bool is_solid_implementation(void) const {return slave_solid;}
	inline OMediaEngineImpMaster *get_imp_master(void) const {return master;}
	inline OMediaEngine *get_engine(void) const {return engine;}

	inline void *get_slave_key_ptr(void) const {return slave_key_ptr;}
	inline unsigned long get_slave_key_long(void) const {return slave_key_long;}

	omtshared virtual void master_modified(void);

	
	protected:

	inline void set_solid_implementation(bool s) {slave_solid = s;}
	
	omt_EngineImpSlaveList::iterator		engine_iterator;
	omt_EngineImpSlaveList::iterator		master_iterator;
	OMediaEngineImpMaster					*master;
	OMediaEngine							*engine;
	unsigned long							slave_key_long;
	void									*slave_key_ptr;
	bool									slave_solid;
	omt_EngineSlaveImpState					slave_state;

};


// * Master implementation

class OMediaEngineImpMaster : public OMediaEngineImplementation
{
	public:

	// * Construction

	OMediaEngineImpMaster();
	virtual ~OMediaEngineImpMaster();

	// * Slave

	omtshared OMediaEngineImpSlave *find_implementation(OMediaEngine *engine, unsigned long key_long, void *key_ptr, bool key_long_flags =false);
	inline omt_EngineImpSlaveList *get_imp_slaves(void) {return &slaves;}
	omtshared virtual void delete_imp_slaves(void);

	omtshared virtual void update_imp_slaves(void);

	omtshared virtual void lock(omt_LockFlags flags);
	omtshared virtual void unlock(void);


	public:

	omt_EngineImpSlaveList			slaves;
};



#endif

