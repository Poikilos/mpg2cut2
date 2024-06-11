

 int iGOP_SecsAct, iGOP_Hour, iGOP_Min, iGOP_Sec, iGOP_Frame, iRemain;
 __int64 i64GOP_TC, i64ADJ_TC;
 unsigned int uDrop_Flag, uRest;


// Convert GOPTC time code to PTS base format 
void GOPTC_2PTS(__int64 *P_TC)
{

  uDrop_Flag = (*lpMpeg_TC_ix  ) &0x80;       // Dropflag  1bit
  
  /*
  iGOP_SecsAct = (((*lpMpeg_TC_ix  ) &0x7C)    * 900) // Hour      5bits  (900=3600/4)
           + (((*lpMpeg_TC_ix++) &0x03)    * 960) // Min-Major 2bits.. (960=60*16)
           +((((*lpMpeg_TC_ix  ) &0xF0)>>4)*  60) // Min-Minor 4bits+MARK
           + (((*lpMpeg_TC_ix++) &0x07)<<3)       // Sec-Major 3bits..
           + (((*lpMpeg_TC_ix  ) &0xE0)>>5  );    // Sec-Minor 3bits
  */

  iGOP_Hour   = ((*lpMpeg_TC_ix  ) &0x7C)>>2;    // Hour      5bits  

  iGOP_Min    = ((*lpMpeg_TC_ix++) &0x03)<<4;    // Min-Major 2bits..
  iGOP_Min   += ((*lpMpeg_TC_ix  ) &0xF0)>>4;    // Min-Minor 4bits  +MARK

  iGOP_Sec    = ((*lpMpeg_TC_ix++) &0x07)<<3;    // Sec-Major 3bits..
  iGOP_Sec   += ((*lpMpeg_TC_ix  ) &0xE0)>>5;    // Sec-Minor 3bits

  iGOP_Frame  = ((*lpMpeg_TC_ix++) &0x1F)<<1;     // Frame-Major 5bits
  iGOP_Frame += ((*lpMpeg_TC_ix  ) &0x80)>>7;     // Frame-Minor 1bit


  uRest       = ((*lpMpeg_TC_ix  ) &0x7F);

  iGOP_SecsAct =  (((iGOP_Hour * 60) + iGOP_Min) * 60) + iGOP_Sec;

  if (!iFrame_Period_ms)
       iFrame_Period_ms = 3000;

  i64GOP_TC = ((__int64)(iGOP_SecsAct) * 90000)
            + ((__int64)(iGOP_Frame    * iFrame_Period_ms * 90))
              ; // Number of clock cycles @ 90k/s

  *P_TC = i64GOP_TC;

  if (DBGflag)
  {
     sprintf(szBuffer, "   %s %02dh %02dm %02ds %02df  R=%06d  T=%08d Prev=%d f=%dms",
            "GOP:", iGOP_Hour, iGOP_Min, iGOP_Sec, iGOP_Frame, iGOP_SecsAct, 
                                  (int)(i64GOP_TC/90), 
                                  (int)(i64Adjust_TC[cGOP_START_CODE][0]/90),
                                  iFrame_Period_ms);
     DBGout(szBuffer);
  }

}




// Convert  PTS base format back to GOPTC time code
void PTS_2GOPTC(__int64 P_TC)
{

  i64ADJ_TC = P_TC;

  // Convert back to HH:MM:SS:FF

  iGOP_SecsAct = (int)(i64ADJ_TC / 1000);

  iGOP_Hour    = iGOP_SecsAct              / 3600;
  iRemain      = iGOP_SecsAct - (iGOP_Hour * 3600);

  iGOP_Min     = iRemain             / 60;
  iGOP_Sec     = iRemain - (iGOP_Min * 60);

  iGOP_Frame   = (int)(i64ADJ_TC - (iGOP_SecsAct * 1000))
                 / iFrame_Period_ms;


  if (DBGflag)
  {
     sprintf(szBuffer, "   %s %02dh %02dm %02ds %02df  R=%06d  T=%08d A=%d",
           " ==>", iGOP_Hour, iGOP_Min, iGOP_Sec, iGOP_Frame, iGOP_SecsAct, 
                                   (int)(i64ADJ_TC), iFrame_Period_ms);
     DBGout(szBuffer);
  }


  // Rebuild the Time Code, starting from the end byte
  if (uBroken_Flag > 255)
  {
    *lpMpeg_TC_ix-- = (BYTE)(( ( (iGOP_Frame &0x01)<<7)  // Frame-Minor 1bit
                                | uRest)  & 0xDF);    // Mark as NOT broken link for ANY GOP
  }
  else
  {
  
    *lpMpeg_TC_ix-- = (BYTE)(((iGOP_Frame &0x01)<<7)    // Frame-Minor 1bit
                             |   uRest | uBroken_Flag); // Mark as broken link if first GOP in this clip range
    uBroken_Flag = 0;
  }


  *lpMpeg_TC_ix-- = (BYTE)(  ((iGOP_Frame &0x3E)>>1)   // Frame-Major 5bits
                          | ((iGOP_Sec   &0x07)<<5)); // Sec-Minor   3bits

  *lpMpeg_TC_ix-- = (BYTE)(  ((iGOP_Sec  &0x38)>>3)   // Sec-Major 3bits
                          | 0x08                     // Mark      1bit
                          | ((iGOP_Min &0x0F)<<4));  // Min-Minor 4bits

  *lpMpeg_TC_ix   = (BYTE)(  ((iGOP_Min  &0x30)>>4)   // Min-Major 2bits
                          | ((iGOP_Hour &0x1F)<<2)   // Hour      5bits
                          | uDrop_Flag);             // Dropflag  1bit

}


// Convert internal format PTS to Mpeg format, including marker bits
void PTS_2PTSM(unsigned __int64 *P_PTSM,
                        __int64 *P_PTS,
               unsigned char     P_Prefix) // PTS-DTS flag + marker bit
{

  unsigned char *lpPTS, *lpPTSM;

  if (DBGflag)
  {
      sprintf(szBuffer, " PTS2PTSM  PTS=x%08x-%08x", *P_PTS);
      DBGout(szBuffer) ;
  }

  lpPTS  = ((unsigned char *)(P_PTS))+4;
  lpPTSM =  (unsigned char *)(P_PTSM);

  // Split into 2 lines due to compiler problem
  *lpPTSM    = (unsigned char)(P_Prefix                   // 4 bit sentinel
                             | ((*lpPTS--)&0x01)<<3);     // 1 bit
  *lpPTSM++ |= (unsigned char)(((*lpPTS  )&0xC0)>>5);     // 2 bits + marker

  // Split into 2 lines due to compiler problem
  *lpPTSM    = (unsigned char)(((*lpPTS--)&0x3F)<<2);    // 6 bits
  *lpPTSM++ |= (unsigned char)(((*lpPTS  )&0xC0)>>6);    // 2 bits

  // Split into 2 lines due to compiler problem
  *lpPTSM    = (unsigned char)(0x01
                             | ((*lpPTS--)&0x3F)<<2);    // 6 bits
  *lpPTSM++ |= (unsigned char)(((*lpPTS  )&0x80)>>6);    // 1 bit + marker


  // Split into 2 lines due to compiler problem
  *lpPTSM    = (unsigned char)(((*lpPTS--)&0x7F)<<1);     // 7 bits
  *lpPTSM++ |= (unsigned char)(((*lpPTS  )&0x80)>>7);    // 1 bits

  *lpPTSM++ = (unsigned char)(0x01
                            | ((*(lpPTS  )&0x7F)<<1));   // 7 bits + marker

  if (DBGflag)
  {
      sprintf(szBuffer, "          PTSM=x%08x-%08x", *P_PTSM);
      DBGout(szBuffer) ;
  }


  return;
}
/*
  unsigned uPTSM;

  uPTSM = 0x03010021
             | ( (P_PTS<<28) & 0x00000007 ) // 3 bits + mark
             | ( (P_PTS<<12) & 0x0000F800 ) // 5 bits
             | ( (P_PTS<< 8) & 0x00000700 ) // 3 bits
             | ( (P_PTS    ) & 0x00F80000 ) // 5 bits
             | ( (P_PTS>> 9) & 0x00060000 ) // 2 bits + mark
             | ( (P_PTS>>16) & 0xFC000000 ) // 6 bits
          ;
  return (uPTSM);
*/





// Convert Mpeg format PTS to internal SIGNED 64 but format
void PTSM_2PTS(__int64 *P_PTS)  // FROM *lpMpeg_TC_ix2
{

  unsigned char *lpPTS;

  lpPTS = (unsigned char *)(P_PTS) + 4;
  *(int*) lpPTS = 0; // Clear the high order bits


  *lpPTS    = (unsigned char)(((*lpMpeg_TC_ix2  )&0x08)>>3);    // 1 bit

#ifdef RJDBG_PTS
          if (DBGflag)
              DBGln4("                        %02X-%02X  %02X-%02X",
                        *lpMpeg_TC_ix2, *lpPTS,
                        ((*lpMpeg_TC_ix2)&0x06),
                       (((*lpMpeg_TC_ix2)&0x06)<<5) );
#endif

   lpPTS--;

  // Split into 2 lines due to compiler problem
  *lpPTS    = (unsigned char)(((*lpMpeg_TC_ix2++)&0x06)<<5);    // 2 bits -MARK
  *lpPTS   |= (unsigned char)(((*lpMpeg_TC_ix2  )&0xFC)>>2);    // 6 bits

#ifdef RJDBG_PTS
          if (DBGflag)
              DBGln4("                        %02X=%02X  %02X-%02X",
                        *lpMpeg_TC_ix2,   *lpPTS,
                      ((*lpMpeg_TC_ix2  )&0xFC),
                     (((*lpMpeg_TC_ix2  )&0xFC)>>2));
#endif

  lpPTS--;

  // Split into 2 lines due to compiler problem
  *lpPTS    = (unsigned char)(((*lpMpeg_TC_ix2++)&0x03)<<6);    // 2 bits
  *lpPTS   |= (unsigned char)(((*lpMpeg_TC_ix2  )&0xFC)>>2);    // 6 bits

#ifdef RJDBG_PTS
     if (DBGflag)
              DBGln2("                        %02X=%02X",
                        *lpMpeg_TC_ix2, *lpPTS );
#endif

   lpPTS--;

  // Split into 2 lines due to compiler problem
  *lpPTS    = (unsigned char)(((*lpMpeg_TC_ix2++)&0x02)<<6);    // 1 bit -MARK
  *lpPTS   |= (unsigned char)(((*lpMpeg_TC_ix2  )&0xFE)>>1);    // 7 bits

#ifdef RJDBG_PTS
     if (DBGflag)
              DBGln2("                        %02X=%02X",
                        *lpMpeg_TC_ix2, *lpPTS );
#endif

   lpPTS--;

  // Split into 2 lines due to compiler problem
  *lpPTS    = (unsigned char)(((*lpMpeg_TC_ix2++)&0x01)<<7);    // 1 bit
  *lpPTS   |= (unsigned char)(((*lpMpeg_TC_ix2++)&0xFE)>>1);    // 7 bits -MARK

#ifdef RJDBG_PTS
     if (DBGflag)
              DBGln2("                        %02X=%02X\n",
                        *(lpMpeg_TC_ix2-1), *lpPTS );

#endif

  // Theoretically we should multiply this by 300
  // but in a domestic environment, I don't think it is worth the overhead.


}


/*
  unsigned char cTS[5];

  // Convert Mpeg format PTS to internal format, removing marker bits
  // *BUT*  slightly different because these are SIGNED and 64 bits!

  cTS[0] =  (unsigned char)((*lpMpeg_TC_ix2++)& 0x06); //0x0e);
  cTS[1] =  (unsigned char)( *lpMpeg_TC_ix2++);
  cTS[2] =  (unsigned char)((*lpMpeg_TC_ix2++)& 0xfe);
  cTS[3] =  (unsigned char)( *lpMpeg_TC_ix2++);
  cTS[4] =  (unsigned char)((*lpMpeg_TC_ix2++)& 0xfe);

  // Combine into a SIGNED integer
  return (int)( ((unsigned int)(cTS[0])<<28)
              | ((unsigned int)(cTS[1])<<21)
              | ((unsigned int)(cTS[2])<<13)
              | ((unsigned int)(cTS[3])<<06)
              | ((unsigned int)(cTS[4])>>02) ); // Discard low-order bits

*/





// Convert internal format SCR to Mpeg format, including marker bits
static void SCR_2SCRM(unsigned char *P_SCRM,
                      unsigned char *P_SCR,
                      unsigned char  P_Prefix) // SCR flag + marker bit
{

  unsigned char *lpSCR, *lpSCRM, cSuffix;

  lpSCR  = P_SCR+4;
  lpSCRM = P_SCRM;

  // Split into 4 lines due to compiler problem
  *lpSCRM    = (unsigned char)(P_Prefix);                // 2 bit sentinel
  *lpSCRM   |= (unsigned char)(((*lpSCR--)&0x01)<<5);    // 1 bit
  *lpSCRM   |= (unsigned char)(((*lpSCR  )&0xC0)>>3);    // 2 bits + marker
  *lpSCRM++ |= (unsigned char)(((*lpSCR  )&0x30)>>4);    // 2 bits

  // Split into 2 lines due to compiler problem
  *lpSCRM    = (unsigned char)((( (*lpSCR--) &0x0F)<<4) );  // 4 bits
  *lpSCRM++ |= (unsigned char)((( (*lpSCR  ) &0xF0)>>4) );  // 4 bits

  // Split into 4 lines due to compiler problem
  *lpSCRM    = (unsigned char)(((*lpSCR--)&0x0F)<<4);   // 4 bits
  *lpSCRM   |= (unsigned char)(((*lpSCR  )&0x80)>>4);   // 1 bit 
  *lpSCRM   |= (unsigned char)(0x04);                   // 1 marker
  *lpSCRM++ |= (unsigned char)(((*lpSCR  )&0x60)>>5);   // 2 bits

  // Split into 2 lines due to compiler problem
  *lpSCRM    = (unsigned char)(((*lpSCR--)&0x1F)<<3);   // 5 bits
  *lpSCRM++ |= (unsigned char)(((*lpSCR  )&0xE0)>>5);   // 3 bits


  // Split into 3 lines due to compiler problem
  cSuffix   = (unsigned char)((*lpSCRM) & 0x07);
  *lpSCRM   = (unsigned char)(((*lpSCR  )&0x1F)<<3);   // 5 bits + marker
  *lpSCRM  |= (unsigned char)(cSuffix);      // 0x04 // bottom 3 bits are retained, as they  
  // SCR Extension ignored

  return;
}





// Convert Mpeg format SCR to internal SIGNED 64 bit format
void SCRM_2SCR(unsigned char *P_SCR) //  CRM-114
{

  unsigned char *lpSCR;

  lpSCR = P_SCR+4;
  *(int*) lpSCR = 0;

                                                             // -MARK -MARK
  *lpSCR    = (unsigned char)(((*lpMpeg_TC_ix2  )&0x20)>>5); // 1 bit

#ifdef RJDBG_PTS
   if (DBGflag)
              DBGln4("                        %02X-%02X  %08lX-%08lX",
                        *lpMpeg_TC_ix2, *lpSCR,
                        *(__int64*)(lpMpeg_TC_ix2+1), *(__int64*)(P_SCR));
#endif

   lpSCR--;

  *lpSCR   = (unsigned char)(((*lpMpeg_TC_ix2  )&0x18)<<3); // 2 bits -MARK
  *lpSCR  |= (unsigned char)(((*lpMpeg_TC_ix2++)&0x03)<<4); // 2 bits
  *lpSCR  |= (unsigned char)(((*lpMpeg_TC_ix2  )&0xF0)>>4); // 4 bits

#ifdef RJDBG_PTS
   if (DBGflag)
              DBGln2("                        %02X-%02X",
                        *lpMpeg_TC_ix2, *lpSCR);
#endif

   lpSCR--;

  *lpSCR    = (unsigned char)(((*lpMpeg_TC_ix2++)&0x0F)<<4); // 4 bits
  *lpSCR   |= (unsigned char)(((*lpMpeg_TC_ix2  )&0xF0)>>4); // 4 bits

#ifdef RJDBG_PTS
     if (DBGflag)
              DBGln2("                        %02X=%02X",
                        *lpMpeg_TC_ix2, *lpSCR);
#endif

   lpSCR--;

  *lpSCR    = (unsigned char)(((*lpMpeg_TC_ix2  )&0x08)<<4); // 1 bits -MARK
  *lpSCR   |= (unsigned char)(((*lpMpeg_TC_ix2++)&0x03)<<5); // 2 bits
  *lpSCR   |= (unsigned char)(((*lpMpeg_TC_ix2  )&0xF8)>>3); // 5 bits

#ifdef RJDBG_PTS
     if (DBGflag)
              DBGln2("                        %02X=%02X",
                        *lpMpeg_TC_ix2, *lpSCR );
#endif

   lpSCR--;

  *lpSCR    = (unsigned char)(((*lpMpeg_TC_ix2++)&0x07)<<5); // 3 bits
  *lpSCR   |= (unsigned char)(((*lpMpeg_TC_ix2  )&0xF8)>>3); // 5 bits -MARK

  // EXTENSION - IGNORED
  // Theoretically we should multiply major by 300 then add extension
  // but in a domestic environment, I don't think it is worth the overhead.

  if (DBGflag)
  {
#ifdef RJDBG_PTS
     DBGln2("                        %02X=%02X",
                           *lpMpeg_TC_ix2, *lpSCR );
#endif

     DBGln4("                 SCR=%04d ms  =%06u Ticks x%04X\n",
                   ( ( *(__int64*)(lpSCR) ) / 90),
                             *(__int64*)(lpSCR), *(__int64*)(lpSCR), 0x00);
     /*
     i64Tmp1 = ( ( *(__int64*)(lpSCR) )/90);
     DBGln4a("                 SCR=%I64d ms  =%I64d Ticks  x%I64X\n",
                               &i64Tmp1,    lpSCR,         lpSCR, 
                                    // (__int64*)(lpSCR), 
                                    //            (__int64*)(lpSCR), 
                                    NULL);
     */
  }
}






//__forceinline
void Out_DeGap_TC() //__int64 *lpPrev_TC)
{
   __int64 *lpPrev_TC;
   __int64 *lpADJ_TC;
   __int64 i64PTS_Orig,  i64PTS_New;
   __int64 i64Accum_ADJ, i64PTS_Diff, i64PTS_SCR_Diff, i64DTS_Diff=0;
   __int64 i64DTS;

  unsigned char  cBridge, cPTS_Sentinel;
  int iStreamReady_Flag; //, iDiff;
  //unsigned uPTSM, uPTSM_Minor=0;


  lpPrev_TC = &i64Adjust_TC[uSubStream_Id][0];
  lpADJ_TC  = &i64Adjust_TC[uSubStream_Id][1]; 

  cBridge = 0;
  lpMpeg_TC_ix2 = lpMpeg_PTS_ix;

  // We want to retain the initial origin, 
  // so no reset for the first sequence
  iStreamReady_Flag = iStreamReady[uSubStream_Id];
  //if (*lpPrev_TC)                  // || iCtl_Out_TC_Force) 
  //  iStreamReady_Flag = 1;
  //else            
  //  iStreamReady_Flag = 0;


  // Time stamps come in different formats
  if (uSubStream_Id == cPACK_START_CODE)  // PACK SCR is slightly different format to PTS/DTS
      SCRM_2SCR((unsigned char*)(&i64PTS_Orig));
  else
  if (uSubStream_Id == cGOP_START_CODE)  // GOP TC is slightly different format to PTS/DTS
      GOPTC_2PTS(&i64PTS_Orig);
  else
      PTSM_2PTS(&i64PTS_Orig);


  // Lock audio adjustment to latest video adjuster
  if (cStream_Cat == 'A')
     i64Accum_ADJ = i64Video_ADJ;
  else
  if (process.Suspect_SCR_Flag   && iCtl_Out_TC_Force // Ch.7
  &&  uSubStream_Id == cPACK_START_CODE)
  {
      if (iStreamReady_Flag && iPack_Ctr)
          i64Accum_ADJ = i64PTS_Orig - i64SCROut_TC - i64PackTickRate;
      else
          i64Accum_ADJ = 0;
  }
  else
  {
     i64PTS_Diff = i64PTS_Orig - *lpPrev_TC;

     // Trap any gaps that are bigger than the selected tolerance


     // STRANGE PHENOMENA:
     //
     //   32bit arithmetic only finds expected gaps,
     //         BUT DOES NOT WORK !
     //   64bit arithmetic finds MORE gaps than expected,
     //         BUT SOMETIMES WORKS !
     //
     //   MY BRAIN HURTS !

     if (i64PTS_Diff > i64Out_SCR_FWD_Tol 
     ||  i64PTS_Diff < i64Out_SCR_BWD_Tol) 

     //iDiff = (int)(i64PTS_Diff >>2);
     //if (iDiff > iOut_SCR_FWD_Tol 
     //||  iDiff < iOut_SCR_BWD_Tol) 
     {
         /*
         // Optionally  mark GOP as broken at discontinuity
         if (i64PTS_Orig > 1000) // Don't mark Panasonic reset on new chapter 
         {
            uBroken_Flag = (unsigned)iCtl_SetBrokenGop<<5; // Mpeg Broken Link flag is in a higher order bit.
            if (DBGflag)
                DBGout("BREAK GOP");
         }
         */

         i64CH7_ADJ   = 0;
         iBetweenPacks = 0;

         if (DBGflag)
         {
             DBGln4("\n** T Bridge T=%07dms, =x%08X\n         Prev=%07dms Diff=%05dtk",
                      (i64PTS_Orig/90), 
                      ((*(__int64*)(lpMpeg_PTS_ix))),   // &PTS_MASK_0),
                      (int)((*lpPrev_TC)/90), 
                      (int)(i64PTS_Diff/90)); // (unsigned)(cStream_Id));
             DBGln4("T-Break Tol: FWD=%d BWD=%d    PACK#%03d SCR=%d\n", 
                     (int)(i64Out_SCR_FWD_Tol), (int)(i64Out_SCR_BWD_Tol),
                     iPack_Ctr, i64PTS_Orig);
         }

         // Save the new adjuster for this type of TC

         i64Accum_ADJ = i64PTS_Diff + *lpADJ_TC;

         // Allow a safety gap of 4 frames in case PTS out of order

         if (cStream_Cat == 'V') 
             i64Accum_ADJ -= i64Out_PTS_Gap_Add;
         else
             i64Accum_ADJ -= i64Out_SCR_Gap_Add;

         //i64Accum_ADJ = i64Accum_ADJ & 0xFFFFFFFFFFFFFFE0;

         if (iStreamReady_Flag)
         {
             *lpADJ_TC  = i64Accum_ADJ;

             //if (uSubStream_Id == cPACK_START_CODE)     
             //   iOut_TC_Bridges++;
             //else
             if (cStream_Cat == 'V')
             {
                 i64Video_ADJ = *lpADJ_TC;
                 iOut_TC_Bridges++;
             }

             cBridge = 1;
 
            if (DBGflag)
            {
                sprintf(szBuffer, "         BRIDGE#%d  Stream=x%02X  Gap=%ds Tot=%ds Pack#%03d Rdy=#d",
                               iOut_TC_Bridges, uSubStream_Id, (int)(i64Accum_ADJ/90000), (int)(*lpADJ_TC/90000), iPack_Ctr, iStreamReady_Flag);
                DBGout(szBuffer);
            }
         } // endif StreamReady;
     } // endif Gap detected
     else
     {
       i64Accum_ADJ  = *lpADJ_TC;  // reload adjuster for this stream

       if (cStream_Cat == 'V')
       {
          if (i64PTS_Diff)
          {
             /*
             // Track actual rate via Video PTS
             if (iBetweenPacks)
             {
               //i64_PTS_CurrPosn = i64_CurrCopied + lpMpeg_PKT_Anchor - lpMpeg_FROM
               i64PackTickRate = i64PTS_Diff / iBetweenPacks;
               if (i64PackTickRate > 400)
                   i64PackTickRate = 400;
               else
               if (i64PackTickRate < 100)
                   i64PackTickRate = 100;
               //else
               //    i64PackTickRate = iPackTickRate; 

               iBetweenPacks = 0;
             }
             */
          }
          else
          if (process.Suspect_SCR_Flag   && iCtl_Out_TC_Force  
          &&  iStreamReady_Flag)
          {
              // We don't want redundant PTS/DTS
              iKill_PTS_Flag = 1;
          }

       } // endelse zero

     } // endif Video


     // Ch.7 SCR increments are are only on stream switch boundaries - Grrr... Argh...
     /*
     if (i64PTS_Diff < 0 && i64PTS_Diff > -180000  &&  uSubStream_Id == cPACK_START_CODE)
     {
        if (!i64PTS_Diff)
        {
           if (! iCtl_Out_TC_Force && cPut_Stream_Id != cPACK_START_CODE) 
           {
              iOut_TC_Adjust = 0; // Abandon TC adjustment for this save
              i64Accum_ADJ   = 0;
           }
           else
           {
               i64CH7_ADJ   += 420;
               i64Accum_ADJ -= i64CH7_ADJ; //  auto-increment SCR - this must match the FWD tolerance for Ch.7
           }
        }
        else
        if (iCtl_Out_TC_Force) 
        {
             i64Accum_ADJ = i64PTS_Orig - i64SCROut_TC - 100;
             i64CH7_ADJ   = 0;
        }
     } // ENDIF Ch7 fix pack
     */

  } // endif NON-audio

  
  // Current T becomes the next Previous
  *lpPrev_TC = i64PTS_Orig;

  iStreamReady[uSubStream_Id] = 1;

  // Only apply changes if adjustment required
  if (i64Accum_ADJ && iStreamReady_Flag)
  {
     // Apply adjustment
     i64PTS_New     = i64PTS_Orig - i64Accum_ADJ;

     if (uSubStream_Id == cPACK_START_CODE)  // PACK SCR is slightly different format to PTS/DTS
     {
         i64SCROut_TC = i64PTS_New;
         // Convert PTS back into Mpeg format
         cPTS_Sentinel = (unsigned char)(((*lpMpeg_PTS_ix) & 0xC0) | 0x04); // Retaining original sentinel nybble

         // re-insert marker bits and reverse bytes
         SCR_2SCRM(lpMpeg_PTS_ix, (unsigned char*)(&i64PTS_New),
                                 cPTS_Sentinel);
     }
     else
     if (uSubStream_Id == cGOP_START_CODE)  // GOP TC is slightly different format to PTS/DTS
         PTS_2GOPTC((i64PTS_New/90));
     else
     {
       // Allow for missing Video while audio continues
       if (cStream_Cat == 'V')
       {
           if (i64PTS_New < i64AudioOut_PTS)
               i64PTS_New = i64AudioOut_PTS;

           if (i64PTS_Diff && !iKill_PTS_Flag)
           {
              i64PTS_SCR_Diff = (i64PTS_New/90) - (i64SCROut_TC/90);

#ifdef RJDBG_PTS
              if (DBGflag)
              {
                  DBGln4("\n    PTS-SCR = %05d  Pack#%05d SCR=%09ums PTS=%09ums",
                           (int)(i64PTS_SCR_Diff),
                           iPack_Ctr,
                           (int)(i64SCROut_TC/90), 
                           (int)(i64PTS_New/90));
              }
#endif
              if (i64PTS_New < i64SCROut_TC)
                  i64PTS_New = i64SCROut_TC;

              if (i64PTS_SCR_Diff < 120)
              {
                if (i64PackTickRate > 150)
                {
                 if (i64PTS_SCR_Diff < 0)
                     i64PackTickRate -=10;
                  else
                     i64PackTickRate--;
                }
              }
              else
              if (i64PTS_SCR_Diff > 120)
              {
                 if (i64PackTickRate < 500)
                 {
                    if (i64PTS_SCR_Diff > 1000)
                        i64PackTickRate +=10;
                    else
                        i64PackTickRate++;
                 }
              }
           } 
       }
       else
       {
           // Remember latest adjusted audio PTS
           if (cStream_Cat == 'A')
               i64AudioOut_PTS = i64PTS_New;
       }


       // Convert PTS back into Mpeg format
       cPTS_Sentinel = (unsigned char)(((*lpMpeg_PTS_ix) & 0xF0) | 0x01); // Retaining original sentinel nybble

       // re-insert marker bits and reverse bytes
       PTS_2PTSM((unsigned __int64*)(lpMpeg_PTS_ix), &i64PTS_New,  cPTS_Sentinel);

       // Is there a DTS decode time stamp ?
       if (cPES_Field_Flags & 0x40)
       {
          // Maintain DTS relative to PTS
          lpMpeg_DTS_ix = lpMpeg_TC_ix2;
          PTSM_2PTS(&i64DTS);
          i64DTS_Diff     = i64DTS     - i64PTS_Orig;
          i64DTS          = i64PTS_New + i64DTS_Diff;
          PTS_2PTSM((unsigned __int64*)(lpMpeg_DTS_ix), &i64DTS, 0x11);  // re-insert marker bits and reverse bytes
       }
     }

     if (DBGflag) // && cBridge)
         DBGln4("         NEW T=x%08X =%07u, ADJ=%05d, DTS=%07u\n",
                         ((*(__int64*)(lpMpeg_PTS_ix))), // uPTSM,
                                  i64PTS_New, i64Accum_ADJ, i64DTS); 
                                             // (unsigned)(cStream_Id));
  }
  else
  if (uSubStream_Id == cPACK_START_CODE)  // PACK SCR is slightly different format to PTS/DTS
  {
      i64SCROut_TC = i64PTS_Orig;
  }


  return;
}



//-----------------------------------------------------------------


// Create PTS on the packet header
void Out_Invent_PTS()
{

  __int64 i64PTS_New, i64Increment;
  int iTmp1;
  
  unsigned char *lpPESflags;

  if (cStream_Cat == 'V')
  {
    // Use last GOP Video PTS increment to calculate the next
    i64PTS_New = i64Adjust_TC[uSubStream_Id][2]
               * 2                               
               - i64Adjust_TC[uSubStream_Id][3];

  }
  else
  if (cStream_Cat == 'A')
  {
    iTmp1 = iBitRate[uSubStream_Id];
    if (!iTmp1)
        i64Increment = 250 * 90;  // Australian DTV default = 250ms
    else
        i64Increment = (iPkt_Between_Len * (8 * 90000) / iTmp1);

    i64PTS_New = i64Adjust_TC[uSubStream_Id][0]
               + i64Increment;

    if (iGOP_PTS_Chk
    &&  i64PTS_New > i64Adjust_TC[0xE0         ][0] )
    {
       // Use prev GOP Audio Delay to calculate the next Audio PTS
       i64PTS_New = i64Adjust_TC[0xE0         ][2] 
                  + i64Adjust_TC[0xE0         ][3]
                  - i64Adjust_TC[uSubStream_Id][2];
    }

  }
  else
    return;

  i64Adjust_TC[uSubStream_Id][0] = i64PTS_New;
  //i64PTS_New += i64Adjust_TC[uSubStream_Id][1]; // Nett Adjuster for joins

  // Convert PTS back into Mpeg format
  //cPTS_Sentinel = (unsigned char)(0x81); // Create PTS sentinel marker bits

  // re-insert remaining marker bits and reverse bytes

  lpPESflags = lpMpeg_PKT_Anchor+3;   // look at PES HDR Field flags

  if (DBGflag)
  {
      DBGln4("   Invent %c PTS=x%08X  HdrLen=%u  Flags=x%02X",
                       cStream_Cat,
                       i64PTS_New, uPES_Hdr_Len, *lpPESflags); 
      DBGln4("           Prev=x%08X x%08X   Diff=%dms",
                       i64Adjust_TC[uSubStream_Id][2],
                       i64Adjust_TC[uSubStream_Id][3],
                      (i64Adjust_TC[uSubStream_Id][2]
                     - i64Adjust_TC[uSubStream_Id][3])/90,
                        i64PTS_New);

    if (cStream_Cat == 'A')
    {
        DBGln4("             E0=x%08X x%08X   Diff=%dms   Aud=%dKbps",
                           i64Adjust_TC[0xE0         ][2], 
                           i64Adjust_TC[0xE0         ][3],
                          (i64Adjust_TC[0xE0         ][3]
                         - i64Adjust_TC[uSubStream_Id][2])/90,
                           uMPA_kBitRate);
    }
  }

  if (uPES_Hdr_Len > 4) // Is there header space ?
  {
    if ( ! *lpPESflags)          // Is the space  available ?
    {
       *lpPESflags      = 0x80;  // Set PES HDR Field flags
       cPES_Field_Flags = 0x80;  // Remember it
    }

     PTS_2PTSM((unsigned __int64*)(lpMpeg_PTS_ix), &i64PTS_New,
                                   0x81); // cPTS_Sentinel);
     iOut_Invent_Done++;
  }

 return;

}





void Out_TC_Rewind(         __int64 *lpP_PTSM, 
                            __int64 *lpP_PTS, 
                   unsigned   int      P_uAdj)
{

           __int64 i64PTS;
  unsigned __int64 u64PTSM; //, *i64PTS_ix;
  unsigned   int   *lpP_PTSM_32;
           __int64 *lpP_PTS_DBG;
  unsigned char cPTS_Sentinel; //, *lpPTSM;
  unsigned int iReturn;

  lpP_PTS_DBG  = (__int64*)(lpP_PTS);
  lpP_PTSM_32 = (unsigned int*)(lpP_PTSM);

  if (DBGflag)
  {
      sprintf(szBuffer, "                 REWIND=x%08x  ADJ=%04u =%ums", *lpP_PTSM_32, P_uAdj, (P_uAdj/90));
      DBGout(szBuffer) ;
  }

  lpMpeg_TC_ix2 = (unsigned char *)(lpP_PTSM);
  PTSM_2PTS(&i64PTS);
  if (DBGflag)
  {
      sprintf(szBuffer, "                    PTS=x%08x-%x =%08d-%d  =%dms", i64PTS, i64PTS, (int)(i64PTS/90));
      DBGout(szBuffer) ;
  }

  if (i64PTS > P_uAdj)
      i64PTS = i64PTS - P_uAdj;
  else
      i64PTS = 0;

  if (DBGflag)
  {
      sprintf(szBuffer, "                    PTS=x%08x-%x =%08d-%d  =%dms", i64PTS, i64PTS, (int)(i64PTS/90));
      DBGout(szBuffer) ;
  }

  // Convert PTS back into Mpeg format
  cPTS_Sentinel = (unsigned char)(((*lpMpeg_TC_ix2) & 0xF0) | 0x01); // Retaining original sentinel nybble

  // re-insert marker bits and reverse bytes
  //i64PTS_ix = &i64PTS;
  PTS_2PTSM(&u64PTSM, &i64PTS, cPTS_Sentinel);

  *lpP_PTSM_32 = (int)u64PTSM;

  if (DBGflag)
  {
      sprintf(szBuffer, "                    NEW=x%08x", *lpP_PTSM_32);
      DBGout(szBuffer) ;
      sprintf(szBuffer, "                    PTS=x%08x-%x =%08d-%d  =%dms", i64PTS, i64PTS, (int)(i64PTS/90));
      DBGout(szBuffer) ;
  }

  iReturn = (unsigned int)i64PTS;
  *lpP_PTS = iReturn;
  return; // iReturn;

}




