@echo .
@if %OS%.==. goto :ZIP
@if %OS%.==Windows_NT. goto :SEVEN
@choice /c:YNS ZIP source ?
@if errorlevel 3 goto :SEVEN
@if errorlevel 2 goto :FIN
@echo .
:zip
pkzip -exx -o -P C:\TEMP\Mpg2Cut2_UnstableSource.ZIP  *.*  AC3Dec\*.*  -x*.ncb -x*.opt -x*.bak -x*.OLD
@if errorlevel 1 PAUSE
@echo .
@echo . ZIPPED to C:\TEMP
@echo .
@goto :X_7

:SEVEN
c:\MISCWIN\7-ZIP\7Z.exe a -x@$_zip_exc.txt C:\TEMP\Mpg2Cut2_UnstableSource.7Z  *.*  "AC3Dec\*.*" 
@if errorlevel 1 PAUSE
@echo .
@echo . 7-ZIP to C:\TEMP
@echo .
:X_7
@dir C:\TEMP\Mpg2*.*
@pause
:FIN