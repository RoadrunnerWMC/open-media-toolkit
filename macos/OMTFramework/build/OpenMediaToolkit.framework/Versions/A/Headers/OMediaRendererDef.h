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
#ifndef OMEDIA_RendererDef_H
#define OMEDIA_RendererDef_H
 
#include "OMediaTypes.h"
#include "OMediaEngineID.h"
#include "OMediaPixelFormat.h"

#include <vector>
#include <string>

typedef unsigned long omt_RendererAttributes;

const omt_RendererAttributes omcrdattr_Accelerated 			= (1<<0);			// Hardware accelerated
const omt_RendererAttributes omcrdattr_Blending 			= (1<<2);			// Blending
const omt_RendererAttributes omcrdattr_Texture 				= (1<<3);			// Texture-mapping
const omt_RendererAttributes omcrdattr_AntiAliasing 		= (1<<4);			// Anti-aliasing
const omt_RendererAttributes omcrdattr_TextureFiltering 	= (1<<5);			// Texture filtering
const omt_RendererAttributes omcrdattr_TextureColor 		= (1<<6);			// Texture color modulation
const omt_RendererAttributes omcrdattr_Fog 					= (1<<7);			// Fog support
const omt_RendererAttributes omcrdattr_Gouraud 				= (1<<8);			// Gouraud shading
const omt_RendererAttributes omcrdattr_MipMapping 			= (1<<9);			// Texture mip-mapping
const omt_RendererAttributes omcrdattr_TLAccelerated 		= (1<<10);		// Transform and light hardware accelerated


typedef unsigned short omt_ZBufferBitDepthFlags;
const omt_ZBufferBitDepthFlags omfzbc_NoZBuffer =  0;
const omt_ZBufferBitDepthFlags omfzbc_8Bits =  (1<<0);
const omt_ZBufferBitDepthFlags omfzbc_16Bits = (1<<1);
const omt_ZBufferBitDepthFlags omfzbc_24Bits = (1<<2);
const omt_ZBufferBitDepthFlags omfzbc_32Bits = (1<<3);
const omt_ZBufferBitDepthFlags omfzbc_64Bits = (1<<4);


typedef void *omt_RendererPrivateID;

class OMediaRendererDef
{
	public:

	omt_EngineID					engine_id;
	omt_RendererPrivateID			private_id;

	string							name;
	omt_RendererAttributes			attributes;	
	omt_ZBufferBitDepthFlags		zbuffer_depth;
	omt_PixelFormat					pixel_format;

	char							private_data[16];
};

typedef vector<OMediaRendererDef> 	omt_RendererDefList;



#endif

