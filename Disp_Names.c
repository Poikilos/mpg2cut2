
// NAMES DIALOG  &  NEWNAME (Rename) DIALOG

#include "global.h"
#include "PLUG.h"
#include "commctrl.h"

#define true  1
#define false 0

int bAscend=0;
int nSortedCol=-1;


int F580_ReOpen(int iEnough);
int F650_Rename();



//-------------------------

LRESULT CALLBACK F630_Newname_Dialog(HWND, UINT, WPARAM, LPARAM);

// Interface for Rename exit
typedef (WINAPI *pfnRNInit) (void*lsName, 
                             int *iFile_Current, int *iFile_Limit,
                             HWND  hWnd);
pfnRNInit RNInit;
void  F610_RenamePlugIn();

//-----------------------
void F600_NewName_Setup()
{
  int iRC;
  
  MParse.Stop_Flag = 2;
  iFileToRename = File_Ctr;

  if (PlugFileRename.iActive)
  {
     F610_RenamePlugIn();
  }
  else
  if (File_Limit < 1 
  || iFileToRename < 0
  || iFileToRename > File_Final)
  {
      MessageBeep(MB_OK);
  }
  else
  {
      // hNewnameDlg = CreateDialog(
      iRC = DialogBox(hInst, (LPCTSTR)IDD_NEWNAME_DIALOG,
                                  hWnd_MAIN, (DLGPROC)F630_Newname_Dialog);
      DSP5_Main_FILE_INFO();
    
  }

  MultiFile_SizeCalc();

}



//-------------------------------------------

void  F610_RenamePlugIn()
{
  int iRC, iEnough, iTmp1, iTmp_Total, iTmp_Curr, iTmp_Start;
  FARPROC lpRC;


  if (cRenamePlugIn_MultiMode == 'S')
  {
     if (iFileToRename < File_Limit)
     {
        process.i64RestorePoint[iFileToRename] = _telli64(FileDCB[iFileToRename]);
        _close(iFileToRename);
     }
  }
  else
  {
     iTmp1 = File_Limit;
     while (iTmp1) // Close all previous files 
     {
         iTmp1--;
         process.i64RestorePoint[iTmp1] = _telli64(FileDCB[iTmp1]);
         _close(FileDCB[iTmp1]);
     }
  }

  if (PlugFileRename.hDll)
     lpRC = PlugFileRename.fpInit;
  else
     lpRC = Plug81_Load_DLL(&PlugFileRename.hDll,
                            &PlugFileRename.fpInit,
                            &szRenamePlugIn_Name[0]); // TODO: do on app init only!

  if (! PlugFileRename.hDll || !lpRC)
    iRC = 0x6969;
  else
  {
      RNInit=(pfnRNInit)GetProcAddress(PlugFileRename.hDll,"Init"); // lpRC; //  

      if (cRenamePlugIn_MultiMode == 'M')
      {   // Multi-file mode supplies all input file names
          iTmp_Total =  File_Limit;  // Allow access to all used file names
          iTmp_Curr = iFileToRename; // Current file occurence# being viewed
          iTmp_Start = 0;      // Allow access from start of File_Name table
      }
      else
      {   // Single File mode - supplies only one occurence
          iTmp_Total = 1;              // Only access one occurence of table
          iTmp_Curr = 0;               // Use the first occurence passed
          iTmp_Start = iFileToRename;  // Pass this File Name as the first & only occurence
      }

      iRC = RNInit(&File_Name[iTmp_Start], &iTmp_Total, &iTmp_Curr, hWnd_MAIN);

      // FreeLibrary(PlugFileRename.hDll);  Defer unload until the end.
  }

  iEnough = 0;

  // Next action depends on nature of Plug-In :-
  //  - ASynchronous Plug-in - it must issue ReOpen via API
  //  - Synchronous  Plug-in - we can ReOpen as soon as it returns here.

  if (cRenamePlugIn_AsyncMode != 'A')  // NOT Async
  {
   if (cRenamePlugIn_MultiMode == 'S')
   {
     if (iFileToRename < File_Limit)
     {
        iEnough = F580_ReOpen(iEnough);
     }
   }
   else
   {
     iFileToRename = 0;
     while (iFileToRename < File_Limit) // Re-open all previous files 
     {
        iEnough = F580_ReOpen(iEnough);
        iFileToRename++;
     }

     File_Limit = iFileToRename;
     File_Final = File_Limit - 1;
   }
  }
}



//-------------------------------------------
LRESULT CALLBACK F630_Newname_Dialog(HWND hDialog, UINT message,
                                    WPARAM wParam, LPARAM lParam)
{
  int iEnough;
  int iWidth, iNameLen, iDirLen, iDrvLen; //, iHeight;
  char *lpSplit;

  HWND hNewName, hTxt;

  MParse.Stop_Flag = 2;
  iEnough = 0;

  switch (message)
  {
     case WM_INITDIALOG:

          if (iCtl_Out_DeBlank || iCtl_Out_MixedCase)
               FileNameTidy(&szTemp[0], File_Name[iFileToRename]);
          else
          {
             strcpy(szTemp, File_Name[iFileToRename]);
          }

          lpSplit = lpLastSlash(&szTemp[0]) + 1;
          if (!lpSplit)
               lpSplit = &szTemp[0];

          SetDlgItemText(hDialog, IDC_NEWNAME, lpSplit);

          *lpSplit = 0;
          lpSplit = strchr(szTemp, ':');
          if (lpSplit)
              lpSplit++;
          else
            lpSplit = &szTemp[0];

          // Alow for Network style double slash 
          if (*lpSplit == '\\')
               lpSplit++;
          if (*lpSplit == '\\')
               lpSplit++;

          SetDlgItemText(hDialog, IDC_NEWFOLDER, lpSplit);

          *lpSplit = 0;
          SetDlgItemText(hDialog, IDC_DRIVE, szTemp);
          
          FileDate2Gregorian(&File_Date[iFileToRename], 
                             &File_Greg[iFileToRename], &szBuffer, &" ");
          SetDlgItemText(hDialog, IDC_CREATED, szBuffer);

//          SetDlgItemText(hDialog, IDC_ATTR_NAME_2, "File Date:");

          ShowWindow(hDialog, SW_SHOW);

          hTxt = GetDlgItem(hDialog, IDC_NEWNAME);
          SetFocus(hTxt);

          iEnough = 1;
          //return true;
          break;


     case WM_COMMAND:
        switch (LOWORD(wParam))
        {
          case IDOK:

             iEnough = 0;

             iDrvLen = GetDlgItemText(hDialog,  IDC_DRIVE,    szTMPname, sizeof(szTMPname));
             lpSplit = stpcpy0(szTemp,  szTMPname);

             iDirLen = GetDlgItemText(hDialog, IDC_NEWFOLDER, szTMPname, sizeof(szTMPname));

             //if (szTMPname[0] != '\\')
             //    lpSplit = stpcpy0(lpSplit, &"\\");

             lpSplit = stpcpy0(lpSplit, szTMPname);

             iNameLen = GetDlgItemText(hDialog, IDC_NEWNAME, szTMPname, sizeof(szTMPname));
             if (iNameLen < 3 )
             {
                 sprintf(szBuffer,"File Name must be be in style :- NAME.EXT\n\n EG  FRED.MPG\n\nrc(%d)", 
                                                                       iNameLen);
                 MessageBox( NULL, szBuffer, "Mpg2Cut2 - That does not compute",
                                 MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
             }
             else
             {
                if (szTMPname[0] != '\\'
                &&  *(lpSplit-1) != '\\' )
                    lpSplit = stpcpy0(lpSplit, &"\\");

                lpSplit = stpcpy0(lpSplit, szTMPname);

                SetDlgItemText(hDialog, IDC_RESULT, szTemp);

                iEnough = F650_Rename();
            } // END Going For it
  
             break;


          case IDCANCEL:
               iEnough = 2;
               //DestroyWindow(hDialog);
               //return true;
               break;

         //   default:
         //      DestroyWindow(hDialog);
         //      hLumDlg = NULL;
         //      return true;

        } // END Switch LOWORD (COMMAND)

        break;

      case WM_SIZE:
           hNewName = GetDlgItem(hDialog, IDC_NEWNAME);
           iWidth   = LOWORD(lParam)-12; // - listbox left pos.
           //iHeight  = HIWORD(lParam)-24;
           SetWindowPos(hNewName, NULL, 0, 0, iWidth, 36, SWP_NOMOVE);
           return true;


  } // ENDSWITCH message

  // KILL ?
  if (iEnough > 1)
  {
      //DestroyWindow(hDialog);
      EndDialog(hDialog, iEnough);
      hNewnameDlg = 0;
      iEnough = 1;
  }

  return iEnough;
}





void F705_VideoList_LINE(int);
void F710_VideoList_Rebuild(int);
void F720_VideoList_ADD(int);
void F790_Video_OK(HWND hNamesDlg);
void F701_ADD(int);

void F707_NewName_Line();
void F709_DEL_Line();
void F702_Add_COL(HWND, int,  LV_COLUMN);



int iVidList_ctr;
HWND hNamesDlg;
HWND hList;


//----------------------------------------------------
LRESULT CALLBACK F700_Video_List(HWND P_hNamesDlg, UINT message,
                           WPARAM wParam, LPARAM lParam)
{
  int i; //, j, k;
  int iWidth, iHeight;
  unsigned int uTmp1;
  LPNMLISTVIEW pnm;
  RECT rTemp;
  

  LV_COLUMN lvc;
  // LV_ITEM lvi;
  // int iSel;



  MParse.Stop_Flag = 1;
  hNamesDlg = P_hNamesDlg;

  switch (message)
  {

    case WM_INITDIALOG:

      SetWindowText(hNamesDlg, VideoList_Title);

      iFileListChg_Flag = 0;   iVidList_ctr = 0;

      if (! iCtl_BasicName_Panel)
      {

        hList=GetDlgItem(hNamesDlg, IDC_VIEW1);
/*
        ZeroMemory(&lvc,sizeof(lvc));
        ZeroMemory(&lvi,sizeof(lvi));

        lvc.mask = LVCF_TEXT | LVCF_WIDTH;
        lvc.cx = 70;
        lvc.pszText = "Created";
        SendDlgItemMessage(hNamesDlg,IDC_VIEW1,LVM_INSERTCOLUMN,0,(LPARAM)&lvc);

        lvc.pszText = "Filename";
        SendDlgItemMessage(hNamesDlg,IDC_VIEW1,LVM_INSERTCOLUMN,1,(LPARAM)&lvc);
*/

        lvc.mask = LVCF_TEXT | LVCF_WIDTH;
      
        lvc.pszText = "Filename";
        lvc.cx = iCtl_ColumnWidth[0];
        F702_Add_COL(hNamesDlg, 0, lvc);


        lvc.pszText = "Created";
        lvc.cx = iCtl_ColumnWidth[1];
        F702_Add_COL(hNamesDlg, 1, lvc);

        if (! iCtl_Readability)
            SendDlgItemMessage(hNamesDlg, IDC_VIEW1, WM_SETFONT,
                             (WPARAM)(hDefaultGuiFont),
                                                     false);

      }

      if (File_Limit)
          F710_VideoList_Rebuild(0);
      else
      if (VideoList_MODE != 'd')
      {
          F720_VideoList_ADD(0);
      }

      if (File_Limit)
      {
         if (iCtl_BasicName_Panel)
            SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_SETCURSEL,
                                   File_Limit-1, 0);

         else
            ListView_SetItemState(hList, File_Limit-1,
                    LVIS_SELECTED | LVIS_FOCUSED, 
                    LVIS_SELECTED | LVIS_FOCUSED);
      }

          
      if (VideoList_MODE == 'd' // CONFIRMING DELETE OF ALL LISTED FILES ?
      &&  process.EDL_Used
      &&  szEDLname[0] != '*')
      {
         strcpy(File_Name[File_Limit], szEDLname);
         F705_VideoList_LINE(File_Limit);
      }

      return true;


    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case ID_ADD:
          F701_ADD(0);
          break;

        case ID_ADD_LIKE:
          F701_ADD(4);
          break;

        case ID_DEL:
          F709_DEL_Line();
          break;


        case ID_ONLY:
          if (File_Limit)
          {
             if (iCtl_BasicName_Panel)
             {
                i = SendDlgItemMessage(hNamesDlg, IDC_LIST,
                                                  LB_GETCURSEL, 0, 0);
                SendDlgItemMessage(hNamesDlg, IDC_LIST,
                                              LB_DELETESTRING, i, 0);
             }
             else
             {
                i = ListView_GetNextItem(hNamesDlg, -1, LVNI_SELECTED);
                SendDlgItemMessage(hNamesDlg, IDC_VIEW1,
                                              LVM_DELETEITEM, i, 0);
             }

             if (i >= File_Limit
             ||  i < 0)
                process.EDL_Used = 0;
             else
             {
                 F560_RemoveOtherFiles(i);
             }

            iVidList_ctr = 1;

            iFileListChg_Flag = 1;

             if (iCtl_BasicName_Panel)
             {
                SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_SETCURSEL, 0, 0);
             }
             //else
             //   ListView_SetItemState(hList, File_Limit-1,
             //         LVIS_SELECTED | LVIS_FOCUSED, 
             //         LVIS_SELECTED | LVIS_FOCUSED);
          }
          break;



        case ID_INFO:
          iCtl_Name_Info = 1 - iCtl_Name_Info;
          F710_VideoList_Rebuild(0);
         break;

        case ID_NEWNAME:
             F707_NewName_Line();
             break;


        case IDOK:
          F790_Video_OK(hNamesDlg);
          if (VideoList_MODE == 'd') // DELETE ALL LISTED FILES
          {
              F950_Close_Files('d');
          }
          return true;

        case IDCANCEL:
          F790_Video_OK(hNamesDlg);

          return true;

      } // END-SWITCH WM_COMMAND
      break;


      // WIDE
      case WM_SIZE:
           iWidth  = LOWORD(lParam);  // -12; // - listbox left pos.
           iHeight = HIWORD(lParam); //-60;

           if (iCtl_BasicName_Panel)
             uTmp1 = IDC_LIST;
           else
             uTmp1 = IDC_VIEW1;

            hList   = GetDlgItem(hNamesDlg, uTmp1);

           GetWindowRect(hList, &rTemp);	           
           SetWindowPos(hList, NULL, 
                                rTemp.left, rTemp.top,
                               (iWidth    - rTemp.left), 
                               (iHeight   - rTemp.top), 
                               SWP_NOMOVE);

           return true;
      
      case WM_NOTIFY:
           switch (LOWORD(wParam))
           {
            case IDC_VIEW1:
                 pnm = (LPNMLISTVIEW)lParam;    

                 if( ((LPNMHDR)lParam)->code == NM_DBLCLK)
                 {                               
                    hList = (LPNMHDR)pnm->hdr.hwndFrom;
                    i = ListView_GetNextItem(hList, -1, LVNI_SELECTED);

                    if (i >= 0 && i < File_Limit)
                    {
                       Mpeg_Stop_Rqst();

                       iKick.Action = ACTION_NEW_CURRLOC;
                       iKick.File = i;
                       iKick.Loc  = 0;

                       MPEG_processKick();

                       //MessageBox(0,"TODO: double click","",0); // TODO: Jump to file
                    }
                 }
                 else
                 /*
                 if( ((LPNMHDR)lParam)->code == LVN_COLUMNCLICK)
                 {
                     bAscend = ! bAscend;

                     nSortedCol = ((LPNMLISTVIEW)lParam)->iSubItem;
                     hList = (LPNMHDR)pnm->hdr.hwndFrom;
                     SendMessage(hList,LVM_SORTITEMS, nSortedCol,
                                                     (LPARAM)&LVSorter);
                 }
                 else 
                 */
                 if( ((LPNMHDR)lParam)->code == LVN_KEYDOWN)
                 {
                     LPNMLVKEYDOWN plvKeyDown = (LPNMLVKEYDOWN) lParam;

                     if(plvKeyDown->wVKey == VK_F2)
                     {
                        F707_NewName_Line();
                        //MessageBox(0,"TODO: rename","",0); // TODO: Call internal rename dialog
                     }
                     else
                     if(plvKeyDown->wVKey == VK_INSERT)
                     {
                        F701_ADD(0);
                     }
                     else
                     if(plvKeyDown->wVKey == VK_DELETE)
                     {
                        F709_DEL_Line();
                     }

                 }
           }
           break; 
      
  } // END-SWITCH message

  return false;

}





//==================================
/*
void F705_VideoList_LINE(int iP_Line)
{

  void *szVLine;

      if (iCtl_Name_Info)
      {
          FileDate2Gregorian(&File_Date[iP_Line], 
                             &File_Greg[iP_Line], // iFileToRename],  
                             &szBuffer, 
                              File_Name[iP_Line]);
          szVLine = &szBuffer;
      }
      else
          szVLine = File_Name[iP_Line];

    //if (iP_Line <= iVidList_ctr)
    //{
         SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_DELETESTRING, 
                            iP_Line,  0);

         SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_INSERTSTRING, 
                            iP_Line,  (LPARAM)szVLine);
    //}
    //else
    //{
    //   SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_ADDSTRING, 0,
    //                                             (LPARAM)szVLine);
    //   iVidList_ctr++;
    //}

}
*/


void F710_VideoList_Rebuild(int P_From)
{

  int i, iFrom;

  if (iCtl_BasicName_Panel)
    iFrom = P_From;
  else
  {
    iFrom = 0;
    SendDlgItemMessage(hNamesDlg, IDC_VIEW1, LVM_DELETEALLITEMS, 0, 0);
  }


  for (i = iFrom;  
       i < File_Limit; 
       i++)
  {
    //SendDlgItemMessage(hNamesDlg, IDC_VIEW1,
    //                                   LVM_DELETEITEM,  i, 0);
    F705_VideoList_LINE(i);
  }


}


//--------------------------------------------

void F701_ADD(int P_Like)
{

  if (VideoList_MODE == 'd')
  {
      MessageBeep(MB_OK);
  }
  else
  {
      F720_VideoList_ADD(P_Like);

      if (File_Limit)
      {
          if (iCtl_BasicName_Panel)
              SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_SETCURSEL,
                                                File_Final, 0);
          //else
          //     ListView_SetItemState(hList, File_Limit-1,
          //          LVIS_SELECTED | LVIS_FOCUSED, 
          //          LVIS_SELECTED | LVIS_FOCUSED);
      }

      SetFocus(hNamesDlg);

  }
}

//---------------------------------------------

void F702_Add_COL(HWND P_hNamesDlg, int P_iCol,  LV_COLUMN P_lvc)
{

  int iTmp1;

  // Protect against losing sight of field

  iTmp1 = VGA_Width - 30;

  if (P_lvc.cx > iTmp1) 
      P_lvc.cx = iTmp1; 

  if (P_lvc.cx < 50)
      P_lvc.cx = 50; 
  
  SendDlgItemMessage(P_hNamesDlg, IDC_VIEW1, LVM_INSERTCOLUMN,
                     P_iCol, (LPARAM)&P_lvc);

}




//---------------------------------------------

void F707_NewName_Line()
{
  int i;
        
  if (iCtl_BasicName_Panel)
      i = SendDlgItemMessage(hNamesDlg, IDC_LIST,
                                        LB_GETCURSEL, 0, 0);
  else
      i = ListView_GetNextItem(hNamesDlg, -1, LVNI_SELECTED);

  iFileToRename = i;

  if (File_Limit < 1 
  || iFileToRename < 0 
  || iFileToRename >= File_Limit)

      MessageBeep(MB_OK);

  else
  {
      DialogBox(hInst,     (LPCTSTR)IDD_NEWNAME_DIALOG, 
                hWnd_MAIN, (DLGPROC)F630_Newname_Dialog);

      F705_VideoList_LINE(i);
  }

  if (iCtl_BasicName_Panel)
  {
     if (i >= File_Limit)
         i  = File_Limit-1;

     SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_SETCURSEL, i, 0);
  }
  //else
  //   ListView_SetItemState(hList, File_Limit-1,
  //                  LVIS_SELECTED | LVIS_FOCUSED, 
  //                  LVIS_SELECTED | LVIS_FOCUSED);

}


//------------------------------------------

void F709_DEL_Line()
{
  int i;

  szEDLprev[0] = 0;

  if (File_Limit)
  {
     if (iCtl_BasicName_Panel)
     {
         i = SendDlgItemMessage(hNamesDlg, IDC_LIST,
                                           LB_GETCURSEL, 0, 0);

         SendDlgItemMessage(hNamesDlg, IDC_LIST,
                                       LB_DELETESTRING, i, 0);
     }
     else
     {
         i = ListView_GetNextItem(hNamesDlg, -1, LVNI_SELECTED);

         SendDlgItemMessage(hNamesDlg, IDC_VIEW1,
                                       LVM_DELETEITEM,  i, 0);
     }

     if (i >= File_Limit
     ||  i <  0)

         process.EDL_Used = 0; 

     else
     {
         F570_RemoveFile(i, 1);
     }

     iVidList_ctr--;

     iFileListChg_Flag = 1;

     if (iCtl_BasicName_Panel)
     {
         if (i >= File_Limit)
             i  = File_Limit-1;

         SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_SETCURSEL, i, 0);
     }
     //else
     //    ListView_SetItemState(hList, File_Limit-1,
     //               LVIS_SELECTED | LVIS_FOCUSED, 
     //               LVIS_SELECTED | LVIS_FOCUSED);
 
  }

}


//-------------------------------------------
void F720_VideoList_ADD(int P_Like)
{

  int i;

  i = File_Limit;

  F100_IN_OPEN('a', P_Like);

  if (i == 0 && File_Limit)
      iFileListChg_Flag = 1;

  F710_VideoList_Rebuild(i);

  
}  

//  char seq;
/*
  if (X800_PopFileDlg  (szInput, hNamesDlg, INPUT_VOB, -1, 
                      &"Add Mpeg2 File"))
  {
    while (_findfirst(szInput, &seqfile) != -1L)
    {
      SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_ADDSTRING, 0,
                                        (LPARAM)szInput);

      strcpy(File_Name[File_Limit], szInput);
      FileDCB[File_Limit] = _open(szInput,
                          _O_RDONLY | _O_BINARY | _O_SEQUENTIAL);

      File_Final = File_Limit;
      File_Limit++;

      break;

    //  strncpy(szInput+strlen(szInput)-5, &seq, 1);
    }
  } 
 */




//------------------------------------

void F790_Video_OK(HWND hNamesDlg)
{
  int iCol;

  // Save the User adjusted Column Widths
  if (! iCtl_BasicName_Panel)
  {
    for (iCol = 0; iCol < 2; iCol++)
    {
       iCtl_ColumnWidth[iCol] = ListView_GetColumnWidth(hList, iCol); 
    }

  }


  EndDialog(hNamesDlg, 0);


  if (VideoList_MODE != 'd' && iFileListChg_Flag)
  {
      //ProcessReset("CAN"); // Reset after finishing F700_Video_List dialog

      if (File_Limit)
      {
          //C100_Clip_DEFAULT('a');
          T590_Trackbar_SEL();

          iKick.Action = ACTION_INIT;
          MPEG_processKick();

      }
  }
}







// Default File filter

//#define DEFAULT_DESC 
//#define DEFAULT_FILT 
//#define DEFAULT_ALLF 
//#define DEFAULT_ALLX

char *szFilter, *szBufPtr, *szSlashPTR, *szTmp1;

void X805_Default_Filter()
{
   szBufPtr = stpcpy1(szBufPtr, "MPEG Stream (*.vob; *.mpg; *.mpeg; *.m2p; *.m2v; *.mpv; *.m1p; *.m1v; *.EVO; *.ts; *.pva; *.m2t)");  //    DEFAULT_DESC);
   szBufPtr = stpcpy1(szBufPtr, "*.vob;*.mpg;*.mpeg;*.m2p;*.m2v;*.mpv;*.m1p;*.m1v;*.EVO;*.get*;*.part*;*.0*;*.PVA;*.ts;*.m2t");    //    DEFAULT_FILT);
   szBufPtr = stpcpy1(szBufPtr, "All Files (*.*)");  //    DEFAULT_ALLF);
   szBufPtr = stpcpy1(szBufPtr, "*.*"  );  //    DEFAULT_ALLX);
         
         //szBufPtr = stpcpy1(szBufPtr,     "All Files (*.*)");
         //szBufPtr = stpcpy1(szBufPtr,     "*.*");

   *szBufPtr = 0;  // END of buffer needs extra null

   szFilter  = &szBuffer[0]; // Point to start of buffer
}


//------------------------------------------------------------
int X801_FileListPlugIn(PTSTR pstrFileName, HWND hOwner, 
                       int P_Action, int P_Trunc, char *lp_P_Title);




int X800_PopFileDlg(PTSTR pstrFileName, HWND hOwner, 
                       int P_Action, int P_Trunc, char *lp_P_Title)
{
  int iTmp, iRC;
  DWORD dTmp1;

  iRC = 0;
//  int count = 0;

  szSlashPTR = lpLastSlash(pstrFileName);
  if (szSlashPTR)
      szSlashPTR++;
  else
      szSlashPTR = pstrFileName;

  ofn.lpstrInitialDir = NULL;

  // Default File filter
  szBufPtr = &szBuffer[0];
  X805_Default_Filter();



  switch (P_Action)
  {
    case INPUT_VOB:

      // Optionally build partial file name wild cards 
      // based on previous name

      lpFName = szSlashPTR; // lpLastSlash(&szInput[0]);

      if (szInput[0] > ' '  &&  P_Trunc >= 0)
      {
         //if (lpFName)
         //   lpFName++;
         //else
         //  lpFName = &szInput[0];
 
         strncpy(szFile_Prefix, lpFName, MAX_LIKE_LEN);
         iTmp = P_Trunc;
         if(szFile_Prefix[iTmp] == 0)
            iTmp = iTmp/2;
         szFile_Prefix[iTmp] = 0;
 
         iTmp = sprintf(szTemp, 
                "%s*.%s",
                //"%s*.vob;%s*.mpg;%s*.mpeg;%s*.m2p;%s*.m2v;%s*.mpv;%s*.m1p;%s*.m1v;%s*.get*;%s*.part*;%s*.0*",
                //szFile_Prefix, szFile_Prefix, szFile_Prefix, szFile_Prefix, 
                //szFile_Prefix, szFile_Prefix, szFile_Prefix, szFile_Prefix, 
                //szFile_Prefix, szFile_Prefix, szFile_Prefix, szFile_Prefix,
                szFile_Prefix, cInExt);

         // How come string handling is easier in Cobol than in "C" ? !        
         szBufPtr = stpcpy1(&szBuffer[0], "Similar MPEG-2 Stream");
         szBufPtr = stpcpy1(szBufPtr,     &szTemp[0]);
         szBufPtr = stpcpy1(szBufPtr,     "MPG Files (*.mpg)");
         szBufPtr = stpcpy1(szBufPtr,     "*.mpg");
         szBufPtr = stpcpy1(szBufPtr,     "VOB Files (*.VOB)");
         szBufPtr = stpcpy1(szBufPtr,     "*.VOB");
         
         X805_Default_Filter();

      }

      break;



    case SAVE_VOB:

        iTmp = 0;
        szBufPtr = &szBuffer[0];

        if (ofn.lpstrDefExt)
        {

          if (! stricmp(ofn.lpstrDefExt, "M2V")
          ||  ! stricmp(ofn.lpstrDefExt, "M1V"))
          {
             iTmp = sprintf(szTemp, "*.%s;*.m1v;*.m2v;*.mpv;",
                                    ofn.lpstrDefExt);

             szBufPtr = stpcpy1(&szBuffer[0], "Mpeg Elementary Stream");
          }
          else
          if (stricmp(szOut_Xtn_RULE, "vob")
          &&  *ofn.lpstrDefExt > ' ')
          {
              iTmp = sprintf(szTemp, "*.%s;*.vob;*.mpg;*.mpeg;*.m2p;*.EVO;*.m2t;",
                                     ofn.lpstrDefExt);

              szBufPtr = stpcpy1(&szBuffer[0], "MPEG Stream");
          }

        }

        if (iTmp)
        {
           // How come string handling is easier in Cobol than in "C" ? ! !        
           szBufPtr = stpcpy1(szBufPtr,     &szTemp[0]);
           szBufPtr = stpcpy1(szBufPtr,     "All Files (*.*)");
           szBufPtr = stpcpy1(szBufPtr,     "*.*");
          *szBufPtr = 0;
             
          szFilter = (char *)(&szBuffer);
        }

        if (iCtl_Out_Folder_Active) // Override default folder ?
        {
            if (process.iOutFolder                        // NOT FIRST TIME
            &&  iCtl_Out_Folder_Active != 2               // NOT "MOST RECENT"
            && (WindowsVersion < 0x80000000 || DBGflag )) //  Windows NT, Win2k, WinXP ?
            {
               strcpy(szTMPname,  pstrFileName);
               szTmp1 = lpLastSlash(pstrFileName);  // How about: (szSlashPTR-1)
               if (!szTmp1)
                    szTmp1 = pstrFileName;
               *szTmp1 = 0;

            }
            else
            {
                strcpy(szTMPname, &szCtl_Out_Folder[0]); // Win98 allows both at same time
            }

            ofn.lpstrInitialDir = &szTMPname[0];

            if ((WindowsVersion < 0x80000000 || DBGflag ) // Windows NT+ ?
                || ! iCtl_Out_Folder_Both)                // Treat Win98 same as Win2K
            {
                strcpy(pstrFileName, szSlashPTR);
            }
            
        }
        //else
        //      ofn.lpstrInitialDir = NULL;
        
      break;



    case SAVE_BMP:
              ofn.lpstrInitialDir = &szCtl_BMP_Folder[0];
              ofn.lpstrDefExt = &"BMP";
              if (WindowsVersion < 0x80000000 || DBGflag ) // Windows NT ?
              {
                  strcpy(pstrFileName, szSlashPTR);
              }
              szFilter = TEXT ("Image (*.bmp)\0*.*.bmp\0")  \
                     TEXT ("All Files (*.*)\0*.*\0");
      break;


    case LOAD_EDL:
    case SAVE_EDL:

              ofn.lpstrDefExt = &"EDL";

              //if (WindowsVersion < 0x80000000 || DBGflag ) // Windows NT ?
              //{
              //    strcpy(pstrFileName, szSlashPTR);
              //}
              szFilter = TEXT ("Edit List (*.EDL)\0*.EDL\0")  \
                     TEXT ("All Files (*.*)\0*.*\0");
      break;

    case LOAD_CHAP:
    case SAVE_CHAP:

              ofn.lpstrDefExt = &"CHAP";
              if (WindowsVersion < 0x80000000 || DBGflag ) // Windows NT ?
              {
                  strcpy(pstrFileName, szSlashPTR);
              }
              szFilter = TEXT ("Edit List (*.CHAP)\0*.CHAP\0")  \
                     TEXT ("All Files (*.*)\0*.*\0");
      break;


/*   case SAVE_AVI:
         szFilter = TEXT ("AVI File (*.avi)\0*.avi; *.ac3; *.wav; *.mpa\0")   \
            TEXT ("All Files (*.*)\0*.*\0");
         break;

      case OPEN_D2V:
         szFilter = TEXT ("DVD2AVI Project File (*.d2v)\0*.d2v\0")  \
            TEXT ("All Files (*.*)\0*.*\0");
         break;

      case SAVE_D2V:
         szFilter = TEXT ("DVD2AVI Project File (*.d2v)\0*.d2v; *.ac3; *.wav; *.mpa\0")   \
            TEXT ("All Files (*.*)\0*.*\0");
         break;

      case OPEN_WAV:
      case SAVE_WAV:
         szFilter = TEXT ("WAV File (*.wav)\0*.wav\0")  \
            TEXT ("All Files (*.*)\0*.*\0");
         break;*/
  }

  ofn.lStructSize     = sizeof (OPENFILENAME) ;
  ofn.hwndOwner       = hOwner ;
  ofn.hInstance       = hInst ;
  ofn.lpstrFilter     = szFilter ;
  ofn.nMaxFile        = _MAX_PATH ;
  ofn.nMaxFileTitle   = _MAX_PATH ;
  ofn.lpstrFile       = pstrFileName;
  ofn.lpstrTitle      = lp_P_Title;


  //if (P_Trunc > 0)        // Mask ?
  //    *pstrFileName = 0;  // Don't show mask in file name area

  //ofn.lpstrDefExt       = "mpg";

  strcpy(szMsgTxt,"Accessing...");
  DSP1_Main_MSG(0,0);
  iMsgLife = 0;   szMsgTxt[0] = 0;


  // Open for READ ?
  if  (P_Action > 0)
  {
       ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER
                              | OFN_ALLOWMULTISELECT;// | OFN_HIDEREADONLY;
       ofn.nMaxFile   = 16384 ;

       if (PlugFileList.iActive)
           iRC = X801_FileListPlugIn(pstrFileName, hOwner, 
                                   P_Action, P_Trunc, lp_P_Title);
       else
           iRC = 0x6969;   // Exit not invoked 

       if (iRC == 0x6969) // Exit not invoked
       {
           iRC = GetOpenFileName(&ofn);
           if (!iRC)                          // Failed ?
           {
             dTmp1 = CommDlgExtendedError();  // What went wrong ?
             if (dTmp1)                       // Technical Problem ?
             {
                if (dTmp1 == FNERR_INVALIDFILENAME)  // Typo ?
                {
                    sprintf(szBuffer, "BAD STRUCTURE in File Name or Folder Name\n\nFile = %s\n\n",
                                            pstrFileName);
                }
                else
                    sprintf(szBuffer, "Cannot show a File List Window For\n\nFile =%s\n\n",
                                            pstrFileName);
 
                MessageBox(hWnd_MAIN, szBuffer, szAppName, MB_OK);
                szInput[0] = 0;  // RESET THE INPUT FILE NAME
             }
           }
       }
       iTmp = iRC;
  }
  else
  {
      // Open for OUTPUT
      ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER;

      //if (PlugFileList.iActive)
      //    iRC = X801_FileListPlugIn(pstrFileName, hOwner, 
      //                             P_Action, P_Trunc, lp_P_Title)
      //else
           iRC = GetSaveFileName(&ofn);

           if  (P_Action == SAVE_VOB)
           {   // Remember the output folder
               strcpy(&szOutFolder[0], ofn.lpstrFile);
               szTmp1 = lpLastSlash(&szOutFolder[0]);
               if (!szTmp1)
                    szTmp1 = &szOutFolder[0];
              *szTmp1 = 0;
              if (iCtl_Out_Folder_Active == 2)  // Use most recent output folder
                  strcpy(&szCtl_Out_Folder[0], &szOutFolder[0]);
           }

  }

  if (iMsgLife <= 0)
  {
    //  Repair the underlying window info
    TextOut(hDC, 0, iMsgPosY, BLANK44, 44); // Would be better as a Paint
    if (!iMainWin_State || iViewToolBar >= 256)
         DSP2_Main_SEL_INFO(1);
    UpdateWindow(hWnd_MAIN);
  }


  return iRC;
}








//--------------------------------------------------------
typedef (WINAPI *pfnFBInit) (OPENFILENAME* ofn);
pfnFBInit FBInit;

int X801_FileListPlugIn(PTSTR pstrFileName, HWND hOwner, 
                       int P_Action, int P_Trunc, char *lp_P_Title)
{      

  OPENFILENAME ofnx; 
  int iRC;
  FARPROC lpRC;


  if (PlugFileList.hDll)
     lpRC = PlugFileList.fpInit;
  else
     lpRC = Plug81_Load_DLL(&PlugFileList.hDll,
                            &PlugFileList.fpInit,
                            &szFileListPlugIn_Name[0]); // TODO: do on app init only!

  if (! PlugFileList.hDll || !lpRC)
    iRC = 0x6969;
  else
  {

    FBInit=(pfnFBInit)GetProcAddress(PlugFileList.hDll,"Init"); //lpRC; // 

    ofnx.lStructSize     = sizeof (OPENFILENAME) ;
    ofnx.hwndOwner = hOwner; // hWnd;
    ofnx.lpstrTitle = lp_P_Title;
    ofnx.lpstrInitialDir = NULL;
    ofnx.nMaxFile   = 16384;
    ofnx.nMaxFile        = _MAX_PATH ;
    ofnx.nMaxFileTitle   = _MAX_PATH ;
    ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;

    iRC = FBInit(&ofnx);

    lstrcpy(pstrFileName, ofnx.lpstrFile);
    
    // FreeLibrary(PlugFileList.hDll);  Defer unload until the end.
  }

  return iRC;
}






// --------------------------------------------------------
void F920_Init_Names()
{
  int i ;

  File_Final = 0; File_Limit = 0; process.iOut_Part_Ctr = 0;

  for (i=0; i<=MAX_FILE_NUMBER; i++)
  {
            File_Name[i] = (char*)malloc(_MAX_PATH);
            if (File_Name[i] == NULL)
            {
                Err_Malloc(&"F920");
                return;
            }
  }

}



//--------------------------------------------

// Convert C File Time Stamp from Julian to Gregorian 
// 

void FileDate2Gregorian(IFILEDATE* P_FileDate, 
                        struct tm* P_Gregorian, 
                             void* szP_Gregorian,
                             void* szP_More)
{
  struct tm tmTemp1;
  TmpGregTime = localtime(P_FileDate);

  // Conia DVD recorder sets invalid File Dates 
  //                                          (1955 - Back To The Future !)
  if (TmpGregTime > 0)
     memcpy(P_Gregorian, TmpGregTime, sizeof(File_Greg[0]));
  else
  {
     ZeroMemory(P_Gregorian, sizeof(File_Greg[0]));
     TmpGregTime = &tmTemp1; //P_Gregorian;

     RJ_Date2Gregorian(P_FileDate, P_Gregorian, szP_Gregorian, szP_More);

     sprintf(szMsgTxt, "BAD FILE DATE: x%X  Maybe: %s", 
                       P_FileDate, szP_Gregorian);
     if (! iWarnBadDate && !iSuppressWarnings)
     {
       iWarnBadDate = 1;
       MessageBox(hWnd_MAIN, szMsgTxt, "Mpg2Cut2 - FILE ERROR",
                                        MB_ICONSTOP | MB_OK);
     }
     return;
  }

  sprintf(szP_Gregorian, "%04d-%02d-%02d  %02d%02d.%02d  %s", // %08x_%08d ", 
                            (TmpGregTime->tm_year + 1900), 
                            (TmpGregTime->tm_mon + 1),
                             TmpGregTime->tm_mday, 
                             TmpGregTime->tm_hour, 
                             TmpGregTime->tm_min, 
                             TmpGregTime->tm_sec,
                             //*P_FileDate, *P_FileDate,
                             szP_More);

}



/*
//--------------------------------------------

// Convert WIN32 API File Time Stamp from Julian to Gregorian
//           Don't know the name of the system routine to do this

void FileDate2Gregorian(void* P_FileDate, 
                  struct tm * P_Gregorian, 
                        void* szP_Gregorian,
                        void* szP_More)
{
  int iDateTime, iDaysSince1601, iDaysSince1900;
  int iYear, iMth, iDay, iLept;
  int iTime, iHour, iMin, iSec, iT2;

  iDateTime  = (int) ( ( *(__int64*)(P_FileDate) ) / 1000000000); // Strip the Nanoseconds

  iDaysSince1601 = iDateTime / 86400;       // Days since 1601 Jan 01
  iDaysSince1900 = iDaysSince1601 - 87294;  // Days between 1601 and 1900 (sorta)
      
  iLept =  iDaysSince1900 / 1461;           // Days per 4 year cycle
            
  iYear = (iDaysSince1900 -  iLept) / 365;  // Days per NON-Leap year;

  iDay  =  iDaysSince1900 - (iLept * 1461) - (iYear * 365);

  if ( ! iYear && iDay > 59)  // Allow for final leap day
    iDay--;

  if (iDay < 32)
  {
    iMth  = 1;    // JAN 31
    iDay -= 0; 
  }
  else
  if (iDay < 60)
  {
    iMth  = 2;    // FEB 28
    iDay -= 31; 
  }
  else
  if (iDay < 91)
  {
    iMth  = 3;    // MAR 31
    iDay -= 59; 
  }
  else
  if (iDay < 122)
  {
    iMth  = 4;    // APR 30
    iDay -= 90; 
  }
  else
  if (iDay < 152)
  {
    iMth  = 05;    // MAY 31
    iDay -= 121; 
  }
  else
  if (iDay < 183)
  {
    iMth  = 6;    // JUN 30
    iDay -= 151; 
  }
  else
  if (iDay < 213)
  {
    iMth  = 7;    // JUL 31
    iDay -= 182; 
  }
  else
  if (iDay < 244)
  {
    iMth  = 8;    // AUG 31
    iDay -= 212; 
  }
  else
  if (iDay < 275)
  {
    iMth  = 9;    // SEP 30
    iDay -= 243; 
  }
  else
  if (iDay < 305)
  {
    iMth  = 10;    // OCT 31
    iDay -= 274; 
  }
  else
  if (iDay < 336)
  {
    iMth  = 11;    // NOV 30
    iDay -= 304; 
  }
  else
  if (iDay < 366)
  {
    iMth  = 12;    // DEC 31
    iDay -= 335; 
  }
  else
  {
    iMth  = 13;    // ERROR
    iDay -= 365; 
  }


  iYear = ((iLept * 4) + iYear + 19);

  // TIME
  iTime  = iDateTime - (iDaysSince1601 * 86400);

  iHour = iTime / 3600;
  iT2   = iHour * 3600;

  iMin = (iTime - iT2) / 60;
  iSec =  iTime - iT2  - (iMin * 60);

  sprintf(szP_Gregorian,"%04d-%02d-%02d_%02d%02d%02d %s", 
                         iYear, iMth, iDay, iHour, iMin, iSec, *szP_More);

}
*/





//--------------------------------------------

// Convert C File Time Stamp from Julian to Gregorian
//           Since intrinsic function "localtime()" is unreliable

void RJ_Date2Gregorian(int* P_FileDate, 
                  struct tm * P_Gregorian, 
                        void* szP_Gregorian,
                        void* szP_More)
{
  unsigned int uDate, uTime;

  int iYear, iMth, iDay, iHour, iMin, iSec;
  int iLept, iLim;


  uTime  = (*P_FileDate) >>16;  //  / 0x010000; 
  uDate  = (*P_FileDate) & 0xFFFF; 
            
  iDay  =  uDate & 0x1F;
  iMth  = (uDate & 0xE0) / 32;

  iYear =((uDate & 0xFF00) / 256);

  // Allow for negative year from Conia (xFF)

  // NOTE: Windows reports xFF as 1955 by truncating 255 to 2 digits,
  //       despite the public doc saying to treat it as offset from 1980, 
  //       So I shall do the same as Windows practice.

  // Bit Layouts: ff_ftime and ff_fdate


  if (iYear < 128)
      iYear = iYear + 1980;
  else
  {
      //iYear = 1724 + iYear; // According to public doc
      iYear = iYear % 100;
  }

  iSec  = (uTime & 0x001F) * 2;   
  iMin  = (uTime & 0x07E0) >>5 ;    
  iHour = (uTime & 0xF800) >>11;   

  // Allow for invalid File TimeStamps from Conia 

  // Conia Hour=31 Min=63 Sec=62
  // shows as H=23   M=04 in Windows Explorer (Win98SE)

  // I cannot figure out how Windows converts invalid times
  // So I will just apply my own rules

  if (iSec > 59)
  {
      iSec -= 60;
      iMin++;
  }
  if (iMin > 59)
  {
      iMin -= 60;
      iHour++;
  }
  if (iHour > 23)
  {
      iHour -= 24;
      iDay++;
  }

  if (iDay > 27)
  {
     switch (iMth)
     {

     case  4:    // APR 30
     case  6:    // JUN 30
     case  9:    // SEP 30
     case 11:    // NOV 30
          if (iDay > 31)
          {
              iDay -= 31;
              iMth++;  
          }
     break;


     case  2:    // FEB 27 or 28
           iLept = iYear / 4;
           if (iLept)
               iLim = 28;
           else
               iLim = 27;

           if (iDay > iLim)
           {
               iDay -= iLim;
               iMth++;    // JAN 31
           }
     break;

     default: 
          if (iDay > 31)
          {
              iDay -= 31;
              iMth++;  
          }
     break;
     }// END SWITCH Month
  }

  if (iMth > 12)
  {
      iMth -= 12;
      iYear++;
  }

  P_Gregorian->tm_year  = iYear;
  P_Gregorian->tm_mon   = iMth;
  P_Gregorian->tm_mday  = iDay;
  P_Gregorian->tm_wday  = 0; // Could use Zueller's Congruence, but too much like hard work.
  P_Gregorian->tm_yday  = 0;
  P_Gregorian->tm_hour  = iHour;
  P_Gregorian->tm_min   = iMin;
  P_Gregorian->tm_sec   = iSec;
  P_Gregorian->tm_isdst = 0;

  sprintf(szP_Gregorian,"%04d-%02d-%02d  %02d%02d%02d %s", 
                         iYear, iMth, iDay, iHour, iMin, iSec, szP_More);

}








//--------------------------------------------------------------

// From WewantWideScreen 2nd March 2006

/*

Made FileDate2GregorianEx to replicate windows explorer date system.
(Merge with existing FileDate2Gregorian?)
If possible you can add code to source but comment it out like other
listview code in F700_list(). I haven't added mpeg_time column, need
to wire in lvm_sortitems for file menu sort items, wm_size line. Did
quick test filenames/dates seem to sort, F2 key for rename + double
click currently show message boxes.

*/


void FileDate2GregorianEx(IFILEDATE* P_FileDate,
                        struct tm* P_Gregorian,
                             void* szP_Gregorian,
                             void* szP_More)
{

  int iRC;
  struct _SYSTEMTIME SysTime;
  //struct tm tmTemp1;


  TmpGregTime = localtime(P_FileDate);
  // Conia DVD recorder sets invalid File Dates 
  //                                          (1955 - Back To The Future !)
  if (TmpGregTime > 0)
     memcpy(P_Gregorian, TmpGregTime, sizeof(File_Greg[0]));
  else
  {
     ZeroMemory(P_Gregorian, sizeof(File_Greg[0]));
     TmpGregTime = P_Gregorian;

     RJ_Date2Gregorian(P_FileDate, P_Gregorian, szP_Gregorian, szP_More);

     sprintf(szMsgTxt, "BAD FILE DATE: x%X  Maybe: %s", 
                       P_FileDate, szP_Gregorian);

     if (! iWarnBadDate && !iSuppressWarnings)
     {
       iWarnBadDate = 1;
       MessageBox(hWnd_MAIN, szMsgTxt, "Mpg2Cut2 - FILE ERROR",
                                          MB_ICONSTOP | MB_OK);
     }
     //return;
  }



  SysTime.wYear         = (TmpGregTime->tm_year + 1900);
  SysTime.wMonth        = (TmpGregTime->tm_mon + 1);
  SysTime.wDayOfWeek    =  TmpGregTime->tm_wday;
  SysTime.wDay          =  TmpGregTime->tm_mday;
  SysTime.wHour         =  TmpGregTime->tm_hour;
  SysTime.wMinute       =  TmpGregTime->tm_min; 
  SysTime.wSecond       =  TmpGregTime->tm_sec;
  SysTime.wMilliseconds =  0;

  // Default format is International Gregorian 
  // (Used on IBM mainframes (eg ISPF), plus Japan & some other countries)
  sprintf(szTmp32, "%04d-%02d-%02d ",
                            (TmpGregTime->tm_year + 1900),
                            (TmpGregTime->tm_mon + 1),
                             TmpGregTime->tm_mday);

  // Optionally convert to User's personalized date format
  if (! iCtl_Date_Internationale)
  {
    iRC = GetDateFormat(LOCALE_USER_DEFAULT, 
                      DATE_SHORTDATE, 
                      &SysTime, 
                      NULL, 
                      &szTmp32[0], 
                      32);

    //   if (iRC <= 0)
    //      Msg_LastError("UsrDate ", 706, 'b');
  }

  sprintf(szP_Gregorian, "%s %02d:%02d %s",
                             szTmp32,
                             TmpGregTime->tm_hour,
                             TmpGregTime->tm_min,
                             szP_More);

}




void F705_VideoList_LINE(int iP_Line)
{
  LV_ITEM lvi;
  void *szVLine;
  int nItem;
         
  if (iCtl_BasicName_Panel)
  {
      if (iCtl_Name_Info)
      {
          FileDate2Gregorian(&File_Date[iP_Line], 
                             &File_Greg[iP_Line], // iFileToRename],  
                             &szBuffer, 
                              File_Name[iP_Line]);
          szVLine = &szBuffer;
      }
      else
          szVLine = File_Name[iP_Line];

     SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_DELETESTRING, 
                                     iP_Line,  0);

     SendDlgItemMessage(hNamesDlg, IDC_LIST, LB_INSERTSTRING, 
                                    iP_Line,  (LPARAM)szVLine);
  }
  else
  {
     FileDate2GregorianEx(&File_Date[iP_Line],
                       &File_Greg[iP_Line], //iFileToRename],
                       &szBuffer,"");

     nItem=SendDlgItemMessage(hNamesDlg, IDC_VIEW1, LVM_GETITEMCOUNT, 0, 0);
        
     lvi.mask = LVIF_TEXT | LVIF_PARAM;

     lvi.iItem = nItem;
     lvi.lParam = iP_Line;

     lvi.iSubItem = 0;

     lvi.pszText = File_Name[iP_Line];
     nItem=SendDlgItemMessage(hNamesDlg, IDC_VIEW1, LVM_INSERTITEM, 0, 
                                                 (LPARAM)(&lvi));
        
     lvi.iSubItem = 1;
     lvi.pszText = szBuffer;
     SendDlgItemMessage(hNamesDlg, IDC_VIEW1, LVM_SETITEMTEXT, 
                                                  nItem,  (LPARAM)(&lvi));
  }

}



