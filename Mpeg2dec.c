
#define DBG_RJ
 
 
/*
 *  MPEG2DEC - Copyright (C) Mpeg Software Simulation Group 1996-99
 *  DVD2AVI  - Copyright (C) Chia-chen Kuo - April 2001
 *  Mpg2Cut2 - Various Authors
 *
 *  Part of DVD2AVI - a free MPEG-2 converter
  *  DVD2AVI - Copyright  (C)  Chia-chen  Kuo  -  April  2001
  *
  *  DVD2AVI - a free  MPEG-2 converter
  *
  *  DVD2AVI  is  free  software;  you  can  redistribute  it  and/or  modify
  *  it  under  the  terms  of  the  GNU  General  Public  License  as  published  by
  *  the  Free  Software  Foundation;  either  version  2,  or  (at  your  option)
  *  any  later  version.
  *
  *  DVD2AVI  is  distributed in the  hope  that  it  will  be  useful,
  *  but  WITHOUT  ANY  WARRANTY;  without  even  the  implied  warranty  of
  *  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
  *  GNU  General  Public  License  for  more  details.
  *
  *  You  should  have  received  a  copy  of  the  GNU  General  Public  License
  *  along  with  GNU  Make;  see  the  file  COPYING.  If  not,  write  to
  *  the  Free  Software  Foundation,  675  Mass  Ave,  Cambridge,  MA  02139,  USA.
  *
  */

#include  "global.h"
#include "TXT.h"
#include "Nav_JUMP.h"
#include  "getbit.h"
#include "GetBit_Fast.h"
#include "MPV_PIC.h"
#include "PIC_BUF.h"
//#include <commctrl.h>

#include "Audio.h"
#include "AC3Dec\A53_interface.h"
#include "mpalib.h"
#include "mpalib_more.h"

#include "Mpg2Cut2_API.h"


static  void  JumpChk(),  JumpCalc(char);
void  Mpeg_INIT_Find_SEQ();

#include  "wave_out.h"

__int64    i64Tmp1;

  __int64    i64JumpMin,  i64CurrByteRateAvg, i64Tmp1; //, i64RestartLoc;
  int  iRestart; //, iRestartFile;
 int iWantType;



//------------------------------------------------------

DWORD  WINAPI  MPEG2Dec(LPVOID  nDUMMY)
{
  int  i,  iRC,  iTmp1;
  //  int  iTmp1,  iTmp2,  iTmp3,  iTmp4  ;

  unsigned uDiffPTS;
  __int64 i64DiffLoc; //, i64NewByteRate;


  const  unsigned  char  HIGH_VALUES[8]  =  {  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF,  0xFF}  ;

  iKick.Action = 0;

  ZeroMemory(&PktStats,   sizeof(PktStats));

  ZeroMemory(&PlayCtl,    sizeof(PlayCtl));
  PlayCtl.iFPS_Dec_Pct = 100;
  PlayCtl.iPalTelecide_ctr = 24;
  PktChk_Audio = 25; PktChk_Any = 600;

  MParse.Pause_Flag  =  0;  MParse.Stop_Flag    =  0;
  MParse.Fault_Flag  =  0;

  if (process.Action !=  ACTION_RIP)
  {
    if (uVid_PID_All)
        uCtl_Vid_PID = STREAM_AUTO;

    if (uAud_PID_All)
        uCtl_Aud_PID = STREAM_AUTO;
  }

  uVid_PID  = STREAM_AUTO;  iWant_VOB_ID = -1;

  iTmp1  =  process.Action;



  if  (process.Action    !=  ACTION_FWD_FRAME)
  {
      Frame_Number    =  0;
      Top_Field_Built =  0; Bot_Field_Built = 0;
      MParse.Rip_Flag =  0; 
      iRestart  =  1;
      memcpy(&PrevTC,  &CurrTC,  sizeof(CurrTC));
      memcpy(&process.ViewSSCRM[0],  &HIGH_VALUES[0],  sizeof(process.ViewSSCRM));
  }
  Second_Field    =  0;




  if (process.Action  < 0 
  ||  process.Action >= ACTION_INIT)
  {
      ZeroMemory(&Prev_PTS,  sizeof(Prev_PTS));
  }
  ZeroMemory(&process.PrevSSCRM[0],  sizeof(process.PrevSSCRM));

  getbit_VOB_ID  =  999999;  getbit_CELL_ID  =  999999;
  Sound_Max  =  1;      iVideoBitRate_Bytes  =  0;
  iRender_TimePrev  =  0;

  process.Pack_Min_Size  =  99999999;
  process.Pack_Max_Size  =  0;
  process.PACK_Sample_Ctr = 0;
  process.i64Pack_Sum_Size = 0;

  uGot_Video_Stream  =  uCtl_Video_Stream;

  iGOPperiod  =  10000;
  iGOPrelative  =  0;
  iGOPtot  =  12;  // Initial Estimate (PAL std)
  iGOPtime  =  iCURR_TIME_ms();

  iAudio_Lock  =  0;
  strcpy(AC3_Err_Txt,  "  ");


  if (uCtl_Aud_PID == STREAM_NONE)
  {
    ZeroMemory(&mpa_Ctl,        sizeof(mpa_Ctl));
    ZeroMemory(&SubStream_CTL,  sizeof(SubStream_CTL)); // AC3Stream)*6*CHANNELS_MAX));
    ZeroMemory(&iAudio_Trk_FMT, sizeof(iAudio_Trk_FMT));
  }
  else
  {
    for  (i=0;  i<CHANNELS_MAX;  i++)
    {
       mpa_Ctl[i].rip = 0;
       SubStream_CTL[0][i].rip = 0;
       SubStream_CTL[1][i].rip = 0;
       SubStream_CTL[2][i].rip = 0;
       SubStream_CTL[3][i].rip = 0;
       SubStream_CTL[4][i].rip = 0;
       SubStream_CTL[5][i].rip = 0;
    }
  }

  ZeroMemory(&wav,  sizeof(wav)); // struct  WAVStream));

  // Message area clean-up

  //if  (iPreview_Clip_Ctr >= iEDL_ctr)
       iMsgLife--;
  if  (iMsgLife <= 0  && iMsgLife > -3)
  {
     DSP_Msg_Clear();
     if (iViewToolBar >= 256) //  || iTool_Stacked)
         DSP2_Main_SEL_INFO(0);
  }

  if (process.Action >= 0 || process.PrevAct >= 0)
     i64PreCalc_Loc = Calc_Loc(&iPreCalc_File, -4, 0); 

  if (process.KEY_Loc >= 0)
  {
     i64PreCalc_Key_Loc  = process.KEY_Loc;
     iPreCalc_Key_File   = process.KEY_File;
  }
  else
  {
     i64PreCalc_Key_Loc  = i64PreCalc_Loc;
     iPreCalc_Key_File   = iPreCalc_File;
  }


  if (DBGflag)
  {
      DBGln4(" PRECALC Loc=x%08X  F=%d  Act=%d PrevAct=%d\n",
                      i64PreCalc_Loc, iPreCalc_File,
                      process.Action, process.PrevAct);
  }

  //  RJ    crude but pessimistic estimate of where Last GOP MIGHT start
  //      Errs on the side of safety 

  i64CurrByteRateAvg  =  process.ByteRateAvg[process.CurrFile];
  if (!i64CurrByteRateAvg)
       i64CurrByteRateAvg = 666000;

  process.Last_Gop_Loc_Est  =  process.length[process.CurrFile]  -  (i64CurrByteRateAvg * 3 / 2);


  //Last_Gop_Blk_Est  =  (process.total  -  i64CurrByteRateAvg  )
  //                            /  (__int64)(MPEG_SEARCH_BUFSZ)  ;
  //if  (DBGflag)
  //{
  //  int  iLen;
  //  iTmp1 =  (int)(i64CurrByteRateAvg  /  1000);
  //  iLen  =  sprintf(szBuffer,  "kBps=%1d",  iTmp);
  //  TextOut(hDC,  0,    60,  szBuffer,  iLen);
  //}


  //  Backwards  Accelerator  stores  a list of consecutive  locations

  if  (process.Action  ==  ACTION_FWD_GOP)
  {
      Nav_Jump_Fwd(&BwdGop);

               // Calc stuff for ByteAvg  
               if (process.CurrFile == 0
               &&  process.VideoPTS  > 0
               &&  process.VideoPTS != PTS_NOT_FOUND)
               {
                  if (process.VideoPrevPTS > 0
                  &&  process.VideoPTS > process.VideoPrevPTS)
                  {
                      uDiffPTS = process.VideoPTS - process.VideoPrevPTS;
                      i64DiffLoc = process.CurrLoc  - process.PREV_Curr_Loc;
                      if (i64DiffLoc > 32000) 
                      {
                         // convert to bytes per second
                         process.i64NewByteRate = (int)(i64DiffLoc * 45000 / uDiffPTS);
                         i64CurrByteRateAvg  = 
                             ( (process.ByteRateAvg[process.CurrFile] * 5) 
                                  + process.i64NewByteRate) / 6; 
                         process.ByteRateAvg[process.CurrFile] = i64CurrByteRateAvg;
                         //if (DBGflag)
                         //{
                         //    sprintf(szMsgTxt, "%d %d", 
                         //           process.i64NewByteRate, i64CurrByteRateAvg);
                         //    DSP1_Main_MSG(0,1);
                         //}
                          
                      }
                  }

                  // Remember for next time
                  process.VideoPrevPTS  = process.VideoPTS;
                  process.PREV_Curr_Loc = process.CurrLoc;
               } 
            
          
  }

  else
  {
      process.AudioPTS    =  PTS_NOT_FOUND;
      process.VideoDTS    =  PTS_NOT_FOUND;
        gopTC.VideoPTS    =  PTS_NOT_FOUND;  process.uGOPbefore = 0;
      process.uViewSCR    =  PTS_NOT_FOUND;

      process.uGOP_TCorrection = 0;
      //process.Delay_Sign[0] =  '?';
      //process.DelayPTS      =  0; 
      if (process.Action != ACTION_FWD_FRAME
      &&  process.Action != ACTION_RIP)
          process.iGOP_Ctr = 0;

      if  (process.Action    ==  ACTION_BWD_GOP
       &&  process.PrevAct   >=  ACTION_BWD_GOP
       &&  process.PrevAct   <   ACTION_FWD_GOP2)
      {
          Nav_Jump_BWD(&BwdGop);
      }  //  END  BWD_GOP
      else
      {
          // Most other things break the BwdGop chain - reset LIFO stack;
          if  (process.Action  !=  ACTION_RIP)
          {
               BwdGop.ix   = 0;  
               BwdGop.iOrg = 0;
          }

          if  (process.Action  ==  ACTION_FWD_JUMP)
          {
                Nav_Jump_Fwd(&BwdFast1);

          }
          else
          if  (process.Action    ==  ACTION_BWD_JUMP
          &&   process.PrevAct   ==  ACTION_FWD_JUMP)
          {
               Nav_Jump_BWD(&BwdFast1);
          }  //  END  BWD_GOP
          else
          {
            //  Everything else breaks the BwdFast1 chain - reset LIFO stack;
            BwdFast1.ix = 0;  BwdFast1.iOrg = 0;
            process.VideoPrevPTS = 0;
          }

      }  //endelse  FAST BWDGOP

  }  //endelse  BWD


  // SKIP FILE forward is a bit twisted

  if (process.Action  == ACTION_SKIP_FILE)
  {
     if (process.CurrFile >= File_Final)
     {
         process.Action    = ACTION_BWD_GOP;

         iPreCalc_Key_File  = File_Final;
         i64PreCalc_Key_Loc = process.length[iPreCalc_Key_File] - 8192;

         process.CurrFile  = iPreCalc_File  = iPreCalc_Key_File;
         process.CurrLoc   = i64PreCalc_Loc = i64PreCalc_Key_Loc;

         process.startFile = process.CurrFile;
         process.startLoc  = process.CurrLoc;

              /*
              if (process.CurrLoc < process.Last_Gop_Loc_Est)
              {
                  process.CurrLoc = process.Last_Gop_Loc_Est;
              }
              else
              {
                  process.Action = ACTION_FWD_GOP;
              }
             */

     }
     else
     {
         process.Action  = ACTION_NEW_CURRLOC;

         process.CurrFile++;
         process.CurrLoc = 0 ;
     }
  }




  //  WHAT  TYPE  OF  ACTION  HAS  BEEN  REQUESTED  ?


  //  BACKWARD  jumps  calculated  from  previous  START  point  (startLoc  startFile)
  //  RANDOM    jump  based  on  new  given  value  of  currLoc  or  startrunloc
  //  FORWARD  SEEK  calculated  from  what  we  LAST  read  (typically  END  of  frame)


  if  (process.Action  <  0)    //  BACKWARD  ?
  {
    process.startFile = process.CurrFile = iPreCalc_Key_File;
    process.startLoc  = process.CurrLoc  = i64PreCalc_Key_Loc; 

    switch  (process.Action)
    {
     case  ACTION_BWD_JUMP4:

        process.LocJump  =    i64CurrByteRateAvg  *  iJumpSecs[5] * 3 / 2;
        if  (process.CurrLoc  >  (-process.LocJump)  ||  process.CurrFile)
        {
          break  ;
        }
        else  process.Action  =  ACTION_BWD_JUMP2  ;

     case  ACTION_BWD_JUMP2:

        process.LocJump  =    i64CurrByteRateAvg  *  iJumpSecs[5];
        if  (process.CurrLoc  >  (-process.LocJump)  ||  process.CurrFile)
        {
          break  ;
        }
        else  process.Action  =  ACTION_BWD_JUMP  ;


    case  ACTION_BWD_JUMP:

        process.LocJump  =    i64CurrByteRateAvg  *  iJumpSecs[4];
        if  (process.CurrLoc  <  (-process.LocJump)  &&  !  process.CurrFile)
        {
            process.LocJump  =  -process.CurrLoc  ;
        }
        break;


    case  ACTION_BWD_GOP2:

        i64JumpMin  =  -10  *  MPEG_SEARCH_BUFSZ;
        process.LocJump  =    i64CurrByteRateAvg  *  iJumpSecs[3];
        if (process.LocJump  >  i64JumpMin)
            process.LocJump  =  i64JumpMin;

        if  (process.CurrLoc  >  (-process.LocJump)  ||  process.CurrFile)
        {
            break;
        }
        else  process.Action  =  ACTION_BWD_GOP  ;


    default:    //ACTION_BWD_GOP:
        process.LocJump  =  -i64CurrByteRateAvg  /  4  ;
        i64JumpMin  =  MPEG_SEARCH_BUFSZ;  //  Performance  cheat  -  but  means  low  bitrate  files  may  get  irregular  backwards  seek
        if (process.LocJump  >  i64JumpMin  )  //  NOTE:  Negative  numbers  complemented  comparator
            process.LocJump  =  i64JumpMin  ;

        if  (process.CurrLoc > (-  process.LocJump) ||  process.CurrFile)
        {
        }
        else
        {
            process.LocJump  =  -process.CurrLoc  ;
            //MessageBeep(MB_OK);
        }
        break;

    }  //  END  SWITCH  BWD  type

    JumpCalc('L');
    GOPBack()  ;

  }    //  END  BACKWARD  locates

  else
  switch  (process.Action)
  {
    case  ACTION_FWD_JUMP4:

        process.LocJump  =  i64CurrByteRateAvg  *  iJumpSecs[2] * 3 / 2;
        if  (process.CurrFile  <  File_Final
        ||  (process.CurrLoc  +  process.LocJump)  <  process.Last_Gop_Loc_Est)
        {
            JumpCalc('L');
            break  ;
        }
        else  
          process.Action  =  ACTION_FWD_JUMP2;

    case  ACTION_FWD_JUMP2:

        process.LocJump  =  i64CurrByteRateAvg  *  iJumpSecs[2];
        if  (process.CurrFile  <  File_Final
        ||  (process.CurrLoc  +  process.LocJump)  <  process.Last_Gop_Loc_Est)
        {
            JumpCalc('L');
            break  ;
        }
        else 
          process.Action  =  ACTION_FWD_JUMP;



    case  ACTION_FWD_JUMP:

        process.LocJump  =  i64CurrByteRateAvg  *  iJumpSecs[1];

        if  (process.CurrFile  >=  File_Final)
        {
           i64Tmp1 = process.Last_Gop_Loc_Est / 3; // Allow for small files
           if (process.LocJump > i64Tmp1)
               process.LocJump = i64Tmp1;
        }


        if  (process.CurrFile  >=  File_Final
        &&  (process.CurrLoc  +  process.LocJump)  >  process.Last_Gop_Loc_Est)
        {
          if  (process.CurrLoc  <  process.Last_Gop_Loc_Est)
          {
              process.LocJump  =  process.Last_Gop_Loc_Est  -  process.CurrLoc;
              JumpCalc('L');
              break  ;
          }
          else
            process.Action  =  ACTION_FWD_GOP2  ;

        }
        else
        {
          JumpCalc('L');
          break  ;
        }



    case  ACTION_FWD_GOP2  :
        process.LocJump  =  i64CurrByteRateAvg  *  iJumpSecs[0];
        i64JumpMin  =  10  *  MPEG_SEARCH_BUFSZ;
        if (process.LocJump  <  i64JumpMin)
            process.LocJump  =  i64JumpMin;

        if  (process.CurrFile  <  File_Final
        ||  (process.CurrLoc  +  process.LocJump)  <  process.Last_Gop_Loc_Est)
        {
            JumpCalc('L');
            break  ;
        }
        else  process.Action  =  ACTION_FWD_GOP  ;


    case  ACTION_FWD_GOP:

        if (process.ALTPID_Loc > 0)
        {
           process.CurrFile = process.ALTPID_File;
           process.CurrLoc  = process.ALTPID_Loc;
        }

        process.LocJump  =  0;  //  -i64CurrByteRateAvg  /  4  ;
        //i64JumpMin  =  2  *  MPEG_SEARCH_BUFSZ;
        //if  (process.LocJump  <  i64JumpMin)
        //    process.LocJump  =  i64JumpMin;


        if  (    process.CurrFile  <  File_Final
        ||  ((   process.CurrLoc  /*  +  process.LocJump  */  )
              <  process.length[File_Final]))
            //<  process.Last_Gop_Loc_Est))
        {
          JumpCalc('=');    //  L');
          break  ;
        }
        else
        {
          //  An  option  would  be  to  PLAY  the  last  GOP
          //  so  that  the  user  could  see  the  final  picture
          //  but  this  is  not  safe  until  decoder  is  fully  fault  tolerant
          //  process.Action  =  ACTION_PLAY  ;

                Mpeg_EOF();
                break  ;
        }

    case  ACTION_RIP:
    case  ACTION_PLAY:
      process.LocJump  =  0  ;
      JumpCalc('S');  //  start  from  startLoc  -  endloc  varies  between  PLAY  vs  RIP
      process.Action  =  ACTION_RIP;
      Set_Priority(GetCurrentProcess (), iCtl_Priority[1],  1,  1);

      break;

    case  ACTION_NEW_RUNLOC:
      process.LocJump  =  0  ;
      JumpCalc('%');    //  start  from  startrunloc
      break;


    default:  //case  ACTION_NEW_CURRLOC:
      process.LocJump  =  0  ;
      JumpCalc('L');
      break;

  }  //endswitch





  if (DBGflag)
  {
      sprintf(szBuffer, "\nTrackSel=%d FPSOveride=%d ALock=%d ", 
                   iAudio_SEL_Track, iView_FrameRate_Code,
                   iAudio_Lock);
      DBGout(szBuffer);
  }


  process.PrevAct  =  process.Action;

  // BEGIN DECODING 

  GetBlk_RdAHD_RESET();

  //  search  MPEG-2  Sequence  Header
  if  ( ! MParse.SeqHdr_Found_Flag)
  {
      Mpeg_INIT_Find_SEQ();
  }


  if (iAutoPlay && process.Action == ACTION_INIT)
  {
      process.Action = ACTION_RIP; 
      if (iMPALib_Status != 0)
          iPlayAudio = 1;
  }

  FrameRate2FramePeriod();

  if (process.Action == ACTION_RIP
  &&  !MParse.Tulebox_SingleStep_flag)
  {
     Mpeg_Drop_Init();
  }

  // D2VFile_Init();


  File_Ctr  =    process.startFile;

  if  (process.Action  !=  ACTION_FWD_FRAME)
  {
      if  (process.startLoc  >  process.length[process.startFile])
      {
          if  (process.startFile  <  File_Final)
          {
              process.startLoc  =  0;
              process.startFile++;
          }
          else
          {
            Mpeg_EOF();
            Mpeg_KILL(1004);
            return  0;
          }
      }  //  ENDIF  FILE  BOUNDARY

      //if  (DBGflag)
      //    DBGln2("    SEEK  @=%X  F=%d",  process.startLoc,  process.startFile);

      _lseeki64(FileDCB[process.startFile],  process.startLoc,  SEEK_SET);
          //  (process.startLoc  /  MPEG_SEARCH_BUFSZ)  *  MPEG_SEARCH_BUFSZ,  SEEK_SET);
      MParse.NextLoc = process.startLoc;
      process.LocJump  =  0;
      iRestart  =  1;
      getBLOCK_Packet(iRestart)  ;
        //if  (DBGflag)
        //    DBGout("    Restart  done");  //    @=%X  F=%d",  process.startLoc,  process.startFile);
  }  //  ENDIF  NOT  FWD_FRAME


  process.op  =  0  ;

  if  (process.Action  ==  ACTION_FWD_FRAME)
  {
    GetHdr_PIC(99);

    if  (MPEG_Pic_Type  ==  I_TYPE)
    {
        //  Set  CURRLOC  using  the  SEQ-GOP-PIC  hierarchy
        iRC  =  Get_Hdr_Loc(&process.CurrLoc,  &process.CurrFile);
        //  process.CurrLoc    =  process.PIC_Loc;
        //  process.CurrFile  =  process.PIC_File;
    }
                      //if  (MPEG_Pic_Structure  !=  FULL_FRAME_PIC)  //  Allow  for  interlaced  fields
    Pic_DECODE();      //  <===============  ESCAPE  POINT


  }
  else
  {
    if (process.Action  ==  ACTION_RIP)
        MParse.Rip_Flag  =  1;  //RJ-  Made  conditional  to  AVOID  FALSE  TRIGGER

    //  Process  control  info  without  decoding  until  a  picture  found
    GetHdr_PIC(I_TYPE);    //  while  (GetHdr_PIC(-1))  &&  MPEG_Pic_Type  !=  I_TYPE)  ;

    //if  (DBGflag)
    //     DBGln4("    *GOT*  Type=%d  @=%d  F=%d",  MPEG_Pic_Type,  process.PIC_Loc,  process.PIC_File,  0x00);

    //process.CurrFile  =  d2v_curr.file;
    //process.CurrBlk    =  d2v_curr.lba;
    iRC  =  Get_Hdr_Loc(&process.CurrLoc,  &process.CurrFile);
    //process.CurrLoc    =  process.PIC_Loc;
    //process.CurrFile  =  process.PIC_File;

    Pic_DECODE();    //  <===============  ESCAPE  POINT

    //  if  reached  this  point  without  escaping,
    //  then  maybe  the  I-frame  has  a  P-field.
    //  Just  to  be  clean,  skip  any  B-frames  without  decoding  them
    //if  (DBGflag)
    //     DBGln4("    *MORE*  Type=%d    @=%d  F=%d",  MPEG_Pic_Type,  process.PIC_Loc,  process.PIC_File,  0x00);

    while  (GetHdr_PIC(99)    &&  MPEG_Pic_Type  ==  B_TYPE)
    {
        if (DBGflag)
            DBGln2("    MORE  @=%d  F=%d",  process.PIC_Loc,  process.PIC_File);
    };

  }

  Pic_DECODE();    //  <===============  ESCAPE  POINT



  PlayCtl.uPrev_Time_ms[0]  =  timeGetTime();
  process.op  =  PlayCtl.uPrev_Time_ms[1] = PlayCtl.uPrev_Time_ms[0] ;

  if  (process.Action  ==  ACTION_RIP)
  {
      iWantType = 99;

      while  (  GetHdr_PIC(iWantType)
        // &&      process.Action == ACTION_RIP  // Allow for elegant stop
          &&  !MParse.Stop_Flag)
      {
         Pic_DECODE();    //  <==============  INTERNAL  ESCAPE  POINT

         // Preview multi-clips
         if (process.Action == ACTION_RIP && iPreview_Clip_Ctr < iEDL_ctr
         &&  MParse.Summary_Adjust < 0
         &&((MParse.NextLoc >= process.endLoc 
                  //&& (RdEOB - RdPTR) < 8192
                   &&  File_Ctr == process.endFile) 
               ||  File_Ctr  > process.endFile) 
         )
         {
               C160_Clip_Preview();
               File_Ctr = process.startFile;
               _lseeki64(FileDCB[File_Ctr], process.startLoc, SEEK_SET );
               MParse.NextLoc = process.startLoc;
  
               RdEndPkt = RdPTR = RdEOB;
               RdEndPkt_4 = RdEndPkt-4;  RdEndPkt_8 = RdEndPkt-8;
               BitsLeft = 0;

               //Mpeg_READ(); 
               iWantType = I_TYPE;
             }
         else
         if (MParse.FastPlay_Flag >= CUE_SLOW)  // CUE mode
         {
            iWantType = I_TYPE;
          
            if (MParse.FastPlay_Flag >= (CUE_SLOW+2)) // SUPER-CUE mode matches = ACTION_FWD_JUMPx on Auto
            {
               iTmp1 = MParse.FastPlay_Flag - (CUE_SLOW+2);
               process.LocJump = i64CurrByteRateAvg  *  iJumpSecs[iTmp1];

               process.CurrLoc = Calc_Loc(&process.CurrFile, -4, 0) ; 
               if  (process.CurrFile <  File_Final
               ||  (process.CurrLoc  +  process.LocJump) < process.Last_Gop_Loc_Est)
               {
                   JumpCalc('L');
                   _lseeki64(FileDCB[process.CurrFile], process.startLoc, SEEK_SET);
                   MParse.NextLoc = process.startLoc;
               }
            }
         }
         else
            iWantType = 99;


      }
  }

  if  (MParse.Stop_Flag)
  {
      if  (DBGflag)  
        TextOut(hDC,  0,  50,  "Null  time",  9);
      MParse.Fault_Flag  =  97;
      Write_Frame(NULL,  d2v_curr,  0);
  }

  //DSP5_Main_FILE_INFO();
  Mpeg_KILL(1005);

  return  0;
}



//  THIS  ROUTINE  NEEDS  TO  BE  RATIONALISED  !
//----------------------------------------------------------------------
void JumpCalc(char  P_Mode)
{
  int  iFile_Ctr1;
  iFile_Ctr1  =  File_Ctr;  //  remember  where  we  entered
  File_Final  =  File_Limit  -  1;

  process.PREV_Pack_File = process.PACK_File;
  process.PREV_Pack_Loc  = process.PACK_Loc;

  //  Clear  the  current  PACK  location  if  we  jump
  if  (process.LocJump    ||  process.Action  !=  ACTION_FWD_FRAME)  //  P_Mode  !=  '=')
  {
    process.PACK_Loc   =  -1;    //  ptr  to  MOST  RECENT  Pack  header
    process.VIDPKT_Loc =  -1;    //  ptr  to  MOST  RECENT  Video packet header
    process.NAV_Loc    =  -1;    //  ptr  to  MOST  RECENT  VOB  NAV  pack  header
    process.SEQ_Loc    =  -1;    //  ptr  to  MOST  RECENT  SEQ  HDR
    process.GOP_Loc    =  -1;    //  ptr  to  MOST  RECENT  GOP  HDR
    process.KEY_Loc    =  -1;    //  ptr  to  MOST  RECENT  KEY  FRAME  (I-Frame)
    process.PIC_Loc    =  -1;    //  ptr  to  MOST  RECENT  PIC
    process.ALTPID_Loc = -1;   //  ptr  to Alternate PID
    process.ViewPTS    =  0xFFFFFFFF;  //  pts  to  MOST  RECENT  SEQ/GOP/PIC
    process.ViewPTSM   =  0xFFFFFFFF;  //  pts  to  MOST  RECENT  SEQ/GOP/PIC
  }

  //  BACKWARD  jumps  are  calculated  from  previous  START  point
  //  RANDOM    jump  based  on  new  given  value  of  currLoc  or  startrunloc
  if  (process.LocJump  <  0    ||  P_Mode  !=  '=')
  {
    if  (  P_Mode  ==  'S')
         process.startrunloc  =  process.startLoc  +  process.LocJump
                              +  process.origin[process.startFile];
    else
    if  (  P_Mode  !=  '%')
         process.startrunloc  =  process.CurrLoc  +  process.LocJump
                              +  process.origin[process.CurrFile];
  }
  else
  {
      //  FORWARD  seek  is  calculated  from  what  we  LAST  USED  (typically  END  of  frame)
      process.startLoc       =  process.KILL_Loc;              
      process.startFile      =  process.KILL_File;               
      if  (process.startLoc  >    process.length[process.startFile])    //  [process.CurrFile])
           process.startLoc  =    process.length[process.startFile];    //  [process.CurrFile];
      if  (process.startLoc  <  0)
           process.startLoc  =  0;
      process.startrunloc    =  process.startLoc
                             +  process.origin[process.startFile];  //  [process.CurrFile];
  }  //  endif

  if  (process.startrunloc  <  0)
       process.startrunloc  =  0;

  if  (process.Action  !=  ACTION_RIP)
  {
      process.endrunloc =  process.total  -  MPEG_SEARCH_BUFSZ;
      process.endFile   =  File_Final;
      process.endLoc    =  (process.length[process.endFile]
                            //      /  MPEG_SEARCH_BUFSZ    -  1
                        )    //      *  MPEG_SEARCH_BUFSZ
                        ;
  }
  else  //  endrunloc  is  useless  with  multi-clips  -  dummy  value  for  now...
      process.endrunloc  =  process.total  -  MPEG_SEARCH_BUFSZ;


  //  Calculate  new  start  loc,  allowing  for  file  boundaries

  process.startLoc  =  process.startrunloc;
  process.CurrFile  =  0;
  while  (process.startLoc  >=  process.length[process.CurrFile])

  {
    if  (process.CurrFile  <  File_Final)
      {
        process.startLoc  -=  process.length[process.CurrFile];
        process.CurrFile++;
      }
    else
      process.startLoc  =  process.endLoc  -  MPEG_SEARCH_BUFSZ;
  }

  if  (MParse.iVOB_Style)
      process.startLoc  =  (process.startLoc  &  0xFFFFFFFFFFFFF800);  //  VOBS  align  on  2k  boundaries


  process.CurrLoc  =  process.startLoc;
  if  (process.CurrFile  !=  iFile_Ctr1)
  {
      _lseeki64(FileDCB[process.CurrFile],  process.startLoc,  SEEK_SET);
      MParse.NextLoc = process.startLoc;
      MessageBeep(MB_OK);
  }

  //  Reset  every  copy  of  the  current  file  indicator  (Why  are  there  so  many  of  these  ?)
  process.startFile =  File_Ctr  =  d2v_curr.file  =  process.CurrFile;
  process.run       =  process.origin[process.CurrFile];
  //  process.LocJump  =  0;

}




//-----------------------------------------
void  Mpeg_INIT_Find_SEQ()
{
  int  code, iTmp1;
  BYTE*  lp_MpegPacket;


  process.PAT_Loc   =  -1;    // ptr to 1st TS PAT header

//    __int64  lfsr;

/*
      if  (KeyOp_Flag==KEY_OP)
      {
        lfsr  =  KeyOp(File_Limit,  File_Name,  hTrack);

        lfsr0  =  (int)(lfsr>>32);
        lfsr1  =  (int)(lfsr  &  0xffffffff);

        if  (lfsr)
        {
          KeyOp_Flag  =  KEY_INPUT;
          CheckMenuItem(hMenu,  IDM_KEY_OFF,  MF_UNCHECKED);
          CheckMenuItem(hMenu,  IDM_KEY_INPUT,  MF_CHECKED);
          CheckMenuItem(hMenu,  IDM_KEY_OP,  MF_UNCHECKED);
        }
        else
        {
          KeyOp_Flag  =  KEY_OFF;
          CheckMenuItem(hMenu,  IDM_KEY_OFF,  MF_CHECKED);
          CheckMenuItem(hMenu,  IDM_KEY_INPUT,  MF_UNCHECKED);
          CheckMenuItem(hMenu,  IDM_KEY_OP,  MF_UNCHECKED);
        }
      }
*/


      *(DWORD  *)(&PTS_Flag)  =  (DWORD)('----');  //  This  is  a  lot  easier  in  Cobol  !

      File_Ctr  =  0;
      _lseeki64(FileDCB[0],  0,  SEEK_SET);
      MParse.NextLoc = 0;

      //  get  a  first  block  of  Mpeg  data  and  scan  for  the  next  packet  start

      getBLOCK_Packet(1);
      iRestart  =  0;

      //  SCAN  UNTIL  WE  FIND  AN  MPEG  SEQ  HDR
      //        ?  What  about  if  there  are  none  to  be  found  ?
      //  This  code  is  subtly  different  to  the
      //  corresponding  part  of  Get_NextPacket  routine.
      //  HERE  we  allow  for  NON-PACKETIZED  elementary  stream.

      //  BUT it's very tedious having to duplicate code here - Grrr ! Argh !

      while  (!MParse.SeqHdr_Found_Flag
          &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL    //  RJ  ALLOW  FOR  BAD  DATA
          &&  !MParse.Stop_Flag)
      {
        GetB_Show_Next_Start_Code(1);
        code  =  Get_Bits(32);

        if  (code  ==  PACK_START_CODE)
        {
            MParse.SystemStream_Flag  =  1;
            process.Preamble_PackHdr_Found  =  1;
            process.PACK_Loc  =  Calc_Loc(&process.PACK_File,  -4,  1)  ;  //  PERFORMANCE  HIT  ?
            lp_MpegPacket  =  Mpeg_BytePtr()  -4;
            if  (*((UNALIGNED  DWORD*)(lp_MpegPacket))  !=  uPACK_START_CODE)    //  01ba
            {
                lp_MpegPacket     -=  4;  //  Not  quite  sure  how  to  alternate  between  the  Get_Bits  sub-sys  and  the  Get_Byte  sub-system
                process.PACK_Loc  -=  4;
            }
            memcpy  (&process.Preamble_PackHdr[0],  lp_MpegPacket,  sizeof(process.Preamble_PackHdr)-4);
            //if  (DBGflag)  DBGln2("\n    PreamblePackLoc=x%04X  (%d)\n",  process.PACK_Loc,  process.PACK_Loc);
         
            process.Mpeg2_Flag = ( lp_MpegPacket[4] & 4);  //Get_Bits(2) + 1; //

            if (!process.Mpeg2_Flag)
            {
              if ( ! Mpeg_Version_Alerted)
                  F595_NotMpeg2_Msg(0);
            }

        }
        else
        if  (code  ==  PRIVATE_STREAM_2) //  &&  !  iInPS2_Audio)
        {
            Got_PS2_Pkt(1);  //Got_PS2_NAV(1);
            if  (process.preamble_len  <  1  &&  ! process.Preamble_Known)
            {
                process.preamble_len  =  process.NAV_Loc  ;
                //if  (DBGflag)  DBGln2("\n*  PREAMBLE=x%04X  (%d)  *NAV*\n",  process.preamble_len,  process.preamble_len);
            }
            InputBuffer_NEXT_fill(0);
          }
        else
        if  (code  ==  SYSTEM_START_CODE)
        {    
            lp_MpegPacket  =  Mpeg_BytePtr()  -4;
            if  (*((UNALIGNED  DWORD*)(lp_MpegPacket))  !=  uSYSTEM_START_CODE)    //  01bb
                lp_MpegPacket  -=  4;    //  Not  quite  sure  how  to  alternate  between  the  Get_Bits  sub-sys  and  the  Get_Byte  sub-system
            memcpy  (&process.Preamble_SysHdr[0],  lp_MpegPacket,  sizeof(process.Preamble_SysHdr)-4);
            process.Preamble_SysHdr_Found  =  1;
            process.iFixedRate = ( *(lp_MpegPacket+3) & 0x02);
        }
        else
        if  (code  ==  0x01E0)  // Std Video Packet
        {    
            code  =  Get_Bits(16); // PES pkt length
            code  =  Get_Bits(8);  // PES boring flags
            //if ((code & 0xC0) == 0xC0) // Mpeg2 ?
            {
              code  =  Get_Bits(8);  // PTS-DTS flags
              process.Got_PTS_Flag = code & 0x80;
            }
        }
        else
        if  (code  ==  0x01BE)  //Padding Packet can be full of garbage
        {    
            iTmp1  =  Get_Bits(16); // PES pkt length
            if (iTmp1 < 32768)
            {
                while (iTmp1 > 0)
                {
                   code  =  Get_Bits(8);
                   iTmp1--;
                }
            }
        }
        else
        if  (code  ==  xSEQ_HDR_CODE)
        {
            gothdr_SEQ();
            if  (!  process.Keep_Broken_GOP
                &&  process.NAV_Loc   <  0
                &&  process.FromLoc  ==  0  &&  process.FromFile  ==  0)
            {
                  C510_Sel_FROM_MARK(0);  //  Default  FROM  point  =  TIDY  start  position
            }
            //Calc_PhysView_Size();    
            //View_MOUSE_CHK(MAKELPARAM(100,200));
            //Chg2YUV2(0,0);
        }
        else
        if  (code  ==  GROUP_START_CODE)
             gothdr_GOP();
        else
        if  (code  ==  PICTURE_START_CODE)
             gothdr_PICTURE();
        else
        if  (code  >=  SLICE_START_CODE_MIN
         &&  code  <=  SLICE_START_CODE_MAX  )
        {
            iTmp1 = code;
            if (process.preamble_len  <  1  &&    !  process.Preamble_Known)
                Mpeg_PreAmble_Alert(MPEG_Pic_Type);
        }


      }  //endwhile  !mpegseqhdr

      process.FromPTS   =  process.Preamble_PTS   =  process.VideoPTS;
      process.FromPTSM  =  process.Preamble_PTSM  =  process.VideoPTSM;

}



//---------------------------------------------------------
//  Reached  end  of  last  file
void  Mpeg_EOF()
{

  int iRC;

  strcpy(szMsgTxt,FILE_END_OF_FILE);  // "END OF FILE"); 
  DSP1_Main_MSG(0,1);
  iMsgLife = 2;
  UpdateWindow(hWnd_MAIN);
  //MessageBeep(MB_OK);

  process.PACK_Loc  =  process.length[File_Final];
  process.PACK_File =                 File_Final ;

  iRC = Get_Hdr_Loc(&process.CurrLoc,  &process.CurrFile);

  T599_Trackbar_END();


  //  Indicate  EOF  by  greying  out  the  Overlay
  if  (DDOverlay_Flag && MParse.iColorMode==STORE_YUY2 && iShowVideo_Flag)
  {
        if (iDDO_Frame_Ready)
            D200_UPD_Overlay();

        if (MPEG_Pic_Type == I_TYPE || process.Action != ACTION_RIP)
              Write_Frame(bwd_ref_frame, d2v_bwd,  Frame_Number-1 /* 0 */);
        else
        if (MPEG_Pic_Type == P_TYPE)
        {
              Write_Frame(fwd_ref_frame,  d2v_fwd, Frame_Number-1);
              if (PlayCtl.uPendingSeq[I_TYPE] > MPEG_Pic_Temporal_Ref)
                  Write_Frame(bwd_ref_frame, d2v_bwd,  Frame_Number /* 0 */);
        }
        else
        //if (MPEG_Pic_Type == B_TYPE)
        {
              Write_Frame(aux_frame,   d2v_curr, Frame_Number-1);
              Write_Frame(aux_frame,   d2v_curr, Frame_Number-1);
              if (PlayCtl.uPendingSeq[I_TYPE] > MPEG_Pic_Temporal_Ref)
                  Write_Frame(bwd_ref_frame, d2v_bwd,  Frame_Number /* 0 */);
              if (PlayCtl.uPendingSeq[P_TYPE] > MPEG_Pic_Temporal_Ref)
                  Write_Frame(fwd_ref_frame,  d2v_fwd, Frame_Number);
        }

        RenderYUY2(0); // Indicate EOF
        Sleep(10);
  }

  MParse.Fault_Flag  =  98;

  Write_Frame(NULL,  d2v_curr,  0);

}




//-------------------------------------------------
//  Kill  the  Mpeg  decoder  thread
void  Mpeg_KILL(int  P_Caller)
{
#define  false  0

  process.KILL_Loc  =  Calc_Loc(&process.KILL_File,  -4,  0)  ;  //  PERFORMANCE  HIT  ?
//
  if (DBGflag)
      DBGln4("    KILL  Caller=%d    CurrLoc=%X  File=%d    Err=%d\n",
                  P_Caller,    process.CurrLoc,    process.startFile,
                                                      MParse.Fault_Flag);  //process.Action);

  if (iAudioDBG)
      DBGAud(&"mkil");


  //if  (DBGflag)  TextOut(hDC,  0,  80,  "Mpeg_Kill",  9);

/*    if  ((AVI_Flag  ||  D2V_Flag)  &&  iAudio_SEL_Track!=TRACK_NONE  &&
      ((SubStream_CTL[FORMAT_AC3][iAudio_SEL_Track].rip  &&  AC3_Flag==AUDIO_DECODE)  ||  wav.rip))
    {
      if  (SRC_Flag)
      {
          EndSRC(wav.file);
          wav.size  =  ((int)(0.91875*wav.size)>>2)<<2;
      }

      Normalize(NULL,  44,  wav.filename,  wav.file,  44,  wav.size);
      EndWAV(wav.file,  wav.size);
    }
*/

  if (process.Action  ==  ACTION_INIT  
  ||  process.Action  ==  ACTION_RIP )
  {
/*    if  (D2V_Flag)
      {
          if  (MParse.Stop_Flag)
            fprintf(D2VFile,  "  9\n\nINTERRUPTED");
          else
            fprintf(D2VFile,  "  9\n\nFINISHED");
      }*/
/*
      _fcloseall();    //  <=====  ?  ?  ?  ?  ?
      if  (DBGflag)
      {
          DBGflag=0;
          DBGctl();
      }
*/
      /*if  (Decision_Flag)
      {
          if  (Sound_Max  >  1)
          {
            AC3_PreScale_Ratio  =  327.68  *  Norm_Ratio  /  Sound_Max;

            if  (AC3_PreScale_Ratio  >  1.0  &&  AC3_PreScale_Ratio  <  1.01)
                AC3_PreScale_Ratio  =  1.0;

            sprintf(szBuffer,  "%.2f",  AC3_PreScale_Ratio);
            SetDlgItemText(hStats,  IDC_INFO,  szBuffer);

//            CheckMenuItem(hMenu,  IDM_PRESCALE,  MF_CHECKED);
//            CheckMenuItem(hMenu,  IDM_NORM,  MF_UNCHECKED);
            Normalization_Flag  =  false;
          }
          else
          {
            SetDlgItemText(hStats,  IDC_INFO,  "N.A.");
//            CheckMenuItem(hMenu,  IDM_PRESCALE,  MF_UNCHECKED);
          }
      }*/

      AVI_Flag  =  false;
      D2V_Flag  =  false;
      //Decision_Flag  =  false;
      //iShowVideo_Flag = false;
      //Menu_Main_Enable();

      //if (File_Limit)
      //    DSP2_Main_SEL_INFO(0);

  }


  // Flush the Sound output buffers
  if(iWAV_Init)
  {
    WAV_Flush();  
    if (iAudio_Force44K)
        WAV_WIN_Audio_close();
  }
  else
  if (iAudioDBG)
      DBGAud(&"0WAV");


  //if  (DBGflag)  TextOut(hDC,  0,  80,  "Mpeg_Kill",  9);
  if  (process.Action  ==  ACTION_RIP)
  {
      //if (DBGflag)
      //    DBGout("FORE");
      //SetForegroundWindow(hWnd);

      if  (!MParse.Stop_Flag && MParse.ShowStats_Flag)
      {
          MessageBeep(MB_OK);
          SetDlgItemText(hStats,  IDC_REMAIN,  "FINISH");
      }
  }


  //if  (byAC3_Init  ||  iMPAdec_Init  ||  iWAV_Init)
  //{
    byAC3_Init  = 0; iMPAdec_Init  =  0;
  //}

  iPlayAudio  =  0;   iEDL_Reload_Flag = 0;

  MParse.Tulebox_SingleStep_flag = 0;
  MParse.Stop_Flag = 1;

  if (iViewToolBar > 1)
      TextOut(hDC, iTimeX-25, iTimeY, ".", 1) ; // Avoid Deadlock somehow.
  // Potential for thread deadlock - need to redesign this 

  if (iKick.Action) // Is there another Mpeg Action waiting ?
  {
      if (iAudioDBG)
          DBGAud(&"+ACT");
  }
  else
  {
      if (DBGflag)
          DBGout("Inf ");
     
      if (iAudioDBG)
          DBGAud(&"Inf ");

      //DSP5_Main_FILE_INFO();
      PostMessage(hWnd_MAIN, RJPM_UPD_MAIN_INFO, 0, 0); // Avoid Deadlock

      /*
      if (process.Action == ACTION_RIP)
      {
         if (DBGflag)
             DBGout("SHOW FINAL POS");

          //T590_Trackbar_SEL();
          if (iViewToolBar)
              PostMessage(hWnd, RJPM_UPD_TRACKBAR, 0, 0); // Avoid Deadlock
      }
      
      if (DBGflag)
          DBGout("MENU ENABLE");
      Menu_Main_Enable();
      */

      if (MParse.ShowStats_Flag)
      {
         if (DBGflag)
             DBGout("STAT");
         if (iAudioDBG)
             DBGAud(&"STAT");

         S300_Stats_Audio_Desc();
      }

  }

  if (DBGflag)
      DBGout("MPEG_EXIT");
  
  //if (iAudioDBG)
  //    DBGAud(&"CBK ");

  if (mycallbacks.hMPEGKill)
        SetEvent(mycallbacks.hMPEGKill);

  //if (iAudioDBG)
  //    DBGAud(&"X   ");

  ExitThread(0);
}

