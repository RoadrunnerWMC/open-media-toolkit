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


 

#ifndef OMEDIA_USBHIDUsages_H
#define OMEDIA_USBHIDUsages_H

#include "OMediaTypes.h"


// Usage page

enum omt_HIDUsagePage
{
	omhidupc_Undefined 				= 0x00,
	omhidupc_GenericDesktopControl	= 0x01,
	omhidupc_SimulationControl		= 0x02,
	omhidupc_VRControl				= 0x03,
	omhidupc_SportControl			= 0x04,
	omhidupc_GameControl			= 0x05,
	
	omhidupc_OMTControl				= 0xFFFF	// These are non-usb values defined
};												// by OMT. When OMT detects
												// informations that does not fit
												// in usb definitions, it uses this
												// special page type to store usage info.
												// See omt_HIDOMTUsage.

// Usage types

enum omt_HIDGenericDesktopUsage
{
	omhidgduc_Undefined = 0x00,
	omhidgduc_Pointer = 0x01,
	omhidgduc_Mouse = 0x02,
	omhidgduc_Joystick = 0x04,
	omhidgduc_GamePad = 0x05,
	omhidgduc_Keyboard = 0x06,
	omhidgduc_Keypad = 0x07,
	omhidgduc_MultiaxisController = 0x08,
	omhidgduc_X = 0x30,
	omhidgduc_Y = 0x31,
	omhidgduc_Z = 0x32,
	omhidgduc_Rx = 0x33,
	omhidgduc_Ry = 0x34,
	omhidgduc_Rz = 0x36,
	omhidgduc_Slider = 0x36,
	omhidgduc_Dial = 0x37,
	omhidgduc_Wheel = 0x38,
	omhidgduc_HatSwitch = 0x39,
	omhidgduc_Start = 0x3d,
	omhidgduc_Select = 0x3e,
	omhidgduc_Vx = 0x40,
	omhidgduc_Vy = 0x41,
	omhidgduc_Vz = 0x42,
	omhidgduc_Vbrx = 0x43,
	omhidgduc_Vbry = 0x44,
	omhidgduc_Vbrz = 0x45,
	omhidgduc_Vno = 0x46
};


enum omt_HIDSimulationControlUsage
{
	omhidscuc_Undefined = 0x00,
	omhidscuc_FlightSimulationDevice = 0x01,
	omhidscuc_AutomobileSimulationDevice = 0x02,
	omhidscuc_TankSimulationDevice = 0x03,
	omhidscuc_SpaceshipSimulationDevice = 0x04,
	omhidscuc_SubmarineSimulationDevice = 0x05,
	omhidscuc_SailingSimulationDevice  = 0x06,
	omhidscuc_MotorcycleSimulationDevice = 0x07,
	omhidscuc_SportsSimulationDevice = 0x08,
	omhidscuc_AirplaneSimulationDevice = 0x09,
	omhidscuc_HelicopterSimulationDevice = 0x0A,
	omhidscuc_MagicCarpetSimulationDevice = 0x0B,
	omhidscuc_BicycleSimulationDevice = 0x0C,
	omhidscuc_FlightControlStick = 0x20,
	omhidscuc_FlightStick = 0x21,
	omhidscuc_CyclicControl = 0x22,
	omhidscuc_CyclicTrim = 0x23,
	omhidscuc_FlightYoke = 0x24,
	omhidscuc_TrackControl = 0x25,
	omhidscuc_Aileron = 0xB0,
	omhidscuc_AileronTrim = 0xB1,
	omhidscuc_AntiTorqueControl = 0xB2,
	omhidscuc_AutopilotEnable = 0xB3,
	omhidscuc_ChaffRelease = 0xB4,
	omhidscuc_CollectiveControl = 0xB5,
	omhidscuc_DiveBrake = 0xB6,
	omhidscuc_ElectronicCountermeasures = 0xB7,
	omhidscuc_Elevator = 0xB8,
	omhidscuc_ElevatorTrim = 0xB9,
	omhidscuc_Rudder = 0xBA,
	omhidscuc_Throttle = 0xBB,
	omhidscuc_FlightCommunications = 0xBC,
	omhidscuc_FlareRelease = 0xBD,
	omhidscuc_LandingGear = 0xBE,
	omhidscuc_ToeBrake = 0xBF,
	omhidscuc_Trigger = 0xC0,
	omhidscuc_WeaponsArm = 0xC1,
	omhidscuc_WeaponsSelect = 0xC2,
	omhidscuc_WingFlaps = 0xC3,
	omhidscuc_Accelerator = 0xC4,
	omhidscuc_Brake = 0xC5,
	omhidscuc_Clutch = 0xC6,
	omhidscuc_Shifter = 0xC7,
	omhidscuc_Steering = 0xC8,
	omhidscuc_TurretDirection = 0xC9,
	omhidscuc_BarrelElevation = 0xCA,
	omhidscuc_DivePlane = 0xCB,
	omhidscuc_Ballast = 0xCC,
	omhidscuc_BicycleCrank = 0xCD,
	omhidscuc_HandleBars = 0xCE,
	omhidscuc_FrontBrake = 0xCF,
	omhidscuc_RearBrake = 0xD0
};


enum omt_HIDVRControlUsage
{
	omhidvruc_Undefined = 0x00,
	omhidvruc_Belt = 0x01 ,
	omhidvruc_BodySuit = 0x02 ,
	omhidvruc_Flexor = 0x03 ,
	omhidvruc_Glove = 0x04 ,
	omhidvruc_HeadTracker = 0x05 ,
	omhidvruc_HeadMountedDisplay = 0x06 ,
	omhidvruc_HandTracker = 0x07 ,
	omhidvruc_Oculometer = 0x08 ,
	omhidvruc_Vest = 0x09 ,
	omhidvruc_AnimatronicDevice = 0x0A ,
	omhidvruc_StereoEnable = 0x20 ,
	omhidvruc_DisplayEnable = 0x21
};

enum omt_HIDSportControlUsage
{
	omhidspuc_Undefined = 0x00,
	omhidspuc_BaseballBat = 0x01,
	omhidspuc_GolfClub = 0x02,
	omhidspuc_RowingMachine = 0x03,
	omhidspuc_Treadmill = 0x04,
	omhidspuc_Oar = 0x30,
	omhidspuc_Slope = 0x31,
	omhidspuc_Rate = 0x32,
	omhidspuc_StickSpeed = 0x33,
	omhidspuc_StickFaceAngle = 0x34,
	omhidspuc_StickHeelToe = 0x35,
	omhidspuc_StickFollowThrough = 0x36,
	omhidspuc_StickTempo = 0x37,
	omhidspuc_StickTypeNAry = 0x38,
	omhidspuc_StickHeight = 0x39,
	omhidspuc_Putter = 0x50,
	omhidspuc_Iron1 = 0x51,
	omhidspuc_Iron2 = 0x52,
	omhidspuc_Iron3 = 0x53,
	omhidspuc_Iron4 = 0x54,
	omhidspuc_Iron5 = 0x55,
	omhidspuc_Iron6 = 0x56,
	omhidspuc_Iron7 = 0x57,
	omhidspuc_Iron8 = 0x58,
	omhidspuc_Iron9 = 0x59,
	omhidspuc_Iron10 = 0x5A,
	omhidspuc_Iron11 = 0x5B,
	omhidspuc_SandWedge = 0x5C,
	omhidspuc_LoftWedge = 0x5D,
	omhidspuc_PowerWedge = 0x5E,
	omhidspuc_Wood1 = 0x5F,
	omhidspuc_Wood3 = 0x60,
	omhidspuc_Wood5 = 0x61,
	omhidspuc_Wood7 = 0x62,
	omhidspuc_Wood9 = 0x63
};

enum omt_HIDGameControlUsage
{
	omhidgmuc_Undefined = 0x00,
	omhidgmuc_3DGameController = 0x01,
	omhidgmuc_PinballDevice = 0x02,
	omhidgmuc_GunDevice = 0x03,
	omhidgmuc_PointofView = 0x20,
	omhidgmuc_TurnRightLeft = 0x21,
	omhidgmuc_PitchRightLeft = 0x22,
	omhidgmuc_RollForwardBackward = 0x23,
	omhidgmuc_MoveRightLeft = 0x24,
	omhidgmuc_MoveForwardBackward = 0x25,
	omhidgmuc_MoveUpDown = 0x26,
	omhidgmuc_LeanRightLeft = 0x27,
	omhidgmuc_LeanForwardBackward = 0x28,
	omhidgmuc_HeightofPOV = 0x29,
	omhidgmuc_Flipper = 0x2A,
	omhidgmuc_SecondaryFlipper = 0x2B,
	omhidgmuc_Bump = 0x2C,
	omhidgmuc_NewGame = 0x2D,
	omhidgmuc_ShootBall = 0x2E,
	omhidgmuc_Player = 0x2F,
	omhidgmuc_GunBolt = 0x30,
	omhidgmuc_GunClip = 0x31,
	omhidgmuc_GunSelector = 0x32,
	omhidgmuc_GunSingleShot = 0x33,
	omhidgmuc_GunBurst = 0x34,
	omhidgmuc_GunAutomatic = 0x35,
	omhidgmuc_GunSafety = 0x36,
	omhidgmuc_GamepadFireJump = 0x37,
	omhidgmuc_GamepadTrigger = 0x39
};

enum omt_HIDOMTUsage
{
	omhidomuc_Undefined,
	omhidomuc_AxisRollTrim,
	omhidomuc_AxisPitchTrim,
	omhidomuc_AxisYawTrim,
	omhidomuc_DeltaX,
	omhidomuc_DeltaY,
	omhidomuc_DeltaZ,
	omhidomuc_DeltaRx,
	omhidomuc_DeltaRy,
	omhidomuc_DeltaRz,
	omhidomuc_PadMove,
	omhidomuc_SecondaryFire,
	omhidomuc_Jump,
	omhidomuc_PauseResume,
	omhidomuc_SlideLeft,
	omhidomuc_SlideRight,
	omhidomuc_MoveForward,
	omhidomuc_MoveBackward,
	omhidomuc_TurnLeft,
	omhidomuc_TurnRight,
	omhidomuc_LookLeft,
	omhidomuc_LookRight,
	omhidomuc_LookUp,
	omhidomuc_LookDown,
	omhidomuc_Next,
	omhidomuc_Previous,
	omhidomuc_SideStep,
	omhidomuc_Run,
	omhidomuc_Look
};

// HID Usage class

class OMediaHIDUsage
{
	public:
	
	inline OMediaHIDUsage() {usage_page = omhidupc_Undefined; unknown_type=0; }

	omt_HIDUsagePage	usage_page;
	
	// Type
	
	union
	{
		omt_HIDGenericDesktopUsage			desktop_type;
		omt_HIDSimulationControlUsage		simulation_type;
		omt_HIDVRControlUsage				vr_type;
		omt_HIDSportControlUsage			sport_type;
		omt_HIDGameControlUsage				game_type;
		omt_HIDOMTUsage						omt_type;
		unsigned short						unknown_type;
	};

};


#endif

