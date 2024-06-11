    
   
// MPEGCUT2 Ver 2 based on Dark Avengers's Ver 1.15

// Ver 2.x mods by RJ (RocketJet) :-    
  
// HISTORY 
// Location method changed from Block Address to Byte Address
//     Original block method could get wrong "FROM" and "TO" points
//     even on moderate blocksize
//     Plus it skipped over some interesting control info at start of file

// Allow more formats as input.
//   - allow Mpeg-1 files.
//   - allow CDXA RIFF-Wrapped Mpeg files.
//   - allow Transport streams (Needs more work)
//   - allow PVA streams (Needs more work)
// 
// Tarted up output file format:
//   - Allow for NAV Packs inside VOB files.
//   - Copy/Fix control information (PREAMBLE)  from start of  input file.
//   - Warn user if file starts mid-gop.
//     Allow choice to skip or keep crud.
//     Useful if working with output from some DTV cards.
//   - Match audio timestamps against video FROM-TO timestamps.
//   - Allow padding of TO point to include next key-frame.
//   - Allow user to pause/cancel/slow while Saving Output.
//   - Allow Demuxed output.
//   - Adjust Timestamps when joining multiple clips (SCR,PTS,DTS,GOP-TC)
//            This is still a too buggy.

// Tarted up gui a little bit:
//   - Ensure Control Bar visible when canvas > screen (now near top)
//   - Accept filename passed from Windows Explorer
//   - Allows keys a little more like VirtualDub
//   - Reset "out" to EOF when "in" follows previous out
//   - Allow for MULTIPLE ranges (Needs more work)
//   - More Toolbar Buttons.
//   - Aspect Ratio honoured in YUY2 display
//   - DVD2AVI 1.77 Gamma adjusment for people like me with crappy ATI cards
//   - Zoom Out if Mpeg canvas much larger than screen. (HDTV).
//   - Snapshot changes :
//      . If display is YUY2, autoconvert data to RGB for BMP
//      . De-Interlace BMP using SHIFT-B (or optionally default)
//         This will separate out the 2 fields of the frame.
//      . [B] button flashes once to let you know the snapshot worked.
//      . BMP file names based on Input file name & Frame Time Stamp
//      . User defined BMP folder
//      . HQ Aspect Ratio correction.  Bi-cubic.
//   - Make a sound if a request cannot be actioned
//   - Allow scroll key to reach the end, and to RETREAT !
//   - User defined default OUTPUT folder
//   - More info on stats screen
//   - Release Direct Draw Overlay when minimized.


//   - Performance:
//      . Less video buffer shuffling to convert to YUY format.
//      . Skip frames if behind allows playback of HD on sub-GHz CPU
//      . Luminance adjustment no longer double handles, in YUY2 mode.
//      . Increased Mpeg search buffer Size to 32K. (Holy IBM-360 Batman !)
//      . Misc other changes

// - MPEG Decoder
//   . Fault Tolerance significantly increased.
//   . GopBack - somewhat more rugged now.
//   . Mpeg-1 viewing reinstated.
//   . Transport Stream & PVA viewing & cutting - needs more work.

//   - Audio Decoder:
//      . Postpone loading until needed
//      . More info if MPAlib.dll does not initialize
//      . Allow option to load other decoders instead of MPAlib
//      . Trapped misc crashes arising from interface problems.
//      . Volume boost for low-level sound.
//      . Partial support for Audio in Private Stream 2.
//      . Slow/Fast Playback.

//   - Edit Decision List extensions:
//     . Save and restore EDL.
//     . Preview entire EDL, with summary option.

// TODO :-

//   - Time Stamp adjustment needs more work.
//     Also extend to be able to fix up Ch.7 weird SCRs
//     This is the most important TODO,
//     but it is a lot easier said than done !

//   - Edit Decision List extensions:
//     . Show range of previous selections in EDL
//     . Allow Modify of previous edit decisions before saving.

//   - Replace missing Audio or Video frames with dummies
//      Especially useful with files from DTV cards. (Grrr! Argh!)


//  - Gradually cleaning up source code :
//      - Moving towards clean compiles
//      - Modularization - Barely begun
//      - Structured variable names - Barely begun
 


// Known BUGS :(
//   - Sometimes I-Frame does not display when skipping Backwards
//
// Old Bugs :-
//   - Bad MPEG data can crash Assembler routines
//                Especially getpic.c @mc0..5
//   - MPALIB
//     . does NOT decode MONO properly.
//     . Occasional Storage violations.

// Further documentation on changes are on the website.
//===================================================================



// MPEGCUT is derived from Kuo's DVD2AVI and MSSG's MPEG2DEC


// *  MPEG2DEC - Copyright (C) Mpeg Software Simulation Group 1996-99
// *  DVD2AVI  - Copyright (C) Chia-chen Kuo - April 2001
// *  Mpeg2Cut - Dark Avenger and others
// *  Mpg2Cut2 - RocketJet & various other authors
// *             GNU GPL Ver3 or later.
  

// *
// *  DVD2AVI - a free MPEG-2 converter
// *
// *   DVD2AVI is free software; you can redistribute it and/or modify
// *   it under the terms of the GNU General Public License as published by
// *   the Free Software Foundation; either version 2, or (at your option)
// *   any later version.
// *
// *   DVD2AVI is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// *   GNU General Public License for more details.
// *
// *   You should have received a copy of the GNU General Public License
// *   along with GNU Make; see the file COPYING.   If not, write to
// *   the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
// *


// SSE2 code by Dmitry Rozhdestvensky



#include "windows.h"

#define GLOBAL
extern "C"
{
#include "TXT.h"
#include "global.h"
#include <commctrl.h>
#include <fcntl.h>

#include "Gui_Comm.h"
    
#include "Audio.h"
#include "MPA_DA.h"
#include "wave_out.h"
#include "Wave_CS.h"

#include "AC3Dec\A53_interface.h"
#include "AC3Dec\ac3.h"

#include "DDRAW_CTL.h"
#include <direct.h>
#include <vfw.h>
 
#include "MPV_PIC.h" 
#include "mpalib.h"
#include "mpalib_more.h"

#include "PIC_BUF.h"

#include "getbit.h"
#include "GetBlk.h"
#include "out.h"
#include "Nav_JUMP.h"
#include "Buttons.h"

#include "PLUG.h"
#include "Mpg2Cut2_API.h"
}

#include "errno.h"


#define OIC_BANG            32515
#define OBM_ZOOM            32748
//#define OEMRESOURCE
//#include "winuser.h"


//#define InitWIDTH    480
//#define INIT_HEIGHT    240

#define MAX_LOADSTRING      100

DWORD SysErrNum;

ATOM MyRegisterClass(HINSTANCE);


void B500_CONTINUE_PLAY_HERE();
void B501_Play_Button(int);
void B510_PLAY_HERE();
void B150_PLAY_FASTER(DWORD);
void B153_Fast_Msg();
void B160_PLAY_SLOW(DWORD);
void B550_PLAY(int P_Mode);

static void B400_Key_USER(WPARAM) ;
LRESULT  B200_Msg_USER_API(UINT message, WPARAM wParam, LPARAM lParam);
LRESULT  B201_Msg_USER    (UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK Preferences_Dialog(HWND hPrefDlg, UINT message, WPARAM wParam, LPARAM lParam);

static void B910_Main_INIT(HWND, /* UINT, */ WPARAM, LPARAM) ;
void B380_Volume_Window();

RECT iPrevWinPos;

void VolBoostChg()
{
  if (iCtl_Volume_Boost)
      VOL303_Vol_Boost_On();
  else
      VOL304_Vol_Boost_Off();
}


static char MPA_NAME[5][13]
    = {" ", "MPAlib.dll", "MPA_MMX.dll", "MPA_SSE1.dll", "MPA_SSE2.dll"};

//static


int iTmp4;
char *ext;

unsigned int uLastHelpTime;
unsigned int uTmp1, uTmp2;

/*
static void D2Vsave();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Statistics(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AudioList(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK Delay(HWND, UINT, WPARAM, LPARAM);
*/
LRESULT CALLBACK ClipResize(HWND, UINT, WPARAM, LPARAM);
//LRESULT CALLBACK Normalization(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);


void Set_Frame_Rate(int);
void Set_Video_Stream(const UINT, const UINT);
void Set_VPIDs(const UINT, const UINT);
void Set_OutFolderMode(const unsigned P_Menu_ix);

void Set_Menu_Array(int P_New_ix, int *P_Menu_Fld);

int DRC_MenuId[] = {IDM_DRC_NONE,  IDM_DRC_LIGHT, IDM_DRC_NORMAL,
                    IDM_DRC_HEAVY, IDM_DRC_VERYHEAVY, 0};


void IDCT_SetHardware(int, unsigned);



static void X100_INIT(HINSTANCE, LPSTR );
static void CheckFlag(void);
static void OpenAudioFile(HWND);
DWORD WINAPI ProcessWAV(LPVOID n);

OPENFILENAME sfn;
int iTemp[32];

int OLDParmConfirm, OLDiCtl_WarnBadStart;

char szPath[_MAX_PATH];
char szWindowClass[MAX_LOADSTRING];

// HWND hClipResizeDlg, hNormDlg;

int SoundDelay[MAX_FILE_NUMBER];




//----------------------------------------------------


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow)
{
   MSG msg;
   HACCEL hAccel; 
   // int iTmp1, iTmp2;
   // DWORD dRC; 

   // Initialize global strings
   LoadString(hInstance, IDC_GUI, szWindowClass, MAX_LOADSTRING);
   //MyRegisterClass(hInstance);

   // Perform application initialization
   hInst = hInstance;
   hWnd_MAIN = NULL;

// Initialise controls

   X100_INIT(hInstance, lpCmdLine);

   iBMP_BufSize = 0;
   DSP_Button_Abbr();

   // populate child windows

   //hOpenButton = CreateWindow("BUTTON", "O",
   //                     WS_CHILD | WS_VISIBLE | WS_DLGFRAME,
   //                     0, 0,
   //                     iTool_Ht, iTool_Ht,
   //                     hWnd, (HMENU) IDM_OPEN,
   //                     hInst, NULL);

   ToolBar_Create(); 

   ShowWindow(hWnd_MAIN, nCmdShow) ;
   //wCmdShow = nCmdShow;

   // Load accelerators
   hAccel = LoadAccelerators(hInstance, (LPCTSTR)IDR_ACCELERATOR);

   CheckFlag();
 
   GetBlk_AHD_INIT();  // Allocate File Read Buffers

   // Open file name/s passed via Parm area
   if (cPassed1 > ' ')
   {
     ext = lpDOT(szInput);
     if (! strnicmp(ext, &".EDL", 4) ) // EDL files get different treatment
     {
         strcpy(szEDLname, szInput);
         C800_Clip_FILE(LOAD_EDL, 0, 'o');
         File_PreLoad = 0;
     }

     hThread_PARM2CLIP = CreateThread(NULL, 0, C900_Parm2Clip,
                                   0, 0, &threadId_PARM2CLIP);

   }
   else
     iParmConfirm = 1;

  if (iCtl_View_RGB_Always && !szMsgTxt[0])
  {
     strcpy(szMsgTxt, "OVL Never");
     DSP1_Main_MSG(0,0);
  }
  else
  if (!iViewToolBar && !szMsgTxt[0])
  {
     strcpy(szMsgTxt, "F11 = Toolbars");
     DSP1_Main_MSG(0,0);
  }


   // Main message loop
   while (GetMessage(&msg, NULL, 0, 0) > 0)
   {

     if (!TranslateAccelerator(hWnd_MAIN, hAccel, &msg))
     {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
     }
   }

   return msg.wParam;
}




//-----------------------------------------------------------
// Processes messages for the main window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
             LPARAM lParam)
{


#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  // message that will be supported
                                        // by the OS 
#endif


   int  iRC, iTmp1;
   char *lpCmdName, cTmp1;
   unsigned int uPrev_MoveMsg=0;
// int OLDParmConfirm, OLDiCtl_WarnBadStart;

   DWORD wmId, wmEvent;
   LRESULT MsgReturn;

   MsgReturn = false;

   wmId    = LOWORD(wParam);
   wmEvent = HIWORD(wParam);


    //static HINSTANCE hLibrary;

   /*
   if (DBGflag)
   {
       sprintf(szMsgTxt,"\n*MSG=%d  Id=%d  Event=%d", message, wmId, wmEvent);
       DBGout(szMsgTxt);
   }
   */

   switch (message)
   {
      case WM_CREATE:
           B910_Main_INIT(hWnd, /* message, */ wParam, lParam) ;
           Calc_PhysView_Size();    // from WM_CREATE
           break;


      // APPLICATION MENU COMMANDS
      case WM_COMMAND:

         Calc_PhysView_Size();      // from MenuCommand

         // parse the menu selections
         switch (wmId)
         {

            case IDM_EXIT:
                 MParse.SeqHdr_Found_Flag = 0;
                 MParse.Stop_Flag = 1;
                 if (!iFin_Done)
                    iRC = Y100_FIN();
                 else
                     iRC = 0;

                 if (iRC != 3)
                     DestroyWindow(hWnd);
                 break;

            case NULL:
               break;


            case IDM_BUF_VERYLARGE:
                 Out_SetBufSz(0);    
                 break;
            case IDM_BUF_LARGE:
                 Out_SetBufSz(1);    
                 break;
            case IDM_BUF_MEDIUM:
                 Out_SetBufSz(2);  
                 break;
            case IDM_BUF_SMALL:
                 Out_SetBufSz(3);   
                 break;


            // Priority for Random Access Navigation
            case IDM_PRI_RAN_HIGH:
                 Set_Priority(hMain_GUI, 1, 0, 1);
                 break;
            case IDM_PRI_RAN_NORMAL:
                 Set_Priority(hMain_GUI, 2, 0, 1);
                 break;
            case IDM_PRI_RAN_LOW:
                 Set_Priority(hMain_GUI, 3, 0, 1);
                 break;

            // Priority for PLAY/PREVIEW
            case IDM_PP_HIGH:
                 Set_Priority(hThread_MPEG, 1, 1, 0);
                 break;
            case IDM_PP_NORMAL:
                 Set_Priority(hThread_MPEG, 2, 1, 0);
                 break;
            case IDM_PP_LOW:
                 Set_Priority(hThread_MPEG, 3, 1, 0);
                 break;

            // Priority for OUTPUT file creation
            case IDM_PRI_OUT_HIGH:
                 Set_Priority(hThread_OUT, 1, 2, 1);
                 break;
            case IDM_PRI_OUT_NORMAL:
                 Set_Priority(hThread_OUT, 2, 2, 1);
                 break;
            case IDM_PRI_OUT_LOW:
                 Set_Priority(hThread_OUT, 3, 2, 1);
                 break;

            case IDM_OUT_DEBLANK:
                 ToggleMenu('T', &iCtl_Out_DeBlank, IDM_OUT_DEBLANK);
                 break;
            case IDM_OUT_MIXEDCASE:
                 ToggleMenu('T', &iCtl_Out_MixedCase, IDM_OUT_MIXEDCASE);
                 break;


            case IDM_OUT_KEEPDATE:   // Retain FileDate
                 ToggleMenu('T', &iCtl_Out_KeepFileDate, IDM_OUT_KEEPDATE);
                 break;



            case IDM_WARN_SIZE_1:
                 ToggleMenu('T', &iCtl_WarnSize_1, IDM_WARN_SIZE_1);
                 break;
            case IDM_WARN_SIZE_2:
                 ToggleMenu('T', &iCtl_WarnSize_2, IDM_WARN_SIZE_2);
                 break;
            case IDM_WARN_SIZE_3:
                 ToggleMenu('T', &iCtl_WarnSize_3, IDM_WARN_SIZE_3);
                 break;
            case IDM_WARN_SIZE_4:
                 ToggleMenu('T', &iCtl_WarnSize_4, IDM_WARN_SIZE_4);
                 break;


            case IDM_DROP_ASK:
                 Set_DropDefault(0);
                 break;
            case IDM_DROP_APPEND:
                 Set_DropDefault(1);
                 break;
            case IDM_DROP_OPEN:
                 Set_DropDefault(2);
                 break;


            case IDM_DEFSORT_OFF:
                 Set_SortDefault(-1);
                 break;
            case IDM_DEFSORT_NAME:
                 Set_SortDefault(0);
                 break;
            case IDM_DEFSORT_SCR:
                 Set_SortDefault(1);
                 break;
            case IDM_DEFSORT_DATE:
                 Set_SortDefault(2);
                 break;



            case IDM_XTN_MPG_UPPER:
                 Set_XTN_PS("MPG");
                 break;
            case IDM_XTN_MPG_LOWER:
                 Set_XTN_PS("mpg");
                 break;
            case IDM_XTN_M2P:
                 Set_XTN_PS("m2p");
                 break;
            case IDM_XTN_VID_MP2:
                 Set_XTN_PS("mp2");
                 if (!stricmp(szOut_Xtn_AUD, "MP2"))
                     Set_XTN_AUD("MPA");
                 break;
            case IDM_XTN_VOB:
                 Set_XTN_PS("vob");
                 break;
            case IDM_XTN_SAME:
                 Set_XTN_PS("$");
                 break;


            case IDM_XTN_MPA:
                 Set_XTN_AUD("MPA");
                 break;
            case IDM_XTN_M2A:
                 Set_XTN_AUD("M2A");
                 break;
            case IDM_XTN_M1A:
                 Set_XTN_AUD("M1A");
                 break;
            case IDM_XTN_MP3:
                 Set_XTN_AUD("MP3");
                 break;
            case IDM_XTN_MP2:
                 Set_XTN_AUD("MP2");
                 if (!stricmp(szOut_Xtn_RULE, "mp2"))
                     Set_XTN_PS("m2p");
                 break;
            case IDM_XTN_MP1:
                 Set_XTN_AUD("MP1");
                 break;

            case IDM_OUT_PART_AUTO:
                 ToggleMenu('T', &iCtl_OutPartAuto, IDM_OUT_PART_AUTO);
                 break;


            case IDM_DEL2RECYCLE:
                 ToggleMenu('T', &iCtl_RecycleBin, IDM_DEL2RECYCLE);
                 break;


            case IDM_TRACKBAR_BIG:  // Full Width Trackbar
                 ToggleMenu('T', &iCtl_Trackbar_Big, IDM_TRACKBAR_BIG); // Full Width Trackbar
                 D500_ResizeMainWindow(Overlay_Width, Overlay_Height, 1);
                 break;


            case IDM_HELP_PF1:
               // Optionally show long form of message
               // if (szMsgTxt[0] > ' ')
               // {
               //    DSP1_Main_MSG(1,0);
               //    break;
               // }

            case IDM_HELP_TXT:
                 uTmp1 = timeGetTime();
                 uTmp2 = (uTmp1 - uLastHelpTime) / 1000;
                 uLastHelpTime = uTmp1;

                 if (uTmp2 > 2)
                 {
                    strcpy(szBuffer, szINI_Path);
                    ext = strrchr(szBuffer, '\\')+1;
                    strcpy(ext, "Mpg2Cut2.txt");

                    ShellExecute(NULL, "open",
                                    szBuffer,
                                      NULL, NULL, SW_SHOWNORMAL);
                 }

                 break;


            case IDM_ABOUT:
                 DialogBox(hInst, (LPCTSTR)IDD_ABOUT, hWnd_MAIN, (DLGPROC)About);
                 break;

            case IDM_HOMEPAGE:
                 ShellExecute(NULL, "open",
                      "http://rocketjet4.tripod.com/Mpg2Cut2.htm",
                   // "http://www.geocities.com/rocketjet4/",
                   // "http://darkav.de.vu/",
                                      NULL, NULL, SW_SHOWNORMAL);
                  break;

            case IDM_NET_FORUM:
                 ShellExecute(NULL, "open",
                         "http://groups.google.com/group/rocketjetty",
                      // "http://p068.ezboard.com/frocketjettyfrm2",
                      // "http://pub31.ezboard.com/fdarkavengerssiteforumfrm1",
                                      NULL, NULL, SW_SHOWNORMAL);
                  break;


           case IDM_DBG:
                DBGctl();
                break;

           case IDM_AUDIO_DBG:
                ToggleMenu('T', &iAudioDBG, IDM_AUDIO_DBG);
                if (iAudioDBG)
                    DSP_Msg_Clear();
                break;

           case IDM_DBGSTR:
                ToggleMenu('T', &bDBGStr, IDM_DBGSTR);
                break;

           /*
           // Time Format pop-up menu
           // { "BLK",  "Rel",  "GOP",   "PTS",  "TOD", "PTS", "SCR"}; 
           case ID_FMT_BLK:
                iView_TC_Format = 0;
                DSP3_Main_TIME_INFO();
                break;
           case ID_FMT_REL:
                iView_TC_Format = 1;
                DSP3_Main_TIME_INFO();
                break;
           case ID_FMT_GOP:
                iView_TC_Format = 2;
                DSP3_Main_TIME_INFO();
                break;
           case ID_FMT_PTS:
                iView_TC_Format = 3;
                DSP3_Main_TIME_INFO();
                break;
           case ID_FMT_TOD:
                iView_TC_Format = 4;
                DSP3_Main_TIME_INFO();
                break;
           case ID_FMT_SCR:
                iView_TC_Format = 5;
                DSP3_Main_TIME_INFO();
                break;
           case ID_FMT_TOD_REL:
                iView_TC_Format = 7;
                DSP3_Main_TIME_INFO();
                break;
           */

            default:
               if (iBusy && message != IDM_CLIP_SAVE)
               {
                  if (wmId > 0x7FFF)
                  {
                     sprintf(szMsgTxt, OUTPUT_IN_PROGESS,
                                          wmId, wmEvent) ;
                     DSP1_Main_MSG(0,1);
                  }
                  MsgReturn = DefWindowProc(hWnd, message, wParam, lParam);
                  break;
               }
               else
                  MsgReturn = B200_Msg_USER_API(
                                //B201_Msg_USER(
                                                message, wParam, lParam);
         } // END_SWITCH

         break;



         case WM_HSCROLL:
           
           //wmId = LOWORD(wParam)

           /*
           if (DBGflag)
           {
             switch(wmId) 
             {
             case SB_BOTTOM: //   Scrolls to the lower right.
               strcpy(szMsgTxt,"SB_BOTTOM"); break;
             case SB_ENDSCROLL: //     Ends scroll.
               strcpy(szMsgTxt,"SB_ENDSCROLL"); break;
             case SB_LINELEFT: //     Scrolls left by one unit.
               strcpy(szMsgTxt,"SB_LINELEFT"); break;
             case SB_LINERIGHT: //     Scrolls right by one unit.
               strcpy(szMsgTxt,"SB_LINERIGHT"); break;
             case SB_PAGELEFT: //     Scrolls left by the width of the window.
               strcpy(szMsgTxt,"SB_PAGELEFT"); break;
             case SB_PAGERIGHT: //     Scrolls right by the width of the window.
               strcpy(szMsgTxt,"SB_PAGERIGHT"); break;
             case SB_THUMBPOSITION: //     Scrolls to the absolute position. The current position is specified by the nPos parameter.
               strcpy(szMsgTxt,"SB_THUMBPOSITION"); break;
             case SB_THUMBTRACK: //     Drags scroll box to the specified position. The current position is specified by the nPos parameter.
               strcpy(szMsgTxt,"SB_THUMBTRACK"); break;
             case SB_TOP: //       
               strcpy(szMsgTxt,"SB_TOP"); break;
             default: 
               sprintf(szMsgTxt,"HSCROLL SB_%d", wmId);
             }
           
             DBGout(szMsgTxt);
             DSP1_Main_MSG(0,0);
           }
           */

           if (wmId == SB_ENDSCROLL)
             break;

           if (iBusy)
           {
              //sprintf(szMsgTxt, "OUTPUT IN PROGESS.... %d",
              //                message) ;
              //DSP5_Main_FILE_INFO();
              //MessageBeep(MB_OK); 
              MsgReturn = DefWindowProc(hWnd, message, wParam, lParam);
           }
          else
          {

             if (DBGflag)
             {
                  sprintf(szDBGln, "HSCROLL wmId=%04X", wmId); 
                  DBGout(szDBGln);
             }


              if (!MParse.Stop_Flag)
                  Mpeg_Stop_Rqst();

              Calc_PhysView_Size(); // from HScroll

              if (! File_Limit)
                 MessageBeep(MB_OK);
              else
              {
              //if (wmId < 0x8000 &&  MParse.Stop_Flag ) // This needs to be more sophisticated to allow scrolling during playback
                  T600_Msg_HSCROLL(wmId); //wmId, wmEvent);
              }

              if (DBGflag)
                  DBGout("HSCROLL END");

          }

          break;



      case WM_KEYDOWN:
         if ( wParam == 'B' ||  wParam == 'C' ||  wParam == 'Z'
         ||   wParam == 'U')
           MsgReturn = DefWindowProc(hWnd, message, wParam, lParam);
         else
         {
            Calc_PhysView_Size();     // from keydown
            B400_Key_USER(wParam) ;
         }
         break;


      case WM_MOVE:
         
         if (DBGflag)
         {
             DBGout("*WM_MOVE");
         }
         

         if (!IsIconic(hWnd))
         { 
           if (iMainWin_State > 0) // Maximized Window ?
           {
               iZoom = iCtl_Zoom_Wanted; // Reset xFrom position.
               Prev_Clip_Height = 0;
               // View_MOUSE_ALIGN(-1);

               // iPred_Prev_Width = 1;
           }

           RECT iWin;
           GetWindowRect(hWnd_MAIN, &iWin);

           if (iPrevWinPos.left  != iWin.left  || iPrevWinPos.top    != iWin.top
           ||  iPrevWinPos.right != iWin.right || iPrevWinPos.bottom != iWin.bottom)
           {
              iPrevWinPos.left   = iWin.left;  
              iPrevWinPos.top    = iWin.top;
              iPrevWinPos.right  = iWin.right; 
              iPrevWinPos.bottom = iWin.bottom;

              Calc_PhysView_Size();  // from WM_MOVE

              if ( MParse.SeqHdr_Found_Flag)
                   Mpeg_Aspect_Calc();

              if (MParse.ShowStats_Flag)
                 Stats_Show(false, -1);

              //if (iCtl_Ovl_Release && MParse.iColorMode==STORE_YUY2
              // && ! iBusy)
              // {
              //     if (! DDOverlay_Flag)
              //     {
              //         D100_CHECK_Overlay();
              //         if (DDOverlay_Flag)
              //              D200_UPD_Overlay();
              //     }
              // }

              if (MParse.SeqHdr_Found_Flag)
              {
                  // if(!iBusy)
                  {
                    View_Rebuild_Chk(1);

                    /*
                    if (MParse.iColorMode==STORE_YUY2)
                    {
                       if(DDOverlay_Flag)
                         RenderYUY2(1);
                    }
                    else
                       RenderRGB24();
                    */
                  }

                  if (uPrev_MoveMsg != WM_MOVE)
                  {
                      if (iViewToolBar > 256)
                          DSP2_Main_SEL_INFO(1);
                      else
                          iMsgLife = 1;
                      DSP3_Main_TIME_INFO();
                  }
              }
           }
         }

         uPrev_MoveMsg = WM_MOVE;

         break;


      case WM_DISPLAYCHANGE:

         /*
         if (DBGflag)
         {
             DBGout("*WM_DSPCHG");
         }
         */

         Calc_PhysView_Size();        // from Display chg 
         if ( MParse.SeqHdr_Found_Flag)
              Mpeg_Aspect_Calc();

         VGA_New_Width =  LOWORD(lParam) ; // GetSystemMetrics(SM_CXSCREEN) ;
         // iVGA_Avail_Width = VGA_New_Width - 1;
         if (VGA_New_Width != VGA_Width)
         {
            VGA_Width  = VGA_New_Width  ;
            VGA_Height = HIWORD(lParam) ; // GetSystemMetrics(SM_CYSCREEN);

            if (VGA_Width  < 640)
                VGA_Width  = GetSystemMetrics(SM_CXSCREEN);
            if (VGA_Height < 480)
                VGA_Height = GetSystemMetrics(SM_CYSCREEN) ;


            Prev_Coded_Width = 0;
            if ( ! iBusy)
            {
              if (DDOverlay_Flag)
                   D300_FREE_Overlay();
              D100_CHECK_Overlay();
              if (MParse.iColorMode==STORE_YUY2)
                    Chg2YUV2(1, 0);
            }
         }
         break;

/*
      case WM_SIZE:
         
         if (DBGflag)
         {
             DBGout("*WM_SIZE");
         }
         

         if (IsIconic(hWnd))
         {
           if (DDOverlay_Flag && MParse.SeqHdr_Found_Flag && lpOverlay)
                   IDirectDrawSurface_UpdateOverlay(lpOverlay,
                              NULL, lpPrimary, NULL, DDOVER_HIDE, NULL);
         }
         else
         {
             Calc_PhysView_Size();       // from WM_SIZE
             if ( MParse.SeqHdr_Found_Flag)
                  Mpeg_Aspect_Calc();

             
             //if (iCtl_Ovl_Release && MParse.iColorMode==STORE_YUY2
             //&& ! iBusy)
             //{
             //   if (! DDOverlay_Flag)
             //    {
             //        D100_CHECK_Overlay();
             //        if (DDOverlay_Flag)
             //             D200_UPD_Overlay();
             //    }
             //}
             

            if (MParse.ShowStats_Flag)
                Stats_Show(false, -1);

            if (MParse.SeqHdr_Found_Flag && ! iBusy)
            {
               if (DDOverlay_Flag && MParse.iColorMode==STORE_YUY2)
               {
                 
                 //  if(! iBusy)
                 //  RenderYUY2(1);
                 
               }
               else
                  RenderRGB24();
            }
         }

         break;
*/

      // API Interface as per WeWantWideScreen 01 Jan 2006
      case RJPM_GET_APIVER:
             return API_VER;

      case RJPM_GET_APPVER:
             return APP_VER;

      case RJPM_GET_APPDATE:
             return (long)(&__DATE__);

      case RJPM_GET_EDLLIST:
             return (long)&EDList;

      case RJPM_GET_FILELIMIT:
             return File_Limit;

      case RJPM_THUMB2CLIPBOARD:
      case RJPM_THUMB2MEM:
      case RJPM_BMP_DEINT:
      case RJPM_BMP_ASIS:
      case RJPM_BMP_THUMB:
      case RJPM_BMP_CLIPBOARD:

                 if (File_Limit)
                     SNAP_Save(message, (ThumbRec*)lParam);  // v20503

                 break;

      case RJPM_LOADFILE: // Updated 10.12.2005
           
            lstrcpy(szInput,(char*)wParam);
            OLDParmConfirm=iParmConfirm;
            OLDiCtl_WarnBadStart=iCtl_WarnBadStart;
            iParmConfirm      = 0;   
            iSuppressWarnings = 1;
            iCtl_WarnBadStart=0;
            F100_IN_OPEN('o', -1);
            File_Limit=1;
            RefreshVideoFrame();
            iParmConfirm=OLDParmConfirm;
            iSuppressWarnings = 0;
            iCtl_WarnBadStart=OLDiCtl_WarnBadStart;
            break;


      case RJPM_APPENDFILE:  // ADD file
            lstrcpy(szInput,(char*)wParam);
            OLDParmConfirm=iParmConfirm;
            OLDiCtl_WarnBadStart=iCtl_WarnBadStart;

            iParmConfirm=0;
            iCtl_WarnBadStart=0;

            F100_IN_OPEN('a', -1);
            RefreshVideoFrame();
            iParmConfirm=OLDParmConfirm;
            iCtl_WarnBadStart=OLDiCtl_WarnBadStart;
            break;

      case RJPM_REFRESHFRAME:
            RefreshVideoFrame();
            break;

      case RJPM_SIGNAL_OVL:  // VISTA CRAP
            D200_UPD_Overlay();
            //if (PlayCtl.iPlayed_Frames < 2)
            //{
            //    View_MOUSE_ALIGN(-1);
            //    SetForegroundWindow(hWnd_MAIN);
            //}
            break;

      case RJPM_DISPLAY_SET:
           if (wParam)
              cTmp1 = 'S';
           else
              cTmp1 = 'C';

           ToggleMenu(cTmp1, &iCtl_ShowVideo_Flag, IDM_DISPLAY);
           iShowVideo_Flag = iCtl_ShowVideo_Flag;

           if (process.Action == ACTION_RIP
           &&  !MParse.Tulebox_SingleStep_flag)
           {
               Mpeg_Drop_Init();
           }

           break;



     case RJPM_GET_CURFILE:                
          return File_Ctr;


     case RJPM_SET_CURFILE:
          Mpeg_Stop_Rqst();

          if (wParam < (unsigned)File_Limit)
          {
              process.CurrFile = File_Ctr = wParam;

              iKick.Loc = 0;
              iKick.Action = ACTION_NEW_CURRLOC;
              iKick.File   = process.CurrFile ;

              process.CurrFile = iKick.File ;
              process.CurrLoc  = iKick.Loc  ;

              if (WaitForSingleObject(hThread_MPEG, 0) == WAIT_OBJECT_0)
              {
                  iShowVideo_Flag = iCtl_ShowVideo_Flag;
                  process.Action   = iKick.Action ;
                  hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec,
                                       0, 0, &threadId_MPEG);
              }
          }

          break;

     case RJPM_UPD_TRACKBAR:
          T110_Upd_Posn_TrackBar();
          break;

     case RJPM_UPD_MAIN_INFO:
          DSP5_Main_FILE_INFO();
          break;

      case WM_MOUSEWHEEL:
         // Quick and dirty - does not allow for fine graduatation wheels

         iTmp1 = (short) HIWORD(wParam);
         
         if (iCtl_Wheel_Scroll < 0)
           break;

         if (iCtl_Wheel_Scroll == 0 && iTmp1 >= 0) 
         {
             strcpy(szMsgTxt, "No BWD Frame");
             DSP1_Main_MSG(0,0);
             break;
         }

         switch (iCtl_Wheel_Scroll)
         {
           case 0: 
              wParam  = ID_FWD_FRAME; 
              break;
           case 4:
              if (iTmp1 >= 0)  wParam  = (unsigned int) hABack1; 
              else             wParam  = (unsigned int) hAFwd1; 
              break;
           case 3:
              if (iTmp1 >= 0)  wParam  = (unsigned int) hABack2; 
              else             wParam  = (unsigned int) hAFwd2; 
              break;
           case 2:
              if (iTmp1 >= 0)  wParam  = (unsigned int) hABack3; 
              else             wParam  = (unsigned int) hAFwd3; 
              break;
           default:
              if (iTmp1 >= 0)  wParam  = (unsigned int) hABack4; 
              else             wParam  = (unsigned int) hAFwd4; 
              break;
         }

     case RJPM_B200_USERMSG:
          MsgReturn = B201_Msg_USER(WM_COMMAND,wParam,lParam);
          break;

            


      case RJPM_CLOSEFILE: // 20502+
           if (!File_Limit) 
               return FALSE;

           szEDLprev[0] = 0;

           if (wParam==-1)  // All.
           {
               iTmp1 = File_Limit;
               while (iTmp1) // Close all previous files
               {
                      iTmp1--;
                      process.i64RestorePoint[iTmp1] = _telli64(FileDCB[iTmp1]);
                      _close(FileDCB[iTmp1]);
               }
           }
           else            // Single File.
           {
               if (wParam < (unsigned)File_Limit)
               {
                   process.i64RestorePoint[wParam] = _telli64(FileDCB[wParam]);
                   _close(FileDCB[wParam]);
               }
               else
                 return FALSE;
           }

           return TRUE;


      case RJPM_CLOSENAMEDFILE:
           if (!File_Limit) 
             return FALSE;

           szEDLprev[0] = 0;

           iTmp1 = File_Limit;
           while (iTmp1) // Close all previous files
           {
                  iTmp1--;


                  if (!lstrcmpi(File_Name[iTmp1],(char*)wParam))
                  {
                      _close(FileDCB[iTmp1]);
                      return TRUE;
                  }
           }

           process.iOut_Part_Ctr = 0;

           return FALSE;


      case RJPM_GET_FILENAME:
                        
           if (wParam < (unsigned)File_Limit)
              return (long)File_Name[wParam];
           else
              return NULL;


     case RJPM_GET_TIMECODES:

       if (File_Limit)
       {
          ((TimeCodesRec*)lParam)->iView_TC_Format = iView_TC_Format;

          memcpy(&((TimeCodesRec*)lParam)->GOP,  &gopTC,  sizeof(TimeCodeRec));
          memcpy(&((TimeCodesRec*)lParam)->Curr, &CurrTC, sizeof(TimeCodeRec));

          PTS_2Field(process.VideoPTS, 0);
          memcpy(&((TimeCodesRec*)lParam)->PTS,  &ptsTC,  sizeof(TimeCodeRec));

          RelativeTC_SET();
          memcpy(&((TimeCodesRec*)lParam)->Relative, &RelativeTC,
                       sizeof(TimeCodeRec));

          RelativeTC_SET();
          Relative_TOD();
          memcpy(&((TimeCodesRec*)lParam)->TOD, &ShowTC, sizeof(ShowTC));

          PTS_2Field(process.uViewSCR, 0);
          memcpy(&((TimeCodesRec*)lParam)->SCR,  &ptsTC,  sizeof(TimeCodeRec));


          return TRUE;
       }
       return FALSE; 


     case RJPM_GET_CALLBACKS:
          return (long)&mycallbacks;

     case RJPM_SET_RUNLOC: // 20504

         SendMessage(hTrack,TBM_SETPOS,TRUE,wParam);

          //if (WaitForSingleObject(hThread_MPEG, 150) == WAIT_OBJECT_0)
          {
              process.trackPrev   = wParam;
              process.startrunloc = process.total * wParam / TRACK_PITCH;

              iKick.Action = process.Action = ACTION_NEW_RUNLOC;
              MPEG_processKick();
          }
          break;

     // Allow disable of specific warnings.
      case RJPM_GET_POPUPS: // 20504

           return (iCtl_WarnSize_1     )     
                + (iCtl_WarnSize_2  <<1) 
                + (iCtl_WarnSize_3  <<2)  
                + (iCtl_WarnSize_4  <<3) 
                + (iCtl_Play_Info   <<4)  
                + (iCtl_WarnMpeg1   <<5) 
                + (iCtl_WarnBadStart<<6)
                + (iParmConfirm <<7);


      case RJPM_SET_POPUPS:  // 20504

                  iCtl_WarnSize_1   =  wParam & 0x1;
                  iCtl_WarnSize_2   = (wParam>>1) & 0x1;
                  iCtl_WarnSize_3   = (wParam>>2) & 0x1;
                  iCtl_WarnSize_4   = (wParam>>3) & 0x1;
                  iCtl_Play_Info    = (wParam>>4) & 0x1;
                  iCtl_WarnMpeg1    = (wParam>>5) & 0x1;
                  iCtl_WarnBadStart = (wParam>>6) & 0x1;
                  iParmConfirm      = (wParam>>7) & 0x1;
            return 0;


      case RJPM_REMOVEFILE:  // 20504
      {
           int nFound=-1;
           int i;

           for(i=0;i<File_Limit;i++)
           {
               if (!lstrcmpi(File_Name[i],(char*)wParam))
                   nFound=i;
           }

           if (nFound!=-1)
           {
               if ((File_Limit > 1) && (nFound < File_Limit-1))
               {
                   for (i=nFound;i<File_Limit-1;i++)
                   {
                       File_Name[i]    = File_Name[i+1];
                       File_Date[i]    = File_Date[i+1];
                       FileDCB[i]      = FileDCB[i+1];
                       cStartSCR[i][0] = cStartSCR[i+1][0];
                       cStartSCR[i][1] = cStartSCR[i+1][1];
                       File_Greg[i]    = File_Greg[i+1];
                   }
               }
               if (File_Limit) 
                   File_Limit--;                   
           }
           File_Final = File_Limit - 1;
           break;


      } 

      case RJPM_REOPENALLFILES: // 20505
           F590_ReOpenAllFiles(cRenamePlugIn_MultiMode);
           DSP5_Main_FILE_INFO();
           break;


      // END API SECTION


      // SYSTEM MENU
      case WM_SYSCOMMAND:

         //Calc_PhysView_Size();    // From SYSCOMMAND
         wmId     = wParam & 0xFFF0 ;

         if (wmId == SC_MINIMIZE)
           lpCmdName = &"MINIMIZE";
         else
         if (wmId == SC_MAXIMIZE)
           lpCmdName = &"MAXIMIZE";
         else
         if (wmId == SC_RESTORE)
           lpCmdName = &"RESTORE";
         else 
         //if (wmId == SC_SHOW)
         //  lpCmdName = &"SHOW";
         //else
           lpCmdName = &"MSG";

         if (wmId == SC_MINIMIZE)
         {
             iShowVideo_Flag = 0;
             Sleep(75);
             iPrevWinPos.right = -1;  // Allow subsequent restore to redraw overlay
             
             if (DBGflag)
             {
                 sprintf(szDBGln, "MINIMIZE %d", iPhysView_Height);
                 DBGout(szDBGln);
             }
             
   
             if (DDOverlay_Flag && iCtl_Ovl_Release)
                 D300_FREE_Overlay();

             iMainWin_State = -iMainWin_State;
         }


         if (wmId == SC_MAXIMIZE)
         {
           iMainWin_State = 1;
          
         }

         if (wmId == SC_RESTORE)
         {
           if (iMainWin_State < 0)
               iMainWin_State = 1;
           else
           {
               iMainWin_State = 0;

               /*
               Overlay_Width = Coded_Pic_Width - 16;
               if (Overlay_Width > VGA_Width)
                   Overlay_Width = VGA_Width;
               orect.right  = prect.right  = iPhysView_Width =  Overlay_Width;
               orect.bottom = prect.bottom = iPhysView_Height=  Overlay_Height;
               */

               //GetWindowRect(hWnd, &wrect);
               //GetClientRect(hWnd, &crect);
               //MoveWindow(hWnd, wrect.left, wrect.top, 600, 200, true);
 
           }

           Prev_Coded_Width = 16;

           VGA_GetSize(); 
           Calc_PhysView_Size();
           
           DSP2_Main_SEL_INFO(1);
         }

         if (wmId == SC_RESTORE
         ||  wmId == SC_MAXIMIZE)
         {
             if (iMainWin_State == 1)
                 iViewToolBar = iCtl_ViewToolbar[1];
             else
                 iViewToolBar = iCtl_ViewToolbar[0];
             Toolbar_Chg();
         }

         if (iCtl_View_Centre_Crop && iAspectOut > 2700)
         {
           iView_Centre_Crop = iCtl_View_Centre_Crop; // iMainWin_State;
         }
         else
           iView_Centre_Crop = 0;

        ToggleMenu('=', &iView_Centre_Crop, IDM_VIEW_CTR);



         if (wmId == SC_MAXIMIZE
         ||  wmId == SC_RESTORE)
         {
             iShowVideo_Flag = iCtl_ShowVideo_Flag;
             Prev_Coded_Width = 1;
             uPrev_MoveMsg = wmId;

             /*
             Calc_PhysView_Size();           // from MAXIMIZE / RESTORE

             if ( MParse.SeqHdr_Found_Flag)
             {
                  Mpeg_Aspect_Calc(); 

                  View_Rebuild_Chk(1);
             }
             */

             
             if (DBGflag)
             {
                 sprintf(szDBGln, "\nMSG=%s (%d = x%04X)  MAX=%d SQ=%d  Asp=%d.%d  Phys=%d.%d",
                         lpCmdName, wmId, wmId,
                         iMainWin_State, MParse.SeqHdr_Found_Flag,
                         iAspect_Width, iAspect_Height,
                         iPhysView_Width, iPhysView_Height);
                 DBGout(szDBGln);
             }
             


             //DSP2_Main_SEL_INFO(1);
             //DSP3_Main_TIME_INFO();

         }

 /*
         if (wmId == SC_MAXIMIZE
         ||  wmId == SC_RESTORE)
         {
             if (iCtl_Ovl_Release)
             {
                 if (DDOverlay_Flag)
                     D300_FREE_Overlay();
                 D100_CHECK_Overlay();
                 if (DDOverlay_Flag)
                     D200_UPD_Overlay();
             }
         }
*/
         //  default:
         /*
         if (DBGflag)
         {
             sprintf(szDBGln, "\n*END SYSCMD  %s (%d)\n", lpCmdName, wmId);
             DBGout(szDBGln);
         }
         */

         MsgReturn = DefWindowProc(hWnd, message, wParam, lParam);

         break;

      //case WM_SHOW:    // winuser.h
      case SW_SHOW:
           if (!iMainWin_State || iViewToolBar >= 256)
              DSP2_Main_SEL_INFO(1);

         MsgReturn = DefWindowProc(hWnd, message, wParam, lParam);

         break;



/*
      case WM_DISPLAYCHANGE:
      case WM_PAINT:
         // MessageBeep(MB_OK) ;
         VGA_Width = GetSystemMetrics(SM_CXSCREEN) ;
         if (DDOverlay_Flag && MParse.SeqHdr_Found_Flag)
               IDirectDrawSurface_UpdateOverlay(lpOverlay,
                              NULL, lpPrimary, NULL, DDOVER_HIDE, NULL);
         D500_ResizeMainWindow(iAspect_Width, iAspect_Height, 1) ;
         MainPaint() ;
         break;
*/



      case WM_DROPFILES:
           Mpeg_Stop_Rqst();
           Calc_PhysView_Size();            // from DROPFILES
           F300_DropFilesProc(wParam);
           break;



      case WM_LBUTTONDBLCLK:           // Double click = Zoom
           Calc_PhysView_Size();        // From DoubleClick
           if (iCtl_Zoom_Wanted > 1)
           { 
               iCtl_Zoom_Wanted--;
               Set_Zoom(iCtl_Zoom_Wanted);
           }
           View_MOUSE_CHK(lParam);

           break;

      case WM_LBUTTONUP:
           Calc_PhysView_Size();            // From ButttonUP
           View_MOUSE_CHK(lParam);

           break;

      /*
      case WM_RBUTTONDOWN:     // TODO: Opposite Click to context menu
           Calc_PhysView_Size();               // from RButtonDown
           if (iCtl_Zoom_Wanted < 3)
           {
               iCtl_Zoom_Wanted++;
               Set_Zoom(iCtl_Zoom_Wanted);
           }
           View_MOUSE_CHK(lParam);

           break;
      */

      case WM_DESTROY: 
           MParse.SeqHdr_Found_Flag = 0;
           if (!iFin_Done)
               Y100_FIN();
           PostQuitMessage(0);
           break;


      default:
           if (DBGflag 
           && wmId >  2
           && wmId != 1110  && wmId != 1206 
           && wmId != 1194  && wmId != 1166)
           {
               if (wmId == WM_DISPLAYCHANGE)
                        lpCmdName = &"DSPCHG";
               else
               if (wmId == WM_PAINT)
                        lpCmdName = &"PAINT";
               else
                        lpCmdName = &"?";

               sprintf(szDBGln, "\nMSG=%s (%04d = x%04X)",
                         lpCmdName, wmId, wmId);
               DBGout(szDBGln);
           }             

           MsgReturn = DefWindowProc(hWnd, message, wParam, lParam);
           break;
   }

   // DSP5_Main_FILE_INFO();
   return MsgReturn ;
}




//----------------------------------------------------------
// Translate API command values into Internal values
// Just in case the compiler has re-allocated them differently

LRESULT B200_Msg_USER_API(UINT message, WPARAM wParam, LPARAM lParam)
{

  LRESULT lRC;
  DWORD wmId, wmRest, dTmp1;
  wmId    = LOWORD(wParam);
  wmRest  = wParam & 0xFFFF0000;

  int ix;

  const DWORD dValue[] =
  {
        RJPC_FROM_MARK,    ID_FROM_MARK,
        RJPC_LEFT_ARROW,   ID_LEFT_ARROW,
        RJPC_RIGHT_ARROW,  ID_RIGHT_ARROW,
        RJPC_TO_MARK,      ID_TO_MARK,
        RJPC_TRACKBAR,     ID_TRACKBAR,
        RJPC_ADD,          ID_ADD,
        RJPC_DEL,          ID_DEL,
        RJPC_TO_SHIFT,     ID_TO_SHIFT,
        RJPC_RIGHT_SHIFT,  ID_RIGHT_SHIFT,
        RJPC_DOWN_SHIFT,   ID_DOWN_SHIFT,
        RJPC_HOME,         ID_HOME,
        RJPC_UP_SHIFT,     ID_UP_SHIFT,
        RJPC_LEFT_SHIFT,   ID_LEFT_SHIFT,
        RJPC_FROM_SHIFT,   ID_FROM_SHIFT,
        RJPC_FILE_CLOSE,         IDM_FILE_CLOSE,
        RJPC_FILE_CLOSE_CURR,    IDM_FILE_CLOSE_CURR,
        RJPC_FILE_CLOSE_OTHERS,  IDM_FILE_CLOSE_OTHERS,
        RJPC_FWD_FRAME,  ID_FWD_FRAME,
        0, 0
  };

  ix = 0;
  dTmp1 = 0xFFFFFFFF;
  lRC = false;

  while (dTmp1 != 0)
  {
     dTmp1 = dValue[ix];

     if (dTmp1 == wmId)
     {
         wmId = dValue[ix+1];
         dTmp1 = 0;
     }

     if (dTmp1 == 0)
     {
         wParam = wmId | wmRest;
         lRC = B201_Msg_USER(WM_COMMAND, wParam, lParam);
     }
     else
       ix +=2;
  }

  return lRC;

}


//--------------------------------------
void B203_File_Names()
{
  unsigned int uTmp1;

  Mpeg_Stop_Rqst();

  VideoList_MODE = 'o';
  strcpy(VideoList_Title, "Open");

  if (iCtl_BasicName_Panel)
      uTmp1 = IDD_FILELIST;
  else
      uTmp1 = IDD_FILEVIEW;

  DialogBox(hInst, (LPCTSTR)uTmp1, 
                              hWnd_MAIN,
                             (DLGPROC)F700_Video_List);

  DSP5_Main_FILE_INFO();
}


//----------------------------------------------------------
LRESULT  B201_Msg_USER(UINT message, WPARAM wParam, LPARAM lParam)
{
  int iRC, iTmp1, iTmp2, iTmp3;
  unsigned uTmp1;
  /* unsigned */ char *lpTST1, *lpTST2, cTST;


   DWORD wmId, wmEvent;
   wmId     = LOWORD(wParam);
   wmEvent  = HIWORD(wParam);

   if (iCtl_KB_MarkOpt)
   {
      iKB_MARK_FROM = ID_FROM_SHIFT;
      iKB_MARK_TO   = ID_TO_SHIFT;
   }
   else
   {
      iKB_MARK_FROM = ID_FROM_KEY;
      iKB_MARK_TO   = ID_TO_KEY;
   }

  //if (DBGflag)
  //{
  //   sprintf(szMsgTxt, "wmId=%d =x%04X", wmId, wmId);
  //   DSP1_Main_MSG(0,0);
  //}


   switch (wmId)
   {
      case IDM_OPEN:
           if (iCtl_F3_Names)
              B203_File_Names();
           else
           {
              F100_IN_OPEN('o', -1);
           }
           break;

      case IDM_APPEND: // ADD file
           F100_IN_OPEN('a', -1);
           break;

      case IDM_FILE_FILTER:
           if (*(DWORD*)(lpFName) == '_STV') //  x:\VTS_
              iTmp1 = 6;
           else
              iTmp1 = 4;

           F100_IN_OPEN('a', iTmp1);
           break;

      case IDM_FILE_MORE:  // Append More Like

           if (*(DWORD*)(lpFName) == '_STV') //  x:\VTS_
              lpTST1 = &szInput[7];           // Minimum of 7 similar bytes
           else
              lpTST1 = &szInput[5];           // Minimum of 5 similar bytes

           lpTST2 = &szInput[MAX_LIKE_LEN];   // Maximum amount of similarity

           // Look for a delimiter
           for (;; lpTST1++)
           {
              cTST = *lpTST1;
              if (     (cTST < '0' || cTST > '9')  // Non-numeric ?
                   &&  (cTST < 'A' || cTST > 'Z')  // Not a capital letter
                   &&  (cTST < 'a' || cTST > 'z')  // Not a lower-case letter
              || (lpTST1 > lpTST2))                // Reached Limit ?
              {
                break;
              } 
           }

           iTmp1 = lpTST1 - &szInput[0];
           if (iTmp1 <= 0)
               iTmp1  = 1;

           F100_IN_OPEN('a', iTmp1); 
           break;

      case IDM_FILE_CLOSE_CURR:
           Mpeg_Stop_Rqst();
           szEDLprev[0] = 0;

           if (File_Limit > 1)
           {
               F570_RemoveFile(File_Ctr, 1);
               Tick_CLEAR();
               T590_Trackbar_SEL();
               if (File_Ctr >= File_Final
               &&  process.CurrLoc)
               {
                  iKick.Action = ACTION_SKIP_FILE;
               }
               else
                  iKick.Action = ACTION_FWD_GOP;

               if (File_Limit)
                   MPEG_processKick(); //hThread_MPEG) ;
               break;
           }

      case IDM_FILE_CLOSE:
           Mpeg_Stop_Rqst();
           szEDLprev[0] = 0;
           F900_Close_Release('c');
           break;

      case IDM_FILE_CLOSE_OTHERS:
           Mpeg_Stop_Rqst();
           szEDLprev[0] = 0;
           F560_RemoveOtherFiles(File_Ctr);
           Tick_CLEAR();
           T590_Trackbar_SEL();
           T100_Upd_Posn_Info(1);
           break;



      case IDM_FILE_NAMES:
           if (! iCtl_F3_Names)
              B203_File_Names();
           else
              F100_IN_OPEN('o', -1);
            break;

      case IDM_SORT_NAME:
            F800_SORT_ALL(0);
            DSP5_Main_FILE_INFO();
            break;

      case IDM_SORT_SCR:
            F800_SORT_ALL(1);
            DSP5_Main_FILE_INFO();
            break;

      case IDM_SORT_DATE:
            F800_SORT_ALL(2);
            DSP5_Main_FILE_INFO();
            break;

      case IDM_FILE_DELETE:
           Mpeg_Stop_Rqst();
           if (iEDL_ctr)
           {
               MessageBox(hWnd_MAIN, SAVE_CLIPS_BEFORE_DELETE, szAppName, MB_OK);
           }
           else
           {
              Stats_Kill();
              VideoList_MODE = 'd';
              strcpy(VideoList_Title, DELETE_ALL_FILES);

              if (iCtl_BasicName_Panel)
                 uTmp1 = IDD_FILELIST;
              else
                 uTmp1 = IDD_FILEVIEW;

              DialogBox(hInst, (LPCTSTR)uTmp1, 
                                 hWnd_MAIN,
                               (DLGPROC)F700_Video_List);
              VideoList_MODE = ' ';
           }

           break;

 

      case IDM_CLIP_ADD:
           C350_Clip_ADD('+', 1) ;
           break;

      case IDM_CLIP_DEL:
           C400_Clip_DEL() ;
           break;

      case IDM_CLIP_CLEAR:
           C450_Clip_DEL_ALL() ;
           break;

      case IDM_UNDO:
           Mpeg_Stop_Rqst();
           C500_Clip_UNDO(); 
           break; 

      case IDM_SEL_ALL:
           Mpeg_Stop_Rqst();
           C100_Clip_DEFAULT('o');
           break;

      case IDM_CLIP_SAVE:
           Mpeg_Stop_Rqst();
           if (File_Limit) // iEDL_ctr)
               iRC = C800_Clip_FILE(SAVE_EDL, 0, 'o');
           else
              MessageBeep(MB_OK);
           break;

      case IDM_CLIP_EXPORT:
           Mpeg_Stop_Rqst();
           if (iEDL_ctr)
               iRC = C800_Clip_FILE(SAVE_CHAP, 0, 'o');
           else
              MessageBeep(MB_OK);
           break;

      case IDM_CLIP_LOAD:
           Mpeg_Stop_Rqst();
           iRC = C800_Clip_FILE(LOAD_EDL, 0, 'o');
           break;

      case IDM_CLIP_APPEND:
           Mpeg_Stop_Rqst();
           iRC = C800_Clip_FILE(LOAD_EDL, 0, 'a');
           break;

      case IDM_CLIP_IMPORT:
           Mpeg_Stop_Rqst();  
           iRC = C800_Clip_FILE(LOAD_CHAP, 0, 'o');
           break;



      case IDM_PREVIEW:
           iPreview_Clip_Ctr = 900;

           File_Ctr           =  process.FromFile;
           process.startFile  =  process.FromFile;
           process.startLoc   =  process.FromLoc;

           if (iCtl_To_Pad) // Option to grab extra video frame
           {
              process.endFile    =  process.ToPadFile;
              process.endLoc     =  process.ToPadLoc;
           }
           else
           {
              process.endFile    =  process.ToViewFile;
              process.endLoc     =  process.ToViewLoc;
           }
                                 //-    //(process.ToPadBlk - 1) *
                                 //      MPEG_SEARCH_BUFSZ;

           Mpeg_Stop_Rqst();
           B550_PLAY(ACTION_RIP);
           break;

      case IDM_PREVIEW_LAST: // Play last 3 seconds of selection
           iPreview_Clip_Ctr = 900;
           process.startFile  =  process.ToViewFile;
           process.startLoc   =  process.ToViewLoc
                              - (3*process.ByteRateAvg[process.ToViewFile]);
           if (process.startLoc < 0)
           {
              process.startLoc = 0;  // Make this smarter at some stage
           }

           // Don't go outside actual selection
           if (process.startFile <  process.FromFile
           || (process.startLoc  <  process.FromLoc
           &&  process.startFile == process.FromFile))
           {
             process.startFile  =  process.FromFile;
             process.startLoc   =  process.FromLoc;
           }

           if (iCtl_To_Pad) // Option to grab extra video frame
           {
              process.endFile    =  process.ToPadFile;
              process.endLoc     =  process.ToPadLoc;
           }
           else
           {
              process.endFile    =  process.ToViewFile;
              process.endLoc     =  process.ToViewLoc;
           }

           Mpeg_Stop_Rqst();
           B550_PLAY(ACTION_RIP);
           break;

      case IDM_PLAY_CLIPS: // IDM_PREVIEW_CLIPS:
           DSP_Msg_Clear();
           if (iViewToolBar >= 256) //  || iTool_Stacked)
               DSP2_Main_SEL_INFO(0);
           iPreview_Clip_Ctr = 0;
           MParse.Summary_Section = 0;
           C160_Clip_Preview();

           Mpeg_Stop_Rqst(); 
           B550_PLAY(ACTION_RIP);
           break;





      case ID_CUE2_FWD: // Super-Cue mode

           if (MParse.FastPlay_Flag < (iCtl_CUE_BUTTON_Speed + 1))
           {
               MParse.FastPlay_Flag = (iCtl_CUE_BUTTON_Speed + 1); // This will be incremented by B150
               B150_PLAY_FASTER(wmId);
           }
           else
           {
               if (byAC3_Init)   // AC3 decoder does not like jumps 
                   InitialAC3(); //     so reset decoder
               B555_Normal_Speed(1);
           }

           break;


      case ID_CUE_FWD:
           if (MParse.FastPlay_Flag < iCtl_CUE_BUTTON_Speed)
           {
               MParse.FastPlay_Flag = iCtl_CUE_BUTTON_Speed - 1;  // This will be incremented by B150
               iPlayAudio = 0;
               B150_PLAY_FASTER(wmId);
           }
           else
           {
               MParse.FastPlay_Flag  = 0;
               if (byAC3_Init)   // AC3 decoder does not like jumps 
                   InitialAC3(); //     so reset decoder
               B555_Normal_Speed(1);
           }

           break;

// IDM_CUE_SLOW


      case IDM_PLAY_FAST_2:

           MParse.FastPlay_Flag = 1;  // This will be incremented by B150
           MParse.SlowPlay_Flag = 0;
           iPlayAudio = iWantAudio;
           B150_PLAY_FASTER(wmId);
           break;

      case IDM_PLAY_FAST_1:

           MParse.FastPlay_Flag = 0;  // This will be incremented by B150
           MParse.SlowPlay_Flag = 0;
           iPlayAudio = iWantAudio;
           B150_PLAY_FASTER(wmId);
           break;


      case IDM_PLAY_FASTER:

           B150_PLAY_FASTER(wmId);
           break;


      case IDM_PLAY_SLOWER:

           if (MParse.FastPlay_Flag) // Return to normal  // || MParse.SlowPlay_Flag > 2  
           {
              if (MParse.FastPlay_Flag > 1) // Very Fast ?
              {
                  MParse.FastPlay_Flag--; // Less fast
                  if (MParse.FastPlay_Flag <= MAX_WARP)
                      iPlayAudio = iWantAudio;
                  B153_Fast_Msg();
              }
              else
                   B555_Normal_Speed(1);
           }
           else
           {
               if (MParse.SlowPlay_Flag == 1)
                   MParse.SlowPlay_Flag =  2;
               else
                   MParse.SlowPlay_Flag += 2;

               B160_PLAY_SLOW(wmId);
           }

           B500_CONTINUE_PLAY_HERE();
           break;


      case IDM_PLAY_SLOW_1:
 
           MParse.FastPlay_Flag = 0;
           MParse.SlowPlay_Flag = 1;
           iPlayAudio = iWantAudio;
           B160_PLAY_SLOW(IDM_PLAY_SLOW_1);
           break;

      case IDM_PLAY_SLOW_2A:

           MParse.FastPlay_Flag = 0;
           MParse.SlowPlay_Flag = 4;
           iPlayAudio = iWantAudio;
           B160_PLAY_SLOW(IDM_PLAY_SLOW_2A);
           break;
 



      case IDM_PLAY_HERE:
           B501_Play_Button(0);
           break; 

      case IDM_STOP:

          if (process.Action == ACTION_RIP)
          {
              process.Action  = ACTION_FWD_GOP; // Allow for elegant stop
              PlayCtl.iStopNextFrame = 1;
          }

          MParse.FastPlay_Flag = 0;  MParse.SlowPlay_Flag = 0;  

          if (MParse.Pause_Flag)
          {
              MParse.Pause_Flag = 0;
              ResumeThread(hThread_MPEG);
          }

          if (process.Action != ACTION_RIP && MParse.iVOB_Style)
          {
             MParse.iVOB_Style = 0; // allow for VOB files that are not really DVD (no NAV Packs)
             if ( ! MParse.Stop_Flag)
                Sleep(20);             // Allow time for gentle stop
          }

          if ( ! MParse.Stop_Flag)
             Sleep(50); // Allow for elegant stop

          if ( ! MParse.Stop_Flag)
          {
             MParse.Stop_Flag = 3;
             Sleep(250);
          }
          else
          {
             Sleep(250);
             if (MParse.Stop_Flag >= 3)
             {
                 strcpy(szMsgTxt,"Waiting...");
                 DSP1_Main_MSG(1,1); 
             }
          }
           
          //Menu_Main_Enable();
          break;


      case IDM_PAUSE:
           if (MParse.Stop_Flag || MParse.Tulebox_SingleStep_flag)
           {
              B500_CONTINUE_PLAY_HERE();
           }
           else
           if (MParse.Pause_Flag)
           {
              iRender_TimePrev  = 0;
              MParse.Pause_Flag = 0;
              ResumeThread(hThread_MPEG);
           }
           else
           {
              SuspendThread(hThread_MPEG);
              MParse.Pause_Flag = 1;
           }

           break;

      case IDM_PREVIEW_SUMMARY:
          ToggleMenu('T', &iCtl_Play_Summary, IDM_PREVIEW_SUMMARY);
          if (!MParse.Stop_Flag && iCtl_Play_Summary)
          {
             MParse.Summary_Section = 0;
             C160_Clip_Preview();
             File_Ctr = process.startFile;
             _lseeki64(FileDCB[File_Ctr], process.startLoc, SEEK_SET );
             MParse.NextLoc = process.startLoc;
          }
          break;





      case IDM_SAVE:
           if (File_Limit && ! iBusy) // && MParse.SystemStream_Flag >= 0)
           {
                process.iOutUnMux   = 0;  
                process.iOut_AutoSPLIT = 0;
                iOut_Audio_All = 1;
                OUT_SAVE('L');  // SAVE ALL CLIPS in EDL
           } 
           else
                MessageBeep(MB_OK); 
           break;


      case IDM_SAVE_PARTS:   // IDM_SAVE_PARTIAL
           if (File_Limit && ! iBusy) // && MParse.SystemStream_Flag >= 0)
           {

                    OUT_SAVE('P');  // SAVE ALL CLIPS in EDL
           }
           else 
           {
                MessageBeep(MB_OK); Sleep(500);
                Beep(200,     // DWORD  dwFreq,	Hz frequency in hertz 
                     800);    // DWORD duration, in milliseconds            
           }
           break;


      case IDM_SAVE_THIS:
            if (File_Limit && ! iBusy) // && MParse.SystemStream_Flag >= 0)
            {
               process.iOutUnMux   = 0;
               process.iOut_AutoSPLIT = 0;
               iOut_Audio_All = 1;
               OUT_SAVE('1') ;  // SAVE 1 CLIP
               DSP3_Main_TIME_INFO();
            }
            else
               MessageBeep(MB_OK);
            break; 


      case IDM_SAVE_UNMUX:
            if (File_Limit && ! iBusy)
            {
             if (MParse.SystemStream_Flag < 0)
             {
                 MessageBox(hWnd_MAIN, UNMUX_NOT_SUPPORTED_TS,
                              Mpg2Cut2_SORRY,  MB_OK);
             }
             else
             {
               process.iOutUnMux   = 1;
               process.iOut_AutoSPLIT = 0;
               OUT_SAVE('L') ;  // SAVE ALL CLIPS in EDL
             }
            }
            else
               MessageBeep(MB_OK);
            break;


      case IDM_FILE_GARBAGE:
           if (! iBusy)
           {
              if (szInput[2] != '$')
              {
                strcpy(&szInput[2],  "$GARBAGE.mpg");
                strcpy(&szOutput[2], "$GARBAGE.mpg");
              }
              OUT_SAVE('G') ;  // RECLAIM FREE SPACE GARBAGE
              iView_TC_Format = 0;
              DSP3_Main_TIME_INFO();
            }
            else
               MessageBeep(MB_OK);
            break;


/*
      case IDM_PROCESS_WAV:
            DialogBox(hInst, (LPCTSTR)IDD_FILELIST, hWnd, (DLGPROC)AudioList);
            break;

      case IDM_SAVE_D2V:
            D2Vsave() ;
            break;
*/


      case IDM_RGB_ALWAYS:
           ToggleMenu('T', &iCtl_View_RGB_Always, IDM_RGB_ALWAYS);

           if (iCtl_View_RGB_Always)
           {
               Chg2RGB24(1,0);
           }
           else
           {
               Chg2YUV2(1, 0);
           }

           break;


      case IDM_STORE_RGB24:
            Chg2RGB24(1,0);
            break;


      case IDM_STORE_YUY2:
            szMsgTxt[0] = 0;
            DSP_Msg_Clear(); 
            Chg2YUV2(1, 0);
            break;


      case IDM_YUV_SWAP:
           Lum_Swap_UV(1);
          break;

      case IDM_YUV_FAST:
          ToggleMenu('T', &iCtl_View_Fast_YUV, IDM_YUV_FAST);
          iView_Fast_YUV = 0;
          break;

      case IDM_RGB_FAST:
          ToggleMenu('T', &iCtl_View_Fast_RGB, IDM_RGB_FAST);
          iView_Fast_RGB = 0;
          if (MParse.SeqHdr_Found_Flag && MParse.iColorMode==STORE_RGB24)
             Store_RGB24(curr_frame, 0); 

          break;


      case IDM_VIEW_HUGE:
          ToggleMenu('T', &iCtl_View_Limit2k, IDM_VIEW_HUGE);
          break;

      case IDM_OVL_RELEASE:
          ToggleMenu('T', &iCtl_Ovl_Release, IDM_OVL_RELEASE);
          break;

      case IDM_OVL_DWORD:
           ToggleMenu('T', &iCtl_Ovl_DWord, IDM_OVL_DWORD);
           break; 

           // VISTA CRAP
      case IDM_OVL_NOTIFY_DEF:
           Set_OVL_Notify(0);
           break;
      case IDM_OVL_NOTIFY_FRAMECHG:
           Set_OVL_Notify(1);
           break;
      case IDM_OVL_NOTIFY_UPDWINDOW:
           Set_OVL_Notify(2);
           break;
      case IDM_OVL_SIGNAL_GUI:
           Set_OVL_Notify(3);
           break;

      case IDM_OVL_SIGNAL_ATI:
           ToggleMenu('T', &iCtl_OVL_ATI_Bug, IDM_OVL_SIGNAL_ATI);
           break;

           
      case IDM_OVL_MASK_LEADTEK:
           DeleteObject(hBrush_MASK); 
           DeleteObject(hBrush_MSG_BG); 
           if (iCtl_Mask_Colour == iColor_Menu_BG) // Overlay key was Mid Grey ?
           {
               iCtl_Mask_Colour = 0x000600;  // Overlay key = Dark Green
               iTmp3 = iCtl_Text_Colour;
               iTmp2 = iCtl_Back_Colour;
               uTmp1 = MF_UNCHECKED;
           }
           else 
           {
               iCtl_Mask_Colour = iColor_Menu_BG; // Overlay key Mid Grey
               iTmp2            = iColor_Menu_BG;
               iTmp3 = 0x010101;
               uTmp1 = MF_CHECKED;
           }

           CheckMenuItem(hMenu, IDM_OVL_MASK_LEADTEK, uTmp1);
           CheckMenuItem(hMenu, IDM_OVL_MASK_LEADTEK_BLK, MF_UNCHECKED);

           hBrush_MASK   = CreateSolidBrush(iCtl_Mask_Colour);
           hBrush_MSG_BG = CreateSolidBrush(iTmp2);
           SetBkColor(  hDC, iTmp2);  
           SetTextColor(hDC, iTmp3); 

           if (DDOverlay_Flag)
           {
               DD_OverlayMask(2);  // FillRect(hDC, &crect, hBrush_MASK);
           }

           strcpy(szMsgTxt,"Next Session");
           DSP1_Main_MSG(1,1); 

           //if (DDOverlay_Flag && MParse.iColorMode==STORE_YUY2) // && iShowVideo_Flag)
           //{
           //    D300_FREE_Overlay(); 
           //    View_Rebuild_Chk(1);
           //}
           break;
           
           
      case IDM_OVL_MASK_LEADTEK_BLK:
           DeleteObject(hBrush_MASK);  
           DeleteObject(hBrush_MSG_BG); 
           if (iCtl_Mask_Colour == 0) // Overlay key was Black ?
           {
               iCtl_Mask_Colour = iColor_Menu_BG;  // Overlay key = Leadtek Grey
               iTmp3 = iCtl_Text_Colour;
               iTmp2 = iCtl_Back_Colour;
               uTmp1 = MF_UNCHECKED;
               strcpy(szMsgTxt,"Next Session");
           }
           else 
           {
               iCtl_Mask_Colour   = 0; // Overlay key Black
               iCtl_Mask_Fallback = 1; // Temporary - this session only.
               iTmp2 = 0x010101;
               iTmp3 = 0xFFEEEE;
               uTmp1 = MF_CHECKED;
           }

           CheckMenuItem(hMenu, IDM_OVL_MASK_LEADTEK, MF_UNCHECKED);
           CheckMenuItem(hMenu, IDM_OVL_MASK_LEADTEK_BLK, uTmp1);

           hBrush_MASK = CreateSolidBrush(iCtl_Mask_Colour);
           hBrush_MSG_BG = CreateSolidBrush(iTmp2);
           SetBkColor(  hDC, iTmp2);  
           SetTextColor(hDC, iTmp3); 

           if (DDOverlay_Flag)
           {
               DD_OverlayMask(2);  // FillRect(hDC, &crect, hBrush_MASK);
           }
           DSP1_Main_MSG(1,1); 

           //if (DDOverlay_Flag && MParse.iColorMode==STORE_YUY2) // && iShowVideo_Flag)
           //{
           //    D300_FREE_Overlay(); 
           //    View_Rebuild_Chk(1);
           //}
           break;
           

      case IDM_OVL_FULLKEY:
           ToggleMenu('T', &iCtl_OVL_FullKey, IDM_OVL_FULLKEY);
           strcpy(szMsgTxt,"Next Session");
           DSP1_Main_MSG(1,1); 

           //if (DDOverlay_Flag && MParse.iColorMode==STORE_YUY2) // && iShowVideo_Flag)
           //{
           //    D300_FREE_Overlay(); 
           //    View_Rebuild_Chk(1);
           //}           
           break;





      case IDM_ADD_OFF:
           Set_ADD(0);
           break;

      case IDM_ADD_REMIND:
           Set_ADD(1);
           break;

      case IDM_ADD_AUTO:
           Set_ADD(2);
           break;

      case IDM_EDIT_AUTOSAVE:
           ToggleMenu('T', &iCtl_EDL_AutoSave, IDM_EDIT_AUTOSAVE);
           MParse.EDL_AutoSave = iCtl_EDL_AutoSave;
           break;


/*
      case IDM_ALIGN_TOP:
            Set_ALIGN_VERT(0, 1);
            break;

      case IDM_ALIGN_AUTO:
            Set_ALIGN_VERT(1, 1);
            break;

      case IDM_ALIGN_MID:
            Set_ALIGN_VERT(2, 1);
            break;

      case IDM_ALIGN_BOT:
            Set_ALIGN_VERT(3, 1);
            break;


      case IDM_ALIGN_LEFT:
            Set_ALIGN_HORIZ(0, 1);
            break;

      case IDM_ALIGN_CTR:
            Set_ALIGN_HORIZ(2, 1);
            break;

      case IDM_ALIGN_RIGHT:
            Set_ALIGN_HORIZ(3, 1);
            break;
*/

      case IDM_VIEW_CTR_MAX:
           ToggleMenu('T', &iCtl_View_Centre_Crop, IDM_VIEW_CTR_MAX);
           if (iMainWin_State >= 0)
               View_Ctr_Crop();
           break;

      case IDM_VIEW_CTR:
           ToggleMenu('T', &iView_Centre_Crop, IDM_VIEW_CTR);
           View_Ctr_Crop();
           break;


      case IDM_ASPECT_MISMATCH:
           ToggleMenu('T', &iCtl_AspMismatch, IDM_ASPECT_MISMATCH);
           Mpeg_Aspect_Resize();
           View_Rebuild_Chk(0);
           break;

      case IDM_VIEW_CONVERGE_RED_H:
           ToggleMenu('T', &iConverge_Red_H, IDM_VIEW_CONVERGE_RED_H);
           View_Rebuild_Chk(0);
           break;

      case IDM_VIEW_CONVERGE_RED_V:
           ToggleMenu('T', &iConverge_Red_V, IDM_VIEW_CONVERGE_RED_V);
           View_Rebuild_Chk(0);
           break;

      case IDM_VIEW_CONVERGE_RED_BOTH:
           ToggleMenu('S', &iConverge_Red_V, IDM_VIEW_CONVERGE_RED_V);
           ToggleMenu('S', &iConverge_Red_H, IDM_VIEW_CONVERGE_RED_H);
           View_Rebuild_Chk(0);
           break;

      case IDM_VIEW_CONVERGE_BLUE_H:
           ToggleMenu('T', &iConverge_Blue_H, IDM_VIEW_CONVERGE_BLUE_H);
           View_Rebuild_Chk(0);
           break;

      case IDM_VIEW_CONVERGE_BLUE_V:
           ToggleMenu('T', &iConverge_Blue_V, IDM_VIEW_CONVERGE_BLUE_V);
           View_Rebuild_Chk(0);
           break;

      case IDM_VIEW_CONVERGE_BLUE_BOTH:
           ToggleMenu('S', &iConverge_Blue_H, IDM_VIEW_CONVERGE_BLUE_H);
           ToggleMenu('S', &iConverge_Blue_V, IDM_VIEW_CONVERGE_BLUE_V);
           View_Rebuild_Chk(0);
           break;

      case IDM_VIEW_CONVERGE_ALL: 
           ToggleMenu('S', &iConverge_Red_V, IDM_VIEW_CONVERGE_RED_V);
           ToggleMenu('S', &iConverge_Red_H, IDM_VIEW_CONVERGE_RED_H);
           ToggleMenu('S', &iConverge_Blue_H, IDM_VIEW_CONVERGE_BLUE_H);
           ToggleMenu('S', &iConverge_Blue_V, IDM_VIEW_CONVERGE_BLUE_V);
           View_Rebuild_Chk(0);
           break;

      case IDM_VIEW_CONVERGE_NONE:
           ToggleMenu('C', &iConverge_Red_V, IDM_VIEW_CONVERGE_RED_V);
           ToggleMenu('C', &iConverge_Red_H, IDM_VIEW_CONVERGE_RED_H);
           ToggleMenu('C', &iConverge_Blue_H, IDM_VIEW_CONVERGE_BLUE_H);
           ToggleMenu('C', &iConverge_Blue_V, IDM_VIEW_CONVERGE_BLUE_V);
           View_Rebuild_Chk(0);
           break;




      case IDM_ASPECT_RETAIN:
          ToggleMenu('T', &iCtl_Aspect_Retain, IDM_ASPECT_RETAIN);
          break;

      case IDM_ASP_MPEG1_FORCE:
           Set_Aspect_MPG1(2);
           break;

      case IDM_ASP_MPEG1_GUESS:
           Set_Aspect_MPG1(1);
           break;

      case IDM_ASP_MPEG1_STD:
           Set_Aspect_MPG1(0);
           break;


      case IDM_ASPECT_OFF:
           Set_Aspect_Mode(0);
           break;

      case IDM_ASPECT_TV:
           Set_Aspect_Mode(1);
           break;

      case IDM_ASPECT_WIDE:
           Set_Aspect_Mode(2);
           break;

      case IDM_ASPECT_70mm:
           Set_Aspect_Mode(3);
           break;

      case IDM_ASPECT_STD:
           Set_Aspect_Mode(4);
           break;

      case IDM_ASPECT_NARROW:  // Narrow cropped capture - used in the bad old days
           Set_Aspect_Mode(9); // Fudge setting for crappy old captures
           break;

      case IDM_ASPECT_119:
           Set_Aspect_Mode(10);
           break;
      case IDM_ASPECT_132:
           Set_Aspect_Mode(11);
           break;
      case IDM_ASPECT_150:
           Set_Aspect_Mode(12);
           break;
      case IDM_ASPECT_167:
           Set_Aspect_Mode(13);
           break;
      case IDM_ASPECT_185:
           Set_Aspect_Mode(14);
           break;
      case IDM_ASPECT_235:
           Set_Aspect_Mode(15);
           break;
      case IDM_ASPECT_239:
           Set_Aspect_Mode(16);
           break;
      case IDM_ASPECT_255:
           Set_Aspect_Mode(17);
           break;
      case IDM_ASPECT_259:
           Set_Aspect_Mode(18);
           break;
      case IDM_ASPECT_276:
           Set_Aspect_Mode(19);
           break;
      case IDM_ASPECT_400:
           Set_Aspect_Mode(20);
           break;




      case IDM_FRAME_RATE_STD: 
           Set_Frame_Rate(0);
           break;
      case IDM_FRAME_RATE_23:
           Set_Frame_Rate(1);
           break;
      case IDM_FRAME_RATE_24:
           Set_Frame_Rate(2);
           break;
      case IDM_FRAME_RATE_25:
           Set_Frame_Rate(3);
           break;
      case IDM_FRAME_RATE_29:
           Set_Frame_Rate(4);
           break;
      case IDM_FRAME_RATE_30:
           Set_Frame_Rate(5);
           break;
      case IDM_FRAME_RATE_12:
           Set_Frame_Rate(9);
           break;
      case IDM_FRAME_RATE_16:
           Set_Frame_Rate(10);
           break;
      case IDM_FRAME_RATE_18:
           Set_Frame_Rate(11);
           break;
      case IDM_FRAME_RATE_20:
           Set_Frame_Rate(12);
           break;
      case IDM_FRAME_RATE_06:
           Set_Frame_Rate(13);
           break;
      case IDM_FRAME_RATE_02:
           Set_Frame_Rate(14);
           break;
      case IDM_FRAME_RATE_01:
           Set_Frame_Rate(15);
           break;





      case IDM_DEINT_CURR:
          if (Deint_VIEW)
          {
             Deint_VIEW = 0 ;
             uTmp1 = MF_UNCHECKED;

             if (MParse.SeqHdr_Found_Flag)
             {
                if (DDOverlay_Flag && MParse.iColorMode==STORE_YUY2)
                    Cnv_422_yuy2_FLD(0, y444, u422, v422, yuy2);
                //else
                //    conv444toRGB24_FLD(y444, u422, v422, rgb24);
             }

          }
          else
          {
             Deint_VIEW = 1 ;
             uTmp1 = MF_CHECKED;
          }

          CheckMenuItem(hMenu, IDM_DEINT_CURR, uTmp1);

          Deint_Auto_CURR = 0;

          Mpeg_Aspect_Calc();
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

          break;



      case IDM_DEINT_AUTO:
          ToggleMenu('T', &Deint_AUTO_View, IDM_DEINT_AUTO);
          Deint_Auto_CURR = Deint_AUTO_View;
          break;

      case IDM_DEINT_VOB:
          ToggleMenu('T', &Deint_VOB, IDM_DEINT_VOB);
          break;

      case IDM_DEINT_SNAP:
          ToggleMenu('T', &Deint_SNAP, IDM_DEINT_SNAP);
          break;


      case IDM_CROP_TOP:
          ToggleMenu('T', &iCtl_CropTop, IDM_CROP_TOP);
          break;


      case IDM_ZOOM_AUTO:
          process.iView_Extra_Crop = 0;
          Set_Zoom(-1);
          break;

      case IDM_ZOOM_OFF:
          process.iView_Extra_Crop = 0;
          Set_Zoom(0);
          break;

      case IDM_ZOOM_COMPACT:
          process.iView_Extra_Crop = 0;
          Set_Zoom(1);
          break;

      case IDM_ZOOM_HALF:
          process.iView_Extra_Crop = 0;
          Set_Zoom(2);
          break;

      case IDM_ZOOM_THIRD:
          process.iView_Extra_Crop = 0;
          Set_Zoom(3);
          break;

      case IDM_ZOOM_RETAIN:
          ToggleMenu('T', &iCtl_Zoom_Retain, IDM_ZOOM_RETAIN);
          break;



      case IDM_ZOOM_OUT:
          if (iZoom > 2)
          {
            if ((Coded_Pic_Width*100 / VGA_Width) < 110)
            //&&   MPEG_Seq_aspect_ratio_code > 2
            //&&   iView_Aspect_Mode          > 2)
                 iZoom = 0 ;
            else
                 iZoom = 1 ;
          }
          else
          if (iZoom == 0)
          { 
            if (process.iView_Extra_Crop)
                process.iView_Extra_Crop = 0;
            else
            if (iView_Centre_Crop)
                iView_Centre_Crop = 0;
            else
                iZoom++;
          }
          else
          {
             iZoom++;
          }

          iCtl_Zoom_Wanted = iZoom;
          Set_Zoom(iZoom);

          break;


      case IDM_ZOOM_IN:  // NOT FINISHED YET ! 
          if (iZoom == 0) 
          {
            if (iCtl_View_Centre_Crop || iAspectOut <= 2711)
            {
                process.iView_Extra_Crop = 1;
                //iView_Centre_Crop        = 1;
            }
            //else
            //    iView_Centre_Crop = 1;
            iView_Centre_Crop = 1;
            ToggleMenu('=', &iView_Centre_Crop, IDM_VIEW_CTR);

            process.iView_TopMask    = -2;
          }
          else
          {
             iZoom-- ;
          }

          iCtl_Zoom_Wanted = iZoom;
          Set_Zoom(iZoom);

          break;



      case IDM_SKIP_BEHIND_HD: 
          ToggleMenu('T', &iCtl_Drop_Behind, IDM_SKIP_BEHIND_HD);
          PlayCtl.iDrop_Behind   = iCtl_Drop_Behind;
          PlayCtl.iDrop_B_Frames_Flag = (iCtl_Drop_Behind && 0xFF);
          break;

      case IDM_SKIP_BEHIND_SD:
          ToggleMenu('T', ((char*)(&iCtl_Drop_Behind)+1), IDM_SKIP_BEHIND_SD);
          PlayCtl.iDrop_Behind   = iCtl_Drop_Behind;
          PlayCtl.iDrop_B_Frames_Flag = (iCtl_Drop_Behind && 0xFF00);
          break;

      case IDM_SKIP_PTS:
          ToggleMenu('T', &iCtl_Drop_PTS, IDM_SKIP_PTS);
          PlayCtl.iDrop_PTS_Flag = iCtl_Drop_PTS;
          break;
      case IDM_FIELD_DROP:
          ToggleMenu('T', &iField_Drop, IDM_FIELD_DROP);
          break;

      case IDM_CUE_SLOW:
          ToggleMenu('T', &iCtl_NotSoFast, IDM_CUE_SLOW);
          if (iCtl_NotSoFast)
              iCtl_CUE_BUTTON_Speed = CUE_SLOW;
          else
              iCtl_CUE_BUTTON_Speed = CUE_SLOW+1;
          break;


      case IDM_PLAY_AUDLOCK:
          ToggleMenu('T', &iCtl_Play_AudLock, IDM_PLAY_AUDLOCK);
          if (iAudio_SEL_Track != TRACK_NONE)
             iAudio_Lock = iCtl_Play_AudLock;
          break;

      case IDM_PLAY_SYNC:
          ToggleMenu('T', &iCtl_Play_Sync, IDM_PLAY_SYNC);
          break;


      case IDM_ERR_ANL:
          ToggleMenu('T', &Err_Analysis, IDM_ERR_ANL);
          break;

      case IDM_YV12:
           if ( ! iBusy)
           {
              if (DDOverlay_Flag) 
                   D300_FREE_Overlay();

              ToggleMenu('T', &iCtl_YV12, IDM_YV12);

              D100_CHECK_Overlay();
              if (MParse.iColorMode==STORE_YUY2)
                    Chg2YUV2(1, 0) ;
           }

          break;



      case IDM_PARM_CONFIRM:
         ToggleMenu('T', &iCtl_ParmConfirm, IDM_OUT_MIXEDCASE);
         break;


      case IDM_PARM_CLIP:
         ToggleMenu('T', &iCtl_ParmClipSpec, IDM_PARM_CLIP);
         break;


      case IDM_TOOLTIPS:
         ToggleMenu('T', &iCtl_ToolTips, IDM_TOOLTIPS);
         if (iViewToolBar >= 256) 
         {
           ToolBar_Destroy();
           ToolBar_Create();
         }
         break;


//      case IDM_LOC_BLK:
//            Loc_Method = 0;
//            CheckMenuItem(hMenu, IDM_LOC_BLK, MF_CHECKED);
//            CheckMenuItem(hMenu, IDM_LOC_HDR, MF_UNCHECKED);
//            break;

//      case IDM_LOC_HDR:
//            Loc_Method = 2;
//            CheckMenuItem(hMenu, IDM_LOC_HDR, MF_CHECKED);
//            CheckMenuItem(hMenu, IDM_LOC_BLK, MF_UNCHECKED);
//            break;

      case IDM_MPEG_ANY:
         ToggleMenu('T', &iPES_Mpeg_Any, IDM_MPEG_ANY);
         Chg2RGB24(1,0);
         break;

      case IDM_BYTE_SYNC:
          ToggleMenu('T', &iCtl_Byte_Sync, IDM_BYTE_SYNC);
          break;


      case IDM_DRIVE_SEGMENTS:
          ToggleMenu('T', &iCtl_Drv_Segments, IDM_DRIVE_SEGMENTS);
          break;


      case IDM_VIEW_COLS:
          ToggleMenu('T', &iCtl_BasicName_Panel, IDM_VIEW_COLS);
          break;


      case IDM_ADD_PAD:
         ToggleMenu('T', &iCtl_To_Pad, IDM_ADD_PAD);
         if (iCtl_To_Pad && iCtl_VOB_Style)
             MessageBox(hWnd_MAIN, INCLUDE_TO_NOT_VOB,
                              Mpg2Cut2_WARNING,  MB_OK);
         break;
 
      case IDM_OUT_SD_HDR:
         ToggleMenu('T', &iCtl_Out_Fix_SD_Hdr, IDM_OUT_SD_HDR);
         break;

      case IDM_OUT_FIX_ERRORS:
         ToggleMenu('T', &iCtl_Out_Fix_Errors, IDM_OUT_FIX_ERRORS);
         break;

      case IDM_OUT_KILLPAD:
         ToggleMenu('T', &iCtl_Out_KillPadding, IDM_OUT_KILLPAD);
         break; 

      case IDM_OUT_DROPCRUD:
         ToggleMenu('T', &iCtl_Out_DropCrud, IDM_OUT_DROPCRUD);
         process.iOut_DropCrud = iCtl_Out_DropCrud;
         break; 

      case IDM_OUT_DEFLAG:
         ToggleMenu('T', ((char*)(&iCtl_Out_Fix_SD_Hdr)+1), IDM_OUT_DEFLAG);
         break;

      case IDM_OUT_INTERLACE:
         ToggleMenu('T', ((char*)&iCtl_Out_Force_Interlace), IDM_OUT_INTERLACE);
         break;


      case IDM_OUT_PARSE:
         ToggleMenu('T', &iCtl_Out_Parse, IDM_OUT_PARSE);
         break;

      case IDM_OUT_TC_ADJUST:
         ToggleMenu('T', &iCtl_Out_TC_Adjust, IDM_OUT_TC_ADJUST);
         break;
      case IDM_OUT_TC_FORCE:
         ToggleMenu('T', &iCtl_Out_TC_Force, IDM_OUT_TC_FORCE);
         break;
      case IDM_PTS_INVENT:
         ToggleMenu('T', &iCtl_Out_PTS_Invent, IDM_PTS_INVENT);
         break;
      case IDM_OUT_HIDE_AUDIO:
         ToggleMenu('T', &iOut_HideAudio, IDM_OUT_HIDE_AUDIO);
         break;
      case IDM_OUT_FIX_PKT_LEN:
         ToggleMenu('T', &iOut_FixPktLens, IDM_OUT_FIX_PKT_LEN);
         break;
      case IDM_OUT_PS1PS2:  // Convert Private Stream 1 to PS2 - TWINHAN PROBLEM
         ToggleMenu('T', &iOut_PS1PS2, IDM_OUT_PS1PS2);
         break;


      case IDM_OUT_BROKEN_FLAG:
         //ToggleMenu('T', &iCtl_SetBrokenGop, IDM_OUT_BROKEN_FLAG);
         if (iCtl_SetBrokenGop == 1)
         {
           iCtl_SetBrokenGop = 0;
           uTmp1 = MF_UNCHECKED;
         }
         else
         {
           iCtl_SetBrokenGop = 1;
           uTmp1 = MF_CHECKED;
         }
         CheckMenuItem(hMenu, IDM_OUT_BROKEN_FLAG,    uTmp1);
         CheckMenuItem(hMenu, IDM_OUT_BROKEN_CLR,     MF_UNCHECKED);
         break;

      case IDM_OUT_BROKEN_CLR:
         if (iCtl_SetBrokenGop < 0)
         {
           iCtl_SetBrokenGop = 0;
           uTmp1 = MF_UNCHECKED;
         }
         else
         {
           iCtl_SetBrokenGop = -1;
           uTmp1 = MF_CHECKED;
         }
         CheckMenuItem(hMenu, IDM_OUT_BROKEN_CLR,     uTmp1);
         CheckMenuItem(hMenu, IDM_OUT_BROKEN_FLAG,    MF_UNCHECKED);
         break;



      case IDM_OUT_PTS_MATCH:         // Audio Matching
         ToggleMenu('T', &iCtl_Out_PTS_Match, IDM_OUT_PTS_MATCH);
         break;
 
      case IDM_OUT_PARSE_ALL_PKTS:
         ToggleMenu('T', &iCtl_Out_Parse_AllPkts, IDM_OUT_PARSE_ALL_PKTS);
         break;

      case IDM_OUT_PARSE_DEEP:
         ToggleMenu('T', &iCtl_Out_Parse_Deep, IDM_OUT_PARSE_DEEP);
         break;

      case IDM_OUT_ALIGN_VIDEO:
         ToggleMenu('T', &iCtl_Out_Align_Video, wmId);
         break;
      case IDM_OUT_ALIGN_AUDIO:
         ToggleMenu('T', &iCtl_Out_Align_Audio, wmId);
         break; 

      case IDM_PARSE_SPLIT_START:
         ToggleMenu('T', &iCtl_Out_Parse_SplitStart, wmId);
         break;

      //case IDM_OUT_KEEP_AC3HDR:
      //   ToggleMenu('T', &iCtl_Out_Keep_Ac3Hdr, wmId);
      //   break;

      case IDM_PARSE_UNTICK:   // _NONE_OF_THE_ABOVE

           iCtl_SetBrokenGop    = 0; iCtl_Out_TC_Adjust     = 0;
           iCtl_Out_PTS_Match   = 0; 
           iCtl_Out_Align_Video = 0; iCtl_Out_Align_Audio   = 0;
           iCtl_Out_Parse_Deep  = 0; iCtl_Out_Parse_AllPkts = 0;

           Set_Parse_Ticks();
         break;

      case IDM_PARSE_BUCKET:

           iCtl_SetBrokenGop    = 1; //iCtl_Out_TC_Adjust    = 1;
           iCtl_Out_PTS_Match   = 1; 
           iCtl_Out_Align_Video = 1; iCtl_Out_Align_Audio   = 1;
           // iCtl_Out_Parse_Deep  = 1; 
           // iCtl_Out_Parse_AllPkts = 1;

           Set_Parse_Ticks();
         break;


      case IDM_VOB_CHUNKS:
         ToggleMenu('T', &iCtl_VOB_Style, IDM_VOB_CHUNKS);
         if (iCtl_To_Pad && iCtl_VOB_Style)
             MessageBox(hWnd_MAIN, INCLUDE_TO_NOT_VOB,
                              Mpg2Cut2_WARNING,  MB_OK);
         break;

      case IDM_OUT_SEQEND:
         ToggleMenu('T', &iCtl_Out_Seq_End, IDM_OUT_SEQEND);
         break;

      case IDM_OUT_SYS_MPEG:
         iCtl_Out_SysHdr_Mpeg = 1;
         CheckMenuItem(hMenu, IDM_OUT_SYS_MPEG,      MF_CHECKED);
         CheckMenuItem(hMenu, IDM_OUT_SYS_VOB,       MF_UNCHECKED);
         break;

      case IDM_OUT_SYS_VOB:
         iCtl_Out_SysHdr_Mpeg = 0;
         CheckMenuItem(hMenu, IDM_OUT_SYS_MPEG,      MF_UNCHECKED);
         CheckMenuItem(hMenu, IDM_OUT_SYS_VOB,        MF_CHECKED);
         break;

      case IDM_OUT_SYS_CLIP:
         ToggleMenu('T', &iCtl_Out_SysHdr_EveryClip, IDM_OUT_SYS_CLIP);
         break;

      case IDM_OUT_SYS_UNLOCK:
         ToggleMenu('T', &iCtl_Out_SysHdr_Unlock, IDM_OUT_SYS_UNLOCK);
         break; 
  
      //case IDM_OUT_DEMUX:
      //   ToggleMenu('T', &iCtl_Out_Demux, IDM_OUT_DEMUX);
      //   break;


      case IDM_OUT_FOLDER_SAME:
           Set_OutFolderMode(0);
           break;

      case IDM_OUT_FOLDER_RECENT:
           Set_OutFolderMode(2);
           strcpy(&szCtl_OutFolder[0], &szOutFolder[0]);
           break;

      case IDM_OUT_FOLDER_EVERY:
           Set_OutFolderMode(3);
           Set_Folder(&szCtl_OutFolder[0], NULL, 0, // &iCtl_OutFolder_Active, IDM_OUT_FOLDER_FIRST, 
                     0, SAVE_VOB, &"\\DUMMY.MPG");

           strcpy(szOutput, szCtl_OutFolder);
           strcpy(&szOutFolder[0], &szCtl_OutFolder[0]);
           break;

      case IDM_OUT_FOLDER_FIRST:
           Set_OutFolderMode(1);
           Set_Folder(&szCtl_OutFolder[0], &iCtl_OutFolder_Active, IDM_OUT_FOLDER_FIRST, 
                     0, SAVE_VOB, &"\\DUMMY.MPG");

           strcpy(szOutput, szCtl_OutFolder);
           strcpy(&szOutFolder[0], &szCtl_OutFolder[0]);
           break;

      case IDM_OUT_FOLDER_DUAL:
           ToggleMenu('T', &iCtl_OutFolder_Both, IDM_OUT_FOLDER_DUAL);
           break;


      // Similar stuff for BMP folder

      case IDM_BMP_FOLDER_TOGGLE:
         ToggleMenu('T', &iCtl_BMP_Folder_Active, IDM_BMP_FOLDER_TOGGLE);
         iBMP_Folder_Active = iCtl_BMP_Folder_Active;
         break;

      case IDM_BMP_FOLDER:
         Set_Folder(&szCtl_BMP_Folder[0], &iCtl_BMP_Folder_Active, IDM_BMP_FOLDER_TOGGLE,
                                 0, SAVE_BMP, &"\\DUMMY.BMP");
         iBMP_Folder_Active = iCtl_BMP_Folder_Active;
         break;




      case IDM_WARN_BAD_START:
          ToggleMenu('T', &iCtl_WarnBadStart, IDM_WARN_BAD_START);
         break;
      case IDM_WARN_BAD_SYSHDR:
          ToggleMenu('T', &iCtl_WarnBadSysHdr, IDM_WARN_BAD_SYSHDR);
         break;

      case IDM_WARN_MPEG1:
          ToggleMenu('T', &iCtl_WarnMpeg1, IDM_WARN_MPEG1);
         break;
      case IDM_WARN_FMT_TS:
          ToggleMenu('T', &iCtl_WarnTS,    IDM_WARN_FMT_TS);
         break;
      case IDM_WARN_FMT_TSMPG:
          ToggleMenu('T', &iCtl_WarnTSmpg, IDM_WARN_FMT_TSMPG);
         break;
      case IDM_WARN_FMT_CDXA:
          ToggleMenu('T', &iCtl_WarnCDXA,  IDM_WARN_FMT_CDXA);
         break;
         
      case IDM_WARN_DONE:
          ToggleMenu('T', &iCtl_WarnDone,  IDM_WARN_DONE);
         break;

      case IDM_PREAMBLE_MAX:
          Set_Preamble_Mode(9);
          break;

      case IDM_PREAMBLE_SMALL:
          Set_Preamble_Mode(1);
          break;

      //case IDM_PREAMBLE_POWERVCR:
      //   Set_Preamble_Mode(3);
      //    break;

      case IDM_PREAMBLE_NONE:
          Set_Preamble_Mode(0);
          break;

      case IDM_PREAMBLE_VTS:
         ToggleMenu('T', &iCtl_Out_Preamble_VTS, IDM_PREAMBLE_VTS);
         break;




      case IDM_AUDIO_PS2:
         ToggleMenu('T', &iCtl_Audio_PS2, IDM_AUDIO_PS2);
         break;

      case IDM_VOL_SLIDERS:
           if (iCtl_KB_V_Popup) 
               VOL340_Up();
           else
               B380_Volume_Window(); 
           break;

      case IDM_VOLUME_UP:
           if (!iCtl_KB_V_Popup)
               VOL340_Up();
           else
               B380_Volume_Window();
           break;

      case IDM_VOLUME_DOWN:
           VOL320_Down();
           break; 

      case IDM_VOLUME_BOOST:
         ToggleMenu('T', &iCtl_Volume_Boost, IDM_VOLUME_BOOST);
         if (!iCtl_Volume_Boost)
             VOL304_Vol_Boost_Off();
         else
         {
             iVol_BoostCat_Done[iVol_Boost_Cat] = 0;
             VOL303_Vol_Boost_On();
         }
         break;

      case IDM_BOOST_MPA_TRAD:
         ToggleMenu('T', &iCtl_Vol_BoostCat_Flag[FORMAT_MPA], IDM_BOOST_MPA_TRAD);
         if (iVol_Boost_Cat == FORMAT_MPA)
         {
             iCtl_Volume_Boost = iCtl_Vol_BoostCat_Flag[FORMAT_MPA];
             VolBoostChg();
         }
         break;
      case IDM_BOOST_MPA_TRENDY:
         ToggleMenu('T', &iCtl_Vol_BoostCat_Flag[FORMAT_MPA_TRENDY], IDM_BOOST_MPA_TRENDY);
         if (iVol_Boost_Cat == FORMAT_MPA_TRENDY)
         {
             iCtl_Volume_Boost = iCtl_Vol_BoostCat_Flag[FORMAT_MPA_TRENDY];
             VolBoostChg();
         }
         break;
      case IDM_BOOST_AC3:
         ToggleMenu('T', &iCtl_Vol_BoostCat_Flag[FORMAT_AC3], IDM_BOOST_AC3);
         if (iVol_Boost_Cat == FORMAT_AC3)
         {
             iCtl_Volume_Boost = iCtl_Vol_BoostCat_Flag[FORMAT_AC3];
             VolBoostChg();
         }
         break;
      case IDM_BOOST_DTS:
         ToggleMenu('T', &iCtl_Vol_BoostCat_Flag[FORMAT_DTS], IDM_BOOST_DTS);
         if (iVol_Boost_Cat == FORMAT_DTS)
         {
             iCtl_Volume_Boost = iCtl_Vol_BoostCat_Flag[FORMAT_DTS];
             VolBoostChg();
         }
         break;
      case IDM_BOOST_LPCM:
         ToggleMenu('T', &iCtl_Vol_BoostCat_Flag[FORMAT_LPCM], IDM_BOOST_LPCM);
         if (iVol_Boost_Cat == FORMAT_LPCM)
         {
             iCtl_Volume_Boost = iCtl_Vol_BoostCat_Flag[FORMAT_MPA];
             VolBoostChg();
         }
         break;

      case IDM_VOLUME_LIMITING:  // Turn down volume on loud files - we want it not too loud.  Volume_Limiter
         VOL203_Volume_Target();
         break;


      case IDM_VOLUME_AUTO:  // Automatic Volume Control
         ToggleMenu('T', &iCtl_Volume_AUTO, IDM_VOLUME_AUTO);

         if (iCtl_Volume_AUTO) 
             VOL301_Volume_Boost_Start();
         break;


      case IDM_VOLUME_AUTO_UP:  // Automatic Volume Control - UP
         if ( iCtl_Volume_AUTO <= 0
         ||   iVolume_AUTO < K_BOOST_DENOM*2
         ||  (iVolume_AUTO < K_BOOST_DENOM*4
                 && iAudio_SEL_Format >= FORMAT_AC3)
            )
         {
             ToggleMenu('S', &iCtl_Volume_AUTO, IDM_VOLUME_AUTO);
             VOL301_Volume_Boost_Start();
         }
         else
         {
             VOL337_Volume_Bolder();
         }
         break; 

      case IDM_VOL_RETAIN:
         ToggleMenu('T', &iCtl_Volume_Retain, IDM_VOL_RETAIN);
         if (hVolDlg0)
              Vol_Show_Chks();
         break;

      case IDM_VOLUME_MUTE: 
           VOL210_MUTE_Toggle();
           break;

      case IDM_VOL_STARKEY:  // StarKey Keyboard Volume Controls
           ToggleMenu('T', &iCtl_Vol_StarKey, IDM_VOL_STARKEY);
           break;

      case IDM_VOLUME_GENTLE:
         if (iCtl_Volume_SlowAttack == 0)
             iCtl_Volume_SlowAttack  = 64;
         else
             iCtl_Volume_SlowAttack = -iCtl_Volume_SlowAttack;

         if (iCtl_Volume_SlowAttack > 0)
             uTmp1 = MF_CHECKED;
         else
             uTmp1 = MF_UNCHECKED;
         
         CheckMenuItem(hMenu, IDM_VOLUME_GENTLE,  uTmp1);
         break;
  
      case IDM_AUDIO_KARAOKE:
          ToggleMenu('T', &MParse.Karaoke_Flag, IDM_AUDIO_KARAOKE);
          break;
      case IDM_AUDIO_ANTIPHASE:
          ToggleMenu('T', &MParse.Anti_Phase, IDM_AUDIO_ANTIPHASE);
          break;

      case IDM_AUDIO_AHEAD:
         ToggleMenu('T', &iCtl_AudioAhead, IDM_AUDIO_AHEAD);
         RdAHD_Flag = iCtl_AudioAhead;
         GetBlk_RdAHD_RESET();
         break;

      case IDM_AUDIO_THREAD:
         ToggleMenu('T', &iCtl_AudioThread, IDM_AUDIO_AHEAD);
         break;

      case IDM_AUDIO_CRC:
         ToggleMenu('T', &iCtl_Audio_CRC, IDM_AUDIO_CRC);
         AC3_CRC_Chk = iCtl_Audio_CRC;
         break;

      case IDM_AUDIO_44K:
         ToggleMenu('T', &iAudio_Force44K, IDM_AUDIO_44K);
         if(iWAV_Init && MParse.Stop_Flag)
         {
            WAV_Flush();  
            if (iAudio_Force44K)
            {
                WAV_WIN_Audio_close();
            }
         }
         if (iAudio_Force44K)
             ToggleMenu('C', &iCtl_PALTelecide, IDM_AUDIO_PALTEL);
         break;

      case IDM_AUDIO_PALTEL:
         ToggleMenu('T', &iCtl_PALTelecide, IDM_AUDIO_PALTEL);
         if (iCtl_PALTelecide)
         {
             Set_Frame_Rate(2);
             ToggleMenu('C', &iAudio_Force44K, IDM_AUDIO_44K);
         }
         else
             Set_Frame_Rate(0);
         break;



      case IDM_MPA_AUTO:
         ClearMPALib(-1);
         break;

      case IDM_MPAlib:
         ClearMPALib(1);
         break;

      case IDM_Mpa_MMX:
         ClearMPALib(2);
         break;

      case IDM_Mpa_SSE1:
         ClearMPALib(3);
         break;

      case IDM_Mpa_SSE2:
         ClearMPALib(4);
         break;



      case IDM_FILE_WIN32API:
         ToggleMenu('T', &iCtl_File_WIN32, IDM_FILE_WIN32API);
         break;


      case IDM_TRACK_MEMO:
            ToggleMenu('T', &iCtl_Track_Memo, IDM_TRACK_MEMO);
            if (iCtl_Track_Memo)
                Set_AudioTrack(TRACK_AUTO);
            break;


      case IDM_TRACK_AUTO:
            Set_AudioTrack(TRACK_AUTO);
            break;

      case IDM_TRACK_NONE:
            Set_AudioTrack(TRACK_NONE);
            break;

      case IDM_TRACK_1:
            Set_AudioTrack(TRACK_1);
            break;

      case IDM_TRACK_2:
            Set_AudioTrack(TRACK_2);
            break;

      case IDM_TRACK_3:
            Set_AudioTrack(TRACK_3);
            break;

      case IDM_TRACK_4:
            Set_AudioTrack(TRACK_4);
            break;

      case IDM_TRACK_5:
            Set_AudioTrack(TRACK_5);
            break;

      case IDM_TRACK_6:
            Set_AudioTrack(TRACK_6);
            break;

      case IDM_TRACK_7:
            Set_AudioTrack(TRACK_7);
            break;

      case IDM_TRACK_8:
            Set_AudioTrack(TRACK_8);
            break;


      case IDM_AC3:
            iWant_Aud_Format = FORMAT_AC3;
            CheckMenuItem(hMenu, IDM_AC3, MF_CHECKED);
            CheckMenuItem(hMenu, IDM_MPA, MF_UNCHECKED);
            CheckMenuItem(hMenu, IDM_LPCM, MF_UNCHECKED);
            CheckMenuItem(hMenu, IDM_SELECT, MF_UNCHECKED);
            break;

      case IDM_MPA:
            iWant_Aud_Format = FORMAT_MPA;
            CheckMenuItem(hMenu, IDM_AC3, MF_UNCHECKED);
            CheckMenuItem(hMenu, IDM_MPA, MF_CHECKED);
            CheckMenuItem(hMenu, IDM_LPCM, MF_UNCHECKED);
            CheckMenuItem(hMenu, IDM_SELECT, MF_UNCHECKED);
            break;



      case IDM_LPCM:
           iWant_Aud_Format = FORMAT_LPCM;
           CheckMenuItem(hMenu, IDM_AC3, MF_UNCHECKED);
           CheckMenuItem(hMenu, IDM_MPA, MF_UNCHECKED);
           CheckMenuItem(hMenu, IDM_LPCM, MF_CHECKED);
           CheckMenuItem(hMenu, IDM_SELECT, MF_UNCHECKED);
           break;

      /*
      case IDM_AC3_DEMUXALL:
               AC3_Flag = AUDIO_DEMUXALL;
               CheckMenuItem(hMenu, IDM_AC3_DEMUXALL, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_AC3_DEMUXONE, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_AC3_DECODE, MF_UNCHECKED);
               break;

      case IDM_AC3_DEMUXONE:
               AC3_Flag = AUDIO_DEMUXONE;
               CheckMenuItem(hMenu, IDM_AC3_DEMUXALL, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_AC3_DEMUXONE, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_AC3_DECODE, MF_UNCHECKED);
               break;

      case IDM_AC3_DECODE:
               AC3_Flag = AUDIO_DECODE;
               CheckMenuItem(hMenu, IDM_AC3_DEMUXALL, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_AC3_DEMUXONE, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_AC3_DECODE, MF_CHECKED);
               break;
      */


      case IDM_DRC_NONE:
               AC3_DRC_FLag = DRC_NONE;
               Set_Menu_Array(AC3_DRC_FLag, &DRC_MenuId[0]);
               break;

      case IDM_DRC_LIGHT:
               AC3_DRC_FLag = DRC_LIGHT;
               Set_Menu_Array(AC3_DRC_FLag, &DRC_MenuId[0]);
               break;

      case IDM_DRC_NORMAL:
               AC3_DRC_FLag = DRC_NORMAL;
               Set_Menu_Array(AC3_DRC_FLag, &DRC_MenuId[0]);
               break;

      case IDM_DRC_HEAVY:
               AC3_DRC_FLag = DRC_HEAVY;
               Set_Menu_Array(AC3_DRC_FLag, &DRC_MenuId[0]);
               break;

      case IDM_DRC_VERYHEAVY:
               AC3_DRC_FLag = DRC_VERYHEAVY;
               Set_Menu_Array(AC3_DRC_FLag, &DRC_MenuId[0]);
               break;


      case IDM_DSDOWN:
            ToggleMenu('T', &AC3_DSDown_Flag, IDM_DSDOWN);
            break;
/*
      case IDM_PRESCALE:
               if (AC3_PreScale_Ratio != 1.0
               || !MParse.SeqHdr_Found_Flag
               || !IsWindowEnabled(hTrack))
               {
                  CheckMenuItem(hMenu, IDM_PRESCALE, MF_UNCHECKED);
                  AC3_PreScale_Ratio = 1.0;
               }
               else
               {
                  Decision_Flag = true;
                  iShowVideo_Flag = 0;

                  EnableMenuItem(hMenu, IDM_OPEN, MF_GRAYED);
                  EnableMenuItem(hMenu, IDM_SAVE, MF_GRAYED);
                  EnableMenuItem(hMenu, IDM_LOAD_D2V, MF_GRAYED);
                  EnableMenuItem(hMenu, IDM_SAVE_D2V, MF_GRAYED);
                  EnableMenuItem(hMenu, IDM_STOP, MF_ENABLED);

                  Menu_Main_Disable(true, true);
                  Stats_Show(true, 1);

                  process.Action = ACTION_RIP;
                  AC3_PreScale_Ratio = 1.0;

                  if (WaitForSingleObject(hThread_MPEG, 0)==WAIT_OBJECT_0)
                     hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);
               }
               break;


            case IDM_MPA_DEMUXALL:
               MPA_Flag = AUDIO_DEMUXALL;
               CheckMenuItem(hMenu, IDM_MPA_DEMUXALL, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_MPA_DEMUXONE, MF_UNCHECKED);
               break;

            case IDM_MPA_DEMUXONE:
               MPA_Flag = AUDIO_DEMUXONE;
               CheckMenuItem(hMenu, IDM_MPA_DEMUXALL, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_MPA_DEMUXONE, MF_CHECKED);
               break;
*/

      case IDM_SELECT:
               iWant_Aud_Format = FORMAT_AUTO;
               CheckMenuItem(hMenu, IDM_AC3, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_MPA, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_LPCM, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SELECT, MF_CHECKED);
               break;


      case IDM_IDCT_MMX:
               IDCT_SetHardware(IDCT_MMX, IDM_IDCT_MMX);
               break;
      case IDM_IDCT_SSEMMX:
               IDCT_SetHardware(IDCT_SSEMMX, IDM_IDCT_SSEMMX);
               break;
      case IDM_IDCT_FPU:
               IDCT_SetHardware(IDCT_FPU, IDM_IDCT_FPU);
               break;
      case IDM_IDCT_REF:
               IDCT_SetHardware(IDCT_REF, IDM_IDCT_REF);
               break;
      case IDM_IDCT_SSE2:
               IDCT_SetHardware(IDCT_SSE2, IDM_IDCT_SSE2);
               break;




      case IDM_FIELD_EXPERIMENT:
          ToggleMenu('T', &iField_Experiment, IDM_FIELD_EXPERIMENT);
          break;



      case IDM_FO_NONE:
               MParse.FO_Flag = FO_NONE;
               CheckMenuItem(hMenu, IDM_FO_NONE, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_FO_FILM, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_FO_SWAP, MF_UNCHECKED);
               SetDlgItemText(hStats, IDC_INFO, "");
               break;

      case IDM_FO_FILM:
               MParse.FO_Flag = FO_FILM;
               CheckMenuItem(hMenu, IDM_FO_NONE, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_FO_FILM, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_FO_SWAP, MF_UNCHECKED);
               SetDlgItemText(hStats, IDC_INFO, "");
               break;


      case IDM_FO_SWAP:
               MParse.FO_Flag = FO_SWAP;
               CheckMenuItem(hMenu, IDM_FO_NONE, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_FO_FILM, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_FO_SWAP, MF_CHECKED);
               SetDlgItemText(hStats, IDC_INFO, "");
               break;


      case IDM_TVSCALE:  //   ....----....----
               RGB_Scale  = 0x1000200010002000;
               RGB_Offset = 0x0000000000000000;
               RGB_CBU    = 0x000038B4000038B4;
               RGB_CGX    = 0xF4FDE926F4FDE926;
               RGB_CRV    = 0x00002CDD00002CDD;

               MParse.PC_Range_Flag = false;
               CheckMenuItem(hMenu, IDM_TVSCALE, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_PCSCALE, MF_UNCHECKED);

               RefreshVideoFrame();
               break;


      case IDM_PCSCALE:  //   ....----....----
               RGB_Scale  = 0x1000254310002543;
               RGB_Offset = 0x0010001000100010;
               RGB_CBU    = 0x0000408D0000408D;
               RGB_CGX    = 0xF377E5FCF377E5FC;
               RGB_CRV    = 0x0000331300003313;

               MParse.PC_Range_Flag = true;
               CheckMenuItem(hMenu, IDM_TVSCALE, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_PCSCALE, MF_CHECKED);

               RefreshVideoFrame();
               break; 


      case IDM_LUMINANCE_TOGGLE:
           if (iCtl_Lum_Deselector && iLumEnable_Flag[iColorSpaceTab])
           {
              if (iLum_Deselected)
                  iLum_Deselected = 0;
              else
              {
                  iLum_Deselected = 1;
              }
           }
           else
             ToggleMenu('T', &iLumEnable_Flag[iColorSpaceTab], IDM_LUMINANCE);

           Lum_Filter_Init(iColorSpaceTab);
           RefreshVideoFrame();
           break;
/*
      case IDL_LUM_BC_UP:
          Lum_BC_Adj(+5);
          RefreshVideoFrame();
          break;

      case IDL_LUM_BC_DOWN:
          Lum_BC_Adj(-5);
          RefreshVideoFrame();
          break;
*/
      case IDL_LUM_BOLD:
          Lum_Bold();
          RefreshVideoFrame();
          break;

      case IDL_LUM_C:
          Lum_C();
          RefreshVideoFrame();
          break;

      case IDL_LUM_DEFAULT:
          Lum_Default();
          RefreshVideoFrame();
          break;

      case IDM_LUMINANCE:
           iBMP_Wanted = 0;
           if (hLumDlg!=NULL)
           {
              DestroyWindow(hLumDlg);
              hLumDlg = NULL;
              //MessageBeep(MB_OK) ;
           }
           else
           {
              if (!hCSR_CROSS)
              {
                   hCSR_CROSS = LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS));
                   SysErrNum = GetLastError();
                   if (SysErrNum && DBGflag)
                       Msg_LastError("SetCursor RC=", SysErrNum, 'b');
              }
              hLumDlg = CreateDialog(hInst, (LPCTSTR)IDD_LUMINANCE,
                                          hWnd_MAIN, (DLGPROC)Luminance_Dialog);
           }
           break;

      case IDM_LUM_DESEL:
          ToggleMenu('T', &iCtl_Lum_Deselector, IDM_LUM_DESEL);

          if (!iCtl_Lum_Deselector
          || (process.CurrFile  <  process.ToViewFile)
          || (process.CurrFile  == process.ToViewFile
              && process.CurrLoc < process.ToViewLoc)
             )
              iLum_Deselected = 0;
          else
              iLum_Deselected = 1;
               
          Lum_Filter_Init(0);
          Lum_Filter_Init(1);
          RefreshVideoFrame();
          break;
   

      case IDM_VIEW_NEGATIVE:
          Lum_Negative(1);
          break;


      case IDM_SAT_RETAIN:
           ToggleMenu('T', &iCtl_SAT_Retain, IDM_SAT_RETAIN);
           break;

      case IDM_SINE:  // SIN CITY - Still experimental
          ToggleMenu('T', &iSat_Sine, IDM_SINE);
          if (iSat_Sine) 
              iSatAdj_Flag  = 1;    //    [iColorSpaceTab]
          Lum_Filter_Init(0);
          Lum_Filter_Init(1);
          RefreshVideoFrame();
          break;



      case IDM_VIEW_INVERT:
          ToggleMenu('T', &iView_Invert, IDM_VIEW_INVERT);

          if (MParse.SeqHdr_Found_Flag 
          && (MParse.Stop_Flag || MParse.Tulebox_SingleStep_flag))
          {
             if (DDOverlay_Flag && MParse.iColorMode!=STORE_RGB24) // && iShowVideo_Flag)
                 RenderYUY2(1);
             else
                 RenderRGB24();
          }

          break;


      case ID_VIEW_TOOL_TOGGLE:  // Buttons & Scrollbar
//         if (iViewToolBar)
//             iViewToolBar = 0;
//         else
//             iViewToolBar = 257;
//
//         Toolbar_Chg();
//         break;

      case ID_VIEW_TOOL_BOTH:  // Buttons & Scrollbar
         if (iViewToolBar != 257)
             iViewToolBar  = 257;
         else
             iViewToolBar = 0;
         Toolbar_Chg();
         break;

      case ID_VIEW_TOOL_NONE:  // Buttons & Scrollbar
         if (iViewToolBar != 0)
             iViewToolBar  = 0;
         else
             iViewToolBar = 257;
         Toolbar_Chg();
         break;

      case ID_VIEW_BUTTONSONLY:     // Buttons Only
         if (iViewToolBar != 256)
             iViewToolBar  = 256;
         else
             iViewToolBar = 0;

         Toolbar_Chg();
         break;

      case ID_VIEW_SCROLLONLY:    // Scrollbar Only
         if (iViewToolBar != 1)
             iViewToolBar  = 1;
         else
             iViewToolBar = 0;

         Toolbar_Chg();
         break;


 
      case IDM_POSTPROC:

           iRC = DialogBox(hInst, (LPCTSTR)IDD_POSTPROC,
                                   hWnd_MAIN, (DLGPROC)PostProc_Dialog);
           break;



      case IDM_PLUGINS:

           iRC = DialogBox(hInst, (LPCTSTR)IDD_EXIT_CTL,
                                   hWnd_MAIN, (DLGPROC)ExitCtl_Dialog);
           break;



      case IDM_FILE_NEWNAME:

           F600_NewName_Setup();
           DSP2_Main_SEL_INFO(1);
           break;



      case IDM_MULTI_ANGLE:
           ToggleMenu('T', &iCtl_MultiAngle, IDM_MULTI_ANGLE);
           break;


      case IDM_VIDTRK_AUTO:
            Set_Video_Stream(STREAM_AUTO,               IDM_VIDTRK_AUTO);
            break;
      case IDM_VIDTRK_ALL:
            Set_Video_Stream(STREAM_ALL,                IDM_VIDTRK_ALL);
            break;
      case IDM_VIDTRK_1:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_1, IDM_VIDTRK_1);
            break;
      case IDM_VIDTRK_2:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_2, IDM_VIDTRK_2);
            break;
      case IDM_VIDTRK_3:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_3, IDM_VIDTRK_3);
            break;
      case IDM_VIDTRK_4:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_4, IDM_VIDTRK_4);
            break;
      case IDM_VIDTRK_5:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_5, IDM_VIDTRK_5);
            break;
      case IDM_VIDTRK_6:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_6, IDM_VIDTRK_6);
            break;
      case IDM_VIDTRK_7:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_7, IDM_VIDTRK_7);
            break;
      case IDM_VIDTRK_8:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_8, IDM_VIDTRK_8);
            break;
      case IDM_VIDTRK_9:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_9, IDM_VIDTRK_9);
            break;
      case IDM_VIDTRK_10:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_10, IDM_VIDTRK_10);
            break;
      case IDM_VIDTRK_11:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_11, IDM_VIDTRK_11);
            break;
      case IDM_VIDTRK_12:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_12, IDM_VIDTRK_12);
            break;
      case IDM_VIDTRK_13:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_13, IDM_VIDTRK_13);
            break;
      case IDM_VIDTRK_14:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_14, IDM_VIDTRK_14);
            break;
      case IDM_VIDTRK_15:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_15, IDM_VIDTRK_15);
            break;
      case IDM_VIDTRK_16:
            Set_Video_Stream(VIDEO_ELEMENTARY_STREAM_16, IDM_VIDTRK_16);
            break;


      case IDM_PID_ALL:
           uVid_PID_All = 1;
           uCtl_Vid_PID  = STREAM_AUTO;
           CheckMenuItem(hMenu, IDM_PID_CURRENT, MF_UNCHECKED);
           CheckMenuItem(hMenu, IDM_PID_ALL,  MF_CHECKED);
           break;

      case IDM_PID_CURRENT:
           uVid_PID_All = 0;
           CheckMenuItem(hMenu, IDM_PID_CURRENT, MF_CHECKED);
           CheckMenuItem(hMenu, IDM_PID_ALL,  MF_UNCHECKED);
           break;
      /*
      case IDM_PID_ALL:
            Set_VPIDs(STREAM_ALL,                    IDM_PID_ALL);
            break;
      case IDM_PID_AUTO:
            Set_VPIDs(STREAM_AUTO,                   IDM_PID_AUTO);
            break;
      case IDM_PID_1:
            Set_VPIDs(0, IDM_PID_1);
            break;
      case IDM_PID_2:
            Set_VPIDs(1, IDM_PID_2);
            break;
      case IDM_PID_3:
            Set_VPIDs(2, IDM_PID_3);
            break;
      case IDM_PID_4:
            Set_VPIDs(3, IDM_PID_4);
            break;
      case IDM_PID_5:
            Set_VPIDs(4, IDM_PID_5);
            break;
      case IDM_PID_6:
            Set_VPIDs(5, IDM_PID_6);
            break;
      case IDM_PID_7:
            Set_VPIDs(6, IDM_PID_7);
            break;
      case IDM_PID_8:
            Set_VPIDs(7, IDM_PID_8);
            break;
      case IDM_PID_9:
            Set_VPIDs(8, IDM_PID_9);
            break;
      case IDM_PID_10:
            Set_VPIDs(9, IDM_PID_10);
            break;
      case IDM_PID_11:
            Set_VPIDs(10, IDM_PID_11);
            break;
      case IDM_PID_12:
            Set_VPIDs(11, IDM_PID_12);
            break;
      case IDM_PID_13:
            Set_VPIDs(12, IDM_PID_13);
            break;
      case IDM_PID_14:
            Set_VPIDs(13, IDM_PID_14);
            break;
      case IDM_PID_15:
            Set_VPIDs(14, IDM_PID_15);
            break;
      case IDM_PID_16:
            Set_VPIDs(15, IDM_PID_16);
            break;
      */

      case IDM_AUDPID_NONE:
           uCtl_Aud_PID = STREAM_NONE;
           uAud_PID_All = 0;
           CheckMenuItem(hMenu, IDM_AUDPID_AUTO,  MF_UNCHECKED);
           CheckMenuItem(hMenu, IDM_AUDPID_NONE,  MF_CHECKED);
           break;

      case IDM_AUDPID_AUTO:
           uCtl_Aud_PID = STREAM_AUTO;
           uAud_PID_All = 1;
           CheckMenuItem(hMenu, IDM_AUDPID_AUTO,  MF_CHECKED);
           CheckMenuItem(hMenu, IDM_AUDPID_NONE,  MF_UNCHECKED);
           break;


      case IDM_WHEEL_NONE:
           Set_Wheel_Scroll(-1);
           break;
      case IDM_WHEEL_FRAME:
           Set_Wheel_Scroll(0);
           break;
      case IDM_WHEEL_GOP:
           Set_Wheel_Scroll(4);
           break;
      case IDM_WHEEL_JUMP:
           Set_Wheel_Scroll(3);
           break;
      case IDM_WHEEL_JUMP2:
           Set_Wheel_Scroll(2);
           break;
      case IDM_WHEEL_JUMP4:
           Set_Wheel_Scroll(1);
           break;


      case IDM_KBNAV_VDUB:
         ToggleMenu('T', &iCtl_KB_NavOpt, IDM_KBNAV_VDUB);
         DSP_Button_Abbr();
         if (iViewToolBar)
         {
            ToolBar_Destroy();
            ToolBar_Create();
         }
         break;

      case IDM_KBMARK_VDUB:
         ToggleMenu('T', &iCtl_KB_MarkOpt, IDM_KBMARK_VDUB);
         break;
      case IDM_KB_STOPPLAY:
         ToggleMenu('T', &iCtl_KB_NavStopPlay, IDM_KB_STOPPLAY);
         break;

      case IDM_KB_V_POPUP:
         ToggleMenu('T', &iCtl_KB_V_Popup, IDM_KB_V_POPUP);
         break;


      case IDM_NAV_INDEX:
         ToggleMenu('T', &iCTL_FastBack, IDM_NAV_INDEX);
         break;

 //     case IDM_VOB_NAV:
 //        ToggleMenu('T', &iVob_Nav, IDM_VOB_NAV);
 //        break;


/*
      case IDM_CLIPRESIZE:
               if (hClipResizeDlg!=NULL)
               {
                  DestroyWindow(hClipResizeDlg);
                  hClipResizeDlg = NULL;
               }
               else
                  hClipResizeDlg = CreateDialog(hInst, (LPCTSTR)IDD_CLIPRESIZE, hWnd, (DLGPROC)ClipResize);
               break;
*/
/*
      case IDM_NORM:
               if (hNormDlg!=NULL)
               {
                  DestroyWindow(hNormDlg);
                  hNormDlg = NULL;
               }
               else
                  hNormDlg = CreateDialog(hInst, (LPCTSTR)IDD_NORM, hWnd, (DLGPROC)Normalization);
               break;
*/

 
      case IDM_STATISTICS:   // Toggls STATS screen
               if (MParse.ShowStats_Flag)
               {
                  Stats_Kill();
                  View_Rebuild_Chk(0);
               }
               else
                  Stats_Show(true, 1);
               break;



      case IDM_DISPLAY:
           ToggleMenu('T', &iCtl_ShowVideo_Flag, IDM_DISPLAY);
           iShowVideo_Flag = iCtl_ShowVideo_Flag;

           if (process.Action == ACTION_RIP
           &&  !MParse.Tulebox_SingleStep_flag)
           {
               Mpeg_Drop_Init();
           }
           break;


      case IDM_PLAY_INFO:
          ToggleMenu('T', &iCtl_Play_Info, IDM_PLAY_INFO);
          break;

      case IDM_BMP_SHIFT:
      case IDM_BMP_ASIS:
      case IDM_BMP_THUMB:
      case IDM_BMP_CLIPBOARD:

           if (!File_Limit)
               break;

           if (!iBMP_Preview)
           {
               if (File_Limit)
                   SNAP_Save(wmId, NULL); // (ThumbRec*)lParam) ;
               else
                   MessageBeep(MB_OK);

               break;
           }

           if (iBMP_Preview == 1)
               iBMP_Preview  = 0;

      case IDM_BMP_SNAP_PREVIEW:
           iBMP_Wanted = 1;
           if (hLumDlg!=NULL)
           {
              DestroyWindow(hLumDlg);
              hLumDlg = NULL;
              MessageBeep(MB_OK) ;
           }
           else
           {
              hLumDlg = CreateDialog(hInst, (LPCTSTR)IDD_LUMINANCE,
                                          hWnd_MAIN, (DLGPROC)Luminance_Dialog);
           }
           break;


      case IDM_BMP_PREVIEW_OFF:
           Set_Bmp_AutoPreview(0);
           break;
      case IDM_BMP_PREVIEW_1:
           Set_Bmp_AutoPreview(1);
           break;
      case IDM_BMP_PREVIEW_ALL:
           Set_Bmp_AutoPreview(2);
           break;

      case IDM_BMP_FMT_BICUBIC:
           Set_Bmp_Fmt(0);
           break;
      case IDM_BMP_FMT_SUBSAMPLE:
           Set_Bmp_Fmt(1);
           break;
      case IDM_BMP_FMT_RAW:
           Set_Bmp_Fmt(2);
           break;




      case IDM_TIME_REL:
           Set_Time_Fmt(1);
           break;
      case IDM_TIME_GOP:
           Set_Time_Fmt(2);
           break;
      case IDM_TIME_PTS:
           Set_Time_Fmt(3);
           break;
      case IDM_TIME_FILE:
           Set_Time_Fmt(4);
           break;
      case IDM_TIME_SCR:
           Set_Time_Fmt(5);
           break;
      case IDM_TIME_HEX:
           Set_Time_Fmt(6);
           break;
      case IDM_TIME_TOD_REL:
           Set_Time_Fmt(7);
           break;

      case IDM_DATE_INTERNATIONALE:
           ToggleMenu('T', &iCtl_Date_Internationale, IDM_DATE_INTERNATIONALE);
           break;

      case IDM_READABILITY:
           ToggleMenu('T', &iCtl_Readability, IDM_READABILITY);
           if (iViewToolBar > 1)
           {
               Toolbar_Chg();
               //ToolBar_Destroy();
               //ToolBar_Create();
           }
           break;


      case IDM_F3_NAMES:
           ToggleMenu('T', &iCtl_F3_Names, IDM_F3_NAMES);
           break;
      case IDM_F5_TOGGLER:
           ToggleMenu('T', &iCtl_F5_Toggler, IDM_F5_TOGGLER);
           break;

 

      case IDM_ASSOCIATE:
           Ini_Associate();
           break;

      case IDM_PREFERENCES:
           DialogBox(hInst, (LPCTSTR)IDD_PREFERENCES, hWnd_MAIN, 
                            (DLGPROC)Preferences_Dialog);
           break;


      case IDM_ACT_MEDIA_PLAYER_CLASSIC:
           P9_EXT_ACT(0);
           break;

      case IDM_ACT_WMP2:
           P9_EXT_ACT(1);
           break;

      case IDM_ACT_WMP:
           P9_EXT_ACT(2);
           break;

      case IDM_ACT_CREATIVE:
           P9_EXT_ACT(3);
           break;

      case IDM_ACT_POWERDVD:
           P9_EXT_ACT(4);
           break;

      case IDM_ACT_VLC:
           P9_EXT_ACT(5);
           break;

      case IDM_ACT_BBINFO2:
           P9_EXT_ACT(6);
           break;


      case ID_LEFT_SHIFT:  // Left Bracket with Shift

           Mpeg_MaybeStop_Rqst();

           if (iCtl_KB_NavOpt) iKick.Action = ACTION_BWD_GOP2 ;
           else                iKick.Action = ACTION_BWD_JUMP2;

           MPEG_processKick();

           break;


      case ID_LEFT_ARROW:

           Mpeg_MaybeStop_Rqst();

           if (iCtl_KB_NavOpt) iKick.Action = ACTION_BWD_GOP;
           else                iKick.Action = ACTION_BWD_JUMP;

           MPEG_processKick();

           break;


      case ID_RIGHT_SHIFT:  // Right Bracket with shift

           Mpeg_MaybeStop_Rqst(); 

           if (iCtl_KB_NavOpt) iKick.Action = ACTION_FWD_GOP2;
           else                iKick.Action = ACTION_FWD_JUMP2;

           MPEG_processKick();

           break;



      case ID_RIGHT_ARROW:

           Mpeg_MaybeStop_Rqst();

           if (iCtl_KB_NavOpt) iKick.Action = ACTION_FWD_GOP;
           else                iKick.Action = ACTION_FWD_JUMP;

           MPEG_processKick(); 

           break;


      // JUMP forward to start of next file
      case ID_RIGHT_CTRL:
 
           Mpeg_MaybeStop_Rqst(); 

           iKick.Action  = ACTION_SKIP_FILE;

           MPEG_processKick();

           break;
 

      case ID_UP_SHIFT:

           Mpeg_MaybeStop_Rqst();

           if (iCtl_KB_NavOpt) iKick.Action = ACTION_BWD_JUMP2;
           else                iKick.Action = ACTION_FWD_GOP2;

           MPEG_processKick();

           break;


      case ID_UP_ARROW:

           Mpeg_MaybeStop_Rqst();

           if (iCtl_KB_NavOpt) iKick.Action = ACTION_BWD_JUMP;
           else                iKick.Action = ACTION_FWD_GOP;

           MPEG_processKick();

           break;



      // JUMP back to a start of file based on last SEARCH start point
      case ID_LEFT_CTRL:

           Mpeg_MaybeStop_Rqst();

           iKick.File = process.CurrFile ;

           if (process.CurrLoc  < process.ByteRateAvg[process.CurrFile]
           ||  process.CurrFile < File_Ctr)
               if (process.CurrFile > 0)
                   iKick.File--;

           if (iKick.File < 0)
               MessageBeep(MB_OK);
           else
           {
             iKick.Action = ACTION_NEW_CURRLOC;
             iKick.Loc    = 0 ;

             process.CurrFile = iKick.File ;
             process.CurrLoc  = iKick.Loc  ;

             MPEG_processKick();

           }

           break;


      case ID_DOWN_SHIFT:

           Mpeg_MaybeStop_Rqst();

           if (iCtl_KB_NavOpt == 1) iKick.Action = ACTION_FWD_JUMP2;
           else                     iKick.Action = ACTION_BWD_GOP2;

           MPEG_processKick();

           break;



      case ID_DOWN_ARROW:

           Mpeg_MaybeStop_Rqst();

           if (iCtl_KB_NavOpt == 1) iKick.Action = ACTION_FWD_JUMP;
           else                     iKick.Action = ACTION_BWD_GOP;

           MPEG_processKick(); 

           break;



//                  Advance 1 frame

      case ID_FWD_FRAME:

           MParse.Tulebox_SingleStep_flag = 1;
           MParse.Tulebox_prev_frame_number = Frame_Number;

           // If already playing, treat like Pause button
           if (process.Action == ACTION_RIP && ! MParse.Stop_Flag)
           {
              if (MParse.Pause_Flag)
              {
                  MParse.Pause_Flag = 0;
                  ResumeThread(hThread_MPEG);
              }

           }
           else
           {
              if (! MParse.Stop_Flag)
              {
                  MParse.Stop_Flag = 2;
                  Sleep(75); // Allow other task to stop
              }

              B510_PLAY_HERE();
           }


        break;


      // The old code for single frame stepping 
      // should the frames in physical file order,
      // which is not the correct temporal order.
      // 
      // Never did figure out exactly how temporal order is controlled.

      case ID_FWD_FRAME_OLD:

           Mpeg_Stop_Rqst();

           // Disallow marking in middle of GOP

           EnableWindow(hMarkLeft, false);
           EnableWindow(hMarkRight, false);

           iKick.Action = ACTION_FWD_FRAME;

           MPEG_processKick();

           break;



//    Backward a frame by toggling buffers

      case ID_BWD_FRAME:
           if (IsWindowEnabled(hTrack)
           &&  DDOverlay_Flag && MParse.iColorMode==STORE_YUY2) // && iShowVideo_Flag)
               RenderYUY2(-1);
           else
              MessageBeep(MB_OK);

           break;



      case IDM_CLIP_FROM:
      case ID_FROM_MARK:
         C510_Sel_FROM_MARK(1);
         break;


      case IDM_CLIP_TO:
      case ID_TO_MARK:
         C520_Sel_TO_MARK();
         break;


         // JUMP TO THE FROM MARKER
      case ID_FROM_KEY:
      case ID_FROM_SHIFT:
         if (wmId == (DWORD)(iKB_MARK_FROM))
             C510_Sel_FROM_MARK(1);
         else
            B570_GO_FROM();
         break;


      case ID_TO_KEY:
      case ID_TO_SHIFT:
         if (wmId == (DWORD)(iKB_MARK_TO))
            C520_Sel_TO_MARK();
         else
            B580_GO_TO();
         break;

      case IDM_CLIP_SPLIT:
           Mpeg_Stop_Rqst(); 
           C600_Clip_Split();
           break;



/*
      case IDM_SRC_NONE:
               SRC_Flag = SRC_NONE;
               CheckMenuItem(hMenu, IDM_SRC_NONE, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_SRC_LOW, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_MID, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_HIGH, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_UHIGH, MF_UNCHECKED);
               break;


      case IDM_SRC_LOW:
               SRC_Flag = SRC_LOW;
               CheckMenuItem(hMenu, IDM_SRC_NONE, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_LOW, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_SRC_MID, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_HIGH, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_UHIGH, MF_UNCHECKED);
               break;


      case IDM_SRC_MID:
               SRC_Flag = SRC_MID;
               CheckMenuItem(hMenu, IDM_SRC_NONE, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_LOW, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_MID, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_SRC_HIGH, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_UHIGH, MF_UNCHECKED);
               break;


      case IDM_SRC_HIGH:
               SRC_Flag = SRC_HIGH;
               CheckMenuItem(hMenu, IDM_SRC_NONE, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_LOW, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_MID, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_HIGH, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_SRC_UHIGH, MF_UNCHECKED);
               break;


      case IDM_SRC_UHIGH:
               SRC_Flag = SRC_UHIGH;
               CheckMenuItem(hMenu, IDM_SRC_NONE, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_LOW, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_MID, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_HIGH, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_SRC_UHIGH, MF_CHECKED);
               break;
*/
/*      case IDM_KEY_OFF:
               KeyOp_Flag = KEY_OFF;
               CheckMenuItem(hMenu, IDM_KEY_OFF, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_KEY_INPUT, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_KEY_OP, MF_UNCHECKED);
               break;


      case IDM_KEY_INPUT:
               KeyOp_Flag = KEY_INPUT;
               CheckMenuItem(hMenu, IDM_KEY_OFF, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_KEY_INPUT, MF_CHECKED);
               CheckMenuItem(hMenu, IDM_KEY_OP, MF_UNCHECKED);
               break;


      case IDM_KEY_OP:
               KeyOp_Flag = KEY_OP;
               CheckMenuItem(hMenu, IDM_KEY_OFF, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_KEY_INPUT, MF_UNCHECKED);
               CheckMenuItem(hMenu, IDM_KEY_OP, MF_CHECKED);
               break;
*/

      default:
               MessageBeep(MB_OK) ;
               sprintf(szMsgTxt, "Unk Inp Code = %d ", wmId);

               // MessageBox(hWnd, szBuffer, "RJ DEBUGS", MB_OK);
               return DefWindowProc(hWnd_MAIN, message, wParam, lParam);
   }

 return false;
}





//---------------------------


void B380_Volume_Window()
{
           if (hVolDlg!=NULL)
           {
              DestroyWindow(hVolDlg);
              hVolDlg = NULL;
              //MessageBeep(MB_OK) ;
           }
           else
           {
              hVolDlg = CreateDialog(hInst, (LPCTSTR)IDD_VOLUME,
                                 hWnd_MAIN, (DLGPROC)Volume_Dialog);
           }
  
}





//---------------------------


void B390_Audio_Cycle()
{
  unsigned int uTmp1;
  int iTmp2;

  uTmp1 = iAudio_SEL_Track + 1;

  for(;;uTmp1++)
  {
       if (uTmp1 > 7)
       {
           uTmp1 = 0;
       }

       if (uTmp1 == iAudio_SEL_Track)
       {
           sprintf(szMsgTxt, AUDIO_NO_OTHER_TRK);
           DSP1_Main_MSG(0,1);
           break;
       }
       else
       {
           iTmp2 = iAudio_Trk_FMT[uTmp1];

           if (iTmp2)
           {
               iWant_Aud_Format  = iTmp2;
               iAudio_SEL_Track  = uTmp1;
               iAudio_SEL_Format = iTmp2;
               sprintf(szMsgTxt, "Trk#%d", iAudio_SEL_Track+1);
               DSP1_Main_MSG(0,0);
               break;
           }
       }


  }

  iVol_PREV_Cat = 0;

}

//----------------------------------------------------------
void B400_Key_USER(WPARAM wParam)
{
#define VK_0 48 // 0x30
#define VK_1 49 // 0x31
#define VK_9 57 // 0x39

unsigned int uTmp1;

//int iRC;
  if (DBGflag)
  {
     sprintf(szMsgTxt, "Key#%d =x%04X ='%c'", wParam, wParam, wParam);
     DSP1_Main_MSG(0,0);
  }

  if (wParam == VK_SHIFT    ||  wParam == VK_CONTROL  // ||  wParam == VK_ALT
  ||  wParam == VK_RWIN     ||  wParam == VK_LWIN
  ||  wParam == VK_APPS     ||  wParam == VK_CAPITAL
  ||  wParam == VK_MULTIPLY ||  wParam == ID_CUE2_FWD
  ||  wParam == VK_F5
  ||  wParam == 'V')  
  //  wParam == 16        ||  wParam == 17) // Shift  and Ctrl keys
  {
  }
  else
  if ( ! File_Limit)
  {
     MessageBeep(MB_OK);
  }
  else
  if (wParam == 'a' || wParam == 'A') // Audio Track Cycle
  {
     B390_Audio_Cycle();
  }
  else

         // StarKey special function keys
  if (wParam == 173)
  {
    if (iCtl_Vol_StarKey)
        VOL210_MUTE_Toggle();
  }
  else
  if (wParam == 174)
  {
    if (iCtl_Vol_StarKey)
            VOL320_Down();
  }
  else
  if (wParam == 175)
  {
    if (iCtl_Vol_StarKey)
            VOL340_Up();
  }
  else
  if (wParam == 178)
  {
            Mpeg_Stop_Rqst();
  }
  else
  if (wParam == 179)  // Play/Pause Toggler
  {
            B501_Play_Button(1);
  }
  else
  if (wParam == VK_SNAPSHOT
  ||  wParam == 'B'
  ||  wParam == 'b')  // Allow for keyboard accelerator missing lower-case B
  {
        if (File_Limit
        && (MParse.Stop_Flag || MParse.Pause_Flag))
        {
            if (wParam == 'B') // Ascii 66
                uTmp1 = IDM_BMP_SHIFT;
            else
                uTmp1 = IDM_BMP_ASIS;

            SNAP_Save(uTmp1, NULL);
        }
        else
            MessageBeep(MB_OK);

  }
  else
  if (wParam != '/' && wParam != 0xBF) // Allow for Tulebox suspended task
  {
    if (hThread_MPEG && iCtl_KB_NavStopPlay // Allow scroll keys to interrupt while playing
    &&  ! MParse.Stop_Flag)
    {
          Mpeg_Stop_Rqst();
          return;
    }
    

    switch (wParam)
    {
         // NUMPAD Keys
         case VK_UP:

            if (WaitForSingleObject(hThread_MPEG, 0)==WAIT_OBJECT_0)
            {
                iShowVideo_Flag = iCtl_ShowVideo_Flag;

                process.Action = ACTION_BWD_JUMP;
                hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);
            }
            break;

         case VK_DOWN:

            if (WaitForSingleObject(hThread_MPEG, 0)==WAIT_OBJECT_0)
            {
                iShowVideo_Flag = iCtl_ShowVideo_Flag;

                process.Action = ACTION_FWD_JUMP;
                hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);
            }
            break;


         case VK_LEFT:
         case 166:  // StarKey Back 

           Mpeg_MaybeStop_Rqst();
           iKick.Action = ACTION_BWD_GOP;
           MPEG_processKick(); 

           /*
            if (WaitForSingleObject(hThread_MPEG, 0)==WAIT_OBJECT_0)
            {
                iShowVideo_Flag = iCtl_ShowVideo_Flag;

                process.Action = ACTION_BWD_GOP;
                hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);
            }
            */

            break;

         case VK_RIGHT:
         case 167:  // StarKey Forward 

           Mpeg_MaybeStop_Rqst();
           iKick.Action = ACTION_FWD_GOP;
           MPEG_processKick(); 

           /*
            if (WaitForSingleObject(hThread_MPEG, 0) == WAIT_OBJECT_0)
            {
                iShowVideo_Flag = iCtl_ShowVideo_Flag;

                process.Action = ACTION_FWD_GOP;
                hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);
            }
            */

            break;



         case VK_PRIOR:  // Page up   PGUP
         case 177:       // StarKey Fast Bwd

            //if (WaitForSingleObject(hThread_MPEG, 0)==WAIT_OBJECT_0)
            {
                iShowVideo_Flag = iCtl_ShowVideo_Flag;

                //process.Action = ACTION_BWD_JUMP2;
                //hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);

                Mpeg_MaybeStop_Rqst();
                iKick.Action = ACTION_BWD_JUMP2;
                MPEG_processKick();

            }
            break;

         case VK_NEXT:  // Page Down  PGDOWN
         case 176:       // StarKey Fast Fwd

            //if (WaitForSingleObject(hThread_MPEG, 0)==WAIT_OBJECT_0)
            {
                iShowVideo_Flag = iCtl_ShowVideo_Flag;

                //process.Action = ACTION_FWD_JUMP2;
                //hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);

                Mpeg_MaybeStop_Rqst();
                iKick.Action = ACTION_FWD_JUMP2;
                MPEG_processKick();

            }
            break;



         case VK_EREOF:  // IBM 3270 Erase to OF
             C520_Sel_TO_MARK();
            break;


         default:

            int iTmp1;
            if (wParam >= VK_0  &&  wParam <= VK_9)
            {
                // Jump to a Clip
                C320_Sel2Clip();
                if (wParam == VK_0)
                  iTmp1 = iEDL_ctr;
                else
                {
                  iTmp1 = wParam - VK_1;
                  if (iEDL_ctr > 9) // Quick and dirty fudge
                     iTmp1 = iTmp1 + iEDL_ctr - 9 ;
                }

                if (iTmp1 <= iEDL_ctr  && iTmp1 >= 0)
                {
                     iKick.File   = EDList.FromFile[iTmp1] ;
                     iKick.Loc    = EDList.FromLoc[iTmp1] ;
                     iKick.Action = ACTION_NEW_CURRLOC ;

                     process.uGOP_TCorrection = EDList.uFrom_TCorrection[iTmp1];
                     process.uGOP_FPeriod_ps  = EDList.uFrom_FPeriod_ps [iTmp1];
                     iFrame_Period_ps         = process.uGOP_FPeriod_ps;
         
                     MPEG_processKick(); //hThread_MPEG) ;
                                         //iKick.Action, iKick.File, iKick.Blk) ;
                }
            }
            else
            if (DBGflag)
            {
                sprintf(szMsgTxt, "UNK KEY=%d =x%02X =%c", wParam, wParam, wParam);
                DSP1_Main_MSG(0,0);
            }

            break;

    }// END SWITCH

  } //END_IF GotFile

}




//-----------------------------------

void B500_CONTINUE_PLAY_HERE()
{
  MParse.Tulebox_SingleStep_flag = 0;

  if (MParse.Pause_Flag)
  {
      iRender_TimePrev  = 0;
      MParse.Pause_Flag = 0;
      ResumeThread(hThread_MPEG);
  }
  else
  if (MParse.Stop_Flag)
      B510_PLAY_HERE();
}



void B501_Play_Button(int P_Toggler)
{
           if ((iCtl_F5_Toggler || P_Toggler)
           &&   !MParse.Stop_Flag  )
               Mpeg_Stop_Rqst();
           else
           {
               B555_Normal_Speed(0);

               iRender_TimePrev  = 0;
               if (MParse.Tulebox_SingleStep_flag)
               {
                 MParse.Tulebox_SingleStep_flag = 0;
                 Mpeg_Drop_Init();
               }

               B500_CONTINUE_PLAY_HERE();
           }
}



void B510_PLAY_HERE()
{
  int iRC;

  iPreview_Clip_Ctr = 999;


  iRC = Get_Hdr_Loc(&process.startLoc, &process.startFile);
  if (iRC < 0)
  {
           process.startFile  =  process.CurrFile;
           process.startLoc   =  process.CurrLoc;
  }

  B550_PLAY(ACTION_PLAY);

}


void B52_AudioChk()
{
        if (iWantAudio
        && (iMPALib_Status || byAC3_Init)
        &&  MParse.FastPlay_Flag <= MAX_WARP )
            iPlayAudio = 1;
        else
            iPlayAudio = 0;
}

//-----------------------------------

void B550_PLAY(int P_Mode)
{

  if (MParse.Pause_Flag)
  {
      MParse.Stop_Flag = true;
      MParse.Tulebox_SingleStep_flag = 0;
      MParse.Pause_Flag = 0;
      ResumeThread(hThread_MPEG);
      Sleep(10);
  }

  VGA_GetSize();
  Calc_PhysView_Size();


  if ( ! File_Limit
        //||  (MParse.SystemStream_Flag < 0 && ! iPES_Mpeg_Any)
  ||  iBusy  )
  {
      MessageBeep(MB_OK);
  }
  else
  if (! MParse.Stop_Flag)
        MParse.Stop_Flag = true;
  else
  //if (IsWindowEnabled(hTrack) && MParse.Stop_Flag)
  {
      iShowVideo_Flag = iCtl_ShowVideo_Flag;

      // Load Audio decoder only when needed
      /*if ( iMPALib_Status < 0 && (iAudio_SEL_Track != TRACK_NONE) )
            MPALib_Init(NULL);
        if (byMPALib_OK) */

        B52_AudioChk();

        if (iMsgLife <= 0)
            iMsgLife  = 1;
        else
            iMsgLife += 1;

        if (iViewToolBar <= 256)
        {
          if (iPreview_Clip_Ctr >= 900 )
              iMsgLife += 3;
          //else
          //if (iMainWin_State > 0)
          //     DSP_Msg_Clear();
        }


        /*
        EnableMenuItem(hMenu, IDM_OPEN,       MF_GRAYED);
        EnableMenuItem(hMenu, IDM_SAVE,       MF_GRAYED);
        EnableMenuItem(hMenu, IDM_FILE_NAMES, MF_GRAYED);
        //EnableMenuItem(hMenu, IDM_LOAD_D2V, MF_GRAYED);
        //EnableMenuItem(hMenu, IDM_SAVE_D2V, MF_GRAYED);
        */

        EnableMenuItem(hMenu, IDM_STOP, MF_ENABLED);

        Menu_Main_Disable(false, false);
        if (iCtl_Play_Info)
            Stats_Show(true, 0);
        process.Action = P_Mode;

        if (WaitForSingleObject(hThread_MPEG, 0)==WAIT_OBJECT_0)
            hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);
    }

}


void  B555_Normal_Speed(int P_Tell)
{
  int iTmp1;

  MParse.SlowPlay_Flag = 0;
  MParse.FastPlay_Flag = 0;

  CheckMenuItem(hMenu, IDM_PLAY_SLOWER,  MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_PLAY_SLOW_1,  MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_PLAY_SLOW_2A, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_PLAY_FASTER,  MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_PLAY_FAST_1,  MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_PLAY_FAST_2,  MF_UNCHECKED);

  if (MParse.ShowStats_Flag)
     SetDlgItemText(hStats, IDC_MPAdec_NAME, " ");

  // Decrease the chance of landing in a B-FRAME
  if (PlayCtl.iDrop_B_Frames_Flag)
  {
      if (MPEG_Pic_Type == B_TYPE)
      {
         if (PREV_Pic_Type !=  B_TYPE)
           iTmp1 = 60;
         else
           iTmp1 = 30;
         Sleep(iTmp1);
      }
  } 

  PlayCtl.iDrop_Behind   = iCtl_Drop_Behind;
  PlayCtl.iDrop_B_Frames_Flag = 0;

  B52_AudioChk();

  Set_Priority(hThread_MPEG, iCtl_Priority[1],  1,  1);

  if (P_Tell)
  {
      strcpy(szMsgTxt, "Normal Speed");
      DSP1_Main_MSG(0,0);
  }


}




//------------------------------------------------

void B570_GO_FROM()
{
// if (IsWindowEnabled(hTrack))
// {
  if ( ! File_Limit)
  {
    MessageBeep(MB_OK);
  }
  else
  {
    if (hThread_MPEG)
        MParse.Stop_Flag = 2;  // Allow scroll while playing

    //Ed_Prev_Act = '{';
    C310_Pos_MEMO(201);

    SetFocus(hWnd_MAIN);

    iKick.File   = process.FromFile ;
    iKick.Loc    = process.FromLoc ;
    iKick.Action = ACTION_NEW_CURRLOC ;

    process.CurrFile = iKick.File ;
    process.CurrLoc  = iKick.Loc  ;

    process.uGOP_TCorrection = EDList.uFrom_TCorrection[iEDL_ctr];
    process.uGOP_FPeriod_ps  = EDList.uFrom_FPeriod_ps [iEDL_ctr];
    iFrame_Period_ps         = process.uGOP_FPeriod_ps;
         
    MPEG_processKick();       //hThread_MPEG) ;
                              //     iKick.Action, iKick.File, iKick.Blk) ;


  }  // ENDIF Got File/s
}





//----------------------------------
void B580_GO_TO()
{
//if (IsWindowEnabled(hTrack))
//{
  if ( ! File_Limit)
  {
     MessageBeep(MB_OK);
  }
  else
  {
    if (hThread_MPEG)
        MParse.Stop_Flag = 2;  // Allow scroll while playing

    //Ed_Prev_Act = '}';
    C310_Pos_MEMO(201);

    SetFocus(hWnd_MAIN);

    iKick.File    = process.ToViewFile ;
    //iKick.Blk     = process.ToViewBlk - 1 ;
    iKick.Loc     = process.ToViewLoc; //  - (MPEG_SEARCH_BUFSZ );

    if (iKick.Loc >= process.length[iKick.File])
    {
        iKick.Loc -= MPEG_SEARCH_BUFSZ;
    }

    if (process.ToPadLoc < 0)
        process.ToPadLoc = 0;

    if (process.ToViewPTS != PTS_NOT_FOUND)
        process.VideoPTS = process.ToViewPTS;


    iKick.Action  = ACTION_NEW_CURRLOC ;
    process.CurrFile = iKick.File ;
    process.CurrLoc  = iKick.Loc  ;

    MPEG_processKick(); //hThread_MPEG) ;
                         //iKick.Action, iKick.File, iKick.Blk) ;
    /*
    if (WaitForSingleObject(hThread_MPEG, 0) == WAIT_OBJECT_0)
    {
        iShowVideo_Flag = iCtl_ShowVideo_Flag;
        process.Action = iKick.Action ;

        hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec,
                                       0, 0, &threadId_MPEG);
    }
    */

  }

}





void Mpeg_MaybeStop_Rqst()
{
           if (process.Action > ACTION_FWD_GOP
           &&  process.Action < ACTION_FWD_GOP2)
               Mpeg_Stop_Rqst();
}






//----------------------------
 
void Set_AudioTrack(int P_Track) 
{
  const unsigned MENU_TRACK[8] =
  {IDM_TRACK_1, IDM_TRACK_2, IDM_TRACK_3, IDM_TRACK_4,
   IDM_TRACK_5, IDM_TRACK_6, IDM_TRACK_7, IDM_TRACK_8};

  iAudio_SEL_Track = P_Track;
  iWant_Aud_Format = iAudio_Trk_FMT[iAudio_SEL_Track];

  CheckMenuItem(hMenu, IDM_TRACK_AUTO, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_TRACK_NONE, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_TRACK_1, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_TRACK_2, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_TRACK_3, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_TRACK_4, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_TRACK_5, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_TRACK_6, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_TRACK_7, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_TRACK_8, MF_UNCHECKED);

  if (iAudio_SEL_Track == TRACK_AUTO)
  {
      CheckMenuItem(hMenu, IDM_TRACK_AUTO, MF_CHECKED);
      uCtl_Aud_PID = STREAM_AUTO;
  }
  else
  if (iAudio_SEL_Track == TRACK_NONE)
  {
      CheckMenuItem(hMenu, IDM_TRACK_NONE, MF_CHECKED);
      uCtl_Aud_PID = STREAM_NONE;
  }
  else
  {
     if (iAudio_SEL_Track < 0 || iAudio_SEL_Track > 7)
         iAudio_SEL_Track = 0;

     uCtl_Aud_PID = uAudio_Track_PID[iAudio_SEL_Track];
     CheckMenuItem(hMenu, MENU_TRACK[iAudio_SEL_Track], MF_CHECKED);
  }


  // Allow auto fallback
  PlayCtl.iGOP_Ctr = 0;
  PlayCtl.iAudio_SelStatus = 0;
  iVol_PREV_Cat = 0;


}




//----------------------------
void Set_Priority(HANDLE P_Process, const int P_New_Priority,
                         const int P_Target,
                         const int P_Apply_Now)
{
  const unsigned MENU_PP[3][3] =
  {IDM_PRI_RAN_HIGH,   IDM_PRI_RAN_NORMAL,   IDM_PRI_RAN_LOW,
   IDM_PP_HIGH,        IDM_PP_NORMAL,        IDM_PP_LOW,
   IDM_PRI_OUT_HIGH,   IDM_PRI_OUT_NORMAL,   IDM_PRI_OUT_LOW};

  const unsigned WIN_PRI_CLASS[3] =
  {HIGH_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, IDLE_PRIORITY_CLASS};

  int iRC, W_Pri_Ix;
 
  iRC = 1;

  if (P_New_Priority > 0 && P_New_Priority < 4)
     W_Pri_Ix = P_New_Priority - 1;
  else
     W_Pri_Ix = 1;  // Default = Normal


  if (P_Apply_Now)
     iRC = SetPriorityClass(P_Process, WIN_PRI_CLASS[W_Pri_Ix]);

  //if (iRC) 
  {
     iCtl_Priority[P_Target] = P_New_Priority;
     CheckMenuItem(hMenu, MENU_PP[P_Target][0],   MF_UNCHECKED);
     CheckMenuItem(hMenu, MENU_PP[P_Target][1],   MF_UNCHECKED);
     CheckMenuItem(hMenu, MENU_PP[P_Target][2],   MF_UNCHECKED);

     CheckMenuItem(hMenu, MENU_PP[P_Target][W_Pri_Ix],  MF_CHECKED);
  }


}


//------------------------

void Set_OutFolderMode(const unsigned P_Menu_ix)
{
  const unsigned OUT_FOLDER_MODE[4] =
  {IDM_OUT_FOLDER_SAME, IDM_OUT_FOLDER_FIRST, IDM_OUT_FOLDER_RECENT,
                        IDM_OUT_FOLDER_EVERY};


  CheckMenuItem(hMenu, IDM_OUT_FOLDER_SAME,   MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_OUT_FOLDER_FIRST,   MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_OUT_FOLDER_RECENT, MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_OUT_FOLDER_EVERY,  MF_UNCHECKED);

  CheckMenuItem(hMenu, OUT_FOLDER_MODE[P_Menu_ix], MF_CHECKED);

  iCtl_OutFolder_Active = P_Menu_ix;
}



//----------------------------

void Set_Video_Stream(const unsigned P_New_Angle,
                       const unsigned P_MenuId)
{

   CheckMenuItem(hMenu, IDM_VIDTRK_AUTO, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_ALL,  MF_UNCHECKED);

   CheckMenuItem(hMenu, IDM_VIDTRK_1, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_2, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_3, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_4, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_5, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_6, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_7, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_8, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_9, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_10, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_11, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_12, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_13, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_14, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_15, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_VIDTRK_16, MF_UNCHECKED);

   uCtl_Video_Stream       = P_New_Angle;
   CheckMenuItem(hMenu, P_MenuId, MF_CHECKED);

}



//----------------------------
/*
void Set_VPIDs(const unsigned P_New_PID,
                       const unsigned P_MenuId)
{

   CheckMenuItem(hMenu, IDM_PID_CURRENT, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_ALL,  MF_UNCHECKED);

   CheckMenuItem(hMenu, IDM_PID_1,  MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_2,  MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_3,  MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_4,  MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_5,  MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_6,  MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_7,  MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_8,  MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_9,  MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_10, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_11, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_12, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_13, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_14, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_15, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PID_16, MF_UNCHECKED);

   uVid_PID_All = 0;
   if (P_New_PID < 16)
     uCtl_Vid_PID  = uPID_Map[P_New_PID];
   else
   if (P_New_PID == STREAM_ALL)
     uVid_PID_All = 1;
   else
     uCtl_Vid_PID  = STREAM_AUTO;

   CheckMenuItem(hMenu, P_MenuId, MF_CHECKED);


   Set_Video_Stream(STREAM_AUTO,  IDM_VIDTRK_AUTO);
}
*/



void B150_PLAY_FASTER(DWORD wmId)
{
  if (MParse.SlowPlay_Flag > 0
  // || MParse.FastPlay_Flag > (iCtl_CUE_BUTTON_Speed + 2)
  ) // Return to normal ?
  {
      if (MParse.SlowPlay_Flag == 1)
          B555_Normal_Speed(1);
      else
      {
          if (MParse.SlowPlay_Flag == 2)
              MParse.SlowPlay_Flag  = 1;
          else
              MParse.SlowPlay_Flag -= 2;

          B160_PLAY_SLOW(wmId);
      }
  }
  else
  {
      Set_Priority(hThread_MPEG, PRIORITY_NORMAL,  1,  1);

      if (MParse.FastPlay_Flag < (iCtl_CUE_BUTTON_Speed + 2))
          MParse.FastPlay_Flag++;

      CheckMenuItem(hMenu, wmId, MF_CHECKED);
      B153_Fast_Msg();
      PlayCtl.iDrop_Behind   = 257;

      if (!cpu.sse2 && !cpu._3dnow)  // Slower CPU ?
          PlayCtl.iDrop_B_Frames_Flag = MParse.FastPlay_Flag;

  }

  B500_CONTINUE_PLAY_HERE();

}


void B153_Fast_Msg()
{
      sprintf(szMsgTxt,"FAST-%d", MParse.FastPlay_Flag);
      if (MParse.ShowStats_Flag)
      {
          SetDlgItemText(hStats, IDC_MPAdec_NAME, szMsgTxt);
      }
      
      DSP1_Main_MSG(0,0);
      iMsgLife=8;
}



void B160_PLAY_SLOW(DWORD wmId)
{
   CheckMenuItem(hMenu, wmId, MF_CHECKED);

   sprintf(szMsgTxt, "SLOW-%d", MParse.SlowPlay_Flag);

   if (MParse.ShowStats_Flag)
       SetDlgItemText(hStats, IDC_MPAdec_NAME, szMsgTxt);

   DSP1_Main_MSG(0,0);

   B500_CONTINUE_PLAY_HERE();

}



/*

//------------------------------------------
LRESULT CALLBACK Delay(HWND hDelayDlg, UINT message,
                       WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
      case WM_INITDIALOG:
         SetDlgItemText(hDelayDlg, IDC_DELAY, "0");
         return true;

      case WM_COMMAND:
         if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
         {
            SoundDelay[File_Limit] = GetDlgItemInt(hDelayDlg,
                                       IDC_DELAY, NULL, true);
            if (abs(SoundDelay[File_Limit]) > 10000000)
                SoundDelay[File_Limit] = 0;

            EndDialog(hDelayDlg, 0);
            return true;
         }
   }
   return false;
}

*/

//#include "PREF.CPP"



LRESULT CALLBACK About(HWND hAboutDlg, UINT message, WPARAM wParam,
                                  LPARAM lParam)
{
   switch (message)
   {
      case WM_INITDIALOG:

         // Get full path name
         SetDlgItemText(hAboutDlg, IDA_VER_STR, szAppVer);

         // Get compilation date
         strcpy( szTemp, "Build: YYYY-MMM-DD");
         memcpy( &szTemp[12], &__DATE__,    3 );
         memcpy( &szTemp[16], &__DATE__[4], 2 );
         memcpy( &szTemp[7],  &__DATE__[7], 4 );

         SetDlgItemText(hAboutDlg, IDA_BUILD, szTemp);

         GetModuleFileName(NULL,  szTMPname, sizeof(szTMPname));
         SetDlgItemText(hAboutDlg, IDA_PATH, szTMPname);

         sprintf(szTemp,"%d.%d   x%08X", 
                        winVer.dwMajorVersion, winVer.dwMinorVersion,
                        WindowsVersion);
         SetDlgItemText(hAboutDlg, IDA_WINVER, szTemp);

         

         return true;

      case WM_COMMAND:
         if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
         {
            EndDialog(hAboutDlg, 0);
            return true;
         }
   }
   return false;
}


// register the window class 
ATOM MyRegisterClass(HINSTANCE hInstance)
{

   WNDCLASSEX wcex;

   wcex.cbSize        = sizeof(WNDCLASSEX);
   wcex.style         = CS_HREDRAW
                        | CS_CLASSDC    // VISTA CRAP
                     // | CS_DBLCLKS
                        | CS_VREDRAW; 
   wcex.lpfnWndProc   = (WNDPROC)WndProc;
   wcex.cbClsExtra    = false;
   wcex.cbWndExtra    = false;
   wcex.hInstance     = hInstance; 
   wcex.hIcon         = LoadIcon(hInstance, (LPCTSTR)IDI_MOVIE);
   wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground = CreateSolidBrush(iCtl_Mask_Colour);
   wcex.lpszMenuName  = (LPCSTR)IDC_GUI;
   wcex.lpszClassName = szWindowClass;
   wcex.hIconSm       = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

   return RegisterClassEx(&wcex);
}

/*
char *ExtFilter[3] =
{
   ".avi", ".d2v", ".wav"
};
*/




// --------------------------------------------------------
void ClearMPALib(int P_Mpa_NUM)
{
  const int MPA_MENU[5]
    = { IDM_MPA_AUTO, IDM_MPAlib, IDM_Mpa_MMX, IDM_Mpa_SSE1, IDM_Mpa_SSE2};

  int iTry;
  char cAct;

  iTry = 0;

  if (!MParse.Stop_Flag)
      Mpeg_Stop_Rqst();

  if(!MPAdec.hDLL)
  {
    FreeLibrary(MPAdec.hDLL); 
    MPAdec.hDLL = 0;  byMPALib_OK = 0;
  }

  iMPALib_Status = -1 ; // Set decoder status to UNKNOWN
  if (MParse.ShowStats_Flag)
  {
     SetDlgItemText(hStats, IDC_MPAdec_NAME, " ");
     SetDlgItemText(hStats, STATS_MPAdec_STATUS, " ");
  }

  
  if (P_Mpa_NUM < 0)
      cAct = 'T';
  else
  if (P_Mpa_NUM)
  {
     iTry = P_Mpa_NUM;
     cAct = 'C';
  }
  else
     cAct = 'S';

  ToggleMenu(cAct, &iMpa_AUTO, IDM_MPA_AUTO);

  if (iMpa_AUTO)
  {
     iTry = iMPA_Best;
     iCtl_AudioDecoder = 0;
  }
  else
  {
      iTry = iCtl_AudioDecoder = P_Mpa_NUM;
  }


  CheckMenuItem(hMenu, IDM_MPAlib,     MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_Mpa_MMX,    MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_Mpa_SSE1,   MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_Mpa_SSE2,   MF_UNCHECKED);

  if (iTry >= 0)
  {
      CheckMenuItem(hMenu,  MPA_MENU[iTry], MF_CHECKED);
      strcpy(szMPAdec_NAME, MPA_NAME[iTry]) ;
  }

}



// --------------------------------------------------------
void Out_SetBufSz(int P_Size_Ix)  // Sets iMpeg_Copy_BufSz
{ 
  const int MENU_ID[4]
    = { IDM_BUF_VERYLARGE, IDM_BUF_LARGE, IDM_BUF_MEDIUM, IDM_BUF_SMALL};

  const int BUFSZ[4] 
    = { K_8MB, K_4MB, K_1MB, K_256KB};

   CheckMenuItem(hMenu, IDM_BUF_VERYLARGE, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_BUF_LARGE,     MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_BUF_MEDIUM,    MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_BUF_SMALL,     MF_UNCHECKED);

   CheckMenuItem(hMenu, MENU_ID[P_Size_Ix], MF_CHECKED);

   iCtl_Copy_BufSz_Ix = P_Size_Ix;
   iMpeg_Copy_BufSz = BUFSZ[iCtl_Copy_BufSz_Ix];
}
 


// --------------------------------------------------------
void IDCT_SetHardware(int P_Value, unsigned P_Menu_Code)
{
  MParse.iDCT_Flag =  P_Value;

  CheckMenuItem(hMenu, IDM_IDCT_MMX,    MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_IDCT_SSEMMX, MF_UNCHECKED);  // SSE = Pentium 3
  CheckMenuItem(hMenu, IDM_IDCT_FPU,    MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_IDCT_REF,    MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_IDCT_SSE2,   MF_UNCHECKED);  // SSE2 = Pentium 4

  CheckMenuItem(hMenu, P_Menu_Code,     MF_CHECKED);

  SwitchIDCT();

}



//-----------------------------
// Set, Clear or Toggle a menu variable
//  'S' = Set to 1
//  'C' = Clr to 0
//  'T' = Toggles between 0 and 1
//  '=' = Sets to match variable
//---------------------------------------------------------
void ToggleMenu(char P_Act, void *P_Value, int P_NAME)
{

  UINT uMark;

   if (P_Act == 'C'
   || (P_Act == 'T' && *(char*)(P_Value) ))
   {
       *(char*)(P_Value) = 0 ;
   }

   else
   if (P_Act != '=')
   {
       *(char*)(P_Value) = 1 ;
   }


   if (*(char*)(P_Value))
       uMark = MF_CHECKED;
   else
       uMark = MF_UNCHECKED;

   CheckMenuItem(hMenu, P_NAME, uMark);

}



void Set_Folder(char *lsP_Folder, int *lpP_Active, int iP_MenuItem,
               const int iP_AlwaysReset, 
               const int iP_SAVE_TYPE, const char *lpszFileType)
{
  int iRC, iTmpLen;
  char *ls_FolderSlash;
  char *szSlashPTR;
  char szTemp2[_MAX_PATH];

  if (iP_MenuItem)
      ToggleMenu('S', lpP_Active, iP_MenuItem);


  if ( iP_AlwaysReset  ||  !*lsP_Folder)
  {
     iTmpLen = (int)(lpLastSlash(&szOutput[0]) - &szOutput[0]);
     if (iTmpLen > 0)
     {
         memcpy(lsP_Folder, &szOutput, iTmpLen);
         ls_FolderSlash = lsP_Folder+iTmpLen;
      //*ls_FolderSlash++ = '\\';
        *ls_FolderSlash = 0;
     }
     else
        *lsP_Folder = 0;
  }


  strcpy(szTemp2, lsP_Folder);
  strcat(szTemp2, lpszFileType);

  iRC = X800_PopFileDlg(szTemp2, hWnd_MAIN, iP_SAVE_TYPE, -1, &"Choose Folder");

  szSlashPTR = lpLastSlash(szTemp2);
  if (!szSlashPTR)
       szSlashPTR = &szTemp2[0];
 *szSlashPTR = 0x00;
  strcpy(lsP_Folder, szTemp2);

  return ;
}


void Set_Bmp_AutoPreview(const int P_NewSetting)
{

   const int MenuFld[3] = {IDM_BMP_PREVIEW_OFF,
                           IDM_BMP_PREVIEW_1,
                           IDM_BMP_PREVIEW_ALL};


   iCtl_BMP_Preview = P_NewSetting ;
   iBMP_Preview     = iCtl_BMP_Preview;

   CheckMenuItem(hMenu, MenuFld[0],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[1],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[2],      MF_UNCHECKED);

   CheckMenuItem(hMenu, MenuFld[P_NewSetting], MF_CHECKED);

}



void Set_Bmp_Fmt(const int P_NewSetting)
{

   const int MenuFld[3] = {IDM_BMP_FMT_BICUBIC,
                           IDM_BMP_FMT_SUBSAMPLE,
                           IDM_BMP_FMT_RAW
   };


   iCtl_BMP_Aspect = P_NewSetting ;

   CheckMenuItem(hMenu, MenuFld[0],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[1],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[2],      MF_UNCHECKED);

   CheckMenuItem(hMenu, MenuFld[P_NewSetting], MF_CHECKED);

}





void Set_Time_Fmt(const int P_NewSetting)
{

   const int MenuFld[8] = {0, IDM_TIME_REL,  IDM_TIME_GOP, IDM_TIME_PTS, 
                              IDM_TIME_FILE, IDM_TIME_SCR, 
                              IDM_TIME_HEX,  IDM_TIME_TOD_REL
   };


   iView_TC_Format = P_NewSetting ;
   iCtl_Time_Fmt   = P_NewSetting ;

   //CheckMenuItem(hMenu, MenuFld[0],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[1],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[2],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[3],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[4],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[5],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[6],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[7],      MF_UNCHECKED);

   CheckMenuItem(hMenu, MenuFld[P_NewSetting], MF_CHECKED);

   DSP3_Main_TIME_INFO();

}



//---------------------------------------------------------
void  Set_Aspect_Mode(int P_NewSetting)
{
   const int MenuFld[] = {IDM_ASPECT_OFF,  IDM_ASPECT_TV,
                            IDM_ASPECT_WIDE, IDM_ASPECT_70mm,
                            IDM_ASPECT_STD,  0, 0, 0, 0,
                            IDM_ASPECT_NARROW,  // Cropped 4:3
             IDM_ASPECT_119,  IDM_ASPECT_132,  IDM_ASPECT_150,
             IDM_ASPECT_167,  IDM_ASPECT_185,  IDM_ASPECT_235,
             IDM_ASPECT_239,  IDM_ASPECT_255,  IDM_ASPECT_259,
             IDM_ASPECT_276,  IDM_ASPECT_400
   };
   int iTmp1;

  iView_Aspect_Mode = P_NewSetting;

  for (iTmp1=0; iTmp1<21; iTmp1++)
  {
      CheckMenuItem(hMenu, MenuFld[iTmp1], MF_UNCHECKED);
  }

  CheckMenuItem(hMenu, MenuFld[P_NewSetting], MF_CHECKED);

 if (MParse.SeqHdr_Found_Flag)
 {
   /*
   if (! iView_Aspect_Mode  &&  ! Deint_VIEW)
   {
         iVertInc = 0;
         iAspVert = 2048; iAspHoriz = 2048;
   } 
   else
   */
     Mpeg_Aspect_Resize();

   View_Rebuild_Chk(0);
 }

}


void Set_Aspect_MPG1(int P_NewSetting)
{

   const int MenuFld[] = {IDM_ASP_MPEG1_STD, 
                          IDM_ASP_MPEG1_GUESS, 
                          IDM_ASP_MPEG1_FORCE, 0  
   };
   int iTmp1;

  iCtl_View_Aspect_Mpeg1_Force = P_NewSetting;

  for (iTmp1=0; MenuFld[iTmp1]; iTmp1++)
  {
      CheckMenuItem(hMenu, MenuFld[iTmp1], MF_UNCHECKED);
  }

  CheckMenuItem(hMenu, MenuFld[P_NewSetting], MF_CHECKED);

 if (MParse.SeqHdr_Found_Flag)
 {
     Mpeg_Aspect_Resize();
     View_Rebuild_Chk(0);
 }

}


//---------------------------------------------------------
void  Set_Frame_Rate(int P_NewSetting)
{
   const int MenuFld[16] = {IDM_FRAME_RATE_STD, IDM_FRAME_RATE_23,
                           IDM_FRAME_RATE_24,  IDM_FRAME_RATE_25,
                           IDM_FRAME_RATE_29,  IDM_FRAME_RATE_30,
                           0,  0,  0,
                           IDM_FRAME_RATE_12,  IDM_FRAME_RATE_16,
                           IDM_FRAME_RATE_18,  IDM_FRAME_RATE_20,
                           IDM_FRAME_RATE_06,  IDM_FRAME_RATE_02,
                           IDM_FRAME_RATE_01
   };

   iOverride_FrameRate_Code = P_NewSetting;
   if (iOverride_FrameRate_Code)
       iView_FrameRate_Code = iOverride_FrameRate_Code;
   else
       iView_FrameRate_Code = MPEG_Seq_frame_rate_code;
   FrameRate2FramePeriod();

   if (! P_NewSetting)
   {
     if ((MPEG_Seq_frame_rate_code == 5 && iView_FrameRate_Code == 1)
     ||  (MPEG_Seq_frame_rate_code == 3 && iView_FrameRate_Code == 11))
     {
          MParse.SlowPlay_Flag = 1;
     }
     else
     {
         MParse.SlowPlay_Flag = 0;
         iAudio_Lock = 0;
     }
   }

   CheckMenuItem(hMenu, MenuFld[0],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[1],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[2],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[3],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[4],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[5],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[6],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[7],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[8],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[9],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[10],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[11],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[12],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[13],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[14],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[15],      MF_UNCHECKED);

   CheckMenuItem(hMenu, MenuFld[P_NewSetting], MF_CHECKED);
}




//---------------------------------------------------------
void  Set_DropDefault(int P_NewSetting)
{
   const int MenuFld[3] = {IDM_DROP_ASK, IDM_DROP_APPEND,
                           IDM_DROP_OPEN,

   };


   iCtl_DropAction = P_NewSetting;

   CheckMenuItem(hMenu, MenuFld[0],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[1],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[2],      MF_UNCHECKED);

   CheckMenuItem(hMenu, MenuFld[P_NewSetting], MF_CHECKED);
}



//---------------------------------------------------------
void  Set_SortDefault(int P_NewSetting)
{
   const int MenuFld[] = {IDM_DEFSORT_OFF,
                          IDM_DEFSORT_NAME, IDM_DEFSORT_SCR,
                          IDM_DEFSORT_DATE};
   
   int iMenu_ix;


   iCtl_FileSortSeq = P_NewSetting;
   iMenu_ix = iCtl_FileSortSeq + 1;

   CheckMenuItem(hMenu, MenuFld[0],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[1],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[2],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[3],      MF_UNCHECKED);

   CheckMenuItem(hMenu, MenuFld[iMenu_ix], MF_CHECKED);
}



//---------------------------------------------------------
void  Set_Zoom_Menu(int P_NewSetting)
{
   const unsigned int MenuFld[] = {IDM_ZOOM_AUTO, 
                                   IDM_ZOOM_OFF,  IDM_ZOOM_COMPACT,
                                   IDM_ZOOM_HALF, IDM_ZOOM_THIRD,

   };
 
  //if (P_NewSetting < 1)
  //     iZoom = 1;
  //else
       iZoom = P_NewSetting;

  CheckMenuItem(hMenu, MenuFld[1],      MF_UNCHECKED);
  CheckMenuItem(hMenu, MenuFld[2],      MF_UNCHECKED);
  CheckMenuItem(hMenu, MenuFld[3],      MF_UNCHECKED);
  CheckMenuItem(hMenu, MenuFld[4],      MF_UNCHECKED);

  CheckMenuItem(hMenu, MenuFld[P_NewSetting + 1], MF_CHECKED);

}
                             


//---------------------------------------------------------
void  Set_Zoom(int P_NewSetting)
{
  if (DBGflag)
      DBGout("ZOOM");

  iCtl_Zoom_Wanted = P_NewSetting;

  CheckMenuItem(hMenu, IDM_ZOOM_AUTO,   MF_UNCHECKED);
  Set_Zoom_Menu(P_NewSetting);
  Prev_Clip_Height = 0;
  View_MOUSE_ALIGN(-1);
}
 

void  Set_OVL_Notify(int P_NewSetting)  // VISTA CRAP
{
  const int MenuFld[4] = {IDM_OVL_NOTIFY_DEF,  IDM_OVL_NOTIFY_FRAMECHG,
                          IDM_OVL_NOTIFY_UPDWINDOW,
                          IDM_OVL_SIGNAL_GUI};

   if (P_NewSetting <= 0)  // Init Check
       P_NewSetting = -P_NewSetting;
   else
       Chg2YUV2(1, 0); 

   iCtl_VistaOVL_mod = P_NewSetting;


   CheckMenuItem(hMenu, MenuFld[0],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[1],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[2],      MF_UNCHECKED);
   CheckMenuItem(hMenu, MenuFld[3],      MF_UNCHECKED);

   CheckMenuItem(hMenu, MenuFld[P_NewSetting], MF_CHECKED);

}


void Set_Wheel_Scroll(int P_Amt)
{
  
   int MenuFld[] = {IDM_WHEEL_NONE,   IDM_WHEEL_FRAME,
                     IDM_WHEEL_JUMP4, IDM_WHEEL_JUMP2, IDM_WHEEL_JUMP, 
                     IDM_WHEEL_GOP
                    };
   int MenuIx;

   iCtl_Wheel_Scroll = P_Amt;

   CheckMenuItem(hMenu, IDM_WHEEL_NONE,      MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_WHEEL_FRAME,     MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_WHEEL_GOP,       MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_WHEEL_JUMP,      MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_WHEEL_JUMP2,     MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_WHEEL_JUMP4,     MF_UNCHECKED);


   if (iCtl_Wheel_Scroll < 4)
        MenuIx = iCtl_Wheel_Scroll + 1;
   else MenuIx = 5;

   CheckMenuItem(hMenu, MenuFld[MenuIx], MF_CHECKED);

}



void Set_Menu_Array(int P_New_ix, int *P_Menu_Fld)
{
  
   int j, iMenu_New_ix, *lp_iMenu_Fld;

   iMenu_New_ix  = P_New_ix;
   lp_iMenu_Fld  = P_Menu_Fld;

   for (j=0; (*lp_iMenu_Fld) != 0; lp_iMenu_Fld++)
   {
     if (j==iMenu_New_ix)
        CheckMenuItem(hMenu, *lp_iMenu_Fld,  MF_CHECKED);
     else
        CheckMenuItem(hMenu, *lp_iMenu_Fld,  MF_UNCHECKED);
     j++;
   }

}


//---------------------------------------------------------
void  Set_Preamble_Mode(int P_Mode)
{
   int MenuFld[4] = {IDM_PREAMBLE_NONE, IDM_PREAMBLE_SMALL,
                     IDM_PREAMBLE_MAX //, IDM_PREAMBLE_POWERVCR
                    };
   int MenuIx;

   iCtl_Out_Preamble_Flag = P_Mode ;

   CheckMenuItem(hMenu, IDM_PREAMBLE_NONE,      MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PREAMBLE_SMALL,     MF_UNCHECKED);
   //CheckMenuItem(hMenu, IDM_PREAMBLE_POWERVCR,  MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PREAMBLE_MAX,       MF_UNCHECKED);

   if (iCtl_Out_Preamble_Flag < 3)
        MenuIx = iCtl_Out_Preamble_Flag;
   else MenuIx = 2;

   CheckMenuItem(hMenu, MenuFld[MenuIx], MF_CHECKED);

}



// --------------------------------------------------------
// Default File Extension for Program Streams
void Set_XTN_PS(char P_Xtn[8])
{
  CheckMenuItem(hMenu, IDM_XTN_VOB,        MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_MPG_UPPER,  MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_MPG_LOWER,  MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_M2P,        MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_VID_MP2,    MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_SAME,       MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_PS_OTHER,   MF_UNCHECKED);

  strcpy(szOut_Xtn_RULE, P_Xtn);

   if (!stricmp(P_Xtn, "vob"))
       CheckMenuItem(hMenu, IDM_XTN_VOB,  MF_CHECKED);
   else
   if (!stricmp(P_Xtn, "m2p"))
       CheckMenuItem(hMenu, IDM_XTN_M2P,  MF_CHECKED);
   else
   if (!stricmp(P_Xtn, "mp2"))
       CheckMenuItem(hMenu, IDM_XTN_VID_MP2,  MF_CHECKED);
   else
   if (!strcmp(P_Xtn, "MPG"))
       CheckMenuItem(hMenu, IDM_XTN_MPG_UPPER,  MF_CHECKED);
   else
   if (!stricmp(P_Xtn, "mpg"))
       CheckMenuItem(hMenu, IDM_XTN_MPG_LOWER, MF_CHECKED);
   else
   if (!stricmp(P_Xtn, "$"))
       CheckMenuItem(hMenu, IDM_XTN_SAME, MF_CHECKED);
   else
       CheckMenuItem(hMenu, IDM_XTN_PS_OTHER, MF_CHECKED);

}





// --------------------------------------------------------
// Default File Extension for Program Streams
void Set_XTN_AUD(char P_Xtn[8])
{
  CheckMenuItem(hMenu, IDM_XTN_MPA,        MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_M2A,        MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_M1A,        MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_MP3,        MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_MP2,        MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_MP1,        MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_XTN_AUD_OTHER,  MF_UNCHECKED);

  strcpy(szOut_Xtn_AUD, P_Xtn);

 if (!stricmp(P_Xtn, "MPA"))
     CheckMenuItem(hMenu, IDM_XTN_MPA,  MF_CHECKED);
 else
 if (!stricmp(P_Xtn, "M2A"))
     CheckMenuItem(hMenu, IDM_XTN_M2A,  MF_CHECKED);
 else
 if (!stricmp(P_Xtn, "M1A"))
     CheckMenuItem(hMenu, IDM_XTN_M1A,  MF_CHECKED);
 else
 if (!stricmp(P_Xtn, "MP3"))
     CheckMenuItem(hMenu, IDM_XTN_MP3,  MF_CHECKED);
 else
 if (!stricmp(P_Xtn, "MP2"))
     CheckMenuItem(hMenu, IDM_XTN_MP2,  MF_CHECKED);
 else
 if (!stricmp(P_Xtn, "MP1"))
     CheckMenuItem(hMenu, IDM_XTN_MP1,  MF_CHECKED);
 else
     CheckMenuItem(hMenu, IDM_XTN_AUD_OTHER,  MF_CHECKED);

}


// --------------------------------------------------------
void Set_ADD(int P_Mode)
{
  unsigned int uItem;

  CheckMenuItem(hMenu, IDM_ADD_OFF,     MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_ADD_REMIND,  MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_ADD_AUTO,    MF_UNCHECKED);

   switch (P_Mode)
   {
      case 0:
         uItem = IDM_ADD_OFF;
         break;
      case 1:
         uItem = IDM_ADD_REMIND;
         break;
      case 2:
         uItem = IDM_ADD_AUTO;
         break;
      default:
         uItem = 0;
   }
   CheckMenuItem(hMenu, uItem,    MF_CHECKED);

   Add_Automation = P_Mode;
}

/*
// --------------------------------------------------------
void Set_ALIGN_VERT(int P_Mode, int P_Refresh)
{
  CheckMenuItem(hMenu, IDM_ALIGN_AUTO,    MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_ALIGN_TOP,     MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_ALIGN_MID,     MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_ALIGN_BOT,     MF_UNCHECKED);

   switch (P_Mode)
   {
      case 0:
         CheckMenuItem(hMenu, IDM_ALIGN_TOP,    MF_CHECKED);
         break;
      case 1:
         CheckMenuItem(hMenu, IDM_ALIGN_AUTO,   MF_CHECKED);
         break;
      case 2:
         CheckMenuItem(hMenu, IDM_ALIGN_MID,    MF_CHECKED);
         break;
      case 3:
         CheckMenuItem(hMenu, IDM_ALIGN_BOT,    MF_CHECKED);
         break;
   }

   iView_Align_VERT = P_Mode;

   if (P_Refresh)
   {
      if(DDOverlay_Flag && MParse.iColorMode==STORE_YUY2) // && iShowVideo_Flag)
         RenderYUY2(1);
      else
         RenderRGB24();
    }


}

// --------------------------------------------------------
void Set_ALIGN_HORIZ(int P_Mode, int P_Refresh)
{

  CheckMenuItem(hMenu, IDM_ALIGN_LEFT,    MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_ALIGN_CTR,     MF_UNCHECKED);
  CheckMenuItem(hMenu, IDM_ALIGN_RIGHT,   MF_UNCHECKED);

   switch (P_Mode)
   {
      case 0:
         CheckMenuItem(hMenu, IDM_ALIGN_LEFT,  MF_CHECKED);
         break;
      case 2:
         CheckMenuItem(hMenu, IDM_ALIGN_CTR,    MF_CHECKED);
         break;
      case 3:
         CheckMenuItem(hMenu, IDM_ALIGN_RIGHT,  MF_CHECKED);
         break;
   }

   iView_Align_HORIZ = P_Mode;

   if (P_Refresh && DDOverlay_Flag) // && iShowVideo_Flag)
      RenderYUY2(1);

}

*/




// --------------------------------------------------------
static void B910_Main_INIT(HWND hWnd, /* UINT message, */ WPARAM wParam,
                                                          LPARAM lParam)
{
  int iTxt;
  DragAcceptFiles(hWnd, true);

  AC3_PreScale_Ratio = 1.0;

  hDC       = GetDC(hWnd);
  //hBrush_MASK    = CreateSolidBrush(iCtl_Mask_Colour);
  hMenu     = GetMenu(hWnd);
  hMain_GUI  = GetCurrentProcess();


  if (iCtl_Mask_Colour == iColor_Menu_BG)  // Overlay key Mid Grey ?
  {
      iTxt          = 0x010101;
      iColor_Msg_BG = iCtl_Mask_Colour;
  }
  else
  if (iCtl_Mask_Colour == 0)              // Overlay key Black ?
  {
      iTxt          = 0xFFEEEE;
      iColor_Msg_BG = iCtl_Mask_Colour;
  }
  else
  {
      iTxt          = iCtl_Text_Colour;
      iColor_Msg_BG = iCtl_Back_Colour;
  }

  SetTextColor(hDC, iTxt); 
  SetBkColor(  hDC, iColor_Msg_BG);  // Background = Overlay key


  // register VFAPI 
  /*
  HKEY key; DWORD trash;

  if (RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\VFPlugin", 0, "",
                             REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                                                NULL, &key, &trash)
                  == ERROR_SUCCESS)
  {

         if (_findfirst("DVD2AVI.vfp", &vfpfile) != -1L)
         {
                  strcat(szBuffer, "DVD2AVI.vfp");

                  RegSetValueEx(key, "DVD2AVI", 0, REG_SZ, (LPBYTE)szBuffer, strlen(szBuffer));
                  CheckMenuItem(hMenu, IDM_VFAPI, MF_CHECKED);
         }

      }
      RegCloseKey(key);
  }

  */


  /*
  // load DLL
  DVD_PlugIn_Flag = true;
  if ((hLibrary = LoadLibrary("OpenDVD.dll")) == NULL)
            DVD_PlugIn_Flag = false;
  else
  {
           if ((KeyOp = (pfnKeyOp) GetProcAddress(hLibrary, "KeyOp")) == NULL)
               DVD_PlugIn_Flag = false;

            if ((BufferOp = (pfnBufferOp) GetProcAddress(hLibrary, "BufferOp")) == NULL)
               DVD_PlugIn_Flag = false;
   }
  */

  //if (!DVD_PlugIn_Flag)
  //   DeleteMenu(GetSubMenu(hMenu,4), 3, MF_BYPOSITION);


// RJ used to initialize much stuff here

}




//-----------------------------------------------------
static void CheckFlag()
{
/*   CheckMenuItem(hMenu, IDM_KEY_OFF, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_KEY_INPUT, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_KEY_OP, MF_UNCHECKED);
*/
   //if (!DVD_PlugIn_Flag)
   //   KeyOp_Flag = 0;

/*   switch (KeyOp_Flag)
   {
      case KEY_OFF:
         CheckMenuItem(hMenu, IDM_KEY_OFF, MF_CHECKED);
         break;

      case KEY_INPUT:
         CheckMenuItem(hMenu, IDM_KEY_INPUT, MF_CHECKED);
         break;

      case KEY_OP:
         CheckMenuItem(hMenu, IDM_KEY_OP, MF_CHECKED);
         break;
   }
*/
   CheckMenuItem(hMenu, IDM_IDCT_MMX,    MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_IDCT_SSEMMX, MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_IDCT_SSE2,   MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_IDCT_FPU,    MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_IDCT_REF,    MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_PCSCALE,     MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_TVSCALE,     MF_UNCHECKED);
   CheckMenuItem(hMenu, IDM_FO_FILM,     MF_UNCHECKED);

   if (cpu.ssemmx && MParse.iDCT_Flag==IDCT_MMX)
      MParse.iDCT_Flag = IDCT_SSEMMX;

   if (!cpu.ssemmx && MParse.iDCT_Flag==IDCT_SSEMMX)
      MParse.iDCT_Flag = IDCT_MMX;

   // MAY NOT NEED THIS NEXT BIT ANYMORE
   switch (MParse.iDCT_Flag)
   {
      case IDCT_MMX:
         CheckMenuItem(hMenu, IDM_IDCT_MMX,  MF_CHECKED);
         break;

      case IDCT_SSEMMX:
         CheckMenuItem(hMenu, IDM_IDCT_SSEMMX, MF_CHECKED);
         break;

      case IDCT_SSE2:
         CheckMenuItem(hMenu, IDM_IDCT_SSE2, MF_CHECKED);
         break;

      case IDCT_FPU:
         CheckMenuItem(hMenu, IDM_IDCT_FPU, MF_CHECKED);
         break;

      case IDCT_REF:
         CheckMenuItem(hMenu, IDM_IDCT_REF, MF_CHECKED);
         break;
   }
   SwitchIDCT();


   if (MParse.PC_Range_Flag)
   {
      RGB_Scale  = 0x1000254310002543;
      RGB_Offset = 0x0010001000100010;
      RGB_CBU = 0x0000408D0000408D;
      RGB_CGX = 0xF377E5FCF377E5FC;
      RGB_CRV = 0x0000331300003313;

      CheckMenuItem(hMenu, IDM_PCSCALE, MF_CHECKED);
   }
   else
   {
      RGB_Scale  = 0x1000200010002000;
      RGB_Offset = 0x0000000000000000;
      RGB_CBU = 0x000038B4000038B4;
      RGB_CGX = 0xF4FDE926F4FDE926;
      RGB_CRV = 0x00002CDD00002CDD;

      CheckMenuItem(hMenu, IDM_TVSCALE, MF_CHECKED);
   }



   switch (MParse.FO_Flag)
   {
      case FO_NONE:
         CheckMenuItem(hMenu, IDM_FO_NONE, MF_CHECKED);
         CheckMenuItem(hMenu, IDM_FO_FILM, MF_UNCHECKED);
         CheckMenuItem(hMenu, IDM_FO_SWAP, MF_UNCHECKED);
         break;

      case FO_FILM:
         CheckMenuItem(hMenu, IDM_FO_NONE, MF_UNCHECKED);
         CheckMenuItem(hMenu, IDM_FO_FILM, MF_CHECKED);
         CheckMenuItem(hMenu, IDM_FO_SWAP, MF_UNCHECKED);
         break;

      case FO_SWAP:
         CheckMenuItem(hMenu, IDM_FO_NONE, MF_UNCHECKED);
         CheckMenuItem(hMenu, IDM_FO_FILM, MF_UNCHECKED);
         CheckMenuItem(hMenu, IDM_FO_SWAP, MF_CHECKED);
         break;
   }


   Set_AudioTrack(iAudio_SEL_Track); //RJTRK


   switch (iWant_Aud_Format)
   {
      case FORMAT_AC3:
         CheckMenuItem(hMenu, IDM_AC3, MF_CHECKED);
         break;

      case FORMAT_MPA:
         CheckMenuItem(hMenu, IDM_MPA, MF_CHECKED);
         break;

/*      case FORMAT_LPCM:
         CheckMenuItem(hMenu, IDM_LPCM, MF_CHECKED);
         break;
*/
      case FORMAT_AUTO:
         CheckMenuItem(hMenu, IDM_SELECT, MF_CHECKED);
         break;
   }

/*   switch (AC3_Flag)
   {
      case AUDIO_DEMUXALL:
         CheckMenuItem(hMenu, IDM_AC3_DEMUXALL, MF_CHECKED);
         break;

      case AUDIO_DEMUXONE:
         CheckMenuItem(hMenu, IDM_AC3_DEMUXONE, MF_CHECKED);
         break;

      case AUDIO_DECODE:
         CheckMenuItem(hMenu, IDM_AC3_DECODE, MF_CHECKED);
         break;
   }
*/

   Set_Menu_Array(AC3_DRC_FLag, &DRC_MenuId[0]);
   /*
   switch (AC3_DRC_FLag)
   {
      case DRC_NONE:
         CheckMenuItem(hMenu, IDM_DRC_NONE, MF_CHECKED);
         break;

      case DRC_LIGHT:
         CheckMenuItem(hMenu, IDM_DRC_LIGHT, MF_CHECKED);
         break;

      case DRC_NORMAL:
         CheckMenuItem(hMenu, IDM_DRC_NORMAL, MF_CHECKED);
         break;

      case DRC_HEAVY:
         CheckMenuItem(hMenu, IDM_DRC_HEAVY, MF_CHECKED);
         break;
   }
   */

   if (AC3_DSDown_Flag)
      CheckMenuItem(hMenu, IDM_DSDOWN, MF_CHECKED);

/*   switch (MPA_Flag)
   {
      case AUDIO_DEMUXALL:
         CheckMenuItem(hMenu, IDM_MPA_DEMUXALL, MF_CHECKED);
         break;

      case AUDIO_DEMUXONE:
         CheckMenuItem(hMenu, IDM_MPA_DEMUXONE, MF_CHECKED);
         break;
   }

   switch (SRC_Flag)
   {
      case SRC_NONE:
         CheckMenuItem(hMenu, IDM_SRC_NONE, MF_CHECKED);
         break;

      case SRC_LOW:
         CheckMenuItem(hMenu, IDM_SRC_LOW, MF_CHECKED);
         break;

      case SRC_MID:
         CheckMenuItem(hMenu, IDM_SRC_MID, MF_CHECKED);
         break;

      case SRC_HIGH:
         CheckMenuItem(hMenu, IDM_SRC_HIGH, MF_CHECKED);
         break;

      case SRC_UHIGH:
         CheckMenuItem(hMenu, IDM_SRC_UHIGH, MF_CHECKED);
         break;
   }

   if (Norm_Ratio > 100)
   {
      CheckMenuItem(hMenu, IDM_NORM, MF_CHECKED);
      Normalization_Flag = true;
      Norm_Ratio -= 100;
   }
*/

 //  Set_Priority(iCtl_Priority[0], 0, 1);

}



// -------------------------------------------------------------
// Release various buffers and clear selection and position controls
void ProcessReset(char P_Reason[3])
{
  strcpy(RecoveryReason, P_Reason);

   if (MParse.SeqHdr_Found_Flag)
   {
      D300_FREE_Overlay();
      //PicBuffer_Free();
   }

   MParse.SeqHdr_Found_Flag = false;

   if (iViewToolBar & 1)
   {
     SendMessage(hTrack, TBM_SETPOS, (WPARAM) false, 0);
     SendMessage(hTrack, TBM_SETSEL, (WPARAM) false, (LPARAM) MAKELONG(0, 0));
     SendMessage(hTrack, TBM_CLEARTICS, (WPARAM) true, 0);
   }

   Menu_Main_Disable(false, true);

   //EnableMenuItem(hMenu, IDM_SAVE, MF_GRAYED);
//   EnableMenuItem(hMenu, IDM_SAVE_D2V, MF_GRAYED);
//   EnableMenuItem(hMenu, IDM_BMP_ASIS,  MF_GRAYED);
//   EnableMenuItem(hMenu, IDM_PROCESS_WAV, MF_ENABLED);

   Clip_Left = Clip_Right = Clip_Top = Clip_Bottom
        = Squeeze_Width = Squeeze_Height = 0;

   ClipResize_Flag = false;
   //iCtl_Zoom_Wanted = -1;

   MPEG_Seq_progressive_sequence = 1; // Allow for old fashioned encoders that assume progressive

//   CheckMenuItem(hMenu, IDM_CLIPRESIZE, MF_UNCHECKED);

   AC3_PreScale_Ratio = 1.0;
//   CheckMenuItem(hMenu, IDM_PRESCALE, MF_UNCHECKED);

   strcpy(szBuffer, szAppName) ; 
   strcat(szBuffer, "- Reset ") ;
   strcat(szBuffer, P_Reason) ;   
   SetWindowText(hWnd_MAIN, szBuffer);

   if (File_Limit)  
   {
//      EnableMenuItem(hMenu, IDM_PROCESS_WAV, MF_GRAYED);

      ZeroMemory(&process, sizeof(PROCESS));
      process.iOut_DropCrud = iCtl_Out_DropCrud;

      BwdGop.ix   = 0;  BwdGop.iOrg   = 0;
      BwdFast1.ix = 0;  BwdFast1.iOrg = 0;
      process.trackright = TRACK_PITCH;
      process.iView_TopMask = -1;

      //MParse.SystemStream_Flag = false;
      iShowVideo_Flag = iCtl_ShowVideo_Flag;
      MultiFile_SizeCalc();
   }
   else
      DragAcceptFiles(hWnd_MAIN, true);

//RJTMP    D500_ResizeMainWindow(TOOL_BAR_WIDTH, INIT_HEIGHT, 1);

}

// END Recovery func






//-------------------------------------------
void Msg_LastError(char P_Module[32], int P_RC, char P_ACTION)
{
  int iRC;
  DWORD SysErrNum;

  sprintf(szMsgTxt, "%s rc=x%02X\n\n", P_Module, P_RC);

  SysErrNum = GetLastError();
  iRC = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | 80, NULL,
                      SysErrNum, 0, szTemp, sizeof(szTemp), NULL);
  strcat(szMsgTxt, szTemp);

  if (DBGflag)
  {
      DBGout(szMsgTxt);
  }

  if (P_ACTION == 'b')
  {
      MessageBox ( NULL, szMsgTxt, Mpg2Cut2_ERROR,
                MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
  }
}



//---------------------------------------------------
void ERRMSG_File(char P_Desc[32], char P_Action,
                 int P_rc1,       char P_DSN[_MAX_PATH],
                 int P_rc2, int P_Caller)
{
   /*
   __int64 W_rc1 ;
   W_rc1 = (__int64) (P_rc1) ;
   ERRMSG_FileXA(P_Desc, P_Action, W_rc1, P_DSN, P_Caller) ;

}


//---------------------------------------------------
void ERRMSG_FileXA(char P_Desc, char P_Action,
                      __int64   P_rc1,    char P_DSN[_MAX_PATH])
{
*/
  char szAction[16], szTmp1[_MAX_PATH*3], szFriendly[_MAX_PATH] ;
  DWORD SysErrNum;
  int iRC;

  szFriendly[0] = 0;

   switch (P_Action)
   {
      case 'o':
      case 'O':
         strcpy(szAction, FILE_OPEN);
         break;

      case 'c':
      case 'C':
         strcpy(szAction, FILE_CLOSE);
         break;

      case 'r':
      case 'R':
         strcpy(szAction, FILE_READ);
         break;

      case 'w':
      case 'W':
         strcpy(szAction, FILE_WRITE);
         break;

      case 's':
      case 'S':
         strcpy(szAction, FILE_SEEK);
         break;

      case 'u':
      case 'U':
         strcpy(szAction, FILE_UPDATE);
         break;

      default:
         sprintf(szAction, "MSG=%c", P_Action) ;
         break;

   }

  SysErrNum = GetLastError();
  iRC = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | 80, NULL,
                      SysErrNum, 0, szTemp, sizeof(szTemp), NULL);

  if ( strnicmp(&szTemp[0], &IS_NORMAL, 6) )
  {
     if (P_rc1 == 13)
     {
        strcpy(szTemp, FILE_LOCKED);
     }
     else
     if (P_rc1 == 1)
     {
        strcpy(szTemp, FILE_BAD_SECTOR);
     }
     else
     if (P_rc1 == ENOENT || P_rc1 == 2) 
     {
        strcpy(szTemp, FILE_PATH_NOT_FOUND);
     }
     else
     if (P_rc1 == EACCES) 
     {
        strcpy(szTemp, FILE_PERMISSION_DENIED);
     }
     else
     if (P_rc1 == EBADF) 
     {
        strcpy(szFriendly, FILE_BAD_NUMBER);
     }
     else
       szTemp[0] = 0;
  }

  //                                               =x%LX - x%LX
  sprintf(szTmp1, FILE_SORRY_ERROR,
                 szFriendly, szTemp, P_Desc, 
                                szAction,  P_rc1,  P_rc2, P_Caller, P_DSN);

  MessageBox(hWnd_MAIN, szTmp1, Mpg2Cut2_FILE_ERROR,
                            MB_ICONSTOP | MB_OK);


   if (DBGflag)
   {
      fclose(DBGfile) ;
      DBGflag = false;
   }

}






unsigned char *lpNextChr;

// Convert a Parm Token into an integer
int X110_TK_Number()
{
  int iTmp1;

  lpNextChr = stpToken((unsigned char*)(&szTmp32[0]), lpNextChr, 1);
  iTmp1   =  iDeEdit(&szTmp32[0], 32);
  return iTmp1;
}



// Convert a series of integers into a Time Stamp

void X120_TK_HHMMSSFF(TC_HMSFR *lpTC)
{
  int iHH, iMM, iSS, iFF, iTmp1;

  iHH = 0;
  iMM = 0;
  iSS = X110_TK_Number();
  iFF = 0;

  if (cTK_Delim == ':')
  {  
    iMM = iSS;
    iSS = X110_TK_Number();
  }

  if (cTK_Delim == ':')
  {
    iHH = iMM;
    iMM = iSS;
    iSS = X110_TK_Number();
  }
  
  if (cTK_Delim == '.')
  {
     iFF = X110_TK_Number();
  }
  
  // Check for overflow
  if (iSS > 59)
  {
    iTmp1 = iSS / 60;
    iMM  += iTmp1;
    iSS  -= (iTmp1 * 60);
  }

  if (iMM > 59)
  {
    iTmp1 = iMM / 60;
    iHH  += iTmp1;
    iMM  -= (iTmp1 * 60);
  }

  lpTC->hour     = iHH;
  lpTC->minute   = iMM;
  lpTC->sec      = iSS;
  lpTC->frameNum = iFF;

}




//---------------------------------------------------


void X100_INIT(HINSTANCE hInstance, LPSTR lpCmdLine)
{

  DWORD dwStyle;
  // CWnd *pwnd;

  threadId_MAIN = GetCurrentThreadId();

  strcpy(szAppName, "Mpg2Cut2 ");

  DBGfile = 0; iDBGsuffix = 0; DBGflag = false;
  DBGctr = 0;  DBG_Alert_ctr = 0;
  bDBGStr = 0;

  iFin_Done = 0;
  iMainWin_State = 0; iAutoPlay = 0; iOutNow = 0;
  //iViewToolBar = 256;

  memset(&BLANK44[0], ' ', 44);
  BLANK44[44] = 0;

  ZeroMemory(&MParse, sizeof(MParse));
  MParse.Stop_Flag = true;

  hThread_OUT = 0; hThread_MPEG = 0; 
  hThread_SPEAKER = 0; dwSPEAKER_ThreadId = 0;

  ofn.lpstrInitialDir = NULL;  ofn.nFileOffset = 0;
  szInFolder[0] = 0;


  iBusy = 0;  uLastHelpTime = 0; hCSR_CROSS = 0;
  process.Delay_Sign[1] = 0;

  WAV_Fmt_Brief[0] = 0;
  //iWAVEOUT_Scheduled_Blocks = 0;
  //PlayedWaveHeadersCount = 0;
  //iWave_MsgAlerted = 0;

  VGA_GetSize();
  //VGA_Width  = GetSystemMetrics(SM_CXSCREEN) ;
  //VGA_Height = GetSystemMetrics(SM_CYSCREEN) ;


  iCtl_Text_Colour = 0xFFFEFE;  // Text = Bright Blue
  iCtl_Mask_Colour = 0x000600;  // Overlay key = Very Dark Green
  iCtl_Back_Colour = iCtl_Mask_Colour; // 0x000000;  // OLD Background = Black

  INI_VARS_BeforeMenu();
  INI_GET();
    
  if (iCtl_OVL_FullKey)
      iCtl_Mask_Colour = 0x000000;  // Overlay key = Black

  hBrush_MASK = CreateSolidBrush(iCtl_Mask_Colour);

  ToolBar_Metrics();  // Calculate metrics


  F920_Init_Names();

  if (_getcwd(szBuffer, _MAX_PATH)!=NULL)
  {
      if (szBuffer[strlen(szBuffer)-1] != '\\')
          strcat(szBuffer, "\\");

      strcpy(szPath, szBuffer);
  }

   
  iTmp16k_len = (MAX_FILE_NUMBER+2)*_MAX_PATH;

  lpTmp16K = (char *) malloc(iTmp16k_len);
  if (lpTmp16K == NULL)
     Err_Malloc(&"X100");


  // grab filename from parameter area          RJ

  char szTmp1[_MAX_PATH] ;
  char *lpTmp2, *lpTmp3;

  cPassed1 = 0 ;

  FromTC.hour = -1;  ToTC.hour = -1; iFromTC_Style = 1;
  szOutParm[0] = 0;  File_PreLoad = 0;
  process.length[0] = 0;

  // Dummy command lines for testing complicated invokation options
  //  Another alternative is to use BAT files.
  //lpCmdLine = &"\"G:\\$_VIDCAP\\_T\\A_QUITE_LONG_FOLDER_NAME_THIS_ONE_IS_I_THINK_GABBA_GABBA_HEY_\\KKL7701.MPG\" \"G:\\$_VIDCAP\\_T\\A_QUITE_LONG_FOLDER_NAME_THIS_ONE_IS_I_THINK_GABBA_GABBA_HEY_\\KKL7701M.MPG\"  \"G:\\$_VIDCAP\\_T\\A_QUITE_LONG_FOLDER_NAME_THIS_ONE_IS_I_THINK_GABBA_GABBA_HEY_\\KKL7701N.MPG\"  \"G:\\$_VIDCAP\\_T\\A_QUITE_LONG_FOLDER_NAME_THIS_ONE_IS_I_THINK_GABBA_GABBA_HEY_\\KKL7701P.MPG\" O=TEST  ";
  //lpCmdLine = &"G:\\$_VIDCAP\\_T\\p8_PALL.MPG ";
  //lpCmdLine = &"G:\\$_VIDCAP\\_T\\$_KKL_9313.EDL";


  if (lpCmdLine)
  {
    strncpy(szTmp1, lpCmdLine, (sizeof(szTmp1)-1));
    cPassed1 = (char)szTmp1[0] ;

    if (cPassed1 > 0)  // ' ')
    {

      lpTmp3 = strchr(lpCmdLine, '/');
      if (lpTmp3)
          iCtl_ParmClipSpec = 1;
      lpTmp3 = strchr(lpCmdLine, '"');
      if (lpTmp3)
          iCtl_ParmClipSpec = 1;

      if (iCtl_ParmClipSpec) // Allow multi-token parm format
      {
        // NEW FORMAT PARM LINE

        // First Parm is File Name, maybe in quotes with embedded spaces

        lpNextChr = stpToken((unsigned char*)(&szTmp1[0]), 
                             (unsigned char*)(lpCmdLine),  0);

        //   MessageBox( NULL, szTmp1, "Mpg2Cut2 - PARM WORD",
        //                           MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
           
        if (szTmp1[0] == '!' || szTmp1[0] <= ' ') // Parm commented out or blank
            cPassed1 = 0;
        else
        {

         strcpy(szInput, szTmp1);
         strcpy(File_Name[File_PreLoad], szTmp1);  
         File_PreLoad++;

         // Optional Extra parms

         while (*lpNextChr && *lpNextChr != '!')
         {
           lpNextChr = stpToken((unsigned char*)(&szTmp1[0]), lpNextChr, 0);

           if (cTK_Delim  == '"') // append another file name - MUST be in DOUBLE QUOTES
           {
               if (File_PreLoad < MAX_FILE_NUMBER)
               {
                  strcpy(szInput, szTmp1);
                  strcpy(File_Name[File_PreLoad], szTmp1);  
                  File_PreLoad++;
               }
               else
                  strcpy(szMsgTxt, "TOO MANY INPUT FILES !");

           }
           else
           {
             if (szTmp1[0] == '/') // remove slash
             {
                strcpy(szTMPname, &szTmp1[1]);
                strcpy(&szTmp1[0], szTMPname);
             }


             if (!stricmp(szTmp1,"FROM"))
             {
                 X120_TK_HHMMSSFF(&FromTC);
             }
             else
             if (!stricmp(szTmp1,"TO"))
             {
                 X120_TK_HHMMSSFF(&ToTC);
             }
             else
             if (!stricmp(szTmp1,"O")
             ||  !stricmp(szTmp1,"OUT"))
             {
                lpNextChr = stpToken((unsigned char*)(&szOutParm[0]), lpNextChr, 0);
             }
             else
             if (!stricmp(szTmp1,"DBG"))
             {
                 DBGctl();
             }
             else
             if (!stricmp(szTmp1,"ABS"))
             {
                iFromTC_Style = 0;
             }
             else
             if (!stricmp(szTmp1,"REL"))
             {
                iFromTC_Style = 1;
             }
             else
             if (!stricmp(szTmp1,"MAX")) // Maximize screen
             {
                iMainWin_State = 1;
             } 
             else
             if (!stricmp(szTmp1,"PLAY"))
             {
                iAutoPlay = 1;
             }
             else
             if (!stricmp(szTmp1,"DEFER"))
             {
                iOutNow = -1;
             }
             else
             if (!stricmp(szTmp1,"NOW"))
             {
                iOutNow =  1;
             }
             else
             if (!stricmp(szTmp1,"REM"))  // Comment out the rest of the parm area
             {
                break;
             }
             else
             if (szTmp1[0] > ' ')
             {
                 if (szTmp1[0] != '!') // Comment out the rest of the parm area
                 {
                     sprintf(szBuffer, PARM_UNKNOWN_KW,
                                 szTmp1) ;
                     MessageBox(hWnd_MAIN, szBuffer, szAppName, MB_ICONSTOP | MB_OK);
                 }
                 break;
             }
           }
         } // end-while

         if (DBGflag)
         {
           sprintf(szBuffer, "File=%s\n\nFrom=%02dh %02dm %02ds %02df\n     To=%02dh %02dm %02ds %02df\n",
                   szInput, 
                   FromTC.hour, FromTC.minute, FromTC.sec, FromTC.frameNum,
                     ToTC.hour,   ToTC.minute,   ToTC.sec,   ToTC.frameNum);
           DBGout(szBuffer);
           MessageBox(hWnd_MAIN, szBuffer, szAppName, MB_OK);
         }
         
        }  // end-else
                
      }
      else
      {
         // OLD SINGLE PARM EXTRACTION
         if (cPassed1 == '"' || cPassed1 == 0x27)  // Single or double quote
         {
            strcpy(szInput, lpCmdLine+1);
            lpTmp2 = strchr(szInput, cPassed1) ;
            if (lpTmp2)
                *lpTmp2 = NULL;
         }
         else
         {
            strcpy(szInput, lpCmdLine);
         }
         if (szInput[0])
         {
             strcpy(File_Name[File_PreLoad], szInput);  
             File_PreLoad++;
         }
      }

    }
  }
 
//else
// {
//   sprintf(szInput, "lpCmdLine-%d \n",lpCmdLine);
// }

  sprintf(szAppTitle, "Mpg2Cut2 - Development Version %s", szAppVer); // You don't expect this to work do you ?"

  MyRegisterClass(hInstance);

  if (iMainWin_State > 0)
      dwStyle = WS_MAXIMIZE;
  else 
      dwStyle = 0;

  dwStyle = dwStyle | ( WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME));

   // prepare main window
   hWnd_MAIN = CreateWindow(szWindowClass, szAppTitle,
                       dwStyle,    
                       Restore_X, Restore_Y, 
                       (iToolbarWidth+6), (MIN_OVL_HEIGHT+150), //360,
                                 NULL, NULL, hInstance, NULL);

  iPrevWinPos.left = Restore_X; iPrevWinPos.top = Restore_Y;

  MainWin_Rect();
  Calc_PhysView_Size();

  // RJ MOVED macroblock tile mallocs into the frame subpool system (block, p_block)

  Initialize_REF_IDCT();
  Initialize_FPU_IDCT();


  INI_MERGE(); 


mycallbacks.hMPEGKill=CreateEvent(0,FALSE,FALSE,0);

}



 



void Err_Malloc(void *P_Proc)
{
  sprintf(szBuffer, MEM_ALLOC_ERROR,
          P_Proc);

  MessageBox( NULL, szBuffer, Mpg2Cut2_ERROR,
                                   MB_OK | MB_SETFOREGROUND | MB_TOPMOST);

  Y100_FIN();
  PostQuitMessage(0);
  MParse.Stop_Flag=1;
  ExitThread(0);

}

//--------------------------------------------------------------
//
// Various functions moved into separate modules

// But since I don't know how to tell "C" that I have done this
// without gettting strange link errors,
// some are still compiled as part of this module


// Direct Draw handling
#include "DDRAW_CTL.cpp"






