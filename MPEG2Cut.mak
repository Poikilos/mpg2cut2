# Microsoft Developer Studio Generated NMAKE File, Based on MPEG2Cut.dsp
!IF "$(CFG)" == ""
CFG=MPEG2Cut - Win32 Debug
!MESSAGE No configuration specified. Defaulting to MPEG2Cut - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "MPEG2Cut - Win32 Release" && "$(CFG)" != "MPEG2Cut - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MPEG2Cut.mak" CFG="MPEG2Cut - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MPEG2Cut - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MPEG2Cut - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MPEG2Cut - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Mpg2Cut2-RELEASE.exe" "$(OUTDIR)\MPEG2Cut.bsc"


CLEAN :
	-@erase "$(INTDIR)\bit_allocate.obj"
	-@erase "$(INTDIR)\bit_allocate.sbr"
	-@erase "$(INTDIR)\bitstream.obj"
	-@erase "$(INTDIR)\bitstream.sbr"
	-@erase "$(INTDIR)\coeff.obj"
	-@erase "$(INTDIR)\coeff.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\da.obj"
	-@erase "$(INTDIR)\da.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\downmix.obj"
	-@erase "$(INTDIR)\downmix.sbr"
	-@erase "$(INTDIR)\exponent.obj"
	-@erase "$(INTDIR)\exponent.sbr"
	-@erase "$(INTDIR)\getbit.obj"
	-@erase "$(INTDIR)\getbit.sbr"
	-@erase "$(INTDIR)\gethdr.obj"
	-@erase "$(INTDIR)\gethdr.sbr"
	-@erase "$(INTDIR)\getpic.obj"
	-@erase "$(INTDIR)\getpic.sbr"
	-@erase "$(INTDIR)\gui.obj"
	-@erase "$(INTDIR)\gui.res"
	-@erase "$(INTDIR)\gui.sbr"
	-@erase "$(INTDIR)\idctfpu.obj"
	-@erase "$(INTDIR)\idctfpu.sbr"
	-@erase "$(INTDIR)\idctref.obj"
	-@erase "$(INTDIR)\idctref.sbr"
	-@erase "$(INTDIR)\imdct.obj"
	-@erase "$(INTDIR)\imdct.sbr"
	-@erase "$(INTDIR)\motion.obj"
	-@erase "$(INTDIR)\motion.sbr"
	-@erase "$(INTDIR)\mpeg2dec.obj"
	-@erase "$(INTDIR)\mpeg2dec.sbr"
	-@erase "$(INTDIR)\parse.obj"
	-@erase "$(INTDIR)\parse.sbr"
	-@erase "$(INTDIR)\rematrix.obj"
	-@erase "$(INTDIR)\rematrix.sbr"
	-@erase "$(INTDIR)\sanity_check.obj"
	-@erase "$(INTDIR)\sanity_check.sbr"
	-@erase "$(INTDIR)\store.obj"
	-@erase "$(INTDIR)\store.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wave_out.obj"
	-@erase "$(INTDIR)\wave_out.sbr"
	-@erase "$(OUTDIR)\MPEG2Cut.bsc"
	-@erase "$(OUTDIR)\Mpg2Cut2-RELEASE.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /G6 /MT /W3 /Ox /Ot /Oa /Og /Oi /Gf /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FAcs /Fa"$(INTDIR)\\" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\MPEG2Cut.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\gui.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MPEG2Cut.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\da.sbr" \
	"$(INTDIR)\getbit.sbr" \
	"$(INTDIR)\gethdr.sbr" \
	"$(INTDIR)\getpic.sbr" \
	"$(INTDIR)\gui.sbr" \
	"$(INTDIR)\idctfpu.sbr" \
	"$(INTDIR)\idctref.sbr" \
	"$(INTDIR)\motion.sbr" \
	"$(INTDIR)\mpeg2dec.sbr" \
	"$(INTDIR)\store.sbr" \
	"$(INTDIR)\wave_out.sbr" \
	"$(INTDIR)\bit_allocate.sbr" \
	"$(INTDIR)\bitstream.sbr" \
	"$(INTDIR)\coeff.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\downmix.sbr" \
	"$(INTDIR)\exponent.sbr" \
	"$(INTDIR)\imdct.sbr" \
	"$(INTDIR)\parse.sbr" \
	"$(INTDIR)\rematrix.sbr" \
	"$(INTDIR)\sanity_check.sbr"

"$(OUTDIR)\MPEG2Cut.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ddraw.lib dxguid.lib vfw32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib idctmmx.obj /nologo /subsystem:windows /pdb:none /machine:I386 /out:"$(OUTDIR)\Mpg2Cut2-RELEASE.exe" /mapinfo:exports /mapinfo:lines 
LINK32_OBJS= \
	"$(INTDIR)\da.obj" \
	"$(INTDIR)\getbit.obj" \
	"$(INTDIR)\gethdr.obj" \
	"$(INTDIR)\getpic.obj" \
	"$(INTDIR)\gui.obj" \
	"$(INTDIR)\idctfpu.obj" \
	"$(INTDIR)\idctref.obj" \
	"$(INTDIR)\motion.obj" \
	"$(INTDIR)\mpeg2dec.obj" \
	"$(INTDIR)\store.obj" \
	"$(INTDIR)\wave_out.obj" \
	"$(INTDIR)\bit_allocate.obj" \
	"$(INTDIR)\bitstream.obj" \
	"$(INTDIR)\coeff.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\downmix.obj" \
	"$(INTDIR)\exponent.obj" \
	"$(INTDIR)\imdct.obj" \
	"$(INTDIR)\parse.obj" \
	"$(INTDIR)\rematrix.obj" \
	"$(INTDIR)\sanity_check.obj" \
	"$(INTDIR)\gui.res"

"$(OUTDIR)\Mpg2Cut2-RELEASE.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MPEG2Cut - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\MPG2Cut2-DBG.exe" "$(OUTDIR)\MPEG2Cut.bsc"


CLEAN :
	-@erase "$(INTDIR)\bit_allocate.obj"
	-@erase "$(INTDIR)\bit_allocate.sbr"
	-@erase "$(INTDIR)\bitstream.obj"
	-@erase "$(INTDIR)\bitstream.sbr"
	-@erase "$(INTDIR)\coeff.obj"
	-@erase "$(INTDIR)\coeff.sbr"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\crc.sbr"
	-@erase "$(INTDIR)\da.obj"
	-@erase "$(INTDIR)\da.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\downmix.obj"
	-@erase "$(INTDIR)\downmix.sbr"
	-@erase "$(INTDIR)\exponent.obj"
	-@erase "$(INTDIR)\exponent.sbr"
	-@erase "$(INTDIR)\getbit.obj"
	-@erase "$(INTDIR)\getbit.sbr"
	-@erase "$(INTDIR)\gethdr.obj"
	-@erase "$(INTDIR)\gethdr.sbr"
	-@erase "$(INTDIR)\getpic.obj"
	-@erase "$(INTDIR)\getpic.sbr"
	-@erase "$(INTDIR)\gui.obj"
	-@erase "$(INTDIR)\gui.res"
	-@erase "$(INTDIR)\gui.sbr"
	-@erase "$(INTDIR)\idctfpu.obj"
	-@erase "$(INTDIR)\idctfpu.sbr"
	-@erase "$(INTDIR)\idctref.obj"
	-@erase "$(INTDIR)\idctref.sbr"
	-@erase "$(INTDIR)\imdct.obj"
	-@erase "$(INTDIR)\imdct.sbr"
	-@erase "$(INTDIR)\motion.obj"
	-@erase "$(INTDIR)\motion.sbr"
	-@erase "$(INTDIR)\mpeg2dec.obj"
	-@erase "$(INTDIR)\mpeg2dec.sbr"
	-@erase "$(INTDIR)\parse.obj"
	-@erase "$(INTDIR)\parse.sbr"
	-@erase "$(INTDIR)\rematrix.obj"
	-@erase "$(INTDIR)\rematrix.sbr"
	-@erase "$(INTDIR)\sanity_check.obj"
	-@erase "$(INTDIR)\sanity_check.sbr"
	-@erase "$(INTDIR)\store.obj"
	-@erase "$(INTDIR)\store.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wave_out.obj"
	-@erase "$(INTDIR)\wave_out.sbr"
	-@erase "$(OUTDIR)\MPEG2Cut.bsc"
	-@erase "$(OUTDIR)\MPG2Cut2-DBG.exe"
	-@erase "$(OUTDIR)\MPG2Cut2-DBG.map"
	-@erase "$(OUTDIR)\MPG2Cut2-DBG.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\MPEG2Cut.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\gui.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MPEG2Cut.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\da.sbr" \
	"$(INTDIR)\getbit.sbr" \
	"$(INTDIR)\gethdr.sbr" \
	"$(INTDIR)\getpic.sbr" \
	"$(INTDIR)\gui.sbr" \
	"$(INTDIR)\idctfpu.sbr" \
	"$(INTDIR)\idctref.sbr" \
	"$(INTDIR)\motion.sbr" \
	"$(INTDIR)\mpeg2dec.sbr" \
	"$(INTDIR)\store.sbr" \
	"$(INTDIR)\wave_out.sbr" \
	"$(INTDIR)\bit_allocate.sbr" \
	"$(INTDIR)\bitstream.sbr" \
	"$(INTDIR)\coeff.sbr" \
	"$(INTDIR)\crc.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\downmix.sbr" \
	"$(INTDIR)\exponent.sbr" \
	"$(INTDIR)\imdct.sbr" \
	"$(INTDIR)\parse.sbr" \
	"$(INTDIR)\rematrix.sbr" \
	"$(INTDIR)\sanity_check.sbr"

"$(OUTDIR)\MPEG2Cut.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ddraw.lib dxguid.lib vfw32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib idctmmx.obj /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\MPG2Cut2-DBG.pdb" /map:"$(INTDIR)\MPG2Cut2-DBG.map" /debug /machine:I386 /out:"$(OUTDIR)\MPG2Cut2-DBG.exe" /pdbtype:sept /mapinfo:exports /mapinfo:lines 
LINK32_OBJS= \
	"$(INTDIR)\da.obj" \
	"$(INTDIR)\getbit.obj" \
	"$(INTDIR)\gethdr.obj" \
	"$(INTDIR)\getpic.obj" \
	"$(INTDIR)\gui.obj" \
	"$(INTDIR)\idctfpu.obj" \
	"$(INTDIR)\idctref.obj" \
	"$(INTDIR)\motion.obj" \
	"$(INTDIR)\mpeg2dec.obj" \
	"$(INTDIR)\store.obj" \
	"$(INTDIR)\wave_out.obj" \
	"$(INTDIR)\bit_allocate.obj" \
	"$(INTDIR)\bitstream.obj" \
	"$(INTDIR)\coeff.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\downmix.obj" \
	"$(INTDIR)\exponent.obj" \
	"$(INTDIR)\imdct.obj" \
	"$(INTDIR)\parse.obj" \
	"$(INTDIR)\rematrix.obj" \
	"$(INTDIR)\sanity_check.obj" \
	"$(INTDIR)\gui.res"

"$(OUTDIR)\MPG2Cut2-DBG.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MPEG2Cut.dep")
!INCLUDE "MPEG2Cut.dep"
!ELSE 
!MESSAGE Warning: cannot find "MPEG2Cut.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "MPEG2Cut - Win32 Release" || "$(CFG)" == "MPEG2Cut - Win32 Debug"
SOURCE=.\CLIPS.c
SOURCE=.\D2V.c
SOURCE=.\da.c

"$(INTDIR)\da.obj"	"$(INTDIR)\da.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DBG.c
SOURCE=.\DDRAW.c
SOURCE=.\getbit.c

"$(INTDIR)\getbit.obj"	"$(INTDIR)\getbit.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gethdr.c

"$(INTDIR)\gethdr.obj"	"$(INTDIR)\gethdr.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\getpic.c

"$(INTDIR)\getpic.obj"	"$(INTDIR)\getpic.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gui.cpp

!IF  "$(CFG)" == "MPEG2Cut - Win32 Release"

CPP_SWITCHES=/nologo /G6 /MT /W3 /Ox /Ot /Oa /Og /Oi /Op /Gf /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FAcs /Fa"$(INTDIR)\\" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\MPEG2Cut.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\gui.obj"	"$(INTDIR)\gui.sbr" : $(SOURCE) "$(INTDIR)" ".\DDRAW.c"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "MPEG2Cut - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\MPEG2Cut.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\gui.obj"	"$(INTDIR)\gui.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\idctfpu.c

"$(INTDIR)\idctfpu.obj"	"$(INTDIR)\idctfpu.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\idctref.c

"$(INTDIR)\idctref.obj"	"$(INTDIR)\idctref.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\IN_OPEN.c
SOURCE=.\INI.c
SOURCE=.\motion.c

"$(INTDIR)\motion.obj"	"$(INTDIR)\motion.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mpeg2dec.c

"$(INTDIR)\mpeg2dec.obj"	"$(INTDIR)\mpeg2dec.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SNAPS.c
SOURCE=.\store.c

"$(INTDIR)\store.obj"	"$(INTDIR)\store.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\wave_out.c

"$(INTDIR)\wave_out.obj"	"$(INTDIR)\wave_out.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gui.rc

"$(INTDIR)\gui.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\bit_allocate.c

"$(INTDIR)\bit_allocate.obj"	"$(INTDIR)\bit_allocate.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\bitstream.c

"$(INTDIR)\bitstream.obj"	"$(INTDIR)\bitstream.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\coeff.c

"$(INTDIR)\coeff.obj"	"$(INTDIR)\coeff.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\crc.c

"$(INTDIR)\crc.obj"	"$(INTDIR)\crc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\decode.c

"$(INTDIR)\decode.obj"	"$(INTDIR)\decode.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\downmix.c

"$(INTDIR)\downmix.obj"	"$(INTDIR)\downmix.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\exponent.c

"$(INTDIR)\exponent.obj"	"$(INTDIR)\exponent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\imdct.c

"$(INTDIR)\imdct.obj"	"$(INTDIR)\imdct.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\parse.c

"$(INTDIR)\parse.obj"	"$(INTDIR)\parse.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\rematrix.c

"$(INTDIR)\rematrix.obj"	"$(INTDIR)\rematrix.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\AC3Dec\sanity_check.c

"$(INTDIR)\sanity_check.obj"	"$(INTDIR)\sanity_check.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

