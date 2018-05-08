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
 
 
#include "OMediaWinStoreStartInfo.h"
 
extern void omt_main(void);

INT CALLBACK WinMain(HINSTANCE inst, HINSTANCE prev_inst, CHAR *commandLine, INT showState)
{
	OMediaWinStoreStartInfo::store(inst, prev_inst, commandLine, showState);

	char	*name,*p;
	name = new char[512];
	if (GetModuleFileName(omc_NULL,name,512))
	{
		p = name + strlen(name);

		if (p!=name)
		for(;;)
		{
			p--;
			
			if (p==name)
			{
				name[0]=0;
				break;
			}

			if (*p=='\\' || *p==':')
			{
				p[1] = 0;
				break;
			}
		}

		if (name[0]) SetCurrentDirectory( name);	
	}
	delete [] name;

	omt_main();

	return 0;
}
