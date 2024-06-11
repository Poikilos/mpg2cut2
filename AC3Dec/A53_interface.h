/* 
 *	Copyright (C) Aaron Holtzman - May 1999
 *
 *  This file is part of ac3dec, a free Dolby AC-3 stream decoder.
 *	
 *  ac3dec is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  ac3dec is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */


typedef unsigned int	uint_32;
typedef unsigned short	uint_16;
typedef unsigned char	uint_8;

typedef signed int		sint_32;
typedef signed short	sint_16;
typedef signed char		sint_8;


void InitialAC3(void);

uint_32 ac3_decode_data(uint_8 *data_start, uint_32 length, uint_32 start);

unsigned char AC3Dec_Buffer[49152];		// 48KB/frame for 64~448 Kbps

uint_32 A53_sampling_rate;

int     AC3_DRC_FLag;
int     AC3_DSDown_Flag;
double  AC3_PreScale_Ratio;

int     AC3_CRC_Chk;
char    AC3_Err_Txt[16];
int   byAC3_Init;

int  AC3_DDPLUS;
char *AC3Mode[8]
#ifdef GLOBAL
=
{
  "1+1", "1/0", "2/0", "3/0", "2/1", "3/1", "2/2", "3/2"
}
#endif
;


int AC3Rate[32]
#ifdef GLOBAL
=
{
  32, 40, 48, 56, 
  64, 80, 96, 112, 
  128, 160, 192, 224, 
  256, 320, 384, 448, 
  512, 576, 640, 755, // 755k => 754500
  1344, 1408, 1411, 1472, 
  1510, 1920, 2048, 3072, // 1510 => 1509750
  3840, 0, 0, 0
}
#endif
;


#define DRC_NONE    0
#define DRC_LIGHT   1
#define DRC_NORMAL  2
#define DRC_HEAVY   3 
#define DRC_VERYHEAVY 4 



