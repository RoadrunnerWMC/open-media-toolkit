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

 
#include "OMediaRetarget.h"
#include "OMediaFilePath.h"
#include "OMediaWinRtgFilePath.h"
#include "OMediaError.h"
#include "OMediaWinRtgWindow.h"
#include "OMediaWindow.h"

OMediaFilePath::OMediaFilePath(void)
{
	retarget = new OMediaWinRtgFilePath;
	if (!retarget) omd_EXCEPTION(omcerr_OutOfMemory);
}

OMediaFilePath::OMediaFilePath(string name, const OMediaFilePath *base_path,omt_AppType appt, omt_FileType filet)
{
	retarget = new OMediaWinRtgFilePath;
	if (!retarget) omd_EXCEPTION(omcerr_OutOfMemory);

	setpath(name, base_path, appt, filet);
}

OMediaFilePath::OMediaFilePath(string name, omt_AppType appt, omt_FileType filet)
{
	retarget = new OMediaWinRtgFilePath;
	if (!retarget) omd_EXCEPTION(omcerr_OutOfMemory);

	setpath(name, appt, filet);
}


OMediaFilePath::~OMediaFilePath(void)
{
	delete retarget;
}

void OMediaFilePath::setpath(string filename, omt_AppType appt, omt_FileType filet)
{
	app_type = appt;
	file_type =filet;
	
	((OMediaWinRtgFilePath*)retarget)->path = filename;
}

void OMediaFilePath::setpath(string filename, const OMediaFilePath *base_path,omt_AppType appt, omt_FileType filet)
{
	string	bpath;

	app_type = appt;
	file_type =filet;
	
	bpath = ((OMediaWinRtgFilePath*)(base_path->getretarget()))->path;

	long	p;
	p = bpath.size()-1;
	while(p>=0)
	{
		if (bpath[p]=='/' ||
			bpath[p]=='\\' ||
			bpath[p]==':')
		{
			bpath.resize(p+1);
			break;
		}

		p--;
	}

	if (p>0)
		((OMediaWinRtgFilePath*)retarget)->path = bpath;
	else
		((OMediaWinRtgFilePath*)retarget)->path = "";
	
	((OMediaWinRtgFilePath*)retarget)->path.append(filename);
}

void OMediaFilePath::get_filename(string &str)
{
	string	bpath;

	bpath = ((OMediaWinRtgFilePath*)(getretarget()))->path;

	long	p;
	p = bpath.size()-1;
	while(p>=0)
	{
		if (bpath[p]=='/' ||
			bpath[p]=='\\' ||
			bpath[p]==':') 
		{
			p++;
			if (p>=bpath.size()) 
			{
				str = "";
				return;
			}
			break;
		}

		p--;
	}

	if (p>0)
		str= bpath.c_str()+p;
	else
		str = bpath;
}

void OMediaFilePath::set_default_preference_path(string filename, omt_AppType appt, omt_FileType filet)
{
	char	cbuff[MAX_PATH+1];
	UINT	res;

	app_type = appt;
	file_type =filet;

	res = GetWindowsDirectory(cbuff,MAX_PATH);
	if (res==0) omd_OSEXCEPTION(GetLastError());
	cbuff[res] = 0;

	((OMediaWinRtgFilePath*)retarget)->path = cbuff;

	if (cbuff[res-1]!='\\') ((OMediaWinRtgFilePath*)retarget)->path += "\\";
	((OMediaWinRtgFilePath*)retarget)->path += filename;
}


void OMediaFilePath::setpath(const OMediaFilePath *newpath)
{
	*((OMediaWinRtgFilePath*)retarget) = *((OMediaWinRtgFilePath*)(newpath->getretarget()));

	app_type = newpath->getapptype();
	file_type = newpath->getfiletype();
}

bool OMediaFilePath::ask_user(OMediaWindow *superwindow, OMediaAskUserFilters &filterobj, bool save_mode, string title, string prompt, string defaultname)
{
	char						ctitle[256];
	static char				result[512];
	static char				filter_str[512];
	short					i,p;
	static OPENFILENAME		ofn;
	omt_AskUserList		&filter = filterobj.filters;
	omt_RTGDefineLocalTypeObj(OMediaWinRtgWindow,winrtg,superwindow);	

	for(i=0,p=0; i<filter.size();i++)
	{
		strcpy(filter_str+p, filter[i].descriptor.c_str());
		p += filter[i].descriptor.length()+1;

		strcpy(filter_str+p, filter[i].pattern.c_str());
		p += filter[i].pattern.length()+1;
	}

	
	filter_str[p] = 0;
	filter_str[p+1] = 0;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = winrtg->hwnd;
	ofn.lpstrFile = result;
	ofn.nMaxFile = 512;
	ofn.lpstrFilter = filter_str;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = omc_NULL;
	ofn.nMaxFileTitle =0;
	ofn.lpstrInitialDir = omc_NULL;	
	
	if (title.length())
	{
		strcpy(ctitle, title.c_str());
		ofn.lpstrTitle = ctitle;
	}
	else ofn.lpstrTitle = omc_NULL;

	if (!save_mode)
	{
		ofn.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
		if (!GetOpenFileName(&ofn)) return false;	
	}
	else
	{
		ofn.Flags = OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
		if (!GetSaveFileName(&ofn)) return false;
	}

	((OMediaWinRtgFilePath*)retarget)->path = result;

	return true;
}

