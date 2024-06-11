@echo .
@if %OS%.==. goto :ZIP
@if %OS%.==Windows_NT. goto :SEVEN
@choice /c:YNS ZIP source ?
@if errorlevel 3 goto :SEVEN
@if errorlevel 2 goto :FIN
@echo .
:zip
@DEL    C:\TEMP\Mpg2Cut2_OLD011_Source.zip 
@rename C:\TEMP\Mpg2Cut2_OLD010_Source.zip  Mpg2Cut2_OLD011_Source.zip
@rename C:\TEMP\Mpg2Cut2_OLD009_Source.zip  Mpg2Cut2_OLD010_Source.zip
@rename C:\TEMP\Mpg2Cut2_OLD008_Source.zip  Mpg2Cut2_OLD009_Source.zip
@rename C:\TEMP\Mpg2Cut2_OLD007_Source.zip  Mpg2Cut2_OLD008_Source.zip
@rename C:\TEMP\Mpg2Cut2_OLD006_Source.zip  Mpg2Cut2_OLD007_Source.zip
@rename C:\TEMP\Mpg2Cut2_OLD005_Source.zip  Mpg2Cut2_OLD006_Source.zip
@rename C:\TEMP\Mpg2Cut2_OLD004_Source.zip  Mpg2Cut2_OLD005_Source.zip
@rename C:\TEMP\Mpg2Cut2_OLD003_Source.zip  Mpg2Cut2_OLD004_Source.zip
@rename C:\TEMP\Mpg2Cut2_OLD002_Source.zip  Mpg2Cut2_OLD003_Source.zip
@rename C:\TEMP\Mpg2Cut2_OLD001_Source.zip  Mpg2Cut2_OLD002_Source.zip
@rename C:\TEMP\Mpg2Cut2_UnstableSource.zip Mpg2Cut2_OLD001_Source.zip
pkzip -exx -o -P C:\TEMP\Mpg2Cut2_UnstableSource.ZIP  *.*  AC3Dec\*.*  -x*.ncb -x*.opt -x*.bak -x*.OLD
@if errorlevel 1 PAUSE
@echo .
@echo . ZIPPED to C:\TEMP
@echo .
@goto :X_7

:SEVEN
@DEL    C:\TEMP\Mpg2Cut2_OLD011_Source.7Z 
@rename C:\TEMP\Mpg2Cut2_OLD010_Source.7Z  Mpg2Cut2_OLD011_Source.7Z
@rename C:\TEMP\Mpg2Cut2_OLD009_Source.7Z  Mpg2Cut2_OLD010_Source.7Z
@rename C:\TEMP\Mpg2Cut2_OLD008_Source.7Z  Mpg2Cut2_OLD009_Source.7Z
@rename C:\TEMP\Mpg2Cut2_OLD007_Source.7Z  Mpg2Cut2_OLD008_Source.7Z
@rename C:\TEMP\Mpg2Cut2_OLD006_Source.7Z  Mpg2Cut2_OLD007_Source.7Z
@rename C:\TEMP\Mpg2Cut2_OLD005_Source.7Z  Mpg2Cut2_OLD006_Source.7Z
@rename C:\TEMP\Mpg2Cut2_OLD004_Source.7Z  Mpg2Cut2_OLD005_Source.7Z
@rename C:\TEMP\Mpg2Cut2_OLD003_Source.7Z  Mpg2Cut2_OLD004_Source.7Z
@rename C:\TEMP\Mpg2Cut2_OLD002_Source.7Z  Mpg2Cut2_OLD003_Source.7Z
@rename C:\TEMP\Mpg2Cut2_OLD001_Source.7Z  Mpg2Cut2_OLD002_Source.7Z
@rename C:\TEMP\Mpg2Cut2_UnstableSource.7Z Mpg2Cut2_OLD001_Source.7Z
c:\MISCWIN\7-ZIP\7Z.exe a -x@$_zip_exc.txt C:\TEMP\Mpg2Cut2_UnstableSource.7Z  *.*  "AC3Dec\*.*" 
@if errorlevel 1 PAUSE
@echo .
@echo . 7-ZIP to C:\TEMP
@echo .
:X_7
@dir C:\TEMP\Mpg2*.*
@pause
:FIN