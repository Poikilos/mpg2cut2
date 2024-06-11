/* Copyright (c) 2002, John Edwards
   Subsequent enhancements by RocketJet

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//    WAVE_OUT.H - Necessary stuff for WIN_AUDIO


#include <stdio.h>
#include <windows.h>


#define Cdecl               __cdecl
#define __attribute__(x)
//#define sleep(__sec)        Sleep ((__sec) * 1000)
#define inline              __inline
#define restrict

//// constants /////////////////////////////////////////////////////

#define CD_SAMPLE_FREQ         44.1e3
#define SAMPLE_SIZE            16
#define SAMPLE_SIZE_STRING     ""
#define WINAUDIO_FD            ((FILE_T)-128)
#define FILE_T                 FILE*
#define INVALID_FILEDESC       NULL

//// Simple types //////////////////////////////////////////////////

typedef signed   int        Int;        // at least -32767...+32767, fast type
typedef unsigned int        Uint;       // at least 0...65535, fast type
typedef long double         Ldouble;    // most exact floating point format

//// procedures/functions //////////////////////////////////////////
// wave_out.c

// ROCKETJET - MULTI-THREADING
int iCtl_AudioThread
#ifdef GLOBAL
 = 0
#endif
;

DWORD  dwSPEAKER_ThreadId
#ifdef GLOBAL
 = 0
#endif
;
HANDLE hThread_SPEAKER 
#ifdef GLOBAL
 = 0
#endif
;
#define WAVEOUT_SETPARMS    WM_USER+101
#define WAVEOUT_PLAYSAMPLES WM_USER+102
#define WAVEOUT_CLOSE       WM_USER+199 


int WAV_Set_Open();       // FILE_T dummyFile , Ldouble SampleFreq, Uint BitsPerSample, Uint Channels);
Uint     WAVEOUT_SampleFreq;
Uint     WAVEOUT_BitsPerSample;
Uint     WAVEOUT_Channels;


// int
void WAV_WIN_Play_Samples(void* buff, size_t len);
void WAV_WIN_Play_Buffer (void* P_Wav_Data, size_t P_Len);

void WAV_WIN_Audio_close(void);
void WAV_WIN_Spkr_close(void), WAV_Flush();
/*static */
void WAV_Free_Memory( void);


