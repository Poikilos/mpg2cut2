// 

#include "windows.h"

#include "global.h"
#define true 1
#define false 0
#include <commctrl.h>
#include "Mpg2Cut2_API.h"



HWND hLumDlg0;
HCURSOR hNewCursor, hOldCursor;

int iLast_Button;
int iButtonIni, iButton_LUM_OVR;
int iLast_Reset=1;

void Lum_Button_Mark()
{
  char szTitle[4];
  //unsigned int uStatus;

  /*
  if (iLumEnable_Flag[iColorSpaceTab])
  {
    if (iLast_Button == IDL_LUM_A)
        uStatus = BST_CHECKED;
    else
        uStatus = BST_UNCHECKED;
    CheckDlgButton(hLumDlg,IDL_LUM_A,uStatus);
    
    if (iLast_Button == IDL_LUM_BOLD)
        uStatus = BST_CHECKED;
    else
        uStatus = BST_UNCHECKED;
    CheckDlgButton(hLumDlg,IDL_LUM_BOLD,uStatus);
    
    if (iLast_Button == IDL_LUM_C)
        uStatus = BST_CHECKED;
    else
        uStatus = BST_UNCHECKED;
    CheckDlgButton(hLumDlg,IDL_LUM_C,uStatus);
    
    if (iLast_Button == IDL_LUM_DEFAULT)
        uStatus = BST_CHECKED;
    else
        uStatus = BST_UNCHECKED;
    CheckDlgButton(hLumDlg,IDL_LUM_DEFAULT,uStatus);
    
  }
  
  else
  */
  {
    if (iLast_Button == IDL_LUM_A)
      strcpy(szTitle,"A.");
    else
      strcpy(szTitle,"A");
    SetDlgItemText(hLumDlg,IDL_LUM_A,szTitle);

    if (iLast_Button == IDL_LUM_BOLD)
      strcpy(szTitle,"B.");
    else
      strcpy(szTitle,"B");
    SetDlgItemText(hLumDlg,IDL_LUM_BOLD,szTitle);

    if (iLast_Button == IDL_LUM_C)
      strcpy(szTitle,"C.");
    else
      strcpy(szTitle,"C");
    SetDlgItemText(hLumDlg,IDL_LUM_C,szTitle);

    if (iLast_Button == IDL_LUM_DEFAULT)
        strcpy(szTitle,"D.");
    else
       strcpy(szTitle,"D");
    SetDlgItemText(hLumDlg,IDL_LUM_DEFAULT,szTitle);
  }
  

}


void LumEnable_Set()
{
     if (iLumEnable_Flag[iColorSpaceTab] == 0)
     {
         iLumEnable_Flag[iColorSpaceTab] = 1;
         SendDlgItemMessage(hLumDlg0, IDL_LUM_CHK, BM_SETCHECK,
                                                   BST_CHECKED, 0);
         CheckMenuItem(hMenu, IDM_LUMINANCE, MF_CHECKED);
     }
}


void Button_LUM_GO()
{
  iLumGamma [iColorSpaceTab] = iLumGamma [iButtonIni];  
  iLumGain  [iColorSpaceTab] = iLumGain  [iButtonIni]; 
  iLumOffset[iColorSpaceTab] = iLumOffset[iButtonIni]; 

  Lum_Button_Mark();
}

/*
void Lum_BC_Adj(int P_Adj)
{
  iLumEnable_Flag[iColorSpaceTab] = 1;
  iLumGain  [iColorSpaceTab] += P_Adj; 
  iLumOffset[iColorSpaceTab] += P_Adj; 
}
*/


void Lum_SetNum_Gain()
{
  sprintf(szTemp, "%d", iLumGain[iColorSpaceTab]-128);
  SetDlgItemText(hLumDlg0, IDC_CONTRAST, szTemp);
}

void Lum_SetPos_Gain()
{
  SendDlgItemMessage(hLumDlg0, IDL_CONTRAST_SLIDER, TBM_SETPOS, 1, iLumGain[iColorSpaceTab]);
  Lum_SetNum_Gain();
}


//--------

void Lum_SetNum_Offset()
{
  sprintf(szTemp, "%d", iLumOffset[iColorSpaceTab]);
  SetDlgItemText(hLumDlg0, IDC_BRIGHT, szTemp);
}

void Lum_SetPos_Offset()
{
  SendDlgItemMessage(hLumDlg0, IDL_BRIGHT_SLIDER, TBM_SETPOS, 1, iLumOffset[iColorSpaceTab]+128);
  Lum_SetNum_Offset();
}

//--------

void Lum_SetNum_Gamma()
{
  sprintf(szTemp, "%3d", iLumGamma[iColorSpaceTab] );
  SetDlgItemText(hLumDlg0, IDC_GAMMA, szTemp);
}




//--------

void Lum_SetNum_Sat()
{
  sprintf(szTemp, "%d", iSatGain[iColorSpaceTab]-100);
  SetDlgItemText(hLumDlg0, IDC_SAT_GAIN, szTemp);
}

void Lum_SetPos_Sat()
{
  SendDlgItemMessage(hLumDlg0, IDC_SAT_SLIDER, TBM_SETPOS, 1, 
                               iSatGain[iColorSpaceTab] + 99);
  Lum_SetNum_Sat();
}


//--------


void Lum_SetNum_Blue()
{
  sprintf(szTemp, "%3d", iSatAdd_U[iColorSpaceTab]);
  SetDlgItemText(hLumDlg0, IDC_BLUE_OFFSET, szTemp);
}
void Lum_SetPos_Blue()
{
  SendDlgItemMessage(hLumDlg0, IDC_BLUE_SLIDER, TBM_SETPOS, 1, 
                                   iSatAdd_U[iColorSpaceTab] + 20);
  Lum_SetNum_Blue();
}

//--------

void Lum_SetNum_Red()
{
  sprintf(szTemp, "%3d", iSatAdd_V[iColorSpaceTab]);
  SetDlgItemText(hLumDlg0, IDC_RED_OFFSET, szTemp);
}

void Lum_SetPos_Red()
{
  SendDlgItemMessage(hLumDlg0, IDC_RED_SLIDER, TBM_SETPOS, 1, 
                             iSatAdd_V[iColorSpaceTab] + 20);
  Lum_SetNum_Red();
}

//--------




void Lum_SetPos_Sliders()
{
  Lum_SetPos_Gain();
  Lum_SetPos_Offset();

  SendDlgItemMessage(hLumDlg0, IDL_GAMMA_SLIDER,  TBM_SETPOS, 
                                1, iLumGamma[iColorSpaceTab]);
  Lum_SetNum_Gamma();

  Lum_SetPos_Sat();
  Lum_SetPos_Blue();
  Lum_SetPos_Red();

  iLast_Button = 0;
  Lum_Button_Mark();
}


void LumLock_Bright()
{
  iLumOffset[iColorSpaceTab] = iLumGain[iColorSpaceTab] - 128;
  Lum_SetPos_Offset();
}


void LumLock_Gain()
{
  iLumGain[iColorSpaceTab] = (iLumOffset[iColorSpaceTab]) + 128;
  Lum_SetPos_Gain();
}


void Lum_A()
{
  iButtonIni = 5; 
  iLast_Reset = 1;  iLast_Button = IDL_LUM_A;
  iLumEnable_Flag[iColorSpaceTab] = 1;
  Button_LUM_GO();
}

void Lum_Bold() // can be invoked from GUI too
{
  iButtonIni = 3; 
  iLast_Reset = 1;  iLast_Button = IDL_LUM_BOLD;
  iLumEnable_Flag[iColorSpaceTab] = 1;
  Button_LUM_GO();
}

void Lum_C()
{
  iButtonIni = 4; 
  iLast_Reset = 1;  iLast_Button = IDL_LUM_C;
  iLumEnable_Flag[iColorSpaceTab] = 1;
  Button_LUM_GO();
}

void Lum_Default()
{
  iButtonIni = 2; 
  iLast_Reset = 1;  iLast_Button = IDL_LUM_DEFAULT;
  iLumEnable_Flag[iColorSpaceTab] = 1;
  Button_LUM_GO();
}



void SatLock_Blue()
{
  iSatAdd_U[iColorSpaceTab] =  iSatAdd_V[iColorSpaceTab] / 3;
  Lum_SetPos_Blue();
}

void SatLock_Red()
{
  iSatAdd_V[iColorSpaceTab] =  iSatAdd_U[iColorSpaceTab] * 3;
  Lum_SetPos_Red();
}



void Lum_SetRange(UINT P_Control, UINT P_Base, UINT P_Range, UINT P_TicFreq)
{
  SendDlgItemMessage(hLumDlg0, P_Control, TBM_SETRANGE, 0,
                                 MAKELPARAM(P_Base, P_Range));

  SendDlgItemMessage(hLumDlg0, P_Control, TBM_SETTICFREQ, 
                                               P_TicFreq, 0);
}


  WPARAM uSetting;
  unsigned int uSet_ButtonId;

void Lum_Set_Button_Chk()
{
  SendDlgItemMessage(hLumDlg0, uSet_ButtonId, BM_SETCHECK,
                               uSetting, 0);
}



void Lum_Show_All()
{

  if (iLumGamma[iColorSpaceTab] > 400) iLumGamma[iColorSpaceTab] = 400;
  if (iLumGamma[iColorSpaceTab] <   0) iLumGamma[iColorSpaceTab] =   0;

  Lum_SetPos_Sliders();

  if (iLumEnable_Flag[iColorSpaceTab])
      uSetting = BST_CHECKED;
  else
      uSetting = BST_UNCHECKED;
  uSet_ButtonId = IDL_LUM_CHK;
  Lum_Set_Button_Chk();

  if (iLumLock_Flag)
      uSetting = BST_CHECKED;
  else
      uSetting = BST_UNCHECKED;
  uSet_ButtonId = IDL_LUMLOCK_CHK;
  Lum_Set_Button_Chk();

  if (iSatAdj_Flag)
      uSetting = BST_CHECKED;
  else
      uSetting = BST_UNCHECKED;
  uSet_ButtonId = IDL_SAT_CHK;
  Lum_Set_Button_Chk();

  if (iSat_VHS)
      uSetting = BST_CHECKED;
  else
      uSetting = BST_UNCHECKED;
  uSet_ButtonId = LUM_SAT_VHS_CHK;
  Lum_Set_Button_Chk();

  if (iView_SwapUV)
      uSetting = BST_CHECKED;
  else
      uSetting = BST_UNCHECKED;
  uSet_ButtonId = LUM_SAT_SWAP_CHK;
  Lum_Set_Button_Chk();

  if (iSatLock_Flag)
      uSetting = BST_CHECKED;
  else
      uSetting = BST_UNCHECKED;
  uSet_ButtonId = IDC_SATLOCK_CHK;
  Lum_Set_Button_Chk();

  Lum_Button_Mark();

  ShowWindow(hLumDlg0, SW_SHOW);
}


void L602_MODE_RGB()
{
     Chg2RGB24(1, hLumDlg) ;
     iColorSpaceTab = 1;
}


void SatEnable_Set()
{
     if (iSatAdj_Flag == 0)
     {
         iSatAdj_Flag = 1; 
         SendDlgItemMessage(hLumDlg0, IDL_SAT_CHK, BM_SETCHECK,
                                                   BST_CHECKED, 0);
         CheckMenuItem(hMenu, IDM_LUMINANCE, MF_CHECKED);
     }
}



void Cursor_Old()
{  
  if (hOldCursor)
  {
      SetCursor(hOldCursor);
  }

  if (iButton_LUM_OVR > 0)
  {
      uSet_ButtonId = IDL_LUM_OVR;
      uSetting      = BST_UNCHECKED;
      Lum_Set_Button_Chk();
  }

  hOldCursor = 0;
  iButton_LUM_OVR = 0;
}


void Button_LUM_OVR()
{
  iLumGamma [iButtonIni] = iLumGamma [iColorSpaceTab];  
  iLumGain  [iButtonIni] = iLumGain  [iColorSpaceTab]; 
  iLumOffset[iButtonIni] = iLumOffset[iColorSpaceTab]; 

  Cursor_Old();
 
}


void Lum_Swap_UV(const int P_Reshow)
{
          Chg2YUV2(1, 0) ;
          Set_Toggle_Menu('T', &iView_SwapUV, IDM_YUV_SWAP);
          iView_SwapUV = iView_SwapUV<<1;

          if (P_Reshow
          &&  MParse.SeqHdr_Found_Flag && MParse.Stop_Flag)
          {
             if (DDOverlay_Flag && MParse.iColorMode==STORE_YUY2) // && iShowVideo_Flag)
                 RenderYUY2(1);
             //else
             //    RenderRGB24();
          }

}

void Lum_Negative(const int P_Reshow)
{
          Set_Toggle_Menu('T', &iView_Negative, IDM_VIEW_NEGATIVE);
          if (iView_Negative)
              iLumEnable_Flag[iColorSpaceTab] = 1;
          Lum_Filter_Init(0);
          Lum_Filter_Init(1);
          if (P_Reshow)
              RefreshVideoFrame();
}


//----------------------------------------------------------------
LRESULT CALLBACK Luminance_Dialog(HWND hDialog, UINT message,
                                WPARAM wParam, LPARAM lParam)
{

  RECT lumrect;

  static int iLast_Y_Slider=0, iLast_UV_Slider=0;
  static int iOrig_ColorSpace;


  int iTmp1;
  int iWidth, iHeight, iLowerLimit;
  int iGapLeft, iGapRight, iGapTop, iGapBot, iDockTop;

  hLumDlg0 = hDialog;

  /*
  if (iLast_Button >= 0
           && message >  2 && message !=   9  && message !=   297
           && message !=   20  && message !=   48  && message !=   85
           && message != 1110  && message != 1206 
           && message != 1252  && message != 1202 
           && message != 1194  && message != 1166)
  {
    iTmp1 = message;
  }
  */


  switch (message)
  {
  case WM_INITDIALOG:

         hOldCursor  = 0;
         iOrig_ColorSpace = MParse.iColorMode;
         iColorSpaceTab   = iOrig_ColorSpace;
         iButton_LUM_OVR = 0;

         if (iCtl_Lum_Deselector && iLum_Deselected)
         {
            iLum_Deselected = 0; // Option to remove boosting when outside selection
            Lum_Filter_Init(0);
            Lum_Filter_Init(1);
            RefreshVideoFrame();
         }

         if (iBMP_Wanted)
         {
             L602_MODE_RGB();
             iTmp1 = IDL_MODE_RGB;
         }
         else
         if (MParse.iColorMode == STORE_RGB24)
         {
           iTmp1 = IDL_MODE_RGB;
           iColorSpaceTab = 1;
         }
         else
           iTmp1 = IDL_MODE_OVL;

         SendDlgItemMessage(hLumDlg0, iTmp1, BM_SETCHECK, BST_CHECKED, 0);

         Lum_SetRange(IDL_GAMMA_SLIDER,    0, 402, 256);
         Lum_SetRange(IDL_CONTRAST_SLIDER, 0, 512, 128);
         Lum_SetRange(IDL_BRIGHT_SLIDER,   0, 512, 128);

         Lum_SetRange(IDC_SAT_SLIDER,    0, 499,  64);
         Lum_SetRange(IDC_BLUE_SLIDER,   0,  40,  20);
         Lum_SetRange(IDC_RED_SLIDER,    0,  40,  20);

         GetWindowRect(hLumDlg0, &lumrect);
         iWidth  = lumrect.right  - lumrect.left;
         iHeight = lumrect.bottom - lumrect.top;
         iLowerLimit = rcAvailableScreen.bottom - iHeight;

         iDockTop = 0;
         if (iMainWin_State)
         {
             //if (iViewToolBar)
             {  // Align to top of screen
                lumrect.top = rcAvailableScreen.top; // Align to top of screen
             }
             // else
             // {  // Align to bottom of screen
             //    lumrect.top   = rcAvailableScreen.bottom - iHeight;
             //    lumrect.left  = rcAvailableScreen.left;
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

            // Maybe align Lum window on left edge of main window
            if (iGapLeft >= iWidth) // Plenty space to the left ?
            {
                lumrect.left  = wrect.left   - iWidth;
            }
            else
            if (iGapTop >= iHeight) // Plenty space above ?
            {
                lumrect.left = wrect.left;
                iDockTop = 1;
            }
            else
            if (iGapBot >= iHeight) // Plenty space below ?
            {
                lumrect.top  = wrect.bottom;
                lumrect.left = wrect.left;
            }
            else
            if (iGapRight >= iWidth) // Plenty on right ?
            {                  
                lumrect.left  = wrect.right;// Align to side of window
            }
            else
            //    Maybe partially overlap on left
            if (iGapTop  >= (iHeight * 3 / 4)) // reasonable space above ?
            {
                   lumrect.left  = rcAvailableScreen.left;
                   iDockTop = 1;
            } 
            else
            //    Maybe partially overlap below
            if (iGapBot  >= (iHeight / 3))     // reasonable space below ?
            {
                   lumrect.top  = iLowerLimit;
                   iDockTop = 1;
            } 
            else
            //    Maybe partially overlap on left
            if (iGapLeft >= (iWidth / 2)      // reasonable space on left
            ||  iGapLeft >= (iGapRight - 40)) // Better than on right ?
            {
                   lumrect.left  = rcAvailableScreen.left;
                   iDockTop = 1;
            } 
            else
            //     Partial overlap at right
            {      // Align to side of screen
                   lumrect.left  = rcAvailableScreen.right - iWidth;
                   iDockTop = 1;
            }

         } // END_ELSE not Maximized

         //  stay on screen
         if (lumrect.left < 0)
             lumrect.left = 0;


         if (iDockTop) // Lum overlap with main window ?
         {
             //  move Position UP to reduce obscuring of video area

             iGapTop = wrect.top - rcAvailableScreen.top;// - Edge_Width;
             if (iGapTop > iHeight)
             {
                 lumrect.top = wrect.top - iHeight;
             }
             else
             {
                 lumrect.top = rcAvailableScreen.top;
             }
         }

         //  stay on screen
         if (lumrect.top > iLowerLimit)
             lumrect.top = iLowerLimit;


         MoveWindow(hLumDlg0, lumrect.left,  lumrect.top,
                                             iWidth, iHeight, false);
         Lum_Show_All();

         return true;


     case WM_COMMAND:
        switch (LOWORD(wParam))
        {
          case IDL_MODE_OVL:
               iColorSpaceTab = 0;
               SendDlgItemMessage(hLumDlg0, IDL_MODE_OVL, BM_SETCHECK,
                                                            BST_CHECKED, 0);
               SendDlgItemMessage(hLumDlg0, IDL_MODE_RGB, BM_SETCHECK,
                                                            BST_UNCHECKED, 0);
               Lum_Show_All();
               //Lum_Filter_Init(0);

               Chg2YUV2(1, hLumDlg);

               //RefreshVideoFrame();
               break;

          case IDL_MODE_RGB:
               L602_MODE_RGB();
               SendDlgItemMessage(hLumDlg0, IDL_MODE_RGB, BM_SETCHECK,
                                                          BST_CHECKED, 0);
               SendDlgItemMessage(hLumDlg0, IDL_MODE_OVL, BM_SETCHECK,
                                                          BST_UNCHECKED, 0);
               Lum_Show_All();

               //RefreshVideoFrame();
               break;


          case IDL_LUM_A:

               if (iButton_LUM_OVR <= 0)
               {
                   Lum_A();
                   Lum_Show_All();  // Lum_SetPos_Sliders();
                   RefreshVideoFrame();
               }
               else
               {
                   iButtonIni = 5; 
                   Button_LUM_OVR();
               }

               break;


          case IDL_LUM_BOLD:

               iButtonIni = 3; 
               if ( iButton_LUM_OVR <= 0)
               {
                    Lum_Bold();
                    Lum_Show_All();  // Lum_SetPos_Sliders();      
                    RefreshVideoFrame();
               }
               else
               {
                    iButtonIni = 3; 
                    Button_LUM_OVR();
               }

               break;


          case IDL_LUM_C:

               if (iButton_LUM_OVR <= 0)
               {
                   Lum_C();
                   Lum_Show_All();  // Lum_SetPos_Sliders();
                   RefreshVideoFrame();
               }
               else
               {
                   iButtonIni = 4; 
                   Button_LUM_OVR();
               }

               break;


          case IDL_LUM_DEFAULT:
               if ( iButton_LUM_OVR <= 0)
               {
                   Lum_Default();
                   Lum_Show_All();  // Lum_SetPos_Sliders();
                   RefreshVideoFrame();
               }
               else
               {
                   iButtonIni = 2; 
                   Button_LUM_OVR();
               }

               break;


          case IDL_LUM_ZERO:
               if ( iButton_LUM_OVR <= 0)
               {
                    iLast_Reset = 0;  iLast_Button = IDL_LUM_ZERO;

                    iLumGain[iColorSpaceTab] = 128; 
                    iLumOffset[iColorSpaceTab] = 0; 

                    //if (iColorSpaceTab)
                    //  iTmp1 = 150;
                    //else
                    //  iTmp1 = 130;
                    iLumGamma[iColorSpaceTab] = 100; //iTmp1;
                    iLumEnable_Flag[iColorSpaceTab] = 0;

                    Lum_Show_All();  // Lum_SetPos_Sliders();

                    RefreshVideoFrame();
               }
               else
               {
                   iButton_LUM_OVR = 0;  // Escape button changer
                   MessageBeep(MB_OK);
               }

               break;


          case IDL_SAT_BOLD:
               if ( iButton_LUM_OVR <= 0)
               {
                    iLast_Button = IDL_SAT_BOLD;
                    iSatAdd_U[iColorSpaceTab]   = 2;   
                    iSatAdd_V[iColorSpaceTab]   = 6;

                    iSatGain[0] = 100; 
                    iSatGain[1] = 100;

                    iSatAdj_Flag = 1;  iSatLock_Flag = 0;

                    Lum_Show_All();  // Lum_SetPos_Sliders();

                    RefreshVideoFrame();
               }
               else
               {
                   MessageBeep(MB_OK);
               }
               break;


          case IDL_SAT_DEFAULT:
               if ( iButton_LUM_OVR <= 0)
               {
                    iLast_Button = IDL_SAT_DEFAULT;
                    iSatAdd_U[iColorSpaceTab]   = 1;   
                    iSatAdd_V[iColorSpaceTab]   = 1;
                    iSatGain[0] = 100; iSatGain[1] = 100;

                    iSatAdj_Flag = 1;  iSatLock_Flag = 0;

                    Lum_Show_All();  // Lum_SetPos_Sliders();

                    RefreshVideoFrame();
               }
               else
               {
                   MessageBeep(MB_OK);
               }

               break;


          case IDL_SAT_ZERO:
               if ( iButton_LUM_OVR <= 0)
               {
                   iLast_Button = IDL_SAT_ZERO;
                   iSatAdd_U[iColorSpaceTab]   = 0;   iSatAdd_V[iColorSpaceTab]   = 0; 
                   iSatGain[0] = 100; iSatGain[1] = 100;
                   iSatAdj_Flag = 0;

                   Lum_SetPos_Sliders();

                   RefreshVideoFrame();
               }
               else
               {
                   iButton_LUM_OVR = 0;  // Escape button changer
                   MessageBeep(MB_OK);
               }

               break;


          case IDL_LUM_NEG_CHK:
               if (!iView_Negative)
               {
                  //Set_Toggle_Menu('S', &iCtl_View_Fast_YUV, IDM_YUV_FAST);
                  iView_Fast_YUV = 1;
               }
               Lum_Negative(0);

               Lum_Show_All();  // Lum_SetPos_Sliders();
               RefreshVideoFrame();
               break;




          case IDL_LUM_CHK:
               if (SendDlgItemMessage(hLumDlg0,
                       IDL_LUM_CHK, BM_GETCHECK, 1, 0) == BST_CHECKED)
               {
                  CheckMenuItem(hMenu, IDM_LUMINANCE, MF_CHECKED);
                  iLumEnable_Flag[iColorSpaceTab] = 1;
               }
               else
               {
                  CheckMenuItem(hMenu, IDM_LUMINANCE, MF_UNCHECKED);
                  iLumEnable_Flag[iColorSpaceTab] = 0;
               }

               RefreshVideoFrame();
               break;



          case LUM_SAT_VHS_CHK:
               if (SendDlgItemMessage(hLumDlg0,
                         LUM_SAT_VHS_CHK, BM_GETCHECK, 1, 0) == BST_CHECKED)
               {
                  iSat_VHS = 1;
                  iSatAdj_Flag = 1;
                  
                  //iSatAdd_U[iColorSpaceTab]   = 1;   
                  //iSatAdd_V[iColorSpaceTab]   = 1; 
                  //iSatGain[0] = 100; 
                  //iSatGain[1] = 100;

                  //Set_Toggle_Menu('S', &iCtl_View_Fast_YUV, IDM_YUV_FAST);
                  iView_Fast_YUV = 1;
               }
               else
               {
                  iSat_VHS = 0;
               }

               Lum_Show_All();  // Lum_SetPos_Sliders();

               RefreshVideoFrame();
               break;


          case LUM_SAT_SWAP_CHK:
               if (!iView_SwapUV)
               {
                  iSatAdj_Flag = 1; 
                  //Set_Toggle_Menu('S', &iCtl_View_Fast_YUV, IDM_YUV_FAST);
                  iView_Fast_YUV = 1;
               }
               Lum_Swap_UV(0);

               Lum_Show_All();  // Lum_SetPos_Sliders();

               RefreshVideoFrame();
               break;


          case IDL_SAT_CHK:
               if (SendDlgItemMessage(hLumDlg0,
                         IDL_SAT_CHK, BM_GETCHECK, 1, 0) == BST_CHECKED)
               {
                  iSatAdj_Flag = 1;
                  //Set_Toggle_Menu('S', &iCtl_View_Fast_YUV, IDM_YUV_FAST);
                  iView_Fast_YUV = 1;
               }
               else
                  iSatAdj_Flag = 0;

               RefreshVideoFrame();
               break;


          case IDL_LUMLOCK_CHK:
               if (SendDlgItemMessage(hLumDlg0,
                       IDL_LUMLOCK_CHK, BM_GETCHECK, 1, 0) == BST_CHECKED)
               {
                  iLumLock_Flag = true;
                  if (iLast_Y_Slider == IDL_CONTRAST_SLIDER)
                      LumLock_Bright();
                  else
                      LumLock_Gain();

               }
               else
               {
                  iLumLock_Flag = false;
               }

               RefreshVideoFrame();
               break;



          case IDC_SATLOCK_CHK:
               if (SendDlgItemMessage(hLumDlg0,
                       IDC_SATLOCK_CHK, BM_GETCHECK, 1, 0) == BST_CHECKED)
               {
                  iSatLock_Flag = true;

                  if (iLast_UV_Slider == IDC_BLUE_SLIDER)
                      SatLock_Red();
                  else
                      SatLock_Blue();
               }
               else
               {
                  iSatLock_Flag = false;
               }

               RefreshVideoFrame();
               break;

          case IDL_LUM_OVR: // Next button will get a new setting
               if (iButton_LUM_OVR <= 0)
               {
                   uSet_ButtonId = IDL_LUM_OVR;
                   uSetting      = BST_CHECKED;
                   Lum_Set_Button_Chk();

                   iButton_LUM_OVR = 1;
                   hNewCursor = hCSR_CROSS;
                   if (hNewCursor)
                       hOldCursor = SetCursor(hNewCursor);
                   else
                       hOldCursor  = 0;
               }
               else
               {
                   Cursor_Old(); // SetCursor(hOldCursor);
               }

               break;




               
               //case IDL_EXIT:
          case IDCANCEL:

               if (hOldCursor)
               {
                   Cursor_Old(); // SetCursor(hOldCursor);
               }
 
               DestroyWindow(hLumDlg0);
               hLumDlg = NULL;

               if (iBMP_Wanted)
                   SNAP_Save(IDM_BMP_ASIS, NULL); // (ThumbRec*)lParam) ;

               if (iOrig_ColorSpace    != MParse.iColorMode)
                  if (iOrig_ColorSpace == STORE_YUY2)
                      Chg2YUV2(1, hLumDlg);
                  else
                      Chg2RGB24(1, hLumDlg);

               if (!iMainWin_State || iViewToolBar > 1)
                   DSP2_Main_SEL_INFO(1);
               DSP3_Main_TIME_INFO();

               return true;

         //   default:
         //      DestroyWindow(hLumDlg0);
         //      hLumDlg = NULL;
         //      return true;

         }
         break;



      case WM_HSCROLL:
         switch (GetWindowLong((HWND)lParam, GWL_ID))
         {
            case IDL_GAMMA_SLIDER:
               iLumGamma[iColorSpaceTab] = SendDlgItemMessage(hLumDlg0,
                                        IDL_GAMMA_SLIDER, TBM_GETPOS, 0, 0);
               LumEnable_Set();
               Lum_SetNum_Gamma();
               break;

            case IDL_CONTRAST_SLIDER:
               iLumGain[iColorSpaceTab] = SendDlgItemMessage(hLumDlg0, 
                                         IDL_CONTRAST_SLIDER, TBM_GETPOS, 0, 0);
               LumEnable_Set();
               Lum_SetNum_Gain();
               iLast_Y_Slider = IDL_CONTRAST_SLIDER;
               if (iLumLock_Flag)
                   LumLock_Bright();
               break;

            case IDL_BRIGHT_SLIDER:
               iLumOffset[iColorSpaceTab] = SendDlgItemMessage(hLumDlg0, 
                                IDL_BRIGHT_SLIDER, TBM_GETPOS, 0, 0) 
                                - 128;

               LumEnable_Set();
               Lum_SetNum_Offset();
               iLast_Y_Slider = IDL_BRIGHT_SLIDER;
               if (iLumLock_Flag)
                   LumLock_Gain();
               break;


            case IDC_SAT_SLIDER:
               iSatGain[iColorSpaceTab] = SendDlgItemMessage(hLumDlg0, 
                                         IDC_SAT_SLIDER, TBM_GETPOS, 0, 0)
                                         - 99;
               SatEnable_Set();
               Lum_SetNum_Sat();
               break;

            case IDC_BLUE_SLIDER:
               iSatAdd_U[iColorSpaceTab] = SendDlgItemMessage(hLumDlg0,
                                        IDC_BLUE_SLIDER, TBM_GETPOS, 0, 0)
                                        - 20;

               SatEnable_Set();
               Lum_SetNum_Blue();

               iLast_UV_Slider = IDC_BLUE_SLIDER;
               if (iSatLock_Flag)
                    SatLock_Red();
               break;

            case IDC_RED_SLIDER:
               iSatAdd_V[iColorSpaceTab] = SendDlgItemMessage(hLumDlg0,
                                        IDC_RED_SLIDER, TBM_GETPOS, 0, 0)
                                        - 20;

               SatEnable_Set();
               Lum_SetNum_Red();
               iLast_UV_Slider = IDC_RED_SLIDER;
               if (iSatLock_Flag)
                    SatLock_Blue();
               break;
         }

         RefreshVideoFrame();
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


