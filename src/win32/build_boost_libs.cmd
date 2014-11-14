@echo off
rem Launch this file in the boost/ folder.

rem Check if we're in the correct folder
if not exist libs goto :error
if not exist boost goto :error

mkdir stage

cd libs\filesystem\build
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static release stage
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static debug stage
copy stage\*.lib ..\..\..\stage

cd ..\..\..\libs\system\build
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static release stage
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static debug stage
copy stage\*.lib ..\..\..\stage

cd ..\..\..\libs\date_time\build
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static release stage
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static debug stage
copy stage\*.lib ..\..\..\stage

cd ..\..\..\libs\program_options\build
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static release stage
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static debug stage
copy stage\*.lib ..\..\..\stage

cd ..\..\..\libs\thread\build
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static release
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static debug
copy stage\*.lib ..\..\..\stage
copy ..\..\..\bin.v2\libs\thread\build\msvc-10.0\release\link-static\runtime-link-static\threading-multi\*.lib ..\..\..\stage
copy ..\..\..\bin.v2\libs\thread\build\msvc-10.0\debug\link-static\runtime-link-static\threading-multi\*.lib ..\..\..\stage

cd ..\..\..\libs\regex\build
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static release stage
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static debug stage
copy stage\*.lib ..\..\..\stage

cd ..\..\..\libs\chrono\build
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static release
bjam --toolset=msvc-10.0 link=static threading=multi runtime-link=static debug
copy ..\..\..\bin.v2\libs\chrono\build\msvc-10.0\release\link-static\runtime-link-static\threading-multi\*.lib ..\..\..\stage
copy ..\..\..\bin.v2\libs\chrono\build\msvc-10.0\debug\link-static\runtime-link-static\threading-multi\*.lib ..\..\..\stage


cd ..\..\..\stage
start .

goto :success

:error
echo Please launch this file in the boost folder you downloaded and extracted from sourceforge.
echo Note that you need to have the path to bjam.exe in your PATH environment variable.
echo 
echo Example: 
echo   cd c:\Downloads\boost_1_51_0\
echo   c:\Games\Doom3\darkmod_src\win32\build_boost_libs.cmd
goto :eof

:success
echo Successfully built the libraries, please copy them to the w32deps/boost/libs/ folder now.