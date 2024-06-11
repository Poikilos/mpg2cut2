# Microsoft Developer Studio Project File - Name="MPEG2Cut" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=MPEG2Cut - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MPEG2Cut.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MPEG2Cut - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MT /W3 /Zd /Ox /Ot /Oa /Og /Oi /Gf /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FAs /FD /c
# SUBTRACT CPP /Fr /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ddraw.lib dxguid.lib vfw32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib idctmmx.obj /nologo /subsystem:windows /pdb:none /machine:I386 /out:"Release\Mpg2Cut2-RELEASE.exe" /mapinfo:exports /mapinfo:lines

!ELSEIF  "$(CFG)" == "MPEG2Cut - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W4 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FAs /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x404 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ddraw.lib dxguid.lib vfw32.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib idctmmx.obj /nologo /subsystem:windows /incremental:no /map /debug /machine:I386 /out:"Debug/MPG2Cut2-DBG.exe" /pdbtype:sept /mapinfo:exports /mapinfo:lines
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "MPEG2Cut - Win32 Release"
# Name "MPEG2Cut - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Aspect.c
# End Source File
# Begin Source File

SOURCE=.\Buttons.c
# End Source File
# Begin Source File

SOURCE=.\ClipFile.c
# End Source File
# Begin Source File

SOURCE=.\CLIPS.c
# End Source File
# Begin Source File

SOURCE=.\DBG.c
# End Source File
# Begin Source File

SOURCE=.\DDRAW_CTL.cpp
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Disp_Names.c
# End Source File
# Begin Source File

SOURCE=.\Disp_win.c
# End Source File
# Begin Source File

SOURCE=.\Get_INIT.c
# End Source File
# Begin Source File

SOURCE=.\GetAudio.c
# End Source File
# Begin Source File

SOURCE=.\getbit.c
# End Source File
# Begin Source File

SOURCE=.\GetBlk.c
# End Source File
# Begin Source File

SOURCE=.\gethdr.c
# End Source File
# Begin Source File

SOURCE=.\getpic.c
# End Source File
# Begin Source File

SOURCE=.\GopBack.c
# End Source File
# Begin Source File

SOURCE=.\gui.cpp

!IF  "$(CFG)" == "MPEG2Cut - Win32 Release"

# ADD CPP /Oi /Op /FAs

!ELSEIF  "$(CFG)" == "MPEG2Cut - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\idctfpu.c
# End Source File
# Begin Source File

SOURCE=.\idctref.c
# End Source File
# Begin Source File

SOURCE=.\IN_FILES.c
# End Source File
# Begin Source File

SOURCE=.\INI.c

!IF  "$(CFG)" == "MPEG2Cut - Win32 Release"

# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "MPEG2Cut - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\INI_REG.c
# End Source File
# Begin Source File

SOURCE=.\LUM_WIN.c
# End Source File
# Begin Source File

SOURCE=.\motion.c
# End Source File
# Begin Source File

SOURCE=.\MPA_DA.c
# End Source File
# Begin Source File

SOURCE=.\mpeg2dec.c

!IF  "$(CFG)" == "MPEG2Cut - Win32 Release"

# ADD CPP /FAs

!ELSEIF  "$(CFG)" == "MPEG2Cut - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Nav_JUMP.c
# End Source File
# Begin Source File

SOURCE=.\NO_OPT.c
# ADD CPP /Od
# End Source File
# Begin Source File

SOURCE=.\OUT.c
# End Source File
# Begin Source File

SOURCE=.\Out_BUF.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Out_Clip.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Out_FAST.c

!IF  "$(CFG)" == "MPEG2Cut - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "MPEG2Cut - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Out_Hdrs.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Out_PART.c
# End Source File
# Begin Source File

SOURCE=.\Out_PKTS.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Out_Progress.c
# End Source File
# Begin Source File

SOURCE=.\Out_SPLIT.c
# End Source File
# Begin Source File

SOURCE=.\Out_TC.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\Parm2Clip.c
# End Source File
# Begin Source File

SOURCE=.\Plug.c
# End Source File
# Begin Source File

SOURCE=.\Pref.cpp
# End Source File
# Begin Source File

SOURCE=.\Render.c
# End Source File
# Begin Source File

SOURCE=.\SNAPS.c
# End Source File
# Begin Source File

SOURCE=.\STATS_WIN.c
# End Source File
# Begin Source File

SOURCE=.\store.c
# End Source File
# Begin Source File

SOURCE=.\Timing.c
# End Source File
# Begin Source File

SOURCE=.\TRACKBAR.c
# End Source File
# Begin Source File

SOURCE=.\TxtFunc.c
# End Source File
# Begin Source File

SOURCE=.\Volume.c
# End Source File
# Begin Source File

SOURCE=.\WAV_NOOPT.c

!IF  "$(CFG)" == "MPEG2Cut - Win32 Release"

# ADD CPP /Od

!ELSEIF  "$(CFG)" == "MPEG2Cut - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\wave_out.c
# End Source File
# Begin Source File

SOURCE=.\YUV_444.c
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Audio.h
# End Source File
# Begin Source File

SOURCE=.\Buttons.h
# End Source File
# Begin Source File

SOURCE=.\DDRAW_CTL.h
# End Source File
# Begin Source File

SOURCE=.\getbit.h
# End Source File
# Begin Source File

SOURCE=.\GetBit_Fast.h
# End Source File
# Begin Source File

SOURCE=.\GetBlk.h
# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\Gui_Comm.h
# End Source File
# Begin Source File

SOURCE=.\MPA_DA.h
# End Source File
# Begin Source File

SOURCE=.\MPA_HDR.h
# End Source File
# Begin Source File

SOURCE=.\MPA_SIZE.h
# End Source File
# Begin Source File

SOURCE=.\mpalib.h
# End Source File
# Begin Source File

SOURCE=.\mpalib_more.h
# End Source File
# Begin Source File

SOURCE=.\Mpg2Cut2_API.h
# End Source File
# Begin Source File

SOURCE=.\MPV_PIC.h
# End Source File
# Begin Source File

SOURCE=.\Nav_JUMP.h
# End Source File
# Begin Source File

SOURCE=.\out.h
# End Source File
# Begin Source File

SOURCE=.\PIC_BUF.h
# End Source File
# Begin Source File

SOURCE=.\PLUG.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\TXT.h
# End Source File
# Begin Source File

SOURCE=.\TXT_EN.h
# End Source File
# Begin Source File

SOURCE=.\Wave_CS.h
# End Source File
# Begin Source File

SOURCE=.\wave_out.h
# End Source File
# Begin Source File

SOURCE=.\Yuv2rgb_mmx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\F_Open.ico
# End Source File
# Begin Source File

SOURCE=.\Flop35.ico
# End Source File
# Begin Source File

SOURCE=.\gui.rc
# End Source File
# Begin Source File

SOURCE=.\movie.ico
# End Source File
# Begin Source File

SOURCE=.\snap.ico
# End Source File
# End Group
# Begin Group "a53dec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AC3Dec\A53_interface.h
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\ac3.h
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\bit_allocate.c
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\bitstream.c
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\bitstream.h
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\coeff.c
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\crc.c
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\decode.c

!IF  "$(CFG)" == "MPEG2Cut - Win32 Release"

!ELSEIF  "$(CFG)" == "MPEG2Cut - Win32 Debug"

# ADD CPP /Od

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AC3Dec\downmix.c
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\exponent.c
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\imdct.c
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\parse.c
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\rematrix.c
# End Source File
# Begin Source File

SOURCE=.\AC3Dec\sanity_check.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\idctmmx.asm

!IF  "$(CFG)" == "MPEG2Cut - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "MPEG2Cut - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
