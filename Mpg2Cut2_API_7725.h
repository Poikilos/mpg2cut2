
// Mpg2Cut2 API 

#ifndef inc_mpgapi
#define inc_mpgapi

#define RJPM_BASE           WM_USER+50

#define RJPM_GET_APIVER     RJPM_BASE
#define RJPM_GET_APPVER     RJPM_BASE+1
#define RJPM_GET_APPDATE    RJPM_BASE+3
#define RJPM_GET_FILELIMIT  RJPM_BASE+4
#define RJPM_GET_EDLLIST    RJPM_BASE+5


#define RJPM_LOADFILE       RJPM_BASE+6
#define RJPM_APPENDFILE     RJPM_BASE+7
#define RJPM_REFRESHFRAME   RJPM_BASE+8

#define RJPM_B200_USERMSG   RJPM_BASE+9

#define RJPM_CLOSEFILE      RJPM_BASE+10
#define RJPM_CLOSENAMEDFILE RJPM_BASE+11
#define RJPM_GET_FILENAME   RJPM_BASE+12

#define RJPM_GET_CURFILE    RJPM_BASE+13  // v20503
#define RJPM_SET_CURFILE    RJPM_BASE+14
#define RJPM_GET_TIMECODES  RJPM_BASE+15
#define RJPM_GET_CALLBACKS  RJPM_BASE+16

#define RJPM_REMOVEFILE     RJPM_BASE+17
#define RJPM_REOPENALLFILES RJPM_BASE+18


#define RJPM_THUMB2CLIPBOARD RJPM_BASE+21
#define RJPM_THUMB2MEM       RJPM_BASE+22
#define RJPM_BMP_DEINT       RJPM_BASE+23
#define RJPM_BMP_ASIS        RJPM_BASE+24
#define RJPM_BMP_THUMB       RJPM_BASE+25
#define RJPM_BMP_CLIPBOARD   RJPM_BASE+26

#define RJPM_DISPLAY_SET    RJPM_BASE+31
#define RJPM_SET_RUNLOC     RJPM_BASE+32
#define RJPM_GET_POPUPS     RJPM_BASE+33
#define RJPM_SET_POPUPS     RJPM_BASE+34

#define RJPM_UPD_TRACKBAR   RJPM_BASE+41
#define RJPM_UPD_MAIN_INFO  RJPM_BASE+42

// USER COMMANDS
#define RJPC_FROM_MARK                    1
#define RJPC_LEFT_ARROW                   2
#define RJPC_RIGHT_ARROW                  3
#define RJPC_TO_MARK                      4
#define RJPC_TRACKBAR                     5
#define RJPC_ADD                          100
#define RJPC_DEL                          101
#define RJPC_TO_SHIFT                     32667
#define RJPC_RIGHT_SHIFT                  32848
#define RJPC_DOWN_SHIFT                   32849
#define RJPC_HOME                         32850
#define RJPC_UP_SHIFT                     32852
#define RJPC_LEFT_SHIFT                   32853
#define RJPC_FROM_SHIFT                   32866

#define RJPC_FWD_FRAME                   32671

#define RJPC_FILE_CLOSE                  32947
#define RJPC_FILE_CLOSE_CURR             33136
#define RJPC_FILE_CLOSE_OTHERS           33137



#define RJPC_AUTO                         -1


// THUMB2MEM
typedef struct {
  int cbSize;


  int nImageWidth;
  int nImageHeight;
  int iBmpAspect; // Future use
  int iLum;


  BYTE MarginColour; // COLOREF?
  WORD reserved1;
  BYTE reserved2;


  DWORD  dwRGB24Size;
  LPBYTE lpRGB24;
} ThumbRec;



typedef struct {
  int hour;
  int minute;
  int sec;
  int frameNum;
  int RunFrameNum;
} TimeCodeRec;


typedef struct {
        int iView_TC_Format;
        TimeCodeRec Relative;
        TimeCodeRec GOP;
        TimeCodeRec PTS;
        TimeCodeRec Curr;
        TimeCodeRec TOD;
        TimeCodeRec SCR;
} TimeCodesRec;


typedef struct {
        HANDLE hMPEGKill;
} tagCallbacks;


#endif


tagCallbacks mycallbacks;


void SNAP_Save(UINT, ThumbRec* lpThumbRec);
int  SNAP_Buffer_Alloc(int BMPHdr_Size);
void SNAP_Resample_RGB(int P_Bmp_Aspect,  // SubSample or Bicubic
                  unsigned char *lpBMP_Frame);
