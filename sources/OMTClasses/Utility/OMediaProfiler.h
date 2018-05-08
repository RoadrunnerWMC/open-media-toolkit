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
#ifndef OMEDIA_Profiler_H
#define OMEDIA_Profiler_H

#include "OMediaTimer.h"
#include <string>

#include <vector>

#ifndef omd_PROFILE_ON
//#define omd_PROFILE_ON
#endif


#ifdef omd_PROFILE_ON
class OMediaProfileSection;

typedef vector<OMediaProfileSection*> omt_ProfileSectionList;


class OMediaProfileSection
{
	public:
	
	OMediaProfileSection(OMediaProfileSection *parent, string n) 
	{
		name = n;
		max_time = average_time = total_time = min_time = 0;
		time_pc = 0;
		hits = 0;
		parent_section = parent;
		first_pass = true;
	}
	
	~OMediaProfileSection(void)
	{
		for(omt_ProfileSectionList::iterator i=sub_sections.begin();
			i!=sub_sections.end();
			i++)
		{
			delete (*i);
			*i = NULL;
		}
	}

	inline void profile_start(void)
	{
		timer.start();
	}

	inline void profile_end(void)
	{
		double elapsed = double(timer.getelapsed());
		total_time += elapsed;
		hits++;
		if (first_pass)
		{
			first_pass = false;
			max_time = min_time = elapsed;
		}
		else
		{
			if (elapsed<min_time) min_time = elapsed;
			if (elapsed>max_time) max_time = elapsed;
		}
	}
	
	inline OMediaProfileSection *find_root(void)
	{
		if (!parent_section) return this;
		return parent_section->find_root();
	}
	
	inline void compute_stats(void)
	{		
		if (first_pass) return;
		
		average_time = total_time/double(hits);
		
		OMediaProfileSection* root = find_root();

		if (root==this) 
		{
			time_pc = 100.0;
		}
		else
		{
			if (root->total_time!=0.0) time_pc = (total_time/root->total_time)*100.0;
		}
	}
	
	string				name;
	
	double						min_time,max_time,average_time,total_time;
	double						time_pc;
	double						hits;	
	bool					first_pass;
	
	OMediaTimer					timer;
	omt_ProfileSectionList		sub_sections;
	OMediaProfileSection		*parent_section;
};

class OMediaProfiler
{
	public:
	
	OMediaProfiler() {}
	~OMediaProfiler() 
	{
		for(omt_ProfileSectionList::iterator i=main_list.begin();
			i!=main_list.end();
			i++)
		{
			delete (*i);
			(*i) = NULL;
		}	
	}

	omt_ProfileSectionList		main_list;

	inline OMediaProfileSection *new_section(OMediaProfileSection *parent, string name)
	{
		OMediaProfileSection	*s = new OMediaProfileSection(parent,name);
		if (!parent) main_list.push_back(s);
		else parent->sub_sections.push_back(s);
		return s;
	}

	omtshared static void dump_stats(string filename);

	omtshared static OMediaProfileSection *current_section;
	omtshared static OMediaProfiler 		base;
};



#define omd_PROFILE_BEGIN(varname, desc_str) static OMediaProfileSection *varname; 	\
											 if (!varname) varname = OMediaProfiler::base.new_section(OMediaProfiler::current_section,desc_str);	\
											 OMediaProfiler::current_section = varname; \
											 OMediaProfiler::current_section->profile_start()


#define omd_PROFILE_END OMediaProfiler::current_section->profile_end(); OMediaProfiler::current_section = OMediaProfiler::current_section->parent_section

#define omd_PROFILE_SETCURRENT(varname) OMediaProfiler::current_section = varname

#else
#define omd_PROFILE_BEGIN(varname, desc_str)
#define omd_PROFILE_END
#define omd_PROFILE_SETCURRENT(varname)
#endif


#endif

