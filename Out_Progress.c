
#include "windows.h"
#include "global.h"
#include <commctrl.h>
#include "out.h"

#define true 1
#define false 0





void Progress_Kill(HWND hProgress_Link)
{
  if (hProgress)
  {
      hProgress = NULL;
      if (Out_PauseFlag)
      {
          Out_PauseFlag = 0;
          ResumeThread(hThread_OUT);
      }
      DestroyWindow(hProgress_Link);
  }

}


void Out_Progress_Title(HWND hProgress_Link)
{
     sprintf(szBuffer, "Saving %d clips - %s", iEDL_OutClips,
                                                    szOutput);
     SetWindowText(hProgress_Link, szBuffer);
     strcpy(szMsgTxt, "Saving...");
     Out_Status_Msg();
}

// ------------------------------------------------------------
//Symbiose
LRESULT CALLBACK OUT_DlgProgress(HWND hProgress_Link, UINT message,
                                 WPARAM wParam, LPARAM lParam)
{
  RECT rect;

  int cnX, cnY, wX, wY, iOver, iSafety;
  unsigned uAction;

  switch (message)
  {
    case WM_INITDIALOG:

          GetWindowRect(hProgress_Link, &rect);

          wX = (rect.right  - rect.left);
          wY = (rect.bottom - rect.top);

          GetWindowRect(hWnd_MAIN, &rect);

          // Don't allow window to fall off bottom of screen
          cnY = rect.top + iTopMargin + 100; // rect.bottom - wY - 30; // (rect.top + rect.bottom) / 2;
          iSafety = iVGA_Avail_Height - wY;
          if (cnY >= iSafety)
              cnY  = iSafety;
          

          cnX = (rect.right + rect.left) / 2;
          cnX -= (wX/2);

          // Don't allow window to fall off sides of screen
          if (cnX < 0)
              cnX = 0;
          else
          {
            iSafety = VGA_Width - wX;
            if (cnX > iSafety)
                cnX = iSafety;
          }

          hBar    = GetDlgItem(hProgress_Link, IDC_PROGRESS_BAR);
          hBar2   = GetDlgItem(hProgress_Link, IDC_PROGRESS_BAR2);
          //hPtxt   = GetDlgItem(hProgress_Link, IDC_PROGRESS_TXT);
          //hETA    = GetDlgItem(hProgress_Link, IDP_PROGRESS_ETA);
          //hPtxt2  = GetDlgItem(hProgress_Link, IDC_PROGRESS_TXT2);
          hPause  = GetDlgItem(hProgress_Link, IDP_PAUSE);
          //hCancel = GetDlgItem(hProgress_Link, IDCANCEL);

          //EnableWindow(hPause, true);
          //EnableWindow(hCancel, true);

          
          if (! iCtl_Readability)
          {
             //SendMessage(hProgress_Link, 
             SendDlgItemMessage(hProgress_Link, IDC_PROGRESS_TXT,
                                WM_SETFONT,
                             (WPARAM)(hDefaultGuiFont),
                                                     false);
          }
          

          // sprintf(szBuffer, "Saving %d clips - %d MB / %d MB - %s",
          //                      iEDL_OutClips,  iEDL_TotMB, iInputTotMB,
          Out_Progress_Title(hProgress_Link);

          Out_Priority_Chg(0);

          ShowWindow(hProgress_Link, SW_SHOW);

          iOver = (cnY + wY + 13 - iVGA_Avail_Height);
          if (iOver > 0)
          {
            cnY -= iOver; 
          }

          MoveWindow(hProgress_Link, cnX, cnY,  wX,  wY, true);

          SendMessage(hBar,  PBM_SETRANGE, 0, 100*65536);
          SendMessage(hBar,  PBM_SETPOS, 0, 0);
          SendMessage(hBar2, PBM_SETRANGE, 0, 100*65536);
          SendMessage(hBar2, PBM_SETPOS, 0, 0);

          return true;


    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
           case IDM_EXIT:
              Progress_Kill(hProgress_Link);
              return true;

           case IDCANCEL:
           case IDP_CANCEL:   // FALSE TRIGGER FOR SOME REASON

              iOut_Error = 8;
              Out_CanFlag = 'C' ;
              Progress_Kill(hProgress_Link);
              return true;


          case IDP_PAUSE:

              if (Out_PauseFlag)
              {
                  Out_PauseFlag = 0;
                  uAction = BST_UNCHECKED; SetWindowText(hPause, "Pause");
                  SetDlgItemText(hProgress, IDC_PROGRESS_TXT, " ");
                  ResumeThread(hThread_OUT);
              }
              else
              {
                  Out_PauseFlag = 1;
                  uAction = BST_CHECKED;  SetWindowText(hPause, "Resume");
                  SuspendThread(hThread_OUT);
                  iOutSuspCtr++;
                  sprintf(szMsgTxt,
                          "Clip #%d \t\t\t    PAUSED", iOut_Clip_ctr);
                  if (DBGflag) DBGout(szMsgTxt);
                  //Out_Status_Msg();
                  SetDlgItemText(hProgress, IDC_PROGRESS_TXT, szMsgTxt);
              }
              CheckDlgButton(hProgress_Link, IDP_PAUSE, uAction);
              SetDlgItemText(hProgress, IDP_PROGRESS_ETA, " ");

              return true;


          case IDP_SLOW_CHK:
               iCtl_Priority[2] = PRIORITY_LOW;
               Out_Priority_Chg(1);
               return true;

          case IDP_NORM_CHK:
               iCtl_Priority[2] = PRIORITY_NORMAL;
               Out_Priority_Chg(1);
               return true;

          case IDP_QUICK_CHK:
               iCtl_Priority[2] = PRIORITY_QUICK;
               Out_Priority_Chg(1);
               return true;

          case IDP_HIGH_CHK:
               iCtl_Priority[2] = PRIORITY_HIGH;
               Out_Priority_Chg(1);
               return true;
        }

        //default:
          //sprintf(szBuffer, "msg=%d", message) ;
          //MessageBox(hWnd, szBuffer, "Hmmmm....", MB_OK);
          //MessageBeep(MB_OK);
        break;

  }

  return false;
}



//----------------------------------

int iPrev_Priority = 424242;


//---------------------------------------
 void Out_Priority_Chg(const int P_Manual)
{
  unsigned uRadio[4];
  int iBreathe;

  uRadio[0] = BST_UNCHECKED;
  uRadio[1] = BST_UNCHECKED;
  uRadio[2] = BST_UNCHECKED;
  uRadio[3] = BST_UNCHECKED;

  switch (iCtl_Priority[2])
  {
     case PRIORITY_HIGH:
           iBreathe = 2;
           uRadio[1] = BST_CHECKED;
           break;

     case PRIORITY_QUICK:
           iBreathe = 2;
           uRadio[0] = BST_CHECKED;
           break;

     case PRIORITY_LOW:
           iBreathe = 0;
           uRadio[3] = BST_CHECKED;
           break;

     default:
           iCtl_Priority[2] = PRIORITY_NORMAL;
           iBreathe = 1;
           uRadio[2] = BST_CHECKED;
           break;

  }

  Set_Priority(hThread_OUT, iCtl_Priority[2], 2, 1);

  // Repeated hitting of SLOW or FAST gives extra effect
  if (P_Manual 
  && iCtl_Priority[2] == iPrev_Priority 
  && iBreathe != 1)
  {
    if (iBreathe == 0)
    {
      iOut_Breathe_PktLim    /= 2;
      iOut_Breathe_PerBigBlk *= 2; 
    }
    else
    {
      iOut_Breathe_PktLim    *= 2;
      iOut_Breathe_PerBigBlk /= 2; 
    }

  }
  else
  {
    iOut_Breathe_PerBigBlk = iCtl_Out_Breathe_PerBigBlk[iBreathe];
    iOut_Breathe_PktLim    = iCtl_Out_Breathe_PktLim   [iBreathe];
  }

  //if (iOut_Parse_Deep && iOut_ParseAllPkts && iBreathe < 2)
  //    iOut_Breathe_PerBigBlk +=32;

  SendDlgItemMessage(hProgress, IDP_QUICK_CHK, BM_SETCHECK, uRadio[0], 0);
  SendDlgItemMessage(hProgress, IDP_HIGH_CHK,  BM_SETCHECK, uRadio[1], 0);
  SendDlgItemMessage(hProgress, IDP_NORM_CHK,  BM_SETCHECK, uRadio[2], 0);
  SendDlgItemMessage(hProgress, IDP_SLOW_CHK,  BM_SETCHECK, uRadio[3], 0);

  iPrev_Priority = iCtl_Priority[2];

}


