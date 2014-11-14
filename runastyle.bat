rem A script to run Astyle for the sources
rem Astyle is a source indenter / formatter.
@echo off
CLS
rem options are in astyle-options.ini
SET STYLE=--options=astyle-options.ini
rem also add brackets to unbracketed functions.
SET OPTIONS=--add-brackets

rem carefull here do not enable everything 
rem astyle can fuck up asm code majorly.
rem idlib has one place that comes to mind in the SIMD code.
astyle %STYLE% %OPTIONS% -r src/game/*.cpp
astyle %STYLE% %OPTIONS% -r src/game/*.h
rem astyle %STYLE% %OPTIONS% -r src/d3xp/*.cpp
rem astyle %STYLE% %OPTIONS% -r src/d3xp/*.h
astyle %STYLE% %OPTIONS% -r src/cm/*.cpp
astyle %STYLE% %OPTIONS% -r src/cm/*.h
astyle %STYLE% %OPTIONS% -r src/framework/*.cpp
astyle %STYLE% %OPTIONS% -r src/framework/*.h
rem astyle %STYLE% %OPTIONS% -r src/libs/*.cpp
rem astyle %STYLE% %OPTIONS% -r src/libs/*.h
astyle %STYLE% %OPTIONS% -r src/renderer/*.cpp
astyle %STYLE% %OPTIONS% -r src/renderer/*.h
astyle %STYLE% %OPTIONS% -r src/sound/*.cpp
astyle %STYLE% %OPTIONS% -r src/sound/*.h
astyle %STYLE% %OPTIONS% -r src/sys/*.cpp
astyle %STYLE% %OPTIONS% -r src/sys/*.h
astyle %STYLE% %OPTIONS% -r src/typeinfo/*.cpp
astyle %STYLE% %OPTIONS% -r src/typeinfo/*.h
astyle %STYLE% %OPTIONS% -r src/tools/*.cpp
astyle %STYLE% %OPTIONS% -r src/tools/*.h
astyle %STYLE% %OPTIONS% -r src/ui/*.cpp
astyle %STYLE% %OPTIONS% -r src/ui/*.h

rem astyle %STYLE% %OPTIONS% -r extras/*.cpp
rem astyle %STYLE% %OPTIONS% -r extras/*.h

pause



