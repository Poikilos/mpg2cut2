
#define DBG_RJ


#include "global.h"
#include <math.h>
#include <sys/timeb.h>

#include "Audio.h"
#include "wave_out.h"



//---------------------------------
int iCURR_TIME_ms()
{
  static struct _timeb ts2;

  _ftime( &ts2 );
  return (int)(((ts2.time)*1000+(ts2.millitm) & 0x7FFFFFFF));

}





int iWavQue_Frames;
//int iStat_ms;

//------------------------------------

void Timing_DropMore()
{
  int iTrig;

  if (DBGflag)
  {
     sprintf(szBuffer, " iDrop=%d  FPS=%d%%  Height=%d   WavBlks=%d ",
                         PlayCtl.iDrop_Behind,
                         PlayCtl.iFPS_Dec_Pct, Coded_Pic_Height,
                        iWAVEOUT_Scheduled_Blocks);
     DBGout(szBuffer);
  }


  // TODO: FIND OUT WHY FRAME DROPPER STUFFS UP FIELD MODE DECODE SEQUENCE
  //
  //       Luckily most VOBs & PAL DTV files that I have seen
  //       use plain old Frame mode encoding, so are NOT impacted.
  //
  //       BUT the BBC DVD of "The Goodies" seems to be a FIELD MODE encode.
  //       Presumably some NTSC files will be FIELD mode too.
  //       As TV encodes get smarter, expect more Field mode encoders.
  //
  // The Field mode bug is most obvious when DEINTerlacer is also active.
  // So perhaps the bug is there, or shared with that.


  // TRAP POOR PERFORMANCE

  if  (MPEG_Pic_Structure == FULL_FRAME_PIC  // Not supported on field mode encodes
  &&  
      (PlayCtl.iDrop_Behind >  1            // Drop allowed on SD ?
       ||  MParse.iColorMode == STORE_RGB24  // slow interface
       ||  Coded_Pic_Height  >  576          // HD - large canvas
       ||  iFrame_Rate_int   >  33)         // High frame rate HD - auto Drop frames
      )
  {
      if (PlayCtl.iFPS_Dec_Pct > 140
      ||  PlayCtl.iMaxedOut
   //   ||  PlayCtl.iBehindFrames <= 0
      || (iWAVEOUT_Scheduled_Blocks > WAVEOUT_MAX_BLOCKS //WAVEOUT_MID_PKTS_CUSHION)
                                           && iWAV_Init)
         )
      {
         if (PlayCtl.iDrop_B_Frames_Flag > 0
             && MParse.FastPlay_Flag < CUE_SLOW)
             PlayCtl.iDrop_B_Frames_Flag--;
      }
      else

      if (PlayCtl.iFPS_Dec_Pct <= 50  //  (way down from nominal frame rate)
      || (iWAVEOUT_Scheduled_Blocks < WAVEOUT_LOW_PKTS_CUSHION
           && iWAV_Init))
      {

         if ((iWAVEOUT_Scheduled_Blocks   < WAVEOUT_MID_FINE_PKTS_CUSHION
              &&  MParse.SlowPlay_Flag <= 0)
         // ||  PlayCtl.iDrop_B_Frames_Flag > 2)
         || *File_Name[File_Ctr] == 'Z') // My optical drive.  TODO: Generalize this by interrogating drive info
              iTrig = 95;
         else
         if (MParse.FastPlay_Flag > 2 && !cpu.sse2
         &&  PlayCtl.iDrop_B_Frames_Flag == 1
         ||  Coded_Pic_Height > 576)
              iTrig = 90;
         else
              iTrig = 80;

         if (iWAVEOUT_Scheduled_Blocks < WAVEOUT_MID_MID_PKTS_CUSHION
         &&  PlayCtl.iBehindFrames > 1
            )
         {
             if (PlayCtl.iFPS_Dec_Pct < iTrig
             &&  PlayCtl.iBehindFrames > 4)
                 PlayCtl.iDrop_B_Frames_Flag = 2;
             else
                 PlayCtl.iDrop_B_Frames_Flag = 1;
         }
      }


      //process.Pack_Min_Size = process.Pack_Max_Size;
      process.Pack_Max_Size = 0;
  }

  if (DBGflag)
  {
      sprintf(szBuffer, " DropB=%d ",
                                 PlayCtl.iDrop_B_Frames_Flag);
      DBGout(szBuffer);
  }
}




//-----------------------------------------------------


int Store_Timing_Chk(int P_Overlay)
{
  // TIMING CHECK
  // WAIT if ahead
  // SKIP DELTA FRAMES if way behind during RIP/PLAY

  // DWORD dwTime2;
  int iTmp2;
  int iTime2, iFrame_Diff_ms, iPeriod_Adj;
  int iResult, iRenderTime, iTmp1;
  int iFramesTillNextAudio, iAudioInterAdj, iQuePerInterleave;
  int iQueVsNext;
  unsigned char cAudNew;

  iResult = 0;
  iRenderTime = 0;  iQueVsNext = 0;

// int iTmp1;

  if (iDDO_Frame_Ready)
  {
      R250_SIGNAL_Overlay(); // D200_UPD_Overlay();
  }

  if (MParse.FastPlay_Flag)
  {
      iPeriod_Adj = iFrame_Period_ms;
      if (iOverride_FrameRate_Code == 0
      ||  MParse.FastPlay_Flag >= CUE_SLOW)
      {
         switch (MParse.FastPlay_Flag)
         {
           case 0: break;
           case 1: iPeriod_Adj = iPeriod_Adj * 2 / 3; break;
           case 2: iPeriod_Adj = iPeriod_Adj / 2;     break;
           case 3: iPeriod_Adj = iPeriod_Adj / 3;     break;
           case 4: iPeriod_Adj = iPeriod_Adj / 4;     break; // MAX_WARP

           case CUE_SLOW: 
                if (iGOPtot == 1) // allow for I-Frame-only streams
                    iPeriod_Adj = iPeriod_Adj / 8;     
                else
                    iPeriod_Adj = iPeriod_Adj * 2 / 3;     // CUE-SLOW
                break; 

            default:  // CUE_QUICK..SUPER-CUE
              iPeriod_Adj = iPeriod_Adj
                          * (MParse.FastPlay_Flag + 1)
                          / (MParse.FastPlay_Flag * 3);  // 1=0.666 2=0.5
         }
      }
  }
  else
  if (MParse.SlowPlay_Flag > 0 
  &&  iOverride_FrameRate_Code == 0)
  {
             if (MParse.SlowPlay_Flag == 1)
                 iTmp2 = 5;
             else
                 iTmp2 = MParse.SlowPlay_Flag * 4;

             iPeriod_Adj = iFrame_Period_ms * iTmp2 / 4;
   //        iPeriod_Adj = iFrame_Period_ms
   //                    *  (MParse.SlowPlay_Flag * 3) 
   //                    / ((MParse.SlowPlay_Flag) + 1);
  }
  else
          iPeriod_Adj = iFrame_Period_ms;


  // Calculate time taken to decode this frame
  iTime2 = iCURR_TIME_ms();
  if (iRender_TimePrev == 0)
  {
      iFrame_Diff_ms = 0;
      //process.iCatchUp = 0;
  }
  else
  {
     iRenderTime =  iTime2 - iRender_TimePrev;
     iFrame_Diff_ms = iPeriod_Adj - iRenderTime; // - process.iCatchUp;
     //if (iCtl_AudioThread)
     //    iFrame_Diff_ms -=40;  // Experimental kludge

  }

  //iRender_TimePrev = iTime2;

   if (process.iWavBytesPerMs < 1)   // Allow for silly settings
       process.iWavBytesPerMs = 192; // default like 48k samples/sec

    if (DBGflag)
    {
        sprintf(szBuffer, "\nTIMING %c ALock=%d PlayAud=%d  WavInit=%d  Playedhdrs=%d",
                   Coded_Pic_Abbr[MPEG_Pic_Type],
                   iAudio_Lock, iPlayAudio, iWAV_Init,
                   PlayedWaveHeadersCount);
        DBGout(szBuffer);
    }


/* // Lip sync code will have to wait for major rehash of PLAY mode

  // Estimate how much audio still in buffer
  iWavQue_ms = process.iWavQue_Len / process.iWavBytesPerMs;
  if (iCtl_Play_Sync && Delay_Sign[0] == '-')
  {
     iSync_Diff_ms = iWavQue_ms - (int)(process.Delay_ms) - 20;
  }
  else */

  if (!iAudio_Lock || !iPlayAudio || !iWAV_Init)
  {
    cAudNew = 'F';
    iSync_Diff_ms = iFrame_Diff_ms;

    //if (Frame_Number > 24) //  ||  ! iAudio_Lock)
    // if (iWAV_Init)
    if (PlayCtl.iFPS_Dec_Pct < 110)
    {
        if (!cpu.mmx) // Allow for ultra-slow old cpu
           iSync_Diff_ms=0;  
        else
        if (PlayCtl.iFPS_Dec_Pct > 95)
           iSync_Diff_ms--;
        else
        if (PlayCtl.iFPS_Dec_Pct > 90)
           iSync_Diff_ms-=2;
        else
        if (PlayCtl.iFPS_Dec_Pct > 80)
           iSync_Diff_ms-=3;
        else
        if (PlayCtl.iFPS_Dec_Pct > 70)
           iSync_Diff_ms-=5;
        else
        if (PlayCtl.iFPS_Dec_Pct > 50)
           iSync_Diff_ms-=7;
        else
        if (PlayCtl.iFPS_Dec_Pct > 40)
           iSync_Diff_ms-=10;
        else
           iSync_Diff_ms-=20;

        if (!cpu.sse2)
           iSync_Diff_ms-=1;
    }
    else
    if (PlayCtl.iFPS_Dec_Pct > 140)
    {
        if (PlayCtl.iFPS_Dec_Pct > 160)
           iSync_Diff_ms+=2;
        else
        if (PlayCtl.iFPS_Dec_Pct > 200)
           iSync_Diff_ms+=3;
        else
           iSync_Diff_ms++;
    }

/*
    if (PREV_Pic_Type == B_TYPE && MPEG_Pic_Type == B_TYPE) // Expecting a P or I frame ?
    {
      if (PlayCtl.iFPS_Dec_Pct < 105)
         iSync_Diff_ms = iSync_Diff_ms * 9 / 10;
    }

    if ((iGOPrelative+1) == iGOPtot && iGOPtot > 4)
    {
      if (PlayCtl.iFPS_Dec_Pct < 102)
         iSync_Diff_ms = iSync_Diff_ms * 8 / 10;
    }
*/

  } // END-IF Ignore Audio 
  else
  if (PlayCtl.iFPS_Dec_Pct < 50                             // Way-Slow
  &&  iWAVEOUT_Scheduled_Blocks < WAVEOUT_HIGH_PKTS_CUSHION // Way-Ahead
  &&  PlayCtl.iDrop_B_Frames_Flag > 1)
  {
     cAudNew = '0';
     iSync_Diff_ms = 0;
  }
  else
  {
    while ( PlayedWaveHeadersCount > 0 )                // free used blocks ...
        WAV_Free_Memory();

    iWavQue_ms = process.iWavQue_Len / process.iWavBytesPerMs;

    if (iPeriod_Adj > 5)
        iTmp1 = iPeriod_Adj;
    else
        iTmp1 = 5;
    iWavQue_Frames = iWavQue_ms / iTmp1;

    // allow for unknown interleave at start
    if (process.iAudio_Interleave)
        iAudioInterAdj = process.iAudio_Interleave + 1;
    else
        iAudioInterAdj = 30;  // roughly 1 second default


    iFramesTillNextAudio = iAudioInterAdj - process.iAudio_InterFrames;

    iTmp1 = iAudioInterAdj - Frame_Number;
    if (iFramesTillNextAudio < 0 || iTmp1 <= 0 )
    {
        iFramesTillNextAudio = iTmp1;
        if (iFramesTillNextAudio <= 0)
            iFramesTillNextAudio = iAudioInterAdj / 2 + 1;
    }

    iFramesTillNextAudio +=3;

    iQueVsNext        = (iWavQue_Frames - iFramesTillNextAudio) ;

    iQuePerInterleave = iQueVsNext * 100  // (iWavQue_Frames+1) * 10
                      / iAudioInterAdj;

    if (DBGflag)
    {
        sprintf(szBuffer, "\nWAVE %c      Aud=%d %03dms Done=%d  Q/I=%d QvN=%d Fq=%d Fi=%d",
                   Coded_Pic_Abbr[MPEG_Pic_Type],
                   iWAVEOUT_Scheduled_Blocks, iWavQue_ms,
                   PlayedWaveHeadersCount,
                   iQuePerInterleave, iQueVsNext,
                   iFramesTillNextAudio, iAudioInterAdj);
        DBGout(szBuffer);
    }

    //if (iWAVEOUT_Scheduled_Blocks < 1)   // Run out of Audio ?
    //    iSync_Diff_ms = iDropTrigger_ms; // Drop the frame
    //else
    if ((iWAVEOUT_Scheduled_Blocks <  WAVEOUT_LOW_PKTS_CUSHION // Nearing Cliff ?
             //  || iWavQue_Frames <  iFramesTillNextAudio
                 || iQueVsNext < 2
             //  || iQuePerInterleave < 3
      //        ||        iWavQue_ms <= iPeriod_Adj
      //        ||  (iGOPrelative+2) >= iGOPtot   // Nearing the "hard part" at the start of the next GOP ?
              )
      //&& Frame_Number > 4        // Allow for missing audio at start
    )
    {

        cAudNew = '2';  // Low Audio pool

        if (! MParse.FastPlay_Flag)
        {
          if (PlayCtl.iFPS_Dec_Pct > 133 // Don't go overboard
          &&  MParse.SlowPlay_Flag <= 0)
          {
             cAudNew = 'G';
          }
          else
          if (iWavQue_Frames < 5)
          {
             cAudNew = 'Z';
             iSync_Diff_ms = iFrame_Diff_ms / 3;
          }
          else
          if (iWavQue_Frames < iAudioInterAdj)
          {
             cAudNew = '1';
             iSync_Diff_ms = iFrame_Diff_ms / 2;
          }

          if ((iGOPrelative+1) >= iGOPtot
          || (PREV_Pic_Type == B_TYPE && MPEG_Pic_Type == B_TYPE))
             iSync_Diff_ms = iSync_Diff_ms  * 4 / 10;
          //else
          //   iSync_Diff_ms = iSync_Diff_ms  * 7 / 10;

        }

        else   // FastPlay

        if (iWAVEOUT_Scheduled_Blocks
        &&  iAudioInterAdj > 4)  // chunky ?
        {
          if ((iGOPrelative+1) == iGOPtot
          || (PREV_Pic_Type == B_TYPE && MPEG_Pic_Type == B_TYPE))
             iSync_Diff_ms = iFrame_Diff_ms     / 5;
          else
             iSync_Diff_ms = iFrame_Diff_ms * 2 / 5;
        }
        else // Not chunky or no audio in buffer
        if (iSync_Diff_ms > 2 ) // && MParse.FastPlay_Flag < CUE_SLOW
            iSync_Diff_ms = 2;
        //else
        //    iSync_Diff_ms = 0;
        //if (PlayCtl.iDrop_Behind || )
        if (!PlayCtl.iMaxedOut)
            Timing_DropMore();

    }

    else
    if (iWAVEOUT_Scheduled_Blocks >= WAVEOUT_HIGH_PKTS_CUSHION) // Way-Ahead
    {
        cAudNew = '9';
        iSync_Diff_ms = iPeriod_Adj * 9 / 8 ;        // Tea Break

        if (PlayCtl.iDrop_B_Frames_Flag > 0 && PlayCtl.iFPS_Dec_Pct > 145)
            PlayCtl.iDrop_B_Frames_Flag--;

    }
    else
    if (iQueVsNext < 4) // iQuePerInterleave < 50)   // In the zone ?
    {
        cAudNew = '5';

        //if (cpu.sse2)  // Fast machine ?  (P4 or better)
        //   iTmp1 = 2;
        //else
        //   iTmp1 = 3;

        if (iQueVsNext < 3) // iTmp1) // iQuePerInterleave < 25)
        {
          if (cpu.sse2)  // Fast machine ?  (P4 or better)
              iTmp1 = 1;
          else
          if (cpu.mmx)  // Fast machine ?  (P4 or better)
              iTmp1 = 2;
          else
              iTmp1 = 10;

          iSync_Diff_ms = iFrame_Diff_ms - iTmp1; // Below Nominal

          if (iQueVsNext < 2) // iQuePerInterleave < 25)
          {
            cAudNew = '4';
            if (PREV_Pic_Type == B_TYPE && MPEG_Pic_Type == B_TYPE) // Expecting a P or I frame ?
               iSync_Diff_ms = iSync_Diff_ms  * 7 / 10;


            if ((iGOPrelative+1) == iGOPtot && iGOPtot > 4)  // Expecting I-Frame ?
            {
              if (PlayCtl.iFPS_Dec_Pct < 100
              ||  iQuePerInterleave    < 20)
                  iSync_Diff_ms = iSync_Diff_ms * 7 / 10;
              else
                  iSync_Diff_ms = iSync_Diff_ms * 9 / 10;
            }

          }

        }
        else
          iSync_Diff_ms = iFrame_Diff_ms - 1; // almost Nominal


        //if (iWAVEOUT_Scheduled_Blocks < WAVEOUT_MID_FINE_PKTS_CUSHION)
        //{
        //       cAudNew = '3';
        //       iSync_Diff_ms = iSync_Diff_ms - 5;
        //}

        if (PlayCtl.iDrop_B_Frames_Flag > 1 && PlayCtl.iFPS_Dec_Pct > 190)
            PlayCtl.iDrop_B_Frames_Flag--;


        //if (MParse.FastPlay_Flag)
        //{
        //  iSync_Diff_ms = iSync_Diff_ms / 3;
        //}
    }
    /*
    if (iWAVEOUT_Scheduled_Blocks < WAVEOUT_MID_PKTS_CUSHION) // In the zone ?
    {
        if (MParse.FastPlay_Flag)
            iSync_Diff_ms = iFrame_Diff_ms * 3 / 5;
        else
        if (iWavQue_Frames > (process.iAudio_Interleave * 3))
        {
            iSync_Diff_ms = iFrame_Diff_ms + 1;//iPeriod_Adj;  // go easy
        }
        else
        {
            iSync_Diff_ms = iFrame_Diff_ms;                // go Nominal

            //if (PREV_Pic_Type == B_TYPE && MPEG_Pic_Type == B_TYPE) // Expecting a P or I frame ?
            //   iSync_Diff_ms = iSync_Diff_ms / 2;

            if ((iGOPrelative+1) == iGOPtot && iGOPtot > 4)
            {
                if (PlayCtl.iFPS_Dec_Pct < 105)
                   iSync_Diff_ms = iSync_Diff_ms * 7 / 10;
            }
        }

        if (PlayCtl.iDrop_B_Frames_Flag > 1 && PlayCtl.iFPS_Dec_Pct > 190)
            PlayCtl.iDrop_B_Frames_Flag--;


        cAudNew = 'M';

    }
    */
    else                                             // Way ahead ?
    {
       cAudNew = '7';
       iSync_Diff_ms = iFrame_Diff_ms;             // Go Nominal

       if (iQueVsNext > 4) // && iGOPrelative > 4 && iGOPtot > 2)
       {
           cAudNew = 'I';
           iSync_Diff_ms+= 1; // + (MParse.SlowPlay_Flag);
           if (iQueVsNext > 6)
           {
               cAudNew = 'J';
               iSync_Diff_ms += 1; // + (MParse.SlowPlay_Flag);
               if (iQueVsNext > 7)
               {
                   cAudNew = 'K';
                   iSync_Diff_ms += 1; // + (MParse.SlowPlay_Flag);
               }
           }
       }
       /*
       if (! MParse.FastPlay_Flag)
       {
             //if (process.iAudio_Interleave < 5
             //&&  iWAVEOUT_Scheduled_Blocks > WAVEOUT_MID_FINE_PKTS_CUSHION)
                   iSync_Diff_ms = iFrame_Diff_ms;//iPeriod_Adj; // Slack off
             //else
             //    iSync_Diff_ms = iFrame_Diff_ms - 1;         // Go Nominal
       }
       else
       {
          if (process.iAudio_Interleave > 4
          ||  iWAVEOUT_Scheduled_Blocks > WAVEOUT_MID_FINE_PKTS_CUSHION)
              iSync_Diff_ms = iFrame_Diff_ms;//iPeriod_Adj;            // Slack off
          else
              iSync_Diff_ms = iFrame_Diff_ms * 9 / 10; // Almost nominal


       }
       */

          //if (PREV_Pic_Type == B_TYPE && MPEG_Pic_Type == B_TYPE)
          //       iSync_Diff_ms = iSync_Diff_ms * 3 / 5;

       if (iWAVEOUT_Scheduled_Blocks > WAVEOUT_MID_PKTS_CUSHION)
       {
         if (PREV_Pic_Type == P_TYPE && MPEG_Pic_Type == B_TYPE
         || MParse.SlowPlay_Flag > 0)
         {
              iSync_Diff_ms+=2;
              cAudNew = 'L';

              if (iQueVsNext > 6 && iGOPrelative > 4 && iGOPtot > 2)
                 iSync_Diff_ms += 1 + (MParse.SlowPlay_Flag);
         }

         if (iWAVEOUT_Scheduled_Blocks > WAVEOUT_MID_HIGH_PKTS_CUSHION)
         {
               cAudNew = '8';
               if (iWAVEOUT_Scheduled_Blocks > WAVEOUT_HIGH_PKTS_CUSHION)
                   iSync_Diff_ms+=2;
               else
                   iSync_Diff_ms+=1;
         }

         if (MParse.FastPlay_Flag)
         {
               iSync_Diff_ms+=1;
         }
         else
         if (MParse.SlowPlay_Flag > 0)
         {
               cAudNew = 'S';
               iSync_Diff_ms += MParse.SlowPlay_Flag;
         }
       }
       else
       if (MParse.SlowPlay_Flag > 0)
       {
               cAudNew = 'T';
               iSync_Diff_ms+= MParse.SlowPlay_Flag;
       }
       else
       if ((iGOPrelative+1) == iGOPtot && iGOPtot > 4 )
       {
             if (PlayCtl.iFPS_Dec_Pct < 90)
             {
                 cAudNew = '6';
                 iSync_Diff_ms = iSync_Diff_ms * 9 / 10;
             }
       }

       if (PlayCtl.iDrop_B_Frames_Flag > 0
       &&  (PlayCtl.iFPS_Dec_Pct > 160
            || (iWAVEOUT_Scheduled_Blocks > WAVEOUT_MID_MID_PKTS_CUSHION
                && MParse.FastPlay_Flag < 3)))
           PlayCtl.iDrop_B_Frames_Flag--;

    }  // END MIDDLE
  } // END Lock To Audio




  if (DBGflag || iAudioDBG)
  {

     if (cAudState != cAudNew
     ||  MPEG_Pic_Type == I_TYPE
     ||  iGOPrelative  == 6)
     {
         cAudState = cAudNew;
         iTmp1 = sprintf(szBuffer, "D%d   %c%+d q=%04dms:%03df B=%03d f=%02dms %02dms %d.%d.%d",
                          PlayCtl.iDrop_B_Frames_Flag,
                          cAudState, iQueVsNext, 
                                          iWavQue_ms, iWavQue_Frames,
                                                     iWAVEOUT_Scheduled_Blocks, 
                                                iSync_Diff_ms, iPeriod_Adj,
                                        iAudio_Lock, iPlayAudio, iWAV_Init);
         //TextOut(hDC, 0, iMsgPosY, szBuffer, iTmp1);
         ExtTextOut(hDC, 0, iMsgPosY, 
                    0,	// text-output options 
                 NULL,	// optional clipping and/or opaquing rectangle 
             szBuffer, iTmp1,
                 NULL); 	// pointer to array of intercharacter spacing values  
     }
  }

  if (DBGflag)
  {

     sprintf(szBuffer, "SYNC %c %dms,  Render=%dms, CatchUp=%dms\n   WAV=%dblks =%03dms %dB @%d Done=%d, Trig=%d, Diff=%d, Rdy=%d",
                        cAudNew,
                         iSync_Diff_ms, iRenderTime, process.iCatchUp,
                         iWAVEOUT_Scheduled_Blocks, iWavQue_ms,
                         process.iWavQue_Len, process.iWavBytesPerMs,
                         PlayedWaveHeadersCount,
                                       iSleepTrigger_ms, iFrame_Diff_ms, iDDO_Frame_Ready);
     DBGout(szBuffer);
  }




  if (MParse.Stop_Flag) // No delay when stopping
  {
  }
  else
  if (iSync_Diff_ms > 0 )
  {
    //if (iSync_Diff_ms < 20000)
    //{
        if (iSync_Diff_ms > 1001)
            iSync_Diff_ms = 1001;              // Safety Dance

        if (PlayCtl.iFPS_Dec_Pct < 97
        &&  iWAVEOUT_Scheduled_Blocks < WAVEOUT_HIGH_PKTS_CUSHION
        &&  iWAV_Init)
            iSync_Diff_ms = iSync_Diff_ms - 1; // Breathing space

        //else
        //if (PlayCtl.iFPS_Dec_Pct > 120)
        //    iSync_Diff_ms = iSync_Diff_ms + 1; // Rounding factor on fast machines

        if (iSync_Diff_ms > 0)
        {
            if (PlayCtl.iBehindFrames > 0)
                PlayCtl.iBehindFrames--;

            if (iFrame_Diff_ms < 0
            //  ||  (P_Overlay  && iAudio_Lock && !iDDO_Frame_Ready
                   //&& iSync_Diff_ms < iSleepTrigger_ms
            //       )
               )
            {
               // postpone sleep until the next frame decoded
               if (DBGflag)
               {
                  DBGout("   NOSLEEP");
               }
            }
            else
            {
               if (DBGflag)
               {
                   sprintf(szBuffer, " Sleep %dms, Period=%d, Rend=%d\n   WAV=%d %03dms",
                              iSync_Diff_ms, iPeriod_Adj, // iFrame_Period_ms,
                              iFrame_Diff_ms,
                              iWAVEOUT_Scheduled_Blocks, iWavQue_ms);
                   DBGout(szBuffer);
               }
               Sleep(iSync_Diff_ms);  // <=== MOVE THIS INTO SEPARATE THREAD for UpdateOverlay ******
               iSync_Diff_ms = 0;     // process.iCatchUp  = 0;
            }
        }

        PlayCtl.iMaxedOut = 0;
    //} // END-IF > 20000
  }
  else
  if (PlayCtl.iMaxedOut)
      PlayCtl.iMaxedOut = 0;
  else
  {
      // We are behind
      PlayCtl.iBehindFrames++;

    /*
    if (iGOPtot > 5)
        process.iCatchUp  -= iSync_Diff_ms; // We are behind - remember for next time
    if (process.iCatchUp  > iPeriod_Adj)
        process.iCatchUp  = iPeriod_Adj;
    */

    // If WAY behind, skip delta frames 

    if (iSync_Diff_ms <= iDropTrigger_ms // Some frames are more complex than others, so big latitude
    && PlayCtl.iDrop_Behind
    && MPEG_Pic_Structure == FULL_FRAME_PIC // Not supported on non-interlace
    && MPEG_Pic_Type != I_TYPE)          // Not sure if this will handle TOP/BOT field encoded frames
    {
       if (PlayCtl.iBehindFrames > 1)
       {
          PlayCtl.iDropped_Frames++;
          iResult = 1;

               if (DBGflag)
               {
                   sprintf(szBuffer, " DROP.  %dms TRIG=%dms",
                                 iSync_Diff_ms, iDropTrigger_ms);
                   DBGout(szBuffer);
               }
       }
    }
  }

  iRender_TimePrev = iCURR_TIME_ms();

  return iResult;

}

