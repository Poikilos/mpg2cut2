
// #define DBG_RJ
 

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/* 
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * According to Wikipedia, the patents covering mpeg-1 video
 * and audio layers 1 & 2  have now expired.
 *
 * Commercial implementations of MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

#include "global.h"
#include "getbit.h"
#include "GetBit_Fast.h"
#include "MPV_PIC.h"
#include "Nav_JUMP.h"

/*
static int iFrame_rate_Table[16] =
{
  0, 23, 24, 25, 29, 30, 50, 59, 60, // Integer part of frame rate
  -1, -1, -1, -1, -1, -1, -1   // reserved
};
static int mFrame_rate_Table[16] =
{
  0, 976, 0, 0, 97, 0, 0, 94, 0, // mantissa part of frame rate
  -1, -1, -1, -1, -1, -1, -1   // reserved
};
*/

const unsigned char HIGH_VALUES[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} ;


static void sequence_extension(void);
static void sequence_display_extension(void);
static void quant_matrix_extension(void);
static void picture_display_extension(void);
static void picture_coding_extension(void);
static void copyright_extension(void);
static int  extra_bit_information(void);
static void extension_and_user_data(void);
static void Auto_Deint_Calc();

//__int64 Calc_Loc(int *Calc_File, int Calc_Offset, int);
void OrgTC_SET();
//void RelativeTC_SET();



//----------------------------------------------------------------------
// decode headers from one input stream 
int GetHdr_PIC(int P_Want_Type)
{
  int iSkip_Ctr, iSince_Ctr, iCurrFile;
    __int64 i64CurrLoc;

  iSkip_Ctr = 0; iSince_Ctr = 0;

  while (  MParse.Fault_Flag < CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
      && ! MParse.Stop_Flag ) 
  {
    
    // PLAN: Here be a good place to check for a jump request during playing
    //       one day....


    // Moved end of file/run test up to here for more orderly finish
    i64CurrLoc = Calc_Loc(&iCurrFile, -4, 0) ; 

#ifdef DBG_RJ
       if (DBGflag)
       {
          DBGln4("\nGetHdr_Pic - Curr=%d.%08X  End=%d.%08X", //  File#%d Last=%d", 
                                    iCurrFile, i64CurrLoc,
                                   process.endFile, process.endLoc);
       }
#endif

    // Preview multi-clips
    if (process.Action == ACTION_RIP && iPreview_Clip_Ctr < iEDL_ctr
    && MParse.Summary_Adjust < 0

    && ((MParse.NextLoc >= process.endLoc 
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

          Mpeg_READ(); 
  
    }

    if (iCurrFile == process.endFile && i64CurrLoc >= process.endLoc  // startLoc      D2V END
    ||  iCurrFile  > process.endFile) 
    {

      if (process.Action == ACTION_RIP && iPreview_Clip_Ctr < iEDL_ctr)
      {
         C160_Clip_Preview();
         File_Ctr = process.startFile;
         _lseeki64(FileDCB[File_Ctr], process.startLoc, SEEK_SET );
         MParse.NextLoc = process.startLoc;
         RdEndPkt = RdPTR = RdEOB;
         RdEndPkt_4 = RdEndPkt-4;  RdEndPkt_8 = RdEndPkt-8;
         BitsLeft = 0;
      }
      else
      if (process.Action > 0 || PlayCtl.iEOFhit > 3)
      {
        //  process.CurrLoc  = process.startLoc ;
        //  process.CurrFile = process.startFile ;
        MParse.Fault_Flag = 97;
        Write_Frame(NULL, d2v_curr, 0);
        T100_Upd_Posn_Info(1);
        return 1;                            // *** ESCAPE POINT ***
      }
      else
        return 0;
    }


    /* look for next start code */
    GetB_Show_Next_Start_Code(1);

    switch (Get_Bits(32))
    {
      case xSEQ_HDR_CODE:
        gothdr_SEQ();
        iSkip_Ctr++;
        break;  

      case GROUP_START_CODE:
        gothdr_GOP();
        iSkip_Ctr++;
        break; 


      case PICTURE_START_CODE:
        gothdr_PICTURE();
        if (! (MParse.iVOB_Style   // VOB sensitivity disabled
               &&  iIn_VOB)        //     and is named as a VOB 
        ||  process.NAV_Loc >= 0)  // VOB locked to NAV pack OK
        {
          if (MPEG_Pic_Type <= P_Want_Type
          || (process.iLongGOP  &&  process.Action == ACTION_NEW_CURRLOC))
              return 1;                            // *** ESCAPE POINT ***

          /*
          if (iSkip_Ctr > 300) // Frame_Number
          {
            if (MPEG_Pic_Type == P_TYPE) 
            //   &&  PlayCtl.iPlayed_Frames < 2)
            {
              process.iLongGOP = 1;  // Allow peeking into very long GOP - KDVD
              return 1; 
            }// *** ESCAPE POINT ***
          }
          */
        }
         
        if ((iSkip_Ctr - iSince_Ctr) > 66)   //  && MParse.iVOB_Style)
        {
            if (MParse.iVOB_Style)
            {
              strcpy(szMsgTxt, "No VOB NAV PACK...");
              MParse.iVOB_Style = 0;
              if (process.Action  ==  ACTION_INIT)
              {
                  _lseeki64(FileDCB[process.startFile],  
                                    0, // process.startLoc,  
                                    SEEK_SET);
                  MParse.NextLoc = 0;
                  File_Ctr = iCurrFile = process.startFile;
                  getBLOCK_Packet(1);
                  iSkip_Ctr = 0;
              }

            }
            else
              strcpy(szMsgTxt, "Long GOP...");

            DSP1_Main_MSG(0,0);
            iSince_Ctr = iSkip_Ctr;
        }

        iSkip_Ctr++;

    } // endswitch GetBits

  } //endWHILE no probs

  return 0;  // *** ESCAPE POINT ***
}




//-----------------------------------------------------------------
/* decode sequence header */
void gothdr_SEQ()
{
  int MPEG_Seq_constrained_params_flag;
//  int MPEG_Seq_NomBitRate400;
  int MPEG_Seq_vbv_buffer_size;
  int i, iNext, iRate, iThisFile, iOffset;
  __int64 i64ThisLoc, i64TMP;


  // Remember Context info 

  if (process.VIDPKT_Loc == -1)
  {
     if (MParse.SystemStream_Flag)
        iOffset = -8;
     else
        iOffset = -4;

    i64ThisLoc = Calc_Loc(&iThisFile, iOffset, 0) ;
  }
  else
  {
       i64ThisLoc  = process.VIDPKT_Loc;
         iThisFile = process.VIDPKT_File;
  }

  PlayCtl.uPendingSeq[1] = 0; 
  PlayCtl.uPendingSeq[2] = 0; 
  PlayCtl.uPendingSeq[3] = 0; 

  if (process.PACK_Loc != -1) // Allow for jump into middle of packet
  {
    // Don't split pack, unless it's a ridiculously long pack
    if (   iThisFile != process.PACK_File
    || ((i64ThisLoc   - process.PACK_Loc) < 1024000))
    {
       i64ThisLoc  = process.PACK_Loc;
         iThisFile = process.PACK_File;
    }

    if (Mpeg_PES_Version == 2
    &&  MParse.SystemStream_Flag > 0)  // Program Stream
    {

       memcpy(&process.ViewSSCRM[0], &process.CurrSSCRM[0], sizeof(process.ViewSSCRM));

       lpMpeg_TC_ix2 = &process.ViewSSCRM[0];
       SCRM_2SCR((unsigned char *)(&i64TMP));
       process.uViewSCR = (unsigned int)(i64TMP/2);

      if (! process.Got_PTS_Flag)         // SEQ hdr pkt missing PTS ?
      {
       if (!process.Missing_PTS_Flag)
       {

         process.Missing_PTS_Flag = 1;
         if (iPreview_Clip_Ctr > iEDL_ctr)
         {
            sprintf(szMsgTxt,"MISSING PTS. Loc=x%06X",
                                            (int)(process.PACK_Loc));
           if (process.Action == ACTION_RIP
           ||  process.Action == ACTION_INIT)
           {
               MessageBeep(MB_OK);
               if (process.Action == ACTION_RIP
               &&  iMsgLife <= 0)
                   DSP1_Main_MSG(0,0);
           }
         }
       }
      }
    }
  }

  
  process.SEQ_Loc  = i64ThisLoc;
  process.SEQ_File = iThisFile;

  if (process.ViewPTS == 0xFFFFFFFF)
  {
      process.ViewPTS  = process.VideoPTS;  // pts to MOST RECENT SEQ/GOP/PIC
      process.ViewPTSM = process.VideoPTSM; // pts to MOST RECENT SEQ/GOP/PIC  
  }
  
  // Calculate increment in PTS between GOPs
  //       allowing for unsigned PTS field
  if (process.VideoPTS > process.uVid_PTS_Seq_Prev)
  {
    process.iVid_PTS_Seq_Diff = (process.VideoPTS - process.uVid_PTS_Seq_Prev)
                                / 45;
  }
  else
  {
    process.iVid_PTS_Seq_Diff = (process.uVid_PTS_Seq_Prev - process.VideoPTS)
                                / -45;
  }
     
  process.uVid_PTS_Seq_Prev = process.VideoPTS;
  


  // Get around difficulty parsing some files - eg PowerVCR 
  if (process.preamble_len < 1 &&  ! process.Preamble_Known
        // && iCtl_Out_Preamble_Flag == 3 
     )
  {
    process.preamble_len  = process.SEQ_Loc ; 
    if (DBGflag) DBGln2("\n* PREAMBLE=x%04X (%d) *SEQ*\n", process.preamble_len, process.preamble_len);
    if (MParse.iVOB_Style && process.NAV_Loc < 0 &&  ! process.Preamble_Known)
        Mpeg_PreAmble_Alert(0);

  }

//#ifdef DBG_RJ
              if (DBGflag) 
                  DBGln2("\nSEQ.   SEQLoc=%x   PREAMBLE len=%x\n",  
                           process.SEQ_Loc, process.preamble_len ) ;
//#endif
                

  MPEG_Seq_horizontal_size    = Get_Bits(12);
  MPEG_Seq_vertical_size      = Get_Bits(12);
  MPEG_Seq_aspect_ratio_code  = Get_Bits(4);

  // optional override of frame rate
  MPEG_Seq_frame_rate_code     = Get_Bits(4);

   if (iOverride_FrameRate_Code)
       iView_FrameRate_Code = iOverride_FrameRate_Code;
   else
       iView_FrameRate_Code = MPEG_Seq_frame_rate_code;


  // Interrupt CUE mode on change of particulars
  if (iAspect_Curr_Code         != MPEG_Seq_aspect_ratio_code
  ||  iFrame_Rate_Code          != MPEG_Seq_frame_rate_code)   // TIVO may change frame rate at ad breaks
  {
      iAspect_Curr_Code  = MPEG_Seq_aspect_ratio_code;
      iFrame_Rate_Code   = MPEG_Seq_frame_rate_code; 
      if (MParse.FastPlay_Flag > MAX_WARP
      &&  process.Action == ACTION_RIP) // Some UK ads - Different Aspect ratio (well sometimes)
          PlayCtl.iStopNextFrame++;     // Stop after next I_Frame

  }
  
  MPEG_Seq_NomBitRate400      = Get_Bits(18); // units of 400 bits/sec

  InputBuffer_Flush(1);  // marker bit

  MPEG_Seq_vbv_buffer_size         = Get_Bits(10);
  MPEG_Seq_constrained_params_flag = Get_Bits(1);

  MPEG_Seq_load_intra_quantizer_matrix = Get_Bits(1) ;  //RJ
  if (MPEG_Seq_load_intra_quantizer_matrix)
  {
    for (i=0; i<64; i++)
      MPEG_Seq_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(8);
  }
  else
  {
    for (i=0; i<64; i++)
      MPEG_Seq_intra_quantizer_matrix[i] = default_intra_quantizer_matrix[i];
  }


  MPEG_Seq_load_non_intra_quantizer_matrix = Get_Bits(1) ; 
  if (MPEG_Seq_load_non_intra_quantizer_matrix)
  {
    for (i=0; i<64; i++)
      MPEG_Seq_non_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(8);
  }
  else
  {
    for (i=0; i<64; i++)
      MPEG_Seq_non_intra_quantizer_matrix[i] = 16;  // default non-intra
  }


  /* copy luminance to chrominance matrices */
  for (i=0; i<64; i++)
  {
    chroma_intra_quantizer_matrix[i]     = MPEG_Seq_intra_quantizer_matrix[i];
    chroma_non_intra_quantizer_matrix[i] = MPEG_Seq_non_intra_quantizer_matrix[i];
  }

  MPEG_ProfLvl_Escape = ' ';
  MPEG_Profile = 0;
  MPEG_Level   = 0;


  Mpeg_SEQ_Version = 1; // Default to Mpeg-1 if no Extension follows.
  MPEG_iFrame_rate_extension_n = 0;
  MPEG_iFrame_rate_extension_d = 0;
  fFrame_rate_extension_n = 1.0; 
  fFrame_rate_extension_d = 1.0; 

  extension_and_user_data();
  FrameRate2FramePeriod();

  Auto_Deint_Calc();

  // Convert bitrate from Bytes per Picture to Bps and kbps
  // use S.I. "k" = decimal 1000  NOT traditional computer 1024

  iNom_ByteRate  = (int)(MPEG_Seq_NomBitRate400 * 50); // Note 50=400/8
  iNom_kBitRate  = iNom_ByteRate / 125 ;  // Note 125=1000/8

  // need to calculate ByteRate ?
  if (process.ByteRateAvg[File_Ctr] == BYTERATE_DEF
   || process.Prev_Seq_BitRate != MPEG_Seq_NomBitRate400)
  {
      process.Prev_Seq_BitRate  = MPEG_Seq_NomBitRate400;

    // Nominal bitrates are not always reliable
    // not as good as  measured average (not calculated yet)

    process.kBitRateAvg = iNom_kBitRate;

    // sometimes the nominal is total bulldust
    // Let's trap suspicious bitrates
    // DTV header maybe stream rate, not channel rate
    if (iNom_kBitRate  >= 9000  // Higher than most DVDs 
    ||  iMuxChunkRate  <  MPEG_Seq_NomBitRate400) // Ch.7 via Nebula
    {
      if (Coded_Pic_Height <= 576) // && ! MPEG_Seq_progressive_sequence) 
      {
         if (iNom_kBitRate  >  50000)  // Weird files
             process.kBitRateAvg = iNom_kBitRate / 12;
         else
         if (iNom_kBitRate  >= 13000)  // Ch.7
             process.kBitRateAvg = iNom_kBitRate * 2 / 7;
         else
             process.kBitRateAvg = iNom_kBitRate * 5 / 7;
      }
      else
      {
         if (iNom_kBitRate  >  50000)  // Weird files
             process.kBitRateAvg = iNom_kBitRate / 8;
         else
         if (iNom_kBitRate  >= 13000)  // Ch.7
             process.kBitRateAvg = iNom_kBitRate * 2 / 7;
         else
             process.kBitRateAvg = iNom_kBitRate * 5 / 7;
      }
    }  // END-IF Bulldust Nominal
    

    iRate = (int) (process.kBitRateAvg * 1000 / 8);
    if (iRate < MPEG_SEARCH_BUFSZ)
        iRate = MPEG_SEARCH_BUFSZ;

    process.ByteRateAvg[File_Ctr] = iRate;
    // Propogate info to appended file
    iNext = File_Ctr + 1;
    if (process.ByteRateAvg[iNext] == BYTERATE_DEF)
        process.ByteRateAvg[iNext] =  iRate;
  } // END-IF Time to calculate ByteRate



  // Fill in the stats panel
  if (MParse.ShowStats_Flag)
  {
    S120_Stats_Hdr_Seq();
  }

  Decoder_INIT();

  if (! MParse.SeqHdr_Found_Flag)
  {
     MParse.SeqHdr_Found_Flag = 1;
     if (iEDL_ctr)
        DSP2_Main_SEL_INFO(1);
  }

}



//----------------------------------------------------------------------
/* decode group of pictures header */
/* ISO/IEC 13818-2 section 6.2.2.6 */

/*static*/ void gothdr_GOP()
{

  int iThisFile, iOffset;
  __int64 i64ThisLoc, i64TMP;

  process.iGOP_Ctr++;   PlayCtl.iGOP_Ctr++;

  MParse.iGOPsSinceNAV++;
  if (MParse.iGOPsSinceNAV > 3
  // &&  ! iIn_VOB        //     and is NOT named as a VOB 
  &&  MParse.iVOB_Style)  // VOB sensitivity enabled
        
  {
        MParse.iVOB_Style = 0;  // disable VOB sensitivity 
        strcpy(szMsgTxt, "Not a true DVD VOB");
        DSP1_Main_MSG(0,0);
  }

  // Remember Context info 

  if (process.VIDPKT_Loc == -1)
  {
     if (MParse.SystemStream_Flag)
        iOffset = -8;
     else
        iOffset = -4;

      i64ThisLoc = Calc_Loc(&iThisFile, iOffset, 0) ;
  }
  else
  {
       i64ThisLoc  = process.VIDPKT_Loc;
       iThisFile   = process.VIDPKT_File;
  }

  if (process.PACK_Loc != -1)    // Allow for jump into middle of packet
  {
      // Don't split pack, unless it's a ridiculously long pack
      if ( iThisFile != process.PACK_File
      || ((i64ThisLoc - process.PACK_Loc) < 1024000))
      {
         i64ThisLoc = process.PACK_Loc;
         iThisFile  = process.PACK_File;
      }

  }

  process.GOP_Loc  = i64ThisLoc ;
  process.GOP_File = iThisFile;


  if (process.ViewPTS == 0xFFFFFFFF)
  {
      process.ViewPTS  = process.VideoPTS;  // pts to MOST RECENT SEQ/GOP/PIC
      process.ViewPTSM = process.VideoPTSM; // pts to MOST RECENT SEQ/GOP/PIC  
  }

  if (memcmp(&process.ViewSSCRM[0], &HIGH_VALUES[0], sizeof(process.ViewSSCRM)))
  {
      memcpy(&process.ViewSSCRM[0], &process.CurrSSCRM[0], sizeof(process.ViewSSCRM));
      lpMpeg_TC_ix2 = &process.ViewSSCRM[0];
      SCRM_2SCR((unsigned char *)(&i64TMP));
      process.uViewSCR = (unsigned int)(i64TMP/2);

  }

  gop_drop_flag   = Get_Bits(1);

  gopTC.hour      = Get_Bits(5);  // GOP Time Code is hh:mm:ss.frame
  gopTC.minute    = Get_Bits(6);

  InputBuffer_Flush(1);  // marker bit

  gopTC.sec      = Get_Bits(6);
  gopTC.frameNum = Get_Bits(6);


  gop_closed_flag      = Get_Bits(1);
  gop_broken_link_flag = Get_Bits(1);

  extension_and_user_data();

  gopTC.RunFrameNum  = (((gopTC.hour * 3600 + gopTC.minute)  * 60)
                        + gopTC.sec) * 24   + gopTC.frameNum ;

  gopTC.VideoPTS          = process.VideoPTS; 
  gopTC.VideoDTS          = process.VideoDTS; 
  gopTC.AudioPTS          = process.AudioPTS; 

  process.Delay_Calc_Flag = 1;


  if ( ! gopTC.RunFrameNum  &&  process.VideoPTS)
  {
      PTS_2Field( process.VideoPTS, IDC_VID_PTS);
      //memcpy(&gopTC, &ptsTC, sizeof(ptsTC));
  }

  memcpy(&CurrTC, &gopTC, sizeof(gopTC));
  if (OrgTC.RunFrameNum  < 0
   && gopTC.RunFrameNum >= 0)
      OrgTC_SET();

}

 
//-----------------------------------------
/* decode picture header */
/* ISO/IEC 13818-2 section 6.2.3 */

/*static */void gothdr_PICTURE()
{
  int vbv_delay;
  int full_pel_forward_vector;
  int forward_f_code;
  int full_pel_backward_vector;
  int backward_f_code;
  int Extra_Information_Byte_Count;

  // int iThisFile;
  __int64 i64TMP; // i64ThisLoc, 


  iPICtime = iCURR_TIME_ms();

  process.PIC_Loc = Calc_Loc(&process.PIC_File, -4, 0) ;

  process.iAudio_InterFrames++;
  process.iVid_PTS_Frames++;
  Pic_Started = 1;

  MPEG_Pic_Temporal_Ref = Get_Bits(10);

  PREV_Pic_Type         = MPEG_Pic_Type;
  MPEG_Pic_Type         = Get_Bits(3);   // I-P-B

  if (Mpeg_SEQ_Version != 2)
  {
    if (MPEG_Pic_Type == 4)  // D = dc intra-coded ISO/IEC11172-2
        MPEG_Pic_Type = I_TYPE; // Pretend
    if (!MPEG_Pic_Structure)
         MPEG_Pic_Structure  = FULL_FRAME_PIC; // Allow for missing PIC CODING  EXTENSION hdr
  }

  d2v_curr.type = MPEG_Pic_Type;

  CurrTC.VideoPTS  = process.VideoPTS;
  CurrTC.VideoDTS  = process.VideoDTS;
  CurrTC.AudioPTS  = process.AudioPTS;

  // Manually increment the Frame Number inside a GOP
  if (MPEG_Pic_Type == I_TYPE)
  {

     if (process.ViewPTS == 0xFFFFFFFF)
     {
         process.ViewPTS  = process.VideoPTS;  // pts to MOST RECENT SEQ/GOP/PIC
         process.ViewPTSM = process.VideoPTSM; // pts to MOST RECENT SEQ/GOP/PIC  
     }

     if (process.Action  ==  ACTION_RIP && iCTL_FastBack)
         Nav_Jump_Fwd(&BwdGop);

     if ( ! iGOPrelative)
     {
          process.uGOPbefore = MPEG_Pic_Temporal_Ref * iFrame_Period_ms * 45;

          process.uGOP_TCorrection = MPEG_Pic_Temporal_Ref;
          process.uGOP_FPeriod_ps  = iFrame_Period_ps;

     }

     if ( ! gopTC.RunFrameNum &&  process.VideoPTS)
     {
         PTS_2Field( process.VideoPTS, IDC_VID_PTS);
         //memcpy(&gopTC,  &ptsTC, sizeof(ptsTC));
         memcpy(&CurrTC, &ptsTC, sizeof(ptsTC));
     }

     //process.Delay_Calc_Flag = 1;

     if (memcmp(&process.ViewSSCRM[0], &HIGH_VALUES[0], sizeof(process.ViewSSCRM) ) ) 
     {
         memcpy(&process.ViewSSCRM[0], &process.CurrSSCRM[0], sizeof(process.ViewSSCRM));
         lpMpeg_TC_ix2 = &process.ViewSSCRM[0];
         SCRM_2SCR((unsigned char *)(&i64TMP));
         process.uViewSCR = (unsigned int)(i64TMP/2);

     }

     //if (process.Action == ACTION_RIP)
     {
        if (iGOPrelative)
        {
           if (iGOPrelative < 12 && process.iGOP_Ctr < 2) 
               iGOPrelative = 12;
           iGOPperiod = (iGOPrelative /*+1*/ ) * iFrame_Period_ms; //+1=generosity
        }
        iGOPtot = iGOPrelative;
        iGOPrelative = 0;
        iGOPtime = iPICtime;
     }
  }
  else
  {
      CurrTC.RunFrameNum++;
      CurrTC.VideoPTS = process.VideoPTS;
      CurrTC.VideoDTS = process.VideoDTS;
      CurrTC.AudioPTS = process.AudioPTS;
      if (CurrTC.frameNum < 25)
          CurrTC.frameNum++;
      else
      {
        CurrTC.frameNum = 0;
        if (CurrTC.sec < 59)
            CurrTC.sec++;
        else
        {
            CurrTC.sec = 0;
            if (CurrTC.minute < 59)
                CurrTC.minute++;
            else
            {
               CurrTC.minute = 0;
               CurrTC.hour++;
            }
        }
     }
  }

  iGOPrelative++;
     
  // GetDelay();      // OLD Delay Calc point

//  if (process.PACK_Loc == -1)    // Allow for jump into middle of packet
//  {
//      process.PACK_Loc  = process.PIC_Loc;
//      process.PACK_File = process.PIC_File;
//      if (DBGflag) DBGout("\n**** Jump Into Middle of PACK ****\n");
//  }

  
  //if (MPEG_Pic_Type == I_TYPE)
  //    process.KEY_Loc  = process.PACK_Loc ;
  //    process.KEY_File = process.PACK_File;

  if ( process.preamble_len < 1  &&  ! process.Preamble_Known
          //  || (MParse.iVOB_Style && process.NAV_Loc < 0) 
     )
  {
     Mpeg_PreAmble_Alert(MPEG_Pic_Type);
  }


  if (MPEG_Pic_Type == I_TYPE)
  {
      process.KEY_Loc  = process.PIC_Loc;
      process.KEY_File = process.PIC_File;
  }

  // Remember Context info

#ifdef DBG_RJ
  if (DBGflag)
      DBGln4("**** PIC Type=%d GOPLOC=%x, SEQLOC=%x, PACKLOC=%x\n",
                   MPEG_Pic_Type, 
                   process.GOP_Loc, process.SEQ_Loc, process.PACK_Loc);
#endif

  // RJ  End of file/run test was temorarily here (Ver 2.1)

  PlayCtl.iUnReportedFrames++;

  if (MPEG_Pic_Type == I_TYPE                //d2v_curr.type == I_TYPE
      || MParse.Fault_Flag == 97             // RJ update pointers on EOF
      || process.Action == ACTION_FWD_FRAME  // != ACTION_RIP
      || PlayCtl.iUnReportedFrames > 30)
  {
     T100_Upd_Posn_Info( (process.Action == ACTION_RIP 
                          || MParse.Fault_Flag == 97 ));  // RJ update pointers on EOF  

     // RJ  End of file/run test was originally here (Ver 1)

     DSP3_Main_TIME_INFO();

     if (MParse.ShowStats_Flag  && MParse.Fault_Flag != 97
     //&& (MPEG_Pic_Type == I_TYPE || process.Action != ACTION_RIP)
     )
     {
         S100_Stats_Hdr_Main(0);
         S200_Stats_Pic_Main(0);
         if (process.Action == ACTION_RIP)
             Stats_FPS();
     }

     PlayCtl.iUnReportedFrames = 0;

  } // END I-TYPE


  vbv_delay = Get_Bits(16);

  if (MPEG_Pic_Type == P_TYPE || MPEG_Pic_Type==B_TYPE)
  {
    full_pel_forward_vector = Get_Bits(1);
    forward_f_code = Get_Bits(3);
    //if (iPES_Mpeg_Any)
    //{
       MPEG_Pic_f_code[0][0] = forward_f_code;
       MPEG_Pic_f_code[0][1] = forward_f_code;
    //}
  }

  if (MPEG_Pic_Type == B_TYPE)
  {
    full_pel_backward_vector = Get_Bits(1);
    backward_f_code = Get_Bits(3);
    //if (iPES_Mpeg_Any)
    //{
        MPEG_Pic_f_code[1][0] = backward_f_code;
        MPEG_Pic_f_code[1][1] = backward_f_code;
    //}
  }
 
  Extra_Information_Byte_Count = extra_bit_information();
  extension_and_user_data();
}



//-------------------------------------------
/* decode slice header */
/* ISO/IEC 13818-2 section 6.2.4 */
int slice_header()
{
  int slice_vertical_position_extension;
  int quantizer_scale_code;
  int slice_picture_id_enable = 0;
  int slice_picture_id = 0;
  int extra_information_slice = 0;

  slice_vertical_position_extension = MPEG_Seq_vertical_size>2800 ? Get_Bits(3) : 0;

  quantizer_scale_code = Get_Bits(5);
  quantizer_scale = MPEG_Pic_q_scale_type ? Non_Linear_quantizer_scale[quantizer_scale_code] : quantizer_scale_code<<1;

  /* slice_id introduced in March 1995 as part of the video corridendum
     (after the IS was drafted in November 1994) */
  if (Get_Bits(1))
  {
      Get_Bits(1);  // intra slice

      slice_picture_id_enable = Get_Bits(1);
      slice_picture_id = Get_Bits(6);

      extra_information_slice = extra_bit_information();
  }

  return slice_vertical_position_extension;
}


//---------------------------------------
/* decode extension and user data */
/* ISO/IEC 13818-2 section 6.2.2.2 */
static void extension_and_user_data()
{
  int code, ext_ID;

  GetB_Show_Next_Start_Code(0);

  //          WARNING - ASSIGNMENT INSIDE CONDITION ...
  while ( ((  code =  Show_Bits(32))  == EXTENSION_START_CODE 
           || code                    == USER_DATA_START_CODE)
   &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
   && ! MParse.Stop_Flag ) 
  {
    if (code == EXTENSION_START_CODE) 
    {
      InputBuffer_Flush(32);
      ext_ID = Get_Bits(4);

      switch (ext_ID)
      {
        case SEQUENCE_EXTENSION_ID:
          sequence_extension();
          break;
        case SEQUENCE_DISPLAY_EXTENSION_ID:
          sequence_display_extension();
          break;
        case QUANT_MATRIX_EXTENSION_ID:
          quant_matrix_extension();
          break;
        case PICTURE_DISPLAY_EXTENSION_ID:
          picture_display_extension();
          break;
        case PICTURE_CODING_EXTENSION_ID:
          picture_coding_extension();
          break;
        case COPYRIGHT_EXTENSION_ID:
          copyright_extension();
          break;
      }
      GetB_Show_Next_Start_Code(0);
    }

    else
    {
      InputBuffer_Flush(32); // ISO/IEC 13818-2  sections 6.3.4.1 and 6.2.2.2.2
      GetB_Show_Next_Start_Code(0);  // skip user data
    }
  }
}



/* decode sequence extension */
/* ISO/IEC 13818-2 section 6.2.2.3 */
static void sequence_extension()
{
  //int MPEG_Seq_profile_level_flags;
  int Escape_flag;
  int low_delay;

  int horizontal_size_extension;
  int vertical_size_extension;
  int bit_rate_extension;
  int vbv_buffer_size_extension;

  Mpeg_SEQ_Version = 2;

  if (MParse.SystemStream_Flag < 1)
  {
     Mpeg_PES_Version = 2;
     process.Mpeg2_Flag = 4;
  }

  Escape_flag = Get_Bits(1);
  if (Escape_flag)
    MPEG_ProfLvl_Escape = '#';
  else
    MPEG_ProfLvl_Escape = '@';


  MPEG_Profile                  = Get_Bits(3);
  MPEG_Level                    = Get_Bits(4);

  MPEG_Seq_progressive_sequence    = Get_Bits(1);
  MPEG_Seq_chroma_format           = Get_Bits(2);

  horizontal_size_extension    = Get_Bits(2);
  vertical_size_extension      = Get_Bits(2);
  bit_rate_extension           = Get_Bits(12);
  InputBuffer_Flush(1);  // marker bit
  vbv_buffer_size_extension    = Get_Bits(8);
  low_delay                    = Get_Bits(1);

  MPEG_iFrame_rate_extension_n     = Get_Bits(2);
  MPEG_iFrame_rate_extension_d     = Get_Bits(5);
  fFrame_rate_extension_n = (MPEG_iFrame_rate_extension_n + 1);
  fFrame_rate_extension_d = (MPEG_iFrame_rate_extension_d + 1);

  FrameRate2FramePeriod();

  MPEG_Seq_horizontal_size = (horizontal_size_extension<<12)
                             | (MPEG_Seq_horizontal_size & 0x0fff);
  MPEG_Seq_vertical_size = (vertical_size_extension<<12) 
                             | (MPEG_Seq_vertical_size & 0x0fff);

  Auto_Deint_Calc();

}



//-------------------------------------
/* decode sequence display extension */
static void sequence_display_extension()
{
  int MPEG_Seq_video_format;
  int MPEG_Seq_color_description;

  int color_primaries;
  int transfer_characteristics;
  int matrix_coefficients;
  int display_horizontal_size;
  int display_vertical_size;

  Mpeg_SEQ_Version = 2;

  if (MParse.SystemStream_Flag < 1)
  {
     Mpeg_PES_Version = 2;
     process.Mpeg2_Flag = 4;
  }


  MPEG_Seq_video_format      = Get_Bits(3);
  MPEG_Seq_color_description = Get_Bits(1);

  if (MPEG_Seq_color_description) // Color space controls
  {
     color_primaries          = Get_Bits(8);
     transfer_characteristics = Get_Bits(8);
     matrix_coefficients      = Get_Bits(8);
  }

  // Pan-Scan window size
  display_horizontal_size = Get_Bits(14);
  InputBuffer_Flush(1);  // marker bit
  display_vertical_size   = Get_Bits(14);
}




//--------------------------------------
/* decode quant matrix entension */
/* ISO/IEC 13818-2 section 6.2.3.2 */
static void quant_matrix_extension()
{
  int i;

  MPEG_Seq_load_intra_quantizer_matrix = Get_Bits(1) ;
  if (MPEG_Seq_load_intra_quantizer_matrix)
    for (i=0; i<64; i++)
       chroma_intra_quantizer_matrix     [scan[ZIG_ZAG][i]]
        = MPEG_Seq_intra_quantizer_matrix[scan[ZIG_ZAG][i]] 
        = Get_Bits(8);


  MPEG_Seq_load_non_intra_quantizer_matrix = Get_Bits(1) ;
  if (MPEG_Seq_load_non_intra_quantizer_matrix)
    for (i=0; i<64; i++)
      chroma_non_intra_quantizer_matrix      [scan[ZIG_ZAG][i]]
        = MPEG_Seq_non_intra_quantizer_matrix[scan[ZIG_ZAG][i]]
        = Get_Bits(8);

  MPEG_Seq_load_chroma_intra_quantizer_matrix = Get_Bits(1) ;
  if (MPEG_Seq_load_chroma_intra_quantizer_matrix)
    for (i=0; i<64; i++)
      MPEG_Seq_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(8);

      MPEG_Seq_load_chroma_non_intra_quantizer_matrix = Get_Bits(1) ;

  if (MPEG_Seq_load_chroma_non_intra_quantizer_matrix)
    for (i=0; i<64; i++)
      chroma_non_intra_quantizer_matrix[scan[ZIG_ZAG][i]] = Get_Bits(8);
}



//---------------------------------------
/* decode picture display extension */
/* ISO/IEC 13818-2 section 6.2.3.3. */
static void picture_display_extension()
{
  int frame_center_horizontal_offset[3];
  int frame_center_vertical_offset[3];

  int i;
  int number_of_frame_center_offsets;
  
  Mpeg_SEQ_Version = 2;

  if (MParse.SystemStream_Flag < 1)
  {
     Mpeg_PES_Version = 2;
     process.Mpeg2_Flag = 4;
  }



  /* based on ISO/IEC 13818-2 section 6.3.12
     (November 1994) Picture display extensions */

  /* derive number_of_frame_center_offsets */

  if (MPEG_Seq_progressive_sequence)
  {
    if (MPEG_Pic_repeat_first_field)
    {
      if (MPEG_Pic_top_field_first)
        number_of_frame_center_offsets = 3;
      else
        number_of_frame_center_offsets = 2;
    }
    else
      number_of_frame_center_offsets = 1;
  }
  else
  {
    if (MPEG_Pic_Structure != FULL_FRAME_PIC)
        number_of_frame_center_offsets = 1;
    else
    {
      if (MPEG_Pic_repeat_first_field)
        number_of_frame_center_offsets = 3;
      else
        number_of_frame_center_offsets = 2;
    }
  }

  /* now parse */
  for (i=0; i<number_of_frame_center_offsets; i++)
  {
    frame_center_horizontal_offset[i] = Get_Bits(16);
    InputBuffer_Flush(1);  // marker bit

    frame_center_vertical_offset[i] = Get_Bits(16);
    InputBuffer_Flush(1);  // marker bit
  }
}


//---------------------------------------------------------------
/* decode picture coding extension */
static void picture_coding_extension()
{

  int v_axis;
  int field_sequence;
  int sub_carrier;
  int burst_amplitude;
  int sub_carrier_phase;

  Mpeg_SEQ_Version = 2;

  if (! MParse.SystemStream_Flag)
      Mpeg_PES_Version = 2;

  MPEG_Pic_f_code[0][0] = Get_Bits(4); // forward horizontal
  MPEG_Pic_f_code[0][1] = Get_Bits(4); // forward vertical 
  MPEG_Pic_f_code[1][0] = Get_Bits(4); // backward horizontal 
  MPEG_Pic_f_code[1][1] = Get_Bits(4); // backward vertical 

  MPEG_Pic_intra_dc_precision   = Get_Bits(2);
  MPEG_Pic_Structure            = Get_Bits(2);  // Which field first

  if (! MPEG_Pic_Structure) // Allow for bad setting
        MPEG_Pic_Structure  = FULL_FRAME_PIC;

  MPEG_Pic_top_field_first        = Get_Bits(1);
  MPEG_Pic_pred_frame_dct         = Get_Bits(1);
  MPEG_Pic_concealment_motion_vectors = Get_Bits(1);
  MPEG_Pic_q_scale_type           = Get_Bits(1);
  MPEG_Pic_intra_vlc_format       = Get_Bits(1);
  MPEG_Pic_alternate_scan         = Get_Bits(1);
  MPEG_Pic_repeat_first_field     = Get_Bits(1);
  MPEG_Pic_chroma_420_type        = Get_Bits(1);
  MPEG_Pic_Origin_progressive     = Get_Bits(1); // Progressive_Frame
  MPEG_Pic_composite_display_flag = Get_Bits(1);

  d2v_curr.Progressive_Format  =  MPEG_Pic_Origin_progressive;
  d2v_curr.Fld1_Top_Rpt = (MPEG_Pic_top_field_first<<1) + MPEG_Pic_repeat_first_field;

  if (MPEG_Pic_composite_display_flag)
  {
    v_axis            = Get_Bits(1);
    field_sequence    = Get_Bits(3);
    sub_carrier       = Get_Bits(1);
    burst_amplitude   = Get_Bits(7);
    sub_carrier_phase = Get_Bits(8);
  }

  Auto_Deint_Calc();

}


//------------------------------------
/* decode extra bit information */
/* ISO/IEC 13818-2 section 6.2.3.4. */
static int extra_bit_information()
{
  int Byte_Count = 0;

  while ( Get_Bits(1) &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
                      && ! MParse.Stop_Flag ) 
  {
    InputBuffer_Flush(8);
    Byte_Count++;
  }

  return Byte_Count;
}


//---------------------------------------------
/* Copyright extension */
/* ISO/IEC 13818-2 section 6.2.3.6. */
/* (header added in November, 1994 to the IS document) */
static void copyright_extension()
{
  int copyright_flag;
  int copyright_identifier;
  int original_or_copy;
  int copyright_number_1;
  int copyright_number_2;
  int copyright_number_3;

  int reserved_data;

  copyright_flag =       Get_Bits(1);
  copyright_identifier = Get_Bits(8);
  original_or_copy =     Get_Bits(1);

  /* reserved */
  reserved_data = Get_Bits(7);

  InputBuffer_Flush(1); // marker bit
  copyright_number_1 =   Get_Bits(20);
  InputBuffer_Flush(1); // marker bit
  copyright_number_2 =   Get_Bits(22);
  InputBuffer_Flush(1); // marker bit
  copyright_number_3 =   Get_Bits(22);
}




//----------------------------------------------------------------------
void Mpeg_PreAmble_Alert(int P_Pic_Type)
{
  //const char Coded_Pic_Abbr[8] = {'0', 'I', 'P', 'B', '4', '5', '6', '7'};
  int iRC, iFile;
  char szMissing[16];
  __int64  i64Loc;


// If Preamble is being recorded at a picture packet
// Then file must have started part way through a GOP
// Alert user so that he knows what he is missing out on.

  if ( ! process.Broken_start_type)
  {
      process.Broken_start_type = Coded_Pic_Abbr[P_Pic_Type];
      iRC = 9999;
      szMissing[0] = 0;
    
      if (MParse.iVOB_Style && process.NAV_Loc < 0)
      {
           strcpy(szMissing, "VOB PS2");
      }
      else
      if (process.SEQ_Loc >= 0 || process.GOP_Loc >= 0)
          iRC = IDOK;
      else       
      if (P_Pic_Type == I_TYPE)
           strcpy(szMissing, "GOP");
      else 
           strcpy(szMissing, "KEY-FRAME") ;


      if (iRC == 9999  && iCtl_WarnBadStart)
      {
          i64Loc = Calc_Loc(&iFile, -4, 0);
          sprintf(szBuffer, 
                  "MISSING FIRST %s header on input file (x%04X)\n\nInput file starts PART WAY THROUGH GOP/SEQ.\n\nSKIP TO BETTER START POINT ?",
                                 szMissing, (int)(i64Loc));
          iRC = //MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - WARNING",
                //              MB_OKCANCEL);
                Warning_Box(&szBuffer[0], 0, &iCtl_WarnBadStart, IDM_WARN_BAD_START, MB_OKCANCEL);
          
      }

#ifdef DBG_RJ
      if (DBGflag)
      {
          char szTmp1[256];
          sprintf(szTmp1, "%s RESP=%d\n\n", szBuffer, iRC);
          iRC = DBGout(szTmp1);
      }
#endif


      if (iRC == IDCANCEL)  // REQUEST TO KEEP BROKEN GOP AT START
      {
        iIn_VOB = MParse.iVOB_Style = 0;  // Temporarily turn off need for NAV PACKs
        process.Keep_Broken_GOP = 1;
        if (process.NAV_Loc > -1)
            process.preamble_len  = process.NAV_Loc ;
        else 
        if (process.SEQ_Loc > -1)
            process.preamble_len  = process.SEQ_Loc ;
        else 
        if (process.GOP_Loc > -1) 
            process.preamble_len  = process.GOP_Loc ;
        else
        if (process.KEY_Loc > -1) 
            process.preamble_len  = process.KEY_Loc ;
        else
        if (process.PACK_Loc > -1) 
            process.preamble_len  = process.PACK_Loc ;
        else
            process.preamble_len  = Calc_Loc(&process.TMP_File, -4, 0) ;
      }
      process.Preamble_Known = 1;

#ifdef DBG_RJ
      if (DBGflag) 
            DBGln2("PREAMBLE ALERT   RunLoc=%x   PREAMBLE len=%x\n",  
                 process.SEQ_Loc, process.preamble_len ) ;
#endif
      
  }
}




//--------------------------------------------------------------------
static void Auto_Deint_Calc()
{
  unsigned int uTmp1;

  if (Deint_Auto_CURR)
  {
    if (  MPEG_Seq_vertical_size <= 288  // half-frames are too hard for me to de-interlace automatically
      ||  MPEG_Seq_progressive_sequence  // Ahhh, if only this had been adopted as the standard for new material !
      || (!Deint_VOB && 
          (    (fFrame_rate == 25.0 && iIn_VOB) // VOBs tend to be Cinema films, which in PAL std coding SHOULD NOT have interlacing artefacts, although in practice there is some crud around.
            ||  fFrame_rate == 16.0 ||  fFrame_rate == 18.0 || fFrame_rate == 24.0  // Cine frame rates (I forget what 8mm & 9.5mm std rates are, just too long ago for my forgettory)
         )))
          Deint_VIEW = 0;
    else  Deint_VIEW = 1;
  }

 if (Deint_VIEW)
     uTmp1 = MF_CHECKED;
 else
     uTmp1 = MF_UNCHECKED;

 CheckMenuItem(hMenu, IDM_DEINT_CURR, uTmp1);


}
 


//---------------------------------------
void OrgTC_SET()
{
  if (iFrame_Rate_int == 0)
      iFrame_Rate_int = 30;

  OrgTC.hour      = CurrTC.hour;
  OrgTC.minute    = CurrTC.minute;
  OrgTC.sec       = CurrTC.sec;
  OrgTC.frameNum  = CurrTC.frameNum;
  OrgTC.RunFrameNum = (((OrgTC.hour * 3600 + OrgTC.minute)  * 60)
                             + OrgTC.sec) * iFrame_Rate_int
                             + OrgTC.frameNum + 1 ;
  OrgTC.VideoPTS  = CurrTC.VideoPTS;
  OrgTC.VideoDTS  = CurrTC.VideoDTS;
  OrgTC.AudioPTS  = CurrTC.AudioPTS;

  //RelativeTC.RunFrameNum  = 0;
  //RelativeTC.hour         = 0;
  //RelativeTC.frameNum     = 0;
  //RelativeTC.minute       = 0;
  //RelativeTC.sec          = 0;
  //RelativeTC.frameNum     = 0;
}


void RelativeTC_SET()
{
  if (iFrame_Rate_int == 0)
      iFrame_Rate_int = 30;

  RelativeTC.VideoPTS     = CurrTC.VideoPTS     - OrgTC.VideoPTS;
  RelativeTC.VideoDTS     = CurrTC.VideoDTS     - OrgTC.VideoDTS;
  RelativeTC.AudioPTS     = CurrTC.AudioPTS     - OrgTC.AudioPTS;

  if (gopTC.RunFrameNum  > 0 // Trap missing GOP time stamp  (Aust Ch.7 SD)
  && (ptsTC.RunFrameNum <= 0 // Trap missing PTS 
      || (iView_TC_Format != 1 && iView_TC_Format != 4)
      )
  )
  {
      RelativeTC.hour         = CurrTC.hour     - OrgTC.hour;
      RelativeTC.minute       = CurrTC.minute   - OrgTC.minute;
      RelativeTC.sec          = CurrTC.sec      - OrgTC.sec;
      RelativeTC.frameNum     = CurrTC.frameNum - OrgTC.frameNum;
      RelativeTC.RunFrameNum  = CurrTC.RunFrameNum;
  }
  else
  {
      PTS_2Field(RelativeTC.VideoPTS, 0);

      memcpy(&RelativeTC, &ptsTC, sizeof(ptsTC));

      //RelativeTC.hour         = ptsTC.hour;
      //RelativeTC.minute       = ptsTC.minute;
      //RelativeTC.sec          = ptsTC.sec;
      //RelativeTC.frameNum     = ptsTC.frameNum;
      //RelativeTC.RunFrameNum  = ptsTC.RunFrameNum;

  }



  if (RelativeTC.frameNum < 0)
  {
      RelativeTC.frameNum = RelativeTC.frameNum + iFrame_Rate_int;
      RelativeTC.sec--;
  }

  if (RelativeTC.sec < 0)
  {
      RelativeTC.sec = RelativeTC.sec + 60;
      RelativeTC.minute--;
  }

  if (RelativeTC.minute < 0)
  {
      RelativeTC.minute = RelativeTC.minute + 60;
      RelativeTC.hour--;
  }

}



#include "Audio.h"

void FrameRate2FramePeriod()
{

  fFrame_rate = frame_rate_Table[iView_FrameRate_Code];

  if (!iOverride_FrameRate_Code
  && (!MPEG_iFrame_rate_extension_n ||
      !MPEG_iFrame_rate_extension_d))
      fFrame_rate = (fFrame_rate 
                    * (fFrame_rate_extension_n))
                    / (fFrame_rate_extension_d);

  if (iAudio_Force44K)
      fFrame_rate = fFrame_rate * 44.1 / 48.0;

  if  (  !  fFrame_rate)
  {
    //fFrame_rate  =  .001f;
     if (Coded_Pic_Height == 576 || Coded_Pic_Height == 288)
     {
         //iFrame_Period_ps = 40000000;
         fFrame_rate = 25.000;
     }
     else
     {
         //iFrame_Period_ps = 33366666;
         fFrame_rate = 29.97002;
     }
  }

  // Copy floating point info into integers for later
  iFrame_Rate_ms   = (int)(fFrame_rate * 1000);
  iFrame_Rate_int      = (iFrame_Rate_ms+500) / 1000;
  iFrame_Rate_dsp      =  iFrame_Rate_ms      / 1000;
  iFrame_Rate_mantissa = (iFrame_Rate_ms - (iFrame_Rate_dsp*1000)) / 10 ;

  fFrame_Rate_Orig  =  (MParse.FO_Flag==FO_FILM)  ?
                          fFrame_rate  *  0.8f  :  fFrame_rate;

  iFrame_Period_ps  =  (int)((1000000000/fFrame_Rate_Orig)  /*-1*/  );

  iFrame_Period_ms  = (iFrame_Period_ps  + 500000)  /  1000000;
  iSleepTrigger_ms  =  iFrame_Period_ms  *  2;
  iDropTrigger_ms   = -iFrame_Period_ms  *  20;
}




//-----------------------------------------------------
// Convert Presentation Time Stamp into a display field

// There is something wrong with PTS calc,
// either here or in the extract section.

// EG  7D8C2300 (Major part, MSBF)
// PowerDVD says 0h 25m 47s
//          NOT  0h 27m 36s

void PTS_2Field(unsigned P_PTS, int P_Field)
{
  int iTot_Seconds, iTot_Minutes;

  // Set default frame timings if not set yet
  if (! iFrame_Period_ps)
  {
     FrameRate2FramePeriod();
  }

  ptsTC.RunFrameNum = P_PTS / 45 * 1000000 / iFrame_Period_ps;

  iTot_Seconds      = P_PTS / 45000 ;     // 90000;
  iTot_Minutes      = iTot_Seconds / 60;

  ptsTC.sec    = iTot_Seconds - (iTot_Minutes * 60);

  ptsTC.hour   = iTot_Minutes / 60;
  ptsTC.minute = iTot_Minutes - (ptsTC.hour * 60);

  ptsTC.frameNum  = (P_PTS - (iTot_Seconds * 45000)) / 45 / iFrame_Period_ms;
                  // ptsTC.RunFrameNum - (iTot_Seconds *  fFrame_rate) ;

  if (P_Field == IDC_VID_PTS)
      ptsTC.VideoPTS    = P_PTS;
  ptsTC.EffectivePTS    = P_PTS;

  if (ptsTC.hour)
       sprintf(szBuffer, "%d:%02d:%02d;%02d", 
              ptsTC.hour, ptsTC.minute, ptsTC.sec, ptsTC.frameNum);
  else
  if (ptsTC.minute)
       sprintf(szBuffer, "%02d:%02d;%02d",
                          ptsTC.minute, ptsTC.sec, ptsTC.frameNum);
  else
       sprintf(szBuffer, "%02d;%02d",   ptsTC.sec, ptsTC.frameNum);

#ifdef DBG_RJ
  if (DBGflag)
  {
      sprintf(szBuffer, "PTS: %02d  x%X",  P_PTS, P_PTS);
      //if (ptsTC.hour >20)
      //            MessageBox(hWnd, szBuffer, "Mpg2Cut2 - BUG !",
      //                                       MB_ICONSTOP | MB_OK);
  }
#endif

 
  if (P_Field)
      SetDlgItemText(hStats, P_Field, szBuffer);
}
   




//----------------------------------------------
int Get_Hdr_Loc(__int64 *P_Loc, int *P_File)
{
  int iRC;

#ifdef DBG_RJ
  if (DBGflag)
  {
    if (MParse.iVOB_Style)
    {
        DBGln4("* HDR NAV=x%08X GOP=x%08X KEY=x%08X PACK=x%08X\n",
                  process.NAV_Loc, process.GOP_Loc,
                  process.KEY_Loc, process.PACK_Loc);
    }
    else
    {
        DBGln4("* HDR SEQ=x%08X GOP=x%08X KEY=x%08X PACK=x%08X\n",
                  process.SEQ_Loc, process.GOP_Loc,
                  process.KEY_Loc, process.PACK_Loc);
    }
  }
#endif


  iRC = 0;
  if (MParse.iVOB_Style && process.NAV_Loc >= 0)
  {
      *P_Loc  = process.NAV_Loc;
      *P_File = process.NAV_File;
      iEDL_Start_Type = -3; // Starts with VOB NAV PACK
  }
  else 
  if (process.SEQ_Loc >= 0)
  {
      *P_Loc  = process.SEQ_Loc;
      *P_File = process.SEQ_File;
      iEDL_Start_Type = -2; // Starts with SEQ HDR
  }
  else 
  if (process.GOP_Loc >= 0)
  {
      *P_Loc  = process.GOP_Loc;
      *P_File = process.GOP_File;
      iEDL_Start_Type = -1;  // Starts with GOP
  }
  else 
  if (process.KEY_Loc >= 0)
  {
      // Fudge to increase chance of finding SEQ/GOP
      if (MParse.iVOB_Style)
          *P_Loc  = process.KEY_Loc - 6144;
      else
          *P_Loc  = process.KEY_Loc - 2048;

      if (*P_Loc < 0)
          *P_Loc = 0;

      *P_File = process.KEY_File;
      iEDL_Start_Type = 0;     // Starts with I-FRAME 
  }
  else 
  if (process.PIC_Loc >= 0)
  {
     *P_Loc  = process.PIC_Loc;
     *P_File = process.PIC_File;
     iEDL_Start_Type = 1;      // Start with B or P Frame
  }
  else 
  if (process.PACK_Loc >= 0)
  {
     *P_Loc  = process.PACK_Loc;
     *P_File = process.PACK_File;
     iEDL_Start_Type = 8;   // Ill-defined state, but we do have a pack
  }
  else
  {
      iRC = -1;
      iEDL_Start_Type = 9; // Starts with totally unknown state
  }

  return iRC;
}


#include "GetBlk.h"

//-------------------------------------------------------------------
// calculate how far into current input file we are 
__int64 Calc_Loc(int *Calc_File, int Calc_Offset, int P_Bit_Mode)
{
  __int64 i64val, i64_Tell1, i64_Tell2, i64ix;
  int     iFile, iTmp1;

  if (File_Ctr < 0 || File_Ctr > MAX_FILE_NUMBER)
  {
      MessageBox ( NULL, "Corrupted File_Ctr", "Mpg2Cut2 - BUG !", 
                 MB_OK | MB_ICONEXCLAMATION 
                       | MB_SETFOREGROUND );

    MParse.Fault_Flag = CRITICAL_ERROR_LEVEL;
    MParse.Stop_Flag = 9;
    File_Ctr = 0;
  }

  if (RdAHD_Flag)
  {
      iFile     =   iRdAHD_TellBefore_File[iRdAHD_CurrIx];
      i64_Tell1 = i64RdAHD_TellBefore_Loc [iRdAHD_CurrIx];
      i64_Tell2 = i64_Tell1;
  }
  else
  {
      i64_Tell1 = i64_Tell2 = _telli64(FileDCB[File_Ctr]); 
      // Because TELL points to NEXT disk byte to be read,
      // we need to calculate backwards to figure out
      // what position current byte of current buffer corresponds to.

      // Sadly, the length of the most recent read is not stored consistently.
      // (Although I am getting towards fixing that inconsistency.)
      // The inconsistency arises AFTER reaching a file boundary
      // so the following "IF..ELSE"  handles the 2 different scenarios.

      if ( i64_Tell1  > MPEG_SEARCH_BUFSZ || File_Ctr == 0)
           i64_Tell1 -= (__int64) (File_ReadLen);
      else i64_Tell1 -= MPEG_SEARCH_BUFSZ;

      iFile = File_Ctr;
  }

  // How far into the buffer are we ?
  i64ix  = (__int64) (RdPTR - RdBFR);

  // There can be an offset up to 8 bytes
  // between RdPtr and latest Data Field delivered.
  // Get_Bits() uses CurrentBfr+NextBfr. BUT Get_Next_Packet() DONT !
  if (P_Bit_Mode)
    i64ix -= (__int64)(((BitsLeft+7)>>3) + 4) ; 

  i64val = i64_Tell1 + i64ix + Calc_Offset;  // Calc_Offset is normally negative, so effectively a subtraction
  if (!MParse.SystemStream_Flag && Calc_Offset == -4)
  {
      i64val -= 8;
      if (i64val < 0)
          i64val = 0;
  }

  // This proc should only be invoked AFTER 1st block of 1st file is read.
  // ANY other situation is a bug.
  if (i64val < 0)
  { 
    if (File_Ctr)
    {
       iFile = File_Ctr - 1;
       i64val += process.length[iFile];
    }
    else
    {
      i64_Tell1 = 0;
      if (File_Ctr)
      {
        iTmp1 = (int)(i64_Tell2) ;
        sprintf(szBuffer,"BUG - FILE POS SLIPPAGE %d\n\nFile# %d   Len=%d\n\nRecovered.", 
                                       iTmp1, File_Ctr, File_ReadLen) ;
        if (DBG_Alert_ctr < 2)
        {
          DBG_Alert_ctr++;
          MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - BUG !",
                                             MB_ICONSTOP | MB_OK);
        }
        if (DBGflag) DBGout(szBuffer);
      }
    }
  }
                  
  
#ifdef DBG_RJ
        if (DBGflag)   
            DBGln4("           CalcLoc: POS=%x   Tell=%x, ix=%x %d", 
                              i64val, i64_Tell1, i64ix, i64ix) ;
#endif

  *Calc_File = iFile;

  return i64val ;
}

