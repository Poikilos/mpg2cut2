
// #define DBG_RJ
 
/*
 *  MPEG2DEC - Copyright (C) Mpeg Software Simulation Group 1996-99
 *  DVD2AVI  - Copyright (C) Chia-chen Kuo - April 2001
 *  Mpg2Cut2 - Various Authors
 *
 *  Part of DVD2AVI - a free MPEG-2 converter
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
#include "getbit.h" 
#include "GetBit_Fast.h"

#include "Audio.h"


//              (RdBFR + MPEG_SEARCH_BUFSZ)
#define MPEG_PTR_BUFF_CHK                      \
while (RdPTR >= RdEOB                         \
   &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL   \
   && ! MParse.Stop_Flag )            \
{                                             \
  Mpeg_READ();                                \
  RdPTR -= MPEG_SEARCH_BUFSZ;                 \
}







//-----------------------------------------------

void PTS_Err_Msg(unsigned P_Prev_IX, unsigned P_Curr_PTS, char P_Type[16])
{
  char szTmp1[16]; //, szTmp2[16];
  unsigned iTmp1, W_Prev_PTS;

  if (P_Prev_IX < 25)
      W_Prev_PTS = Prev_PTS[P_Prev_IX];
  else
      W_Prev_PTS = 0;

  if (iMsgLife > 0 && !szMsgTxt[0])
    return;

  if (W_Prev_PTS < 90000 
  &&  P_Curr_PTS < 90000)
  {
     sprintf(szTmp1,   "%dms", (W_Prev_PTS/45));
     sprintf(szBuffer, "%dms", (P_Curr_PTS/45));
  }
  else
  {
     PTS_2Field(W_Prev_PTS, 0);
     strcpy(szTmp1,szBuffer);
     PTS_2Field(P_Curr_PTS, 0);
  }

  iTmp1 = (W_Prev_PTS - P_Curr_PTS) / 45; 

  sprintf(szMsgTxt,"%s Seq Err: %s < %s. %ums Ch=%o Loc=%d",
                     P_Type, szBuffer, szTmp1, iTmp1, P_Prev_IX,
                                                (int)(process.PACK_Loc));

   //Prev_PTS[P_Prev_IX] = P_Curr_PTS;
}



static int iPkt_2Use_Len, iPkt_Avail_Len;

// There is something wrong with PTS calc,
// either here or in the display section.
// OR in the choice of Time field ???

// EG  7D8C2300 (Major part, MSBF)
// PowerDVD says 0h 25m 47s
//          NOT  0h 27m 36s


//-----------------------------------------------


//__forceinline static
unsigned Get_PTS(int P_Char1)
{
  unsigned uPTS;

  if (P_Char1)
    process.Curr_TSM[0] =  (unsigned char)(P_Char1   & 0x0e);
  else
  {
    process.Curr_TSM[0] =  (unsigned char)(Get_Byte()& 0x0e);
    getbit_iPkt_Hdr_Len_Remaining--;
  }

  process.Curr_TSM[1] =  (unsigned char)(Get_Byte());
  process.Curr_TSM[2] =  (unsigned char)(Get_Byte()& 0xfe);
  process.Curr_TSM[3] =  (unsigned char)(Get_Byte());
  process.Curr_TSM[4] =  (unsigned char)(Get_Byte()& 0xfe);


  uPTS =       ((unsigned)(process.Curr_TSM[0])<<28)
             | ((unsigned)(process.Curr_TSM[1])<<21)
             | ((unsigned)(process.Curr_TSM[2])<<13)
             | ((unsigned)(process.Curr_TSM[3])<<06)
             | ((unsigned)(process.Curr_TSM[4])>>02); // Discard low-order bit#33

  getbit_iPkt_Hdr_Len_Remaining -= 4;

#ifdef DBG_RJ
  if (DBGflag)
  {
     sprintf(szDBGln, " PTS %c %08u", uGot_Pkt_Type, uPTS);
     DBGout(szDBGln);

  }
#endif


  return (uPTS);
}





/*
//-------------------------------------------
unsigned int Get_Bits_All(unsigned int N)
{
  register Val;
  N -= BitsLeft;
  Val = (CurrentBfr << (32 - BitsLeft)) >> (32 - BitsLeft);

  if (N)
    Val = (Val << N) + (NextBfr >> (32 - N));

  CurrentBfr = NextBfr;
  BitsLeft = 32 - N;
  InputBuffer_NEXT_fill(0);

  return Val;
}
*/




//-------------------------------------------------------------------
// read a block and scan for the next packet
void getBLOCK_Packet(int P_NewBuf)
{

#ifdef DBG_RJ
  if (DBGflag)
  {
      sprintf(szBuffer, "    getBLOCK  Sys=%d\n", MParse.SystemStream_Flag);
      DBGout(szBuffer);
  }
#endif

  if (process.Action == ACTION_RIP && iPreview_Clip_Ctr < iEDL_ctr)
  {

       if ((MParse.NextLoc >= process.endLoc 
            && (RdEOB - RdPTR) < 8192
            &&  File_Ctr == process.endFile) 
       ||  File_Ctr  > process.endFile) 
       {

          C160_Clip_Preview();
          File_Ctr = process.startFile;
          _lseeki64(FileDCB[File_Ctr], process.startLoc, SEEK_SET );
          MParse.NextLoc = process.startLoc;
 
          P_NewBuf = 1;
          // RdEndPkt = RdPTR = RdEOB;
          // RdEndPkt_4 = RdEndPkt-4;  RdEndPkt_8 = RdEndPkt-8;
          // BitsLeft = 0;
       }
  }

  if (P_NewBuf)
  {
     if (DBGflag)
     {
        sprintf(szDBGln, "Forcing New Block.  SysTyp=%d", 
                                                 MParse.SystemStream_Flag);
        DBGout(szDBGln);
     }

    RdPTR = RdBFR + MPEG_SEARCH_BUFSZ;  // force a new read
    RdEOB = RdBFR;  RdEOB_4 = RdEOB-4;  RdEOB_8 = RdEOB-8;
    RdEndPkt = RdPTR;   RdEndPkt_4 = RdEndPkt-4;  RdEndPkt_8 = RdEndPkt-8;
    BitsLeft = 0;
  }

  if (MParse.SystemStream_Flag)
  {
    if (RdPTR >= RdEndPkt)
    {
       if (DBGflag)
       {
          DBGout(" Get 1st pkt in New Block");
       }
       Get_Next_Packet();
    }

    CurrentBfr = *RdPTR++ << 24;

    if (RdPTR >= RdEndPkt)
       Get_Next_Packet();
    CurrentBfr += *RdPTR++ << 16;

    if (RdPTR >= RdEndPkt)
       Get_Next_Packet();
    CurrentBfr += *RdPTR++ << 8;

    if (RdPTR >= RdEndPkt)
       Get_Next_Packet();
    CurrentBfr += *RdPTR++;
  }
  else
  {
    InputBuffer_FILL();

    CurrentBfr =    (*RdPTR    << 24) + (*(RdPTR+1) << 16)
                 + (*(RdPTR+2) << 8)  +  *(RdPTR+3);
    RdPTR += 4;
  }

  InputBuffer_NEXT_fill(0);
  BitsLeft = 32;
}





//-------------------------------------
void InputBuffer_FILL()
{
  Mpeg_READ();

  /*
  if (KeyOp_Flag && (RdBFR[20] & 0x10))
  {
    BufferOp(RdBFR, lfsr0, lfsr1);
    RdBFR[20] &= ~0x10;
  }
  */

  RdPTR = RdBFR;

  if (MParse.SystemStream_Flag)
  {
    RdEndPkt -= MPEG_SEARCH_BUFSZ; 
    RdEndPkt_4 = RdEndPkt-4; RdEndPkt_8 = RdEndPkt-8;
  }
  else
    iVideoBitRate_Bytes += File_ReadLen;
}


//-------------------------------------------------
// Fill the Get_Bits Routine's forward bit buffer (NextBfr)

// static 
void InputBuffer_NEXT_fill(int P_Bias)
{

BiasLoop:
  // Near the end of the packet - slow down to byte grabs
  if (MParse.SystemStream_Flag && RdPTR >= RdEndPkt_4)
  {
    if (RdPTR >= RdEndPkt)
      Get_Next_Packet();

    NextBfr = Get_Byte() << 24;

    if (RdPTR >= RdEndPkt)
      Get_Next_Packet();

    NextBfr += Get_Byte() << 16;

    if (RdPTR >= RdEndPkt)
      Get_Next_Packet();

    NextBfr += Get_Byte() << 8;

    if (RdPTR >= RdEndPkt)
      Get_Next_Packet();

    NextBfr += Get_Byte();
  }

  // NOT Near the edge - grab 4 bytes at once
  else 
  if (RdPTR < RdBFR+MPEG_SEARCH_BUFSZ-4)
  {
    // Optional acceleration when trying to find next start code
    if (P_Bias  && (*(UNALIGNED unsigned*)(RdPTR) & 0xFE0000FF)) // Skip word that DONT start with 00 NOR end with either (00 or 01)
    {
        CurrentBfr = 0xFEFEFEFE; // dummy to allow for skipped boring data
        RdPTR += 4;
        goto BiasLoop;
    }
    
    NextBfr = (*RdPTR << 24) + (*(RdPTR+1) << 16)
                             + (*(RdPTR+2) <<  8)
                             +  *(RdPTR+3);
    RdPTR += 4;
  }
  else
  {
    // Elementary Streams don't have any packet boundaries to worry about
    if (RdPTR >= RdEOB)   //RdBFR+MPEG_SEARCH_BUFSZ)
        InputBuffer_FILL();

    NextBfr = *RdPTR++ << 24;

    if (RdPTR >= RdEOB)   //RdBFR+MPEG_SEARCH_BUFSZ)
        InputBuffer_FILL();

    NextBfr += *RdPTR++ << 16;

    if (RdPTR >= RdEOB)   //RdBFR+MPEG_SEARCH_BUFSZ)
        InputBuffer_FILL();

    NextBfr += *RdPTR++ << 8;

    if (RdPTR >= RdEOB)   //RdBFR+MPEG_SEARCH_BUFSZ)
      InputBuffer_FILL();

    NextBfr += *RdPTR++;
  }
}




//--------------------------------------------------------
// Calculate a pointer to where the next byte will be,
// allowing for what is already in the Get_Bit buffers
BYTE* Mpeg_BytePtr()  // MAY NOT BE RELIABLE YET !
{

  BYTE* lp_Calc;

  lp_Calc = RdPTR;

  if (BitsLeft)
      lp_Calc = lp_Calc - ((BitsLeft +7) /8) - 4;

  return lp_Calc;
}


int iTmp1, iTmp2, iTmp3;

//--------------------------------------------------------




void PTS_Video_Analysis()
{
    
  // Calculate increment in PTS between GOPs
  //       allowing for unsigned PTS field
  if (CandidatePTS > process.VideoPTS)
  {
    process.iVid_PTS_Diff = (CandidatePTS - process.VideoPTS)
                                / 45;
  }
  else
  {
    process.iVid_PTS_Diff = (process.VideoPTS - CandidatePTS)
                                / -45;
  }


  if (process.VideoPTS != CandidatePTS)
  {
      process.VideoPTS = CandidatePTS;
      if (process.iVid_PTS_Frames)
      {
          process.iVid_PTS_Resolution = process.iVid_PTS_Frames;
          process.iVid_PTS_Frames = 0;
      }
  }
  
  memcpy(&process.VideoPTSM, &process.Curr_TSM[0], 4);

  //getbit_input_code = Get_Byte();
  //process.VideoPTS  = (getbit_input_code      & 0x0e)   << 29;
  //process.VideoPTS |= (Get_Short()     & 0xfffe) << 14;
  //process.VideoPTS |= (Get_Short()>>1) & 0x7fff;
  //      //process.VideoPTS /= 90;

  PTS_Flag[0] = 'V';

  // OOPS - should be checking the DTS for video seq, NOT the PTS !
  //if (process.Action == ACTION_FWD_GOP
  // && uCtl_Video_Stream < STREAM_AUTO
  // && process.VideoPTS < Prev_PTS[24])
  //    PTS_Err_Msg(24, process.VideoPTS, "Video");
  //Prev_PTS[24] = process.VideoPTS;

  if (process.VideoPTS != process.SkipPTS  || !process.SkipPTS)
  {
      getbit_iDropPkt_Flag = 0;
      process.SkipPTS = 0;
  }


  /*
  if (iCtl_MultiAngle)
  {
     // What about the DECODE Time Stamp ?
     if ( ! (getbit_PES_HdrFld_Flags & 0x40) ) // Is there a DTS field ?
     {
              //getbit_iDropPkt_Flag = 0;
     }
     else
     {
         process.VideoDTS = Get_PTS(0);

         // Checking the DTS for video seq, NOT the PTS !

          if (process.VideoDTS < Prev_PTS[24])
          {
              if (iCtl_MultiAngle)
              {
                 if (process.Action == ACTION_RIP)
                     getbit_iDropPkt_Flag = 1;
              }
              else
              if (process.Action == ACTION_FWD_GOP
              &&  uCtl_Video_Stream < STREAM_AUTO)
              {
                  PTS_Err_Msg(24, process.VideoDTS, "Video");
              }

              if (! getbit_iDropPkt_Flag)
                  Prev_PTS[24] = process.VideoDTS;

              //if (process.VideoPTS != process.SkipPTS  || !process.SkipPTS)
              //{
              //  getbit_iDropPkt_Flag = 0;
              //  process.SkipPTS = 0;
              //}
          } // ENDIF SEQ ERR
     } // END - GOT A DTS

  } // END MultiAngle Checking
  */

}






//----------------------------------------------------------
  static int i, iRC;
  // static int getbit_AC3_1stTime,
  static int iAdapt, iTS_VID_hdr;

  unsigned uTmp1, uTmp2;

#ifdef DBG_RJ
  __int64 i64TmpLoc;
  int     iTmpFile;
#endif

  //DWORD uTmp99;

  //char cMPA_ID;
  //int iBlkCtr;



//---------------------------------------------------------
void  Mpeg1_PesHdr()       // Mpeg-1
{
  int iSentinel;
  iSentinel = Mpeg_PES_Byte1;
  while (iSentinel >= 0x80 && getbit_iPkt_Len_Remain > 0) // Mpeg-1 stuffing bytes
  {
     iSentinel = Get_Byte();
     getbit_iPkt_Len_Remain--;
  }

  if (iSentinel >= 0x40    && getbit_iPkt_Len_Remain > 0) // Mpeg-1 STD Buffer
  {
      Get_Byte();
      iSentinel = Get_Byte();
      getbit_iPkt_Len_Remain -=2;
  }

  if (iSentinel >= 0x30    && getbit_iPkt_Len_Remain > 0) // Mpeg-1 PTS + DTS
  {
      getbit_iPkt_Hdr_Len_Remaining = 9;
  }
  else
  if (iSentinel >= 0x20    && getbit_iPkt_Len_Remain > 0)// Mpeg-1 PTS
  {
      getbit_iPkt_Hdr_Len_Remaining = 4;
  }
  else
      getbit_iPkt_Hdr_Len_Remaining = 0;



  if (iSentinel >= 0x20   && getbit_iPkt_Len_Remain > 0)
  {
     CandidatePTS = Get_PTS(iSentinel);
     getbit_iPkt_Len_Remain -= 4;
     if (uGot_Pkt_Type == 'V')
        PTS_Video_Analysis();
     else
        PTS_Audio_Analysis();
  }


}


//----------------------------------------------------------


void Got_PS2_Pkt(int P_BitMode)
{
  Got_PS2_NAV(0);
  process.SkipPTS = 0;

  //iAudio_Trk_FMT[7] = FORMAT_PS2;
  //getbit_AUDIO_ID = 0xCCCC;

  if (P_BitMode)
      getbit_iPkt_Len_Remain = Get_Bits(16);
  else
      getbit_iPkt_Len_Remain = Get_Short();


  if (DBGflag)
  {
      sprintf(szBuffer, "PS2_L%d", getbit_iPkt_Len_Remain);
      DBGout(szBuffer);
  }

  // Allow VOB preservation if VOB NAV PACK found in non-VOB file
  if (! iInPS2_Audio)
  {
    if (getbit_iPkt_Len_Remain ==  980
    ||  getbit_iPkt_Len_Remain == 1018)
    {
        MParse.iVOB_Style =  iCtl_VOB_Style;
        MParse.iGOPsSinceNAV = 0;
    }
  }

  if (MParse.SystemStream_Flag < 0)
      getbit_iPkt_Len_Remain = RdEndPkt - RdPTR;


  if (getbit_iPkt_Len_Remain < 5)
  {
      sprintf(szBuffer, "PS2_L%d", getbit_iPkt_Len_Remain);
      RdPTR  += 3;
  }
  else
  {
      RdPTR++;
      getbit_PES_HdrFld_Flags = Get_Byte(); // Optional field Flag bits
      getbit_iPkt_Hdr_Len_Remaining  = Get_Byte();
      sprintf(szBuffer, "PS2.%x L%d H%d",
                        getbit_input_code, getbit_iPkt_Len_Remain, getbit_iPkt_Hdr_Len_Remaining);
      /*
            if (getbit_PES_HdrFld_Flags >= 0x80) // Is there a PTS ?
            {
                process.AudioPTS = Get_PTS(0);

                PTS_Flag[3] = '2';
            }

            RdPTR += getbit_iPkt_Hdr_Len_Remaining;

            getbit_AUDIO_ID = Get_Byte();
            sprintf(szBuffer, "PS2_%x", getbit_AUDIO_ID);

            getbit_iPkt_Len_Remain -= (getbit_iPkt_Hdr_Len_Remaining+4);
      */
  }

  SetDlgItemText(hStats, IDC_PS2_STATUS, szBuffer );


  if (++getbit_VOBCELL_Count == 2)
  {
      RdPTR  += 22 ; //25;
      getbit_VOB_ID  = /*(unsigned short)*/ Get_Short();  //RJ CAREFUL OF CAST
      getbit_CELL_ID = /*(unsigned short)*/ Get_Short();  //      "
      RdPTR  += (getbit_iPkt_Len_Remain - 29);

      if (process.Action == ACTION_RIP
      &&  iWant_VOB_ID < 0)
          iWant_VOB_ID = getbit_VOB_ID;

      sprintf(szBuffer, "Vob%d Cell%d", getbit_VOB_ID, getbit_CELL_ID);
      if (strcmp(StatsPrev.VobTxt, szBuffer))
      {
          SetDlgItemText(hStats, IDC_VOB_ID, szBuffer);

          if (MParse.SeqHdr_Found_Flag) // Allow for clearing of Status line when first SEQ found
              strcpy(StatsPrev.VobTxt, szBuffer);

          if (iMsgLife > 0 && !szMsgTxt[0])
            StatsPrev.VobTxt[0] = 0;
          else
          {
            if (process.Action != ACTION_INIT)
            {
                strcpy(szMsgTxt, szBuffer);
                iMsgLife = 1;
            }
          }
      }
  } 
  else
     RdPTR += getbit_iPkt_Len_Remain - 3;

  if (P_BitMode)
      RdPTR -=8;

#ifdef DBG_RJ
              if (DBGflag)
                  DBGln4("\nNAV.   NAVLoc=%x  PKT Len=%d  VobID=%d  CellID=%d\n\n",
                           process.NAV_Loc, getbit_iPkt_Len_Remain,
                           getbit_VOB_ID, getbit_CELL_ID) ;
#endif

}





//----------------------------------------------------------
void Get_Next_Packet()
{
  //char cTmp1;
  int iTmp1, iLen;
  unsigned uTmp1;
  MENUITEMINFO PIDmenu;
  const unsigned int uPID_MenuId[16] =
  { 
    IDM_PID_1, IDM_PID_2, IDM_PID_3, IDM_PID_4, IDM_PID_5, IDM_PID_6,
    IDM_PID_7, IDM_PID_8, IDM_PID_9, IDM_PID_10, IDM_PID_11, IDM_PID_12,
    IDM_PID_13, IDM_PID_14, IDM_PID_15, IDM_PID_16
  };
  //iBlkCtr = 0;

#ifdef DBG_RJ
  if (DBGflag)
  {
      sprintf(szDBGln, "GET PACKET %03d Sys=%d Fault=%d Stop=%d",
                         PktStats.iChk_AnyPackets,
                         MParse.SystemStream_Flag,
                         MParse.Fault_Flag, MParse.Stop_Flag);
      DBGout(szDBGln);
      i64TmpLoc = Calc_Loc(&iTmpFile, 0, 0);
      DBGln4("           Loc=x%06X, RdPtr=x%04X EOP=x%04X EOB=x%04X",
               i64TmpLoc, 
               (__int64)(RdPTR-RdBFR), 
               (__int64)(RdEndPkt-RdBFR),
               (__int64)(RdEOB-RdBFR));
  }
#endif


  while (MParse.Fault_Flag < CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
    && ! MParse.Stop_Flag )
  {

    // Preview multi-clips
    if (process.Action == ACTION_RIP && iPreview_Clip_Ctr < iEDL_ctr
    && MParse.Summary_Adjust < 0
    
    && ((MParse.NextLoc >= process.endLoc 
            && (RdEOB - RdPTR) < 8192
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

          Mpeg_READ(); 
    }

    PktStats.iChk_AnyPackets++;
    // NEBULA capture routines drops video frames when CPU hits 100%,
    // but audio keeps going, leaving vision jerky or frozen for duration
    if (PktStats.iChk_AudioPackets >  PktChk_Audio
    ||  PktStats.iChk_AnyPackets   >  PktChk_Any)
    {
         GetChkPoint();
         if (MParse.Stop_Flag)
         {
                if (DBGflag)
                {
                  DBGout("FALL_OUT 1");
                }

            MParse.Fault_Flag = 97;
            return;
         }
    }


    if (MParse.SystemStream_Flag < 0) // Transmission stream (TS or PVA) ?
    {
Transmission_Find:
     if (MParse.SystemStream_Flag == -1) // Transport stream (TS) ?
     {
Transport_Find:
        getbit_input_code = Get_Byte();
        iTmp1 = getbit_input_code;
        if (getbit_input_code != 0x47)
        {
          PktStats.iTS_ReSyncs++;
          while (getbit_input_code != 0x47)
          {
            if (  MParse.Fault_Flag >= CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
               || MParse.Stop_Flag )
            {
                if (DBGflag)
                {
                  DBGout("FALL_OUT 2");
                }

                return;
            }

            PktStats.iTS_BadBytes++;
            getbit_input_code = Get_Byte();
          }
        }

        {
           PktStats.iTS_Packets++;
           process.PREV_Pack_Loc  = process.PACK_Loc;
           process.PREV_Pack_File = process.PACK_File;
           process.PACK_Loc  = Calc_Loc(&process.PACK_File, -1, 0) ; // PERFORMANCE HIT ?
        }

        uTmp1 =  Get_Short();

        /*
        if (uTmp1 & 0x80) // Transport error ?
        {
           if (DBGflag)
           {
               DBGout("TS PKT ERROR FLAG");
           }
           goto Transport_Find;
        }
        */

        uGot_PID = uTmp1 & 0x1FFF;

        //  PAT control packet ?
        if (!uGot_PID && getbit_input_code == 0x47)
        {
           // Remember first PAT location
           if (process.PAT_Loc < 0)
           {
               process.PAT_Loc  = process.PACK_Loc;
               process.PAT_File = process.PACK_File;
           }

           // Remember PAT data
           if (!uGot_PID && process.PAT_Len < (sizeof(PAT_Data) - 189) )
           {
              RdEndPkt = RdPTR + 186;
              if (RdEndPkt < RdEOB)
              {
                 PAT_Data[process.PAT_Len] = 0x47;
                 process.PAT_Len++;
                 PAT_Data[process.PAT_Len] = (unsigned char )(uTmp1 / 256);
                 process.PAT_Len++;
                 PAT_Data[process.PAT_Len] = (unsigned char )(uTmp1 & 0xFF);
                 process.PAT_Len++;
                 memcpy(&PAT_Data[process.PAT_Len], RdPTR, 186);
                 process.PAT_Len +=186; // includes next sync byte
                 
                 // trim to sync byte to allow for short packet
                 /*
                 while (PAT_Data[process.PAT_Len] != 0x47)
                 {
                     if (DBGflag)
                     {
                         DBGout("  TRIM PAT");
                     }

                     process.PAT_Len--;
                 }
                 */
                 
                 process.PAT_Len--;
              }
           }
        }


        iAdapt   = (Get_Byte()>>4) & 3;
        if (iAdapt > 1)
        {
          getbit_iPkt_Hdr_Len_Remaining = Get_Byte();
          RdPTR += getbit_iPkt_Hdr_Len_Remaining; // Skip rest of Header
          MPEG_PTR_BUFF_CHK
          getbit_iPkt_Hdr_Len = getbit_iPkt_Hdr_Len_Remaining + 1;
        }
        else
          getbit_iPkt_Hdr_Len = 0;

        /*
        if (!uGot_PID && process.PAT_Len < (sizeof(PAT_Data) - 188) )
        {
        }
        */

        if (iAdapt == 0 || iAdapt == 2)
        {
           if (DBGflag)
           {
               DBGout("TS PKT NO PAYLOAD");
           }
           goto Transport_Find;
        }


        getbit_iPkt_Len_Remain = 184 - getbit_iPkt_Hdr_Len;

        RdEndPkt = RdPTR + getbit_iPkt_Len_Remain; RdEndPkt_4 = RdEndPkt-4; RdEndPkt_8 = RdEndPkt-8;
        


#ifdef DBG_RJ
        if (DBGflag)
        {
           sprintf(szBuffer,"TS PKT#%03d [x%02X]  Len=%d  PID=x%04X=%03d Adapt=%d",
               PktStats.iTS_Packets, iTmp1,
               getbit_iPkt_Len_Remain, uGot_PID, uGot_PID, getbit_iPkt_Hdr_Len) ;
           DBGout(szBuffer);
        }
#endif

        iTS_VID_hdr = 0;

        if (getbit_iPkt_Len_Remain < 4)
        {
          //RdPTR = RdEndPkt;
          //MPEG_PTR_BUFF_CHK
           if (DBGflag)
           {
               DBGout("TS PKT LENGTH ERROR");
           }
          goto Transport_Find;
        }

        {
          if (!RdPTR[0] && !RdPTR[1] && RdPTR[2] == 0x01
            && RdPTR[3]  >= 0xE0     && RdPTR[3] <= 0xEF) // VIDEO STREAM ?
          {
             iTS_VID_hdr = 1;

             if (DBGflag)
             {
               DBGout("*VIDEO* PES HDR FOUND");
             }
          }
        }

#ifdef DBG_RJ
            if (DBGflag)
            {
               sprintf(szBuffer,"     PID=x%04X=%03d CTL_PID=x%04X=%03d  Match=%d",
                      uGot_PID, uGot_PID, uCtl_Vid_PID, uCtl_Vid_PID, 
                                     (uGot_PID == uCtl_Vid_PID)) ;
               DBGout(szBuffer);
            }
#endif
        // Some packets are more interesting than others

        if (! iTS_VID_hdr)
        {

           if (DBGflag)
           {
                 DBGout("  NO Video PES Hdr");
           }

           if (uGot_PID == uCtl_Vid_PID) // Selected PID - data without PES hdr
           {
              if (DBGflag)
              {
                 DBGout("  Pid# Matches OK");
              }
               return;            //   <======  ESCAPE POINT ========== <<<
           }
           else
           if (uGot_PID == uCtl_Aud_PID)
           {
              if (iPlayAudio)
              {
                 getbit_iPkt_Len_Remain = RdEndPkt - RdPTR; // Maybe unnecessary ????
                 if (uCtl_Aud_Stream == cPRIVATE_STREAM_1
                 &&  SubStream_CTL[FORMAT_AC3][0].rip)  // Has this track been setup ?
                 {
                     PS1_Convert();
                     //goto Transport_Find;
                 } // ENDIF Track set up ?
                 else
                 if (uCtl_Aud_Stream == 0xC0
                  && mpa_Ctl[0].rip)
                 {
                     Got_MPA_PayLoad();
                     //goto Transport_Find;
                 } // ENDIF Track set up ?

              }  // ENDIF iPlayAudio
              goto Transport_Find;
           }
           else
           {  

             
             if (uCtl_Aud_PID == STREAM_AUTO // Looking For Audio Track
             &&  uGot_PID < 999990
             &&  !RdPTR[0] && !RdPTR[1] && RdPTR[2] == 0x01)
             {
               uTmp1 = RdPTR[3];
               if (  (uTmp1 >= 0xC0  && uTmp1 <= 0xCF   // MPA AUDIO STREAM ?
                      && iWant_Aud_Format <= FORMAT_MPA )
                   ||
                     (uTmp1 == cPRIVATE_STREAM_1 // 0xBD  // PS1 AUDIO STREAM ? 
                      && iWant_Aud_Format !=  FORMAT_MPA)  // Non-mpeg - usually AC3
                  )
               {
                 if (DBGflag)
                 {
                    DBGout("*AUDIO* PES HDR FOUND");
                 }

                 if (uCtl_Vid_PID < STREAM_AUTO // We have previously synchronized
                 &&  uCtl_Vid_PID < uGot_PID)    // Australia - common practice    
                 {
                     uCtl_Aud_PID    = uGot_PID;
                     uCtl_Aud_Stream = uTmp1;
                 }
               }
             }
             else
             
             {

               // Data packet without PES hdr is otherwise useless
               if (uCtl_Vid_PID < STREAM_AUTO) // We have previously synchronized
               {
                   RdPTR = RdEndPkt;   // Skip to end of packet
                   MPEG_PTR_BUFF_CHK
               }

               if (DBGflag)
               {
                  sprintf(szDBGln, "  REJECTED.");
                  DBGout(szDBGln);
               }
               goto Transport_Find;

             } // end NOT interesting Audio
           }  // end NOT selected PID

        } // END Headerless packet

        else
        {  // Got a PES hdr within the packet

           // On change of PID, look it up in table
           if (uGot_PID != uCtl_Vid_PID  &&  uGot_PID != uPrev_PID )
           {
             for (uTmp1=0; uTmp1<16; uTmp1++)
             {
                uTmp2 = uPID_Map[uTmp1];
                if (uTmp2 == STREAM_AUTO)
                {
                   uPID_Map[iTmp1] = uGot_PID;
                   iLen = sprintf(szTmp32, "PID %03d", uGot_PID);
                   ZeroMemory(&PIDmenu, sizeof(PIDmenu));
                   PIDmenu.cbSize     = sizeof(PIDmenu); 
                   PIDmenu.fType      = MFT_STRING; 
                   PIDmenu.dwTypeData = &szTmp32[0];
                   PIDmenu.cch        = iLen;
                   //SetMenuItemInfo(hMenu, uPID_MenuId[iTmp1], 0, &PIDmenu	);
                   break;
                }
                else
                if (uTmp2 == uGot_PID)
                {
                   break;
                }
             }
           }
           uPrev_PID = uGot_PID;

           /*
           if (uCtl_Vid_PID < STREAM_AUTO)
           {
              // Unwanted PID, but PES hdr is worth remembering

              // Allow for manual inspection
              //    of available PIDs that are interleaved

              if (process.Action != ACTION_RIP
              &&  iTS_VID_hdr
              &&  process.ALTPID_Loc <= 0)
              {
                 process.ALTPID_Loc  = Calc_Loc(&process.ALTPID_File, -1, 0) ; 
              }

              RdPTR = RdEndPkt;
              MPEG_PTR_BUFF_CHK
              goto Transport_Find;
           
           } // END Boring PID
           */

         } // END PES HDR PKT

     } // END TS packet

     else
     {
       // PVA Format
PVA_Find:
        getbit_input_code = Get_Byte();
        //iTmp1 = getbit_input_code;
        while (getbit_input_code != 0x4156) // 'AV' = PVA  Stream
        {
          if (  MParse.Fault_Flag >= CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
             || MParse.Stop_Flag )
              return;

          getbit_input_code = (getbit_input_code & 0xFF) * 256 + Get_Byte();
        }

        {
           process.PREV_Pack_Loc  = process.PACK_Loc;
           process.PREV_Pack_File = process.PACK_File;
           process.PACK_Loc  = Calc_Loc(&process.PACK_File, -2, 0) ; // PERFORMANCE HIT ?
        }

        uGot_PID = Get_Byte();
        uTmp1    = Get_Short();
        uTmp2    = Get_Byte(); // PTS,DTS Flags
        process.Got_PTS_Flag = uTmp2 & 0x10;
        getbit_iPkt_Len_Remain = Get_Short(); // Length of packet payload area


        if (process.Got_PTS_Flag)   // Video PTS in this pack ?
        {
           if (uGot_PID == 1)
           {
               CandidatePTS =  (Get_Short()<16) ||  Get_Short();  // Get the 32bit PTS
               PTS_Video_Analysis();
               getbit_iPkt_Len_Remain -=4;
           }


          //getbit_iPkt_Hdr_Len_Remaining = uTmp2 & 0x3;
          //if (getbit_iPkt_Hdr_Len_Remaining)
          //{

          //    MPEG_PTR_BUFF_CHK
          //}


        }



        RdEndPkt = RdPTR + getbit_iPkt_Len_Remain; RdEndPkt_4 = RdEndPkt-4; RdEndPkt_8 = RdEndPkt-8;


#ifdef DBG_RJ
        if (DBGflag)
        {
           sprintf(szBuffer,"PVA PKT @x%04X  Len=%d  PID=x%04X=%03d Flags=%d",
               (int)(process.PACK_Loc),
               getbit_iPkt_Len_Remain, uGot_PID,  uGot_PID, uTmp2) ;
           DBGout(szBuffer);
        }
#endif

        if (getbit_iPkt_Len_Remain < 1)
          goto PVA_Find;


        if (uGot_PID == 1) // VIDEO STREAM ?
        {
            uVid_PID = uGot_PID;
            PktStats.iVid_Packets++;
            iVideoBitRate_Bytes += getbit_iPkt_Len_Remain; //RdEndPkt-RdPTR;

            return;            //   <======  ESCAPE POINT ========== <<<
        }

        // Usually suppress Audio payload
        // BUT experimentally let it get through 
        //     as it should be proper PES format.
        if (uCtl_Aud_PID == STREAM_NONE)
            goto PVA_Find;

     } // END PVA format packet

       getbit_input_code = Get_Short();
       getbit_input_code = (getbit_input_code<<16) + Get_Short();

       // skip until code found with valid Mpeg Code sentinel prefix
       while (  MParse.Fault_Flag < CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
           && ! MParse.Stop_Flag
           &&( (getbit_input_code & 0xffffff00) != 0x00000100))
       {
            if (RdPTR >= RdEndPkt) // Allow for stray continuation packets
                goto Transmission_Find;
            getbit_input_code = (getbit_input_code<<8) + Get_Byte();
       }

    } // END Transmission Stream Handling
    else
    {  // Program Stream or Elementary stream
       getbit_input_code = Get_Short();
       getbit_input_code = (getbit_input_code<<16) + Get_Short();

       // skip until code found with valid Mpeg Code sentinel prefix
       while (  MParse.Fault_Flag < CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
           && ! MParse.Stop_Flag
           &&( (getbit_input_code & 0xffffff00) != 0x00000100) )
       {
            getbit_input_code = (getbit_input_code<<8) + Get_Byte();
       }
    }

#ifdef DBG_RJ
    if (DBGflag)
    {
        i64TmpLoc = Calc_Loc(&iTmpFile, -4, 0) ;
        sprintf(szBuffer, "      GOT CODE=%08X Loc=x%08X  RdPtr=%08X EOB=%08X", getbit_input_code, (int)i64TmpLoc, RdPTR, RdEOB);
        DBGout(szBuffer);
    }
#endif


    //switch (getbit_input_code)
    //{
    if (  MParse.Fault_Flag >= CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
       || MParse.Stop_Flag )
    {
        return;
    }
    else
    if (getbit_input_code >= VIDEO_ELEMENTARY_STREAM_1
    &&  getbit_input_code <= VIDEO_ELEMENTARY_STREAM_16)
    {
       uGot_Pkt_Type = 'V';
       uGot_Video_Stream = getbit_input_code;

       if (process.PACK_Loc == -1)
       {
           process.VIDPKT_Loc = Calc_Loc(&process.VIDPKT_File, -4, 0) ;
       }

       getbit_iPkt_Len_Remain = Get_Short();

      // Transport stream PES length does not apply - calc from TS packet
      if (MParse.SystemStream_Flag < 0)
          getbit_iPkt_Len_Remain = RdEndPkt - RdPTR;
      else
      {
          RdEndPkt = RdPTR + getbit_iPkt_Len_Remain; RdEndPkt_4 = RdEndPkt-4; RdEndPkt_8 = RdEndPkt-8;
      }

     getbit_iDropPkt_Flag = 1;

     
     if (iCtl_MultiAngle
     &&  process.Action == ACTION_RIP
     &&  iWant_VOB_ID >= 0
     &&  iWant_VOB_ID != getbit_VOB_ID)
     {
         getbit_iDropPkt_Flag = 1;
     }
     else
     if ((getbit_input_code == uCtl_Video_Stream || uCtl_Video_Stream >= STREAM_AUTO)
     &&  (uCtl_Vid_PID      == uGot_PID          || uCtl_Vid_PID      >= STREAM_AUTO))
     {
         if (uCtl_Video_Stream == STREAM_AUTO)
             uCtl_Video_Stream = uGot_Video_Stream;

         if (uCtl_Vid_PID == STREAM_AUTO) //  || process.Action == ACTION_RIP)
             uCtl_Vid_PID = uGot_PID;
         uVid_PID = uGot_PID;

         PktStats.iVid_Packets++;
         iVideoBitRate_Bytes += getbit_iPkt_Len_Remain; //RdEndPkt-RdPTR;

         Mpeg_PES_Byte1 = Get_Byte();
         Mpeg_PES_Version = Mpeg_PES_Byte1>>6;

         /*
         if ( (Mpeg_PES_Byte1 & 0xE0) == 0xA0   // Mpeg2 Scrambling ? 
         &&  ! process.iEncryptAlerted)
         {
            process.iEncryptAlerted = 1;
            iRC = MessageBox(hWnd_MAIN, "ENCRYPTED data - Probably won't work.\n\nCONTINUE ?",  "Mpg2Cut2 - Warning", MB_OKCANCEL);
            if (iRC != IDOK)
                MParse.Stop_Flag = 1;           
         }
         */

         /*
         // Quality check - SCR should always increase within a PES, never decrease
         if (process.Action  >= 0
         && (memcmp(&process.CurrSSCRM[0], &process.PrevSSCRM[0], sizeof(process.CurrSSCRM)) < 0)
         && !MParse.Stop_Flag
         &&  MParse.Fault_Flag < CRITICAL_ERROR_LEVEL)
         {
             sprintf(szMsgTxt,"SCR SEQ ERR. Loc=%d",
                                                (int)(process.PACK_Loc));
         }
         */

         memcpy(&process.PrevSSCRM[0], &process.CurrSSCRM[0], sizeof(process.CurrSSCRM));

         //if (Mpeg_PES_Version == 2     // Mpeg-2 format ? //if ((getbit_input_code & 0xc0) == 0x80)
         //||  iPES_Mpeg_Any)
         {
           if (Mpeg_PES_Version != 2)  // Mpeg-1 - PES Header is different layout
           {
              getbit_iDropPkt_Flag = 0;
              Mpeg1_PesHdr();
           }
           else
           { // Mpeg-2 PES
             getbit_PES_HdrFld_Flags = Get_Byte();
             process.Got_PTS_Flag    = getbit_PES_HdrFld_Flags & 0x80;
             getbit_iPkt_Hdr_Len_Remaining  = Get_Byte();

             if (! process.Got_PTS_Flag) // Is there NO PTS field ?
             {
                if (iDrop_B_Now_flag && PlayCtl.iDrop_PTS_Flag)
                {
                    getbit_iDropPkt_Flag = 1;          // skip
                    PktStats.iSubTit_Packets++;
                }
                else
                    getbit_iDropPkt_Flag = 0;          // ok
             }
             else
             {
               CandidatePTS = Get_PTS(0);
               PTS_Video_Analysis();
             } // END-IF ELSE - GOT A PTS


           } // END-IF Mpeg-2 ?

           RdPTR += getbit_iPkt_Hdr_Len_Remaining; // Skip rest of Header

         } // END-IF Process-ANY or Mpeg-2
      } // END-IF Wanted Angel
      else
        getbit_iDropPkt_Flag = 1;

      if (getbit_iDropPkt_Flag)
          RdPTR = RdEndPkt;
      else
         return;                 //   <======  ESCAPE POINT ========== <<<

        //break;
    } // ENDIF Video Stream

    else
    if (getbit_input_code == PACK_START_CODE)
    {
         if (process.PACK_Loc >= 0
         || (process.Action != ACTION_FWD_GOP
          && process.Action != ACTION_INIT))
         {
            process.PREV_Pack_Loc  = process.PACK_Loc;
            process.PREV_Pack_File = process.PACK_File;
         }
         process.PACK_Loc  = Calc_Loc(&process.PACK_File, -4, 0) ; // PERFORMANCE HIT ?

#ifdef DBG_RJ
         if (DBGflag && process.PACK_Loc < 0xAA00)
             DBGln2("\n  PackLoc=x%04X (%d) *SEQ*\n", process.PACK_Loc, process.PACK_Loc);
#endif


                 //uTmp99 = *(DWORD*)(RdPTR);
         if ( (   process.Action >= ACTION_FWD_GOP
             //&& process.Action <= ACTION_INIT
               && MParse.FastPlay_Flag <= MAX_WARP
              )
         &&   process.PREV_Pack_File == process.PACK_File
         &&   process.PREV_Pack_Loc  >= 0
            )
         {
             process.Pack_Prev_Size = (int)(process.PACK_Loc
                                          - process.PREV_Pack_Loc);

             if (process.Pack_Prev_Size > 3       // censor silly values
             &&  process.Pack_Min_Size  > process.Pack_Prev_Size)
                 process.Pack_Min_Size  = process.Pack_Prev_Size;

             if (process.Pack_Prev_Size < 25600000  // censor silly values
             &&  process.Pack_Max_Size  < process.Pack_Prev_Size)
                 process.Pack_Max_Size  = process.Pack_Prev_Size;
             
             // Calc average pack size
             process.i64Pack_Sum_Size += process.Pack_Prev_Size;
             process.PACK_Sample_Ctr++;
             process.Pack_Avg_Size = (int)(process.i64Pack_Sum_Size / process.PACK_Sample_Ctr);

         }

         memcpy(&process.CurrSSCRM[0], RdPTR++, sizeof(process.CurrSSCRM));

         if (!memcmp(&process.CurrSSCRM[0], &process.PrevSSCRM[0],sizeof(process.CurrSSCRM))
         &&  process.Action > 0 &&  process.Action < ACTION_NEW_CURRLOC
         && !MParse.Stop_Flag
         &&  MParse.Fault_Flag < CRITICAL_ERROR_LEVEL)
         {
           if (!process.Suspect_SCR_Flag && iCtl_Out_TC_Adjust)
           {
               sprintf(szMsgTxt,"WEIRD SCR. Loc=%d",
                                                (int)(process.PACK_Loc));
               process.Suspect_SCR_Flag = 1;
               if (process.Action == ACTION_RIP
               ||  process.Action == ACTION_INIT)
                   MessageBeep(MB_OK);
           }
         }


         process.Mpeg2_Flag = ( process.CurrSSCRM[0] & 4);  //Get_Bits(2) + 1; //


         if (!process.Mpeg2_Flag) // Mpeg1 ?
         {
             if ( ! Mpeg_Version_Alerted)
                 F595_NotMpeg2_Msg(0);

            RdPTR += 4;  // Short SCR for Mpeg-1
            // RdPTR += 7;  // Mpeg 1 PACK hdr is short

            *((unsigned char*)(&iMuxChunkRate)+3) = 0;
            *((unsigned char*)(&iMuxChunkRate)+2) = *RdPTR++;
            *((unsigned char*)(&iMuxChunkRate)+1) = *RdPTR++;
            *((unsigned char*)(&iMuxChunkRate)  ) = *RdPTR++;
            iMuxChunkRate = iMuxChunkRate & 0x003FFFFF;

         }
         else
         {
            //process.CurrSSCRM[5] = (unsigned char)(Get_Byte() );
            RdPTR += 5;  // Long SCR for Mpeg-2
            //   RdPTR += 8;

            *((unsigned char*)(&iMuxChunkRate)+3) = 0;
            *((unsigned char*)(&iMuxChunkRate)+2) = *RdPTR++;
            *((unsigned char*)(&iMuxChunkRate)+1) = *RdPTR++;
            *((unsigned char*)(&iMuxChunkRate)  ) = *RdPTR++;
            iMuxChunkRate = iMuxChunkRate>>2;
         }

         getbit_VOBCELL_Count = 0;

        //break;
    }

    else
    if (getbit_input_code == PRIVATE_STREAM_2
    && (! iInPS2_Audio
        || (RdPTR[2] != 0x80)              // VOB NAV is not Mpeg-2 flagged
     //   || (*(SHORT*)(RdPTR[0]) == 0xD403) // VOB PS2 1st length=980 IBM-370 format
        ) ) 
    {
        Got_PS2_Pkt(0);
        Packet_Aud_Inc();
        //break;
    }


    else
    if (getbit_input_code == PRIVATE_STREAM_1
     || getbit_input_code == PRIVATE_STREAM_2)
    {

        Got_PrivateStream();

        RdPTR += getbit_iPkt_Len_Remain;
        Packet_Aud_Inc();
    //  break;
    } // ENDIF PS1-PS2


    else
    if (getbit_input_code >= AUDIO_ELEMENTARY_STREAM_0   // MPA = Mpeg Audio
    &&  getbit_input_code <= AUDIO_ELEMENTARY_STREAM_7)
    {
       if (process.iAudioAC3inMPA)  // Allow for crap muxer that puts AC3 inside an MPA packet type
       {
           Got_PrivateStream();
       }
       else
       {
          uGot_Pkt_Type = 'A';
          Got_MPA_Pkt();
       }

       RdPTR += getbit_iPkt_Len_Remain;
       Packet_Aud_Inc();
    //break;
    }  // ENDIF MPEG AUDIO STREAM

    else
    if (getbit_input_code == sPADDING_STREAM_ID)
    {
          PktStats.iPad_Packets++;
          getbit_iPkt_Len_Remain = Get_Short();
    }

    else //      default:
    {
        if (getbit_input_code >= SYSTEM_START_CODE)
        {
           if (getbit_input_code == SYSTEM_START_CODE)
           {
               MParse.SizeCommitted = 0;
           }
           else
           //if (getbit_input_code != sPADDING_STREAM_ID )
           {
              PktStats.iUnk_Packets++;
              getbit_Unk_Stream_Id = getbit_input_code;
           }

           if (MParse.Stop_Flag)
           {
               MParse.Fault_Flag = 97;
               return;
           }

           getbit_iPkt_Len_Remain = Get_Short();
           if (getbit_iPkt_Len_Remain > 6)
           {
             getbit_iPkt_Len_Remain -= 4; // Safety allowance for BUG@6117
             if (getbit_iPkt_Len_Remain > 8192)
                 getbit_iPkt_Len_Remain = 2; // Allow for weird contructs on unknwon packet types
           }

#ifdef DBG_RJ
           if (DBGflag)
           {
               sprintf(szBuffer,"      OTH CODE=%08X Len=%d=x%04X", getbit_input_code, getbit_iPkt_Len_Remain, getbit_iPkt_Len_Remain);
               DBGout(szBuffer);
           }
#endif

           //RdPTR += getbit_iPkt_Len_Remain; // <=== BAD ASSUMPTION - DOES NOT APPLY ON DTV FILES

        }
        else
           RdPTR++;
        //break;
    }

    MPEG_PTR_BUFF_CHK

  } // ENDFOR
}




//-------------------------------------------
void Got_PS2_NAV(int P_BitMode)
{
        PktStats.iPS2_Packets++;

        // Remember Context info
        if (process.PACK_Loc == -1) // Allow for jump into middle of packet
        {
            process.PACK_Loc = Calc_Loc(&process.PACK_File, -4, P_BitMode) ;
#ifdef DBG_RJ
            if (DBGflag) DBGout("\n**** Jump Into Middle of PACK ****\n\n");
#endif
        }

        //if (getbit_iPkt_Len_Remain < 1000)   // VOB NAV packs come in pairs. Remember 1st one.
        {
           process.NAV_Loc  = process.PACK_Loc  ;
           process.NAV_File = process.PACK_File  ;
        }
}


/*


//----------------------------------------------
void Get_Next_Packet_Start()
{

// Fiddle about clearing bit buffers, etc, etc, etc
// to search for an Mpeg start code
//                                       ** CHECK PERFORMANCE **

  InputBuffer_Flush(BitsLeft & 7);
  Get_Next_Packet();
}
*/




//---------------------------------------




