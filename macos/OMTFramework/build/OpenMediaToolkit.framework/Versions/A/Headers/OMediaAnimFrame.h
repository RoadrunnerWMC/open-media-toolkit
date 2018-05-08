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
#ifndef OMEDIA_AnimFrame_H
#define OMEDIA_AnimFrame_H

#include "OMediaTypes.h"
#include "OMediaWorldUnits.h"
#include "OMediaClassStreamer.h"

#include <list>

class OMediaAnimDef;
class OMediaAnimFrame;

typedef list<OMediaAnimFrame *>	omt_FrameList;

class OMediaAnimFrame : public OMediaClassStreamer
{
	
	public:
	
	// * Constructor/Destructor

	omtshared OMediaAnimFrame();
	omtshared virtual ~OMediaAnimFrame();

	// * Anim

	inline OMediaAnimDef *getanim_def(void) {return its_anim;}

	omtshared virtual void link(OMediaAnimDef *anim_def, long sequence =0);
	omtshared virtual void unlink(void);
	
	inline long getsequence(void) {return sequence;}


	// * Frame offset/size
	
	omtshared virtual void setoffset(omt_WUnit x, omt_WUnit y, omt_WUnit z);
	inline omt_WUnit getoffsetx(void) const {return offset_x;}
	inline omt_WUnit getoffsety(void) const {return offset_y;}
	inline omt_WUnit getoffsetz(void) const {return offset_z;}

	inline void setoffsetx(omt_WUnit x) {setoffset(x,offset_y,offset_z);}
	inline void setoffsety(omt_WUnit y) {setoffset(offset_x,y,offset_z);}
	inline void setoffsetz(omt_WUnit z) {setoffset(offset_x,offset_y,z);}


	// * Frame duration
	
	// Frame based
	inline void setframespeed_fb(long framesperupdate) {speed_fb = framesperupdate;}
	inline long getframespeed_fb(void) const {return speed_fb;}

	// Time based
	omtshared virtual void setframespeed_tb(omt_WFramePerSec fps);
	inline omt_WFramePerSec getframespeed_tb(void) const {return speed_fps_tb;}
	
	inline omt_WFramePerSec getmillisecperframe(void) const {return speed_tb;}

	// * Streaming

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);


	// * Container node
	
	inline omt_FrameList::iterator get_container_node(void) {return container_node;}
	inline void set_container_node(omt_FrameList::iterator i) {container_node = i;}


	protected:	
	
	OMediaAnimDef			*its_anim;
	omt_WUnit				offset_x,offset_y,offset_z;
	long					speed_fb;
	omt_WMilliSec			speed_tb;
	omt_WFramePerSec		speed_fps_tb;
	long					sequence;
	omt_FrameList::iterator	container_node;
};



#endif

