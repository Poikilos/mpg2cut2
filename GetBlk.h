

int iCtl_AudioAhead;

#define RD_AHD_MAX 32

unsigned char *RdAHDBFR[RD_AHD_MAX], *lpRdAHD_EOB[RD_AHD_MAX], *RdAHD;
unsigned char *RdAHD_malloc; 
int  RdAHD_Flag, RdAHD_EOF;
int iRdAHD_len, iRdAHD_File; 
int iRdAHD_NextIx, iRdAHD_CurrIx;

    int   iRdAHD_DataLen[RD_AHD_MAX];
    int   iRdAHD_TellBefore_File[RD_AHD_MAX];
    int   iRdAHD_TellAfter_File[RD_AHD_MAX];
__int64 i64RdAHD_TellBefore_Loc [RD_AHD_MAX];
__int64 i64RdAHD_TellAfter_Loc [RD_AHD_MAX];


void GetBlk_AHD_INIT();
