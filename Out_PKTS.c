//
//       MPEG OUTPUT PACKET MODULE
//
// Contains the main routines for creating output MPEG packets 
//
//
// I don't know how to tell "C" that
// this is a separately compiled sub-routine
// so it is still an "include" of an include of Out.c
//

//#include "windows.h"
//#include "global.h"
//#include <commctrl.h>
//#include "out.h"


// #define RJDBG_PTS 1


#include "MPA_HDR.h"
//#include "Audio.h"

// Time Stamp Adjustment is now in separate file    (ADJ_TC)
#include "Out_TC.c"


 int iPTS_Comparison, iAudioTrail, iTargetTailArmed;
 int iVortex_Curr_Ctr, iVortex_Token;


//------------------------------------------------
// Scan the big buffer looking for packets to zap
//
// Target 0 = Kill excess AUDIO until Audio PTS reaches Video START Trigger PTS
// Target 1 = Kill excess VIDEO  "      "    "     "       "  *END*    "      "
//
// Armed 0 = Stats only
// Armed 1 = KILL KILL KILL
//
void Out_FILTER()
{

  int iActive, iHDR_Srch_Depth, iHDR_Adjust, iNeedNow; //, iOverflow_Len;
  int iOffset;
  unsigned int uTmp1;
  __int64 i64Tmp_PTS;

  if ( ! iOut_Target_Tail)
  {
      strcpy(szFTarget, "Head");
      i64PrimaryTrigger_PTSM = 0xFFFFFFFF; // Flag as UNKNOWN - Pickup while scanning
      i64VidFromTrigger_PTSM = 0xFFFFFFFF;
      //i64Prev_PTSM[0xE0] = 0;
      ZeroMemory(&i64Prev_PTSM, sizeof(i64Prev_PTSM));
  }
  else
  {
      strcpy(szFTarget, "TAIL");                
      i64Prev_PTSM[0xE0] = i64PrimaryTrigger_PTSM; // 0;
  }

  iTargetTailArmed = (iOut_Target_Tail && iOut_PTS_Matching);
  iVortex_Curr_Ctr = 0; iVortex_Token = 0;
  iAudioTrail  = 0; 
  iActive = 1;

  if (iCtl_Out_Force_Interlace)
      iHDR_Srch_Depth = 99;
  else
  if (iOut_TC_Adjust  ||  uBroken_Flag    || iOutFRatioCode > 0)
      iHDR_Srch_Depth = 4;
  else
      iHDR_Srch_Depth = 1;

  if (uBroken_Flag    || iOut_Fix_Hdrs_Vid 
                      || iCtl_Out_Force_Interlace 
                      || iOut_TC_Adjust  || iOut_PTS_Invent)
      iHDR_Adjust = 1;
  else
      iHDR_Adjust = 0;

  // Video Alignment at START of Video clip
  //iTmp1 = (iOut_Align_Video && ! iOut_Target_Tail);
  if (iOut_Align_Video && ! iOut_Target_Tail)
    iNeedNow = 1;
  else
    iNeedNow = 0;

  //ZeroMemory(lpMPA_C0_save, 16);  
  //ZeroMemory(lpAC3_80_save, 16);

  memset(&cStreamNeedsAligning_FRONT_Flag[0xE0], iNeedNow, 16);

  // Alignment at END of Video clip

  iTmp1 = (iOut_Align_Video && iTargetTailArmed);
  memset(&cStreamNeedsAligning_REAR_Flag[0xE0],    iTmp1, 16);

  // Flag ALL Audio streams from 0x80 thru 0xDF 

  memset(&cStreamNeedsAligning_REAR_Flag[SUB_AC3], iTmp1, (0xE0-SUB_AC3));

  // Including MpegAudio, which is controlled by SB_MPA rather than 0xC0,
  // To avoid collision with DDPLUS
  memset(&cStreamNeedsAligning_REAR_Flag[SUB_MPA], iTmp1, (32));


  //if (iInPS2_Audio)
  //   memset(&cStreamNeedsAligning_REAR_Flag[0xBF], iTmp1, 16);

  // Audio Alignment at START of clip
  if (iOut_Align_Audio && !iOut_Target_Tail)
    iNeedNow = 1;
  else
    iNeedNow = 0;

  // Flag ALL Audio streams from 0x80 thru 0xDF
  if (!iOut_Target_Tail)
  {
      // Cover all std audio substream-ids
      memset(&cStreamNeedsAligning_FRONT_Flag[SUB_AC3], iNeedNow, (0xE0-SUB_AC3));
      // and SUB_MPA as noted above
      memset(&cStreamNeedsAligning_FRONT_Flag[SUB_MPA], iNeedNow, (32));
  }

  //if (iInPS2_Audio)
  //   memset(&cStreamNeedsAligning_FRONT_Flag[0xBF], iNeedNow, 1); // PS2

  cStream_Id  = 0;    uSubStream_Id  = 0;   cDummy1 = 0; cDummy2 = 0;
  cStream_Cat = '?';  uSubStream_Cat = '?';
  cPut_Stream_Id = 0xAF;
  lpMpeg_PKT_Anchor = lpMpeg_FROM;


  if (process.iOutUnMux ||
      (iCtl_Out_KillPadding 
             // && !iFixedRate 
             && (!iOutVOB || !MParse.iVOB_Style)))
    cBoringStreamDefaultAct = 'D';
  else                   
    cBoringStreamDefaultAct = 'A';


  if (DBGflag)
  {
      iTmp3 = lpMpeg_PKT_Anchor - lpMpeg_Copy_Buffer;
      sprintf(szBuffer, "\n----- FILTER %s   Offset=%d   Target=%d   Armed=%d,  Trig_PTS=%08X PAD=%d  FixEdge=%d",
                                 szFTarget, iTmp3, iOut_Target_Tail, iOut_PTS_Matching, 
                                           i64PrimaryTrigger_PTSM, iCtl_To_Pad, iOut_FixPktEdge);
      DBGout(szBuffer);
      sprintf(szBuffer, "                Align: Video=%d,%08X  Audio=%d,%08X\n",
                 iOut_Align_Video, (int)(cStreamNeedsAligning_REAR_Flag[0xE0]),
                 iOut_Align_Audio, (int)(cStreamNeedsAligning_FRONT_Flag[SUB_MPA]));
      DBGout(szBuffer);
  }


  // PKT SYNCING

  while (iActive
      && !Out_CanFlag)
  {
    // Search for a PS/PES Start Code Prefix: 0x00 0x00 0x01
    if ( ! *lpMpeg_PKT_Anchor)             // 0x00
    {
      lpMpeg_PKT_Anchor++;
      if (! *lpMpeg_PKT_Anchor)            //      0x00
      {
        lpMpeg_PKT_Anchor++;
        // Allow for big blocks of nulls
SkipNulls: // for(;;)
        {
           if ( ! *lpMpeg_PKT_Anchor)
           {
              if (lpMpeg_PKT_Anchor < lpMpeg_EOI
              //  && iRange_FirstBlk
                  && !Out_CanFlag)
              {
                     lpMpeg_PKT_Anchor++;
                     goto SkipNulls;  // PERFORMANCE:- goto instead of "for"
              }
              else
              {
                lpMpeg_PKT_Anchor-=2;
                goto Vortex;
              }
           }
        }
        /*
        while (lpMpeg_PKT_Anchor < lpMpeg_EOI
               // && iRange_FirstBlk
                  && !Out_CanFlag
                  && !(*lpMpeg_PKT_Anchor))
        {
                        lpMpeg_PKT_Anchor++;
        }
        */

        if (*lpMpeg_PKT_Anchor == 0x01)      //          0x01
        {
           lpMpeg_PKT_Anchor++;   
           iOut_CheckedPackets++;
           cMpeg_Out_Pkt_ACT = 0;

           if (iVortex_Curr_Ctr)
           {
              iOut_Resyncs++;
              iOut_Vortex_Bytes += iVortex_Curr_Ctr;
              if (!cOut_ResetStreamId)
              {
                   cOut_ResetStreamId   = cStream_Id;
                   cOut_RS_PrevStreamId = cPut_Stream_Id;
              }

              if (DBGflag)
              {
                 sprintf(szBuffer, "  *UNKNOWN BITS x%04X GAP=%d before x%02X", 
                                      iVortex_Token, iVortex_Curr_Ctr, 
                                                         (int)(cStream_Id));
                 DBGout(szBuffer) ;
              }
              cPut_Stream_Id = 0xAF; // reset to arbitrary stream
              ZeroMemory(iStartCodePart, sizeof(iStartCodePart)); // Reset ALL Stream context flags


              if (process.iOut_DropCrud)
                  lpMpeg_FROM = lpMpeg_PKT_Anchor - 3; // Reset past the crud
           }

           if (iOut_PS1PS2  // Convert Private Stream 1 to PS2 - TWINHAN PROBLEM
           &&  *lpMpeg_PKT_Anchor == 0xBD)
               *lpMpeg_PKT_Anchor  = 0xBF;

           cStream_Id    = *lpMpeg_PKT_Anchor++;

           uSubStream_Id  = cStream_Id;
           uSubStream_Cat = /*(BYTE)*/ (uSubStream_Id&0xF0);
           if (uSubStream_Cat  == 0xC0)  // Avoid collision between MPA and DDPLUS
           {
               uSubStream_Id += SUB_MPA - 0xC0;
               uSubStream_Cat = SUB_MPA;
           }

           iKill_PTS_Flag = 0;
#ifdef DBG_FULL
           if (DBGflag)
           {
               sprintf(szBuffer, "\nPKT %02X    Pack#%03d",
                                    cStream_Id, iPack_Ctr);
               DBGout(szBuffer) ;
           }
#endif

           if ( cStream_Id >= 0xE0 && cStream_Id <= 0xEF) // Video Streams
           {
                cStream_Cat  = 'V';
                iGOP_Memo_ix = 0;
           }
           else
           if ( cStream_Id >= 0xC0 && cStream_Id <= 0xCF
           ||  (cStream_Id == 0xBF && iInPS2_Audio)
           ||   cStream_Id == 0xBD)
           {
                cStream_Cat  = 'A';
                iGOP_Memo_ix = 1;
           }
           else
           {
              cStream_Cat  = '?';
              iGOP_Memo_ix = 2;
           }

           // Calc some lengths - although only useful for PES packets

           lpMpeg_ix3 = lpMpeg_PKT_Anchor;     // Begin just after start code
           cPES_Field_Flags = 0;        // PES Field flags

           if ( cStream_Id == cPACK_START_CODE ) // PACK header  0xBA
           {
                iPack_Ctr++;   iOut_CheckedPackets--;
                iBetweenPacks++;
                uSentinel = (*lpMpeg_ix3)<<1;   // Mpeg Version
                if (uSentinel >= 0x80)  
                    iPkt_Between_Len  = 10;     // Default for Mpeg-2 - excluding stuffing bytes
                else
                    iPkt_Between_Len  = 8;      // Default for Mpeg-1

                lpMpeg_End_Packet  = lpMpeg_PKT_Anchor + iPkt_Between_Len;  
                lpMpeg_PES_BeginData = lpMpeg_ix3;
           }
           else
           {
              // Usually a PES packet
              // 2 byte data length field is in IBM370 format, NOT Intel
              iPkt_Between_Len  =                 (*(lpMpeg_ix3++))<<8;
              iPkt_Between_Len  = (iPkt_Between_Len | *(lpMpeg_ix3++)) 
                                + 2;
              // Number of bytes BETWEEN the END of one pkt stat code 
              //                    and the START of the next.
              // Which is 2 bytes LONGER than PES Data_Length field
              // But is 4 bytes SHORTER than the FULL packet length

              lpMpeg_End_Packet  = lpMpeg_PKT_Anchor + iPkt_Between_Len;  

              // Save Mpeg type 
              uSentinel = *lpMpeg_ix3;

              if ( cStream_Id == 0xE0  // Most common 
              ||   cStream_Id == 0xC0
              ||   cStream_Id == 0xBD
              ||  (cStream_Id == 0xBF && iInPS2_Audio)
              ||  (      cStream_Id != 0xBC // PSM = program_stream_map
                      && cStream_Id != 0xBE // Padding stream
                      && cStream_Id != 0xBF // PS2 VOB NAV Packet
                      && cStream_Id != 0xF0 // ECM 
                      && cStream_Id != 0xF1 // EMM 
                      && cStream_Id != 0xFF // PSD = program_stream_directory
                      && cStream_Id != 0xF2 // DSMCC_stream
                      && cStream_Id != 0xF8 // ITU-T Rec. H.222.1 type E
                   )
                 )
              {
                // Special Packet length fix option
                if (iOut_FixPktEdge)  
                {
                   // check for known case of input stream error 
                   //     - packet length off by 1 or 2 bytes
                   lpMpeg_ix4 = lpMpeg_End_Packet - 1;
                   if (*(UNALIGNED DWORD*)lpMpeg_ix4 == 0xBA010000)
                   {
                      iPkt_Between_Len--;
                      iTmp1 = iPkt_Between_Len - 2;
                      *(lpMpeg_PKT_Anchor)   = (char)(iTmp1/256);
                      *(lpMpeg_PKT_Anchor+1) = (char)(iTmp1);
                      iFixEdges++;
                   }
                   else
                   {
                     lpMpeg_ix4--;;
                     if (*(UNALIGNED DWORD*)lpMpeg_ix4 == 0xBA010000)
                     {
                      iPkt_Between_Len -=2;;
                      iTmp1 = iPkt_Between_Len - 2;
                      *(lpMpeg_PKT_Anchor)   = (char)(iTmp1/256);
                      *(lpMpeg_PKT_Anchor+1) = (char)(iTmp1);
                      iFixEdges++;
                     }
                   }
                }

                iPES_Mpeg2 = ((uSentinel>>6) == 2);

                if (iPES_Mpeg2) // Mpeg-2 style PES ?
                {
                    uPES_Hdr_Len         = (*(lpMpeg_ix3+2));

                    if (iOut_PS1PS2  // Convert Private Stream 1 to PS2 - TWINHAN PROBLEM
                        &&  *lpMpeg_PKT_Anchor == 0xBF)
                    {
                        uPES_Hdr_Len +=4;  // flag PS1 control info as being inside hdr
                      *(lpMpeg_ix3+2) = (unsigned char)(uPES_Hdr_Len);
                    }
               

                    uPkt_Hdr_Full_Len    = uPES_Hdr_Len + 3;
                    lpMpeg_PES_BeginData = lpMpeg_ix3 + uPkt_Hdr_Full_Len;
                    cPES_Field_Flags     = *(lpMpeg_ix3+1);   // PES HDR Field flags
                    lpMpeg_PTS_ix        = lpMpeg_PKT_Anchor+5;
                 }
                 else
                 {  // Mpeg-1 has different format PES HDR
                    Out_PES_Hdr_Mpeg1();
                 }
              }
              else 
              {     // Exceptions have no PES HDR

                    uPES_Hdr_Len  = 0;
                    uPkt_Hdr_Full_Len  = 2;
                    lpMpeg_PES_BeginData = lpMpeg_ix3 + uPkt_Hdr_Full_Len;
                    cPES_Field_Flags = 0; 
                    lpMpeg_PTS_ix = lpMpeg_PKT_Anchor+2;
              }

              
           }

           if (process.iOutUnMux && iOut_UnMux_Fmt != 1)
               lpMpeg_FROM = lpMpeg_PES_BeginData;


#ifdef DBG_FULL
           if (DBGflag)
           {
               sprintf(szBuffer, "          Len=%04d   Flags=x%02X OUT#%d",
                                  iPkt_Between_Len, cPES_Field_Flags, 
                                  iOut_PackHdrs);
               DBGout(szBuffer) ;
               sprintf(szBuffer, "            ix2=x%06X     PES Hdr=%02d   OFFSET=%d",
                                 (lpMpeg_PKT_Anchor-lpMpeg_Copy_Buffer),
                                  uPES_Hdr_Len, 
                                 (lpMpeg_PES_BeginData-lpMpeg_PKT_Anchor));
               DBGout(szBuffer) ;
               sprintf(szBuffer, "            EOI=x%06XX    PKT Hdr=%d",
                                 (lpMpeg_EOI-lpMpeg_Copy_Buffer),
                                  uPkt_Hdr_Full_Len);
               DBGout(szBuffer) ;
           }
#endif

           if (cPES_Field_Flags & 0x80)          // Is a PTS present ?
           {
              memcpy(&i64Curr_PTSM, lpMpeg_PTS_ix, 5); // Grab high order part of Time Stamp
              i64Curr_PTSM &= i64PTS_MASK_0;           // Get rid of marker bits

#ifdef DBG_FULL
              if (DBGflag)
              {
                  sprintf(szBuffer, "\tPTSM FILE=x%08X\n\t      INT=x%08X",
                            *(unsigned int*)(lpMpeg_PTS_ix), 
                             (unsigned int )(i64Curr_PTSM));
                  DBGout(szBuffer);

              }
#endif
              if (cStream_Id == 0xE0)               // Video Stream
              {
                 iTmp1 = memcmp(&i64Curr_PTSM, &i64Prev_PTSM[0xE0], 5); // Compare high order part of PTS
                 if (iTmp1 < 0 // SEQUENCE ERROR ?
                 && !process.Suspect_SCR_Flag) 
                 {
                    //  Figure out whether it is a big break or not
                    // Convert relevant PTSMs to PTS binaries.
                    i64Curr_PTS = 0;
                    lpMpeg_TC_ix2 = (unsigned char*)(&i64Curr_PTSM); 
                    PTSM_2PTS(                      (&i64Curr_PTS));

                    i64Prev_PTS =  0;
                    lpMpeg_TC_ix2 = (unsigned char*)(&i64Prev_PTSM[0xE0]); 
                    PTSM_2PTS(                      (&i64Prev_PTS));

                    // Convert to milliseconds
                    i64Prev_PTS = (i64Prev_PTS>>8) * 256 / 90;
                    i64Curr_PTS = (i64Curr_PTS>>8) * 256 / 90;
                    iTmp1 = (int)(i64Prev_PTS - i64Curr_PTS);

                      if (DBGflag)
                      {
                          sprintf(szBuffer, "\n**PTS SEQ ERR**  Diff=%d ms\n  PTS=x%08X  =%d ms  Pack#%d\n  WAS=x%08X  =%d ms\n",
                                              iTmp1, 
                                              (unsigned int)(i64Curr_PTSM),      
                                              (unsigned int)(i64Curr_PTS), 
                                              iPack_Ctr,
                                              (unsigned int)(i64Prev_PTSM[0xE0]), 
                                              (unsigned int)(i64Prev_PTS));
                          DBGout(szBuffer) ;
                      }

                    if (iTmp1 > 200)  // Allow for Panasonic DVD recorder
                    {
                       if (iOut_Target_Tail)
                           cStreamNeedsAligning_REAR_Flag[uSubStream_Id] = 0;
                       else
                            i64PrimaryTrigger_PTSM = 0;          //  ABANDON SHIP !
                       //    i64PrimaryTrigger_PTSM = i64Curr_PTSM;  // Hoist The Main Brace

                       if (DBGflag)
                       {
                          sprintf(szBuffer, "*RESET TRIG*  %s  PACK#%d\n", 
                                                szFTarget, iPack_Ctr);
                          DBGout(szBuffer);
                       }

                    }
                 }
              }  // END-IF  Primary Video Stream

           }
           else
           {
             //if (cStream_Cat == 'V')               // Video Streams
             //    i64Curr_PTSM = i64Prev_PTSM[0xE0];
             //else
             if (cStream_Id  != cPACK_START_CODE)
             {
                 i64Curr_PTSM = i64Prev_PTSM[uSubStream_Id]; 
             }
           }


           // All packets come here
           if (iOut_PTS_Matching)
               iPTS_Comparison = memcmp(&i64Curr_PTSM, &i64PrimaryTrigger_PTSM, 4); // Compare high order part of PTS
           else
               iPTS_Comparison = iOut_Target_Tail;


#ifdef DBG_FULL
           if (DBGflag)
           {
               if (cStream_Id != cPACK_START_CODE)
               {
                  lpMpeg_TC_ix2 = (unsigned char *)(&i64Curr_PTSM);
                  PTSM_2PTS(&i64Tmp_PTS);

                  sprintf(szBuffer,
                  "   PTS=%04ums =%08d  x%08X   TST=%d %s Flags=%X  Len=%d, Hdr=%u",
                                (unsigned int)(i64Tmp_PTS/90),
                                (unsigned int)(i64Tmp_PTS), 
                                (unsigned int)(i64Curr_PTSM), 
                                // *(unsigned*)(lpMpeg_PKT_Anchor+5),
                                 iPTS_Comparison,
                                 szFTarget, 
                                 *(lpMpeg_ix3+1), 
                                 iPkt_Between_Len, uPES_Hdr_Len);
                  DBGout(szBuffer) ;
                  sprintf(szBuffer,
                  "   Ref=%04ums =%08u  x%08X\n  Diff=%04dms",
                     (unsigned int)(i64PrimaryTrigger_PTS/90), 
                     (unsigned int)(i64PrimaryTrigger_PTS),
                                      i64PrimaryTrigger_PTSM,
                     (unsigned int)((i64PrimaryTrigger_PTS-i64Tmp_PTS)/90));
                  DBGout(szBuffer) ;
               
               }
           }  // ENDIF DBG
#endif

           iVortex_Curr_Ctr = 0; iVortex_Token = 0;
           iGOP_Memo[0] = 0;

           if ( cStream_Cat == 'V')   // Video Streams
           {

             if (iTargetTailArmed
             // &&     (iPTS_Comparison >  0  // Disabled #6717 
                  &&   ! cStreamNeedsAligning_REAR_Flag[uSubStream_Id])
             {
                 // iOut_SkippedVideoPackets++; // Prefer to only count Audio at the moment
                 cMpeg_Out_Pkt_ACT = 'D';    // flag to skip this packet
             }
             else
             {
                cMpeg_Out_Pkt_ACT = 'A'; //  span video packet at start

                // If it is the first Head Video packet - store the PTS
                if (i64PrimaryTrigger_PTSM == 0xFFFFFFFF)
                {
                    i64PrimaryTrigger_PTSM   = i64Curr_PTSM;
                    i64VidFromTrigger_PTSM = i64PrimaryTrigger_PTSM;
                    
                    lpMpeg_TC_ix2 = (unsigned char *)(&i64VidFromTrigger_PTSM);
                    PTSM_2PTS(&i64Tmp_PTS);
                    i64VidFromTrigger_PTS = i64Tmp_PTS;
                    
                    if (iPES_Mpeg2)
                    {
                      // Correct PTS for B-frames displaying before I-Frame
                      if (W_Clip.uFrom_TCorrection
                      &&  W_Clip.uFrom_FPeriod_ps)
                      {
                          uTmp1 = (W_Clip.uFrom_FPeriod_ps 
                                          /1000000 * 90
                                 * W_Clip.uFrom_TCorrection);
                                        
#ifdef DBG_FULL
                          if (DBGflag)
                          {
                            sprintf(szBuffer, "             B-Frame %ut  TCorr=%02u  Period=%u",
                                         uTmp1, W_Clip.uFrom_TCorrection,
                                                W_Clip.uFrom_FPeriod_ps);
                            DBGout(szBuffer) ;
                          }
#endif
                          Out_TC_Rewind(&i64VidFromTrigger_PTSM,
                                        &i64VidFromTrigger_PTS,
                                         uTmp1);

                          i64PrimaryTrigger_PTSM = i64VidFromTrigger_PTSM;
                      }

                      uTmp1 = 90000; // Arbitrary safety margin = 1sec
                      Out_TC_Rewind(  &i64PrimaryTrigger_PTSM, 
                                    &i64PrimaryTrigger_PTS,
                                       uTmp1);

                      i64PrimaryTrigger_PTSM &= PTS_MASK_0;  // Turn off mask bits for comparison usage
                     
#ifdef DBG_FULL
                      if (DBGflag)
                      {
                          sprintf(szBuffer, " *VIDEO FROM*  PTS=x%08x : %08x ",
                                         *(unsigned*)(lpMpeg_PKT_Anchor+5),
                                                     i64PrimaryTrigger_PTSM);
                          DBGout(szBuffer) ;
                      }
#endif

                    } // ENDIF  Mpeg2 PES format
                }

                // Is packet data area long enough to include rate ? (??)
                if ( iPkt_Between_Len > 10)
                {
                   lpMpeg_ix3 = lpMpeg_PES_BeginData;  // Skip PES header fields
#ifdef DBG_FULL
                   if (DBGflag)
                   {
                      sprintf(szBuffer, "  VideoData=%08X", *(UNALIGNED DWORD*)(lpMpeg_ix3) );
                      DBGout(szBuffer) ;
                   }
#endif
                   // copy data up to end of final pic

                   // Always Keep ?
                   if (! iOut_Target_Tail    
                   // || (iOut_PTS_Matching  && iPTS_Comparison < 0    // Disabled #6717
                   //    && cStreamNeedsAligning_REAR_Flag[uSubStream_Id])
                       ) 
                   {
                      // Find first control header 
                      iGOPFixed_Flag = 0;
                      if (iHDR_Adjust
                      || cStreamNeedsAligning_FRONT_Flag[uSubStream_Id])
                      {
                         Out_Vid_Hdr_SCAN(iHDR_Srch_Depth);
                      }

                   } // ENDIF Always Keep


                   // Maybe Keep Leading Part of Packet?
                   else
                   if (iPTS_Comparison >= 0)
                   {
                      // Only split if Alignment still needed
                      if (cStreamNeedsAligning_REAR_Flag[uSubStream_Id]
                           // &&  iPTS_Comparison == 0  // #6115->DISABLED
                         )
                      {
                          // Preamble MAY require copying of SEQ/GOP HDRs as well
                          if (!iPreambleOnly_Flag)
                             iTmp2 = 1;       // Search til SEQ or GOP or PIC
                          else
                          if (process.iSEQHDR_NEEDED_clip1)
                             iTmp2 = process.iSEQHDR_NEEDED_clip1; // Specific search till GOP or PIC
                          else
                             iTmp2 = 2;       // Non-Specific search til GOP or PIC

                          Out_Vid_Hdr_SCAN(iTmp2);

                          iOffset = lpMpeg_ix3 - lpMpeg_PES_BeginData;
#ifdef DBG_FULL
                          if (DBGflag)
                          {
                              sprintf(szBuffer, "HDR-%d Offset %04d, Pre=%d\n", 
                                                iScanResult, iOffset, iPreambleOnly_Flag);
                              DBGout(szBuffer);
                          }
#endif

                          if (iScanResult < 99) // Found some headers ? (WAS < 3)
                          {
                             if (iOffset < 5) // At the start ?
                             {
                               cMpeg_Out_Pkt_ACT = 'D'; // flag to skip this packet
                               if (DBGflag)
                               {
                                  DBGout("  DROPPED");
                               }
                             }
                             else
                             {
                               lpMpeg_SPLIT_ix = lpMpeg_ix3 - 4;
                               Out_Filter_Split_Rear(&iOut_SplitVideo_PostPackets);    // Keep the start
                             }
                          }

                          cStreamNeedsAligning_REAR_Flag[uSubStream_Id] = 0;

                      } // ENDIF Clip Finish Packet to be split
                      else
                        cMpeg_Out_Pkt_ACT = 'D'; // flag to skip this packet

                   } // ENDIF pts comparison

                } // ENDIF Packet long enough to include rate


                if (iOut_Target_Tail && cMpeg_Out_Pkt_ACT == 'A')
                {
                    iOut_AddedVideoPackets++;
                    iOut_AddedVideoBytes += (iPkt_Between_Len + 4);

#ifdef DBG_FULL
                       if (DBGflag)
                       {
                          sprintf(szBuffer, "*ADD VID* %s PACK#%d\n", 
                                                szFTarget, iPack_Ctr);
                          DBGout(szBuffer);
                       }
#endif
                }

             }  // END NOT DELETING

             i64Prev_PTSM[0xE0] = i64Curr_PTSM;

           }  // ENDIF Video Stream



           else
           {
              // MPEG AUDIO ?
              if (cStream_Id >= 0xC0 && cStream_Id <= 0xCF )
              {
                  cStream_Cat = 'A';

                  if (iOut_Audio_All || cOut_SubStreamWanted[cStream_Id])
                      cMpeg_Out_Pkt_ACT = 'A';  //  span packet without dropping
                  else
                      cMpeg_Out_Pkt_ACT = 'D';  //  Drop Packet
              }

              else
              // Private Stream 2 - MAYBE DTV Audio OR Maybe VOB NAV PACK
              if (cStream_Id == 0xBF)
              {
                  if (iInPS2_Audio)
                  {
                      cStream_Cat    = 'A';
                      uSubStream_Cat = SUB_AC3;
                      cMpeg_Out_Pkt_ACT = 'A';  //  span packet without dropping
                      if (process.iOutUnMux && iOut_UnMux_Fmt != 1) // && !iCtl_Out_Keep_Ac3Hdr)
                      {
                          lpMpeg_FROM = lpMpeg_PES_BeginData+3; // skip the AC3/DTS ctl
                      }
                  }
                  else
                  {
                    //if (iOut_TC_Adjust) // TimeStamp adjustment supresses NAV Packs
                    //  cMpeg_Out_Pkt_ACT = 'D';
                    //else
                       cMpeg_Out_Pkt_ACT = cBoringStreamDefaultAct;
                  }
              }

              else
              {
                 cStream_Cat = '?';
                 // Private Stream 1 ?
                 if (cStream_Id == 0xBD)
                 {
                    lpMpeg_ix3 = lpMpeg_PES_BeginData;
                    // is it an audio sub-stream ?
                    uSubStream_Id  = *lpMpeg_ix3;
                    uSubStream_Cat = /*(BYTE)*/ (uSubStream_Id & 0xF0);

                    if (iOut_Audio_All || cOut_SubStreamWanted[uSubStream_Id])
                        cMpeg_Out_Pkt_ACT = 'A';  //  span packet without dropping
                    else
                        cMpeg_Out_Pkt_ACT = 'D';  //  Drop Packet

                    if (uSubStream_Cat == SUB_AC3 // Includes DTS range
                    ||  uSubStream_Cat == SUB_DDPLUS)
                    {
                        cStream_Cat = 'A';

                        if (process.iOutUnMux && iOut_UnMux_Fmt != 1)
                        {
                          //if (iCtl_Out_Keep_Ac3Hdr)
                          //    iTmp1 = 1; // just skip the sub-stream-id
                          //else
                              iTmp1 = 4; // also skip the AC3/DTS ctl
                          lpMpeg_FROM = lpMpeg_PES_BeginData + iTmp1; 
                        } 
                    }
                    else
                    if (uSubStream_Cat == SUB_PCM)
                    {
                        cStream_Cat = 'A';
                        cMpeg_Out_Pkt_ACT = 'A';  //  span packet without dropping
                        if (process.iOutUnMux && iOut_UnMux_Fmt != 1)
                        {
                          //if (iCtl_Out_Keep_Ac3Hdr)
                          //    iTmp1 = 1; // just skip the sub-stream-id
                          //else
                              iTmp1 = 7; // also skip the LPCM ctl
                          lpMpeg_FROM = lpMpeg_PES_BeginData + iTmp1; 
                        }
                    }
                    else
                    if (uSubStream_Cat == SUB_SUBTIT)
                    {
                        cStream_Cat = 'A';
                        cMpeg_Out_Pkt_ACT = 'A';  //  span packet without dropping
                        if (process.iOutUnMux && iOut_UnMux_Fmt != 1)
                             lpMpeg_FROM = lpMpeg_PES_BeginData+1;
                    }
                    else
                      cMpeg_Out_Pkt_ACT = cBoringStreamDefaultAct;
                 }
              }

              // By now we know if it is an Audio type stream
              if ( cStream_Cat == 'A' && cMpeg_Out_Pkt_ACT != 'D') // Audio Category ?
              {
                if (iOut_HideAudio)
                  cMpeg_Out_Pkt_ACT = 'D'; //   flag to skip this packet
                else
                // is Audio PTS before trigger Video PTS ?
                if (iPTS_Comparison < 0)
                {
                    if (iOut_Target_Tail)  // Keep Audio until out-point trigger ?
                    {
                       // TODO:
                       // if Big Sequence Error in Audio PTS ?
                       //    ABORT filtering
                       // else
                      // Allow for audio trig not reached, even though within tail
                      if (cStreamNeedsAligning_FRONT_Flag[uSubStream_Id]
                      &&  memcmp(&i64Curr_PTSM, &i64VidFromTrigger_PTSM, 4) < 0)
                      {
                         cMpeg_Out_Pkt_ACT = 'D'; //   flag to skip this packet
                      }
                      else
                      {  
                         Out_SplitChk_FRONT_Audio();
                         
                         if (cMpeg_Out_Pkt_ACT != 'D')
                         {
                            cStreamNeedsAligning_FRONT_Flag[uSubStream_Id] = 0;  
                            iOut_AddedAudioPackets++;  
                            iOut_AddedAudioBytes += (iPkt_Between_Len + 4);
                         }
                      }
                    }
                    else
                    { // Audio at Start to be skipped - MAYBE


                      // OLD CODE NOT EXACT ENOUGH
                      //if (iOut_PTS_Matching)
                      //{
                      //   cMpeg_Out_Pkt_ACT = 'D'; //   flag to skip this packet
                      //   iOut_SkippedAudioPackets++;
                      //}

                      // SPLIT CAN BE EARLIER THAN CRUDE PTS MATCHING SUGGESTS
                      if (cStreamNeedsAligning_FRONT_Flag[uSubStream_Id])                                           Out_SplitChk_FRONT_Audio();
                      {
                         Out_SplitChk_FRONT_Audio();
                         //if (cMpeg_Out_Pkt_ACT != 'D')
                         //{
                         //   cStreamNeedsAligning_FRONT_Flag[uSubStream_Id] = 0;
                         //}
                      }
                     } 
                } // ENDIF AUDIO before Trigger

                else // PTS comparison >= REF
                if (iOut_Target_Tail)  // Keep Audio only up till trigger ? (end of clip audio padding)
                {
                  if (iOut_PTS_Matching)
                  {
                      iAudioTrail++;

                      if (cStreamNeedsAligning_REAR_Flag[uSubStream_Id])
                      {
                          cMpeg_Out_Pkt_ACT = 'A'; // Keep this one, continue checking in case of multiple audio tracks
                          cStreamNeedsAligning_REAR_Flag[uSubStream_Id] = 0;
                          Out_Split_Audio_Rear();
                      }  // ENDIF Needs Aligning 
                      else
                        cMpeg_Out_Pkt_ACT = 'D'; //   flag to skip this packet
                          
                  } // ENDIF PTS Matcning enabled

                  /*
                  // TODO :- Allow trailing Audio Alignment when not doing mathcing
                  else
                  {
                      if (cStreamNeedsAligning_REAR_Flag[uSubStream_Id])
                      {
                          cMpeg_Out_Pkt_ACT = 'A'; // Keep this one, continue checking in case of multiple audio tracks
                          cStreamNeedsAligning_REAR_Flag[uSubStream_Id] = 0;
                          Out_Split_Audio_Rear();
                      }  // ENDIF Needs Aligning 
                      else
                        cMpeg_Out_Pkt_ACT = 'D'; //   flag to skip this packet
                  }
                  //  END TODO
                  */ 

                }
                else
                {  // Leading Packet
                   cMpeg_Out_Pkt_ACT = 'A'; // Keep this one, continue checking in case of multiple audio tracks
                   Out_SplitChk_FRONT_Audio();
                         
                   if (cMpeg_Out_Pkt_ACT != 'D')
                   {
                       cStreamNeedsAligning_FRONT_Flag[uSubStream_Id] = 0;
                   }

                }

                i64Prev_PTSM[uSubStream_Id] = i64Curr_PTSM;


              } // ENDIF Audio  Category 

              else
              if ( cStream_Id == cPACK_START_CODE ) // PACK header
              {
                if (iAudioTrail > 0 /*1*/)  // Only set during END-CLIP PTS matching
                {
                    cMpeg_Out_Pkt_ACT = 'D';
                    iPkt_Between_Len = -4;
                    Out_COMMIT_PKT(1, 8801);        // Copy what's been done
                    lpMpeg_PKT_Anchor = lpMpeg_EOD; // ESCAPE
                    cMpeg_Out_Pkt_ACT = 0;          // Need no more
                    iActive = 0;
                    if (DBGflag)
                    {
                       DBGout("  *TERMINAL*") ;
                    }
                }
                else
                {
                  if (iOut_TC_Adjust || iOut_Fix_SD_Hdr || iOut_FixPktEdge)
                  {
                      iPkt_Between_Len = Out_Fix_Pack_Hdr(lpMpeg_PKT_Anchor-4, 0) -4;
                  }
                  else
                  {
                     // PACK headers have packet length in a funny form
                     if ((*lpMpeg_PKT_Anchor) & 0x40)  // Mpeg-2 or later ?
                     {
                        lpMpeg_ix3 = lpMpeg_PKT_Anchor + 9;
                        iPkt_Between_Len = ((*lpMpeg_ix3) & 7 ) + 10;
                     }
                     else
                        iPkt_Between_Len = 8;    // Mpeg-1 is fixed length
                  }

                  // Detect orphaned PACK header
                  if ((cPut_Stream_Id == cPACK_START_CODE  &&  iOut_PTS_Matching) //  &&  ! MParse.iVOB_Style)
                  ||  process.iOutUnMux
                  ||  (iOut_FixPktLens && iPkt_Between_Len < 8)
                  )
                  {
                      cMpeg_Out_Pkt_ACT = 'D'; // NOT QUITE RIGHT - Better to flush PREVIOUS Pack Header
                      if (DBGflag)
                      {
                          sprintf(szBuffer,"  *PACK HDR DROPPED*  Len=%d, Out#%d", iPkt_Between_Len, iOut_PackHdrs);
                          DBGout(szBuffer) ;
                      }
                  }
                  else
                  {
                      cMpeg_Out_Pkt_ACT = 'A'; // Keep this one, continue checking in case of multiple audio tracks
                      iOut_PackHdrs++;
                  }

                } // ENDELSE NOT FINAL
              } // ENDIF PACK HDR

              else
              if (cStream_Id == 0xBF) // Private Stream 2 = VOB NAV Pack OR PS2 Audio
              {
                  if (iInPS2_Audio)
                  {
                      cStream_Cat    = 'A';
                      uSubStream_Cat = SUB_AC3;
                      cMpeg_Out_Pkt_ACT = 'A';  //  span packet without dropping
                      if (process.iOutUnMux && iOut_UnMux_Fmt != 1) // && !iCtl_Out_Keep_Ac3Hdr)
                      {
                          lpMpeg_FROM = lpMpeg_PES_BeginData+3; // skip the AC3/DTS ctl
                      }
                  }
                  else
                  {
                    //if (iOut_TC_Adjust) // TimeStamp adjustment supresses NAV Packs
                    //  cMpeg_Out_Pkt_ACT = 'D';
                    //else
                       cMpeg_Out_Pkt_ACT = cBoringStreamDefaultAct;
                  }
              }

              else
              if (cStream_Id == 0xBB) // System Header
              {
                  cMpeg_Out_Pkt_ACT = cBoringStreamDefaultAct;
                  lpMpeg_ix3 = lpMpeg_PKT_Anchor-4;
                  iPkt_Between_Len = Out_Fix_SysHdr(lpMpeg_ix3, "PKT", 0) - 4;
              }


              else
              if (cStream_Id == 0xBE) // Padding Stream
              {
#ifdef DBG_FULL
                 if (DBGflag)
                 {
                     DBGout("   *PADDING FOUND*") ;
                 }
#endif
                  // Temporarily suppressed the removal of padding streams
                  //   to make regression testing easier
                  //if (iOutVOB)
                     cMpeg_Out_Pkt_ACT = cBoringStreamDefaultAct;
                  //else
                  //   cMpeg_Out_Pkt_ACT = 'D';
                     iTmp1 = iPkt_Between_Len + 4;
                     if (cMpeg_Out_Pkt_ACT == 'D')
                         iOutPaddingBytes -= iTmp1;
                     else
                         iOutPaddingBytes += iTmp1;
              }

              else
              if (cStream_Id == 0xB9) // System END code
              {
                  cMpeg_Out_Pkt_ACT = 'D';
                  iPkt_Between_Len  = 0;
              }

              else
              {
                  iOut_UnkPackets++;
                  if (!cOut_UnkStreamId)
                  {
                       cOut_UnkStreamId = cStream_Id;
                  }

                  if (DBGflag)
                  {
                      sprintf(szBuffer, "      *UNKNOWN* START CODE TYPE=%02X", (int)(cStream_Id) );
                      DBGout(szBuffer) ;
                  }
              } // END UNKNOWN STREAM_ID

           } // END VIDEO FALSE


           //   ALL ROADS LEAD TO ROME 

           //   span packet if we know its size
           if (cMpeg_Out_Pkt_ACT)
           {
#ifdef DBG_FULL
              if (DBGflag)
              {
                  sprintf(szBuffer, " Jump=%u =x%04X Act=%c ", iPkt_Between_Len, iPkt_Between_Len, cMpeg_Out_Pkt_ACT);
                  DBGout(szBuffer) ;
              }
#endif
              if (iOut_TC_Adjust)
              {
                // Do we need to adjust PTS DTS info on this packet ?
                if (cMpeg_Out_Pkt_ACT != 'D'  && cStream_Cat != '?')
                {

#ifdef DBG_FULL
                  if (DBGflag)
                      DBGln2("   CHK PESflags=x%02X GOP=%d", 
                                          cPES_Field_Flags, iGOP_PTS_Chk);
#endif                  
                  if (! (cPES_Field_Flags & 0x80))    // Is PTS missing ?
                  {
                    if (iOut_PTS_Invent)
                    {
                      if ((cStream_Cat == 'V' && iGOP_PTS_Chk > 1)
                      ||  (cStream_Cat == 'A'))   // && iGOP_PTS_Chk))
                      {
                         Out_Invent_PTS();
                         //iGOP_Memo = 1;
                         if (iGOP_PTS_Chk)
                             iGOP_PTS_Chk--;
                      }
                    }
                  }
                  else
                  if (iOut_TC_Adjust
                  && (cPES_Field_Flags & 0x80))         // Is a PTS present ?
                  {
                     Out_DeGap_TC(); // &i64Adjust_TC[uSubStream_Id][0]); //
                  }
                  else
                  {
                     lpMpeg_TC_ix2 = lpMpeg_PTS_ix;
                     PTSM_2PTS(&i64Adjust_TC[uSubStream_Id][0]);
                  }

                  if (iGOP_Memo [iGOP_Memo_ix]) // Is this the a GOP start PTS for this stream ?
                  {
                    i64Adjust_TC[uSubStream_Id][3] = i64Adjust_TC[uSubStream_Id][2];
                    i64Adjust_TC[uSubStream_Id][2] = i64Adjust_TC[uSubStream_Id][0];
                    iGOP_Memo [iGOP_Memo_ix] = 0;
                  }
                } // END INTERESTING PACKET
              } // END INVENT PTS

              // Recalculate end, in case I forgot  
              lpMpeg_End_Packet  = lpMpeg_PKT_Anchor + iPkt_Between_Len;  

              // Do we want to avoid accumulating this packet ?
              if ((cMpeg_Out_Pkt_ACT == 'D'  
                    // &&  (MParse.iVOB_Style // iOut_PTS_Matching)
                        //  || process.iOutParseMore  
                        //  || iOut_Target_Tail    // FIX FLOW-ON BUG FROM AUDIO POST-CUT MATCHING
                    //    )
                   )
              || process.iOutParseMore  
              || process.iOutUnMux
              || process.iOut_DropCrud
              || iKill_PTS_Flag
              || iOut_FixPktEdge
              || lpMpeg_End_Packet >= lpMpeg_EOI) 
              {
                  if (cMpeg_Out_Pkt_ACT != 'D')
                     cPut_Stream_Id = cStream_Id;

                  Out_COMMIT_PKT(1, 8802); // This will also increment lpMpeg_PKT_Anchor as needed

                  // EXPERIMENTAL CODE FOR A VERY SUSPECT STREAM
                  /*
                  if (iOut_FixPktEdge)
                  {
                     iTmp1 = 0;
                     if (*(UNALIGNED SHORT*)lpMpeg_PKT_Anchor == 0x0100)
                     {
                         lpMpeg_FROM--;
                         lpMpeg_PKT_Anchor--;
                        *lpMpeg_PKT_Anchor = 0;
                         iFixEdges++;
                         iTmp1 = 1;
                     }
                     else
                     if (*lpMpeg_PKT_Anchor == 0x01)
                     {
                         lpMpeg_FROM-=2;
                         lpMpeg_PKT_Anchor -=2;
                        *(UNALIGNED SHORT*)lpMpeg_PKT_Anchor = 0;
                         iFixEdges++;
                         iTmp1 = 2;
                     }
                     if (iTmp1 && DBGflag)
                     {
                        sprintf(szBuffer, "\n**EDGE FIX %d -> x%04X\n",
                                                  iTmp1,
                                       *(UNALIGNED DWORD*)lpMpeg_PKT_Anchor);
                        DBGout(szBuffer) ;
    
                     }
                  }
                  */
              }
              else
              {
                  cPut_Stream_Id = cStream_Id;
                  lpMpeg_PKT_Anchor += iPkt_Between_Len; // Span this packet
                  iPkt_Between_Len = 0;           // Packet done
                  cMpeg_Out_Pkt_ACT = 0;
              }

           } // ENDIF Standard Jump processing
           else
              cPut_Stream_Id = cStream_Id;


        } // ENDIF START CODE PREFIX found 0x01
        else
        {
Vortex:
            if (iVortex_Curr_Ctr < 4)
                iVortex_Token = (iVortex_Token * 256) + *(lpMpeg_PKT_Anchor);
            if (!cOut_RS_Weird)
            {
                 cOut_RS_Weird = *(lpMpeg_PKT_Anchor);
            }
            lpMpeg_PKT_Anchor++;
            iVortex_Curr_Ctr++;
        }

       } // ENDIF 2nd 0x00 found
       else
       {
            if (iVortex_Curr_Ctr < 4)
                iVortex_Token = (iVortex_Token * 256) + *(lpMpeg_PKT_Anchor);
            if (!cOut_RS_Weird)
            {
                 cOut_RS_Weird = *(lpMpeg_PKT_Anchor);
            }
            lpMpeg_PKT_Anchor++;
            iVortex_Curr_Ctr++;
       }
    } // ENDIF   1st 0x00 found
    else
    {
            if (iVortex_Curr_Ctr < 4)
                iVortex_Token = (iVortex_Token * 256) + *(lpMpeg_PKT_Anchor);
            if (!cOut_RS_Weird)
            {
                 cOut_RS_Weird = *(lpMpeg_PKT_Anchor);
            }
            lpMpeg_PKT_Anchor++;
            iVortex_Curr_Ctr++;
    } 



    // Packet has been fully processed

#ifdef DBG_FULL
    if (DBGflag && iVortex_Curr_Ctr < 12)
    {
       sprintf(szBuffer, "Bounds Chk x%02X Anchor=x%06X\n               EOI=x%06X  Len=%d CAN=%c",
                        *lpMpeg_PKT_Anchor, (lpMpeg_PKT_Anchor-lpMpeg_Copy_Buffer), 
                                     (lpMpeg_EOI-lpMpeg_Copy_Buffer), 
                                               iPkt_Between_Len, Out_CanFlag);
       DBGout(szBuffer) ;
    
    }
#endif

//Rebuf_Chk:
    if (lpMpeg_PKT_Anchor >= lpMpeg_EOI)
    {
       if (DBGflag)
       {
           DBGln4(" *EOI* ParseMore=%d Active=%d\n  FilePos=x%08X\n      End=x%08X",
                  process.iOutParseMore, iActive, i64_CurrPos, i64_CurrEnd);
           DBGln2("       Anchor=x%08X\n          EOD=x%08X",
                            (__int64)(lpMpeg_PKT_Anchor), (__int64)(lpMpeg_EOD));
       }

       // Some options require a lot more output parsing
       // Is there still selected data to be scanned within this file ?  

       // TODO:
       
       //   REDESIGN this section and Mpeg_BIG_READ
       //   to be able to self-grab extra data at end of clip
       //   even if it needs to read the start of the next file to do it

       if (process.iOutParseMore
       && iActive     // &&  ! iOut_Target_Tail
       && !Out_CanFlag)
       {
         if (i64_CurrPos < i64_CurrEnd
                  //||  i64_CurrEnd < process.length[iCurrFile]
            )
         {
             iTmp1 = Out_REBUF(lpMpeg_PKT_Anchor, 8882);         // go get it
             lpMpeg_PKT_Anchor = lpMpeg_Copy_Buffer; // reset scan to start of buffer
         }
         else
         if (lpMpeg_PKT_Anchor < lpMpeg_EOD && lpMpeg_EOI < lpMpeg_EOD )
         {
             lpMpeg_EOI = lpMpeg_EOD;
             ZeroMemory(lpMpeg_EOD, 4096);
         }
         else
         {
           iActive = 0;
           if (DBGflag)
           {
               DBGout("*END RANGE*") ;
           }
         }
       }
       else
       {
           iActive = 0;
           if (DBGflag)
           {
               DBGout("**ENDED**") ;
           }

       }
    }
  } //   ENDWHILE PKT SYNCING Filter Active








  // Allow for Out trigger not reached (eg premature EOF)

  cMpeg_Out_Pkt_ACT = 0;

  if (process.iOutUnMux)
     lpMpeg_FROM = lpMpeg_EOD;
  else
  if ((iTargetTailArmed)
  &&  ! iAudioTrail  && !Out_CanFlag)
      Out_COMMIT_PKT(1, 8883);
  else
  if (DBGflag)
  {
       DBGout("\n**NO FINAL COMMIT**\n\n") ;
  }




  //  Adjust length to match resulting points
  iMpeg_ToDo_Len = lpMpeg_EOD - lpMpeg_FROM;

  if (DBGflag)
  {
       sprintf(szBuffer, "\n\n*END FILTER*   PTS=%d  Gap=%ds\n\n",
                              i64Adjust_TC[0][1],
                             (i64Adjust_TC[0][1]/90000));
       DBGout(szBuffer) ;
  }

  return;
}






//-----------------------------------------
void Out_Padme(const int P_Len)
{
  const BYTE Mpeg2PadCode[4] = {0x00, 0x00, 0x01, cPADDING_STREAM_ID};

  int iRC, W_Remaining;
  unsigned short uShort1;

  iRC = 0;
  ZeroMemory(szBuffer, sizeof(szBuffer));

  W_Remaining = P_Len;

  while (W_Remaining > 2048) W_Remaining -=2048;

  if (W_Remaining > 6)
  {
     if (DBGflag)
     {
         DBGout("*CREATE PADDING*") ;
     }
     iOutPaddingPkts++;

     iRC = Out_RECORD(Mpeg2PadCode, sizeof(Mpeg2PadCode), 8914);
     W_Remaining = W_Remaining - sizeof(Mpeg2PadCode);
     if (iRC > -1)
     {
        uSwapFormat(&uShort1, &W_Remaining, 2);
        Out_RECORD(&uShort1, 2, 8915);
     }
  }

  while (W_Remaining > sizeof(szBuffer) && iRC > -1)
  {
      iRC = Out_RECORD(&szBuffer[0], sizeof(szBuffer), 8916);
      W_Remaining = W_Remaining - sizeof(szBuffer);
  }

  if  (W_Remaining > 0  && iRC > -1)
  {
      iRC = Out_RECORD(&szBuffer[0], W_Remaining, 8917);
  }

}




//-------------------------------------------------
int Out_RECORD(const void* P_Data,
               const int P_Len,  const int P_Caller)
{
  int iWrite_RC, iRqst_Len, iAnswer, iRC, iOutFile;
  int iTime1, iTime2, iTimeDiff; //, iTimeHurdle;;

  char *lpFrom, *lpStreamAbbr, *ext;
  char *lpFmtDelim, *lpSetExt;

  iRC = 0;
  iRqst_Len = P_Len;
  lpFrom = (char*)(P_Data);

  if (iOut_UnMuxAudioOnly && cStream_Cat != 'A')
     return 0;

  if (uSubStream_Id == 0xE0 || ! process.iOutUnMux)
      iOutFile = MAX_FILE_NUMBER;
  else
  {
    // Look up which output file contains this substream

    iOutFile = MAX_FILE_NUMBER;
    while (uFileSubStream_Id[iOutFile] != uSubStream_Id)
    {
      if (iOutFile > File_UnMux_Limit)
          iOutFile--;
      else
      {
          uFileSubStream_Id[iOutFile] = (unsigned short)(uSubStream_Id);
          // BUILD FILE NAME
          strcpy(File_Name[iOutFile], szOutput);
          ext = strrchr(File_Name[iOutFile], '.');
          if (!ext)
          {
               ext = stpcpy1(File_Name[iOutFile], &"Mpg2Cut2.");
          }

          if ((uSubStream_Id & 0x1F) != 00)
          {
               iTmp1 = sprintf(ext, "_Trk-%02X", uSubStream_Id);
               ext += iTmp1;
          }

          lpFmtDelim = (char *)(&"_");
          if (iOut_UnMux_Fmt == 2)
            lpSetExt = (char *)(&"WAV");
          else
          if (iOut_UnMux_Fmt == 1)
            lpSetExt = (char *)(&"M2P");
          else
          {
            lpSetExt = (char *)(&"");
            lpFmtDelim = (char *)(&".");
          }

          //if (uSubStream_Cat == 0xC0 && cStream_Id != 0xBD)
          if   (uSubStream_Cat == SUB_MPA)
          {
              lpStreamAbbr = &szOut_Xtn_AUD[0];

              //if (Mpeg_PES_Version == 2)
              //    lpStreamAbbr = (char*)(&".m2a");
              //else
              //    lpStreamAbbr = (char*)(&".m1a");
          }
          else
          if (uSubStream_Cat == SUB_AC3)
              lpStreamAbbr = (char*)(&"AC3");
          else
          if (uSubStream_Cat == SUB_DDPLUS)
              lpStreamAbbr = (char*)(&"DDP");
          else
          if (uSubStream_Cat == SUB_DTS)
              lpStreamAbbr = (char*)(&"DTS");
          else
          if (uSubStream_Cat == SUB_PCM)
              lpStreamAbbr = (char*)(&"PCM");
          else
          if (uSubStream_Cat == SUB_SUBTIT)
              lpStreamAbbr = (char*)(&"M2S");
          else
          {
              lpStreamAbbr = (char*)(&"M2Z");
              if (DBGflag)
              {
                  sprintf(szBuffer, "*UNKNOWN STREAM TYPE: x%02X SubStream=%02X", cStream_Id, uSubStream_Id);
                  DBGout(szBuffer);
              }
          }

          *ext++ = *lpFmtDelim;
          ext = stpcpy0(ext, lpStreamAbbr);
          if (*lpSetExt > ' ')
          {
              *ext++ = '.';
               ext = stpcpy0(ext, lpSetExt);
          }


          // OPEN FILE
          iTmp1 = OutFile_CREATE(iOutFile);

          if (iTmp1 >= 0)
          {
             // CHECK FOR ERROR
             if (File_UnMux_Limit > File_Limit)
                 File_UnMux_Limit--;
             else
             {
                MessageBox(hWnd_MAIN, "ERROR - Too Many Files", "Mpg2Cut2", MB_OK);
                iOut_Error += 16;
                Out_CanFlag = 'C' ;
                return -16;
             }
          }

      }

    } // ENDWHILE  File Lookup
  } // END UNMUXING SECONDARY STREAMS


retry:
  iTime1    =  iCURR_TIME_ms();

  /*BOOL WriteFile(

    HANDLE  hFile,	// handle of file to write to 
    LPCVOID  lpBuffer,	// address of data to write to file 
    DWORD  nNumberOfBytesToWrite,	// number of bytes to write 
    LPDWORD  lpNumberOfBytesWritten,	// address of number of bytes written 
    LPOVERLAPPED  lpOverlapped 	// addr. of structure needed for overlapped I/O  
   );

   If the function succeeds, the return value is TRUE.
   BUT - ALSO CHECK THAT BYTES WRITTEN == BYTES REQUESTED

   If the function fails, the return value is FALSE. 
   To get extended error information, call GetLastError. 

   */

  iWrite_RC = _write(FileDCB[iOutFile], lpFrom, iRqst_Len);

  iErrNo = errno;
  iTime2    = iCURR_TIME_ms();


#ifdef DBG_FULL
  if (DBGflag)
  {
       sprintf(szBuffer, "\n\n*WRITE*   LEN=%d  RC=%02d\n\n", iRqst_Len, iWrite_RC);
       DBGout(szBuffer) ;
  }
#endif

  if (iWrite_RC < 0)
  {
     ERRMSG_File("MpegOut", 'w', iErrNo,  File_Name[iOutFile], P_Len, P_Caller) ;

     //sprintf(szBuffer,"ERROR %d.%d WRITING %d to %s\nfrom caller# %d",
     //                  errno, iWrite_RC, P_Len, szOutput, P_Caller) ;
     //MessageBox(hWnd, szBuffer, "Mpg2Cut2 Write Error.",
     //                                MB_ICONSTOP | MB_OK);
     iRC = -2;
  }

  else
  {
    i64_CurrCopied         += iWrite_RC;
    i64_TotCopied          += iWrite_RC;
    iProgress_pending_MB   += iWrite_RC;
    if (iProgress_pending_MB > 1024000)
        Out_Progress_Chk(1);

    if (iWrite_RC != iRqst_Len)
    {
      sprintf(szBuffer,"INCOMPLETE WRITE to %s\nRC=%d\n\nDISK MAY BE FULL\n\nPlease Empty Recycle Bin,\n then click OK",
                                    szOutput, iWrite_RC ) ;
      iAnswer = MessageBox(NULL, szBuffer, "Mpg2Cut2 - FILE ERROR",
                                     MB_ICONSTOP | MB_OKCANCEL
                                      | MB_SETFOREGROUND | MB_TOPMOST);
      if (iAnswer  == IDOK)
      {
        lpFrom    += iWrite_RC;
        iRqst_Len -= iWrite_RC;
        if (iRqst_Len > 0)
          goto retry;
      }
      iRC = -1;
    }
    else
    {
      iTimeDiff = iTime2 - iTime1;

      if (iRqst_Len > 2048 && iOut_Clip_ctr)
      {
        if (iTimeDiff
        && i64_CurrCopied > 2048000
        && ((iRqst_Len / iTimeDiff) < 300
             // ||   iTimeDiff > 2000
            )
        )
        {
           if  (iMsgTime == MAXINT31)  // MAXINT)    // (! iMsgLife)
           {
               strcpy(szMsgTxt,"DISK SLOW - OUTPUT"); // Could be failing Hard Drive OR drive had powered down since last use OR Fragmentation OR Competing Task 
               DSP1_Main_MSG(0,0);
               UpdateWindow(hWnd_MAIN);
           }
           iMsgTime = iTime2;
        }
      }
      else
      {
        //iMsgLife--;

        if  ((iTime2 - iMsgTime) >= 3000)
        {
           DSP_Msg_Clear();
           DSP2_Main_SEL_INFO(0);
           iMsgTime = MAXINT31;  // MAXINT
        } 
      }
    }
  }

  if (iRC)
     Out_CanFlag = 'E';

  // Breathe every so often
  if (P_Len > 128)
  {
    iOut_Breathe_PktCtr++;
    if (iOut_Breathe_PktCtr >= iOut_Breathe_PktLim)
    {
      Sleep(1); // Allow other tasks to breathe for 1 ms
      iOut_Breathe_Tot ++; // Add 1 ms to total
      iOut_Breathe_PktCtr = 0;
    }
  }


  return iRC;
}

