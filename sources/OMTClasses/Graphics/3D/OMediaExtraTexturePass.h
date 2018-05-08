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
#ifndef OMEDIA_ExtraTexturePass_H
#define OMEDIA_ExtraTexturePass_H

#include "OMediaRenderConstants.h"

#include <list>
#include <vector>

class OMediaCanvas;

class OMediaExtraTexturePassUV
{
public:

	float	u,v;
};

typedef vector<OMediaExtraTexturePassUV> omt_ExtraTexturePassUV;


class OMediaExtraTexturePass
{
public:

	// * Construction

	omtshared OMediaExtraTexturePass();
	omtshared OMediaExtraTexturePass(const OMediaExtraTexturePass &t);
	omtshared ~OMediaExtraTexturePass();

	// * Texture

	omtshared void set_texture(OMediaCanvas	*texture);
	OMediaCanvas *get_texture(void) const {return texture;}

	// * Attributes

	inline void set_texture_address_mode(const omt_TextureAddressMode am)
		{texture_address_mode = am;}

	inline omt_TextureAddressMode get_texture_address_mode(void) const
		{return texture_address_mode;}

	inline void set_texture_color_operation(const omt_TextureColorOperation am)
		{texture_color_operation = am;}

	inline omt_TextureColorOperation get_texture_color_operation(void) const
		{return texture_color_operation;}


	// * Operator

	omtshared OMediaExtraTexturePass &operator = (const OMediaExtraTexturePass &tp);

protected:

	OMediaCanvas				*texture;
	omt_TextureAddressMode		texture_address_mode;
	omt_TextureColorOperation	texture_color_operation;
};

typedef list<OMediaExtraTexturePass> omt_ExtraTexturePassList;


#endif