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

// #include "../global.h"
//#define GLOBAL
//extern "C"
//{
#include "ac3.h"
#include "A53_interface.h"
//}


#include "bitstream.h"

void bit_allocate(uint_16 fscod, bsi_t *bsi, audblk_t *audblk);

static audblk_t audblk;
static bsi_t bsi;
static syncinfo_t syncinfo;

// the floating point samples for one audblk
static stream_samples_t samples;

// the integer samples for the entire frame (with enough space for 2 ch out)
// if this size change, be sure to change the size when muting
static sint_16 s16_samples[2 * 6 * 256];
static short *ptrAC3Dec_Buffer = (short*)AC3Dec_Buffer;

// storage for the syncframe
#define               BUFFER_MAX_SIZE  16384 // 4096
static uint_8  buffer[BUFFER_MAX_SIZE];
static sint_32 Ac3Buf_size;



uint_32 decode_buffer_syncframe(syncinfo_t *syncinfo, uint_8 **start, uint_8 *end)
{
	uint_8 *cur = *start;
	uint_16 syncword = syncinfo->syncword;
	uint_32 ret = 0;

	// find an ac3 sync frame
resync:
	while(syncword != 0x0b77)
	{
		if(cur >= end)
			goto done;
		syncword = (syncword << 8) + *cur++;
	}

	// need the next 3 bytes to decide how big the frame is
	while(Ac3Buf_size < 3)
	{
		if(cur >= end)
			goto done;

		buffer[Ac3Buf_size++] = *cur++;
	}
	
	parse_syncinfo(syncinfo, buffer);

	if (syncinfo->frame_size==0)		// CRITICAL CONDITION
		goto done;

	while (Ac3Buf_size < (syncinfo->frame_size<<1) - 2)
	{
		if(cur >= end)
			goto done;

		buffer[Ac3Buf_size++] = *cur++;
	}

	// check the crc over the entire frame

  if (AC3_CRC_Chk)
  {
	  if (crc_process_frame(buffer, (syncinfo->frame_size<<1) - 2))
	  {
        strcpy(AC3_Err_Txt, "Aud.CRC ERR");

		    syncword = 0xffff;
		    Ac3Buf_size = 0;
		    goto resync;
    }
	}

	// if we got to this point, we found a valid ac3 frame to decode
	bitstream_init(buffer);
	// get rid of the syncinfo struct as we already parsed it
	bitstream_get(24);

	// reset the syncword for next time
	syncword = 0xffff;
	Ac3Buf_size = 0;

  /*
  if (AC3_DDPLUS)
  {
     if ()
	    goto resync;
  }
  */

	ret = 1;

done:
	syncinfo->syncword = syncword;
	*start = cur;
	return ret;
}





//---------------------------------------------------------------------



uint_32 ac3_decode_data(uint_8 *data_start, uint_32 length, uint_32 start)
{
	uint_8 *data_end = data_start + length;
	uint_32 i ;
//	uint_32 j = start ;

	while (decode_buffer_syncframe(&syncinfo, &data_start, data_end))
	{
		if (error_flag)
		{
        strcpy(AC3_Err_Txt, "Ac3 ERR");

			  ZeroMemory(s16_samples, sizeof(sint_16) * 256 * 2 * 6);
			  error_flag = 0;
			continue;
		}

		parse_bsi(&bsi);

		for (i=0; i<6; i++)
		{
			ZeroMemory(samples, sizeof(double) * 256 * (bsi.nfchans + bsi.lfeon));

			parse_audblk(&bsi, &audblk);

			exponent_unpack(&bsi, &audblk); 
			if (error_flag)
				goto error;

			bit_allocate(syncinfo.fscod, &bsi, &audblk);

			coeff_unpack(&bsi, &audblk, samples);
			if (error_flag)
				goto error;

			if (bsi.acmod == 0x2)
				rematrix(&audblk, samples);

			imdct(&bsi, &audblk, samples);

			downmix(&audblk, &bsi, samples, &s16_samples[i * 512]);

			sanity_check(&bsi, &audblk);
			if (error_flag)
				goto error;
		}

		memcpy(&AC3Dec_Buffer[start], s16_samples, 6144);
		start += 6144;
error:
		;
	}

	A53_sampling_rate = syncinfo.sampling_rate;

/*	if (Decision_Flag || (!SRC_Flag && Normalization_Flag))
		for (i=(j>>1); i<(start>>1); i++)
			if (Sound_Max < abs(ptrAC3Dec_Buffer[i]))
			{
				Sound_Max = abs(ptrAC3Dec_Buffer[i]);

				if (Decision_Flag && Sound_Max > Norm_Ratio)
				{
					sprintf(szBuffer, "%.2f", 327.68 * Norm_Ratio / Sound_Max);
					SetDlgItemText(hStats, IDC_INFO, szBuffer);
				}
			}
*/
	return start;
}


//---------------------------------------
void InitialAC3()
{
	error_flag = Ac3Buf_size = 0;

	ZeroMemory(&syncinfo, sizeof(syncinfo));
	ZeroMemory(&bsi, sizeof(bsi));
	ZeroMemory(&audblk, sizeof(audblk));

	drc_init();
	imdct_init();
	exponent_init();
	mantissa_init();

	srand(0);
	byAC3_Init = 1;
}
