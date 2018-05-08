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
 
#ifndef OMEDIA_FilePath_H
#define OMEDIA_FilePath_H


#include "OMediaRetarget.h"

#include <string>

#include <vector>

typedef unsigned long omt_FileType;
typedef unsigned long omt_AppType;

class OMediaWindow;

class OMediaAskUserFilter
{
	public:
	omt_FileType		file_type;
	string		descriptor;
	string		pattern;
};

typedef vector<OMediaAskUserFilter> omt_AskUserList;

class OMediaAskUserFilters
{
	public:
	omt_AskUserList		filters;
};

class OMediaFilePath
{
	public:

	omtshared OMediaFilePath(void);
	omtshared OMediaFilePath(string name, omt_AppType appt ='null', omt_FileType filet ='null');
	omtshared OMediaFilePath(string name, const OMediaFilePath *base_path, omt_AppType appt ='null', omt_FileType filet ='null');
	omtshared virtual ~OMediaFilePath(void);

	omtshared virtual void setpath(string name, omt_AppType appt ='null', omt_FileType filet ='null');
	omtshared virtual void setpath(string name, const OMediaFilePath *base_path, omt_AppType appt ='null', omt_FileType filet ='null');
	omtshared virtual void setpath(const OMediaFilePath *newpath);

	omtshared virtual void set_default_preference_path(string filename, omt_AppType appt ='null', omt_FileType filet ='null');


	inline OMediaRetarget	*getretarget(void) const {return retarget;}

	inline omt_AppType getapptype(void) const {return app_type;}
	inline omt_FileType getfiletype(void) const {return file_type;}

	inline void setapptype(omt_AppType a) {app_type = a;}
	inline void setfiletype(omt_FileType f) {file_type = f;}

	omtshared virtual bool ask_user(OMediaWindow *superwindow, OMediaAskUserFilters &filter, bool save_mode, string title ="", string prompt ="", string default_name="");

	omtshared virtual void get_filename(string &str);

	protected:
	
	OMediaRetarget	*retarget;
	omt_AppType app_type;
	omt_FileType file_type;
};



#endif