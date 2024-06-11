
#include "global.h"
#include "Audio.h"


#if defined WIN32 || defined _WIN32

#include <string.h>
#include <errno.h>
#define VERSION_STRING "\n WAVE_OUT 0.7.1_rj\n"
#endif
 
#include "wave_out.h"
#include "Wave_CS.h"

int  SPKR_Open();
void SPKR_Play_Buffer(void* P_Wav_Data, size_t P_Len);

WAVEFORMATEX  outFormat;


//---------------------------------------------------------------


void WAV_WIN_Spkr_close(void)
{

  if (DBGflag)
      DBGout("SPKE");

  if (iAudioDBG)
      DBGAud(&"SPKE");

  // SetPriorityClass ( GetCurrentProcess (), IDLE_PRIORITY_CLASS); // NORMAL_PRIORITY_CLASS

  if ( hWAVEdev != NULL )
  {
    WAV_Flush();

    if (DBGflag)
        DBGout("SPKS");

    if (iAudioDBG)
        DBGAud(&"SPKS");

    waveOutReset (hWAVEdev);      // reset the device
    waveOutClose (hWAVEdev);      // close the device
    hWAVEdev = NULL;  
    if (DBGflag)
        DBGout("FLU ");

    if (iAudioDBG)
         DBGAud(&"FLU ");

    Sleep(5);

  }

  if (WAV_cs_FLAG) // Try to prevent CRASH IN KERNEL32 (Win98SE)
  {
     if (DBGflag)
         DBGout("WAVD");

     if (iAudioDBG)
         DBGAud(&"WAVD");
     Sleep(10);

     DeleteCriticalSection ( &WAV_Critical_Section );
     WAV_cs_FLAG = 0;
  }

  iWAVEOUT_Scheduled_Blocks = 0;
  iWAV_Init = 0;

  return; //  0;
}





//-------------------------
// Process Message Queue for Speaker

DWORD  WINAPI  SPEAKER_Que(LPVOID  nDUMMY)
{
  MSG msg;

  dwSPEAKER_ThreadId = GetCurrentThreadId();

         if (DBGflag)
         {
             DBGout("\nSPKR QUE\n");
         }

   //  message loop
   while (GetMessage(&msg, NULL, 0, 0) > 0 )
   {
         if (DBGflag)
         {
             sprintf(szBuffer, "SKPR Msg=%d", msg.message);
             DBGout(szBuffer);
         }

         switch (msg.message)
         {
           case WAVEOUT_SETPARMS:
                SPKR_Open();
           break;


           case WAVEOUT_PLAYSAMPLES:
                if (DBGflag)
                {
                    sprintf(szBuffer, "SPKR SAMPLES Len=%d Buff=x%08X",
                                msg.lParam, msg.wParam);
                    DBGout(szBuffer);
                }
                SPKR_Play_Buffer((void *)msg.wParam, msg.lParam);

           break;

           case WM_DESTROY:
           case WM_QUIT:
           case WAVEOUT_CLOSE:
                if (DBGflag)
                {
                    DBGout("SPKR CLOSE");
                }
                WAV_WIN_Spkr_close();
                return 0;
           break;

           default:
                if (DBGflag)
                {
                    DBGout("\nSPKR *UNK MSG*\n");
                }
           break;

         } // endswitch

   } // end while

   if (DBGflag)
       DBGout("\nSPKR *END*\n");
   
   return msg.wParam;

}


//-----------------------------------------------------------------
/*
 *  This function registers already played WAVE chunks. 
                      Freeing is done by WAV_Free_Memory(),
 */

static void CALLBACK
wave_callback ( HWAVE hWave, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
{
  if ( uMsg == WOM_DONE ) 
  {
    EnterCriticalSection ( &WAV_Critical_Section );
    PlayedWaveHeaders [PlayedWaveHeadersCount++] = (WAVEHDR*) dwParam1;
    LeaveCriticalSection ( &WAV_Critical_Section );

    if (DBGflag)
    {
       sprintf(szBuffer, "    *PLAYED* Aud=%d    Done=%d", iWAVEOUT_Scheduled_Blocks, PlayedWaveHeadersCount);
       DBGout(szBuffer);
    }
    if (DBGflag || iAudioDBG)     //if (cAudState != '*')
    {
      if (PlayedWaveHeadersCount >= iWAVEOUT_Scheduled_Blocks)
      {
          cAudState = '-';
          TextOut(hDC, 0, iMsgPosY, "==========", 10);
      }
    }
    

  }
}



/*static */ void
WAV_Free_Memory ( void )
{
  WAVEHDR*  wh;
  HGLOBAL   hg;

  EnterCriticalSection ( &WAV_Critical_Section );

  wh = PlayedWaveHeaders [--PlayedWaveHeadersCount];
  iWAVEOUT_Scheduled_Blocks--;                   // decrease the number of USED blocks

  LeaveCriticalSection ( &WAV_Critical_Section );

  waveOutUnprepareHeader ( hWAVEdev, wh, sizeof (WAVEHDR) );

  hg = GlobalHandle ( wh -> lpData );   // Deallocate the buffer memory
  GlobalUnlock (hg);
  GlobalFree   (hg);

  hg = GlobalHandle ( wh );             // Deallocate the header memory
  GlobalUnlock (hg);
  GlobalFree   (hg);

  process.iWavQue_Len -= iWavBlock_Len[iWavBlock_From++]; // Decrement total left to be played
  if (iWavBlock_From >= WAVEOUT_MAX_BLOCKS) iWavBlock_From = 0; // from a FIFO buffer

  if (DBGflag)
  {
      sprintf(szBuffer, "       FREE  Aud=%d    Done=%d", iWAVEOUT_Scheduled_Blocks, PlayedWaveHeadersCount);
      DBGout(szBuffer);
  }

}


//-----------------------------------------------------------------

/*
  WAVEFORMATEX  CurrFormat;

  PostThreadMessage(dwSPEAKER_ThreadId,
    UINT  uMsg,	// message to post
    WPARAM  wParam,	// first message parameter
    LPARAM  lParam 	// second message parameter
   );
*/

int SPKR_Open()
{
  int  iRC, iLen;
  UINT deviceID = WAVE_MAPPER;
  char szFormat[128];

  if (iAudioDBG)
     TextOut(hDC, 0, iMsgPosY, "WAV OPN=&d  ",  11);

  iRC = waveOutOpen ( &hWAVEdev, deviceID, &outFormat, (DWORD)wave_callback, 0, CALLBACK_FUNCTION );

  sprintf(szFormat, "Format=%d Bits=%u Ch=%u Freq=%d/s Align=%d AvgBps=%d RC=%d",
              outFormat.wFormatTag, 
              WAVEOUT_BitsPerSample,   
                                 WAVEOUT_Channels, outFormat.nSamplesPerSec,
                                  (int)(outFormat.nBlockAlign), 
                                            outFormat.nAvgBytesPerSec, iRC);
  if (iAudioDBG)
  {
     iLen = sprintf(szDBGln, "WAV OPN=&d  ", iRC);
     TextOut(hDC, 0, iMsgPosY, szDBGln,  iLen);
  }

  if (DBGflag)
      DBGout(szFormat);

  if (iRC != MMSYSERR_NOERROR)
   {
      wav_MM_ERR(iRC, &szFormat[0]);

    /*switch ( iRC )
    {
      case MMSYSERR_ALLOCATED:   return Box ( "Device is already open.", szFormat);
      case MMSYSERR_BADDEVICEID: return Box ( "The specified device is out of range.", szFormat);
      case MMSYSERR_NODRIVER:    return Box ( "There is no audio driver in this system.", szFormat );
      case MMSYSERR_NOMEM:       return Box ( "Unable to allocate sound memory.", szFormat );
      case WAVERR_BADFORMAT:     return Box ( "This audio format is not supported.", szFormat );
      case WAVERR_SYNC:          return Box ( "The device is synchronous.", szFormat );
      default:                   return Box ( "Unknown media error.", szFormat );
    }*/

    iPlayAudio  = 0;
    iAudio_Lock = 0;
  }
  else 
  {
    iWAV_Init = 1;  

    if (iPlayAudio)
    {
      waveOutReset ( hWAVEdev );
      if (iPlayAudio)
      {
         InitializeCriticalSection ( &WAV_Critical_Section );
         WAV_cs_FLAG = 1;
         //SetPriorityClass ( GetCurrentProcess (), HIGH_PRIORITY_CLASS );
         return 0;
      }
    }
    Sleep(1);
  }

  return -1;
}


// Set Speaker PCM Attributes 
int WAV_Set_Open()
{

/*
 *  extended waveform format structure used for all non-PCM formats. this
 *  structure is common to all non-PCM formats.
 
typedef struct tWAVEFORMATEX
{
    WORD        wFormatTag;         /* format type
    WORD        nChannels;          /* number of channels (i.e. mono, stereo...) 
    DWORD       nSamplesPerSec;     /* sample rate 
    DWORD       nAvgBytesPerSec;    /* for buffer estimation
    WORD        nBlockAlign;        /* block size of data 
    WORD        wBitsPerSample;     /* number of bits per sample of mono data 
    WORD        cbSize;             /* the count in bytes of the size of 
            /* extra information (after cbSize) 
} WAVEFORMATEX_t
*/ 

  int iRC;

  iWavBlock_From = iWavBlock_To = 0;

  if ( waveOutGetNumDevs () < 1 )
    return Box ( "No audio device present.", "" );

  outFormat.wFormatTag      = WAVE_FORMAT_PCM;
  outFormat.wBitsPerSample  = (unsigned short) WAVEOUT_BitsPerSample;
  outFormat.nChannels       = (unsigned short) WAVEOUT_Channels;
  outFormat.nSamplesPerSec  = WAVEOUT_SampleFreq; // (unsigned long)(WAVEOUT_SampleFreq + 0.5);
  outFormat.nBlockAlign     = (unsigned short)((outFormat.wBitsPerSample 
                                      + 7) / 8 * outFormat.nChannels);
  outFormat.nAvgBytesPerSec = outFormat.nSamplesPerSec * outFormat.nBlockAlign;

//  sprintf(szFormat, "%hu", outFormat.nSamplesPerSec) ;
//  SetDlgItemText(hStats, IDC_SAMPLE_RATE, szFormat);
//  sprintf(szFormat, "%hu", outFormat.wBitsPerSample) ;
//  SetDlgItemText(hStats, IDC_SAMPLE_DEPTH, szFormat);
//  sprintf(szFormat, "%hu", outFormat.nChannels) ;
//  SetDlgItemText(hStats, STATS_AUDIO_CHANNELS, szFormat);

  WAV_Fmt_Flag = 1;
  sprintf(WAV_Fmt_Brief, "%hu %hu %hu", outFormat.nSamplesPerSec, 
                            outFormat.wBitsPerSample, outFormat.nChannels);

  if (DBGflag)
  {
      sprintf(szBuffer, "WAV SET %s", WAV_Fmt_Brief);
      DBGout(szBuffer);
  }


  // FUDGE - TRAP SUSPICIOUS SETTINGS

  //  no longer be necessary
  //if (outFormat.nChannels < 2)     // && outFormat.nSamplesPerSec == 48000 &
  //    outFormat.nChannels = 2;

  if (outFormat.nSamplesPerSec <    1000
   || outFormat.nSamplesPerSec > 1024000)
  {
      outFormat.nSamplesPerSec =   48000;
      outFormat.nChannels = 2;
  }

  if (outFormat.nSamplesPerSec ==  48000 && iAudio_Force44K)
      outFormat.nSamplesPerSec =   44100;


  if (outFormat.nAvgBytesPerSec <    1000
   || outFormat.nAvgBytesPerSec > 8192000)
      outFormat.nAvgBytesPerSec = 1024000;

  if (outFormat.wBitsPerSample !=  8
   && outFormat.wBitsPerSample != 16
   && outFormat.wBitsPerSample != 24
   && outFormat.wBitsPerSample != 32
   && outFormat.wBitsPerSample != 48
   && outFormat.wBitsPerSample != 64)
      outFormat.wBitsPerSample  = 16;

   process.iWavBytesPerMs = outFormat.nSamplesPerSec 
                          * outFormat.wBitsPerSample
                          * outFormat.nChannels / 8000 ;

   if (process.iWavBytesPerMs < 1)   // Allow for silly settings
       process.iWavBytesPerMs = 192; // 48k samples/sec

  sprintf(WAV_Fmt_Brief, "%hu %hu %hu", outFormat.nSamplesPerSec, 
                            outFormat.wBitsPerSample, outFormat.nChannels);

  if (DBGflag)
  {
      sprintf(szBuffer, "WAV SET %s", WAV_Fmt_Brief);
      DBGout(szBuffer);
  }

   if (iCtl_AudioThread)
   {
       if (!hThread_SPEAKER)
       {
           hThread_SPEAKER = CreateThread(NULL, 0, 
                                          SPEAKER_Que, 0, 0, 
                                       &dwSPEAKER_ThreadId);
           if (DBGflag)
           {
               sprintf(szBuffer, "    Handle=x%08X Id=x%08X",
                                 hThread_SPEAKER, dwSPEAKER_ThreadId);
               DBGout(szBuffer);
           }
           if (!hThread_SPEAKER)
           {
             Msg_LastError("SPKR CREATE", 0,  'b');
           }

           Sleep(5);
           //if (hThread_SPEAKER)
           //  iRC = 0;
           //else
           //  iRC = -1;

       }

       iRC = PostThreadMessage(dwSPEAKER_ThreadId,
                         WAVEOUT_SETPARMS, 0, 0);
                        // WPARAM  wParam,	// first message parameter
                        // LPARAM  lParam 	// second message parameter
           if (DBGflag)
           {
               sprintf(szBuffer, "    Post RC=%02d  msg=%d", 
                                     iRC, WAVEOUT_SETPARMS);
               DBGout(szBuffer);
           }

       Sleep(5);

       iRC = 1 - iRC;
       
   }
   else
   {
     iRC = SPKR_Open();
     if (iCtl_Audio_InitBreathe)
         Sleep(iCtl_Audio_InitBreathe);
   }


   if (DBGflag)
   {
       sprintf(szBuffer, "WAV SET RC=%02d", iRC);
       DBGout(szBuffer);
   }
   return iRC;

}



//-----------------

void WAV_WIN_Play_Samples (void* P_Wav_Data, size_t P_Len)
{
  int iRC;

  if (DBGflag)
  {
      sprintf(szBuffer, "WAV PLAY Len=%d  Buff=x%08X",
                             P_Len, P_Wav_Data);
      DBGout(szBuffer);
  }
   

  if (iCtl_AudioThread)
  {
      iRC = PostThreadMessage(dwSPEAKER_ThreadId,
                         WAVEOUT_PLAYSAMPLES, 
                         (WPARAM)(P_Wav_Data), 
                         (LPARAM)(P_Len));
           if (DBGflag)
           {
               sprintf(szBuffer, "    Post RC=%02d  msg=%d", 
                                     iRC, WAVEOUT_PLAYSAMPLES);
               DBGout(szBuffer);
           }
      Sleep(1);

  }
  else
  {
    SPKR_Play_Buffer(P_Wav_Data, P_Len);
  }

}


void WAV_Flush()
{
    while ( iWAVEOUT_Scheduled_Blocks > 0 )
    {
      Sleep(10);  // iWAVEOUT_Scheduled_Blocks * 5);
       while ( PlayedWaveHeadersCount > 0 )         // free used blocks ...
         WAV_Free_Memory ();
    }

    Sleep(5);
}



//--------------------

void WAV_WIN_Audio_close(void)
{

  if (iCtl_AudioThread && hThread_SPEAKER)
  {
      //iRC = PostThreadMessage(dwSPEAKER_ThreadId,
      //                   WAVEOUT_CLOSE, 0, 0);
  }
  else
  {
      WAV_WIN_Spkr_close();
  }


}

