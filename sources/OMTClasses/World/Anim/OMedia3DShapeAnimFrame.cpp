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
 

#include "OMedia3DShapeAnimFrame.h"
#include "OMedia3DShape.h"

OMedia3DShapeAnimFrame::OMedia3DShapeAnimFrame()
{
	shape = omc_NULL;
}

OMedia3DShapeAnimFrame::~OMedia3DShapeAnimFrame()
{
	if (shape) shape->db_unlock();
}

void OMedia3DShapeAnimFrame::set_shape(OMedia3DShape *s)
{
	if (shape) shape->db_unlock();
	shape =s;
	if (shape) shape->db_lock();
}

void OMedia3DShapeAnimFrame::read_class(OMediaStreamOperators &stream)
{
	OMediaDBObjectStreamLink	slink;

	OMediaAnimFrame::read_class(stream);

	stream>>slink;
	set_shape((OMedia3DShape*)slink.get_object());
}

void OMedia3DShapeAnimFrame::write_class(OMediaStreamOperators &stream)
{
	OMediaDBObjectStreamLink	slink;

	OMediaAnimFrame::write_class(stream);

	slink.set_object(shape);
	stream<<slink;
}


