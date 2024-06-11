
// Buttons. Buttons. Who's got the buttons ?

#include "windows.h"
#include "WINUSER.H"
#include "global.h"
#include "Buttons.h"
#define true 1
#define false 0
#include <commctrl.h>

//#define TOOL_BAR_WIDTH    476
int iFULLBAR_WIDTH, iPLAYBAR_WIDTH, iMIN_STACKED_WIDTH, iMAX_STACKED_WIDTH; //  560
int iSEPARATOR;

HWND  hStop,  hPlayP, hSingle, 
      hSlower, hFaster,
      hSlow1, hSlow2, hFast1, hFast2;

HWND  hBack1,  hBack2, hBack3, hBack4,  
      hFwd1,   hFwd2,  hFwd3,  hFwd4; 

int iPlayP_PosX, iStop_PosX,  iSingle_PosX;
int iSlow1_PosX, iSlow2_PosX, iSlower_PosX;
int iFast1_PosX, iFast2_PosX, iFaster_PosX, iFastX_PosX;

#define SPACER 2
//int iToolPosX[15]
int iLum_PosX, iZoom_PosX, iAddB_PosX, iVol_PosX;
int iTrackBar_PosX;

int iMarkL_PosX, iBack1_PosX, iBack2_PosX, iBack3_PosX, iBack4_PosX;
int iMarkR_PosX, iFwd1_PosX,  iFwd2_PosX,  iFwd3_PosX,  iFwd4_PosX;



// TTM_SETDELAYTIME
void CreateToolTip(HWND hWnd, LPSTR strTT)
{
  RECT rect;
  LPTSTR lptstr = strTT;
  HWND hWndTT;
  TOOLINFO ti;


  hWndTT = CreateWindowEx(WS_EX_TOPMOST,TOOLTIPS_CLASS,NULL,
                        WS_POPUP | TTS_NOPREFIX,CW_USEDEFAULT,CW_USEDEFAULT,
                        CW_USEDEFAULT,CW_USEDEFAULT,hWnd,NULL,hInst,NULL);
                
  if (hWndTT)
  {                       
      ti.cbSize = sizeof(TOOLINFO);
      ti.uFlags = TTF_SUBCLASS;
      ti.hwnd = hWnd;
      ti.hinst = hInst;
      ti.uId = 0;
      ti.lpszText = lptstr;


      GetClientRect(hWnd, &rect);
      ti.rect.left = rect.left;
      ti.rect.top = rect.top;
      ti.rect.right = rect.right;
      ti.rect.bottom = rect.bottom;

      SendMessage(hWndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
  }
}



HWND TB_Create(char *P_lpszDesc,   // int P_DISABLED,
                           int P_iX, 
                           int P_iY,
                         HMENU P_uNumber)
{
  HWND hHandle;
  hHandle = CreateWindow("BUTTON", 
               P_lpszDesc,

                           // BS_ICON | 
               // P_DISABLED  | 
               WS_CHILD | WS_VISIBLE | WS_DLGFRAME,

               P_iX, P_iY,
                           iTool_Wd, iTool_Ht,
                           hWnd_MAIN, 
               P_uNumber, 
                           hInst, NULL);

  if (hFont2
  && (!iCtl_Readability || DBGflag || iAudioDBG))
    SendMessage(hHandle, WM_SETFONT, (WPARAM)(hFont2), 0);

  return hHandle;

}


//-----------------------------------------------

void Toolbar_Chg()
{

  if (iMainWin_State == 0) 
      iCtl_ViewToolbar[0] = iViewToolBar;
  else
      iCtl_ViewToolbar[1] = iViewToolBar;

  Prev_Coded_Height = 0;

  VGA_GetSize();
  Calc_PhysView_Size();

  ToolBar_Destroy();

  if (! iViewToolBar)
  {
           ToolBar_Metrics();
           //iTimeY = 1; iMsgPosY = 1;
           if (iView_yFrom  > iTool_Ht)
           {
               iView_yFrom = (iView_yFrom - iTool_Ht) &  0xFFFFFFFC;
           }
           else
               iView_yFrom  = 0;
  } 

  DD_OverlayMask(1);
  
  if (iViewToolBar)
  {
           //SetBkColor(hDC, iCtl_Back_Colour);  // Background = Black
           ToolBar_Metrics();
           Sleep(50);
           ToolBar_Create();
           T590_Trackbar_SEL(); 

           //if (File_Limit)
           //    Enable_Disable(true, 0, true);
  }

  View_MOUSE_ALIGN(-1);
  View_Rebuild_Chk(1);

}

//----------------------------------------

void PlayTool_Metrics()
{

  int iAllow; //, iTmp1;
  
  iAllow = 13;

  iStop_PosX   = iVol_PosX + iTool_Wd + iSEPARATOR;  // 4*Number of buttons in shortest toolbar)+gap

  if (iSkipBar_PosY) // Stacked ?
  {
     iStop_PosX   = iToolbarWidth - iPLAYBAR_WIDTH; // -7; // + (iTool_Wd / 2); //((15 * iTool_Wd) / 2); // iMarkL_PosX;
     iMarkL_PosX  = iStop_PosX;
     //iSingle_PosX = iStop_PosX + iTool_Wd;
     //iSlow1_PosX  = iSingle_PosX + iTool_Wd;
     //iPlayP_PosX  = iSlow1_PosX + iTool_Wd;
  }
  else
  //if (iTrackbar_Big)
  {
     // iStop_PosX  = iToolbarWidth - (iPLAYBAR_WIDTH * 2); // ((41 * iTool_Wd)/2); // 33 = 2*Number of left hand buttons + 1
     //if (iToolbarWidth > (iTool_Wd * 26))
     //    iStop_PosX -= (iTool_Wd + 9);
     //iTmp1 = iTool_Wd * 20 / 2; // (2*Number of buttons) in MARK/SKIP toolbar
     iMarkL_PosX  = iToolbarWidth - iPLAYBAR_WIDTH; // - 6;  //iTmp1;
     if (iToolbarWidth > 26 * iTool_Wd)
         iStop_PosX   = iMarkL_PosX - iPLAYBAR_WIDTH - iTool_Wd; 
  }
  //else
  //{
  //   iStop_PosX  = iToolbarWidth - ( 11 * iTool_Wd);
  //   iPlayP_PosX  = iStop_PosX + iTool_Wd;
  //}

  iSingle_PosX = iStop_PosX    + iTool_Wd; //  + SPACER;
  iSlow2_PosX  = iSingle_PosX  + iTool_Wd;
  iSlow1_PosX  = iSlow2_PosX   + iTool_Wd;
  iSlower_PosX = iSlow1_PosX   + iTool_Wd;
  iPlayP_PosX  = iSlower_PosX  + iTool_Wd; //  + SPACER;

  iFaster_PosX  = iPlayP_PosX  + iTool_Wd; //  + SPACER;
  iFast1_PosX   = iFaster_PosX + iTool_Wd;
  iFast2_PosX   = iFast1_PosX  + iTool_Wd;
  iFastX_PosX   = iFast2_PosX  + iTool_Wd; //  + SPACER;


  iBack1_PosX    = iMarkL_PosX + iTool_Wd; //  + SPACER;
  iBack2_PosX    = iBack1_PosX + iTool_Wd;
  iBack3_PosX    = iBack2_PosX + iTool_Wd; 
  iBack4_PosX    = iBack3_PosX + iTool_Wd; 

  iFwd4_PosX    = iBack4_PosX  + iTool_Wd; //  + SPACER; 
  iFwd3_PosX    = iFwd4_PosX   + iTool_Wd;
  iFwd2_PosX    = iFwd3_PosX   + iTool_Wd;
  iFwd1_PosX    = iFwd2_PosX   + iTool_Wd;
  iMarkR_PosX   = iFwd1_PosX   + iTool_Wd; //  + SPACER;

}




void PlayTools_Create() // Extra buttons for special playback
{
  PlayTool_Metrics();

  hSingle = TB_Create(&"/",  iSingle_PosX, iPlayBar_PosY, (HMENU) ID_FWD_FRAME);

  hSlower = TB_Create(&"p-", iSlower_PosX, iPlayBar_PosY, (HMENU) IDM_PLAY_SLOWER);
  hFaster = TB_Create(&"p+", iFaster_PosX, iPlayBar_PosY, (HMENU) IDM_PLAY_FASTER);

  hSlow1  = TB_Create(&"s",  iSlow1_PosX, iPlayBar_PosY, (HMENU) IDM_PLAY_SLOW_1);
  hSlow2  = TB_Create(&"S",  iSlow2_PosX, iPlayBar_PosY, (HMENU) IDM_PLAY_SLOW_2A);
  hFast1  = TB_Create(&"f",  iFast1_PosX, iPlayBar_PosY, (HMENU) IDM_PLAY_FAST_1);
  hFast2  = TB_Create(&"FF", iFast2_PosX, iPlayBar_PosY, (HMENU) IDM_PLAY_FAST_2);

  hFastX  = TB_Create(&"*",  iFastX_PosX, iPlayBar_PosY, (HMENU) ID_CUE_FWD);

  if (iCtl_ToolTips)
  {
     CreateToolTip(hSingle, " SINGLE Frame Advance    (/) ");
     CreateToolTip(hSlower,  " Play SLOWER     (F9) ");
     CreateToolTip(hFaster,  " Play FASTER     ( Shift-F9 ) ");
     CreateToolTip(hSlow1,  " Play SLOW -33% ");
     CreateToolTip(hSlow2,  " Play SLOW -50% ");
     CreateToolTip(hFast1,  " Play FAST +50% ");
     CreateToolTip(hFast2,  " Play FAST +100% ");
     CreateToolTip(hFastX,  " PLAY EXTRA FAST  (NumPad-*) ");
  }
}



//----------------------------------------------------------------

void DSP_ButtonFont_Sizing()  // Scale button size to screen res
{

  char *szlpFontName;

  hDefaultGuiFont = GetStockObject(DEFAULT_GUI_FONT);


  /*
  BOOL GetCharWidth32(hDC,       // handle of device context 
    UINT 'W',        //  iFirstChar,  // first character in range to query  
    UINT 'W',        //  iLastChar,  // last character in range to query 
    LPINT &uDGF_Height,  lpBuffer  // address of buffer for widths 
   );

  if (iCtl_Readability)
      iTool_Wd  = (uDGF_Height *  2) + 2;
  else  */
      iTool_Wd  = (VGA_Width  * 30) / 800 + 1;  //

  iTool_Ht  = iTool_Wd;

  //if (VGA_Height <= 600)
  //    iTool_Ht   += 2;

  uFontHeight = iTool_Ht - 2;
  if (iCtl_Readability)
      uFontHeight = uFontHeight * 6 / 10;
  else
      uFontHeight = uFontHeight / 2;

  // For High-Res display, scale the font to make it readable
  if (VGA_Height <= 600
  && (!DBGflag && !iAudioDBG))
  {
      if (hFont1)
      {
          DeleteObject(hFont1);
          hFont1 = 0;
      }

  }
  else
  {
    if (DBGflag || iAudioDBG)
        szlpFontName = &"Courier New"; // DBG needs fixed pitch font
    else
        szlpFontName = &"Arial";

    hFont1 = CreateFont( 
                         uFontHeight,    // logical height of font 
                                   0,    // logical average character width 
                                   0,    // angle of escapement 
                                   0,    // base-line orientation angle 
                                 400,    // font weight 
                                   0,    // italic attribute flag 
                                   0,    // underline attribute flag 
                                   0,    // strikeout attribute flag 
                        ANSI_CHARSET,    // character set identifier 
                  OUT_DEFAULT_PRECIS,    // output precision 
                 CLIP_DEFAULT_PRECIS,    // clipping precision 
                 CLIP_DEFAULT_PRECIS,    // output quality 
      (DEFAULT_PITCH || FF_DONTCARE),    // pitch and family 
      szlpFontName    //  &"Verdana"     // pointer to typeface name string 
                                   );   

      //SendMessage(hMenu, WM_SETFONT, (WPARAM)(hFont1), MAKELPARAM(TRUE, 0));
      if (DBGflag || iAudioDBG)
          SendMessage(hWnd_MAIN, WM_SETFONT, (WPARAM)(hFont1), 0);

    
  }

  if (iCtl_Readability)
    hFont2 = hFont1;
  else
    hFont2 = (HFONT)(hDefaultGuiFont);


}




void D501_RESET_MainWindow()
{
  if (DBGflag)
  {
    DBGout("D501_RESET");
  }

   D500_ResizeMainWindow(iMIN_STACKED_WIDTH,  // iFULLBAR_WIDTH, 
                         MIN_OVL_HEIGHT, 1);
}




//-------------------------------------------------------------------
void D500_ResizeMainWindow(int P_Width, int P_Height, int P_Full)
{

  int iWindow_Mode;

  if (DBGflag)
      DBGout("D500_RESIZE");

  if (IsIconic(hWnd_MAIN))
    return;

  MainWin_Rect();

  if (iMainWin_State > 0)
     iWindow_Mode = SW_SHOWMAXIMIZED; // 0;
  else
     iWindow_Mode = 0;

  if (MParse.SystemStream_Flag < 0
  &&  Overlay_Width  >= P_Width 
  &&  Overlay_Height >= P_Height)
  {
      if (DBGflag)
          DBGout("   SKIPPED");

      return;
  }
    

  Overlay_Width  = P_Width ;
  Overlay_Height = P_Height ;



  //if (Overlay_Width  > VGA_Width ) Overlay_Width   = VGA_Width ;
  //if (Overlay_Height > VGA_Height) Overlay_Height = VGA_Height;

  if (Overlay_Width  > (VGA_Width - Edge_Width) )
  {
      Overlay_Width   = VGA_Width ;
      if (Prev_Coded_Width == 0)
         iWindow_Mode = SW_SHOWMAXIMIZED ;
  }


  ToolBar_Metrics();

  
  if (DBGflag)
  {
    sprintf(szDBGln, "RESIZE=%d.%d  W=%d.%d  C=%d.%d  O=%d.%d  T=%d.%d",
                     Overlay_Width, Overlay_Height,
                     wrect.right, wrect.bottom,
                     crect.right, crect.bottom,
                     Overlay_Width, Overlay_Height,
                     iToolbarWidth, iTool_Wd);
    DBGout(szDBGln);
  }


  //if (Overlay_Height > MIN_OVL_HEIGHT && iView_Aspect_Mode)
  //    Client_Height = Overlay_Height * iAspVert / 2048 + Edge_Height;
  // else
        Client_Height = Overlay_Height + Edge_Height;


  if (Client_Height > iViewMax_Height)
  {
      Client_Height = iViewMax_Height;
      //if (Prev_Coded_Width == 0)
      iWindow_Mode = SW_SHOWMAXIMIZED;
  }

  Client_Height += iTopMargin;



  if (Overlay_Width < iToolbarWidth)
      Client_Width  = iToolbarWidth;
  else
      Client_Width  = Overlay_Width;

  Client_Width = Client_Width + Edge_Width;


  if (iViewToolBar)
  {
    //MoveWindow(hOpenButton,    0, 0,
    //                              iTool_Wd, iTool_Ht, true);
    MoveWindow(hBmpButton,  0,            0,   iTool_Wd, iTool_Ht, true);
    MoveWindow(hAddButton,  iTool_Wd,     0,   iTool_Wd, iTool_Ht, true);
    MoveWindow(hLumButton,  iLum_PosX,    0,   iTool_Wd, iTool_Ht, true);
    MoveWindow(hZoomButton, iZoom_PosX,   0,   iTool_Wd, iTool_Ht, true);

    MoveWindow(hVolButton,  iVol_PosX,    iSkipBar_PosY,   iTool_Wd, iTool_Ht, true);

    MoveWindow(hMarkLeft,  iMarkL_PosX,  iSkipBar_PosY, iTool_Wd, iTool_Ht, true);
    MoveWindow(hBack4,     iBack4_PosX,  iSkipBar_PosY, iTool_Wd, iTool_Ht, true);
    MoveWindow(hBack3,     iBack3_PosX,  iSkipBar_PosY, iTool_Wd, iTool_Ht, true);
    MoveWindow(hBack2,     iBack2_PosX,  iSkipBar_PosY, iTool_Wd, iTool_Ht, true);
    MoveWindow(hBack1,     iBack1_PosX,  iSkipBar_PosY, iTool_Wd, iTool_Ht, true);
    MoveWindow(hFwd1,      iFwd1_PosX,   iSkipBar_PosY, iTool_Wd, iTool_Ht, true);
    MoveWindow(hFwd2,      iFwd2_PosX,   iSkipBar_PosY, iTool_Wd, iTool_Ht, true);
    MoveWindow(hFwd3,      iFwd3_PosX,   iSkipBar_PosY, iTool_Wd, iTool_Ht, true);
    MoveWindow(hFwd4,      iFwd4_PosX,   iSkipBar_PosY, iTool_Wd, iTool_Ht, true);
    MoveWindow(hMarkRight, iMarkR_PosX,  iSkipBar_PosY, iTool_Wd, iTool_Ht, true);

    //MoveWindow(hMsgTxt,      0,           iTool_Wd, iToolPosX[4], iTool_Wd,  true);
    //MoveWindow(hTimeTxt,     iMarkL_PosX, iTool_Wd, iTrackBar_PosX, iTool_Wd,  true);

    MoveWindow(hStop,   iStop_PosX,  iPlayBar_PosY, iTool_Wd, iTool_Ht, true);
    MoveWindow(hPlayP,  iPlayP_PosX, iPlayBar_PosY, iTool_Wd, iTool_Ht, true);

    
    //if (iTrackbar_Big)
    {
       //if (!hFastX)
       //{
       //  PlayTools_Create();
       //}
       MoveWindow(hSingle, iSingle_PosX, iPlayBar_PosY, iTool_Wd, iTool_Ht, true);
       MoveWindow(hSlow1,  iSlow1_PosX,  iPlayBar_PosY, iTool_Wd, iTool_Ht, true);
       MoveWindow(hSlow2,  iSlow2_PosX,  iPlayBar_PosY, iTool_Wd, iTool_Ht, true);
       MoveWindow(hSlower, iSlower_PosX, iPlayBar_PosY, iTool_Wd, iTool_Ht, true);
       MoveWindow(hFaster, iFaster_PosX, iPlayBar_PosY, iTool_Wd, iTool_Ht, true);
       MoveWindow(hFast1,  iFast1_PosX,  iPlayBar_PosY, iTool_Wd, iTool_Ht, true);
       MoveWindow(hFast2,  iFast2_PosX,  iPlayBar_PosY, iTool_Wd, iTool_Ht, true);
       MoveWindow(hFastX,  iFastX_PosX,  iPlayBar_PosY, iTool_Wd, iTool_Ht, true);
    }
  }

  if (iViewToolBar)
  {
    MoveWindow(hTrack,     iTrackBar_PosX, iTrackBar_PosY, iTrack_Wd, iTool_Wd, true);
  }


  if (P_Full || MParse.Stop_Flag || MParse.Pause_Flag)
  {
    if (!iMainWin_State)
    {

      MoveWindow(hWnd_MAIN, wrect.left, wrect.top,
                     Client_Width, Client_Height, true);
  
      Calc_PhysView_Size();
      if (DBGflag)
      {
        sprintf(szDBGln, "              NEW CLIENT=%d.%d",
                    Client_Width, Client_Height);
        DBGout(szDBGln);
      }
    }
    /*
    else
    {
       WINDOWPLACEMENT.length = sizeof(WINDOWPLACEMENT);
       WINDOWPLACEMENT.showCmd = SW_SHOWMAXIMIZED;
       SetWindowPlacement(hWnd,
                 WINDOWPLACEMENT *lpwndpl // address of structure with position data
                          )
    }
    */

    //SetRect(&orect, 0, 0,  Client_Width, Client_Height);
    if (iMsgLife > 0)
        DSP1_Main_MSG(1,0);

  }
 
   //if (iWindow_Mode)
   //{
   //      ShowWindow(hWnd, iWindow_Mode);
   //}

/*
  if (DBGflag)
  {
    sprintf(szDBGln, "Window=%d.%d  Max=%d  Full=%d\n",
             Client_Width, Client_Height, iWindow_Mode, P_Full);
    DBGout(szDBGln);
  }
*/

}



//--------------------------------------------------------------
void ToolBar_Metrics()
{

  int iTrue_Width, iTmp1, iTrig;
  //Calculate and store the position of each control

  //int iEdge2;
  //iEdge2 = Edge_Width<<1;
  //if (iEdge2 < 1) iEdge2 = 4;

  if (VGA_Height < 480)
      VGA_Height = 480;

  DSP_ButtonFont_Sizing();    // Scale button size to screen res
   
  iSEPARATOR          =  iTool_Wd/2;  // Min gap between toolbars
  iPLAYBAR_WIDTH      =  10 * iTool_Wd; //  + (3*SPACER+1);
  iMIN_STACKED_WIDTH  =  14 * iTool_Wd + iSEPARATOR; // Width of first 2 toolbars, including separator
  iMAX_STACKED_WIDTH  =  22 * iTool_Wd;      // Stack trigger point
  iFULLBAR_WIDTH      =  25 * iTool_Wd + (iSEPARATOR); // *2);
  if (iFULLBAR_WIDTH >= (VGA_Width - 80))
      iFULLBAR_WIDTH  =  VGA_Width - 2;


  iPlayBar_PosY = 0;  iSkipBar_PosY = 0;

  if (Overlay_Width  < iTool_Wd)  // Nothing loaded ?
      Overlay_Width  = iMIN_STACKED_WIDTH;

  if (iMainWin_State == 0)
      iTrue_Width = Overlay_Width;
  else
  {
      iTrue_Width = VGA_Width;
      iFULLBAR_WIDTH = VGA_Width;
  }

  // Calc positions of Left-Hand buttons
  iAddB_PosX = iTool_Wd;
  iLum_PosX  = iAddB_PosX + iTool_Wd;
  iZoom_PosX = iLum_PosX  + iTool_Wd;
  //iSelMsgX   = iZoom_PosX * 2;   // [3] + 15;


  // Trackbar Defaults
  iTrackbar_Big  = iCtl_Trackbar_Big; // && (File_Limit || iMainWin_State > 0);
  iTrack_Wd      = iToolbarWidth;
  if (iViewToolBar > 1)
     iTrackBar_PosY = iTool_Ht;
  else
     iTrackBar_PosY = 0;
  iTrackBar_PosX = 0;                             // Trackbar

  iTopMargin  = (iTool_Ht * 3 / 5) + 3; // Should really interrogate Text Height    
  iMsgPosY    = 1;

  iTrig = iVGA_Avail_Height - 70;
  if (iCentre_Cropped && iMainWin_State)
      iTrig = iTrig * 2 / 3;

  if (iViewToolBar < 257 && MParse.iColorMode != STORE_RGB24)
  {
    if (iAspect_Height > iTrig
    ||  iMainWin_State > 0
       )
    {
        iTopMargin   = 1;
    }
    //else
    //    iTopMargin   = 24;

    //if (iViewToolBar)
    //    iTopMargin += iTool_Ht;

  }


  iToolButton_Cnt = 15;

  // Calc positions of Left-Hand buttons and Trackbar

  if (iTrue_Width    < iFULLBAR_WIDTH)
  {
      iToolbarWidth  = iFULLBAR_WIDTH; 
      if (! iViewToolBar)
      {
         iMsgPosY    = 1;
         //iToolbarWidth  = iMIN_STACKED_WIDTH; // iMAX_STACKED_WIDTH;
      }
      else
      {

         if (iViewToolBar < 257)
             iTmp1 = iTool_Ht; 
         else
             iTmp1 = iTool_Ht * 2;

         iTopMargin += iTmp1;
         iMsgPosY   += iTmp1; 

         if (iTrue_Width  <= iMAX_STACKED_WIDTH) // Stack trigger point
         {   // Stack the toolbars vertically
             if (iTrue_Width   <= iMIN_STACKED_WIDTH)   
                 iToolbarWidth  = iMIN_STACKED_WIDTH;  // Width of first 2 toolbars
             else
                 iToolbarWidth  = iTrue_Width;

             iVol_PosX = iZoom_PosX;
             
             //iPlayBar_PosY   = iTool_Ht; 
             iSkipBar_PosY   = iTool_Ht;

             if (iViewToolBar > 1) 
             {
                 if (iViewToolBar & 1)
                     iTrackBar_PosY = iTool_Ht * 2;

                 //iSelMsgX       = 0;
                 iMsgPosY        = iTrackBar_PosY + iTool_Ht;
                 iTopMargin     += iTool_Ht;
             }
         }
         else
           iVol_PosX = iZoom_PosX  + iTool_Wd;

      }

      if (iTrue_Width <= iToolbarWidth)
          iTrue_Width  = iToolbarWidth + 2; //iFULLBAR_WIDTH * 2 / 3;

      iTrackbar_Big  = 1;
      iTrackBar_PosX = 0;                          // Trackbar
      //iToolPosX[4]   = iZoom_PosX + iLum_PosX;  // Message Text (NOT IMPLEMENTED)
  }
  else
  {   // Wider than minimum
      if (iTrue_Width >= (VGA_Width - 80))
          iToolbarWidth = VGA_Width - 2;//- iEdge2;
      else
      if (iTrue_Width > iToolbarWidth)
          iToolbarWidth = iTrue_Width ;//- iEdge2;
      else
         iToolbarWidth  = iFULLBAR_WIDTH; 

      iVol_PosX = iZoom_PosX  + iTool_Wd;

      if (iViewToolBar)
      {
        iTrack_Wd   = iToolbarWidth;
        if (iViewToolBar > 256)
            iTmp1 = iTool_Ht * 2;
        else
            iTmp1 = iTool_Ht;

        iTopMargin += iTmp1;
        iMsgPosY   += iTmp1; 

      }
      else
         iMsgPosY   = 1;

      //iSelMsgX       = iZoom_PosX + 45;   // [3] + 15;

  }

  if (Overlay_Height < MIN_OVL_HEIGHT)
      Overlay_Height = MIN_OVL_HEIGHT;

  iTrack_Wd      = iToolbarWidth;


  // Calc positions of Right-Hand buttons

  PlayTool_Metrics();

  if (iSkipBar_PosY)
      iSelMsgX   = iStop_PosX;
  else
  if (iView_TC_Format == 7 && process.total > 999*1024*1024) // Lengthy sel stats ?
      iSelMsgX   = iSingle_PosX; // dual time codes need more space on right
  else
  if (!iMainWin_State || VGA_Width < 800)
      iSelMsgX   = iSlow2_PosX;   // iSingle_PosX;
  else
      iSelMsgX   = iSlow1_PosX;

  iTimeX = iToolbarWidth - (7 * iTool_Wd) - 3; // 5; // iMarkL_PosX - 15;
  if (iTimeX < 0)
      iTimeX = 0;

  if (iTrue_Width >= iFULLBAR_WIDTH)  // 480 // (iAspect_Width >= 640)
  {
      iTimeY = iMsgPosY; //iTool_Ht;
  }
  else
  {
     // iTimeX  = iToolPosX[4] - 20;// - iTool_Wd;
     iTimeY     = iTopMargin;
     iTopMargin = iTopMargin + 24; // = iTopMargin<<1;
  }

  if (iMainWin_State > 0) // Maximized Window ?
     Calc_PhysView_Size();  // TODO: BUG INVOKING FROM HERE WHEN NOT MAX
  else
     iViewMax_Height  =  iVGA_Avail_Height - crect.top + wrect.top // KLUDGE
                                  - iTopMargin;


}




//-------------------


void  CreateToolTips_ALL()
{
  char *lpDesc;

   CreateToolTip(hBmpButton, " BMP Snapshot ");
   CreateToolTip(hAddButton, " ADD Selection to Clip List ");

   CreateToolTip(hStop,   " STOP     (esc)");
   CreateToolTip(hPlayP,   " PLAY ");

   CreateToolTip(hMarkLeft,  " MARK START of Clip ");
   CreateToolTip(hMarkRight, " MARK END of Clip ");


   CreateToolTip(hLumButton, " LUMINANCE Display Adjustment (Gamma) ");
   CreateToolTip(hZoomButton, " ZOOM ");

   if (iCtl_KB_V_Popup)
     lpDesc = &"VOLUME Control (v)";
   else
     lpDesc = &"VOLUME Control (Ctrl-V)";
   CreateToolTip(hVolButton, lpDesc);

   //if (iCtl_KB_NavOpt)
   {
      CreateToolTip(hBack1,  " STEP BACK 1 GOP  (Prev Key-Frame)   (<) ");
      CreateToolTip(hFwd1,   " STEP Forward 1 GOP  (Next Key-Frame) (>) ");

      CreateToolTip(hBack2,  " JUMP BACK  about 1 sec  ( Shift-< ) ");
      CreateToolTip(hFwd2,   " JUMP Forward  about 1 sec   ( Shift-> ) ");

      CreateToolTip(hBack3,  " JUMP Back MORE  about 20 sec  (^) ");
      CreateToolTip(hFwd3, " JUMP Forward MORE  about 20 sec  (DownArrow) ");

      CreateToolTip(hBack4,  " JUMP Back LOTS  about 40 sec  ( Shift-^ ) ");
      CreateToolTip(hFwd4, " JUMP Forward LOTS  about 40 sec  ( Shift-DownArrow ) ");
   }
   //else
   //{
   //   CreateToolTip(hBack1,  " JUMP BACK ");
   //   CreateToolTip(hFwd1, " JUMP Forward ");

   //   CreateToolTip(hBack2,  " STEP BACK 1 GOP  (Previous I-Frame) ");
   //   CreateToolTip(hFwd2, " STEP Forward 1 GOP  (Next I-Frame) ");
   //}

}



void DSP_Button_Abbr()
{
   // Key Assigment Options also impact button order
   // Because they are linked via the accelerator

   if (iCtl_KB_NavOpt)       // Approximate VDUB
   {
      hABack1 = (HMENU) ID_LEFT_ARROW;
      hABack2 = (HMENU) ID_LEFT_SHIFT;
      hABack3 = (HMENU) ID_UP_ARROW;
      hABack4 = (HMENU) ID_UP_SHIFT;
      hAFwd1  = (HMENU) ID_RIGHT_ARROW;
      hAFwd2  = (HMENU) ID_RIGHT_SHIFT;
      hAFwd3  = (HMENU) ID_DOWN_ARROW;
      hAFwd4  = (HMENU) ID_DOWN_SHIFT;
     /*
      strcpy(szAbbr1, "<<"); // "«") ;
      strcpy(szAbbr2, "<") ;
      strcpy(szAbbr3, ">") ;
      strcpy(szAbbr4, ">>"); // "»") ;
      strcpy(szAbbr5, "««");  
      strcpy(szAbbr6, "»»");  
      */
   }
   else
   {                         // Original DVD2AVI keys
      hABack1 = (HMENU) ID_DOWN_ARROW;
      hABack2 = (HMENU) ID_DOWN_SHIFT;
      hABack3 = (HMENU) ID_LEFT_ARROW;
      hABack4 = (HMENU) ID_LEFT_SHIFT;
      hAFwd1  = (HMENU) ID_UP_ARROW;
      hAFwd2  = (HMENU) ID_UP_SHIFT;    
      hAFwd3  = (HMENU) ID_RIGHT_ARROW;
      hAFwd4  = (HMENU) ID_RIGHT_SHIFT;
     /*
      strcpy(szAbbr1, "««") ;
      strcpy(szAbbr2, "<<"); // "«") ;
      strcpy(szAbbr3, ">>"); // "»") ;
      strcpy(szAbbr4, "»»") ;
      strcpy(szAbbr5, ">"); 
      strcpy(szAbbr6, "<");
     */
   }
}


//--------------------------------------

void ToolBar_Destroy()
{
  // iViewToolBar = 0;
  DestroyWindow(hVolButton);
  DestroyWindow(hZoomButton);
  DestroyWindow(hLumButton);
  DestroyWindow(hAddButton);
  DestroyWindow(hBmpButton);

  DestroyWindow(hTrack);

  DestroyWindow(hStop);
  DestroyWindow(hPlayP);

  //if (hFastX)  
  //{
    DestroyWindow(hFastX);    
    DestroyWindow(hFaster);   DestroyWindow(hSlower);
    DestroyWindow(hFast1);    DestroyWindow(hFast2);
    DestroyWindow(hSlow1);    DestroyWindow(hSlow2);
    DestroyWindow(hSingle);
  //}

  DestroyWindow(hMarkLeft);
  DestroyWindow(hBack4);
  DestroyWindow(hBack3);
  DestroyWindow(hBack2);
  DestroyWindow(hBack1);
  DestroyWindow(hFwd1);
  DestroyWindow(hFwd2);
  DestroyWindow(hFwd3);
  DestroyWindow(hFwd4);
  DestroyWindow(hMarkRight);

  hFastX = 0;
}




//--------------------------------------

//--------------------------------------

void BmpButton_Create()
{
  //HANDLE hImg;

  hBmpButton = TB_Create(&"B", // WS_DISABLED,
                          0, 0, 
                         (HMENU) IDM_BMP_ASIS);

  /*
#define OIC_BANG            32515
  // iTemp[0] = OIC_BANG;

  hImg     = LoadImage(0,    // NULL,   // hInst,
                      IDI_HAND, // OIC_BANG,// (char*)(&iTemp[0]), // MAKEINTRESOURCE(IDI_SNAP),
                      IMAGE_ICON,
                      0,0,0);

  SendMessage(hBmpButton, BM_SETIMAGE,
              (WPARAM)IMAGE_ICON,
              (LPARAM)hImg);
  */
  
}



//--------------------------------------

void AddButton_Create()
{
  hAddButton = TB_Create(&"+", // WS_DISABLED, 
                                 iTool_Wd, 0, 
                                  (HMENU) IDM_CLIP_ADD);

}



//--------------------------------------

void MarkLeftButton_Create()
{
   hMarkLeft = TB_Create(&"[", // WS_DISABLED,
               iMarkL_PosX, iSkipBar_PosY, 
               (HMENU) ID_FROM_MARK);
}



//--------------------------------------

void MarkRightButton_Create()
{
   hMarkRight = TB_Create(&"]", // WS_DISABLED, 
               iMarkR_PosX, iSkipBar_PosY, (HMENU) ID_TO_MARK);

}



//--------------------------------------

void ToolBar_Create()
{
  HMENU hTemp;

  if (iViewToolBar >= 256)
  {
     BmpButton_Create();
  
     AddButton_Create();

     hLumButton = TB_Create(&"L",  // 0, 
                  iLum_PosX, 0, (HMENU) IDM_LUMINANCE);

     hZoomButton = TB_Create(&"Z", //  /* WS_DISABLED*/ 0,
                   iZoom_PosX, 0, 
                   (HMENU) IDM_ZOOM);

     if (iCtl_KB_V_Popup)
         hTemp = (HMENU) IDM_VOLUME_UP;
     else
         hTemp = (HMENU) IDM_VOL_SLIDERS;

     hVolButton = TB_Create(&"V",  // 0, 
                  iVol_PosX, iSkipBar_PosY, hTemp);
  
     //iTemp[0] = OBM_ZOOM;
     //SendMessage(hBmpButton, BM_SETIMAGE,
     //                       /* (WPARAM)*/ IMAGE_BITMAP,

                         /*
                         (LPARAM)LoadBitmap(hInst, 
                                           MAKEINTRESOURCE(IMG_BITMAP)) 
                         */
                         /*
                         (LPARAM)LoadImage(NULL,  // hInst,
                                           (char*)(&iTemp[0]),
                                           IMAGE_BITMAP ,0,0,0) */
      //            );
  
  

     // More buttons on Right Hand side of trackbar

     hStop = TB_Create(&"º", // WS_DISABLED,  // STOP Lozenge
                  iStop_PosX, iPlayBar_PosY,   
                  (HMENU) IDM_STOP);

     hPlayP = TB_Create(&"P", // WS_DISABLED,
                  iPlayP_PosX, iPlayBar_PosY,  
                  (HMENU) IDM_PLAY_HERE);

  
     //if (iTrackbar_Big)
     {
         PlayTools_Create();
     }
  

     MarkLeftButton_Create();

     // hPrevFile = TB_Create(&"|<",
     //           iToolPosX[??],  iSkipBar_PosY, 
     //           (HMENU) ID_LEFT_CTRL);

     hBack4   = TB_Create("9<", // "««",
                iBack4_PosX, iSkipBar_PosY, hABack4);

     hBack3   = TB_Create("4<", // ² "««",
                iBack3_PosX, iSkipBar_PosY, hABack3);

     hBack2   = TB_Create("<<", // "«",
                iBack2_PosX, iSkipBar_PosY, hABack2);

     hBack1   = TB_Create("<", 
                iBack1_PosX, iSkipBar_PosY,  hABack1);

     hFwd1    = TB_Create(">", 
                iFwd1_PosX, iSkipBar_PosY, hAFwd1);

     hFwd2 = TB_Create(">>", // "»",
                iFwd2_PosX, iSkipBar_PosY, hAFwd2);

     hFwd3  = TB_Create(">4", // ² "»»",
                iFwd3_PosX, iSkipBar_PosY, hAFwd3);

     hFwd4  = TB_Create(">9", // "»»",
                iFwd4_PosX, iSkipBar_PosY, hAFwd4);

     // hNextFile = TB_Create(&">|",
     //           iToolPosX[??],  iSkipBar_PosY, 
     //           (HMENU) ID_RIGHT_CTRL);

     MarkRightButton_Create();

     //if (File_Limit)
     //    Enable_Disable(true, 0, true);

     if (iCtl_ToolTips)
        CreateToolTips_ALL();
  }


  if (iViewToolBar & 1)
  {

     hTrack = CreateWindow(TRACKBAR_CLASS, NULL,
                WS_CHILD | WS_VISIBLE
                              // | WS_DISABLED
                              // | TBS_NOTICKS  | TBS_TOP
                                 | TBS_ENABLESELRANGE,
                     iTrackBar_PosX, iTrackBar_PosY, 
                     iTrack_Wd,    iTool_Wd,
                     hWnd_MAIN, (HMENU) ID_TRACKBAR, hInst, NULL);

     SendMessage(hTrack, TBM_SETRANGE, (WPARAM) true,
                        (LPARAM) MAKELONG(0, TRACK_PITCH));
     if (process.total)
         T100_Upd_Posn_Info(0);
  }

}



