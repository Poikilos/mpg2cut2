//
//       MPEG OUTPUT PACKET MODULE
//
// Contains the main routines for handling the big output buffer 
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


//-----------------------------------------------------------
int Mpeg_BIG_READ(BYTE *lpP_Into, const int P_Caller)   // BigRead
{
  int iERR, iAnswer, iTmp1, iContigBad;
  int iMpeg_BufferRemaining_Len;
  int iTime1, iTime2, iTimeDiff; //, iTimeHurdle;;
  __int64 i64RC, i64_Big_ToDo, i64_Before_Pos, i64_New, i64Skip;

  iContigBad = 0;

  i64_Before_Pos = _telli64(FileDCB[iCurrFile]);

resume:
  // How much data left in current range on this file ?
  i64_Big_ToDo = i64_CurrEnd - i64_CurrPos;


  // Convert to integer and apply limits

  if (i64_Big_ToDo   > K_8MB )
      iMpeg_ToDo_Len = K_8MB;  
  else
      iMpeg_ToDo_Len = (int)(i64_Big_ToDo);


  // What is max size of buffer ?
  if (iCtl_Priority[2] == PRIORITY_LOW &&  ! iRange_FirstBlk)
  {
      iMpeg_Copy_BufLimit = iMpeg_Copy_BufSz/16;
      if (iMpeg_Copy_BufLimit < 65536)
          iMpeg_Copy_BufLimit = 65536;
  }
  else
      iMpeg_Copy_BufLimit = iMpeg_Copy_BufSz;

  // Sometimes there is unprocessed data at start of buffer
  iMpeg_BufferRemaining_Len = iMpeg_Copy_BufLimit
                            - (lpP_Into - lpMpeg_Copy_Buffer);

  // Take the smaller of the two figures
  if (iMpeg_ToDo_Len > iMpeg_BufferRemaining_Len)
  {
      iMpeg_ToDo_Len = iMpeg_BufferRemaining_Len;
      
      // Trim to end on a 32k cluster boundary
      // so that subsequent reads will be aligned.

      if (iMpeg_ToDo_Len > 0x8000)
      {
         iTmp1 = (int)(i64_Before_Pos) & 0x7FFF;
         iMpeg_ToDo_Len -= iTmp1;
      }

  }

  iTime1 = iCURR_TIME_ms();

  // read a BIG block of data
  iMpeg_Read_Len = _read(FileDCB[iCurrFile], lpP_Into,
                                             iMpeg_ToDo_Len);
  iErrNo = errno;
  iTime2 = iCURR_TIME_ms();

  if (DBGflag)
  {
      sprintf(szBuffer, "BigRead Got=x%08X  Wanted=x%08X",
                                iMpeg_Read_Len,  iMpeg_ToDo_Len);
      DBGout(szBuffer);
  }

  iTimeDiff = iTime2 - iTime1;

  if (iOut_Clip_ctr)
  {

    if (iTimeDiff
        && i64_CurrCopied > 1024000
        && ((iMpeg_ToDo_Len / iTimeDiff) < 300
       ))
    {
      if  (iMsgTime == MAXINT31)  // MAXINT)    // (! iMsgLife)
      {
          strcpy(szMsgTxt,"DISK SLOW - INPUT"); // Maybe failing Hard Drive OR drive had powered down since last use OR Fragmentation OR Competing Task 
          DSP1_Main_MSG(0,0);
          UpdateWindow(hWnd_MAIN);
      }
      iMsgTime = iTime2;
    }
    else
    {
        //iMsgLife--;
        if  ((iTime2 - iMsgTime) >= 3000)    // (! iMsgLife)
        {
           DSP_Msg_Clear();
           DSP2_Main_SEL_INFO(0);
           iMsgTime = MAXINT31;  // MAXINT
        } 
    }
  }


  if (iMpeg_Read_Len > 0)
  {
      lpMpeg_EOD       = lpP_Into + iMpeg_Read_Len;
      ZeroMemory(lpMpeg_EOD, 256);

      i64_CurrPos     += iMpeg_Read_Len;

      // Allow a cushion to reduce bounds checking overhead
      //if (iOut_TC_Adjust)
      //    lpMpeg_EOI = lpMpeg_EOD - 256; // More sensitive when scanning all packs
      //else
            lpMpeg_EOI = lpMpeg_EOD - CPY_EOI_CUSHION; 
  }


  // alert on bad read
  iERR = 0;

  if (iMpeg_Read_Len == iMpeg_ToDo_Len) 
      iContigBad = 0;
  else
  {
     iIn_Errors++;
     sprintf(szBuffer,"READ ERR #%d", iIn_Errors);                
     SetDlgItemText(hProgress, IDP_PROGRESS_ETA, szBuffer);

     if (iIn_AutoResume > 0 && iMpeg_Read_Len > 0)
     {
       iIn_AutoResume--;
       iAnswer  = IDOK;
     }
     else
     {
        iMpeg_Diff_Len =  iMpeg_ToDo_Len - iMpeg_Read_Len ;
        sprintf(szBuffer,
           "*** DISK READ ERROR ***\n\nStatus %d for %d\nWant: %d  Got: %d  Diff: %d\nREADING %s\n\nPos=x%08X %08X\nLen=x%08X %08X\nEOD=x%08X\nEOI=x%08X\n",
                          iErrNo, P_Caller, 
                          iMpeg_ToDo_Len, iMpeg_Read_Len, iMpeg_Diff_Len,
                          File_Name[iCurrFile], 
                          (i64_CurrPos),  (process.length[iCurrFile]),
                          (lpMpeg_EOD-lpMpeg_Copy_Buffer), 
                          (lpMpeg_EOI-lpMpeg_Copy_Buffer));

        if (DBGflag)  DBGout(szBuffer) ;

        iAnswer = MessageBox(hWnd_MAIN, szBuffer,
                         "Mpg2Cut2 - FILE ERROR",
                                        MB_ICONSTOP      | MB_OKCANCEL
                                      | MB_SETFOREGROUND | MB_TOPMOST);
       if (iAnswer  == IDOK)
           iIn_AutoResume = 30;
     }


     if (iMpeg_Read_Len > 0)
     {
             lpP_Into       += iMpeg_Read_Len;
             iMpeg_ToDo_Len -= iMpeg_Read_Len;
             i64_Big_ToDo   -= iMpeg_Read_Len;
             i64_Before_Pos += iMpeg_Read_Len;
     }
               
     // SKIP to start of next recovery block (32k = 0x8000)
     // or further if a lot of contiguouous errors
     if ( iContigBad < 2)
          i64Skip =  32*1024;
     else
     if ( iContigBad < 4)
          i64Skip =  64*1024;
     else
     if ( iContigBad < 8)
          i64Skip = 128*1024;
     else
     if ( iContigBad < 9)
          i64Skip = 256*1024;
     else
          i64Skip = 512*1024;

     i64_New = (i64_Before_Pos + i64Skip) & 0xFFFFFFFFFFFF8000;
     iTmp1   = (int)(i64_New -  i64_Before_Pos);
     iMpeg_ToDo_Len -= iTmp1;

     
     // Allow for loss part way through a packet
     // creating a bunch of zeros to replace the missing packet

     if (!iContigBad)
     {
       if (iTmp1 > 8192) // Majority of packets are way smaller than 8k
           iTmp1 = 8192; // So don't need to overdo the zeros.
       ZeroMemory(lpP_Into, iTmp1);
       lpP_Into += iTmp1;
     }
     iContigBad++;

     i64_CurrPos = i64_Before_Pos = i64_New;
     i64RC = _lseeki64(FileDCB[iCurrFile], i64_Before_Pos, SEEK_SET);

     if (iAnswer  == IDOK)
     {
         if (iMpeg_ToDo_Len > 0)
         {
             if (iMpeg_Read_Len < 1)
                 iIn_AutoResume = 0;

             goto resume;
         }
     }

     iERR = -1;

  }


  lpMpeg_FROM      = lpMpeg_Copy_Buffer;
  iMpeg_Out_Offset = 0 ;

  if (iERR)
     Out_CanFlag = 1;

  return iERR;
}



//-----------------------------------------------------------


//-----------------------------------------------------------------
// move the unused portion of current buffer to the start of buffer
// then fill remainder the buffer after that
int  Out_REBUF(BYTE *P_Curr_ix, int P_Caller)
{
  int iRC;
  BYTE *lpMpeg_Gzinta;

  lpMpeg_Gzinta  = lpMpeg_Copy_Buffer; // May need to reload buffer
  iOverflow_Len  = lpMpeg_EOD - P_Curr_ix; 
  if (iOverflow_Len > 0)
  {
     memcpy(lpMpeg_Copy_Buffer, P_Curr_ix, iOverflow_Len);
     lpMpeg_Gzinta += iOverflow_Len; // Adjust reload slot
  }

  if (DBGflag)
  {
      sprintf(szDBGln,"*REBUF* P%d  PktPtr=x%08X, Shuffle=%d", 
                          P_Caller, P_Curr_ix, iOverflow_Len);
      DBGout(szDBGln);
      DBGln2("        Filepos=x%08X, End=x%08X",
                               (i64_CurrPos), (i64_CurrEnd)); 

      sprintf(szDBGln,"old=%08X %08X %08X %08X\nnew=                  %08X %08X", 
                          *(DWORD*)(P_Curr_ix-8),  *(DWORD*)(P_Curr_ix-4),  
                          *(DWORD*)P_Curr_ix,      *(DWORD*)(P_Curr_ix+4),
                          *(DWORD*)lpMpeg_Copy_Buffer, *(DWORD*)(lpMpeg_Copy_Buffer+4));
      DBGout(szDBGln);
  }

  // read a BIG block of data

  iRC = Mpeg_BIG_READ(lpMpeg_Gzinta, (-P_Caller));

  return iRC;
}



//-------------------------------------------------------------
// ES: Copy or skip CURRENT packet
// PS: Accumulate or Copy UP TO current packet
//     optionally DELETE or HIDE current pack

// P_Full=1 commits up to END of current packet
// P_Full=0 commits before START of current packet

void  Out_COMMIT_PKT(int P_Full, const int P_Caller)
{
  BYTE *lpNEXT_Pkt, *lpUPTO_Pkt, *lpCushioning;
  BYTE uPESFlags;
  int iRC, iDefer_Flag;
  int iMpeg_Out_CommitLen;

  // Experimental fix for Ch.7 - May need to edit the packet header
  if (iKill_PTS_Flag && P_Full)
  {
     // Clear the PTS/DTS presence indicators
     uPESFlags = (unsigned char)(cPES_Field_Flags & 0x3F);
     if (uPESFlags)
     {
       // TODO:-
       //   Save a copy of the PTS/DTS info
       //   Move the other hdr fields up
       //   copy the PTS/DTS into the remaining area
     }
     else
       *(lpMpeg_PKT_Anchor+3) = uPESFlags;

     iKill_PTS_Flag = 0;
  }


  lpNEXT_Pkt = lpMpeg_PKT_Anchor + iPkt_Between_Len;


  // Calc cumulative length of data up to the appropriate start code
  //                                        (either current or next)

  if (P_Full)
  {
      // NORMAL = up to END of current packet
      lpUPTO_Pkt = lpNEXT_Pkt;
  }
  else
  {
      // SHORT = up to START of current packet
      lpUPTO_Pkt = lpMpeg_PKT_Anchor - 4; 
  }

  iMpeg_Out_CommitLen = lpUPTO_Pkt - lpMpeg_FROM; 

  iDefer_Flag = 0;


  // Deletion handling is different

  if (cMpeg_Out_Pkt_ACT == 'D' && P_Full)   // Delete requested ?
  {
    if (process.iOutUnMux)
    {
        iMpeg_Out_CommitLen = 0;
    }
    else
    if ((iOutVOB  && MParse.iVOB_Style)  // Maintain Pack size ?
    ||  iOut_HideAudio)
    {
       if (cStream_Id != cPACK_START_CODE)  // Don't kill PACK hdr
       {
          *(lpMpeg_PKT_Anchor-1)   = cPADDING_STREAM_ID;    // HIDE THIS PACKET

          if (cPut_Stream_Id == cPACK_START_CODE)
              iDefer_Flag = 1;
          if (DBGflag)
          {
              sprintf(szDBGln, "  *PKT => PAD   Defer=%d", iDefer_Flag);
              DBGout(szDBGln);
          }
          //iOutPaddingPkts++;
       }
    }
    else
    {   // NON-VOB means REAL deletes
        // so want to copy PREVIOUS Packet UP TO Current Packet Start Code

        // Reset Copy length for PREVIOUS packet
        iMpeg_Out_CommitLen =  lpMpeg_PKT_Anchor - 4 - lpMpeg_FROM;

        if (DBGflag)
        {
            sprintf(szDBGln, "*EXCLUDED* %s Stream=%02X", 
                                      szFTarget, cStream_Id);
        }
    }
  }  // END-IF DELETE PKT


  iOverflow_Len = lpMpeg_FROM + iMpeg_Out_CommitLen - lpMpeg_EOD;

  if (DBGflag 
#ifdef DBG_FULL
    && iOverflow_Len > -32768
#endif
    )
  {
      sprintf(szBuffer,"**COMMIT P%d ACT=%c-%d x%02X Len=x%d, From=x%06X, UpTo=x%06X %08X   Curr@x%06X x%02X",
               P_Caller,  
               cMpeg_Out_Pkt_ACT, P_Full, 
               uSubStream_Id,
                             iMpeg_Out_CommitLen,
                             (int)(lpMpeg_FROM-lpMpeg_Copy_Buffer),
                                 (int)(lpUPTO_Pkt-lpMpeg_Copy_Buffer),
                                 *(unsigned int*)lpUPTO_Pkt, 
                                     (int)(lpMpeg_PKT_Anchor-lpMpeg_Copy_Buffer),
                                     *lpMpeg_PKT_Anchor);
      DBGout(szBuffer);
  }



  if ( !iDefer_Flag)
  {
     if (iMpeg_Out_CommitLen > 0)
     {
         if (iOverflow_Len > 0)
             iMpeg_Out_CommitLen -= iOverflow_Len;

         if (iMpeg_Out_CommitLen > 0 )
             Out_RECORD(lpMpeg_FROM, iMpeg_Out_CommitLen, 8424); // write partial

         if (DBGflag && iOverflow_Len > -32768)
             DBGln4("   Committed Len=%d Overflow=%d  Currpos=x%08X, CurEnd=x%08X",
                  iMpeg_Out_CommitLen,  iOverflow_Len, 
                                    (int)(i64_CurrPos), (int)(i64_CurrEnd));

         // We may need to reload the big buffer
         if (process.iOutParseMore && !Out_CanFlag
         && i64_CurrPos < i64_CurrEnd)
         {
            // Is the last packet split across buffers ?
            if (iOverflow_Len > 0)
            {
                  if (DBGflag)
                  {
                      DBGout("   *COMMIT CROSSING*");
                  }

                  // read a BIG block of data
                  iRC = Mpeg_BIG_READ(lpMpeg_Copy_Buffer, P_Caller);
                  if (! iRC)
                  {
                     Out_RECORD(lpMpeg_Copy_Buffer, iOverflow_Len, 8427);
                     lpUPTO_Pkt = lpMpeg_Copy_Buffer + iOverflow_Len;
                     if (P_Full)
                     {
                        lpNEXT_Pkt = lpUPTO_Pkt;
                     }
                     else
                     {
                        lpMpeg_PKT_Anchor = lpUPTO_Pkt + 4;
                        lpNEXT_Pkt = lpMpeg_PKT_Anchor + iPkt_Between_Len;
                     }
                  }
            }
            else
            {  // Packet was totally within current buffer
               // so it has been written in full.
               iOverflow_Len = 0;

               // BUT - May need to shuffle data from the safety cushion

               // When current packet not comitted
               // Allow deeper penetration of the cushion
               if (P_Full)
                   lpCushioning = lpMpeg_EOI;
               else
                   lpCushioning = (BYTE *)(((unsigned int)lpMpeg_EOI
                                           +(unsigned int)lpMpeg_EOD) / 2);


               if (lpUPTO_Pkt >= lpCushioning
               &&  !iOut_FixPktEdge) // special case - delay till later
               {
                  if (DBGflag)
                  {
                      sprintf(szDBGln,"   *COMMIT IN CUSHION*   Full=%d",
                                                              P_Full);
                      DBGout(szDBGln);
                  }

                  iRC = Out_REBUF(lpUPTO_Pkt, 8425);
                  //if (! iRC)
                  //{
                  //     Out_RECORD(lpMpeg_Copy_Buffer, iOverflow_Len, 8524);
                  //}

                  if (P_Full)
                  {
                     lpNEXT_Pkt = lpMpeg_Copy_Buffer;
                     lpUPTO_Pkt = lpNEXT_Pkt;
                  }
                  else
                  {  // restore packet pointers
                     lpMpeg_PKT_Anchor = lpUPTO_Pkt + 4;
                     lpNEXT_Pkt = lpMpeg_PKT_Anchor + iPkt_Between_Len;
                  }
               } // ENDIF Within Safety cushion


            } //ENDELSE Packet Overflow

         } // ENDIF More filtering work to do
     }

     // Reset controls for next packet
     lpMpeg_FROM = lpUPTO_Pkt;

  } // ENDIF Chunk to be written

  if (P_Full)
  {
      // Reset controls for next packet
      lpMpeg_PKT_Anchor = lpNEXT_Pkt;
      iPkt_Between_Len = 0;
      cMpeg_Out_Pkt_ACT = 0;
  }
  else
  {
    lpMpeg_PES_BeginData = lpMpeg_PKT_Anchor + uPkt_Hdr_Full_Len + 5;
  }

}




