//
//       MPEG OUTPUT MODULE - Main
//
// Contains the main high level routines for creating the output file
// 
//

#include "windows.h"
#include "global.h"
#include <commctrl.h>
#include <fcntl.h>
#include "errno.h"

#include "out.h"
#include "Audio.h"
#include "TXT.h"

#define true 1
#define false 0


LRESULT CALLBACK OUT_DlgProgress(HWND, UINT, WPARAM, LPARAM);


int Out_Find_Sentinel(int, __int64, int, DWORD, int P_Mode) ;
int Out_RANGE(int, __int64, int, __int64, char) ;
int Out_CLIP(int);

int Out_GO();// char);

//DWORD WINAPI  Out_GO(LPVOID n);

/* static ? */ void Out_Garbage();
void Out_FILTER();
void Out_Padme(const int);


static void SCR_2SCRM(unsigned char *,
                      unsigned char *,
                      unsigned char); // SCR-DTS flag + marker bit


///* static ? */ int    Out_ACTION();//char);
DWORD WINAPI    Out_ACTION(LPVOID n);

//char Outfilename[MAX_FILE_NUMBER][_MAX_PATH];

BYTE *lpMpeg_Preamble;

int iMpeg_Out_Offset, iMpeg_ToDo_Len;
unsigned int uMpeg_PackHdrAdj;
int iErrNo;
int iOut_SystemStream_Flag;
int iIn_Errors, iIn_AutoResume, iOut_FileCreated_Flag;
int iOut_Align_Video, iOut_Align_Audio;

__int64 i64Out_SCR_FWD_Tol, i64Out_SCR_BWD_Tol;
int   iOut_SCR_FWD_Tol,   iOut_SCR_BWD_Tol;
int iBetweenPacks; //, iPackTickRate;
__int64 i64PackTickRate;
__int64 i64Out_SCR_Gap_Add, i64Out_PTS_Gap_Add;
__int64 i64AudioOut_PTS, i64Video_ADJ;
__int64 i64SCROut_TC, i64CH7_ADJ;
//__int64 i64_PTS_CurrPosn, i64_PTS_PrevPosn;

// unsigned char uConv[4]={0x00, 0x00, 0x00, 0x00};

unsigned short uFileSubStream_Id[256+32];
__int64 i64SubStream_Bytes[256+32];

int  iOut_SkippedAudioPackets, iOut_SkippedAudioBytes; // , iOut_SkippedVideoPackets;
int  iOut_AddedAudioPackets,   iOut_AddedAudioBytes;
int  iOut_AddedVideoPackets,   iOut_AddedVideoBytes, iOut_AddedVideoSEQs;
int  iOut_TC_Bridges;
int  iOut_Vortex_Bytes, iOut_Resyncs, iOut_UnkPackets;
int  iOut_PackHdrs;

unsigned char cOut_UnkStreamId;
unsigned char cOut_RS_Weird, cOut_ResetStreamId, cOut_RS_PrevStreamId; 

         int iTmp1, iTmp2, iTmp3, iTmp8;
unsigned int uTmp3, uTmp4;

// const char Coded_Pic_Abbr[8] = {'0', 'I', 'P', 'B', '4', '5', '6', '7'};


// Swap between Least Significant ordering and Most Significant ordering
void uSwapFormat(void *P_Out, void *P_In, const int P_Len)
{
  unsigned char *w_In;
  unsigned char  w_Out[4];

  w_In = (unsigned char*)(P_In);

  if (P_Len > 3)
  {
     w_Out[3] = *w_In;
     w_In=w_In+1;
  }
  if (P_Len > 2)
  {
     w_Out[2] = *w_In;
     w_In=w_In+1;
  }

  w_Out[1] = *w_In;
  w_In=w_In+1;

  w_Out[0] = *w_In;


  //  speed - handle common cases specifically
  switch (P_Len)
  {
    case 4: *(DWORD*)(P_Out) = *(DWORD*)(&w_Out[0]); break;
    case 2: *(unsigned short*)(P_Out) = *(unsigned short*)(&w_Out[0]); break;
    case 1: *(unsigned  char*)(P_Out) = *(unsigned  char*)(&w_Out[0]); break;
    default:
              memcpy(&P_Out,            &w_Out[0], P_Len);
  }
  return;

}

//---------------------------------

    int iFreeMB;
__int64 i64FreeBytes;

  int   iOut_Bridged_Flag, iOutVOB;

 char cOut_ACT;
  int iKill_PTS_Flag;

 DWORD dwOrgCanvas;
 unsigned int uOrgWidth, uOrgHeight, uOrgAspect, uOrgFrameRate;
 int iOut_Fix_Chroma_Fmt, iOut_FixPktEdge, iFixEdges;

int iOut_UnMux_Cancel;

int iTrks_MPA_Sel[8], iTrks_SUB_Sel[8];
int iTrks_AC4_Sel[8], iTrks_DTS_Sel[8], iTrks_DDP_Sel[8];
int iTrks_LPCM;

HWND hUnMuxOptDlg; //, hTracksDlg;
LRESULT CALLBACK UnMuxOption_Dialog(HWND hDialog,  UINT message,
                                  WPARAM wParam, LPARAM lParam);

//LRESULT CALLBACK Tracks_Dialog(HWND, UINT, WPARAM, LPARAM);


void Out_Status_Msg()
{
      DSP1_Main_MSG(0,0);
      iMsgLife = 0; szMsgTxt[0] = 0;
      UpdateWindow(hWnd_MAIN);
      Sleep(100);
}

void Out_Accessing_Msg()
{
  
      strcpy(szMsgTxt,FILE_ACCESSING); // "Accessing..."
      Out_Status_Msg();
}





//------------------------------------------------

char *lpOut_Part_Num;
char  szPart_Num[8];
int   iPart_Num_Needed;


char *lpOut_DOT, *lpTST;
char *lpSetExt, *lpTemp, szGotExt[16];


void Out_SzPart()
{
  sprintf(szPart_Num, "_%03d.", (process.iOut_Part_Ctr + 1));
}



void  Out_Name_Part_Xtn(int P_Accept)
{
  // char szTrail[32];
  int iOverride, iAccept;
  char cTST;

  iAccept   = P_Accept;

  iOverride = -1;

  Out_SzPart();

  lpOut_DOT = strrchr(szOutput, '.');

  if (! lpOut_DOT ) // Make sure there is a dot
  {
       strcat(szOutput, ".");
       lpOut_DOT = strrchr(szOutput, '.');
       //if (!lpSetExt || *lpSetExt <= ' ')
       //{
       //     lpSetExt = &szGotExt[0];
       //}
       //else
            strcpy(lpOut_DOT+1, lpSetExt);
  }

  if (iAccept > 0)  // Remember user's choice of Extension
  {
      strncpy(szGotExt, lpOut_DOT+1, 15);
      lpSetExt = &szGotExt[0];
  }
  else
  if (iAccept == 0 || process.iOutUnMux)  // Default extension
  {
      strcpy(lpOut_DOT+1, lpSetExt);
  }


  lpOut_Part_Num = lpOut_DOT - 4;   //  _001.

  // Is there ALREADY a PART sequence number in the name ?
  if (iPart_Num_Needed && lpOut_Part_Num >= &szOutput[0])
  {
      if (iPart_Num_Needed < 0)
          iPart_Num_Needed = 1;

      lpTST = lpOut_Part_Num;
      if (*lpTST == '_' || *lpTST == '-' )
      {
           lpTST++;
           cTST = *lpTST;
           if (cTST >= '0' && cTST <= '9' )
           {
               iOverride = (int)(cTST) - '0'; // 1st digit

               lpTST++;
               cTST = *lpTST;
               if (cTST >= '0' && cTST <= '9' )
               {
                   iOverride = (iOverride*10) + (int)(cTST) - '0'; // 2nd digit

                   lpTST++;
                   cTST = *lpTST;
                   if (cTST >= '0' && cTST <= '9' )
                   {
                       iOverride = (iOverride*10) + (int)(cTST) - '0'; // 3rd digit

                       if (iAccept > 0)  // Optionally accept the user's initial suggestion
                       {
                           process.iOut_Part_Ctr = iOverride - 1;
                           Out_SzPart();
                       }
                       else
                       {
                           // overwrite existing part number
                           strncpy(lpOut_Part_Num, szPart_Num, 4);
                           strcpy(szDBGln, szOutput);
                       }

                       iPart_Num_Needed = -1;
                  }
               }
           }
      } 
  }
      

  if (iPart_Num_Needed > 0)  // May need to insert a _001 style part number ?
  {
      strcpy(szTemp, lpOut_DOT+1);     // save the current extension
      lpOut_DOT = stpcpy0(lpOut_DOT, szPart_Num); // insert Part_number and DOT
      strcpy(lpOut_DOT, szTemp);      // restore the current extension
      lpOut_DOT--;                    // point back to new dot
  }

  ofn.lpstrDefExt = lpSetExt;

  // VOB file are special too
  if (lpOut_DOT && _stricmp(lpOut_DOT, ".vob") ==0 && !process.iOutUnMux)
      iOutVOB = 1;
  else
      iOutVOB = 0;


}


void Out_ReInstatePicture()      // Rebuild/Restore video display
{
  int iOK = 0;

  // szMsgTxt[0] = 0;

  DSP_Msg_Clear();


  if (!MParse.SeqHdr_Found_Flag)
  {
      iKick.Action = ACTION_INIT;
      MPEG_processKick();
  }
  else
  if (IsIconic(hWnd_MAIN))
  {
  }
  else
  {
    if (MParse.iColorMode != STORE_RGB24
    &&  iCtl_Ovl_Release && !iCtl_View_RGB_Always)
    {
      iShowVideo_Flag = iCtl_ShowVideo_Flag;
      if (DDOverlay_Flag)
          D300_FREE_Overlay();
      D100_CHECK_Overlay();
      if (DDOverlay_Flag)
      {
          RenderYUY2(1);
          //D200_UPD_Overlay();
          iOK = 1;
      }   
    }
  
    if (iOK == 0)
    {
      RenderRGB24();
    }
  }

  iBusy = 0;
}


int Out_Dup_Name_TST(char *lpOutName)
{
  int iRC2;

  iRC2 = F503_Dup_Name_TST(lpOutName,
         &"OUTPUT filename same as INPUT.\n\n%s\n\nTry a different name.");
  return iRC2;
}

//----------------------------------------

void OUT_SAVE_GO(char P_Act)
{

  const int K_FrameCode[7] 
    = { 2,  2,  2,  3,   2,   2,  2};
  const int K_FrameRatio[7] 
    = {0x37,  0x65,  0x43,   0x64, 0x4b,   0x6b,  0x17};

  const char *lpPARSE_QRY =
  &"\n\nAnd turn on Full Output Parsing ?\n\nPLEASE CHECK THE OUTPUT AFTERWARDS.";

  int iRC, iRC2;
  unsigned long iSectorsPerCluster, iBytesPerSector,
                iFreeClusters, iTotalClusters ;

  int  iTmp8, iLen;
  unsigned int uTmp1;

  char szDRIVE[4], szTitle[32], szReference[_MAX_PATH];
  char szFileSystem[16], szVolName[64], szTmp160[160];
  char cTmp1;

   //__int64 i64Tmp1; //, i64Tmp2;

  cOut_ACT = P_Act;

  File_Final       = File_Limit - 1;
  ZeroMemory(&uFileSubStream_Id, sizeof(uFileSubStream_Id));
  ZeroMemory(&uPTS_Accounted,    sizeof(uPTS_Accounted));
  
  iOut_FileCreated_Flag = 0;
  iOut_Rate = 0; iFixedRate = 1;
  iPack_Ctr = 0;
  iOut_SkippedAudioPackets = 0;
  iOut_SplitVideo_PrePackets = 0;  iOut_SplitVideo_PostPackets = 0;
  iOut_SplitAudio_PrePackets = 0;  iOut_SplitAudio_PostPackets = 0;

  iOut_AddedAudioPackets = 0; iOut_AddedAudioBytes = 0;
  iOut_AddedVideoPackets = 0; iOut_AddedVideoBytes = 0;
  iOut_AddedVideoSEQs = 0;
  iOut_CheckedPackets = 0;    iOutPaddingPkts = 0;

  iOut_TC_Bridges    = 0;
  iOut_Vortex_Bytes   = 0;    
  iOut_Resyncs        = 0;    iOut_UnkPackets     = 0;  iFixEdges = 0;  
  iOut_PackHdrs       = 0;    iOut_Force_Interlace_ctr = 0; 
  iOut_CannotFix_ctr  = 0;

  iOut_Breathe_PktCtr  = 0;   iOut_Breathe_PktLim  = 0;
  iOut_Invent_Needed   = 0;   iOut_Invent_Done     = 0;
  iGOP_Memo[0]         = 0;   iGOP_Memo[1]         = 0; iGOP_Memo[2] = 0;

  
  cOut_ResetStreamId = 0;  cOut_UnkStreamId = 0;  cOut_RS_Weird = 0;

  if (process.preamble_len < 0
  ||  process.preamble_len > 65536)
  {
     sprintf(szBuffer, "*BAD PREAMBLE* %d", process.preamble_len);
     MessageBox(hWnd_MAIN, szBuffer,  "Mpg2Cut2 - BUG", MB_OK);

     if (DBGflag)
     {
         DBGout(szBuffer);
     }

     process.preamble_len = 4096;
  } 

  // Handling of current selection varies depending on action
  switch (cOut_ACT)
  {
    case '1':
      strcpy(szTitle, "Save 1 Clip");
      C320_Sel2Clip();
      break;

    case 'G':
      strcpy(szTitle, "Turn GARBAGE into File");
      break;

    default:
      if (cOut_ACT == 'P')
         strcpy(szTitle, "Save Mpeg2 File parts");
      else
         strcpy(szTitle, "SAVE Mpeg2 File");
      if (Ed_Prev_Act != '+'  // Current Selection has not been added
      && (Ed_Prev_Act != '-' || iEDL_ctr == 0)) // nor deleted
      {
         if (iEDL_ctr == 0)
              cTmp1 = '+';  // Don't need confirmation if no other clips
         else
              cTmp1 = 'C';  // Better confirm ADD

         C350_Clip_ADD(cTmp1, 0);
      }
  }

  if (cOut_ACT != 'G')
  {
     if (MParse.SystemStream_Flag < 0) 
     // || Mpeg_PES_Version != 2)
     {
       /*
        sprintf(szBuffer, "%s FORMAT NOT SUPPORTED\n\nCONTINUE ?",
                               StatsPrev.VobTxt);
        iRC2 = MessageBox(hWnd, szBuffer,  "Mpg2Cut2 - CONFIRM", MB_OKCANCEL);
        */
        if (Mpeg_Version_Alerted < 2)
        {
          iRC2 = F595_NotMpeg2_Msg(1);
          if (iRC2 != IDOK)
            return;
        }
     }


     if (cOut_ACT == '1')
     {
        iEDL_ClipFrom = iEDL_ctr;
        iEDL_ClipTo   = iEDL_ClipFrom + 1;
     }
     else
     {
        iEDL_ClipFrom = 0;
        iEDL_ClipTo   = iEDL_ctr;
     }

     C000_Clip_TOTAL_MB(cOut_ACT);
  }


  if (process.iOutUnMux)
  {
      DialogBox(hInst, (LPCTSTR)IDD_UNMUX,
                                hWnd_MAIN, (DLGPROC)UnMuxOption_Dialog);
      if (iOut_UnMux_Cancel)
      {
          iBusy = 0;
          return;
      }
  }
  else
  {
      iOut_UnMuxAudioOnly = 0;
      iOut_UnMux_Fmt = 0;
  }

  /*
  if (X800_PopFileDlg(szOutput, hWnd, SAVE_AVI, -1, &"AVI File SAVE"))
  {
      AVI_Flag = true;
      iShowVideo_Flag = false;
      EnableMenuItem(hMenu, IDM_OPEN, MF_GRAYED);
      EnableMenuItem(hMenu, IDM_SAVE, MF_GRAYED);
      EnableMenuItem(hMenu, IDM_LOAD_D2V, MF_GRAYED);
      EnableMenuItem(hMenu, IDM_SAVE_D2V, MF_GRAYED);
      EnableMenuItem(hMenu, IDM_STOP, MF_ENABLED);

      Menu_Main_Disable(true, true);
      ShowStatistics(true);

      process.Action = ACTION_RIP;

      if (WaitForSingleObject(hThread_MPEG, 0)==WAIT_OBJECT_0)
          hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);
  }
  */


  // Leadtek option implies repaint of mask area
  //if (iCtl_Mask_Colour == iColor_Menu_BG // Overlay key Mid Grey ?  (Leadtek compat)
  //&&  MParse.iColorMode != STORE_RGB24)  // Overlay mode ?
  //    DD_OverlayMask(0); // black out overlay area
  



  _fmode  = _O_BINARY;
  iPlayAudio  = 0;

  Out_CanFlag = 0 ;   Out_PauseFlag = 0;  iOutSuspCtr = 0;
  iOut_Clip_ctr = 0; iOut_Bridged_Flag = 0;


  if (iCtl_Out_KillPadding 
             // && !iFixedRate 
             && (!iOutVOB || !MParse.iVOB_Style))
    iOut_KillPadding = 1;
  else
    iOut_KillPadding = 0;



  iOut_Parse_AllPkts = (iCtl_Out_Parse_AllPkts && iCtl_Out_Parse)
                    |   process.iOut_DropCrud  // Force packet parsing  
                    |   iOut_KillPadding
                    |  !iOut_Audio_All;

  
  if (iOut_FixPktLens 
  &&  !process.iOut_DropCrud && iCtl_Out_Parse
  // && iCtl_Out_Fix_Errors  
  )
  {
      iOut_FixPktEdge = 1;
      iOut_Parse_AllPkts = 1;
  }

  if (iOut_PS1PS2)  // Convert Private Stream 1 to PS2 - TWINHAN PROBLEM
  {
      iOut_Parse_AllPkts = 1;
      iInPS2_Audio = 1;
  }
  

  iOut_Parse_Deep = iCtl_Out_Parse_Deep;

  if (iCtl_Out_Fix_SD_Hdr
   && Coded_Pic_Width < 769 && Coded_Pic_Height < 577)
      iOut_Fix_SD_Hdr = iCtl_Out_Fix_SD_Hdr;
  else
      iOut_Fix_SD_Hdr = 0;

  iOut_Align_Video = iCtl_Out_Align_Video && process.Mpeg2_Flag;
  if (process.iSEQHDR_NEEDED_clip1)
  {
    iOut_Align_Video = 1;
  }
  iOut_Align_Audio = iCtl_Out_Align_Audio && process.Mpeg2_Flag;


  // ASPECT RATIO ADJUSTMENT option

  iOut_Fix_Aspect = 0;
  uOrgAspect = MPEG_Seq_aspect_ratio_code;

  // I am not confident enough yet to change Mpeg-1.

  if (iView_Aspect_Mode < 4 // Nice boring aspect ratio ?
  && process.Mpeg2_Flag && Mpeg_SEQ_Version == 2) // Pure Mpeg-2 ?
  {
      if (iCtl_Out_Parse)
        szTmp160[0] = 0;
      else
        strcpy(szTmp160, lpPARSE_QRY);

      sprintf(szBuffer, "FORCE Aspect Ratio to %s ?%s\n",
                         Mpeg2_Aspect_Ratio_Name[iView_Aspect_Mode + 1],
                               szTmp80);
      iRC2 = MessageBox(hWnd_MAIN, szBuffer,  "Mpg2Cut2 - CONFIRM", MB_YESNO);
      switch (iRC2)
      {
         case IDOK: 
         case IDYES: 
              uOrgAspect = iView_Aspect_Mode + 1;

              iOut_Fix_Aspect = 1;
              iOut_Parse_AllPkts = 1;
              // iOut_Parse_Deep    = 1;
              iCtl_Out_Parse |= 1;
              break;
     }
  }

  iOut_Fix_Frame_Rate =  0;
  iOutFRatioCode      = -1;
  uOrgFrameRate       = MPEG_Seq_frame_rate_code;

  // Mpeg-1 can only support a limited range of frame rates
  // Mpeg-2 can support others, but requires extra fiddling
  if (iOverride_FrameRate_Code
  && (iOverride_FrameRate_Code < 9  || process.Mpeg2_Flag))    //  ||  DBGflag))
  {
      if (iCtl_Out_Parse)
        szTmp160[0] = 0;
      else
        strcpy(szTmp160, lpPARSE_QRY);

      sprintf(szBuffer, "FORCE Frame Rate to %02f FPS ?%s",
                               frame_rate_Table[iOverride_FrameRate_Code]);
      iRC2 = MessageBox(hWnd_MAIN, szBuffer,  "Mpg2Cut2 - CONFIRM", MB_YESNO);
      switch (iRC2)
      {
         case IDOK: 
         case IDYES: 
              iOut_Fix_Frame_Rate = 1;
              if (iOverride_FrameRate_Code < 9)  // Standard Rate Code ?
              {
                  iOutFrameRateCode = iOverride_FrameRate_Code; // SEQ hdr only
              }
              else
              {  // Non-Standard rates implemented as Ratio of a Std rate
                 iTmp1 = iOverride_FrameRate_Code - 9;
                 iOutFrameRateCode = K_FrameCode [iTmp1]; // for SEQ hdr
                 iOutFRatioCode    = K_FrameRatio[iTmp1]; // for seq EXTN hdr
              }
              uOrgFrameRate       = iOutFrameRateCode;

              iOut_Parse_AllPkts = 1;
              iOut_Parse_Deep    = 1;
              iCtl_Out_Parse |= 1;
         break;
     }
  }


  // Calculate default code for canvas fix
  if ((   (MPEG_Seq_horizontal_size == (MPEG_Seq_horizontal_size & 7))     // Evenly Divisible by 8
        ||(MPEG_Seq_vertical_size   == (MPEG_Seq_vertical_size   & 7)))    // Evenly Divisible by 8
     &&    MPEG_Seq_horizontal_size &&  MPEG_Seq_vertical_size)
  {
    uOrgWidth  = MPEG_Seq_horizontal_size;
    uOrgHeight = MPEG_Seq_vertical_size;
  }
  else
  { // Emergency default
    //dwOrgCanvas = 0x3340022d;  // PAL - 720 x 576 Wide 25fps
    uOrgWidth  = 720;
    uOrgHeight = 576;
  }
    
  dwOrgCanvas =  (uOrgWidth    / 16)
              + ((uOrgWidth    & 0x00F) * 256 * 16)
              + ((uOrgHeight   / 256)  * 256)
              + ( uOrgHeight   & 0x0FF)
              +  uOrgAspect    * 16 * 16777216
              +  uOrgFrameRate      * 16777216;

 iOut_Fix_Chroma_Fmt = iCtl_Out_Fix_Errors;
  
 iOut_Fix_Hdrs_Vid =  iOut_Fix_SD_Hdr 
                   ||( iCtl_Out_Parse
                       && (   iOut_Fix_Aspect  || iOut_Fix_Frame_Rate
                           || iCtl_Out_Fix_Errors));

  if (Mpeg_PES_Version == 2          // EXCLUDE MPEG-1 - Not enough PTS info
  &&  MParse.SystemStream_Flag >= 0) // EXCLUDE Transport Streams)
  {
    iOut_PTS_Matching = iCtl_Out_PTS_Match;
  }
  else
  {
    iOut_PTS_Matching = 0;
  }

  if (!iFrame_Rate_int)
       iFrame_Rate_int = 30;

  // Allow for Ch.7 DTV SCR separation between Video and Audio streams 
  if (iCtl_Out_TC_Force)  // Fix Australian Ch.7 Nebula DTV capture
  {
      i64Out_SCR_FWD_Tol = 180000;
      i64Out_SCR_BWD_Tol = 0;
        iOut_SCR_FWD_Tol = 45000; 
        iOut_SCR_BWD_Tol = 0;
      i64Out_SCR_Gap_Add = 421;  // This must match the SCR auto-increment amount
      i64Out_PTS_Gap_Add = 421; 
  }
  else
  {
      i64Out_SCR_FWD_Tol =  180000;
      i64Out_SCR_BWD_Tol = -180000;
        iOut_SCR_FWD_Tol =  45000;
        iOut_SCR_BWD_Tol = -45000;
        i64Out_SCR_Gap_Add = 146;
        i64Out_PTS_Gap_Add = (__int64)((4*90000 / iFrame_Rate_int));
  }

  if (DBGflag)
  {
    DBGln4("T-Break Tol: FWD=%d BWD=%d PAD=%d  %dFPS\n\n", 
          i64Out_SCR_FWD_Tol, i64Out_SCR_BWD_Tol,
          i64Out_SCR_Gap_Add, iFrame_Rate_int);
  }

  i64AudioOut_PTS  = 0;   i64Video_ADJ = 0;
  i64SCROut_TC    = 0;   i64CH7_ADJ = 0;
  //i64_PTS_PrevPosn = 0;
  i64PackTickRate  = 230; // iPackTickRate    = 420; 
  iBetweenPacks = 0;
  iKill_PTS_Flag = 0;

  if (Mpeg_PES_Version == 2 && process.Mpeg2_Flag && iCtl_Out_Parse)
  {
    iOut_TC_Adjust  = (iCtl_Out_TC_Adjust   
                       && (!process.Suspect_SCR_Flag || iCtl_Out_TC_Force));
    iOut_PTS_Invent = iCtl_Out_PTS_Invent;
  }
  else
  {
    iOut_TC_Adjust  = 0;
    iOut_PTS_Invent = 0;
  }

  process.iOutParseMore = ( process.iOutUnMux
                              || (iCtl_Out_Parse
                                  &&  (    iOut_TC_Adjust 
                                        || iOut_PTS_Invent   
                                        || iOut_Parse_AllPkts
                                        //  || iOut_Parse_Deep
                                        || iOut_Fix_Aspect
                                        || iOut_Fix_Frame_Rate
                                        || iCtl_Out_Fix_Errors
                                       )
                                 )
                          )
            //&& ((Mpeg_PES_Version == 2 && process.Mpeg2_Flag)
            //              || process.iOut_DropCrud)
            && MParse.SystemStream_Flag >= 0; // EXCLUDE Transport Streams

                                 
  if (iCtl_OutPartAuto || process.iOut_AutoSPLIT)
      iPart_Num_Needed = 1;
  else
      iPart_Num_Needed = 0;


  iRC = 1;
  while(iRC)
  {
      lpSetExt = NULL;
      // Default File Extension
      // Build into both original default and FileDialog default
      if (process.iOutUnMux || !MParse.SystemStream_Flag)
      {
        if (iOut_UnMuxAudioOnly)
        {
          if (iOut_UnMux_Fmt == 2)
            lpSetExt = (char *)(&"WAV");
          else
          if (iOut_UnMux_Fmt == 1)
            lpSetExt = (char *)(&"M2P");
          else
          if (iAudio_Trk_FMT[0] == FORMAT_AC3)
            lpSetExt = (char *)(&"AC3");
          else
          if (iAudio_Trk_FMT[0] == FORMAT_DDPLUS)
            lpSetExt = (char *)(&"DDP");
          else
          if (iAudio_Trk_FMT[0] ==  FORMAT_DTS)
            lpSetExt = (char *)(&"DTS");
          else 
          if (iAudio_Trk_FMT[0] == FORMAT_LPCM)
            lpSetExt = (char *)(&"PCM");
          else
            lpSetExt = (char *)(&szOut_Xtn_AUD[0]);
        }
        else
        {
          if (iOut_UnMux_Fmt == 2)
            lpSetExt = (char *)(&"AVI");
          else
          if (iOut_UnMux_Fmt == 1)
            lpSetExt = (char *)(&"MPV");
          else
          if (Mpeg_PES_Version == 2)
            lpSetExt = (char *)(&"M2V");
          else
            lpSetExt = (char *)(&"M1V");
        }
      }

      else // Not an elementary stream

      if (MParse.SystemStream_Flag == -1) // TS
      {
          lpSetExt = (char *)(&"TS");
      }
      else
      if (MParse.SystemStream_Flag == -2) // PVA
      {
          lpSetExt = (char *)(&"PVA");
      }
      else
      if ( szOut_Xtn_RULE[0] > '$')  // Specific Preference ?
      {
          lpSetExt = (char *)(&szOut_Xtn_RULE);
      }


      if (lpSetExt && *lpSetExt)  // Got a non-null extension ?
      {
      }
      else
      {
         lpSetExt = &"mpg";
         if (File_Limit > 0)
         {
             // Same as input extension
             lpTemp = strrchr(File_Name[File_Limit-1], '.');
             if (lpTemp) // Is there an extension ?
                 lpSetExt = lpTemp + 1;
         }
      }

      Out_Name_Part_Xtn(0);
      strcpy(szReference, szOutput);

      Out_Accessing_Msg();

      iRC2 = -1;
      while (iRC2 < 0)
      {
         if (iOutNow <= 0)
         {
             // Let user choose output file name and path
             iRC = X800_PopFileDlg(szOutput, hWnd_MAIN, SAVE_VOB, -1, &szTitle[0]);
             DSP_Msg_Clear(); // DSP_Blank_Msg_Clean();
         }
         else
           iRC = 1;

         iOutNow = 0;

         if (iRC)
         {
            if (iPart_Num_Needed)
            {
              if (stricmp(szReference,  szOutput)) // Did user change file name ?
              {
                  Out_Name_Part_Xtn(1);
                  // TODO: Check if overwriting, with user confirmation
              }
            }

            process.iOutFolder_Flag = 1;

            iRC2 = Out_Dup_Name_TST(&szOutput[0]);
            if (iRC2 == 0)  // No Duplicates ?
                iRC = 1;    // Allow Save to begin
            else
                iRC = 0;    // Duplicate = NO SAVE
         }
         else
            iRC2 = 0;
      }


      if (iRC)
      {

         // Find Free space on Output Drive
         iFreeMB = iEDL_TotMB; // Default if cannot use interface

         if (iOut_UnMuxAudioOnly)
             iLen = iEDL_TotMB / 8;
         else
             iLen = iEDL_TotMB;

         if (szOutput[1] ==':')
         {
            strcpy(szDRIVE, "C:\\");
            szDRIVE[0] = szOutput[0];
            Out_Accessing_Msg();
            iRC2 = GetDiskFreeSpace(&szDRIVE[0], //&szOutput[0],
                                    &iSectorsPerCluster, &iBytesPerSector,
                                    &iFreeClusters, &iTotalClusters);
            DSP_Msg_Clear(); // DSP_Blank_Msg_Clean();
            if (iRC2)
            {
                iTmp8 = iFreeClusters * iSectorsPerCluster
                               / 1024 * iBytesPerSector / 1024;
                if(iTmp8 != 2047)  // Is it a supported volume format ?
                  iFreeMB = iTmp8;
                else
                if (cOut_ACT == 'G')
                   iFreeMB = 2047;
            }
         }

         if (cOut_ACT == 'G') iLen = iFreeMB;

         if (iFreeMB < iLen)
         {
              sprintf(szBuffer, FILE_NO_SPACE,
                                    szDRIVE[0], iFreeMB, iEDL_TotMB);
              iRC2 = MessageBox(hWnd_MAIN, szBuffer,  szAppName, MB_YESNOCANCEL);
              switch (iRC2)
              {
                case IDNO:  iFreeMB = iEDL_TotMB; break;
                case IDYES: break;
                default:    iRC = 0;
              }
         }

         
         if (iFreeMB >= iEDL_TotMB)
         {
             // Trap file >4GB not compat with FAT file system
             iRC = 1;
             if (iEDL_TotMB > 4095 && cOut_ACT != 'P')
             {
                strcpy(szFileSystem, "NTFS");
                szVolName[0] = 0;
                GetVolumeInformation(
                        &szDRIVE[0],       // addr of root directory of the file system 
                        &szVolName[0],     // addr of name of the volume  (VOLSER)
                  sizeof(szVolName),    // len  of lpVolumeNameBuffer 
                         NULL,          // addr of volume serial number 
                         NULL,          // addr of system’s maximum filename length
                        &uTmp1,         // addr of file system flags 
                        &szFileSystem[0],  // addr of name of file system 
                  sizeof(szFileSystem));  // len  of lpFileSystemNameBuffer 

                if(stricmp(szFileSystem, "NTFS"))
                {
                   sprintf(szBuffer, FILE_TOO_BIG,
                                      szFileSystem, szDRIVE[0], szVolName);
                   iRC2 = MessageBox(hWnd_MAIN, szBuffer,  szAppName, MB_YESNOCANCEL);
                   switch (iRC2)
                   {
                      case IDNO:  break;
                      case IDYES: break;
                      default:    iRC = 0;
                   }
                }
             }

             if (iRC)
             {
                 Out_GO(); //cOut_ACT);
                 iRC = 0;
             }
         }
      } // ENDIF
  } //ENDWHILE


  return ;
}

//----------------------------------------

void OUT_SAVE(char P_Act)
{

  //char cOut_ACT;
  int iRC;

  if (DBGflag) DBGout("\n***\n*** OUT-SAVE STARTED\n***\n");

  cOut_ACT = P_Act;

  Mpeg_Stop_Rqst();

  //Enable_Disable(false, 0, true);
  iBusy = 1;
  hThread_OUT = 0;
  Sleep(50);
            
  if (DDOverlay_Flag && iCtl_Ovl_Release)
      D300_FREE_Overlay();

  // Leadtek option implies repaint of mask area
  //if (iCtl_Mask_Colour == iColor_Menu_BG // Overlay key Mid Grey ?  (Leadtek compat)
  //&&  MParse.iColorMode != STORE_RGB24)  // Overlay mode ?
  //    DD_OverlayMask(0); // black out overlay area
  
  if (cOut_ACT == 'P')  // Save Parts ?
  {
      iRC = DialogBox(hInst, (LPCTSTR)IDD_TRACKSEL,
                                   hWnd_MAIN, (DLGPROC)Out_Part_Dialog);

      process.iOutUnMux   = 0;
      if (iCtl_Out_SplitSegments == -1)
          process.iOut_AutoSPLIT  =  1;
      else
          process.iOut_AutoSPLIT  =  0;

      if (!iRC)
          cOut_ACT = 0; // Abandon
  }


  if (cOut_ACT)
      OUT_SAVE_GO(cOut_ACT);

  // Allow for cancel 
  if (!hThread_OUT)
  {
      Out_ReInstatePicture();
  }

}



//------------------------------------------------
// CREATE OUTPUT FILE
int OutFile_CREATE(int iP_File)
{
  int iRC, iRC2, iTmp1;

  char *lpName;

  lpName = File_Name[iP_File];

  iTmp1 = F690_FileName_ChkChars(lpName);
  if (iTmp1)
  {
       MessageBox( NULL, szBuffer, "Mpg2Cut2 - That does not compute",
                                   MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
      return -42;  // ABANDON
  }
  else
  {
      iRC2 = Out_Dup_Name_TST(lpName);
      if (iRC2)  //  Duplicates ?
          return -22;  // ABANDON
  }


  // HERE BE THE PLACE FOR USING WIN32 "HANDLE CreateFile"
  // Also need to change :=
  //     - PARSER OUTPUT BUFFERING to match physical segment size
  //     - WRITEs AND CLOSE
  //     - MAYBE Adapt buffering to suit "FILE_FLAG_NO_BUFFERING"
  /*

  CreateFile(

    &szOutput,                // pointer to name of the file
    GENERIC_WRITE,            // access (read-write) mode
    FILE_SHARE_WRITE,         // share mode
    NULL,                     // pointer to security attributes
    CREATE_ALWAYS,            // how to create
    FILE_FLAG_WRITE_THROUGH,  // file attributes
    HANDLE hTemplateFile  // handle to file with attributes to copy
   );

  */

  iRC = -1; //ENOENT;          // Default to create NEW file
  if (cOut_ACT == 'G')         // Try to extend existing file
      iRC =  _open(lpName,  O_RDWR);
  if (iRC == -1) //ENOENT)           // Allow for file not found
      iRC = _creat(lpName, _S_IREAD | _S_IWRITE);

  FileDCB[iP_File] = iRC;   // Remember

  // *SUS* - "fopen" and "_open" have different meaning for RC=0 - WIDE IMPACT TO FIX
  if(FileDCB[iP_File] < 0)
  {
      ERRMSG_File("MpegOUT", 'o', 
                             errno, // FileDCB[iP_File],
                             lpName, 0, 8800) ;
  }
  return iRC;
}



//-------------------------------------------------------------------


int Out_File_BEGIN()
{
  int iRC;

  if (iOut_SystemStream_Flag != 0)
      iPS_Block1 = 2;
  else
      iPS_Block1 = 0;

  //if (iOutVOB) //MParse.iVOB_Style = iCtl_VOB_Style;
  //else         MParse.iVOB_Style = 0;

  if (DBGflag)
  {
      sprintf(szBuffer, "Out Xtn=%s Vob=%d Preserve=%d-%d", 
                    lpOut_DOT, iOutVOB, MParse.iVOB_Style, iCtl_VOB_Style);
      DBGout(szBuffer);
  }

  strcpy(File_Name[MAX_FILE_NUMBER], szOutput);

  if (iOut_UnMuxAudioOnly)
  {
     iRC = 0;
     File_UnMux_Limit = MAX_FILE_NUMBER;
  }
  else
  {
     if (iOut_FileCreated_Flag)
         Out_Progress_Title(hProgress);
     else
         Out_Accessing_Msg();

     iRC = OutFile_CREATE(MAX_FILE_NUMBER);
     File_UnMux_Limit = MAX_FILE_NUMBER - 1;

     DSP_Msg_Clear(); // DSP_Blank_Msg_Clean();
  }
  uFileSubStream_Id[MAX_FILE_NUMBER] = 0xE0;

  return iRC;
}




void OUT_File_END()
{ 
  int iFileIx, iRC;
  __int64 i64Tmp1;
   
  HANDLE hInFile, hOutFile;
  DWORD uFileAttr;
  FILETIME ftCreated, ftLastAccess, ftLastWrite;


  // DVD files need to be multiple of 2K

  if (iOutVOB  && MParse.iVOB_Style
  && iOut_SystemStream_Flag > 0)
  {
        i64Tmp1 = _filelengthi64(FileDCB[MAX_FILE_NUMBER]);
        i64Tmp1 = (i64Tmp1 & 2047);
        iTmp8   = 2048 - (int)(i64Tmp1) ;
        Out_Padme(iTmp8);
  }



  // Count, Stamp and Close each of the output streams
  for (iFileIx = MAX_FILE_NUMBER; iFileIx > File_UnMux_Limit; iFileIx--)
  {
      i64SubStream_Bytes[iFileIx] += _filelengthi64(FileDCB[iFileIx]);

      /*
      if (iCtl_Out_KeepFileDate)
      {
        // Find minimum File Creation Date/Time
        //memset(File_Date[MAX_FILE_NUMBER+1][0], &0xFFFFFFFFFFFFFFFF, sizeof(File_Date[MAX_FILE_NUMBER+1][0]) );
        *(__int64*)(&File_Date[MAX_FILE_NUMBER+1][0]) = 0xFFFFFFFFFFFFFFFF;
        for (iTmp2 = 0; iTmp2 > File_Final; iTmp2++)
        {
          if (   *(__int64*)(&File_Date[MAX_FILE_NUMBER+1]) 
               > *(__int64*)(&File_Date[iTmp2])
          &&  (! *(__int64*)(&File_Date[iTmp2]) ) )
          {
              *(__int64*)(&File_Date[MAX_FILE_NUMBER+1]) 
            = *(__int64*)(&File_Date[iTmp2]);
          }

        }

        if (*(__int64*)(&File_Date[MAX_FILE_NUMBER+1]) != 0xFFFFFFFFFFFFFFFF)
        {
           SetFileTime(FileDCB[File_Final],  //*** HANDLE PROBLEM ! ! ! !
                    &File_Date[File_Final],  // address of creation time 
                    NULL,                       // address of last access time  
                     &File_Date[File_Final]); // address of last write time 

        }

      }
      */



      //iRC = fflush(FileDCB[iFileIx]); // Force flushing of buffers to help narrow down I/O errors
      //if (iRC)
      //{
      //    Msg_LastError("OutFlush", iRC, 'b');
      //}


      //  CloseHandle(FileDCB[iFileIx]);  // hFile
      iRC = _close(FileDCB[iFileIx]);
      if (iRC)
      {
          Msg_LastError("OutClose", iRC, 'b');
      }

      if (iCtl_Out_KeepFileDate)
      {
         uFileAttr = 0;
         hInFile = CreateFile(File_Name[File_Final], 
                              0, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, uFileAttr, NULL);  
         iRC = GetFileTime(hInFile, &ftCreated, &ftLastAccess, &ftLastWrite);
         CloseHandle(hInFile); 

         if (iRC)
         {
            uFileAttr = FILE_ATTRIBUTE_ARCHIVE;
            hOutFile = CreateFile(File_Name[iFileIx], 
                               (GENERIC_READ | GENERIC_WRITE), 
                               FILE_SHARE_WRITE, NULL,
                               OPEN_EXISTING, uFileAttr, NULL);
            if (hOutFile == INVALID_HANDLE_VALUE)
            {
                Msg_LastError("FileTime0", 9600, 'b');
            }
            else
            {
              //GetFileTime(hOutFile, 0, &ftLastAccess, 0); // Preserve last access date
              iRC = SetFileTime(hOutFile, &ftCreated, &ftLastAccess, &ftLastWrite);
              if (! iRC)
                Msg_LastError("FileTime ", 9696, 'b');

              CloseHandle(hOutFile);
            }
         }
      }

  }; // endfor

  iOut_FileCreated_Flag = -1;

  if (!Out_CanFlag && !iOut_Error)
      process.iOut_Part_Ctr++;
  

}



//--------------------------------------------
int Out_GO()  // char P_Act)
//DWORD WINAPI Out_GO(LPVOID nParam)
{
  int iRC;
  //__int64   ViewLoc;

  //ViewLoc = Calc_Loc(&process.CurrFile, 0, 0);


  // Elementary Streams are different to Program Streams

  dwStartSentinel = uPACK_START_CODE; // 0xBA010000;  // Program PACK Hdr sentinel 01BA
  dwEndSentinel   = uPACK_START_CODE; // 0xBA010000;  // Program PACK Hdr sentinel 01BA
  //byInpVidOnly = 0;
  strcpy(szStartDesc, "PACK");

  if (MParse.SystemStream_Flag == 0 )  // Elementary Video
  {
      dwStartSentinel = uSEQ_HDR_CODE; // SEQ Hdr sentinel marks start of clip
      if (iCtl_To_Pad)
        dwEndSentinel   = uPICTURE_START_CODE;
      else
        dwEndSentinel   = uSEQ_HDR_CODE;
      //byInpVidOnly=1;
      strcpy(szStartDesc, "SEQ");
  }
  else
  if (MParse.SystemStream_Flag > 0 )  // Elementary Video
  {
    if (process.Pack_Avg_Size > 32760 // Twinhan huge pkts 
    && (! iOutVOB  || !MParse.iVOB_Style))
    {
        dwEndSentinel   = uVIDPKT_STREAM_1; // 0xE0010000;
    }
  }


  // Are we starting a Program Stream ?
  if (process.iOutUnMux)
     iOut_SystemStream_Flag = 0;
  else
     iOut_SystemStream_Flag = MParse.SystemStream_Flag;

  /*
  if (File_Final > 0)
  {
      //char *lpOut_DOT;
      lpOut_DOT = strrchr(szOutput, '.') ;
      if ( ! lpOut_DOT)
          lpOut_DOT = &szOutput[0];
  }
  if ( ! lpOut_DOT)
         lpOut_DOT = &szOutput[0];
  */

  Out_Name_Part_Xtn(-1);

  iRC = Out_File_BEGIN();

  if (iRC >= 0)
  {
     iOut_FileCreated_Flag = 1;

     if (cOut_ACT == 'G')
     {
        Out_Garbage(); // SQUAT A HUGE FREE SPACE FOR RECLAMATION FILE
     }

     else
     {
        //if (DDOverlay_Flag)    // Do we have our foot on the overlay resources ?
        //    D300_FREE_Overlay(); // Chg2RGB24(0,0);// Let go, while we are busy copying files.

        // Let user know something is happening
        hProgress = CreateDialog(hInst,
                              (LPCTSTR)IDD_PROGRESS, hWnd_MAIN,
                              (DLGPROC)OUT_DlgProgress);

        if (! iCtl_Readability)
        {
             SendMessage(hProgress, WM_SETFONT,
                             (WPARAM)(hDefaultGuiFont),
                                                     false);
        }


        // OUTPUT ACCORDING TO ACTION REQUESTED

        // maybe ? CHANGE THE FOLLOWING TO USE C function "beginthread" to avoid memory leak;
        // iRC = Out_ACTION();// cOut_ACT);

        iRC = WAIT_OBJECT_0;
        if (hThread_OUT)
        {
           iRC = WaitForSingleObject(hThread_OUT, 0);
        }
        if (iRC == WAIT_OBJECT_0)
        {
            hThread_OUT = CreateThread(NULL, 0, Out_ACTION, 0, 0,
                                               &OUT_threadId);
        }
     } //endelse NOT Garbage
  } //endif FileDCB ok

  if (DBGflag) DBGout("\nOUT-FILE *END*\n\n");
  return 1 ;
}



//-----------------------------------------------
void  Out_Garbage()
{
  int iRC;
  __int64 i64RC;
  char cNULL = 0;


// SQUAT A HUGE FREE SPACE FOR RECLAMATION FILE

  if (iFreeMB > 50)  // Allow for Norton cushion
  {
      iFreeMB -=50;
      if (iFreeMB > 4095)  // FAT32 limit
          iFreeMB = 4095;
  }


  i64FreeBytes =  iFreeMB;
  i64FreeBytes =  i64FreeBytes  * 1048576 -1;

  sprintf(szBuffer, "This will attempt to reclaim %d MB\n\n of data from free space pool\n\ninto %s",
                                         iFreeMB, szOutput);
  iRC = MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2", MB_OKCANCEL);
  if (iRC == IDOK)
  {
      iRC = -1;
      while (iRC < 0 || (errno ==  ENOSPC && i64FreeBytes > 1048576))
      {
           i64RC = _lseeki64(FileDCB[MAX_FILE_NUMBER], i64FreeBytes,
                                                                SEEK_END);
           if (i64RC < 1)
           {
               iRC = (int)(i64RC);
             ERRMSG_File("MpegOUT", 's', FileDCB[MAX_FILE_NUMBER], szOutput,
                                               (int)(i64RC), 8802);
           }
           else
           {
              errno = 0;
              iRC = Out_RECORD(&cNULL, 1, 8901 );
              if (errno ==  ENOSPC)  // Not enough space
              {
                    i64FreeBytes = i64FreeBytes>>1; // Try half as much
              }
            }
      } // endwhile

      if (iRC > -1)
      {
         iFreeMB = (int)(i64FreeBytes / 1048576);
         sprintf(szBuffer, "Some SUCCESS !\n\n Reclaimed %d MB from free space\n\ninto %s",
                                         iFreeMB, szOutput);
          MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2", MB_OKCANCEL);
      }

      //  CloseHandle(FileDCB[MAX_FILE_NUMBER]);  // hFile

      _close(FileDCB[MAX_FILE_NUMBER]);

      strcpy(szInput, szOutput);
  } // endif OK

}




  int iBitRate[256+32];
  int iStreamReady[256+32];

    int     iProgress_Prev_Time, iProgress_New_Time, iProgress_1st_Time;
    int     iProgress_DiffTime,  iProgress_TimeRate;
    int     iProgress_Prev_pct;
  __int64 i64Progress_Prev_Copied, i64_CurrCopied, i64_TotCopied;
    int     iProgress_ETACycle,  iProgress_pending_MB;

    void Out_Progress_Chk(int);

#include "Nav_JUMP.h"



//-----------------------------------------------------------------


// Analyse Action request for Output
// int Out_ACTION() //char P_Act)

DWORD WINAPI Out_ACTION(LPVOID nParam)
{
 int iRC, iResp; //, iOut_Error;
 int i, iTmp1, iFileIx, iLen, iOutMB;
 char szPad[16];

 char szUnMuxStats[1024], *lpszUnMuxStat;
 char cUnit;


  // Actions: '1' = Save current selection without adding into EDL
  //          'L' = Save all clips listed in EDL

  if (DBGflag) DBGout("\nOUT-ACTION *BGN*\n");
  //iCtl_Priority[2] = iCtl_Gui_Priority;

  iOut_Error = 0;  iDeepLook = 0; iDeepFound = 0;
  iIn_Errors = 0;  iIn_AutoResume = 0;
  ZeroMemory(&i64Adjust_TC, sizeof(i64Adjust_TC));
  ZeroMemory(&iBitRate, sizeof(iBitRate));
  ZeroMemory(&iStreamReady, sizeof(iStreamReady));
  
  iGOP_PTS_Chk = 0;
  iProgress_Prev_Time = timeGetTime();
  iProgress_1st_Time  = iProgress_Prev_Time;
  iMsgTime = MAXINT31; 

  i64Progress_Prev_Copied = 0;
  iProgress_ETACycle = 0;
  iProgress_pending_MB  = 0;
  i64_TotCopied = 0;



  // Allocate a much bigger buffer for sequential copying

  lpMpeg_Copy_MALLOC = (BYTE*)malloc(K_8MB+32768+6144);  //)iMpeg_Copy_BufSz) ;
  lpMpeg_Copy_Buffer = (BYTE*)((int)(lpMpeg_Copy_MALLOC + 6144)&0xFFFFF000);  // "insert before" area and aligm to 4k boundary

  if ( !(lpMpeg_Copy_MALLOC)) // || lpMpeg_Copy_Buffer == NULL)
  {
      strcpy(szMsgTxt, "OUTPUT BUFFER ALLOC FAILED !") ;
      iOut_Error = 1;
      DSP1_Main_MSG(0,0);
      UpdateWindow(hWnd_MAIN);
  }
  else
  {

    //lpMPA_C0_save = malloc(K_PKT_SAVE_BUF_LEN);  
    //lpAC3_80_save = malloc(K_PKT_SAVE_BUF_LEN);  

    for ( i = iEDL_ClipFrom ;
        ( i < iEDL_ClipTo  &&  iOut_Error == 0) ;  i++)
    {
       if (Out_CanFlag)
       {
          iOut_Error = 86;
          break;
       }

       if (iOut_FileCreated_Flag <= 0) //  || process.iOut_AutoSPLIT)
       {
          Out_Name_Part_Xtn(-1);
          iRC = Out_File_BEGIN();
          if (iRC < 0)
          {
              iOut_Error++;
              Out_CanFlag = 1;
              break;
          }
       }

       iOut_Error += Out_CLIP(i);

       if (process.iOut_AutoSPLIT)
           OUT_File_END();
    }

    if (!process.iOut_AutoSPLIT)
        OUT_File_END();

    lpszUnMuxStat = &szUnMuxStats[0];

    iTmp1 = MAX_FILE_NUMBER - File_UnMux_Limit;
    if (process.iOutUnMux)
    {
        sprintf(szBuffer, "%d STREAMS\n", iTmp1);
        lpszUnMuxStat = stpcpy0(lpszUnMuxStat, szBuffer);
    }
    else
        szUnMuxStats[0] = 0;

    // Stats for each current output file
    for (iFileIx = MAX_FILE_NUMBER; iFileIx > File_UnMux_Limit; iFileIx--)
    {
      if (process.iOutUnMux)
      {
        iLen = (int)(i64SubStream_Bytes[iFileIx] >>10);
        if (iLen > 9999)
        {
          iLen = iLen >>10;
          cUnit = 'M';
        }
        else
        {
          cUnit = 'K';
        }

        sprintf(szBuffer,   "\n  x%02X = %d %cB",
                            uFileSubStream_Id[iFileIx],
                                       iLen, cUnit);
        lpszUnMuxStat = stpcpy0(lpszUnMuxStat, szBuffer);

      }
    }

    if (process.iOutUnMux)
        lpszUnMuxStat = stpcpy0(lpszUnMuxStat, &"\n");

    if (hProgress)
        PostMessage(hProgress, WM_COMMAND, ((DWORD)(IDM_EXIT)), 0); // IDCANCEL

    iProgress_Prev_Time     = iProgress_1st_Time;

    if (iProgress_Prev_Time > 900) // Don't be too boastful
        iProgress_Prev_Time -=900;

    
    iOutMB   = (int)(i64_TotCopied>>20); // Update for final stats
    i64_CurrCopied = i64_TotCopied;
    i64Progress_Prev_Copied = 0;
    Out_Progress_Chk(-1);

    strcpy(szMsgTxt, "Done.");
    Out_Status_Msg();
    SetWindowText(hWnd_MAIN, "DONE !");
    UpdateWindow(hWnd_MAIN);

    // Check if an error occurred after file started
    if (iOut_Error && iOut_FileCreated_Flag)
    {
      sprintf(szBuffer, "DELETE THE PARTIAL OUTPUT FILE ?\n\n%s", szOutput);
      iResp = MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - CONFIRM", MB_OKCANCEL);
      if (iResp ==1)
      {
          DeleteFile(szOutput);
      }

      iOut_Error += 16;
    }
    else
      process.iOut_Part_Ctr++;


    // On Success - Prompt user for direction
    if ( iOut_Error == 0  &&  iOut_Clip_ctr > 0 )
    {
        if (iOut_Bridged_Flag)
            iOut_Clip_ctr--;

        /*
        if (iOut_Parse_Deep && iOut_Parse_AllPkts
        &&  iDeepLook       && !iDeepFound 
        &&  iOutMB > 5      && !iOut_Resyncs)
            strcpy(szMsgTxt, "\nDeep Parse All Packets - not needed this time\n");
        else
            //sprintf(szMsgTxt, "\n%d Deep Headers in %d Packets\n", iDeepFound, iDeepLook);
        */
            szMsgTxt[0] = 0;

        szTemp[0] = 0;
        if (iOut_PTS_Matching && iCtl_Out_Parse)
          sprintf(szTemp, "Pre-Roll:\n  - 0 Video.  %d Split.\n  -%2d Audio.  %d Split.\nPost-Roll:\n  +%2d Audio %3dKB  %d Split\n  +%2d Video %3dKB  %d Split  %d Seq\n",
                                             iOut_SplitVideo_PrePackets,
                   iOut_SkippedAudioPackets, iOut_SplitAudio_PrePackets,
                   iOut_AddedAudioPackets, ((iOut_AddedAudioBytes+512)/1024), iOut_SplitAudio_PostPackets,
                   iOut_AddedVideoPackets, ((iOut_AddedVideoBytes+512)/1024), iOut_SplitVideo_PostPackets,  
                   iOut_AddedVideoSEQs);

        szDBGln[0] = 0;
        if (iOut_TC_Adjust)
            sprintf(szDBGln, "\n\n%d TC bridges (%ds)\n", iOut_TC_Bridges,
                                           (i64Adjust_TC[0xE0][1]/90000) );
 
         szTmp80[0] = 0;
         if (iOut_Invent_Needed || iOut_Invent_Done || DBGflag)
         {
           sprintf(szTmp80, "\n%d GOPs without PTS. Invented %d\n",
                             iOut_Invent_Needed, iOut_Invent_Done);
         }

         szTMPname[0] = 0;
         if (iOut_UnkPackets || iOut_Resyncs || iFixEdges)
             sprintf(szTMPname,"\n%d Unknown Packets x%02X\n%d Resets   %d Fixes.\n%d weird bytes x%02X\n    Between x%02X and x%02X",
                          iOut_UnkPackets, (unsigned int)(cOut_UnkStreamId),
                                     iOut_Resyncs, iFixEdges, iOut_Vortex_Bytes, 
                                   cOut_RS_Weird, cOut_RS_PrevStreamId, cOut_ResetStreamId);
        
         szTmp256[0] = 0;
         if (iOut_CannotFix_ctr)
         {
           sprintf(szTmp256,"\n%d HDRs could NOT be fixed.",
                          iOut_CannotFix_ctr);
         }
        
         szTmp100[0] = 0;
         if (iOut_Force_Interlace_ctr || iCtl_Out_Force_Interlace)
         {
           sprintf(szTmp100,"\n%d Pics forced to Interlaced.",
                          iOut_Force_Interlace_ctr);
         }
        

        iOut_AddedAudioBytes /= 1000;
        iOut_AddedVideoBytes /= 1000;
        iOutPaddingBytes      = (iOutPaddingBytes + 125000) / (1024*1024);
        //if (iOutPaddingBytes  > 1024)
        {
            //iOutPaddingBytes /= 1024;
            sprintf(szPad, "%d", iOutPaddingBytes);
        }
        //else
        //{
        //    iOutPaddingBytes /= 103;
        //    sprintf(szPad, "0.%03d", iOutPaddingBytes);
        //}

        sprintf(szBuffer, 
         "\nSAVED %d CLIPS  %d MB\n%s%s\n%d sec = %d kB/s\n\nPacket Stats:\n%sChecked %d.    %s MB Padding.%s%s%s%s\n\n*REMOVE* from Edit Decision List ?\n",
                         iOut_Clip_ctr, iOutMB, szUnMuxStats, szMsgTxt,
                        (iProgress_DiffTime/1000), 
                         iProgress_TimeRate,
                            szTemp,
                         iOut_CheckedPackets, szPad, 
                         szTMPname, szDBGln, szTmp80, szTmp256, szTmp100);

        if (iProgress_DiffTime > 9000)
            MessageBeep(MB_OK);

        iRC = MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - CONFIRM", MB_OKCANCEL);

        if (DBGflag)
        {
            DBGout(szBuffer); 
            DBGctl(); DBGctl();
        }

        if (iRC==1 || iRC==IDOK || iRC==IDYES)
        {
          Ed_Prev_EDL_Ctr  = iEDL_ctr ;
          if (cOut_ACT != '1')
          {
              C333_Clip_Clear();
          }
          else
          {
            if (Ed_Prev_Act == '+')
            {
                iEDL_ctr--; iEDL_Chg_Flag = 1;
            }
            Ed_Prev_Act2 = Ed_Prev_Act; Ed_Prev_Act  = '-';
          }
          process.iWarnSize_1 = 0; process.iWarnSize_2 = 0;
          process.iWarnSize_3 = 0; process.iWarnSize_4 = 0;
        }

        if (iCtl_WarnDone)
            SetForegroundWindow(hWnd_MAIN);  // SetFocus(hWnd);
        else
            if (iMainWin_State < 0)
            {
                Sleep(500);
                MessageBeep(MB_OK);
            }
        
        C000_Clip_TOTAL_MB('a');
    }

    //free(lpAC3_80_save);
    //free(lpMPA_C0_save);
    free(lpMpeg_Copy_MALLOC);
  }

  // Restore current browse position for further user selection
   _lseeki64(FileDCB[process.CurrFile],  process.CurrLoc, SEEK_SET);
 //_lseeki64(FileDCB[process.ToPadFile], process.ToPadLoc, SEEK_SET);

  if (iMainWin_State >= 0 && (iViewToolBar & 1))
      T110_Upd_Posn_TrackBar();

  BwdGop.ix   = 0;  BwdGop.iOrg   = 0;
  BwdFast1.ix = 0;  BwdFast1.iOrg = 0;


  //if (MParse.iColorMode != STORE_RGB24)
  //    Chg2YUV2(1);  // Grab overlay back


  Out_ReInstatePicture();

  //Enable_Disable(true, false, true);
  DSP5_Main_FILE_INFO();

  if (iCtl_Out_PostProc)
  {
     if (iCtl_Out_PostShow)
     {
         iRC = DialogBox(hInst,     (LPCTSTR)IDD_POSTPROC,
                         hWnd_MAIN, (DLGPROC)PostProc_Dialog);
     }
     else
       iOut_PostProc_OK = 1;

     if (iOut_PostProc_OK)
     {

         if (iCtl_Out_PostQuote)
             strcpy(szTemp, "\"");
         else
             szTemp[0] = 0;

         sprintf(szBuffer,"%s%s%s %s", szTemp, szOutput, szTemp,
                                               szCtl_Out_ProcLine_B);

         ShellExecute(NULL, "open",   &szCtl_Out_ProcLine_A[0], 
                                    &szBuffer[0],  NULL, SW_SHOWNORMAL);

     }
  }
           
  SetFocus(hWnd_MAIN);

  return 1; // iRC;
}


__int64  i64PTS;



int  Out_Fix_Pack_Hdr(BYTE *P_lpPack, int P_Immediate);

const BYTE  MPEG2PackHdr[]
    =  {0x00, 0x00, 0x01, cPACK_START_CODE, // 01BA sentinel
        0x44, 0x00, 0x04, 0x00, 0x04, 0x01, // Imbedded marker bits with dummy SCR
        0x01, 0x38, 0x83,   // 22bit Max_Mux_Rate (rate_bound); 2b
        0x00};              // reserved;   stuffing length;

const BYTE  MPEG1PackHdr[]
    =  {0x00, 0x00, 0x01, cPACK_START_CODE, // 01BA sentinel
        0x21, 0x00, 0x01, 0x00, 0x01,       // Imbedded marker bits with dummy SCR
        0x80, 0x17,0x71};     // 22bit Max_Mux_Rate (rate_bound); 

const BYTE MPEG2Sentinel[] = {0x00,0x00,0x01};

const BYTE MPEG2SeqEnd[]   = {0x00,0x00,0x01,0xB7};

unsigned char Pack_From_SSCRM[20], Pack_To_SSCRM[20]; //, Pack_Prev_SSCRM[6];


int iPacketLen, iPackAllowance;
unsigned uMpegWord[8];



void Out_MpegEnd()
{

  if (iOut_SystemStream_Flag > 0)
  {
        if (iOutVOB  && MParse.iVOB_Style)
        {
          // Create a new PACK header
          iPacketLen = Out_Fix_Pack_Hdr(&Pack_To_SSCRM[0], 1);

          /*
          if (Mpeg_PES_Version == 2)
             iTmp1 = 10;
          else
             iTmp1 = 9;

          if (iOut_TC_Adjust)
              SCR_2SCRM(        &Pack_To_SSCRM[4],
                        (BYTE*)(&i64Adjust_TC[0xBA][0]),
                                0x44);
          Out_RECORD(&Pack_To_SSCRM[0], iTmp1 , 8121);

          // Mux rate
          if (Mpeg_PES_Version == 2)
              Out_RECORD(&MPEG2PackHdr[10], (sizeof(MPEG2PackHdr)-10), 8112);
          else
              Out_RECORD(&MPEG1PackHdr[9],  (sizeof(MPEG1PackHdr)-9),  8112);
           */
        }

        // Wrap inside the correct video stream packet

        Out_RECORD(&MPEG2Sentinel[0], 3, 8114 ); // 00 00 01

        if (uCtl_Video_Stream >= STREAM_AUTO)
            uMpegWord[0] = (VIDEO_ELEMENTARY_STREAM_1>>24);  // E0
        else
            uMpegWord[0] =  uCtl_Video_Stream>>24;

        uMpegWord[0]  |=  0x000C00;             // SeqEnd_Length=12 bytes
        Out_RECORD(&uMpegWord[0],     3, 8115 );

        uMpegWord[1]  =  0x058080; // Flag bytes, HdrLen (5=PTS)
        Out_RECORD(&uMpegWord[1],     3, 8116 );

        i64PTS = (__int64)(uEDL_TotTime + OrgTC.VideoPTS) <<2;
        PTS_2PTSM((unsigned __int64*)&uMpegWord[2],  &i64PTS, 0x21);

        Out_RECORD(&uMpegWord[1],     5, 8117 );

  }


  Out_RECORD(&MPEG2SeqEnd[0], sizeof(MPEG2SeqEnd), 8118 );

  // NOTE ADD CODE TO COPY THE SYSTEM HEADER HERE (01bb)

  if (iOutVOB && MParse.iVOB_Style)
      Out_Padme(2048 -14 -sizeof(MPEG2SeqEnd) -sizeof(MPEG2PackHdr) );

}

struct
{
    int       FromFile;
    __int64   FromBlk;
    __int64   FromLoc;
    //unsigned  FromPTS, FromPTSM;

    unsigned  uFrom_TCorrection;
    unsigned  uFrom_FPeriod_ps;

    int       ToFile;
    __int64   ToBlk;
    __int64   ToLoc;
    unsigned  ToPTS;
    unsigned  ToPTSM;

} W_Clip;


//---------------------------------------------------
// Output a selected clip from the Edit Decision List


int Out_CLIP(int P_ClipNo)
{

  int  iOffset, iRC ;
  __int64 StartPos, EndPos;
  __int64 i64Preamble_Out, i64PreMinimum;


  //W_Clip.FromPTS  = EDList.FromPTS [P_ClipNo] ;
  //W_Clip.FromPTSM = EDList.FromPTSM[P_ClipNo] ;

  W_Clip.FromFile = EDList.FromFile[P_ClipNo] ;
  W_Clip.FromLoc  = EDList.FromLoc [P_ClipNo] ;

  W_Clip.uFrom_TCorrection = EDList.uFrom_TCorrection[P_ClipNo];
  W_Clip.uFrom_FPeriod_ps  = EDList.uFrom_FPeriod_ps [P_ClipNo];

  W_Clip.ToFile = EDList.ToViewFile [P_ClipNo] ;
  W_Clip.ToLoc  = EDList.ToViewLoc  [P_ClipNo] ;
  W_Clip.ToPTS  = EDList.ToViewPTS  [P_ClipNo] ;
  W_Clip.ToPTSM = EDList.ToViewPTSM [P_ClipNo] ;


  iPackAllowance = 2048;
  
  if (iCtl_To_Pad) // Option to grab extra video frame
  {
      W_Clip.ToFile = EDList.ToPadFile [P_ClipNo] ;
      W_Clip.ToLoc  = EDList.ToPadLoc  [P_ClipNo] ;
      W_Clip.ToPTS  = EDList.ToPadPTS  [P_ClipNo] ;
      W_Clip.ToPTSM = EDList.ToPadPTSM [P_ClipNo] ;

      if (process.Pack_Max_Size < MPEG_SEARCH_BUFSZ) // Allow for under-estimate
          iPackAllowance = MPEG_SEARCH_BUFSZ; 
      else
      if (process.Pack_Max_Size < iMpeg_Copy_BufSz)
          iPackAllowance = process.Pack_Max_Size; // Allow for TwinHan - 1 huge pack per GOP]
      else
          iPackAllowance = iMpeg_Copy_BufSz;     
  }
  else
  {
      iPackAllowance = MPEG_SEARCH_BUFSZ; // *2;
  }


  if (DBGflag)
  {
      sprintf(szBuffer,"\n** CLIP#%d **\n\n", P_ClipNo);
      DBGout(szBuffer);
  }


  i64_CurrCopied  = 0;  // Initialize at start of current clip



  // Convert vague block pointers into Pack specific byte pointers


  //switch (Loc_Method)
  //{              
  //  case 2:   // The new method based on stored pointers to packets

      StartPos =   W_Clip.FromLoc ;
      if (StartPos < 0) 
          StartPos = 0;

      EndPos   =   W_Clip.ToLoc;

      // patch for bug in TO padding calculation
      //if (iCtl_To_Pad) EndPos = EndPos + (MPEG_SEARCH_BUFSZ * 3); // ALLOW FOR BUG

   //   break;

      // Original method based on block pointers
      // Does not allow for multiple PACKS (c.2k) in the one block (10k+)
   // default:
   //     StartPos = ( W_Clip.FromBlk    * MPEG_SEARCH_BUFSZ)
   //                - process.origin[W_Clip.FromFile];
   //     EndPos   = ((W_Clip.ToBlk + 2) * MPEG_SEARCH_BUFSZ)
   //                - process.origin[W_Clip.ToFile];
   //   break;
  //}


      // Since neither method is completely accurate,
      // search for the appropriate Pack marker

  // Default Pack Header in case none found
  if (Mpeg_PES_Version == 2)
  {
      memcpy(&Pack_From_SSCRM[0], &MPEG2PackHdr, 20);
  }
  else
  {
      memcpy(&Pack_From_SSCRM[0], &MPEG1PackHdr, 12);
  }


  // First for the Selection START Point
  iOffset = Out_Find_Sentinel(W_Clip.FromFile, 
                                     StartPos, MPEG_SEARCH_BUFSZ*2,
                                               dwStartSentinel, 1);

  if (DBGflag) 
  {
    sprintf(szBuffer, "\nFrom Offset %d\n", iOffset);
    DBGout(szBuffer);
  }

  if (iOffset != -255)
  {
      StartPos = StartPos + iOffset;
      if (iCtl_Out_Preamble_Flag==1) // MAY NEED TO COPY TIME STAMP Later
          memcpy(&Pack_From_SSCRM[0], (lpMpeg_Copy_Buffer+iOffset), 20);
  }
  memcpy(&Pack_To_SSCRM[0], &Pack_From_SSCRM[0], 20);

  //int iTmp1 ;
  //iTmp1 = (int)(StartPos);


  // Then for the Selection END Point
  if (EndPos < process.length[W_Clip.ToFile] - 8)
  {
     iOffset = Out_Find_Sentinel(W_Clip.ToFile, EndPos, iPackAllowance,   //  (iMpeg_Copy_BufSz/8),
                                              dwEndSentinel, 2);

     if (iOffset == -255)
         EndPos = EndPos + MPEG_SEARCH_BUFSZ;
     else
     {
         EndPos = EndPos + iOffset;
         if (iCtl_Out_Preamble_Flag==1) // Update the default END info
             memcpy(&Pack_To_SSCRM[0], (lpMpeg_Copy_Buffer+iOffset), 20);
     }
  }
  else
    EndPos = process.length[W_Clip.ToFile];

  //RJDBG
  if (DBGflag) DBGln4(
      "\nCalcs. preamble=%x  FromLoc=x%04X StartPack=x%04X EndPos=x%04X\n",
        process.preamble_len,  W_Clip.FromLoc, StartPos, EndPos);

  // Optionally copy the preamble of the first file
  // (Assumes preamble is fully within the first file)

  if (iCtl_Out_Preamble_Flag > 0  &&  iPS_Block1 > 0  // First Block of First Clip
  &&  iOut_SystemStream_Flag > 0)
  {
    // NOT MUCH LEFT INSIDE THIS SITUATION - TODO: TRIM DOWN CODE 
    switch (iCtl_Out_Preamble_Flag)
    {
    case 1:  // Only copy Headers = first pack at start of first file

        if (!process.iPreamblePackAtStart // Is it invalid start ?
        ||  (    process.Preamble_SysHdr_Found // Is it already provided for ?
             && !process.iSEQHDR_NEEDED_clip1) // No extra work needed ?
        )
        {
          i64Preamble_Out = 0;
          break;
        }

        if (iOutVOB && MParse.iVOB_Style)
          i64PreMinimum = 2000;
        else
          i64PreMinimum = 4;
        i64Preamble_Out = Out_Find_Sentinel(0, i64PreMinimum, 32768,      // (iMpeg_Copy_BufSz/8),
                                            uVIDPKT_STREAM_1, // dwStartSentinel,
                                            0) 
                          + 4;  // Length of first pack = distance to next start of pack
        if (i64Preamble_Out < process.preamble_len) // Truncate preamble to one pack
            break;

    default: // Copy everything up to first SEQ HDR's pack
        i64Preamble_Out = process.preamble_len; // Good for VOBs
        break;
    }

    if (W_Clip.FromFile == 0
    && StartPos <= (i64Preamble_Out + 32) )
    {
       if (DBGflag) DBGout("\n*** BRIDGING PREAMBLE ***\n");
       StartPos = 0;
       iOut_Clip_ctr++;  iOut_Bridged_Flag = 1;
    }
    else
    {
       if (i64Preamble_Out > 0)
       {
           if (DBGflag) DBGout("\nSeparate Preamble\n");
           Out_RANGE(0, 0, 0, i64Preamble_Out, 'P');
       }
    }
  }

  // Now copy selection using EXACT position of FROM and TO markers
  iRC = Out_RANGE(W_Clip.FromFile, StartPos,
                  W_Clip.ToFile,   EndPos, 0);

  // Optionally insert Mpeg SEQUENCE END code at end of clip

  if (iCtl_Out_Seq_End && P_ClipNo < iEDL_ClipTo 
      // && Mpeg_PES_Version == 2
      )
  {
     Out_MpegEnd();
  }

  // Optionally reset for SysHdr every time, BUT NOT PREAMBLE

  if (iOut_SystemStream_Flag != 0)
  {
      if (iCtl_Out_SysHdr_EveryClip || process.iOut_AutoSPLIT)
          iPS_Block1 = process.iOut_AutoSPLIT + 1;
  }

  return iRC;

}



//---------------------------------------------------------------------
// Search from a given position in one of the file
// to locate the address of the first PACK hdr found from that point
// NOTE:- For elementary video, uses SEQ HDR rather than PACK HDR

/* static ? */ 
int Out_Find_Sentinel(int P_FileNum, __int64 P_Loc, 
                      int P_Allowance, DWORD P_Sentinel,
                      int P_Mode)
{
  char *szMode[3] = {"PRE", "BGN", "END"};
  
  __int64  i64_RC, W_Loc ;
  int      iMpeg_Read_Len, iOffset, iPktOffset;
  BYTE *lpGzinta, *lpEND;

  DWORD dwSENT_1; //, dwSENT_2;

  register unsigned int uTST;
  register unsigned char *lpScan_ix, *lpScan_End;

  if (DBGflag) DBGout("\nOut_Find_Sentinel STARTED\n"); // Blknum=%d\n", P_BlkNum);}

  iPktOffset = -1;

  dwSENT_1 = P_Sentinel;
  // seek to approximately where the mark points to

  W_Loc = P_Loc ;// - 16;
  //if (W_Loc < 0) W_Loc = 0;


  // Allow for padding pointing past EOF
  if (P_Loc >= process.length[P_FileNum] - 4)
    return 0;

  lpGzinta = lpMpeg_Copy_Buffer;
  lpEND    = lpMpeg_Copy_Buffer + iMpeg_Copy_BufSz - P_Allowance;

  i64_RC = _lseeki64(FileDCB[P_FileNum],
                     W_Loc,
                  // P_BlkNum * MPEG_SEARCH_BUFSZ,
                     SEEK_SET) ;

  if (i64_RC < 0 ) // == -1L)  // RJ What about other bad RC value ?
  {
    //  ERRMSG_FileXA("MpegIn", 's', i64_RC, File_Name[P_FileNum]);
    ERRMSG_File("MpegIn", 's', (int)(i64_RC), File_Name[P_FileNum],
                                                         0, 8803);
    iMpeg_Read_Len = 0;
  }
  else
  {
    // read a block of data
ReadSome:
    iMpeg_Read_Len = Mpeg_READ_Buff(P_FileNum,  // _read(FileDCB[P_FileNum], 
                                   lpGzinta,  P_Allowance, 8866);

    lpGzinta += iMpeg_Read_Len;

    if (iMpeg_Read_Len < 8) // P_Allowance)
    {
        // Small block - leave pointer at default

        // sprintf(szBuffer, "MARK VERY NEAR EOF. Len=%d",  iMpeg_Read_Len ) ;
        // MessageBox(hWnd, szBuffer, szAppName, MB_ICONSTOP | MB_OK);
        // if (iMpeg_Read_Len < 0 
        // ||  P_Mode != 2)
        //    iMpeg_Read_Len = 0;
        // return iMpeg_Read_Len;

    }
    else
    {
      if (MParse.SystemStream_Flag < 0) // Transmission stream ?
      {
        if (MParse.SystemStream_Flag == -1) // Transport stream ?
        {
         
           lpScan_End = lpMpeg_Copy_Buffer + iMpeg_Read_Len - 4;
           for (lpScan_ix  = lpMpeg_Copy_Buffer;
                lpScan_ix < lpScan_End;
                lpScan_ix++)
           {
             if (*lpScan_ix == 0x47)                                             
             {
               // set the EXACT position of the TS PACK marker and RETURN
               iOffset = lpScan_ix - lpMpeg_Copy_Buffer; // - 16;
               return iOffset ;
             } // endif
           } //endfor
        }
        else
        {                                               // PVA Stream
          lpScan_End = lpMpeg_Copy_Buffer + iMpeg_Read_Len - 4;
          for (lpScan_ix  = lpMpeg_Copy_Buffer;
               lpScan_ix < lpScan_End;
               lpScan_ix++)
          {
             if (*(UNALIGNED short*)(lpScan_ix) == 0x5641) // 'AV'                                          
             {
                 // set the EXACT position of the PVA PACK marker and RETURN
                 iOffset = lpScan_ix - lpMpeg_Copy_Buffer; // - 16;
                 return iOffset ;
             } // endif
          } //endfor
        }
      }
      else
      {
        // Find a Start Of Pack/Seq/pkt marker in the selected block
        lpScan_End = lpMpeg_Copy_Buffer + iMpeg_Read_Len - 4;
        for (lpScan_ix  = lpMpeg_Copy_Buffer;
             lpScan_ix < lpScan_End;
             lpScan_ix++)
        {
          if (!*lpScan_ix) // NULL
          {
            uTST = *(UNALIGNED DWORD*)(lpScan_ix);  // ((UNALIGNED DWORD*)(lpMpeg_Copy_Buffer+ix))

            if (uTST  == dwSENT_1
            ||  uTST  == uPACK_START_CODE
            ||  uTST  == uVIDPKT_STREAM_1
               )
                                             /*PS=PACK  ES=SEQ PAD=PIC*/
            {
               // set the EXACT position of the hdr sart code and RETURN
               iOffset = lpScan_ix - lpMpeg_Copy_Buffer; // - 16;

               //RJDBG
               if (DBGflag)
               {
                  DBGln2("Out PACK HDR OK ix=%x.  ParmLoc=%x\n",
                        iOffset, W_Loc);
               }

               return iOffset ;

            } // endif Good Match

          } // ENDIF NULL
        } //endfor
      } // endelse SystemStreamFlag

      if (lpGzinta < lpEND)
        goto ReadSome;
    } // endelse iMpeg_Read_Len
    
  } //endelse iSeek

  //if (iMpeg_Read_Len < P_Allowance)
  //{
    // if (P_Mode != 2)
    //     iOffset = 0;
    // else
    //     iOffset = iMpeg_Read_Len;
  //}
  //else
  {
    iTmp1 = (int) W_Loc ;
    sprintf(szBuffer,
            "%s %s HDR NOT FOUND.\n\nGot x%08X\nNot x%08X\n\nFile#%d Loc x%X ",
             szMode[P_Mode], szStartDesc,
                         *((UNALIGNED DWORD*)(lpMpeg_Copy_Buffer)),
                               P_Sentinel,     P_FileNum, iTmp1) ;
          MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - BUG !",
                                                    MB_ICONSTOP | MB_OK);
          if (DBGflag) DBGout(szBuffer)  ;

  }

  iOffset = -255;
  return iOffset ;
}
 

#include "Out_Clip.c"


// Moved all the output parsing code into separate module
#include "Out_PKTS.c"



//----------------------------------------------------------



unsigned int uTick[3];

int iNew_Cat, iNew_Fmt;

void TRKS_Category_Set(HWND hDialog, int iCat, int iFmt)
{
  iNew_Cat = iCat;

  uTick[0] = BST_UNCHECKED;
  uTick[1] = BST_UNCHECKED;
  //uTick[2] = BST_UNCHECKED;

  uTick[iCat] = BST_CHECKED;

  SendDlgItemMessage(hDialog, IDU_UNMUX_ALL,        BM_SETCHECK, 
                                                           uTick[0], 0);
  SendDlgItemMessage(hDialog, IDU_UNMUX_AUDIO_ONLY, BM_SETCHECK, 
                                                           uTick[1], 0);

//}
//void TRKS_Format_Set(HWND hDialog, int iFmt)
//{


  iNew_Fmt = iFmt;
  uTick[0] = BST_UNCHECKED;
  uTick[1] = BST_UNCHECKED;
  uTick[2] = BST_UNCHECKED;

  uTick[iFmt] = BST_CHECKED;


  SendDlgItemMessage(hDialog, IDU_UNMUX_FMT_RAW,    BM_SETCHECK, 
                                                           uTick[0], 0);
  SendDlgItemMessage(hDialog, IDU_UNMUX_FMT_PES,    BM_SETCHECK, 
                                                           uTick[1], 0);
  SendDlgItemMessage(hDialog, IDU_UNMUX_FMT_RIFF,   BM_SETCHECK, 
                                                           uTick[2], 0);
}







//----------------------------------------------------------------
LRESULT CALLBACK UnMuxOption_Dialog(HWND hDialog,  UINT message,
                                  WPARAM wParam, LPARAM lParam)
{

  int iEnough;

  iEnough = 0;

  switch (message)
  {
     case WM_INITDIALOG:

         TRKS_Category_Set(hDialog, iOut_UnMuxAudioOnly, iOut_UnMux_Fmt);

         //TRKS_Format_Set(hDialog, iOut_UnMux_Fmt);

         iOut_UnMux_Cancel = 0;
         
         ShowWindow(hDialog, SW_SHOW);
         iEnough = 1;
         break;




     case WM_COMMAND:
        switch (LOWORD(wParam))
        {

          case IDU_UNMUX_AUDIO_ONLY:
               TRKS_Category_Set(hDialog, 1, iNew_Fmt);
          break;
          case IDU_UNMUX_ALL:
               TRKS_Category_Set(hDialog, 0, iNew_Fmt);
          break;
          case IDU_UNMUX_SELECTIVE_AUDIO_ONLY:
               TRKS_Category_Set(hDialog, -1, iNew_Fmt);
          break;


          case IDU_UNMUX_FMT_RAW:
               TRKS_Category_Set(hDialog, iNew_Cat, 0);
          break;
          case IDU_UNMUX_FMT_PES:
               TRKS_Category_Set(hDialog, iNew_Cat, 1);
          break;
          case IDU_UNMUX_FMT_RIFF:
               TRKS_Category_Set(hDialog, iNew_Cat, 2);
          break;


          case IDOK:

               iOut_UnMuxAudioOnly = iNew_Cat;
               iOut_UnMux_Fmt      = iNew_Fmt;

               if (iOut_UnMuxAudioOnly >= 0)
                   iOut_Audio_All = 1;

               /*
               if (SendDlgItemMessage(hDialog,
                       IDU_UNMUX_AUDIO_ONLY, BM_GETCHECK, 1, 0) == BST_CHECKED)
                   iOut_UnMuxAudioOnly = 1;
               else
                   iOut_UnMuxAudioOnly = 0;

               if (SendDlgItemMessage(hDialog,
                       IDU_UNMUX_FMT_RIFF, BM_GETCHECK, 1, 0) == BST_CHECKED)
                   iOut_UnMux_Fmt = 2;
               else
               if (SendDlgItemMessage(hDialog,
                       IDU_UNMUX_FMT_PES,  BM_GETCHECK, 1, 0) == BST_CHECKED)
                   iOut_UnMux_Fmt = 1;
               else
                   iOut_UnMux_Fmt = 0;
               */

               iEnough = 2;
               break;


          case IDCANCEL:

               if ( ! iOut_UnMux_Cancel)
               {
                   iOut_UnMux_Cancel = 1;
               }

              iEnough = 2;
         }

         break;

   }

  // KILL ?
  if (iEnough > 1)
  {
      //DestroyWindow(hDialog);
      EndDialog(hDialog, iEnough);
      iEnough = 2;
  }

  return iEnough;
}



//----------------------------------------------------------


void Out_Progress_Chk(int P_Show)
{
  int iCurr_pct, iCum_pct, iCurrMB, iOutMB, iSanityLimit;
  int iFudgeClipMB, iFudgeTotMB;
  __int64 i64DiffBytes, i64ErrOffset, i64FudgeClipTotal;

  int iTmp2, iTmp3; // iTmp1

  __int64 i64Tmp1;

  iProgress_pending_MB = 0;

  if (iOut_UnMuxAudioOnly)
  {
      i64FudgeClipTotal = i64_ClipTotBytes / 20;
      iFudgeClipMB      = iClipMB / 20;
      iFudgeTotMB       = iEDL_TotMB / 20;
  }
  else
  {
      i64FudgeClipTotal = i64_ClipTotBytes;
      iFudgeClipMB      = iClipMB;
      iFudgeTotMB       = iEDL_TotMB;
  }

  if (i64FudgeClipTotal)
  {

      iCurr_pct = (int)(i64_CurrCopied * 100 / i64FudgeClipTotal);
  }
  else
      iCurr_pct = 0;

  // maybe time to refresh progress info

  iProgress_New_Time = timeGetTime();

  //gettime(&TimeSYS);
  //iProgress_New_Time = (((TimeSYS.ti_hour * 60
  //          + TimeSYS.ti_min) * 60
  //          + TimeSYS.ti_sec) * 100
  //          + TimeSYS.ti_hund) * 10;

  iProgress_DiffTime  = iProgress_New_Time   - iProgress_Prev_Time;
  i64DiffBytes = (i64_CurrCopied - i64Progress_Prev_Copied);

  if (iProgress_DiffTime < 0)
      iProgress_DiffTime += 86400000; // Allow for Midnight crossing
  iProgress_DiffTime++;

  if (iProgress_DiffTime > 1000         // Info every 1 seconds
  || (iCurr_pct != iProgress_Prev_pct && iProgress_DiffTime > 500)  // Info every percent, within reason
  ||  ! P_Show)                         // Final calc
  {
      iProgress_TimeRate = (int)(i64DiffBytes / (__int64)(iProgress_DiffTime)); //  only count write activity, not read overhead

      if (cpu.sse2) // P4 ?    SATA on P4 can be way faster than ATA on P3
      {
        if (winVer.dwMajorVersion >= 7)  // Later OS probably run on very fast machine
            iSanityLimit = 60000; 
        else
        if (winVer.dwMajorVersion >= 6)  // Vista and later probably run on a fast machine
            iSanityLimit = 50000; 
        else
            iSanityLimit = 40000; 
      }
      else // Probably P3 ATA
      if (iCtl_Priority[2] == PRIORITY_HIGH)
        iSanityLimit = 18000;
      else
        iSanityLimit = 15000;

      // Trap silly rates figures
      if (iProgress_TimeRate > iSanityLimit  && P_Show > 0)
          iProgress_TimeRate = iSanityLimit;

      iProgress_Prev_Time     = iProgress_New_Time;
      i64Progress_Prev_Copied = i64_CurrCopied;

      iCurrMB = (int)(i64_CurrCopied>>20);  // binary divide by 1MB = / 1048576) ;
      iOutMB  = (int)(i64_TotCopied >>20);   

      if (P_Show > 0)
      {
          sprintf(szMsgTxt,
               "Clip #%d  %d MB / %d MB clip. %d %%... %d kB/s ",
                  iOut_Clip_ctr,
                         iCurrMB, iFudgeClipMB,
                         iCurr_pct, 
                         iProgress_TimeRate);
          if (DBGflag) DBGout(szMsgTxt);
          //Out_Status_Msg();
          SetDlgItemText(hProgress, IDC_PROGRESS_TXT, szMsgTxt);
          SendMessage(hBar, PBM_SETPOS,  iCurr_pct, 0);


          iCum_pct = (iOutMB * 100 / (iFudgeTotMB + 1));

          if (iOutMB < 5)
              iOut_Rate = iProgress_TimeRate * 2 / 3;

          iProgress_ETACycle++;
          if (iProgress_ETACycle > 3 && iOutMB > 8)
          {
              iProgress_ETACycle = 0;
              iOut_Rate = (iOut_Rate * 10 + iProgress_TimeRate) / 11; // Weighted average kB/s
              if (iOut_Rate > 0)
              {
                 // Estimate time left
                 iTmp2 = ((iFudgeTotMB - iOutMB) * 1024 / iOut_Rate) + 1; // How many seconds left
                 iTmp3 = iTmp2 / 60; // Minutes
                 if (iTmp3)
                 {
                     iTmp2 = iTmp2 - (iTmp3 * 60);
                 }

                 if (iIn_Errors)
                   sprintf(szTmp32, "%d ERRORS", iIn_Errors);
                 else
                   szTmp32[0] = 0;

                 sprintf(szBuffer, "%1dm %2ds left.%s", //   %dkBps avg%c",
                                         iTmp3, iTmp2, szTmp32); // , iOut_Rate, ' ');
                 SetDlgItemText(hProgress, IDP_PROGRESS_ETA, szBuffer);
                 
                 // show time remaining on the task icon
                 SetWindowText(hWnd_MAIN, szBuffer);
                 UpdateWindow(hWnd_MAIN);

                 if ((iTmp3 < 0 || iTmp2 < 10)  // Allow for 10second margin for unplanned padding due to trouble finding END PACK HDR
                 && !iOut_UnMuxAudioOnly)
                 {
                    if (iCurrFile == W_Clip.ToFile)
                    {
                      i64Tmp1 = _telli64(FileDCB[iCurrFile]);
                      i64ErrOffset =  i64Tmp1 - (W_Clip.ToLoc);
                    }
                    else
                    {
                      i64ErrOffset =  0;
                    }
                    sprintf(szBuffer,
                        "CLIP BOUNDARY ERROR.\n\nOverrun = %d K %d\n\nFile:  %d Pos x%08X %x\nFrom %d Pos x%08X %x\n-- To %d Pos x%08X %x",
                             i64ErrOffset/1024, 
                             iCurrFile, i64Tmp1, 
                             W_Clip.FromFile,  W_Clip.FromLoc, 
                             W_Clip.ToFile,    W_Clip.ToLoc);

                    if (i64ErrOffset > 1024000)
                    {
                       MessageBox(hWnd_MAIN, szBuffer, "Mpg2Cut2 - BUG !",
                                                      MB_ICONSTOP | MB_OK);
                    }

                    if (DBGflag)
                        DBGout(szBuffer);
                 }
              }
          }

          sprintf(szBuffer,
               "Overall  %d MB written of %d MB total. %d %%", // .... %d kB/s  ",
                             iOutMB,     iFudgeTotMB,  iCum_pct);
          if (DBGflag) DBGout(szBuffer)  ;
          SetDlgItemText(hProgress,
                         // hDlg,
                         IDC_PROGRESS_TXT2, szBuffer);
          SendMessage(hBar2, PBM_SETPOS, iCum_pct, 0);

          EnableWindow(hCancel, true) ;
          iProgress_Prev_pct = iCurr_pct;

      } // ENDIF  P_SHOW > 0

  } // ENDIF Time for a refresh of the info

}



