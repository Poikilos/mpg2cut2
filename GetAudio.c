
#include "global.h"
#include "getbit.h"
#include "GetBit_Fast.h"
 
#include "Audio.h"
#include "AC3Dec\A53_interface.h"
#include "wave_out.h"
#include "MPA_DA.h"
#include "mpalib.h"
#include "mpalib_more.h"

#define GLOBAL 
#include "MPA_HDR.h"

 


//              (RdBFR + MPEG_SEARCH_BUFSZ)
#define MPEG_PTR_BUFF_CHK                      \
while (RdPTR >= RdEOB                         \
   &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL   \
   && ! MParse.Stop_Flag )            \
{                                             \
  Mpeg_READ();                                \
  RdPTR -= MPEG_SEARCH_BUFSZ;                 \
}







#define CONV_A53_PKT_PCM       \
{                              \
  getAudio_size = 0;            \
  while (getbit_iPkt_Len_Remain > 0     \
    &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL  \
    && ! MParse.Stop_Flag && iPlayAudio)        \
  {                                     \
    if (getbit_iPkt_Len_Remain+RdPTR > RdEOB)   \
    {  \
       getAudio_size = ac3_decode_data(RdPTR, RdEOB-RdPTR, getAudio_size); \
       \
       getbit_iPkt_Len_Remain -= RdEOB-RdPTR; \
       Mpeg_READ();          \
       RdPTR = RdBFR;         \
    }                          \
    else                        \
    {  \
       getAudio_size = ac3_decode_data(RdPTR, getbit_iPkt_Len_Remain, getAudio_size);         \
       \
       RdPTR += getbit_iPkt_Len_Remain; \
       getbit_iPkt_Len_Remain = 0;       \
    }                   \
  }                      \
}



//void Got_MPA_PayLoad();



static char *FTType[5] = {
  "48KHz", "44.1KHz", "44.1KHz", "44.1KHz", "44.1KHz"
};

/*
static char *szAC3ChannelDash[9] = {
  "1+1", "1_0", "2_0", "3_0", "2_1", "3_1", "2_2", "3_2", "5_2"
};
*/

static unsigned char PCM_Buffer[MPEG_SEARCH_BUFSZ];
static short     *ptrPCM_Buffer = (short*)PCM_Buffer;
static unsigned int  PCM_SamplingRate;


char *lpTmpMPA;
static unsigned int getbit_MPA_Track;
int iGot_Trk_FMT;


void GetChkPoint()
{
  unsigned int uTmp1;

  if (process.AudioPTS != PTS_NOT_FOUND)
  {
         PTS_2Field( process.AudioPTS, IDC_AUDIO_PTS);
         //memcpy(&gopTC,  &ptsTC, sizeof(ptsTC));
         memcpy(&CurrTC, &ptsTC, sizeof(ptsTC));
  }

  uTmp1 = timeGetTime();
  if (( uTmp1 - PktStats.uPrevTimeUpd) > 500)
  {
          PktStats.uPrevTimeUpd = uTmp1;
          DSP3_Main_TIME_INFO();
          Stats_FPS();
  }

  if (MParse.ShowStats_Flag)
  {
         S100_Stats_Hdr_Main(1);
         S200_Stats_Pic_Main(1);  // Keep stats relevant when Nebula clags up
  }

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


//----------------------------------------------
// NEBULA capture routines drops video frames when CPU hits 100%,
// but audio keeps going, leaving vision jerky or frozen for duration
void Packet_Aud_Inc()
{
    getbit_MPA_Track = 0;

    PktStats.iChk_AudioPackets++;

    if (PktStats.iChk_AudioPackets > PktChk_Audio)
    {
      GetChkPoint();
    }
}




//---------------------------------------------

void WAV_Byte_Swap(unsigned int  uQWord_Len,
                   unsigned char *lp_IN,
                   unsigned char *p_OUT,
                   unsigned int   P_Len)
{
    unsigned char *lpConvIn0, *lpConvIn1, *lpConvEND;
    unsigned char cTMP[4];
    unsigned char *lp_OUT;

    lpConvEND = lp_IN + P_Len;
    lpConvIn0 = lp_IN;

    lp_OUT = (unsigned char*)p_OUT;

    if (uQWord_Len == 0) // 16 bits
    {
        //lpConvIn1 = lp_IN+1;
        while (lpConvIn0 < lpConvEND)
        {
               cTMP[1] =  *lpConvIn0;
               cTMP[0] =  *lpConvIn0;
               lpConvIn0 +=2;
              *(short*)lp_OUT = *(short*)&cTMP[0];
               lp_OUT    +=2;
               //lpConvIn1+=2;
        }
    }
    else
    if (uQWord_Len == 1) // 20 bits
    {
    }
    else
    if (uQWord_Len == 2) // 24 bits - THIS IS JUST A GUESS - PROBABLY WRONG
    {
        lpConvIn1 = lp_IN+2;
        while (lpConvIn1 < lpConvEND)
        {
               cTMP[0] =  *lpConvIn1--;
               cTMP[1] =  *lpConvIn1--;
               cTMP[2] =  *lpConvIn1--;
              *lp_OUT++ = cTMP[0];
              *lp_OUT++ = cTMP[1];
              *lp_OUT++ = cTMP[2];
               lpConvIn1+=3;
        }
    }
}


/*

*/


void WAV_Packet_Warp(void *P_Buffer, int P_Len)
{
// My attempt at Judder reduction doesn't seem to be terribly effective
// so to save some CPU, you can comment out the #define

// #define JUDDER_REDUCTION
#define JUDDER_LIM 1

#ifdef JUDDER_REDUCTION
  int iTmp1, iRef1, iToCross; //, iMin;
  void *P_SrchLim, *P_EndLim, *P_EndBuf, *P_Right;
static int iBefore_Left = 0, iBefore_Right = 0;
#endif

  int iMPA_PCM_Len0, iMPA_PCM_Len1, iMPA_PCM_Len2; 

  if (!iPlayAudio)
    return;

  iMPA_PCM_Len0 = P_Len;

  // Force subsets to quad byte boundary for 16 bit stereo

  if (MParse.FastPlay_Flag == 1)
      iMPA_PCM_Len1 = (iMPA_PCM_Len0 /  6) *  4; // 2 Thirds size // WAS: >>5) * 20;
  else
  if (MParse.FastPlay_Flag == 3)
      iMPA_PCM_Len1 = (iMPA_PCM_Len0 / 12) *  4; // 1 Third size  // WAS: >>6) * 20;
  else
  if (MParse.FastPlay_Flag == 4)
      iMPA_PCM_Len1 = (iMPA_PCM_Len0 / 16) *  4; // 1 Qtr size    // WAS: >>6) * 20;
  else
  {
      iMPA_PCM_Len1 = (iMPA_PCM_Len0 /  8) *  4; // Halve size,
      if (MParse.SlowPlay_Flag == 1)
          iMPA_PCM_Len2 = (iMPA_PCM_Len0 / 12) *  4; // 1 Third size    // WAS: >>6) * 20;
  }


                         /*
                         if (uMPA_Channel_ix == 3) // Mono Experiment
                         {
                            WAV_Byte_Swap(0, (BYTE*)fPCMData, (BYTE*)fPCMData,
                                             iMPA_PCM_Len1);
                         }
                         */



#ifdef JUDDER_REDUCTION

  if ((MParse.FastPlay_Flag
       //&& MParse.FastPart >= 0
       && Frame_Number > 5)
  ||  MParse.SlowPlay_Flag > 0)
  {
      if (MParse.FastPlay_Flag && iMPA_PCM_Len1 > 256)
      {
          (char*)(P_EndBuf) = (char*)(P_Buffer) + iMPA_PCM_Len0 - 4;

          // Reduce amount of judder by looking for zero crossing or low volume point

          P_SrchLim = (char*)(P_Buffer) + iMPA_PCM_Len1;
          P_EndLim  = P_Buffer;
          iRef1 =  *(short*)(P_Buffer);
          for (;;)
          {
             if (P_EndLim >= P_SrchLim)
             {
                //cAudState = ' ';
                //TextOut(hDC, 0, iMsgPosY, "// ", 2); // szBuffer, iTmp1);
                break;
             }

             iTmp1 =  *(short*)(P_EndLim);

             /*
             if ( iTmp1 > JUDDER_LIM && iTmp1 > -JUDDER_LIM)
             {

                //if (cAudState != '-')
                //{
                    cAudState = '-';
                    TextOut(hDC, 0, iMsgPosY, "-", 1); // szBuffer, iTmp1);
                //}

               P_Buffer = P_EndLim;
               break;
             }
             */

             if (iRef1 >= 0)
             {
               if ( iTmp1 <= 0)
               {
                   //cAudState = '-';
                   //TextOut(hDC, 0, iMsgPosY, "-", 1); // szBuffer, iTmp1);
                   P_Buffer = P_EndLim;
                   break;
               }
             }
             else
             {
               if ( iTmp1 > 0)
               {
                   //cAudState = '-';
                   //TextOut(hDC, 0, iMsgPosY, "-", 1); // szBuffer, iTmp1);
                   P_Buffer = P_EndLim;
                   break;
               }
             }

             (char*)(P_EndLim) += 4;
          }

          /*  SMOOTHING MAY CAUSE PROBLEMS
          // Smooth the join a little
          *(short*)(P_Buffer) = (*(short*)(P_Buffer) + iBefore_Left)  /2;
            (char*)(P_Right)  =    (char*)(P_Buffer) + 2;
          *(short*)(P_Right)  = (*(short*)(P_Right)  + iBefore_Right) /2;
          */

          // END OF SUBSET


          // Reduce amount of judder by looking for zero crossing or low volume point
          P_EndLim  = (char*)(P_Buffer) + iMPA_PCM_Len1
                       // - PlayCtl.iAudio_Warp_Accum
                         - 256;
          P_SrchLim = P_EndLim;


          iRef1 =  *(short*)(P_EndBuf);
          iTmp1 =  *(short*)(P_EndLim);
          // To match direction, may need to find an extra zero crossing
          if ((iRef1 >= 0 && iTmp1 >= 0)
          ||  (iRef1 <  0 && iTmp1 <  0))
               iToCross = 1;
          else
               iToCross = 0;

          while (P_EndLim < P_SrchLim) // P_EndBuf)
          {
             iTmp1 =  *(short*)(P_EndLim);
             /*
             if ( iTmp1 > JUDDER_LIM && iTmp1 > -JUDDER_LIM)
             {
                PlayCtl.iAudio_Warp_Accum = (char*)P_EndLim - (char*)P_SrchLim;
                iMPA_PCM_Len1 = (char*)(P_EndLim) - (char*)(P_Buffer);
                break;
             }
             */

             if (iRef1 >= 0)
             {
               if ( iTmp1 <= 0)
               {
                 if (iToCross)
                 {
                    iToCross--;
                    iRef1 = iTmp1;
                 }
                 else
                 {
                    PlayCtl.iAudio_Warp_Accum = (char*)P_EndLim - (char*)P_SrchLim;
                    iMPA_PCM_Len1 = (char*)(P_EndLim) - (char*)(P_Buffer);
                    break;
                 }
               }
             }
             else
             {
               if ( iTmp1 > 0)
               {
                 if (iToCross)
                 {
                    iToCross--;
                    iRef1 = iTmp1;
                 }
                 else
                 {
                    PlayCtl.iAudio_Warp_Accum = (char*)P_EndLim - (char*)P_SrchLim;
                    iMPA_PCM_Len1 = (char*)(P_EndLim) - (char*)(P_Buffer);
                    break;
                 }
               }
             }

             (char*)(P_EndLim) += 4;
          }
          iBefore_Left   = *(short*)(P_EndLim);
          (char*)(P_Right) = (char*)(P_EndLim) + 2;
          iBefore_Right  = *(short*)(P_Right);
      }
  }


#endif  // END JUDDER REDUCTION




  WAV_WIN_Play_Samples (P_Buffer, iMPA_PCM_Len1);


  if (MParse.SlowPlay_Flag > 0   && iPlayAudio)
  {
      if (MParse.SlowPlay_Flag == 1 && Frame_Number > 30)
        WAV_WIN_Play_Samples (P_Buffer, iMPA_PCM_Len2);
      else
      {
        WAV_WIN_Play_Samples (P_Buffer, iMPA_PCM_Len1);
        if (MParse.SlowPlay_Flag > 2 && iPlayAudio)
        {
            WAV_WIN_Play_Samples (P_Buffer, iMPA_PCM_Len1);
            if (MParse.SlowPlay_Flag > 3 && iPlayAudio)
            {
              WAV_WIN_Play_Samples (P_Buffer, iMPA_PCM_Len1);
            }
        }
      }
  }


  if (!MParse.FastPlay_Flag && iPlayAudio)
  {
      P_Buffer = (char *)(P_Buffer) + iMPA_PCM_Len1;
      iMPA_PCM_Len1 = iMPA_PCM_Len0 - iMPA_PCM_Len1;

      WAV_WIN_Play_Samples (P_Buffer, iMPA_PCM_Len1);
      if (MParse.SlowPlay_Flag > 1)
      {
          WAV_WIN_Play_Samples (P_Buffer, iMPA_PCM_Len1);
          if (MParse.SlowPlay_Flag > 2 && iPlayAudio)
          {
              WAV_WIN_Play_Samples (P_Buffer, iMPA_PCM_Len1);
              if (MParse.SlowPlay_Flag > 3 && iPlayAudio)
              {
                  WAV_WIN_Play_Samples (P_Buffer, iMPA_PCM_Len1);
              }
          }
      }
  }

}




void GetDelay()
{
  unsigned int uVideoPTS;

  if (process.AudioPTS == PTS_NOT_FOUND
  ||  process.VideoPTS == PTS_NOT_FOUND)
  {
      //process.DelayPTS = 0;
      //process.Delay_Calc_Flag = 1;
  }
  else
  {
    process.Delay_Calc_Flag = 0;

    // Adjust for B-frames to be displayed before I-frame
    if (process.Mpeg2_Flag)
    {

      if (gopTC.VideoPTS > process.uGOPbefore)
          uVideoPTS = gopTC.VideoPTS - process.uGOPbefore;
      else
          uVideoPTS = 0;
    }
    else
       uVideoPTS = gopTC.VideoPTS;

    if (uVideoPTS > process.AudioPTS)
    {
          process.DelayPTS = uVideoPTS - process.AudioPTS;
          process.Delay_Sign[0] = '-';
    }
    else
    {
         process.DelayPTS = process.AudioPTS - uVideoPTS;
         process.Delay_Sign[0] = '+';
    }

    process.Delay_ms = process.DelayPTS / 45; //90;

    if (process.Delay_ms > 10000) // Suppress if unrealistic delay
    {
       process.Delay_Sign[0] = '?';
       process.Delay_ms      = 0;    process.DelayPTS      = 0;
       process.Delay_Calc_Flag = 1;  // Try Again Later
    }
  }


}



void PTS_Audio_Analysis()
{
  if (getbit_MPA_Track == iAudio_SEL_Track)
  {
      process.AudioPTS = CandidatePTS;

      if (process.Delay_Calc_Flag)
          GetDelay();

  }


#ifdef DBG_RJ
                 if (DBGflag  && CandidatePTS > 0xFF000000)
                 {
                     sprintf(szBuffer, "PTS %02d  x%X\n\nStreamId=x%04X",  CandidatePTS, CandidatePTS, getbit_MPA_Track);
                     MessageBox(hWnd, szBuffer, "Mpg2Cut2 - BUG !",
                                              MB_ICONSTOP | MB_OK);
                 }
#endif

                 PTS_Flag[1] = 'A';
                 if (CandidatePTS < Prev_PTS[getbit_MPA_Track])
                 {
                   if (iCtl_MultiAngle)
                   {
                     if (process.Action == ACTION_RIP)
                         getbit_iDropPkt_Flag = 1;
                   }
                   else
                   if (process.Action == ACTION_FWD_GOP)
                   {
                     PTS_Err_Msg(getbit_AUDIO_ID, CandidatePTS, "MPG Audio");
                   }

                 }
                 if (! getbit_iDropPkt_Flag)
                     Prev_PTS[getbit_MPA_Track] = CandidatePTS;
}



static unsigned int getbit_AC3_Track;
int iMPA_PCM_Len1;

float fPCMData[1152*4*2];

unsigned char *RdPTR_Peek;

static int getAudio_size;


void Audio_Fallback_Chk()
{
  int i;

  // Fallback to auto selection if current selection not working
  if ( iAudio_SEL_Track != TRACK_AUTO
  &&  !PlayCtl.iAudio_SelStatus
  &&   PlayCtl.iGOP_Ctr > 9
  &&   MParse.FastPlay_Flag <= 0)
  {
      strcpy(szMsgTxt, "Auto Track Reset");
      DSP1_Main_MSG(0,1);
      for  (i=0;  i<CHANNELS_MAX;  i++)
      {
         mpa_Ctl[i].rip = 0;
         SubStream_CTL[iGot_Trk_FMT][i].rip = 0;
      }
      //iAudio_SEL_Track = TRACK_AUTO;
      Set_AudioTrack(TRACK_AUTO);
      PlayCtl.iAudio_SelStatus = 1;
  }
}







//----------------------------------------------------

  unsigned uBitsPerSample, uChannel_ix, uChannels, uQWord_Len;
  int iDecodeOK_Flag;

void PS1_Convert()
{
  unsigned char *lpConvEND, *lpConvOUT, *lpConvIN,
                *lpConvSPLIT, *lpTemp;
  int iUpTo, iLPCM_Len;
  int iTmp1, iTmp2;

  if (iGot_Trk_FMT == FORMAT_LPCM)  // LPCM
               {
                  iDecodeOK_Flag = 1;
                  lpConvEND = RdPTR + getbit_iPkt_Len_Remain;
                  lpConvOUT = &AC3Dec_Buffer[PlayCtl.iPCM_Remainder];
                  iUpTo     = getbit_iPkt_Len_Remain + PlayCtl.iPCM_Remainder;
                  iLPCM_Len = getbit_iPkt_Len_Remain;

                  // Check for end of current block
                  if (lpConvEND > RdEOB)
                  {
                      lpConvIN = lpConvOUT;
                      iTmp1 = RdEOB-RdPTR;
                      memcpy(lpConvOUT, RdPTR, iTmp1);
                      lpTemp = lpConvOUT + iTmp1;
                      getbit_iPkt_Len_Remain -= iTmp1; 
                      
                      Mpeg_READ();            
                      RdPTR = RdBFR; 
                      memcpy(lpTemp, RdPTR, getbit_iPkt_Len_Remain);
                  }
                  else
                    lpConvIN = RdPTR;

                  getAudio_size = iUpTo & 0xFFFFC;
                  lpConvSPLIT = &AC3Dec_Buffer[getAudio_size];
                  //iTmp3 = PlayCtl.iPCM_Remainder;
                  iTmp2 = PlayCtl.iPCM_Remainder & 1;
                  PlayCtl.iPCM_Remainder = iUpTo - getAudio_size;

                  // Finish split word from PREVIOUS packet
                  // using  orphaned odd byte from CURRENT packet
                  if (iTmp2)
                  {
                     *(lpConvOUT-1) = *lpConvIN++;
                       lpConvOUT++;
                       iLPCM_Len--;
                  }


                  WAV_Byte_Swap(uQWord_Len, lpConvIN, lpConvOUT, 
                                iLPCM_Len);

                  // Prepare orphaned odd byte from CURRENT packet
                  //if (PlayCtl.iPCM_Remainder & 1)
                  //{
                  //     lpConvOUT   +=  getbit_iPkt_Len_Remain;
                  //   *(lpConvOUT-1) = *lpConvOUT;
                  //}

               }
               else
               { 
                 if (!byAC3_Init)
                 {
                      InitialAC3();
                 }
                 iDecodeOK_Flag = byAC3_Init;

                 if (byAC3_Init)
                 {
                    CONV_A53_PKT_PCM

                    if (AC3_Err_Txt[0])
                    {
                       iMsgLife = 3;
                       strcpy(szAudio_Status, AC3_Err_Txt);
                       if (MParse.ShowStats_Flag)
                          Stats_Volume_Boost();
                       AC3_Err_Txt[0] = 0;
                    }
                    PCM_SamplingRate = A53_sampling_rate;
                    iDecodeOK_Flag = byAC3_Init;
                 }
               }

               if ( ! PlayCtl.iAC3_Attempted && iDecodeOK_Flag)
               {
                   PlayCtl.iAC3_Attempted = 1;

                   if (iWAV_Init  
                   && (   WAVEOUT_SampleFreq    != PCM_SamplingRate
                       || WAVEOUT_BitsPerSample != uBitsPerSample
                       || WAVEOUT_Channels      != uChannels
                      ))
                   {
                       WAV_WIN_Audio_close();
                   }

                   if (!iWAV_Init)
                   {
                        WAVEOUT_SampleFreq    = PCM_SamplingRate;
                        WAVEOUT_BitsPerSample = uBitsPerSample;
                        WAVEOUT_Channels      = uChannels;
                        iPlay_SrcChannels     = uChannels;

                        WAV_Set_Open();
                   }
               }

               if (-wav.delay >  getAudio_size)
                    wav.delay += getAudio_size;
               else
               {
                 // Cannot adjust when wav.delay is positive
                 if (wav.delay > 0)
                     wav.delay = 0;

                 /*
                 if (SRC_Flag)
                   Wavefs44(wav.file, getAudio_size+wav.delay, AC3Dec_Buffer-wav.delay);
                 else
                   fwrite(AC3Dec_Buffer-wav.delay, getAudio_size+wav.delay, 1, wav.file);
                 */

                 if ((iDecodeOK_Flag // && iGot_Trk_FMT == FORMAT_AC3
                                 //|| iGot_Trk_FMT == FORMAT_LPCM
                                 )
                 && iWAV_Init && iPlayAudio  
                 && MParse.FastPlay_Flag <= MAX_WARP )
                 {
                     iMPA_PCM_Len1 = getAudio_size + wav.delay;
                     WAV_Packet_Warp( (AC3Dec_Buffer-wav.delay),
                                       iMPA_PCM_Len1);


                    if (iGot_Trk_FMT == FORMAT_LPCM) // Shuffle remainder to start of buffer for next time LPCM
                    {
                        *(UNALIGNED DWORD*)(&AC3Dec_Buffer) 
                      = *(UNALIGNED DWORD*)(lpConvSPLIT);
                    }

                 }
                 wav.size += getAudio_size+wav.delay;
                 wav.delay = 0;
               }
}



void Private_PID_LookUp()
{
     getbit_AC3_Track = 0;
     while (SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uPID  != uGot_PID
            && getbit_AC3_Track <= CHANNELS_MAX)
     {
        if(  SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uPID == 0)
             SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uPID  = uGot_PID;
        else
             getbit_AC3_Track++;
     }
}





void Got_PrivateStream() 
{
  int iLPCM_Attr;
  unsigned uChannel_ix, uBitRate, uBitRate_ix, uSampleRate, uSampleRate_ix;
  int i, iTmp1, iTmp3;
  //unsigned char cTmp1, cTmp2;

  iDecodeOK_Flag = byAC3_Init;

  getbit_StreamID = getbit_input_code;
  process.SkipPTS = 0;

  getbit_iPkt_Len_Remain     = Get_Short();

  // Transport stream PES length does not apply - calc from TS packet
  if (MParse.SystemStream_Flag < 0)
      getbit_iPkt_Len_Remain = RdEndPkt - RdPTR;
  else
  {
      RdEndPkt = RdPTR + getbit_iPkt_Len_Remain; RdEndPkt_4 = RdEndPkt-4; RdEndPkt_8 = RdEndPkt-8;
  }


  getbit_PES_Gate         = Get_Byte(); //
  getbit_PES_HdrFld_Flags = Get_Byte(); // PES header field flags
  getbit_iPkt_Hdr_Len     = Get_Byte();
  getbit_iPkt_Hdr_Len_Remaining = getbit_iPkt_Hdr_Len;

  if (DBGflag)
  {
      sprintf(szBuffer,"      PS AUDIO Len=%d=x%04X  PS2Audio=%d",
                            getbit_iPkt_Len_Remain, getbit_iPkt_Len_Remain, iInPS2_Audio);
      DBGout(szBuffer);
  }

  if (getbit_PES_HdrFld_Flags >= 0x80) // Is there a PTS ?
  {
      CandidatePTS = Get_PTS(0); // Keep until we decide this is selected track

      PTS_Flag[2] = '1';

  }

  RdPTR += getbit_iPkt_Hdr_Len_Remaining;

  getbit_iPkt_Len_Remain -= (getbit_iPkt_Hdr_Len+3);  // 4=previous 3 + next 1

  if (MParse.SystemStream_Flag <= 0  // Transport Stream
  ||  getbit_StreamID != PRIVATE_STREAM_1)
      getbit_AUDIO_ID    = SUB_AC3;
  else
  {
      getbit_SubStreamID = Get_Byte();
      getbit_iPkt_Len_Remain--;
  }

  getbit_AUDIO_ID    = getbit_SubStreamID;

  if (getbit_StreamID == PRIVATE_STREAM_1)  // PS1
  {
      if (! PktStats.iPS1_Packets)
            SetDlgItemText(hStats, IDC_AUDIO_PS1, "PS1" );
      PktStats.iPS1_Packets++;
      //iAudio_Trk_FMT[6] = FORMAT_PS1;
  }
  else
  {  // PS2 Audio
     if (! PktStats.iPS2_Packets)
           SetDlgItemText(hStats, IDC_AUDIO_PS2, "PS2" );
     PktStats.iPS2_Packets++;
     //iAudio_Trk_FMT[7] = FORMAT_PS2;
     getbit_AUDIO_ID = SUB_AC3; // JUST CHUCK DATA AT THE AC3 DECODER
  }

  // if (rj_Audio_Code==Audio_Dunno)
  //       rj_Audio_Code = getbit_AUDIO_ID ;  //RJTRK

  getbit_AC3_Track = 99;
  AC3_DDPLUS = 0;

  // Is it an AC3 substream ?
  if (getbit_AUDIO_ID >= SUB_AC3
  &&  getbit_AUDIO_ID < (SUB_AC3 +CHANNELS_MAX))
  {
      iGot_Trk_FMT = FORMAT_AC3;
      if (MParse.SystemStream_Flag < 0) // Transmission stream (TS or PVA) ?
          Private_PID_LookUp();
      else
          getbit_AC3_Track = getbit_AUDIO_ID - SUB_AC3;
  }
  else
  // Is it a DTS substream ?
  if (getbit_AUDIO_ID >= SUB_DTS
  &&  getbit_AUDIO_ID < (SUB_DTS +CHANNELS_MAX))
  {
      iGot_Trk_FMT = FORMAT_DTS;
      if (MParse.SystemStream_Flag < 0) // Transmission stream (TS or PVA) ?
          Private_PID_LookUp();
      else
          getbit_AC3_Track = getbit_AUDIO_ID - SUB_DTS;
  }
  else
  // Is it a Sub-Title substream ?
  if (getbit_AUDIO_ID >= SUB_SUBTIT
  &&  getbit_AUDIO_ID < (SUB_SUBTIT +CHANNELS_MAX))
  {
      iGot_Trk_FMT = FORMAT_SUBTIT;
      if (MParse.SystemStream_Flag < 0) // Transmission stream (TS or PVA) ?
          Private_PID_LookUp();
      else
          getbit_AC3_Track = getbit_AUDIO_ID - SUB_SUBTIT;
  }
  else
  // Is it  DD+ ?
  if (getbit_AUDIO_ID >= SUB_DDPLUS
  &&  getbit_AUDIO_ID < (SUB_DDPLUS +CHANNELS_MAX))
  {
      iGot_Trk_FMT = FORMAT_DDPLUS;
      AC3_DDPLUS = 1;
      if (MParse.SystemStream_Flag < 0) // Transmission stream (TS or PVA) ?
          Private_PID_LookUp();
      else
          getbit_AC3_Track = getbit_AUDIO_ID - SUB_DDPLUS;
  }
  else
  // Is it an LPCM substream ?
  if (getbit_AUDIO_ID >= SUB_PCM
  &&  getbit_AUDIO_ID < (SUB_PCM +CHANNELS_MAX-4))
  {
      iGot_Trk_FMT = FORMAT_LPCM;
      if (MParse.SystemStream_Flag < 0) // Transmission stream (TS or PVA) ?
          Private_PID_LookUp();
      else
          getbit_AC3_Track  = getbit_AUDIO_ID - SUB_PCM;

      iTmp1 = Get_Byte(); // No of Frames starting in this packet (does not include tail from previous packet, but does include last partial frame in this packet)

      iPS_Frame_Offset = Get_Short(); // Offset to start of 1st frame for this PTS = first access unit pointer

      iTmp3 = Get_Byte(); // Flags:
                   //    audio emphasis on-off        1 bit
                   //    audio mute on-off            1 bit
                   //    reserved                     1 bit
                   //    audio frame number           5 bits (within Group of Audio frames).

      iLPCM_Attr = Get_Byte(); // LPCM Attributes:
                   // quantization word length code   2 bits  0 = 16, 1 = 20, 2 = 24, 3 = reserved
                   // audio sampling frequency        2 bits  (48khz = 0, 96khz = 1)
                   // reserved                        1 bit
                   // number of audio channels - 1    3 bit  (e.g. stereo = 1)

      uQWord_Len = iLPCM_Attr >>6;
      uBitsPerSample = (uQWord_Len + 4) * 4;


      if ((iLPCM_Attr & 0x30) == 0x10)
      {
          PCM_SamplingRate = 96000;
      }
      else
      {
          PCM_SamplingRate = 48000;
      }

      SubStream_CTL[FORMAT_LPCM][getbit_AC3_Track].uBitRate_ix = uBitsPerSample;
      SubStream_CTL[FORMAT_LPCM][getbit_AC3_Track].uSampleRate = PCM_SamplingRate;
     

      uChannel_ix      = (iLPCM_Attr & 0x03) + 1;
      SubStream_CTL[FORMAT_LPCM][getbit_AC3_Track].uChannel_ix = uChannel_ix;

      iTmp1 = Get_Byte(); // Dynamic Range control (0x80 if off)
                          //      Dynamic Range X     3 bits
                          //      Dynamic Range Y     5 bits
                          // linear gain = 2^(4-(X+(Y/30)))
                          //  in dB gain = 24.082 - 6.0206 X - 0.2007 Y

      getbit_iPkt_Len_Remain -= 6;

  }
  else
  {
      iGot_Trk_FMT     = FORMAT_UNK;
      getbit_AC3_Track = CHANNELS_MAX+1; // getbit_AUDIO_ID;
  }

  SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].cStream 
       = (unsigned char)(getbit_AUDIO_ID);
  

  if (iGot_Trk_FMT != FORMAT_LPCM)
  {
      uBitsPerSample  = 16;
      uChannels       = 2; // Internal decoder always outputs stereo
  }

  Audio_Fallback_Chk();

  // Is it a playable format ?
  if (getbit_AC3_Track < CHANNELS_MAX
  &&  iGot_Trk_FMT  < FORMAT_SUBTIT)
  {
      // Auto detect Audio track
      if (iAudio_SEL_Track == TRACK_AUTO
      &&  iGot_Trk_FMT  <= FORMAT_DDPLUS
      &&  (iWant_Aud_Format == iGot_Trk_FMT ||  ! iWant_Aud_Format)
      )
      {
          iAudio_SEL_Track  =  getbit_AC3_Track;
          iAudio_SEL_Format =  iGot_Trk_FMT;
      }

      // PTS stuff
      Prev_PTS_IX = getbit_AC3_Track  + 8;
      if (getbit_PES_HdrFld_Flags >= 0x80) // PTS present ?
      {
          if (CandidatePTS < Prev_PTS[Prev_PTS_IX])
          {
              if (iCtl_MultiAngle)
              {
                 if (process.Action == ACTION_RIP)
                     getbit_iDropPkt_Flag = 1;
              }
              else
              if (process.Action == ACTION_FWD_GOP)
              {
                  PTS_Err_Msg(Prev_PTS_IX, CandidatePTS, "PS Audio");
              }
          }

          if (getbit_iDropPkt_Flag)
            return;
          else
              Prev_PTS[Prev_PTS_IX] = CandidatePTS;
      } // END-IF PTS processing


      if (getbit_AC3_Track == iAudio_SEL_Track)  // is it the one to play?
      {
         PlayCtl.iAudio_SelStatus = 1;

         if (getbit_PES_HdrFld_Flags >= 0x80) // PTS present ?
         {
             process.AudioPTS = CandidatePTS;

             if (process.Delay_Calc_Flag)
                 GetDelay();
         }

         process.PES_Audio_CRC_Flag = Mpeg_PES_Byte1 & 0x02;
         if (process.iAudio_InterFrames)
         {
                  process.iAudio_Interleave = process.iAudio_InterFrames;
                  process.iAudio_InterFrames = 0;
         }
      }

#ifdef DBG_RJ
          if (DBGflag  && CandidatePTS > 0xFF000000)
          {
                sprintf(szBuffer, "PTS %02d  x%X\n\nStreamId=x%04X\n\nSubStream=x%04x",  CandidatePTS, CandidatePTS, getbit_StreamID, getbit_SubStreamID);
                MessageBox(hWnd, szBuffer, "Mpg2Cut2 - BUG !",
                                            MB_ICONSTOP | MB_OK);
          }
#endif

      // LOCATE TO FIRST SYNCWORD

      // Skip control info at start of packet
      if (iGot_Trk_FMT == FORMAT_AC3
      ||  iGot_Trk_FMT == FORMAT_DTS      // DD-DTS  decoding is experimental
      ||  iGot_Trk_FMT == FORMAT_DDPLUS)  // DD-PLUS decoding is experimental
      {
          if (MParse.SystemStream_Flag > 0
          &&  getbit_StreamID == PRIVATE_STREAM_1)  // PS1
          {
              RdPTR += 3;  getbit_iPkt_Len_Remain -= 3; // Skip syncword count and first offset pointer

              // TODO: Theoretically we could use the offset pointer to avoid scanning later
              //       BUT - Sometime the offset pointer is WRONG !
              MPEG_PTR_BUFF_CHK                  // skip forward, reading more if needed
          }
      }


      // TODO:

      // IF   this code was changed to avoid using Get_Byte() / Get_Short()
      // THEN it could peek ahead at the AC3 header
      // TO   check for format changes


      // getbit_AC3_1stTime = 0;

      // Is this the first time we've hit this track ?
      if ( (    !SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].rip    // &&  MParse.Rip_Flag
            // &&  !iAudio_Trk_FMT[getbit_AC3_Track]
            )
      // OR track type has changed
      //||  iAudio_Trk_FMT[getbit_AC3_Track] != iGot_Trk_FMT
      )
      {
         //getbit_AC3_1stTime = 1;
         iAudio_Trk_FMT[getbit_AC3_Track] = iGot_Trk_FMT;
         //iAudio_Track_Stream[getbit_AC3_Track] = getbit_AUDIO_ID;

         process.iAudio_PS1_Found = 1; 

         if (iGot_Trk_FMT == FORMAT_LPCM)  // LPCM
         {
            iTmp1 = iPS_Frame_Offset - 4;
            if (iTmp1 > 0)
            {
               RdPTR                  += iTmp1; // Skip to start of audio frame
               getbit_iPkt_Len_Remain -= iTmp1;
               MPEG_PTR_BUFF_CHK
            }
         }
         else
         if (iGot_Trk_FMT == FORMAT_AC3
         ||  iGot_Trk_FMT == FORMAT_DTS      // DD-DTS  is only a guess
         ||  iGot_Trk_FMT == FORMAT_DDPLUS)  // DD-PLUS is only a guess
         {
             getbit_input_code = Get_Byte();
             getbit_input_code = (getbit_input_code & 0xff)<<8 | Get_Byte();

             // Look for AC3 syncword
             i = 0;
             while (getbit_input_code != 0xb77  // AC3 sync sentinel ??
               &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
               && ! MParse.Stop_Flag )
             {
                 getbit_input_code = (getbit_input_code & 0xff)<<8 | Get_Byte();
                 i++;
             }
             iPS_Frame_Offset = i;

             if (MParse.ShowStats_Flag)
             {
                 sprintf(szBuffer, "%d", iPS_Frame_Offset);
                 SetDlgItemText(hStats, IDC_AC3_OFFSET, szBuffer);
             }

             Get_Short();  // skip 2 bytes

             
             if (iGot_Trk_FMT == FORMAT_DTS)      // DTS info is experimental
             {

               Get_Short();  // skip 2 bytes
               iAC3_Attr    = Get_Bits(32); // get 4 bytes

               if (iAudioDBG)
               {
                 uChannel_ix    = (iAC3_Attr >>14) &0x0F; // 6bits, but we only use 4bits
                 uSampleRate_ix = (iAC3_Attr >>10) &0x0F; // 4bits
                 uBitRate_ix    = (iAC3_Attr>>5) & 0x1F;  // 5bits
               }
               else
               {
                 uChannel_ix    =  7; // DUMMY VALUE
                 uSampleRate_ix = 14; // DUMMY VALUE 
                 uBitRate_ix    = 11; // DUMMY VALUE 
               }

               uBitRate       = dca_bit_KRates[uBitRate_ix];
               uSampleRate    = dca_sample_rates[uSampleRate_ix];

               RdPTR -= 10; 
             }  
             else             
             {
               iAC3_Attr    = Get_Byte();

               uSampleRate_ix = iAC3_Attr >>6;  // implied from frame size code       
               uSampleRate    = uAC3_SampleRate[uSampleRate_ix];

               uBitRate_ix    = (iAC3_Attr>>1) & 0x1f;
               uBitRate       =  iAC3KRate[uBitRate_ix];

                                 Get_Byte();
               uChannel_ix    = (Get_Byte()>>5) & 0x07;
               RdPTR -= 7; 
             }

             getbit_iPkt_Len_Remain -= i;

             if (SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uBitRate    != uBitRate
             ||  SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uChannel_ix != uChannel_ix
             ||  SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uSampleRate != uSampleRate)
             {
                 SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uSampleRate    = uSampleRate;
                 SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uSampleRate_ix = uSampleRate_ix;
                 SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uBitRate       = uBitRate;
                 SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uBitRate_ix    = uBitRate_ix;
                 SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uChannel_ix    = uChannel_ix;

                 if (getbit_MPA_Track == iAudio_SEL_Track
                 &&  byAC3_Init)
                 {
                    byAC3_Init = 0;
                 }
             }


         } // end-if AC3


         if  (getbit_AC3_Track == iAudio_SEL_Track)  // is it the one to play?
         {
             // Categorize track for optional volume boosting         
             iVol_PREV_Cat = iVol_Boost_Cat;
             if (iGot_Trk_FMT == FORMAT_AC3
             ||  iGot_Trk_FMT == FORMAT_DTS     // DD-DTS  is only experimental
             ||  iGot_Trk_FMT == FORMAT_DDPLUS  // DD-PLUS cannot really be decoded
                )
             {
                iVol_PREV_Cat = iVol_Boost_Cat;
                if (uChannel_ix > 2 // Better than stereo ?
                ||  iGot_Trk_FMT > FORMAT_AC3) // DTS or DD+
                {
                    iVol_Boost_Cat = FORMAT_DTS;
                }
                else
                {
                    iVol_Boost_Cat = FORMAT_AC3;
                } // end primitive AC3
             } // endif A52 types
             else
             if (iGot_Trk_FMT == FORMAT_LPCM)
             {
                 if (uSampleRate >= 48000)
                     iVol_Boost_Cat = FORMAT_LPCM;
                 else
                     iVol_Boost_Cat = FORMAT_MPA;
             } // endif LPCM

             if (iVol_PREV_Cat != iVol_Boost_Cat)
                 VOL309_Boost_Cat_Begin();

             if (iPlayAudio)
             {
                 /* CONVERSION TO WAV PCM NO LONGER SUPPORTED
                 sprintf(szBuffer, "%s AC3 T%02d %sch %dKbps %s.wav", szOutput, iAudio_SEL_Track+1,

                 szAC3ChannelDash[SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uChannel_ix], 
                 iAC3KRate[SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].uBitRate_ix], 
                 FTType[SRC_Flag]);
                 strcpy(wav.filename, szBuffer);
                 wav.file = fopen(szBuffer, "wb");
                 fwrite(WAVHeader, sizeof(WAVHeader), 1, wav.file);
                 */

                wav.delay = ((CandidatePTS-process.VideoPTS)/45) * 192;

                /*
                if (SRC_Flag)
                {
                      DownWAV(wav.file);
                      InitialSRC();
                }
                */

                 if (wav.delay > 0)
                 {
                      /*
                      if (SRC_Flag)
                          wav.delay = ((int)(0.91875*wav.delay)>>2)<<2;
                      for (i=0; i<wav.delay; i++)
                        fputc(0, wav.file);
                      */
                      if (wav.delay < 1920000 
                      &&  iCtl_Out_Align_Audio && iCtl_Out_Parse)
                          wav.size += wav.delay;

                      wav.delay = 0;
                  }

                  //-------------------------------------------
                  
                  //else
                  //  SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].rip = 1;
             } // endif iPlayAudio
         } // END-IF Selected format for playing ?
         //else
          SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].rip = 1;

      } // END-IF 1st Time Hit This Track



      // Play this track ?
      if (iPlayAudio)
      {
         if (getbit_AC3_Track == iAudio_SEL_Track)  // is it the one to play?
         {
            if (SubStream_CTL[iGot_Trk_FMT][getbit_AC3_Track].rip)  // Has this track been setup ?
            {
               PS1_Convert();
            } // ENDIF Track set up ?

         } // ENDIF Play this track ?

      }  // ENDIF iPlayAudio

  } // ENDIF AC3 SUB-STREAM-ID

  else
  if ((getbit_AUDIO_ID & 0xF0) == SUB_SUBTIT)
  {
       PktStats.iSubTit_Packets++;
  } // ENDIF Sub-Title SUB-STREAM-ID

  else
  if (getbit_StreamID == PRIVATE_STREAM_2)
  {
      if (RdPTR < (RdEOB_8))
      {
          sprintf(szBuffer, "%x.%08x %d.%d",
                  getbit_SubStreamID, *(int*)(RdPTR),
                              getbit_iPkt_Hdr_Len_Remaining, getbit_iPkt_Len_Remain);
          SetDlgItemText(hStats, IDC_PS2_STATUS, szBuffer );
      }

      if (++getbit_VOBCELL_Count == 2)
      {
           RdPTR  += 17;
           getbit_VOB_ID  = /*(unsigned short)*/Get_Short();  //RJ CAREFUL OF CAST
           getbit_CELL_ID = /*(unsigned short)*/Get_Short();  //      "

           //RdPTR  += getbit_iPkt_Len_Remain - 29;
           getbit_iPkt_Len_Remain -=21;


           if (process.Action == ACTION_RIP
           &&  iWant_VOB_ID < 0)
               iWant_VOB_ID = getbit_VOB_ID;

           sprintf(szBuffer, "Vob%d Cell%d", getbit_VOB_ID, getbit_CELL_ID);;
           SetDlgItemText(hStats, IDC_VOB_ID, szBuffer);
      }
  } // END-IF PS2 NON-AC3 substream
}


unsigned char cMPA_Hdr[4];


//----------------------------------------------------------
void Got_MPA_Hdr()
{
  int  iRC;
  char cFrameOK;
  char *lpHzFull;

  if (uMPA_kBitRate < 100) // Don't abbreviate units when potential confusion
      lpHzFull = &"Hz";
  else
      lpHzFull = &"";
  
  sprintf( mpa_Ctl[getbit_MPA_Track].desc, "mp%d%s %dk %s %dk%s",
             uMPA_Layer,
             MPA_EXTN_ABBR[uMPA_25_LSF],
             uMPA_kBitRate, //(uMPA_Sample_Hz/1000),
             MPG_CH_MODE_ABBR[uMPA_Channel_ix],
             (uMPA_Sample_Hz / 1000),
             lpHzFull);

  //if (iMPA_FrameLen < 128) // Check for cruddy control data
  //    iMPA_FrameLen = 128;

  
  // Validate frame structure if requested
  iMPA_FrameOK = 1;
  //if (uMPA_Layer < 3) // dunno about mp3
  {
      RdPTR_Peek = RdPTR + iMPA_FrameLen - 4;
      if (RdPTR_Peek < RdEndPkt)
      {
         if (RdPTR_Peek < RdEOB)
         {
            // if (*RdPTR_Peek != 0xFF) // Points to another Syncword ?
            if (*(UNALIGNED short*)RdPTR_Peek != *(UNALIGNED short*)&cMPA_Hdr[0]) // Points to similar Syncword ?
                  iMPA_FrameOK = 0;
         }
      }
  }


  if (MParse.ShowStats_Flag)
  {
     if (StatsPrev.iFrameLen != iMPA_FrameLen
     ||  StatsPrev.iFrameOK  != iMPA_FrameOK)
     {
         if (iMPA_FrameOK)
             cFrameOK = 0;
         else
             cFrameOK = 'Z';
         sprintf(szBuffer, "%d%c", iMPA_FrameLen, cFrameOK);
         SetDlgItemText(hStats, IDC_AC3_OFFSET, szBuffer);
         StatsPrev.iFrameLen = iMPA_FrameLen;
         StatsPrev.iFrameOK  = iMPA_FrameOK;
     }
  }



  if (iMPA_FrameOK
  &&  uMPA_Layer > 1 && uMPA_Layer < 4) // A format we can decode ?
  {
       mpa_Ctl[getbit_MPA_Track].uLayer         = uMPA_Layer;
       mpa_Ctl[getbit_MPA_Track].uMPA_Sample_Hz = uMPA_Sample_Hz;
     //process.uMPA_Sample_Hz[getbit_MPA_Track] = uMPA_Sample_Hz;
       process.uMPA_Mpeg_Ver[getbit_MPA_Track]  = uMPA_Mpeg_Ver;

     if (iAudio_SEL_Track == getbit_MPA_Track)   /*|| !MPA_Flag*/
     {
        process.PES_Audio_CRC_Flag = Mpeg_PES_Byte1 & 0x02;

        // Categorize the select audio track for optional boosting
        iPlay_SrcChannels     = uMPA_Channels; 

        // We may ned to reset the boost
        iVol_PREV_Cat = iVol_Boost_Cat;
        if (uMPA_Sample_Hz >= 48000)
            iVol_Boost_Cat = 7;
        else
            iVol_Boost_Cat = FORMAT_MPA;

        if (iVol_PREV_Cat != iVol_Boost_Cat)
            VOL309_Boost_Cat_Begin();
               
        if (MParse.Rip_Flag  &&  iPlayAudio)
        {
           mpa_Ctl[getbit_MPA_Track].rip = 1;

           if (! iMPAdec_Init) // Are we yet to try loading MPA decoder ?
           {
              if (iMPALib_Status < 0)
                   MPALib_Init(NULL);

              if(byMPALib_OK)  // Did it load OK ?
                   MPAdec.mlInit(&MPAdec.mp, (1<<15)-1);

              MPAdec.dwFree=1152*4*2;
              lpTmpMPA = (char*)&AC3Dec_Buffer;

              iMPAdec_Init=1; // We have tried to load
           }

           if (byMPALib_OK)
           {
             // Have the audio atributes changed ?
             if (iWAV_Init  
             && (   WAVEOUT_SampleFreq    != uMPA_Sample_Hz
                 || WAVEOUT_BitsPerSample != 16   // Hard coded 16bit samples from decoder
                 || WAVEOUT_Channels      != uMPA_Channels
                ))
             {
                   WAV_WIN_Audio_close();
             }

             if (!iWAV_Init)
             {                 
               WAVEOUT_SampleFreq    = uMPA_Sample_Hz;
               WAVEOUT_BitsPerSample = 16;  // Hard coded 16bit samples from decoder
               WAVEOUT_Channels      = uMPA_Channels; 

              //if (iPlayAudio)
              {
                iRC = WAV_Set_Open();

                if (iRC == 0)
                {
                  iWAV_Init = 1;

                  if (process.Delay_Sign[0] == '-'
                      && iCtl_Out_Align_Audio && iCtl_Out_Parse)
                  {
                     PktStats.iAudDelayBytes = (process.Delay_ms * uMPA_Sample_Hz)
                                             / 500 ;
                     if (uMPA_Channel_ix != 3)
                         PktStats.iAudDelayBytes *= 2;
                  } // endif adjust for delay
                } // endif opened OK
              } // endif iPlayAudio (REMOVED)
             } // endif Need to open WAV output
           } // endif MPAlib OK
        } // endif need sound
     } // ENDIF  Selected Track ?
     else
       mpa_Ctl[getbit_MPA_Track].rip = 1;

  } // ENDIF  Frame looks OK

}



#include "MPA_SIZE.h"   // Calculates uMPA_FrameLen

//----------------------------------------------------------
void Chk_MPA_Hdr()
{
  // Accidental bit patterns may mimic MPA syncword bit
  // Try to filter out such accidents
  // by doing some analysis of the other bit settings 

  if (cMPA_Hdr[1] &0x10)   // Mpeg2.5 extension steals low-order bit of syncword
  {
      uMPA_Mpeg25   = 0;
      uMPA_25_LSF   = 0;
  }
  else
  {
      uMPA_Mpeg25   = 1;
      uMPA_25_LSF   = 2;
  }

  if (cMPA_Hdr[1] &0x08)   // LowSamplingFrequency Extension uses the old "ID" Flag
      uMPA_LSF      = 0;
  else
  {
      uMPA_LSF      = 1;
      uMPA_25_LSF  += 1;
  }

  uMPA_Mpeg_Ver    = 4-((cMPA_Hdr[1]>>3)&3);  // 1=Mpeg1 2=mpeg2 3=Reserved 4=mpeg2.5
  uMPA_Layer_Ix    = 3- (cMPA_Hdr[1]>>1)&3;
  uMPA_SampFreq_Ix =    (cMPA_Hdr[2]>>2)&3;
  uMPA_Padding     =    (cMPA_Hdr[2]>>1)&1;
  uMPA_kBitRate_Ix =    (cMPA_Hdr[2]>>4)&15;
  uMPA_Channel_ix  =    (cMPA_Hdr[3]>>6)&3;

  uMPA_Channels  = uMPA_Channel_ix==3?1:2;  // #3=Mono, others=Stereo
  uMPA_Layer     = uMPA_Layer_Ix + 1;

  uMPA_Sample_Hz = MPA_SAMPLE_HZ[uMPA_25_LSF][uMPA_SampFreq_Ix];
  uMPA_kBitRate  = MPA_KBIT_RATE[uMPA_LSF][uMPA_Layer_Ix][uMPA_kBitRate_Ix];

  // not allowed to change number of channels within the one stream
  //if (process.uMPA_Sample_Hz[getbit_MPA_Track] == uMPA_Sample_Hz
  //||  process.uMPA_Sample_Hz[getbit_MPA_Track] == 0)

  // if previously found mpeg2 or earlier in same stream
  // then Dis-Allow mpeg2.5/LSF 
  if (process.uMPA_Mpeg_Ver[getbit_MPA_Track] >= uMPA_Mpeg_Ver
  ||  process.uMPA_Mpeg_Ver[getbit_MPA_Track] == 0)
  {
      MPA_FrameLen();

      if (iMPA_FrameLen > 0)
          Got_MPA_Hdr();
  }

}



//----------------------

void Got_MPA_PayLoad()
{
   unsigned char cSense;
   unsigned int  uSense, uTst;

   cSense =   0xE0;   // Allow for Mpeg2.5 
   uSense = 0xFFE0; // 0xFFF8;  // Syncword + ind
                        /*
   if (Mpeg_PES_Version != 2)  // Mpeg-1 - PES Header is different layout
   {
       cSense = 0xF5;
       uSense = 0xFFF5;
   }
   else
   {
       cSense = 0xFA;   // 0xF8;
       uSense = 0xFFFA; // 0xFFF8;  // Syncword + ind
   }
   */


  // Is this track still awaiting setup ?
  if ( ! mpa_Ctl[getbit_MPA_Track].rip && getbit_iPkt_Len_Remain > 4)
  {
     // Scan for an Mpeg Audio SyncWord
     getbit_input_code  = Get_Byte();
     getbit_input_code  = (getbit_input_code /* & 0xFF*/ )<<8;
     getbit_input_code |= Get_Byte();

     getbit_iPkt_Len_Remain -= 2;

     //i = 0;
     while (getbit_input_code <  uSense  // Syncword(11-12bits) + mpeg_ind(1bit)
       &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL   // RJ ALLOW FOR BAD DATA
       && ! MParse.Stop_Flag
       &&   getbit_iPkt_Len_Remain > 4) // Cannot handle hdr split across packets
     {
           getbit_input_code  = (getbit_input_code & 0xFF);
           getbit_input_code  =  getbit_input_code   <<8;
           getbit_input_code |= Get_Byte();
           getbit_iPkt_Len_Remain--;
           //i++;
     }

        /*sprintf(szBuffer, "%s MPA T%02d DELAY %dms.mpa_Ctl[", szOutput, getbit_MPA_Track+1, (process.AudioPTS-process.VideoPTS)/90);
        /mpa_Ctl[getbit_MPA_Track].file = fopen(szBuffer, "wb");*/

     // Recheck audio syncword in case of premature end
     uTst = getbit_input_code & 0x0006;
     if (getbit_input_code >=  uSense      // at least 11 bits sets
     && (uTst != 0))  // valid layer 
     {
         cMPA_Hdr[0] = (unsigned char)(getbit_input_code / 256);
         cMPA_Hdr[1] = (unsigned char)(getbit_input_code);

         // cTmp1 = (cMPA_Hdr[1] & 0xF8);
     
     //if (cMPA_Hdr[0]>=0xFF &&  (cMPA_Hdr[1] >= cSense)) //& 0xF8) == 0xF8) //  >>5==7))  // // Syncword(12bits) + mpeg_ind(1bit)
     //{

         cMPA_Hdr[2] = (unsigned char)Get_Byte();
         if (cMPA_Hdr[2] <  0xF0    // Integrity check - bitrate index
         &&  cMPA_Hdr[2] >= 0x10)    // Free format is too hard
         {
           cMPA_Hdr[3] = (unsigned char)Get_Byte();

           Chk_MPA_Hdr();

           // It is possible for an MPA header to be split across packets
           // GetByte may have traversed to a new packet
           // The following nasty kludge will handle this, SOMETIMES.
           if ((RdPTR - RdBFR) > 4) // ONLY if we have NOT crossed BUFFERS
           {  // Reinstate MPA header by clobbering previous data
              *(--RdPTR) = cMPA_Hdr[3];
              *(--RdPTR) = cMPA_Hdr[2];
              *(--RdPTR) = cMPA_Hdr[1];
              *(--RdPTR) = cMPA_Hdr[0];
              getbit_iPkt_Len_Remain+=2;
           }
         }
     }  // END - Got an Audio SyncByte ?


  } // ENDIF First Time this track

  // else
  if (mpa_Ctl[getbit_MPA_Track].rip
  && (iAudio_SEL_Track == getbit_MPA_Track)
  && (iWAV_Init && iMPAdec_Init && iPlayAudio && byMPALib_OK))
  {

       while (getbit_iPkt_Len_Remain > 0
         &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL
         && ! MParse.Stop_Flag )
       {
         if (getbit_iPkt_Len_Remain+RdPTR > RdEOB)
         {
             MPAdec.nRet =MPAdec.mlDecode(&MPAdec.mp,
                                    (char*)RdPTR, RdEOB-RdPTR,
                                    (char*)fPCMData,
                                           MPAdec.dwFree,
                                          &MPAdec.nSize);
             while (MPAdec.nRet == ML_OK
               &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL
               && ! MParse.Stop_Flag )
             {
               if (MPAdec.nSize && iPlayAudio && MParse.FastPlay_Flag <= MAX_WARP)
               {
                 float2int(fPCMData,(BYTE*)fPCMData,
                                         MPAdec.nSize/sizeof(float));

                 /*
                 if (uMPA_Channel_ix == 3) // Mono Experiment
                 {
                   WAV_Byte_Swap(0, (BYTE*)fPCMData, (BYTE*)fPCMData,
                                    MPAdec.nSize/2);
                 }
                 */
                 iMPA_PCM_Len1 = MPAdec.nSize/2;
                 if (PktStats.iAudDelayBytes > 0)
                 {
                   if (iMPA_PCM_Len1 <= PktStats.iAudDelayBytes)
                   {
                       iMPA_PCM_Len1 -= PktStats.iAudDelayBytes;
                       PktStats.iAudDelayBytes = 0;
                   }
                   else
                   {
                       PktStats.iAudDelayBytes -= iMPA_PCM_Len1;
                       iMPA_PCM_Len1 = 0;
                   }
                 }

                 if (iMPA_PCM_Len1 > 0)
                     WAV_Packet_Warp((BYTE*)fPCMData, iMPA_PCM_Len1);
               }

               MPAdec.nRet = MPAdec.mlDecode(&MPAdec.mp, NULL, 0,
                     (char*)fPCMData, MPAdec.dwFree,&MPAdec.nSize);
             }
             getbit_iPkt_Len_Remain -= RdEOB-RdPTR;
             Mpeg_READ();
             RdPTR = RdBFR;
         }
         else
         {
            MPAdec.nRet =MPAdec.mlDecode(&MPAdec.mp,
                                  (char*)RdPTR,
                                        getbit_iPkt_Len_Remain,
                                 (char*)fPCMData,MPAdec.dwFree,
                                       &MPAdec.nSize);
             while(MPAdec.nRet  == ML_OK
              &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL
              && ! MParse.Stop_Flag )
             {
               if (MPAdec.nSize && iPlayAudio && MParse.FastPlay_Flag <= MAX_WARP )
               {
                  float2int(fPCMData,(BYTE*)fPCMData,
                                      MPAdec.nSize/sizeof(float));
                  iMPA_PCM_Len1 = MPAdec.nSize/2;
                  if (PktStats.iAudDelayBytes > 0)
                  {
                    if (iMPA_PCM_Len1 <= PktStats.iAudDelayBytes)
                    {
                       iMPA_PCM_Len1 -= PktStats.iAudDelayBytes;
                       PktStats.iAudDelayBytes = 0;
                    }
                    else
                    {
                       PktStats.iAudDelayBytes -= iMPA_PCM_Len1;
                       iMPA_PCM_Len1 = 0;
                    }
                  }


                  if (iMPA_PCM_Len1 > 0)
                      WAV_Packet_Warp((BYTE*)fPCMData, iMPA_PCM_Len1);

               }

               MPAdec.nRet = MPAdec.mlDecode(&MPAdec.mp,NULL,0,
                                      (char*)fPCMData,
                                    MPAdec.dwFree,&MPAdec.nSize);
             } // endwhile OK

             RdPTR += getbit_iPkt_Len_Remain;
             getbit_iPkt_Len_Remain = 0;
         }  // END ELSE
       } // END WHILE PktLenRemaining

  } // ENDIF to be played

}


//----------------------

void Got_MPA_Pkt()
{
  //int i;

  //getbit_AUDIO_ID  = getbit_input_code;

  if (MParse.SystemStream_Flag >= 0) // Transmission stream (TS or PVA) ?
      getbit_MPA_Track = getbit_input_code - AUDIO_ELEMENTARY_STREAM_0;
  else
  {
    getbit_MPA_Track = 0;
    while (mpa_Ctl[getbit_MPA_Track].uPID  != uGot_PID
           && getbit_MPA_Track <= CHANNELS_MAX)
    {
       if(  mpa_Ctl[getbit_MPA_Track].uPID == 0)
            mpa_Ctl[getbit_MPA_Track].uPID  = uGot_PID;
       else
            getbit_MPA_Track++;
    }
  }
  mpa_Ctl[getbit_MPA_Track].cStream = (unsigned char)(getbit_input_code);

  process.SkipPTS = 0;
  PktStats.iMPA_Packets++;

  getbit_iPkt_Len_Remain = Get_Short();

  // Transport stream PES length does not apply - calc from TS packet
  if (MParse.SystemStream_Flag < 0)
      getbit_iPkt_Len_Remain = RdEndPkt - RdPTR;
  else
  {
      RdEndPkt = RdPTR + getbit_iPkt_Len_Remain; RdEndPkt_4 = RdEndPkt-4; RdEndPkt_8 = RdEndPkt-8;
  }

  Mpeg_PES_Byte1   = Get_Byte();
  getbit_iPkt_Len_Remain--;

  Mpeg_PES_Version = Mpeg_PES_Byte1>>6;
  getbit_iDropPkt_Flag = 1;


  Audio_Fallback_Chk();
  
  if ((iWant_Aud_Format == FORMAT_MPA || !iWant_Aud_Format)/* && (AVI_Flag || D2V_Flag)*/)
  {
      // Auto detect Audio track
      if (iAudio_SEL_Track == TRACK_AUTO)
      {
          iAudio_SEL_Track  =  getbit_MPA_Track ;
          iAudio_SEL_Format =  FORMAT_MPA;
      }

      //if (Mpeg_PES_Version == 2     // Mpeg-2 format ? //if ((getbit_input_code & 0xc0) == 0x80)
      //||  iPES_Mpeg_Any)
      {
        getbit_iDropPkt_Flag = 0;
        iAudio_Trk_FMT[getbit_MPA_Track] = FORMAT_MPA;

        process.iAudio_MPA_Found = 1; 

        if (getbit_MPA_Track == iAudio_SEL_Track)  // is it the one to play?
        {
            PlayCtl.iAudio_SelStatus = 1;

            if (process.iAudio_InterFrames)
            {
                process.iAudio_Interleave = process.iAudio_InterFrames;
                process.iAudio_InterFrames = 0;
            }
        }


        if (Mpeg_PES_Version != 2)  // Mpeg-1 - NO HEADER processing - EXPERIMENTAL
        {
            Mpeg1_PesHdr();
            getbit_iPkt_Len_Remain -= getbit_iPkt_Hdr_Len_Remaining;
        }
        else
        {
            getbit_PES_HdrFld_Flags = Get_Byte();   // Field Flags
            getbit_iPkt_Hdr_Len = Get_Byte();              // PES_header_data_length
            getbit_iPkt_Hdr_Len_Remaining = getbit_iPkt_Hdr_Len;

            if (getbit_PES_HdrFld_Flags>=0x80) // PTS present ?
            {
               CandidatePTS = Get_PTS(0);
               PTS_Audio_Analysis();
            }

            getbit_iPkt_Len_Remain -= (getbit_iPkt_Hdr_Len+2);
        }

        RdPTR += getbit_iPkt_Hdr_Len_Remaining;  // Discard rest of header -boring stuff

        MPEG_PTR_BUFF_CHK
      }

      if (! getbit_iDropPkt_Flag)
      {
         /* // This needs further work
         if (!mpa_Ctl[0].rip
             &&  (    RdPTR[0] == 0x0B && RdPTR[1] == 0x77
                  ||  process.iAudioAC3inMPA))  // Allow for crap muxer that puts AC3 inside an MPA packet type
         {
             if (!iWarnAC3inMPA)
             {
               iWarnAC3inMPA = 1;
               strcpy(szMsgTxt, "AC3 in MPA pkt.    ");
               DSP1_Main_MSG(0, 1);
             }
             //process.iAudioAC3inMPA = 1;  
             //Got_PrivateStream();
         }
         else
         */
            Got_MPA_PayLoad();
      } // END - Not to be dropped ?

  } // END - Interested in MPA format ?

}



