attrib *.* -r -a -s -h /s /d

del *.exe /q
del *.pdb /q
del *.exp /q
del *.lib /q
del *.map /q

del src\*.suo /s /q
del src\*.user /s /q
del src\*.sdf /s /q
del src\*.map /s /q

del src\tdm_update\*.suo /s /q
del src\tdm_update\*.user /s /q
del src\tdm_update\*.sdf /s /q
del src\tdm_update\*.map /s /q

rmdir src\bin /s /q
rmdir src\ipch /s /q
rmdir src\build /s /q
rmdir darkmod /s /q

attrib +r +h .git

echo Done

pause
