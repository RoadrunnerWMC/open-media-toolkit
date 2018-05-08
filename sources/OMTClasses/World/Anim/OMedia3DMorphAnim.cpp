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
 

#include "OMedia3DMorphAnim.h"
#include "OMedia3DMorphAnimFrame.h"
#include "OMedia3DShape.h"


omt_VertexList	OMedia3DMorphAnim::ivertex_buffer;
omt_NormalList	OMedia3DMorphAnim::inormal_buffer;
omt_ColorList	OMedia3DMorphAnim::icolor_buffer;


//-------------------------------------------------------------------

OMedia3DMorphAnimDef::OMedia3DMorphAnimDef()
{
	compression_level = omclc_DefaultCompression;
}

OMedia3DMorphAnimDef::~OMedia3DMorphAnimDef()
{
	db_update();
	purge();
}

OMediaAnimFrame *OMedia3DMorphAnimDef::create_frame(long sequence)
{
	OMediaAnimFrame *frame = new OMedia3DMorphAnimFrame;
	frame->link(this,sequence);
	return frame;
}


OMediaDBObject *OMedia3DMorphAnimDef::db_builder(void)
{
	return new OMedia3DMorphAnimDef;
}

void OMedia3DMorphAnimDef::read_class(OMediaStreamOperators &stream)
{
	OMediaAnimDef::read_class(stream);
}

void OMedia3DMorphAnimDef::write_class(OMediaStreamOperators &stream)
{
	OMediaAnimDef::write_class(stream);
}

unsigned long OMedia3DMorphAnimDef::get_approximate_size(void)
{
	unsigned long siz = sizeof(*this);
	for(vector<omt_FrameList*>::iterator i=sequences.begin();
		i!=sequences.end();
		i++)
	{
		siz += (*i)->size() * sizeof(OMedia3DMorphAnimFrame);
	}

	return siz;
}

unsigned long OMedia3DMorphAnimDef::db_get_type(void) const
{
	return OMedia3DMorphAnimDef::db_type;
}

//---------------------------------------------------------------

OMedia3DMorphAnim::OMedia3DMorphAnim()
{
	morph_flags = 0;
	morph_param = 0.0f;
}

OMedia3DMorphAnim::~OMedia3DMorphAnim()
{
}

void OMedia3DMorphAnim::set_anim_def(OMedia3DMorphAnimDef *def)
{
	OMediaAnim::set_anim_def(def);
}


void OMedia3DMorphAnim::update_logic(float elapsed)
{
	OMediaElement::update_logic(elapsed);
	OMediaAnim::update_logic(elapsed);

	if (current_frame)
		morph_param = 1.0f-(current_frame_tbcount/current_frame->getmillisecperframe());
}

void OMedia3DMorphAnim::render_draw_shape(OMediaRendererInterface *rdr_i, OMedia3DShape	*shape, bool &pass2)
{
	unsigned long	nv,nn,nc;
	OMedia3DMorphAnimFrame	*current_frame = (OMedia3DMorphAnimFrame*) OMediaAnim::current_frame, 
							*next_frame = (OMedia3DMorphAnimFrame*) OMediaAnim::next_frame;
						
	if (!shape) return;

	if (!current_frame || !next_frame) 
	{
		rdr_i->draw_shape(shape,pass2);	
		return;
	}

	OMediaRendererOverrideVertexList	ovlist;

	shape->lock(omlf_Read);
	nv = shape->get_vertices()->size();
	nn = shape->get_normals()->size();
	nc = shape->get_colors()->size();

	if (current_frame==next_frame || !play_timebased || current_frame->getmillisecperframe()==0)
	{
		ovlist.vertex_list = (current_frame->get_vertices()->size()==nv)?current_frame->get_vertices():NULL;
		ovlist.normal_list = (current_frame->get_normals()->size()==nn)?current_frame->get_normals():NULL;
		ovlist.color_list = (current_frame->get_colors()->size()==nc)?current_frame->get_colors():NULL;
	}
	else
	{
		// Vertices

		if (morph_flags&ommorf_InterpolateVertices)
		{
			if (current_frame->get_vertices()->size()==nv &&
				next_frame->get_vertices()->size()==nv)
			{
				omt_VertexList::iterator			di,si,ni,iend;

				ovlist.vertex_list = &ivertex_buffer;

				if (nv>ivertex_buffer.size()) ivertex_buffer.resize(nv);

				for(di = ovlist.vertex_list->begin(),
					si = current_frame->get_vertices()->begin(),
					ni = next_frame->get_vertices()->begin(),
					iend = current_frame->get_vertices()->end();

					si!= iend;
					
					di++,
					si++,
					ni++)
				{
					(*di).x = (*si).x + ((*ni).x - (*si).x) * morph_param;
					(*di).y = (*si).y + ((*ni).y - (*si).y) * morph_param;
					(*di).z = (*si).z + ((*ni).z - (*si).z) * morph_param;
				}


			}
			else ovlist.vertex_list = NULL;		
		}
		else
		{
			if (current_frame->get_vertices()->size()==nv)
			{
				ovlist.vertex_list = current_frame->get_vertices();
			}
			else ovlist.vertex_list = NULL;	
		}

		// Normals

		if (morph_flags&ommorf_InterpolateVNormals)
		{
			if (current_frame->get_normals()->size()==nn &&
				next_frame->get_normals()->size()==nn)
			{
				omt_NormalList::iterator			di,si,ni,iend;

				ovlist.normal_list = &inormal_buffer;

				if (nn>inormal_buffer.size()) inormal_buffer.resize(nn);

				for(di = ovlist.normal_list->begin(),
					si = current_frame->get_normals()->begin(),
					ni = next_frame->get_normals()->begin(),
					iend = current_frame->get_normals()->end();

					si!= iend;
					
					di++,
					si++,
					ni++)
				{
					(*di).x = (*si).x + ((*ni).x - (*si).x) * morph_param;
					(*di).y = (*si).y + ((*ni).y - (*si).y) * morph_param;
					(*di).z = (*si).z + ((*ni).z - (*si).z) * morph_param;

					(*di).normalize();
				}
			}
			else ovlist.normal_list = NULL;		
		}
		else
		{
			if (current_frame->get_normals()->size()==nn)
			{
				ovlist.normal_list = current_frame->get_normals();
			}
			else  ovlist.normal_list = NULL;	
		}
	
		// Colors

		if (morph_flags&ommorf_InterpolateVColors)
		{
			if (current_frame->get_colors()->size()==nc &&
				next_frame->get_colors()->size()==nc)
			{
				omt_ColorList::iterator			di,si,ni,iend;

				ovlist.color_list = &icolor_buffer;

				if (nc>icolor_buffer.size()) icolor_buffer.resize(nc);

				for(di = ovlist.color_list->begin(),
					si = current_frame->get_colors()->begin(),
					ni = next_frame->get_colors()->begin(),
					iend = current_frame->get_colors()->end();

					si!= iend;
					
					di++,
					si++,
					ni++)
				{
					(*di).alpha = (*si).alpha + ((*ni).alpha - (*si).alpha) * morph_param;
					(*di).red = (*si).red + ((*ni).red - (*si).red) * morph_param;
					(*di).green = (*si).green + ((*ni).green - (*si).green) * morph_param;
					(*di).blue = (*si).blue + ((*ni).blue - (*si).blue) * morph_param;
				}
			}
			else ovlist.color_list = NULL;		
		}
		else
		{
			if (current_frame->get_colors()->size()==nc)
			{
				ovlist.color_list = current_frame->get_colors();
			}
			else ovlist.color_list = NULL;	
		}
	

	}

	rdr_i->draw_shape(shape,pass2,&ovlist);	

	shape->unlock();
}


void OMedia3DMorphAnim::pause(bool p)
{
	OMediaElement::pause(p);
	OMediaAnim::pause(p);
}

void OMedia3DMorphAnim::get_dynamic_offset(float &x, float &y, float &z)
{
	if (current_frame)
	{
		x = current_frame->getoffsetx();
		y = current_frame->getoffsety();
		z = current_frame->getoffsetz();
	}
	else x=y=z=0;
}

OMediaDBObject *OMedia3DMorphAnim::db_builder(void)
{
	return new OMedia3DMorphAnim;
}

void OMedia3DMorphAnim::read_class(OMediaStreamOperators &stream)
{
	OMedia3DShapeElement::read_class(stream);

	stream>>morph_flags;
	OMediaAnim::read_class(stream);
}

void OMedia3DMorphAnim::write_class(OMediaStreamOperators &stream)
{
	OMedia3DShapeElement::write_class(stream);

	stream<<morph_flags;
	OMediaAnim::write_class(stream);
}

unsigned long OMedia3DMorphAnim::get_approximate_size(void)
{
	return OMedia3DShapeElement::get_approximate_size();
}

unsigned long OMedia3DMorphAnim::db_get_type(void)
{
	return OMedia3DMorphAnim::db_type;
}

void OMedia3DMorphAnim::unlink_animdef_from_structure(OMediaAnimDef *animdef)
{
	OMediaElement::unlink_animdef_from_structure(animdef);

	if (get_anim_def()==animdef) set_anim_def(NULL);
}

