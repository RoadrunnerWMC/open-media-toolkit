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
 
#include "OMediaAnimFrame.h"
#include "OMediaAnim.h"


OMediaAnimFrame::OMediaAnimFrame()
{
	its_anim = omc_NULL;
	offset_x = offset_y = offset_z = 0;
	speed_fb = 1;
	speed_tb = 40;		// 25 frames per sec (default)
	sequence = 0;
}

OMediaAnimFrame::~OMediaAnimFrame() 
{
	unlink();
}

void OMediaAnimFrame::link(OMediaAnimDef *anim, long newseq) 
{
	omt_FrameList	*list;

	unlink();
	
	its_anim = anim;
	sequence = newseq;
	
	list = anim->getsequence(newseq);
	list->push_back(this);
	
	container_node = --(list->end());	
}

void OMediaAnimFrame::unlink(void) 
{
	if (its_anim)
 	{
	 	omt_FrameList	*list;
	 	
		list = its_anim->getsequence(sequence);
 		list->erase(container_node);
 		its_anim = omc_NULL;
 	}
}


void OMediaAnimFrame::setoffset(omt_WUnit x, omt_WUnit y, omt_WUnit z)
{
	offset_x = x;
	offset_y = y;
	offset_z = z;
}

void OMediaAnimFrame::setframespeed_tb(omt_WFramePerSec fps) 
{
	speed_fps_tb = fps; 
	speed_tb = (fps)?1000.0f/fps:0.0f;
}

void OMediaAnimFrame::read_class(OMediaStreamOperators &stream)
{
	stream>>offset_x;
	stream>>offset_y;
	stream>>offset_z;
	stream>>speed_fb;
	stream>>speed_tb;
	stream>>speed_fps_tb;
}

void OMediaAnimFrame::write_class(OMediaStreamOperators &stream)
{
	stream<<offset_x;
	stream<<offset_y;
	stream<<offset_z;
	stream<<speed_fb;
	stream<<speed_tb;
	stream<<speed_fps_tb;
}

