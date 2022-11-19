@echo off

if "%1%" == "clean" (
	del *.obj *.exe *.lib *.exp *.pdb
	exit /b
)

cl /nologo /W3 rf.c ignore.c include\common\strl.c getopt\getopt.c /link /out:rf.exe