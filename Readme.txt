Design
------
To achieve faster speed and smaller size, this project is implemented using old-schooled Win32 programming, no MFC, no ATL, no WinForm.

The downloading of remote URL is done in another thread, such that the main thread UI will not be blocked.


Build and Run
-------------
This project is designed using Visual Studio 2017, with Windows SDK version being 10.0.10240.0.

To build this app for running on a Windows PC, do build the "Release" version instead of the "Debug" version. That is because the exe build with Debug versions might require debug versions of core Windows dlls, which might not be available on a Windows PC without Visual Studio or the supporting libs/dlls of Visual Studio installed.

Output Directory
----------------
The root directory being directory containing the InstallerSubProject.sln file.

If you build for Debug version, the Debug version of the exe will be located under folder "Debug".
If you build for Release version, the Release version of the exe will be located under folder "Release".

Intermediate files (like object files, etc.) will be located under Project directory. As for this app, the project directory is "InstallerSub".