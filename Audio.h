


// AUDIO Handling

#define AUDIO_DEMUXALL    0
#define AUDIO_DEMUXONE    1
#define AUDIO_DECODE    2

#define CHANNELS_MAX      8
#define TRACK_AUTO    254
#define TRACK_NONE    255
#define TRACK_1     0
#define TRACK_2     1
#define TRACK_3     2
#define TRACK_4     3
#define TRACK_5     4
#define TRACK_6     5
#define TRACK_7     6
#define TRACK_8     7



#define Audio_Dunno 0xABCD;
//int rj_Audio_Code; // = Audio_Dunno;

#define K_BOOST_DENOM  16        // Granularity of volume settings
#define K_VOL_CEILING_DEF  8191  // Default Volume Ceiling when activated. // 8191 = quarter of max (max=32767)
#define K_BOOST_SILLY (K_BOOST_DENOM*64)
#define K_BOOST_TOO_SILLY (K_BOOST_SILLY*6/5)


char *FORMAT_ABBR[]
#ifdef GLOBAL
=
{ &"",     &"MPa",  &"PCM",  &"DD", &"DTS", &"DD+", 
  &"PS1",  &"PS2",  &"SUB",  &"UNK"
}
#endif
;

int iWant_Aud_Format;
#define FORMAT_AUTO     0
#define FORMAT_MPA      1
#define FORMAT_LPCM     2
#define FORMAT_AC3      3
#define FORMAT_DTS      4
#define FORMAT_DDPLUS   5
#define FORMAT_DDP      5
//#define FORMAT_PS1      7
//#define FORMAT_PS2      8
#define FORMAT_SUBTIT   9
#define FORMAT_UNK     10


unsigned int iAudio_SEL_Track, iAudio_SEL_Format, iAC3_Attr;
int iAudio_Trk_FMT[CHANNELS_MAX+1];   // Maps track number into a format
unsigned char  cAudio_Track_Stream[CHANNELS_MAX+1]; // Maps track number into a Stream_id 
unsigned short uAudio_Track_PID[CHANNELS_MAX+1]; // Maps track number into a Stream_id 

int iCtl_Audio_PS2, iCtl_Audio_CRC; 
int iCtl_Volume_Boost, iCtl_Volume_Boost_Flags[9], iCtl_Volume_Boost_Cat;
; 
int iCtl_Volume_Boost_MPA_48k, iCtl_Volume_Boost_MPA_other;
int iCtl_Volume_Boost_AC3, iCtl_Volume_Boost_LPCM;
int iCtl_Volume_AUTO, iCtl_Volume_SlowAttack;
int iCtl_Volume_Limiting, iCtl_Volume_Ceiling;
int iCtl_Vol_StarKey;
int iCtl_Audio_InitBreathe;


int iPlayAudio, iWAV_Init, iMPAdec_Init, iCtl_AudioDecoder;
int iWantAudio;
int iInPS2_Audio;
int iVolume_Boost, iVolume_AUTO, iVolume_UnBoost_Recent;
int iVolume_Ceiling;

int iAudio_Lock;  
int iAudio_Force44K, iCtl_PALTelecide;
int iSlider_Skip;

void PS1_Convert();
void Got_MPA_PayLoad();

HWND hVolDlg
#ifdef GLOBAL
 = NULL
#endif
;
HWND hVolDlg0
#ifdef GLOBAL
 = NULL
#endif
;


LRESULT CALLBACK Volume_Dialog(HWND hDialog, UINT message,
                               WPARAM wParam, LPARAM lParam);

void Volume_Init();
void VOL203_Volume_Target();
void VOL204_Volume_Mute(), VOL206_Volume_UN_Mute(), VOL210_MUTE_Toggle();
void VOL300_Volume_Boost();
void VOL301_Volume_Boost_Start(), VOL302_Maybe_Reset();
void VOL303_Vol_Boost_On(), VOL304_Vol_Boost_Off();
void VOL320_Down(), VOL340_Up();
void VOL337_Volume_Bolder();
void Vol_Show_All(), Vol_Show_Chks();


 
int AC3_Flag;
int MPA_Flag;
//int Decision_Flag;
int SRC_Flag;
int Normalization_Flag;
int Norm_Ratio;
short Sound_Max;



typedef struct {
//  FILE          *file;
  int            rip;
  int            delay;
  unsigned       uLayer;
  unsigned       uMPA_Sample_Hz;
  unsigned short uPID;
  unsigned char  cStream;
  char           desc[33];

} MPAStream;
MPAStream mpa_Ctl[CHANNELS_MAX+2];


typedef struct {
//  FILE          *file;
  int           rip;
  unsigned int  uChannel_ix, uBitRate_ix, uSampleRate_ix;
  unsigned int               uBitRate,    uSampleRate;
  unsigned short uPID;
  unsigned char cStream, uFiller;
} AC3Stream;
AC3Stream SubStream_CTL[6][CHANNELS_MAX+2];



struct WAVStream {
  int           rip;
  int           size;
  int           delay;
} wav;




//--------------------------------------
unsigned int uAC3_SampleRate[4]
#ifdef GLOBAL
=
{ 48000, 44100, 32000, 96000
}
#endif
;



int dca_sample_rates[] 
#ifdef GLOBAL
=
{
    0,      8000, 16000,  32000, 
    0,         0, 11025,  22050, 
    44100,     0,     0,  12000, 
    24000, 48000, 96000, 192000
}
#endif
;

int dca_bit_KRates[] 
#ifdef GLOBAL
=
{
     32,   56,   64,    96, 
    112,  128,  192,   224, 
    256,  320,  384,   448, 
    512,  576,  640,   768,
    896, 1024, 1152,  1280, 
   1344, 1408, 1411,  1472, 
   1536, 1920, 2048,  3072, 
   3840, 
   1/*open*/, 2/*variable*/, 3/*lossless*/
}
#endif
;

unsigned int dca_channels[] 
#ifdef GLOBAL
=
{
    1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6, 6, 7, 8, 8
}
#endif
;

//-----------------------------------------------------


/* wavefs44.c */
//void InitialSRC(void);
//void Wavefs44(FILE *file, int size, unsigned char *buffer);
//void EndSRC(FILE *file);
//void Wavefs44File(int delay, int now, int total);
//void EndWAV(FILE *file, int size);
//void DownWAV(FILE *file);
//BOOL CheckWAV(void);

// Audio buffering rate controls
#define WAVEOUT_MAX_BLOCKS    164 // DTV+WARP MEANS LOTS OF SMALL PACKETS TOGETHER
#define WAVEOUT_HIGH_PKTS_CUSHION  80 // 3
#define WAVEOUT_MID_HIGH_PKTS_CUSHION  60 
#define WAVEOUT_MID_MID_PKTS_CUSHION  40 
#define WAVEOUT_MID_PKTS_CUSHION    14 // 6 // 4
#define WAVEOUT_MID_FINE_PKTS_CUSHION  12 // 8 
#define WAVEOUT_LOW_PKTS_CUSHION    4 // 6 // 12 // 3 // 2 


volatile int iWavQue_ms
#ifdef GLOBAL
 = 0
#endif
;

volatile int  iWAVEOUT_Scheduled_Blocks
#ifdef GLOBAL 
 = 0
#endif
;
volatile int  PlayedWaveHeadersCount          // free index
#ifdef GLOBAL
 = 0
#endif
;


int  WAV_Fmt_Flag;
char WAV_Fmt_Brief[24];
unsigned char cAudState;

int iWave_MsgAlerted
#ifdef GLOBAL
 = 0
#endif
;

/*
unsigned char WAVHeader[44]
#ifdef GLOBAL
=
{
  0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00,
  0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
  0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00,
  0x80, 0xbb, 0x00, 0x00, 0x00, 0xee, 0x02, 0x00,
  0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61,
  0x00, 0x00, 0x00, 0x00
}
#endif
;
*/

