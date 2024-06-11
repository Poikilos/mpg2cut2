
// Separately compiled with   OPTIMZE = MINIMUM SIZE  (with SP5 maint)

// This choice of optimization may seeem counter-intuitive.
// That's because it is !

// Why we do NOT use Optimize for Max Speed here :-
//   - Without SP5 generates long winded code for byte level IF statements,
//   - SP5 generates much better code for byte level IF statements,
//     BUT the handling of GOTO is seriously weird !
// THUS we do not use that optimizaion.


#include "global.h" 
#include "out.h"


unsigned char *lpMpeg_ScanLimit, *lpMpeg_ShallowEnd;
unsigned char *lpMpeg_Almost_EndPacket;


int iRepeat;



//-------------------------------------

void Out_Split_Hdr_Msg(const char P_Act[])
{
  iDeepFound++;
  iDeepNow = 1;
  iOut_CannotFix_ctr++;

  sprintf(szMsgTxt,"Cannot %s split %s hdr", P_Act, szHdrAbbr);
  DSP1_Main_MSG(0,0);
  iMsgLife = 0; szMsgTxt[0] = 0;

  UpdateWindow(hWnd_MAIN);   
}


//-------------------------------------

void Out_Vid_Hdr_GOT()
{
  //iDbg = lpMpeg_ix3 - lpMpeg_PES_BeginData;
  //lpDebug =      (DWORD*)(lpMpeg_ix3);
  //unsigned char *lpMpeg_TMP;
  unsigned char cTmp1;

  
  if (uHdrType == cPICTURE_START_CODE)
  {
      if (iOut_Parse_Deep && !iPreambleOnly_Flag) 
          iScanResult |= 1;
      else
          iScanResult |= 4;

      // uPTS_Accounted[uSubStream_Id] +=  (45 * iFrame_Period_ps / 1000000) ;

      if (DBGflag)
      {
          lpMpeg_ix4   = lpMpeg_ix3 + 4;
          sprintf(szBuffer, "   PIC HDR.  Off=%03d Type=%d ",
                              (lpMpeg_ix3 - lpMpeg_PES_BeginData - 2),
                             (((*lpMpeg_ix4)&0x28)>>3));
          DBGout(szBuffer);
      }

  } // END PIC HDR found
  else
  if (uHdrType == cSEQ_HDR_CODE)
  {
     strcpy( szHdrAbbr, "SEQ");

     if (lpMpeg_ix3 > lpMpeg_ShallowEnd)  // Found in the Deep ?
     {
         iDeepFound++;
         iDeepNow = 1;
     }

#ifdef DBG_FULL
     if (DBGflag)
     {
         sprintf(szBuffer, "   SEQ HDR.  Off=%03d  Fix=%d.%d",
                                   (lpMpeg_ix3 - lpMpeg_PES_BeginData - 4),
                                        iOut_Fix_SD_Hdr, iOut_Fix_Aspect);
         DBGout(szBuffer);
     }
#endif

     if (cStreamNeedsAligning_FRONT_Flag[uSubStream_Id] )
     {
         lpMpeg_SPLIT_ix = lpMpeg_ix3 - 4;
         //lpMpeg_ix5 = lpMpeg_PES_BeginData;
         if (lpMpeg_SPLIT_ix > lpMpeg_PES_BeginData)
         {
             Out_Filter_Split_Front(&iOut_SplitVideo_PrePackets);
         }
         
         cStreamNeedsAligning_FRONT_Flag[uSubStream_Id] = 0;   

     }

     iScanResult |= 1;

     if (lpMpeg_ix3 > lpMpeg_Almost_EndPacket) // lpMpeg_End_Packet - 5)
     {
         Out_Split_Hdr_Msg("check");
     }
     else
     {
       if (iOut_PTS_Invent)
       {
         if (DBGflag)
             DBGln2("   SEQ PESflags=x%02X PrevGOP=%d", 
                        cPES_Field_Flags, i64Adjust_TC[uSubStream_Id][2]); 

         iGOP_Memo[0] = 1;  iGOP_Memo[1] = 1;

         if( !(cPES_Field_Flags & 0x80))
         {
             iGOP_PTS_Chk = 2;
             iOut_Invent_Needed++;
             if (DBGflag)
                 DBGout("   GOP FLAG SET");
         }
       } // END PTS INVENT

       if (iOut_Fix_SD_Hdr || iOut_Fix_Aspect)
       {
          Out_Fix_Hdr_Vid_SEQ();
       }

       // Frame Rate - Base

       /* REMOVED - To avoid recalculation, will just use the one from the decoder 
       lpMpeg_TMP = lpMpeg_ix3 + 3;
       uTmp1 = *lpMpeg_TMP & 0x0F; // Frame Rate Code
       */


     }


  } // END SEQ HDR found

  else
  if (uHdrType == cEXTN_HDR_CODE
  && (iOutFRatioCode > 0  || iOut_Fix_SD_Hdr > 127
                          || iCtl_Out_Force_Interlace) )
  {

    if (lpMpeg_ix3 > lpMpeg_Almost_EndPacket) // (lpMpeg_End_Packet - 5))
    {
         strcpy(szHdrAbbr, "EXTN");
         Out_Split_Hdr_Msg("check");
    }
    else
    {
      cTmp1 = (unsigned char)(*(lpMpeg_ix3) & 0xF0);
      if( cTmp1 == 0x10 ) //  high order nybble = 1  SEQ EXTN
      {
          strcpy(szHdrAbbr, "SEQ EXTN");

          if (lpMpeg_ix3 > lpMpeg_ShallowEnd)  // Found in the Deep ?
          {
              iDeepFound++;
              iDeepNow = 1;
          }

          if (DBGflag)
          {
              sprintf(szBuffer, "   EXTN SEQ HDR.  Off=%03d  Fix=%d",
                                   (lpMpeg_ix3 - lpMpeg_PES_BeginData - 4),
                                              iOutFRatioCode);
              DBGout(szBuffer);
          }

          if (lpMpeg_ix3 > lpMpeg_Almost_EndPacket) // lpMpeg_End_Packet - 5)
          {
             Out_Split_Hdr_Msg("check");
          }
          else
             Out_Fix_Hdr_Vid_EXTN_SEQ();
      }  // END EXTN SEQ HDR found
      else
      if( cTmp1 == 0x80 ) //  high order nybble = 8  PIC EXTN
      {
          if (iCtl_Out_Force_Interlace)
          {
              Out_Fix_Hdr_PIC_EXTN_SEQ();
          }
      }  // END EXTN PIC HDR found
    } // Within Packet range
  } // END EXTN HDR found

  else
  if (uHdrType == cGROUP_START_CODE)   // (xB8 = GOP)
  {
      strcpy(szHdrAbbr, "GOP");

      iGOPFixed_Flag    = 1;

      if (lpMpeg_ix3 > lpMpeg_ShallowEnd)  // Found in the Deep ?
      {
          iDeepFound++;
          iDeepNow = 1;
      }

      if (DBGflag)
      {
          sprintf(szBuffer, "   GOP HDR.  Off=%03d  Fix=%d.%u Adj=%d",
                                  (lpMpeg_ix3 - lpMpeg_PES_BeginData - 4),
                                       iOut_TC_Adjust, uBroken_Flag,
                                            i64Adjust_TC[0][0]);
          DBGout(szBuffer);
      }


      if (cStreamNeedsAligning_FRONT_Flag[uSubStream_Id] )
      {
          lpMpeg_SPLIT_ix = lpMpeg_ix3 - 4;
          //lpMpeg_ix5 = lpMpeg_PES_BeginData;
          if (lpMpeg_SPLIT_ix > lpMpeg_PES_BeginData)
          {
             Out_Filter_Split_Front(&iOut_SplitVideo_PrePackets);
          }
          
          cStreamNeedsAligning_FRONT_Flag[uSubStream_Id] = 0;

      }

      iScanResult |= 2;

     if (lpMpeg_ix3 >= lpMpeg_Almost_EndPacket) //  > lpMpeg_End_Packet - 4)
     {
         Out_Split_Hdr_Msg("check");
     }
     else
     {
       // Maybe fix GOP header
       if (iOut_TC_Adjust && !iCtl_Out_KeepFileDate)
       {
          uSubStream_Id = cGOP_START_CODE; // Dummy setting for GOP;
          lpMpeg_TC_ix  = lpMpeg_ix3;
          Out_DeGap_TC(); 
          uSubStream_Id = cStream_Id; // Reset to true setting 
       }
                   
       if (uBroken_Flag)
       {
          lpMpeg_ix4   = lpMpeg_ix3 + 3;

          if (DBGflag)
          {
              sprintf(szBuffer, "  GOP BREAK: x%02X   Flag x%02X",
                                   (int)(*lpMpeg_ix4), uBroken_Flag);
              DBGout(szBuffer);
          }

          if (uBroken_Flag > 255)
              *lpMpeg_ix4   = (unsigned char)((*lpMpeg_ix4) & 0xDF); // Mark as NOT broken link for ANY GOP
          else
          {
              *lpMpeg_ix4   = (unsigned char)((*lpMpeg_ix4) | uBroken_Flag); // Mark as broken link if first GOP in this clip range
              uBroken_Flag = 0;
          }

       }
     }

  } // END GOP HDR found


  
#ifdef DBG_FULL
  if (DBGflag)
  {
     sprintf(szBuffer, "       Res=%d. Rpt=%d",
                               iScanResult,  iRepeat);
     DBGout(szBuffer);
  }
#endif  


}



//----------------------------------
// Scan for an interesting Video header
// If found, optionally split with ix3 pointing just after it
// Fix the header if requested to
// Repeat if requested to
void  Out_Vid_Hdr_SCAN(int P_Repeat)
{

  int iFull, iDeeper;
  unsigned int uTst;
  //int iDbg;
  //DWORD *lpDebug;

  register unsigned char *lpMpeg_TST;
  unsigned char *lpMpeg_ORG;


  iRepeat = P_Repeat;

  //iDbg = lpMpeg_ix3 - lpMpeg_Copy_Buffer;
  //lpDebug =      (DWORD*)(lpMpeg_ix3);
  lpMpeg_ORG = lpMpeg_ix3;  // Remember where we started payload
  lpMpeg_ShallowEnd = lpMpeg_ix3 + 256;
  if (lpMpeg_ShallowEnd > lpMpeg_End_Packet)
      lpMpeg_ShallowEnd = lpMpeg_End_Packet;

  lpMpeg_Almost_EndPacket = lpMpeg_End_Packet - 5;

  iDeeper = 0;
  if (iOut_Target_Tail)
  {
    if (cStreamNeedsAligning_REAR_Flag[uSubStream_Id])
       iDeeper = 1;
  }
  else
  {
    if (cStreamNeedsAligning_FRONT_Flag[uSubStream_Id])
       iDeeper = 1;
  }

  if (iOut_Parse_Deep             // Deep Parse ?
  ||  iDeeper)                    // Start or END of selection
  {
      lpMpeg_ScanLimit = lpMpeg_End_Packet;
      iDeepLook++;
      iFull = 1;
      if (DBGflag)
      {
        sprintf(szDBGln, "   Payload=%d  Bgn=0x%08X", 
                         (int)(lpMpeg_End_Packet - lpMpeg_ix3 +1),
                         *(DWORD *)(lpMpeg_ix3));
        DBGout(szDBGln);
      }
  }
  else
  {
      lpMpeg_ScanLimit = lpMpeg_ShallowEnd;
      iFull = 0;
  }

  iScanResult = 0;
  iDeepNow = 0;

#ifdef DBG_FULL
  if (DBGflag)
  {
     sprintf(szBuffer, " VID SCAN. Rpt=%d  Full=%d  Restart=x%02X",
                                  iRepeat, iFull,
                            iStartCodePart[uSubStream_Id]);
     DBGout(szBuffer);
  }
#endif


  // Try to find a major start code

  // Because of poor compiler optimization of the original "while"
  // I have restructured using goto's for speed.
  // Requires SP5 maint to work efficiently
  //   ICK ! 

  lpMpeg_TST = lpMpeg_ix3;  // Local Var for speed

  // May need to restart if start code was split across packets

  if (iFull  // Only restart when parsing entire packet
  && iCtl_Out_Parse_SplitStart)
  {
     uTst = iStartCodePart[uSubStream_Id];

     if (uTst == 0) goto Part_0_Entry;
     if (uTst == 1) goto Part_1_Entry;
     if (uTst == 2) goto Part_2_Entry;
     if (uTst == 3) goto Part_3_Entry;

     uHdrType = uTst - 1024;
     goto Part_4_Entry;
  }
  else
    iStartCodePart[uSubStream_Id] = 0;

// THE FOLLOWING 4 LINES OF CODE ARE SPEED CRITICAL 
//      - scans nearly every byte of the video data

Part_0_Entry: // Optimization target for skip non-zero
  if (lpMpeg_TST <= lpMpeg_ScanLimit) 
  {
      if (*lpMpeg_TST++)            // chk 0x00
          goto Part_0_Entry;       // Optimize compilation 
      else
      {

// END OF HIGH SPEED CODE

Part_1_Entry:
         if (lpMpeg_TST <= lpMpeg_ScanLimit) 
         {
            if (*lpMpeg_TST++)       // chk 0x00
               goto Part_0_Entry;  
            else
            {
Part_2_Entry:
               if (lpMpeg_TST <= lpMpeg_ScanLimit) 
               {
                  if (*lpMpeg_TST > 1) // 0x01
                  {
                     lpMpeg_TST++;
                     goto Part_0_Entry; 
                  }
                  else
                  if (*lpMpeg_TST == 0)
                  {
                     lpMpeg_TST++;
                     goto Part_2_Entry; 
                  }
                  else
                  {
                     lpMpeg_TST++;
Part_3_Entry:
                     if (lpMpeg_TST <= lpMpeg_ScanLimit) 
                     {
                        uTst = *lpMpeg_TST;
                        if ( ! uTst  
                        ||  (  uTst >= 0xB2 && uTst <= 0xB8))
                        {
                          uHdrType   = *lpMpeg_TST++;
                          lpMpeg_ix3 =  lpMpeg_TST;  // Reload from Local Var
Part_4_Entry:
                          if (lpMpeg_TST <= lpMpeg_ScanLimit)  
                          {
                              Out_Vid_Hdr_GOT();
                              lpMpeg_TST = lpMpeg_ix3;  // Local Var for speed
                              iStartCodePart[uSubStream_Id] = 0;
                          }
                          else
                              iStartCodePart[uSubStream_Id] = iFull * 1024 + uHdrType;
                        }


                        if (lpMpeg_TST > lpMpeg_ScanLimit) 
                        {
                            iScanResult    = 99;
                            iGOPFixed_Flag = 99;
                        }
                        else
                        if (iScanResult < iRepeat)
                            goto Part_0_Entry; // Optimize compilation
                     }
                     else
                     {
                        iStartCodePart[uSubStream_Id] = iFull * 3;
                        iScanResult    = 99;
                     }
                  }
               }
               else
               {
                 // Check for broken packet parsing or broken structure
                 uTst = *(UNALIGNED unsigned short*)(lpMpeg_TST);

                 if (uTst == 0xBA01  // Pack Hdr
                 ||  uTst == 0xE001  // Vid Hdr
                 ||  uTst == 0xC001) // MPa Hdr     
                 {
                    iStartCodePart[uSubStream_Id] = 0;
                    if (DBGflag)
                    {
                       sprintf(szBuffer, "  *PKT OVERSCAN* Ovr=%d, Span=%d, PACK#%03d Pkt#%04d Nxt=0x00%04X", 
                          (int)(lpMpeg_End_Packet - lpMpeg_TST +1),
                          (int)(lpMpeg_TST        - lpMpeg_ORG),
                                    iPack_Ctr, iOut_CheckedPackets, uTst);
                       DBGout(szBuffer) ;
                    }
                 }
                 else
                 {
                   iStartCodePart[uSubStream_Id] = iFull * 2;
                 }
                 iScanResult    = 99;
               }
            }
         }
         else
         {
               iStartCodePart[uSubStream_Id] = iFull * 1;
               iScanResult    = 99;
         }
      }
  }
  else
  {
       iStartCodePart[uSubStream_Id] = 0;
       iScanResult    = 99;
  }


  lpMpeg_ix3 = lpMpeg_TST;  // Reload from Local Var

#ifdef DBG_FULL
  if (DBGflag)
      DBGln4("   SCPart=%d\n   StartCd=x%04X :xB301   Tgt=%d, GOPFixed=%d, Result=x%02X",
                  iStartCodePart[uSubStream_Id],
                (*(UNALIGNED unsigned short*)(lpMpeg_ix3)),
                                              iGOPFixed_Flag, iScanResult);
#endif
}


