/*
  MPEG2DEC - Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved.
  DVD2AVI -  Copyright (C) Chia-chen Kuo - April 2001
  Mpeg2Cut - Dark Avenger and others
  Mpg2Cut2 - RocketJet and others
*/

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * According to Wikipedia, the patents covering mpeg-1 video
 * and audio layers 1 & 2  have now expired.
 *
 * Commercial implementations of MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */
#include <windows.h>
//#include <commctrl.h>
#include <stdio.h>

#include <winreg.h>
//#include <direct.h>
#include <io.h>
//#include <fcntl.h>
#include "resource.h"
//#include "mpalib.h"

#include <time.h>
#include <SYS\stat.h>

char szAppTitle[50]; // "Mpg2Cut2 - Development Version 2.8.6" // You don't expect this to work do you ?"


// When changing version,
// remember to change version strings in Resources as well.
#define APP_VER 20806
#define API_VER 20802
char szAppVer[8]
#ifdef GLOBAL
 = "2.8.6d"
#endif
;



//#define 1MB  1048576
#define MAXINT64 1>>62 - 1
#define MAXINT31 0x7FFFFFFF

#define MAX_FILE_NUMBER     128
#define CHANNELS_MAX      8

#define K_8MB   8*1024*1024
#define K_4MB   4*1024*1024
#define K_1MB     1024*1024
#define K_256KB    256*1024

#define SetMemory memset
#define SetMem    memset
#define MemSet    memset

HINSTANCE hInst;

HWND hWnd_MAIN; // hDlg;
HWND hStats, hTrack, hMsgTxt, hTimeTxt;

HANDLE hThread_MPEG, hThread_OUT, hThread_PARM2CLIP;

DWORD threadId_MAIN
#ifdef GLOBAL
 = 0
#endif
;

DWORD threadId_MPEG
#ifdef GLOBAL
 = 0
#endif
;


DWORD threadId_PARM2CLIP
#ifdef GLOBAL
 = 0
#endif
;
DWORD OUT_threadId;



// MAJOR FILE CONTROLS
int   FileDCB[MAX_FILE_NUMBER+1];
char *File_Name[MAX_FILE_NUMBER+1];
unsigned cStartSCR[MAX_FILE_NUMBER][2];
unsigned cEndSCR[MAX_FILE_NUMBER][2];
int /* FILETIME */ File_Date[MAX_FILE_NUMBER+1]; //[3];  WIN32 API FORMAT
typedef int /* __int64 */ IFILEDATE;
struct tm File_Greg[MAX_FILE_NUMBER+1];

int iCtl_Date_Internationale, iCtl_Readability, iCtl_ToolTips;
int iCtl_Wheel_Scroll;
int iToolButton_Cnt;


char *lpLastSlash(char *);
char *lpDOT(char *);

char szAppName[256];

char szTmp32[32], szTmp80[80], szTmp100[100], szTmp256[256];
char szMsgTxt[_MAX_PATH*6], szBuffer[_MAX_PATH*10];
char szDBGln[512];
char szMPG_ErrTxt[16];

char szTemp[_MAX_PATH+32];
char szTMPname[_MAX_PATH+32];
char BLANK44[45]; //  "                                            "
#define BLANK0 ""

struct _stati64 TmpStat;
struct tm *TmpGregTime;

/* code definition */
#define PICTURE_START_CODE      0x0100
#define cPICTURE_START_CODE     0x01
#define sPICTURE_START_CODE     0x0001
#define uPICTURE_START_CODE     0x00010000 // PIC

#define SLICE_START_CODE_MIN    0x0101
#define SLICE_START_CODE_MAX    0x01AF

#define USER_DATA_START_CODE    0x1B2

#define xSEQ_HDR_CODE           0x1B3        // SEQ
#define uSEQ_HDR_CODE           0xB3010000   // SEQ
#define sSEQ_HDR_CODE           0xB301       // SEQ
#define cSEQ_HDR_CODE           0xB3         // SEQ

#define EXTENSION_START_CODE    0x1B5
#define sEXTN_HDR_CODE          0xB501       // Extension
#define cEXTN_HDR_CODE          0xB5         // Extension

#define SEQUENCE_END_CODE     0x1B7

#define GROUP_START_CODE      0x1B8      // GOP
#define sGROUP_START_CODE     0xB801     // GOP
#define cGROUP_START_CODE     0xB8
#define cGOP_START_CODE       0xB8

#define SYSTEM_END_CODE       0x1B9

#define PACK_START_CODE       0x1BA      // PACK START
#define uPACK_START_CODE      0xBA010000 // PACK START (FULL)
#define cPACK_START_CODE      0xBA       // PACK START

#define SYSTEM_START_CODE     0x1BB      // System_Header_Start_Code
#define uSYSTEM_START_CODE    0xBB010000 // System_Header (FULL)

#define PRIVATE_STREAM_1      0x01BD    // Usually DVD audio
#define cPRIVATE_STREAM_1     0xBD      // Usually DVD audio
#define PRIVATE_STREAM_2      0x01BF    // DVD Nav Pack OR DTV Audio

#define uVIDPKT_STREAM_1  0xE0010000

#define VIDEO_ELEMENTARY_STREAM_1  0x1E0
#define VIDEO_ELEMENTARY_STREAM_2  0x1E1
#define VIDEO_ELEMENTARY_STREAM_3  0x1E2
#define VIDEO_ELEMENTARY_STREAM_4  0x1E3
#define VIDEO_ELEMENTARY_STREAM_5  0x1E4
#define VIDEO_ELEMENTARY_STREAM_6  0x1E5
#define VIDEO_ELEMENTARY_STREAM_7  0x1E6
#define VIDEO_ELEMENTARY_STREAM_8  0x1E7
#define VIDEO_ELEMENTARY_STREAM_9  0x1E8
#define VIDEO_ELEMENTARY_STREAM_10 0x1E9
#define VIDEO_ELEMENTARY_STREAM_11 0x1EA
#define VIDEO_ELEMENTARY_STREAM_12 0x1EB
#define VIDEO_ELEMENTARY_STREAM_13 0x1EC
#define VIDEO_ELEMENTARY_STREAM_14 0x1ED
#define VIDEO_ELEMENTARY_STREAM_15 0x1EE
#define VIDEO_ELEMENTARY_STREAM_16 0x1EF
#define STREAM_AUTO 999990
#define STREAM_NONE 987654
#define STREAM_ALL  999999

#define sPADDING_STREAM_ID    0x01BE
#define cPADDING_STREAM_ID      0xBE

#define AUDIO_ELEMENTARY_STREAM_0  0x1C0
#define AUDIO_ELEMENTARY_STREAM_1  0x1C1
#define AUDIO_ELEMENTARY_STREAM_2  0x1C2
#define AUDIO_ELEMENTARY_STREAM_3  0x1C3
#define AUDIO_ELEMENTARY_STREAM_4  0x1C4
#define AUDIO_ELEMENTARY_STREAM_5  0x1C5
#define AUDIO_ELEMENTARY_STREAM_6  0x1C6
#define AUDIO_ELEMENTARY_STREAM_7  0x1C7
#define AUDIO_ELEMENTARY_STREAM_15 0x1CF

#define SUB_SUBTIT        0x20
#define SUB_AC3           0x80
#define SUB_DTS           0x88
#define SUB_PCM           0xA0
#define SUB_PS2           0xBF
#define SUB_DDPLUS        0xC0
//                        DDPLUS sub-code is same as Mpeg-Audio Stream-id
//                        Hopefully won't find the 2 types intermixed.

/* extension start code IDs */
#define SEQUENCE_EXTENSION_ID           1
#define SEQUENCE_DISPLAY_EXTENSION_ID   2
#define QUANT_MATRIX_EXTENSION_ID       3
#define COPYRIGHT_EXTENSION_ID          4
#define PICTURE_DISPLAY_EXTENSION_ID    7
#define PICTURE_CODING_EXTENSION_ID     8

#define ZIG_ZAG                 0
#define MB_WEIGHT               32
#define MB_CLASS4               64


#define MACBLK_INTRA        1
#define MACBLK_PATTERN      2
#define MACBLK_MOTION_BWD   4
#define MACBLK_MOTION_FWD   8
#define MACBLK_QUANT        16


#define MC_FIELD    1
#define MC_FRAME    2
#define MC_16X8     2
#define MC_DMV      3

#define MV_FIELD    0
#define MV_FRAME    1

#define CHROMA420   1
#define CHROMA422   2
#define CHROMA444   3

#define MPEG_SEARCH_BUFSZ 32768 // WAS 10240
//2048
//10240

#define STORE_RGB24   1
#define STORE_YUY2    0

#define IDCT_MMX    1
#define IDCT_SSEMMX   2
#define IDCT_FPU    3
#define IDCT_REF    4
#define IDCT_SSE2   5



#define FO_NONE     0
#define FO_FILM     1
#define FO_SWAP     2

#define SRC_NONE    0
#define SRC_LOW     1
#define SRC_MID     2
#define SRC_HIGH    3
#define SRC_UHIGH   4

#define TRACK_PITCH     480
#define MIN_D2V_WIDTH     288
#define MIN_D2V_HEIGHT    128

#define CRITICAL_ERROR_LEVEL  50


typedef struct {
  int     type;
  int     file;
  __int64   lba;
  int     Progressive_Format;
  int     Fld1_Top_Rpt;
} D2VData;

D2VData d2v_bwd, d2v_fwd, d2v_curr;



// MPEG2DECODER

#define ACTION_NOTHING -696969

#define ACTION_BWD_JUMP4 -40
#define ACTION_BWD_JUMP2 -20
#define ACTION_BWD_JUMP  -10
#define ACTION_BWD_GOP2   -5
#define ACTION_BWD_GOP    -1

#define ACTION_FWD_GOP     1

#define ACTION_FWD_FRAME   66

#define ACTION_RIP        110
#define ACTION_PLAY       130


#define ACTION_FWD_GOP2   200
#define ACTION_FWD_JUMP   250
#define ACTION_FWD_JUMP2  290
#define ACTION_FWD_JUMP4  294

#define ACTION_INIT         300
#define ACTION_NEW_CURRLOC  301
#define ACTION_NEW_RUNLOC   302
#define ACTION_SKIP_FILE    303


struct PROCESS {  // process.

  __int64     length[MAX_FILE_NUMBER];
  __int64     i64RestorePoint[MAX_FILE_NUMBER];
  __int64     GoodLoc1;
  __int64     total;
  __int64     run;
  __int64     startrunloc;
  __int64     endrunloc;

  __int64   startLoc;
  __int64   endLoc;
  //__int64   CurrBlk;
  //__int64   FromBlk;
  //__int64   ToPadBlk;
  __int64   ToPadLoc;
  //__int64   ToViewBlk;
  __int64   ToViewLoc;

  __int64   BackLoc; // Previous GopBack start point
    int     BackFile; // "         "     start file

  __int64   origin[MAX_FILE_NUMBER]; // Logical offset at start of file
  //__int64   First_Blk;
  //__int64   Pack_Blk;

  __int64   FromLoc;  // moved here because of something strange happening
  __int64   CurrLoc;
  __int64     LocJump ;     // RJ Allow generalized jumping
  __int64     preamble_len;  // length of control info at start of file, prio to first SEQ

  __int64     PAT_Loc;      // ptr to 1st TS PAT header
  __int64     NAV_Loc;      // ptr to MOST RECENT VOB NAV PACK header
  __int64     PACK_Loc;     // ptr to MOST RECENT pack header
  __int64     VIDPKT_Loc;   // ptr to MOST RECENT Video Packet header
  __int64     SEQ_Loc;      // ptr to MOST RECENT SEQ HDR
  __int64     GOP_Loc;      // ptr to MOST RECENT GOP HDR
  __int64     KEY_Loc;      // ptr to MOST RECENT KEY FRAME (I-Frame)
  __int64     PIC_Loc;      // ptr to MOST RECENT PIC
  __int64     KILL_Loc;     // ptr to MOST RECENT END-PROCESS BYTE
  __int64     ALTPID_Loc;   // ptr to ALternate PID I-FRAME Location

  __int64     PREV_Pack_Loc;    // ptr to PREVIOUS pack header
  __int64     PREV_Curr_Loc;    // ptr to PREVIOUS CurrLoc

   __int64  Last_Gop_Loc_Est ;  // Approx ptr to final GOP

  __int64   kBitRateAvg, i64NewByteRate; // usually measured from data
  __int64   ByteRateAvg[MAX_FILE_NUMBER];


  int       trackleft;
  int       trackright;
  int       trackPrev;

  int       Action;

  int       startFile;

  int       endFile;

  int       CurrFile;
  __int64   DUMMYc;
  unsigned  CurrPTS;
  unsigned char Curr_TSM[5];

  unsigned int uGOP_TCorrection, uGOP_FPeriod_ps;

  int       FromFile;
  __int64   DUMMYf;
  unsigned  FromPTS;
  unsigned  FromPTSM;
  unsigned int uFrom_TCorrection, uFrom_FPeriod_ps;

  int       ToPadFile;
  unsigned  ToPadPTS;
  unsigned  ToPadPTSM;

  int       ToViewFile;
  unsigned  ToViewPTS;
  unsigned  ToViewPTSM;


  unsigned int  op;
  unsigned int  ed;
  unsigned int  elapsed;
  unsigned int  remain;

#define PTS_NOT_FOUND  1 // Cannot use -1 because PTS is unsigned
#define PTS_MASK_0       0xFFFEFF0E // 0x000100F1
#define i64PTS_MASK_0  0xFEFFFEFF0E
#define SCR_MASK_0       0xFFFBFF3B // 0011 1011 1111 1111 1111 1011 1111 1111 1111 1011 1111 1110

  unsigned int VideoPTS, VideoPTSM, VideoDTS, VideoPrevPTS;
  unsigned int ViewPTS,  ViewPTSM,  ViewDTS;
  unsigned int AudioPTS, DelayPTS, Delay_ms, SkipPTS, uGOPbefore;

  int  Delay_Calc_Flag;
  char Delay_Sign[2];
  char szDelay[16];

  unsigned char CurrSSCRM[6],  ViewSSCRM[6], PrevSSCRM[6];
  unsigned int uViewSCR;

  int iGOP_Ctr;
  int iAudio_Ovfl_Ctr, iAudio_SelStatus, iAudioAC3inMPA;
  int iAudio_InterFrames, iAudio_Interleave;
  int iSEQHDR_NEEDED_clip1, iPreamblePackAtStart, iLongGOP;
  int iVid_PTS_Frames, iVid_PTS_Resolution;
  unsigned int uVid_PTS_Seq_Prev;
  int iVid_PTS_Seq_Diff, iVid_PTS_Diff;

  float     percent;
  int       iAudio_Error;
  unsigned uMPA_Mpeg_Ver[CHANNELS_MAX];
  //DWORD     uMPA_Sample_Hz[CHANNELS_MAX];

  int       First_File;
  int       First_Offset;
  int       Pack_File;
  int       Pack_Offset;

  //int       from_Offset;
  //int       to_Offset;

  int     PAT_File;     // ptr to FIRST TS PAT header
  int     NAV_File;     // ptr to MOST RECENT VOB NAV pack header
  int     PACK_File;    // ptr to MOST RECENT pack header
  int     VIDPKT_File;  // ptr to MOST RECENT pack header
  int     SEQ_File;     // ptr to MOST RECENT SEQ HDR
  int     GOP_File;     // ptr to MOST RECENT GOP HDR
  int     KEY_File;     // ptr to MOST RECENT KEY FRAME (I-Frame)
  int     PIC_File;     // ptr to MOST RECENT PIC
  int     KILL_File;    // ptr to MOST RECENT END-PROCESS
  int     ALTPID_File;  // ptr to ALternate PID
  int     TMP_File;     // ptr to
  int     PREV_Pack_File;   // ptr to PREVIOUS pack header

  int     PrevAct;

  char    Broken_start_type;  // test for missing GOP header at start
  int     iUnique;
  int     iOutUnMux, iOutParseMore, iOutFolder_Flag;
  int     iOut_Part_Ctr, iOut_AutoSPLIT;

  int     Pack_Prev_Size, Pack_Max_Size, Pack_Min_Size, Pack_Avg_Size;
  int     PACK_Sample_Ctr;
  __int64 i64Pack_Sum_Size;

  int Mpeg2_Flag;        // Mpeg Version - Hopefully Mpeg2
  int iOut_DropCrud;     // Drop CDXA RIFF Wrapper (Non-Mpeg Data)
  int iView_Extra_Crop, iView_TopMask;

  int Suspect_SCR_Flag;   // Australian Ch.7 weird SCR pattern
  int Missing_PTS_Flag, Got_PTS_Flag;   // TwinHan Bug
  int BlksSinceLastEffect;

  int Prev_Seq_BitRate, iFixedRate;

  int PES_Audio_CRC_Flag;
  int iWavQue_Len, iWavBytesPerMs;
  int iPMM_Ctr, iPSD_Ctr;
  int iCatchUp;
  int iAudio_PS1_Found, iAC3_used;
  int iAudio_MPA_Found;

  BYTE Preamble_PackHdr[24],  Preamble_SysHdr[64];
  int Preamble_PackHdr_Found, Preamble_SysHdr_Found, Preamble_Known;
  unsigned  Preamble_PTS, Preamble_PTSM;

  int Keep_Broken_GOP;
  int iWarnSize_1, iWarnSize_2, iWarnSize_3, iWarnSize_4;
  int iEncryptAlerted, iBadSYSAlerted;
  int EDL_Used;
  int PAT_Len;

} process;

#define BYTERATE_DEF 123456


struct PLAY_CTLS{
  int uPrev_Frames,  iPlayed_Frames, iUnReportedFrames;
  int iShown_Frames, iDropped_Frames, iBehindFrames;
  int iDecoded_hFPS, iFPS_Dec_Pct;
  int iDrop_Behind, iDrop_PTS_Flag;
  int iDrop_B_Frames_Flag, iErrMsg, iMaxedOut;
  unsigned int  uPrev_Time_ms[2],  uPrev_Time10_ms;
  DWORD iVideoTime_DIFF_ms, iAudioTime_DIFF_ms;
  unsigned int uPrev10_Time_ms;
  int iAudio_Warp_Accum, iPalTelecide_ctr;
  int iAudioFloatingOvfl;
  unsigned int uAud_Packets, uAudioOvflPkts;
  int iEOFhit, iStopNextFrame;
  int iAC3_Attempted, iPCM_Remainder, iPCM_Orphan;
  int iGOP_Ctr, iAudio_SelStatus;
  int iWarpDone, iWarpToggle;
  unsigned int uPendingSeq[4];
  //unsigned int uAngle;
} PlayCtl;

typedef struct {
  unsigned int iVid_Packets, iPad_Packets, iUnk_Packets, iSubTit_Packets;
  unsigned int iMPA_Packets, iPS1_Packets, iPS2_Packets;
           int iChk_AnyPackets, iChk_AudioPackets;
  unsigned int iTS_Packets, iTS_ReSyncs, iTS_BadBytes;
  unsigned int uPrevTimeUpd;
  int iAudDelayBytes;
} PACKET_CTRS;

PACKET_CTRS PktStats;

int PktChk_Audio, PktChk_Any;
#define PKTCHK_AUDIO_TS 1000
#define PKTCHK_ANY_TS  24000


int  Stream_Header_Len;

unsigned char PAT_Data[8192];  // TS Stream - Pgm Access Tbl




// OUT.C
int iOut_PTS_Invent, iOut_HideAudio;
int iOut_FixPktLens, iOut_PS1PS2;

void OUT_SAVE(char);
void Out_Split_Hdr_Msg(const char P_Act[]);
void SCRM_2SCR(unsigned char *); //  CRM-114




LRESULT CALLBACK PostProc_Dialog(HWND, UINT, WPARAM, LPARAM);
//HWND  hPostDlg;

LRESULT CALLBACK ExitCtl_Dialog(HWND hDialog, UINT message,
                                WPARAM wParam, LPARAM lParam);
//HWND  hExitDlg;



OPENFILENAME ofn;

int iDummy1;
int  iMpeg_Copy_BufSz, iCtl_Copy_BufSz_Ix;  // default 4 MB
BYTE *lpMpeg_Copy_Buffer, *lpMpeg_Copy_MALLOC;  // Big buffer for file copy
unsigned char *lpMpeg_TC_ix2;

char szStartDesc[7];



int iOut_PostProc_OK;
//int iOut_KeepFileDate;




struct CPU {
  BOOL          mmx;
  BOOL          _3dnow;
  BOOL          ssemmx;
  BOOL          ssefpu;
  BOOL          sse2;
} cpu;


/* decoder operation control flags */
int AVI_Flag;
int PicBuffer_Canvas_Size;
int D2V_Flag;
int DDOverlay_Flag;
int iShowVideo_Flag, iCtl_ShowVideo_Flag; // Was: Display_Flag
int iCtl_CropTop;

int Clip_Width, Clip_Height, Resize_Width, Resize_Height;
int iPreview_Clip_Ctr;
int iViewToolBar, iCtl_ViewToolbar[2];




struct
{
int FO_Flag;
int iDCT_Flag;
int Rip_Flag;
int PC_Range_Flag;
int ShowStats_Flag;
int Pause_Flag, SlowPlay_Flag, FastPlay_Flag, FastPart;
int Summary_Section, Summary_Adjust;
__int64 NextLoc;
int Anti_Phase, Karaoke_Flag;
int Fault_Flag, Fault_More, Fault_Prev;
int Stop_Flag,  iMultiRqst;
int iColorMode;
int SeqHdr_Found_Flag, ReInit, SizeCommitted;
int SystemStream_Flag;
int Tulebox_SingleStep_flag;
int Tulebox_prev_frame_number;
int EDL_AutoSave;
int iVOB_Style, iGOPsSinceNAV;
} MParse;

#define MAX_WARP 4
#define CUE_SLOW  MAX_WARP+1
int iCtl_CUE_BUTTON_Speed, iCtl_NotSoFast;

int iWarnBadDate, iWarnAC3inMPA;
int ClipResize_Flag;

void Store_RGB24(unsigned char *src[], int P_SnapOnly); //, DWORD frame); //RJSUS3a29

char Out_CanFlag ;
int Out_PauseFlag;
void  UpdInfo(), MainPaint();

int  Loc_Method; // 0= Original Block method; 1=Header Pointers
int  iCtl_To_Pad;

// Keyboard options
int iCtl_KB_NavOpt, iCtl_KB_MarkOpt, iCtl_KB_NavStopPlay;
int iCtl_KB_V_Popup;
int iKB_MARK_FROM, iKB_MARK_TO;
int iCtl_F3_Names, iCtl_F5_Toggler;

int iCTL_FastBack;
int iBusy, iCtl_Drv_Segments;

int iJumpSecs[6];


int Edge_Width, Edge_Height;

// int ClientWidth, ClientHeight ;
int iMainWin_State;
int Overlay_Width, Overlay_Height;
int iPhysView_Width, iPhysView_Height, iMenuHeight;
int Prev_Coded_Width, Prev_Coded_Height;

int VGA_Width, VGA_Height, VGA_New_Width ;
RECT rcAvailableScreen;

char RecoveryReason[3] ;

//APPBARDATA TaskBar;
//int iTaskBar_Width, iTaskBar_Height;
int iVGA_Avail_Height, iVGA_Avail_Width;
int iViewMax_Height, iViewMax_Width;

//void CalcRestoreSize(int, int) ;

char *lpTmp16K, *lpTmpIx, *lpInputIx, *lpFromIx;
int    iTmp16k_len;

char cPassed1, cTK_Delim, szOutParm[_MAX_PATH];

char szInput[_MAX_PATH], *lpFName;
char szInFolder[_MAX_PATH];

#define MAX_LIKE_LEN 42
char szFile_Prefix[MAX_LIKE_LEN+8];

char szOutput[_MAX_PATH]; // , *lpOut_DOT;
char szOutFolder[_MAX_PATH];
char szName[_MAX_PATH]; //, szOutFolder_DEF[_MAX_PATH];
char szInExt3_lowercase[4], szInExt[8];
char szOut_Xtn_RULE[8], szOut_Xtn_DEF[8] ;
char szOut_Xtn_AUD[8];

char szLOAD_Path[MAX_PATH], szINI_Path[_MAX_PATH];
char szEDLname[_MAX_PATH], szEDLprev[_MAX_PATH];

int iMsgLife, iMsgLen;
int iAutoPlay, iOutNow;

//#include "OpenDVD_0.h"

HMENU hMenu; HDC hDC;
int iDDO_LineCtr, iDDO_Frame_Ready;


//FILE *D2VFile;



unsigned char *lpBMP_Buffer;
int iBMP_BufSize;

//unsigned char *end_bwd_ref[3], *end_fwd_ref[3];
//unsigned char *end_aux[3], *end_curr[3];

__int64 RGB_Scale, RGB_Offset, RGB_CRV, RGB_CBU, RGB_CGX;

void Lum_Filter_Init(int);

unsigned int Frame_Number;


typedef struct TC_HMSFR
{
  int hour;
  int minute;
  int sec;
  int frameNum;
  int RunFrameNum;
  unsigned VideoPTS, VideoDTS, AudioPTS, EffectivePTS;
}  TC_HMSFR;

 TC_HMSFR OrgTC;

 TC_HMSFR  gopTC ;

  int gop_drop_flag;
  int gop_closed_flag;
  int gop_broken_link_flag;



TC_HMSFR CurrTC;
TC_HMSFR PrevTC;
TC_HMSFR RelativeTC;

TC_HMSFR  ptsTC ;

typedef struct TC_HMSF
{
  short int hour;
  short int minute;
  short int sec;
  short int frameNum;
}  TC_HMSF;

TC_HMSF  FromTC[33];
TC_HMSF  ToTC[33];

int iParmTC_ctr;
int iParmTC_Style;

int Coded_Pic_Width,  Coded_Pic_Height, Coded_Pic_Size ;
int Chroma_Width, Chroma_Height, Chroma_Size, RGB24_size;
int Mpeg_MacroBlk_Array_Limit;
int mb_width, mb_height;
int DOUBLE_WIDTH, HALF_WIDTH, HALF_WIDTH_D8;
int CLIP_AREA, HALF_CLIP_AREA;
;

int ScanMode_code;
static char *ScanMode_Name[2] = {"interlaced", "progressive"};

int Second_Field;
int Top_Field_Built, Bot_Field_Built;
int Pic_Started;

char MPEG_Seq_chroma_Desc[5][7]
#ifdef GLOBAL
= { {"4:2:00"}, {"4:2:0"}, {"4:2:2"}, {"4:4:4"}, {""} } // Zero=Mpeg-1
#endif
;

int MPEG_Profile;
char *MPEG_Profile_Desc[8]
#ifdef GLOBAL
= { "", "HP", "SS",  "SNR", "MP", "SP", "6P", "7P"}
#endif
;

char MPEG_ProfLvl_Escape;

int MPEG_Level;
char *MPEG_Level_Desc[16]
#ifdef GLOBAL
= { "",   "H1L", "H2L",   "H3L",
    "HL", "H5L", "H1440", "H7L",
    "ML", "M9L",
    "LL", "LBL", "LCL", "LDL",   "LEL", "LFL" }
#endif
;

char Coded_Pic_Abbr[8]
#ifdef GLOBAL
 = {'0', 'i', 'p', 'b', '4', '5', '6', '7'}
#endif
;

double fFrame_rate, fFrame_Rate_Orig;
double fFrame_rate_extension_n, fFrame_rate_extension_d;

int iFrame_Period_ms, iFrame_Period_ps;
int iDropTrigger_ms, iSleepTrigger_ms;
int iFrame_Rate_ms, iFrame_Rate_int, iFrame_Rate_dsp, iFrame_Rate_mantissa;
int iFrame_Rate_Code;
int iRender_TimePrev;

int Video_Type, FILM_Purity, NTSC_Purity ;

int iVideoBitRate_Bytes, iVideoBitRate_Avg, iMuxChunkRate;
int iNom_kBitRate, iNom_ByteRate;

int Clip_Top, Clip_Bottom, Clip_Left, Clip_Right;
int Squeeze_Width, Squeeze_Height;

int iView_Aspect_Mode, iCtl_Aspect_Retain;
int iView_FrameRate_Code, iOverride_FrameRate_Code;
int iView_Centre_Crop;
int iField_Drop, iField_Experiment;
int iCtl_View_Aspect_Mpeg1_Force, iCtl_View_Aspect_Adjust;
int iView_SwapUV, iView_Neg_Lum, iView_Neg_Sat, iView_Invert;
int iCtl_VHS_Threshold, iCtl_SinThreshold;
int iCtl_View_Fast_YUV, iView_Fast_YUV;
int iView_Limit2k, iCtl_View_Limit2k;
int iCtl_View_Fast_RGB, iView_Fast_RGB, iCtl_View_RGB_Always;
int iCtl_Ovl_Release, iCtl_Ovl_DWord, iCtl_OVL_ATI_Bug;
int iCtl_OVL_FullKey, iCtl_YV12;
int iCtl_View_Centre_Crop, iCtl_AspMismatch;

int iAspectIn, iAspectOut, iVertInc, iAspVert, iAspHoriz;
int iView_Width,  iATI_BugMe;
;

int iView_TC_Format;
int iCtl_Play_Info;

//#define MASKCOLOR       RGB(0, 6, 0)
//#define MASK_TWINHAN RGB(212, 208, 200)

int iColor_Menu_BG, iColor_Msg_BG;
int iCtl_Text_Colour, iCtl_Back_Colour, iCtl_Mask_Colour, iCtl_Mask_Fallback;
int iCtl_BMP_Aspect, iCtl_BMP_Preview, iBMP_Preview;



int               iGet_xFromBytes, iGet_yFrom;
int iGet_Width,   iGet_WidthBytes;
int iGet_Height,  iGet_Bot;
int iPut_Width;
POINT iView_Point;


int iAspect_Height, iAspect_Curr_Code;
int iAspect_Width, iAspect_Width2;
int iPred_Prev_Width, iAspect_Width_Max;
//int iAspect_Width_Unzoom;
int iCtl_Zoom_Wanted, iZoom, iCtl_Zoom_Retain; //, iCtl_Zoom_Default;
int iSrcUse; //, iZoom_OLD, iZoomHeight;
int iView_xFrom, iView_xFromBytes, iView_yFrom;
int iView_xMiddle, iView_yMiddle;
//int iViewWidth2;
int iPrim_Width;
int DOUBLE_CLIP_WIDTH;
int iOverload_Width, iOverload_Height;
int Prev_Clip_Width, Prev_Clip_Height;

int Deint_VIEW, Deint_Auto_CURR, Deint_AUTO_View, Deint_SNAP, Deint_VOB;

int  Restore_X, Restore_Y, Restore_Width, Restore_Height;


int iOut_Fix_SD_Hdr, iOut_Fix_Hdrs_Vid, iIn_VOB;
int iPES_Mpeg_Any;

int iGOPtime, iGOPrelative, iGOPtot, iGOPdiff, iGOPperiod, iPICtime;
int iGOPdrop_Ctr, iGOPdec_Ctr;

void Mpeg_Aspect_Calc(), Mpeg_Aspect_Resize();



/* ISO/IEC 13818-2 section 6.2.2.1:  gothdr_SEQ() */
int MPEG_Seq_horizontal_size ;  // also updated from extension
int MPEG_Seq_vertical_size ;    // also updated from extension
int MPEG_Seq_frame_rate_code;
int MPEG_iFrame_rate_extension_n;
int MPEG_iFrame_rate_extension_d;

int MPEG_Seq_aspect_ratio_code;
int MPEG_Seq_NomBitRate400 ;
/* ISO/IEC 13818-2 section 6.2.2.3:  sequence_extension() */
int MPEG_Seq_progressive_sequence;
int MPEG_Seq_chroma_format;

int MPEG_Seq_intra_quantizer_matrix[64];
int MPEG_Seq_non_intra_quantizer_matrix[64];
// following 2 fields are derived from the Mpeg_Seq Header
int chroma_intra_quantizer_matrix[64];
int chroma_non_intra_quantizer_matrix[64];

int MPEG_Seq_load_intra_quantizer_matrix; // also updated from extension
int MPEG_Seq_load_non_intra_quantizer_matrix; //    "       "     "
int MPEG_Seq_load_chroma_intra_quantizer_matrix;
int MPEG_Seq_load_chroma_non_intra_quantizer_matrix;


/* ISO/IEC 13818-2 section 6.2.3: gothdr_PICTURE() */

int MPEG_Pic_Type, PREV_Pic_Type;
#define I_TYPE      1
#define P_TYPE      2
#define B_TYPE      3
unsigned int MPEG_Pic_Temporal_Ref;
/* ISO/IEC 13818-2 section 6.2.3.1: picture_coding_extension() header */
int MPEG_Pic_f_code[2][2];
int MPEG_Pic_Structure;
#define TOP_FIELD   1
#define BOTTOM_FIELD  2
#define FULL_FRAME_PIC 3

int MPEG_Pic_pred_frame_dct;
int MPEG_Pic_Origin_progressive;
int MPEG_Pic_composite_display_flag;
int MPEG_Pic_concealment_motion_vectors;
int MPEG_Pic_q_scale_type;
int MPEG_Pic_intra_dc_precision;
int MPEG_Pic_top_field_first;
int MPEG_Pic_repeat_first_field;
int MPEG_Pic_intra_vlc_format;
int MPEG_Pic_alternate_scan;
int MPEG_Pic_chroma_420_type;


//--------------------------------------------------

char *Mpeg2_Aspect_Ratio_Name[16]
#ifdef GLOBAL
= { "asp0",  "VGA",  "4:3",   "16:9",  "2.21",   "asp5",  "asp6",  "asp7",
    "asp8", "asp9", "asp10", "asp11", "asp12",  "asp13", "asp14", "asp15"}
#endif
;


// For Mpeg-1 Aspect is actually Pixel aspect, not Frame Aspect.
char *Mpeg1_Aspect_Ratio_Name[16]
#ifdef GLOBAL
   = { "asp0",     "VGA",     "a.6735",   "a.7031",
       "a.7615",   "a.8055",  "a.8437",   "a.8935",
       "a.9375",   "a.9815",   "a1.0255", "a1.0695",
       "a1.1250",  "a1.1575",  "a1.2015", "asp15"}
#endif
;

void Set_Aspect_MPG1(int P_NewSetting);
//--------------------------------------------------

/* gethdr.c */
int GetHdr_PIC(int);
void gothdr_SEQ(void);
void PTS_2Field(unsigned, int);
void RelativeTC_SET();
int Get_Hdr_Loc(__int64 *, int *);
void FrameRate2FramePeriod();

/*__forceinline static */ void gothdr_GOP(void);
/*__forceinline static */ void gothdr_PICTURE(void);

__int64 Calc_Loc(int *Calc_File, int Calc_Offset, int P_BitMode);


/* getpic.c */

void Pic_DECODE(void);

// getbit.c
void GetBlk_RdAHD_RESET();

//int Packet_Length, Packet_Header_Length, getAudio_size;
unsigned int getbit_AUDIO_ID;
int getbit_VOBCELL_Count;
int /*unsigned short*/ getbit_VOB_ID, getbit_CELL_ID;

//unsigned int getbit_AC3_Track, getbit_MPA_Track;
//void Get_Next_Packet_Start();
void getBLOCK_Packet(int);

int Mpeg_READ_Buff(int P_FileNum, unsigned char *P_Buffer, int P_iRqstLen, int P_Caller);


void  Mpeg1_PesHdr();


int File_Ctr;
int File_Limit, File_Final, File_New_From;
int File_UnMux_Limit, File_PreLoad;
int File_ReadLen;

int Mpeg_PES_Version, Mpeg_SEQ_Version;
int getbit_PES_HdrFld_Flags;


unsigned int getbit_input_code;



unsigned char getbit_Unk_Stream_Id;
static unsigned int CandidatePTS;
void PTS_Err_Msg(unsigned P_Prev_IX, unsigned P_Curr_PTS, char P_Type[16]);
unsigned Get_PTS(int P_Char1);

unsigned int getbit_StreamID, getbit_SubStreamID;

int getbit_PES_Gate, iPS_Frame_Offset;
int getbit_iPkt_Len_Remain;
int getbit_iPkt_Hdr_Len, getbit_iPkt_Hdr_Len_Remaining;
int getbit_iDropPkt_Flag, Mpeg_PES_Byte1;

unsigned int Prev_PTS_IX;
unsigned Prev_PTS[25];
char     PTS_Flag[4];


// GetAudio.c
void GetDelay();
void Packet_Aud_Inc();
void Got_PrivateStream();
void Got_MPA_Pkt();
void PTS_Audio_Analysis();
void GetChkPoint();


// STATS_WIN.c

void S120_Stats_Hdr_Seq();
void S100_Stats_Hdr_Main(int), S200_Stats_Pic_Main(int);
void S300_Stats_Audio_Desc();
void S333_Trk_Audio_Desc(HANDLE hDial, unsigned int *TXT_FLD);
void S370_AudioTrackDesc(int Track_Format, int Int_Track, int Ext_Track);

void Stats_FPS();



struct{
   int iFrameLen, iFrameOK;
   char desc[13];
   int iAudio_Interleave;
   int iVid_PTS_Resolution;
  char VobTxt[24];
} StatsPrev;



/* gui.cpp */

HANDLE hMain_GUI;
DWORD MPEG_processKick(); //HANDLE) ;
void Mpeg_KILL(int); // void);

void B555_Normal_Speed(int);

void Set_Priority(HANDLE, const int, const int, const int);
void Set_Zoom(int), Set_Zoom_Menu(int);
void Set_Aspect_Mode(int);

void Set_DropDefault(int), Set_SortDefault(int);
void Set_Folder(char *, int *, int, const int, const int, const char*);
void Set_Time_Fmt(const int);

void Set_Bmp_AutoPreview(const int P_NewSetting);
void Set_Bmp_Fmt(int);
void Out_SetBufSz(int);



void ClearMPALib(int);
void ProcessReset(char[3]);

struct _finddata_t vfpfile, seqfile;


// TRACKBAR.c

void T100_Upd_Posn_Info(int);
void T110_Upd_Posn_TrackBar();
void T580_Trackbar_CLIP();
void T590_Trackbar_SEL();
void T599_Trackbar_END();
void Tick_Ctl(int, int, __int64);
void Tick_CLEAR();
void T600_Msg_HSCROLL(UINT); //DWORD, DWORD);

// SNAPS.C
                  //int iBMPwidth, int iBMPheight );
#define BMP_ASPECT_BICUBIC 0
#define BMP_ASPECT_SUBSAMPLE 1
#define BMP_ASPECT_RAW 2

int iBMP_Folder_Active;


// PLUG.c

void P9_EXT_ACT(int P_EXT_ACT_NO);




// Separate Structure for building the next decoder request
// to avoid conflict with current decode in progress
struct
{
   int Action ;
   int File;
   __int64 Loc ;
} iKick ;



// CLIPS.c
void C000_Clip_TOTAL_MB(char);
void C100_Clip_DEFAULT(char);
void C140_Clip_EOF();
void C160_Clip_Preview();
void C550_Clip2Selection();

void C310_Pos_MEMO(int);
void C320_Sel2Clip();
void C323_Clip2Clip(int P_From, int P_To);
void C333_Clip_Clear();
void C350_Clip_ADD(char, int);
void C400_Clip_DEL(), C450_Clip_DEL_ALL();
void C500_Clip_UNDO();
void C510_Sel_FROM_MARK(int), C520_Sel_TO_MARK();
void B570_GO_FROM(), B580_GO_TO();

// ClipFile.c
void C600_Clip_Split();
void C700_Clip_DeReference(int);
int  C800_Clip_FILE(int, int, char);
void C888_AutoFile(int P_SysTemp);


// Edit Decision List
struct
{
  int       FromFile[204];
  //__int64   FromBlk [204];
  __int64   FromLoc [204];
  unsigned  FromPTS [204],  FromPTSM[204]; //, FromSSCR[204];
  unsigned int uFrom_TCorrection[204], uFrom_FPeriod_ps[204];

  int       ToPadFile[204];
  //__int64   ToPadBlk [204];
  __int64   ToPadLoc [204];
  unsigned  ToPadPTS [204], ToPadPTSM[204]; //, ToPadSSCR[204];

  int       ToViewFile[204];
  //__int64   ToViewBlk [204];
  __int64   ToViewLoc [204];
  unsigned  ToViewPTS [204], ToViewPTSM[204]; //, ToViewSSCR[204];


  unsigned  uClip_MB[204], uClip_Secs[204];

} EDList;


int  iEDL_ctr;           // Pointer to next spot in EDL table
int  iEDL_Chg_Flag, iEDL_Reload_Flag;


char Ed_Prev_Act, Ed_Prev_Act2;       // Remember last action for possible undo
int  Ed_Prev_EDL_Ctr;
int  iEDL_TotMB, iEDL_Start_Type;
unsigned uEDL_TotTime;
int  iEDL_ClipFrom, iEDL_ClipTo, iEDL_OutClips;

     // Need some info about current selection
int iSelMB, iClipMB, iInputTotMB; //, wCmdShow ;

char szSelTxt[80];
//char cSelStatus;
        /* struc SelStat
          {
          char chMB[12]
          char chPos[12]
        } */

// Parm2Clip.c

DWORD WINAPI  C900_Parm2Clip(LPVOID n);


//-------------------------------------------


// DDRAW_ctl.c
void D100_CHECK_Overlay(void);
void D200_UPD_Overlay();
void D300_FREE_Overlay();
void Chg2RGB24(int, HWND), Chg2YUV2(int, HWND), DD_OverlayMask(int);
void VGA_GetSize(), Calc_PhysView_Size();
void DD_PtrLost_Box(const char *P_PtrDesc);
void Flag2RGB();
void Set_OVL_Notify(int P_NewSetting);

HBRUSH hBrush_MASK, hBrush_MSG_BG;

DWORD WindowsVersion;  // OLD FORMAT stupid Version number from GetVersion()

OSVERSIONINFO winVer;  // NEW FORMAT sensible Version number from GetVersionEx(&winVer)

RECT wrect, crect, orect, prect, srect;
int DDraw_Surface_Size, DDraw_Canvas_Size, DDraw_lPitch, DDraw_dwWidth;
int iVistaOVL_mod, iCtl_VistaOVL_mod;



// DISP_WIN.cpp
void DSP1_Main_MSG(const int, const int),      DSP5_Main_FILE_INFO();
void DSP2_Main_SEL_INFO(int), DSP3_Main_TIME_INFO();
void DSP_Msg_Clear();
//void DSP_Blank_Msg_Clean();
void DSP_Button_Abbr();
void DSP_ButtonFont_Sizing();
void MainWin_Rect();

HFONT hFont1
#ifdef GLOBAL
 = 0
#endif
;
HFONT hFont2
#ifdef GLOBAL
 = 0
#endif
;
HGDIOBJ hDefaultGuiFont
#ifdef GLOBAL
 = 0
#endif
;

void BmpButton_Create();
void AddButton_Create();
void MarkLeftButton_Create();
void MarkRightButton_Create();

unsigned int uFontHeight; //, uDGF_Height;
int iTool_Ht, iTool_Wd, iTrackBar_PosY, iPlayBar_PosY, iSkipBar_PosY;
int iToolbarWidth, iTrack_Wd;
int iTopMargin;
int iTimeX, iTimeY, iMsgPosY, iSelMsgX;
int Client_Width, Client_Height;
int iCentre_Cropped;  // iCenter_Cropped
int iConverge_Red_H,  iConverge_Red_V;
int iConverge_Blue_H, iConverge_Blue_V;

void Menu_Main_Disable(int, int);
void View_MOUSE_ALIGN(LPARAM lParam), View_MOUSE_CHK(LPARAM lParam);
void ToolBar_Metrics();
void ToolBar_Create();
void ToolBar_Destroy();
void Toolbar_Chg();

void View_Ctr_Crop();
void View_Rebuild_Chk(int);
void D500_ResizeMainWindow(int width, int height, int P_Full);
void D501_RESET_MainWindow();

void RefreshVideoFrame(void);
//void Menu_Main_Enable();
//void Enable_Disable(int, int, int);
void Relative_TOD();

struct
{
  int hour;
  int minute;
  int sec;
  int frameNum;
  int RunFrameNum;
} ShowTC ;

char ShowTC_AM_PM[4];


// LUM_WIN.c

HWND hLumDlg;
int iLumEnable_Flag[2];  // WAS Luminance_Flag
int iCtl_Lum_Deselector, iLum_Deselected; // Option to remove boosting when outside selection
int iLumLock_Flag, iSatLock_Flag;          // Slider Lock RJ
int iSat_VHS, iSat_Sine;
int iSatAdj_Flag, iSatAdd_U[2], iSatAdd_V[2], iSatGain[2]; // Color UV-Hue Saturation
int iCtl_SAT_Retain;
int iBMP_Wanted;

// iLum [0]=RecentYUV; [1]=RecentRGB; [2]=[D]; [3]=[B]; [4]=[C]; [5]=[A];
int iLumOffset[6], iLumGain[6], iLumGamma[6];
int iColorSpaceTab;

LRESULT CALLBACK Luminance_Dialog(HWND, UINT, WPARAM, LPARAM);
HCURSOR hCSR_CROSS;

void Lum_Swap_UV(const int P_Reshow);
void Lum_Negative(const int P_Reshow);
void Lum_Bold();
void Lum_C();
void Lum_Default();
//void Lum_BC_Adj(int P_Adj);

//unsigned char GammaMask[256];


//DBG.c

FILE *DBGfile;

int  DBGflag,iDBGsuffix, iAudioDBG;
int bDBGStr;  // For DbgView
void DBGln2(char[256], __int64, __int64) ;
//void DBGln3(char[80], __int64, __int64, __int64) ;
void DBGln4(char[256], __int64, __int64, __int64, __int64) ;
void DBGln4a(char[256], void*, void*, void*, void*) ;
int DBGout(char P_Rec[256]);
//void ERRMSG_FileXA(char[32], char, __int64, char[_MAX_PATH], int, int) ;
int DBGctr, DBG_Alert_ctr;
void DBGctl();
void DBGAud(void *P_Text);

void Msg_LastError(char[32], int, char);
void ERRMSG_File(char[32], char, int, char[_MAX_PATH], int, int) ;
void Err_Malloc(void *);

// IN_FILES.c

int  F100_IN_OPEN(char, int) ;
void F150_File_Begin(char);

void F300_DropFilesProc(WPARAM) ;
int  F500_IN_OPEN_TRY(char);
int  F503_Dup_Name_TST(char *P_Name, const char *P_Msg);
int  F505_IN_OPEN_TST(char cP_Act);
void F590_ReOpenAllFiles(char);
int  F591_Ask_Trojan(const int, const void *P_Desc,
                    int *P_Ctl, const unsigned int P_MenuId);
int  F595_NotMpeg2_Msg(int), F594_TS_Warn_Msg();
void F600_NewName_Setup();
int  F690_FileName_ChkChars(unsigned char *lpNameChr);
void F920_Init_Names();
int  F999_Del_or_Recycle(char *); //LPCTSTR);

int iFileToRename;
int iFileListChg_Flag;

void F800_SORT_ALL(int), F850_SORT_NAMES(int);
void F560_RemoveOtherFiles(int P_Keep);
void F570_RemoveFile(int, int);
void F900_Close_Release(char);
void F950_Close_Files(char);
void MultiFile_SizeCalc();

LRESULT CALLBACK F700_Video_List(HWND, UINT, WPARAM, LPARAM);
char VideoList_MODE, VideoList_Title[20];

void FileDate2Gregorian(IFILEDATE*, struct tm*,  void*,  void*);
void RJ_Date2Gregorian(int*,        struct tm*,  void*,  void*);


int iDeEdit(char *, int);
int iParmConfirm, iSuppressWarnings;

HWND hNewnameDlg;

// PopFileDlg
int X800_PopFileDlg(PTSTR, HWND, int, int, char *);
#define SAVE_AVI    -1
#define SAVE_D2V    -2
#define SAVE_WAV    -3
#define SAVE_VOB    -7
#define SAVE_BMP    -8
#define SAVE_EDL    -9
#define SAVE_CHAP   -10
#define OPEN_D2V    4
#define INPUT_VOB   5
#define OPEN_WAV    6
#define LOAD_EDL    9
#define LOAD_CHAP  10

char *stpcpy0    (char *P_Dest, char *P_Src);
char *stpcpy1    (char *P_Dest, char *P_Src);
unsigned char *stpToken   (unsigned char *P_Dest, unsigned char *P_Src, int);
void FileNameTidy(char *P_Dest, char *P_Src);



// INI.c
void INI_VARS_BeforeMenu(), INI_GET(), INI_MERGE();
void INI_SAVE();
void Ini_Associate();
void MenuTick(UINT uItem), MenuUnTick(UINT uItem);
void MenuTickCtl(UINT uItem, UINT uStatus);

int Warning_Box(char *lpP_Text, char *lpP_Title,
                int  *lpP_WarnCtl, unsigned int lpP_Warn_ID,
                unsigned int uMode);


// INI_REG.c
void RegGet_External_Path(int P_EXT_ACT_NO);
char szExtAct_PathType[7];
char szExtAct_Path[_MAX_PATH];
/*
char szMediaPlayerClassic[_MAX_PATH];
char szVLC[_MAX_PATH]; // VideoLanC
char szWinMediaPlayer2[_MAX_PATH];
char szWinMediaPlayer[_MAX_PATH];
char szCreativePlayCtr[_MAX_PATH];
*/


void Stats_Show(int, int), Stats_Kill();
void Stats_Volume_Boost();

void Set_AudioTrack(int);
void Set_XTN_PS(char [8]), Set_XTN_AUD(char [8]);
void Set_ADD(int);
//void Set_ALIGN_VERT(int, int), Set_ALIGN_HORIZ(int, int);
void Set_Preamble_Mode(int P_Mode);
void ToggleMenu(char, void*, int);
void Set_Parse_Ticks();
void Set_Wheel_Scroll(int);

#define PRIORITY_QUICK   0
#define PRIORITY_HIGH    1
#define PRIORITY_NORMAL  2
#define PRIORITY_LOW     3

int Add_Automation;
int iCtl_EDL_AutoSave, iCtl_RecycleBin;
int iCtl_Track_Memo,  iCtl_Name_Info, iCtl_FileSortSeq;
int iCtl_Priority[3];
int iCtl_BasicName_Panel, iCtl_ColumnWidth[6];
int iCtl_Trackbar_Big, iTrackbar_Big;

int iCtl_File_WIN32
#ifdef GLOBAL
 = 0
#endif
;

int iCtl_VOB_Style, iCtl_Out_Parse, iCtl_Out_Seq_End;
int iCtl_Out_Fix_SD_Hdr, iCtl_Out_SysHdr_Mpeg;
int iCtl_Out_SysHdr_Unlock, iCtl_Out_SysHdr_EveryClip;
int iCtl_Out_Demux, iCtl_Out_PTS_Match;
int iCtl_Out_TC_Adjust, iCtl_Out_TC_Force, iCtl_Out_PTS_Invent;
int iCtl_Out_Fix_Errors;
int iCtl_Out_Force_Interlace;
int iCtl_Out_KillPadding, iCtl_Out_DropCrud;
int iCtl_Out_Parse_Extras, iCtl_Out_Parse_Deep, iCtl_Out_Parse_AllPkts;
int iCtl_Out_Parse_SplitStart, iCtl_Out_SplitSegments;
int iCtl_SetBrokenGop;
int iCtl_Out_Align_Video;
int iCtl_Out_Align_Audio;
//int iCtl_Out_Keep_Ac3Hdr;
int iCtl_Out_Preamble_Flag, iCtl_Out_Preamble_VTS;
int iCtl_Out_DeBlank, iCtl_Out_MixedCase, iCtl_OutPartAuto;
int iCtl_Out_PostProc, iCtl_Out_PostQuote, iCtl_Out_PostShow;
int iCtl_OutFolder_Active, iCtl_BMP_Folder_Active;
int iCtl_OutFolder_Both;
int iCtl_Out_KeepFileDate;
char szCtl_OutFolder[_MAX_PATH];
char szCtl_BMP_Folder[_MAX_PATH];
char szCtl_Out_ProcLine_A[_MAX_PATH*2];
char szCtl_Out_ProcLine_B[_MAX_PATH*2];

int iCtl_Drop_Behind, iCtl_Drop_PTS;
int iCtl_Play_AudLock, iCtl_Play_Sync, iCtl_Play_Summary, Err_Analysis;
int iCtl_Byte_Sync, iCtl_DropAction;

int iCtl_WarnSize_1, iCtl_WarnSize_2, iCtl_WarnSize_3, iCtl_WarnSize_4;
int iCtl_WarnMpeg1, iCtl_WarnTS, iCtl_WarnTSmpg, iCtl_WarnCDXA, iCtl_WarnETC;
int iCtl_WarnBadStart, iCtl_WarnNoHdr, iCtl_WarnBadSysHdr;
int iCtl_WarnDone;

int iCtl_ParmClipSpec, iCtl_ParmConfirm;
int iCtl_Time_Fmt;

int iCtl_Out_Breathe_PerBigBlk[3];
int iCtl_Out_Breathe_PktCtr[3], iCtl_Out_Breathe_PktLim[3];


unsigned int uCtl_Video_Stream, uGot_Video_Stream;
unsigned int uCtl_Vid_PID, uCtl_Aud_PID, uGot_PID;
unsigned int uVid_PID, uPrev_PID, uVid_PID_All; // , uGot_PID_Stream;
unsigned int uAud_PID_All;
unsigned int uCtl_Aud_Stream;
unsigned uPID_Map[16], uPID_Map_Used;
int iCtl_MultiAngle, iWant_VOB_ID;

char uGot_Pkt_Type;


int Mpeg_Version_Alerted;
int Mpeg_Version_Alerts_Session;
void MPEG_File_Reset();

void SwitchIDCT();

/* idct */

extern void __fastcall MMX_IDCT(short *block);
extern void __fastcall SSEMMX_IDCT(short *block);
extern /*"C"*/  void __fastcall IDCT_CONST_PREFETCH(void);

//extern "C" void memcopy_init();
//extern "C" void memcopy(void *dest, void *src, int n);

void Initialize_FPU_IDCT(void);
void __fastcall FPU_IDCT(short *block);
void Initialize_REF_IDCT(void);
void __fastcall REF_IDCT(short *block);
extern /*"C"*/ void __fastcall  SSE2MMX_IDCT (short *block);


/* mpeg2dec.c */

void  GOPBack(void)  ;
__int64    i64PreCalc_Loc, i64PreCalc_Key_Loc;
  int      iPreCalc_File, iPreCalc_Key_File;

DWORD WINAPI MPEG2Dec(LPVOID n);
void Decoder_INIT(void);
void Mpeg_Drop_Init();
void Mpeg_PreAmble_Alert(int);
void Mpeg_EOF();

void PicBuffer_Alloc();
void PicBuffer_Free();

void  Mpeg_Stop_Rqst(), Mpeg_MaybeStop_Rqst();


/* norm.c */
void Normalize(FILE *WaveIn, int WaveInPos, char *filename, FILE *WaveOut, int WaveOutPos, int size);


// Render.c

void Render_Init();
void Lum_Filter_OVL(unsigned char *src, unsigned char *dst);
void Lum_Filter_RGB(unsigned char *src, unsigned char *dst);
__forceinline void R250_SIGNAL_Overlay();


/* store.c */
void Write_Frame(unsigned char *src[], D2VData d2v, DWORD frame);
void Cnv_422_yuy2_FLD(int, unsigned char *py, unsigned char *pu,
                       unsigned char *pv, unsigned char *dst);
//__forceinline static
void Store_YUY2a(unsigned char *src[]);// , DWORD frame);
void RenderYUY2(int),  RenderF420(int);
void RenderRGB24(void);
int DetectVideoType(int frame, int rff);



// Timing.c

int iCURR_TIME_ms();
int Store_Timing_Chk(int P_Overlay);
int iSync_Diff_ms;

void Timing_DropMore();

int Mpeg_FrameRate_INT;
double fFrame_rate_MPEG;

double frame_rate_Table[18]
#ifdef GLOBAL
=
{
  18.0,     // Allow for illegal value of zero
  ((24.0*1000.0)/1001.0),    24.0,  25.0,
  ((30.0*1000.0)/1001.0),    30.0,  50.0,  ((60.0*1000.0)/1001.0),
  60.0,
  // rest are "reserved",
  // but turn up in non-std files.  Arbitrary values.  see also K_FrameRateCode
  12.0,    16.0,  18.0,  20.0,
   6.0,     2.0,   1.0,
  ((25.0*44.1)/48.0), ((30.0*1000.0*44.1)/1001.0/48.0) // For 48k=>44.1k adjust
}
#endif
;

