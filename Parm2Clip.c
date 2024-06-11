#include "global.h"
#include "getbit.h"


  __int64 i64OrgSCR, i64EndSCR, i64EndLoc;
  __int64 i64LowSCR, i64HighSCR;
  __int64 i64LowLoc, i64HighLoc;


 int iReadLen;


void C990_Read_8k()
{
  _lseeki64(FileDCB[File_Ctr], process.startLoc, SEEK_SET );

  RdEOB    = RdPTR = RdBFR;
  iReadLen = Mpeg_READ_Buff(File_Ctr,  // _read(FileDCB[File_Ctr],  
                           RdEOB, 8192, 9966);
  RdEOB   += iReadLen;  RdEOB_4 = RdEOB-4;  RdEOB_8 = RdEOB-8;
}


void SplitSCR(unsigned int *P_SCR)
{

   PTS_2Field(*P_SCR, 0);
   sprintf(szBuffer, "    %02dh %02dm %02ds %02df",
                      ptsTC.hour, ptsTC.minute, ptsTC.sec, ptsTC.frameNum);

}


//-------------------------------------------------------

int C920_Get_SCR(__int64 *lpP_SCR)
{

  int iRC;
  iRC = 0;

Part_0_Entry: // Optimization target for skip non-zero
  if (RdPTR < RdEOB) 
  {
      if (*RdPTR++) 
          goto Part_0_Entry; // Optimize compilation 
      else
      {
         if (RdPTR < RdEOB) 
         {
            if (*RdPTR++) 
               goto Part_0_Entry;  
            else
            {
Part_2_Entry:
               if (RdPTR < RdEOB) 
               {
                  if (*RdPTR > 1)
                  {
                     RdPTR++;
                     goto Part_0_Entry; 
                  }
                  else
                  if (*RdPTR == 0)
                  {
                     RdPTR++;
                     goto Part_2_Entry; 
                  }
                  else
                  {
                     RdPTR++;
                     if (RdPTR < RdEOB) 
                     {
                        if (*RdPTR != 0xBA) // PACK HDR
                            goto Part_0_Entry; // Optimize compilation
                        else
                        {
                           RdPTR++;
                           if ((RdEOB - RdPTR) >= 6) // Room for SCRM ?
                           {
                              MParse.SystemStream_Flag = 1;
                              lpMpeg_TC_ix2 = RdPTR;
                              SCRM_2SCR((unsigned char *)(lpP_SCR));
                              if (DBGflag)
                              {
                                 SplitSCR((unsigned int *)(lpP_SCR));
                                 DBGout(szBuffer);
                              }
                              iRC = 1;
                           }
                        }
                     }
                  }
               }
            }
         }
      }
  }
  return iRC;
}




// Translate Time Co-ordinate into RBA

void C980_TC2RBA(TC_HMSFR *lpTC, int *lpFile, __int64 *lpLoc, int P_End)
{

  __int64 i64CurrSCR, i64PrevLoc, i64DiffSCR, i64DiffLoc;
  __int64 i64_Want_SCR, i64LowTol, i64HighTol, i64TMP;

  int iProbeLimit;

  *lpFile = 0; 
  *lpLoc  = 0;

  iProbeLimit = 64;
  if (P_End)
  {
     i64LowTol  = 0;
     i64HighTol = 3600; //  45000;
  }
  else
  {
     i64LowTol  = -3600; // -45000;
     i64HighTol = 0;
  }

  // Convert Relative Time to Relative SCR

  i64TMP  = (((lpTC->hour * 60) + lpTC->minute * 60) + lpTC->sec) * 90000;
          + (lpTC->frameNum * 90000 / 30); // Approx frame rate is close enough, since most GOPs are around half a second long
          - 3600;

  i64_Want_SCR = i64TMP;

  // Adust Relative SCR to Absolute

  if (iFromTC_Style)
      i64_Want_SCR += i64OrgSCR;
  
  if (DBGflag)
      DBGln4("REL=%d   ABS=%d ORG=%d  END=%d", 
             i64TMP, i64_Want_SCR, i64OrgSCR, i64EndSCR);

  if (i64_Want_SCR < i64OrgSCR)
  {
      sprintf(szBuffer, "SELECTED TIME preceeds first SCR on file.\n\nSEL=%d\nORG=%d", 
                (int)i64_Want_SCR, (int)i64OrgSCR);
      MessageBox(hWnd_MAIN, szBuffer, szAppName,  MB_OK);
      i64_Want_SCR = i64OrgSCR;
  }
  if (i64_Want_SCR > (i64EndSCR + 45000))
  {
      sprintf(szBuffer,"SELECTED TIME exceeds last SCR on file.\n\nSEL=%d\nEof=%d",  
                (int)i64_Want_SCR, (int)i64EndSCR);
      MessageBox(hWnd_MAIN,  szBuffer, szAppName,  MB_OK);
      if (P_End)
          i64_Want_SCR = i64EndSCR;
      else
      {
        iFromTC_Style = 0;  // Try Absolute style instead
        i64_Want_SCR -= i64OrgSCR;
      }
  }

  // Do a weighted binary search
 
  i64LowSCR  = i64OrgSCR;
  i64HighSCR = i64EndSCR;

  process.startLoc = 0;
  i64LowLoc  = 0;
  i64HighLoc = i64EndLoc;

  i64CurrSCR = i64_Want_SCR;

Probe:
  i64PrevLoc = process.startLoc;
  process.startLoc = (( (i64_Want_SCR - i64LowSCR) 
                      * (i64HighLoc   - i64LowLoc))
                      / (i64HighSCR   - i64LowSCR))
                                      + i64LowLoc;
  C990_Read_8k();
  C920_Get_SCR(&i64CurrSCR);

  i64DiffSCR = i64CurrSCR - i64_Want_SCR;
  i64DiffLoc = process.startLoc - i64PrevLoc;

  if (DBGflag)
      DBGln4("SCR=%d Diff=%d Loc=x%08X Prev=x%08X",
         i64CurrSCR, i64DiffSCR, process.startLoc, i64PrevLoc);


  if (iProbeLimit)
  {
    iProbeLimit--;
    if (i64DiffSCR < i64LowTol)
    {
       i64LowSCR = i64CurrSCR;
       i64LowLoc = process.startLoc;
       goto Probe;
    }
    else
    if (i64DiffSCR > i64HighTol)
    {
       i64HighSCR = i64CurrSCR;
       i64HighLoc = process.startLoc;
       goto Probe;
    }

  }

  *lpFile = 0; 
  *lpLoc  = process.startLoc;

}





// Convert Parm seletion times to a clip

void C905_Parm2Clip()
{

  i64OrgSCR = 0;
  i64EndSCR = 0x01FFFFFFFF;

  i64EndLoc = process.length[File_Final];

  //  Get the First SCR on the file
  process.endLoc   = 8192;
  process.startLoc = 0;
  if (process.startLoc < 0)
      process.startLoc = 0;

  File_Ctr = 0;

  C990_Read_8k();
  C920_Get_SCR(&i64OrgSCR);


  //  Get the Last SCR on the file
  process.endLoc   = i64EndLoc;
  process.startLoc = process.endLoc - 8192;
  if (process.startLoc < 0)
      process.startLoc = 0;

  File_Ctr = File_Final;

  C990_Read_8k();
  C920_Get_SCR(&i64EndSCR);

  if (DBGflag)
      DBGln2("\nORG=%d\nEND=%d\n", i64OrgSCR, i64EndSCR);


  // Check that SCR is at least a littl bit sensible
  if (i64EndSCR <= i64OrgSCR)
  {
      MessageBox(hWnd_MAIN, 
        "SCR on input file is NOT a consistent time base.\n\nNOT COMPATIBLE WITH THIS MODE.", 
                                                        szAppName,  MB_ICONSTOP | MB_OK);
      return;
  }

  // Translate FROM time into RBA

  if (FromTC.hour < 0)
  {
      process.FromFile = 0;
      process.FromLoc  = 0;
  }
  else
      C980_TC2RBA(&FromTC, &process.FromFile, &process.FromLoc, 0);


  // Translate TO   time into RBA

  if (ToTC.hour < 0)
  {
     C140_Clip_EOF();
  }
  else
     C980_TC2RBA(&ToTC, &process.ToViewFile, &process.ToViewLoc, 1);

  process.ToPadFile = process.ToViewFile;
  process.ToPadLoc  = process.ToViewLoc;

  T590_Trackbar_SEL();
  DSP2_Main_SEL_INFO(0);
  Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act = ']';
  // Add the selection to the Clip list
  C350_Clip_ADD('+', 0);

}


//---------------------------------------------------------


DWORD WINAPI  C900_Parm2Clip(LPVOID n)
{
  DWORD dRC;
  int iPre;

  // Load each file name previously extracted from the parameter line
  for (iPre = 0; iPre < File_PreLoad; iPre++)
  {
     strcpy(szInput, File_Name[iPre]);  
     F100_IN_OPEN('a', 0);
     Sleep(25);
  }
  iParmConfirm = 1;


  // Process Selection Info extracted from Parm area, if present

  if (FromTC.hour >= 0   &&  File_Limit)
  {
     // Check that there is no decode in progress

     dRC = WaitForSingleObject(hThread_MPEG, 0);
     if (dRC != WAIT_OBJECT_0 
     &&  dRC != 0xFFFFFFFF )
     {
         sprintf(szMsgTxt, "* MPEG Wait RC=x%04X",  dRC);
         if (DBGflag)
             DBGout(szMsgTxt);

         DSP1_Main_MSG(0,0);
     }
     else
     {
         C905_Parm2Clip();
          
         // SAVE the Selection

         if (szOutParm[0])
             strcpy(szOutput, szOutParm);           
         OUT_SAVE('L') ;  // SAVE ALL CLIPS in EDL
     }
  }

  FromTC.hour = -1;  ToTC.hour = -1; 

  if (DBGflag)
      DBGctl();

  return 0;
}





/*
    iKick.File    = ?? ;
    iKick.Loc     = process.?? - 8192; 

    if (iKick.Loc >= process.length[iKick.File])
    {
        iKick.Loc -= MPEG_SEARCH_BUFSZ;
    }

    iKick.Action  = ACTION_NEW_CURRLOC ;
    process.CurrFile = iKick.File ;
    process.CurrLoc  = iKick.Loc  ;

    MPEG_processKick();
*/
