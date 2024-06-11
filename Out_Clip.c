//
//       MPEG OUTPUT PACKET MODULE
//
// Contains the main routines for handling output clips / ranges
//
//
// I don't know how to tell "C" that
// this is a separately compiled sub-routine
// so it is still an "include" of an include of Out_PKTS.c
//

//#include "windows.h"
//#include "global.h"
//#include <commctrl.h>
//#include "out.h"


// #define RJDBG_PTS 1

 


//-------------------------------------------------------------

  __int64 i64_CurrPos,  i64_CurrEnd;
  __int64 i64_ClipTotBytes;
      int iCurrFile; //, EndFile ;






#include "Out_Hdrs.c"



//----------------------------

/*   uSSCR1 = ((unsigned int)(*lpSSCRM   & 0x38)<<26)
            + ((unsigned int)(*lpSSCRM++ & 0x03)<<27)
            + ((unsigned int)(*lpSSCRM++       )<<27)

*/


//-------------------------------------------------------------------
int iMpeg_Copy_BufLimit, iMpeg_Read_Len, iMpeg_Diff_Len;
int iRange_FirstBlk;



#include "Out_BUF.c"

void Out_Preamble_TS()
{
  
  // May need to create TS File Preamble

      if (DBGflag)
      {
        sprintf(szDBGln, "TS Preamble len=%d  pkts=%d",
                         process.PAT_Len, (process.PAT_Len/188));
        DBGout(szDBGln);
        strcpy(szMsgTxt, szDBGln);
      }

      iTmp1 = EDList.FromFile[iEDL_ClipFrom];
      if (process.PAT_File  < iTmp1
      || (process.PAT_File == iTmp1
          && process.PAT_Loc >= 0
          && process.PAT_Loc < EDList.FromLoc[iEDL_ClipFrom]) )
      {
         Out_RECORD(&PAT_Data[0], process.PAT_Len, 8047);
      }

}



//-------------------------------------------------------------------
/* static */
int Out_RANGE(int  P_FromFile, __int64 P_StartPos,
                     int  P_ToFile,   __int64 P_EndPos,
                     char P_Preamble_Only)
{
  int iRC;

  int iTmp1; //, iTmp2, iTmp3;

  iPreambleOnly_Flag = P_Preamble_Only;

  if (iCtl_Out_Parse || process.iOutUnMux)
      uBroken_Flag = (unsigned)iCtl_SetBrokenGop<<5; // Mpeg Broken Link flag is in a higher order bit.
  else
      uBroken_Flag = 0;

  iRange_FirstBlk = 1;

  ZeroMemory(iStartCodePart, sizeof(iStartCodePart));

  i64_CurrPos  = P_StartPos ;

  // Remember things for percentage calcs

  i64_ClipTotBytes = P_EndPos - P_StartPos;
            // + process.origin[P_ToFile] -  process.origin[P_FromFile];

  // Calc clip length allowing for multiple files
  iTmp1 = P_FromFile;
  while (iTmp1 < P_ToFile)
  {
     i64_ClipTotBytes += process.length[iTmp1];
     iTmp1++;
  }

  iClipMB = (int)(i64_ClipTotBytes / 1048576);

  //RJDBG
  if (DBGflag)
  {
    DBGln4("Out_RANGE. startpos=x%08X, endpos=x%08X  Diff=%dKB Pre=%d\n",
                          P_StartPos,  P_EndPos, (i64_ClipTotBytes/1024),
                                                           iPreambleOnly_Flag);
  }

  iProgress_Prev_pct = 0;

                                   // MOVED Dialog creation away from here

  // a selection may cover multiple input files
  for ( iCurrFile  = P_FromFile  ;
       (iCurrFile <= P_ToFile && !Out_CanFlag );
        iCurrFile++)

  {
    i64_CurrEnd = process.length[iCurrFile];
    if (iCurrFile == P_ToFile  &&  P_EndPos < i64_CurrEnd)
        i64_CurrEnd   = P_EndPos ;


    //if (P_EndPos > 0) P_EndPos-- ;
    // restart at Exact FROM position

    _lseeki64(FileDCB[iCurrFile], i64_CurrPos, SEEK_SET);


    if (DBGflag)
        DBGln2("            Currpos=x%08X, CurEnd=x%08X",
                      (int)(i64_CurrPos), (int)(i64_CurrEnd));

    Out_Progress_Chk(0);

    // Copy the selected data
    while (i64_CurrPos < i64_CurrEnd)
    {

       iOut_Breathe_Tot = 0;

       iTmp1 = 1;
       Out_Priority_Chg(0);

      // read a BIG block of data
      iRC = Mpeg_BIG_READ(lpMpeg_Copy_Buffer, 8051);

      // abandon on bad read
      if (iRC)
      {
         break;
      }


      // First block of clip
      // may need headers added at start

      if (iPS_Block1)
      {
          iPS_Block1 = 0;


          if (iOut_SystemStream_Flag > 0)
              Out_PS_File_Hdrs();
          else
          // May need to create TS File Preamble
          if (iOut_SystemStream_Flag == -1)  // Transport stream (TS) ?
             //&&  ! process.iOutUnMux)
          {
             Out_Preamble_TS();
          }

          // Minimum preamble means only copy as far as System Header
          if (iPreambleOnly_Flag && iCtl_Out_Preamble_Flag < 2)
          {
              lpMpeg_FROM = lpMpeg_EOD;
              i64_CurrPos = i64_CurrEnd;          // <==== SET FOR LATER ESCAPE
          }

      }  // ENDIF first output block of file

      // output the rest of the big block

      iMpeg_ToDo_Len = lpMpeg_EOD - lpMpeg_FROM;
      if (iMpeg_ToDo_Len > 0   &&  !Out_CanFlag)
      {
          if ((  (iCtl_Out_Parse   &&  iRange_FirstBlk) 
               || process.iOutParseMore)
          &&  !iPreambleOnly_Flag)
          {
             if (MParse.SystemStream_Flag > 0)
             {
                 iOut_Target_Tail = 0;
                 Out_FILTER();
             }
             else
             if (MParse.SystemStream_Flag == 0)
             {
               // Mpeg Elementary Video only
               if (uBroken_Flag || iOut_Fix_Hdrs_Vid)
               {
                   lpMpeg_ix3 = lpMpeg_FROM;
                   lpMpeg_End_Packet = lpMpeg_EOD;  // Simulate a very large packet
                   iOut_Target_Tail = 0;
                   Out_Vid_Hdr_SCAN(4); // Scan all video data
               }
             }

             // update iMpeg_ToDo_Len to match
             iRange_FirstBlk = 0;

          } // ENDIF Parsing 1st block of range

          // output rest of BIG buffer NOT copied during filtering
          if (iMpeg_ToDo_Len > 0 && !Out_CanFlag && !process.iOutUnMux)
          {
             iRC = Out_RECORD(lpMpeg_FROM, iMpeg_ToDo_Len, 8006);
             if (iRC < 0)
               return iRC;
          }
      }



      // Make sure we have done provided enough breathing
      iOut_Breathe_Tot = iOut_Breathe_PerBigBlk - iOut_Breathe_Tot;
      if (iOut_Breathe_Tot > 0)
      {
          if (iOut_Breathe_Tot > 10000) // Limit 10 seconds
              iTmp1 = 10000;
          else
              iTmp1 = iOut_Breathe_Tot;
          Sleep(iTmp1); // Allow other tasks to breathe
      }

      if (Out_CanFlag)
      {
         break;
      } 



    } // ENDWHILE i64_CurrPos < P_EndPos

    i64_CurrPos = 0 ; // Reset pointer for next file

  } // endfor (iCurrFile++)


  //if (iProgress_Prev_pct < 95)
      Out_Progress_Chk(1);

  //  GRAB EXTRA AUDIO TO REACH VIDEO PTS OUT-TIME

        // Need to move this code inside the filter, to allow for HJSPLIT
  if ((iCtl_Out_Parse 
       && (!iPreambleOnly_Flag   || process.iSEQHDR_NEEDED_clip1
          ))
  && !Out_CanFlag)
  {
    if (MParse.SystemStream_Flag > 0)
    {
      i64_CurrPos = i64_CurrEnd;
      //iRC = Mpeg_BIG_READ(lpMpeg_Copy_Buffer, 8936);
      iCurrFile      = P_ToFile;
      iMpeg_ToDo_Len = iMpeg_Copy_BufSz;
      iMpeg_Read_Len = Mpeg_READ_Buff(iCurrFile,  // _read(FileDCB[iCurrFile], 
                                     lpMpeg_Copy_Buffer,  iMpeg_ToDo_Len, 8863);

      lpMpeg_EOD = lpMpeg_Copy_Buffer + iMpeg_Read_Len;
      lpMpeg_EOI = lpMpeg_EOD - CPY_EOI_CUSHION; // 256;

      // Allow for break across files  (REWORK THIS INTO FILTER)
      if (iMpeg_Read_Len < (iMpeg_ToDo_Len - 8192)
      //  && !iRC
          &&   iCurrFile < File_Final)
      {
          //iRC = Mpeg_BIG_READ(lpMpeg_EOD, 8937);
          iMpeg_ToDo_Len = iMpeg_Copy_BufSz - iMpeg_Read_Len;
          iMpeg_Read_Len = Mpeg_READ_Buff(iCurrFile, // _read(FileDCB[iCurrFile],
                                 lpMpeg_EOD, iMpeg_ToDo_Len, 8864);
          lpMpeg_EOD = lpMpeg_EOD + iMpeg_Read_Len;
          lpMpeg_EOI = lpMpeg_EOD - CPY_EOI_CUSHION; // 256;
      }

      // If there is at least one pack extra, then filter it
      if (lpMpeg_EOD > (lpMpeg_Copy_Buffer + 2048))
      {
          lpMpeg_FROM = lpMpeg_Copy_Buffer;

          if (iPreambleOnly_Flag)
              i64PrimaryTrigger_PTSM = 0;
          else
              i64PrimaryTrigger_PTSM = W_Clip.ToPTSM;

          iOut_Target_Tail = 1;
          Out_FILTER();
      }
    }
    else
    if (MParse.SystemStream_Flag == 0)  // Mpeg Elementary Video only
    {
      if (uBroken_Flag || iOut_Fix_Hdrs_Vid)
      {
          lpMpeg_End_Packet = lpMpeg_EOD;  // Simulate a very large packet
          iOut_Target_Tail = 0;
          Out_Vid_Hdr_SCAN(4); // Scan all video data
      }
    }

  }

  iOut_Clip_ctr++;

  return 0;
}

