//
// 
#include "global.h" 

#include "Audio.h"
#include "mpalib.h" 
#include "mpalib_more.h"
#include "AC3Dec\A53_interface.h"

#include "PLUG.h"
#include "Buttons.h"
//#include "AC3Dec\ac3.h"
#include "errno.h"
#include "GetBlk.h"
#include "Out.h"

#define true 1
#define false 0

#define  K_DefaultY_FORMAT "DefaultY=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"
#define  K_PatchHdr_FORMAT "PatchHdr=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"
#define  K_AudioX_FORMAT "AudioX=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"
#define  K_Y2           "Y2=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"
#define  K_N2           "N2=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"
#define  K_N3           "N3=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"
#define  K_LumPresets "LumPresets=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"
#define  K_VOLUME       "VOLUME=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n"


FILE *INIFile;
int INI_Version;
#define PGM_INI  1

int iTmp2, iValCtr;
int iCtl_Warn_Fmts;

void   Reg_ExternalActions();

void MenuTick(UINT uItem)
{
  CheckMenuItem(hMenu, uItem,  MF_CHECKED);
}
void MenuUnTick(UINT uItem)
{
  CheckMenuItem(hMenu, uItem,  MF_UNCHECKED);
}
void MenuTickCtl(UINT uItem, UINT uStatus)
{
  CheckMenuItem(hMenu, uItem,  uStatus);
}


//--------------------------------------------
// Translate Quotes and Commas to allow for "scanf" limitations
void ParmTidy(char *P_Dest, int P_Mode)
{
  char cTmp;
  
  cTmp = 'x';
  while (cTmp)
  {
    cTmp = *P_Dest;
    if (P_Mode)
    {
       if (cTmp == '"')           // Double Quote
          *P_Dest = (char)(0x92); // Convert to close single quote
       else
       if (cTmp == ',')           // Comma
          *P_Dest = (char)(0xB8); // Convert to cedilla
    }
    else
    {
       if (cTmp == (char)(0x92))    // Convert from close single quote
          *P_Dest = '"';    // Double Quote
       else
       if (cTmp == (char)(0xB8))    // Convert from cedilla
          *P_Dest = ',';    // Comma
    }

    *P_Dest++;
  }

  return;
}





//----------------------------------------------------------
void INI_VARS_BeforeMenu()
{
  // What kind of CPU features available ?
  __asm
  {
    mov     eax, 1
    cpuid
    test    edx, 0x00800000   // STD MMX = Pentium Pro or better
    jz      TEST_SSE
    mov     [cpu.mmx], 1

TEST_SSE:
    test    edx, 0x02000000   // STD SSE = Pentium 3 or better
    jz      TEST_SSE2
    mov     [cpu.ssemmx], 1
    mov     [cpu.ssefpu], 1

TEST_SSE2:
    test    edx, 0x04000000    // SSE2 SSE = Pentium 4 or better
    jz      TEST_3DNOW
    mov     [cpu.sse2], 1

TEST_3DNOW:
/*           DISABLED DUE TO PROBLEMS FOR END-USER
             WRONGLY GIVES ATHLON THUNDERBIRD SSEMMX & 3DNOW :-

            "my Thunderbird Athlon (CPU-Z says Family 6, Model 4) supports 
                            MMX, SSE MMX & 3DNOW!, 
              but SSE is only supported on AthlonXP (Family 6, Model 6) 
                            Palomino-Thoroughbred-Barton core chips. 
              CPU-Z doesn't list it as supporting SSE."

   TODO: If someone can give me correct rules for 3DNOW,
         and is prepared to test it for me,
         then I am happy to put in the appropriate code.


    mov     eax, 0x80000001
    cpuid
    test    edx, 0x80000000   // 3D NOW = AMD
    jz      TEST_SSEMMX
    mov     [cpu._3dnow], 1
TEST_SSEMMX:
    test    edx, 0x00400000   // SSE MMX
    jz      TEST_END
    mov     [cpu.ssemmx], 1
TEST_END:
*/
  }

  iVistaOVL_mod = 0;  iCtl_VistaOVL_mod = 0;
  WindowsVersion = GetVersion();      // OLD STYE VERSION NUMBER

  winVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if( ! GetVersionEx(&winVer) )       // NEW STYLE VERSION NUMBER
  {   //  DIDN'T WORK - SO INFER IT FROM THE OLD STYLE
      if (WindowsVersion < 0x80000000) //  Windows NT, Win2k, WinXP ?
          winVer.dwMajorVersion = 5;
      else
          winVer.dwMajorVersion = 4;
  }

  //REG   CHECK / CREATE ROOT KEY
  PicBuffer_Canvas_Size = -1;

  // Set defaults in case none found stored

  Restore_X = 0 ;
  Restore_Y = 2;
  Restore_Width  = VGA_Width  / 2 ;
  Restore_Height = VGA_Height / 2 ;

  INI_Version = PGM_INI;

  //iMPA_Best = 0;
  MParse.iDCT_Flag  =  IDCT_REF;

  MParse.PC_Range_Flag = true;
  MParse.FO_Flag = FO_NONE;
  iAudio_SEL_Track = TRACK_1;

  PlugFileRename.iStatus = -1;
  PlugFileList.iStatus  = -1;
  strcpy(szAudio_Status, "-") ;

  iWant_Aud_Format = FORMAT_AUTO;

  AC3_Flag = AUDIO_DECODE;
  AC3_DRC_FLag = DRC_HEAVY;
  AC3_DSDown_Flag = true;

  MPA_Flag = AUDIO_DEMUXONE;
  SRC_Flag = SRC_NONE;
  Norm_Ratio = 100;

  iCtl_Priority[0] = PRIORITY_NORMAL;
  iCtl_Priority[1] = PRIORITY_HIGH;
  iCtl_Priority[2] = PRIORITY_NORMAL;


  iMpeg_Copy_BufSz = K_4MB; // Default 4 MB
  iCtl_AudioAhead = 0;

  iCtl_Lum_Deselector = 0;
  iLumLock_Flag  = 0;    iSatLock_Flag  = 0;
  iSatAdj_Flag   = 0;
  iSatAdd_U[0]   = 0;    iSatAdd_U[1] = 0;
  iSatAdd_V[0]   = 0;    iSatAdd_V[1] = 0;
  iSatGain[0]    = 100;  iSatGain[1]  = 100;
  iColorSpaceTab = 0;

  iCtl_Aspect_Retain = 0;
  iView_Aspect_Mode = 4;  // Default = Standard

  iView_FrameRate_Code = 0; iOverride_FrameRate_Code = 0;
  fFrame_rate_extension_n = 1.0;
  fFrame_rate_extension_d = 1.0;

  iConverge_Red_H  = 0;  iConverge_Red_V  = 0;
  iConverge_Blue_H = 0;  iConverge_Blue_V = 0;

  iShowVideo_Flag = 1; iCtl_ShowVideo_Flag = 1;

  iCtl_Time_Fmt = 1;       iCtl_Date_Internationale = 0;

  iView_Centre_Crop = 0;
  iView_SwapUV = 0;        iView_Negative = 0;  iView_Invert = 0;
  iCtl_View_Fast_YUV  = 0; iView_Fast_YUV  = 0; 
  iCtl_View_Fast_RGB = 0;  iView_Fast_RGB = 0; 
  iCtl_View_Centre_Crop = 1;  
  iCtl_AspMismatch   = 0;

  iViewToolBar = 0; iCtl_ViewToolbar[0] = 0; iCtl_ViewToolbar[1] = 0;
  iCtl_Trackbar_Big = 1;  
  hFastX = 0;
  iPlayBar_PosY = 0;  iSkipBar_PosY = 0;

  iField_Drop = 0;
  iCtl_Ovl_DWord = 0;   iCtl_YV12 = 0;
  iField_Experiment = 0; 
  iCtl_Drv_Segments = 0;  
  
  
  uCtl_Video_Stream  = STREAM_AUTO; // VIDEO_ELEMENTARY_STREAM_1; // 
  uCtl_Vid_PID = STREAM_ALL;  
  uCtl_Aud_PID = STREAM_NONE; // STREAM_ALL;  
  uGot_PID = 999997;
  uVid_PID_All = 1;  uAud_PID_All = 0;

  iPES_Mpeg_Any = 0;


  iPred_Prev_Width  = 0;

  iCtl_Out_Preamble_Flag = 2 ;
  iCtl_Out_Preamble_VTS = 1 ;
  iCtl_Out_PTS_Invent = 0;  iCtl_Out_Force_Interlace = 0;
  iOut_HideAudio  = 0;  iOut_FixPktLens = 0;  iOut_PS1PS2 = 0;
  iCtl_Out_Parse_SplitStart = 1;
  iCtl_Out_SplitSegments    = 0;       
  iCtl_Out_KillPadding      = 0;  iCtl_OutPartAuto  = 0;
  iCtl_Out_DropCrud         = 0; // This is not sticky, since it is too scary

  iOut_UnMuxAudioOnly = 0;  iOut_UnMux_Fmt = 0;
  iOut_Audio_All = 1; 
  
  iOut_Audio_TrkSel[0] = 0;   iOut_Audio_TrkSel[1] = 0;
  iOut_Audio_TrkSel[2] = 0;   iOut_Audio_TrkSel[3] = 0;
  iOut_Audio_TrkSel[4] = 0;   iOut_Audio_TrkSel[5] = 0;
  iOut_Audio_TrkSel[6] = 0;   iOut_Audio_TrkSel[7] = 0;
  ZeroMemory(&cOut_SubStreamWanted,  sizeof(cOut_SubStreamWanted));
  

  Deint_AUTO_View = 1;  Deint_SNAP = 1;  iCtl_CropTop = 0;
  iCtl_BMP_Aspect = BMP_ASPECT_BICUBIC;
  iCtl_BMP_Preview = 1;

  iCtl_Drop_Behind = 0;
  iCtl_Drop_PTS = 0;
  iCtl_Play_AudLock = 1;
  iCtl_Play_Sync = 0; // Not yet ready for implementation
  iCtl_Byte_Sync = 0;
  iCtl_Play_Info = 0; iCtl_Play_Summary = 0;
  iCtl_Zoom_Wanted = -1;  // Set for AUTO zoom at start
  iCtl_Zoom_Retain = 0; 
  //iZoom_OLD =  1;
  iAspVert = 2048; iAspHoriz = 2048;
  iCtl_Ovl_Release = 1;  iCtl_View_Limit2k = 1;

  iCtl_Out_PostProc  = 0; iCtl_Out_PostShow = 1; 
  iCtl_Out_PostQuote = 0;
  szCtl_Out_ProcLine_A[0] = 0;  szCtl_Out_ProcLine_B[0] = 0;

  iCtl_SetBrokenGop    = 0;
  iCtl_Out_PTS_Match   = 1;   
  iCtl_Out_Align_Video = 1; // Experimental
  //iCtl_Out_Keep_Ac3Hdr = 1;

  iCtl_Audio_PS2  = 0;       
  iCtl_Volume_Boost = 0;       
  iVol_Boost_Cat = 0;       iVol_PREV_Cat = 0;
  iVol_BoostCat_Done[0] = 0;   iVol_BoostCat_Done[1] = 0; 
  iVol_BoostCat_Done[2] = 0;   iVol_BoostCat_Done[3] = 0; 
  iVol_BoostCat_Done[4] = 0;   iVol_BoostCat_Done[5] = 0; 
  iVol_BoostCat_Done[6] = 0;   iVol_BoostCat_Done[7] = 0; 

  iCtl_Volume_AUTO = 0; 
  iAudio_Force44K = 0;   iCtl_PALTelecide = 0;
  //iCtl_RecycleBin = 0;

  Loc_Method = 2;
  iCtl_To_Pad = 0;  // I-Frame TO Padding is not yet smart enough to be the default
  strcpy(szOut_Xtn_RULE,"MPG"); // "$");
  iSuppressWarnings = 0;   iCtl_WarnNoHdr = 1;
  iWarnBadDate = 0;  iWarnAC3inMPA = 0;

  iCtl_KB_NavOpt  = 1;
  iCtl_KB_MarkOpt = 0;
  iCtl_F3_Names = 0; iCtl_F5_Toggler = 0;
  iCtl_Wheel_Scroll = 1;  iCtl_FileSortSeq = 1;
  iCTL_FastBack = 1;
  iCtl_Name_Info = 0; iCtl_BasicName_Panel = 1;

  szInput  [0] = 0;
  szOutput [0] = 0; szCtl_OutFolder[0] = 0; szOutFolder[0] = 0;
  szEDLname[0] = 0;
}



//------------------------------------------------------
void INI_GET()
{
  DWORD iLen;
  char *ext;
  int iTmp1, iCtl_Warn_Fmts;
  int iDUMMY;

  // Find where we were loaded from
  iLen = GetModuleFileName(NULL, szLOAD_Path, sizeof(szLOAD_Path));
  ext = strrchr(szLOAD_Path, '\\');
  if (ext)
  {
     ext++;
    *ext = 0;
  }


  // Find out menu background color
  iColor_Menu_BG = GetSysColor(COLOR_MENU);

  strcpy(szINI_Path, szLOAD_Path);
  strcat(szINI_Path, "Mpg2Cut2.ini");

  INIFile = fopen(szINI_Path, "r") ;
  //if (INIFile == NULL)
  //    INIFile = fopen("MPEG2Cut.ini", "r");

  // *********************************************************************
  //
  //  CHANGED SO THAT MISSING INIFile WILL TRIGGER DEFAULT SETTINGS BELOW
  //
  // *********************************************************************

  /*
  if  (INIFile == NULL)
  {
       if (DBGflag) DBGout("INI file NOT FOUND");
  }
  */
  //else  
  {
    if (! INIFile) iValCtr = 0;
    else
    {
      iValCtr = fscanf(INIFile, "INI_Version=%d\n", &INI_Version);
    //if (INI_Version != PGM_INI)
    //{
    //   if (DBGflag) DBGout("INI file WRONG VERSION");
    //}
    //else
    //{

      iValCtr = fscanf(INIFile, "Window_Position=%d,%d\n", &Restore_X, &Restore_Y);
      iValCtr = fscanf(INIFile, "iDCT_Algorithm=%d\n",  &MParse.iDCT_Flag);
      iValCtr = fscanf(INIFile, "YUVRGB_Scale=%d\n",  &MParse.PC_Range_Flag);
      iValCtr = fscanf(INIFile, "Field_Operation=%d,%d\n", &MParse.FO_Flag,
                                                      &iCtl_View_Fast_YUV);
    }
      if (iValCtr < 2) 
          iCtl_View_Fast_YUV = 1;
      else  
          iCtl_View_Fast_YUV = 1 -  iCtl_View_Fast_YUV;

    if (! INIFile) iValCtr = 0;
    else  
    {
      iValCtr = fscanf(INIFile, "Track_Number=%d\n", &iAudio_SEL_Track);
      iValCtr = fscanf(INIFile, "Channel_Format=%d\n",  &iTmp1);  // SUPPRESED :- &iWant_Aud_Format);
//    iValCtr = fscanf(INIFile, "AC3=%d\n", &AC3_Flag);
//    iValCtr = fscanf(INIFile, "DR_Control=%d\n", &AC3_DRC_FLag);
      iValCtr = fscanf(INIFile, "DS_Downmix=%d\n", &AC3_DSDown_Flag);
//    iValCtr = fscanf(INIFile, "MPA=%d\n", &MPA_Flag);
//    iValCtr = fscanf(INIFile, "SRC_Precision=%d\n", &SRC_Flag);
//    iValCtr = fscanf(INIFile, "Norm_Ratio=%d\n", &Norm_Ratio);
      iValCtr = fscanf(INIFile, "Process_Priority=%d\n", &iCtl_Priority[0]);
    }

    if (! INIFile) iValCtr = 0;
    else  
    {
      iValCtr = fscanf(INIFile, "Luminance=%d,%d,%d,%d,%d,%d\n",
                           &iLumGain[0], &iLumOffset[0],&iLumGamma[0],
                           &iLumEnable_Flag[0], &iTmp1, 
                           &iLumEnable_Flag[1]);
      iLumLock_Flag = iTmp1 & 0xFF;
      iSatLock_Flag = iTmp1 / 256;
    }

    if (iValCtr < 5)
    {
        iLumGain[0] = 128; iLumOffset[0] = 0; iLumGamma[0] = 130; // YUY2
        iLumEnable_Flag[0] = 1;  
    }

    if (! INIFile) iValCtr = 0;
    else  
    {
      iValCtr = fscanf(INIFile, "Keyboard=%d,%d\n", &iCtl_KB_NavOpt, &iCtl_KB_MarkOpt);
    }

    if (iValCtr < 1) iCtl_KB_NavOpt = 2;      // default to Vdub nav

    iCtl_KB_NavOpt  = iCtl_KB_NavOpt>>1;
    iCtl_KB_MarkOpt = iCtl_KB_MarkOpt>>1;

    if (! INIFile) iValCtr = 0;
    else  
    {
      iValCtr = fscanf(INIFile, "Audio_Decoder=%s\n", &szMPAdec_NAME);
      iValCtr = fscanf(INIFile, "AddAuto=%d,%d\n",    &Add_Automation,
                                                    &iCtl_To_Pad);
    }
    if (iValCtr < 1)
    {
        Add_Automation = 2;          // default to AUTO
    }

    if (! INIFile) iValCtr = 0;
    else  
    {
      iValCtr = fscanf(INIFile, "Preamble=%d\n",     &iCtl_Out_Preamble_Flag);
    }
    if (iValCtr < 1)  
      iCtl_Out_Preamble_Flag = 1;

    if (! INIFile) iValCtr = 0;
    else iValCtr = fscanf(INIFile, "Deinterlace=%d,%d,%d\n",  &Deint_AUTO_View,
                                                         &Deint_VOB,
                                                         &Deint_SNAP);
    if (iValCtr < 3)
    {
      if (iValCtr < 1)
          Deint_AUTO_View = 1;
      if (iValCtr < 2)
          Deint_VOB = Deint_AUTO_View;
      Deint_SNAP = Deint_AUTO_View;   
    }

    if (! INIFile) iValCtr = 0;
    else 
    {
      iValCtr = fscanf(INIFile, "OutXtn=%s\n",             &szOut_Xtn_RULE);
      iValCtr = fscanf(INIFile, "InputFile=\"%[^\"]\"\n",  &szInput);
      iValCtr = fscanf(INIFile, "OutputFile=\"%[^\"]\"\n", &szOutput);
      iValCtr = fscanf(INIFile, "OutFolder=%d,\"%[^\"]\"\n",
                                             &iCtl_OutFolder_Active,
                                            &szCtl_OutFolder);
    }
    if (iValCtr < 2)
    {
         szCtl_OutFolder[0] = 0;
         if (iValCtr < 1)
           iCtl_OutFolder_Active = 0;
    }
    strcpy(&szOutFolder[0], &szCtl_OutFolder[0]);

    if (! INIFile) iValCtr = 0;
    else
    {
      // New parameters that default to TRUE
      iValCtr = fscanf(INIFile,K_DefaultY_FORMAT, // "DefaultY=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                              &iCtl_OutFolder_Both,  &iCtl_Audio_CRC,
                              &iCtl_Out_PTS_Match,    &iCtl_Out_Align_Video,
                              &iCtl_Out_Preamble_VTS, &iCtl_Play_AudLock,
                              &iCTL_FastBack,            &iCtl_Track_Memo, 
                              &iCtl_WarnSize_1,       &iCtl_WarnSize_2,
                              &iCtl_WarnSize_3,       &iCtl_WarnSize_4,
                              &iCtl_KB_NavStopPlay,   &iCtl_Ovl_Release,
                              &iCtl_SetBrokenGop,     &iCtl_Warn_Fmts,
                              &iCtl_Time_Fmt,         &iCtl_WarnBadStart,        
                              &iCtl_EDL_AutoSave,     &iCtl_RecycleBin, 
                              &iCtl_View_Centre_Crop);
    }
    if (iValCtr < 1)
    {
                        iCtl_Audio_CRC        =1; iCtl_Track_Memo      =1;
                        iCtl_Out_PTS_Match    =1; iCtl_Out_Align_Video =1;
                        iCtl_Out_Preamble_VTS =1; iCtl_Play_AudLock    =1;
                        iCtl_WarnSize_1       =1; iCtl_WarnSize_2      =1;
                        iCtl_WarnSize_3       =1; iCtl_WarnSize_4      =1;
                        iCtl_KB_NavStopPlay   =1; iCtl_SetBrokenGop    =1;
                        iCtl_Warn_Fmts        =1; iCtl_Time_Fmt        =1;
                        iCtl_WarnBadStart     =1; iCtl_EDL_AutoSave    =1;
                        iCtl_RecycleBin       =1; iCtl_OutFolder_Both  =1;
                        iCtl_View_Centre_Crop =1;
    }

    // bit flags for format warnings
    if (iCtl_Warn_Fmts   == 1)             // Old style
        iCtl_Warn_Fmts   =  0xFFFFFFFF;  // Old style needs to be upgraded
    // bitulate
    iCtl_WarnMpeg1 = iCtl_Warn_Fmts & 0x01;
    iCtl_WarnTS    = iCtl_Warn_Fmts & 0x02;
    iCtl_WarnTSmpg = iCtl_Warn_Fmts & 0x04; // TS named .mpg
    iCtl_WarnCDXA  = iCtl_Warn_Fmts & 0x08; 
    iCtl_WarnDone  = iCtl_Warn_Fmts & 0x10; 
    iCtl_WarnETC   = iCtl_Warn_Fmts & 0xFFFFFFE0; // and the rest

    iCtl_WarnBadSysHdr = (1 - (iCtl_WarnBadStart / 256));
    iCtl_WarnBadStart  = iCtl_WarnBadStart &0xFF;

    // New parameters that default to ZERO

    if (! INIFile) iValCtr = 0;
    else
    {
      iValCtr = fscanf(INIFile,K_PatchHdr_FORMAT,
                      &iCtl_Out_Fix_SD_Hdr,   &iCtl_Out_Parse,
                      &iCtl_Out_Seq_End,      &iCtl_VOB_Style,
                      &iCtl_Out_TC_Adjust,    &iCtl_Out_Align_Audio,
                      &iCtl_Out_Parse_Extras, &iCtl_Out_DeBlank, 
                      &iCtl_Out_MixedCase,    &iCtl_Out_KeepFileDate, 
                      &iCtl_Out_TC_Force,     &iCtl_ParmConfirm,
                      &iCtl_BMP_Aspect,       &iCtl_View_Aspect_Mpeg1_Force, 
                      &iCtl_Out_SysHdr_Mpeg,  &iCtl_Out_Fix_Errors);
    }

    if (iValCtr < 12)
    {
                    iCtl_Out_Fix_SD_Hdr=0;       iCtl_Out_Parse = 1;
                    iCtl_Out_Seq_End=0;          iCtl_VOB_Style=0;
                    iCtl_Out_TC_Adjust=0;        iCtl_Out_Align_Audio = 1;
                    iCtl_Out_Parse_Extras = 0; // 129; 
                    iCtl_Out_DeBlank=0; 
                    iCtl_Out_MixedCase=0;        iCtl_Out_KeepFileDate=0; 
                    iCtl_Out_TC_Force=0;         iCtl_ParmConfirm=0;
    }

    if (iValCtr < 16)
    {
                    iCtl_View_Aspect_Mpeg1_Force = 0;
                    iCtl_Out_SysHdr_Mpeg = 0;
                    iCtl_Out_Fix_Errors  = 0;
    }

    // Suppressed following 2 options due to unknown bugs
    iCtl_Out_Parse_AllPkts = 0; // iCtl_Out_Parse_Extras & 0xFF;
    iCtl_Out_Parse_Deep    = 0; // iCtl_Out_Parse_Extras / 256; 



    if (! INIFile) iValCtr = 0;
    else
    {
      iValCtr = fscanf(INIFile,K_AudioX_FORMAT,
                      &iCtl_Volume_Boost, &iCtl_Audio_PS2,
                      &iCtl_Play_Sync,   &iCtl_Drop_Behind,
                      &iCtl_Play_Info,   &iCtl_Drop_PTS, 
                      &iCtl_Priority[1], &iCtl_Priority[2],
                      &iCtl_DropAction,  &iCtl_Play_Summary,
                      &iCtl_Name_Info,   &iCtl_Date_Internationale, 
                      &iTmp1, // &iCtl_BasicName_Panel, 
                      &iCtl_ColumnWidth[0], &iCtl_ColumnWidth[1],
                      &iCtl_ColumnWidth[2], &iCtl_ColumnWidth[3],
                      &iCtl_ColumnWidth[4], &iCtl_ColumnWidth[5],
                      &iCtl_AudioDecoder,
                      &iCtl_View_RGB_Always);
    }

    if (iValCtr < 11)
    {
                      iCtl_Volume_Boost=0; iCtl_Audio_PS2=0;
                      iCtl_Play_Sync=0;   iCtl_Drop_Behind=0;
                      iCtl_Play_Info=0;   iCtl_Drop_PTS=0; 
                      iCtl_Priority[1]=0; iCtl_Priority[2]=0;
                      iCtl_DropAction=0;  iCtl_Play_Summary=0;
                      iCtl_Name_Info=0; 
                      iCtl_ColumnWidth[0]=0; iCtl_ColumnWidth[1]=0;
                      iCtl_ColumnWidth[2]=0; iCtl_ColumnWidth[3]=0;
                      iCtl_ColumnWidth[4]=0; iCtl_ColumnWidth[5]=0;
                      iCtl_View_RGB_Always=0; 
                      iCtl_Drop_Behind = 257; // Should have been in a different group
    }


    if (iCtl_Volume_Boost > 128)
        iCtl_Volume_Boost = 128;

    if (iCtl_ColumnWidth[0] <= 0) 
        iCtl_ColumnWidth[0]  = 620;
    if (iCtl_ColumnWidth[1] <= 0) 
        iCtl_ColumnWidth[1]  = 150;

    if (! INIFile) iValCtr = 0;
    else iValCtr = fscanf(INIFile, "JumpSpan=%d,%d,%d,%d,%d,%d\n",  
                                      &iJumpSecs[0], &iJumpSecs[1], &iJumpSecs[2], 
                                      &iJumpSecs[3], &iJumpSecs[4], &iJumpSecs[5]); 
    if (iValCtr < 6)                                                         
    {
      iJumpSecs[0] =  2; iJumpSecs[1] =  20; iJumpSecs[2] =  50; 
      iJumpSecs[3] = -2; iJumpSecs[4] = -20; iJumpSecs[5] = -50; 
    }

    if (! INIFile) iValCtr = 0;               
    else           iValCtr = fscanf(INIFile, "Proc0A=%d,\"%[^\"]\"\n", // %d,\"%s\"\n",
                                             &iCtl_Out_PostProc,
                                            &szCtl_Out_ProcLine_A);

    if (! INIFile) iValCtr = 0;               
    else           iValCtr = fscanf(INIFile, "Proc0B=%d,\"%[^\"]\"\n", // %d,\"%s\"\n",
                                             &iCtl_Out_PostQuote,
                                            &szCtl_Out_ProcLine_B);
    if (! INIFile) iValCtr = 0;               
    else           iValCtr = fscanf(INIFile, "BMPFolder=%d,\"%[^\"]\"\n",
                                             &iCtl_BMP_Folder_Active,
                                            &szCtl_BMP_Folder);
    if (iValCtr < 2)
    {
         szCtl_BMP_Folder[0] = 0;
         if (iValCtr < 1)
           iCtl_BMP_Folder_Active = 0;
    }


    if (! INIFile) iValCtr = 0;
    else
    {
      iValCtr = fscanf(INIFile, "RenamePlugIn=%d,%c,\"%[^\"]\"\n",
                                          &PlugFileRename.iActive,
                                          &cRenamePlugIn_MultiMode, 
                                         &szRenamePlugIn_Name);
    }

    if (iValCtr < 3)
    {
       PlugFileRename.iActive = 0;
       cRenamePlugIn_MultiMode = 'S';
       cRenamePlugIn_AsyncMode = 0;
       strcpy(szRenamePlugIn_Name, ".\\WideRename.dll");
    }
    else
    {
       if (cRenamePlugIn_MultiMode == 'T')
       {
           cRenamePlugIn_MultiMode = 'S';
           cRenamePlugIn_AsyncMode = 'A';
       }
       else
       if (cRenamePlugIn_MultiMode == 'N')
       {
           cRenamePlugIn_MultiMode = 'M';
           cRenamePlugIn_AsyncMode = 'A';
       }
    }


    if (! INIFile) iValCtr = 0;
    else
    {
      iValCtr = fscanf(INIFile, "FileListPlugIn=%d,%c,\"%[^\"]\"\n",
                                          &PlugFileList.iActive,
                                          &cFileListPlugIn_MultiMode, 
                                         &szFileListPlugIn_Name);
    }
    if (iValCtr < 3)
    {
       PlugFileList.iActive = 0;
       cFileListPlugIn_MultiMode = 'S';
       cFileListPlugIn_AsyncMode = 0;
       strcpy(szFileListPlugIn_Name, ".\\WideFiles.dll");
    }
    else
    {
       if (cFileListPlugIn_MultiMode == 'T')
       {
           cFileListPlugIn_MultiMode = 'S';
           cFileListPlugIn_AsyncMode = 'A';
       }
       else
       if (cFileListPlugIn_MultiMode == 'N')
       {
           cFileListPlugIn_MultiMode = 'M';
           cFileListPlugIn_AsyncMode = 'A';
       }
    }





    if (! INIFile) iValCtr = 0;
    else
    {
      iValCtr = fscanf(INIFile, "Breathe=(%d,%d,%d),Pkts=(%d,%d,%d)\n",
                                          &iCtl_Out_Breathe_PerBigBlk[0],
                                          &iCtl_Out_Breathe_PerBigBlk[1],
                                          &iCtl_Out_Breathe_PerBigBlk[2],
                                          &iCtl_Out_Breathe_PktLim[0],
                                          &iCtl_Out_Breathe_PktLim[1],
                                          &iCtl_Out_Breathe_PktLim[2]
                                          );
    }

    if (iValCtr < 6
    || (iCtl_Out_Breathe_PerBigBlk[0] == 256 &&
        iCtl_Out_Breathe_PerBigBlk[1] == 128 &&
        iCtl_Out_Breathe_PerBigBlk[2] ==  32 &&
        iCtl_Out_Breathe_PktLim[0] ==   3    &&
        iCtl_Out_Breathe_PktLim[1] ==  32    &&
        iCtl_Out_Breathe_PktLim[2] == 256))
    {
        if (cpu.sse2)
        {
            iCtl_Out_Breathe_PerBigBlk[0] = 200;
            iCtl_Out_Breathe_PerBigBlk[1] = 128;
            iCtl_Out_Breathe_PktLim[0]    =   8;   
            iCtl_Out_Breathe_PktLim[1]    =  32;
        }
        else
        {
            iCtl_Out_Breathe_PerBigBlk[0] = 200;
            iCtl_Out_Breathe_PerBigBlk[1] =  16;
            iCtl_Out_Breathe_PktLim[0]    =   4;   
            iCtl_Out_Breathe_PktLim[1]    = 128;
        }

        iCtl_Out_Breathe_PerBigBlk[2] =    1;
        iCtl_Out_Breathe_PktLim[2]    = 2048;
    }



    if (! INIFile) iValCtr = 0;
    else
    {
      iValCtr = fscanf(INIFile, "Color=x%06X,BG=x%06X,MASK=x%06X\n",
                    &iCtl_Text_Colour, 
                    &iTmp1, // &iCtl_Back_Colour, 
                    &iCtl_Mask_Colour);
    }
    if (iValCtr < 3)
    {
       iCtl_Text_Colour = 0xFFFEFE;  // Text = Bright Blue
       iCtl_Mask_Colour = 0x000600;  // Overlay key = Dark Green
       iCtl_Back_Colour = iCtl_Mask_Colour; //0x000000;  // Background = Black
    }
    iCtl_Mask_Fallback = 0;
               

    if (INIFile)
    {
      iValCtr = fscanf(INIFile, "EDL=\"%[^\"]\"\n", &szEDLname);
    }
    if (szEDLname[0] <= ' ')
        strcpy(szEDLname, "*.EDL");

    if (INIFile)
    {
      iValCtr = fscanf(INIFile, "LumBMP=%d,%d,%d,%d,%d,%d\n",
                           &iLumGain[1], &iLumOffset[1],&iLumGamma[1],
                           &iLumEnable_Flag[1], &iTmp1, &iTmp1);
    }

    if (iValCtr < 4)
    {
       iLumGain[1] = 128; iLumOffset[1] = 0; iLumGamma[1] = 130; // RGB-BMP
       iLumEnable_Flag[1] = 1;
    }

    if (INIFile)
    {
      iValCtr = fscanf(INIFile, "OutXtnAudio=%s\n",       &szOut_Xtn_AUD);
    }
    if (iValCtr < 1  
    ||  szOut_Xtn_AUD[0] <= ' ')
        strcpy(szOut_Xtn_AUD, "M2A");

    if (! INIFile) iValCtr = 0;
    else
    {
      // More New parameters that default to TRUE (You Are at Y2)
      iValCtr = fscanf(INIFile, K_Y2, 
                           &iCtl_View_Fast_RGB,     &iCtl_Readability, 
                           &iCtl_Copy_BufSz_Ix,     &iCtl_ToolTips,
                           &iCtl_Trackbar_Big,      &iCtl_Wheel_Scroll, 
                           &iCtl_FileSortSeq,       &iCtl_Out_SysHdr_EveryClip,
                           &iCtl_Out_SysHdr_Unlock, &iCtl_View_Limit2k, 
                           &iCtl_BMP_Preview,       &iDUMMY, 
                           &iCtl_Out_PostShow, 
                           &iDUMMY, 
                           &iDUMMY, &iDUMMY);  // Don't go West  
    }
    if (iValCtr < 16)
    {
                        iCtl_View_Fast_RGB = 1; iCtl_Readability  = 1;
                        iCtl_Copy_BufSz_Ix = 1; iCtl_ToolTips     = 1;
                        iCtl_Trackbar_Big  = 1; iCtl_Wheel_Scroll = 1;
                        iCtl_Out_SysHdr_EveryClip = 1;
                        iCtl_Out_SysHdr_Unlock    = 1;
                        iCtl_View_Limit2k = 1;  iCtl_BMP_Preview  = 1;
                        iCtl_Out_PostShow = 1;  
    }


    if (! INIFile) iValCtr = 0;
    else
    {
      // More New parameters that default to ZERO
      iValCtr = fscanf(INIFile, K_N2, 
                              &iCtl_F3_Names, &iCtl_ParmClipSpec, 
                              &iCtl_Out_Force_Interlace, 
                              &iCtl_Out_KillPadding,
                              &iCtl_VistaOVL_mod, 
                              &iCtl_Volume_AUTO,
                              &iCtl_OVL_FullKey, 
                              &AC3_DRC_FLag,
                              &iCtl_Zoom_Retain, &iCtl_Zoom_Wanted,
                              &iCtl_Aspect_Retain, &iView_Aspect_Mode,
                              &iCtl_Out_SplitSegments, 
                              &iCtl_YV12, &iCtl_AspMismatch,
                              &iCtl_CropTop);    
    }
    if (iValCtr < 16)
    {
      iCtl_F3_Names = 0;  iCtl_ParmClipSpec = 0;
      AC3_DRC_FLag = DRC_HEAVY;
      iCtl_Zoom_Retain = 0;  iCtl_Zoom_Wanted = -1;
      iCtl_Aspect_Retain = 0;
      iView_Aspect_Mode = 4;
    }
    else
    {
       AC3_DRC_FLag = AC3_DRC_FLag + 3;
       if (AC3_DRC_FLag > DRC_VERYHEAVY)
           AC3_DRC_FLag = DRC_VERYHEAVY;
       else
       if (AC3_DRC_FLag < DRC_NONE)
           AC3_DRC_FLag = DRC_NONE;

       iCtl_Zoom_Wanted  -= 1;
       iView_Aspect_Mode += 4;
    }

    iCtl_F5_Toggler = iCtl_F3_Names / 256;
    iCtl_F3_Names   = iCtl_F3_Names & 255;



    if (INIFile)
    {
      iValCtr = fscanf(INIFile, K_LumPresets, 
                           &iLumGain[2], &iLumOffset[2],&iLumGamma[2],  // Default
                           &iLumGain[3], &iLumOffset[3],&iLumGamma[3],  // Bold
                              &iDUMMY, &iDUMMY, &iDUMMY, 
                           &iLumGain[4], &iLumOffset[4],&iLumGamma[4],  // C
                           &iLumGain[5], &iLumOffset[5],&iLumGamma[5],  // A
                              &iDUMMY, &iCtl_SinThreshold, &iCtl_VHS_Threshold); 
    }

    if (iValCtr < 6)
    {
       iLumGain[2] = 128; iLumOffset[2] =  0; iLumGamma[2] = 130; // Default
       iLumGain[3] = 168; iLumOffset[3] = 40; iLumGamma[3] = 130;  // Bold
    }

    if (iLumGamma[4] == 0 && iLumGain[4] == 0 && iLumOffset[4] == 0)
    {
       iLumGain[4] = 148; iLumOffset[4] = 20; iLumGamma[4] = 130;  // C
    }
    if (iLumGamma[5] == 0 && iLumGain[5] == 0 && iLumOffset[5] == 0)
    {
       iLumGain[5] = 178; iLumOffset[5] = 50; iLumGamma[5] = 130;  // A
    }

    if (iCtl_VHS_Threshold ==  0)
        iCtl_VHS_Threshold  = 16;
    if (iCtl_SinThreshold  ==  0)
        iCtl_SinThreshold   = iCtl_VHS_Threshold;


    if (! INIFile) iValCtr = 0;
    else
    {
      // More New parameters that default to ZERO
      iValCtr = fscanf(INIFile, K_N3, 
                              &iCtl_View_Aspect_Adjust, 
                              &iSatAdd_U[0], &iSatAdd_V[0], &iSatGain[0],
                              &iTmp2, // = (iCtl_SAT_Retain * 256 + iSatAdj_Flag),
                              &iCtl_Lum_Deselector, 
                              &iCtl_OutPartAuto, &iCtl_OVL_ATI_Bug, 
                              &iCtl_NotSoFast, 
                              &uAud_PID_All, &iCtl_KB_V_Popup,
                              &iDUMMY,
                              &iDUMMY, &iDUMMY,
                              &iCtl_ViewToolbar[0], &iCtl_ViewToolbar[1]);    
    }
    if (iValCtr < 16)
    {
        iCtl_View_Aspect_Adjust = 100;
        iSatGain[0]  = 100;  
    }
    else
    {
       iCtl_SAT_Retain = iTmp2 / 256;
       if (iCtl_SAT_Retain)
           iSatAdj_Flag = (iTmp2 & 0xFF);

       iCtl_View_Aspect_Adjust = iCtl_View_Aspect_Adjust + 100;
       if (iCtl_View_Aspect_Adjust < 5)
           iCtl_View_Aspect_Adjust = 100;
       iSatGain[0] = iSatGain[0] + 100;
    }

    iCtl_ViewToolbar[0] = 257 - iCtl_ViewToolbar[0];
    iCtl_ViewToolbar[1] = 257 - iCtl_ViewToolbar[1];
    iViewToolBar = iCtl_ViewToolbar[0];

    iTmp1 = 0;
    if (! INIFile) iValCtr = 0;
    else
    {
      // Volume Control parameters
      iValCtr = fscanf(INIFile, K_VOLUME, 
                              &iCtl_Vol_BoostCat_Init[FORMAT_MPA], // Legacy style mpeg (44kHz or less)
                              &iCtl_Vol_BoostCat_Init[FORMAT_MPA_TRENDY],       // Modern style mpeg (48kHz or higher)
                              &iCtl_Vol_BoostCat_Init[FORMAT_AC3],             // Basic AC3 (stereo or mono)
                              &iCtl_Volume_Limiting, &iCtl_Volume_Ceiling,
                              &iCtl_Volume_SlowAttack,
                              &iCtl_Vol_StarKey, 
                              &iCtl_Vol_BoostCat_Init[FORMAT_LPCM],        // LPCM
                              &iCtl_Vol_BoostCat_Init[FORMAT_DTS],         // DD+ or AC3 with more than 2 channels
                              &iTmp1, 
                              &iCtl_Volume_Retain, 
                              &iDUMMY,
                              &iDUMMY, &iDUMMY,
                              &iCtl_Vol_Prev_Denom, &iCtl_Audio_InitBreathe);    
    }

    iCtl_Volume_Retain = 1- iCtl_Volume_Retain;

    if (iTmp1 == 0)
        iTmp1  = 0xFFFFFE; // Default to boosting everything except MPA_TRAD
    
    if (iCtl_Vol_Prev_Denom != K_BOOST_DENOM) // Settings based on previous scale are no good
    {
        iCtl_Vol_BoostCat_Init[FORMAT_MPA_TRENDY] =  0;
        iCtl_Vol_BoostCat_Init[FORMAT_MPA]  =  0;
        iCtl_Vol_BoostCat_Init[FORMAT_AC3]  =  0;
        iCtl_Vol_BoostCat_Init[FORMAT_LPCM] =  0;
        iCtl_Vol_BoostCat_Init[FORMAT_DTS]  =  0;
    }
    else
    if (iCtl_Volume_Retain)
    {
        //memcpy(&iCtl_Vol_BoostCat_Done, &iVol_BoostCat_Init,
        //                        sizeof(iCtl_Vol_BoostCat_Done));
      
        iVol_BoostCat_Done[FORMAT_MPA] = 
            iCtl_Vol_BoostCat_Init[FORMAT_MPA];
        iVol_BoostCat_Done[FORMAT_MPA_TRENDY] = 
            iCtl_Vol_BoostCat_Init[FORMAT_MPA_TRENDY];
        iVol_BoostCat_Done[FORMAT_AC3] =  
            iCtl_Vol_BoostCat_Init[FORMAT_AC3]; 
        iVol_BoostCat_Done[FORMAT_LPCM] = 
            iCtl_Vol_BoostCat_Init[FORMAT_LPCM];
        iVol_BoostCat_Done[FORMAT_DTS] =   
            iCtl_Vol_BoostCat_Init[FORMAT_DTS];  
      
    }

    if (iCtl_Vol_BoostCat_Init[FORMAT_MPA_TRENDY] <= 0)
        iCtl_Vol_BoostCat_Init[FORMAT_MPA_TRENDY]  = (K_BOOST_DENOM*3); // 46; // 16;
    if (iCtl_Vol_BoostCat_Init[FORMAT_MPA]  <=  0)
        iCtl_Vol_BoostCat_Init[FORMAT_MPA]   = (K_BOOST_DENOM*2); // 30; //  8;
    if (iCtl_Vol_BoostCat_Init[FORMAT_AC3]  <=  0)
        iCtl_Vol_BoostCat_Init[FORMAT_AC3]   = (K_BOOST_DENOM*4); // 62; // 24;
    if (iCtl_Vol_BoostCat_Init[FORMAT_LPCM] <=  0)
        iCtl_Vol_BoostCat_Init[FORMAT_LPCM]  = (K_BOOST_DENOM*2); // 30; //  8;
    if (iCtl_Vol_BoostCat_Init[FORMAT_DTS]  <=  0)
        iCtl_Vol_BoostCat_Init[FORMAT_DTS]   = (K_BOOST_DENOM*16);

    // Debitulate the Boost Flags by category
    iCtl_Vol_BoostCat_Flag[0] = (iTmp1 & 1);
    if (iTmp1 & 2)
        iCtl_Vol_BoostCat_Flag[1] = 1;
    else
        iCtl_Vol_BoostCat_Flag[1] = 0;
    if (iTmp1 & 4)
        iCtl_Vol_BoostCat_Flag[2] = 1;
    else
        iCtl_Vol_BoostCat_Flag[2] = 0;
    if (iTmp1 & 8)
        iCtl_Vol_BoostCat_Flag[3] = 1;
    else
        iCtl_Vol_BoostCat_Flag[3] = 0;
    if (iTmp1 & 16)
        iCtl_Vol_BoostCat_Flag[4] = 1;
    else
        iCtl_Vol_BoostCat_Flag[4] = 0;
    if (iTmp1 & 32)
        iCtl_Vol_BoostCat_Flag[5] = 1;
    else
        iCtl_Vol_BoostCat_Flag[5] = 0;
    if (iTmp1 & 64)
        iCtl_Vol_BoostCat_Flag[6] = 1;
    else
        iCtl_Vol_BoostCat_Flag[6] = 0;
    if (iTmp1 & 128)
        iCtl_Vol_BoostCat_Flag[7] = 1;
    else
        iCtl_Vol_BoostCat_Flag[7] = 0;


    if (iCtl_Volume_Boost)
        iVolume_Boost = iCtl_Vol_BoostCat_Init[FORMAT_MPA];
    else
        iVolume_Boost = 0;

    if (iCtl_Volume_AUTO && iCtl_Volume_Boost)
        iVolume_AUTO  = iCtl_Vol_BoostCat_Init[FORMAT_MPA];
    else
        iVolume_AUTO = 0;

    if (iCtl_Volume_Ceiling == 24000     // old default was a bit boring
    ||  iCtl_Volume_Ceiling == 32767 )  
        iCtl_Volume_Ceiling =  0; // reset 
    else
    if (iCtl_Volume_Limiting)
    {
        if (iCtl_Volume_Ceiling <= 0)  // default
            iVolume_Ceiling = K_VOL_CEILING_DEF; // quarter of max (max=32767)
        else
        if (iCtl_Volume_Ceiling < 512) // disallow silly values
            iVolume_Ceiling = 512;
        else
           iVolume_Ceiling = iCtl_Volume_Ceiling; // user defined ceiling
    }
    else
        iVolume_Ceiling = 32767; // no limit means limit = max

    if (INIFile) fclose(INIFile);
  }



  // Check that titlebar is not too hidden by taskbar
  if (Restore_X < rcAvailableScreen.left)
      Restore_X = rcAvailableScreen.left;

  if (Restore_Y < rcAvailableScreen.top)
      Restore_Y = rcAvailableScreen.top;


  iTmp1 = VGA_Width - 200;
  if (Restore_X > iTmp1)
      Restore_X = iTmp1;

  iTmp1 = VGA_Height - 200;
  if (Restore_Y > iTmp1)
      Restore_Y = iTmp1;

    
  if (szInput[0] == '.')
      szInput[0] = 0;
  lpFName = lpLastSlash(&szInput[0]);
  if (lpFName)
      lpFName++;
  else
      lpFName = &szInput[0];

  if (szOutput[0] == '.')
      szOutput[0] = 0;
  if (szEDLname[0] == '.')
      szEDLname[0] = 0;
  szEDLprev[0] = 0;
  if (szCtl_OutFolder[0] == '.')
      szCtl_OutFolder[0]  =  0;
  if (szCtl_BMP_Folder[0] == '.')
      szCtl_BMP_Folder[0]  =  0;
  if (szCtl_Out_ProcLine_A[0] == '.')
      szCtl_Out_ProcLine_A[0]  =  0;
  if (szCtl_Out_ProcLine_B[0] == '.')
      szCtl_Out_ProcLine_B[0]  =  0;


}


//---------------------------------------
void INI_MERGE()
{
  HANDLE hZERO=0;
  unsigned uTmp1;

  if (WindowsVersion < 0x80000000) //  Windows NT, Win2k, WinXP ?
    EnableMenuItem(hMenu, IDM_FILE_GARBAGE,  MF_GRAYED);

  iMPA_Best = 1;  

  if (cpu.mmx)
  {
    MenuTick(IDM_MMX);
    iMPA_Best = 2;
    MParse.iDCT_Flag  =  IDCT_MMX;
  }
  else
    EnableMenuItem(hMenu, IDM_IDCT_MMX,   MF_GRAYED);


  if (cpu.ssemmx)
  {
    MenuTick(IDM_SSEMMX);
    EnableMenuItem(hMenu, IDM_IDCT_MMX,    MF_GRAYED);
    iMPA_Best = 3;
    MParse.iDCT_Flag  =  IDCT_SSEMMX;
  }
  else
    EnableMenuItem(hMenu, IDM_IDCT_SSEMMX, MF_GRAYED);


  if (cpu.ssefpu)
  {
    MenuTick(IDM_SSEFPU);
    iMPA_Best = 3;
  }
  else
    EnableMenuItem(hMenu, IDM_IDCT_FPU, MF_GRAYED);


  if (cpu.sse2)
  {
      MenuTick(IDM_SSE2);
      // MParse.iDCT_Flag  =  IDCT_SSE2;
  }
  else
     EnableMenuItem(hMenu, IDM_IDCT_SSE2,   MF_GRAYED);


  if (cpu._3dnow)
      MenuTick(IDM_3DNOW);
  else
      EnableMenuItem(hMenu, IDM_IDCT_3DNOW,  MF_GRAYED);

  iMpa_AUTO = 0;
  if (iCtl_AudioDecoder >= 0 
  &&  iCtl_AudioDecoder <= 3)
  {
      ClearMPALib(iCtl_AudioDecoder);
  }
  else
  if (iCtl_AudioDecoder == 0)
      iMpa_AUTO = 1;

  if (uAud_PID_All)
      uTmp1 = IDM_AUDPID_AUTO;
  else
      uTmp1 = IDM_AUDPID_NONE;
  CheckMenuItem(hMenu, uTmp1, MF_CHECKED);



  if (iCtl_KB_NavOpt)
      MenuTick(IDM_KBNAV_VDUB);
  if (iCtl_KB_MarkOpt)
      MenuTick(IDM_KBMARK_VDUB);
  if (iCtl_KB_NavStopPlay)
      MenuTick(IDM_KB_STOPPLAY);
  if (iCtl_KB_V_Popup)
      MenuTick(IDM_KB_V_POPUP);

  if (iCtl_F3_Names)
      MenuTick(IDM_F3_NAMES);
  if (iCtl_F5_Toggler)
      MenuTick(IDM_F5_TOGGLER);
  
  if (iCTL_FastBack)
      MenuTick(IDM_NAV_INDEX);
  if (iCtl_View_Fast_YUV)
      MenuTick(IDM_YUV_FAST);
  if (iCtl_View_Fast_RGB)
      MenuTick( IDM_RGB_FAST);
  if (iCtl_View_Centre_Crop)
      MenuTick(IDM_VIEW_CTR_MAX);
  if (iCtl_AspMismatch)
      MenuTick(IDM_ASPECT_MISMATCH);

  if (iCtl_YV12)
      MenuTick(IDM_YV12);


  MenuTick(IDM_ASPECT_STD);
  MenuTick(IDM_LOC_HDR);

  Set_Preamble_Mode(iCtl_Out_Preamble_Flag);

  if (iCtl_Out_Preamble_VTS)
      MenuTick(IDM_PREAMBLE_VTS);
  if (iCtl_Play_Sync)
      MenuTick(IDM_PLAY_SYNC);
  if (iCtl_Play_Summary)
      MenuTick(IDM_PREVIEW_SUMMARY);

  if (iCtl_Drop_Behind)
  {
     MenuTick(IDM_SKIP_BEHIND_HD);
     if (iCtl_Drop_Behind > 1)
        MenuTick(IDM_SKIP_BEHIND_SD);
  }

  if (iCtl_Drop_PTS)
      MenuTick(IDM_SKIP_PTS);


  // if never set before
  if (iCtl_NotSoFast == 0)
  {   // default based on cpu speed
      if (!cpu.sse2 && !cpu._3dnow)  // Slower CPU ?
          iCtl_NotSoFast = 0;
      else
          iCtl_NotSoFast = 1;
  }
  else
    iCtl_NotSoFast--; // retrieved setting is warped to distinguish from never set before


  if (iCtl_NotSoFast)
  {
     MenuTick(IDM_CUE_SLOW);  // iCtl_NotSoFast
     iCtl_CUE_BUTTON_Speed = CUE_SLOW;
  }
  else
     iCtl_CUE_BUTTON_Speed = CUE_SLOW+1;



  if (iCtl_Out_DeBlank)
      MenuTick(IDM_OUT_DEBLANK);
  if (iCtl_Out_MixedCase)
      MenuTick(IDM_OUT_MIXEDCASE);

  if (iCtl_OutPartAuto)
       MenuTick(IDM_OUT_PART_AUTO);


  if (iCtl_ParmConfirm)
      MenuTick(IDM_PARM_CONFIRM);
  iParmConfirm = iCtl_ParmConfirm;

  if (iCtl_ParmClipSpec)
      MenuTick(IDM_PARM_CLIP);

  Set_XTN_PS(szOut_Xtn_RULE);
  Set_XTN_AUD(szOut_Xtn_AUD);
  Set_ADD(Add_Automation);

  if (iCtl_To_Pad)
      MenuTick(IDM_ADD_PAD);

  if (Deint_AUTO_View)
  {
      MenuTick(IDM_DEINT_AUTO);
      Deint_VIEW = Deint_Auto_CURR;
  }
  Deint_Auto_CURR = Deint_AUTO_View;

  if (Deint_VOB)
      MenuTick(IDM_DEINT_VOB);

  if (Deint_SNAP)
      MenuTick(IDM_DEINT_SNAP);

  if (Deint_AUTO_View)
  {
      MenuTick(IDM_DEINT_CURR);
  }


  if (iCtl_Out_KeepFileDate)
      MenuTick(IDM_OUT_KEEPDATE);

  if      (iCtl_OutFolder_Active == 0)   uTmp1 = IDM_OUT_FOLDER_SAME;
  else if (iCtl_OutFolder_Active == 2)   uTmp1 = IDM_OUT_FOLDER_RECENT;
  else if (iCtl_OutFolder_Active == 3)   uTmp1 = IDM_OUT_FOLDER_EVERY;
  else                                   uTmp1 = IDM_OUT_FOLDER_FIRST;

  MenuTick(uTmp1);

  if (iCtl_OutFolder_Both)
      MenuTick(IDM_OUT_FOLDER_DUAL);

  if (iCtl_Out_PostProc)
      MenuTick(IDM_POSTPROC);
  ParmTidy(&szCtl_Out_ProcLine_A[0], 0);
  ParmTidy(&szCtl_Out_ProcLine_B[0], 0);

  if (iCtl_Out_Seq_End)
      MenuTick(IDM_OUT_SEQEND);

  if (iCtl_Out_Fix_SD_Hdr)
  {
      MenuTick(IDM_OUT_SD_HDR);
      if (iCtl_Out_Fix_SD_Hdr > 127)
          MenuTick(IDM_OUT_DEFLAG);
  }

  if (iCtl_Out_SysHdr_Mpeg)
      MenuTick(IDM_OUT_SYS_MPEG);
  else
      MenuTick(IDM_OUT_SYS_VOB);

  if (iCtl_Out_SysHdr_EveryClip)
      MenuTick(IDM_OUT_SYS_CLIP);
  if (iCtl_Out_SysHdr_Unlock)
      MenuTick(IDM_OUT_SYS_UNLOCK);
  
  if (iCtl_Out_Fix_Errors)
      MenuTick(IDM_OUT_FIX_ERRORS);
  if (iCtl_Out_Fix_Errors)
      MenuTick(IDM_OUT_FIX_ERRORS);

  if (iCtl_Out_Force_Interlace)
      MenuTick(IDM_OUT_INTERLACE);

  Out_SetBufSz(iCtl_Copy_BufSz_Ix);
  Set_Parse_Ticks();

  if (iCtl_BMP_Folder_Active)
      MenuTick(IDM_BMP_FOLDER_TOGGLE);
  iBMP_Folder_Active = iCtl_BMP_Folder_Active;

  Set_Time_Fmt(iCtl_Time_Fmt);

  Set_Bmp_AutoPreview(iCtl_BMP_Preview);
  Set_Bmp_Fmt(iCtl_BMP_Aspect);
  Set_Aspect_MPG1(iCtl_View_Aspect_Mpeg1_Force);

  if (iCtl_CropTop)
      MenuTick(IDM_CROP_TOP);

  // This could be made table driven...

  if (iCtl_VOB_Style)
      MenuTick(IDM_VOB_CHUNKS);

  if (iCtl_Play_AudLock)
      MenuTick(IDM_PLAY_AUDLOCK);
  if (iCtl_Play_Info)
      MenuTick(IDM_PLAY_INFO);

  if (iCtl_Audio_PS2)
      MenuTick(IDM_AUDIO_PS2);

  if (iCtl_Volume_Boost)
      MenuTick(IDM_VOLUME_BOOST);
  if (iCtl_Vol_BoostCat_Flag[FORMAT_MPA])
      MenuTick(IDM_BOOST_MPA_TRAD);
  if (iCtl_Vol_BoostCat_Flag[FORMAT_MPA_TRENDY])
      MenuTick(IDM_BOOST_MPA_TRENDY);
  if (iCtl_Vol_BoostCat_Flag[FORMAT_AC3])
      MenuTick(IDM_BOOST_AC3);
  if (iCtl_Vol_BoostCat_Flag[FORMAT_DTS])
      MenuTick(IDM_BOOST_DTS);
  if (iCtl_Vol_BoostCat_Flag[FORMAT_LPCM])
      MenuTick(IDM_BOOST_LPCM);

  if (iCtl_Volume_AUTO)
      MenuTick(IDM_VOLUME_AUTO);
  if (iCtl_Volume_Limiting > 0)
      MenuTick(IDM_VOLUME_LIMITING);
  if (iCtl_Volume_SlowAttack > 0)
      MenuTick(IDM_VOLUME_GENTLE);
  if (iCtl_Volume_Retain)
     MenuTick(IDM_VOL_RETAIN);
  if (iCtl_Vol_StarKey)
      MenuTick(IDM_VOL_STARKEY);
  if (iCtl_Track_Memo)
      MenuTick(IDM_TRACK_MEMO);
  if (iCtl_Audio_CRC)
      MenuTick(IDM_AUDIO_CRC);

  if (iCtl_WarnSize_1)
      MenuTick(IDM_WARN_SIZE_1);
  if (iCtl_WarnSize_2)
      MenuTick(IDM_WARN_SIZE_2);
  if (iCtl_WarnSize_3)
      MenuTick(IDM_WARN_SIZE_3);
  if (iCtl_WarnSize_4)
      MenuTick(IDM_WARN_SIZE_4);

  if (iCtl_WarnMpeg1)
      MenuTick(IDM_WARN_MPEG1);
  if (iCtl_WarnTS)
      MenuTick(IDM_WARN_FMT_TS);
  if (iCtl_WarnTSmpg)
      MenuTick(IDM_WARN_FMT_TSMPG);
  if (iCtl_WarnCDXA)
      MenuTick(IDM_WARN_FMT_CDXA);
  if (iCtl_WarnDone)
      MenuTick(IDM_WARN_DONE);
  

  Mpeg_Version_Alerts_Session = 0;
  if (iCtl_WarnBadStart)
      MenuTick(IDM_WARN_BAD_START);
  if (iCtl_WarnBadSysHdr)
      MenuTick(IDM_WARN_BAD_SYSHDR);

  if (iCtl_EDL_AutoSave)
      MenuTick(IDM_EDIT_AUTOSAVE);
  if (iCtl_RecycleBin)
      MenuTick(IDM_DEL2RECYCLE);
  if (iCtl_ToolTips)
      MenuTick(IDM_TOOLTIPS);
  if (iCtl_Trackbar_Big)
      MenuTick(IDM_TRACKBAR_BIG);
 
  Set_Wheel_Scroll(iCtl_Wheel_Scroll);

  Set_Priority(hMain_GUI, iCtl_Priority[0], 0, 1);
  Set_Priority(hZERO,    iCtl_Priority[1], 1, 0);
  Set_Priority(hZERO,    iCtl_Priority[2], 2, 0);

  Set_DropDefault(iCtl_DropAction);
  Set_SortDefault(iCtl_FileSortSeq);

  AC3_CRC_Chk    = iCtl_Audio_CRC;
  AC3_Err_Txt[0] = 0;

  if (iCtl_Ovl_Release)
      MenuTick(IDM_OVL_RELEASE);

  if (iCtl_OVL_FullKey)
      MenuTick(IDM_OVL_FULLKEY);

  if (iCtl_OVL_ATI_Bug)
      MenuTick(IDM_OVL_SIGNAL_ATI);

  iColor_Msg_BG = iCtl_Mask_Colour;
  if (iCtl_Mask_Colour == iColor_Menu_BG) // Overlay key Mid Grey ?
      CheckMenuItem(hMenu, IDM_OVL_MASK_LEADTEK, MF_CHECKED);
  else
  if (iCtl_Mask_Colour == 0) // Overlay key Black ?
      CheckMenuItem(hMenu, IDM_OVL_MASK_LEADTEK_BLK, MF_CHECKED);
  else
      iColor_Msg_BG = iCtl_Back_Colour;

  hBrush_MSG_BG = CreateSolidBrush(iColor_Msg_BG);



  if ( ! iLumEnable_Flag[0])
      MenuTick(IDM_LUMINANCE);
  else
  {
    MenuTick(IDM_LUMINANCE);
    if (iLumGain[0] == 0  &&  iLumOffset[0] == 0 )
    {
        iLumGain[0] = 158; iLumOffset[0] = 0; iLumGamma[0] = 130;
    }
  }

  iLum_Deselected = 0;
  if (iCtl_Lum_Deselector)
      MenuTick(IDM_LUM_DESEL);
  if (iCtl_SAT_Retain)
      MenuTick(IDM_SAT_RETAIN);

  Set_OVL_Notify(-iCtl_VistaOVL_mod);
  if (iCtl_View_Limit2k)
      MenuTick(IDM_VIEW_HUGE);
  iView_Limit2k = iCtl_View_Limit2k;

  Lum_Filter_Init(-1);


  if (VGA_Width == 0)
      VGA_Width = 640 ;

  //iVGA_Avail_Width = VGA_Width - 1;


  Restore_Width = VGA_Width - 160 ;


  //rj_Audio_Code = iAudio_SEL_Track ;

  RdAHD_Flag    = iCtl_AudioAhead;


  //sprintf(szBuffer, "Width = %d  Height = %d ", Main_Width, Main_Height) ;
  //MessageBox(hWnd, szBuffer, "RJ DEBUGS", MB_OK);


  // MPALib_Init(NULL); RJ - POSTPONED UNTIL NEEDED

  byAC3_Init = 0; iMPAdec_Init = 0; iWAV_Init = 0; 
  iPlayAudio = 0; iWantAudio = 1;


  szExtAct_PathType[0] = '*';    szExtAct_PathType[1] = '*';
  szExtAct_PathType[2] = '*';    szExtAct_PathType[3] = '*';
  szExtAct_PathType[4] = '*';    szExtAct_PathType[5] = '*';
  szExtAct_PathType[6] = '='; 
  /*
  szMediaPlayerClassic[0] = '*';
  szVLC[0]                = '*';
  szWinMediaPlayer2[0]    = '*';
  szWinMediaPlayer[0]     = '*';
  szCreativePlayCtr[0]    = '*';

  Reg_ExternalActions();
  */
  if (iCtl_Date_Internationale)
      MenuTick(IDM_DATE_INTERNATIONALE);
  if (iCtl_Readability)
      MenuTick(IDM_READABILITY);


  DSP_ButtonFont_Sizing();    // Scale button size to screen res

  if (iCtl_View_RGB_Always)
  {
     if (iCtl_View_RGB_Always)
      MenuTick(IDM_RGB_ALWAYS);
     Flag2RGB();
  }


  if (iCtl_Zoom_Retain)
      MenuTick(IDM_ZOOM_RETAIN);
  else
      iCtl_Zoom_Wanted = -1;

  Set_Zoom_Menu(iCtl_Zoom_Wanted);


  if (iCtl_Aspect_Retain)
  {
      MenuTick(IDM_ASPECT_RETAIN);
      Set_Aspect_Mode(iView_Aspect_Mode);
  }
  else
      iView_Aspect_Mode = 4;


}


//----------------------------

void  Set_Parse_Ticks()
{
  unsigned int uTmp1, uTmp2;

  if (iCtl_Out_Parse)
      MenuTick(IDM_OUT_PARSE);
  if (iCtl_Out_PTS_Match)
      MenuTick(IDM_OUT_PTS_MATCH);   // Audio Matching

  uTmp1 = MF_UNCHECKED; uTmp2 = MF_UNCHECKED;
  if (iCtl_SetBrokenGop > 0)
    uTmp1 = MF_CHECKED;
  else
  if (iCtl_SetBrokenGop < 0)
    uTmp2 = MF_CHECKED;

  CheckMenuItem(hMenu, IDM_OUT_BROKEN_FLAG, uTmp1);
  CheckMenuItem(hMenu, IDM_OUT_BROKEN_CLR,  uTmp2);

  if (iCtl_Out_Align_Video)
      MenuTick(IDM_OUT_ALIGN_VIDEO);
  if (iCtl_Out_Align_Audio)
      MenuTick(IDM_OUT_ALIGN_AUDIO);

  if (iCtl_Out_TC_Adjust)
      MenuTick(IDM_OUT_TC_ADJUST);
  if (iCtl_Out_TC_Force)
      MenuTick(IDM_OUT_TC_FORCE);

  if (iCtl_Out_Parse_AllPkts)
      MenuTick(IDM_OUT_PARSE_ALL_PKTS);
  if (iCtl_Out_Parse_Deep)
      MenuTick(IDM_OUT_DEEP );

  if (iCtl_Out_KillPadding)
      MenuTick(IDM_OUT_KILLPAD);
}




//---------------------------------------------------------------
// Save controls to INI file
void INI_SAVE()
{
  const int iDummy_TRUE  = 1;
  const int iDummy_FALSE = 0;
  int iTmp1;

  // iCtl_View_Fast_YUV = 0;
  GetWindowRect(hWnd_MAIN, &wrect);

  INIFile = fopen(szINI_Path, "w+") ;
  if (INIFile == NULL)
  {
     if (DBGflag) DBGout("INI file FAILED to Output");
  }
  else
  { 
    fprintf(INIFile, "INI_Version=%d\n", INI_Version);
    fprintf(INIFile, "Window_Position=%d,%d\n",
                              wrect.left, wrect.top);
    fprintf(INIFile, "iDCT_Algorithm=%d\n",  MParse.iDCT_Flag);
    fprintf(INIFile, "YUVRGB_Scale=%d\n",    MParse.PC_Range_Flag);
    fprintf(INIFile, "Field_Operation=%d,%d\n", MParse.FO_Flag,
                                               (1-iCtl_View_Fast_YUV));
    if (iCtl_Track_Memo)
        iAudio_SEL_Track = TRACK_AUTO;
    fprintf(INIFile, "Track_Number=%d\n",  iAudio_SEL_Track);

    iWant_Aud_Format = FORMAT_AUTO; // Suppressed saving - Too annoying !
    fprintf(INIFile, "Channel_Format=%d\n", iWant_Aud_Format);
    //fprintf(INIFile, "AC3=%d\n", AC3_Flag);
    //fprintf(INIFile, "DR_Control=%d\n", AC3_DRC_FLag);
    fprintf(INIFile, "DS_Downmix=%d\n", AC3_DSDown_Flag);

    //fprintf(INIFile, "MPA=%d\n", MPA_Flag);
    //fprintf(INIFile, "SRC_Precision=%d\n", SRC_Flag);
    //fprintf(INIFile, "Norm_Ratio=%d\n", 100 * Normalization_Flag + Norm_Ratio);
    fprintf(INIFile, "Process_Priority=%d\n", iCtl_Priority[0]);
    fprintf(INIFile, "Luminance=%d,%d,%d,%d,%d,%d\n",
                iLumGain[0], iLumOffset[0], iLumGamma[0],
                iLumEnable_Flag[0], (iLumLock_Flag + (iSatLock_Flag*256)),  
                iLumEnable_Flag[1]);
    fprintf(INIFile, "Keyboard=%d,%d\n", (iCtl_KB_NavOpt<<1), (iCtl_KB_MarkOpt<<1));
    if (szMPAdec_NAME[0] <= ' ')
        strcpy(szMPAdec_NAME, "MPAlib.dll");
    fprintf(INIFile, "Audio_Decoder=%s\n",      szMPAdec_NAME);
    fprintf(INIFile, "AddAuto=%d,%d\n",         Add_Automation, iCtl_To_Pad);
    fprintf(INIFile, "Preamble=%d\n",           iCtl_Out_Preamble_Flag);
    fprintf(INIFile, "Deinterlace=%d,%d,%d\n",  Deint_AUTO_View,
                                                Deint_VOB, Deint_SNAP);

    fprintf(INIFile, "OutXtn=%s\n",         szOut_Xtn_RULE);
    if (szInput[0]  <= ' ')
        strcpy(szInput, ".");
    fprintf(INIFile, "InputFile=\"%s\"\n",  szInput);

    if (szOutput[0] <= ' ')
        strcpy(szOutput, ".");
    fprintf(INIFile, "OutputFile=\"%s\"\n", szOutput);

    if (szCtl_OutFolder[0] <= ' ')
        strcpy(szCtl_OutFolder, ".");
    fprintf(INIFile, "OutFolder=%d,\"%s\"\n", iCtl_OutFolder_Active,
                                             szCtl_OutFolder);


    iCtl_Warn_Fmts = iCtl_WarnETC; 
     // debitulate
    if (iCtl_WarnMpeg1)
        iCtl_Warn_Fmts |= 0x01;
    if (iCtl_WarnTS)
        iCtl_Warn_Fmts |= 0x02;
    if (iCtl_WarnTSmpg)
        iCtl_Warn_Fmts |= 0x04; // TS named .mpg
    if (iCtl_WarnCDXA)
        iCtl_Warn_Fmts |= 0x08; 
    if (iCtl_WarnDone)
        iCtl_Warn_Fmts |= 0x10; 


    fprintf(INIFile, K_DefaultY_FORMAT,
                     iCtl_OutFolder_Both,  iCtl_Audio_CRC,
                     iCtl_Out_PTS_Match,    iCtl_Out_Align_Video,
                     iCtl_Out_Preamble_VTS, iCtl_Play_AudLock,
                     iCTL_FastBack,            iCtl_Track_Memo, 
                     iCtl_WarnSize_1,       iCtl_WarnSize_2,
                     iCtl_WarnSize_3,       iCtl_WarnSize_4,
                     iCtl_KB_NavStopPlay,   iCtl_Ovl_Release,
                     iCtl_SetBrokenGop,     iCtl_Warn_Fmts, 
                     iCtl_Time_Fmt,         
                   ((1-iCtl_WarnBadSysHdr)*256 + iCtl_WarnBadStart),
                     iCtl_EDL_AutoSave,     iCtl_RecycleBin, 
                     iCtl_View_Centre_Crop);

    iCtl_Out_Parse_Extras = iCtl_Out_Parse_Deep * 256 
                          + iCtl_Out_Parse_AllPkts;

    fprintf(INIFile, K_PatchHdr_FORMAT,
                     iCtl_Out_Fix_SD_Hdr,   iCtl_Out_Parse,
                     iCtl_Out_Seq_End,      iCtl_VOB_Style,
                     iCtl_Out_TC_Adjust,    iCtl_Out_Align_Audio,
                     iCtl_Out_Parse_Extras, iCtl_Out_DeBlank,
                     iCtl_Out_MixedCase,    iCtl_Out_KeepFileDate, 
                     iCtl_Out_TC_Force,     iCtl_ParmConfirm,  
                     iCtl_BMP_Aspect,       iCtl_View_Aspect_Mpeg1_Force, 
                     iCtl_Out_SysHdr_Mpeg,  iCtl_Out_Fix_Errors);

    fprintf(INIFile, K_AudioX_FORMAT,
                     iCtl_Volume_Boost, iCtl_Audio_PS2,
                     iCtl_Play_Sync,    iCtl_Drop_Behind,
                     iCtl_Play_Info,    iCtl_Drop_PTS,
                     iCtl_Priority[1],  iCtl_Priority[2],
                     iCtl_DropAction,   iCtl_Play_Summary, 
                     iCtl_Name_Info,    iCtl_Date_Internationale, 
                     iDummy_FALSE, // iCtl_BasicName_Panel, 
                     iCtl_ColumnWidth[0], iCtl_ColumnWidth[1],
                     iCtl_ColumnWidth[2], iCtl_ColumnWidth[3],
                     iCtl_ColumnWidth[4], iCtl_ColumnWidth[5],
                     iCtl_AudioDecoder, 
                     iCtl_View_RGB_Always);

    fprintf(INIFile, "JumpSpan=%d,%d,%d,%d,%d,%d\n",  
                               iJumpSecs[0],  iJumpSecs[1],  iJumpSecs[2], 
                               iJumpSecs[3],  iJumpSecs[4],  iJumpSecs[5]); 

    if (szCtl_Out_ProcLine_A[0] <= ' ')
        strcpy(szCtl_Out_ProcLine_A, ".");
    if (szCtl_Out_ProcLine_B[0] <= ' ')
        strcpy(szCtl_Out_ProcLine_B, ".");

    ParmTidy(&szCtl_Out_ProcLine_A[0], 1);
    ParmTidy(&szCtl_Out_ProcLine_B[0], 1);

    fprintf(INIFile, "Proc0A=%d,\"%s\"\n", iCtl_Out_PostProc,
                                          szCtl_Out_ProcLine_A);
    fprintf(INIFile, "Proc0B=%d,\"%s\"\n", iCtl_Out_PostQuote,
                                          szCtl_Out_ProcLine_B);

    if (szCtl_BMP_Folder[0] <= ' ')
        strcpy(szCtl_BMP_Folder, ".");
    fprintf(INIFile, "BMPFolder=%d,\"%s\"\n", iCtl_BMP_Folder_Active,
                                             szCtl_BMP_Folder);


    if (szRenamePlugIn_Name[0] <= ' ')
        strcpy(szRenamePlugIn_Name, ".");

    if (cRenamePlugIn_AsyncMode)
        cRenamePlugIn_MultiMode++;

    fprintf(INIFile, "RenamePlugIn=%d,%c,\"%s\"\n", 
                                              PlugFileRename.iActive,
                                              cRenamePlugIn_MultiMode,
                                             szRenamePlugIn_Name);


    if (szFileListPlugIn_Name[0] <= ' ')
        strcpy(szFileListPlugIn_Name, ".");
    if (cFileListPlugIn_AsyncMode)
        cFileListPlugIn_MultiMode++;
    fprintf(INIFile, "FileListPlugIn=%d,%c,\"%s\"\n", 
                                              PlugFileList.iActive,
                                              cFileListPlugIn_MultiMode,
                                             szFileListPlugIn_Name);

    fprintf(INIFile, "Breathe=(%d,%d,%d),Pkts=(%d,%d,%d)\n",
                                          iCtl_Out_Breathe_PerBigBlk[0],
                                          iCtl_Out_Breathe_PerBigBlk[1],
                                          iCtl_Out_Breathe_PerBigBlk[2],
                                          iCtl_Out_Breathe_PktLim[0],
                                          iCtl_Out_Breathe_PktLim[1],
                                          iCtl_Out_Breathe_PktLim[2]
                                          );

    // LeadTek Black only works for current session
    // so reset to LeadTek Grey on exit.
    if (iCtl_Mask_Colour     == 0    // Overlay key was Black ?
    &&  iCtl_Mask_Fallback   == 1)
        iCtl_Mask_Colour = iColor_Menu_BG;

    fprintf(INIFile, "Color=x%06X,BG=x%06X,MASK=x%06X\n",
                       iCtl_Text_Colour, iCtl_Back_Colour, iCtl_Mask_Colour);

    if (szEDLname[0] <= ' ')
        strcpy(szEDLname, "*.EDL");
    fprintf(INIFile, "EDL=\"%s\"\n", szEDLname);

    fprintf(INIFile, "LumBMP=%d,%d,%d,%d,%d,%d\n",
                           iLumGain[1], iLumOffset[1],iLumGamma[1],
                           iLumEnable_Flag[1], 0, 0);

    fprintf(INIFile, "OutXtnAudio=%s\n",         szOut_Xtn_AUD);

    fprintf(INIFile, K_Y2, 
                              iCtl_View_Fast_RGB,  iCtl_Readability,
                              iCtl_Copy_BufSz_Ix,  iCtl_ToolTips,
                              iCtl_Trackbar_Big,   iCtl_Wheel_Scroll,
                              iCtl_FileSortSeq,    iCtl_Out_SysHdr_EveryClip,
                              iCtl_Out_SysHdr_Unlock,
                              iCtl_View_Limit2k,   iCtl_BMP_Preview,
                              iDummy_TRUE,         iCtl_Out_PostShow, 
                              iDummy_TRUE,
                              iDummy_TRUE, iDummy_TRUE);    

    fprintf(INIFile, K_N2, 
                            ((iCtl_F5_Toggler*256)+iCtl_F3_Names), 
                            iCtl_ParmClipSpec, 
                            iCtl_Out_Force_Interlace, iCtl_Out_KillPadding,
                            iCtl_VistaOVL_mod, iCtl_Volume_AUTO,
                            iCtl_OVL_FullKey,  
                            (AC3_DRC_FLag - 3),
                            iCtl_Zoom_Retain, (iCtl_Zoom_Wanted + 1),
                            iCtl_Aspect_Retain, (iView_Aspect_Mode - 4),
                            iCtl_Out_SplitSegments, 
                            iCtl_YV12, iCtl_AspMismatch,
                            iCtl_CropTop);

    fprintf(INIFile, K_LumPresets,
                           iLumGain[2], iLumOffset[2], iLumGamma[2],  // Default
                           iLumGain[3], iLumOffset[3], iLumGamma[3],  // Bold
                           0, 0, 0,      
                           iLumGain[4], iLumOffset[4], iLumGamma[4],  // C
                           iLumGain[5], iLumOffset[5], iLumGamma[5],  // A
                           0, iCtl_SinThreshold, iCtl_VHS_Threshold);

    fprintf(INIFile, K_N3, 
                           (iCtl_View_Aspect_Adjust - 100), 
                            iSatAdd_U[0], iSatAdd_V[0], (iSatGain[0] - 100),
                          ((iCtl_SAT_Retain * 256) + iSatAdj_Flag),
                            iCtl_Lum_Deselector,
                            iCtl_OutPartAuto,  iCtl_OVL_ATI_Bug,
                           (iCtl_NotSoFast+1), 
                            uAud_PID_All, iCtl_KB_V_Popup,
                            iDummy_FALSE, 
                            iDummy_FALSE, iDummy_FALSE,
                           (257 - iCtl_ViewToolbar[0]), 
                           (257 - iCtl_ViewToolbar[1]));

    // Rebitulate the Boost Flags
    if (iCtl_Vol_BoostCat_Flag[0]) iTmp1 =   1;
    else                           iTmp1 =   0;
    if (iCtl_Vol_BoostCat_Flag[1]) iTmp1 +=  2;
    if (iCtl_Vol_BoostCat_Flag[2]) iTmp1 +=  4;
    if (iCtl_Vol_BoostCat_Flag[3]) iTmp1 +=  8;
    if (iCtl_Vol_BoostCat_Flag[4]) iTmp1 += 16;
    if (iCtl_Vol_BoostCat_Flag[5]) iTmp1 += 32;
    if (iCtl_Vol_BoostCat_Flag[6]) iTmp1 += 64;
    if (iCtl_Vol_BoostCat_Flag[7]) iTmp1 +=128;

    if (iCtl_Volume_Retain)
    {
      //memcpy(&iCtl_Vol_BoostCat_Init, &iVol_BoostCat_Done,
      //                                    sizeof(iCtl_Vol_BoostCat_Init));
      
        iCtl_Vol_BoostCat_Init[FORMAT_MPA] = 
            iVol_BoostCat_Done[FORMAT_MPA];
        iCtl_Vol_BoostCat_Init[FORMAT_MPA_TRENDY] = 
            iVol_BoostCat_Done[FORMAT_MPA_TRENDY];
        iCtl_Vol_BoostCat_Init[FORMAT_AC3] =  
            iVol_BoostCat_Done[FORMAT_AC3]; 
        iCtl_Vol_BoostCat_Init[FORMAT_LPCM] = 
            iVol_BoostCat_Done[FORMAT_LPCM];
        iCtl_Vol_BoostCat_Init[FORMAT_DTS] =   
            iVol_BoostCat_Done[FORMAT_DTS];  
      
    }

    fprintf(INIFile, K_VOLUME, 
                            iCtl_Vol_BoostCat_Init[FORMAT_MPA],
                            iCtl_Vol_BoostCat_Init[FORMAT_MPA_TRENDY],
                            iCtl_Vol_BoostCat_Init[FORMAT_AC3], 
                            iCtl_Volume_Limiting, iCtl_Volume_Ceiling,                            
                            iCtl_Volume_SlowAttack, 
                            iCtl_Vol_StarKey,
                            iCtl_Vol_BoostCat_Init[FORMAT_LPCM],
                            iCtl_Vol_BoostCat_Init[FORMAT_DTS],  
                            iTmp1,
                            (1-iCtl_Volume_Retain), 
                            iDummy_FALSE,
                            iDummy_FALSE,  iDummy_FALSE,
                            K_BOOST_DENOM, iCtl_Audio_InitBreathe);

    fclose(INIFile);
  }

}



//----------------------------------------------------------------

char *lpWarnText, *lpWarnTitle;
int  *lpWarnCtl;

unsigned int uWarn_ID, uMode;
int iWarnDlgStatus;


LRESULT CALLBACK Warning_Dialog(HWND hWarn,  UINT message,
                                WPARAM wParam, LPARAM lParam)
{

  int iEnough, iRC;
  unsigned int uMark;
  DWORD wmId; //, wmEvent;

  iEnough = 0;

  switch (message)
  {
     case WM_INITDIALOG:

         // Static Text box honours Carriage Return !   YAY !
         SetDlgItemText(hWarn,  WARN_TEXT3, lpWarnText); 

         if (lpWarnTitle)
           if (*lpWarnTitle)
               SetWindowText(hWarn, lpWarnTitle);

         if (uMode == MB_YESNOCANCEL)
         {
             SendDlgItemMessage(hWarn, IDNO, WM_ENABLE, 1, 0);	
             SendDlgItemMessage(hWarn, IDNO, BM_SETSTYLE, (WPARAM) LOWORD(BS_PUSHBUTTON), 1);	
         }
         
         ShowWindow(hWarn, SW_SHOWNORMAL);

         iEnough = 1;
         iWarnDlgStatus = 1;
         break;



     case WM_COMMAND:
        switch (LOWORD(wParam))
        {
          case IDOK:
               iRC = IDOK;
               iEnough = 2;
               break;

          case IDNO:
               iRC = IDNO;
               iEnough = 2;
               break;

          case IDCANCEL:
          case IDM_EXIT:
          //case WM_DESTROY:
               iRC = IDCANCEL;
               iEnough = 2;
               break;

          case WARN_NEVER:

            if (SendDlgItemMessage(hWarn,
                      WARN_NEVER, BM_GETCHECK, 1, 0) == BST_CHECKED)
            {
                *(char*)(lpWarnCtl) = 0;
                 uMark = MF_UNCHECKED;
            }
            else
            {
                *(char*)(lpWarnCtl) = 0;
                 uMark = MF_CHECKED;
            }

            if (uWarn_ID)
                CheckMenuItem(hMenu, uWarn_ID, uMark);

            break;

         }

         break;

               // SYSTEM MENU
      case WM_SYSCOMMAND:

         //Calc_PhysView_Size();    // From SYSCOMMAND
         wmId     = wParam & 0xFFF0 ;

         if (wmId == SC_MINIMIZE)
         {
             // Umm.... ???
         }
         break;


     case WM_DESTROY:
     case IDM_EXIT:
               iRC = IDCANCEL;
               iEnough = 2;
               break;
  }

  // KILL ?
  if (iEnough > 1 && iWarnDlgStatus)
  {
      iWarnDlgStatus = 0; // Avoid recursion
      //DestroyWindow(hDialog);
      EndDialog(hWarn, iRC);
  }

  return (DefWindowProc(hWarn, message, wParam, lParam));

}


// Display a message box with the option to suppress in future
int Warning_Box(char *lpP_Text, 
                char *lpP_Title, 
                int  *lpP_WarnCtl, unsigned int P_Warn_ID,
                unsigned int P_Mode)
{
  int iRC;

  lpWarnText  =  lpP_Text;
  lpWarnTitle =  lpP_Title;
  lpWarnCtl   =  lpP_WarnCtl;
  uWarn_ID    =    P_Warn_ID;
  uMode       =    P_Mode;

  iRC = DialogBox(hInst,     (LPCTSTR)IDD_WARNING,
                  hWnd_MAIN, (DLGPROC)Warning_Dialog);

  SetForegroundWindow(hWnd_MAIN);
  SetFocus(hWnd_MAIN);

  return iRC;

}


