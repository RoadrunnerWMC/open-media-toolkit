How to use OMT on Windows
--------------------------

** Supported compiler **

Currently we support only Visual C++ on windows. 


** Setting global variables **

Before you can use the VC++ projects you have to set a system global variable to the path of the OMT directory.

For example:

Set OMT2ROOT=C:\OMT

It is also recommended to add the path of your dll binaries to the global "PATH" variable so they are accessible from anywhere on your disk.

For example:

Set path=%OMT2ROOT%\mswindows\omtdll;%path%


** Using the OMT VC++ project builder **

Because OMT headers are stored in many different folders, it is painful to create a new VC++ project ready to be used with OMT. For this reason, there is a small application that you can use to generate a new OMT VC++ project. You will find it in "mswindows/tools/OMTProjectBuilder/". Simply launch it and choose a destination directory. The application automatically generates a startup project ready to be compiled.


