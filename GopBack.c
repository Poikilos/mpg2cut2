
#define DBG_RJ

#include  "global.h"


//---------------------------------------------------
void GOPBack()   // BwdGop
{
      int  iSearchFile,  iEndSrch_File,  iRC,  iAT_File  ;
  __int64  i64Search_Loc,  i64EndSrch_Loc,  BackJump,  iAT_Loc,  i64Rate;
  int iLooking, iLoopLimit;

  GetBlk_RdAHD_RESET();


  MParse.FastPlay_Flag = 0;
  MParse.SlowPlay_Flag = 0;
  PlayCtl.iDrop_Behind = 0;
  PlayCtl.iDrop_B_Frames_Flag = 0;

  if (MParse.Stop_Flag)
  {
      MParse.Fault_Flag = CRITICAL_ERROR_LEVEL;
      return;
  }


  iSearchFile   =  process.startFile;
  i64Search_Loc =  process.startLoc;

  i64Rate  =  process.ByteRateAvg[process.startFile];

   
  BackJump =  i64Rate / 4;

  if (Coded_Pic_Height < 576)
  {
     if (BackJump  >  MPEG_SEARCH_BUFSZ)
         BackJump  =  MPEG_SEARCH_BUFSZ;
  }
  else
  if (Coded_Pic_Height <= 720)
  {
     if (BackJump  >  51200)
         BackJump  =  51200;
  }
  else
  {
     if (BackJump  >  102400)
         BackJump  =  102400;
  }

  if (BackJump  <  MPEG_SEARCH_BUFSZ)
      BackJump  =  MPEG_SEARCH_BUFSZ;

  if (process.PrevAct < 0  
  &&  iSearchFile   >=  process.BackFile
  &&  i64Search_Loc >=  process.BackLoc)
  {
      BackJump = BackJump * 2;
  }

  process.BackFile = iSearchFile;
  process.BackLoc  = i64Search_Loc;

  iEndSrch_File   =  iPreCalc_Key_File;
  i64EndSrch_Loc  =  i64PreCalc_Key_Loc  -  (MPEG_SEARCH_BUFSZ/8)  ;

  while  (i64EndSrch_Loc  <  0)
  {
      if  (iEndSrch_File  >  0)
      {
        iEndSrch_File--;
        i64EndSrch_Loc += process.length[iEndSrch_File] - MPEG_SEARCH_BUFSZ;
      }
      else
      {
        i64EndSrch_Loc  =  0;
      }
  }


    //  Allow  for  bug  in  calcs
  if  (iEndSrch_File  <=  iSearchFile)
  {
    if (iEndSrch_File  <  iSearchFile)
    {
        iEndSrch_File  =  iSearchFile;
        i64EndSrch_Loc =  i64Search_Loc;
    }
    else
    if (i64EndSrch_Loc <  i64Search_Loc)
        i64EndSrch_Loc =  i64Search_Loc;
  }


  if  (DBGflag)
            DBGln4("GOPBACK  Limits  @=x%08X  F=%d    FileLen=x%X  Jump=%d\n",
                    i64EndSrch_Loc,  iEndSrch_File,
                    process.length[iEndSrch_File],
                    BackJump);


  iLoopLimit = 50;


//  STEP  backwards  until  we  find  a  GOP
//iLooking  =  1;
//while  (iLooking  >=  0)
//{

Retry:
    iLoopLimit--;
    if      (iLoopLimit == 46) BackJump <<= 1;
    else if (iLoopLimit == 40) BackJump <<= 1;

    //  Check  for  File  boundary  crossed
    while  (i64Search_Loc  <  0)
    {
      if  (iSearchFile  >  0)
      {
          iSearchFile--;
          i64Search_Loc  +=  process.length[iSearchFile]  -  MPEG_SEARCH_BUFSZ;
      }
      else
      {
        i64Search_Loc  =  0;
      }
    }

    process.startFile  =  File_Ctr  =  iSearchFile;
    process.startLoc  =  i64Search_Loc  ;
    process.PACK_Loc  =  -1;    //  ptr  to  MOST  RECENT  pack  header
    process.NAV_Loc    =  -1;    //  ptr  to  MOST  RECENT  VOB  NAV  HDR
    process.SEQ_Loc    =  -1;    //  ptr  to  MOST  RECENT  SEQ  HDR
    process.GOP_Loc    =  -1;    //  ptr  to  MOST  RECENT  GOP  HDR
    process.KEY_Loc    =  -1;    //  ptr  to  MOST  RECENT  KEY  FRAME  (I-Frame)
    process.PIC_Loc    =  -1;    //  ptr  to  MOST  RECENT  PIC

    if (uVid_PID_All)
        uCtl_Vid_PID = STREAM_AUTO;


    process.Delay_Calc_Flag = 0;

    _lseeki64(FileDCB[process.startFile],  process.startLoc,  SEEK_SET);
    MParse.NextLoc = process.startLoc;

    MParse.Fault_Flag  =  00;

    //  Escape  if  Start  of  file  reached
    if  ((i64Search_Loc  <=  0  &&  iSearchFile  ==  0)  
          //  &&  iCtl_Out_Preamble_Flag  >  1
           ||  iLoopLimit <= 0
           ||  MParse.Stop_Flag
        )
    {
        MessageBeep(MB_OK);
        iLooking  =  -1;
        //break;
    }
    else
    {
        MPEG_Pic_Type  =  0;
        getBLOCK_Packet(1);
        if  (DBGflag)
        {
            iAT_Loc  =  _telli64(FileDCB[File_Ctr])  ;
        }

        if (MParse.Stop_Flag)
           iLooking  =  -1;
        else
           iLooking  =  1;

        while  (iLooking  >  0  && iLoopLimit > 0
                 && ! MParse.Stop_Flag )
        {
          if (uVid_PID_All)
              uCtl_Vid_PID = STREAM_AUTO;
          uVid_PID  = STREAM_AUTO;

          iRC       =  GetHdr_PIC(99);
          iAT_Loc   =  process.PIC_Loc;  //telli64(FileDCB[File_Ctr]);
          iAT_File  =  process.PIC_File;
          //iAT_Loc  =  _telli64(FileDCB[File_Ctr]);
          if  (DBGflag)
          {
              DBGln2("BWD  GOT  Pic=%d  @=%X  ",  MPEG_Pic_Type,  iAT_Loc);
              TextOut(hDC,  0,    iMsgPosY,  szDBGln,  22);
          }

          if (MParse.Stop_Flag)
          {
             iLooking  =  -1;
             break;
          }
          else
          //  Have  we  found  a  picture,  within  the  range  ?
          if  (iRC  && iLoopLimit > 0      && PlayCtl.iEOFhit < 6
          &&  (iAT_File  <   iEndSrch_File
          ||  (iAT_File  ==  iEndSrch_File && iAT_Loc < i64EndSrch_Loc)))
          {

            //  THIS  CODE  NEEDS  TO  BE  SMARTER
            //  TO  ALLOW  FOR  FIELD  STRUCTURE  ENCODES with paried I-fields
            //  Luckily,  these  are  mercifully  rare.

              if  ((MPEG_Pic_Type  ==  I_TYPE  
                    &&  (!MParse.iVOB_Style  ||  process.NAV_Loc  >=  0))
              ||  (process.iLongGOP   &&  MPEG_Pic_Type  ==  P_TYPE)    //  Allow  peeking  into  very  long  GOP  -  KDVD
                    //  ||  (i64Search_Loc  <  1  &&  (File_Ctr  ==  0)  &&  iCtl_Out_Preamble_Flag  >  2)
                  )
              {
                  if (DBGflag)
                      DBGln4("** SUCCESS **  @=x%X  F=%d    Long=%d  Nav=x%X",
                                    iAT_Loc,  iAT_File,  process.iLongGOP,  process.NAV_Loc);
                  iLooking  =  -1;
                  break;
              }  //  END  i_Frame
              else
              {
                if (DBGflag)
                {
                    DBGln4("*REJECTED*  PicType=%d  Vob=%d Nav=x%X  Long=%d  ",
                              MPEG_Pic_Type, MParse.iVOB_Style, process.NAV_Loc, process.iLongGOP);
                }
              }

          }  //  END  InRange

          else
          {
              iLooking  =  0;
              if (DBGflag)
              {
                  sprintf(szDBGln,"*NONE IN RANGE*  HdrRC=%d  LoopLim=%d EOF=%d\n",
                                iRC, iLoopLimit, PlayCtl.iEOFhit);
                  DBGout(szDBGln);
                  DBGln4("    @=x%08X  F=%dX\n    E=x%08X  F=%dX",
                                iAT_Loc, iAT_File, 
                                i64EndSrch_Loc, iEndSrch_File);
              }
          }

        }  //  ENDWHILE  loc  <  end


        // We have scanned a range without finding a good I-FRAME

        if (PlayCtl.iEOFhit > 3)
            BackJump *=2;

        if  (iLooking  >=  0  && iLoopLimit > 0 &&  PlayCtl.iEOFhit < 6)
        {
            i64EndSrch_Loc = i64Search_Loc;
            iEndSrch_File  = iSearchFile;
            i64Search_Loc -= BackJump;
            if (MParse.Stop_Flag)
                iLooking = -1;
            else
                goto  Retry;
        }

    }    //  END  NOT  Start  of  File

//}

          //  process.startFile  =  d2v_curr.file;
          //  process.startLoc    =  d2v_curr.lba  *  MPEG_SEARCH_BUFSZ;

  i64PreCalc_Loc = i64Search_Loc;
  iPreCalc_File  = iSearchFile; 


  iRC  =  Get_Hdr_Loc(&process.startLoc,  &process.startFile);

  if (iIn_VOB && process.NAV_Loc < 0)
  {
    process.startLoc -= 4096;
    if (process.startLoc < 0)
        process.startLoc = 0;
    DBGln2("BWD VOB PREV PACK @=%08X  ",  process.startLoc,  process.startLoc);
  }

  if (DBGflag)
      DBGln4("GOPBACK-END  rc=%d    HDR  @=%08X  F=%d\n***\n\n",  iRC,  process.startLoc,  process.startFile,  0x00);

  if  (iRC  <  0)
  {
      process.startFile = iSearchFile;
      process.startLoc  = i64Search_Loc  ;
            //  process.startLoc  =  Calc_Loc(&process.startFile,
            //                              -  (MPEG_SEARCH_BUFSZ  *  2),  1);
  }

  File_Ctr  =  process.startFile;

  //  Allow  for  errors  in  Hdr_Loc  calculation

  iAT_Loc = process.startLoc;
  if (!iIn_VOB                    // Don't fool around with VOBs
  && (iAT_Loc           & 0xFF))  // Don't fool around with Sector Addrs
  {
     if (process.startLoc  >  32)
         process.startLoc -=  32;
     else
         process.startLoc  =  0;
  }

  PktStats.iChk_AnyPackets  =  0;

  return;
}

