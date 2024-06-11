/*
 *  MPEG2DEC - Copyright (C) Mpeg Software Simulation Group 1996-99
 *  DVD2AVI  - Copyright (C) Chia-chen Kuo - April 2001
 *  Mpg2Cut2 - Various Authors
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

unsigned int BitsLeft, CurrentBfr, NextBfr; // , Val;//, File_ReadLen;
unsigned char *RdPTR, *RdEndPkt, *Rd_NextPacket, *RdEndPktHdr,
                      *RdEndPkt_4, *RdEndPkt_8,
              *RdEOB, *RdEOB_4, *RdEOB_8;
//unsigned int  RdGOT;

//unsigned char  RdBFR[MPEG_SEARCH_BUFSZ];  // Consider GetMain
unsigned char *RdBFR;

int iAUD_Enough;

//__forceinline static unsigned int Get_Bits(unsigned int N);
//__forceinline static 
void InputBuffer_NEXT_fill(int);

void InputBuffer_FILL(void);
void Get_Next_Packet(void);
//void InputBuffer_FLUSH_ALL(unsigned int N);
//unsigned int Get_Bits_All(unsigned int N);
void Mpeg_READ(), Mpeg_READ_Adjust(), Mpeg_READ_Bug_Adj();
BYTE* Mpeg_BytePtr();
void Got_PS2_NAV(int);
void Got_PS2_Pkt(int);


