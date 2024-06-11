//
// CLIP list handling
//

#include "global.h"
#include <commctrl.h>
#include "Buttons.h"

#define true  1
#define false 0




//----------------------------------------------------
void C000_Clip_TOTAL_MB(char P_Act)
{  
  int i, j, j_from , j_to;
  int iRC, iBadClip;
  unsigned uFromPTS, uToPTS, uClip_Time;

  __int64 i64ClipB, i64OutB, i64Adj, i64Tmp1;

  iBadClip = 0;

  File_Final = File_Limit - 1;

  if (P_Act == 'a')
  {
    iEDL_ClipFrom = 0;
    iEDL_ClipTo   = iEDL_ctr;
  }
  


  iEDL_OutClips = 0;
  uEDL_TotTime  = 0;
  i64OutB       = 0;

  // Calc totals of clips to be written

  for (i=iEDL_ClipFrom ;  i < iEDL_ClipTo ; i++)
  {

    i64Adj =  EDList.FromLoc [i];

    // Allow for clip crosssing file boundary/s
    j_from = EDList.FromFile   [i];
    j_to   = EDList.ToViewFile [i];

    if (j_from > File_Final  ||  j_to > File_Final)
    {
        if (!iBadClip)
        {
           int iTmp1, iTmp2;
           iTmp1 = (int) i64Adj ;
           iTmp2 = (int) EDList.ToViewLoc [i] ;
           sprintf(szBuffer,
          "BAD FILE in Clip#%d \n\nFrom#%2xx To#%2xx \n\nRange %2xx %2xx ",
                          i, j_from, j_to, iTmp1, iTmp2);
           iRC = MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - BUG !",
                      MB_ICONSTOP | MB_OKCANCEL);

        }
        else 
          iRC = IDOK;

        j_from = EDList.FromFile   [i] = File_Final;
        j_to   = EDList.ToViewFile [i] = File_Final;

        iBadClip++;

        if (iRC == IDCANCEL)
             break;
    }

    i64ClipB = 0;
    for (j = j_from; j < j_to ; j++ )
    {
          i64ClipB += process.length [j] - i64Adj ;
          i64Adj = 0;
    }

    i64ClipB += EDList.ToViewLoc [i] - i64Adj ;
    i64OutB += i64ClipB;

    EDList.uClip_MB[i] = (unsigned int)((i64ClipB + 524288) >>20);

    uFromPTS = EDList.FromPTS   [i];
    uToPTS   = EDList.ToViewPTS [i];

    if (uToPTS > uFromPTS && uToPTS != PTS_NOT_FOUND)
        uClip_Time = (uToPTS - uFromPTS + 22000) / 45000;
    else
    {
         i64Tmp1 =  process.ByteRateAvg[process.CurrFile];
         if (!i64Tmp1)
              i64Tmp1 = 1234567;
         uClip_Time = (unsigned int)(i64ClipB / i64Tmp1);
    }

    EDList.uClip_Secs[i] = uClip_Time;

    uEDL_TotTime += uClip_Time;


    iEDL_OutClips++;
  } 
   
  iEDL_TotMB = (int)((i64OutB + 524288) / 1048576); // include rounding of half a meg

  if (File_Limit && MParse.SeqHdr_Found_Flag) // (P_Act == 'a')
  {
    DSP2_Main_SEL_INFO(1);
    iMsgLife = 3;
  }

  return ;

}



//-----------------------------------------------------------
// Set default clip selection
//
// Actions: 
//   'o' = Open   = Select All (excluding preamble)
//   'a' = Append = Conditionally update TO=oldEOF becomes TO=NewEOF
//   'E' = EOF    = Unconditional set    TO=EOF
//
void C100_Clip_DEFAULT(char P_Act)
{
  if (File_Limit)
  {
     // FROM POINT

     if (P_Act == 'o')
     {
         process.FromFile = 0;
         if (process.Preamble_Known)
         {
            process.FromLoc  = process.preamble_len;
            process.FromPTS  = process.Preamble_PTS;      
            process.FromPTSM = process.Preamble_PTSM;
         }
         else
         {
            process.FromLoc  = 0;
            process.FromPTS  = 0;
            process.FromPTSM = 0;
         }

         iEDL_Start_Type = -9; // Assume file starts with something good
         process.iSEQHDR_NEEDED_clip1 = 0;
          
         //process.FromBlk  = process.FromLoc / MPEG_SEARCH_BUFSZ;            
     }

     File_Final     = File_Limit - 1;

     // TO POINT

     if (P_Act != 'a'        ||  ! iEDL_ctr         
     ||  Ed_Prev_Act == '['  ||  Ed_Prev_Act == 0 )
     {
         C140_Clip_EOF();
     }
  }

  if (P_Act != 'E')
  {
      T590_Trackbar_SEL();
  }
}


void C140_Clip_EOF()
{
         process.ToPadFile = File_Final ;
         //process.ToPadBlk  = (process.length[process.ToPadFile] / MPEG_SEARCH_BUFSZ);
         process.ToPadLoc  =  process.length[process.ToPadFile];
         process.ToPadPTS  = 0;
         process.ToPadPTSM = 0;

         process.ToViewFile = process.ToPadFile;
         //process.ToViewBlk  = process.ToPadBlk;
         process.ToViewLoc  = process.ToPadLoc;  
         process.ToViewPTS  = PTS_NOT_FOUND;
         process.ToViewPTSM = 0;

         process.endFile   = File_Final;
         process.endrunloc = process.total - MPEG_SEARCH_BUFSZ;
         process.endLoc    = (process.length[process.endFile]
                              / MPEG_SEARCH_BUFSZ - 1)
                              * MPEG_SEARCH_BUFSZ;
}




//--------------------------------------------------------------
void  C160_Clip_Preview()
{
  __int64 i64Summary_Size;

  int iFromFile, iEndFile, iLen, iMin, iSecs;


  iSecs = EDList.uClip_Secs[iPreview_Clip_Ctr];
  if (iSecs > 59)
  {
    iMin   = iSecs / 60;
    iSecs -= iMin  * 60;
    sprintf(szTmp32, "%dm %ds", iMin, iSecs);
  }
  else
  {
    sprintf(szTmp32, "%ds", iSecs);
  }

  iLen = sprintf(szBuffer, "Clip#%d  %d MB  %s      ", (iPreview_Clip_Ctr+1),
                              EDList.uClip_MB  [iPreview_Clip_Ctr], 
                              szTmp32);

  TextOut(hDC, iSelMsgX, iMsgPosY, szBuffer, iLen);
  iMsgLife = -10;
  //UpdateWindow(hWnd);


  iFromFile = EDList.FromFile [iPreview_Clip_Ctr];

  i64Summary_Size =  (5*process.ByteRateAvg[iFromFile]);
  if (i64Summary_Size < 2048000)
      i64Summary_Size = 2048000;

             
  process.startFile  =  EDList.FromFile [iPreview_Clip_Ctr];
  process.startLoc   =  EDList.FromLoc  [iPreview_Clip_Ctr];

  if (iCtl_To_Pad) // Option to grab extra video frame
  {
      process.endFile =  EDList.ToPadFile[iPreview_Clip_Ctr];
      process.endLoc  =  EDList.ToPadLoc [iPreview_Clip_Ctr]; 
  }
  else
  {
      process.endFile =  EDList.ToViewFile[iPreview_Clip_Ctr];
      process.endLoc  =  EDList.ToViewLoc [iPreview_Clip_Ctr]; 
  }


  if (MParse.Summary_Section) // Only want to see tail of clip ?
  {
      process.startFile =  process.endFile;
      process.startLoc  = (process.endLoc - i64Summary_Size) / MPEG_SEARCH_BUFSZ;
      process.startLoc  =  process.startLoc * MPEG_SEARCH_BUFSZ;
      //MParse.Summary_Adjust = +1;

      // Allow for crossing file boundary
      while (process.startLoc < 0)
      {
         if (process.startFile > 0)
         {
             process.startFile--;
             process.startLoc +=  process.length[process.startFile];
         }
         else 
            process.startLoc  =  0;
      }

      iPreview_Clip_Ctr++;
      MParse.Summary_Section = 0;  // Next time get the top
  }
  else
  {
    T580_Trackbar_CLIP();

    if (iCtl_Play_Summary // Just want top of clip ?
    && (   (process.endLoc -  process.startLoc > (i64Summary_Size*3)) // Clip big enough to warrant summarizing ?
        || (process.endFile != process.startFile) ) )
    {
      iEndFile        =  process.endFile;
      process.endFile =  process.startFile;
      process.endLoc  = (process.startLoc + i64Summary_Size) / MPEG_SEARCH_BUFSZ;
      process.endLoc  =  process.endLoc * MPEG_SEARCH_BUFSZ;
      MParse.Summary_Adjust = -1;  // Mark for later cushioning for cleaner break

      // Allow for crossing file boundary
      while (process.endLoc > process.length[process.endFile])
      {
         if (process.endFile < iEndFile)
         {
             process.endLoc -=  process.length[process.endFile];
             process.endFile++;
         }
         else 
             process.endLoc = process.length[process.endFile];
      }

      MParse.Summary_Section = 1;  // Next time get the tail
    }
    else
    iPreview_Clip_Ctr++;
  }
/*
	if (DBGflag) 
    DBGln4(
			"\nPREVIEW CLIPS From=x%04X End=x%04X   Sect=%d  Clip=%d\n",
				        process.startLoc, process.endLoc, 
                                 MParse.Summary_Section, iPreview_Clip_Ctr);
*/
}




//-----------------------------------------------------------
// Remember Position info for possible EDIT UNDO (temporary) 
void C310_Pos_MEMO(int P_Clip)
{
  int iClip;
  
  iClip = P_Clip;

  EDList.FromFile[iClip] = process.CurrFile;
  //EDList.FromBlk [201] = process.CurrBlk ;
  EDList.FromLoc [iClip] = process.CurrLoc ;
  EDList.FromPTS [iClip] = process.CurrPTS ;

  EDList.uFrom_TCorrection[iClip] = process.uGOP_TCorrection;
  EDList.uFrom_FPeriod_ps [iClip] = process.uGOP_FPeriod_ps;

}


//-----------------------------------------------------------
// Remember clip info for either :-
//     - possible EDIT UNDO (temporary) 
//     - preparatory to storing permanently


void C321_FromSel2Clip()
{
  EDList.FromFile[iEDL_ctr] = process.FromFile;
  //EDList.FromBlk [iEDL_ctr] = process.FromBlk ;
  EDList.FromLoc [iEDL_ctr] = process.FromLoc ;
  EDList.FromPTS [iEDL_ctr] = process.FromPTS ;
  EDList.FromPTSM[iEDL_ctr] = process.FromPTSM;

  EDList.uFrom_TCorrection[iEDL_ctr] = process.uFrom_TCorrection;
  EDList.uFrom_FPeriod_ps [iEDL_ctr] = process.uFrom_FPeriod_ps;

}

void C322_ToSel2Clip()
{
  EDList.ToPadFile[iEDL_ctr] = process.ToPadFile;
  EDList.ToPadLoc [iEDL_ctr] = process.ToPadLoc ;
  EDList.ToPadPTS [iEDL_ctr] = process.ToPadPTS ;
  EDList.ToPadPTSM[iEDL_ctr] = process.ToPadPTSM ;

  EDList.ToViewFile[iEDL_ctr] = process.ToViewFile;
  EDList.ToViewLoc [iEDL_ctr] = process.ToViewLoc ;
  EDList.ToViewPTS [iEDL_ctr] = process.ToViewPTS ;
  EDList.ToViewPTSM[iEDL_ctr] = process.ToViewPTSM ;
}



void C320_Sel2Clip()
{
  C321_FromSel2Clip();
  C322_ToSel2Clip();

  C000_Clip_TOTAL_MB('a');
}


//------------------



void C323_Clip2Clip(int P_From, int P_To)
{
  // This would be better as a group move
  // Need to recode EDLIST as the equivalent of Cobol Group Occurs.

  EDList.FromFile[P_To]  = EDList.FromFile[P_From];
  EDList.FromLoc [P_To]  = EDList.FromLoc [P_From];
  EDList.FromPTS [P_To]  = EDList.FromPTS [P_From];
  EDList.FromPTSM[P_To]  = EDList.FromPTSM[P_From];

  EDList.uFrom_TCorrection[P_To] = EDList.uFrom_TCorrection[P_From];
  EDList.uFrom_FPeriod_ps [P_To] = EDList.uFrom_FPeriod_ps [P_From];

  EDList.ToPadFile[P_To] = EDList.ToPadFile[P_From];
  EDList.ToPadLoc [P_To] = EDList.ToPadLoc [P_From];
  EDList.ToPadPTS [P_To] = EDList.ToPadPTS [P_From];
  EDList.ToPadPTSM[P_To] = EDList.ToPadPTSM[P_From];

  EDList.ToViewFile[P_To] = EDList.ToViewFile[P_From];
  EDList.ToViewLoc [P_To] = EDList.ToViewLoc [P_From];
  EDList.ToViewPTS [P_To] = EDList.ToViewPTS [P_From];
  EDList.ToViewPTSM[P_To] = EDList.ToViewPTSM[P_From];

  EDList.uClip_MB  [P_To] = EDList.uClip_MB  [P_From];
  EDList.uClip_Secs[P_To] = EDList.uClip_Secs[P_From];

}






void C333_Clip_Clear()
{
  Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act      = 'C';
  Ed_Prev_EDL_Ctr  = iEDL_ctr ;
  iEDL_ctr = 0;
  process.EDL_Used = 0;
  C323_Clip2Clip(0, 202); // Allow for UNDO - First clip tends to get overwritten quickly

}


void C352_Clip_ADD(char, int);


//-----------------------------------------------------------
// Add a clip to the Edit Decision List

// Description Codes :-
//    C = "Current"  
//    P = "Previous"
//    + = No reminder required

void C350_Clip_ADD(char P_DescAbbr, int P_SizeChk)
{
  if ( ! File_Limit)
  {
     MessageBeep(MB_OK);
  }
  else
     C352_Clip_ADD( P_DescAbbr, P_SizeChk);
}





//-----------------------------------------------------------
// Add a clip to the Edit Decision List
void C352_Clip_ADD(char P_DescAbbr, int P_SizeChk)
{

  int iRC, w_Verbose, iPrev, iCurr, iBeep, iNear;
  char szDesc[16];
  int *lp_Ctl_Warn;
  unsigned int uWarn_Id; 
  

  iBeep = 0;
  iEDL_Chg_Flag = 1;

  switch (P_DescAbbr)
  {
    case 'C':
      strcpy(szDesc, "Current");
      break;

    case 'P':
      strcpy(szDesc, "PREVIOUS");
      break;

    default:
      strcpy(szDesc, "");
      break;
  }


  // It's easy to forget to add a clip, so prompt just in case

  if (P_DescAbbr >= 'C')
  {
    if (Add_Automation > 1)
        iBeep = 1;
    else
    {
      sprintf(szBuffer, "%s selection was NOT ADDED to Edit List\n\nDo you want to include it ?", szDesc);
      iRC = MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - CONFIRM", MB_OKCANCEL);
      if (iRC ==2)
        return;
    }
  }

 if (P_DescAbbr != '0')
 {
    if (iViewToolBar >= 256)
    {
        EnableWindow(hAddButton, MF_GRAYED);
    }
    else
    {
      AddButton_Create();
      UpdateWindow(hWnd_MAIN);
    }
 }


  w_Verbose = 0;
  if (iEDL_ctr > 200)
  {
    MessageBox(hWnd_MAIN, "CANNOT STORE ANY MORE CLIPS - MAX 200 REACHED", 
               szAppName, MB_ICONSTOP | MB_OK);
    return;
  }


  // If one end has not changed, alter the previous selection
  if (iEDL_ctr > 0)
  {
    iPrev = iEDL_ctr - 1;

    C323_Clip2Clip(iPrev, 203);  // Save for UNDO

    if ( EDList.FromFile  [iPrev] == process.FromFile
      // && EDList.FromBlk   [iPrev] == process.FromBlk 
      && EDList.FromLoc   [iPrev] == process.FromLoc
      
      || EDList.ToViewFile[iPrev] == process.ToViewFile
      // && EDList.ToViewBlk [iPrev] == process.ToViewBlk 
      && EDList.ToViewLoc [iPrev] == process.ToViewLoc
      )
    {
      //iRC  = MessageBox(hWnd, "Same START point as previous selection...\n\n REPLACE previous selection ?\n",
      //         "Mpg2Cut2 - CONFIRM", MB_OKCANCEL);
      //if (iRC == 1) 

           iEDL_ctr = iPrev;
           //Tick_Ctl(TBM_CLEARTIC, EDList.ToViewFile[iEDL_ctr], 
           //                     EDList.ToViewLoc [iEDL_ctr]);

    }
  }

  // Copy the selection information into the table
  C320_Sel2Clip();

  if (iEDL_ctr < 201)
  {
    iCurr = iEDL_ctr;

    iEDL_ctr++;

    sprintf(szBuffer, "Clip #%d added to Edit Decision List", iEDL_ctr);
    if(iEDL_ctr > 200)
    {
       strcat(szBuffer, "\n\n*** LIMIT REACHED - NO MORE ALLOWED ***");
       w_Verbose++;
    }

    if (w_Verbose)
        MessageBox(hWnd_MAIN, szBuffer, szAppName, MB_OK);
    else
    {
      Tick_Ctl(TBM_SETTIC, process.ToPadFile, process.ToPadLoc);

      if (P_DescAbbr != '0')
      {
           // Flash the ADD button to acknowledge request
           MoveWindow(hAddButton, (iToolbarWidth/2), 150,
                              iTool_Wd, iTool_Ht, true);
           if (iBeep) MessageBeep(MB_OK);

           if (MParse.EDL_AutoSave  &&  iEDL_ctr > 1 && !iEDL_Reload_Flag)
           {
               iRC = C800_Clip_FILE(SAVE_EDL, 1, 'o');
           }

           Sleep(150);
           // ShowWindow(hWnd, wCmdShow);
           if (iViewToolBar >= 256)
           {
               MoveWindow(hAddButton,   iTool_Wd, 0,
                                        iTool_Wd, iTool_Ht, true);
           }
           else
              DestroyWindow(hAddButton);
      }

    }

    Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act = '+';

  }
  
  C000_Clip_TOTAL_MB('a');

  if (P_SizeChk && iEDL_TotMB > 700)
  {
    iNear = 0;
    if (iEDL_TotMB > 3700)
    {
      if ( iCtl_WarnSize_4 && !process.iWarnSize_4)
      {
         process.iWarnSize_4 = 1;
         iNear = 4;
         lp_Ctl_Warn = &iCtl_WarnSize_4; uWarn_Id = IDM_WARN_SIZE_4;

      }
    }
    else
    if (iEDL_TotMB > 2700)
    {
      if ( iCtl_WarnSize_3 && !process.iWarnSize_3)
      {
         process.iWarnSize_3 = 1;
         iNear = 3;
         lp_Ctl_Warn = &iCtl_WarnSize_3; uWarn_Id = IDM_WARN_SIZE_3;
      }
    }
    else
    if (iEDL_TotMB > 1700)
    {
      if ( iCtl_WarnSize_2 && !process.iWarnSize_2)
      {
         process.iWarnSize_2 = 1;
         iNear = 2;
         lp_Ctl_Warn = &iCtl_WarnSize_2; uWarn_Id = IDM_WARN_SIZE_2;
      }
    }
    else
    {
      if ( iCtl_WarnSize_1 && !process.iWarnSize_1)
      {
         process.iWarnSize_1 = 1;
         iNear = 1;
         lp_Ctl_Warn = &iCtl_WarnSize_1; uWarn_Id = IDM_WARN_SIZE_1;
      }
    }

    if (iNear)
    {
       sprintf(szBuffer, "AROUND %d GB TOTAL CLIPS\n\nTime to consider a SAVE.\n",
                                    iNear);
       //MessageBox(hWnd_MAIN, szBuffer, 
       //           "Mpg2Cut2 - Warning", MB_ICONSTOP | MB_OK);
       Warning_Box(&szBuffer[0], 0, lp_Ctl_Warn, uWarn_Id, MB_OKCANCEL);

    }
  }

}




//----------------------------------------------------------
void C400_Clip_DEL()
{
int iRC;
  
  if (iEDL_ctr < 1)
  {
    MessageBox(hWnd_MAIN, "NO CLIPS LEFT TO REMOVE", 
               szAppName, MB_ICONSTOP | MB_OK);
    return;
  }


  sprintf(szBuffer, "REMOVE clip #%d from Edit Decision List ?", iEDL_ctr);
  iRC = MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - CONFIRM", MB_OKCANCEL);

  if (iRC == 1)
  {
    iEDL_ctr--;  iEDL_Chg_Flag = 1;
    //Tick_Ctl(TBM_CLEARTIC, EDList.ToViewFile[iEDL_ctr], 
    //                            EDList.ToViewLoc [iEDL_ctr]);
    Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act = '-';
  }

  C000_Clip_TOTAL_MB('a');
}
 



//----------------------------------------------------------
void C450_Clip_DEL_ALL()
{
int iRC;
  
  if (iEDL_ctr < 1)
    return;


  sprintf(szBuffer, "DELETE ENTIRE Edit Decision List ?", iEDL_ctr);
  iRC = MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - CONFIRM", MB_OKCANCEL);

  if (iRC ==1)
  {
     C333_Clip_Clear();
     iEDL_Chg_Flag = 0;
     SendMessage(hTrack, TBM_CLEARTICS, (WPARAM) true, 0);
  }

  C000_Clip_TOTAL_MB('a');

}



//----------------------------
void  C550_Clip2Selection()
{
        process.FromFile  = EDList.FromFile[iEDL_ctr];
        //process.FromBlk   = EDList.FromBlk [iEDL_ctr];
        process.FromLoc   = EDList.FromLoc [iEDL_ctr];
        process.FromPTS   = EDList.FromPTS [iEDL_ctr];

        process.uFrom_TCorrection = EDList.uFrom_TCorrection[iEDL_ctr];
        process.uFrom_FPeriod_ps  = EDList.uFrom_FPeriod_ps [iEDL_ctr];
        iFrame_Period_ps          = process.uFrom_FPeriod_ps;
         
        process.ToPadFile = EDList.ToPadFile [iEDL_ctr];
        //process.ToPadBlk  = EDList.ToPadBlk  [iEDL_ctr];
        process.ToPadLoc  = EDList.ToPadLoc  [iEDL_ctr];
        process.ToPadPTS  = EDList.ToPadPTS  [iEDL_ctr];
        process.ToPadPTSM = EDList.ToPadPTSM [iEDL_ctr];

        process.ToViewFile = EDList.ToViewFile[iEDL_ctr];
        //process.ToViewBlk  = EDList.ToViewBlk [iEDL_ctr];
        process.ToViewLoc  = EDList.ToViewLoc [iEDL_ctr];
        process.ToViewPTS  = EDList.ToViewPTS [iEDL_ctr];
        process.ToViewPTSM = EDList.ToViewPTSM[iEDL_ctr];

        T590_Trackbar_SEL();
}



//----------------------------------------------------------
void C500_Clip_UNDO()
{

  strcpy(szBuffer, "Sorry, I don't know what to UNDO");

  switch (Ed_Prev_Act)
  {
    case 'C':  // previous was Clear
          iEDL_ctr = Ed_Prev_EDL_Ctr;
          C323_Clip2Clip(202, 0); // First clip tends to get overwritten quickly
          Ed_Prev_Act = '+'; // Ed_Prev_Act2 = 0; 
          sprintf(szBuffer, "EDL restored to %d clips", iEDL_ctr);
      break;

    case '-':  // previous was Remove
          // if (iEDL_ctr < ??? )   
          {
              iEDL_ctr++;
              sprintf(szBuffer, "Clip #%d RESTORED to EDL", iEDL_ctr);
          }
          Ed_Prev_Act = '+'; // Ed_Prev_Act2 = 0;
      break;

    case '|': // previous was split
          iEDL_ctr--;
          Ed_Prev_Act  = '+';
          Ed_Prev_Act2 = ']';

    case '+': // previous was Add
          if (iEDL_ctr > 0)
          {
              C323_Clip2Clip(203, iEDL_ctr); 
              sprintf(szBuffer, "Clip #%d REMOVED from EDL", iEDL_ctr);
              iEDL_ctr--;
          }

          //if (Add_Automation < 2)
          if (Ed_Prev_Act2 == ']'
          ||  Ed_Prev_Act2 == '[')
          {
              Ed_Prev_Act = Ed_Prev_Act2;  Ed_Prev_Act2 = 0;
          }
          else
          {
              Ed_Prev_Act = ' ';  Ed_Prev_Act2 = 0;
          }
          
          // break;

    case '[':
    case ']':
        C550_Clip2Selection();
        Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act = ' ';

        strcpy(szBuffer, "Selection points RESTORED");
      break;

    case '{':
    case '}':
        process.CurrFile = EDList.FromFile[201];
        //process.CurrBlk  = EDList.FromBlk [201];
        process.CurrLoc  = EDList.FromLoc [201];
        process.CurrPTS  = EDList.FromPTS [201];
        Ed_Prev_Act = ' ';
        strcpy(szBuffer, "Position RESTORED");

        process.uGOP_TCorrection = EDList.uFrom_TCorrection[201];
        process.uGOP_FPeriod_ps  = EDList.uFrom_FPeriod_ps [201];
        iFrame_Period_ps         = process.uGOP_FPeriod_ps;

        // NEED TO GIVE MPEGDEC A KICK HERE
        strcpy(szBuffer, "UNDO Position NOT IMPLEMENTED YET");
      break;

    default:
      break;

  }

  DSP2_Main_SEL_INFO(1);

  MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - Undo", MB_OK);
  iEDL_Chg_Flag = 1;
}






//-----------------------------------------
void C510_Sel_FROM_MARK(int P_External)
{
  int iRC;

  if (P_External && iCtl_KB_NavStopPlay)
      MParse.Stop_Flag = 2;

//if (IsWindowEnabled(hTrack))
//{
  if ( ! File_Limit)
  {
     MessageBeep(MB_OK);
  }
  else
  {
     SetFocus(hWnd_MAIN);

     if (iViewToolBar >= 256)
     {
         EnableWindow(hMarkLeft, false);
     }
     else
     {
         MarkLeftButton_Create();
         UpdateWindow(hWnd_MAIN);
     }


     C320_Sel2Clip();

     // New IN point after OUT point implies a NEW CLIP
     //   Note the NOT! operator...
     if ( ! (  (process.CurrFile  <  process.ToViewFile)
            || (process.CurrFile  == process.ToViewFile
             && process.CurrLoc   < (process.ToViewLoc - 2200) ))) // Allow for ~ 1 pack error
     {

         // Previous selection may need to be saved
         if (Ed_Prev_Act == ']' &&  Add_Automation > 0 &&  P_External)
         {
             C350_Clip_ADD('P', 1);
         }

         // Reset OUT point to END, since we are starting a new clip
         C100_Clip_DEFAULT('E');
         //process.ToPadFile = File_Limit-1;
         //process.ToPadBlk   = (process.length[File_Limit-1]
         //                   /MPEG_SEARCH_BUFSZ);
         //process.ToPadLoc   =  process.length[File_Limit-1];
         //process.trackright = (int)(
         //        (process.run+(__int64)process.ToPadBlk*MPEG_SEARCH_BUFSZ)
         //                * TRACK_PITCH /process.total );
     }

     // Store the current position as the "IN" point
     if ((  process.CurrFile <  process.ToPadFile) // May not need this any more..
        || (process.CurrFile == process.ToPadFile
         && process.CurrLoc  <  process.ToPadLoc))
     {
            //process.FromBlk  = process.CurrBlk;
            process.FromLoc  = process.PACK_Loc;
            process.FromFile = process.PACK_File;
       /*
       if (DBGflag)
           DBGln4("FROM:  SEQ=x%08X.%d PACK=x%08X.%d",
                   process.SEQ_Loc, process.SEQ_File, 
                   // process.GOP_Loc, process.KEY_Loc, 
                   process.PACK_Loc, process.PACK_File);
       */
       iRC = Get_Hdr_Loc(&process.FromLoc, &process.FromFile);

       if (iEDL_Start_Type > 0 && iEDL_ctr == 0)
       {
               MessageBox(hWnd_MAIN, 
               "The current position is not a proper SEQ start point.\n\nFirst edit point needs SEQ, GOP or I-Frame hdr.",
                                                 "Mpg2Cut2 - SORRY", MB_OK); 
       }
       // else
       {
           if (iRC < 0)
            {
               process.FromLoc  = Calc_Loc(&process.FromFile,
                     - (MPEG_SEARCH_BUFSZ * 2), 1);
            }

            if (process.FromLoc < 0)
                process.FromLoc = 0;
            /*
            if (DBGflag)
                DBGln2("FROM:  LOC=x%08X.%d", process.FromLoc, process.FromFile);
            */
            process.FromPTS  = process.VideoPTS;
            process.FromPTSM = process.VideoPTSM;

            process.uFrom_TCorrection = process.uGOP_TCorrection;
            process.uFrom_FPeriod_ps  = process.uGOP_FPeriod_ps;

            process.run = process.origin[process.CurrFile];

            T590_Trackbar_SEL();
            //process.trackPrev = process.trackleft;
            //SendMessage(hTrack, TBM_SETPOS, (WPARAM) true,  process.trackleft);

            Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act = '[';
            DSP2_Main_SEL_INFO(0);

            if (! iEDL_ctr)              // First Clip ?
            {
              if (iEDL_Start_Type < -1)  // Got a NAV or SEQ HDR ?
                  process.iSEQHDR_NEEDED_clip1 = 0; // OK
              else
              {
                 if (iEDL_Start_Type >= 0)  // Got a GOP HDR ?
                     process.iSEQHDR_NEEDED_clip1 = 4; // CRIKEY - NO LEADING GOP/SEQ HDR
                 else
                     process.iSEQHDR_NEEDED_clip1 = 2; // CRIKEY - NO LEADING SEQ HDR

                 strcpy(szMsgTxt,"SEQ HDR will come from Preamble");
                 DSP1_Main_MSG(0,0);
                 if (!iCtl_Out_Preamble_Flag)
                 {
                    iCtl_Out_Preamble_Flag = 1;
                    MessageBox(hWnd_MAIN, szMsgTxt, 
                                "Mpg2Cut2 - Warning", MB_ICONSTOP | MB_OK);
                 }

              }
            }
       
       } // endelse Valid
     } // ENDIF within range

  } // ENDIF allowed

  DSP2_Main_SEL_INFO(1);
  Sleep(200);

  if (iViewToolBar >= 256)
  {
     EnableWindow(hMarkLeft, true);
  }
  else
     DestroyWindow(hMarkLeft);


}



//-----------------------------------
void C525_TO_Point();

void C520_Sel_TO_MARK()
{
  //int iRC;

  if (iCtl_KB_NavStopPlay)
      MParse.Stop_Flag = 2;

  if ( ! File_Limit)
  {
    MessageBeep(MB_OK);
  }
  else
  {
       SetFocus(hWnd_MAIN);

       if (iViewToolBar >= 256)
       {
          EnableWindow(hMarkRight, false);
       }
       else
       {
          MarkRightButton_Create();
          UpdateWindow(hWnd_MAIN);
       }

       if ((process.CurrFile  > process.FromFile)
       ||  (process.CurrFile == process.FromFile
        &&  process.CurrLoc   > process.FromLoc))
       {
            C525_TO_Point();

            if (Add_Automation > 1)
            {
               C350_Clip_ADD('+', 1);  // ADD the details into the list
               Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act = '+';
            }
            else
            {
               Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act = ']';
            }
            //process.run = 0;
            //for (i=0; i<process.ToPadFile; i++)
            //   process.run += process.length[i];
            process.run = process.origin[process.CurrFile];
            T590_Trackbar_SEL();
            //SendMessage(hTrack, TBM_SETPOS, (WPARAM) true,
            //                               process.trackright);

            DSP2_Main_SEL_INFO(1);
       }
       else
       {
         strcpy(szMsgTxt, "Mark FROM must preceed Mark TO"); 
         DSP1_Main_MSG(1, 1);
       }


       //if (Add_Automation > 1)
           Sleep(100);

       if (iViewToolBar >= 256)
       {
          EnableWindow(hMarkRight, true);
       }
       else
          DestroyWindow(hMarkRight);


  } // ENDIF Allowed
}



void C525_TO_Point()
{
  int iRC;
            // Remember where the current frame ENDED
            process.ToPadFile = process.CurrFile;
            //process.ToPadBlk  = process.CurrBlk;
            process.ToPadLoc  = Calc_Loc(&process.ToPadFile, -4, 0); // process.PACK_Loc;

            // Store the Time Stamp from the Video PES header
            process.ToPadPTS   = process.VideoPTS;
            process.ToPadPTSM  = process.VideoPTSM;
            if (process.ViewPTS >= 0)
            {
                process.ToViewPTS  = process.ViewPTS;
                process.ToViewPTSM = process.ViewPTSM;
            }
            else
            {
                process.ToViewPTS  = process.VideoPTS;
                process.ToViewPTSM = process.VideoPTSM;
            }
            // Recall where the current GOP/SEQ STARTED
            //         (Cascade allows for absent headers)
            iRC = Get_Hdr_Loc(&process.ToViewLoc, &process.ToViewFile);
            if (iRC < 0)
            {   // What the heck ?   Must be a mess to land here !
                // Estimate Frame Start based on Frame END
                process.ToViewFile = process.ToPadFile;
                process.ToViewLoc  = process.ToPadLoc// process.PACK_Loc;
                              - (process.ByteRateAvg[process.CurrFile] / 2);
            }

            /*
            if (DBGflag)
            {
                DBGln4("**TO:  SEQ=x%08X.%d PACK=x%08X.%d",
                        process.SEQ_Loc, process.SEQ_File, 
                        // process.GOP_Loc, process.KEY_Loc, 
                        process.PACK_Loc, process.PACK_File);
                DBGln4("      VIEW=x%08X.%d PAD=x%08X.%d",
                        process.ToViewLoc, process.ToViewFile,
                        process.ToPadLoc,  process.ToPadFile);
            }
            */

            // Make sure everything is kosher - just in case of surprises
            if (process.ToViewLoc < 0)
                process.ToViewLoc = 0;
            //process.ToViewBlk = process.ToViewLoc / MPEG_SEARCH_BUFSZ;


            C320_Sel2Clip(); // copy the details in case later undo
}



//-------------------------------------------------

void  C600_Clip_Split()
{
  int iRC, iNext; 

  if ( ! File_Limit)
  {
    MessageBeep(MB_OK);
  }
  else
  {
       SetFocus(hWnd_MAIN);

       /*
       if (iViewToolBar >= 256)
       {
          EnableWindow(hSplitClip, false);
       }
       else
       {
          SplitClipButton_Create();
          UpdateWindow(hWnd_MAIN);
       }
       */

       if ((    (process.CurrFile  <   process.FromFile)
            ||  (process.CurrFile  ==  process.FromFile
              && process.CurrLoc   <=  process.FromLoc))

       || ((     process.CurrFile >   process.ToViewFile) 
            ||  (process.CurrFile ==  process.ToViewFile
              && process.CurrLoc  >=  process.ToViewLoc))
        )
       {
         strcpy(szMsgTxt, "Split point must be within current selection."); 
         DSP1_Main_MSG(1, 1);
       }

       else
       {
            C352_Clip_ADD('0', 1); // Make sure current selection is added

            // duplicate the clip points
            iNext = iEDL_ctr;
            iEDL_ctr--;
            C323_Clip2Clip(iEDL_ctr, iNext);

            // change the first to have a new end point based on current
            C525_TO_Point();
            C320_Sel2Clip();

            // change the second to have a new start point based on current
            iEDL_ctr++;
            C310_Pos_MEMO(iEDL_ctr);
            C550_Clip2Selection();   // Rebuild the selection info

            iEDL_ctr++;
            Ed_Prev_Act = '|';

            DSP2_Main_SEL_INFO(1);
       }


       if (MParse.EDL_AutoSave  &&  iEDL_ctr > 1 && !iEDL_Reload_Flag)
       {
               iRC = C800_Clip_FILE(SAVE_EDL, 1, 'o');
       }

       /*
       Sleep(150);
       if (iViewToolBar >= 256)
       {
          EnableWindow(hSplitClip, true);
       }
       else
          DestroyWindow(hSplitClip);
          */


  } // ENDIF Allowed

}




#include "Nav_JUMP.h"

//--------------------------------------
// Remove references to a particular File number

void  C700_Clip_DeReference(int iKillFile)
{

  int iClipCurr, iClipNext, iKeepFlag, iFile, iRenumber;

  //  Scan Edit Decision List

  iClipNext = iClipCurr = 0;

  while (iClipNext < iEDL_ctr)
  {
      if (EDList.FromFile  [iClipNext] == iKillFile
    //&&  EDList.ToPadFile [iClipNext] == iKillFile
      &&  EDList.ToViewFile[iClipNext] == iKillFile)
         iKeepFlag = 0;
      else
      {
         iKeepFlag = 1;

         // FROM point may need to be adjusted
         iFile = EDList.FromFile[iClipNext];
         if (iFile >= iKillFile) 
         {
             iRenumber = iFile-1;
             if (iFile == iKillFile)
             {
                EDList.FromLoc  [iClipNext] = 0 ;
                EDList.FromPTS  [iClipNext] = 0xFFFFFFFF; // process.FirstPTS [iRenumber];
                EDList.FromPTSM [iClipNext] = 0xFFFFFFFF; // process.FirstPTSM[iRenumber];
                
                EDList.uFrom_TCorrection[iClipNext] = process.uGOP_TCorrection;
                EDList.uFrom_FPeriod_ps [iClipNext] = process.uGOP_FPeriod_ps;
             }
             else
                 EDList.FromFile[iClipNext] = iRenumber;
         }

         // Padded-TO point may need to be adjusted
         iFile = EDList.ToPadFile[iClipNext];
         if (iFile >= iKillFile)
         {
             iRenumber = iFile-1;
             if (iFile == iKillFile)
             {
                EDList.ToPadLoc  [iClipNext] = process.length[iRenumber] ;
                EDList.ToPadPTS  [iClipNext] = 0; // process.LastPTS [iRenumber];
                EDList.ToPadPTSM [iClipNext] = 0; // process.LastPTSM[iRenumber];
             }
             
             EDList.ToPadFile[iClipNext] = iRenumber;
         }


         // Standard TO point may need to be adjusted
         iFile = EDList.ToViewFile[iClipNext];
         if (iFile >= iKillFile)
         {
             iRenumber = iFile-1;
             if (iFile == iKillFile)
             {
                 EDList.ToViewLoc  [iClipNext] = process.length[iRenumber] ;
                 EDList.ToViewPTS  [iClipNext] = 0; // process.LastPTS [iRenumber];
                 EDList.ToViewPTSM [iClipNext] = 0; // process.LastPTSM[iRenumber];
             }
             
             EDList.ToViewFile[iClipNext] = iRenumber;
         }

      } // ENDIF Keeper

      // Some clips may need to be kept, and may need shuffling.
      if (iKeepFlag)
      {
         if (iClipCurr != iClipNext)
         {
             C323_Clip2Clip(iClipNext, iClipCurr);
         }
         iClipCurr++;
      }

      iClipNext++;

  } // END WHILE
  
  iEDL_ctr = iClipCurr;


  // Fix the current selection and position - same way

         // FROM point may need to be adjusted
         iFile = process.CurrFile;
         if (iFile >= iKillFile) 
         {
             if (iFile >= File_Final)
             {
                 process.CurrFile =                File_Final;
                 process.CurrLoc  = process.length[File_Final];
             }
             else
             if (iFile > iKillFile && iFile > 0)
             {
                 process.CurrFile = iFile - 1;
             }
             else
             {
                 process.CurrLoc  = 0;
             }
         }

         File_Ctr          = process.CurrFile;
         process.KILL_File = process.CurrFile;
         process.KILL_Loc  = process.CurrLoc;
         process.startFile = process.CurrFile; 
         process.startLoc  = process.CurrLoc;

         // FROM point may need to be adjusted
         iFile = process.FromFile;
         if (iFile >= iKillFile) 
         {
             if (iFile >= File_Final)
             {
                 process.FromFile =                File_Final;
                 process.FromLoc  = process.length[File_Final];
             }
             else
             if (iFile > iKillFile && iFile > 0)
             {
                 process.FromFile = iFile - 1;
             }
             else
             {
                 process.FromLoc  = 0;
             }
             /*
             if (iFile == iKillFile) 
                 process.FromLoc  = 0 ;
             else
                 process.FromFile = iFile - 1 ;
             */
         }

         // Padded-TO point may need to be adjusted
         iFile = process.ToPadFile;
         if (iFile >= iKillFile)
         {
             process.ToPadFile = iFile - 1;
             if (iFile == iKillFile) 
                 process.ToPadLoc  = process.length[process.ToPadFile];
         }


         // Standard TO point may need to be adjusted
         iFile = process.ToViewFile;
         if (iFile >= iKillFile)
         {
             process.ToViewFile = iFile - 1;
             if (iFile == iKillFile) 
                 process.ToViewLoc  = process.length[process.ToViewFile];
             
         }

  //  breaks the FastBack chain 
  BwdGop.ix   = 0;  BwdGop.iOrg   = 0;
  BwdFast1.ix = 0;  BwdFast1.iOrg = 0;


}



