

#define DBG_RJ

#include "global.h"
#include "GetBlk.h"
#include "getbit.h"
#include "GetBit_Fast.h"
#include "Mpg2Cut2_API.h"

#include <errno.h>
#include <stdio.h>


int iBlk_Bytes_Left, iErrNo;
void File_EndLast();
void Next_File(void);



int Mpeg_READ_Buff(int P_FileNum, 
                  unsigned char *P_Buffer, int P_iRqstLen, 
                  int P_Caller) 
{
  int iReturnLen;

  // ???_Point_Before = i64tell( ....

    errno = 0;

    iReturnLen =   _read(FileDCB[P_FileNum], P_Buffer, P_iRqstLen);

    iErrNo = errno;
    if (iErrNo  
       ) // && (iErrNo != EBADF || !iReturnLen))  // Strange thing I don't undestand
    {
      ERRMSG_File("MpegIN", 'r',   // READ ERROR
                             errno, // FileDCB[iP_File],
                             File_Name[P_FileNum], P_FileNum, P_Caller) ;

    }


  // ???_Point_After = i64tell(....

  return iReturnLen;
}


unsigned char *lpEOB;
  int   iAHD_Len;
__int64 i64Telly; 


//-----------------------------------------
void Mpeg_Read_AHEAD()
{
  __int64 i64ToRead; //, i64AtLoc;
  int     iRqstLen, iTime1, iTimeDiff, iTimeHurdle;
  //unsigned char *RdADJ;

  if (RdAHD_Flag)
  {
      RdAHD = RdAHDBFR[iRdAHD_NextIx];
  }
  else
      RdAHD = RdBFR;


  lpEOB = RdAHD; 

  iBlk_Bytes_Left = MPEG_SEARCH_BUFSZ;

Remaining_Calc:
  iRqstLen = iBlk_Bytes_Left;

  if (iRdAHD_File < 0 || iRdAHD_File > MAX_FILE_NUMBER)
  {
      MessageBox ( NULL, "Wrong iRdAHD_File", "Mpg2Cut2 - BUG !",
                 MB_OK | MB_ICONEXCLAMATION
                       | MB_SETFOREGROUND );

      MParse.Fault_Flag = CRITICAL_ERROR_LEVEL;
      MParse.Stop_Flag = 9;
      iRdAHD_File = 0;
  }


  i64Telly = _telli64(FileDCB[iRdAHD_File]);

    iRdAHD_TellBefore_File[iRdAHD_NextIx] = iRdAHD_File;
  i64RdAHD_TellBefore_Loc [iRdAHD_NextIx] = i64Telly;

  if (process.Action == ACTION_RIP
  &&     iRdAHD_File == process.endFile)
  {
      i64ToRead = process.endLoc - i64Telly;
      if (i64ToRead < iBlk_Bytes_Left)
          iRqstLen = (int)(i64ToRead);
  }


//#ifdef DBG_RJ
     if (DBGflag)
     {
       sprintf(szBuffer,"\nREAD Req=%d  File#%d", iRqstLen, iRdAHD_File);
       DBGout(szBuffer);
     }
//#endif



  iAHD_Len = 0;

  if (iRqstLen > 0)
  {
     iTime1 =  iCURR_TIME_ms();

     iAHD_Len = Mpeg_READ_Buff(iRdAHD_File, RdAHD, iRqstLen, 6661);

     iTimeDiff =  iCURR_TIME_ms() - iTime1;

//#ifdef DBG_RJ
     if (DBGflag)
     {
       sprintf(szBuffer,"\n     GOT=%d  %02dms", iAHD_Len, iTimeDiff);
       DBGout(szBuffer);
     }
//#endif

     if (process.Action == ACTION_RIP)
       iTimeHurdle = 100;
     else
       iTimeHurdle = 200;
     if (iTimeDiff > iTimeHurdle)
     {
        if (iTimeDiff > (iTimeHurdle<<2))
            strcpy(szMsgTxt,"DISK PROBLEM"); // Could be failing Hard Drive OR drive had powered down since last use OR Fragmentation OR Competing Task
         else
            strcpy(szMsgTxt,"DISK SLOW"); // Could be failing Hard Drive OR drive had powered down since last use OR Fragmentation OR Competing Task

        DSP1_Main_MSG(0,0);
     }

     process.BlksSinceLastEffect++;
     if (process.BlksSinceLastEffect > 100)
     {
        strcpy(szMsgTxt,"Searching..."); // Twinhan - Megabytes of nulls
        process.BlksSinceLastEffect = 0;
        DSP1_Main_MSG(0,0);
        GetChkPoint();
     }
  }



  lpEOB           += iAHD_Len; 
  iBlk_Bytes_Left -= iAHD_Len;

  if (iBlk_Bytes_Left > 0)  // Room for more ?
  {
     MessageBeep(MB_OK);

#ifdef DBG_RJ
     if (DBGflag)
     {
       sprintf(szBuffer,"\nREAD BREAK - Got=%d  Want=%d File#%d Last=%d",
                              iAHD_Len, iRqstLen, iRdAHD_File, File_Final);
       DBGout(szBuffer);
     }
#endif

     //if (process.Action != ACTION_RIP
     //||  iRdAHD_File < process.endFile)
     //    Next_File();                   // Try next file
     //else
     if (iAHD_Len < iRqstLen          // EOF
     || (_telli64(FileDCB[iRdAHD_File]) >= process.length[iRdAHD_File]) )  // EOF
     {
         if (iRdAHD_File < File_Final)
         {
             Next_File();                   // Try next file
             //goto Remaining_Calc;
          }
          else
          if (RdAHD_Flag)
              RdAHD_EOF = 1;
          else
              File_EndLast();
     }
     else
     if (process.Action == ACTION_RIP
     && (iPreview_Clip_Ctr < iEDL_ctr // MULTI-CLIP PREVIEW ?
         //|| (iCtl_Play_Summary && ! MParse.Summary_Section)  // Allow for summary mode final tail
         ))
     {
         C160_Clip_Preview();
         iRdAHD_File = process.startFile;
         _lseeki64(FileDCB[iRdAHD_File], process.startLoc, SEEK_SET );
         i64Telly = process.startLoc;

         goto Remaining_Calc;
     }
     else
     {
         MParse.Fault_Flag = 97;
         Write_Frame(NULL, d2v_curr, 0);
     }
  }

  if (MParse.Summary_Adjust < 0)
  {
      MParse.Summary_Adjust = 0;
      if (process.endLoc  > 8192)
          process.endLoc -= 8192;
  }

  i64Telly = _telli64(FileDCB[iRdAHD_File]);

  i64RdAHD_TellAfter_Loc [iRdAHD_NextIx] = i64Telly;
    iRdAHD_TellAfter_File[iRdAHD_NextIx] = iRdAHD_File;
    iRdAHD_DataLen       [iRdAHD_NextIx] = lpEOB - RdAHD;

}


//-----------------------------------------
void Next_File()
{
  int iRqstLen, iReadLen; // , i;
  //  unsigned char *Startptr;

  //if (DBGflag)  DBGout("NEXT-FILE");

  if (iRdAHD_File  < File_Final)
  {
    iRdAHD_File++ ;

    process.run = process.origin[iRdAHD_File];
    //process.run = 0;
    //for (i=0; i < iRdAHD_File ; i++)
    //     process.run += process.length[i];

    //DSP5_Main_FILE_INFO();
    PostMessage(hWnd_MAIN, RJPM_UPD_MAIN_INFO, 0, 0); // Avoid Deadlock

    _lseeki64(FileDCB[iRdAHD_File],  0, SEEK_SET);
    i64Telly = 0;

    iRqstLen = iBlk_Bytes_Left;

    iReadLen = Mpeg_READ_Buff(iRdAHD_File, // _read(FileDCB[iRdAHD_File], 
                             lpEOB, iRqstLen, 6662);

    iBlk_Bytes_Left -= iReadLen;

#ifdef DBG_RJ
    if (DBGflag)
    {
       sprintf(szBuffer,"\n     MORE  - Got=%d  Want=%d Rem=%d  End=%d.x%08X", //  File#%d Last=%d",
                              iReadLen, iRqstLen, iBlk_Bytes_Left,
                              process.endFile, (int)(process.endLoc)); //, iRdAHD_File, File_Final);
       DBGout(szBuffer);
    }
#endif


    lpEOB += iReadLen; 

    if (iReadLen < iRqstLen )
    {
        MessageBox( NULL, "Input file smaller than 1 block", szAppName,
                   MB_OK | MB_ICONEXCLAMATION
                 | MB_SETFOREGROUND | MB_TOPMOST);
    }
    /*
    else
    if (iReadLen > 3
      && *( (UNALIGNED DWORD*) (Startptr) ) == 0x46464952) // 'RIFF')
    {
        strcpy(szMsgTxt,"** TROJAN AVI file ***");
        MParse.Fault_Flag = 98;
    }
    */
  }
  else
  {
    if (RdAHD_Flag)
        RdAHD_EOF = 1;
    else
        File_EndLast();
  }
}





//-----------------------------------------

void Mpeg_READ()
{
  int iRdPTR_Offset;
  int iRdEOP_Offset;

  iRdPTR_Offset = RdPTR    - RdBFR; 
  iRdEOP_Offset = RdEndPkt - RdBFR; 

  if (!RdAHD_Flag)
  { 
    Mpeg_Read_AHEAD();
  }
  else
  {

    // Load up the Ahead buffers

    while (iRdAHD_NextIx != iRdAHD_CurrIx 
           && !RdAHD_EOF 
           // && !iAUD_Enough
           )
    {
      iRdAHD_NextIx++;
      if (iRdAHD_NextIx >= RD_AHD_MAX)
          iRdAHD_NextIx  = 0;
      Mpeg_Read_AHEAD();

      if (MParse.FastPlay_Flag > MAX_WARP)
        break;
    }
    
    // Deliver oldest buffer

    if (iRdAHD_NextIx == iRdAHD_CurrIx && RdAHD_EOF)
        File_EndLast();
    else
    {
      iRdAHD_CurrIx++;
      if (iRdAHD_CurrIx >= RD_AHD_MAX)
          iRdAHD_CurrIx  = 0;
      RdBFR = RdAHDBFR[iRdAHD_CurrIx];
    }

  } // end-else Read Ahead
     
   File_Ctr       =   iRdAHD_TellAfter_File[iRdAHD_CurrIx];
   MParse.NextLoc = i64RdAHD_TellAfter_Loc [iRdAHD_CurrIx];
   File_ReadLen   =   iRdAHD_DataLen       [iRdAHD_CurrIx];

   RdPTR          =  RdBFR + iRdPTR_Offset; 

   RdEOB          =    RdBFR + File_ReadLen; 
   RdEOB_4 = RdEOB-4;  RdEOB_8 = RdEOB-8;

   RdEndPkt = RdBFR + iRdEOP_Offset; 
   RdEndPkt_4 = RdEndPkt-4; RdEndPkt_8 = RdEndPkt-8;

  //RdGOT = RdEOB - RdAHD;

  // Look for Trojans
  if (File_ReadLen > 3 && !RdAHD_EOF
  && (*(DWORD*)(RdBFR)) == 0x46464952 // 'RIFF')
  && !iPES_Mpeg_Any  && !process.iOut_DropCrud)
  {
      strcpy(szMsgTxt,"** TROJAN AVI file ***");
      MParse.Fault_Flag = 88;
      Mpeg_KILL(1001);
  }

}



//----------------------------------------

void Mpeg_READ_Adjust()
{
  // , iOldBlkLen;
  // iOldBlkLen = File_ReadLen;
  RdPTR     -= MPEG_SEARCH_BUFSZ; // iOldBlkLen;

  Mpeg_READ();

  RdEndPkt -=  MPEG_SEARCH_BUFSZ; // iOldBlkLen;
  RdEndPkt_4 = RdEndPkt-4; RdEndPkt_8 = RdEndPkt-8;
}


void Mpeg_READ_Bug_Adj()
{
      RdEndPkt   = RdEndPkt - RdPTR + RdBFR; 
      RdEndPkt_4 = RdEndPkt-4;  RdEndPkt_8 = RdEndPkt-8;
      RdPTR    = RdBFR;
}


   //    OpenDVD stuff - I suspect
   //    if (KeyOp_Flag && (RdAHD[20] & 0x10))
   //    {
   //      BufferOp(RdAHD, lfsr0, lfsr1);
   //      RdAHD[20] &= ~0x10;
   //    }


#include "Nav_JUMP.h"

void File_EndLast()
{
  __int64 i64SCR;

  if (DBGflag)  DBGout("*END_LAST*");

  if (process.Action > ACTION_PLAY)
  {
     lpMpeg_TC_ix2 = (unsigned char *)(&process.CurrSSCRM[0]);
     SCRM_2SCR((unsigned char *)(&i64SCR));
     process.VideoPTS = (unsigned int)(i64SCR>>1);
  }


  MParse.Fault_Flag = 98;
  BwdGop.ix = 0; BwdGop.iOrg = 0; // Reset Location Accelerator

  PlayCtl.iEOFhit++;              // Trap problem stream thrashing
  if (PlayCtl.iEOFhit > 10)
    Mpeg_KILL(9969);

  if (RdEOB <= RdEOB-4)
      *(UNALIGNED DWORD*)(RdEOB) = 0;

  if (process.Action >= 0) // Allow for going backwards
      Mpeg_EOF();  // This code should be moved out, once the flow-on effect has been analysed
}




//===========================================


void GetBlk_RdAHD_RESET()
{
  // Clear out read ahead buffers
  //int iTmp1;

  if (iCtl_AudioAhead && process.Action == ACTION_RIP
  &&  MParse.FastPlay_Flag <= MAX_WARP
  &&  iPreview_Clip_Ctr    >= iEDL_ctr)
      RdAHD_Flag = 1;
  else
      RdAHD_Flag = 0;

  if (RdAHD_Flag)
  {
     iRdAHD_CurrIx = RD_AHD_MAX-1;   
     iRdAHD_NextIx = -1;  
  }
  else 
  {
     iRdAHD_CurrIx = 0;   
     iRdAHD_NextIx = 0;
  }

  RdAHD_EOF = 0;
  iRdAHD_File = File_Ctr;
  _lseeki64(FileDCB[File_Ctr],  MParse.NextLoc,  SEEK_SET);


  i64RdAHD_TellBefore_Loc [iRdAHD_CurrIx] = 0;
    iRdAHD_TellBefore_File[iRdAHD_CurrIx] = 0;

  iAUD_Enough = 0;

  /*  NOT WORKING - BUG
  iTmp1 = sizeof(iRdAHD_TellBefore_File);
  memset(&iRdAHD_TellBefore_File, sizeof(iRdAHD_TellBefore_File), -1);
  memset(&iRdAHD_TellAfter_File,  sizeof(iRdAHD_TellAfter_File),  -1);
  memset(&iRdAHD_DataLen,         sizeof(iRdAHD_DataLen),         -1);
  */

}


void GetBlk_AHD_INIT()
{
  int iTmp1, iTmp2;

  RdAHD_malloc = (unsigned char *) malloc(MPEG_SEARCH_BUFSZ * RD_AHD_MAX + 4096);
  // Align on a page boundary
  RdAHDBFR[0] = (unsigned char *)
                 ((((unsigned int)RdAHD_malloc + 4095) / 4096) * 4096); // Align on a page boundary
  iTmp1 = 0;
  for (iTmp2 = 1; iTmp2 < RD_AHD_MAX; iTmp2++)
  {
     RdAHDBFR[iTmp2] = RdAHDBFR[iTmp1] + MPEG_SEARCH_BUFSZ;
     iTmp1 =  iTmp2;
  }

  RdBFR = RdAHDBFR[0];
  if (RdBFR == NULL)
     Err_Malloc(&"X101");

  GetBlk_RdAHD_RESET();
}
