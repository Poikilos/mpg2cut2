/* 
 *  MPEG2DEC - Copyright (C) Mpeg Software Simulation Group 1996-99
 *  DVD2AVI  - Copyright (C) Chia-chen Kuo - April 2001
 *  Mpg2Cut2 - Various Authors
 *
 *  Part of DVD2AVI - a free MPEG-2 converter
 *	
 *  DVD2AVI is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  DVD2AVI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#include "global.h"
#include <math.h>

#define NORM_SIZE 1048576

static short Norm_Table[65536];		// -32768 ~ 32767
static short Norm_Buffer[NORM_SIZE];

static void TwoPass(FILE *WaveIn, int WaveInPos, FILE *WaveOut, int WaveOutPos, int size, int pass);

void Normalize(FILE *WaveIn, int WaveInPos, char *filename, FILE *WaveOut, int WaveOutPos, int size)
{
	int i, norm, trigger = 0;
	double ratio = 1.0;

	if (Normalization_Flag)
	{
		if (WaveIn==NULL)	// In = Out
			WaveIn = fopen(filename, "rb");
		else
			TwoPass(WaveIn, WaveInPos, NULL, 0, size, 0);

		ratio = 327.68 * Norm_Ratio / Sound_Max;

		if (ratio >= 1.01 || ratio < 1.0)
			trigger = 1;
	}
	else if (WaveIn!=NULL)
		trigger = 1;

	for (i=-32768; i<0; i++)
	{
		norm = (int)(ratio * i - 0.5);

		if (norm < -32768)
			Norm_Table[i+32768] = -32768;
		else
			Norm_Table[i+32768] = norm;
	}

	Norm_Table[32768] = 0;

	for (i=1; i<32768; i++)
	{
		if (Norm_Table[i] > -32767)
			Norm_Table[65536-i] = -Norm_Table[i];
		else
			Norm_Table[65536-i] = 32767;
	}

	sprintf(szBuffer, "%.2f", ratio);
	SetDlgItemText(hStats, IDC_INFO, szBuffer);

	if (trigger)
		TwoPass(WaveIn, WaveInPos, WaveOut, WaveOutPos, size, 1);
}

static void TwoPass(FILE *WaveIn, int WaveInPos, FILE *WaveOut, int WaveOutPos, int size, int pass)
{
	int i, rsize, maxsize = size;

	fseek(WaveIn, WaveInPos, SEEK_SET);

	if (pass)
		fseek(WaveOut, WaveOutPos, SEEK_SET);

	process.op = timeGetTime();
	
	while (size > 0)
	{
		rsize = (size >= NORM_SIZE ? NORM_SIZE : size);

		fread(Norm_Buffer, rsize, 1, WaveIn);

		if (pass)
		{
			for (i=0; i<(rsize>>1); i++)
				Norm_Buffer[i] = Norm_Table[Norm_Buffer[i]+32768];

			fwrite(Norm_Buffer, rsize, 1, WaveOut);
		}
		else
			for (i=0; i<(rsize>>1); i++)
				if (Sound_Max < abs(Norm_Buffer[i]))
					Sound_Max = abs(Norm_Buffer[i]);

		size -= rsize;

		process.ed = timeGetTime();
		process.elapsed = (process.ed-process.op)/1000;
		process.percent = (float)(100.0*(maxsize-size)/maxsize);
		process.remain = (int)((process.ed-process.op)*(100.0-process.percent)/process.percent)/1000;

		if (MParse.ShowStats_Flag)
		{
			sprintf(szBuffer, "%d:%02d:%02d", process.elapsed/3600, (process.elapsed%3600)/60, process.elapsed%60);
			SetDlgItemText(hStats, IDC_ELAPSED, szBuffer);

			sprintf(szBuffer, "%d:%02d:%02d", process.remain/3600, (process.remain%3600)/60, process.remain%60);
			SetDlgItemText(hStats, IDC_REMAIN, szBuffer);
		}

		SendMessage(hTrack, TBM_SETPOS, (WPARAM) TRUE, 
                         (int)(process.percent*TRACK_PITCH/100));
	}
}
