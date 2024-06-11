
// Volume controls for Perview/Playback


#include "global.h"
#include "Audio.h"
#include "TXT_EN.h"

void VOL203_Volume_Target()
{
  Set_Toggle_Menu('T', &iCtl_Volume_Limiting, IDM_VOLUME_LIMITING);

  if (iCtl_Volume_Limiting)
  {
     if (iCtl_Audio_Ceiling > 24000)
         iCtl_Audio_Ceiling = 24000; // MAX=32767

     sprintf(szMsgTxt, VOLUME_LIMITING);

  }
  else
  {
     iCtl_Audio_Ceiling = 32767; // MAX=32767
  }

} 




void VOL204_Volume_Mute()
{
     iPlayAudio = 0; iWantAudio = 0;
     CheckMenuItem(hMenu, IDM_VOLUME_MUTE,   MF_CHECKED);
}




void VOL206_Volume_UN_Mute()
{
     iPlayAudio = 1; iWantAudio = 1;
     CheckMenuItem(hMenu, IDM_VOLUME_MUTE,   MF_UNCHECKED);

     if (iAudio_SEL_Track < CHANNELS_MAX)
     {
         SubStream_CTL[FORMAT_AC3][iAudio_SEL_Track].rip = 0;
         mpa_Ctl[iAudio_SEL_Track].rip = 0;
     }
}




void VOL300_Volume_Boost()
{
  /*
  int iBoostMin;

  if (SubStream_CTL[FORMAT_AC3][iAudio_SEL_Track].mode == 7)  // ac3 almost always needs extra boost
      iBoostMin = iCtl_Volume_Boost_AC3;
  else
  if (mpa_Ctl[iAudio_SEL_Track].uMPA_Sample_Hz == 48000)
      iBoostMin = iCtl_Volume_Boost_MPA_48k;
  else
      iBoostMin = iCtl_Volume_Boost_MPA_other;

  // iBoostMin+=7;
  */

  if(! iPlayAudio)
  {
     iPlayAudio = 1;
     CheckMenuItem(hMenu, IDM_VOLUME_MUTE,   MF_UNCHECKED);
  }
  else
  if (!iCtl_Volume_Boost)
  {
      Set_Toggle_Menu('S', &iCtl_Volume_Boost, IDM_VOLUME_BOOST);
      VOL305_Volume_Boost_Start();
  }
  else 
  /*
  if (PlayCtl.iAudioFloatingOvfl)
      PlayCtl.iAudioFloatingOvfl = 0;
  else
  if (iVolume_Boost < iBoostMin)
      iVolume_Boost = iBoostMin;
  else
  */
  if (iVolume_Boost >= (K_BOOST_DENOM*128))
  {
      sprintf(szMsgTxt, VOLUME_AT_MAX);
      DSP1_Main_MSG(0,1);
  }
  else
  if (iVolume_Boost >= (K_BOOST_DENOM*5/4))  // big jumps
      iVolume_Boost = iVolume_Boost * 4 / 3;
  else
  if (iVolume_Boost >= (K_BOOST_DENOM*6/8)) 
      iVolume_Boost+=4;
  else
  if (iVolume_Boost >= (K_BOOST_DENOM*3/8)) 
      iVolume_Boost +=2;
  else
      iVolume_Boost++;

  //if (MParse.ShowStats_Flag)
      Stats_Volume_Boost();
} 





void VOL305_Volume_Lesser()
{

  if (iVolume_BOLD > iVolume_Boost)
  {
      if (iVolume_Boost > K_BOOST_DENOM)
          iVolume_Boost--;
      iVolume_BOLD = iVolume_Boost;
  }
  else
  if (iVolume_Boost == 0)
  {
      iVolume_Boost = (K_BOOST_DENOM*5/8);
  }
  else
  if (iVolume_Boost == 1)
  {
      //if ( ! iCtl_Volume_Limiting)
      //    VOL203_Volume_Target();
      //else
      if (iWantAudio)
      {
          VOL204_Volume_Mute();
      }
  }
  else
  if (iVolume_Boost > (K_BOOST_DENOM*5/8))
      iVolume_Boost = iVolume_Boost * 4 / 5;
  else
  if (iVolume_Boost > (K_BOOST_DENOM*3/8))
      iVolume_Boost-=2;
  else
  if (iVolume_Boost > 1)
      iVolume_Boost--;
  else
  {
      if (iWantAudio)
      {
          VOL204_Volume_Mute();
      }
  }

  if (iVolume_BOLD)
      iVolume_BOLD = iVolume_Boost;

  //if (MParse.ShowStats_Flag)
      Stats_Volume_Boost();
} 


void VOL306_Volume_LessBold()
{
  if (iVolume_BOLD > iVolume_Boost)
  {
      if (iVolume_Boost > 1)
          iVolume_Boost--;
      iVolume_BOLD = iVolume_Boost;
  }
  else
  if (iVolume_BOLD > (K_BOOST_DENOM*5/8))
      iVolume_BOLD = iVolume_BOLD * 4 / 5;
  else
  if (iVolume_BOLD > (K_BOOST_DENOM*3/8))
      iVolume_BOLD-=2;
  else
      iVolume_BOLD--;

  iVolume_Boost = iVolume_BOLD;
  Stats_Volume_Boost();
}




void VOL305_Volume_Boost_Start()
{
   if ( ! iCtl_Volume_Boost)
       iVolume_Boost = 0;
   else
   {
     if (SubStream_CTL[FORMAT_AC3][iAudio_SEL_Track].mode == 7)  // ac3 almost always needs extra boost
         iVolume_Boost = iCtl_Volume_Boost_AC3;
     else
     if (mpa_Ctl[iAudio_SEL_Track].uMPA_Sample_Hz == 48000)
         iVolume_Boost = iCtl_Volume_Boost_MPA_48k;
     else
         iVolume_Boost = iCtl_Volume_Boost_MPA_other;
   }

   if (iCtl_Volume_BOLD)
       iCtl_Volume_BOLD = iVolume_Boost;

   PlayCtl.iAudioFloatingOvfl = 0;

}





void VOL307_Volume_Bolder()
{
  /*
  int iBoostMin;

  if (SubStream_CTL[FORMAT_AC3][iAudio_SEL_Track].mode == 7)  // ac3 almost always needs extra boost
      iBoostMin = iCtl_Volume_Boost_AC3;
  else
  if (mpa_Ctl[iAudio_SEL_Track].uMPA_Sample_Hz == 48000)
      iBoostMin = iCtl_Volume_Boost_MPA_48k;
  else
      iBoostMin = iCtl_Volume_Boost_MPA_other;

  iBoostMin+=7;

  if (iVolume_BOLD < iBoostMin)
      iVolume_BOLD = iBoostMin;
  else
  */

  if (iVolume_BOLD >= (K_BOOST_DENOM*128))  // Do not get silly
  {
      sprintf(szMsgTxt, VOLUME_AT_MAX);
      DSP1_Main_MSG(0,1);
  }
  else
  if (iVolume_BOLD >= (K_BOOST_DENOM*5/8))  // big jumps
      iVolume_BOLD = iVolume_BOLD * 4 / 3;
  else
  if (iVolume_BOLD >= (K_BOOST_DENOM*3/8))  // big jumps
      iVolume_BOLD = iVolume_BOLD+=2;
  else
      iVolume_BOLD = iVolume_BOLD++;

  iCtl_Volume_BOLD = 1;
  CheckMenuItem(hMenu, IDM_VOLUME_BOLD,  MF_CHECKED);
  iCtl_Volume_Boost = 1;
  CheckMenuItem(hMenu,  IDM_VOLUME_BOOST, MF_CHECKED);

  VOL300_Volume_Boost();
}







