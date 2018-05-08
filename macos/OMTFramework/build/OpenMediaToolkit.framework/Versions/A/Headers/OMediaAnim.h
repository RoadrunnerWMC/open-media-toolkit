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

#ifndef OMEDIA_Anim_H
#define OMEDIA_Anim_H


#include "OMediaTypes.h"
#include "OMediaWorldUnits.h"
#include "OMediaAnimFrame.h"
#include "OMediaDBObject.h"

#include <list>
#include <vector>

//-------------------------------------------------------------------
// The anim definition

class OMediaAnimDef : public OMediaDBObject
{
	public:
	
	// * Constructor/Destructor

	omtshared OMediaAnimDef();
	omtshared virtual ~OMediaAnimDef();

	omtshared virtual void purge(void);


	// * Sequence

	inline omt_FrameList *getsequence(long seq)		// If sequence does not exist, it
	{												// creates a new one.
		omt_FrameList	*emptylist;
		while (long(seq)>=long(sequences.size())) 
		{
			emptylist = new omt_FrameList;
			sequences.push_back(emptylist);
		}
		
		return sequences[seq];
	}

	omtshared virtual void setnsequences(unsigned long n);
	inline long getnsequences(void) const {return sequences.size();}

	inline vector<omt_FrameList*> *getsequence_list(void) {return &sequences;}


	// Create and link new frame
	omtshared virtual OMediaAnimFrame *create_frame(long sequence);


	// * Database/streamer support
	
	enum { db_type = 'Aabs' };

	omtshared static OMediaDBObject *db_builder(void);

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	omtshared virtual unsigned long get_approximate_size(void);

	omtshared virtual unsigned long db_get_type(void) const;


	protected:

	vector<omt_FrameList*>	sequences;	
};




//--------------------------------------------------------------------
// The anim instance

class OMediaAnim : public OMediaClassStreamer
{
	public:
	
	// * Constructor/Destructor

	omtshared OMediaAnim();
	omtshared virtual ~OMediaAnim();

	// * Anim definition

	omtshared virtual void set_anim_def(OMediaAnimDef *def);
	inline OMediaAnimDef *get_anim_def(void) {return anim_def;}

	// * Current Sequence

	omtshared virtual void setcurrentsequence(long curseq, bool restart = true);
	inline long getcurrentsequence(void) const {return current_sequence;}

	// * Current frame
		
	omtshared virtual void setcurrentframe(OMediaAnimFrame *frame);	// Frame must be linked!
	omtshared virtual void setcurrentframe(long sequence, long frame_pos);

	
	inline OMediaAnimFrame *getcurrentframe(void) {if (!current_frame) find_current_frame(); return current_frame;}
	omtshared virtual long getcurrentframe_pos(void) const;

	omtshared virtual bool advance_frame(bool reset_timer = true);
	inline OMediaAnimFrame *get_next_frame(void) {return next_frame;}

	// * Play
	
	inline void setplay_loop(bool loop) {play_loop = loop;}
	inline bool getplay_loop(void) const {return play_loop;}

	inline void setplay_reverse(bool rev) {play_reverse = rev;}
	inline bool getplay_reverse(void) const {return play_reverse;}

	inline void setplay_pingpong(bool pp) {play_pingpong = pp;}
	inline bool getplay_pingpong(void) const {return play_pingpong;}

	omtshared virtual void setplay_timebased(bool	timebased);	// (default is true)
	inline bool getplay_timebased(void) const {return play_timebased;}

	inline void reset_timebased_counter(void) {play_started = false;}


	// * Update
	
	omtshared virtual void update_logic(float elapsed);


	// * Pause
	
	omtshared virtual void pause(bool p);		// Pause anim (it is overrided by elements, so it
												// pauses motion too)

	inline bool is_anim_paused(void) const {return pause_count!=0;}
	inline long get_anim_pause_count(void) const {return pause_count;}

	// * Local pause
	
	inline void pause_anim(bool p) {OMediaAnim::pause(p);}
	inline void force_unpause_anim(void) {if (pause_count) {pause_count=1;OMediaAnim::pause(false);}}
	inline bool get_pause_anim(void) {return pause_count!=0;}

	// * Streaming

	omtshared virtual void read_class(OMediaStreamOperators &stream);
	omtshared virtual void write_class(OMediaStreamOperators &stream);

	protected:

	omtshared virtual void find_current_frame(void);

	omtshared virtual OMediaAnimFrame *find_next_frame(void);

	OMediaAnimDef		*anim_def;
	
	OMediaAnimFrame		*current_frame, *next_frame;
	long				current_sequence,updatecount,pause_count;
	bool				play_timebased, play_loop, play_reverse, play_started, play_pingpong;
	omt_WMilliSec		current_frame_tbcount;	
};

#endif

