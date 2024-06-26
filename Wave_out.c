
#define DBG_RJ


/* Based on - Copyright (c) 2002, John Edwards

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* Set TABS = 4 */

#include "global.h"
#include "Audio.h"


/********************************************************************

 function: To provide playback of 16 bit PCM wave data in Win32
           environments from decoded compressed files.

 ********************************************************************/

#if defined WIN32 || defined _WIN32

#include <string.h>
#include <errno.h>
#define VERSION_STRING "\n WAVE_OUT 0.7.2_rj\n"
#endif

#include "wave_out.h"
#include "Wave_CS.h"
#define MAXWAVESIZE     4294967040LU

// This is modified for USE_WIN_AUDIO - ONLY 2002-02-27


//-----------------------------------------------------------------
int Box ( const char* msg1, char msg2[80] )
{
  char szMsgFull[256];
  sprintf (szMsgFull, "%s \n\n %s \n", msg1, msg2);

  if (iWave_MsgAlerted)
  {
     strcpy(szMsgTxt, szMsgFull) ;
     DSP1_Main_MSG(0,1);
  }
  else
  {
      MessageBox ( NULL, szMsgFull, VERSION_STRING,
               MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
      iWave_MsgAlerted = 1;
  }
  return -1;
}



//----------------------------------------------------


void wav_MM_ERR(int p_RC, char* P_Msg2)
{
  char szFormat[80];
  sprintf(szFormat, "RC=%d\n\n%s", p_RC, P_Msg2);
  iPlayAudio = 0;
  iAudio_Lock = 0;


  if (iWav_Err < 2)
   switch ( p_RC )
   {
    case WAVERR_BADFORMAT:     Box ( "Audio format not supported.", szFormat ); break;
    case WAVERR_SYNC:          Box ( "The device is synchronous.", szFormat ); break;

    case  MMSYSERR_ERROR:        Box("Unspecified error.", szFormat ); break;
    case  MMSYSERR_BADDEVICEID:  Box("Device ID out of range.", szFormat); break;
    case  MMSYSERR_NOTENABLED:   Box( "Driver failed enable.", szFormat); break;
    case  MMSYSERR_ALLOCATED:    Box( "Device already allocated.", szFormat); break;
    case  MMSYSERR_INVALHANDLE:  Box( "Device handle is invalid.", szFormat); break;
    case  MMSYSERR_NODRIVER :    Box( "No device driver present.", szFormat); break;
    case  MMSYSERR_NOMEM:        Box( "Media Memory allocation error.", szFormat); break;
    case  MMSYSERR_NOTSUPPORTED: Box( "Function isn't supported.", szFormat); break;
    case  MMSYSERR_BADERRNUM:    Box( "Error value out of range.", szFormat); break;
    case  MMSYSERR_INVALFLAG:    Box( "Invalid flag passed.", szFormat); break;
    case  MMSYSERR_INVALPARAM:   Box( "Invalid parameter passed.", szFormat); break;
    case  MMSYSERR_HANDLEBUSY:   Box( "Handle being used.", szFormat); break;
               // simultaneously on another.
               // thread (eg callback)
    case  MMSYSERR_INVALIDALIAS: Box( "specified alias not found.", szFormat); break;
    case  MMSYSERR_BADDB    : Box( "bad registry database.", szFormat); break;
    case  MMSYSERR_KEYNOTFOUND: Box( "registry key not found.", szFormat); break;
    case  MMSYSERR_READERROR: Box( "registry read error.", szFormat); break;
    case  MMSYSERR_WRITEERROR: Box( "registry write error.", szFormat); break;
    case  MMSYSERR_DELETEERROR: Box( "registry delete error.", szFormat); break;
    case  MMSYSERR_VALNOTFOUND: Box( "registry value not found.", szFormat); break;
    case  MMSYSERR_NODRIVERCB: Box( "driver does not call DriverCallback.", szFormat); break;
    //case  MMSYSERR_LASTERROR: Box( "last error in range.", szFormat);


    default:      Box ( "Unknown Multi-Media error.", szFormat );
  }

  iWav_Err++;
  iPlayAudio = 0;
  return ;
}


//------------------------------------------------------------------

//-----------------------------------------------------------------

int iPrev_Left, iPrev_Right;

//int
void SPKR_Play_Buffer(void* P_Wav_Data, size_t P_Len)
{
  HGLOBAL    hg;
  HGLOBAL    hg2;
  LPWAVEHDR  wh;
  void*      allocptr;
  void*      W_Wav_Pair;
  void*      alloc_Wav_Pair;
  void*      alloc_End;

  unsigned iSPKR_Buf_Len;
  int iRC, iTmp1; //, iTmp2;
  int iBoost_Factor, iKaraoke, iAnti_Phase, iPALTelecide_flag;
  unsigned uOverCtr;
  int iOverflow, iNearEnough; 
  // int iUnderflow

  register int iLeft, iRight;
  int iLimit, iLimit_Neg;
  int iPeak, iTrough;

  
  if (iCtl_PALTelecide
   && iOverride_FrameRate_Code == 2   // Want 24FPS
   && MPEG_Seq_frame_rate_code == 3   //  was 25FPS
   && !iAudio_Force44K)
      iPALTelecide_flag = 1;
  else
      iPALTelecide_flag = 0;

  if (iCtl_Volume_Boost && iVolume_Boost > 0)
      iBoost_Factor = iVolume_Boost; // + 7;
  else
      iBoost_Factor = K_BOOST_DENOM;

  if (MParse.SlowPlay_Flag)              // Slow play sound is too annoying for full volume 
      iBoost_Factor = iBoost_Factor / 2; // K_BOOST_DENOM; // so spare the ears.

  if (iBoost_Factor <= 0)
      iBoost_Factor  = 1;

  //if (iCtl_Volume_Limiting)
  //    iUnderflow = 2;
  //else
  //    iUnderflow = K_BOOST_DENOM;

  
  iSPKR_Buf_Len = P_Len;
  if (iSPKR_Buf_Len < 16)
      iSPKR_Buf_Len = 16;
  else
  if (iSPKR_Buf_Len > 40960000)
      iSPKR_Buf_Len = 40960000;

  if (iPALTelecide_flag)
      iSPKR_Buf_Len = ((iSPKR_Buf_Len * 24) / 100) * 4;

  if (DBGflag)
  {
      sprintf(szBuffer, "WaveOut  IN  Aud=%d %03dms Done=%d  Len=%d Buff=x%08X",
                         iWAVEOUT_Scheduled_Blocks, PlayedWaveHeadersCount,
                                 (iSPKR_Buf_Len / process.iWavBytesPerMs),
                                 P_Len, P_Wav_Data);
      DBGout(szBuffer);
  }


  for (;;)
  {
    while ( PlayedWaveHeadersCount > 0 )                // free used blocks ...
      WAV_Free_Memory ();

    // Is there a free block ...
    if ( iWAVEOUT_Scheduled_Blocks < sizeof( PlayedWaveHeaders)
                                    /sizeof(*PlayedWaveHeaders) ) // WAVEOUT_MAX_BLOCKS
    {
        break;
    }


    if (DBGflag || iAudioDBG)     //if (cAudState != '*')
    {
          cAudState = '*';
          iTmp1 = sprintf(szBuffer, "*  Blks=%03d Max=%03d",
                                         iWAVEOUT_Scheduled_Blocks,
                                     (sizeof( PlayedWaveHeaders)
                                     /sizeof(*PlayedWaveHeaders)));
          TextOut(hDC, 0, iMsgPosY, szBuffer, iTmp1);
    }

    PlayCtl.iMaxedOut = 1;  PlayCtl.iBehindFrames = 0;
    Sleep (5);

               if (DBGflag)
               {
                   sprintf(szBuffer, "WaveOut 5ms, Aud=%d", iWAVEOUT_Scheduled_Blocks);
                   DBGout(szBuffer);
               }
               if (DBGflag || iAudioDBG)     //if (cAudState != '*')
               {
                  cAudState = '-';
                  TextOut(hDC, 0, iMsgPosY, "==========", 10);
               } 
    
  }

  process.iWavQue_Len += iSPKR_Buf_Len;          // Increment total to be played
  iWavBlock_Len[iWavBlock_To++] = iSPKR_Buf_Len; // Remember size of block
  if (iWavBlock_To >= WAVEOUT_MAX_BLOCKS) iWavBlock_To = 0; // in a FIFO buffer

  hg2 = GlobalAlloc ( GMEM_MOVEABLE, iSPKR_Buf_Len );
  if ( hg2 == NULL )   // allocate some memory for a copy of the buffer
  {
      Box ( "GlobalAlloc P_Wav_Data failed.", " " );
      return;
  }

  allocptr = GlobalLock (hg2);
  PlayCtl.uAud_Packets++;

  iKaraoke    = MParse.Karaoke_Flag;
  iAnti_Phase = MParse.Anti_Phase;





  // Here we can call any modification output functions we want....

SCAN_SAMPLES_Start:

  iPeak   = -32766; // begin nowhere near peak
  iTrough =  32767; // begin nowhere near trough

#define OVFL_TRIG_DEFAULT    32767  // was 32767

//#define BOOST_LIMIT  24000  // Aim a little below absolute max, when boosting a lot  // 28000

  if ( iBoost_Factor <= 0)
       iBoost_Factor  = 1;

  if (iBoost_Factor != K_BOOST_DENOM 
        || (iCtl_Volume_AUTO && iCtl_Volume_Boost)
        ||  iCtl_Volume_Limiting
        ||  PlayCtl.iAudioFloatingOvfl > 0
     )
  {
       iLimit = iVolume_Ceiling * K_BOOST_DENOM / iBoost_Factor; // Scale ceiling to allow testing BEFORE boosting
       if (iLimit < 16)
           iLimit = 16;
       else
       if (iLimit > 32767)
           iLimit = 32767;
     
       iLimit_Neg  = (1- iLimit);  // 16bit MIN
  }
  else
  {
     iLimit      =     OVFL_TRIG_DEFAULT;
     iLimit_Neg  = 1 - OVFL_TRIG_DEFAULT;
  }

  iNearEnough = iVolume_Ceiling * 
                 (K_BOOST_DENOM-1) / K_BOOST_DENOM; // Hysteresis zone


  uOverCtr = 0; iOverflow = 1;

  
  if ((iBoost_Factor == 0 && !iCtl_Volume_Limiting)
  &&  ! iKaraoke
  &&  ! iAnti_Phase
  &&  ! iPALTelecide_flag
  &&    PlayCtl.iAudioFloatingOvfl <= 0)
  {
      CopyMemory ( allocptr, P_Wav_Data, iSPKR_Buf_Len );
  }
  else
  {
    // Special Effects
    W_Wav_Pair     = P_Wav_Data;
    alloc_Wav_Pair = allocptr;
    alloc_End      = (char*)alloc_Wav_Pair + iSPKR_Buf_Len;

    while (alloc_Wav_Pair < alloc_End)
    {
      iLeft  =  *(short*)(W_Wav_Pair);
      iRight = *((short*)(W_Wav_Pair)+1);

      if (iAnti_Phase)
      {
          iLeft = - iLeft;
          // *(short*)(W_Wav_Pair)   = (short)(- (*( short*)(W_Wav_Pair)));
      }

      if (iKaraoke)
      {
           iLeft = iLeft - iRight;

           if (iBoost_Factor > K_BOOST_DENOM)
             iLeft <<=2; // iTmp1 <<=2;
           else
             iLeft <<=1;  // iTmp1 <<=1;

           iRight = iLeft;

           //  iTmp1 = (*( short*)(W_Wav_Pair));
           //  iTmp2 = (*((short*)(W_Wav_Pair)+1));
           //  iTmp1 = (iTmp1 - iTmp2);
           //  if (iBoost_Factor > K_BOOST_DENOM)
           //      iTmp1 <<=2;
           //  else
           //      iTmp1 <<=1;
           //  *(short*)(W_Wav_Pair)    = (short)(iTmp1);
           // *((short*)(W_Wav_Pair)+1) = (short)(iTmp1);
      }

      // Optional increase/decrease in volume
      //     to combat the trend of quiet sound tracks - Grrr... Argh..

      
      // Check for overflow   (postive and negative, left and right)
      if ( iLeft >  iLimit )
      {
            uOverCtr++;
            // Calculate overshoot factor
            iTmp1 = iLeft / iLimit;
            if (iOverflow < iTmp1)
                iOverflow = iTmp1;
      }
      else
      if ( iLeft <  iLimit_Neg )
      {
            uOverCtr++;
            // Calculate overshoot factor
            iTmp1 = iLeft / iLimit_Neg;
            if (iOverflow < iTmp1)
                iOverflow = iTmp1;
      }
      else
      if ( iRight >  iLimit )
      {
            uOverCtr++;
            // Calculate overshoot factor
            iTmp1 = iRight / iLimit;
            if (iOverflow < iTmp1)
                iOverflow = iTmp1;
      }
      else
      if ( iRight <  iLimit_Neg )
      {
            uOverCtr++;
            // Calculate overshoot factor
            iTmp1 = iRight / iLimit_Neg;
            if (iOverflow < iTmp1)
                iOverflow = iTmp1;
      }
      else
      {
          iLeft  = iLeft  * iBoost_Factor / K_BOOST_DENOM;
          iRight = iRight * iBoost_Factor / K_BOOST_DENOM;

          // remember the maximum and minimum values
          if (iPeak   < iLeft)
              iPeak   = iLeft;
          if (iTrough > iLeft)
              iTrough = iLeft;
          if (iPeak   < iRight)
              iPeak   = iRight;
          if (iTrough > iRight)
              iTrough = iRight;

          // very rough slowdown of audio for PAL Telecide
          // by interpolating one sample out of every 24
          // result is very noisy, so it is only experimental at this stage
          if (iPALTelecide_flag)
          {
            PlayCtl.iPalTelecide_ctr--;
            if (PlayCtl.iPalTelecide_ctr <= 0)
            {
                PlayCtl.iPalTelecide_ctr    = 24;
                *((short*)alloc_Wav_Pair)   = (iLeft  + iPrev_Left)  / 2;
                *((short*)alloc_Wav_Pair+1) = (iRight + iPrev_Right) / 2;
                alloc_Wav_Pair = (char*)(alloc_Wav_Pair) + 4;
            }
            iPrev_Left  = iLeft;
            iPrev_Right = iRight;
          }

          *((short*)alloc_Wav_Pair)   = iLeft;
          *((short*)alloc_Wav_Pair+1) = iRight;

      } // end (in range)
      

      alloc_Wav_Pair = (char*)(alloc_Wav_Pair) + 4; // increment pointers
      W_Wav_Pair     = (char*)(W_Wav_Pair)     + 4;

    } // ENDFOR (each stereo pair in block)


    // Automatic Volume Control if too much overshoot

    if (uOverCtr)
    {
      iVolume_UnBoost_Recent = iCtl_Volume_SlowAttack;  // Postpone boldness

      //if(PlayCtl.iPlayed_Frames > 15        // Allow for bad audio frame at start
      //        || ! process.Mpeg2_Flag            // DTV would probably not be Mpeg-1
      //        || MPEG_Seq_aspect_ratio_code < 3  // DTV should not use VGA format
      //        || iIn_VOB                         // DTV would probably not be called VOB
      //        || process.NAV_Loc >= 0            // DTV rarely uses NAV packs
      //  )
      {
        //if (iBoost_Factor > 0) 
        {
            iOverflow++;
            iVolume_Boost -= iOverflow;
            if (iVolume_Boost <=0)
                iVolume_Boost  =1;
            iBoost_Factor -= iOverflow;
            if (iBoost_Factor < 0)
                iBoost_Factor = 0;
            if (iVolume_AUTO > K_BOOST_TOO_SILLY) 
                iVolume_AUTO = iVolume_AUTO * 97 / 100;

            if (MParse.ShowStats_Flag || hVolDlg0)
               Stats_Volume_Boost();

            if (iBoost_Factor > 0) //  >= iUnderflow)
              //if (iVolume_Boost > 1)
                  goto SCAN_SAMPLES_Start;
        }
        //else
        //{
        //    PlayCtl.iAudioFloatingOvfl = 1;
        //}
      }
      /*
      else
      if (uOverCtr > (iSPKR_Buf_Len/22))
      {
        PlayCtl.uAudioOvflPkts++;
        if (PlayCtl.uAudioOvflPkts > 4
        //&& (PlayCtl.uAud_Packets / PlayCtl.uAudioOvflPkts) < 10 // More than 10% of packets overflowing ?
           )
        {
          //if (iVolume_Boost > 0)
          {
            iOverflow++;
            iVolume_Boost -= iOverflow;
            if (iVolume_Boost <=0)
                iVolume_Boost  =1;
            iBoost_Factor -= iOverflow;
            if (iBoost_Factor <=0)
                iBoost_Factor  =1;
            if (iVolume_AUTO > K_BOOST_TOO_SILLY)  // 400)
                iVolume_AUTO = iVolume_AUTO * 97 / 100;

            if (MParse.ShowStats_Flag || hVolDlg0)
               Stats_Volume_Boost();

              // Maybe try a temporary reduction to zero
              //if (iBoost_Factor > K_BOOST_DENOM)
              //    iBoost_Factor = 0;

              if (iBoost_Factor > 0)  iVolume_Boost > 1)
                  goto SCAN_SAMPLES_Start;

          }
        }
      }
      */

    } // ENDIF OVERFLOW

    // If we reach here, 
    // then we have been unable to fix up the output packet,
    // so just copy the original

    if (uOverCtr && ! MParse.Karaoke_Flag)
    {
      CopyMemory ( allocptr, P_Wav_Data, iSPKR_Buf_Len );
    }

  } // END-ELSE  NOT ASIS


  // Optional, very rough normalization of audio preview level

  if (iVolume_AUTO > 0  
  &&  iCtl_Volume_Boost > 0
  &&  iVolume_Boost   < iVolume_AUTO
  &&  iVolume_Ceiling > 1344)
  {
           
           if (iAudioDBG && hVolDlg0)
           {
               sprintf(szTmp32, "%d:%d", iVolume_UnBoost_Recent,
                                         iCtl_Volume_SlowAttack);
               SetDlgItemText(hVolDlg0, VOL_DBG, szTmp32);
           }
           

    if (iVolume_UnBoost_Recent > 0)  
    {
        iVolume_UnBoost_Recent--;
    }
    else
    // if there is a significant discrepancy, then reboost.
    if ( iPeak < iNearEnough
    && (iNearEnough + iTrough > 0)) // NOTE: iTrough is negative
    {
        if (iVolume_Boost < K_BOOST_DENOM) // gradual changes when shushing
        {
              iTmp1 = 1;
              iVolume_UnBoost_Recent = 1;
        }
        else
        {
           iTmp1 = ((iVolume_AUTO - iVolume_Boost) / K_BOOST_DENOM);  // Wobbly, since pkts/sec varies between files
           if (!iTmp1)
                iTmp1 = 1;
           else
           if (iTmp1 > 3)
               iTmp1 = 3;
        }

        iVolume_Boost += iTmp1;
    }
  }


  hg = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT,
                    sizeof (WAVEHDR));
  if ( hg == NULL ) // now make a header and WRITE IT!
  {
    Box ( "GlobalAlloc HDR failed.", " " );
    return; // -1;
  }

  wh                   = GlobalLock (hg);
  wh -> dwBufferLength = iSPKR_Buf_Len;
  wh -> lpData         = allocptr;

  iRC = waveOutPrepareHeader ( hWAVEdev, wh, sizeof (WAVEHDR));
  if ( iRC != MMSYSERR_NOERROR )
  {
    if (iWav_Err < 2)
    {
      wav_MM_ERR(iRC, "waveOutPrepareHeader failed.");
    }
    iWav_Err++;
    GlobalUnlock (hg);
    GlobalFree   (hg);
    return; // -1;
  }


  iRC =  waveOutWrite ( hWAVEdev, wh, sizeof (WAVEHDR));
  if ( iRC != MMSYSERR_NOERROR )
  {
    if (iWav_Err < 2)
    {
      wav_MM_ERR(iRC, &"waveOutWrite failed." );
    }
    iWav_Err++;
    GlobalUnlock (hg);
    GlobalFree   (hg);
    return; // -1;
  }

  EnterCriticalSection ( &WAV_Critical_Section );
  iWAVEOUT_Scheduled_Blocks++;      // InterlockedIncrement(&iWAVEOUT_Scheduled_Blocks);
  LeaveCriticalSection ( &WAV_Critical_Section );

  if (DBGflag)
  {
      sprintf(szBuffer, "WaveOut   X  Aud=%d    Done=%d", iWAVEOUT_Scheduled_Blocks, PlayedWaveHeadersCount);
      DBGout(szBuffer);
  }


  return; //  iSPKR_Buf_Len;
}




/* end of wave_out.c */
