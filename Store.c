
#define DBG_RJ


/*
 *  MPEG2DEC - Copyright (C) Mpeg Software Simulation Group 1996-99
 *  DVD2AVI  - Copyright (C) Chia-chen Kuo - April 2001
 *  Mpg2Cut2 - Various Authors
 *
 *  DVD2AVI - a free MPEG-2 converter
 *
 *  DVD2AVI is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  DVD2AVI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include "global.h"
#include <math.h>
#include <sys/timeb.h>
#include "wave_out.h"
#include <vfw.h>
#include "PIC_BUF.h"


#define true 1
#define false 0

#define MAX_AVI_SIZE  2073600000

// __forceinline static void Store_RGB24(unsigned char *src[], int); //DWORD frame);

static void conv420to422(unsigned char *src, unsigned char *dst);
static void conv422to444(unsigned char *src, unsigned char *dst);
static void conv444toRGB24odd(unsigned char *py, unsigned char *pu, unsigned char *pv, unsigned char *dst);
static void conv444toRGB24even(unsigned char *py, unsigned char *pu, unsigned char *pv, unsigned char *dst);
static void Flush_RGB24();
//static void conv422toyuy2odd(unsigned char *py, unsigned char *pu, unsigned char *pv, unsigned char *dst);
static void Flush_YUY2(void);
static void AVIKill(void);
void SEQ_INIT();
void First_Frame_INIT();


//static int gcd(int x, int y);

PAVIFILE pfile;
PAVISTREAM ps_AviStream, psCompressed;
AVICOMPRESSOPTIONS opts;
LPAVICOMPRESSOPTIONS lpopts = &opts;
AVISTREAMINFO strinfo;
ICCOMPRESSFRAMES iccf;
COMPVARS compvars;
PCOMPVARS pcompvars = &compvars;

static char VideoOut[_MAX_PATH];
static int AVI_Init, avi_size, avi_count, frame_count;

static int Top_Field_First, Rpt_First_Field;

static int frame_size;

static int Resize_Flag;
static int PROGRESSIVE_HEIGHT, INTERLACED_HEIGHT;
static int RGB_DOWN1, RGB_DOWN2;
static int NINE_CLIP_WIDTH, QUAD_CLIP_WIDTH ;
static int HALF_CLIP_WIDTH;
static int CLIP_STEP, CLIP_HALF_STEP;

//__int64 LumOffsetMask, LumGainMask, LumGammaMask;  //old LumAdjBuf filter

static int  Tulebox_current_frame_count;



static const __int64 mmmask_0001 = 0x0001000100010001;
static const __int64 mmmask_0002 = 0x0002000200020002;
static const __int64 mmmask_0003 = 0x0003000300030003;
static const __int64 mmmask_0004 = 0x0004000400040004;
static const __int64 mmmask_0005 = 0x0005000500050005;
static const __int64 mmmask_0007 = 0x0007000700070007;
static const __int64 mmmask_0064 = 0x0040004000400040;
static const __int64 mmmask_0128 = 0x0080008000800080;



//---------------------------------------------------------------
void RefreshVideoFrame()
{

  Lum_Filter_Init(iColorSpaceTab);

  if (File_Limit        && MParse.SeqHdr_Found_Flag
  &&  (MParse.Stop_Flag || MParse.Pause_Flag ))
  {
       if (MParse.iColorMode == STORE_YUY2)
       {
           if (iLumEnable_Flag[0] && ! iView_Fast_YUV)
           {
               Lum_Filter_OVL(y_orig, LumAdjBuf);
               y444 = LumAdjBuf;
           }
           else
               y444 = y_orig;

           RenderYUY2(1);
       }
       else
       {
         //RenderRGB24();        //  WHY DID I CHANGE THIS ??????
         if (WaitForSingleObject(hThread_MPEG, 0)==WAIT_OBJECT_0)
         {
             MParse.Fault_Flag = false; 
             iShowVideo_Flag = iCtl_ShowVideo_Flag;
             Pic_Started = true;
             Write_Frame(bwd_ref_frame, d2v_bwd, 0);
         }
       }
  }
  return;

}


//---------------------------------------------------------------

void Write_Frame(unsigned char *src[], D2VData d2v, DWORD frame)
{
  int repeat, iStored ; //, iTmp3, iTmp4 ;

#ifdef DBG_RJ
  if (DBGflag)
  {
      sprintf(szDBGln, "WRITE Mode=%d OVLFLAG=%d",
                 DDOverlay_Flag,
                (MParse.iColorMode==STORE_YUY2)
                           );
      DBGout(szDBGln);
  }
#endif

  process.BlksSinceLastEffect = 0;

  if (MParse.Fault_Flag)
  {
    if (MParse.Fault_Flag < CRITICAL_ERROR_LEVEL)
    {
       if (! MParse.Stop_Flag && !iAudioDBG)
       {
          sprintf(szMPG_ErrTxt, "Vid ERR %d.%d.x%04X ",
                   MParse.Fault_Prev, MParse.Fault_Flag, MParse.Fault_More);
          SetDlgItemText(hStats, IDC_INFO, szMPG_ErrTxt);
       }

       MParse.Fault_Flag = 0;   // fault tolerance
       MParse.Fault_More = 0;   // fault tolerance
    }
    else
    {
/*      if (AVI_Flag)
          AVIKill();*/
      Mpeg_KILL(1010);
    }
  }

  if (Pic_Started)
  {
     ScanMode_code   = d2v.Progressive_Format;
     Top_Field_First = d2v.Fld1_Top_Rpt >>1;
     Rpt_First_Field = d2v.Fld1_Top_Rpt & 0x01;
  }

  if (!frame)
     First_Frame_INIT();
  else
  if (MParse.ReInit)
      SEQ_INIT();




  iStored = 0;
  if (Pic_Started)
  {
     repeat = DetectVideoType(frame, d2v.Fld1_Top_Rpt);

     if (MParse.FO_Flag != FO_FILM || repeat)
     {
       if (MParse.iColorMode==STORE_YUY2)
         Store_YUY2a(src); //, frame);
       else
         Store_RGB24(src, 0); //, frame);

       iStored = 1;
     }

     if (MParse.FO_Flag == FO_FILM && repeat == 2)
     {
       if (MParse.iColorMode == STORE_YUY2)
         Store_YUY2a(src); //, frame);
       else
         Store_RGB24(src, 0); //, frame);

       iStored = 1;
     }
  }

  // URGH - THIS NEXT BIT DOESN'T TRIGGER WHEN PLAYING Field-encoded Pics
  //        because second field may NOT be I_TYPE !

  if ((MPEG_Pic_Type == I_TYPE && frame > 0) // (frame & 15) == 15)
  ||  process.Action == ACTION_FWD_FRAME)
  {
      sprintf(szBuffer, "%d", frame+1);
      SetDlgItemText(hStats, IDC_CODED_NUMBER, szBuffer);

      sprintf(szBuffer, "%d", PlayCtl.iPlayed_Frames);
      SetDlgItemText(hStats, IDC_PLAYBACK_NUMBER, szBuffer);

/*    if (AVI_Flag)
      {
         sprintf(szBuffer, "%d", avi_count-1);
         SetDlgItemText(hStats, IDC_FILE, szBuffer);

         sprintf(szBuffer, "%d MB", avi_size>>20);
         SetDlgItemText(hStats, IDC_FILE_SIZE, szBuffer);
      }
*/
      Stats_FPS();
  }

  /*  if (AVI_Flag && avi_size>=MAX_AVI_SIZE)
    AVIKill();*/

  
  Pic_Started = 0;

  Tulebox_current_frame_count = Frame_Number;
  if(Tulebox_current_frame_count != MParse.Tulebox_prev_frame_number) 
  {
     MParse.Tulebox_prev_frame_number = Tulebox_current_frame_count;
     if (PlayCtl.iStopNextFrame &&  iStored)
     {    
        B555_Normal_Speed(0);
        PlayCtl.iStopNextFrame++;
        if (PlayCtl.iStopNextFrame > 2)
        {
           //process.Action = ACTION_FWD_GOP; 
           MParse.Stop_Flag = 1;
        }
     }
     else
     if  (MParse.Tulebox_SingleStep_flag)
     {
        // SingleStep mods by Tulebox (See EZOARD posting "Frame by frame stepping" Sep 04 2005)

        //if(Single_Step_Flag) 
        {
            T100_Upd_Posn_Info(1);  // RJ update info   
            DSP3_Main_TIME_INFO();
            if (MParse.ShowStats_Flag) 
            {
                S100_Stats_Hdr_Main(0);
                S200_Stats_Pic_Main(1);
            }

           //GUI_SuspendThread();
           MParse.Pause_Flag = 1;
           SuspendThread(hThread_MPEG);
        }
     }

  }

 
}




//----------------------------------

void SEQ_INIT()
{
  if (DBGflag)
  {
    DBGout("SEQ_INIT");
  }

  MParse.ReInit = 0;

  Resize_Flag = 0;

    Clip_Width  = Resize_Width  = Coded_Pic_Width;
    Clip_Height = Resize_Height = Coded_Pic_Height;

    CLIP_AREA = HALF_CLIP_AREA = CLIP_STEP = CLIP_HALF_STEP = 0;


    /* if (ClipResize_Flag)
    {
      if (Clip_Top || Clip_Bottom || Clip_Left || Clip_Right)
      {
        Clip_Width -= Clip_Left+Clip_Right;
        Clip_Height -= Clip_Top+Clip_Bottom;
        Resize_Width = Clip_Width;
        Resize_Height = Clip_Height;

        CLIP_AREA = Coded_Pic_Width * Clip_Top;
        HALF_CLIP_AREA = (Coded_Pic_Width>>1) * Clip_Top;

        CLIP_STEP = Coded_Pic_Width * Clip_Top + Clip_Left;
        CLIP_HALF_STEP = (Coded_Pic_Width>>1) * Clip_Top + (Clip_Left>>1);
      }

      if (Squeeze_Width || Squeeze_Height)
      {
        Resize_Flag = 1;
        Resize_Width -= Squeeze_Width;
        Resize_Height -= Squeeze_Height;
      }
    }*/


    NINE_CLIP_WIDTH   = Clip_Width * 9;
    QUAD_CLIP_WIDTH   = Clip_Width<<2;
    DOUBLE_CLIP_WIDTH = Clip_Width<<1;
    HALF_CLIP_WIDTH   = Clip_Width>>1;

    HALF_WIDTH_D8 = (Coded_Pic_Width>>1) - 8;
    PROGRESSIVE_HEIGHT = (Coded_Pic_Height>>1) - 2;
    INTERLACED_HEIGHT  = (Coded_Pic_Height>>2) - 2;
    RGB_DOWN1 = Clip_Width * (Clip_Height - 1) * 3;
    RGB_DOWN2 = Clip_Width * (Clip_Height - 2) * 3;
}




//----------------------------------

void First_Frame_INIT()
{
  if (DBGflag)
  {
    DBGout("1st_INIT");
  }

  // int i;
  AVI_Init = 1;

    //if (process.Action   != ACTION_FWD_FRAME)
    //  Top_Field_Built = Bot_Field_Built = 0;

    //PlayCtl.iCHK_Frames = 0;   PlayCtl.iPlayed_Frames = 0;
    //PlayCtl.iShown_Frames = 0; PlayCtl.iDropped_Frames = 0;

    frame_size = 0; avi_size = 0;
    avi_count = 1; frame_count = 0;

    SEQ_INIT();

    //LumGainMask = ((__int64)LumGain<<48) + ((__int64)LumGain<<32) + ((__int64)LumGain<<16) + (__int64)LumGain;
    //LumOffsetMask = ((__int64)LumOffset<<48) + ((__int64)LumOffset<<32) + ((__int64)LumOffset<<16) + (__int64)LumOffset;


    /*

    //ext = strrchr(File_Name[File_Flag], '\\');
    //strncat(szBuffer, ext+1, strlen(File_Name[0])-(int)(ext-File_Name[0]));

    sprintf(szTemp, " %d/%d -", File_Ctr+1, File_Limit);
    strcat(szBuffer, szTemp);

    sprintf(szTemp, " %d x %d  ", Resize_Width, Resize_Height);
    strcat(szBuffer, szTemp);

    //  Are these Zoom settings even implemeneted anymore ?

    if (Resize_Width%32 == 0)
      strcat(szBuffer, " [32x]");
    else if (Resize_Width%16 == 0)
      strcat(szBuffer, " [16x]");
    else if (Resize_Width%8 == 0)
      strcat(szBuffer, " [8x]");
    else
      strcat(szBuffer, " [ ]");

    if (Resize_Height%32 == 0)
      strcat(szBuffer, "[32x]");
    else if (Resize_Height%16 == 0)
      strcat(szBuffer, "[16x]");
    else if (Resize_Height%8 == 0)
      strcat(szBuffer, "[8x]");
    else
      strcat(szBuffer, "[ ]");

    // SetWindowText(hWnd, szBuffer);

*/

    if ((   iAspect_Width  != Prev_Clip_Width 
         || iAspect_Height != Prev_Clip_Height)
    &&      ! MParse.iMultiRqst
    && iShowVideo_Flag)
    {
         D500_ResizeMainWindow(iAspect_Width, iAspect_Height, 1);
         Prev_Clip_Width  = iAspect_Width;
         Prev_Clip_Height = iAspect_Height;
    }


    Render_Init();

    if (process.Action == ACTION_RIP
    &&  MParse.FO_Flag == FO_SWAP
    &&  Pic_Started)
    {
      if (Top_Field_First)
        Bot_Field_Built = 1;
      else
        Top_Field_Built = 1;
    }

    if (MParse.FO_Flag != FO_FILM)
    {
      if ((Top_Field_First && MParse.FO_Flag!=FO_SWAP)
      || (!Top_Field_First && MParse.FO_Flag==FO_SWAP))
          SetDlgItemText(hStats, IDC_INFO, "Top");  // Top Field
      else
          SetDlgItemText(hStats, IDC_INFO, "Bot");  // Bottom Field
    }

}




//-----------------------------------------------------------
int DetectVideoType(int frame, int Fld1_Top_Rpt)
{
  static int Old_TRF, Repeat_On, Repeat_Off, Repeat_Init;

  if (frame)
  {
    if ((Fld1_Top_Rpt & 3) == ((Old_TRF+1) & 3))
      FILM_Purity++;
    else
      NTSC_Purity++;
  }
  else
    Video_Type = FILM_Purity = NTSC_Purity = 0;
    Repeat_On  = Repeat_Off  = Repeat_Init = 0;// INDENTED BUT NOT BRACKETED

  Old_TRF = Fld1_Top_Rpt;

  if (Fld1_Top_Rpt & 1)
    Repeat_On ++;
  else
    Repeat_Off ++;

  if (Repeat_Init)
  {
    if (Repeat_Off-Repeat_On == 5)
    {
      Repeat_Off = Repeat_On = 0;
      return 0;
    }
    else if (Repeat_On-Repeat_Off == 5)
    {
      Repeat_Off = Repeat_On = 0;
      return 2;
    }
  }
  else
  {
    if (Repeat_Off-Repeat_On == 3)
    {
      Repeat_Off = Repeat_On = 0;
      Repeat_Init = 1;
      return 0;
    }
    else if (Repeat_On-Repeat_Off == 3)
    {
      Repeat_Off = Repeat_On = 0;
      Repeat_Init = 1;
      return 2;
    }
  }

  return 1;
}



//-------------------------------------------------------------

// Change data format to RBG24

//static
void Store_RGB24(unsigned char *src[], int P_SnapOnly)  //, DWORD frame)
{

unsigned char *lpSaveY, *lpSaveU, *lpSaveV; 

  if (! src  || HALF_WIDTH < 8 || HALF_WIDTH > 2048)  // Check for messy stop situation
    return;

/*  if (AVI_Flag && AVI_Init)
  {
    if (!frame)
    {
      compvars.cbSize = sizeof(compvars);

      if (!ICCompressorChoose(hWnd, ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME,
        lpbirgb, NULL, pcompvars, NULL))
        Mpeg_KILL();

      // set AVI header
      ZeroMemory(&strinfo, sizeof(AVISTREAMINFO));
      strinfo.fccType         = streamtypeVIDEO;
      strinfo.fccHandler        = compvars.fccHandler;
      strinfo.dwQuality       = -1;
      strinfo.dwSuggestedBufferSize = birgb.biSizeImage;
      strinfo.dwScale = 1000;
      strinfo.dwRate = (unsigned int)((MParse.FO_Flag==FO_FILM) ? frame_rate*800 : frame_rate*1000);

      // set AVI save options
      opts.cbFormat = sizeof(birgb);
      opts.fccType = streamtypeVIDEO;
      opts.fccHandler = compvars.fccHandler;
      opts.dwKeyFrameEvery = compvars.lKey;
      opts.dwQuality = compvars.lQ;
      opts.dwBytesPerSecond = compvars.lDataRate<<10;
      opts.dwFlags = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES | AVICOMPRESSF_DATARATE;
      opts.dwInterleaveEvery = 0;
      opts.lpFormat = lpbirgb;
      opts.cbParms = compvars.cbState;
      opts.lpParms = compvars.lpState;
    }

    AVI_Init = 0; avi_size = 0;
    AVIFileInit();

    sprintf(VideoOut, "%s.%02d.avi", szOutput, avi_count++);

    if (AVIFileOpen(&pfile, VideoOut, OF_WRITE | OF_CREATE, NULL) != AVIERR_OK)
    {
      AVIKill();
      Mpeg_KILL();
    }

    if (AVIFileCreateStream(pfile, &ps_AviStream, &strinfo) != AVIERR_OK)
    {
      AVIKill();
      Mpeg_KILL();
    }

    if (AVIMakeCompressedStream(&psCompressed, ps_AviStream, lpopts, NULL) != AVIERR_OK)
    {
      AVIKill();
      Mpeg_KILL();
    }

    if (AVIStreamSetFormat(psCompressed, 0, lpbirgb, birgb.biSize) != AVIERR_OK)
    {
      AVIKill();
      Mpeg_KILL();
    }
  }
*/


  iView_Fast_RGB = 0;


  if (P_SnapOnly)
  {
     lpSaveY = y444;
     lpSaveU = currU;
     lpSaveV = currV;
  }

  currU = u444;
  currV = v444;

  if (MPEG_Seq_chroma_format  < CHROMA422) // == CHROMA420)
  {
      if (iCtl_View_Fast_RGB      // Fast RGB decode
      &&  ! P_SnapOnly)           // Not good enough for snapshots
      {
         iView_Fast_RGB = 2;
         if (iView_SwapUV)
         {
           currU = src[2];  
           currV = src[1];  
         }
         else
         {
           currU = src[1];  
           currV = src[2];  
         }
      }
      else
      {
        conv420to422(src[1], u422);
        conv420to422(src[2], v422);

        conv422to444(u422, u444);
        conv422to444(v422, v444);
      }
  }
  else
  {
      // CHROMA422 assumed
      if (iCtl_View_Fast_RGB      // Fast RGB decode
      &&  ! P_SnapOnly)           // Not good enough for snapshots
      {
         iView_Fast_RGB = 1;      // 422 has chroma info on every line
         if (iView_SwapUV)
         {
           currU = src[2]; // v422; 
           currV = src[1]; // u422;  
         }
         else
         {
           currU = src[1]; // u422; 
           currV = src[2]; // v422;  
         }
      }
      else
      
      {
        conv422to444(src[1], u444);
        conv422to444(src[2], v444);
      }
  }


 
  y_orig = src[0];
  if (iLumEnable_Flag[1])
  {
    Lum_Filter_RGB(src[0], LumAdjBuf);
    y444 = LumAdjBuf;
  }
  else
    y444 = src[0];



  if (Bot_Field_Built)
  {
    conv444toRGB24odd(y444, currU, currV, rgb24);
    if (!P_SnapOnly)
    {
        Top_Field_Built = 1;
        if (MParse.iColorMode==STORE_RGB24)
            Flush_RGB24();
    }

    conv444toRGB24even(y444, currU, currV, rgb24);
    if (!P_SnapOnly)
    {
        Bot_Field_Built = 1;
        if (MParse.iColorMode==STORE_RGB24)
            Flush_RGB24();
    }
  }
  else
  {
    conv444toRGB24even(y444, currU, currV, rgb24);
    if (!P_SnapOnly)
    {
        Bot_Field_Built = 1;
        if (MParse.iColorMode==STORE_RGB24)
            Flush_RGB24();
    }

    conv444toRGB24odd(y444, currU, currV, rgb24);
    if (!P_SnapOnly)
    {
        Top_Field_Built = 1;
        if (MParse.iColorMode==STORE_RGB24)
            Flush_RGB24();
    }

  }


  if (MParse.iColorMode==STORE_RGB24 && !P_SnapOnly)
  {
    if (MParse.FO_Flag!=FO_FILM && Rpt_First_Field)
    {
      if (Top_Field_First)
      {
        Top_Field_Built = 1;
        Flush_RGB24();
      }
      else
      {
        Bot_Field_Built = 1;
        Flush_RGB24();
      }
    }
  }


  if (P_SnapOnly)
  {
     y444  = lpSaveY;
     currU = lpSaveU;
     currV = lpSaveV;
  }

  return;
}





//------------------------------------------------------------

static void Flush_RGB24()
{
  if (Top_Field_Built & Bot_Field_Built)
  {
/*    if (AVI_Flag)
    {
      if (AVIStreamWrite(psCompressed, frame_count++, 1, rgb24,
        birgb.biSizeImage, 0, NULL, &frame_size) != AVIERR_OK)
      {
        AVIKill();
        Mpeg_KILL();
      }

      avi_size += frame_size;
    }
*/
    if (iShowVideo_Flag  && (MParse.iColorMode==STORE_RGB24))
          RenderRGB24();

    PlayCtl.iPlayed_Frames++;
    Top_Field_Built = Bot_Field_Built = 0;
  }
}

 
//---------------------------------------------------------

void Store_YUY2a(unsigned char *src[])    //, DWORD frame)
{
/*  if (AVI_Flag && AVI_Init)
  {
    if (!frame)
    {
      compvars.cbSize = sizeof(compvars);

      if (!ICCompressorChoose(hWnd, ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME,
        lpbiyuv, NULL, pcompvars, NULL))
        Mpeg_KILL();

      // set AVI header
      ZeroMemory(&strinfo, sizeof(AVISTREAMINFO));
      strinfo.fccType         = streamtypeVIDEO;
      strinfo.fccHandler        = compvars.fccHandler;
      strinfo.dwQuality       = -1;
      strinfo.dwSuggestedBufferSize = biyuv.biSizeImage;
      strinfo.dwScale = 1000;
      strinfo.dwRate = (unsigned int)((MParse.FO_Flag==FO_FILM) ? frame_rate*800 : frame_rate*1000);

      // set AVI save options
      opts.cbFormat = sizeof(biyuv);
      opts.fccType = streamtypeVIDEO;
      opts.fccHandler = compvars.fccHandler;
      opts.dwKeyFrameEvery = compvars.lKey;
      opts.dwQuality = compvars.lQ;
      opts.dwBytesPerSecond = compvars.lDataRate<<10;
      opts.dwFlags = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES | AVICOMPRESSF_DATARATE;
      opts.dwInterleaveEvery = 0;
      opts.lpFormat = lpbiyuv;
      opts.cbParms = compvars.cbState;
      opts.lpParms = compvars.lpState;

      iccf.dwRate = strinfo.dwRate;
      iccf.dwScale = strinfo.dwScale;
      iccf.lQuality = compvars.lQ;
      iccf.lDataRate = compvars.lDataRate<<10;
      iccf.lKeyRate = compvars.lKey;

      ICSendMessage(compvars.hic, ICM_COMPRESS_FRAMES_INFO, (WPARAM)&iccf, (DWORD)sizeof(ICCOMPRESSFRAMES));
    }

    AVI_Init = 0; avi_size = 0;
    AVIFileInit();

    sprintf(VideoOut, "%s.%02d.avi", szOutput, avi_count++);

    if (AVIFileOpen(&pfile, VideoOut, OF_WRITE | OF_CREATE, NULL) != AVIERR_OK)
    {
      AVIKill();
      Mpeg_KILL();
    }

    if (AVIFileCreateStream(pfile, &ps_AviStream, &strinfo) != AVIERR_OK)
    {
      AVIKill();
      Mpeg_KILL();
    }

    if (AVIMakeCompressedStream(&psCompressed, ps_AviStream, lpopts, NULL) != AVIERR_OK)
    {
      AVIKill();
      Mpeg_KILL();
    }

    if (AVIStreamSetFormat(psCompressed, 0, lpbirgb, birgb.biSize) != AVIERR_OK)
    {
      AVIKill();
      Mpeg_KILL();
    }

    if (!frame && !ICSeqCompressFrameStart(pcompvars, (LPBITMAPINFO)lpbiyuv))
    {
      AVIKill();
      Mpeg_KILL();
    }
  }
*/
  iView_Fast_YUV = 0;

  if (MPEG_Seq_chroma_format  < CHROMA422) // == CHROMA420)
  {
    if (iCtl_View_Fast_YUV) // && Coded_Pic_Width > 700)  // Fast 4:2:0 decode
    {
       iView_Fast_YUV = 1;
       currU = src[1];
       currV = src[2];
    }
    else
    {
       conv420to422(src[1], u422);
       conv420to422(src[2], v422);
    }
  }
  else
  {
    if (iCtl_View_Fast_YUV) // && Coded_Pic_Width > 700) 
    {
       iView_Fast_YUV = 1;
       currU = src[1];
       currV = src[2];
    }
    else
    {
      u422 = src[1];
      v422 = src[2];
    }
  }


  y_orig = src[0];
  if (iLumEnable_Flag[0] && ! iView_Fast_YUV)
  {
    Lum_Filter_OVL(src[0], LumAdjBuf);
    y444 = LumAdjBuf;
  }
  else
    y444 = src[0];


#ifdef DBG_RJ
  if (DBGflag)
  {
    sprintf(szDBGln, "y444=%08X Lum=%08X\n", y444, LumAdjBuf);
    DBGout(szDBGln);
  }
#endif


  if (Bot_Field_Built)
  {
    Cnv_422_yuy2_FLD(1, y444, u422, v422, yuy2);
    Flush_YUY2();

    if (Deint_VIEW)
        Bot_Field_Built = 1;
    else
        Cnv_422_yuy2_FLD(0, y444, u422, v422, yuy2);

    Flush_YUY2();
  }
  else
  {
    if (Deint_VIEW)
        Bot_Field_Built = 1;
    else
        Cnv_422_yuy2_FLD(0, y444, u422, v422, yuy2);

    Flush_YUY2();

    Cnv_422_yuy2_FLD(1, y444, u422, v422, yuy2);
    Flush_YUY2();
  }


  if (MParse.FO_Flag!=FO_FILM && Rpt_First_Field)
  {
    if (Top_Field_First)
    {
      Top_Field_Built = 1;
      Flush_YUY2();
    }
    else
    {
      Bot_Field_Built = 1;
      Flush_YUY2();
    }
  }

}




//-----------------------------------------------------------

static void Flush_YUY2()
{
/*  void *yuy2c;
  int key_flag, yuy2c_size;
*/


/*
#ifdef DBG_RJ
  if (DBGflag)
  {
    sprintf(szDBGln, "Flush T=%d B=%d\n", Top_Field_Built, Bot_Field_Built);
    DBGout(szDBGln);
  }
#endif
*/

  if (Top_Field_Built & Bot_Field_Built)
  {
/*    if (AVI_Flag)
    {
      if ((yuy2c = ICSeqCompressFrame(pcompvars, 0, yuy2, &key_flag, &yuy2c_size)) == NULL)
      {
        AVIKill();
        Mpeg_KILL();
      }

      if (AVIStreamWrite(ps_AviStream, frame_count++, 1, yuy2c, yuy2c_size,
        key_flag ? AVIIF_KEYFRAME : 0, NULL, &frame_size) != AVIERR_OK)
      {
        AVIKill();
        Mpeg_KILL();
      }

      avi_size += frame_size;
    }
*/
    if (DDOverlay_Flag && iShowVideo_Flag)
       RenderYUY2(1);

    PlayCtl.iPlayed_Frames++;
    Top_Field_Built = Bot_Field_Built = 0;
  }
}





//-------------------------------------------------------------
/* static */ void Cnv_422_yuy2_FLD(int P_OddEven,
                    unsigned char *py, unsigned char *pu,
                    unsigned char *pv, unsigned char *dst)
{
   if (P_OddEven == 0)
   {
       py += CLIP_STEP      + Coded_Pic_Width;
       pu += CLIP_HALF_STEP + HALF_WIDTH;
       pv += CLIP_HALF_STEP + HALF_WIDTH ;
       dst += DOUBLE_CLIP_WIDTH;
       Bot_Field_Built = 1;
   }
   else
   {
       py += CLIP_STEP;
       pu += CLIP_HALF_STEP;
       pv += CLIP_HALF_STEP;
       Top_Field_Built = 1;
   }

   // If Fast420 mode activated
   // then we only need the flagging of Bot_Field_Built and Top_Field_Built
   // we can skip the YUY2 color space conversion
   if (iView_Fast_YUV)
     return;

  __asm
  {
    mov     eax, [py]
    mov     ebx, [pu]
    mov     ecx, [pv]
    mov     edx, [dst]
    mov     esi, 0x00
    mov     edi, [Clip_Height]

yuy2conv:
    movd    mm2, [ebx+esi]
    movd    mm3, [ecx+esi]
    punpcklbw mm2, mm3
    movq    mm1, [eax+esi*2]
    movq    mm4, mm1
    punpcklbw mm1, mm2
    punpckhbw mm4, mm2

    add     esi, 0x04
    cmp     esi, [HALF_CLIP_WIDTH]
    movq    [edx+esi*4-16], mm1
    movq    [edx+esi*4-8], mm4
    jl      yuy2conv

    add     eax, [DOUBLE_WIDTH]
    add     ebx, [Coded_Pic_Width]
    add     ecx, [Coded_Pic_Width]
    add     edx, [QUAD_CLIP_WIDTH]
    sub     edi, 0x02
    mov     esi, 0x00
    cmp     edi, 0x00
    jg      yuy2conv

    emms
  }
}




/*
//-------------------------------------------------------

static void conv420toRGB24_FLD(unsigned char *py, unsigned char *pu,
                               unsigned char *pv, unsigned char *dst)
{

  int iLine;
  DWORD dUtmp, dVtmp;

  unsigned char *Tpy,  *Tpu, *Tpv;

  Tpy = py;
  Tpu = pu;
  Tpv = pv;

  iLine = Clip_Height;

  dUtmp = (*(pu  ) * (16777216+65536) )
        + (*(pu+1) *  257             );

  __asm
  {
    //mov     edi, [Clip_Height]
    mov     esi, 0x00

    mov     edx, [dst]

    mov     eax, [Tpy]
    mov     ebx, [Tpu]
    mov     ecx, [Tpv]

    pxor    mm0, mm0    // XOR qword


convRGB24:
    movd    mm1, [eax+esi]    // MM1=Y   Move doubleword

    //movd    mm3, [ebx+esi]  // MM3=U   Move doubleword
    xor     eax,eax
    mov     al,  [ebx+esi]
    mov     ah,  [ebx+esi]
    mov     [dUtmp], eax
    inc     esi
    mov     al,  [ebx+esi]
    mov     ah,  [ebx+esi]
    imul    eax, 65536

    punpcklbw mm1, mm0      //         Unpack Low Packed 
    punpcklbw mm3, mm0      //         Unpack Low Packed
    
    movd    mm5, [ecx+esi]  //  MM5=V  Move doubleword
    punpcklbw mm5, mm0      //         Unpack Low Packed Byte-Word

  }
    // Convert 4 pels of YUV variables from registers
    // outputting into rgb buffer

#include "YUV2RGB_MMX.H"

  __asm
  {

    // Advance 4 PELS  (12 bytes)

    add     edx, 0x0c         // inc RGB pointer
    movd    [edx-4], mm5      // Move doubleword
    
    add     esi, 0x04         // inc Column number
    cmp     esi, [Clip_Width] // Check for end of line segment
    jl      convRGB24         // END OF LINE SEGMENT ?

    // INCREMENT COMPONENT BASE POINTERS 
    // TO START OF NEXT LINE IN FIELD
    // WHICH IS TWO LINES DISTANCE IN THE BUFFERS

    mov     eax, [Tpy]
    add     eax, [DOUBLE_WIDTH]  // Y
    mov     [Tpy], eax

    mov     ebx, [Tpu]
    add     ebx, [Coded_Pic_Width]  // U
    mov     [Tpu], ebx

    mov     ecx, [Tpv]
    add     ecx, [Coded_Pic_Width]  // V
    mov     [Tpv], ecx

    sub     edx, [NINE_CLIP_WIDTH] // RGB (skip back 3 lines, because windows is upside down)

    mov     esi, 0x00  // Reset Column Number

    mov     edi, [iLine]  // Check Line Number
    sub     edi, 0x02     // Decrement Lines to go
    mov     [iLine], edi  // Save Line Number
    cmp     edi, 0x00     // Finished all wanted Lines ?
    jg      convRGB24

    emms
  }
}

*/



//-------------------------------------------------------

static void conv444toRGB24_FLD(unsigned char *py, unsigned char *pu,
                               unsigned char *pv, unsigned char *dst)
{

  // Converts YUV format data to RGB for a single interlaced field

  // Orignally written for 4:4:4, which is very rare.

  // Now extended to do a very rough conversion of 4:2:0 (and 4:2:2)
  // leaves a bit of color fringing aronud objects
  // TODO:  Figure out the Assembler routine for cleaner 420 conversion

  int iLine, iAlternate;
  int iColorLine, iColorInc, iColorTrig;
  DWORD dUtmp, dVtmp;

  unsigned char *Tpy,  *Tpu, *Tpv;
  unsigned char *Jpy,  *Jpu, *Jpv;
  unsigned char *RightEdge;

  // Had to change a lot of registers to memory variables
  // to allow for more sophisticated 4:2:0 calculations  

  Tpy = Jpy = py;
  Tpu = Jpu = pu;
  Tpv = Jpv = pv;

  iLine = Clip_Height;
  iAlternate = 0;
  RightEdge = py + Clip_Width;

  if (iView_Fast_RGB)
  {
    iColorInc  = 2;
    iColorTrig = iView_Fast_RGB;
    iColorLine = Coded_Pic_Width;
  }
  else
  {
    iColorInc  = 4;
    iColorTrig = 1;
    iColorLine = DOUBLE_WIDTH;
  }

/*
  if (py == 1)
  {
   conv420toRGB24_FLD(py,pu,pv,dst);
  }
*/

  __asm
  {
    mov     edi, [Clip_Height]
    mov     edx, [dst]

    mov     eax, [Tpy]
    mov     ebx, [Tpu]
    mov     ecx, [Tpv]

    pxor    mm0, mm0        // XOR qword

convLine:
    mov     esi, 0x00  // Reset Y  Column Number
    mov     edi, 0x00  // Reset UV Column Number

conv4:
    movd    mm1, [eax+esi]      // MM1=Y   Move doubleword

    movd    mm3, [ebx+edi]  // MM3=U   Move doubleword
    movd   [dUtmp], mm3     //         Save for later

    punpcklbw mm1, mm0      //  Y       Unpack Low Packed 
    punpcklbw mm3, mm0      //  U       Unpack Low Packed
    
    movd    mm5, [ecx+edi]  //  MM5=V  Move doubleword
    movd   [dVtmp], mm5     //         Save for later
    punpcklbw mm5, mm0      //  V      Unpack Low Packed Byte-Word
  }

    // Convert 4 pels of YUV variables from registers
    // outputting into rgb buffer


#include "YUV2RGB_MMX.H"

  __asm
  {
    // Advance 4 PELS  (12 bytes)

    add     edx, 0x0c         // inc RGB pointer
    movd    [edx-4], mm5      // Move doubleword
    
    add     edi, [iColorInc]  // inc UV Column number
    add     esi, 0x04         // inc Y  Column number
    cmp     esi, [Clip_Width] // Check for end of line segment

  /*
    mov     eax, [Jpy]
    add     eax,  0x04
    mov     [Jpy], eax
    cmp     eax, [RightEdge]
  */
    jl      conv4         // END OF LINE SEGMENT ?


    // INCREMENT COMPONENT BASE POINTERS TO START OF NEXT LINE IN FIELD

    mov     eax, [Tpy]
    add     eax, [DOUBLE_WIDTH]  // Y
    mov     [Tpy], eax
    mov     [Jpy], eax

    // Allow for Fast decode of sub-sampled color lines
    mov     edi,[iAlternate]
    add     edi,0x01
    cmp     edi,[iColorTrig]
    jl      dunClr
    xor     edi,edi

    mov     ebx, [Tpu]
    add     ebx, [iColorLine]  // U
    mov     [Tpu], ebx

    mov     ecx, [Tpv]
    add     ecx, [iColorLine]  // V
    mov     [Tpv], ecx

dunClr:
    mov     [iAlternate],edi

    sub     edx, [NINE_CLIP_WIDTH] // RGB

    mov     edi, [iLine]  // Check Line Number
    sub     edi, 0x02     // Decrement Lines to go
    mov     [iLine], edi  // Save Line Number
    cmp     edi, 0x00     // Finished all wanted Lines ?
    jg      convLine

/*
    sub     edi, 0x02  // Decrement Lines to go
    cmp     edi, 0x00  // Finished all wanted Lines ?
    jg      convRGB24

*/
    emms
  }
}



//--------------------------------------------------------------

static void conv444toRGB24odd(unsigned char *py, unsigned char *pu, 
                              unsigned char *pv, unsigned char *dst)
{

  //if (! iView_Fast_RGB  ||  ! Deint_VIEW) // Removed due to inconsistencies between encodes
  {
     py += CLIP_STEP; // Offset for clipping
     pu += CLIP_STEP; // Offset for clipping
     pv += CLIP_STEP; // Offset for clipping
     dst += RGB_DOWN1;

     conv444toRGB24_FLD(py, pu,pv,dst);
  }

}




//-------------------------------------------------------

static void conv444toRGB24even(unsigned char *py, unsigned char *pu,
                               unsigned char *pv, unsigned char *dst)
{
  int iColorLine;

  py += Coded_Pic_Width + CLIP_STEP; // Offset for cropping

  if (iView_Fast_RGB)
     iColorLine = Coded_Pic_Width / 2;
  else
     iColorLine = Coded_Pic_Width;

  pu += iColorLine + CLIP_STEP;
  pv += iColorLine + CLIP_STEP;

  dst += RGB_DOWN2;

  conv444toRGB24_FLD(py, pu,pv,dst);

}


/*
  __asm
  {
    mov     eax, [py]
    mov     ebx, [pu]
    mov     ecx, [pv]
    mov     edx, [dst]
    mov     edi, [Clip_Height]
    mov     esi, 0x00
    pxor    mm0, mm0

convRGB24:
    movd    mm1, [eax+esi]
    movd    mm3, [ebx+esi]
    punpcklbw mm1, mm0
    punpcklbw mm3, mm0
    movd    mm5, [ecx+esi]
    punpcklbw mm5, mm0
    movq    mm7, [mmmask_0128]
    psubw   mm3, mm7
    psubw   mm5, mm7

    psubw   mm1, [RGB_Offset]
    movq    mm2, mm1
    movq    mm7, [mmmask_0001]
    punpcklwd mm1, mm7
    punpckhwd mm2, mm7
    movq    mm7, [RGB_Scale]
    pmaddwd   mm1, mm7
    pmaddwd   mm2, mm7

    movq    mm4, mm3
    punpcklwd mm3, mm0
    punpckhwd mm4, mm0
    movq    mm7, [RGB_CBU]
    pmaddwd   mm3, mm7
    pmaddwd   mm4, mm7
    paddd   mm3, mm1
    paddd   mm4, mm2
    psrld   mm3, 13
    psrld   mm4, 13
    packuswb  mm3, mm0
    packuswb  mm4, mm0

    movq    mm6, mm5
    punpcklwd mm5, mm0
    punpckhwd mm6, mm0
    movq    mm7, [RGB_CRV]
    pmaddwd   mm5, mm7
    pmaddwd   mm6, mm7
    paddd   mm5, mm1
    paddd   mm6, mm2
    psrld   mm5, 13
    psrld   mm6, 13
    packuswb  mm5, mm0
    packuswb  mm6, mm0

    punpcklbw mm3, mm5
    punpcklbw mm4, mm6
    movq    mm5, mm3
    movq    mm6, mm4
    psrlq   mm5, 16
    psrlq   mm6, 16
    por     mm3, mm5
    por     mm4, mm6

    movd    mm5, [ebx+esi]
    movd    mm6, [ecx+esi]
    punpcklbw mm5, mm0
    punpcklbw mm6, mm0
    movq    mm7, [mmmask_0128]
    psubw   mm5, mm7
    psubw   mm6, mm7

    movq    mm7, mm6
    punpcklwd mm6, mm5
    punpckhwd mm7, mm5
    movq    mm5, [RGB_CGX]
    pmaddwd   mm6, mm5
    pmaddwd   mm7, mm5
    paddd   mm6, mm1
    paddd   mm7, mm2

    psrld   mm6, 13
    psrld   mm7, 13
    packuswb  mm6, mm0
    packuswb  mm7, mm0

    punpcklbw mm3, mm6
    punpcklbw mm4, mm7

    movq    mm1, mm3
    movq    mm5, mm4
    movq    mm6, mm4

    psrlq   mm1, 32
    psllq   mm1, 24
    por     mm1, mm3

    psrlq   mm3, 40
    psllq   mm6, 16
    por     mm3, mm6
    movd    [edx], mm1

    psrld   mm4, 16
    psrlq   mm5, 24
    por     mm5, mm4
    movd    [edx+4], mm3

    add     edx, 0x0c
    add     esi, 0x04
    cmp     esi, [Clip_Width]
    movd    [edx-4], mm5

    jl      convRGB24

    add     eax, [DOUBLE_WIDTH]
    add     ebx, [DOUBLE_WIDTH]
    add     ecx, [DOUBLE_WIDTH]
    sub     edx, [NINE_CLIP_WIDTH]
    mov     esi, 0x00
    sub     edi, 0x02
    cmp     edi, 0x00
    jg      convRGB24

    emms
  }
*/

/*


//-----------------------------------------------------------

static void AVIKill()
{
  AVI_Init = 1; frame_count = 0;

  if (ps_AviStream)
    AVIStreamClose(ps_AviStream);

  if (psCompressed)
    AVIStreamClose(psCompressed);

  if (pfile)
    AVIFileClose(pfile);

  if (MParse.Stop_Flag || MParse.Fault_Flag)
  {
    AVIFileExit();

    if (MParse.iColorMode!=STORE_RGB24)
      ICSeqCompressFrameEnd(pcompvars);

    ICCompressorFree(pcompvars);
  }
}
*/

/*
//------------------------------
static int gcd(int x, int y)
{
  int t;

  while (y != 0) {
    t = x % y; x = y; y = t;
  }

  return x;
}
*/






#include "YUV_444.c"