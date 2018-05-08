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

#include "OMediaAnim.h"
#include "OMediaError.h"
#include "OMediaStreamOperators.h"

OMediaAnimDef::OMediaAnimDef()
{
}

OMediaAnimDef::~OMediaAnimDef()
{
	db_update();
	purge();
}

void OMediaAnimDef::purge(void)
{
	// Delete frames
	for(vector<omt_FrameList*>::iterator i = sequences.begin();
		i != sequences.end();
		i++)
	{
		while((*i)->size()) delete *((*i)->begin());
		delete (*i);
	}

	sequences.erase(sequences.begin(),sequences.end());
}

void OMediaAnimDef::setnsequences(unsigned long n)
{
	omt_FrameList						*emptylist;
	vector<omt_FrameList*>::iterator	i;	
	while (n>sequences.size()) 
	{
		emptylist = new omt_FrameList;
		sequences.push_back(emptylist);
	}

	while(n<sequences.size())
	{
		i  = sequences.end();
		i--;
		while((*i)->size()) delete *((*i)->begin());
		sequences.erase(i);
	}
}

OMediaAnimFrame *OMediaAnimDef::create_frame(long sequence)
{
	OMediaAnimFrame *frame = new OMediaAnimFrame;
	frame->link(this,sequence);
	return frame;
}

OMediaDBObject *OMediaAnimDef::db_builder(void)
{
	return new OMediaAnimDef;
}

void OMediaAnimDef::read_class(OMediaStreamOperators &stream)
{
	long				v,ns,nf,seq;
	OMediaAnimFrame		*frame;

	purge();

	OMediaDBObject::read_class(stream);
	stream>>v;

	seq=0;
	stream>>ns;
	while(ns--)
	{
		stream>>nf;
		while(nf--)
		{
			frame = create_frame(seq);
			stream>>frame;
		}

		seq++;
	}
}

void OMediaAnimDef::write_class(OMediaStreamOperators &stream)
{
	long	n;
	long	v=0;

	OMediaDBObject::write_class(stream);
	stream<<v;

	n = sequences.size();
	stream<<n;

	for(vector<omt_FrameList*>::iterator si=sequences.begin();
		si!=sequences.end();
		si++)
	{
		n = (*si)->size();
		stream<<n;
		for(omt_FrameList::iterator fi=(*si)->begin();
			fi!=(*si)->end();
			fi++)
		{
			stream<<(*fi);
		}
	}
}

unsigned long OMediaAnimDef::get_approximate_size(void)
{
	return sizeof(*this);
}

unsigned long OMediaAnimDef::db_get_type(void) const
{
	return OMediaAnimDef::db_type;
}



//-----------------------------------------------------------------


OMediaAnim::OMediaAnim()
{
	anim_def = NULL;
	current_frame = next_frame = omc_NULL;
	current_sequence = 0;
	play_timebased = true;
	play_loop = true;
	play_reverse = false;
	play_started = false;
	play_pingpong = false;
	pause_count = 0;
}

OMediaAnim::~OMediaAnim()
{
	if (anim_def) anim_def->db_unlock();
}

void OMediaAnim::set_anim_def(OMediaAnimDef *def)
{
	if (anim_def!=def)
	{
		current_sequence = 0;
		current_frame = next_frame = NULL;
		play_started = false;
	}

	if (anim_def) anim_def->db_unlock();
	anim_def = def;
	if (anim_def) anim_def->db_lock();
}

void OMediaAnim::setcurrentsequence(long newseq, bool restart)
{
	if (!anim_def) return;

	vector<omt_FrameList*> &sequences = *anim_def->getsequence_list();
	
	if (sequences.size()==0) return;

	if (newseq>=(long)sequences.size())  newseq = sequences.size()-1;

	if (newseq != current_sequence)
	{
		if (sequences[newseq]->size()==0) current_frame = omc_NULL;
		else if (restart || !current_frame) 
		{
			current_frame = *(sequences[newseq]->begin());
		}
		else
		{
			omt_FrameList::iterator	oldf = current_frame->get_container_node();
			omt_FrameList::iterator	newf = sequences[newseq]->begin();

			for(; (oldf!=sequences[current_sequence]->begin() &&
				   newf!=sequences[newseq]->end()); 
				   oldf--, newf++) {}

			if (newf==sequences[newseq]->end()) newf--;
			
			current_frame = (*newf);
		}
	
		current_sequence = newseq;
		play_started = false;
		next_frame = find_next_frame();
	}
}



void OMediaAnim::setcurrentframe(OMediaAnimFrame *frame)
{
	if (!frame || !anim_def)
	{
		current_frame = omc_NULL;
	}
	else
	{
		if (frame->getanim_def()!=anim_def) omd_EXCEPTION(omcerr_SetUnlinkedFrame);
		setcurrentsequence(frame->getsequence());
		current_frame = frame;
		play_started = false;
	}

	next_frame = find_next_frame();
}

void OMediaAnim::setcurrentframe(long newseq, long frame_pos)
{
	if (!anim_def) return;

	setcurrentsequence(newseq);

	vector<omt_FrameList*> &sequences = *anim_def->getsequence_list();
	
	if (sequences.size()==0) return;

	if (sequences[newseq]->size()==0) current_frame=omc_NULL;
	else
	{
		omt_FrameList::iterator	f = sequences[newseq]->begin();
	
		for(; (frame_pos>0 && f!=sequences[newseq]->end()); frame_pos--, f++) {}
	
		if (f==sequences[newseq]->end()) f--;
		
		current_frame = *f;
	}
	
	play_started = false;
	next_frame = find_next_frame();
}

long OMediaAnim::getcurrentframe_pos(void) const
{
	omt_FrameList::const_iterator f;
	long					i;
	
	if (!anim_def) return 0;
	vector<omt_FrameList*> &sequences = *anim_def->getsequence_list();

	if (sequences.size()==0) return 0;


	for(f = sequences[current_sequence]->begin(),i=0;
		f!= sequences[current_sequence]->end();
		f++,i++)
	{
		if ((*f)==current_frame) return i;
	}
		
	return 0;
}

	
void OMediaAnim::setplay_timebased(bool	timebased) 
{
	play_started = false;
	play_timebased = timebased;
}

void OMediaAnim::update_logic(float elapsed)
{
	if (pause_count!=0 || !anim_def) return;

	vector<omt_FrameList*> &sequences = *anim_def->getsequence_list();

	if (!current_frame)
	{
		if (sequences.size() && sequences[current_sequence]->size())
		{
			current_frame = *(sequences[current_sequence]->begin());
			next_frame = find_next_frame();
			play_started = false;
		}
		else return;
	}

	if (play_timebased)
	{
		// Time based
		if (!play_started)
		{
			play_started = true;
			current_frame_tbcount = current_frame->getmillisecperframe();
		}
		else
		{
			omt_WMilliSec mel = omt_WMilliSec(elapsed);

			if (current_frame->getmillisecperframe()!=0)
			{
				for(;;)
				{
					if (mel>=current_frame_tbcount)
					{
						mel -=current_frame_tbcount;
					
						if (advance_frame(false)) 
						{
							current_frame_tbcount = current_frame->getmillisecperframe();
							break;	// If can't advance more, break the loop!
						}
						
						current_frame_tbcount = current_frame->getmillisecperframe();
						if (!current_frame_tbcount) break;
					}
					else
					{
						current_frame_tbcount -= mel;
						break;
					}
				}
			}
		}
	}
	else
	{
		// Frame based
		if (!play_started)
		{
			play_started = true;
			updatecount = 0;
		}
		else
		{
			updatecount++;
			if (updatecount>=current_frame->getframespeed_fb())
			{
				updatecount=0;
				advance_frame(false);
			}
		}
	}	
}

bool OMediaAnim::advance_frame(bool reset_timer)
{
	bool		res = false;

	if (!anim_def) return true;
	vector<omt_FrameList*> &sequences = *anim_def->getsequence_list();


	if (reset_timer) play_started = false;
	if (!current_frame)
	{
		if (sequences.size() && sequences[current_sequence]->size())
		{
			current_frame = *(sequences[current_sequence]->begin());
		}
		else 
		{
			next_frame = NULL;
			return true;
		}
	}

	if (sequences[current_sequence]->size()==1) 
	{
		next_frame = current_frame;
		return true;
	}

	if (!play_reverse)
	{			
		if (current_frame->get_container_node()!=--(sequences[current_sequence]->end()))
		{
			current_frame = *(++(current_frame->get_container_node()));
		}
		else if (play_pingpong)
		{
			play_reverse = true;
			current_frame = *(--(current_frame->get_container_node()));
		}
		else if (play_loop)
		{
			current_frame = *(sequences[current_sequence]->begin());
		}
		else res = true;
	}
	else
	{
		if (current_frame->get_container_node()!=(sequences[current_sequence]->begin()))
		{
			current_frame = *(--(current_frame->get_container_node()));
		}
		else if (play_pingpong)
		{
			play_reverse = false;
			current_frame = *(++(current_frame->get_container_node()));
		}
		else if (play_loop)
		{
			current_frame = *(--(sequences[current_sequence]->end()));
		}
		else res = true;
	}
	
	next_frame = find_next_frame();
	return res;
}

void OMediaAnim::find_current_frame(void)
{
	if (!anim_def) return;
	vector<omt_FrameList*> &sequences = *anim_def->getsequence_list();

	if (sequences.size() && sequences[current_sequence]->size())
	{
		current_frame = *(sequences[current_sequence]->begin());
		play_started = false;
		next_frame = find_next_frame();
	}
	else
	{
		current_frame = next_frame = NULL;
	}
}


OMediaAnimFrame *OMediaAnim::find_next_frame(void)
{
	OMediaAnimFrame	*next_frame;

	if (!anim_def) return NULL;
	vector<omt_FrameList*> &sequences = *anim_def->getsequence_list();

	if (!current_frame)
	{
		if (sequences.size() && sequences[current_sequence]->size())
		{
			next_frame = *(sequences[current_sequence]->begin());
		}
		else return current_frame;
	}

	if (sequences[current_sequence]->size()==1) return current_frame;

	if (!play_reverse)
	{			
		if (current_frame->get_container_node()!=--(sequences[current_sequence]->end()))
		{
			next_frame = *(++(current_frame->get_container_node()));
		}
		else if (play_pingpong)
		{
			next_frame = *(--(current_frame->get_container_node()));
		}
		else if (play_loop)
		{
			next_frame = *(sequences[current_sequence]->begin());
		}
		else return current_frame;
	}
	else
	{
		if (current_frame->get_container_node()!=(sequences[current_sequence]->begin()))
		{
			next_frame = *(--(current_frame->get_container_node()));
		}
		else if (play_pingpong)
		{
			next_frame = *(++(current_frame->get_container_node()));
		}
		else if (play_loop)
		{
			next_frame = *(--(sequences[current_sequence]->end()));
		}
		else return current_frame;
	}
	
	return next_frame;
}



void OMediaAnim::pause(bool p)
{
	if (!p && pause_count==1) play_started = false;		

	if (p) pause_count++;
	else pause_count--;
}

void OMediaAnim::read_class(OMediaStreamOperators &stream)
{
	OMediaDBObjectStreamLink	slink;
	long						s,f;
		
	stream>>slink;
	set_anim_def((OMediaAnimDef*)slink.get_object());

	stream>>s;
	stream>>f;
	
	stream>>pause_count;
	stream>>play_timebased;
	stream>>play_loop;
	stream>>play_reverse;
	stream>>play_pingpong;
	
	setcurrentframe(s,f);
}

void OMediaAnim::write_class(OMediaStreamOperators &stream)
{
	OMediaDBObjectStreamLink	slink;
	long	p = getcurrentframe_pos();

	slink.set_object(anim_def);
	stream<<slink;	

	stream<<current_sequence;
	stream<<p;

	stream<<pause_count;
	stream<<play_timebased;
	stream<<play_loop;
	stream<<play_reverse;
	stream<<play_pingpong;
}


