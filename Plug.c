
// Plug-in controls


#include "global.h"
#include "PLUG.h"
#define true 1
#define false 0


//---------  PLUG-IN CONTROLS --------------------------------------

// Out of habit I used the IBM term "Exit",
// which basically means the same thing as "Plug-In"


int iPostQuote;


void PostProc_Join(HWND hDialog, char* P_Cmd, char* P_Qual)
{

  char szMaybeQuote[4];

  if (iPostQuote)
      strcpy(szMaybeQuote, "\"");
  else
      szMaybeQuote[0] = 0;

  sprintf(szBuffer, "%s %s%s%s %s", P_Cmd,
                            szMaybeQuote, szOutput, szMaybeQuote,
                                    P_Qual
                                    );

  SetDlgItemText(hDialog, IDC_POST_LINE, szBuffer);

  return;
}





//----------------------------------------------------------------
LRESULT CALLBACK PostProc_Dialog(HWND hDialog, UINT message,
                                WPARAM wParam, LPARAM lParam)
{
  int iEnough;
  char szTmp_A[_MAX_PATH*6], szTmp_B[_MAX_PATH*6];
  // char szMaybeQuote[4];

  iEnough = 0;

  switch (message)
  {
     case WM_INITDIALOG:

          iPostQuote = iCtl_Out_PostQuote;
          strcpy(szTmp_A, szCtl_Out_ProcLine_A);
          strcpy(szTmp_B, szCtl_Out_ProcLine_B);

          SetDlgItemText(hDialog, IDC_PROC_LINE_A, szCtl_Out_ProcLine_A);
          SetDlgItemText(hDialog, IDC_PROC_LINE_B, szCtl_Out_ProcLine_B);

          if (iCtl_Out_PostProc)
             SendDlgItemMessage(hDialog, IDC_PROC_ENABLE, BM_SETCHECK,
                                                          BST_CHECKED, 0);
          if (iCtl_Out_PostShow)
             SendDlgItemMessage(hDialog, IDC_PROC_SHOW,   BM_SETCHECK,
                                                          BST_CHECKED, 0);
          if (iCtl_Out_PostQuote)
             SendDlgItemMessage(hDialog, IDC_QUOTES,      BM_SETCHECK,
                                                          BST_CHECKED, 0);

          ZeroMemory(&szBuffer, sizeof(szBuffer));

          PostProc_Join(hDialog, &szCtl_Out_ProcLine_A[0],
                                 &szCtl_Out_ProcLine_B[0]);
          /*
          if (iPostQuote)
              strcpy(szMaybeQuote, "\"");
          else
              szMaybeQuote[0] = 0;

          sprintf(szBuffer, "%s %s%s%s %s", szTmp_A,          // P_Cmd,
                                    szMaybeQuote, szOutput, szMaybeQuote,
                                            szTmp_B           // P_Qual
                                            );

          SetDlgItemText(hDialog, IDC_POST_LINE, szBuffer);
          */


          ShowWindow(hDialog, SW_SHOW);

          return true;


     case WM_COMMAND:
        switch (LOWORD(wParam))
        {

          case IDC_QUOTES:
               iPostQuote = (SendDlgItemMessage(hDialog,
                            IDC_QUOTES, BM_GETCHECK, 1, 0) == BST_CHECKED);

               PostProc_Join(hDialog, &szTmp_A[0], &szTmp_B[0]);
               break;

          case IDC_PROC_LINE_A:
               GetDlgItemText(hDialog, IDC_PROC_LINE_A, szTmp_A,
                                                 sizeof(szTmp_A));
               PostProc_Join(hDialog, &szTmp_A[0], &szTmp_B[0]);

               break;

          case IDC_PROC_LINE_B:
               GetDlgItemText(hDialog, IDC_PROC_LINE_B, szTmp_B,
                                                 sizeof(szTmp_B));

               PostProc_Join(hDialog, &szTmp_A[0], &szTmp_B[0]);

               break;


          case IDOK:

               iCtl_Out_PostProc = (SendDlgItemMessage(hDialog,
                       IDC_PROC_ENABLE, BM_GETCHECK, 1, 0) == BST_CHECKED);

               if (iCtl_Out_PostProc)
               {
                  CheckMenuItem(hMenu, IDM_POSTPROC, MF_CHECKED);
               }
               else
               {
                  CheckMenuItem(hMenu, IDM_POSTPROC, MF_UNCHECKED);
               }


               iCtl_Out_PostQuote = (SendDlgItemMessage(hDialog,
                             IDC_QUOTES, BM_GETCHECK, 1, 0) == BST_CHECKED);

               iCtl_Out_PostShow = (SendDlgItemMessage(hDialog,
                          IDC_PROC_SHOW, BM_GETCHECK, 1, 0) == BST_CHECKED);


               GetDlgItemText(hDialog, IDC_PROC_LINE_A,
                                  szCtl_Out_ProcLine_A,
                           sizeof(szCtl_Out_ProcLine_A));
               GetDlgItemText(hDialog, IDC_PROC_LINE_B,
                                  szCtl_Out_ProcLine_B,
                           sizeof(szCtl_Out_ProcLine_B));
               iOut_PostProc_OK = 1;

               iEnough = 2;
               break;




          case IDCANCEL:
               iOut_PostProc_OK = 0;


               iEnough = 2;
               break;

         }
         break;
   }


  // KILL ?
  if (iEnough > 1)
  {
      //DestroyWindow(hDialog);
      EndDialog(hDialog, iEnough);
      //hPostDlg = NULL;
      iEnough = 1;
  }

  return iEnough;

}


//-----------------

int PlugName_Test(HWND hDialog, char *P_PlugName)
{

  int iRC;

  PLUGIN TestPLug;
  FARPROC lpRC;
  // One day, this module will CHECK
  //  that the named DLL actually exists.

  strcpy(TestPLug.szPathFile, P_PlugName);

  lpRC = Plug81_Load_DLL(&TestPLug.hDll,
                         &TestPLug.fpInit,
                         &TestPLug.szPathFile[0]); // TODO: do on app init only!

  if (! TestPLug.hDll || !lpRC)
  {
    iRC = 0x6969;
    sprintf(szBuffer, "NOT FOUND: %s", TestPLug.szPathFile);
  }
  else
  {
    iRC = 0;
    szBuffer[0] = 0;
    Plug89_Free_DLL(&TestPLug.hDll);
  }

  SetDlgItemText(hDialog, IDX_MSG, szBuffer);
  

  return iRC;
}





//-----------------

int PlugValidate(HWND hDialog, 
                 UINT P_EnableFldId, 
                 UINT P_NameFldId)
{
  int iRC;

  iRC = 0;

  if (SendDlgItemMessage(hDialog,
                    P_EnableFldId, BM_GETCHECK, 1, 0) == BST_CHECKED)
  {
      GetDlgItemText(hDialog, P_NameFldId, szTMPname, sizeof(szTMPname));
      if (szTMPname[0])
         iRC = PlugName_Test(hDialog, &szTMPname[0]);
  }

  return iRC;
}


//----------------------------------------------------------------
LRESULT CALLBACK ExitCtl_Dialog(HWND hDialog, UINT message,
                                WPARAM wParam, LPARAM lParam)
{
  int iEnough, iRC;

  iEnough = 0;

  switch (message)
  {
     case WM_INITDIALOG:

          SetDlgItemText(hDialog, IDX_RENAME_NAME, szRenamePlugIn_Name);
          szTmp32[0] = cRenamePlugIn_MultiMode;
          szTmp32[1] = cRenamePlugIn_AsyncMode;
          szTmp32[2] = 0;
          SetDlgItemText(hDialog, IDX_RENAME_MODE, szTmp32);

          if (PlugFileRename.iActive)
             SendDlgItemMessage(hDialog, IDX_RENAME_ENABLE, BM_SETCHECK,
                                                          BST_CHECKED, 0);


          SetDlgItemText(hDialog, IDX_FILELIST_NAME, szFileListPlugIn_Name);
          szTmp32[0] = cFileListPlugIn_MultiMode;
          szTmp32[1] = cFileListPlugIn_AsyncMode;
          szTmp32[2] = 0;
          SetDlgItemText(hDialog, IDX_FILELIST_MODE, szTmp32);

          if (PlugFileList.iActive)
             SendDlgItemMessage(hDialog, IDX_FILELIST_ENABLE, BM_SETCHECK,
                                                          BST_CHECKED, 0);

          ShowWindow(hDialog, SW_SHOW);

          return true;


     case WM_COMMAND:
        switch (LOWORD(wParam))
        {

          case IDX_RENAME_ENABLE:
                 PlugValidate(hDialog,  IDX_RENAME_ENABLE, IDX_RENAME_NAME);
                 break;

          case IDX_FILELIST_ENABLE:
                 PlugValidate(hDialog,  IDX_FILELIST_ENABLE, IDX_FILELIST_NAME);
                 break;


          //case IDX_RENAME_NAME:
          //     GetDlgItemText(hDialog, IDX_RENAME_NAME, szTmp80,
          //                                       sizeof(szTmp80));
          //     iRC = PlugName_Test(hDialog); //, &szTmp80[0], &szTmp32[0]);
          //     break;


          case IDX_RENAME_MODE:
               GetDlgItemText(hDialog, IDX_RENAME_MODE, szTmp80,
                                                 sizeof(szTmp80));
               if (szTmp80[0] == 'S'
               ||  szTmp80[0] == 'M')
               {
                   cRenamePlugIn_MultiMode = szTmp80[0];
               }
               else
               {
                   SetDlgItemText(hDialog, IDX_MSG, "Need: M=Multi-File, S=Single-File");
               }

               if (szTmp80[1] <= ' '
               ||  szTmp80[1] == 'A')
               {
                   cRenamePlugIn_AsyncMode = szTmp80[1];
               }
               else
               {
                   SetDlgItemText(hDialog, IDX_MSG, "Char2: A=Async blank=Synchronous");
               }
               break;



          //case IDX_FILELIST_NAME:
          //     GetDlgItemText(hDialog, IDX_FILELIST_NAME, szTmp80,
          //                                         sizeof(szTmp80));
          //     iRC = PlugName_Test(hDialog); //, &szTmp80[0], &szTmp32[0]);
          //     break;


          case IDX_FILELIST_MODE:
               GetDlgItemText(hDialog, IDX_FILELIST_MODE, szTmp80,
                                                 sizeof(szTmp80));
               if (szTmp80[0] == 'S'
               ||  szTmp80[0] == 'M')
               {
                   cFileListPlugIn_MultiMode = szTmp80[0];
               }
               else
               {
                   SetDlgItemText(hDialog, IDX_MSG, "Need: M=Multi-File, S=Single-File");
               }

               if (szTmp80[1] <= ' '
               ||  szTmp80[1] == 'A')
               {
                   cFileListPlugIn_MultiMode = szTmp80[1];
               }
               else
               {
                   SetDlgItemText(hDialog, IDX_MSG, "Char2: A=Async blank=Synchronous");
               }

               break;





          case IDOK:

               // Store all the new details for RENAME EXIT

               if (SendDlgItemMessage(hDialog,
                         IDX_RENAME_ENABLE, BM_GETCHECK, 1, 0) == BST_CHECKED)
               {
                   GetDlgItemText(hDialog, IDX_RENAME_NAME, szTmp80,
                                                     sizeof(szTmp80));
                   iRC = PlugName_Test(hDialog, &szTmp80[0]);
                   if (iRC)
                     break;

                   PlugFileRename.iActive = true;
               }
               else
                  PlugFileRename.iActive = false;

               GetDlgItemText(hDialog, IDX_RENAME_NAME, szRenamePlugIn_Name,
                                                 sizeof(szRenamePlugIn_Name));

               GetDlgItemText(hDialog, IDX_RENAME_MODE, szTmp80,
                                                 sizeof(szTmp80));
               cRenamePlugIn_MultiMode = szTmp80[0];
               cRenamePlugIn_AsyncMode = szTmp80[1];

               


               // Store all the new details for FILELIST EXIT
               if (SendDlgItemMessage(hDialog,
                         IDX_FILELIST_ENABLE, BM_GETCHECK, 1, 0) == BST_CHECKED)
               {
                   GetDlgItemText(hDialog, IDX_FILELIST_NAME, szTmp80,
                                                     sizeof(szTmp80));
                   iRC = PlugName_Test(hDialog, &szTmp80[0]);
                   if (iRC)
                     break;

                   PlugFileList.iActive = true;
               }
               else
                  PlugFileList.iActive = false;

               GetDlgItemText(hDialog, IDX_FILELIST_MODE, szTmp80,
                                                 sizeof(szTmp80));
               cFileListPlugIn_MultiMode = szTmp80[0];
               cFileListPlugIn_AsyncMode = szTmp80[1];

               GetDlgItemText(hDialog, IDX_FILELIST_NAME, szFileListPlugIn_Name,
                                                 sizeof(szFileListPlugIn_Name));
               
               
               iEnough = 2;
               break;




          case IDCANCEL:
               iOut_PostProc_OK = 0;


               iEnough = 2;
               break;

         }
         break;
   }


  // KILL ?
  if (iEnough > 1)
  {
      //DestroyWindow(hDialog);
      EndDialog(hDialog, iEnough);
      //hExitDlg = NULL;
      iEnough = 1;
  }

  return iEnough;

}



//----------------------------------------------
// Returns address of Init proc in nominated DLL.

FARPROC Plug81_Load_DLL(HMODULE *lhliby, FARPROC *fpInit,
                        char *P_Name) 
{
  FARPROC lpRC;
  //int iLen;
  //char *ext;

  // Module may be in same folder as Mpg2Cut2
  if (*P_Name    == '.'
  &&   P_Name[1] == '\\') 
  {
    strcpy(szTMPname, szLOAD_Path);
    strcat(szTMPname, P_Name+2);
    //strcpy(P_Name, szTMPname);
  }
  else
    strcpy(szTMPname, P_Name);



  *lhliby=LoadLibrary(szTMPname); 

  if (!*lhliby)
  {
     Msg_LastError("Load: ", 0x6969, 'b');
     sprintf(szMsgTxt, "%s NOT LOADED", szTMPname) ;
     DSP1_Main_MSG(1,0);
     lpRC = 0; // 0x6969;
  }
  else
  {

    *fpInit = GetProcAddress(*lhliby,"Init");
    lpRC = /*(pfnFBInit)*/ *fpInit; // GetProcAddress(*lhliby,"Init");
    if (!lpRC || lpRC == NULL) // FBInit
    {
       Msg_LastError("Init: ", 0x6969, 'b');
       sprintf(szMsgTxt, "%s NO INIT", szTMPname) ;
       DSP1_Main_MSG(1,0);
       //lpRC = 0; //0x6969;
    }
  }

  return lpRC;
}




//----------------------------------------------
// 

int Plug89_Free_DLL(HMODULE *lhliby) 
{
  int iRC;
  FARPROC lpRC;

  //void *FBFinal;

  if (!*lhliby)
  {
    if (DBGflag)
    {
       Msg_LastError("Free: ", 0x6969, 'b');
       sprintf(szMsgTxt, "%s NOT LOADED", szBuffer) ;
       DSP1_Main_MSG(1,0);
    }
    iRC = 0; // 0x6969;
  }
  else
  {

    lpRC = /*(pfnFBInit)*/ GetProcAddress(*lhliby,"Final");
    if (!lpRC || lpRC == NULL) // FBInit
    {
      if (DBGflag)
      {
         Msg_LastError("Init: ", 0x6969, 'b');
         sprintf(szMsgTxt, "%s NO FINAL", szBuffer) ;
         DSP1_Main_MSG(1,0);
      }
       iRC = 0; //0x6969;
    }
    else
      iRC = lpRC(); //FBFinal();
  }

  FreeLibrary(*lhliby);
  *lhliby = NULL;

  return iRC;
}




// EXTERNAL ACTIONS

const char *lpExtAct_KEYNAME[] =
  {
      &"SOFTWARE\\Gabest\\Media Player Classic",
      &"Software\\Microsoft\\Multimedia\\MPlayer2",
      &"Software\\Microsoft\\Multimedia\\WMPlayer",
      &"Software\\Creative Tech\\Creative PlayCenter",
      &"Software\\CyberLink\\PowerDVD",
      &"Software\\VideoLAN\\VLC",
      0,
      0
  };
   
  const char *lpExtAct_VARNAME[] =
  {
       &"ExePath",
       &"Player.Path",
       &"Player.Path",
       &"Path",
       &"InstallPath",
       &"InstallDir",
       0,
       0
  };

   
  const char *lpExtAct_AddOn[] =
  {
       &"",
       &"",
       &"",
       &"\\CTPlay.exe",
       &"\\PowerDVD.exe",
       &"\\VLC.exe",
       &"Mpg2RPT.bat",    // &"bbinfo2.exe",    
       0
  };

   
  const char *lpExtAct_BEFORE[] =
  {
       &"\"",
       &"/play \"",
       &"/play \"",
       &"\"",
       &"\"",
       &"\"",
       &"\"",
       &"",
       0
  };

  const char *lpExtAct_END[] =
  {
       &"\"",
       &"\"",
       &"\"",
       &"\"",
       &"\"",
       &"\"",
       &"\"  0", //  >bbInfo2.RPT",
       0
  };

   /*
  char *lpEXT_ACT_BUFFER[] =
  {
       &szMediaPlayerClassic[0],
       &szWinMediaPlayer2[0],
       &szWinMediaPlayer[0],
       &szCreativePlayCtr[0],
       0
  };
  */
  
  const int iExtAct_MENUNAME[] =
  {
       IDM_ACT_MEDIA_PLAYER_CLASSIC,
       IDM_ACT_WMP2,
       IDM_ACT_WMP,
       IDM_ACT_CREATIVE,
       IDM_ACT_POWERDVD,
       IDM_ACT_VLC,
       IDM_ACT_BBINFO2,
       0
  };
   


void P9_EXT_ACT(int P_EXT_ACT_NO)
{

  char szFolder[_MAX_PATH], *lpDelim, *lpFolder;
  char szCmdLine[_MAX_PATH*4];

  szExtAct_Path[0] = 0;
  if (szExtAct_PathType[P_EXT_ACT_NO] == '*')
  // *lpEXT_ACT_BUFFER[P_EXT_ACT_NO] == '*')
      RegGet_External_Path(P_EXT_ACT_NO);
  else
  if (szExtAct_PathType[P_EXT_ACT_NO] == '=')
  {
      strcpy(szExtAct_Path, szINI_Path);
      lpDelim = strrchr(szExtAct_Path, '\\') + 1;
                       // *lpDelim = 0;
      strcpy(lpDelim,  // strcat (szExtAct_Path, 
             lpExtAct_AddOn[P_EXT_ACT_NO]);

  }



  if (File_Limit  && szExtAct_Path[0])
  {
   
      sprintf(szCmdLine,"%s%s%s", lpExtAct_BEFORE[P_EXT_ACT_NO],
                                  File_Name[File_Ctr],
                                  lpExtAct_END[P_EXT_ACT_NO]);

      sprintf(szMsgTxt, "X: %s",  szCmdLine); // Show what is happening in case something goes wrong
      DSP1_Main_MSG(1,0);

      if (DDOverlay_Flag)  // && iCtl_Ovl_Release)
          D300_FREE_Overlay();

      strcpy(szFolder, File_Name[File_Ctr]);
      lpDelim = lpLastSlash(szFolder);
      if (! lpDelim)
            lpDelim = &szFolder[0];
      *lpDelim = 0;

      if (szExtAct_PathType[P_EXT_ACT_NO] == '=')
         lpFolder = &szLOAD_Path[0];
      else
         lpFolder = &szFolder[0];

      ShellExecute(NULL, "open",   
                          &szExtAct_Path[0], 
                          szCmdLine,  
                          lpFolder,     // NULL, 
                          SW_SHOWNORMAL);
  }
  else
      MessageBeep(MB_OK);


}


//--------------------------
void RegGet_External_Path(int P_EXT_ACT_NO)
{
  int iRC;
  
	HKEY key;
	DWORD disposition;
  unsigned long int uType, uLen, uTmp1;

  // Interrogate Media Player Classic as an External Action

  szExtAct_Path[0] = 0;
  // *lpEXT_ACT_BUFFER[P_EXT_ACT_NO] = 0;
  
  
  iRC = RegCreateKeyEx(HKEY_LOCAL_MACHINE, 
                    lpExtAct_KEYNAME[P_EXT_ACT_NO],       0, "", 0, 
                    KEY_QUERY_VALUE,      NULL, &key, &disposition);

  if (iRC != ERROR_SUCCESS)
  {
      sprintf(szBuffer, "Not Found in System Register.  RC=%d\n\n%s\n%s", iRC, 
                              lpExtAct_KEYNAME[P_EXT_ACT_NO], "") ;
      MessageBox(hWnd_MAIN, szBuffer, szAppName, MB_OK);
  }
  else
  {
     uLen = _MAX_PATH;
     iRC = RegQueryValueEx(key, 
                     lpExtAct_VARNAME[P_EXT_ACT_NO], NULL,       &uType,
           (LPBYTE)  &szExtAct_Path[0],             
       /*  (LPBYTE)  lpEXT_ACT_BUFFER[P_EXT_ACT_NO],  */
                     &uLen);
                       
     if (iRC != ERROR_SUCCESS)
     {
        sprintf(szBuffer, "Not Found in System Register.  RC=%d\n\n%s\n%s", iRC, 
                                lpExtAct_KEYNAME[P_EXT_ACT_NO], 
                                lpExtAct_VARNAME[P_EXT_ACT_NO]) ;
        MessageBox(hWnd_MAIN, szBuffer, szAppName, MB_OK);
     }
     
     if (szExtAct_Path[0])      // *lpEXT_ACT_BUFFER[P_EXT_ACT_NO])
     {
        uTmp1 = MF_ENABLED;
        strcat (szExtAct_Path, 
                lpExtAct_AddOn[P_EXT_ACT_NO]);
     }
     else
        uTmp1 = MF_GRAYED;

     EnableMenuItem(hMenu, iExtAct_MENUNAME[P_EXT_ACT_NO], uTmp1);

     RegCloseKey(key);
  } 
	
}

/*

  
void  Reg_ExternalActions()
{
  int iLoop;

  iLoop = 0;
  while (iExtAct_MENUNAME[iLoop])
  {
     RegGet_External_Path(iLoop);
     iLoop++;
  }
}
*/



