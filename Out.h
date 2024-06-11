
// Fields shared by the various Mpeg Output routines

unsigned char *lpMpeg_PKT_Anchor;
unsigned char *lpMpeg_ix4;  //, *lpMpeg_Flush_ix;
unsigned char *lpMpeg_ix3, *lpMpeg_ix5;
unsigned char *lpMpeg_PES_BeginData, *lpMpeg_End_Packet;
unsigned char *lpMpeg_SPLIT_ix;
unsigned char *lpMpeg_FROM, *lpMpeg_EOD, *lpMpeg_EOI;
#define CPY_EOI_CUSHION 8192

int iPkt_Between_Len; // Number of bytes BETWEEN end of one pkt stat code and the start of the next
unsigned int  uPkt_Hdr_Full_Len, uPES_Hdr_Len;
char cMpeg_Out_Pkt_ACT;
char cBoringStreamDefaultAct;

int iOut_UnMuxAudioOnly, iOut_UnMux_Fmt;
int iOut_Audio_All, iOut_Audio_TrkSel[9];
unsigned char cOut_SubStreamWanted[256];

int iOut_Error, iOut_Force_Interlace_ctr;
int iPack_Ctr, iOut_CheckedPackets, iOutPaddingPkts, iOutPaddingBytes;
int iOut_Parse_AllPkts, iOut_Parse_Deep;
int iDeepLook, iDeepFound, iDeepNow, iOut_CannotFix_ctr;
int iScanResult;
unsigned int uHdrType;
int iPreambleOnly_Flag;
int iOut_Fix_Aspect, iOut_Fix_Frame_Rate;
int iOutFrameRateCode, iOutFRatioCode;

unsigned int uPTS_Accounted[256+32];

// PTSM is NOT really integer - SPOOFING FOR PEFORMANCE
//unsigned   int   uPrimaryTrigger_PTSM, uPrimaryTrigger_PTSM_minor;
         __int64 i64PrimaryTrigger_PTS, i64PrimaryTrigger_PTSM;
//unsigned   int   uVidFromTrigger_PTSM, uVidFromTrigger_PTSM_minor;
         __int64 i64VidFromTrigger_PTS, i64VidFromTrigger_PTSM;
 
     __int64 i64Curr_PTS, i64Prev_PTS;
unsigned __int64 i64Curr_PTSM; //, i64CurrPTSM_minor;
unsigned __int64 i64Prev_PTSM[256+32];
//unsigned int uPrevVideoPTSM, uPrevVideoPTSM_minor;

void PTSM_2PTS(__int64 *P_PTS);
void PTS_2PTSM(unsigned __int64 *,  __int64 *, unsigned char);


int iOut_Target_Tail;
int iOut_PTS_Matching; //, iOut_Parse_Extras;


// Progress Dialog and controls
void Out_Priority_Chg(const int);
void Out_Progress_Chk(int);

HWND hProgress, hBar, hBar2;
HWND hETA, hPtxt, hCancel, hPause;

int iOut_Breathe_Tot;
int iOut_Breathe_PerBigBlk;
int iOut_Breathe_PktCtr, iOut_Breathe_PktLim;
int iMsgTime;


int  Out_RECORD(const void*, const int, const int);

int iOut_Rate, iFixedRate;

DWORD dwStartSentinel;  // Pack or Seq Start sentinel
DWORD dwEndSentinel;    // Pack or Seq or Pic sentinel
//BYTE byInpVidOnly ;    //  Video Elementary Stream flag

int  iPS_Block1; // 2=First Block of First Clip, 1=First Block of Later clip, 0=OFF
int iOverflow_Len;

char iOut_Parse;
char szHdrAbbr[16];
char szFTarget[8];

unsigned char cStream_Id, cDummy1, cDummy2, cDummy4;
unsigned int  uSubStream_Id;
unsigned char cPut_Stream_Id, cStream_Cat;
unsigned  int uSubStream_Cat;// , cFldFlags1;
#define SUB_MPA 256

char cStreamNeedsAligning_FRONT_Flag[256+32];
char cStreamNeedsAligning_REAR_Flag[256+32];
int  iStartCodePart[256+32];

unsigned char cPES_Field_Flags;

int  iOut_SplitVideo_PrePackets, iOut_SplitVideo_PostPackets;
int  iOut_SplitAudio_PrePackets, iOut_SplitAudio_PostPackets;

void Out_Filter_Split_Front(int *);
void Out_Filter_Split_Rear(int *);

void Out_SplitChk_FRONT_Audio();
void Out_Split_Audio_Rear(); 
void Out_Split_Find_Any_Audio_Syncword();

void Out_Fix_Hdr_Vid_SEQ();
void Out_Fix_Hdr_Vid_EXTN_SEQ();
void Out_Vid_Hdr_SCAN(int P_Repeat);
void Out_Fix_Hdr_PIC_EXTN_SEQ();

void  Out_COMMIT_PKT(int P_Full, const int P_Caller);


// Time Stamp Correction 
unsigned int uBroken_Flag;
int iGOP_PTS_Chk, iGOPFixed_Flag, iOut_TC_Adjust;
int iOut_Invent_Needed, iOut_Invent_Done;
int iGOP_Memo[3], iGOP_Memo_ix;
int iPES_Mpeg2;

// unsigned char *lpMPA_C0_save, *lpAC3_80_save;

#define K_PKT_SAVE_BUF_LEN 8192

unsigned char *lpMpeg_PTS_ix, *lpMpeg_DTS_ix, *lpMpeg_TC_ix;
      __int64 i64Adjust_TC[256+32][4]; // #0=Previous #1=AdjAmt 
                              // #2=GOP_Curr #3=GOP_Prev

void Out_DeGap_TC(); //__int64 *lpPrev_TC);


void Out_TC_Rewind(       __int64 *lpP_PTSM, 
                          __int64 *lpP_PTS, 
                   unsigned int      P_uAdj);

LRESULT CALLBACK Out_Part_Dialog(HWND, UINT, WPARAM, LPARAM);
void Out_Progress_Title();

//void TRKS_Category_Set(HWND hDialog, int iCat, int iFmt);


