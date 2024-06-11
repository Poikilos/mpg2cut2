//Dark Avenger, not only line of my code in here...

#include "global.h"

#include "Audio.h"
#include "mpalib.h"
#include "mpalib_more.h"


BYTE byTByPerSample=2;


//----------------------------------------------------
void MPALib_Init(HWND hwnd)

// LOAD AND CHECK AUDIO DECODER
// Success recorded in byte    "byMPALib_OK" 
// Attempt recorded in integer  "iMPALib_Status" -1=Try 0=Don't Try 1=OK


// RJ - LET USER SEE 1st MPALIB ERROR THEN SET DEFAULT TO "NONE" 

{

  if(!MPAdec.hDLL)  FreeLibrary(MPAdec.hDLL);

  MPAdec.hDLL = LoadLibrary(szMPAdec_NAME) ;

  strcpy(szBuffer, szMPAdec_NAME) ;
  strcat(szBuffer, " - ") ;

  if (MPAdec.hDLL == NULL) 
  {
     Msg_LastError("Load: ", 0x6969, ' ');
     strcpy(szAudio_Status, "NOT LOADED") ;
  }
  else 
  {
    // Get Interface functions from the DLL
    MPAdec.mlVersion = (MLVERSION) GetProcAddress(MPAdec.hDLL, TEXT_MLVERSION);
    MPAdec.mlInit    = (MLINIT)    GetProcAddress(MPAdec.hDLL, TEXT_MLINIT);
    MPAdec.mlExit    = (MLEXIT)    GetProcAddress(MPAdec.hDLL, TEXT_MLEXIT);
    MPAdec.mlDecode  = (MLDECODE)  GetProcAddress(MPAdec.hDLL, TEXT_MLDECODE);
      // strcpy(szAudio_Status, "WRONG VERSION") ; // In case version is wron
      // sprintf(szMsgTxt,"V=%d  I=%d  X=%d  D=%d", 
      //                 MPAdec.mlVersion, MPAdec.mlInit,
      //                 MPAdec.mlExit, MPAdec.mlDecode);
  }

  strcat(szBuffer, " ") ;
  strcat(szBuffer, szAudio_Status) ;
  strcat(szBuffer, "\n\n") ;
  strcat(szBuffer, szMsgTxt) ;

  if(!MPAdec.hDLL || !MPAdec.mlInit   || !MPAdec.mlExit 
             // || !MPAdec.mlVersion
                || !MPAdec.mlDecode)
  {
     strcat (szBuffer, "\n\n*** Need BOTH LibMMD.DLL and");
     strcat (szBuffer, szMPAdec_NAME) ;      // from Dark Avenger - http://darkav.de.vu
     strcat (szBuffer," ***\n     to be copied into Windows System folder or Dos PATH.");
     MessageBox(hwnd, szBuffer, szAppName, MB_ICONSTOP);

     if(!MPAdec.hDLL)  FreeLibrary(MPAdec.hDLL);
    //iAudio_SEL_Track = TRACK_NONE ;
     byMPALib_OK = 0;
  }
  else
  {
    strcpy(szAudio_Status, "Loaded") ;
    if (iAudio_SEL_Track == TRACK_NONE)   
        iAudio_SEL_Track = TRACK_AUTO ;
    byMPALib_OK = 1;
  }

  iMPALib_Status = (int) (byMPALib_OK) ; 
  if (MParse.ShowStats_Flag)
  {
     SetDlgItemText(hStats, IDC_MPAdec_NAME, szMPAdec_NAME);
     SetDlgItemText(hStats, STATS_MPAdec_STATUS, szAudio_Status);
  }

  return ;

}


//-----------------------------------------------------------
void float2int(float *fPCMData,  BYTE *pByte,  DWORD dwSamples/*, double dNormGain*/)
{
#include <math.h>

  DWORD k;

  float *pfInput, fCurrent;
  short *psSample;
  long  *plSample;
  

  switch (byTByPerSample)
  {
  case 2://16bit int
    psSample=(short*)(pByte);
    pfInput = fPCMData;

    for (k=0;k<dwSamples;++k) //float to 16 bit integer
    {
       //psSample=(short*)(pByte+(k<<1));
      fCurrent = *pfInput++;
      // Check for out-of-range data returned by MPAlib 
      if (fCurrent > +32766.0f || fCurrent < -32766.0f)
      {
          PlayCtl.iAudioFloatingOvfl = 64;

          if (iVolume_Boost >= K_BOOST_DENOM  
          && (PlayCtl.iPlayed_Frames > 15
              || ! process.Mpeg2_Flag            // DTV would not be Mpeg-1
              || MPEG_Seq_aspect_ratio_code < 3  // DTV would not use VGA format
              || iIn_VOB                         // DTV should not be called VOB
              || process.NAV_Loc >= 0            // DTV should not have NAV packs
          ))
              iVolume_Boost = 0;
      }
      
      if (PlayCtl.iAudioFloatingOvfl > 0)
      {
          if (PlayCtl.iAudioFloatingOvfl > 16)
              fCurrent = fCurrent * 0.6f;
          else
          if (PlayCtl.iAudioFloatingOvfl > 12)
              fCurrent = fCurrent * 0.7f;
          else
          if (PlayCtl.iAudioFloatingOvfl >  8)
              fCurrent = fCurrent * 0.8f;
          else
          if (PlayCtl.iAudioFloatingOvfl >  4)
              fCurrent = fCurrent * 0.9f;
          else
              fCurrent = fCurrent * 0.95f;
      }

      *psSample++=(short)(fCurrent); //fPCMData[k]+((fPCMData[k]>0)?0.5:-0.5));

    } // endfor each sample

    // auto-recover from corrupted data that causes temporary overflow
    if (PlayCtl.iAudioFloatingOvfl > 0)
        PlayCtl.iAudioFloatingOvfl--;

    break;

  case 3:
  case 4://32bit int

    for (k=0;k<dwSamples;++k) 
    {
       plSample=(long*)(pByte+k*byTByPerSample);
      *plSample=(long)(/*dNormGain*/fPCMData[k]+((fPCMData[k]>0)?0.5:-0.5));
    }
    break;
  }
}