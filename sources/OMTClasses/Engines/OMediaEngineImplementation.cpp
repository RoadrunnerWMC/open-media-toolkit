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
 

#include "OMediaEngineImplementation.h"
#include "OMediaError.h"

//--------------------------------------------------

OMediaEngineImplementation::OMediaEngineImplementation()
{
	locked_flags = 0;
	lock_count = 0;
}

OMediaEngineImplementation::~OMediaEngineImplementation()
{
}

void OMediaEngineImplementation::lock(omt_LockFlags flags)
{
	lock_count++;
	if (lock_count==1) locked_flags = flags;	
	else locked_flags |=flags;
}

void OMediaEngineImplementation::unlock(void)
{
	if (lock_count==0) omd_STREXCEPTION("Unbalanced OMediaEngineImplementation lock count");

	lock_count--;
	if (lock_count==0)
	{
		locked_flags = 0;
	}
}

unsigned long OMediaEngineImplementation::get_approximate_size(void)
{
	return 0;
}
	
//--------------------------------------------------

OMediaEngineImpMaster::OMediaEngineImpMaster() {}

OMediaEngineImpMaster::~OMediaEngineImpMaster()
{
	delete_imp_slaves();
}

OMediaEngineImpSlave *OMediaEngineImpMaster::find_implementation(OMediaEngine *engine, 
					unsigned long key_long,
					void		  *key_ptr,
					bool 		  key_long_flags)
{	
	for(omt_EngineImpSlaveList::iterator i = slaves.begin();
		i!=slaves.end();
		i++)
	{
		if (key_long_flags)
		{
			if ((*i)->get_engine()==engine &&
				((*i)->get_slave_key_long()&key_long)!=0 &&
				(*i)->get_slave_key_ptr()==key_ptr) return (*i);
		}
		else
		{
			if ((*i)->get_engine()==engine &&
				(*i)->get_slave_key_long()==key_long &&
				(*i)->get_slave_key_ptr()==key_ptr) return (*i);
		}
	}
	
	return NULL;
}

void OMediaEngineImpMaster::delete_imp_slaves(void)
{
	while(slaves.size()) delete *(slaves.begin());
}

void OMediaEngineImpMaster::lock(omt_LockFlags flags)
{
	OMediaEngineImplementation::lock(flags);
}

void OMediaEngineImpMaster::unlock(void)
{
	if (lock_count==1 && locked_flags&omlf_Write) update_imp_slaves();
	OMediaEngineImplementation::unlock();
}


void OMediaEngineImpMaster::update_imp_slaves(void)
{
	for(omt_EngineImpSlaveList::iterator i = slaves.begin();
		i!=slaves.end();
		i++)
	{
		(*i)->master_modified();
	}
}


//--------------------------------------------------

OMediaEngineImpSlave::OMediaEngineImpSlave(	OMediaEngine *engine, 
											OMediaEngineImpMaster *master)
{
	slave_state = omeisc_Empty;
	slave_solid = false;
	slave_key_long = 0;
	slave_key_ptr = NULL;

	this->master = master;
	master->get_imp_slaves()->push_back(this);
	master_iterator = master->get_imp_slaves()->end();
	master_iterator--;

	this->engine = engine;
	engine->get_implementation_list()->push_back(this);
	engine_iterator = engine->get_implementation_list()->end();
	engine_iterator--;
}

OMediaEngineImpSlave::~OMediaEngineImpSlave()
{
	master->get_imp_slaves()->erase(master_iterator);
	engine->get_implementation_list()->erase(engine_iterator);
}


void OMediaEngineImpSlave::master_modified(void) {}

