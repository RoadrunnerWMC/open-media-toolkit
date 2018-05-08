# Microsoft Developer Studio Project File - Name="Sound" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Sound - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Sound.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Sound.mak" CFG="Sound - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Sound - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Sound - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Sound - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/directx" /I "$(OMT2ROOT)/sources/omtclasses/world/anim" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/input" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/window" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/sound" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/engines" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/application" /I "$(OMT2ROOT)/sources/omtclasses/application" /I "$(OMT2ROOT)/sources/omtclasses/compression" /I "$(OMT2ROOT)/sources/omtclasses/core" /I "$(OMT2ROOT)/sources/omtclasses/engines" /I "$(OMT2ROOT)/sources/omtclasses/graphics" /I "$(OMT2ROOT)/sources/omtclasses/input" /I "$(OMT2ROOT)/sources/omtclasses/math" /I "$(OMT2ROOT)/sources/omtclasses/messages" /I "$(OMT2ROOT)/sources/omtclasses/os" /I "$(OMT2ROOT)/sources/omtclasses/sound" /I "$(OMT2ROOT)/sources/omtclasses/stream" /I "$(OMT2ROOT)/sources/omtclasses/utility" /I "$(OMT2ROOT)/sources/omtclasses/window" /I "$(OMT2ROOT)/sources/omtclasses/world" /I "$(OMT2ROOT)/sources/omtclasses/graphics/3d" /I "$(OMT2ROOT)/sources/omtclasses/graphics/canvas" /I "$(OMT2ROOT)/sources/omtclasses/graphics/core" /I "$(OMT2ROOT)/sources/omtclasses/graphics/renderers" /I "$(OMT2ROOT)/sources/omtclasses/os/opengl/graphics/canvas" /I "$(OMT2ROOT)/sources/omtclasses/os/opengl/graphics/renderers" /I "$(OMT2ROOT)/sources/omtclasses/stream/class" /I "$(OMT2ROOT)/sources/omtclasses/stream/converter" /I "$(OMT2ROOT)/sources/omtclasses/stream/core" /I "$(OMT2ROOT)/sources/omtclasses/stream/database" /I "$(OMT2ROOT)/sources/omtclasses/stream/formatted" /I "$(OMT2ROOT)/sources/omtclasses/world/core" /I "$(OMT2ROOT)/sources/omtclasses/world/element" /I "$(OMT2ROOT)/sources/omtclasses/world/ui" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "omd_DLLImport" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 "$(OMT2ROOT)/mswindows/OMTDLL/release/OMT2.lib" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"../../Sound.exe"

!ELSEIF  "$(CFG)" == "Sound - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/directx" /I "$(OMT2ROOT)/sources/omtclasses/world/anim" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/input" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/window" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/sound" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/engines" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/application" /I "$(OMT2ROOT)/sources/omtclasses/application" /I "$(OMT2ROOT)/sources/omtclasses/compression" /I "$(OMT2ROOT)/sources/omtclasses/core" /I "$(OMT2ROOT)/sources/omtclasses/engines" /I "$(OMT2ROOT)/sources/omtclasses/graphics" /I "$(OMT2ROOT)/sources/omtclasses/input" /I "$(OMT2ROOT)/sources/omtclasses/math" /I "$(OMT2ROOT)/sources/omtclasses/messages" /I "$(OMT2ROOT)/sources/omtclasses/os" /I "$(OMT2ROOT)/sources/omtclasses/sound" /I "$(OMT2ROOT)/sources/omtclasses/stream" /I "$(OMT2ROOT)/sources/omtclasses/utility" /I "$(OMT2ROOT)/sources/omtclasses/window" /I "$(OMT2ROOT)/sources/omtclasses/world" /I "$(OMT2ROOT)/sources/omtclasses/graphics/3d" /I "$(OMT2ROOT)/sources/omtclasses/graphics/canvas" /I "$(OMT2ROOT)/sources/omtclasses/graphics/core" /I "$(OMT2ROOT)/sources/omtclasses/graphics/renderers" /I "$(OMT2ROOT)/sources/omtclasses/os/opengl/graphics/canvas" /I "$(OMT2ROOT)/sources/omtclasses/os/opengl/graphics/renderers" /I "$(OMT2ROOT)/sources/omtclasses/stream/class" /I "$(OMT2ROOT)/sources/omtclasses/stream/converter" /I "$(OMT2ROOT)/sources/omtclasses/stream/core" /I "$(OMT2ROOT)/sources/omtclasses/stream/database" /I "$(OMT2ROOT)/sources/omtclasses/stream/formatted" /I "$(OMT2ROOT)/sources/omtclasses/world/core" /I "$(OMT2ROOT)/sources/omtclasses/world/element" /I "$(OMT2ROOT)/sources/omtclasses/world/ui" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "omd_DLLImport" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 "$(OMT2ROOT)/mswindows/OMTDLL/debug/OMT2Debug.lib" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../../SoundDebug.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Sound - Win32 Release"
# Name "Sound - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="$(OMT2ROOT)/Sources/OMTClasses\Core\OMediaDLLSupport.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)/Sources/OMTClasses\Os\MSWindows\Application\OMediaWinMain.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)/Sources\samples\Sound.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
