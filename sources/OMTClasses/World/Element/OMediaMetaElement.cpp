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

#include "OMediaMetaElement.h"

OMediaMetaElement::OMediaMetaElement()
{
    element = NULL;
}

OMediaMetaElement::~OMediaMetaElement()
{
    db_update();
    if (element) element->db_unlock();
}

void OMediaMetaElement::reset(void)
{
    OMediaElement::reset();

    set_element(NULL);
}

void OMediaMetaElement::set_element(OMediaElement *el)
{
	if (element) element->db_unlock();
	element = el;
	if (element) element->db_lock();
}
	
OMediaDBObject *OMediaMetaElement::db_builder(void)
{
	return new OMediaMetaElement;
}

void OMediaMetaElement::read_class(OMediaStreamOperators &stream)
{
    OMediaDBObjectStreamLink	slink;

    OMediaElement::read_class(stream);
		
    stream>>slink;
    set_element((OMediaElement*)slink.get_object());
}

void OMediaMetaElement::write_class(OMediaStreamOperators &stream)
{
    OMediaDBObjectStreamLink	slink;

    OMediaElement::write_class(stream);

    slink.set_object(element);
    stream<<slink;
}

unsigned long OMediaMetaElement::get_approximate_size(void)
{
    return OMediaElement::get_approximate_size();
}

unsigned long OMediaMetaElement::db_get_type(void)
{
	return OMediaMetaElement::db_type;
}

void OMediaMetaElement::render(OMediaRendererInterface 			*rdr_i,
				OMediaRenderHTransform 			&super_hxform,
				OMediaMatrix_4x4 			&viewmatrix,
				OMediaMatrix_4x4	 		&projectmatrix,
                                omt_LightList	 			*lights,
				omt_RenderPreSortedElementList		*presort,
				omt_RenderModeElementFlags		render_flags)
{
    if (element) element->link(this);
    
    OMediaElement::render(rdr_i,super_hxform,viewmatrix,projectmatrix,lights,presort,render_flags);

    if (element) element->unlink();
}                                

