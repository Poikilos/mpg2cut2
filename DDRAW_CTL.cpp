
//
//
// When I try to compile this separately, 
// I get unresolved external references,
// so it is still an "include" of GUI.CPP
//

//#include "global.h"
//#include "DDRAW_CTL.h"


// rj NOTE - Should encapsulate all the Direct Draw routines into here
//-----------------------------------------------------------------------

#include <ddraw.h>

void  Flag2YUV()
{
     MParse.iColorMode = STORE_YUY2;
     iColorSpaceTab    = 0;
     CheckMenuItem(hMenu, IDM_STORE_RGB24, MF_UNCHECKED);
     CheckMenuItem(hMenu, IDM_STORE_YUY2, MF_CHECKED);
     // EnableMenuItem(hMenu, IDM_BMP, MF_GRAYED);
     CheckMenuItem(hMenu, IDM_DIRECTDRAW, MF_CHECKED);
}


void  Flag2RGB()
{
     MParse.iColorMode = STORE_RGB24;
     iColorSpaceTab    = 1;
     CheckMenuItem(hMenu,   IDM_STORE_RGB24, MF_CHECKED);
     CheckMenuItem(hMenu,   IDM_STORE_YUY2, MF_UNCHECKED);
     // EnableMenuItem(hMenu, IDM_BMP, MF_ENABLED);

     if (DBGflag)
     {
         DBGout("OVL *OFF*\n");
     } 
}




void D100_CHECK_Overlay()
{
HRESULT hRC;
int iRC, iTmp1, iTmp2;
char *lpErrTxt, cZero[4];

  cZero[0] = 0;
  lpErrTxt = &cZero[0];

  if (DBGflag)
  {
      DBGout("D100_OVL");
			fflush(DBGfile);
  }



  // 6414 - Make sure OVL attempt is starting cleanly
 if (lpOverlay || lpPrimary || lpDD2 || lpDD)
 {
     D300_FREE_Overlay();
 }

 DDOverlay_Flag = false;

 szMsgTxt[0] = 0;

 iATI_BugMe = iCtl_OVL_ATI_Bug;


 if (DirectDrawCreate(NULL, &lpDD, NULL)==DD_OK)
 {
  if (lpDD->QueryInterface(IID_IDirectDraw2, (LPVOID*)&lpDD2)==DD_OK)
  {
    if (lpDD2->SetCooperativeLevel(hWnd_MAIN, DDSCL_NORMAL)==DD_OK)
    {
      ZeroMemory(&ddsd, sizeof(DDSURFACEDESC));
      ddsd.dwSize    =  sizeof(DDSURFACEDESC);
      ddsd.dwFlags   =  DDSD_CAPS;
      ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE
                          | DDSCAPS_VIDEOMEMORY;

      if (lpDD2->CreateSurface(&ddsd, &lpPrimary, NULL)==DD_OK)
      {
        ZeroMemory(&halcaps, sizeof(DDCAPS));
        halcaps.dwSize = sizeof(DDCAPS);

        if (lpDD2->GetCaps(&halcaps, NULL)==DD_OK)
        {
          if (halcaps.dwCaps & DDCAPS_OVERLAY)
          {
            ZeroMemory(&ddsd, sizeof(DDSURFACEDESC));
            ddsd.dwSize   = sizeof(DDSURFACEDESC);
            ddsd.dwFlags  = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
            ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY;

            // Allow for interface limitations

            if (VGA_Width  < 640)
                VGA_Width  = GetSystemMetrics(SM_CXSCREEN);
            if (VGA_Height < 480)
                VGA_Height = GetSystemMetrics(SM_CYSCREEN);

            if (iAspect_Width_Max >= VGA_Width)
                ddsd.dwWidth       = VGA_Width - 1;
            else
                ddsd.dwWidth       = iAspect_Width_Max;

            if (Coded_Pic_Height  >= VGA_Height)
                ddsd.dwHeight      = VGA_Height - 1;
            else
                ddsd.dwHeight     = Coded_Pic_Height;

            if (ddsd.dwWidth  < 640)
                ddsd.dwWidth  = 640;
            if (ddsd.dwHeight < 480)
                ddsd.dwHeight = 480; 

            ddPixelFormat.dwFlags = DDPF_FOURCC;
            ddPixelFormat.dwYUVBitCount = 16;

            if (iCtl_YV12)
                ddPixelFormat.dwFourCC = mmioFOURCC('Y','V','1','2');
            else
                ddPixelFormat.dwFourCC = mmioFOURCC('Y','U','Y','2');

FourCC_Retry:
            memcpy(&(ddsd.ddpfPixelFormat), &ddPixelFormat, sizeof(DDPIXELFORMAT));

            hRC = lpDD2->CreateSurface(&ddsd, &lpOverlay, NULL);

            if (hRC == DDERR_INVALIDPIXELFORMAT)
            {
                  strcpy(szMsgTxt, "OVL INV 4CC");

                  if (ddPixelFormat.dwFourCC == mmioFOURCC('Y','U','Y','2')
                  && iView_Fast_YUV)
                  {
                      ddPixelFormat.dwFourCC =  mmioFOURCC('U','Y','V','Y');
                      goto FourCC_Retry;
                  }
                        // '21VY';  'YVYU'; //
            }
            else
            if (hRC != DD_OK)
            { // For a list of error codes see compiler include "DDRAW.H"
              if (hRC == DDERR_LOCKEDSURFACES)
                lpErrTxt = &"OVL Surface LockOut";
              else
                lpErrTxt = &"No YUY2 Surface";

              strcpy(szMsgTxt, lpErrTxt);
            }
            else
            {
              DDOverlay_Flag = true;

              if (DBGflag)
              {
                  DBGout("  OVL OK");
              }

              DDraw_lPitch = ddsd.lPitch;
              if (DDraw_lPitch  < 0)
                  DDraw_lPitch  = -DDraw_lPitch;
              else
              if (DDraw_lPitch == 0)
                  DDraw_lPitch  = (int)((ddsd.dwWidth + 3) / 4 * 4);

              DDraw_dwWidth = (int)(ddsd.dwWidth);
              if (DDraw_dwWidth  < 0)
                  DDraw_dwWidth  = -DDraw_dwWidth;
               else
              if (DDraw_dwWidth == 0)
                  DDraw_dwWidth  = DDraw_lPitch;


              DDraw_Canvas_Size  = DDraw_dwWidth * ddsd.dwHeight; 

              if (ddPixelFormat.dwFourCC != '21VY') // YV12
                  DDraw_Surface_Size = DDraw_lPitch  // DDraw_dwWidth // 
                                     * ddsd.dwHeight * 2;
              else
                  DDraw_Surface_Size = DDraw_Canvas_Size * 3 / 2;


              ZeroMemory(&ddofx, sizeof(DDOVERLAYFX));
              ddofx.dwSize   =   sizeof(DDOVERLAYFX);

              if (iCtl_OVL_FullKey)
              {
                iTmp1 = 0;
                iTmp2 = 0xFFFFFF;
              }
              else
              {
                iTmp1 = iCtl_Mask_Colour;
                iTmp2 = iCtl_Mask_Colour;
              }
              ddofx.dckDestColorkey.dwColorSpaceLowValue  = DDColorMatch(lpPrimary, iTmp1);
              ddofx.dckDestColorkey.dwColorSpaceHighValue = DDColorMatch(lpPrimary, iTmp2); //ddofx.dckDestColorkey.dwColorSpaceLowValue;
            }
          }
          else
            strcpy(szMsgTxt, "No Overlay Capability");

        }
        else
            strcpy(szMsgTxt, "No DDraw Capability");
      }
      else
            strcpy(szMsgTxt, "No DDraw Surface");
    }
    else
            strcpy(szMsgTxt, "No CoopLevel SCL");
  }
  else
            strcpy(szMsgTxt, "No QryIface");
 }
 else
            strcpy(szMsgTxt, "DDCreate Failed");

 if (DBGflag)
 {
    if (szMsgTxt[0])
        DBGout(szMsgTxt);

    iRC = hRC & 0xFFFF;

    // For a list of error codes see compiler include "DDRAW.H"
    sprintf(szBuffer, "     Ovly w=%03d /%03d h=%03d DDERR#%04d x%08X  AspMax=%d\n",
                            ddsd.dwWidth, ddsd.lPitch, ddsd.dwHeight, 
                            iRC, hRC,
                            iAspect_Width_Max);
    DBGout(szBuffer);
  	fflush(DBGfile);
 }


 if (DDOverlay_Flag)
 {
     Flag2YUV();
 }
 else
 {
     Flag2RGB();
     D300_FREE_Overlay();  // Make sure ALL DD resources released
     DSP1_Main_MSG(0,0); 
 }

 //iZoom_OLD = iCtl_Zoom_Wanted;
 //EnableMenuItem(hMenu, IDM_BMP_ASIS, MF_ENABLED);

}

int iWarnRect=0;

//-------------------------
void D200_UPD_Overlay()
{

 HRESULT hRC;
 int iRC, iWidth, iGap, iRetry;
 RECT rOut;
 char cTmp1;

 iRetry = 0;

 if (orect.right  >= VGA_Width)
     orect.right   = VGA_Width  - 1;
 if (orect.bottom >= VGA_Height)
     orect.bottom  = VGA_Height - 1;

 // Allow for Magnify of small canvas

 memcpy(&rOut, &prect, sizeof(rOut));

 if (DBGflag && iMainWin_State)
 {
    DBGout("MAX");
 }


 if (iMainWin_State == 1)
 {
   //Calc_PhysView_Size();
   if (iView_Centre_Crop || iAspectOut < 2700)
   {
      rOut.bottom =  //  iPhysView_Height;  // This is not reliable for some reason
                     VGA_Height;

      iWidth = prect.right  - prect.left;
      rOut.right  =  (rOut.bottom - rOut.top) 
                                      * (iWidth)
                                      / (prect.bottom - prect.top) 
                                      + rOut.left;

      // Center if less wide than screen
      iGap = VGA_Width - rOut.right - rOut.left;
      if (iGap > 0)
      {
        iGap = iGap / 2;
        rOut.left  += iGap;
        rOut.right += iGap;
      }

      if (rOut.right >= VGA_Width)
          rOut.right  = VGA_Width - 1;
   }
   else
   {
      rOut.left   =  0;
      rOut.right  =  VGA_Width; // iPhysView_Width;
      rOut.bottom =  rOut.right * (prect.bottom - prect.top) 
                                / (prect.right  - prect.left)
                                + rOut.top;
      if (rOut.bottom >= VGA_Height)
          rOut.bottom  = VGA_Height - 1;
   }
 }
 else
 {
    // Try to avoid DD InvRect (DirectDraw Invalid Rectangle)

    //if (orect.left   < crect.left)   
    //    orect.left   = crect.left;
    //if (orect.top    < crect.top)    
    //    orect.top    = crect.top;   
    if (orect.right  > crect.right)    // iPhysView_Width)
        orect.right  = crect.right;    // iPhysView_Width;
    if (orect.bottom > crect.bottom)   // iPhysView_Height)
        orect.bottom = crect.bottom;   // iPhysView_Height;


    if (orect.right  > (signed)ddsd.dwWidth)   
        orect.right  = ddsd.dwWidth;   
    if (orect.bottom > (signed)ddsd.dwHeight)  
        orect.bottom = ddsd.dwHeight;   
     

    //if (rOut.left   < prect.left)     
    //    rOut.left   = prect.left;  
    //if (rOut.top    < prect.top)     
    //    rOut.top    = prect.top;    
    //if (rOut.right  > prect.right)    // iPhysView_Width)
    //    rOut.right  = prect.right;    // iPhysView_Width;
    //if (rOut.bottom > prect.bottom)   // iPhysView_Height)
    //    rOut.bottom = prect.bottom;   // iPhysView_Height;
 }



 /*
 if (DBGflag)
 {
      sprintf(szBuffer, "B4     o=%03d..%03d, %03d..%03d\n      dd=   ..%03d,    ..%03d\n       r=%03d..%03d, %03d..%03d",
                         orect.left, orect.right, orect.top, orect.bottom,
                         ddsd.dwWidth, ddsd.dwHeight,
                         rOut.left,  rOut.right,  rOut.top,  rOut.bottom);
      DBGout(szBuffer);
			fflush(DBGfile);
 }
 */
 


UpdOvl_Retry:

 if (lpOverlay && lpPrimary
 && rOut.right  > 0
 && rOut.bottom > 0)
 {
      hRC = IDirectDrawSurface_UpdateOverlay(lpOverlay, &orect,
                                             lpPrimary, &rOut,
           DDOVER_SHOW | DDOVER_DDFX | DDOVER_KEYDESTOVERRIDE, &ddofx);

      if( winVer.dwMajorVersion >= 6 || iCtl_VistaOVL_mod) // Windows Vista is crap
      {
         if (iCtl_VistaOVL_mod == 2)
            UpdateWindow(hWnd_MAIN);
         else
         if (iCtl_VistaOVL_mod < 2 || PlayCtl.iPlayed_Frames < 6)
            SetWindowPos(hWnd_MAIN, 0, 0, 0, 0, 0, // Avery Lee - VirtualDub solution to Vista stupidity
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED); 
      }
  }
  else
     hRC = 0x6901;

  iRC = hRC & 0xFFFF;

  
  if (DBGflag)
  {

      if (hRC == DD_OK)
        cTmp1 = ' ';
      else
        cTmp1 = '*';

      /*
      sprintf(szBuffer, "UpdOvly o=%03d..%03d, %03d..%03d  Zoom=%d  %cDDERR#%03d x%08X",
                         orect.left, orect.right, orect.top, orect.bottom, 
                         iCtl_Zoom_Wanted, cTmp1, iRC, hRC);
      DBGout(szBuffer);
      sprintf(szBuffer, "       dd=   ..%03d,    ..%03d\n        r=%03d..%03d, %03d..%03d",
                         ddsd.dwWidth, ddsd.dwHeight,
                         rOut.left,  rOut.right,  rOut.top,  rOut.bottom);
      DBGout(szBuffer);
      sprintf(szBuffer, "        p=%03d..%03d, %03d..%03d    Asp=%d",
                         prect.left, prect.right, prect.top, prect.bottom,
                         //iPhysView_Width, iPhysView_Height, 
                         iAspectOut);
      DBGout(szBuffer);
      sprintf(szBuffer, "        c=%03d..%03d, %03d..%03d    Max=%d\n",
                         crect.left, crect.right, crect.top, crect.bottom,
                         iMainWin_State);
      DBGout(szBuffer);
      */

			fflush(DBGfile);
  }
  

  iDDO_Frame_Ready = 0;

  if (hRC == DD_OK)
  {
    if (iATI_BugMe  && PlayCtl.iPlayed_Frames < 2)
    {
        // There is a bug in zome ATI video card drivers 
        // where first mapping does not work
        // causing sloping image
        // overcome this by doing first mapping twice.
        iATI_BugMe = 0;
        goto UpdOvl_Retry;
    }
  }
  else
  {
     if (hRC == DDERR_INVALIDRECT)
         sprintf(szMsgTxt, "DD InvRect %d..%d %d..%d   %d..%d %d..%d",
                   orect.left, orect.right,
                   orect.top,  orect.bottom,
                   rOut.left,  rOut.right,
                   rOut.top,   rOut.bottom);
     else
     if (hRC == DDERR_OUTOFCAPS)
     {
        // Win2k - late admission of no overlay
        Chg2RGB24(1, 0);
        strcpy(szMsgTxt, "OVL UNAVAILABLE");
        Sleep(2000);
     }
     else
     if (hRC == DDERR_SURFACELOST)
     {
       if (!iRetry)
       {
         iRetry = 1;
         D300_FREE_Overlay();
         D100_CHECK_Overlay();
         if (DDOverlay_Flag)
           goto UpdOvl_Retry;
       }

       
       // Win2k - late admission of no overlay
       Chg2RGB24(1, 0);
       strcpy(szMsgTxt, "OVL UNAVAILABLE");
       Sleep(2000);
       
     }
     else
         sprintf(szMsgTxt, "DDraw ERR#%d", iRC);

     DSP1_Main_MSG(1,0);
     if (!iWarnRect)
     {
         iWarnRect = 1;
         Sleep(2000);
     }
  }

}





//--------------------------
void D300_FREE_Overlay()
{
  int iRC[5] = {42,42,42,42,42};

  //if (DDOverlay_Flag)
  {
      DDOverlay_Flag = false;
      CheckMenuItem(hMenu, IDM_DIRECTDRAW, MF_UNCHECKED);

      if (lpOverlay)
         iRC[0] = IDirectDrawSurface_UpdateOverlay(lpOverlay, NULL, 
                                                   lpPrimary,
                                                  NULL, DDOVER_HIDE, NULL);
      if (iCtl_Ovl_Release)
      {
        if (lpOverlay)
        {
            iRC[1] = IDirectDrawSurface_Release(lpOverlay);
            lpOverlay = 0;
        }

        if (lpPrimary)
        {
            iRC[2] = IDirectDrawSurface_Release(lpPrimary);
            lpPrimary = 0;
        }
      }

      if (lpDD2)
      {
         iRC[3] = lpDD2->Release();
         lpDD2 = 0;
      }
      if (lpDD)
      {
         iRC[4] = lpDD->Release();
         lpDD = 0;
      }

  }

  if (DBGflag)
  {
      sprintf(szDBGln, "D300_FREE_OVL RC=%d,%d,%d,%d,%d",
                           iRC[0], iRC[1], iRC[2], iRC[3], iRC[4]);
      DBGout(szDBGln);
  }
  // PicBuffer_Canvas_Size = 0;
}




//---------------------------------------------------------------
DWORD DDColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb)
{
  COLORREF  rgbT;
  DWORD   dw = CLR_INVALID;
  HRESULT  hRC;
  HDC     hdc_DDO;
  int iSafety;
  char szAction[16];

  rgbT = 0 ;   //RJ ALLOW FOR PROBLEMS

  hRC = IDirectDrawSurface_GetDC(pdds, &hdc_DDO);

  if (hRC != DD_OK)
    strcpy(szAction,"GetDC");
  else
  {
    rgbT = GetPixel(hdc_DDO, 0, 0);
    SetPixel(hdc_DDO, 0, 0, rgb);

    IDirectDrawSurface_ReleaseDC(pdds, hdc_DDO);
  

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC));
    ddsd.dwSize   =   sizeof(DDSURFACEDESC);

    iSafety = 0;
    while ((hRC = IDirectDrawSurface_Lock(pdds, NULL, &ddsd, 0, NULL))
                                          == DDERR_WASSTILLDRAWING
         && iSafety < 100)
    {
      iSafety++;
      Sleep(25); // Allow other tasks to breathe
    };

    if (DBGflag && iSafety)
    {
       sprintf(szBuffer, "DD Sleep %d*25 Aud=%d", iSafety, iWAVEOUT_Scheduled_Blocks);
       DBGout(szBuffer);
    }


    if (hRC != DD_OK)
       strcpy(szAction,"SurfLock");
    else
    {
       dw = *(DWORD *) ddsd.lpSurface;
       if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
           dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;

       IDirectDrawSurface_Unlock(pdds, NULL);
 
       hRC = IDirectDrawSurface_GetDC(pdds, &hdc_DDO);
       if (hRC != DD_OK)
           strcpy(szAction,"GetDC2");
       else
       {
           SetPixel(hdc_DDO, 0, 0, rgbT);
           IDirectDrawSurface_ReleaseDC(pdds, hdc_DDO);
       }
    }

  }
    
  if (hRC != DD_OK)
  {
      sprintf(szMsgTxt, "DD ERR#%d Mask &s", hRC, szAction);
      DSP1_Main_MSG(1,1);
      // dw = rgb;
  }

  return dw;
}


// Activate = 0 - removes overlay mask
//          = 1 - overlay mask covering entire client area of window
//          = 2 - overlay mask covering area below toolbar and msg area
void DD_OverlayMask(int P_activate)
{
  HBRUSH hBrush_ToUse;
  int    iColorToUse;

  Calc_PhysView_Size(); //GetClientRect(hWnd, &crect);
  if (P_activate != 1)
      crect.top += iTopMargin; // TEMP adjust to skip toolbar area

  if (P_activate > 0)
  {
      hBrush_ToUse = hBrush_MASK;
      iColorToUse  = iCtl_Mask_Colour;
  }
  else
  {
      hBrush_ToUse = CreateSolidBrush(0x050403);
      iColorToUse  = 0x050403;
  }

  Sleep(2);
  FillRect(hDC, &crect, hBrush_ToUse);// Paint video area with Overlay key MASk or not.
  Sleep(2);
  SetBkColor(hDC, iColorToUse); // Background = Overlay Mask key or not;

  if (P_activate <= 0)
      DeleteObject(hBrush_ToUse);

  if (P_activate != 1)
  {
     crect.top -= iTopMargin; // UNDO Temp Adjust
  }

  if (iMsgLife > 1)
      DSP1_Main_MSG(1,0);

  UpdateWindow(hWnd_MAIN);
}


//-------------------------------------------------------

void Chg2RGB24(int P_Redraw, HWND P_Caller)
{
   iDDO_Frame_Ready = 0; 

   if (MParse.iColorMode != STORE_RGB24)
   {
      if (DDOverlay_Flag)
      {
         if (iCtl_Ovl_Release)
            D300_FREE_Overlay();
         else
            IDirectDrawSurface_UpdateOverlay(lpOverlay, NULL, lpPrimary,
                                                  NULL, DDOVER_HIDE, NULL);
      }

      Flag2RGB();

      if (P_Redraw) 
          RefreshVideoFrame();

      if (hLumDlg && P_Caller != hLumDlg)
          PostMessage(hLumDlg, WM_COMMAND, IDL_MODE_RGB, 0);

   }
}



//-----------------------------------------------------

void Chg2YUV2(int P_Redraw, HWND P_Caller)
{
   if (MParse.iColorMode != STORE_YUY2  || !DDOverlay_Flag)
   {
       if (iCtl_Ovl_Release || !DDOverlay_Flag)                 
           D100_CHECK_Overlay();
       else
           Flag2YUV();

       if (DDOverlay_Flag)
       {
           DD_OverlayMask(2);  // FillRect(hDC, &crect, hBrush_MASK);
       }

       if (P_Redraw) 
           RefreshVideoFrame();

      if (hLumDlg && P_Caller != hLumDlg)
          PostMessage(hLumDlg, WM_COMMAND, IDL_MODE_OVL, 0);

   }
}


void DD_PtrLost_Box(const char *P_PtrDesc)
{
  int iRC;

  sprintf(szBuffer,
               "DDraw %s Pointer LOST.\n\n Flag=%d, Recovery=%s\n\nCONTINUE ?",
                      P_PtrDesc, DDOverlay_Flag, RecoveryReason);
  iRC = MessageBox ( NULL, szBuffer, "Mpg2Cut2 - BUG !",
                     MB_OKCANCEL | MB_ICONEXCLAMATION
                           | MB_SETFOREGROUND | MB_TOPMOST);
  if (iRC != IDOK)
     MParse.Fault_Flag = CRITICAL_ERROR_LEVEL;
}

