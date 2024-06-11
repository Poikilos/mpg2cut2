
// Volume controls for Preview/Playback

// TODO:- Fix jitter on boost slider when AVC is on

#include "global.h"
#include "Audio.h"
#include "TXT_EN.h"

#define VOL_ADJ_UP    6 / 5
#define VOL_ADJ_DOWN  5 / 6

unsigned uStatus;

void BoostTick()
{
  MenuTick( IDM_VOLUME_BOOST);
  MenuTick(uBOOST_CAT_MENU[iVol_Boost_Cat]);
}
void BoostUnTick()
{
  MenuUnTick( IDM_VOLUME_BOOST);
  MenuUnTick(uBOOST_CAT_MENU[iVol_Boost_Cat]);
}

void VOL202_Volume_Limit_Higher()
{
  char *lsMSG;
  lsMSG = 0;

  if (iCtl_Volume_Limiting)
  {
      iVolume_Ceiling = iVolume_Ceiling * VOL_ADJ_UP;
      if (iVolume_Ceiling >= 32767) // Reached Max ?
      {
          iVolume_Ceiling = 32767;
          ToggleMenu('C', &iCtl_Volume_Limiting, IDM_VOLUME_LIMITING);
          lsMSG = &VOLUME_LIMIT_OFF;
      }
      else
          lsMSG = &VOLUME_LIMIT_UP;

      iCtl_Volume_Ceiling = iVolume_Ceiling;
  }
  else
          lsMSG = &VOLUME_AT_MAX;  

  if (lsMSG)
  {
      strcpy(szMsgTxt, lsMSG);
      DSP1_Main_MSG(0,1);
  }

}

void VOL203_Ceiling_On()
{
  if (iCtl_Volume_Ceiling < 0)  // disabled
  {
      iCtl_Volume_Ceiling = 0 - iCtl_Volume_Ceiling;
      iVolume_Ceiling = iCtl_Volume_Ceiling; 
  }
     
  if (iCtl_Volume_Ceiling <= 1)  // default
      iCtl_Volume_Ceiling = K_VOL_CEILING_DEF; // quarter of max (max=32767)

  iVolume_Ceiling = iCtl_Volume_Ceiling;

  if (iCtl_Volume_Boost <= 0)
  {
      iCtl_Volume_Boost = 1;
      iVolume_Boost = K_BOOST_DENOM;
      //iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;
      BoostTick();
  }

  iCtl_Volume_Limiting = 1;
  iCtl_Volume_AUTO     = 1;
  iVolume_AUTO = iVolume_Boost;
  MenuTick(IDM_VOLUME_AUTO);


  if (hVolDlg0)
      Vol_Show_All();
}

void VOL203_Ceiling_Off()
{
  iCtl_Volume_Limiting = 0;
  iVolume_Ceiling = 32767; // MAX=32767
}


void VOL203_Volume_Target()
{
  ToggleMenu('T', &iCtl_Volume_Limiting, IDM_VOLUME_LIMITING);

  if (iCtl_Volume_Limiting)
  {
     VOL203_Ceiling_On();
     strcpy(szMsgTxt, VOLUME_LIMITING);
  }
  else
  {
     iVolume_Ceiling = 32767; // MAX=32767
  }

  if (hVolDlg0)
      Vol_Show_All();
} 




void VOL204_Volume_Mute()
{
     iPlayAudio = 0; iWantAudio = 0;
     CheckMenuItem(hMenu, IDM_VOLUME_MUTE,   MF_CHECKED);
     if (hVolDlg0)
        Vol_Show_Chks();
}




void VOL206_Volume_UN_Mute()
{
     iPlayAudio = 1; iWantAudio = 1;
     CheckMenuItem(hMenu, IDM_VOLUME_MUTE,   MF_UNCHECKED);
     if  (hVolDlg0)
          Vol_Show_Chks();

     if (iAudio_SEL_Track < CHANNELS_MAX)
     {
         SubStream_CTL[FORMAT_AC3][iAudio_SEL_Track].rip = 0;
         mpa_Ctl[iAudio_SEL_Track].rip = 0;
     }
}


void VOL210_MUTE_Toggle()
{
  if (iWantAudio)
      VOL204_Volume_Mute();
  else
      VOL206_Volume_UN_Mute();
}




void VOL250_Volume_More()
{
  if (iVolume_Boost >= K_BOOST_SILLY)  // too silly ?
  {
      VOL202_Volume_Limit_Higher();           // maybe increase ceiling
  }
  else
  if (iVolume_Boost >= (K_BOOST_SILLY/4))  // getting silly ?
  {
      iVolume_Boost = iVolume_Boost * 2;  // very big adjust
  }
  else
  if (iVolume_Boost >= (K_BOOST_DENOM*5/8))        // moderate setting ?
      iVolume_Boost = iVolume_Boost * VOL_ADJ_UP;  // moderate adjust
  else
  //if (iVolume_Boost >= (K_BOOST_DENOM*6/8))        
  //    iVolume_Boost+=4;                            
  //else
  if (iVolume_Boost >= (K_BOOST_DENOM*3/8))        // small setting ?
      iVolume_Boost +=2;                           // small adjust
  else
  if (iVolume_Boost <= 0)        // ASIS ?
      iVolume_Boost =  K_BOOST_DENOM+2;  // small adjust above ASIS
  else
      iVolume_Boost++;                             // tiny adjust

  iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;
  //if (hVolDlg0)
  //    Vol_Show_All();
}




void VOL300_Volume_Boost(int P_Mode)
{

  if(! iPlayAudio || !iWantAudio)
     VOL206_Volume_UN_Mute();


  if (!iCtl_Volume_Boost || P_Mode)
  {
      VOL301_Volume_Boost_Start();
  }
  else
      VOL250_Volume_More();

  Stats_Volume_Boost();
} 




void VOL307_Boost_Started()
{
  if (iCtl_Volume_AUTO)
      iVolume_AUTO = iVolume_Boost;
  else
      iVolume_AUTO = 0;

  if (iVolume_Boost > K_BOOST_DENOM)
  {
      PlayCtl.iAudioFloatingOvfl = 0;
      BoostTick();
  }

  //if (iVolume_Boost)
      iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;
  //else
  //    iVol_BoostCat_Done[iVol_Boost_Cat] = K_BOOST_DENOM;
} 


void VOL309_Boost_Cat_Begin()
{
  if (iCtl_Vol_BoostCat_Flag[iVol_Boost_Cat])
  {
      if (iVol_BoostCat_Done[iVol_Boost_Cat]) // Maybe going back to a track adjusted previously
          iVolume_Boost = iVol_BoostCat_Done[iVol_Boost_Cat];
      else
          iVolume_Boost = iCtl_Vol_BoostCat_Init[iVol_Boost_Cat];
      iCtl_Volume_Boost =  1;
      VOL307_Boost_Started();
      iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;
  } // endif this type boosted
  else
  {
      BoostUnTick();
      iCtl_Volume_Boost = 0;  
      iVolume_Boost = 0;
  }

}


void VOL301_Volume_Boost_Start()
{
  // Have we played anything yet ?
  if (iVol_Boost_Cat > 0)
  {
     VOL309_Boost_Cat_Begin();
     iCtl_Vol_BoostCat_Flag[iVol_Boost_Cat] = 1;
     iCtl_Volume_Boost = 1;

     VOL307_Boost_Started();
  }
  else
  {
    // Not sure what to do if audio not yet detected
    // so for now just beep
    MessageBeep(MB_OK);
  }
}


/*
void  VOL302_Maybe_Reset()
{
  iPlayAudio = 0;

  if ((   iCtl_Volume_Boost > 0
       && iCtl_Volume_AUTO  > 0)
  // ||  !MParse.SeqHdr_Found_Flag
  )
  {
      VOL301_Volume_Boost_Start();
      //if (iCtl_Volume_AUTO)
      //    iVolume_AUTO = iVolume_Boost;
      //else
      //    iVolume_AUTO = 0;
  }
}
*/



void VOL303_Vol_Boost_On()
{
  iCtl_Vol_BoostCat_Flag[iVol_Boost_Cat] = 1;
  VOL301_Volume_Boost_Start();
  VOL300_Volume_Boost(1);
}


void VOL304_Vol_Boost_Off()
{
  iCtl_Volume_Boost = 0;
  if (iVol_Boost_Cat)
      iCtl_Vol_BoostCat_Flag[iVol_Boost_Cat] = 0;
  iVolume_Boost = 0;
  //iVol_BoostCat_Done[iVol_Boost_Cat] = 0;
  BoostUnTick();
  Stats_Volume_Boost();
}



void VOL305_Volume_Lesser()
{

  if (iVolume_Boost == 0)  // previously uncontrolled ?
  {
      iVolume_Boost = (K_BOOST_DENOM*5/8); // begin control at slight decrease 
  }
  else
  if (iVolume_Boost > (K_BOOST_DENOM*5/8))          // Big volume ?
      iVolume_Boost = iVolume_Boost * VOL_ADJ_DOWN; // big adjust
  else
  if (iVolume_Boost > (K_BOOST_DENOM*3/8))  // moderate volume ?
      iVolume_Boost-=2;                     // moderate adjust
  else
  if (iVolume_Boost > 1)                    // tiny volume
      iVolume_Boost--;                      // iny adjust
  else
  {   // already at minimum volume
      if (iWantAudio)
      {
          VOL204_Volume_Mute();
      }
  }

  if (iVolume_AUTO > iVolume_Boost
  &&  iVolume_AUTO)
      iVolume_AUTO = iVolume_Boost;

  iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;

  if (iVolume_Boost <= K_BOOST_DENOM)
      BoostUnTick();

  //if (MParse.ShowStats_Flag)
        Stats_Volume_Boost();
} 


void VOL306_Volume_LessBold()
{
  if (iVolume_AUTO  <= 1      // Already at minimum AUTO   setting ?
  ||  iVolume_Boost <= 1)     // Already at minimum Volume setting ?
  {
      VOL305_Volume_Lesser(); // Reduce volume directly
  }
  else
  {
    if (iVolume_AUTO > iVolume_Boost) // AUTO causing overflow ?
    {
       if (iVolume_Boost > 1)        // Above minimum boost ?
           iVolume_Boost--;          // Reduce boost to just under overflow
       iVolume_AUTO = iVolume_Boost; // Bring AUTO down to match 
    }
    else
    if (iVolume_AUTO > (K_BOOST_DENOM*5/8))          // Big setting ?
        iVolume_AUTO = iVolume_AUTO * VOL_ADJ_DOWN;  // Big adjust
    else
    if (iVolume_AUTO > (K_BOOST_DENOM*3/8))         // Moderate setting ?
        iVolume_AUTO-=2;                            // Noderate adjust
    else              
        iVolume_AUTO--;                             // tiny adjust

    iVolume_Boost = iVolume_AUTO;
    iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;
    Stats_Volume_Boost();
  }

}


void VOL320_Down()
{
  if (!iWantAudio)
      MessageBeep(MB_OK);
  else
  if (iVolume_AUTO > 1
  &&  iVolume_Boost)
       VOL306_Volume_LessBold();
  else
       VOL305_Volume_Lesser();
}



void VOL337_Volume_Bolder()
{
  iCtl_Volume_AUTO = 1;
  MenuTick(IDM_VOLUME_AUTO);
  iCtl_Volume_Boost = 1;

  if (iVolume_AUTO >= K_BOOST_SILLY) // too silly ?
  {
      VOL202_Volume_Limit_Higher();         // maybe increase ceiling
  }
  else
  if (iVolume_AUTO >= (K_BOOST_SILLY/4)) // getting silly ?
  {
      iVolume_AUTO = iVolume_AUTO * 2; // very big jump
  }
  else
  if (iVolume_AUTO >= (K_BOOST_DENOM*5/8))  // big jumps
      iVolume_AUTO = iVolume_AUTO * VOL_ADJ_UP;
  else
  if (iVolume_AUTO >= (K_BOOST_DENOM*3/8))  // big jumps
      iVolume_AUTO = iVolume_AUTO+=2;
  else
  {
      if (iVolume_AUTO <= 0)  // Previously turned off ?
      {
         if (iVolume_Boost <= 0)
             iVolume_Boost = K_BOOST_DENOM * VOL_ADJ_UP;
         iVolume_AUTO = iVolume_Boost;
      }
      else
         iVolume_AUTO++;         // small increment
  }

  if (iVolume_Boost > K_BOOST_DENOM)
      BoostTick();

  iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;
  VOL300_Volume_Boost(0);
}





void VOL340_Up()
{
  iCtl_Volume_Boost = 1;
  if (iVolume_Boost > K_BOOST_DENOM)
      BoostTick();


  if (iWantAudio == 0)  // Make sure not muted
      VOL206_Volume_UN_Mute();
  else
  if (!iVolume_Boost)
  {
      iVolume_Boost = K_BOOST_DENOM+2;
      if (iCtl_Volume_AUTO)
          iVolume_AUTO = iVolume_Boost; 
  }
  else
  if (iVolume_AUTO)
      VOL337_Volume_Bolder();
  else
      VOL250_Volume_More();

  if (iCtl_Volume_AUTO
  &&  iVolume_AUTO < iVolume_Boost)
      iVolume_AUTO = iVolume_Boost; 

  iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;
  Stats_Volume_Boost();

}









//----------------------------------------------------------------
#define true 1
#define false 0
#include <commctrl.h>

// VOLUME SLIDER CONTROL



// Lookup table for rough log conversion
// Deliberately linear at start,
// for finer controls at low levels.
#define BOOST_LOG_ARRAYSIZE 64
int iBoost_Log[BOOST_LOG_ARRAYSIZE+1] = 
{   0,    1,    2,    3,    4,    5,    6,    7,
    8,    9,   10,   11,   12,   13,   14,   15,   
   16,   17,   18,   19,   20,   21,   22,   23,   
   24,   25,   26,   27,   28,   29,   30,   31,
   32,   33,   35,   39,   44,   50,   56,   64,
   71,   80,   88,  100,  116,  121,  140,  156,
  200,  216,  256,  280,  316,  416,  516,  640,
  816, 1016, 1280, 1616, 2016, 2560, 3216, 4096, 0xFFFFFF};

int iBoost_Log_ix;

int iBOOST_Converter(int iP_Level)
{
  int ix, iLevel;

  iLevel = iP_Level;
  if (iLevel >= iBoost_Log[BOOST_LOG_ARRAYSIZE/2])
      ix = BOOST_LOG_ARRAYSIZE/2;
  else
      ix = 0;

  while (iBoost_Log[ix] < iLevel)
  {
    ix++;
  }

  if (ix > BOOST_LOG_ARRAYSIZE)
      ix = BOOST_LOG_ARRAYSIZE;

  iBoost_Log_ix = ix;
  return ix;
}


#define LIMIT_LOG_ARRAYSIZE 36
int iLimit_Log[LIMIT_LOG_ARRAYSIZE+1] = 
{  //544,   604,   671,   746,
  829,    921,  1023,  1209,
  1344,  1493,  1659,  1844,
  2047,  2313,  2570,  2856,
  3173,  3525,  3917,  4353, 
  4836,  5374,  5971,  6634,    
  7371,  8191,  8707,  9674, 
 10749, 11943, 13271, 14745,
 16383, 17414, 19349, 21499, 
 23887, 26542, 29491, 32767, 0xFFFFFF};

int iLimit_Log_ix;

int iLIMIT_Converter(int iP_Level)
{
  int ix, iLevel;

  iLevel = iP_Level;
  if (iLevel  < 0)
      iLevel  = 0 - iLevel;
  else
  if (iLevel == 0)
      iLevel  = 32767;

  if (iLevel == 1)
      iLevel  = K_VOL_CEILING_DEF;


  if (iLevel >= iLimit_Log[LIMIT_LOG_ARRAYSIZE/2])
      ix = LIMIT_LOG_ARRAYSIZE/2;
  else
      ix = 0;

  while (iLimit_Log[ix] < iLevel)
  {
    ix++;
  }

  if (ix > LIMIT_LOG_ARRAYSIZE)
      ix = LIMIT_LOG_ARRAYSIZE;

  iLimit_Log_ix = ix;
  return ix;
}




int iSliderPos;




void Vol_SetPos_Sliders()
{

  int iPos, iBase, iBaseDsp;

  if (iCtl_Volume_AUTO)
      iBase = iVolume_AUTO;
  else
      iBase = iVolume_Boost;

  if (iBase    <= 0)
  {
      iBaseDsp  = 0; 
      iBase     = K_BOOST_DENOM;
  }
  else
      iBaseDsp = iBase - K_BOOST_DENOM;

  sprintf(szTemp, "%+d", iBaseDsp); // iSliderPos); // 
  SetDlgItemText(hVolDlg0, VOL_BOOST_NUM, szTemp);

  sprintf(szTemp, "%d%%", ((iVolume_Ceiling+50)*100/32767));
  SetDlgItemText(hVolDlg0, VOL_LIMIT_NUM, szTemp);

  if (iSlider_Skip > 0)
    return;

  iPos = iBOOST_Converter(iBase);
  SendDlgItemMessage(hVolDlg0, VOL_BOOST_SLIDER, TBM_SETPOS, 1, iPos);

  iPos = iLIMIT_Converter(iVolume_Ceiling);
  SendDlgItemMessage(hVolDlg0, VOL_LIMIT_SLIDER, TBM_SETPOS, 1, iPos);

}



void Vol_SetRange(UINT P_Control, UINT P_Base, UINT P_Range, UINT P_TicFreq)
{
  SendDlgItemMessage(hVolDlg0, P_Control, TBM_SETRANGE, 0,
                                 MAKELPARAM(P_Base, P_Range));

  SendDlgItemMessage(hVolDlg0, P_Control, TBM_SETTICFREQ, 
                                               P_TicFreq, 0);
}


  WPARAM uSetting;
  unsigned int uSet_ButtonId;

void Vol_Set_Button_Chk()
{
  SendDlgItemMessage(hVolDlg0, uSet_ButtonId, BM_SETCHECK,
                               uSetting, 0);
}


void Vol_Show_BoostChk()
{
  if (iCtl_Volume_Boost)
      uSetting = BST_CHECKED;
  else
      uSetting = BST_UNCHECKED;
  uSet_ButtonId = VOL_BOOST_CHK;
  Vol_Set_Button_Chk();
}



void Vol_Show_Chks()
{
  Vol_Show_BoostChk();

  if (iCtl_Volume_AUTO)
      uSetting = BST_CHECKED;
  else
      uSetting = BST_UNCHECKED;
  uSet_ButtonId = VOL_AUTO_CHK;
  Vol_Set_Button_Chk();

  if (iWantAudio)
      uSetting = BST_UNCHECKED;
  else
      uSetting = BST_CHECKED;
  uSet_ButtonId = VOL_MUTE_CHK;
  Vol_Set_Button_Chk();


  if (iCtl_Volume_Limiting)
      uSetting = BST_CHECKED;
  else
      uSetting = BST_UNCHECKED;
  uSet_ButtonId = VOL_LIMIT_CHK;
  Vol_Set_Button_Chk();

  //Vol_SetNum_Boost();

  ShowWindow(hVolDlg0, SW_SHOW);
}

void Vol_Show_All()
{

  Vol_Show_Chks();
  Vol_SetPos_Sliders();
}



LRESULT CALLBACK Volume_Dialog(HWND hDialog, UINT message,
                               WPARAM wParam, LPARAM lParam)
{

  RECT Volrect;

//  static int iLast_Y_Slider=0, iLast_UV_Slider=0;
//  static int iOrig_ColorSpace;


  //int iTmp1; 
  int iWidth, iHeight, iVGALowerLimit, iMsg;//, iVolCtl;
  int iGapLeft, iGapRight, iGapTop, iGapBot, iDockTop;

  hVolDlg0 = hDialog;
  iMsg     = message;




  switch (message)
  {
  case WM_INITDIALOG:

         iSlider_Skip = 0;
         Vol_SetRange(VOL_BOOST_SLIDER, 0, BOOST_LOG_ARRAYSIZE-1, 1);
         Vol_SetRange(VOL_LIMIT_SLIDER, 0, LIMIT_LOG_ARRAYSIZE-1, 1);
         SendDlgItemMessage(hVolDlg0, VOL_BOOST_SLIDER, TBM_SETTIC, 
                                                    (WPARAM) 0, 16);

         GetWindowRect(hVolDlg0, &Volrect);
         iWidth  = Volrect.right  - Volrect.left;
         if (iWidth < 10)  iWidth  = 100;
         iHeight = Volrect.bottom - Volrect.top;
         if (iHeight < 10) iHeight = 400;
         iVGALowerLimit = rcAvailableScreen.bottom - iHeight;

         iDockTop = 0;
         if (iMainWin_State)
         {
             //if (iViewToolBar)
             {  // Align to top of screen
                Volrect.top = rcAvailableScreen.top; // Align to top of screen
             }
             // else
             // {  // Align to bottom of screen
             //    Volrect.top   = rcAvailableScreen.bottom - iHeight;
             //    Volrect.left  = rcAvailableScreen.left;
             // }
         }
         else
         {
            Calc_PhysView_Size(); //GetClientRect(hWnd, &crect);
            MainWin_Rect();

            // Horizontal position
            iGapLeft  = wrect.left               - rcAvailableScreen.left;// - Edge_Width 
            iGapTop   = wrect.top + crect.top    - rcAvailableScreen.top + iTopMargin;
            iGapBot   = rcAvailableScreen.bottom - wrect.bottom;
            iGapRight = rcAvailableScreen.right  - wrect.right;// - Edge_Width;

            // Maybe align Vol window on left edge of main window
            if (iGapLeft >= iWidth) // Plenty space to the left ?
            {
                Volrect.left  = wrect.left   - iWidth;
            }
            else
            if (iGapTop >= iHeight) // Plenty space above ?
            {
                Volrect.left = wrect.left;
                iDockTop = 1;
            }
            else
            if (iGapBot >= iHeight) // Plenty space below ?
            {
                Volrect.top  = wrect.bottom;
                Volrect.left = wrect.left;
            }
            else
            if (iGapRight >= iWidth) // Plenty on right ?
            {                  
                Volrect.left  = wrect.right;// Align to side of window
            }
            else
            //    Maybe partially overlap on left
            if (iGapTop  >= (iHeight * 3 / 4)) // reasonable space above ?
            {
                   Volrect.left  = rcAvailableScreen.left;
                   iDockTop = 1;
            } 
            else
            //    Maybe partially overlap below
            if (iGapBot  >= (iHeight / 3))     // reasonable space below ?
            {
                   Volrect.top  = iVGALowerLimit;
                   iDockTop = 1;
            } 
            else
            //    Maybe partially overlap on left
            if (iGapLeft >= (iWidth / 2)      // reasonable space on left
            ||  iGapLeft >= (iGapRight - 40)) // Better than on right ?
            {
                   Volrect.left  = wrect.left + 50; //rcAvailableScreen.left;
                   iDockTop = 1;
            } 
            else
            //     Partial overlap at right
            {      // Align to side of screen
                   Volrect.left  = rcAvailableScreen.right - iWidth;
                   iDockTop = 1;
            }

         } // END_ELSE not Maximized

         //  stay on screen
         if (Volrect.left < 0)
             Volrect.left = 0;


         if (iDockTop) // Vol overlap with main window ?
         {
             //  move Position UP to reduce obscuring of video area

             iGapTop = wrect.top - rcAvailableScreen.top;// - Edge_Width;
             if (iGapTop > iHeight)
             {
                 Volrect.top = wrect.top - iHeight;
             }
             else
             {
                 Volrect.top = rcAvailableScreen.top;
             }
         }

         //  stay on screen
         if (Volrect.top > iVGALowerLimit)
             Volrect.top = iVGALowerLimit;


         MoveWindow(hVolDlg0, Volrect.left,  Volrect.top,
                                             iWidth, iHeight, false);
         Stats_Volume_Boost();
         Vol_Show_All();


         return true;


     case WM_COMMAND:
        switch (LOWORD(wParam))
        {

          case VOL_BOOST_CHK:
               uStatus = SendDlgItemMessage(hVolDlg0,
                        VOL_BOOST_CHK, BM_GETCHECK, 1, 0);

               if (uStatus == BST_CHECKED)
               {
                  VOL303_Vol_Boost_On();
               }
               else
               {
                  VOL304_Vol_Boost_Off();
               }

               MenuTickCtl(IDM_VOLUME_BOOST, uStatus);

               break;




          case VOL_AUTO_CHK:
               uStatus = SendDlgItemMessage(hVolDlg0,
                            VOL_AUTO_CHK, BM_GETCHECK, 1, 0);

               if (uStatus == BST_CHECKED)
               {
                   iCtl_Volume_AUTO = 1;
                   VOL301_Volume_Boost_Start();
               }
               else
               {
                   iVolume_AUTO = 0; iCtl_Volume_AUTO = 0;
               }

               MenuTickCtl(IDM_VOLUME_AUTO, uStatus);

               break;


          case VOL_LIMIT_CHK:
               uStatus = SendDlgItemMessage(hVolDlg0,
                       VOL_LIMIT_CHK, BM_GETCHECK, 1, 0);

               if (uStatus == BST_CHECKED)
               {
                     VOL203_Ceiling_On();               
               }
               else
               {
                    VOL203_Ceiling_Off();  
               }

               MenuTickCtl(IDM_VOLUME_LIMITING, uStatus);

               break;


          case VOL_MUTE_CHK:
               uStatus = SendDlgItemMessage(hVolDlg0,
                       VOL_MUTE_CHK, BM_GETCHECK, 1, 0);

               if (uStatus == BST_CHECKED)
               {
                    VOL204_Volume_Mute();
               }
               else
               {
                    VOL206_Volume_UN_Mute();
               }
 
               break;


          case VOL_BOOST_ZERO:
               if  (iCtl_Volume_AUTO)
                    iVolume_AUTO = K_BOOST_DENOM;
               iVolume_Boost = K_BOOST_DENOM;
               iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;
               iSlider_Skip = -1;
               Stats_Volume_Boost();
               iSlider_Skip = 0;

               break;


                

          case VOL_EXIT:
          case IDCANCEL:

               DestroyWindow(hVolDlg0);
               hVolDlg = NULL;
               hVolDlg0 = 0;

               if (!iMainWin_State || iViewToolBar > 1)
                   DSP2_Main_SEL_INFO(1);
               DSP3_Main_TIME_INFO();

               return true;

         }
         break;



      case WM_HSCROLL:
         switch (GetWindowLong((HWND)lParam, GWL_ID))
         {
            case VOL_BOOST_SLIDER:
               if (iSlider_Skip)
                   break;
               iSlider_Skip = 1;
               iSliderPos = SendDlgItemMessage(hVolDlg0, 
                                       VOL_BOOST_SLIDER, TBM_GETPOS, 0, 0);
               iCtl_Volume_Boost = 1;
               //Vol_Show_BoostChk();

               iBoost_Log_ix = iSliderPos;
               if (iBoost_Log_ix <= 0)
               {
                   iBoost_Log_ix = 0;
                   VOL204_Volume_Mute();
                   iVolume_Boost = 1;
               }
               else
               {
                   if (iBoost_Log_ix > BOOST_LOG_ARRAYSIZE)
                       iBoost_Log_ix = BOOST_LOG_ARRAYSIZE;
                   if(! iPlayAudio || !iWantAudio)
                        VOL206_Volume_UN_Mute();
                   iVolume_Boost = iBoost_Log[iBoost_Log_ix];
               }
               iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;

               if  (iCtl_Volume_AUTO)
                    iVolume_AUTO = iVolume_Boost;
               else
                    iVolume_AUTO = 0;

               if (iVolume_Boost > K_BOOST_DENOM)
                   BoostTick();
               else
               if (iVolume_Boost == K_BOOST_DENOM)
               {
                   iCtl_Volume_Boost = 0; iVolume_Boost = 0;
                   iVol_BoostCat_Done[iVol_Boost_Cat] = iVolume_Boost;
                   BoostUnTick();
               }

               Stats_Volume_Boost();
               iSlider_Skip = 0;
               break;

            case VOL_LIMIT_SLIDER:
               if (iSlider_Skip)
                   break;
               iSlider_Skip = 1;
               iSliderPos = SendDlgItemMessage(hVolDlg0, 
                                       VOL_LIMIT_SLIDER, TBM_GETPOS, 0, 0);

               iLimit_Log_ix = iSliderPos;

               if (iLimit_Log_ix <= 0)
               {
                   iLimit_Log_ix = 1;
                   VOL204_Volume_Mute();
               }
               else
               if(! iPlayAudio || !iWantAudio)
                    VOL206_Volume_UN_Mute();

               
               if (iLimit_Log_ix >= LIMIT_LOG_ARRAYSIZE)
                   iLimit_Log_ix  = LIMIT_LOG_ARRAYSIZE;


               iVolume_Ceiling = iLimit_Log[iLimit_Log_ix];

               if (iVolume_Ceiling < 32767)
               {
                   MenuTick(IDM_VOLUME_LIMITING);
                   if (iCtl_Volume_Limiting <= 0);
                       iCtl_Volume_Limiting  = 1;
                   iCtl_Volume_Ceiling   = iVolume_Ceiling;
               }
               else
               {
                   iCtl_Volume_Limiting = 0;
                   iCtl_Volume_Ceiling   = K_VOL_CEILING_DEF;
                   MenuUnTick(IDM_VOLUME_LIMITING);
               }
               Stats_Volume_Boost();
               iSlider_Skip = 0;
               break;


         }
         break;



      default:
           if (DBGflag 
           && message >  2
           && message !=   20  && message !=    9
           && message != 1110  && message != 1206 
           && message != 1252  && message != 1202 
           && message != 1194  && message != 1166)
           {
               //if (message == WM_DISPLAYCHANGE)
               //    lpCmdName = &"DSPCHG";
               //else
               //    lpCmdName = &"?";
               
               sprintf(szDBGln, "\nMSG=> (%04d = x%04X)",
                         message, message);
               DBGout(szDBGln);

           }             

           //MsgReturn = DefWindowProc(hWnd, message, wParam, lParam);
           break;
   }


   return false;
}






