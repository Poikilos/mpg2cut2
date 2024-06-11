

int WAV_cs_FLAG
#ifdef GLOBAL
 = 0
#endif
;

CRITICAL_SECTION  WAV_Critical_Section;

HWAVEOUT          hWAVEdev
#ifdef GLOBAL
= NULL
#endif
;

WAVEHDR*          PlayedWaveHeaders [WAVEOUT_MAX_BLOCKS];

int iWav_Err
#ifdef GLOBAL
 = 0
#endif
;

int iWavBlock_From, iWavBlock_To;
int iWavBlock_Len[WAVEOUT_MAX_BLOCKS];
 
int Box ( const char* msg1, char msg2[80] );
void wav_MM_ERR(int p_RC, char* P_Msg2);
