
//--------------------------------------------------------------
// Input file Opening and associated routines 
//    + a little bit of Output file stuff too
 
#include "TXT.h"
#include "global.h"
#include "errno.h"
#include <fcntl.h>

#include "Audio.h"
#include "Wave_out.h"

#define true  1
#define false 0

int  F200_Drop_TRY(char);
void F350_Drop_Queries(WPARAM, char);
int  F400_Input_Bld();
int  F550_FileTrojan_Chk();

unsigned uHeader[8];




//---------------------------------------------------------------

int F100_IN_OPEN(char P_Act, int P_Trunc)
{
  char cW_Action, cW_Act1, szTitle[64], cTst;
  int int_RC, w_Trunc;
  //__int64 i64Len1;
  char szFolder[_MAX_PATH], *lsFile;

  MParse.Stop_Flag = true;
  MParse.iMultiRqst = 0;
  iEDL_Reload_Flag = 0;

  if (File_Limit)
      cW_Action = P_Act;
  else
      cW_Action = 'o';
  cW_Act1 = cW_Action;

  //iVolume_PREV_Cat = iVol_Boost_Cat;// VOL302_Maybe_Reset();

  switch (cW_Action)
  {
    case 'a':
      if (!File_Limit)
         cW_Action = 'o';
      strcpy(szTitle, "ADD Mpeg2 File"); 
      break;

    case 'G':
      strcpy(szTitle, "Turn GARBAGE into Mpeg2 File"); 
      break;

    default:
      strcpy(szTitle, "OPEN Mpeg2 File"); 
  }

  cTst = cW_Action;

  // DialogBox(hInst, (LPCTSTR)IDD_FILELIST, hWnd, (DLGPROC)F700_Video_List);



  lsFile = lpLastSlash(szInput);
  if (lsFile)
      lsFile++;
  else
      lsFile = &szInput[0];


  // Optionally suppress the file selection list 

  if (strlen(szInput) < 11)
      w_Trunc = 1;
  else
  {
      w_Trunc = P_Trunc;
      if (w_Trunc <= 0
      &&  *(DWORD*)(lsFile)   == '_STV' //  VTS_ prefix
      &&            lsFile[7] <= '1' ) 
      {
         lsFile[7]    = '*';
         if ( *(DWORD*)(&lsFile[8])  == 'OFI.'    //  .IFO extension
         ||   *(DWORD*)(&lsFile[8])  == 'PUB.')   //  .BUP extension
         {
              *(DWORD*)(&lsFile[8])   = 'BOV.';   //  .VOB instead
         }

         iParmConfirm = 1;
         w_Trunc = 15;
      }
  }

  memcpy(lpTmp16K, &szInput, sizeof(szInput));

  if 
  (!iParmConfirm   // passed via parm area from another program

  ||  (iCtl_Drv_Segments &&                  // Drive segment level access
        (   !stricmp(&szInput[0]+1, &":\\")  // Drive Root Folder
         || !stricmp(&szInput[0]+1, &":")    // Drive name alone
        )
      )
  )

  {
     ofn.nFileOffset = 1;
     int_RC = 1;
  }
  else
    int_RC = X800_PopFileDlg(lpTmp16K, hWnd_MAIN, INPUT_VOB, w_Trunc, szTitle) ;

  if (File_Limit >= File_PreLoad)
      iParmConfirm = 1;

  cTst = cW_Action;

  if (!int_RC)
  {
      DSP_Msg_Clear(); // DSP_Blank_Msg_Clean();
      MessageBeep(MB_OK);
  }
  else
  {
    if (cW_Action != 'a') 
    {
      OrgTC.RunFrameNum = -1;
      Ed_Prev_Act  = 0; Ed_Prev_Act2 = 0; 
      iEDL_ctr     = 0;
      szEDLprev[0] = 0;
      process.iOut_Part_Ctr = 0;
    }

    strcpy(szFolder, lpTmp16K);
    lpTmpIx = (char *)( (int)lpTmp16K + ofn.nFileOffset - 1) ;

    if (*lpTmpIx)
      // single file name ?
    {
      strcpy(szInput, szFolder);
      F500_IN_OPEN_TRY(cW_Action);
    }
    else         
      // Multi-File
    {
      
      MParse.iMultiRqst = 1;
      File_New_From = File_Limit;
      lpTmpIx++;
      while (*lpTmpIx) // repeat until null file name found;
      {
        // Scan Tmp area to build next szInput File Name
        lpInputIx = (char *) &szInput; 
        lpFromIx  = (char *) &szFolder;
        F400_Input_Bld();
        lpFromIx = lpTmpIx;
        *lpInputIx++ = '\\';
        F400_Input_Bld();
        lpTmpIx  = lpFromIx;

        // Try opening it
        int_RC = F500_IN_OPEN_TRY(cW_Action);
        if (int_RC > 0)
            cW_Action = 'a';           // After 1st file, rest are appended
    //  else
    //      *lpTmpIx  = 0;

        if (File_Limit > (MAX_FILE_NUMBER - 5))
        {
            strcpy(szMsgTxt, "TOO MANY INPUT FILES !");
            break;
        }
      } //END_WHILE NOT NULL
      
      F850_SORT_NAMES(iCtl_FileSortSeq);
      if (cW_Action != 'a')
      { // Does this bit really achieve anything ?
//        D500_ResizeMainWindow(iAspect_Width, iAspect_Height, 1);//Clip_Width, Clip_Height); 
        SetForegroundWindow(hWnd_MAIN);

      }

    }// END_IF  Multi-File

    F150_File_Begin(cW_Action);  // cW_Act1);
    if (File_Limit)
    {
      Tick_CLEAR();
      VGA_GetSize();
      if ( ! iEDL_Reload_Flag) // && P_Act == 'o')
      {
         C100_Clip_DEFAULT(cW_Act1); // cW_Action);
      }
      else
      {
        T590_Trackbar_SEL();
      }
    }

  } // END File Name Entered

return int_RC ;
}




void F150_File_Begin(char P_Act)
{

  // Roll changes out
  DSP5_Main_FILE_INFO();
  MParse.iMultiRqst = 0;

  if (P_Act == 'a' || File_Limit > 1)
  {
      sprintf(szMsgTxt, FILE_TOTAL_COUNT, File_Limit);
      iMsgLife=3;
      DSP2_Main_SEL_INFO(1);
  }


  if (File_Limit)
  {
    EnableMenuItem(hMenu, IDM_SAVE, MF_ENABLED);
    T100_Upd_Posn_Info(0); 

    iKick.Action = ACTION_INIT;
    if (P_Act != 'a')
    {
        if (! iEDL_Reload_Flag)
        {
          process.CurrFile = File_Ctr = File_New_From ;
          process.CurrLoc  = 0; // process.CurrBlk = 0 ;
          process.startLoc = process.LocJump = 0 ;
        }
        process.GoodLoc1 = MAXINT64;
        //MParse.SeqHdr_Found_Flag = 0;  <== PLANNED - Check downstream effect on Buffer flags

        if (File_PreLoad <= 0)   // Not Parm2Clip mode 
           MPEG_processKick(); 
    }
    
    else
    {
         if (MParse.SeqHdr_Found_Flag)
         {
             if (iShowVideo_Flag)
             {
                if (MParse.iColorMode == STORE_RGB24)
                   RenderRGB24();
                else
                if (DDOverlay_Flag)
                    RenderYUY2(1);
             }
         }
    }
    

    DSP5_Main_FILE_INFO();
   }

}




//---------------------------------------------------------------

int F400_Input_Bld()
{
  int iStatus, iMax;
  iStatus = 1;
  iMax = _MAX_PATH; 

  while (iStatus && iMax)
  {
      iMax--;
      *lpInputIx = *lpFromIx++;
      if (*lpInputIx) 
           lpInputIx++;
      else
          iStatus = 0;
  } // endwhile

  if ( !iMax)
    iStatus = -1;

  return iStatus ;
}



//void Mpeg_READ();



//------------------------------------
// Try to open a given input file name

int F500_IN_OPEN_TRY(char cP_Act)
{
  int iResult;
  char *ext;

  iResult = -1;  // Default = Fail

  // Analyse Extension

  ext =   strrchr(szInput, '.');
  if (ext) strncpy(szInExt,ext+1,8);
  else     strcpy(szInExt,"");

  if (!stricmp(szInExt,"EDL") )
  {
     iEDL_Reload_Flag = 1;
     strcpy(szEDLname, szInput);
     C800_Clip_FILE(LOAD_EDL, 0, cP_Act); // 0);
  }
  else
  if (File_Limit > (MAX_FILE_NUMBER - 5))
  {
      strcpy(szMsgTxt, "TOO MANY INPUT FILES !");
  }
  else
      iResult = F505_IN_OPEN_TST(cP_Act);


 return iResult;
}




// Check for duplicate DSN
int F503_Dup_Name_TST(char *P_Name, const char *P_Msg, const unsigned int P_Buttons)
{
  int iRC, iSearch;

  // Check for duplicate DSN
  
  iSearch = 0;
  while (iSearch < File_Limit)
  {
     if (! stricmp(P_Name, &File_Name[iSearch][0]))
     {
          sprintf(szBuffer, P_Msg, P_Name);
          iRC = MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2", P_Buttons);
          if (iRC == 1)
              return -1;           // <========= ESCAPE POINT
          else
              return  1;           // <========= ESCAPE POINT
      }
      iSearch++;
  }

  return 0;

}




int F504_FOLDER_TRY(char *lpOther)
{
  char  *lsBuf_Slash, *lsIn_Slash;
  int iRC;

  iRC = -1; // Default = fail

  strcpy(szBuffer, lpOther);

  lsBuf_Slash  = lpLastSlash(&szBuffer[0]);
  if (lsBuf_Slash >= &szBuffer[0])  // Found a slash or colon ?
  {
          lsBuf_Slash++;
          lsIn_Slash   = lpLastSlash(&szInput[0]);
          if (! lsIn_Slash)
                lsIn_Slash = &szInput[0];
          else
          {
            lsIn_Slash++;
          }

          strcpy (lsBuf_Slash, lsIn_Slash); 
          ZeroMemory(&seqfile, sizeof(seqfile));
          iRC = _findfirst(szBuffer, &seqfile);
  }

  return iRC;

}

  




int F505_IN_OPEN_TST(char cP_Act)
{

  int iRC, iRC2, iResult, iLen, iTmp1;
  char W_Action;
  char /*seq,*/ *ext; // , *lsBuf_Slash, *lsIn_Slash;
  //unsigned char uByte1, uByte2;

  iResult = -1;

  W_Action   = cP_Act;

  lpFName = lpLastSlash(&szInput[0]);
  if (lpFName)
      lpFName++;
  else
      lpFName = &szInput[0];

  if (W_Action == 'a')
  {
      // Check for duplicate DSN
      iRC = F503_Dup_Name_TST(&szInput[0], 
                       &FILE_DUP_SKIP_QRY, MB_YESNO);
      if (iRC < 0)
          return -1;           // <========= ESCAPE POINT
  }
  else
  {

    if (DBGflag)
    {
        sprintf(szBuffer,"MSK=x%08X", SCR_MASK_0);
        DBGout(szBuffer);
    }
              
  }

  // Progress messages in case a lot of files to be processed together
  if (W_Action == 'a')
  {
      iTmp1 = sprintf(szTemp, "File#%d  ", (File_Limit+1));
      TextOut(hDC,  iSelMsgX, iMsgPosY,
                    //iTimeX, iTimeY, 
                    szTemp, iTmp1);
      UpdateWindow(hWnd_MAIN);
  }

  ZeroMemory(&seqfile, sizeof(seqfile));
  iRC = _findfirst(szInput, &seqfile) ;

  // If not found then try the EDL folder, or most recent input folder
  if(iRC == -1L)
  {
      iRC = F504_FOLDER_TRY(&szEDLname[0]);

      if (iRC != -1L)
          strcpy(szInput, szBuffer);
      else
      {
        if (szInFolder[0])
        {
            iRC = F504_FOLDER_TRY(szInFolder);
            if (iRC != -1L)
                strcpy(szInput, szBuffer);
        }
      }
          
  }

  if(iRC == -1L)
  {
        ERRMSG_File("MpegIn", 'o', errno,  szInput, 0, 1504) ;
        iRC = -1;
  }
  else
  {
      strncpy(szInExt3_lowercase, szInExt,3);

      // Force Lowercase
      if (szInExt3_lowercase[0] >= 'A' && szInExt3_lowercase[0] <= 'Z')
          szInExt3_lowercase[0] |=  0x20;
      if (szInExt3_lowercase[1] >= 'A' && szInExt3_lowercase[1] <= 'Z')
          szInExt3_lowercase[1] |=  0x20;
      if (szInExt3_lowercase[2] >= 'A' && szInExt3_lowercase[2] <= 'Z')
          szInExt3_lowercase[2] |=  0x20;

      if (szInExt3_lowercase[0] == 'v')
          iIn_VOB = 1;
      else
          iIn_VOB = 0;

      strcpy(szInFolder, szInput); 
      ext = lpLastSlash(szInFolder); //strrchr(szInFolder, '\\');
      if (!ext)
           ext = &szInFolder[0];
      else
           ext++;

      strcpy(szName, ext);
      //strncat(szName, ext+1, strlen(szInput)
      //                   -(int)(ext-szInput[0]));

      *ext = 0;
   


      // close all open files, BUT NOT if we are appending !
      if (W_Action != 'a')
      {
        while (File_Limit)
        {
           _close(FileDCB[File_Limit-1]);
           File_Limit--;
        }
        File_Final = File_Limit;
        TextOut(hDC, iTimeX, iTimeY, BLANK44, 40) ;
        //iOut_KeepFileDate = iCtl_Out_KeepFileDate;
      }


      // Clear out read ahead buffers
      GetBlk_RdAHD_RESET();

      if (! File_Limit)
      {
          // Reset Decoder file analysis state
          MPEG_File_Reset();
      }

      //  VOB  extension  triggers  special  VOB  options
      if  (iIn_VOB && !File_Limit)
      {
           iInPS2_Audio      =  0;
           MParse.iVOB_Style =  iCtl_VOB_Style;
      }
      else
      {
           iInPS2_Audio  =  iCtl_Audio_PS2;
           MParse.iVOB_Style  =  0;
      }

      process.Pack_Avg_Size  =  0;
      iBMP_Preview           = iCtl_BMP_Preview;


      strcpy(File_Name[File_Limit], szInput);
      FileDCB[File_Limit] = _open(szInput, _O_RDONLY | _O_BINARY);
      if (FileDCB[File_Limit] < 0)
      {
        ERRMSG_File("MpegIn", 'o', errno,  szInput, 0, 1505) ;
        iRC = -1;
      }
      else
      {
        File_Final = File_Limit;
        File_Limit++;
        
        iTmp1 = _fstati64(FileDCB[File_Final], &TmpStat);
        if (iTmp1)
        {
            ZeroMemory(&File_Date[File_Final], sizeof(File_Date[File_Final]));
            ZeroMemory(&File_Greg[File_Final], sizeof(File_Greg[File_Final]));
        }
        else
        {
           File_Date[File_Final] = TmpStat.st_ctime;
           // seconds elapsed since 00:00:00 GMT, January 1, 1970
           FileDate2Gregorian(&File_Date[File_Final], 
                              &File_Greg[File_Final], &szBuffer, &" ");
        }


        // I wonder if C has a 64 bit version of 
        //      getftime()  and setftime()  ????

        /*
        // ??? WINDOWS FILE HANDLE MISMATCH WITH "C" FILE HANDLE - GRRR ! ARGH !
        iTmp1 = GetFileTime(fileno(FileDCB[File_Final]), 
                         &File_Date[File_Final], //[0],  // address of creation time 
                         NULL, // &File_Date[File_Final][1],  // address of last access time  
                          NULL, // &File_Date[File_Final][2]); // address of last write time 
        if (! iTmp1)
        {
          ZeroMemory(&File_Date[File_Final], sizeof(File_Date[File_Final]));
        }
        */

        // look for Gregorian date suffix on file name
        //  mktime converts gregorian date-time to C julian


        if (W_Action != 'a')
        {
           iEDL_ctr = 0; iEDL_Chg_Flag = 0; 
           iWant_Aud_Format = FORMAT_AUTO;
           ProcessReset("OPN"); //  Release old buffers - Calc new stuff for File Open
           if (MParse.ShowStats_Flag)
                 Stats_Show(true, -1);

        }
        else
           MultiFile_SizeCalc();


        
        //if (File_Final > 0)
        //   process.ByteRateAvg[File_Final] = process.ByteRateAvg[File_Final-1];
        //else
             process.ByteRateAvg[File_Final] = BYTERATE_DEF;// Default ByteRate
          
        //C100_Clip_DEFAULT(cP_Act);

        //File_Ctr = File_Final;
        //Mpeg_READ();

        iLen = Mpeg_READ_Buff(File_Final,  //  _read(FileDCB[File_Final], 
                                (unsigned char *)&uHeader[0],  
                                32,  5059);

        _lseeki64(   FileDCB[File_Final], 0, SEEK_SET);
        MParse.NextLoc = 0;

        if (iLen < 8)
        {
        }
        else
        {
           if( uHeader[0] == 0xBA010000) // PS PACK header
           {
              if (!File_Final)
                  process.iPreamblePackAtStart = 1;
              // Grab SCR from first pack
              memcpy(&cStartSCR[File_Final][0], &uHeader[1], 6);
              process.Mpeg2_Flag = ( cStartSCR[File_Final][0] & 4); 
              cStartSCR[File_Final][0] &= SCR_MASK_0;

              // Grab PS chunk rate
              
              *((unsigned char*)(&iMuxChunkRate)+3) = 0;
              *((unsigned char*)(&iMuxChunkRate)+2) = *((unsigned char*)(&uHeader[2])+2);
              *((unsigned char*)(&iMuxChunkRate)+1) = *((unsigned char*)(&uHeader[2])+3);
              *((unsigned char*)(&iMuxChunkRate)  ) = *((unsigned char*)(&uHeader[2])+4);
              if (process.Mpeg2_Flag)
                 iMuxChunkRate = iMuxChunkRate>>2;
              else
                 iMuxChunkRate = iMuxChunkRate & 0x003FFFFF;

              if (iMuxChunkRate)
                 process.ByteRateAvg[File_Final] = iMuxChunkRate * 50;

              /*
              if (DBGflag)
              {
                sprintf(szBuffer,"SCR=x%08X %s", (cStartSCR[File_Final][0]), File_Name[File_Final]);
                DBGout(szBuffer);
              }
              */
              
              iRC2 = 0;
           }
           else
           {
             iRC2 =  F550_FileTrojan_Chk();
             if (iRC2 >= 0  && !MParse.SystemStream_Flag 
             &&  uHeader[0] != uSEQ_HDR_CODE // Elementary Vid Stream Hdr sentinel
             &&  process.length[File_Final] >= MPEG_SEARCH_BUFSZ)
             {
               sprintf(szBuffer, NO_MPEG_PACK_SEQ_AT_START, szInput);
               if (iSuppressWarnings)
                 strcpy(szMsgTxt, szBuffer);
               else
                 MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - Warning", MB_OK);
             }
             //FillRect(hDC, &crect, hBrush_MASK);

           }
        }

        if (iRC2 >= 0
        &&  process.length[File_Final] < MPEG_SEARCH_BUFSZ)
        {
            sprintf(szBuffer, FILE_TOO_SMALL,
                              process.length[File_Final],
                              szName);
            MessageBox(hWnd_MAIN, szBuffer, Mpg2Cut2_FILE_ERROR,
                                        MB_ICONSTOP | MB_OK);
            _close(FileDCB[File_Final]);

            File_Final--;
            File_Limit--;
        }
        else
        {
           if (iRC2 < 0)
              return iRC2;
           else
           {
             if (File_Final)
                 memcpy(&cStartSCR[File_Final][0], &cStartSCR[File_Final-1][0], 6);
           }

           if (iCtl_Out_DeBlank || iCtl_Out_MixedCase)
              FileNameTidy(&szOutput[0], &szInput[0]);
           else
           {
               strcpy(szOutput, szInput);
           }

           //cStartSCR[File_Final][0] = cStartSCR[File_Final][0] & 0x7F;


           //process.Action = ACTION_INIT;
           //process.CurrFile = File_Ctr = File_Final ;
           //process.CurrLoc  = process.CurrBlk = 0 ;
           //process.startLoc = process.LocJump = 0 ;

            iResult = 1;
           //Sleep(250); 

        } // END file size chk

      } // END OPEN OK

  } // END File Found

  return iResult;
} // END_PROC F505_IN_OPEN_TST



unsigned uDropTotal, uDropIx;



//-----------------------------------------------------------
void F300_DropFilesProc( WPARAM wParam)
{
  int  iRC;
  char W_Action;

  SetForegroundWindow(hWnd_MAIN);

  iEDL_Reload_Flag = 0;

  strcpy(szMsgTxt,FILE_ACCESSING); // "Accessing..."
  DSP1_Main_MSG(0,0);
  iMsgLife = 0;  szMsgTxt[0] = 0;
  UpdateWindow(hWnd_MAIN);


  switch (iCtl_DropAction)
  {
    case 1:
       W_Action = 'a';
       strcpy(szMsgTxt, "Added");
       break;
    case 2:
       W_Action = 'o';
       break;

    default:
      W_Action = 'o';
      if (File_Limit)
      {
        iRC = MessageBox(hWnd_MAIN, FILE_ADD_QRY, // "ADD to current file list ? ",
                                    szAppName, 
                                    MB_YESNOCANCEL );
        if (iRC == IDYES  || iRC == IDOK )  
            W_Action = 'a';
        else
        if (iRC == IDCANCEL)  
            W_Action = 'z';
      }
  } 
         
  uDropTotal = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, 
                                    szInput, sizeof(szInput));
     
  if (uDropTotal > 0 && W_Action != 'z')
      F350_Drop_Queries(wParam, W_Action);

  DragFinish((HDROP)wParam);
  SetForegroundWindow(hWnd_MAIN);

  //iVolume_PREV_Cat = iVol_Boost_Cat;// VOL302_Maybe_Reset();


  if (W_Action != 'z')
  {
    if (! iEDL_Reload_Flag)
    {
      if (W_Action == 'o')
      {
          szEDLprev[0] = 0;
          ProcessReset("DFL"); // Release old buffers - Calc new stuff for Dropped files
      }
    }

    DSP5_Main_FILE_INFO();
    if (File_Limit)
    {
      File_Final = File_Limit - 1; 

      if (! iEDL_Reload_Flag) // &&  W_Action == 'a')
      {
         //DSP5_Main_FILE_INFO();
         C100_Clip_DEFAULT(W_Action);
         Tick_CLEAR();
         //T590_Trackbar_SEL();
      }
      EnableMenuItem(hMenu, IDM_SAVE, MF_ENABLED);

      iKick.Action = ACTION_INIT;
      MPEG_processKick();
      /*
      if (!threadId_MPEG
        || WaitForSingleObject(hThread_MPEG, 0) == WAIT_OBJECT_0)
      {
        hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);
      }
      */

    }
    else
      File_Final = 0;
  }

    
  if (iMsgLife <= 0)
  {
    //  Repair the underlying window info
    TextOut(hDC, 0, iMsgPosY, BLANK44, 12); // Would be better as a Paint
    UpdateWindow(hWnd_MAIN);
  }

  return ;

}



//--------------------------------------------------------------


int FolderExists(LPCSTR szFolderPath)
{
  HANDLE handle;
  WIN32_FIND_DATA findData;

  handle = FindFirstFile(szFolderPath, &findData);
  if (handle!=INVALID_HANDLE_VALUE) 
  {
      FindClose(handle);
      if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
          return 1;
  }
  return 0;
}



//-----------------------------------------------------------
void F350_Drop_Queries( WPARAM wParam, char cP_Action)
{
  int iRC;
  char W_Action;


  WIN32_FIND_DATA FindData;
  HANDLE hFind;
  char szPath[MAX_PATH];
  char szFindPath[MAX_PATH];


  W_Action = cP_Action;

  if (W_Action == 'o') // New project ?
  {
     while (File_Limit) // Close all previous files 
     {
        File_Limit--;
        _close(FileDCB[File_Limit]);
     }
     File_Final = File_Limit = 0;
     OrgTC.RunFrameNum = gopTC.RunFrameNum = -1;
  }

  File_New_From = File_Limit;

  uDropIx = 0;
  while ( uDropIx < uDropTotal )
  {
    DragQueryFile((HDROP)wParam, uDropIx, szPath, sizeof(szPath));
    if (FolderExists(szPath))
    {  // Drop Folder - courtesy of WeWantWideScreen
       lstrcpy(szFindPath,szPath);
       lstrcat(szFindPath,"\\*.mpg");

       hFind=FindFirstFile(szFindPath,&FindData);              
       while (INVALID_HANDLE_VALUE != hFind)
       {
          lstrcpy(szInput,szPath);
          lstrcat(szInput,"\\");
          lstrcat(szInput,FindData.cFileName);
          iRC = F200_Drop_TRY(W_Action);
          if (iRC > 0)
          {
              if (W_Action != 'a')
              {
                  iEDL_ctr = 0; iEDL_Chg_Flag = 0;
              }
          }
                        
          W_Action = 'a';

          if (!FindNextFile(hFind,&FindData)) 
          {
              FindClose(hFind);
              hFind = INVALID_HANDLE_VALUE;
          }

       } // ENDWHILE NOT INVALID

    } // ENDIF FOLDER
    else
    {
       strcpy(szInput, szPath);
       iRC = F200_Drop_TRY(W_Action);
       if (iRC > 0)
       {
         if (W_Action != 'a')
         {
            iEDL_ctr = 0; iEDL_Chg_Flag = 0;
         }
       }
    } // ENDELSE not a Folder

    uDropIx++;
    W_Action = 'a';
  }

  if (File_Limit > File_New_From)
  {
      F850_SORT_NAMES(iCtl_FileSortSeq);
      strcpy(szOutput, szInput);
  }
}




//-----------------------------------------------------------
// Try to open a given input file name
int F200_Drop_TRY(char P_Act)
{
  char /*seq,*/ *ext;
  int iOkay, iRC;

  iOkay = 0;
  ext = strrchr(szInput, '.');
  if (ext==NULL  ||  
          (_strnicmp(ext, ".m2v",  4)  && _strnicmp(ext, ".m2p", 4)
        && _strnicmp(ext, ".mpv",  4)  && _strnicmp(ext, ".mpg", 4)
        && _strnicmp(ext, ".get",  4)  && _strnicmp(ext, ".par", 4)
        && _strnicmp(ext, ".mpeg", 5)  && _strnicmp(ext, ".vob", 4)
        && _strnicmp(ext, ".m2t",  4)  && _strnicmp(ext, ".pva", 4)
        && _strnicmp(ext, ".ts",   3)
        && _strnicmp(ext, ".EVO",  4)  && _strnicmp(ext, ".m1v", 4)
        && _strnicmp(ext, ".EDL",  4)  && _strnicmp(ext, ".mod", 4) 
        &&  (*ext < '0' || *ext > '9') 
          ))
  {
      sprintf(szBuffer, FILE_XTN_QRY, 
                         ext, szInput);
      iRC = MessageBox(hWnd_MAIN, szBuffer, szAppName, 
                             MB_OKCANCEL );

  }
  else
  {
      iRC = IDOK;
  } //end EXT CHK

  if (iRC == IDYES  || iRC == IDOK )  
      iOkay = F500_IN_OPEN_TRY(P_Act);
  else
     iOkay = 0;

  return iOkay;

}



//---------------------------------------------
// Sort ALL file names into ascending order

void F800_SORT_ALL(int P_Sort_Type)
{

  Mpeg_Stop_Rqst();

  if (iCtl_FileSortSeq < 0)
    return;

  File_New_From = 0;
  F850_SORT_NAMES(P_Sort_Type);

  ZeroMemory(&OrgTC, sizeof(OrgTC));
  OrgTC.RunFrameNum = -1;

  File_Ctr = 0;
  _lseeki64(FileDCB[0], 0, SEEK_SET);
  MParse.NextLoc = 0;

  VideoList_MODE = 'o';
  strcpy(VideoList_Title, "Open");
  DialogBox(hInst, (LPCTSTR)IDD_FILELIST, hWnd_MAIN, (DLGPROC)F700_Video_List);

}



//---------------------------------------------
// Sort new file names into ascending order
// Starting from File_New_From up to File_Final (File_Limit-1)

void F850_SORT_NAMES(int P_Sort_Type)
{
  int iFile1, iScan_FROM, iScan_CURR, iSortType, iTST;
  char cSCR[6];
  __int64 i64Len;
  
  iSortType = P_Sort_Type;
  // Sort into ascending name or time order
  if (threadId_MPEG)
  {
      iKick.Action = ACTION_NOTHING;
      MPEG_processKick();
      WaitForSingleObject(hThread_MPEG, 0);
  }

  // Bubble Sort 

  File_Final = File_Limit - 1;
  iScan_FROM =  File_New_From ;
  while (iScan_FROM < File_Final)
  {
    iScan_CURR  = iScan_FROM + 1;

    while (iScan_CURR <= File_Final)
    {
      iTST = 0;

      
      if (iSortType == 2)  // Sort by File Creation Date
      {
        // Allow for wrap around of time stamp
        if ( *(IFILEDATE*)(&File_Date[iScan_FROM])  
           < *(IFILEDATE*)(&File_Date[iScan_CURR]) ) 
            iTST = -1;
        else
        if ( *(IFILEDATE*)(&File_Date[iScan_FROM])  
           > *(IFILEDATE*)(&File_Date[iScan_CURR]) ) 
            iTST =  1;
      }
      


      if (iSortType == 1  // Sort by Mpeg SCR
      || (iTST == 0 && iSortType == 2))
      {
        // Allow for wrap around of time stamp
        if ( *(unsigned char*)(&cStartSCR[iScan_FROM][0]) >= 0x1A  // Approaching 33bit signed MAXINT aka 32bit SCR wrap
        &&   *(unsigned char*)(&cStartSCR[iScan_CURR][0]) <= 0x0B) // ~3 Hrs after  "wrap"
            iTST = -1;
        else
        if ( *(unsigned char*)(&cStartSCR[iScan_FROM][0]) <= 0x0B  // ~3 Hrs after  "wrap"
        &&   *(unsigned char*)(&cStartSCR[iScan_CURR][0]) >= 0x1A) // Approaching 33bit signed MAXINT aka 32bit SCR wrap
            iTST =  1;
        else
            iTST = memcmp(&cStartSCR[iScan_FROM][0], 
                          &cStartSCR[iScan_CURR][0],6);
      }


      if ( ! iTST)  // Allow for same SCR (maybe should really use PTS not SCR)
          iTST = stricmp(File_Name[iScan_FROM], 
                         File_Name[iScan_CURR]);

      /*
      if (DBGflag)
      {
        sprintf(szBuffer, "TST=%d x%08X %s\n       x%08X %s", 
                           iTST,
                           *(DWORD*)(&cStartSCR[iScan_FROM][0]),
                                      File_Name[iScan_FROM], 
                           *(DWORD*)(&cStartSCR[iScan_CURR][0]),
                                      File_Name[iScan_CURR]);
        DBGout(szBuffer);
      }
      */


      if (iTST > 0)
      {   // SWAP details of the 2 files
           strcpy(szInput, File_Name[iScan_FROM]);
           strcpy(File_Name[iScan_FROM],
                  File_Name[iScan_CURR]);
           strcpy(File_Name[iScan_CURR], szInput);

           iFile1 = FileDCB[iScan_FROM] ;   
           FileDCB[iScan_FROM] = FileDCB[iScan_CURR] ;
           FileDCB[iScan_CURR] = iFile1;

          memcpy(&cSCR[0], &cStartSCR[iScan_FROM], 6);
          memcpy(&cStartSCR[iScan_FROM],
                 &cStartSCR[iScan_CURR],  6);
          memcpy(&cStartSCR[iScan_CURR], &cSCR[0], 6);

          
          memcpy(&File_Date[MAX_FILE_NUMBER], &File_Date[iScan_FROM],      sizeof(File_Date[MAX_FILE_NUMBER]));
          memcpy(&File_Date[iScan_FROM],      &File_Date[iScan_CURR],      sizeof(File_Date[MAX_FILE_NUMBER]));
          memcpy(&File_Date[iScan_CURR],      &File_Date[MAX_FILE_NUMBER], sizeof(File_Date[MAX_FILE_NUMBER]));
          

          i64Len = process.length[iScan_FROM];
          process.length[iScan_FROM] = process.length[iScan_CURR]; 
          process.length[iScan_CURR] = i64Len;
      } // end-if

      iScan_CURR++;

   } //end-while
   iScan_FROM++;
  } // END_WHILE


  MultiFile_SizeCalc();  // Reload file lengths and origins
}



//-----------------

void F560_RemoveOtherFiles(int P_Keep)
{
  int iTmp1, iKillCtr;

  iKillCtr = 0;
  File_Ctr = P_Keep;
  process.CurrFile = P_Keep;

  for (iTmp1 = 0; iTmp1 < File_Limit;)
  {
      if (iTmp1 != File_Ctr)
      {
          F570_RemoveFile(iTmp1, 1);
          //if (iTmp1 < P_Keep)
          //    P_Keep--;
          iKillCtr++;
      }
      else
         iTmp1++;
  }

  strcpy(szOutput, File_Name[0]);

  sprintf(szMsgTxt, FILE_EXCLUDE_COUNT, iKillCtr);
  DSP1_Main_MSG(0,0);
  DSP5_Main_FILE_INFO();

}


void F570_RemoveFile(int iKillFile, int P_Close)
{
  int iTmp1, iTmp2;

  if (P_Close)
       _close(FileDCB[iKillFile]);

  // Remove all references to the file to be killed
  C700_Clip_DeReference(iKillFile);

  // Shuffle files to fill the gap
  iTmp2 = iKillFile; 

  while (iTmp2 < File_Final)
  {
         iTmp1 = iTmp2 + 1;

         FileDCB            [iTmp2] = FileDCB            [iTmp1];
         strcpy(File_Name   [iTmp2],  File_Name          [iTmp1]);
         process.ByteRateAvg[iTmp2] = process.ByteRateAvg[iTmp1];
         //process.length   [iTmp2] = process.length     [iTmp1];

         iTmp2 = iTmp1;
  }

  File_Limit = iTmp2;
  File_Final = File_Limit - 1;

  if (File_Limit < 0)
      File_Limit = 0;

  MultiFile_SizeCalc();


}



//-------------------------------------------



int F580_ReOpen(int iEnough)
{
  __int64 i64RC; //, i64_Tell1;
  int iRC;

  FileDCB[iFileToRename] = _open(File_Name[iFileToRename],
                           _O_RDONLY | _O_BINARY | _O_SEQUENTIAL);

  if (FileDCB[iFileToRename] < 0)
  {
      ERRMSG_File("MpegIn", 'o', errno,  File_Name[iFileToRename], 
                                                           FileDCB[iFileToRename], 0600) ;
      F570_RemoveFile(iFileToRename, 0);

      iEnough = 3;
  }
  else
  {
      i64RC = _lseeki64(FileDCB[iFileToRename], 
                        process.i64RestorePoint[iFileToRename], 
                        SEEK_SET);
      MParse.NextLoc = process.i64RestorePoint[iFileToRename];
      if (i64RC < 0)
      {
          iRC = (int)(i64RC);
          ERRMSG_File("MpegIn", 's', iRC, File_Name[iFileToRename], errno, 1602);
          iEnough = 3;
      }
      else
      {
          iEnough = 2;
          if (iFileToRename == File_Final)
              FileNameTidy(&szOutput[0], File_Name[iFileToRename]);        
      }
  } // END Open OK

  return iEnough;
}



void F590_ReOpenAllFiles(char cRenamePlugIn_MultiMode)
{
  int iEnough;
  iEnough = 0;


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


//------------------------------------------------

int F650_Rename()
{
  int iRC, iEnough, iRC2, iTmp1;
  char *lpOut_DOT;
  
  //__int64 i64_Tell1; //, i64RC;

  iEnough = 0;

  lpOut_DOT = strrchr(szTemp, '.') ;
  if ( ! lpOut_DOT)
  {
      strcat(szTemp, ".mpg");
  }


  iRC =  strcmp(File_Name[iFileToRename], &szTemp[0]);
  if ( iRC == 0)
  {
     //  MessageBeep(MB_OK);
     iEnough = 2;
  }
  else
  {
     iTmp1 = F690_FileName_ChkChars(&szTemp[0]);
     if (iTmp1)
     {
         MessageBox( NULL, szBuffer,"Mpg2Cut2 - That does not compute",
                                   MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
     }
     else
     {
       // Win9x requires file to be closed for rename
       // so save position for later restoration

       process.i64RestorePoint[iFileToRename] = _telli64(FileDCB[iFileToRename]);

       _close(FileDCB[iFileToRename]);

       iRC = rename(File_Name[iFileToRename], &szTemp[0]);

       if (iRC != 0 )
       {
         if (iRC == 22 || iRC == 2)  
             strcpy(szBuffer, FILE_BAD_NAME_FMT);
         else
         if (iRC == EACCES || iRC == 13)  // see "errno.h" NOT double "S" EACCESS
             strcpy(szBuffer, FILE_LOCKED);
         else
         if (iRC == ENOENT)
             strcpy(szBuffer, FILE_PATH_NOT_FOUND);
         else
            sprintf(szBuffer, FILE_RENAME_FAILED, iRC, errno);

         MessageBox( NULL, szBuffer, Mpg2Cut2_ERROR,
                                   MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
       }
       else
       {
          iEnough = 1;
          strcpy(File_Name[iFileToRename], &szTemp[0]);
          strcpy(&szInput[0], &szTemp[0]);

          if (process.EDL_Used)
              C888_AutoFile(0); // Keep the EDL file up to date
       } // END Rename OK

       iRC2 = F580_ReOpen(iEnough);
       if (iEnough)
           iEnough = iRC2;

     } // END Got a different name

  }  // END - Name OK

  return iEnough;
}




int F690_FileName_ChkChars(unsigned char *lpNameChr)
{
  unsigned char K_BADDIES[] = {'>', '<', '|', '*', '?', '"', 0xFF}; //  0x27, 
  unsigned char cTST, cBAD, *lpBAD;
  int iColonFound_Flag;

  iColonFound_Flag = 0;

  for (;;)
  {
    cTST = *lpNameChr++;
    if (cTST == 0)  // end of string
      return 0;

    if (cTST == ':')  // Drive delimiter
    {
      if (iColonFound_Flag)
      {
          goto Baddie;
      }
      else
        iColonFound_Flag = 1;
    }


    
    lpBAD = &K_BADDIES[0];

    for (;;)
    {
      cBAD = *lpBAD++;

      if (cBAD == 0xFF) // end of lst
        break;
      else
      if (cTST == cBAD)
      {
Baddie:
        sprintf(szBuffer, ILLEGAL_CHR_TXT, cTST);
        return 1;
      }

    } // endfor

  } // endfor

}






//---------

int F594_TS_Warn_Msg()    // tsmpg
{
  int iRC, *lpTmp1;
  //int iTmp1;
  unsigned int uTmp2;
  char *lpDesc;
  //int iDBG1, iDBG2, iDBG3, iDBG4;

  //Chg2RGB24(0,0);

  MParse.SystemStream_Flag = -1;
  Mpeg_PES_Version = 2;  process.Mpeg2_Flag = 4;

  //iDBG1 = stricmp(szInExt3_lowercase,"ts");
  //iDBG2 = !iDBG1;
  //iDBG3 = stricmp(szInExt3_lowercase,"m2t");
  //iDBG4 = !iDBG3;
  //iDBG1 = iDBG2 && iDBG4;

  if (stricmp(szInExt3_lowercase,"ts")
  &&  stricmp(szInExt3_lowercase,"m2t") )
  {
          lpTmp1 = &iCtl_WarnTSmpg;
          uTmp2  = IDM_WARN_FMT_TS;
          lpDesc = &"TS format inside .MPG";
  }
  else
  {
          lpTmp1 = &iCtl_WarnTS;
          uTmp2  = IDM_WARN_FMT_TSMPG;
          lpDesc = &"Transport Stream";
  }
      

  iRC = F591_Ask_Trojan(1, lpDesc, lpTmp1, uTmp2);

  return iRC;

}


//---------

int F595_NotMpeg2_Msg(int P_Stage)
{
  int iRC;

  iRC = IDOK; 
 
  S100_Stats_Hdr_Main(0);

  if (iCtl_WarnMpeg1 || P_Stage)
      sprintf(szMsgTxt, NON_Mpeg2_PS,
                          (int)(process.PACK_Loc), StatsPrev.VobTxt);
  else
      strcpy(szMsgTxt, NOT_MPEG2_BRIEF);

  if (! Mpeg_Version_Alerted || P_Stage)
  {
     if (!MParse.SeqHdr_Found_Flag || P_Stage)
     {
        if (iCtl_WarnMpeg1 && Mpeg_Version_Alerts_Session < 3)
        {
           Mpeg_Version_Alerts_Session++;
           iRC = //MessageBox(hWnd_MAIN, szMsgTxt, "Mpg2Cut2 - CONFIRM", MB_OKCANCEL);
                 Warning_Box(&szMsgTxt[0], 0, &iCtl_WarnMpeg1, IDM_WARN_MPEG1, MB_OKCANCEL);

        }  
        else
           iRC = 1;
     }

     Mpeg_Version_Alerted++;
  }

  if (iRC == 1 && !P_Stage)
  {
           ToggleMenu('S', &iPES_Mpeg_Any, IDM_MPEG_ANY);
           //Chg2RGB24(1,0);
  }
  
  DSP1_Main_MSG(0,0);
  
  return iRC;
}


//-----------------------------------

int F550_FileTrojan_Chk()
{
  int iRC, iTmp1; //, *lpTmp1;
  // unsigned int uTmp2;
  // char *lpDesc;
  // unsigned int uTmp1, uTmp2, uTmp3;


  iRC = 0;

  // Should make the following checks available during Garbage scan !
  // As non-interactive displays. 

  if ( *(DWORD*)(&uHeader[0]) == uPACK_START_CODE)   // Mpeg Pack Header
  {
      MParse.SystemStream_Flag = 1;
      if ( (*(char*)(&uHeader[4]))& 0xC0 == 0x40) // Mpeg-2 pack
      {
         Mpeg_PES_Version = 2;  //process.Mpeg2_Flag = 4;
      }
  }
  else
  if ( *(DWORD*)(&uHeader[0]) == uSYSTEM_START_CODE) // Mpeg System Hdr
  {
      MParse.SystemStream_Flag = 1;
  }
  else
  if ( *(DWORD*)(&uHeader[0]) == uVIDPKT_STREAM_1)   // Mpeg PES Vid Header
  {
      MParse.SystemStream_Flag = 1;
      if ( (*(char*)(&uHeader[6]))& 0xC0 == 0x80) // Mpeg-2 PES 
      {
         Mpeg_PES_Version = 2;  //process.Mpeg2_Flag = 4;
      }
  }
  else
  if ( *(char*)(&uHeader[0]) == 0x47) // Mpeg-2 Transport Stream 
  {
      iRC = F594_TS_Warn_Msg();
  }
  else
  if ( *(short*)(&uHeader[0]) == 0x5641) // 'AV' = PVA Transport Stream 
  {
      //Chg2RGB24(0,0);
      iRC = F591_Ask_Trojan(1, &"PVA Stream", &iCtl_WarnTS, IDM_WARN_FMT_TS);
      MParse.SystemStream_Flag = -2;
      Mpeg_PES_Version = 2;  process.Mpeg2_Flag = 4;
  }
  else
  if ( (uHeader[0]) == 0x46464952) // 'RIFF') 
  {
      if (uHeader[2] == 'AXDC' )  // CDXA
      {
         strcpy(szTmp32, "CDXA RIFF");
         process.iOut_DropCrud = 1;
         iTmp1 = 1;
      }
      else
      {
         strcpy(szTmp32, "AVI");
         iTmp1 = 0;
      }

      iRC = F591_Ask_Trojan(iTmp1, &szTmp32[0], &iCtl_WarnCDXA, IDM_WARN_FMT_CDXA);

      if (process.iOut_DropCrud)
          iRC = 0;
  }
  else
  if ( (uHeader[0]) == 0x75B22630) // ASF Sentinel 
  {
        iRC = F591_Ask_Trojan(0, &"ASF", NULL, 0);
  }
  else
  if ( (uHeader[0]) == 0x464D522E) // '.RMF') 
  {
       iRC = F591_Ask_Trojan(0, &"RealMedia", NULL, 0);
  }
  else
  if ( (uHeader[1]) == 0x766F6F6D) // 'moov') 
  {
       iRC = F591_Ask_Trojan(0, &"Quicktime MOV", NULL, 0);
  }
  else
  if ( (uHeader[0])&0x00FFFFFF == 0x00334449) // 'ID3x') 
  {
       iRC = F591_Ask_Trojan(0, &"MP3", NULL, 0);
  }
  else
  if ( (uHeader[2])&0x00FFFFFF == 0x0034706D) // 'MP4x') 
  {
       iRC = F591_Ask_Trojan(0, &"Mpeg-4", NULL, 0);
  }
  else
  if ( (uHeader[1]           ) == 'pytf'  //   ftyp 
    || (uHeader[2]&0x0000FFFF) == 'G3')   //   3GP, 3GG, 3G2
  {
        iRC = F591_Ask_Trojan(0, &"3G media", NULL, 0);
  }
  else
  if ( (uHeader[2]&0x00FFFFFF) == 0x002f2f3a  //   '://') 
    || (uHeader[2]&0x0000FFFF) == 0x00002f2f) //   '//') 
  {
        iRC = F591_Ask_Trojan(0, &"Net URL", NULL, 0);
  }
  else
  if ( (uHeader[0]) == 0x6d74683c      //   '<htm'
  ||   (uHeader[0]) == 0x4d54483c)       //   '<HTM'
  {
        iRC = F591_Ask_Trojan(0, &"WEB PAGE", NULL, 0);
  }
  else
  if ( (uHeader[0] & 0xFFFF) == 'KP'      //   'PK'  - PKZIP
  ||   (uHeader[0] & 0xFFFF) == 'Z7' )    //    '7Z'  - 7ZIP
  {
        iRC = F591_Ask_Trojan(0, &"ZIP compressed", NULL, 0);
  }
  else
  if ( (uHeader[0] & 0xFFFFFF) == 'raR', NULL, 0)    //   'Rar'  - RAR
  {
        iRC = F591_Ask_Trojan(0, &"RAR compressed", NULL, 0);
  }
  else
  if ( (uHeader[0] & 0xFFFFFF) == 0x00088b1F)    //   1F8b08  - GZIP
  {
        iRC = F591_Ask_Trojan(0, &"GZip compressed", NULL, 0);
  }
  else
  if ( (uHeader[0] ) == 'FDP%')    //   'PDF' 
  {
      iRC = F591_Ask_Trojan(0, &"PDF Document", NULL, 0);
  }
  else
  if ( (uHeader[0] & 0x0000FFFF) == 'ZM')    //   'MZ'  EXE file
  {
        iRC = F591_Ask_Trojan(0, &EXE_PGM, NULL, 0);
  }
  else
  if ( (uHeader[0] & 0xFFFF) == 0xFECA)      //   xCAFE - JAVA
  {
        iRC = F591_Ask_Trojan(0, &"JAVA program", NULL, 0);
  }
  else
  if ( (uHeader[0] & 0xFFFFFF) == 'VLF')    //   'FLV'  FLASH VIDEO
  {
        iRC = F591_Ask_Trojan(0, &"FLASH Video", NULL, 0);
  }
  else
  if ( (uHeader[0]           ) == '8FIG')    //   GIF8 - GIF pic
  {
        iRC = F591_Ask_Trojan(0, &"GIF image", NULL, 0);
  }
  else
  if ((    (uHeader[0] & 0xFFFF) == 0xd8FF   //  xFFD8 - JPEG
        && (uHeader[2] & 0xFFFF) == 'FI'     //   ..IF - JPEG
      )
   && (    (uHeader[1] >>16    ) == 'FJ'     //   JFIF - JPEG
        || (uHeader[1] >>16    ) == 'XE'     //   EXIF - JPEG
      ))
  {
        iRC = F591_Ask_Trojan(0, &"JPEG image", NULL, 0);
  }
  else
  if ( (uHeader[0] & 0xFFFFFF00) == 0x474e5000)   //   xPNG pic
  {
        iRC = F591_Ask_Trojan(0, &"PNG image", NULL, 0);
  }
  else
  if ( (uHeader[0] & 0xFFFF) == 'MB')      //   BMxx = BMP file
  {
        iRC = F591_Ask_Trojan(0, &"BMP image", NULL, 0);
  }
  else
  if ( (uHeader[0]        ) == 0xe011cfd0)  // D0 CF 11 E0 = MS DOC file
  {
        iRC = F591_Ask_Trojan(0, &"MS DOC ", NULL, 0);
  }

  
  if (iPES_Mpeg_Any)  // Let anything in
      iRC = 0;
  else
  if (iRC == IDOK)
  {
      iRC = -1; 
  }
  else
    iRC = 0;

  return iRC;

}


//-------------------------------
int F591_Ask_Trojan(const int P_Level, const void *P_Desc,
                    int *P_Ctl, const unsigned int P_MenuId)
{
  char *lsCategory[2] = {"*** Trojan ",      ""      };
  char *lsLevel[2]    = {"",                 " FULLY"};
  char *lsAction[2]   = {"\n\nFILE SKIPPED", ""      };

  int iRC, iZero;
  int *lpCTL_FLAG;
  szBuffer[_MAX_PATH*10];

  iZero = 0;
  if (P_Level > 0)
      lpCTL_FLAG = P_Ctl;
  else
      lpCTL_FLAG = &iZero;

  sprintf(szBuffer, "%s %s file ***\n\nFORMAT NOT%s SUPPORTED\n\n%s%s", 
                          lsCategory[P_Level], P_Desc,   
                             lsLevel[P_Level], szInput, 
                            lsAction[P_Level]);

  if (P_Level
  && (iSuppressWarnings || !(*lpCTL_FLAG)) )
  {
      strcpy(szMsgTxt, szBuffer);
      iRC  = IDOK;
  }
  else
  {
    iRC = //MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - Warning", MB_OK);
          Warning_Box(&szBuffer[0], 0, lpCTL_FLAG, P_MenuId, MB_OKCANCEL);
  }

  if (iRC == IDYES)
      iRC  = IDOK;


  if (DBGflag)
  {
      DBGout(szBuffer);
  }


  return iRC;
}



//-------------------------------------------------------------------
void MultiFile_SizeCalc()
{
   int i;
   process.total = 0;
   for (i=0; i<File_Limit; i++)
   {
         process.origin[i] = process.total ;
         process.length[i] = _filelengthi64(FileDCB[i]);
         process.total    += process.length[i];
   }
}





//-------------------------

void F900_Close_Release(char P_Action)
{
     MParse.Stop_Flag = true;
     Sleep(250);
     if (hThread_MPEG)
         WaitForSingleObject(hThread_MPEG, 0);

     F950_Close_Files(P_Action);

     if (DDOverlay_Flag && iCtl_Ovl_Release)
         D300_FREE_Overlay();

     SetWindowText(hWnd_MAIN, szAppTitle);

     if(iWAV_Init)
         WAV_WIN_Audio_close();
}



//---------------------------------
void F950_Close_Files(char P_ACTION)
{
  int iRC, iKill, iErr, iTot;
  // char szTemp[_MAX_PATH]; //, *szSlashPTR;
  HCURSOR hNewCursor, hOldCursor;

  iKill = iErr = 0;
  iTot = File_Limit;


  // Release downstream buffers
   if (MParse.SeqHdr_Found_Flag)
   {
         D300_FREE_Overlay();
         PicBuffer_Free();
         MParse.SeqHdr_Found_Flag = 0;
   }

   Tick_CLEAR();

   // Make sure all files are closed;  Optional DELETE

   Prev_Coded_Width = 0;
   D501_RESET_MainWindow();

   if (P_ACTION == 'd'  && iCtl_RecycleBin)
   {
       hNewCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
       if (hNewCursor)
           hOldCursor = SetCursor(hNewCursor);
       else
          hOldCursor  = 0;
   }

   while (File_Limit)
   {
     File_Limit--;
     _close(FileDCB[File_Limit]);
     if (P_ACTION == 'd')
     {

         iRC = F999_Del_or_Recycle(File_Name[File_Limit]);

         if ( ! iRC)
         {
             iErr++;
         }
         else
            iKill++;
     }
   }

   if (VideoList_MODE == 'd' 
   &&  process.EDL_Used
   &&  szEDLname[0] != '*') // DELETE The EDL file too
   {
           iRC = F999_Del_or_Recycle(szEDLname);
           if (iRC)
           {
               iKill++;
               if (!szEDLprev[0]
               &&   szEDLprev[0] != '*'
               &&  !stricmp(szEDLprev, szEDLname))
               {
                   iRC = F999_Del_or_Recycle(szEDLprev);
                   if (iRC)
                       iKill++;
               }
           }
           szEDLname[0] = 0;
           szEDLprev[0] = 0;
   }

   if (iKill) // == iTot)
   {
       sprintf(szMsgTxt, FILE_DELETED_N, iKill);
       DSP1_Main_MSG(0,0);
       DSP5_Main_FILE_INFO();
       MParse.Fault_Flag = 0;
       szBuffer[0] = 0;  szMsgTxt[0] = 0;

   }

   File_Final = 0;


   if (P_ACTION == 'd'  && iCtl_RecycleBin && hOldCursor)
   {
      SetCursor(hOldCursor);
   }
}




//-------------------------------------
//
// 0 = FAILED

int  F999_Del_or_Recycle(char *lpDSN)  // LPCTSTR
{
 SHFILEOPSTRUCT Shelly;
 // SHELLEXECUTEINFO Shelly;

 int iRC,iTryOldDel;
 char *lpEND;

  iRC = 0;
  iTryOldDel = 1;

  if (iCtl_RecycleBin)
  {
     strcpy(szMsgTxt,FILE_RECYCLING);
     DSP1_Main_MSG(0,0);
     UpdateWindow(hWnd_MAIN);

     lpEND = stpcpy1(&lpTmp16K[0], lpDSN);
    *lpEND++ = 0;
    *lpEND++ = 0;
    *lpEND++ = 0;

    ZeroMemory(&Shelly, sizeof(Shelly));

    Shelly.hwnd   = hWnd_MAIN;
    Shelly.wFunc  = FO_DELETE; 
    Shelly.pFrom  = &lpTmp16K[0]; 
    Shelly.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;

    iRC = 1 - SHFileOperation(&Shelly);	// Opposite RC to others

    // Shell Recycle has big overhead, so let the system breathe
    Sleep(200);             // Wait Till The Nun Signs Shelly
    DSP_Msg_Clear();
    if (iRC)
        iTryOldDel = 0;
    else
    {
       Msg_LastError("Recycle: Result=", iRC, 'b');
    }

    /*
    ZeroMemory(&Shelly, sizeof(Shelly));
    Shelly.cbSize = sizeof(Shelly);
    Shelly.fMask  = SEE_MASK_FLAG_DDEWAIT; // Wait Till The Nun Signs Shelly
    Shelly.hwnd = hWnd;
    Shelly.lpVerb = &"Delete";
    Shelly.lpFile = &szBuffer[0]; 

    iRC = ShellExecuteEx(&Shelly);	// pointer to SHELLEXECUTEINFO structure
    */
       
    /*  OLD STYLE ShellExecute

      iRC = ShellExecute(NULL, "delete", lpDSN, 
                                              NULL, 
                                              NULL, 
                                              SW_SHOWNORMAL);

      if (iRC > 32)   // normal response
          iRC = 1;    // flag as good
      else
      {
        if (iRC == 0) //    2
          strcpy(szTmp32,"Out Of Memory or Resources");
        else
        if (iRC == SE_ERR_FNF) //    2
          strcpy(szTmp80,"File Not Found");
        else
        if (iRC == SE_ERR_PNF) //              3    
          strcpy(szTmp80,"Path Not Found");
        else
        if (iRC == SE_ERR_ACCESSDENIED) //     5      
          strcpy(szTmp80,"Access Denied");
        else
        if (iRC == SE_ERR_OOM) //              8     
          strcpy(szTmp80,"Out of Memory 8");
        else
        if (iRC == SE_ERR_DLLNOTFOUND) //              32
          strcpy(szTmp80,"DLL NOT FOUND");
        else
        if (iRC == SE_ERR_SHARE) //                    26
          strcpy(szTmp80,"ERR SHARE");
        else
        if (iRC == SE_ERR_ASSOCINCOMPLETE) //          27
          strcpy(szTmp80,"ERR ASSOCINCOMPLETE");
        else
        if (iRC == SE_ERR_DDETIMEOUT) //               28
          strcpy(szTmp80,"DDE TIMEOUT");
        else
        if (iRC == SE_ERR_DDEFAIL) //                  29
          strcpy(szTmp80,"DDE FAIL");
        else
        if (iRC == SE_ERR_DDEBUSY) //                  30
          strcpy(szTmp80,"DDE BUSY");
        else
        if (iRC == SE_ERR_NOASSOC) //                  31
          strcpy(szTmp80,"NO ASSOCIATION");
        else
          strcpy(szTmp80,"Unknown Error Code");


         sprintf(szBuffer, "Recycle attempt failed RC=%d\n\n%s", iRC, szTmp80) ;
         MessageBox(hWnd, szBuffer, szAppName, MB_ICONSTOP | MB_OK);
         iRC = 0;    // flag as bad
      }
      if (iRC == 0)    // out of memory or resources
          iRC =  6969; // Flag as bad
      */
  }

  if (iTryOldDel)
  {
     iRC = DeleteFile(lpDSN);
     if ( ! iRC)
     {
       Msg_LastError("DeleteFile: Result=", iRC, 'b');
     }
  }


return iRC;

}




/*
void BitReverse(void *P_To, void *P_From, int P_Len)
{
  int iLen;
  char *W_To, *W_From;
  char cTst, cTmp;

  iLen   = P_Len;
  W_To   = (char*)P_To + P_Len;
  W_From = (char*)P_From;

  while (W_To > P_To)
  {
    W_To--;
    cTmp = 0x00;
    cTst = *W_From;
    W_From++;
    if (cTst & 0x80)
        cTmp = cTmp || 0x01;
    if (cTst & 0x40)
        cTmp = cTmp || 0x02;
    if (cTst & 0x20)
        cTmp = cTmp || 0x04;
    if (cTst & 0x10)
        cTmp = cTmp || 0x08;
    if (cTst & 0x08)
        cTmp = cTmp || 0x10;
    if (cTst & 0x04)
        cTmp = cTmp || 0x20;
    if (cTst & 0x02)
        cTmp = cTmp || 0x40;
    if (cTst & 0x01)
        cTmp = cTmp || 0x80;
    W_To--;
    *W_To = cTmp;
  };
}
*/







//---------------------------------------------------

/*
LRESULT CALLBACK AudioList(HWND hAudioListDlg, UINT message,
                                        WPARAM wParam, LPARAM lParam)
{
  int i, j;

  switch (message)
  {
    case WM_INITDIALOG:
      File_Limit = 0;
      OpenAudioFile(hAudioListDlg);
      return true;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case ID_ADD:
          OpenAudioFile(hAudioListDlg);
          break;

        case ID_DEL:
          if (File_Limit)
          {
            i= SendDlgItemMessage(hAudioListDlg, IDC_LIST, LB_GETCURSEL, 0, 0);
            SendDlgItemMessage(hAudioListDlg, IDC_LIST, LB_DELETESTRING, i, 0);

            File_Limit--;
            File_Final--;


            for (j=i; j<File_Limit; j++)
            {
              strcpy(File_Name[j], File_Name[j+1]);
              strcpy(Outfilename[j], Outfilename[j+1]);
              SoundDelay[j] = SoundDelay[j+1];
            }

            SendDlgItemMessage(hAudioListDlg, IDC_LIST, LB_SETCURSEL,
                      i>=File_Limit ? File_Limit-1 : i, 0);
          }
          break;

        case IDOK:
        case IDCANCEL:
          EndDialog(hAudioListDlg, 0);

          if (File_Limit)
          {
            ShowStatistics(true);

            if (!threadId_MPEG
            || WaitForSingleObject(hThread_MPEG, 000)==WAIT_OBJECT_0)

              hThread_MPEG = CreateThread(NULL, 0, ProcessWAV,
                    (void *)File_Limit, 0, &threadId_MPEG);
          }
          return true;
      }
      break;
  }
  return false;
}

static void OpenAudioFile(HWND hAudioListDlg)
{
  if (X800_PopFileDlg(szInput, hAudioListDlg, OPEN_WAV, -1, &"Audio File Add"))
  {
    if (!CheckWAV())
    {
      DialogBox(hInst, (LPCTSTR)IDD_ERROR, hAudioListDlg, (DLGPROC)About);
      return;
    }

    if (X800_PopFileDlg(szOutput, hAudioListDlg, SAVE_WAV, -1, &"Audio File SAVE"))
    {
      strcpy(File_Name[File_Limit], szInput);
      strcpy(Outfilename[File_Limit], szOutput);
      DialogBox(hInst, (LPCTSTR)IDD_DELAY, hWnd, (DLGPROC)Delay);
      sprintf(szBuffer, "%s %dms", szInput, SoundDelay[File_Limit]);
      SendDlgItemMessage(hAudioListDlg, IDC_LIST, LB_ADDSTRING, 0,
                              (LPARAM)szBuffer);
      File_Final = File_Limit;
      File_Limit++;
    }
  }

  if (File_Limit)
    SendDlgItemMessage(hAudioListDlg, IDC_LIST, LB_SETCURSEL, File_Limit-1, 0);
}

DWORD WINAPI ProcessWAV(LPVOID n)
{
  int i;

  MParse.Stop_Flag = MParse.Pause_Flag = false;

  EnableMenuItem(hMenu, IDM_OPEN, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_PROCESS_WAV, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_LOAD_D2V, MF_GRAYED);
  EnableMenuItem(hMenu, IDM_STOP, MF_ENABLED);
  EnableMenuItem(hMenu, 5, MF_BYPOSITION | MF_GRAYED);

  DragAcceptFiles(hWnd, false);

  if (!n)
    Wavefs44File(SoundDelay[0], 1, 1);
  else
    for (i=0; i<File_Limit && !MParse.Stop_Flag; i++)
    {
      strcpy(szInput, File_Name[i]);
      strcpy(szOutput, Outfilename[i]);
      Wavefs44File(SoundDelay[i], i+1, File_Limit);
    }

  File_Final = File_Limit = 0;

  EnableMenuItem(hMenu, IDM_OPEN, MF_ENABLED);
  EnableMenuItem(hMenu, IDM_PROCESS_WAV, MF_ENABLED);
  EnableMenuItem(hMenu, IDM_LOAD_D2V, MF_ENABLED);
  //EnableMenuItem(hMenu, IDM_STOP, MF_GRAYED);
  EnableMenuItem(hMenu, 5, MF_BYPOSITION | MF_ENABLED);

  if (!MParse.Stop_Flag)
  {
    MessageBeep(MB_OK);
    SetDlgItemText(hStats, IDC_REMAIN, "FINISH");
  }

  SetForegroundWindow(hWnd);
  DragAcceptFiles(hWnd, true);
  return 0;
}
*/




