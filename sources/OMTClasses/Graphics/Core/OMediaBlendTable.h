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
#ifndef OMEDIA_OMediaBlendTable_H
#define OMEDIA_OMediaBlendTable_H

#include "OMediaTypes.h"
#include "OMediaRendererInterface.h"
#include "OMediaPixelFormat.h"
#include "OMediaEndianSupport.h"

#ifdef omd_LITTLE_ENDIAN
#define omd_RGBAtoARGB_32(p) ((((p)&0xFF)<<16) | (((p)&0xFF0000)>>16) | ((p)&0x00FF00))
#define omd_RGBAMULALPHA_32(p,a) ( ((((((p)>>24))*a)>>8)<<24) | ((p)&0x00FFFFFF) )
#define omd_ARGBtoRGBA_A32(p) ((((p)&0xFF)<<16) | (((p)&0xFF0000)>>16) | ((p)&0xFF00FF00))
#define omd_RGBAUnPackPixelA32(p,a,r,g,b) {a = ((p)>>24)&0xFF; b = ((p)>>16)&0xFF; g = ((p)>>8)&0xFF; r = (p)&0xFF;}
#else
#define omd_RGBAtoARGB_32(p) ((p)>>8)
#define omd_RGBAMULALPHA_32(p,a) ( ((((p)&0xFF)*a)>>8) | ((p)&0xFFFFFF00) )
#define omd_ARGBtoRGBA_A32(p) ((((p)<<8)&0xFFFFFF00)|((p)>>24))
#define omd_RGBAUnPackPixelA32(p,a,r,g,b) {r = ((p)>>24)&0xFF; g = ((p)>>16)&0xFF; b = ((p)>>8)&0xFF; a = (p)&0xFF;}
#endif



#define omd_ARGBPackPixel32(r,g,b) (((r)<<16)|((g)<<8)|((b)))
#define omd_ARGBPackPixelA32(a,r,g,b) (((a)<<24)|((r)<<16)|((g)<<8)|((b)))

#define omd_ARGBUnPackPixel32(p,r,g,b) {r = ((p)>>16)&0xFF; g = ((p)>>8)&0xFF; b = (p)&0xFF;}
#define omd_ARGBUnPackPixelA32(p,a,r,g,b) {a = ((p)>>24)&0xFF; r = ((p)>>16)&0xFF; g = ((p)>>8)&0xFF; b = (p)&0xFF;}

#define omd_ARGBPackPixel16(r,g,b) ( (((r)>>3)<<10)|(((g)>>3)<<5)|((b)>>3) ) 
#define omd_ARGBPackPixelA16(a,r,g,b) ( (((a)>>7)<<15)|(((r)>>3)<<10)|(((g)>>3)<<5)|((b)>>3) )

#define omd_ARGBUnPackPixel16(p,r,g,b) {r = (((p)>>7)&(0x1F<<3)); g = (((p)>>2)&(0x1F<<3)); b = ((p)&0x1F)<<3;}
#define omd_ARGBUnPackPixelA16(p,a,r,g,b) {a = (((p)&(1<<15))?0xFF:0); r = (((p)>>7)&(0x1F<<3)); g = (((p)>>2)&(0x1F<<3)); b = ((p)&0x1F)<<3;}

#define omd_ARGB32to16(p) ( (((p)>>9)&(0x1F<<10)) | (((p)>>6)&(0x1F<<5)) | (((p)>>3)&0x1F) )
#define omd_ARGB32toA16(p) ( (((p)>>16)&(1<<15)) | (((p)>>9)&(0x1F<<10)) | (((p)>>6)&(0x1F<<5)) | (((p)>>3)&0x1F) )

#define omd_ARGB16to32(p) ( (((p)&(0x1F<<10))<<9) | (((p)&(0x1F<<5))<<6) | (((p)&0x1F)<<3) )
#define omd_ARGB16toA32(p) ( (((p)&(1<<15))?(0xFF<<24):0) | (((p)&(0x1F<<10))<<9) | (((p)&(0x1F<<5))<<6) | (((p)&0x1F)<<3) )



typedef unsigned long omt_BlendPixel;
typedef void (*omt_BlendFunctionPtr) (omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);

class OMediaBlendTable
{
	public:

	// RGBA

	omtshared static void omf_bfunc_Zero(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_One(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_Dst_Color(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_Src_Color(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_Inv_Dst_Color(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_Inv_Src_Color(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_Src_Alpha(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_Inv_Src_Alpha(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_Dst_Alpha(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_Inv_Dst_Alpha(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_Src_Alpha_Saturate(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);

	omtshared static omt_BlendFunctionPtr	find_blend_func(omt_BlendFunc f);

	// ARGB

	omtshared static void omf_bfunc_argb_Zero(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_argb_One(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_argb_Dst_Color(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_argb_Src_Color(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_argb_Inv_Dst_Color(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_argb_Inv_Src_Color(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_argb_Src_Alpha(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_argb_Inv_Src_Alpha(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_argb_Dst_Alpha(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_argb_Inv_Dst_Alpha(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	omtshared static void omf_bfunc_argb_Src_Alpha_Saturate(omt_BlendPixel *pixel, omt_BlendPixel *src_pixel, omt_BlendPixel *dest_pixel, short *pixel_res);
	
	omtshared static omt_BlendFunctionPtr	find_blend_func_argb(omt_BlendFunc f);


};



#endif

