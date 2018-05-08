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
 

#include "OMedia3DShapeAnim.h"
#include "OMedia3DShapeAnimFrame.h"
#include "OMedia3DShape.h"

//-------------------------------------------------------------------

OMedia3DShapeAnimDef::OMedia3DShapeAnimDef()
{
}

OMedia3DShapeAnimDef::~OMedia3DShapeAnimDef()
{
	db_update();
	purge();
}

OMediaAnimFrame *OMedia3DShapeAnimDef::create_frame(long sequence)
{
	OMediaAnimFrame *frame = new OMedia3DShapeAnimFrame;
	frame->link(this,sequence);
	return frame;
}


OMediaDBObject *OMedia3DShapeAnimDef::db_builder(void)
{
	return new OMedia3DShapeAnimDef;
}

void OMedia3DShapeAnimDef::read_class(OMediaStreamOperators &stream)
{
	OMediaAnimDef::read_class(stream);
}

void OMedia3DShapeAnimDef::write_class(OMediaStreamOperators &stream)
{
	OMediaAnimDef::write_class(stream);
}

unsigned long OMedia3DShapeAnimDef::get_approximate_size(void)
{
	unsigned long siz = sizeof(*this);
	for(vector<omt_FrameList*>::iterator i=sequences.begin();
		i!=sequences.end();
		i++)
	{
		siz += (*i)->size() * sizeof(OMedia3DShapeAnimFrame);
	}

	return siz;
}

unsigned long OMedia3DShapeAnimDef::db_get_type(void) const
{
	return OMedia3DShapeAnimDef::db_type;
}

//-------------------------------------------------------------------


OMedia3DShapeAnim::OMedia3DShapeAnim()
{
}  

OMedia3DShapeAnim::~OMedia3DShapeAnim()
{
}

void OMedia3DShapeAnim::set_anim_def(OMedia3DShapeAnimDef *def)
{
	OMediaAnim::set_anim_def(def);
}

void OMedia3DShapeAnim::update_logic(float elapsed)
{
	OMediaElement::update_logic(elapsed);
	OMediaAnim::update_logic(elapsed);
	
	if (anim_def && current_frame) set_shape((((OMedia3DShapeAnimFrame*)current_frame)->get_shape()));
	else set_shape(NULL);
}

void OMedia3DShapeAnim::pause(bool p)
{
	OMediaElement::pause(p);
	OMediaAnim::pause(p);
}

void OMedia3DShapeAnim::get_dynamic_offset(float &x, float &y, float &z)
{
	if (current_frame)
	{
		x = current_frame->getoffsetx();
		y = current_frame->getoffsety();
		z = current_frame->getoffsetz();
	}
	else x=y=z=0;
}

void OMedia3DShapeAnim::render_draw_shape(OMediaRendererInterface *rdr_i, OMedia3DShape	*shape, bool &pass2)
{
}

void OMedia3DShapeAnim::unlink_shape_from_structure(OMedia3DShape *shape)
{
	OMediaAnimDef	*def;
	long			s;

	OMedia3DShapeElement::unlink_shape_from_structure(shape);

	def = get_anim_def();
	if (def==NULL) return;

	for(s=0;s<def->getnsequences();s++)
	{
		omt_FrameList *seq = def->getsequence(s);
		
		for(omt_FrameList::iterator fi=seq->begin();
			fi!=seq->end();
			fi++)
		{
			OMedia3DShapeAnimFrame	*fr;
			
			fr = (OMedia3DShapeAnimFrame*)(*fi);
			if (fr->get_shape()==shape) fr->set_shape(NULL);
		}	
	}
}


void OMedia3DShapeAnim::unlink_animdef_from_structure(OMediaAnimDef *animdef)
{
	OMediaElement::unlink_animdef_from_structure(animdef);

	if (get_anim_def()==animdef) set_anim_def(NULL);
}

