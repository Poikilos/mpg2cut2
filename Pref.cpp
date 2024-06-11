
// Preferences Dialog - WeWantWideScreen 


#include "windows.h"
#include "COMMCTRL.H"

#include "global.h"


const int cMAX_PANE=4;

HWND hPane[cMAX_PANE];

int  dwPanePos[cMAX_PANE];
char* OLDCtl_OutFolder[MAX_PATH];
int OLDiCtl_Out_PTS_Match;
int OLDiCtl_SetBrokenGop;
int OLDiCtl_Out_Parse_AllPkts;
int OLDiCtl_Out_Parse_Deep;
int OLDiCtl_Out_Align_Video;
int OLDiCtl_Out_Align_Audio;
int OLDiCtl_Out_TC_Adjust;
int OLDiCtl_Out_PTS_Invent;
int OLDiCtl_Out_Seq_End;
int OLDiCtl_Out_TC_Force;
HWND hPrefDlgG, hChild;
DWORD dwLast;
const int ALLBASE = 6000;
int nCurPage;
DWORD dwLastY;
DWORD dwPaneTop[5];
DWORD dwItemLeft[5];
DWORD dwItemTop[5];
int nItemCount;
int dwTotalHeight;

void DoChecks(HWND hPanel, int iChecked, int nBase)
{
  int i;

  for (i=IDC_CHK_AUDIOMATCHING;i<(IDC_CHK_AUDIOMATCHING+6);i++)
    SendDlgItemMessage(hPanel, nBase+i, BM_SETCHECK, iChecked ? BST_CHECKED : BST_UNCHECKED, 0);
}


LRESULT CALLBACK Pane_Dialog(HWND hPrefDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

  switch (message)
  {
    case WM_INITDIALOG:
      break;
    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case ALLBASE+IDC_BTN_ALL:
          DoChecks(hPane[0],true,ALLBASE);
        DoChecks(hPane[2],true,0);
        break;

      case IDC_BTN_ALL:
        DoChecks(hPane[0],true,ALLBASE);
        DoChecks(hPane[2],true,0);
        break;

      case ALLBASE+IDC_BTN_NONE:
        DoChecks(hPane[0],false,ALLBASE);
        DoChecks(hPane[2],false,0);
        break;

      case IDC_BTN_NONE:
        DoChecks(hPane[0],false,ALLBASE);
        DoChecks(hPane[2],false,0);
        break;
      }
      break;
  }

  // ???
    //Set_Bmp_Fmt(2);
    //Set_Toggle_Menu('T', &iCtl_SetBrokenGop, IDM_OUT_BROKEN_FLAG);
    //Set_Toggle_Menu('T', &iCtl_Out_Align_Video, wmId);
  //Set_Toggle_Menu('T', &iCtl_Out_Align_Audio, wmId);

   return false;
}


int GetRadio(HWND hwin, int nStart, int nEnd)
{
  int i;

  for (i=nStart;i<=nEnd;i++)
  {
    if (IsDlgButtonChecked(hwin,i))
      return i;

  }

  return -1;
}

// Save variables.
void SaveStates()
{
  // Output.
  lstrcpy(OLDCtl_OutFolder[0],szCtl_OutFolder);

  OLDiCtl_Out_PTS_Match=iCtl_Out_PTS_Match;
  OLDiCtl_SetBrokenGop=iCtl_SetBrokenGop;
  OLDiCtl_Out_Parse_AllPkts = iCtl_Out_Parse_AllPkts;
  OLDiCtl_Out_Parse_Deep    = iCtl_Out_Parse_Deep;
  OLDiCtl_Out_Align_Video=iCtl_Out_Align_Video;
  OLDiCtl_Out_Align_Audio=iCtl_Out_Align_Audio;
  OLDiCtl_Out_TC_Adjust=iCtl_Out_TC_Adjust;

  // Experimental.
  OLDiCtl_Out_PTS_Invent=iOut_PTS_Invent;
  OLDiCtl_Out_Seq_End=iCtl_Out_Seq_End;
  // ???
  OLDiCtl_Out_TC_Force=iCtl_Out_TC_Force;
}

void RestoreStates()
{
  // Output.

  lstrcpy(szCtl_OutFolder,OLDCtl_OutFolder[0]);

  iCtl_Out_PTS_Match=OLDiCtl_Out_PTS_Match;
  iCtl_SetBrokenGop=OLDiCtl_SetBrokenGop;
  iCtl_Out_Parse_AllPkts = OLDiCtl_Out_Parse_AllPkts;
  iCtl_Out_Parse_Deep    = OLDiCtl_Out_Parse_Deep;
  iCtl_Out_Align_Video=OLDiCtl_Out_Align_Video;
  iCtl_Out_Align_Audio=OLDiCtl_Out_Align_Audio;
  iCtl_Out_TC_Adjust=OLDiCtl_Out_TC_Adjust;

  // Experimental.
  iCtl_Out_PTS_Invent=OLDiCtl_Out_PTS_Invent;
  iCtl_Out_Seq_End=OLDiCtl_Out_Seq_End;
  // ???
  iCtl_Out_TC_Force=OLDiCtl_Out_TC_Force;

}




void ShowPane(int nItem)
{
  int i;

  if (!nItem)
    ShowWindow(hChild,SW_SHOW); // Show scrollbar for all.
  else
    ShowWindow(hChild,SW_HIDE);

  for (i=0;i<cMAX_PANE;i++)
    ShowWindow(hPane[i],SW_HIDE);

  if (((nItem>=0) && (nItem<cMAX_PANE)) && (nItem!=-1))
    ShowWindow(hPane[nItem],SW_SHOWDEFAULT);
}




bool CALLBACK EnumWindowsProc(HWND hwin, LPARAM lparam)
{
    char title[256], class_name[256];
  char szBuf[256];
  DWORD dwStyle, dwExStyle, dwID;
  RECT rect, rect2;
  int dwX, dwY, dwWidth, dwHeight;
  HFONT hFont;
  HWND hNewWin;
  //DWORD nPage;
  bool bSearch;
  DWORD dwYY;

  hFont=(HFONT)GetStockObject(DEFAULT_GUI_FONT);

  bSearch=true;

  if (hwin==hPane[0])
  {
     nCurPage=0;
     bSearch=false;
     nItemCount=0;
  }
  else if (hwin==hPane[1])
  {
     nCurPage=1;
     bSearch=false;
     nItemCount=0;
  }
  else if (hwin==hPane[2])
  {
     nCurPage=2;
     bSearch=false;
     nItemCount=0;
  }
  else if (hwin==hPane[3])
  {
     nCurPage=3;
     bSearch=false;
     nItemCount=0;
  }

  if (!bSearch)
  {
    GetWindowRect(hwin,&rect2);
    dwHeight=rect2.bottom-rect2.top;
    if ((!nCurPage) || (nCurPage==1))
      dwPaneTop[nCurPage]=0;
    else
      dwPaneTop[nCurPage]=dwTotalHeight+dwHeight;
  }

     if (bSearch)
     {

       if ((nCurPage>=1) && (nCurPage<=3)) {
      GetWindowText(hwin, title, 256);
      GetClassName(hwin, class_name, 256);
      wsprintf(szBuf, "%s has class %s", title, class_name);

      GetClientRect(hwin,&rect);
      GetWindowRect(hwin,&rect2);

       if (nItemCount==0)
       {
         dwItemLeft[nCurPage]=rect2.left;
         dwItemTop[nCurPage]=rect2.top;;
         nItemCount=1;
       }



      dwStyle=GetWindowLong(hwin,GWL_STYLE);
      dwExStyle=GetWindowLong(hwin,GWL_EXSTYLE);
      dwID=GetWindowLong(hwin,GWL_ID);

      dwWidth=rect.right-rect.left;
      dwHeight=rect.bottom-rect.top;

      dwX=rect2.left - dwItemLeft[nCurPage];
      dwYY= (rect2.top - dwItemTop[nCurPage]);
      dwY=dwPaneTop[nCurPage] + dwYY;
      dwID=ALLBASE+dwID;

      wsprintf(szBuf,"cls: %s title: %s width: %d height: %d x: %d y: %d",class_name,title,dwWidth,dwHeight,dwX,dwY);
      //MessageBox(0,szBuf,"",0);

      hNewWin=CreateWindowEx(dwExStyle,class_name,title,dwStyle,dwX,dwY,dwWidth,dwHeight,hPane[0],(HMENU)dwID,hInst,0);

      SendMessage(hNewWin,WM_SETFONT,(WPARAM)hFont,TRUE);

       }

     }


  return true;
}

// Enumerate all windows to create All pane & tab.
void MakePaneAll(HWND hwin)
{
  //RECT rect;

  nCurPage=-1;
  nItemCount=0;
  dwTotalHeight=0;

  EnumChildWindows(hwin,(WNDENUMPROC)EnumWindowsProc,0);
}

// Show variables onto GUI.
void ShowSettings()
{
  int i;
  int iChecked;

  // All.
  if ((iCtl_BMP_Aspect==0) || (iCtl_BMP_Aspect==2))
    CheckRadioButton(hPane[1],IDC_RDO_SAME,IDC_RDO_BICUBIC, !iCtl_BMP_Aspect ? IDC_RDO_SAME+2:IDC_RDO_SAME+0);
  else
    CheckRadioButton(hPane[1],IDC_RDO_SAME,IDC_RDO_BICUBIC,IDC_RDO_SAME+iCtl_BMP_Aspect);

  SetDlgItemText(hPane[1],IDC_EDT_FOLDER,szCtl_OutFolder);

  if ((iCtl_BMP_Aspect==0) || (iCtl_BMP_Aspect==2))
    CheckRadioButton(hPane[0],ALLBASE+IDC_RDO_SAME,ALLBASE+IDC_RDO_BICUBIC, !iCtl_BMP_Aspect ? ALLBASE+IDC_RDO_SAME+2:ALLBASE+IDC_RDO_SAME+0);
  else
    CheckRadioButton(hPane[0],ALLBASE+IDC_RDO_SAME,ALLBASE+IDC_RDO_BICUBIC,ALLBASE+IDC_RDO_SAME+iCtl_BMP_Aspect);

  // Folder.
  SetDlgItemText(hPane[0],ALLBASE+IDC_EDT_FOLDER,szCtl_OutFolder);
  SetDlgItemText(hPane[2],IDC_EDT_FOLDER,szCtl_OutFolder);

  for (i=IDC_CHK_AUDIOMATCHING;i<=(IDC_CHK_AUDIOMATCHING+6);i++)
  {

    switch(i)
    {
    case IDC_CHK_AUDIOMATCHING+0:
      iChecked=iCtl_Out_PTS_Match;
      break;
    case IDC_CHK_AUDIOMATCHING+1:
      iChecked=iCtl_SetBrokenGop;
      break;
    case IDC_CHK_AUDIOMATCHING+2:
      iChecked=iCtl_Out_Parse_AllPkts;
      break;
    case IDC_CHK_AUDIOMATCHING+3:
      iChecked=iCtl_Out_Parse_Deep;
      break;
    case IDC_CHK_AUDIOMATCHING+4:
      iChecked=iCtl_Out_Align_Video;
      break;
    case IDC_CHK_AUDIOMATCHING+5:
      iChecked=iCtl_Out_Align_Audio;
      break;
    case IDC_CHK_AUDIOMATCHING+6:
      iChecked=OLDiCtl_Out_TC_Adjust;
      break;
    }

    SendDlgItemMessage(hPane[0], ALLBASE+i, BM_SETCHECK, iChecked ? BST_CHECKED : BST_UNCHECKED, 0);
    SendDlgItemMessage(hPane[2], i,         BM_SETCHECK, iChecked ? BST_CHECKED : BST_UNCHECKED, 0);

  }

  // IDC_CHK_BYPASSCACHE ???

  // All
  SendDlgItemMessage(hPane[0], ALLBASE+IDC_CHK_SEQEND,       BM_SETCHECK, iCtl_Out_Seq_End  ? BST_CHECKED : BST_UNCHECKED, 0);
  SendDlgItemMessage(hPane[0], ALLBASE+IDC_CHK_AUSCH7FIX,    BM_SETCHECK, iCtl_Out_TC_Force ? BST_CHECKED : BST_UNCHECKED, 0);
  SendDlgItemMessage(hPane[0], ALLBASE+IDC_CHK_CREATEGOPPTS, BM_SETCHECK, iCtl_Out_PTS_Invent   ? BST_CHECKED : BST_UNCHECKED, 0);

  // Experimental.
  SendDlgItemMessage(hPane[2], IDC_CHK_SEQEND,       BM_SETCHECK, iCtl_Out_Seq_End  ? BST_CHECKED : BST_UNCHECKED, 0);
  SendDlgItemMessage(hPane[2], IDC_CHK_AUSCH7FIX,    BM_SETCHECK, iCtl_Out_TC_Force ? BST_CHECKED : BST_UNCHECKED, 0);
  SendDlgItemMessage(hPane[2], IDC_CHK_CREATEGOPPTS, BM_SETCHECK, iCtl_Out_PTS_Invent   ? BST_CHECKED : BST_UNCHECKED, 0);

}




LRESULT CALLBACK Preferences_Dialog(HWND hPrefDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   //TV_INSERTSTRUCT tvi;
   TC_ITEM tci;
   HTREEITEM hitem; // hItem1, hItem2, hItem3;
   TV_ITEM tvItem;
   TV_HITTESTINFO tvhit;
   POINT pt;
   int i;
   int nState;
   HWND hTab;
   SCROLLINFO sbi;
   char szBuf[80];
   HWND hscr;
   RECT rect;
   RECT rectdlg;
   DWORD dwHeight;
   bool bPaneOK;

   switch (message)
   {
  case WM_VSCROLL:
    sbi.cbSize=sizeof(sbi);
    sbi.fMask=SIF_ALL;

    hTab=GetDlgItem(hPrefDlg,IDC_TAB1);

    if (GetScrollInfo(GetDlgItem(hPrefDlg,IDC_SCROLLBAR1),SB_CTL,&sbi))
    {
      i=TabCtrl_GetCurSel(hTab);
      SetWindowPos(hPane[i],NULL,10,50+(sbi.nTrackPos*-1),0,0,SWP_NOSIZE);

      dwPanePos[i]=sbi.nTrackPos;
      wsprintf(szBuf,"%d %d",sbi.nTrackPos,i);
      SetWindowText(hPrefDlg,szBuf);

      SendDlgItemMessage(hPrefDlg,IDC_SCROLLBAR1,SBM_SETPOS,sbi.nTrackPos,0);
    }

     break;

  case WM_INITDIALOG:

      hPrefDlgG=hPrefDlg;

      hTab=GetDlgItem(hPrefDlg,IDC_TAB1);
      hChild=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_PREFERENCES_CHILD),hTab,(DLGPROC)Pane_Dialog,0);
      hPane[0]=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_PREFERENCES_ALL),hChild,(DLGPROC)Pane_Dialog,0);
      hPane[1]=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_PREFERENCES_BMP),hTab,(DLGPROC)Pane_Dialog,0);
      hPane[2]=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_PREFERENCES_OUTPUT),hTab,(DLGPROC)Pane_Dialog,0);
      hPane[3]=CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_PREFERENCES_ZEXP),hTab,(DLGPROC)Pane_Dialog,0);

      bPaneOK=true;

      for (i=0;i<cMAX_PANE;i++)
      {
        if (hPane[i]==NULL)
          bPaneOK=false;
      }

      if (!bPaneOK)
      {
        MessageBox(hPrefDlg,"Problem creating interface.","Error",MB_ICONERROR);
          return FALSE;
      }

      // Set scrollbar range & pos.
      SendDlgItemMessage(hPrefDlg,IDC_SCROLLBAR1,SBM_SETRANGE,40,500);
      SendDlgItemMessage(hPrefDlg,IDC_SCROLLBAR1,SBM_SETPOS,40,0);

      // Get rectangle for dialog & scrollbar.
      GetWindowRect(GetDlgItem(hPrefDlg,IDC_SCROLLBAR1),&rect);
      GetWindowRect(hChild,&rectdlg);

      dwHeight=rect.bottom-rect.top;
      SetWindowPos(hChild,NULL,10,30,rectdlg.right-rectdlg.left,dwHeight,SWP_SHOWWINDOW);

      // Make all window pane.
      MakePaneAll(hTab);

      ZeroMemory(&tci, sizeof(TCITEM));
      tci.mask = TCIF_TEXT | TCIF_IMAGE;
      tci.iImage = -1;
      tci.pszText = "All";
      TabCtrl_InsertItem(hTab,0,&tci);

      tci.pszText = "BMP";
      TabCtrl_InsertItem(hTab,1,&tci);

      tci.pszText = "Output";
      TabCtrl_InsertItem(hTab,2,&tci);

      tci.pszText = "Experimental";
      TabCtrl_InsertItem(hTab,3,&tci);

      ShowPane(0); // All tab.

      for (i=0;i<cMAX_PANE;i++)
        SetWindowPos(hPane[i],NULL,10,50,0,0,SWP_NOSIZE);

      ShowSettings();
      SaveStates();

      break;


    case WM_NOTIFY:
      switch(LOWORD(wParam))
      {
      case IDC_TREE1:
      if(((LPNMHDR)lParam)->code == NM_CLICK)
             {

        GetCursorPos(&pt);

        tvhit.pt.x = pt.x;
        tvhit.pt.y = pt.y;
        ScreenToClient(GetDlgItem(hPrefDlg,IDC_TREE1), &tvhit.pt);

        hitem=TreeView_HitTest(GetDlgItem(hPrefDlg,IDC_TREE1),&tvhit);

        tvItem.mask = TVIF_PARAM;
        tvItem.hItem = hitem;
        TreeView_GetItem(GetDlgItem(hPrefDlg,IDC_TREE1),(LPARAM)&tvItem);
        ShowPane(tvItem.lParam);

             }
      break;
      case IDC_TAB1:
        if(((LPNMHDR)lParam)->code == TCN_SELCHANGE)
        {
          i=TabCtrl_GetCurSel(GetDlgItem(hPrefDlg,IDC_TAB1));
          ShowPane(i);

          hscr=GetDlgItem(hPrefDlg,IDC_SCROLLBAR1);
          if (!i)
             ShowWindow(hscr,SW_SHOW);
          else
             ShowWindow(hscr,SW_HIDE);

        }
        break;
      }
      break;


    case WM_COMMAND:
      switch(LOWORD(wParam))
      {
      case IDC_OK:

        if (SendDlgItemMessage(hPrefDlg,IDC_CHK_SAVE, BM_GETCHECK, 0, 0) == BST_CHECKED)
        {
          MessageBox(0,"Save settings checked.","Message",MB_OK);
        }

        iCtl_BMP_Aspect=GetRadio(hPane[0],IDC_RDO_SAME,IDC_RDO_BICUBIC);

        for (i=IDC_CHK_AUDIOMATCHING;i<=IDC_CHK_ADJUSTTIMESTAMPS;i++)
        {
          nState=IsDlgButtonChecked(hPane[1],i);

          switch(i)
          {
          case IDC_CHK_AUDIOMATCHING+0:
            iCtl_Out_PTS_Match     = nState;
            break;
          case IDC_CHK_AUDIOMATCHING+1:
            iCtl_SetBrokenGop      = nState;
            break;
          case IDC_CHK_AUDIOMATCHING+2:
            iCtl_Out_Parse_AllPkts = nState;
            break;
          case IDC_CHK_AUDIOMATCHING+3:
            iCtl_Out_Parse_Deep    = nState;
            break;
          case IDC_CHK_AUDIOMATCHING+4:
            iCtl_Out_Align_Video   = nState;
            break;
          case IDC_CHK_AUDIOMATCHING+5:
            iCtl_Out_Align_Audio   = nState;
            break;
          case IDC_CHK_AUDIOMATCHING+6:
            iCtl_Out_TC_Adjust     = nState;
            break;
          }
        }

        // Do stuff.
        // ???

        // Restore options.
        RestoreStates();
        EndDialog(hPrefDlg,0);
        break;
      case IDC_CANCEL:
        EndDialog(hPrefDlg,0);
        break;
      case IDC_BTN_FOLDER:
        // Global browser call code. ???
        break;
      }
      break;

    case WM_CLOSE:
      EndDialog(hPrefDlg,0);
      break;
   }

   return false;
}


