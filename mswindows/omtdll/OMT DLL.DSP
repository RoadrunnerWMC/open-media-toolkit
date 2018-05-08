# Microsoft Developer Studio Project File - Name="OMT DLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=OMT DLL - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "OMT DLL.MAK".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OMT DLL.MAK" CFG="OMT DLL - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OMT DLL - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "OMT DLL - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "OMT DLL - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /w /W0 /GX /Zd /O2 /I "$(OMT2ROOT)/extras/zlib-1.1.4" /I "$(OMT2ROOT)/extras/GLH" /I "$(OMT2ROOT)/sources/omtclasses/world/anim" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/directx" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/input" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/window" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/sound" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/engines" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/application" /I "$(OMT2ROOT)/sources/omtclasses/application" /I "$(OMT2ROOT)/sources/omtclasses/compression" /I "$(OMT2ROOT)/sources/omtclasses/core" /I "$(OMT2ROOT)/sources/omtclasses/engines" /I "$(OMT2ROOT)/sources/omtclasses/graphics" /I "$(OMT2ROOT)/sources/omtclasses/input" /I "$(OMT2ROOT)/sources/omtclasses/math" /I "$(OMT2ROOT)/sources/omtclasses/messages" /I "$(OMT2ROOT)/sources/omtclasses/os" /I "$(OMT2ROOT)/sources/omtclasses/sound" /I "$(OMT2ROOT)/sources/omtclasses/stream" /I "$(OMT2ROOT)/sources/omtclasses/utility" /I "$(OMT2ROOT)/sources/omtclasses/window" /I "$(OMT2ROOT)/sources/omtclasses/world" /I "$(OMT2ROOT)/sources/omtclasses/graphics/3d" /I "$(OMT2ROOT)/sources/omtclasses/graphics/canvas" /I "$(OMT2ROOT)/sources/omtclasses/graphics/core" /I "$(OMT2ROOT)/sources/omtclasses/graphics/renderers" /I "$(OMT2ROOT)/sources/omtclasses/os/opengl/graphics/canvas" /I "$(OMT2ROOT)/sources/omtclasses/os/opengl/graphics/renderers" /I "$(OMT2ROOT)/sources/omtclasses/stream/class" /I "$(OMT2ROOT)/sources/omtclasses/stream/converter" /I "$(OMT2ROOT)/sources/omtclasses/stream/core" /I "$(OMT2ROOT)/sources/omtclasses/stream/database" /I "$(OMT2ROOT)/sources/omtclasses/stream/formatted" /I "$(OMT2ROOT)/sources/omtclasses/world/core" /I "$(OMT2ROOT)/sources/omtclasses/world/element" /I "$(OMT2ROOT)/sources/omtclasses/world/ui" /I "$(OMT2ROOT)/sources/omtclasses/graphics/pipeline" /I "$(OMT2ROOT)/sources/omtclasses/graphics/rasterizer" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/graphics/core" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "omd_DLLExport" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x100c /d "NDEBUG"
# ADD RSC /l 0x100c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 opengl32.lib dinput.lib dxguid.lib d3drm.lib ddraw.lib dsound.lib winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"OMT2.dll"

!ELSEIF  "$(CFG)" == "OMT DLL - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /Zi /Od /I "$(OMT2ROOT)/extras/zlib-1.1.4" /I "$(OMT2ROOT)/extras/GLH" /I "$(OMT2ROOT)/sources/omtclasses/world/anim" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/directx" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/input" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/window" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/sound" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/engines" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/application" /I "$(OMT2ROOT)/sources/omtclasses/application" /I "$(OMT2ROOT)/sources/omtclasses/compression" /I "$(OMT2ROOT)/sources/omtclasses/core" /I "$(OMT2ROOT)/sources/omtclasses/engines" /I "$(OMT2ROOT)/sources/omtclasses/graphics" /I "$(OMT2ROOT)/sources/omtclasses/input" /I "$(OMT2ROOT)/sources/omtclasses/math" /I "$(OMT2ROOT)/sources/omtclasses/messages" /I "$(OMT2ROOT)/sources/omtclasses/os" /I "$(OMT2ROOT)/sources/omtclasses/sound" /I "$(OMT2ROOT)/sources/omtclasses/stream" /I "$(OMT2ROOT)/sources/omtclasses/utility" /I "$(OMT2ROOT)/sources/omtclasses/window" /I "$(OMT2ROOT)/sources/omtclasses/world" /I "$(OMT2ROOT)/sources/omtclasses/graphics/3d" /I "$(OMT2ROOT)/sources/omtclasses/graphics/canvas" /I "$(OMT2ROOT)/sources/omtclasses/graphics/core" /I "$(OMT2ROOT)/sources/omtclasses/graphics/renderers" /I "$(OMT2ROOT)/sources/omtclasses/os/opengl/graphics/canvas" /I "$(OMT2ROOT)/sources/omtclasses/os/opengl/graphics/renderers" /I "$(OMT2ROOT)/sources/omtclasses/stream/class" /I "$(OMT2ROOT)/sources/omtclasses/stream/converter" /I "$(OMT2ROOT)/sources/omtclasses/stream/core" /I "$(OMT2ROOT)/sources/omtclasses/stream/database" /I "$(OMT2ROOT)/sources/omtclasses/stream/formatted" /I "$(OMT2ROOT)/sources/omtclasses/world/core" /I "$(OMT2ROOT)/sources/omtclasses/world/element" /I "$(OMT2ROOT)/sources/omtclasses/world/ui" /I "$(OMT2ROOT)/sources/omtclasses/graphics/pipeline" /I "$(OMT2ROOT)/sources/omtclasses/graphics/rasterizer" /I "$(OMT2ROOT)/sources/omtclasses/os/mswindows/graphics/core" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "omd_DLLExport" /YX /FD /I /../OMTClasses/core" " /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x100c /d "_DEBUG"
# ADD RSC /l 0x100c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib dinput.lib dxguid.lib d3drm.lib ddraw.lib dsound.lib winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"OMT2debug.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "OMT DLL - Win32 Release"
# Name "OMT DLL - Win32 Debug"
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\adler32.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\compress.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\crc32.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\deflate.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\deflate.h"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\gzio.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\infblock.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\infblock.h"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\infcodes.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\infcodes.h"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\inffast.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\inffast.h"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\inffixed.h"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\inflate.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\inftrees.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\inftrees.h"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\infutil.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\infutil.h"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\maketree.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\trees.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\trees.h"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\uncompr.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\zconf.h"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\zlib.h"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\zutil.c"
# End Source File
# Begin Source File

SOURCE="..\..\extras\zlib-1.1.4\zutil.h"
# End Source File
# End Group
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMedia3DAxis.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\3d\OMedia3DMaterial.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\3d\OMedia3DMaterial.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMedia3DMorphAnim.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMedia3DMorphAnim.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMedia3DMorphAnimFrame.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMedia3DMorphAnimFrame.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMedia3DPoint.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\3d\OMedia3DPolygon.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\3d\OMedia3DPolygon.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\3d\OMedia3DShape.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\3d\OMedia3DShape.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMedia3DShapeAnim.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMedia3DShapeAnim.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMedia3DShapeAnimFrame.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMedia3DShapeAnimFrame.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Converter\OMedia3DShapeConverter.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Converter\OMedia3DShapeConverter.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMedia3DShapeElement.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMedia3DShapeElement.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMedia3DVector.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMedia3DVector.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaAbstractButton.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaAbstractButton.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMediaAnim.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMediaAnim.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMediaAnimFrame.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMediaAnimFrame.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaAnimPeriodical.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaAnimPeriodical.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Application\OMediaAppHints.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Application\OMediaApplication.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Application\OMediaApplication.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaBlendTable.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaBlendTable.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaBlitter.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaBlitter.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Messages\OMediaBroadcaster.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Messages\OMediaBroadcaster.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaBuildSwitch.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Canvas\OMediaCanvas.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Canvas\OMediaCanvas.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMediaCanvasAnim.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMediaCanvasAnim.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMediaCanvasAnimFrame.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Anim\OMediaCanvasAnimFrame.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaCanvasButton.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaCanvasButton.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Converter\OMediaCanvasConverter.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Converter\OMediaCanvasConverter.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaCanvasElement.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaCanvasElement.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaCanvasFont.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaCanvasFont.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Canvas\OMediaCanvasSlave.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Canvas\OMediaCanvasSlave.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaCaption.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaCaption.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Class\OMediaClassStreamer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Class\OMediaClassStreamer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaCollisionCache.cpp"
# End Source File
# Begin Source File

SOURCE=\Omt2dr4\Sources\OMTClasses\World\Core\OMediaCollisionCache.h
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Compression\OMediaCompression.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Compression\OMediaCompression.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Utility\OMediaCRC.cpp"
# End Source File
# Begin Source File

SOURCE=\Omt2dr4\Sources\OMTClasses\Utility\OMediaCRC.h
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Database\OMediaDataBase.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Database\OMediaDataBase.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Database\OMediaDBObject.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Database\OMediaDBObject.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Messages\OMediaDefMessages.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaDLLSupport.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaDLLSupport.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaDrawInterface.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDX3DShape.cpp"
# End Source File
# Begin Source File

SOURCE=\Omt2dr4\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDX3DShape.h
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXCanvas.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXCanvas.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXForceFeedback.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXForceFeedback.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXInputEngine.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXInputEngine.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXOffscreenBuffer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXOffscreenBuffer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXRenderer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXRenderer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXSound.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXSound.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXSoundChannel.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXSoundChannel.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXSoundEngine.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXSoundEngine.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXVideoEngine.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\DirectX\OMediaDXVideoEngine.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaElement.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaElement.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaElementContainer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaElementContainer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaEndianSupport.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaEngine.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaEngine.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaEngineFactory.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaEngineID.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaEngineImplementation.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaEngineImplementation.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaError.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaError.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Input\OMediaEventManager.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\3d\OMediaExtraTexturePass.cpp"
# End Source File
# Begin Source File

SOURCE=\Omt2dr4\Sources\OMTClasses\Graphics\3d\OMediaExtraTexturePass.h
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Core\OMediaFilePath.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Core\OMediaFileStream.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMediaFixedPoint.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaFocus.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaFocus.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaFont.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Input\OMediaForceFeedback.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Input\OMediaForceFeedback.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Formatted\OMediaFormattedStream.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Formatted\OMediaFormattedStream.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Formatted\OMediaFSChunk.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Formatted\OMediaFSChunkTypeNode.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\OpenGL\Graphics\Renderers\OMediaGL3DShape.cpp"
# End Source File
# Begin Source File

SOURCE=\Omt2dr4\Sources\OMTClasses\Os\OpenGL\Graphics\Renderers\OMediaGL3DShape.h
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\OpenGL\Graphics\Canvas\OMediaGLCanvas.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\OpenGL\Graphics\Canvas\OMediaGLCanvas.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\OpenGL\Graphics\Renderers\OMediaGLRenderer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\OpenGL\Graphics\Renderers\OMediaGLRenderer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaInputEngine.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaInputEngine.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaLayer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaLayer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaLight.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaLight.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Messages\OMediaListener.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Messages\OMediaListener.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMediaMathTools.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMediaMathTools.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMediaMatrix.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Core\OMediaMemStream.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Core\OMediaMemStream.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaMemTools.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Window\OMediaMenu.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Messages\OMediaMessagePort.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Messages\OMediaMessagePort.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Window\OMediaMonitorMap.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaMouseCursor.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaMoveableMem.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaOffscreenBuffer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaOffscreenBuffer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Canvas\OMediaOMTCanvas.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Canvas\OMediaOMTCanvas.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Renderers\OMediaOMTRenderer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Renderers\OMediaOMTRenderer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaParticleEmitter.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaParticleEmitter.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaPeriodical.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaPeriodical.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaPickRequest.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Pipeline\OMediaPipeLight.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Pipeline\OMediaPipeline.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Pipeline\OMediaPipeline.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Pipeline\OMediaPipeMaterial.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Pipeline\OMediaPipePoint.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Pipeline\OMediaPipePolygon.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaPixelFormat.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaPrimitiveElement.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaProcessorDefs.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Utility\OMediaProfiler.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Utility\OMediaProfiler.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaRadioGroup.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaRadioGroup.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Utility\OMediaRandomNumber.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaRastBlitter.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaRastBlitter.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaRateManager.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMediaRect.h"
# End Source File
# Begin Source File

SOURCE=\Omt2dr4\Sources\OMTClasses\Graphics\3d\OMediaRenderConstants.h
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Renderers\OMediaRenderer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Renderers\OMediaRenderer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Renderers\OMediaRendererDef.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Renderers\OMediaRendererInterface.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaRetarget.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaRetarget.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaRetargetDefs.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaRGBColor.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Core\OMediaRGBColor.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Utility\OMediaRLEPacker.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Utility\OMediaRLEPacker.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaScroller.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaScroller.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Rasterizer\OMediaSegmentRasterizer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Rasterizer\OMediaSegmentRasterizer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Window\OMediaSingleWindow.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaSlider.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaSlider.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Sound\OMediaSound.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Sound\OMediaSound.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Sound\OMediaSoundChannel.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Sound\OMediaSoundChannel.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaSoundEngine.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaSoundEngine.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMediaSphere.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaStdButton.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaStdButton.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Core\OMediaStream.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Core\OMediaStream.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Core\OMediaStreamOperators.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Stream\Core\OMediaStreamOperators.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Utility\OMediaString.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Utility\OMediaString.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaStringField.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Ui\OMediaStringField.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Messages\OMediaSupervisor.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Messages\OMediaSupervisor.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaSurfaceElement.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaSurfaceElement.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaSysDefs.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Rasterizer\OMediaTextSegmentRast16.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Rasterizer\OMediaTextSegmentRast16T16.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Rasterizer\OMediaTextSegmentRast32T16.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Rasterizer\OMediaTextSegmentRasterizer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Rasterizer\OMediaTextSegmentRasterizer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaThread.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaTimeCounter.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Utility\OMediaTimer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Rasterizer\OMediaTriangleRasterizer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Rasterizer\OMediaTriangleRasterLine.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Graphics\Rasterizer\OMediaTriangleSegment.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMediaTrigo.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Math\OMediaTrigo.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Core\OMediaTypes.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Input\OMediaUSBHIDUsages.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaVideoEngine.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Engines\OMediaVideoEngine.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaViewPort.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Element\OMediaViewPort.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Sound\OMediaWavFormat.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Application\OMediaWinApplication.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Core\OMediaWinClipboard.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Window\OMediaWindow.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Window\OMediaWindow.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Engines\OMediaWinEngineFactory.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Input\OMediaWinEventManager.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Stream\OMediaWinFilePath.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Stream\OMediaWinFileStream.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Window\OMediaWinFullscreenWindow.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Graphics\Renderers\OMediaWinGLRtgRenderer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Engines\OMediaWinInputEngine.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Core\OMediaWinMemTools.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Core\OMediaWinMouseCursor.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Core\OMediaWinMoveableMem.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Graphics\Core\OMediaWinOffscreenBuffer.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Graphics\Core\OMediaWinOffscreenBuffer.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Stream\OMediaWinRtgFilePath.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Stream\OMediaWinRtgFilePath.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Core\OMediaWinRtgMoveMem.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Core\OMediaWinRtgMoveMem.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Sound\OMediaWinRtgSound.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Window\OMediaWinRtgWindow.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Window\OMediaWinSingleWindow.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Sound\OMediaWinSound.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Sound\OMediaWinSoundChannel.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Engines\OMediaWinSoundEngine.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Application\OMediaWinStoreStartInfo.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Application\OMediaWinStoreStartInfo.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Core\OMediaWinThread.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Core\OMediaWinTimeCounter.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\Os\MSWindows\Engines\OMediaWinVideoEngine.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaWorld.cpp"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaWorld.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaWorldAngle.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaWorldPosition.h"
# End Source File
# Begin Source File

SOURCE="$(OMT2ROOT)\Sources\OMTClasses\World\Core\OMediaWorldUnits.h"
# End Source File
# End Target
# End Project
