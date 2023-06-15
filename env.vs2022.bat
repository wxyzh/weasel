rem Customize your build environment and save the modified copy to env.bat

set WEASEL_ROOT=%CD%

rem REQUIRED: path to Boost source directory
if not defined BOOST_ROOT set BOOST_ROOT=%WEASEL_ROOT%\deps\boost_1_82_0

rem OPTIONAL: architecture, Visual Studio version and platform toolset
set BJAM_TOOLSET=msvc-14.3
set CMAKE_GENERATOR="Visual Studio 17 2022"
set PLATFORM_TOOLSET=v143

rem OPTIONAL: path to additional build tools
rem set DEVTOOLS_PATH=D:\Program Files\Git\cmd;D:\Program Files\Git\usr\bin;D:\Program Files\cmake-3.25.2-windows-x86_64\bin;D:\Program Files\Python\Python311;
