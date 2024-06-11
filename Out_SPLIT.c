
#include "global.h" 
#include "out.h"
#include "MPA_HDR.h"


int iPkt_Diff_Len, iPkt_NewLen;
void Out_Find_MPA_Syncword();
void Out_Find_AC3_Syncword();
void Out_Find_PCM_Syncword();


void Out_Split_Audio_Rear()
{
  int iTmp1;

  Out_Split_Find_Any_Audio_Syncword();

  if (iScanResult > 0) // Found some headers ?
  {
      iTmp1 = lpMpeg_SPLIT_ix - lpMpeg_PES_BeginData;
      if (iTmp1 < 5) // At the start ?
      {
           cMpeg_Out_Pkt_ACT = 'D'; // flag to skip this packet

           if (DBGflag)
           {
               DBGout("  DROPPED");
           }
       }
       else
       {
          Out_Filter_Split_Rear(&iOut_SplitAudio_PostPackets);    // Keep the start
       }
  } // ENDIF Header found

}



//--------------------------------------



__int64 i64Diff_PTS, i64Curr_PTS;


void Out_HexPrt_PESHDR(char P_Desc[4])
{
  sprintf(szBuffer, 
  "%s PKT %02X  PktLen=%02X%02X=%03d~%03d  F1=%02X~%02X  F2=%02X  HdrLen=%02X~%02u PTS=x%08X", 
          P_Desc,
        *(lpMpeg_PKT_Anchor-4),  // Start Code Stream_id
                      *(lpMpeg_PKT_Anchor),    // PES Length Major
                      *(lpMpeg_PKT_Anchor+1),  // PES Length Minor
        ((*lpMpeg_PKT_Anchor)*256+(*(lpMpeg_PKT_Anchor+1))),  // PES Len decimal
                              iPkt_Between_Len,    // Nominal
                             *(lpMpeg_PKT_Anchor+2), // PES flags 1
                              cPES_Field_Flags,   // Nominal
                             *(lpMpeg_PKT_Anchor+3), // PES flags 2
                             *(lpMpeg_PKT_Anchor+4), // PES HDR LEN
                              (uPkt_Hdr_Full_Len-3), // Hdr Extn len
              *(unsigned int*)(lpMpeg_PKT_Anchor+5) // PTS
                                           );
  DBGout(szBuffer) ;

}

// Find the place to split audio packet 
void Out_Split_Find_Any_Audio_Syncword()
{

           __int64  i64Orig_PTS, i64TMP;
  //unsigned __int64  i64Curr_PTSM;
  //unsigned int *i64Prev_PTSM;

  unsigned char cPTS_Sentinel, *lpTMP;
  int iTmp1;
  unsigned int uExtn_len;

  iScanResult = 0;

  // Calculate difference between AudioPTS and adjusted VideoPTS

  // Convert relevant PTSMs to PTS binaries.
  //i64Curr_PTS = 0;
  lpMpeg_TC_ix2 = (unsigned char*)(&i64Curr_PTSM); 
  PTSM_2PTS(                       &i64Curr_PTS);

              if (DBGflag)
              {
                  sprintf(szBuffer, "\tPTSM  @IX=x%08X\n\t      INT=x%08X\n\tPTS=%05dms",
                            *(unsigned int*)(lpMpeg_TC_ix2), 
                             (unsigned int )(i64Curr_PTSM),
                             (unsigned int )(i64Curr_PTS/90));
                  DBGout(szBuffer);

              }

  i64Orig_PTS = i64Curr_PTS;

  if (iOut_PTS_Matching && iCtl_Out_Parse && !iOut_Target_Tail
  &&  i64Curr_PTS  < i64VidFromTrigger_PTS
  &&  i64Curr_PTS  // Need an Audio PTS for calculations to work
  &&  iPES_Mpeg2)
  {
      i64Diff_PTS = i64VidFromTrigger_PTS - i64Curr_PTS;

      if (DBGflag)
      {
          sprintf(szBuffer, "  VREF=%05ums =%ut", 
                               (unsigned int)(i64VidFromTrigger_PTS/90),
                               (unsigned int)(i64VidFromTrigger_PTS)
                               );
          DBGout(szBuffer) ;
          sprintf(szBuffer, "  CALC=%05dms =%ut", 
                               (unsigned int)(i64Curr_PTS/90), 
                               (unsigned int)(i64Curr_PTS)
                               );
          DBGout(szBuffer) ;
          sprintf(szBuffer, "  DIFF=%05dms =%ut", 
                               (unsigned int)(i64Diff_PTS/90),
                               (unsigned int)(i64Diff_PTS)
                               );
          DBGout(szBuffer) ;
      }
  }
  else
      i64Diff_PTS = 0;

  lpMpeg_ix3 = lpMpeg_PES_BeginData;  // Skip the PES header fields
  lpMpeg_ix4 = lpMpeg_ix3;            // Remember where data starts

  if (DBGflag)
      DBGln2("   AUDIO DATA=x%08X PktLen=%04d", 
                            *(DWORD*)(lpMpeg_ix3), iPkt_Between_Len); 

  // Splitable AUDIO format ?
  if (iPkt_Between_Len < 3)  // very short packet ?
  {
  }
  else
  if (cStream_Id >= 0xC0 && cStream_Id <= 0xCF) // MPEG AUDIO STREAM
  {
      Out_Find_MPA_Syncword();
  }
  else
  if (uSubStream_Cat == SUB_AC3
  ||  uSubStream_Cat == SUB_DTS
  ||  uSubStream_Cat == SUB_DDPLUS)
  {
      Out_Find_AC3_Syncword();
  }

  
  else
  if (uSubStream_Cat == SUB_PCM) // LPCM
  {
      // Out_Find_PCM_Syncword();  // TODO: NEEDS WORK
  }
  

  else
  {
    iScanResult = 1; // Stop Looking

      if (DBGflag)
      {
          sprintf(szBuffer, "* UNSUPPORTED TYPE x%02X x%02X", 
                                        cStream_Id, uSubStream_Cat);
          DBGout(szBuffer) ;
      }
  }



  if (iScanResult == -1)  //   && i64Diff_PTS > 40) 
  {
      cMpeg_Out_Pkt_ACT = 'D';    // flag to skip this packet

      if (DBGflag)
      {
          sprintf(szBuffer, " DEL PKT.   PTS Diff=%dms =%d", 
                                 (unsigned int)(i64Diff_PTS/90), 
                                 (unsigned int)(i64Diff_PTS));
          DBGout(szBuffer) ;
      }

  }

  if (DBGflag)
  {
      sprintf(szBuffer, " AUD SPLIT CHK=%d   Ptr=x%06X   Mode=%d",
                            iScanResult, 
                            (lpMpeg_ix3-lpMpeg_Copy_Buffer),
                            iOut_PTS_Matching);
      DBGout(szBuffer);

      i64TMP = i64Curr_PTS - i64Orig_PTS;
      DBGln4("       Orig=%02dms  remain=%02dms\n       Calc=%02dms =x%08X", // \n       Adj=%dms ",                            
                            (i64Orig_PTS/90), (i64Diff_PTS/90),
                            (i64Curr_PTS/90), i64Curr_PTS  // , (i64TMP/90)
                            );
  }

  lpMpeg_SPLIT_ix = lpMpeg_ix3;

  // Has skipping of audio frames adjusted PTS ?
  if ((i64Orig_PTS != i64Curr_PTS 
       // || !(cPES_Field_Flags & 0x80) // NO Real PTS in pkt header
      )
  && iOut_PTS_Matching)
  {
     if (DBGflag)
     {
         DBGout("    REFRESH PTS");
     }
     // Convert PTS back into Mpeg format
     if (cPES_Field_Flags & 0x80)          // Is a PTS present ?
     {
         // Convert PTS back into unmasked Mpeg format
         cPTS_Sentinel = 0x21;
     }
     else
     {
         cPTS_Sentinel = (unsigned char)(((*lpMpeg_PTS_ix) & 0xF0) | 0x01); // Retaining original sentinel nybble
     }

     // re-insert marker bits and reverse bytes
     PTS_2PTSM(&i64Curr_PTSM,  
               &i64Curr_PTS, 
                  cPTS_Sentinel);

   
     if (cMpeg_Out_Pkt_ACT != 'D'      // Retaining packet ?
     &&  !iOut_Target_Tail
     &&  iPES_Mpeg2)              
     {
         if (cPES_Field_Flags & 0x80)       // Is a PTS present ?
         {                                 // clobber it
            memcpy(lpMpeg_PTS_ix, &i64Curr_PTSM, 5);

            // Is there a DTS decode time stamp ?
            //  DTS not implemented yet
         }
         else
         {
              // CHG PKT HDR TO CREATE A PTS !   CRIKEY ! ! ! !
             if (DBGflag)
             {
                 DBGout("\n\n*** INSERTING PTS INTO HDR ***\n\n") ;
                 Out_HexPrt_PESHDR("OLD");
             }

             Out_COMMIT_PKT(0, 8831); // Commit any PREVIOUS packets 

             // Shuffle PKT PREFIX area
             memcpy(szTmp32, lpMpeg_FROM, 9);
             lpMpeg_FROM -= 5;      // make room in uncommitted area
             memcpy(lpMpeg_FROM, szTmp32, 9);

             // adjust other pointers
             lpMpeg_PTS_ix      = lpMpeg_PKT_Anchor;  // +5;
             lpMpeg_PKT_Anchor -= 5;

             // increment pkt data len
             iPkt_Between_Len  +=5;
             iTmp1 = iPkt_Between_Len - 2;
             // Reverse the bytes into the mpeg pes length
            *lpMpeg_PKT_Anchor    =   (unsigned char)  (iTmp1/ 255); // (*(unsigned char*)(&iTmp1)+1);
           *(lpMpeg_PKT_Anchor+1) =   (unsigned char)  (iTmp1);

             // flag presence of PTS
             cPES_Field_Flags = (char)(cPES_Field_Flags | 0x80);
            *(lpMpeg_PKT_Anchor+3) = cPES_Field_Flags;

             // increment hdr len
             lpTMP = lpMpeg_PKT_Anchor+4;
             uPkt_Hdr_Full_Len += 5;
             uExtn_len = uPkt_Hdr_Full_Len - 3;// (unsigned char)((*lpTMP) + 5);
            *lpTMP = (unsigned char)(uExtn_len); 
             // put in PTS
             lpMpeg_PTS_ix = lpTMP+1;
             memcpy(lpMpeg_PTS_ix, &i64Curr_PTSM, 5);

             lpMpeg_PES_BeginData = lpMpeg_PTS_ix + uExtn_len;

             if (DBGflag)
             {
                Out_HexPrt_PESHDR("NEW");
             }

         } // END  INSERTING PTS INTO HDR

     } // END-IF  Retaining packet ?

     i64Curr_PTSM &= i64PTS_MASK_0; // cleanup for internal comprison usage
     i64Prev_PTSM[uSubStream_Id] = i64Curr_PTSM;

  } // END-IF ADJUST PTS ?
}


#include "MPA_SIZE.h"   // Calculates uMPA_FrameLen

//----------------------------------------------
void Out_Find_MPA_Syncword()  // mp1, mp2, mp3 audio
{
 unsigned int  uFrameTime_PTS;
 unsigned char *lpNextHdr, *lpPrevHdr, *lpHopeHdr, *lpStart;

 int iAdjacent_OK;

 // Try to find an Mpeg audio syncword

 lpStart = lpMpeg_ix3;

  while ( ! iScanResult)
  {
     if (lpMpeg_ix3 > lpMpeg_End_Packet)
     {
         iScanResult = -1;
     }
     else

     //MPA Syncword is always Byte aligned

     if (     (   *lpMpeg_ix3         != 255 )
          || (( (*(lpMpeg_ix3+1) )     < 0xE0) ) )  // 0xFA) ) ) //  & 0xF8) != 0xF8) ) 
     // audio syncword - only 11 or 12 bits set
     {
          // Not a syncword - keep looking
          lpMpeg_ix3++;
     }
     else
     {
        // MPA HDR code parallels getaudio.c

        //uMPA_ID_Creature = 4-(lpMpeg_ix3[1]>>3)&3;  // OLD

        if (lpMpeg_ix3[1] &0x10)   // Mpeg2.5 extension steals last bit of syncword
        {
            uMPA_Mpeg25   = 0;
            uMPA_25_LSF   = 0;
        }
        else
        {
            uMPA_Mpeg25   = 1;
            uMPA_25_LSF   = 2;
        }

        if (lpMpeg_ix3[1] &0x08)   // LowSamplingFrequency Extension uses the old "ID" Flag
            uMPA_LSF      = 0;
        else
        {
            uMPA_LSF      = 1;
            uMPA_25_LSF  += 1;
        }

        uMPA_Mpeg_Ver    = 4-((lpMpeg_ix3[1]>>3)&3);  // 0=Mpeg1 1=mpeg2 2=Reserved 3=mpeg2.5
        uMPA_Layer_Ix    = 3- (lpMpeg_ix3[1]>>1)&3;
        uMPA_kBitRate_Ix =    (lpMpeg_ix3[2]>>4)&15;

        if (uMPA_kBitRate_Ix >= 0x0F // Is the bitrate setting invalid ?
        ||  uMPA_kBitRate_Ix == 0x00 // Free Format is too hard
        ||  uMPA_Layer_Ix    == 0    // Is the mpeg layer invalid ?
        //||  uMPA_Mpeg_Ver    == 3    // Is the Mpeg VERSION valid ?
           )
            lpMpeg_ix3++;   // Not a reliable header - keep looking
        else
        {
           uMPA_SampFreq_Ix =   (lpMpeg_ix3[2]>>2)&3;
           uMPA_Padding     =   (lpMpeg_ix3[2]>>1)&1;
           uMPA_Channel_ix  =   (lpMpeg_ix3[3]>>6)&3;

           uMPA_Layer     = uMPA_Layer_Ix + 1;
           uMPA_Sample_Hz = MPA_SAMPLE_HZ[uMPA_25_LSF][uMPA_SampFreq_Ix];
           uMPA_kBitRate  = MPA_KBIT_RATE[uMPA_LSF][uMPA_Layer_Ix][uMPA_kBitRate_Ix];

           MPA_FrameLen();
            
           if (iMPA_FrameLen <= 0)
               lpMpeg_ix3++;   // Not a reliable header - keep looking
           else
           {
             lpNextHdr = lpMpeg_ix3 + iMPA_FrameLen;
             if (DBGflag)
                 lpPrevHdr = lpStart - 5;

             iAdjacent_OK = 1;

             // TODO: NEED some extra tests here so that 
             // frame split across packets
             // will not be accepted
             // *UNLESS* previous frame points to it
             //              (for rejects, maybe store bytes for later ?)



             
             if (uMPA_Layer < 3)   // Further check mp1, mp2. Dunno about mp3
             {
                if (lpNextHdr  < lpMpeg_End_Packet) // Is next frame in same packet ?
                {
                   if (*lpNextHdr  != 0xFF ) // and is there really a header there
                       iAdjacent_OK = 0;
                   else
                       lpHopeHdr = lpNextHdr; 
                }
                else
                if (lpMpeg_ix3 != lpHopeHdr
                &&  lpNextHdr  != lpMpeg_End_Packet) // Is frame end at pkt boundary ?
                {
                    lpPrevHdr = lpMpeg_ix3 - iMPA_FrameLen;
                    if (lpPrevHdr  >= lpStart) // Is prev frame in same packet ?
                    {
                       if ((*lpPrevHdr  ) != 0xFF   // chk prev hdr of same length
                       &&  (*lpPrevHdr-1) != 0xFF   // chk prev hdr of similar len
                       &&  (*lpPrevHdr+1) != 0xFF ) // chk prev hdr of similar len
                           iAdjacent_OK = 0;
                     }
                    else
                    if (uMPA_Sample_Hz < 44100 && Coded_Pic_Width >= 400) // Unlikely combo
                    {
                           iAdjacent_OK = 0;
                    }
                }
             } // end look ahead 

             if (DBGflag)
             {
                sprintf(szBuffer, "   MP%u FRAME %uHz %04u samples len=%d Off=%04d Rem=%04d Prev=x%02X Next=x%02X", 
                                      uMPA_Layer,  uMPA_Sample_Hz, uMPA_Samples,
                                      iMPA_FrameLen,
                                      (lpMpeg_ix3 - lpStart),
                                      (lpMpeg_End_Packet - lpMpeg_ix3),
                                        *lpNextHdr, *lpPrevHdr);
                DBGout(szBuffer) ;
             }

             if (!iAdjacent_OK)  // and is there are nearby header at correct distance
                  lpMpeg_ix3++;   // Not a reliable header - keep looking
             else
             {
               if (!iOut_PTS_Matching)
                   iScanResult = 1; // Stop Looking
               else
               if (uMPA_Sample_Hz)
               {
                   uFrameTime_PTS =  (uMPA_Samples * 90000 / uMPA_Sample_Hz);

                   if (DBGflag)
                   {
                       sprintf(szBuffer, "       FRAME %04ums =%05ut", 
                                              (uFrameTime_PTS/90), uFrameTime_PTS);
                        DBGout(szBuffer) ;
                        sprintf(szBuffer, "        Diff %04ums =%05ut", 
                                        (unsigned int)(i64Diff_PTS/90),
                                        (unsigned int)(i64Diff_PTS));
                        DBGout(szBuffer) ;
                   }

                   // Is this frame entirely ahead of the calculated trigger ?
                   if (uFrameTime_PTS < (unsigned int)(i64Diff_PTS))
                   {
                       i64Diff_PTS  -= uFrameTime_PTS;
                       i64Curr_PTS  += uFrameTime_PTS;
                       // Keep Looking
                       lpMpeg_ix3++;  // lpMpeg_ix3 = lpNextHdr
                       if (DBGflag)
                       { 
                           sprintf(szBuffer, "  SKIP  CALC=%04ums =%05ut", 
                                        (unsigned int)(i64Curr_PTS/90),
                                        (unsigned int)(i64Curr_PTS));
                            DBGout(szBuffer) ;
                       }
                   }
                   else
                   {
                      iScanResult = 1; // Stop Looking
                      uPTS_Accounted[uSubStream_Id] += uFrameTime_PTS;
                   }
               } // END-IF valid sample rate

             } // END-IF look ahead seems ok
           } // END-IF bit combos look OK
        }  // END-IF leading Bits ok
     }  // END-IF  MPA Syncword
  } // ENDWHILE
}








//----------------------------------------------
void Out_Find_AC3_Syncword()
{
  int iOffset;
  unsigned int uChannel_ix, uChannels, uFrameTime_PTS;

  unsigned char *lpAC3_HdrCount, *lpAC3_Offset_Hi, *lpAC3_Offset_Lo; 
  unsigned char *lpAC3_Mode;

unsigned int uAC3Channels[8] =
{
        2,     1,     2,     3,     3,     4,     4,     6
  //  "1+1", "1/0", "2/0", "3/0", "2/1", "3/1", "2/2", "3/2"
}
;



  if (cStream_Id == 0xBD)     // Keep AC3 control area
  {
    lpAC3_HdrCount  = lpMpeg_ix3+1; 
    lpAC3_Offset_Hi = lpMpeg_ix3+2;
    lpAC3_Offset_Lo = lpMpeg_ix3+3;

    // The start of the PS1 packet has a pointer to the first syncword
    iOffset = (*lpAC3_Offset_Hi)*256 + *lpAC3_Offset_Lo;  // +1;

    //*lpAC3_Offset_Hi = 0;  // Reset for aligned pkt
    //*lpAC3_Offset_Lo = 0;  // Reset for aligned pkt

     // Validate it
    if (iOffset < iPkt_Between_Len
    &&  *lpAC3_HdrCount || iOffset != 2)  // BAD Twinhan control info
    {
       lpMpeg_ix3 += iOffset;
    }
  }


  // Try to find an audio syncword
  while ( ! iScanResult)
  {
     if (lpMpeg_ix3 > lpMpeg_End_Packet)
         iScanResult = -1;
     else
     //Syncword is always Byte aligned
     if ((   ( *lpMpeg_ix3      ==0x0B)  
          && (*(lpMpeg_ix3+1)   ==0x77))
 //  ||  (   (*lpMpeg_ix3      ==0x7F)  
 //       && (*(lpMpeg_ix3+1)   ==0xFE))
        )
     {
         if (!iOut_PTS_Matching)
         {
              iScanResult = 1;
         }
         else
         {
           lpAC3_Mode = lpMpeg_ix3+5;
           if (lpAC3_Mode <= lpMpeg_End_Packet)
           {
               uChannel_ix     = ((*lpAC3_Mode)>>5) & 0x07;

               uChannels = uAC3Channels[uChannel_ix];

               uFrameTime_PTS = 8640 / uChannels;
               //  "3/2" => 1440 ticks;
               //  "2/0" => 4320 ticks;
               //  DDPLUS ? ? ? ? ? 


               if (DBGflag)
               {
                   sprintf(szBuffer, "   AC3 FRAME %uch  %04ums =%05ut", 
                                         uChannels, 
                                         uFrameTime_PTS/90),
                                         uFrameTime_PTS;
                   DBGout(szBuffer) ;
                   sprintf(szBuffer, "             Diff %04ums =%05ut", 
                                        (unsigned int)(i64Diff_PTS/90),
                                        (unsigned int)(i64Diff_PTS));
                   DBGout(szBuffer) ;
               }

               // Is this frame entirely ahead of the calculated trigger ?
               if (uFrameTime_PTS < (unsigned int)(i64Diff_PTS))
               {
                   i64Diff_PTS  -= uFrameTime_PTS;
                   i64Curr_PTS  += uFrameTime_PTS;
                   lpMpeg_ix3++; // Keep Looking
                   if (DBGflag)
                   {
                      sprintf(szBuffer, "        CALC=%04ums =%05ut", 
                                        (unsigned int)(i64Curr_PTS/90),
                                        (unsigned int)(i64Curr_PTS));
                      DBGout(szBuffer) ;
                   }
               }
               else
               {
                   iScanResult = 1;
               }
           } // END-IF Inside Packet
         } // end-else searching
     } // end-if frame header
     else
     {
          lpMpeg_ix3++;
     }
  } // ENDWHILE

  if (DBGflag)
      DBGln4("   AC3 Offset=x%04X  =%d  ScanTo=%d,  Result=%d",
                     iOffset, iOffset, 
                     (lpMpeg_ix3-lpMpeg_PES_BeginData-4),
                     iScanResult);
}




//----------------------------------------------

  // HAVE TO THINK ABOUT LPCM - HOW TO MAINTAIN CONTROL INFO AT START

void Out_Find_PCM_Syncword()
{
  int iOffset;

  // unsigned int uChannel_ix, uChannels, uFrameTime_PTS;
  // unsigned char *lpAC3_Mode;

  unsigned char *lpAC3_Offset_Hi, *lpAC3_Offset_Lo;

    //iTmp1 = Get_Byte(); // No of Frames starting in this packet (does not include tail from previous packet, but does include last partial frame in this packet)
    //iPS_Frame_Offset = Get_Short(); // Offset to start of 1st frame for this PTS = first access unit pointer

    //iTmp3 = Get_Byte(); // Flags:
                   //    audio emphasis on-off        1 bit
                   //    audio mute on-off            1 bit
                   //    reserved                     1 bit
                   //    audio frame number           5 bits (within Group of Audio frames).

    //iLPCM_Attr = Get_Byte(); // LPCM Attributes:
                   // quantization word length code   2 bits  0 = 16, 1 = 20, 2 = 24, 3 = reserved
                   // audio sampling frequency        2 bits  (48khz = 0, 96khz = 1)
                   // reserved                        1 bit
                   // number of audio channels - 1    3 bit  (e.g. stereo = 1)

    //uQWord_Len = iLPCM_Attr >>6;
    //uBitsPerSample = (uQWord_Len + 4) * 4;

    //if (iLPCM_Attr & 0x30 == 0x10)
    //{
    //    PCM_SamplingRate = 96000;
    //}
    //else
    //{
    //    PCM_SamplingRate = 48000;
    //}
    //SubStream_CTL[FORMAT_AC3][getbit_AC3_Track].rate = PCM_SamplingRate / 1000;

    //uChannels      = (iLPCM_Attr & 0x03) + 1;
    //SubStream_CTL[FORMAT_AC3][getbit_AC3_Track].uChannel_ix = uChannels;

  iScanResult = 1;

  if (cStream_Id == 0xBD)     // Keep AC3 control area
  {
    lpAC3_Offset_Hi = lpMpeg_ix3+2;
    lpAC3_Offset_Lo = lpMpeg_ix3+3;

    // The start of the PS1 packet has a pointer to the first frame
    iOffset = (*lpAC3_Offset_Hi)*256 + *lpAC3_Offset_Lo;  // +1;

    *lpAC3_Offset_Hi = 0;  // Reset for aligned pkt
    *lpAC3_Offset_Lo = 4;  // Reset for aligned pkt

    if (iOffset < iPkt_Between_Len) // Validate it
    {
       lpMpeg_ix3 += iOffset - 4;
    }
    else
    {
       iScanResult = -1;
    }
  }

    /*

               uFrameTime_PTS = ???? / uChannels;

               // Is this frame entirely ahead of the calculated trigger ?
               if (uFrameTime_PTS < i64Diff_PTS && iOut_PTS_Matching)
               {
                   i64Diff_PTS  -= uFrameTime_PTS;
                   i64Curr_PTS  += uFrameTime_PTS;
                   lpMpeg_ix3++; // Keep Looking
               }
               else
               {
                   iScanResult = 1;
               }
  */

  if (DBGflag)
      DBGln4("   PCM Offset=x%04X  =%d  ScanTo=%d,  Result=%d",
                     iOffset, iOffset, 
                     (lpMpeg_ix3-lpMpeg_PES_BeginData-4),
                     iScanResult);
}





//----------------------------------------------
//  SPLIT A PACKET AT THE START
void Out_Filter_Split_Front(int *P_Split_Ctr)
{
    
  unsigned char *lpMpeg_Pkt,  *lpMpeg_Fudge_ix;
  int iTmp1;
  unsigned int uPkt_Prefix;

  // Calculate packet SPLIT size

  // PES_BGN  = Delete FROM point = start of actual data, AFTER PES Header and substream-id
  // SPLIT_ix = UPTO point = before next MINOR start code (Keep from here onward)

  iPkt_Diff_Len = lpMpeg_SPLIT_ix - lpMpeg_PES_BeginData; // How much to delete ?

  // BUT - Have to keep some control info from start of PES data area
  uPkt_Prefix = uPkt_Hdr_Full_Len; // Keep PES HDR area
  if (cStream_Id == 0xBD)     // Keep AC3 control area
  {
      // Adjust for keeping the PS1 AC3 control prefix area;
      if (uSubStream_Cat == SUB_PCM)
         iTmp1 = 5;   // LPCM has 5 bytes of control info 
      else
         iTmp1 = 4;   // AC3  has 4 bytes of control info

      iPkt_Diff_Len -=iTmp1;
      uPkt_Prefix   +=iTmp1; 
  }

  lpMpeg_Fudge_ix = lpMpeg_PES_BeginData + uPkt_Prefix; // Point to ACTUAL delete FROM point


  if (DBGflag)
  {
      sprintf(szDBGln,"\n   *SPLIT TOP* PktLen=%d PEShdr=%d PKThdr=%d Pfx=%d Split=%d  Fix=x%08X",
                      iPkt_Between_Len, 
                      uPES_Hdr_Len, uPkt_Hdr_Full_Len, uPkt_Prefix,
                      iPkt_Diff_Len, *lpMpeg_PES_BeginData);
      DBGout(szDBGln);
  }

  if (iPkt_Diff_Len < 1) 
  {
     // Cannot handle sync word beginning in previous pkt
     if (iPkt_Diff_Len < 0)
         Out_Split_Hdr_Msg("align");
  }
  else
  {
     (*P_Split_Ctr)++;

     iPkt_NewLen = iPkt_Between_Len - iPkt_Diff_Len
                 - 2; // Convert to PES Data Length

     // Save the PES header area + optional PS1 control area

     memcpy(lpTmp16K, lpMpeg_PKT_Anchor+2, uPkt_Prefix); // Save prefix for Later

     // reposition to packet type code
     lpMpeg_Pkt = lpMpeg_PKT_Anchor - 1; // Packet type is just before packet length field

     // Remove the crud at the start

     // Allow for insufficent space for new packet controls
     if (lpMpeg_Pkt < lpMpeg_Copy_Buffer)  // Maybe problem if recent Rebuf - but probably won't happen...
     {
        ZeroMemory(lpMpeg_Fudge_ix, iPkt_Diff_Len); // Zap the crud;
        if (DBGflag)
        {
           DBGout("SHORT-ZEROIZE");
        }
     }
     else
     {
       Out_COMMIT_PKT(0, 8832); // Commit any PREVIOUS packets 

       if (iPkt_Diff_Len < 7)  // Too short to create a dummy pkt ?
       {
          iTmp1 = iPkt_Diff_Len + 3;  // Allow for creation of extra Pkt Start Code & Length fields
          lpMpeg_Pkt -= 3;

          ZeroMemory(lpMpeg_Pkt, iTmp1); // Zap original packet hdr & prefix;

          lpMpeg_Pkt += iPkt_Diff_Len;
          lpMpeg_FROM = lpMpeg_Pkt;  // Skip the crud 

          if (DBGflag)
          {
             DBGout("SHORT-ZERO-PAD");
          }
       }
       else
       {
          // Create a leading padding packet,
         // iOutPaddingPkts++;
         iTmp1 = iPkt_Diff_Len - 6; // Allow for creation of extra Pkt Start Code & Length fields
         *lpMpeg_Pkt++  = cPADDING_STREAM_ID;
         *lpMpeg_Pkt++  = (unsigned char)(iTmp1 / 256);
         *lpMpeg_Pkt++  = (unsigned char)(iTmp1 & 0xFF);
         *lpMpeg_Pkt    = 0x80; // Flag as Mpeg-2 with no PTS/DTS

          lpMpeg_Pkt   += iTmp1;
       }

       // Optionally suppress the dummy padding packet
       if (cBoringStreamDefaultAct == 'D')
          lpMpeg_FROM = lpMpeg_Pkt;

       // Create a new pkt hdr
       *lpMpeg_Pkt++  = 0x00;
       *lpMpeg_Pkt++  = 0x00;
       *lpMpeg_Pkt++  = 0x01;
       *lpMpeg_Pkt++  = cStream_Id;

       // Reposition to New Pkt length field
       lpMpeg_PKT_Anchor     = lpMpeg_Pkt;

       // Create new length field in new PES prefix area
       iTmp1 = iPkt_NewLen;
       *lpMpeg_Pkt++  = (unsigned char)(iTmp1 / 256);
       *lpMpeg_Pkt++  = (unsigned char)(iTmp1 & 0xFF);
       iPkt_Between_Len  = iPkt_NewLen+2;

       // Restore PES HDR fields
       memcpy(lpMpeg_Pkt, lpTmp16K, uPkt_Prefix); // Restore from prev save

       // Reposition Pkt data ptr
       //lpPESflags     = lpMpeg_Pkt+1;
       //cPESflags      = *lpPESflags;
       lpMpeg_PES_BeginData = lpMpeg_Pkt + uPkt_Hdr_Full_Len;

       if (cStream_Id == 0xBD)     // Fix AC3 control area
       {
          *(UNALIGNED short *)(lpMpeg_PES_BeginData+2) = 0x0100; // Reset the AC3 offset pointer
       }

       if (DBGflag)
       {
          sprintf(szDBGln,"     *NEW*  x%08X",  *(DWORD*)(lpMpeg_PES_BeginData));
          DBGout(szDBGln);
       }

       if (process.iOutUnMux && iOut_UnMux_Fmt != 1)   // Maybe need this more often ?
       {
           lpMpeg_FROM = lpMpeg_PES_BeginData;

           if (uSubStream_Cat == SUB_AC3 // Includes DTS range
           ||  uSubStream_Cat == SUB_DDPLUS)
           {
               lpMpeg_FROM +=4;
           }
           else
           if (uSubStream_Cat == SUB_PCM)
           {
               lpMpeg_FROM +=7;
           }


       }

     }

  }

}



//--------------------------------------------==

void Out_SplitChk_FRONT_Audio()
{
  
  if (cStreamNeedsAligning_FRONT_Flag[uSubStream_Id])
  {
      Out_Split_Find_Any_Audio_Syncword();

      if (iScanResult > 0 && cMpeg_Out_Pkt_ACT != 'D')
      {
          Out_Filter_Split_Front(&iOut_SplitAudio_PrePackets);
          cStreamNeedsAligning_FRONT_Flag[uSubStream_Id] = 0;  
      }
  } // END-IF Needs Aligning at start

}





//----------------------------------------------
//  SPLIT A PACKET AT THE REAR
void Out_Filter_Split_Rear(int *P_Split_Ctr) 
{
  unsigned char *lpMpeg_NEW_ix;
  unsigned char *lpTemp1;

  int iPkt_New_Len;

  // Calculate packet SPLIT size
  //  SPLIT_ix = Delete FROM point = first byte of a minor start code
  //  PKT_Anchor points to packet length field near start of packet
  iPkt_New_Len = lpMpeg_SPLIT_ix - lpMpeg_PKT_Anchor
               - 2;  // Convert to PES Length

  // Point to where new packet needs to begin
  lpMpeg_NEW_ix  = lpMpeg_SPLIT_ix;

  if (DBGflag)
  {
      sprintf(szBuffer,"   *SPLIT TAIL* PktLen=%d HdrLen=%d Split=%d  HDR=x%04X",
                      iPkt_Between_Len, uPES_Hdr_Len,
                                   iPkt_New_Len, // iOut_Align_Video);
                                               *(unsigned*)(lpMpeg_NEW_ix));
      DBGout(szBuffer);
  } 

  // Make sure we are left with enough space for the packet header
  if (iPkt_New_Len <= (signed)(uPkt_Hdr_Full_Len))
      cMpeg_Out_Pkt_ACT = 'D'; //   flag to skip this packet
  else
  {
    (*P_Split_Ctr)++;

    // Modify the OLD packet length, in reversed byte format
      lpTemp1 = (unsigned char*)(&iPkt_New_Len);
    *(lpMpeg_PKT_Anchor)   = *(lpTemp1+1);
    *(lpMpeg_PKT_Anchor+1) =  *lpTemp1;

    if (cStream_Id == 0xBD)     // Fix AC3 control area
    {
        lpTemp1 = lpMpeg_PES_BeginData+1; // skip the SubStream_id
       *lpTemp1++ = 0x00; // Reset the AC3 syncword count
       *lpTemp1++ = 0x00; // Reset the AC3 syncword offset
       *lpTemp1   = 0x00; // Reset the AC3 syncword offset
    }

    // If there is room for a basic packet header
    // then create one

    iPkt_New_Len = lpMpeg_End_Packet - lpMpeg_NEW_ix 
                 - 6; // Convert to PES Data Len
    if (DBGflag)
        DBGln4("   Split=x%06X  End=x%06X PADLen=%d =x%04X",
              (int)(lpMpeg_NEW_ix - lpMpeg_Copy_Buffer),
              (int)(lpMpeg_End_Packet-lpMpeg_Copy_Buffer),
              iPkt_New_Len, iPkt_New_Len);

    if (iPkt_New_Len > 1)
    {
       // Create new PADDING STREAM packet header with no PTS/DTS
       //iOutPaddingPkts++;
       *lpMpeg_NEW_ix++ = 0x00;
       *lpMpeg_NEW_ix++ = 0x00;
       *lpMpeg_NEW_ix++ = 0x01;
       *lpMpeg_NEW_ix++ = cPADDING_STREAM_ID;  // (0xBE)

       // Padding stream PES data length
       *lpMpeg_NEW_ix++ = *((unsigned char*)(&iPkt_New_Len)+1);
       *lpMpeg_NEW_ix++ = *( unsigned char*)(&iPkt_New_Len);

       *lpMpeg_NEW_ix++ = 0x80;  // Mpeg-2 format wih no PTS, etc

       
    } // ENDIF room within packet

    // Fill the rest of the packet with nulls
    while (lpMpeg_NEW_ix <= lpMpeg_End_Packet)  // *SUS*
    {
              *lpMpeg_NEW_ix++ = 0;
    }

  } // ENDELSE Packet length boundary


}


