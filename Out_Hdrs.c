//
//       MPEG OUTPUT PACKET MODULE
//
// Contains the main routines for handling Mpeg HEADERS
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

 
unsigned Mpeg_Mux_Chunk_Rate; // MSBF format - 20,000chunks/sec = 8Mbps



//------------------------------------------------------
int  Out_Fix_Pack_Hdr(BYTE *P_lpPack, int P_Immediate)
{
  int iTotLen;
  unsigned int uTmp3, uTmp4;

  // Check for known bad format pack hdr (short by 1 byte)
  if (iOut_FixPktEdge)
  {
     lpMpeg_ix4 = P_lpPack+11;
     if ( *(UNALIGNED DWORD*)lpMpeg_ix4 == 0xE0010000)
       return 11;
  }

  lpMpeg_ix4 = P_lpPack+4;
  if ((*(lpMpeg_ix4)) & 0x40)  // Mpeg-2 or later ?
  {
      iTotLen = ((*(P_lpPack+13)) & 7 ) + 14; // Allow for stuffing bytes
      if (iOut_Fix_SD_Hdr)
      {
         lpMpeg_ix3 = P_lpPack + 10;
         uTmp3 = *((UNALIGNED unsigned*)(lpMpeg_ix3));
         Mpeg_Mux_Chunk_Rate = (uTmp3 & 0x00FCFFFF); // Don't worry about MSBF, since only testing for zero
         if ( ! Mpeg_Mux_Chunk_Rate || iOut_Fix_SD_Hdr > 127)
         {
             if (iOut_Fix_SD_Hdr > 127)
                uTmp4 = 0x00F04902; // MSBF format - 37500chunks/sec = 15Mbps
             else
                uTmp4 = 0x00803801; // MSBF format - 20000chunks/sec =  8Mbps

             uTmp3 = uTmp3 | uTmp4;
             *((UNALIGNED unsigned*)(lpMpeg_ix3)) = uTmp3;
         }
      }
  }
  else
  {
      iTotLen = 12;                // Mpeg-1 is fixed length
      if (iOut_Fix_SD_Hdr)
      {
         lpMpeg_ix3 = P_lpPack + 9;
         uTmp3 = *((UNALIGNED unsigned*)(lpMpeg_ix3));
         Mpeg_Mux_Chunk_Rate = (uTmp3 & 0x00FEFFEF); // Don't worry about MSBF, since only testing for zero
         if ( ! Mpeg_Mux_Chunk_Rate)
         {
             uTmp3 = uTmp3 | 0x00701700;  // MSBF format - 3,000chunks/sec = 1.2Mbps
             *((UNALIGNED unsigned*)(lpMpeg_ix3)) = uTmp3;
         }
      }
  }

  //memcpy(&Pack_Prev_SSCRM[0], (P_lpPack+4), 6);

  if (iOut_TC_Adjust)
  {
      lpMpeg_PTS_ix = lpMpeg_ix4;
      cStream_Id    = 0xBA; // PACK
      uSubStream_Id = 0xBA;
      cStream_Cat   = 0xBA;
      Out_DeGap_TC(); //&i64Adjust_TC[0xBA][0]); 
  }

   // Copy PACK HDR
  if (P_Immediate)
      Out_RECORD(P_lpPack, iTotLen, 8222);

  return iTotLen;
}



int  Out_Fix_SysHdr(BYTE *P_lpSYS, char P_Type[4], int P_Write)
{
  int iRC, iTotLen, iCalcLen;
  BYTE *lpMpeg_SysLen, *lpMpeg_CHK;

  lpMpeg_ix3 = P_lpSYS + 4;

  // The packet length is 2 bytes, but need to swap the order

  lpMpeg_SysLen = lpMpeg_ix3;

  iPkt_Between_Len  = (*(lpMpeg_ix3++))<<8;
  iPkt_Between_Len |=  *(lpMpeg_ix3++);
  iTotLen = iPkt_Between_Len  + 6;


  if (*((UNALIGNED DWORD*)(P_lpSYS)) != uSYSTEM_START_CODE   // 01bb = SYSTEM_START_CODE
    || iTotLen > 4096)
  {
      sprintf(szBuffer,"*BUG* Wrong Syshdr %02X %02X %02X %02X\n\nLen=%d Type=%s", 
                         *P_lpSYS, *(P_lpSYS+1), *(P_lpSYS+2), *(P_lpSYS+3), 
                                                          iTotLen, P_Type);
      MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2",
                                      MB_ICONSTOP | MB_OK);
      iTotLen = 32;

      if (DBGflag)
          DBGout(szBuffer);
  }

  if (DBGflag)
  {
      sprintf(szBuffer, " SYS HDR Len=%d", iTotLen);
      DBGout(szBuffer);
  }

  // We may need to fix the Max Mux Rate (rate_bound) field for DVD compliance
  if (iOut_Fix_SD_Hdr)
  {
     uTmp3 = *((UNALIGNED unsigned*)(lpMpeg_ix3));
     lpMpeg_TC_ix2 = lpMpeg_ix3;

     Mpeg_Mux_Chunk_Rate  = (int)(((*lpMpeg_TC_ix2++) & 0x7F)<<16);
     Mpeg_Mux_Chunk_Rate |= (int)( (*lpMpeg_TC_ix2++)        <<8);
     Mpeg_Mux_Chunk_Rate |= (int)( (*lpMpeg_TC_ix2++) & 0xFE);
     //uTmp4 = (uTmp3 & 0x00FEFF7F);
     //uSwapFormat(&Mpeg_Mux_Chunk_Rate, &uTmp4, 4);
     Mpeg_Mux_Chunk_Rate = Mpeg_Mux_Chunk_Rate>>1;

     if (DBGflag)
     {
          sprintf(szBuffer, "       MaxMux=%d", Mpeg_Mux_Chunk_Rate);
          DBGout(szBuffer);
     }


     if (iOut_Fix_SD_Hdr > 127)
     {
       if (Mpeg_Mux_Chunk_Rate <= 19999 // (9,999,950 bps)
       ||  Mpeg_Mux_Chunk_Rate < 1)
       {
            //Mpeg_Mux_Chunk_Rate = (Mpeg_Mux_Chunk_Rate/10) + 8000000;
            //uTmp4 = (Mpeg_Mux_Chunk_Rate<<1);
            // FORCE A HIGH RATE
              uTmp3 = (uTmp3 & 0xFF010080)  | 0x00F02401; // 37500<<1 MSBF => 15 Mbps
                    //| uSwapFormat(&uTmp4, &uTmp4, 4);
              *((UNALIGNED unsigned*)(lpMpeg_ix3)) = uTmp3;
       }

       *(lpMpeg_ix3+5) = (unsigned char)(*(lpMpeg_ix3+5)  & 0x7F); // Clr: packet_rate_restriction_flag
       
     }   
     else
     {
       if (Mpeg_Mux_Chunk_Rate > 19999 // (9,999,950 bps)
       ||  Mpeg_Mux_Chunk_Rate < 1)
       {
              // FORCE A LEGAL DVD RATE
              uTmp3 = (uTmp3 & 0xFF010080)  | 0x00409C00;
                    //| uSwapFormat(&uTmp4, &uTmp4, 4);
              *((UNALIGNED unsigned*)(lpMpeg_ix3)) = uTmp3;
       }
     }
     
  } // ENDIF SD HDR fix


  if (iEDL_ctr > 1 && iCtl_Out_SysHdr_Unlock)  // More than one clip ?
  {
         *(lpMpeg_ix3+3) = (unsigned char)(*(lpMpeg_ix3+3)  & 0xFC); // Clr: Fixed_Flag, CSPS_Flag
         *(lpMpeg_ix3+4) = (unsigned char)(*(lpMpeg_ix3+4)  & 0x3F); // Clr: system_audio_lock_flag,  system_video_lock_flag, 
  }

  // Is it a fixed bitrate stream ?
  iFixedRate = ( *(lpMpeg_ix3+3) & 0x02);
    


  iCalcLen = 6;
  lpMpeg_CHK = lpMpeg_SysLen+8;

  while (*lpMpeg_CHK > 0x7F)
  {
    lpMpeg_CHK +=3;
    iCalcLen   +=3;
  }

  // Integrity check - mainly because of BUG circa Jan17 2006 (Build 6117)
  if (iCalcLen != iPkt_Between_Len)
  {

      sprintf(szBuffer, "BAD length in Mpeg Sys Header\n\nGot %d.  Expected %d.\n\nFIX ?",
                                           iPkt_Between_Len, iCalcLen);
      if (DBGflag)
          DBGout(szBuffer);

      if (process.iBadSYSAlerted)
          iRC = process.iBadSYSAlerted;
      else
      if (iPkt_Between_Len == 0x15 || iPkt_Between_Len == 0x0C) // Safety allowance for BUG@6117
          iRC = IDOK;
      else
      {
          iRC = MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2", MB_OKCANCEL);
      }

      if (iRC == IDOK)
      {
         *lpMpeg_SysLen++ = (unsigned char)(iCalcLen /  256);
         *lpMpeg_SysLen   = (unsigned char)(iCalcLen & 0xFF);
          process.iBadSYSAlerted = IDOK;
      }
      else
          process.iBadSYSAlerted = IDOK + 12345;
      
  }

  iCalcLen += 6;

  if (P_Write)
      Out_RECORD(P_lpSYS, iCalcLen, 8223);

  return iCalcLen;
}



//-------------------------------
void Out_PS_File_Hdrs()
{

    const BYTE  MPEG2SysHdr_MPA_SD[]
      = {0x00, 0x00, 0x01, 0xBB, // 01BB sentinel = SYSTEM_START_CODE
         0x00, 0x0C,        // Hdr length (IBM370 format, NOT Intel format)
         0x88, 0x8B, 0x81,  // 1b; 22bit Max_Mux_Rate (rate_bound); 1b
         0x04,  // 6b Audio_bound; 1bit Fixed rate flag;  1bit CSPS flag
         0x21,  // 1bit System_Audio_Lock; 1bit System_Video_Lock; 1bit marker; 0xE1
                //                                          5bit video_bound;
         0x7F,  // 1bit packet_rate_restriction_flag; 7bit RESERVED;

        // stream descriptor list. Entry format=:
           //      8bit stream_id
           //      2bit constant Decimal3=Bin11
           //      1bit P-STD_buffer_bound_scale
           //     13bit P-STD_buffer_size_bound
          0xC0, 0xC0, 0x20,  // Mpeg audio C0 - P-Std=0  size bound =x20 =032
          0xE0, 0xE0, 0xE7}; // Mpeg Video E0 - P-STD=1  size bound =xE7 =231

    const BYTE  MPEG2SysHdr_MPA_HD[]
      = {0x00, 0x00, 0x01, 0xBB, // 01BB sentinel = SYSTEM_START_CODE
         0x00, 0x0C,        // Hdr length (IBM370 format, NOT Intel format)
         0x88, 0x8B, 0x81,  // 1b; 22bit Max_Mux_Rate (rate_bound); 1b
         0x04,  // 6b Audio_bound; 1bit Fixed rate flag;  1bit CSPS flag
         0x21,  // 1bit System_Audio_Lock; 1bit System_Video_Lock; 1bit marker; 0xE1,
                //                                          5bit video_bound;
         0x7F,  // 1bit packet_rate_restriction_flag; 7bit RESERVED;

        // stream descriptor list. Entry format=:
           //      8bit stream_id
           //      2bit constant Decimal3=Bin11
           //      1bit P-STD_buffer_bound_scale
           //     13bit P-STD_buffer_size_bound
         0xC0, 0xC0, 0x80,  // Mpeg audio C0 - P-Std=0  size bound =x20 =032
         0xE0, 0xE0, 0xE7}; // Mpeg Video E0 - P-STD=1  size bound =xE7 =231

    const BYTE  MPEG2SysHdr_VOB[]
      = {0x00, 0x00, 0x01, 0xBB, // 01BB sentinel = SYSTEM_START_CODE
         0x00, 0x0F,        // Hdr length (IBM370 format, NOT Intel format)
         0x88, 0x8B, 0x81,  // 1b; 22bit Max_Mux_Rate (rate_bound); 1b
         0x04,  // 6b Audio_bound; 1bit Fixed rate flag;  1bit CSPS flag
         0x21,  // 1bit System_Audio_Lock; 1bit System_Video_Lock; 1bit marker; 0xE1,  
                //                                          5bit video_bound;
         0x7F,  // 1bit packet_rate_restriction_flag; 7bit RESERVED;

        // stream descriptor list. Entry format=:
           //      8bit stream_id
           //      2bit constant Decimal3=Bin11
           //      1bit P-STD_buffer_bound_scale
           //     13bit P-STD_buffer_size_bound         0xB9, 0xE0, 0xE8,  // B9 flag. P-STD applies to all VIDEO stream_ids
         0xB8, 0xC0, 0x20,  // B8 flag. P-STD applies to all AUDIO stream_ids
         0xBD, 0xE0, 0x3A,  // BD stream = Private Stream 1
         0xBF, 0xE0, 0x02}; // BF stream = Private Stream 2

/*    const BYTE  MPEG2SysHdr[]
    =  {0x00, 0x00, 0x01, 0xBB, // 01BB sentinel = SYSTEM_START_CODE
          00, 0x0C,        // Hdr length (IBM370 format, NOT Intel format)
        0x88, 0x8B, 0x81,  // 1b; 22bit Max_Mux_Rate (rate_bound); 1b
        0x05,  // 6b Audio_bound; 1bit Fixed rate flag;  1bit CSPS flag
        0xE1,  // 1bit System_Audio_Lock; 1bit System_Video_Lock; 1bit marker;
               //                                          5bit video_bound;
        0xFF,  // 1bit packet_rate_restriction_flag; 7bit RESERVED;
        0xC0,  0xC0, 0x80, 0xE0, 0xE0, 0xE6};
           // stream descriptor list. Entry format=:
           //      8bit stream_id
           //      2bit constant Decimal3=Bin11
           //      1bit P-STD_buffer_bound_scale
           //     13bit P-STD_buffer_size_bound
*/

//BYTE *lpMpeg_Source;

     char szTmp8[8];

     int iPack1_OK;

 int iBalanceBytes, iPacketLen, iRC;

 iBalanceBytes = 0;
 iMpeg_Out_Offset = 0;

 cStream_Id    = cPACK_START_CODE;    
 uSubStream_Id = cPACK_START_CODE;

 // Remember first SCR for later time stamp adjustment

 lpMpeg_TC_ix2 = (BYTE*)(&Pack_From_SSCRM[4]);
 SCRM_2SCR((BYTE*)(&i64Adjust_TC[0xBA][0]));  // May not need these lines anymore

 if (DBGflag)
     DBGln4(" FROM PACK SCR= 0x%08X =%d ms  =%u Ticks",
                 *(__int64*)(&Pack_From_SSCRM[4]),
                 (i64Adjust_TC[0xBA][0]/90), i64Adjust_TC[0xBA][0], 0x00);


  // Integrity Test - Output Program Stream MUST start with Pack Hdr
  if  (*( (UNALIGNED DWORD*) (lpMpeg_FROM) ) != dwStartSentinel)
  {
     // Create a PS pack header if none available
     iPack1_OK = 0;
     // Try the one in the preamble
     if (process.Preamble_PackHdr_Found)
     {
          if (DBGflag)
              DBGout(" PREAMBLE PACK HDR CLONED");

          iPacketLen = Out_Fix_Pack_Hdr(process.Preamble_PackHdr, 1);
          iBalanceBytes += iPacketLen;
     }
     else
     {  // INVENT PACK HEADER
          if (DBGflag)
              DBGout(" CLONE PACK HDR");
         iPacketLen = Out_Fix_Pack_Hdr(&Pack_From_SSCRM[0], 1);
         iBalanceBytes += iPacketLen;
         /*
         if (Mpeg_PES_Version == 2)
         {
             Out_RECORD(&Pack_From_SSCRM[0], 10 , 8231);
             Out_RECORD(&MPEG2PackHdr[10], (sizeof(MPEG2PackHdr)-10), 8232);
             iBalanceBytes += sizeof(MPEG2PackHdr);
         }
         else
         {
             Out_RECORD(&Pack_From_SSCRM[0], 9 , 8231);
             Out_RECORD(&MPEG1PackHdr[10], (sizeof(MPEG1PackHdr)-9), 8232);
             iBalanceBytes += sizeof(MPEG1PackHdr);
         }
         */
     }

     // Was the problem external  or internal ?
     if ( ! i64_CurrPos && ! iCurrFile )
         strcpy(szTmp8, "INPUT");
     else
         strcpy(szTmp8, "BUG ! ");

     iTmp1 = (int)(i64_CurrPos);

     if (iCtl_WarnNoHdr)
     {
        sprintf(szBuffer,"%s %s HEADER NOT FOUND.\n\nCreated dummy header\n\nLOC=%x, File=%x",
                      szTmp8, szStartDesc,        iTmp1, iCurrFile);
                   //if (DBGflag) DBGout(szBuffer);
        //MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2",
        //                                MB_ICONSTOP | MB_OKCANCEL);
        iRC = Warning_Box(&szBuffer[0], 0, &iCtl_WarnNoHdr, 0, MB_OKCANCEL);
        if (iRC == IDCANCEL)
            Out_CanFlag = 'C';
     }

  }
  else
  {
      // We have a pack header
      iPack1_OK = 1;
      // uMpeg_PackHdrAdj = ????     <==== Merge this code with FILTER section

      // We may need to adjust it

      //if (iCtl_Out_Preamble_Flag==1)  // This memcpy may be obsolete with the reworking of MIN Preamble process
      //   memcpy( (lpMpeg_FROM+4), &Pack_From_SSCRM[4], 6); // Fix SCR Time

      // OLD Calculate offset where PS sys header needs to be
      // iMpeg_Out_Offset = 14 + (lpMpeg_FROM[13] & 7);

      iMpeg_Out_Offset = Out_Fix_Pack_Hdr(lpMpeg_FROM, 1);

          if (DBGflag)
          {
              sprintf(szBuffer, "PACK HDR Len=%d", iMpeg_Out_Offset);
              DBGout(szBuffer);
          }


  }



  // Point to where PS sys header needs to be
  lpMpeg_FROM = lpMpeg_FROM+iMpeg_Out_Offset;  // point to start code
  iMpeg_Out_Offset = 0;

  // Check if input has a PS header
  if (*((UNALIGNED DWORD*)(lpMpeg_FROM)) == uSYSTEM_START_CODE) // 0xBB010000) // 01bb = SYSTEM_START_CODE
  {
      iMpeg_Out_Offset = Out_Fix_SysHdr(lpMpeg_FROM, "CLP", 1);
      lpMpeg_FROM = lpMpeg_FROM + iMpeg_Out_Offset;  // point to next start code
      iMpeg_Out_Offset = 0;
  } // ENDIF SYSHDR found

  else
  {
    // Try the one in the preamble
    if (process.Preamble_SysHdr_Found && iCtl_Out_Preamble_Flag > 0)
    {
          if (DBGflag)
          {   
              DBGout("PREAMBLE SYS HDR CLONED");
          }

        iPacketLen = Out_Fix_SysHdr(&process.Preamble_SysHdr[0], "Pre", 1);
        iBalanceBytes += iPacketLen;
    }
    else
    {
      // CREATE A PS SYSTEM HEADER

          if (DBGflag)
          {   
              DBGout("DUMMY SYS HDR");
          }



      if (iCtl_Out_SysHdr_Mpeg && !process.iAudio_PS1_Found)
      {
          // Mpeg Video comes in different varieties
  
          if (Coded_Pic_Height <= 576)
          {
             Out_RECORD(&MPEG2SysHdr_MPA_SD[0],
                         sizeof(MPEG2SysHdr_MPA_SD), 8003);
          }
          else
          {
             Out_RECORD(&MPEG2SysHdr_MPA_HD[0],
                         sizeof(MPEG2SysHdr_MPA_HD), 8003);
          }

          iBalanceBytes += sizeof(MPEG2SysHdr_MPA_SD);
      }
      else
      {
           Out_RECORD(&MPEG2SysHdr_VOB[0],
                         sizeof(MPEG2SysHdr_VOB), 8003);

           iBalanceBytes += sizeof(MPEG2SysHdr_VOB);
      }




          //RJDBG
          /*    if (DBGflag)
              {
                  char szTmp1[256];
                  iTmp1 = (int)(i64_CurrPos);
                  sprintf(szTmp1, "\nOUT-SYS HDR CREATED: offset=x%X, Pos=x%X\n",
                                    iMpeg_Out_Offset, iTmp1);
                  DBGout(szTmp1);
              }
          */
    }
  } // ENDIF PS Pack Hdr


  // May need to pad first pack up to 2k

  if (iOutVOB && MParse.iVOB_Style && iBalanceBytes && iPack1_OK )
  {
     //  THIS NEEDS TO BE SMARTER
     // - NEEDS TO CHECK IF A PACK HEADER IS COMING NEXT

      iTmp8 = 2048 - iBalanceBytes;
      Out_Padme(iTmp8);
  }

}


//------------------------------------------------------


//--------------------------
void Out_Fix_Hdr_Vid_SEQ()
{
  unsigned uTmp1;
  unsigned uWidth, uHeight;
  int iRC2;
  unsigned char *lpMpeg_TMP;

  if (iOut_Target_Tail)
       iOut_AddedVideoSEQs++;

  // Check Canvas size - may be corrupted   

  if (iCtl_Out_Fix_Errors                // FixErrors
  &&  (*(UNALIGNED DWORD*)(lpMpeg_ix3) !=  dwOrgCanvas))
  {
     lpMpeg_TMP = lpMpeg_ix3;

     uWidth   = (*lpMpeg_TMP++          *  16) ;
     uWidth  += (*lpMpeg_TMP  / 16);

     uHeight  = (((*lpMpeg_TMP++) & 0x0F) * 256) ;
     uHeight += (  *lpMpeg_TMP      );

    
     if (    !(uWidth  & 7)     // Evenly Divisible by 8
          && !(uHeight & 7)     // Evenly Divisible by 8
     &&  uWidth && uHeight)
     {
         iRC2 = IDNO;
     }
     else
     {
        sprintf(szBuffer, "Bad Picture Dimensions: %u x %u pixels\n\n  Change size to: %u x %u ?\n\n\n\n\n[ Deep=%d SCPart=%d x%02X Pack#%03d Pkt#%03d ]",
                         uWidth, uHeight, uOrgWidth, uOrgHeight, 
                         iDeepNow, iStartCodePart[uSubStream_Id],
                         uHdrType, iPack_Ctr, iOut_CheckedPackets);
        iRC2 = MessageBox(hWnd_MAIN, szBuffer,  "Mpg2Cut2 - CONFIRM", MB_YESNOCANCEL);
        if (DBGflag)
            DBGout(szBuffer);
     }

     switch (iRC2)
     {
         case IDOK: 
         case IDYES: 
             *(UNALIGNED DWORD*)(lpMpeg_ix3) = dwOrgCanvas;
              break;

         case IDNO:
              dwOrgCanvas = *(DWORD*)(lpMpeg_ix3);
              uOrgWidth   = uWidth;
              uOrgHeight  = uHeight;
              break;

         case IDCANCEL: 
              iOut_Error = 8;
              Out_CanFlag = 'C' ;
              break;
     } // ENDSWITCH

  } // ENDIF FIX
  



  if (iOut_Fix_Aspect)
  {
     lpMpeg_TMP = lpMpeg_ix3 + 3;
     uTmp1 = *lpMpeg_TMP & 0x0F;
     *lpMpeg_TMP = (BYTE)(((iView_Aspect_Mode + 1) <<4) | uTmp1);
  }


  if (iOut_Fix_Frame_Rate)
  {
     lpMpeg_TMP = lpMpeg_ix3 + 3;
     uTmp1 = *lpMpeg_TMP & 0xF0;
     *lpMpeg_TMP = (BYTE)((iOutFrameRateCode)  | uTmp1);
  }




  if (iOut_Fix_SD_Hdr)
  {
    // Check bit rate

    lpMpeg_TMP = lpMpeg_ix3 + 4;

#ifdef DBG_FULL
    if (DBGflag)
    {
          sprintf(szBuffer, "    BitRate=%08X", *(UNALIGNED DWORD*)(lpMpeg_TMP) );
          DBGout(szBuffer);
    }
#endif

    if (iOut_Fix_SD_Hdr > 127)
    {
       if (*(lpMpeg_TMP)   < 0x17) // c. 9.4 Mbps
       {
           *(lpMpeg_TMP++) = 0x20;
           //*(lpMpeg_TMP++) = 0x00;

#ifdef DBG_FULL
           if (DBGflag)
           {
              sprintf(szBuffer, "      FIXED=%08X", *(UNALIGNED DWORD*)(lpMpeg_TMP-2) );
              DBGout(szBuffer) ;
           }
#endif
       }
    }
    else
    if (*(lpMpeg_TMP)   > 0x17) // c. 9.4 Mbps
    {
        *(lpMpeg_TMP++) = 0x17;
        *(lpMpeg_TMP++) = 0x00;

#ifdef DBG_FULL
        if (DBGflag)
        {
           sprintf(szBuffer, "      FIXED=%08X", *(UNALIGNED DWORD*)(lpMpeg_TMP-2) );
           DBGout(szBuffer) ;
        }
#endif
    }
  }


}








//--------------------------
void Out_Fix_Hdr_PIC_EXTN_SEQ()
{
  unsigned int uTmp1, uTmp2;
  unsigned char *lpFIX;
  //int iRC2;


  if (iCtl_Out_Force_Interlace)
  {
     lpFIX =  lpMpeg_ix3 + 4;  
     uTmp1 = *lpFIX;
     uTmp2 =  uTmp1 & 0x80; // Progressive bit set ?
     if (uTmp2)
     {
       *lpFIX = (unsigned char)(uTmp1 & 0x7F); // Clear the Progressive Display bit
       iOut_Force_Interlace_ctr++;
     }

  }


}








//--------------------------
void Out_Fix_Hdr_Vid_EXTN_SEQ()
{
  unsigned uTmp1, uTmp2;
  unsigned char *lpFIX;
  int iRC2;

  // Set as NonStd ?
  
  if (iOut_Fix_SD_Hdr > 127)
  {
      //*lpMpeg_ix3 = *lpMpeg_ix3   | 0x08; // Ensure "Escape bit" is set
      lpFIX = lpMpeg_ix3; 
     *lpFIX = (unsigned char)(((*lpFIX) & 0x0F) | 0x40); // "High" level of same profile
  }


  if (iOut_Fix_Chroma_Fmt)
  {
     lpFIX =  lpMpeg_ix3 + 1;   // 2
     uTmp1 = *lpFIX;
     uTmp2 =  uTmp1 & 0x06; 

     if (uTmp2 == 0)
     {
       if (iOut_Fix_Chroma_Fmt > 1)
       {
         iRC2 = IDYES;
       }
       else
       {
         iRC2 = MessageBox(hWnd_MAIN, "Bad Chroma Format: 0\n\n Change to normal: 4:2:0 ?\n",
               "Mpg2Cut2 - CONFIRM", MB_YESNOCANCEL);
       }

       switch (iRC2)
       {
         case IDOK: 
         case IDYES: 
             *lpFIX = (unsigned char)(uTmp2 + 0x02); // Force to 4:2:0
              iOut_Fix_Chroma_Fmt = 2;
              break;

         case IDNO:
              iOut_Fix_Chroma_Fmt = 0;
              break;

         case IDCANCEL: 
              iOut_Error = 8;
              Out_CanFlag = 'C' ;
              break;
       } // ENDSWITCH

     } // ENDIF Bad Chroma_Format
  }
  

  if (iCtl_Out_Force_Interlace)
  {
     lpFIX =  lpMpeg_ix3 + 1;  // 2
     uTmp1 = *lpFIX;
     uTmp2 =  uTmp1 & 0x08; // Progressive bit set ?
     if (uTmp2)
     {
        *lpFIX = (unsigned char)(uTmp1 & 0xF7); // Clear the Progressive Display bit
         iOut_Force_Interlace_ctr++;
     }

  }


  if (iOutFRatioCode > 0)
  {
     lpFIX =  lpMpeg_ix3 + 5;
     uTmp1 = *lpFIX      & 0x80;
    *lpFIX =  (BYTE)((iOutFRatioCode)  | uTmp1);
  }

}



//-------------------------------------------------------


unsigned int uSentinel;

void  Out_PES_Hdr_Mpeg1()
{

  uPkt_Hdr_Full_Len = 0;

  // TEST MOVEd into main filter
  // if (cStream_Id == 0xBE  // Padding Packet 
  // ||  cStream_Id == 0xBF  // Private Stream 2 - DVD NAV PACK
  // {
  //     uPES_Hdr_Len     = 0;
  // }
  // else
  {
    while (uSentinel >= 0x80) // Mpeg-1 stuffing bytes
    {
           //if (DBGflag)
           //    DBGout("Mpeg-1 PES HDR Padding Byte skipped");

           uSentinel = *++lpMpeg_ix3;
           uPkt_Hdr_Full_Len++;
           //iPkt_Len_Remain--;
    }

    if (uSentinel >= 0x40) // Mpeg-1 STD Buffer
    {
         //  if (DBGflag)
         //      DBGout("\nMpeg-1 PES HDR STD Buffer - 2 bytes skipped");

        lpMpeg_ix3   +=2;
        uSentinel     = *lpMpeg_ix3;
        uPkt_Hdr_Full_Len +=2;
      //iPkt_Len_Remain -=2;
    }

    if (uSentinel >= 0x30) // Mpeg-1 PTS + DTS
    {
           if (DBGflag)
               DBGout("\nMpeg-1 PES HDR PTS + DTS");

        uPES_Hdr_Len = 10;
    }
    else
    if (uSentinel >= 0x20)// Mpeg-1 PTS 
    {
           if (DBGflag)
               DBGout("\nMpeg-1 PES HDR PTS only");

        uPES_Hdr_Len = 5;
    }
    else
        uPES_Hdr_Len = 1;

    cPES_Field_Flags = (unsigned char)uSentinel;   // PES Field flags
  }
    
  uPkt_Hdr_Full_Len += uPES_Hdr_Len;

           if (DBGflag)
           {
               sprintf(szBuffer, "\nMpeg-1 PES HdrLen = %02d   PKT HdrLen=%d",
                                   uPES_Hdr_Len, uPkt_Hdr_Full_Len);
               DBGout(szBuffer) ;
           }

  lpMpeg_PES_BeginData = lpMpeg_ix3 + uPES_Hdr_Len;
  lpMpeg_PTS_ix        = lpMpeg_ix3;

  
}



