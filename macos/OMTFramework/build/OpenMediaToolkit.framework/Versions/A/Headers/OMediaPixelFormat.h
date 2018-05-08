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
#ifndef OMEDIA_PixelFormat_H
#define OMEDIA_PixelFormat_H
 
#include "OMediaTypes.h"
#include "OMediaEndianSupport.h"

typedef unsigned long omt_PixelFormat;

const omt_PixelFormat		ompixfc_RGB555 		= (1<<0L);
const omt_PixelFormat		ompixfc_RGB565 		= (1<<1L);
const omt_PixelFormat		ompixfc_ARGB1555 	= (1<<2L);
const omt_PixelFormat		ompixfc_RGB888 		= (1<<3L);
const omt_PixelFormat		ompixfc_ARGB8888 	= (1<<4L);
const omt_PixelFormat		ompixfc_ARGB4444 	= (1<<5L);


const omt_PixelFormat		ompixfc_Best 				= 0xFFFFFFFFL;	// Best format (most of the time try to get 8888)


const omt_PixelFormat		ompixfc_ResBest 			= 0xEFFFFFFFL;	// Best format for the current resolution

const omt_PixelFormat		ompixfc_ResBestAlpha		= 0xDFFFFFFFL;	// Same but try to get an alpha value per pixel

const omt_PixelFormat		ompixfc_ResBestAlpha1bit	= 0xCFFFFFFFL;	// Same but try to get at least 1 bit of alpha per pixel



//.................................................
// Pixel colors

typedef unsigned long omt_RGBAPixel;


#define omd_CGUN_R	0		// Color gun
#define omd_CGUN_G	1
#define omd_CGUN_B	2
#define omd_CGUN_A	3


//.................................................



typedef unsigned long omt_RGBAPixelMask;

const omt_RGBAPixelMask ompixmc_Red   = 0xFF000000UL;
const omt_RGBAPixelMask ompixmc_Green = 0x00FF0000UL;
const omt_RGBAPixelMask ompixmc_Blue  = 0x0000FF00UL;
const omt_RGBAPixelMask ompixmc_Alpha = 0x000000FFUL;
const omt_RGBAPixelMask ompixmc_Full  = 0xFFFFFFFFUL;




#endif


