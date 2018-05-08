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
#ifndef OMEDIA_RenderConstants_H
#define OMEDIA_RenderConstants_H

#include "OMediaTypes.h"

enum omt_FillMode
{
	omfillmc_Point,
	omfillmc_Line,
	omfillmc_Solid
};

enum omt_ShadeMode
{
	omshademc_Flat,
	omshademc_Gouraud
};

enum omt_ZBufferWrite
{
	omzbwc_Disabled,
	omzbwc_Enabled
};

enum omt_ZBufferTest
{
	omzbtc_Disabled,
	omzbtc_Enabled
};

enum omt_ZBufferFunc
{
	omzbfc_Never,
	omzbfc_Always,
	omzbfc_Less,
	omzbfc_LEqual,
	omzbfc_Equal,
	omzbfc_GEqual,
	omzbfc_Greater,
	omzbfc_NotEqual
};


enum omt_LightType
{
	omclt_Directional,			
	omclt_Point,				
	omclt_Spot
};

enum omt_Blend
{
	omblendc_Disabled,
	omblendc_Enabled
};

enum omt_BlendFunc
{
	omblendfc_Zero,
	omblendfc_One,
	omblendfc_Dst_Color,
	omblendfc_Src_Color,
	omblendfc_Inv_Dst_Color,
	omblendfc_Inv_Src_Color,
	omblendfc_Src_Alpha,
	omblendfc_Inv_Src_Alpha,
	omblendfc_Dst_Alpha,
	omblendfc_Inv_Dst_Alpha,
	omblendfc_Src_Alpha_Saturate
};

enum omt_RenderDrawMode
{
	omrdmc_Points,			// points
	omrdmc_Lines,			// npts/2 lines	
	omrdmc_LineStrip,		// connected lines
	omrdmc_LineLoop,		// connected and closed group of lines.
	omrdmc_Triangles,		// triplet of vertices are independent triangle
	omrdmc_TriangleStrip,	// strip. n-2 triangles are drawn
	omrdmc_TriangleFan,		// fan. n-2 triangles are drawn
	omrdmc_Polygon			// convex polygon
};

enum omt_CanvasFiltering
{
	omtfc_Nearest,
	omtfc_Linear,

	omtfc_Nearest_Mipmap_Nearest,
	omtfc_Nearest_Mipmap_Linear,
	omtfc_Linear_Mipmap_Nearest,
	omtfc_Linear_Mipmap_Linear
};

enum omt_TextureAddressMode
{
	omtamc_Clamp,
	omtamc_Wrap
};

enum omt_TextureColorOperation
{
	omtcoc_Replace,
	omtcoc_Modulate
};

#endif

