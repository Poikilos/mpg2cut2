


// AUDIO Handling

#define AUDIO_DEMUXALL    0
#define AUDIO_DEMUXONE    1
#define AUDIO_DECODE    2

#define CHANNELS_MAX      8

#define Audio_Dunno 0xABCD;
//int rj_Audio_Code; // = Audio_Dunno;

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

#define K_BOOST_DENOM  16



char *FORMAT_ABBR[]
#ifdef GLOBAL
=
{ &"",     &"MPa",  &"PCM", &"DD", "&DTS", &"DD+", 
  &"PS1",  &"PS2",  &"SUB",  &"UNK"
}
#endif
;

unsigned int PS1_SampleRate[4]
#ifdef GLOBAL
=
{ 48000, 44100, 32000, 96000
}
#endif
;


unsigned int iAudio_SEL_Track, iAudio_SEL_Format;
int iAudio_Trk_FMT[CHANNELS_MAX+1];   // Maps track number into a format
unsigned char uAudio_Track_Stream[CHANNELS_MAX+1]; // Maps track number into a Stream_id 

int iWant_Aud_Format;
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


int iCtl_Audio_PS2, iCtl_Audio_CRC; 
int iCtl_Volume_Boost, iCtl_Volume_BOLD, iCtl_Volume_SlowAttack;
int iCtl_Volume_Boost_MPA_48k, iCtl_Volume_Boost_MPA_other;
int iCtl_Volume_Boost_AC3;
int iCtl_Volume_Limiting, iCtl_Audio_Ceiling;
int iCtl_Audio_InitBreathe;

void Volume_Init();

int iPlayAudio, iWAV_Init, iMPAdec_Init, iCtl_AudioDecoder;
int iWantAudio;
int iInPS2_Audio;
int iVolume_Boost, iVolume_BOLD, iVolume_UnBoost_Recent;

int iAudio_Lock;  
int iAudio_Force44K;



void VOL300_Volume_Boost();
void VOL204_Volume_Mute();
void VOL206_Volume_UN_Mute();
void VOL305_Volume_Lesser(), VOL306_Volume_LessBold();
void VOL305_Volume_Boost_Start();
void VOL203_Volume_Target();
void VOL307_Volume_Bolder();


 
int AC3_Flag;
int MPA_Flag;
//int Decision_Flag;
int SRC_Flag;
int Normalization_Flag;
int Norm_Ratio;
short Sound_Max;



typedef struct {
//  FILE          *file;
  int           rip;
  int           delay;
  unsigned      uLayer;
  unsigned      uMPA_Sample_Hz;
  unsigned char uStream;
  char          desc[32];

} MPAStream;
MPAStream mpa_Ctl[CHANNELS_MAX];


typedef struct {
//  FILE          *file;
  int           rip;
  unsigned int  mode, uBitRate, uSampleRate;
  unsigned char uStream;
} AC3Stream;
AC3Stream SubStream_CTL[6][CHANNELS_MAX];



struct WAVStream {
  int           rip;
  int           size;
  int           delay;
} wav;



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
#define WAVEOUT_LOW_PKTS_CUSHION    12 // 3 // 2 


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

