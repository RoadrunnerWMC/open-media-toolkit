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
 
#define omd_PROFILE_ON
#include "OMediaProfiler.h"
#include "OMediaFilePath.h"
#include "OMediaFileStream.h"

#include <stdio.h>
#include <string.h>

OMediaProfileSection *OMediaProfiler::current_section =NULL;
OMediaProfiler 		  OMediaProfiler::base;


static void omf_DumpSections(omt_ProfileSectionList &sections, long level, OMediaFileStream *file)
{
	static char	str[512];
	char	linefeed[2];
	linefeed[0] = 0x0D;
	linefeed[1] = 0x0A;

	for(long j=0;j<level;j++) str[j] = ' ';

	for(omt_ProfileSectionList::iterator i=sections.begin();
		i!=sections.end();
		i++)
	{
		OMediaProfileSection *s = (*i);	
		
		s->compute_stats();
		
		sprintf(str+level,"%s: min=%0.3f max=%0.3f avg=%0.3f total=%0.3f totalpc=%ld, hits=%ld",
				s->name.c_str(),
				float(s->min_time),
				float(s->max_time),
				float(s->average_time),
				float(s->total_time),
				long(s->time_pc),
				long(s->hits));
	
		file->write(str,strlen(str));

		#ifdef omd_INTEL
		file->write(linefeed,2);
		#else
		file->write(linefeed,1);		
		#endif
		
		omf_DumpSections(s->sub_sections,level+1,file);
	}
}

void OMediaProfiler::dump_stats(string filename)
{
	OMediaFilePath		path(filename,'CWIE','TEXT');
	OMediaFileStream	file;
	
	file.setpath(&path);
	file.open(omcfp_Write,true,true);
	
	omf_DumpSections(OMediaProfiler::base.main_list,0,&file);
	
	file.close();
}
