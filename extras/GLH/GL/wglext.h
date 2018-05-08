#ifndef __wglext_h_
#define __wglext_h_

#ifdef __cplusplus
extern "C" {
#endif

/*
** Copyright 1998, 1999, NVIDIA Corporation.
** All rights Reserved.
** 
** THE INFORMATION CONTAINED HEREIN IS PROPRIETARY AND CONFIDENTIAL TO
** NVIDIA, CORPORATION.  USE, REPRODUCTION OR DISCLOSURE TO ANY THIRD PARTY
** IS SUBJECT TO WRITTEN PRE-APPROVAL BY NVIDIA, CORPORATION.
** 
** 
** Copyright 1992-1997 Silicon Graphics, Inc.
** All Rights Reserved.
** 
** Portions of this file are UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
** Inc.; the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
** 
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/

/*************************************************************/

/* Extensions */
#define WGL_EXT_extensions_string          1
#define WGL_EXT_pixel_format               1
#define WGL_EXT_swap_control               1
#define WGL_NV_allocate_memory             1

/* EXT_pixel_format */
#define WGL_NUMBER_PIXEL_FORMATS_EXT       0x2000
#define WGL_DRAW_TO_WINDOW_EXT             0x2001
#define WGL_DRAW_TO_BITMAP_EXT             0x2002
#define WGL_ACCELERATION_EXT               0x2003
#define WGL_NEED_PALETTE_EXT               0x2004
#define WGL_NEED_SYSTEM_PALETTE_EXT        0x2005
#define WGL_SWAP_LAYER_BUFFERS_EXT         0x2006
#define WGL_SWAP_METHOD_EXT                0x2007
#define WGL_NUMBER_OVERLAYS_EXT            0x2008
#define WGL_NUMBER_UNDERLAYS_EXT           0x2009
#define WGL_TRANSPARENT_EXT                0x200A
#define WGL_TRANSPARENT_VALUE_EXT          0x200B
#define WGL_SHARE_DEPTH_EXT                0x200C
#define WGL_SHARE_STENCIL_EXT              0x200D
#define WGL_SHARE_ACCUM_EXT                0x200E
#define WGL_SUPPORT_GDI_EXT                0x200F
#define WGL_SUPPORT_OPENGL_EXT             0x2010
#define WGL_DOUBLE_BUFFER_EXT              0x2011
#define WGL_STEREO_EXT                     0x2012
#define WGL_PIXEL_TYPE_EXT                 0x2013
#define WGL_COLOR_BITS_EXT                 0x2014
#define WGL_RED_BITS_EXT                   0x2015
#define WGL_RED_SHIFT_EXT                  0x2016
#define WGL_GREEN_BITS_EXT                 0x2017
#define WGL_GREEN_SHIFT_EXT                0x2018
#define WGL_BLUE_BITS_EXT                  0x2019
#define WGL_BLUE_SHIFT_EXT                 0x201A
#define WGL_ALPHA_BITS_EXT                 0x201B
#define WGL_ALPHA_SHIFT_EXT                0x201C
#define WGL_ACCUM_BITS_EXT                 0x201D
#define WGL_ACCUM_RED_BITS_EXT             0x201E
#define WGL_ACCUM_GREEN_BITS_EXT           0x201F
#define WGL_ACCUM_BLUE_BITS_EXT            0x2020
#define WGL_ACCUM_ALPHA_BITS_EXT           0x2021
#define WGL_DEPTH_BITS_EXT                 0x2022
#define WGL_STENCIL_BITS_EXT               0x2023
#define WGL_AUX_BUFFERS_EXT                0x2024
#define WGL_NO_ACCELERATION_EXT            0x2025
#define WGL_GENERIC_ACCELERATION_EXT       0x2026
#define WGL_FULL_ACCELERATION_EXT          0x2027
#define WGL_SWAP_EXCHANGE_EXT              0x2028
#define WGL_SWAP_COPY_EXT                  0x2029
#define WGL_SWAP_UNDEFINED_EXT             0x202A
#define WGL_TYPE_RGBA_EXT                  0x202B
#define WGL_TYPE_COLORINDEX_EXT            0x202C

/* ARB_pbuffer */
#define WGL_DRAW_TO_PBUFFER_ARB            0x202D
#define WGL_MAX_PBUFFER_PIXELS_ARB         0x202E
#define WGL_MAX_PBUFFER_WIDTH_ARB          0x202F
#define WGL_MAX_PBUFFER_HEIGHT_ARB         0x2030
#define WGL_OPTIMAL_PBUFFER_WIDTH_ARB      0x2031
#define WGL_OPTIMAL_PBUFFER_HEIGHT_ARB     0x2032
#define WGL_PBUFFER_LARGEST_ARB            0x2033
#define WGL_PBUFFER_WIDTH_ARB              0x2034
#define WGL_PBUFFER_HEIGHT_ARB             0x2035

/*************************************************************/


/* WGL_EXT_swap_control */
typedef int (APIENTRY * PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef int (APIENTRY * PFNWGLGETSWAPINTERVALEXTPROC) (void);

/* WGL_EXT_extensions_string */
typedef const char * (APIENTRY * PFNWGLGETEXTENSIONSSTRINGEXTPROC) (void);

/* WGL_EXT_pixel_format */
typedef BOOL (APIENTRY * PFNWGLGETPIXELFORMATATTRIBIVEXTPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, int *piAttributes, int *piValues);
typedef BOOL (APIENTRY * PFNWGLGETPIXELFORMATATTRIBFVEXTPROC) (HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, int *piAttributes, FLOAT *pfValues);
typedef BOOL (APIENTRY * PFNWGLCHOOSEPIXELFORMATEXTPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

/* WGL_NV_allocate_memory */
typedef void * (APIENTRY * PFNWGLALLOCATEMEMORYNVPROC) (int size, float readfreq, float writefreq, float priority);
typedef void (APIENTRY * PFNWGLFREEMEMORYNVPROC) (void *pointer);

#ifdef __cplusplus
}
#endif

#endif /* __wglext_h_ */
