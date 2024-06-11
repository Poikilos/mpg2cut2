
#include "global.h"

char szAUTOname[_MAX_PATH];

void  C888_AutoFile(int P_SysTemp)
{
  // Set Auto-Recovery File for EDL

  char *lpInto, *lsSlash;
  int iLen;
  SYSTEMTIME st;

  if (P_SysTemp)
  {
      iLen = GetEnvironmentVariable("TEMP", szTemp, sizeof(szTemp));
      if (iLen < 1)
      { 
         strcpy(szTemp, "C:\\TEMP"); //     C:\TEMP
         iLen = 7;
      }

      lpInto  = &szTemp[iLen];

      if (*lpInto == '\\')
      {
          *lpInto--;
      }
      

      lsSlash = lpLastSlash( File_Name[0]);
      if (lsSlash < File_Name[0])
      {
        *lpInto++ = '\\';
      }

      lpInto  = stpcpy0(lpInto, lsSlash);
  }
  else
  {
     lpInto  = stpcpy0(&szTemp[0], File_Name[0]);
  }



  if (*(--lpInto) != '.') // Allow for .
  {
    lpInto--;
    if (*lpInto != '.') // Allow for .M
    {
      lpInto--;
      if (*lpInto != '.') // Allow for .TS
      {
       lpInto--;
       if (*lpInto != '.') // Allow for .MPG
       {
            lpInto--;
         if (*lpInto != '.') // Allow for .MPEG
         {
            if (*lpInto != '.') // Allow for ..MPEG
                 lpInto--;
         }
       }
      }
    }
  }
  // lpInto--;
      
  GetLocalTime(&st);

  sprintf(szTmp32, "_%d_%d%d-%d%d.EDL", 
                   st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

  strcpy(lpInto, &szTmp32[0]);

  strcpy(szAUTOname, szTemp);
  
  return;
}





//--------------------------------------------------------------
int  C800_Clip_FILE(int P_Action, int P_Auto, char P_OpenMode)
{
  int iRC, i, iValCtr, iOkay, iTmp1, iTmp2, iTmp3, iBAD_Ctr;
  unsigned int uTmp1, uTmp2;

  __int64 i64Tmp1; //, i64Tmp2;

  int iEDL_FileBase, iEDL_Base;

  char *lsDOT, *lsSlash, *lsFrom; //, *lsEDLname;
  //char szOpenMode[4];
  char szSentinel, szOpenDesc[8];
  FILE *CUTFile;
  unsigned int *HiLoc1, *HiLoc2, *HiLoc3;
  unsigned int *LoLoc1, *LoLoc2, *LoLoc3;

  iRC = 0;  iBAD_Ctr = 0;

  if (P_OpenMode != 'a')
  {
     iEDL_FileBase = 0;
     iEDL_Base     = 0;
     if (P_Action  < 0)
         strcpy(szOpenDesc, "Save");
     else
         strcpy(szOpenDesc, "Reload");
  }
  else
  {
     iEDL_FileBase = File_Limit;
     iEDL_Base     = iEDL_ctr;
     strcpy(szOpenDesc, "Append");
  }

  
  if (P_Action) // Do something ?
  {
     // Calculate default file name
     
     lsFrom = &szEDLname[0];

     if (P_Auto)
     {
       if (!process.EDL_Used)
          C888_AutoFile(0);   // Invent one
       lsFrom = &szAUTOname[0];
     }
     else
     {
        if (File_Limit 
        && (
             // P_Action < 0 ||      //  SAVE or
             process.EDL_Used <= 0 ))      //   no Specific EDL
        {
            lsFrom = &szInput[0];    // Infer one
        }
     }

     if (&szAUTOname[0]  != lsFrom) // Different variables ?
         strcpy(szAUTOname, lsFrom);
     
     lsDOT = lpDOT(&szAUTOname[0]);

     if (P_Action == SAVE_CHAP
     ||  P_Action == LOAD_CHAP)
         strcpy (lsDOT, ".CHAP");
     else
         strcpy (lsDOT, ".EDL");

     // Ask the user
     if (iParmConfirm && !P_Auto)
         iRC = X800_PopFileDlg(szAUTOname, hWnd_MAIN, P_Action, -1, &"Clip Listing");
     else
     {
         ofn.nFileOffset = 1;
         iRC = 1;
     }

     if (stricmp(szEDLname, szAUTOname))
     {
         strcpy (szEDLprev, szEDLname);
     }
     strcpy (szEDLname, szAUTOname);

     iParmConfirm = 1;
  }
  else
  {
    strcpy (szAUTOname, szEDLname); // ?? WHAT WAS I TRYING TO DO HERE ??? 
    iRC = 1;
  }


  if (! iRC)
      MessageBeep(MB_OK);
  else
  {
    if (P_Action < 0) // SAVE ?
    {
        CUTFile = fopen(szAUTOname, "wb");
        if (! CUTFile
        && (P_Auto && !process.EDL_Used))
        {
           C888_AutoFile(1);   // Invent a different one using TEMP
           CUTFile = fopen(szAUTOname, "wb");
        }

        if (! CUTFile)
        {
            //MessageBox(hWnd, "CANNOT SAVE EDIT LIST", "Mpg2Cut2 - ERROR", MB_OK);
            ERRMSG_File("EDL", 'o', -999, szAUTOname, errno, 2200);
            MParse.EDL_AutoSave = 0;
        }
        else
        {
          if (P_Action == SAVE_EDL)
          {
            fprintf(CUTFile, "%s Edit Decision List\n",
                             szAppName);
            strcpy(szBuffer, szAUTOname);
            lsSlash = lpLastSlash(szBuffer);
            if (lsSlash)
               *lsSlash = 0;
            else
                szBuffer[0] = 0;
            fprintf(CUTFile, "D=\"%s\"\n", szBuffer);

            for (i = 0;  i < File_Limit; i++)
            {
               fprintf(CUTFile, "F=");
               fprintf(CUTFile, "%02d,%08u,%02u,\"%s\"\n", 
                              i, process.length[i], File_Name[i]);
            }

          } // endif SAVE_EDL
          else
            {
               fprintf(CUTFile, "[CHAPTERS]\n");
            }

          C320_Sel2Clip(); // Save the current selection area too

          for (i = 0;  i <= iEDL_ctr; i++)
          {
             if (P_Action == SAVE_EDL)
             {
                fprintf(CUTFile, "C=");
                fprintf(CUTFile, "%02d,%08u,x%08X,x%08X,x%08X TO=%04d,%08u,x%08X,x%08X,x%08X TOX=%04d,%08u,x%08X,x%08X,x%08X\n", 
                        EDList.FromFile   [i],
                        EDList.FromPTS    [i],
                        EDList.FromLoc    [i],
                        EDList.FromPTSM   [i],

                        EDList.ToPadFile  [i],
                        EDList.ToPadPTS   [i],
                        EDList.ToPadLoc   [i], 
                        EDList.ToPadPTSM  [i],

                        EDList.ToViewFile [i],
                        EDList.ToViewPTS  [i],
                        EDList.ToViewLoc  [i],
                        EDList.ToViewPTSM [i]);
                fprintf(CUTFile, "c=");
                fprintf(CUTFile, "%02d,%08u,x%08X,0,0,0,0,0,0,0,0,0,0,0\n", 
                        EDList.FromFile   [i],
                        EDList.uFrom_TCorrection[i],
                        EDList.uFrom_FPeriod_ps [i]);
             }
             else
             {
                iTmp1   = EDList.FromFile [i];
                i64Tmp1 = (process.origin   [iTmp1] 
                           + EDList.FromLoc [i]) 
                         / 2048;
                //PTS_2Field(EDList.FromPTS  [i], 0);

                fprintf(CUTFile, "%02d=%07ums Sector=%08d\n", 
                         (i+1),
                         (EDList.FromPTS[i] /45),
                         //ptsTC.hour, ptsTC.minute, ptsTC.sec, ptsTC.frameNum,
                         (int)(i64Tmp1));                        
             }
          }

          if (P_Action == SAVE_EDL)
          {
             fprintf(CUTFile, "P=");
             fprintf(CUTFile, "%02d,%08u,x%08X,x%08X,x%08X\n", 
                              process.CurrFile,
                              process.CurrPTS,
                              process.CurrLoc, 
                              process.Curr_TSM[0]);

             fprintf(CUTFile, "J="); // Preamble len
             fprintf(CUTFile, "%04d\n", 
                              process.preamble_len);
             fprintf(CUTFile, "I="); // Preamble len
             fprintf(CUTFile, "%d,0,0,0,0,0,0,0\n",  
                              process.iSEQHDR_NEEDED_clip1);
             fprintf(CUTFile, "*END*\n");
             fclose(CUTFile);
             iEDL_Chg_Flag = 0;
          }

        } // ENDIF FILE OPENED
    } // ENDIF SAVE


    else
    {   // MUST BE A LOAD
        iEDL_Reload_Flag = 1;
        CUTFile = fopen(szEDLname, "r");
        if (! CUTFile)
        {
            ERRMSG_File("EDL", 'o', -999, szEDLname, errno, 2201);
        }
        else
        {
          szEDLprev[0] = 0;
          iEDL_ctr = iEDL_Base;

          iValCtr = fscanf(CUTFile, "%[^\n]\n",  // Read 1st line
                                     &szTemp);

          if (P_Action == LOAD_CHAP)
          {
            iValCtr = 1;
             for(;;)
             {
                iValCtr = fscanf(CUTFile, "%d=%dms Sector=%u\n", 
                                       &iTmp1, &iTmp2, &uTmp1);
                if (iValCtr < 3)
                  break;

                i64Tmp1 = uTmp1 * 2048;
                iTmp1 = 0;
                while (i64Tmp1 > process.length[iTmp1]  && iTmp1 < File_Limit)
                {
                   i64Tmp1 -= process.length[iTmp1];
                   iTmp1++;
                }
                EDList.FromFile   [iEDL_ctr] = iTmp1;
                EDList.FromLoc    [iEDL_ctr] = i64Tmp1;
                EDList.FromPTS    [iEDL_ctr] = iTmp2 * 45;
                EDList.FromPTSM   [iEDL_ctr] = 0;

                if (iEDL_ctr > iEDL_Base)
                {
                   iTmp3 = iEDL_ctr - 1;
                   EDList.ToPadFile  [iTmp3] = iTmp1;
                   EDList.ToPadLoc   [iTmp3] = i64Tmp1;
                   EDList.ToPadPTS   [iTmp3] = iTmp2 * 45;
                   EDList.ToPadPTSM  [iTmp3] = 0;

                   EDList.ToViewFile [iTmp3] = iTmp1;
                   EDList.ToViewLoc  [iTmp3] = i64Tmp1;
                   EDList.ToViewPTS  [iTmp3] = iTmp2 * 45;
                   EDList.ToViewPTSM [iTmp3] = 0;
                }

                iEDL_ctr++;
             }

             EDList.ToPadFile  [iEDL_ctr] = File_Final;
             EDList.ToPadPTS   [iEDL_ctr] = 0;
             EDList.ToPadLoc   [iEDL_ctr] = process.length[File_Final];
             EDList.ToPadPTSM  [iEDL_ctr] = 0;

             EDList.ToViewFile [iEDL_ctr] = File_Final;
             EDList.ToViewPTS  [iEDL_ctr] = 0;
             EDList.ToViewLoc  [iEDL_ctr] = process.length[File_Final];
             EDList.ToViewPTSM [iEDL_ctr] = 0;

             szSentinel = '*';
          } // END-IF  LOAD_CHAP
          else
          {
             iValCtr = fscanf(CUTFile, "D=\"%[^\"]\"\n", &szTemp);

             F950_Close_Files('c');
             File_Limit = 0;

             while (iValCtr > 0)
             {
               iValCtr = fscanf(CUTFile, "%c=", &szSentinel);
               if (szSentinel == 'F')
               {
                   iValCtr = fscanf(CUTFile, "%d,%u,%u,\"%[^\"]\"\n", 
                                 &i,  
                                 &uTmp1, &uTmp2, // &i64Temp1, (&i64Temp1+4),
                                 &szInput);
                   if (File_Limit
                   &&  !stricmp(szInput, File_Name[File_Limit]) )
                   {
                       iEDL_FileBase--;
                   }
                   else
                   {
                      iOkay = F500_IN_OPEN_TRY('a'); // F505_IN_OPEN_TST('a');

                      //if (iOkay < 1)
                      //{
                      //    ERRMSG_File("MpegIn", 'o', -999, szInput, errno, 2202);
                      //}

                      process.preamble_len = 4096;  // Default Preamble
                      process.Preamble_Known = 1;
                   }
               }
               else
               if (szSentinel == 'C')
               {
                 if (!File_Limit)
                 {
                      iValCtr    =  0;
                      szSentinel = '*';
                      break;
                 }

               // "fscanf" does NOT handle __int64 the same way as "fprintf"
               // so we will simulate the same sort of approach

                  LoLoc1 = &(unsigned int)(EDList.FromLoc   [iEDL_ctr]);
                  HiLoc1 = LoLoc1 +1;

                  LoLoc2 = &(unsigned int)(EDList.ToPadLoc  [iEDL_ctr]);
                  HiLoc2 = LoLoc2 +1;

                  LoLoc3 = &(unsigned int)(EDList.ToViewLoc [iEDL_ctr]);
                  HiLoc3 = LoLoc3 +1;

                  iValCtr = fscanf(CUTFile, "%d,%u,x%X,x%X,x%X TO=%d,%u,x%X,x%X,x%X TOX=%d,%u,x%X,x%X,x%X\n", 
                       &EDList.FromFile   [iEDL_ctr],
                       &EDList.FromPTS    [iEDL_ctr],
                       LoLoc1, HiLoc1,
                       &EDList.FromPTSM   [iEDL_ctr],

                       &EDList.ToPadFile  [iEDL_ctr],
                       &EDList.ToPadPTS   [iEDL_ctr],
                       LoLoc2, HiLoc2,
                       &EDList.ToPadPTSM  [iEDL_ctr],

                       &EDList.ToViewFile [iEDL_ctr],
                       &EDList.ToViewPTS  [iEDL_ctr],
                       LoLoc3, HiLoc3,
                       &EDList.ToViewPTSM [iEDL_ctr] );

                   if (DBGflag)
                   {
                      sprintf(szDBGln, "  EndClip=x%08X x%08X\n",
                        EDList.ToViewLoc[iEDL_ctr]);
                      DBGout(szDBGln);
                   }

                   EDList.FromFile   [iEDL_ctr] += iEDL_FileBase;
                   EDList.ToViewFile [iEDL_ctr] += iEDL_FileBase;
                   EDList.ToPadFile  [iEDL_ctr] += iEDL_FileBase;

                   if (iValCtr > 14)
                      iEDL_ctr++;
                   else
                   {
                      sprintf(szBuffer, "BAD CLIP (#%d)", iEDL_ctr);
                      MessageBox(hWnd_MAIN, szBuffer,  "Mpg2Cut2 - BUG", MB_OK);
                      iValCtr = 0;
                   }
               }
               else
               if (szSentinel == 'c')  // LOWER-CASE = extension to clip info
               {
                   fscanf(CUTFile, "%02d,%08u,x%08X,0,0,0,0,0,0,0,0,0,0,0\n",
                        &iTmp1,
                        &EDList.uFrom_TCorrection[iEDL_ctr],
                        &EDList.uFrom_FPeriod_ps [iEDL_ctr]);
               }
               else
               if (szSentinel == 'P') // Position
               {
                   LoLoc1 = &(unsigned int)(process.CurrLoc);
                   HiLoc1 = LoLoc1 +1;

                   //fscanf(CUTFile, "P=");
                   fscanf(CUTFile, "%02d,%08u,x%08X,x%08X,x%08X\n", 
                              &process.CurrFile,
                              &process.CurrPTS,
                              LoLoc1, HiLoc1,
                              &process.Curr_TSM[0]);

                   if (process.CurrFile < File_Limit)
                   {
                      process.startLoc = process.CurrLoc;
                      File_Ctr = process.startFile = process.CurrFile;
                      _lseeki64(FileDCB[File_Ctr],  process.CurrLoc,  SEEK_SET);
                      MParse.NextLoc = process.CurrLoc;
                   }

               }
               else
               if (szSentinel == 'J') // Preamble len
               {
                   //fscanf(CUTFile, "J=");   // Preamble len
                   iTmp1 = 0;
                   fscanf(CUTFile, "%04d\n", &iTmp1); // 32 bit
                   process.preamble_len = iTmp1;  // 64 bit
                   process.Preamble_Known = 1;
               }
               else
               if (szSentinel == 'I')
               {
                   fscanf(CUTFile, "%d,0,0,0,0,0,0,0\n", 
                              &process.iSEQHDR_NEEDED_clip1);
               }
               else
               {
                 iValCtr = 0;
               }

             } // END-WHILE

          }  // END-ELSE  NORMAL_LOAD

          fclose(CUTFile);
          iEDL_Chg_Flag = 0;

          if (iEDL_ctr > 0)
          {
              if (P_Action == LOAD_EDL)
                 iEDL_ctr--;

              if (File_Limit > EDList.ToPadFile[iEDL_ctr])
              {
                  C550_Clip2Selection(); // Restore the selection area
                  Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act = ' ';
                  iEDL_ctr--;
              }

              Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act = ']';
              if (Add_Automation > 0)
                 C350_Clip_ADD('P', 0);
          }

          C000_Clip_TOTAL_MB('a');

          if (szSentinel == '*')
          {
              sprintf(szBuffer, "%sed: %d Clips, %d Files",
                                           szOpenDesc,
                                          (iEDL_ctr-iEDL_Base),
                                          (File_Limit-iEDL_FileBase) );

              if (P_Action == LOAD_EDL)
                 process.EDL_Used = 1;
          }
          else
          {
             if (szSentinel > ' ')
             {
                ZeroMemory(&szTmp32, 32);
                fread(&szTmp32, 1, 16, CUTFile);

                sprintf(szBuffer, "Unknown EDL CTL: %c %s", 
                                                    szSentinel, szTmp32);
                iBAD_Ctr++;
             }

             fscanf(CUTFile, "\n");
          }

          if (iBAD_Ctr)
              MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2", MB_OK);
          else
          {
              strcpy(szMsgTxt, szBuffer);
              DSP1_Main_MSG(1,0);
          }

          MParse.SeqHdr_Found_Flag = 0;
          if (P_Action && File_Limit) // INTERNAL INVOKATION ?
              F150_File_Begin('o');

          iEDL_Chg_Flag = 0;

          //if(threadId_MPEG)
          //   WaitForSingleObject(hThread_MPEG, 0);
          //B570_GO_FROM();
        } // ENDIF FILE OPENED
    } // END-ELSE LOAD
    
    if (! P_Auto) //  && !iRC)
    {
      strcpy (szOutput, szEDLname);
    }

  } // ENDIF FILE wanted

  if ( //!iRC &&
     (    P_Action == LOAD_EDL
       || P_Action == SAVE_EDL))
  {
      if (P_Auto && process.EDL_Used <= 0)
         process.EDL_Used = -1;
      else
         process.EDL_Used =  1;
  }

  INI_SAVE();   // Remember where we were

  return iRC;
}


