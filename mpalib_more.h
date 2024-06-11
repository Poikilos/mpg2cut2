
// MPALIB (Mpglib) INTERFACE

typedef struct 
{
  MLVERSION mlVersion;
  MLINIT    mlInit;
  MLEXIT    mlExit;
  MLDECODE  mlDecode;

  ML_VERSION Version;

  HINSTANCE hDLL;

  struct mpstr mp;
  int nRet;
  int nSize;
  DWORD dwFree;

  BYTE *byInBuffer;
  WORD wFrameSize;

} MPGStruct;

MPGStruct MPAdec;

char szMPAdec_NAME[13]; 
BYTE byMPALib_OK;

int iMPALib_Status
#ifdef GLOBAL
 = 0   // Set decoder status to UNKNOWN
#endif
;


int iMPA_Best
#ifdef GLOBAL
 = 0
#endif
;

int iMpa_AUTO
#ifdef GLOBAL
 = 1
#endif
;

char szAudio_Status[16] 
#ifdef GLOBAL
 = "-"
#endif
;

