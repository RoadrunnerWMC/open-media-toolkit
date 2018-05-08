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
 

#include "OMediaTypes.h"
#include "OMediaElementContainer.h"
#include "OMediaElement.h"
#include "OMediaDataBase.h"

OMediaElementContainer::OMediaElementContainer()
{
}

OMediaElementContainer::~OMediaElementContainer()
{
	db_update();
	while(elements.size())  delete *(elements.begin());
}

void OMediaElementContainer::reset(void)
{
	while(elements.size())  delete *(elements.begin());
}


OMediaElement *OMediaElementContainer::search_by_id(long id)
{
	OMediaElement	*e;

	for(omt_ElementList::iterator i=elements.begin();
		i!=elements.end();
		i++)
	{
		if ( (*i)->get_id()==id) return (*i);
		e = (*i)->search_by_id(id);
		if (e) return e;
	}

	return NULL;
}

OMediaDBObject *OMediaElementContainer::db_builder(void)
{
	return new OMediaElementContainer;
}

void OMediaElementContainer::read_class(OMediaStreamOperators &stream)
{
	unsigned long				db_type;
	long						l,v;
	omt_DBObjectBuilder			builder;
	OMediaElement				*element;			

	reset();

	OMediaDBObject::read_class(stream);
	
	stream>>v;
	stream>>l;
	while(l--)
	{
		stream>>db_type;
		builder = OMediaDataBase::find_builder(db_type);
		element = (OMediaElement*)builder();
		element->link(this);
		element->read_class(stream);
	}
}

void OMediaElementContainer::write_class(OMediaStreamOperators &stream)
{
	long	l,v=0;
	unsigned long	db_type;

	OMediaDBObject::write_class(stream);

	l = elements.size();
	stream<<v;
	stream<<l;
	for(omt_ElementList::iterator i=elements.begin();
		i!=elements.end();
		i++)
	{
		db_type = (*i)->db_get_type();
		stream<<db_type;
		(*i)->write_class(stream);
	}
}

unsigned long OMediaElementContainer::get_approximate_size(void)
{
	long	siz = sizeof(*this);
	for(omt_ElementList::iterator i=elements.begin();
		i!=elements.end();
		i++)
	{
		siz += (*i)->get_approximate_size();
	}

	return siz;
}

unsigned long OMediaElementContainer::db_get_type(void)
{
	return OMediaElementContainer::db_type;
}

void OMediaElementContainer::container_link_element(OMediaElement *e) {}

