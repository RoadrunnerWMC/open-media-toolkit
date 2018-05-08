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
#ifndef OMEDIA_RadioGroup_H
#define OMEDIA_RadioGroup_H

#include "OMediaListener.h"


class OMediaAbstractButton;

class OMediaRadioGroup : public OMediaListener
{
	public:

	// * Construction

	omtshared OMediaRadioGroup();	  
	omtshared virtual ~OMediaRadioGroup();
	
	// * Add/remove button to the group
	
	omtshared virtual void add_button(OMediaAbstractButton *b);
	omtshared virtual void remove_button(OMediaAbstractButton *b);
	

	// * Receive messages

	omtshared virtual void listen_to_message(omt_Message msg, void *param =omc_NULL);



};


#endif

