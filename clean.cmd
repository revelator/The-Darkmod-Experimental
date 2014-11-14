attrib *.* -r -a -s -h /s /d

del *.exe /q
del *.pdb /q
del *.exp /q
del *.lnk /q

del src\*.suo /q
del src\*.user /q
del src\*.sdf /q

rmdir src\ipch /s /q
rmdir src\build /s /q
rmdir darkmod /s /q

echo Done

pause
