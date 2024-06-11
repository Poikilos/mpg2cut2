
#include "windows.h"
#include "WINUSER.H"
#include "global.h"
#define true 1
#define false 0
// #include <commctrl.h>
// #include "DDRAW_CTL.h"

//char szAbbr1[4], szAbbr2[4], szAbbr3[4], szAbbr4[4], szAbbr5[4], szAbbr6[4];


//------------------------------------------------------
void DSP1_Main_MSG(const int P_Long, const int P_Beep)
{
 
  int iLen;
  char *lpCTmp1;
  char szMain[_MAX_PATH*4];

  if (P_Beep)
      MessageBeep(MB_OK);

  if ( szMsgTxt[0] > ' ' || szMPG_ErrTxt[0] > ' ')
  {
    iLen = sprintf(szMain, "%s %s          \n", szMsgTxt, szMPG_ErrTxt);    
    
    if (DBGflag)
    {
        DBGout(szMain);
    }

    lpCTmp1 = strchr(szMain, '\n');
    if(lpCTmp1)
      if(*lpCTmp1)
         *lpCTmp1 = ' ';

    

    if (szMain[0] == '.')
       iMsgLife = 0;
    else
    if ((    MParse.Fault_Flag >= CRITICAL_ERROR_LEVEL
          && MParse.Fault_Flag <  97
          && szMain[0])
       //         || !MParse.SeqHdr_Found_Flag  
        || szMain[0] == '*')
    {
      MessageBox( NULL, szMain, "Mpg2Cut2 - PROBLEM",
                                   MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      iMsgLife = -1;
      szMain[0] = 0;
      szMsgTxt[0] = 0;
    }
    else
    {
      if (iLen > 54 && !P_Long)  // Let PF1=Long Msg
          iLen = 54;
      TextOut(hDC, 0, iMsgPosY, szMain, iLen);
      iMsgLife = 3 + (MParse.FastPlay_Flag<<1);
      UpdateWindow(hWnd_MAIN);
    }
    if (iMsgLen < iLen)
        iMsgLen = iLen;

    if (Frame_Number)
        szMsgTxt[0] = 0;
    szMPG_ErrTxt[0] = 0;

  }

}



//--------------------------------------------------------------
void DSP2_Main_SEL_INFO(int P_Force)
{
  int iLen; //, iTmp1;
  __int64 i64Tmp1;
  unsigned uSel_Secs, uSel_Min, uTot_Time ;
  char cTimeUnit;

  if (process.ToPadLoc <= 0)
  {
      File_Final = File_Limit-1;
      process.ToPadFile =                 File_Final ;
      //process.ToPadBlk  = (process.length[File_Final]/MPEG_SEARCH_BUFSZ) ;
      process.ToPadLoc  =  process.length[File_Final];
  }

  iSelMB  =  (int) ( (process.origin[process.ToPadFile] + process.ToPadLoc
                    - process.origin[process.FromFile]  - process.FromLoc )
                 //(process.ToPadBlk - process.FromBlk) * MPEG_SEARCH_BUFSZ
                   / 1048576 ) ;
  iInputTotMB =  (int) ((process.total + 524288) / 1048576 ) ;

  
  if (process.ToViewPTS > 0)
  {
      uSel_Secs = (process.ToViewPTS - process.FromPTS + 22000) / 45000 ;// +45000)/90000;
  }
  else
  {
    // Derive time estimate from file size and nominal byte rate
    i64Tmp1 =  process.ByteRateAvg[process.CurrFile];
    if (! i64Tmp1)
      i64Tmp1 = 1234567;
    uSel_Secs = (int)((__int64)(iSelMB) * 1048576 / i64Tmp1);
  }
  

  if (uSel_Secs > 59)
  {
     uSel_Min = uSel_Secs / 60;
     uSel_Secs -= (uSel_Min * 60);
     if (uSel_Min > 600) // Trap silly figures
        szTmp32[0] = 0;  // Suppress the time estimate
     else
        sprintf(szTmp32, "%dm%ds ", uSel_Min, uSel_Secs);
  }
  else
     sprintf(szTmp32, "%ds ", uSel_Secs);


  if (uEDL_TotTime > 120) // 2 mins 
  {
       uTot_Time = uEDL_TotTime / 60;  // 600; 
       cTimeUnit = 'm';
  }
  else
  {
       uTot_Time = uEDL_TotTime; //  / 10; 
       cTimeUnit = 's';
  }

  iLen = sprintf(szSelTxt, " %d clips %d%c   %dmb/%02d   +%s%dmb       ",
                   iEDL_ctr, uTot_Time, cTimeUnit,
                                            iEDL_TotMB, iInputTotMB,
                                            // cSelStatus, 
                                            szTmp32, iSelMB);

  if (MParse.SeqHdr_Found_Flag 
  &&  (iMsgLife <= 0 || P_Force)   // process.Preamble_Known)
  &&
    (process.Action != ACTION_RIP    // Hide boring stuff while playing
     ||  iPreview_Clip_Ctr <= 500    // Not a ClipList Preview
     ||  iViewToolBar >= 256
     || (!iMainWin_State && iViewToolBar > 1) 
     )
  ) 
      TextOut(hDC, iSelMsgX, iMsgPosY, szSelTxt, iLen);

  //process.trackPrev = SendMessage(hTrack, TBM_GETPOS, 0, 0);

}
 

/*
//--------------------------------------------------------------
void DSP365_TC_Edit(TC_HMSFR *xTC)

{

  if (xTC.hour > 00)
         sprintf(szHour, "%dh", xTC.hour);
      else
         szHour[0] = NULL;

      iLen = sprintf(szTemp, " %s %02dm %02ds %02df %c  A%c%s  Z%x   ",
              szHour, xTC.minute, xTC.sec, xTC.frameNum,
                      Coded_Pic_Abbr[MPEG_Pic_Type],
                      process.Delay_Sign[0], process.szDelay,
               rj_Audio_Code );

}
*/


// Adjust TC to Time Of Day, using creation time of FIRST file
// If files are not continous, then gives incorrect time of day

void Relative_TOD() //  "TOD"
{

  ShowTC.frameNum  =  RelativeTC.frameNum;

  ShowTC.sec       =  RelativeTC.sec    + File_Greg[0].tm_sec;
  ShowTC.minute    =  RelativeTC.minute + File_Greg[0].tm_min;
  ShowTC.hour      =  RelativeTC.hour   + File_Greg[0].tm_hour;

  while (ShowTC.sec > 59)
  {
      ShowTC.sec = ShowTC.sec - 60;
      ShowTC.minute++;
  }

  while (ShowTC.minute > 59)
  {
      ShowTC.minute = ShowTC.minute - 60;
      ShowTC.hour++;
  }

  while (ShowTC.hour > 23)
  {
      ShowTC.hour = ShowTC.hour - 24;
  }
  while (ShowTC.hour < 0)
  {
      ShowTC.hour = ShowTC.hour + 24;
  }

  if (ShowTC.hour < 12)
      strcpy(ShowTC_AM_PM, "am");
  else
      strcpy(ShowTC_AM_PM, "pm");
}





//--------------------------------------------------------------

void DSP3_Main_TIME_INFO()
{
  int iTmp, iLen, iOverrideX;
  unsigned int uTmp1;
  char szHour[8], szTOD[24];

  //ShowTC_AM_PM[0] = 0;

  process.szDelay[0] = 0;
  szTOD[0] = 0;

  iOverrideX = iTimeX;  

  if (iViewToolBar >= 256)  
  {
    if (process.Delay_Sign[0] != '?') 
        sprintf(process.szDelay, "a%c%dms", process.Delay_Sign[0], 
                                            process.Delay_ms);
    else
    if (MParse.SystemStream_Flag > 0  // Program Stream
    &&  ! process.Got_PTS_Flag        // SEQ hdr pkt missing PTS ?
    &&  PktStats.iVid_Packets)
        strcpy(process.szDelay, "NO PTS");
  }

  if ( !iView_TC_Format // Show block number instead of time
  || (CurrTC.RunFrameNum < 1 
       && process.CurrLoc > 0 && ! iPES_Mpeg_Any))
  {
      iTmp = (int) (process.CurrLoc / MPEG_SEARCH_BUFSZ);
      iLen = sprintf(szTemp, "Blk#%06X  %c  %s       ",
                // process.CurrFile,
                iTmp,
                Coded_Pic_Abbr[MPEG_Pic_Type],
                process.szDelay);
  }
  else
  if (iView_TC_Format == 5) // Show in HEX
  {
      iTmp = (int) (process.VideoPTSM & PTS_MASK_0);
      iLen = sprintf(szTemp, "x%08x  %c  %s             ",
                              iTmp,
                Coded_Pic_Abbr[MPEG_Pic_Type],
                process.szDelay);

  }
  else 
  if (iView_TC_Format == 4) // Calc Time of day  "TOD"
  {
      RelativeTC_SET();
      Relative_TOD(); 

      iLen = sprintf(szTemp, " %02d:%02d :%02d %s %02df  %c  %s      ",
                   ShowTC.hour, ShowTC.minute, ShowTC.sec, ShowTC_AM_PM, ShowTC.frameNum,
                                         Coded_Pic_Abbr[MPEG_Pic_Type],
                                         process.szDelay );
  }
  else
  {
     if (iView_TC_Format == 7) // Show both REL and TOD
     {
        RelativeTC_SET();
        Relative_TOD();
  
        iOverrideX-=4;
  
        iLen = sprintf(szTOD, " %02d:%02d:%02d%s  ",
                                ShowTC.hour, ShowTC.minute, ShowTC.sec, 
                                                          ShowTC_AM_PM);
        memcpy(&ShowTC, &RelativeTC, sizeof(ShowTC));

     }
     else
     if (iView_TC_Format == 1) // Show time relative to start
     {
              RelativeTC_SET();
              memcpy(&ShowTC, &RelativeTC, sizeof(ShowTC));
     }
     else
     if (iView_TC_Format  == 2  || !MParse.SystemStream_Flag) // GOP time
     {
              memcpy(&ShowTC, &gopTC,      sizeof(ShowTC));
     }
     else
     if (iView_TC_Format  == 3)  // Show time from PTS without adjustment
     {
        if (PktStats.iChk_AudioPackets > PktChk_Audio
        ||  PktStats.iChk_AnyPackets   > PktChk_Any)
            uTmp1 = process.AudioPTS;
        else
            uTmp1 = process.VideoPTS;

        PTS_2Field(uTmp1, 0);
        memcpy(&ShowTC, &ptsTC,      sizeof(ShowTC));
     }
     else
     if (iView_TC_Format  == 6)  // Show time from SCR
     {
              PTS_2Field( process.uViewSCR, 0);
              memcpy(&ShowTC, &ptsTC,      sizeof(ShowTC));
     }
     else                       
     {
              memcpy(&ShowTC, &CurrTC,     sizeof(ShowTC));
     }     


     if (ShowTC.hour > 00)
         sprintf(szHour, "%dh", ShowTC.hour);
     else
     {
         strcpy(szHour, "  "); 
         // szHour[0]   = 0;
         // iOverrideX +=20;
     }

     if (ShowTC.frameNum < -999 
     ||  ShowTC.frameNum >  999)
         ShowTC.frameNum =  999;

     iLen = sprintf(szTemp, "%s %s %02dm %02ds %02df %c   %s   ",
                 szTOD,
                 szHour, ShowTC.minute, ShowTC.sec, ShowTC.frameNum,
                         Coded_Pic_Abbr[MPEG_Pic_Type],
                         process.szDelay );
  }



  // Hiding Toolbar  makes Time less intrusive
  if (iViewToolBar < 256 && iView_TC_Format  != 7)  
  {
    if (iMainWin_State > 0)  
      iOverrideX = VGA_Width - (((iLen-13)/2)*18) - 3;  // Maximizing Window also makes Time less intrusive
    else
    if (iLen > 20)
        iOverrideX += 50; 
    else
        iOverrideX += 70; 
  }

  if (iLen > 1)
  {
      iLen--;
      TextOut(hDC, iOverrideX, iTimeY, szTemp, iLen) ;
  }

  return;
}









//-------------------------------------------------------------
//Update the context information in the title bar
void DSP5_Main_FILE_INFO()
{
static char FieldMode_Name[4][4] = {"?", "Top", "Bot", "p"};

  int  iTmp1;
  char cTmp1, /*cTmp2,*/ szMantissa[6];
  char *lpszAspect, *ext;


      /*            //RJDBG
               if (DBGflag)
               {
                  int iTmp1, iTmp2, iTmp3, iTmp4, iTmp5;
                  iTmp1 = (int)process.SEQ_Loc;
                  iTmp2 = (int)process.GOP_Loc;
                  iTmp3 = (int)process.KEY_Loc;
                  iTmp4 = (int)process.PIC_Loc;
                  iTmp5 = (int)process.PACK_Loc;
                  //iTmp5  = (int)process.FromLoc;
                  //iTmp6  = (int)process.ToPadLoc;
                  //iTmp11 = (int)process.CurrBlk;
                  //iTmp13 = (int)process.FromBlk;
                  //iTmp15 = (int)process.ToPadBlk;
                  sprintf(szMsgTxt, "Seq=%x Gop=%x Key=%x Pic=%x Pack=%x ",
                                  iTmp1, iTmp2, iTmp3, iTmp4,  iTmp5);
                  //DBGout(szMsgTxt) ;
               }
        */
   // Build Main  Title

   strcpy(szBuffer, szAppName) ;

   //   if (DBGflag)
   //   {
   //      sprintf(szTemp," h=%d %d/%d %d. ",
   //                    iAspect_Height, iLineCtr, iSrcUse, iVertInc ) ;
   //      strcat(szBuffer, szTemp);
   //   }

   // Estimate size of selection & original      RJS
   if (File_Limit)
   {
      //strcat(szBuffer, szName) ;
      ext = strrchr(File_Name[File_Ctr], '\\');
      if (!ext)
        ext = File_Name[File_Ctr] -1;
      strncat(szBuffer, ext+1, 50);
      //strncat(szBuffer, ext+1,
      //           strlen(File_Name[File_Ctr])
      //             -(int)(ext-&File_Name[File_Ctr]));

      if (File_Final)
      {
          sprintf(szTemp, " %d/%d ", (File_Ctr+1), File_Limit);
          strcat(szBuffer, szTemp);
      }

      //process.Delay_ms = process.DelayPTS / 45; //90;

      if ( MParse.SeqHdr_Found_Flag )
      {
         if (iNom_kBitRate > 10000)
         {
            iTmp1 = iNom_kBitRate / 1000;
            cTmp1 = 'M';
         }
         else
         {
            iTmp1 = iNom_kBitRate;
            cTmp1 = 'k';
         }

         if (iFrame_Rate_mantissa)
            sprintf(szMantissa, ".%02d", iFrame_Rate_mantissa);
         else
            szMantissa[0] = 0;

         if (process.Mpeg2_Flag || Mpeg_PES_Version == 2)
           lpszAspect = Mpeg2_Aspect_Ratio_Name[MPEG_Seq_aspect_ratio_code];
         else
           lpszAspect = Mpeg1_Aspect_Ratio_Name[MPEG_Seq_aspect_ratio_code];

         sprintf(szTemp,"  %d.%d%c%c%s %s  %2d%sfps  %d%c/s %s",
                MPEG_Seq_horizontal_size, MPEG_Seq_vertical_size,
                          *ScanMode_Name[ScanMode_code], // Is original progressive frame  (MPEG_Pic_Origin_progressive [progressive_frame] from Pic Coding Extension)
                          *ScanMode_Name[MPEG_Seq_progressive_sequence],
                           FieldMode_Name[MPEG_Pic_Structure], // How is encode structured ? ( [picture_structure] from Pic Coding Extension)
                     lpszAspect,
                    iFrame_Rate_dsp, szMantissa,  iTmp1, cTmp1,
               MPEG_Seq_chroma_Desc[MPEG_Seq_chroma_format]
               /* , iFrame_Period_ms */  ) ;
        strcat(szBuffer, szTemp);
      }

   }


   SetWindowText(hWnd_MAIN, szBuffer);

   if (File_Limit)
   {
      DSP2_Main_SEL_INFO(0);  // iMsgLife+=3;
      DSP3_Main_TIME_INFO();
   }

   // DSP1_Main_MSG(0,0);

   return ;
}







HDC hdc_Main;
PAINTSTRUCT ps_Main;


// -------------------------------------------------------------

void VGA_GetSize()
{                
  int rval;

  VGA_Width  = GetSystemMetrics(SM_CXSCREEN);
  VGA_Height = GetSystemMetrics(SM_CYSCREEN);

     
  // gnorkus 
  // As for the question regarding the taskbar size, 
  // you can use the following code to obtain it:


  // This obtains the rect not obscured by the taskbar or any 
  // desktop toolbars. This will vary depending on the 
  // height of the taskbar.

  rval = SystemParametersInfo ( SPI_GETWORKAREA, 0, 
                               &rcAvailableScreen, 0);


  // This obtains the maximum client area for a window that 
  // has been maximized. This will vary depending on the 
  // size and position of the taskbar.

  iVGA_Avail_Height = rcAvailableScreen.bottom - rcAvailableScreen.top;
  if (iVGA_Avail_Height <= 0)
      iVGA_Avail_Height = GetSystemMetrics ( SM_CYFULLSCREEN ); // not very accurate, but better than nothing

  iVGA_Avail_Width  = rcAvailableScreen.right - rcAvailableScreen.left;
  if (iVGA_Avail_Width <= 0)
     iVGA_Avail_Width  = GetSystemMetrics ( SM_CXFULLSCREEN ); // not very accurate, but better than nothing

}


void Calc_PhysView_Size()
{
    int iTmp1;

    if (IsIconic(hWnd_MAIN))
      return;

    GetClientRect(hWnd_MAIN, &crect);

    iTmp1 = crect.bottom - crect.top - iTopMargin - 2;
    if (iTmp1 > 340)
        iPhysView_Height = iTmp1;
    else
        iPhysView_Height = 340;

    iTmp1 = crect.right - crect.left;
    if (iTmp1 > 280)
        iPhysView_Width  = iTmp1; // - 2;
    else
        iPhysView_Width  = 280;

/*
    if ((VGA_Width  - iPhysView_Width ) < 10
    &&  (VGA_Height - iPhysView_Height) < 10)
        iMaximized = SW_SHOWMAXIMIZED;
    else
        iMaximized = 0;
*/

  iViewMax_Height  = crect.bottom - crect.top // iVGA_Avail_Height - crect.top + wrect.top
                                  - iTopMargin;

  iViewMax_Width   =  crect.right - crect.left; // iVGA_Avail_Width - 1;

  /*
  TaskBar.rc.top    = VGA_Height;
  TaskBar.rc.bottom = VGA_Height;
  TaskBar.rc.left   = 0;
  TaskBar.rc.right  = 0;

  
  *pwnd = FindWindow("Shell_TrayWnd", NULL);
  if (pwnd != NULL)
  {

     TaskBar.cbSize = sizeof(APPBARDATA);
     TaskBar.hWnd   = pwnd->m_hWnd;

     iRC = SHAppBarMessage(ABM_GETTASKBARPOS, TaskBar);
  }
  
  

  iTaskBar_Width    = TaskBar.rc.right  - TaskBar.rc.left;
  iTaskBar_Height   = TaskBar.rc.bottom - TaskBar.rc.top;

  //iViewMax_Width    = VGA_Width - 1     - iTaskBar_Width ;
  //iVGA_Avail_Height = VGA_Height        - iTaskBar_Height;

  */

}





void MainWin_Rect()
{
  GetWindowRect(hWnd_MAIN, &wrect);
  GetClientRect(hWnd_MAIN, &crect);
  Edge_Width  = wrect.right  - wrect.left - crect.right;
  Edge_Height = wrect.bottom - wrect.top  - crect.bottom;
  iMenuHeight = crect.top - wrect.top;
  if (iMenuHeight < 0) iMenuHeight = - iMenuHeight;

  //Edge_Width  = GetSystemMetrics(SM_CXBORDER);
  //Edge_Height = GetSystemMetrics(SM_CYBORDER);
}



//-------------------------------------------------
/*
void Enable_Disable(int P_Act, int P_Audio, int P_NavButtons )
{
   bool W_Act;
   UINT uAct;

   if (P_Act)
       W_Act = 1;
   else
       W_Act = 0;

  if (W_Act)
  {
      uAct = MF_ENABLED;
      //if (DBGflag) TextOut(hDC,  60,  50, "Enable", 6);
  }
  else
  {
      uAct = MF_GRAYED;
      //if (DBGflag) TextOut(hDC, 100,  50, "Disable", 7);
  }



   EnableWindow(hAddButton,  W_Act);
   //EnableWindow(hLumButton,  W_Act);
   EnableWindow(hBmpButton,  W_Act);
   //EnableWindow(hZoomButton, W_Act);

   if (W_Act)    // Allow scroll while playing
      EnableWindow(hTrack, W_Act);

   if (P_NavButtons)
   {
       EnableWindow(hMarkLeft, W_Act);

       EnableWindow(hBack2, W_Act);
       EnableWindow(hBack1, W_Act);
       EnableWindow(hFwd1, W_Act);
       EnableWindow(hFwd2, W_Act);

       EnableWindow(hMarkRight, W_Act);
   }
   // "Edit" heading in Menu
   //EnableMenuItem(hMenu, 1, (MF_BYPOSITION | uAct));

   // "Audio" heading in Menu
   if (P_Audio && !W_Act)
      EnableMenuItem(hMenu, 5, (MF_BYPOSITION | uAct));

   DragAcceptFiles(hWnd, W_Act);


}
*/



/*
//-------------------------------------------------
void Menu_Main_Enable()
{
   Enable_Disable(true, 0, true);

   EnableMenuItem(hMenu, IDM_OPEN,         MF_ENABLED);
   EnableMenuItem(hMenu, IDM_FILE_NAMES,   MF_ENABLED);
   EnableMenuItem(hMenu, IDM_SAVE,         MF_ENABLED);
   EnableMenuItem(hMenu, IDM_FILE_NEWNAME, MF_ENABLED);
//   EnableMenuItem(hMenu, IDM_LOAD_D2V, MF_ENABLED);
//   EnableMenuItem(hMenu, IDM_SAVE_D2V, MF_ENABLED);
//   EnableMenuItem(hMenu, IDM_STOP, MF_GRAYED);

   //SendMessage(hTrack, TBM_SETSEL, (WPARAM) true,
   //                  (LPARAM) MAKELONG(process.trackleft,
   //                                             process.trackright));

   DrawMenuBar(hWnd);
}
*/


//-----------------------------------
void Menu_Main_Disable(int P_audio, int P_NavButtons)
{

   //EnableMenuItem(hMenu, IDM_FILE_NEWNAME, MF_GRAYED);

   //Enable_Disable(false, P_audio, P_NavButtons);

   if (hLumDlg!=NULL && P_NavButtons)
   {
      DestroyWindow(hLumDlg);
      hLumDlg = NULL;
   }

/*   if (hClipResizeDlg!=NULL)
   {
      DestroyWindow(hClipResizeDlg);
      hClipResizeDlg = NULL;
   }
   if (hNormDlg!=NULL)

      DestroyWindow(hNormDlg);
      hNormDlg = NULL;
   }
*/

}



//-------------------------------------------------

void CalcRestoreSize(int p_width, int p_height)
{
int Max_Width, Max_Height ;

   Max_Width = VGA_Width - 160 ;
   if ((VGA_Width > 640) && (Coded_Pic_Width > Max_Width))
      Restore_Width  = Max_Width ;
   else
      Restore_Width  = p_width ;



   Max_Height = iVGA_Avail_Height - Edge_Width - iTopMargin ;
   if ((VGA_Height > 480) && (Coded_Pic_Height > Max_Height))
         Restore_Height = Max_Height ;
   else
         Restore_Height = p_height ;



/*   if (IsMaximized(hWnd))
   {
      if (p_width  < Max_Width)  Overlay_Width   =    p_width ;
      else                       Overlay_Width   = Max_Width ;
      if (p_height < Max_Height) Overlay_Height =    p_height ;
      else                       Overlay_Height = Max_Height ;
   }
   else
*/   {
      Overlay_Width  = Restore_Width ;
      Overlay_Height = Restore_Height ;
   }

}



//--------------------------
void View_Ctr_Crop()
{
  if (DBGflag)
      DBGout("CTR");

   iPred_Prev_Width  = 0; 

   /*
   if (iView_Centre_Crop)
   {
       iTmp1 = (iAspect_Width / 3) &  0xFFFFFFFE;
       if (iView_xFrom < iTmp1)
           iView_xFrom = iTmp1;
   }
   */

   if (iMainWin_State >= 0)
   {
      Mpeg_Aspect_Resize();
      View_Rebuild_Chk(0);
   }

}


//--------------------------
void View_ReSize_Chk(int P_Force)
{
  if (MParse.SeqHdr_Found_Flag)
  {
     if (P_Force 
     || (  (   iAspect_Width  != Prev_Clip_Width
            || iAspect_Height != Prev_Clip_Height)
          &&   ! MParse.iMultiRqst
          && iShowVideo_Flag
         ))
     {
             D500_ResizeMainWindow(iAspect_Width, iAspect_Height, 0);
             Prev_Clip_Width  = iAspect_Width;
             Prev_Clip_Height = iAspect_Height;
     }
  }
}


//--------------------------
void   View_Rebuild_Chk(int P_Force)
{
  if (DBGflag)
      DBGout("Rebuild");

  if (MParse.SeqHdr_Found_Flag)
  {
     View_ReSize_Chk(P_Force);

     if (iShowVideo_Flag && File_Limit)
     {
        if (!iBusy)
        {
          if (MParse.iColorMode == STORE_YUY2 && ! DDOverlay_Flag)
          {
             D100_CHECK_Overlay();
          }

          if (prect.right && prect.bottom)
          {
             if (DDOverlay_Flag)
                 D200_UPD_Overlay();

             if (MParse.Stop_Flag || MParse.Tulebox_SingleStep_flag)
             {
                if (MParse.iColorMode != STORE_YUY2)
                    RenderRGB24();
                else
                if (DDOverlay_Flag)
                    RenderYUY2(1);
             }
          } // endif got a picture

        } // endif !Busy

        DSP3_Main_TIME_INFO(); 
        if (iViewToolBar >= 256) //  || iTool_Stacked)
        {
             DSP2_Main_SEL_INFO(1);
        }

     } // endif ShowVideo
  } // endif SEQ HDR found

}




//----------------------------------------------------------------

int xPos, yPos;

void View_MOUSE_CHK(LPARAM lParam)
{

  //HMENU hPopUp;
  int iOverrideX;

  const char *TimeFmtName[8] // for iView_TC_Format
      = { "BLK",  "Rel",  "GOP",   "PTS",  "TOD", "HEX", "SCR", "   "}; 
  

  xPos = (int)(LOWORD(lParam));
  yPos = (int)(HIWORD(lParam)) - iTopMargin;

  if (yPos <  3) // Mouse in or near text area ?
  {
      //if (yPos >= -iTimeY)
      //{
          iOverrideX = iTimeX-40;

          if (xPos >= iOverrideX)  // Mouse on Time Coordinate ?
          {
            /*
            HMENU GetSubMenu(
                             HMENU hMenu,	// handle of menu
                               int nPos	// menu item position
                             );
            //  IDR_TIME_FMT_MENU
            BOOL TrackPopupMenu(
                 HMENU hMenu,	    // handle of shortcut menu
                  UINT 0,	        // screen-position and mouse-button flags
                   int (iOverrideX-10),   // horizontal position, in screen coordinates
                   int (iTopMargin+10),  // vertical position, in screen coordinates
                   int nReserved,	   // reserved, must be zero
                  HWND hWnd_MAIN,	   // handle of owner window
                       NULL);	
            */

            if (iView_TC_Format < 7)
                iView_TC_Format++;
            else
                iView_TC_Format = 0;

            if (iViewToolBar < 256)
                iOverrideX += 45;

            TextOut(hDC, iOverrideX, iTimeY, BLANK44, 40) ;
            TextOut(hDC, iOverrideX, iTimeY, TimeFmtName[iView_TC_Format], 3) ;
            DSP3_Main_TIME_INFO();
          }
          else
          if (xPos >= iSelMsgX)  // Mouse on Clip Info ?
          {
             if (iViewToolBar > 256)
                 DSP2_Main_SEL_INFO(1);
             else
                 TextOut(hDC, 0, iMsgPosY, BLANK44, 44); // Would be better as a Paint

          }
      //}
  }
  else
  {
         View_MOUSE_ALIGN(lParam);
  }


}



//----------------------------------------------------------------
void View_MOUSE_ALIGN(LPARAM lParam)
{

  int iUnseen;

  if (DBGflag)
      DBGout("MouseAlign");


  if (MParse.SeqHdr_Found_Flag)
  {
      Mpeg_Aspect_Resize();  // In case Zoom has changed;
      //View_ReSize_Chk(0);
  }
  

  if (lParam > -1)
  {
        iView_xFrom = (iView_xFrom +
                      ( (xPos - iPhysView_Width/2) * iZoom)
                         * iAspHoriz / 2048)
                                 & 0xFFFFFFFC ;

         if (iView_Invert)
             yPos = iPhysView_Height - yPos;

         if (yPos > 0)
         {

             iView_yFrom = (iView_yFrom +
                                       ( (yPos - (iPhysView_Height/2) )
                                                        * iAspVert / 2048)
                           ) &  0xFFFFFFFC;
         }
  }

  // X

  if (iView_xFrom < 0)
      iView_xFrom = 0;
  else
  {
      if (iView_xFrom > iOverload_Width)
          iView_xFrom = iOverload_Width;// + 2;
  }

  // Y

  iUnseen = iOverload_Height; // + iMenuHeight + iTopMargin;  // crect.top - wrect.top

  if (iView_yFrom < 0)
                             
      iView_yFrom = 0;
  else
  if (iView_yFrom > iUnseen)
      iView_yFrom = iUnseen; //+ 2;
   

  iView_yFrom = iView_yFrom & 0xFFFFFC;
  if (MParse.SeqHdr_Found_Flag)
      Mpeg_Aspect_Calc();  // In case Zoom has changed;

  View_Rebuild_Chk(0);
}






void DSP_Msg_Clear()
{
  RECT msgrect;

  int iWidth, iShared;
  HBRUSH hBrush;

  if (iViewToolBar >= 256          
  || (!iMainWin_State && iViewToolBar > 1) ) 
     iShared = 0;   // Message area is separate from picture area
  else
     iShared = 1;   // Message is superimposed on picture area
  
  if (MParse.iColorMode == STORE_RGB24
  //||  iCtl_Back_Colour  != iCtl_Mask_Colour
     )
  {
      TextOut(hDC,  0,  iMsgPosY,  BLANK44,  44);  
      iWidth = iSelMsgX;
  }
  else
  {
      if (iTimeY > iMsgPosY)
          iWidth = iAspect_Width-1;
      else
          iWidth = iTimeX;

      SetRect(&msgrect, 0, iMsgPosY, iWidth, (iMsgPosY+(uFontHeight+2)));
      if (iShared)          // Message area is shared with video area
        hBrush = hBrush_MASK;
      else
        hBrush = hBrush_MSG_BG;
      FillRect(hDC, &msgrect, hBrush); 
  }

  iMsgLen  =  0; 

  // Conditionally restore selection info
  if (process.Action != ACTION_RIP    // Hide boring stuff while playing
  ||  iPreview_Clip_Ctr <= 500        // Not a ClipList Preview
  ||  !iShared)                       // Message area is separate
  {
     DSP2_Main_SEL_INFO(1); // Repair Clip info
     if (MParse.SystemStream_Flag > 0)  // Program Stream
         DSP3_Main_TIME_INFO();

  }

  //if (! iViewToolBar)
  //   SetBkColor(hDC, iCtl_Back_Colour);  // Background = black
    
  UpdateWindow(hWnd_MAIN);
}

/*
void DSP_Blank_Msg_Clean()
{
    //  Repair the underlying window info
    TextOut(hDC, 0, iMsgPosY, BLANK44, 44); // Would be better as a Paint
    if (!iMainWin_State || iViewToolBar >= 256)
         DSP2_Main_SEL_INFO(1);
    UpdateWindow(hWnd_MAIN);

}
*/


//--------------------------------------------------------------
void MainPaint()
{
         hdc_Main = BeginPaint(hWnd_MAIN, &ps_Main);
         EndPaint(hWnd_MAIN, &ps_Main);
         ReleaseDC(hWnd_MAIN, hdc_Main);

         if (MParse.SeqHdr_Found_Flag && MParse.iColorMode==STORE_RGB24)
            RenderRGB24();
}


