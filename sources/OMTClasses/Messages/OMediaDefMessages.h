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
#ifndef OMEDIA_DefMessages_H
#define OMEDIA_DefMessages_H

#include "OMediaTypes.h"

typedef long omt_Message;

const omt_Message	omsg_NULL = 0;


// * Application

const omt_Message	omsg_Quit=-100;
const omt_Message	omsg_Event=-101;
	
const omt_Message	omsg_MouseClick	    =-102;	// Mouse down
const omt_Message	omsg_MouseClick_Up  =-103;	// Mouse button released



const omt_Message	omsg_KeyDown=-104;		// Message from keyboard broadcaster
const omt_Message	omsg_KeyUp=-105;		// Message from keyboard broadcaster
											// Parameter points to an OMediaEvent object.

const omt_Message	omsg_Select=-106;	
const omt_Message	omsg_ScrollerMoved=-107;

const omt_Message	omsg_LowLevelOSEvent=-108;
const omt_Message	omsg_LostFocus = -109;

const omt_Message	omsg_New =-110;
const omt_Message	omsg_Open =-111;
const omt_Message	omsg_Close =-112;
const omt_Message	omsg_Save =-113;
const omt_Message	omsg_SaveAs =-114;
const omt_Message	omsg_Revert =-115;
const omt_Message	omsg_PageSetup =-116;
const omt_Message	omsg_Print =-117;

const omt_Message	omsg_Undo =-120;
const omt_Message	omsg_Cut =-121;
const omt_Message	omsg_Copy =-122;
const omt_Message	omsg_Paste =-123;
const omt_Message	omsg_Clear =-124;
const omt_Message	omsg_SelectAll =-125;

const omt_Message	omsg_About =-130;

const omt_Message	omsg_OpenFileList =-131;	// Parameter points to vector<OMediaFilePath*>
												// Sent when a document is double clicked on the desktop

const omt_Message	omsg_LowlevelEventFilter = -180;

	
// * Palettes

const omt_Message	omsg_ReplacePalette  		= -200;
const omt_Message	omsg_PaletteModified 		= -201;
const omt_Message	omsg_UnbindPalettePlugins 	= -202;
const omt_Message	omsg_BindPalettePlugins 	= -203;


// * Memory handler

const omt_Message	omsg_MemoryRequired  = -300;	// param points to an bool that
													// should be set to true if some
													// memory has been disposed.

// * Engine

const omt_Message	omsg_EngineDeleted			= -350;
const omt_Message	omsg_OffscreenBufferDeleted	= -351;


// * Button

const omt_Message	omsg_RadioButtonDown = -400;


// * Screen/Window

const omt_Message	omsg_ScreenModeChanged 				= -500;	 // Sent after screen mode has been changed
const omt_Message	omsg_ScreenModeWillChange			= -501;	 // Sent before screen mode will be changed
const omt_Message	omsg_ScreenDeviceTypeChanged		= -502;  // Sent when a new screen device type is installed
const omt_Message	omsg_ResetDisplaySize				= -503; 
const omt_Message	omsg_VideoEngineLinked				= -504;
const omt_Message	omsg_VideoEngineUnlinked			= -505;
const omt_Message	omsg_VideoEngineDeleted				= -506;
const omt_Message	omsg_WindowDeleted					= -507;
const omt_Message	omsg_WindowRendererUpdated			= -508;
const omt_Message	omsg_WindowBoundsChanged			= -509;


// * World/Viewport/Element/Renderer

const omt_Message	omsg_ViewPortClicked	= -600;		// param points to an OMediaPickRequest
														// object
const omt_Message	omsg_ViewPortClicked_Up	= -601;		// param points to an OMediaPickRequest
														// object

const omt_Message	omsg_MouseTrack	 		= -602;		// param points to an OMediaMouseTrackPick
														// object

const omt_Message	omsg_ViewPortDeleted 	= -603;
const omt_Message	omsg_UpdateWorldLogic 	= -604;
const omt_Message	omsg_RenderFrames	 	= -605;
const omt_Message	omsg_RenderPort	 		= -606;
const omt_Message	omsg_VPCheckMouseTrack	= -607;
const omt_Message	omsg_HasFocus			= -608;		// param points to a bool that should
														// be filled with true, if element
														// has the focus


// * Renderer

const omt_Message	omsg_RenderTargetDeleted 	= -700;
const omt_Message	omsg_RendererDeleted 		= -701;
const omt_Message	omsg_RenderPortDeleted 		= -702;
const omt_Message	omsg_RendererSelected 		= -703;

// * Internet

const omt_Message       omsg_InternetStatus         = -800;


#endif

