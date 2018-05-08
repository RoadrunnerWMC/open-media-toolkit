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

#include "OMediaCanvasAnim.h"
#include "OMediaError.h"
#include "OMediaCanvas.h"
#include "OMediaCanvasAnimFrame.h"

//-------------------------------------------------------------------

OMediaCanvasAnimDef::OMediaCanvasAnimDef()
{
}

OMediaCanvasAnimDef::~OMediaCanvasAnimDef()
{
	db_update();
	purge();
}

OMediaAnimFrame *OMediaCanvasAnimDef::create_frame(long sequence)
{
	OMediaAnimFrame *frame = new OMediaCanvasAnimFrame;
	frame->link(this,sequence);
	return frame;
}


OMediaDBObject *OMediaCanvasAnimDef::db_builder(void)
{
	return new OMediaCanvasAnimDef;
}

void OMediaCanvasAnimDef::read_class(OMediaStreamOperators &stream)
{
	OMediaAnimDef::read_class(stream);
}

void OMediaCanvasAnimDef::write_class(OMediaStreamOperators &stream)
{
	OMediaAnimDef::write_class(stream);
}

unsigned long OMediaCanvasAnimDef::get_approximate_size(void)
{
	unsigned long siz = sizeof(*this);
	for(vector<omt_FrameList*>::iterator i=sequences.begin();
		i!=sequences.end();
		i++)
	{
		siz += (*i)->size() * sizeof(OMediaCanvasAnimFrame);
	}

	return siz;
}

unsigned long OMediaCanvasAnimDef::db_get_type(void) const
{
	return OMediaCanvasAnimDef::db_type;
}

//-------------------------------------------------------------------


OMediaCanvasAnim::OMediaCanvasAnim() {}

OMediaCanvasAnim::~OMediaCanvasAnim() {}


void OMediaCanvasAnim::set_anim_def(OMediaCanvasAnimDef *def)
{
	OMediaAnim::set_anim_def(def);
}

void OMediaCanvasAnim::update_logic(float millisecs_elapsed)
{
	OMediaAnim::update_logic(millisecs_elapsed);
	OMediaCanvasElement::update_logic(millisecs_elapsed);

	if (current_frame) set_canvas((((OMediaCanvasAnimFrame*)current_frame)->get_canvas()));
	else set_canvas(NULL);
}

void OMediaCanvasAnim::pause(bool p)
{
	OMediaElement::pause(p);
	OMediaAnim::pause(p);
}

void OMediaCanvasAnim::get_dynamic_offset(float &x, float &y, float &z)
{
	if (current_frame)
	{
		x = current_frame->getoffsetx();
		y = current_frame->getoffsety();
		z = current_frame->getoffsetz();
	}
	else x=y=z=0;
}