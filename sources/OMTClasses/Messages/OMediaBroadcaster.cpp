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
 

#include "OMediaBroadcaster.h"


OMediaBroadcaster::OMediaBroadcaster(void)
{
}

OMediaBroadcaster::~OMediaBroadcaster(void)
{
	for(omt_ListenerList::iterator	i = listeners.begin();
		i != listeners.end();
		i++)
	{
		omt_BroadcasterList::iterator nbi;
	
		for(omt_BroadcasterList::iterator bi = (*i)->getbroadcasters()->begin();
			bi != (*i)->getbroadcasters()->end();)
		{
			nbi = bi; nbi++;
		
			if ((*bi).broadcaster == this) (*i)->getbroadcasters()->erase(bi);
		
			bi = nbi;
		}
	}
}

void OMediaBroadcaster::removelistener(OMediaListener *l)
{
	omt_ListenerList::iterator	ni;

	for(omt_ListenerList::iterator	i = listeners.begin();
		i != listeners.end();)
	{
		ni = i; ni++;
	
		if ((*i)==l)
		{
			omt_BroadcasterList::iterator nbi;
	
			for(omt_BroadcasterList::iterator bi = (*i)->getbroadcasters()->begin();
				bi != (*i)->getbroadcasters()->end();)
			{
				nbi = bi; nbi++;
		
				if ((*bi).broadcaster == this) (*i)->getbroadcasters()->erase(bi);
		
				bi = nbi;
			}
			
			listeners.erase(i);
		}
		
		i = ni;
	}
}

void OMediaBroadcaster::addlistener(OMediaListener *l)
{
	OMediaBroadcasterLink			blink;
	omt_BroadcasterList::iterator	i;
	omt_ListenerList::iterator		li;

	l->getbroadcasters()->push_back(blink);
	i = --(l->getbroadcasters()->end());

	listeners.push_back(l);
	li = --(listeners.end());
	
	(*i).node = li;
	(*i).broadcaster = this;
}

void OMediaBroadcaster::broadcast_message(omt_Message msg, void *param)
{
	omt_ListenerList::iterator i,ni;
	OMediaListener	*l;
	
	for(i = listeners.begin();
		i != listeners.end();)
	{
		ni = i; ni++;
		
		l = (*i);
	
		l->clear_abort();
		l->listen_to_message(msg, param);
		if (l->check_aborted()) break;
	
		i = ni;
	}
}
