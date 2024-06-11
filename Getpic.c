
#define DBG_RJ


/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * According to Wikipedia, the patents covering mpeg-1 video
 * and audio layers 1 & 2  have now expired.
 *
 * Commercial implementations of MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */
/* SSE2 code by Dmitry Rozhdestvensky */

#include "global.h"
#include "getbit.h"
#include "GetBit_Fast.h"
#include "MPV_PIC.h"
#include "PIC_BUF.h"


static int cc_table[32] = {
  0, 0, 0, 0, 1, 2, 1, 2, 1, 2, 1, 2,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  // Allow for bigguns
};

/* private prototypes*/
__forceinline static void Pic_Reuse_Buffers(void);
__forceinline static void Pic_Decode_CONTENT(void);
__forceinline static int Pic_Decode_SLICE(int MBAmax);
__forceinline static void macroblock_modes(int *pmacroblock_type, int *pmotion_type,
  int *pmotion_vector_count, int *pmv_format, int *pdmv, int *pmvscale, int *pdct_type);
__forceinline static void Clear_Block(int count);
__forceinline static void Add_Block(int count, int bx, int by, int dct_type, int addflag);
__forceinline static void motion_compensation(int MBA, int macroblock_type, int motion_type,
  int PMV[2][2][2], int motion_vert_field_sel[2][2], int dmvector[2], int dct_type);
__forceinline static void skipped_macroblock(int dc_dct_pred[3], int PMV[2][2][2],
  int *motion_type, int motion_vert_field_sel[2][2], int *macroblock_type);
__forceinline static int Pic_Decode_Slice_START(int *MBA, int *MBAinc, int dc_dct_pred[3], int PMV[2][2][2]);
__forceinline static int decode_macroblock(int *macroblock_type, int *motion_type, int *dct_type,
  int PMV[2][2][2], int dc_dct_pred[3], int motion_vert_field_sel[2][2], int dmvector[2]);
__forceinline static void Decode_MPEG2_Intra_Block(int comp, int dc_dct_pred[]);
__forceinline static void Decode_MPEG2_Non_Intra_Block(int comp);

__forceinline static int Get_macroblock_type(void);
__forceinline static int Get_I_macroblock_type(void);
__forceinline static int Get_P_macroblock_type(void);
__forceinline static int Get_B_macroblock_type(void);
__forceinline static int Get_D_macroblock_type(void);
__forceinline static int Get_coded_block_pattern(void);
__forceinline static int Get_macroblock_address_increment(void);
__forceinline static int Get_Luma_DC_dct_diff(void);
__forceinline static int Get_Chroma_DC_dct_diff(void);

__forceinline static void form_predictions(int bx, int by, int macroblock_type, int motion_type, int PMV[2][2][2],
  int motion_vert_field_sel[2][2], int dmvector[2]);
static void form_prediction(unsigned char *src[], int sfield, unsigned char *dst[], int dfield,
  int lx, int lx2, int w, int h, int x, int y, int dx, int dy, int average_flag);
__forceinline static void form_component_prediction(unsigned char *src, unsigned char *dst,
  int lx, int lx2, int w, int h, int x, int y, int dx, int dy, int average_flag);

//extern    void ( _fastcall *idct_decoder)(short *block); // idct



//---------------------------------
/* decode one frame */

void Pic_DECODE()
{
  int iDrop_This_flag;

#ifdef DBG_RJ
  if (DBGflag)
  {
       sprintf(szBuffer,"PIC DECODE Type: %c = x%02X",  Coded_Pic_Abbr[MPEG_Pic_Type], MPEG_Pic_Type);
       DBGout(szBuffer);
  }
#endif

  if (MPEG_Pic_Type < 4)
  {
      PlayCtl.uPendingSeq[MPEG_Pic_Type] = MPEG_Pic_Temporal_Ref;
  }

  iGOPdiff = iPICtime - iGOPtime;

  // Accelerate dropping of B-Frames ?
  if (PlayCtl.iDrop_B_Frames_Flag
  &&  MPEG_Pic_Type == B_TYPE
  && process.Action == ACTION_RIP   &&  !MParse.Tulebox_SingleStep_flag
  && (PlayCtl.iDrop_B_Frames_Flag > 1
      || PREV_Pic_Type != MPEG_Pic_Type))
  {
      iDrop_B_Now_flag = 1;

      if (PlayCtl.iDrop_PTS_Flag     && process.Got_PTS_Flag
      && process.iVid_PTS_Resolution <= PlayCtl.iDrop_B_Frames_Flag)
          process.SkipPTS = process.VideoPTS;
  }
  else
    iDrop_B_Now_flag = 0;



  if (    uGot_Video_Stream  != uCtl_Video_Stream
       && uCtl_Video_Stream < STREAM_AUTO)
  {
      //GetB_Show_Next_Start_Code(1); // Get_Next_Packet_Start();   //
      iDrop_This_flag = -1;   //return;
  }
  else
  if (iDrop_B_Now_flag
      //&& PlayCtl.iDrop_Behind            && MPEG_Pic_Type != I_TYPE
      //                           && iGOPdiff > iGOPperiod
        )
  {
      PlayCtl.iDropped_Frames++;
      iGOPdrop_Ctr++;
      //GetB_Show_Next_Start_Code(1); // Get_Next_Packet_Start();   //
      iDrop_This_flag = 1;   //return;
  }
  else  
  {
     iGOPdec_Ctr++;
     iDrop_This_flag = 0;
  }



  if (MPEG_Pic_Structure == FULL_FRAME_PIC && Second_Field)
    Second_Field = 0;


  if (MPEG_Pic_Type != B_TYPE)
  {
    d2v_fwd = d2v_bwd;
    d2v_bwd = d2v_curr;
  }


  /*
  if (D2V_Flag)
      D2V_Pic();
  else
  */

  if (iDrop_This_flag >= 0)  // Select Video track ?
  {

    /* update picture buffer pointers */
    Pic_Reuse_Buffers();

    if (! iDrop_This_flag)
    {

      //  2nd field decode, skip if accelerator conditions apply...
      if (! iField_Drop // Field Accelerator OFF forces decode
      ||  process.Action     != ACTION_RIP
      ||  MParse.Tulebox_SingleStep_flag
      ||  MPEG_Pic_Structure == FULL_FRAME_PIC        || ! Second_Field
      ||  ! Deint_VIEW       || ! iCtl_View_Fast_YUV  ||   iZoom > 2)
      {
          /* decode picture data ISO/IEC 13818-2 section 6.2.3.7 */
          Pic_Decode_CONTENT();
      }
      else
      {
        if (PlayCtl.iDrop_PTS_Flag     && process.Got_PTS_Flag
        && process.iVid_PTS_Resolution <= PlayCtl.iDrop_B_Frames_Flag)
           process.SkipPTS = process.VideoPTS;

        GetB_Show_Next_Start_Code(1); // Skip 2nd field
      }


      /* write or display current or previously decoded reference frame */
      /* ISO/IEC 13818-2 section 6.1.1.11: Frame reordering */

      //  Developing new VERSION of the "IF" cascade.
      //  Trying to get "B" frames to show properly when single stepping
      if (iField_Experiment)
      {
        // Is it time to write out a frame ?   IE 2 fields dpne or a single full-frame ?
        if (MPEG_Pic_Structure == FULL_FRAME_PIC || Second_Field)
        {
          // After the zeroth frame, decode buffer pointers are different, dunno why.
          if (Frame_Number)
          {
            if (MPEG_Pic_Type == B_TYPE)
              Write_Frame(aux_frame,     d2v_curr, Frame_Number-1);
            else
              Write_Frame(fwd_ref_frame, d2v_fwd,  Frame_Number-1);
          }
          else
          {
            if (MPEG_Pic_Type == B_TYPE)
                 Write_Frame(aux_frame,     d2v_curr, /* d2v_fwd,*/  /* d2v_bwd,*/  0);
            //   Write_Frame(fwd_ref_frame, d2v_bwd,  /* d2v_curr,*/ /* d2v_fwd,*/  0);
            else
                Write_Frame(bwd_ref_frame, d2v_bwd,  0);
          }

          // Kill process early if doing a single frame
          if (process.Action != ACTION_RIP)
             Mpeg_KILL(1002);   // <<-----------------------  MAIN ESCAPE ROUTE - Surely we could be more elegant than this ?

        } // ENDIF  Frame ready to show
      } // ENDIF RJ Experimental


      else
      {
        //  CAN'T QUITE FIGURE OUT HOW THIS ORIGINAL "IF" CASCADE OPERATES
        if (Frame_Number
        &&  process.Action != ACTION_FWD_FRAME
        && (MPEG_Pic_Structure == FULL_FRAME_PIC || Second_Field))
        {
           if (MPEG_Pic_Type == B_TYPE)
             Write_Frame(aux_frame,     d2v_curr, Frame_Number-1);
           else
             Write_Frame(fwd_ref_frame, d2v_fwd,  Frame_Number-1);
        }
        else
        if (process.Action != ACTION_RIP
        && (MPEG_Pic_Structure == FULL_FRAME_PIC || Second_Field) )
        {
          if (MPEG_Pic_Type == B_TYPE)
              Write_Frame(aux_frame,     d2v_curr, Frame_Number-1);
          else
              Write_Frame(bwd_ref_frame, d2v_bwd,  0);

          if (MPEG_Pic_Structure != FULL_FRAME_PIC)
              Second_Field = !Second_Field;

          if ( ! Second_Field)
               Frame_Number++;

          Mpeg_KILL(1003);   // <<------------------------  MAIN ESCAPE ROUTE ?
        }
      }//  END NOT Experimental
    } // ENDIF NOT DROPPED
  } // ENDIF  Selected Video Track  (Was: Decision_flag (Whatever that might be))

  if (MPEG_Pic_Structure != FULL_FRAME_PIC)
      Second_Field = !Second_Field;

  if ( ! Second_Field)
      Frame_Number++;

  return;
}



//---------------------------------------------------------------
// reuse old picture buffers as soon as they are no longer needed

static void Pic_Reuse_Buffers()
{
  int cc, iSize, iLite[3];
  unsigned char *tmp;
  unsigned long *lpScan, *lpEnd;

  iLite[0] = 0x77777777;
  iLite[1] = 0xFFFFFFFF;
  iLite[2] = 0x44444444;

// Shuffle Frame Buffer Pointers

  for (cc=0; cc<3; cc++)
  {
    /* B pictures do not need to be save for future reference */
    if (MPEG_Pic_Type == B_TYPE)
    {
       curr_frame      [cc] =  aux_frame [cc];
       curr_full_frame [cc] =  curr_frame[cc];
    }
    else
    {
        /* only update at the beginning of the coded frame */
      if (!Second_Field)
      {
         tmp = fwd_ref_frame[cc];

         /* the previously decoded reference frame is stored coincident with the
            location where the backward reference frame is stored (backwards
            prediction is not needed in P pictures) */

         fwd_ref_frame[cc] = bwd_ref_frame[cc];

         /* update pointer for potential future B pictures */
         bwd_ref_frame[cc] = tmp;

         // Optionally Highlight errors in I-FRAMES
         if (Err_Analysis && process.Action < ACTION_RIP
                          && MPEG_Pic_Type == I_TYPE )
         {
            if (cc)
               iSize = Chroma_Size;
            else
               iSize = Coded_Pic_Size;

            lpScan = (unsigned long *)(bwd_ref_frame[cc]);
            lpEnd  = lpScan + iSize;
            while   (lpScan < lpEnd)
            {
              *lpScan = iLite[cc];
               lpScan = lpScan + 4;
            }
         }

      } // ENDIF (!second_field)

      /* can erase over old backward reference frame since it is not used
         in a P picture, and since any subsequent B pictures will use the
         previously decoded I or P frame as the bwd_ref_frame */

      curr_frame      [cc] = bwd_ref_frame[cc];
      curr_full_frame [cc] = curr_frame   [cc];

    } // END-ELSE !B-TYPE

    if (MPEG_Pic_Structure == BOTTOM_FIELD)
        curr_frame[cc] += (cc==0) ? Coded_Pic_Width : Chroma_Width;
  }

}




//-------------------------------------------------
// decode all macroblocks of the current picture
// stages described in ISO/IEC 13818-2 section 7

static void Pic_Decode_CONTENT()
{
  int MBAmax;

  /* number of macroblocks per picture */
  MBAmax = mb_width * mb_height;

  MParse.Fault_Prev = 0;

  if (MPEG_Pic_Structure != FULL_FRAME_PIC)
      MBAmax >>= 1;

  if (process.Action != ACTION_RIP) // && Mpeg_PES_Version != 2)
      DSP3_Main_TIME_INFO();

  for (;;)
    if (Pic_Decode_SLICE(MBAmax) < 0)
      return;
}



//---------------------------------------------------------
/* decode all macroblocks of the current picture */
/* ISO/IEC 13818-2 section 6.3.16 */
/* return 0 : go to next slice */
/* return -1: go to next picture */

static int Pic_Decode_SLICE(int MBAmax)
{
  int MBA = 0, MBAinc = 0 ;
  int macroblock_type, motion_type ;
  int dct_type = 0 ;             // RJ SET DEFAULT TO ALLOW FOR BAD DATA
  int ret;
  int dc_dct_pred[3], PMV[2][2][2], motion_vert_field_sel[2][2],
      dmvector[2];

  ret = Pic_Decode_Slice_START(&MBA, &MBAinc, dc_dct_pred, PMV) ;
  if ( ret != 1)
    return ret;

  for (;;)
  {
    /* this is how we properly exit out of picture */
    if (MBA >= MBAmax) return -1;   // all macroblocks decoded

    if (MBAinc == 0)
    {
      if (!Show_Bits(23) || MParse.Fault_Flag) // Start_Code or fault
      {
resync:
        if (MParse.Fault_Flag  &&  ! MParse.Stop_Flag  && !iAudioDBG)
        {
           sprintf(szMPG_ErrTxt, "Vid Macro ERR %d.%x",
                        MParse.Fault_Flag, MParse.Fault_More);
           SetDlgItemText(hStats, IDC_INFO, szMPG_ErrTxt);
        }


        MParse.Fault_Flag = 0; MParse.Fault_More = 0;   // fault tolerance 
        return 0; // trigger: go to next slice
      }
      else /* neither Start_Code nor MParse.Fault_Flag */
      {
        /* decode macroblock address increment */
        MBAinc = Get_macroblock_address_increment();
        if (MParse.Fault_Flag)
          goto resync;
      }
    }

    if (MBAinc == 1) /* not skipped */
    {
      if (!decode_macroblock(&macroblock_type, &motion_type,
                                               &dct_type, PMV,
                             dc_dct_pred, motion_vert_field_sel, dmvector))
          goto resync;
    }
    else /* MBAinc!=1: skipped macroblock */
      /* ISO/IEC 13818-2 section 7.6.6 */
      skipped_macroblock(dc_dct_pred, PMV, &motion_type, motion_vert_field_sel, &macroblock_type);

    /* ISO/IEC 13818-2 section 7.6 */
    motion_compensation(MBA, macroblock_type, motion_type, PMV,
              motion_vert_field_sel, dmvector, dct_type);

    /* advance to next macroblock */
    MBA++; MBAinc--;

    if (MBA >= MBAmax)
      return -1;   // all macroblocks decoded
  }
}



//---------------------------------------------------------
/* ISO/IEC 13818-2 section 6.3.17.1: Macroblock modes */

static void macroblock_modes(int *pmacroblock_type, int *pmotion_type,
               int *pmotion_vector_count, int *pmv_format,
               int *pdmv, int *pmvscale, int *pdct_type)
{

  int macroblock_type ;
  int motion_type = 0 ; // RJ SET DEFAULT TO ALLOW FOR BAD DATA
  int motion_vector_count;
  int mv_format, dmv, mvscale, dct_type;


  /* get macroblock_type */

  macroblock_type = Get_macroblock_type();
  if (MParse.Fault_Flag)
    return;

  /* get frame/field motion type */

  if (macroblock_type
      & (MACBLK_MOTION_FWD|MACBLK_MOTION_BWD))
  {
    if (MPEG_Pic_Structure==FULL_FRAME_PIC)
      motion_type = MPEG_Pic_pred_frame_dct ? MC_FRAME : Get_Bits(2);
    else
      motion_type = Get_Bits(2);
  }
  else
    if ((macroblock_type & MACBLK_INTRA)
                         && MPEG_Pic_concealment_motion_vectors)
          motion_type = (MPEG_Pic_Structure==FULL_FRAME_PIC) ?
                         MC_FRAME : MC_FIELD;

  /* derive motion_vector_count, mv_format and dmv, (table 6-17, 6-18) */

  if (MPEG_Pic_Structure == FULL_FRAME_PIC)
  {
    motion_vector_count = (motion_type == MC_FIELD) ? 2 : 1;
    mv_format = (motion_type==MC_FRAME) ? MV_FRAME : MV_FIELD;
  }
  else
  {
    motion_vector_count = (motion_type == MC_16X8) ? 2 : 1;
    mv_format = MV_FIELD;
  }

  dmv = (motion_type == MC_DMV); /* dual prime */

  /*
     field mv predictions in frame pictures have to be scaled
     ISO/IEC 13818-2 section 7.6.3.1 Decoding the motion vectors
  */
  mvscale = (mv_format == MV_FIELD
          && MPEG_Pic_Structure == FULL_FRAME_PIC);

  /* get dct_type (frame DCT / field DCT) */

  dct_type = (MPEG_Pic_Structure == FULL_FRAME_PIC
              && !MPEG_Pic_pred_frame_dct
              && (macroblock_type & (MACBLK_PATTERN|MACBLK_INTRA)) ?
                                                       Get_Bits(1) : 0);

  /* return values */

  *pmacroblock_type = macroblock_type;
  *pmotion_type     = motion_type;
  *pmotion_vector_count = motion_vector_count;
  *pmv_format = mv_format;
  *pdmv       = dmv;
  *pmvscale   = mvscale;
  *pdct_type  = dct_type;
}




//------------------------------------------------------------------
/* move/add 8x8-Block from block[comp] to bwd_ref_frame */
/* copy reconstructed 8x8 block from block[comp] to curr_frame[]

   ISO/IEC 13818-2 section 7.6.8: Adding prediction and coefficient data
   This stage also embodies some of the operations implied by:
   - ISO/IEC 13818-2 section 7.6.7: Combining predictions
   - ISO/IEC 13818-2 section 6.1.3: Macroblock
*/

static void Add_Block(int iP_count,
                      int bx, int by,
                      int dct_type, int addflag)
{

  static const __int64 mmmask_128 = 0x0080008000800080;

  int comp, cc, iincr, bxh, byh, iW_count;
  unsigned char *rfp;
  short *Block_Ptr;

  if      (iP_count >  15) iW_count =  15 ;      // RJ TRAP BAD count PARM
  else if (iP_count <  00) iW_count =  00 ;
  else                     iW_count = iP_count  ;

  for (comp=0 ; comp < iW_count ; comp++)
  {
    Block_Ptr =    block[comp];
    cc        = cc_table[comp];
/* NEED PROCESSOR PACK TO ASSEMBLE SSE2 INSTRUCTIONS
    if(MParse.iDCT_Flag==IDCT_SSE2)
      __asm
      {
        mov  eax,[Block_Ptr]
        prefetcht0 [eax]
      };
*/
    bxh = bx; byh = by;

    if (cc == 0)
    {
      if (MPEG_Pic_Structure == FULL_FRAME_PIC)
      {
        if (dct_type)
        {
          rfp = curr_frame[0] + Coded_Pic_Width*(by+((comp&2)>>1))
                                 + bx + ((comp&1)<<3);
          iincr = DOUBLE_WIDTH;
        }
        else
        {
          rfp = curr_frame[0] + Coded_Pic_Width*(by+((comp&2)<<2))
                                 + bx + ((comp&1)<<3);
          iincr = Coded_Pic_Width;
        }
      }
      else
      {
        rfp = curr_frame[0] + (DOUBLE_WIDTH)*(by+((comp&2)<<2))
                               + bx + ((comp&1)<<3);
        iincr = DOUBLE_WIDTH;
      }
    }
    else
    {
      if (MPEG_Seq_chroma_format != CHROMA444)
          bxh >>= 1;
      if (MPEG_Seq_chroma_format < CHROMA422) //== CHROMA420)
          byh >>= 1;

      if (MPEG_Pic_Structure == FULL_FRAME_PIC)
      {
        if (dct_type && MPEG_Seq_chroma_format > CHROMA420) // != CHROMA420)

        {
          /* field DCT coding */
          rfp = curr_frame[cc] + Chroma_Width * (byh+ ((comp & 2)>>1))
                               + bxh + (comp&8);
          iincr = Chroma_Width << 1 ;
        }
        else
        {
          /* frame DCT coding */
          rfp = curr_frame[cc] + Chroma_Width * (byh+ ((comp & 2)<<2))
                               + bxh + (comp&8);
          iincr = Chroma_Width;
        }
      }
      else
      {
        /* field picture */
        rfp = curr_frame[cc] + (Chroma_Width<<1)*(byh+((comp&2)<<2))
                                + bxh + (comp&8);
        iincr = Chroma_Width << 1;
      }

      if (rfp < frame_pool) // Detect corrupted curr_frame[2] pointer
        return;
    }

    if (addflag)
    {
/* NEED PROCESSOR PACK TO ASSEMBLE SSE2 INSTRUCTIONS
      if(MParse.iDCT_Flag==IDCT_SSE2)
      __asm
      {
        pxor    xmm0,xmm0
        mov    eax, [rfp]
        mov    ebx, [Block_Ptr]
        mov    ecx,[iincr]
        mov    edx,ecx
        add    edx,edx
        add    edx,ecx  ;=iincr*3

        ;--------------------------------------------------
        movdqa    xmm1,[eax]    ;y7-y0 y7-y0
        movdqa    xmm2,xmm1

        punpcklbw  xmm1,xmm0    ;low y7-y0
        paddsw    xmm1,[ebx+32*0]    ;low x7-x0

        punpckhbw  xmm2,xmm0    ;high y7-y0
        paddsw    xmm2,[ebx+32*0+16]  ;high x7-x0

        packuswb  xmm1,xmm2
        movdqa    [eax],xmm1

        ;---------------------------------------------------
        movdqa    xmm3,[eax+ecx]    ;y7-y0 y7-y0
        movdqa    xmm4,xmm3

        punpcklbw  xmm3,xmm0    ;low y7-y0
        paddsw    xmm3,[ebx+32*1]    ;low x7-x0

        punpckhbw  xmm4,xmm0    ;high y7-y0
        paddsw    xmm4,[ebx+32*1+16]  ;high x7-x0

        packuswb  xmm3,xmm4
        movdqa    [eax+ecx],xmm3

        ;---------------------------------------------------
        movdqa    xmm5,[eax+ecx*2]    ;y7-y0 y7-y0
        movdqa    xmm6,xmm5

        punpcklbw  xmm5,xmm0    ;low y7-y0
        paddsw    xmm5,[ebx+32*2]    ;low x7-x0

        punpckhbw  xmm6,xmm0    ;high y7-y0
        paddsw    xmm6,[ebx+32*2+16]  ;high x7-x0

        packuswb  xmm5,xmm6
        movdqa    [eax+ecx*2],xmm5

        ;---------------------------------------------------
        movdqa    xmm1,[eax+edx]    ;y7-y0 y7-y0
        movdqa    xmm2,xmm1

        punpcklbw  xmm1,xmm0    ;low y7-y0
        paddsw    xmm1,[ebx+32*3]    ;low x7-x0

        punpckhbw  xmm2,xmm0    ;high y7-y0
        paddsw    xmm2,[ebx+32*3+16]  ;high x7-x0

        packuswb  xmm1,xmm2
        movdqa    [eax+edx],xmm1
      } else
*/
      __asm
      {
        pxor    mm0, mm0
        mov     eax, [rfp]
        mov     ebx, [Block_Ptr]
        mov     edi, 8
addon:
        movq    mm2, [ebx+8]

        movq    mm3, [eax]
        movq    mm4, mm3

        movq    mm1, [ebx]
        punpckhbw mm3, mm0

        paddsw    mm3, mm2
        packuswb  mm3, mm0

        punpcklbw mm4, mm0
        psllq   mm3, 32

        paddsw    mm4, mm1
        packuswb  mm4, mm0

        por     mm3, mm4
        add     ebx, 16

        dec     edi
        movq    [eax], mm3

        add     eax, [iincr]
        cmp     edi, 0x00
        jg      addon
      }
    }
    else
    {
/* NEED SP 6 MAINT
      if(MParse.iDCT_Flag==IDCT_SSE2)
      __asm{
          mov      eax, [rfp]
        mov      ebx, [Block_Ptr]
        mov    ecx,[iincr]
        mov    edx,ecx
        add    edx,edx
        add    edx,ecx  ;=iincr*3

        movq    xmm7, [mmmask_128]
        pshufd    xmm7,xmm7,0

        ;-----------
        movdqa    xmm0,[ebx+32*0]
        paddsw    xmm0,xmm7

        movdqa    xmm1,[ebx+32*0+16]
        paddsw    xmm1,xmm7

        packuswb  xmm0,xmm1
        movdqa    [eax],xmm0
        ;-----------
        movdqa    xmm2,[ebx+32*1]
        paddsw    xmm2,xmm7

        movdqa    xmm3,[ebx+32*1+16]
        paddsw    xmm3,xmm7

        packuswb  xmm2,xmm3
        movdqa    [eax+ecx],xmm2
        ;-----------
        movdqa    xmm4,[ebx+32*2]
        paddsw    xmm4,xmm7

        movdqa    xmm5,[ebx+32*2+16]
        paddsw    xmm5,xmm7

        packuswb  xmm4,xmm5
        movdqa    [eax+ecx*2],xmm4
        ;-----------
        movdqa    xmm6,[ebx+32*3]
        paddsw    xmm6,xmm7

        paddsw    xmm7,[ebx+32*3+16]

        packuswb  xmm6,xmm7
        movdqa    [eax+edx],xmm6
      } else
*/
      __asm
      {
        mov     eax, [rfp]
        mov     ebx, [Block_Ptr]
        mov     edi, 8

        pxor    mm0, mm0
        movq    mm1, [mmmask_128]
addoff:
        movq    mm3, [ebx+8]
        movq    mm4, [ebx]

        paddsw    mm3, mm1
        paddsw    mm4, mm1

        packuswb  mm3, mm0
        packuswb  mm4, mm0

        psllq   mm3, 32
        por     mm3, mm4

        add     ebx, 16
        dec     edi

        movq    [eax], mm3     //  SOMETIMES CRASHES HERE ON BAD INPUT DATA

        add     eax, [iincr]
        cmp     edi, 0x00
        jg      addoff
      }
    }
  }

  __asm emms;
}





//-----------------------------------------------------------------
/* set scratch pad macroblock to zero */

static void Clear_Block(int count)
{
  int comp;
  short *Block_Ptr;

  for (comp=0; comp<count; comp++)
  {
    Block_Ptr = block[comp];
/* MAYBE NEED TO APPLY PROCESSOR PACK
    if(MParse.iDCT_Flag==IDCT_SSE2)
    __asm
    {
      mov      eax, [Block_Ptr];
      pxor      xmm0, xmm0;
      movdqa    [eax+0 ], xmm0;
      movdqa    [eax+16 ], xmm0;
      movdqa    [eax+32 ], xmm0;
      movdqa    [eax+48 ], xmm0;
      movdqa    [eax+64 ], xmm0;
      movdqa    [eax+80 ], xmm0;
      movdqa    [eax+96 ], xmm0;
      movdqa    [eax+112 ], xmm0;

    } else
*/
    __asm
    {
      mov     eax, [Block_Ptr];
      pxor    mm0, mm0;
      movq    [eax+0 ], mm0;
      movq    [eax+8 ], mm0;
      movq    [eax+16], mm0;
      movq    [eax+24], mm0;
      movq    [eax+32], mm0;
      movq    [eax+40], mm0;
      movq    [eax+48], mm0;
      movq    [eax+56], mm0;
      movq    [eax+64], mm0;
      movq    [eax+72], mm0;
      movq    [eax+80], mm0;
      movq    [eax+88], mm0;
      movq    [eax+96], mm0;
      movq    [eax+104],mm0;
      movq    [eax+112],mm0;
      movq    [eax+120],mm0;
    }
  }

  __asm emms;

}




//--------------------------------------------------------------
/* ISO/IEC 13818-2 section 7.6 */

static void motion_compensation(int MBA, int macroblock_type,
                                int motion_type,
                int PMV[2][2][2],
                int motion_vert_field_sel[2][2],
                int dmvector[2], int dct_type)
{

  int bx, by;
  int comp;
//  short* prefetchpointer;

  /* derive current macroblock position within picture */
  /* ISO/IEC 13818-2 section 6.3.1.6 and 6.3.1.7 */

  bx = 16*(MBA % mb_width);
  by = 16*(MBA / mb_width);

  /* motion compensation */


  if (!(macroblock_type & MACBLK_INTRA))
    form_predictions(bx, by, macroblock_type, motion_type, PMV,
             motion_vert_field_sel, dmvector);
/*
  if(MParse.iDCT_Flag==IDCT_SSE2)
  {
    for (comp=0; comp<Mpeg_MacroBlk_Array_Limit-1; comp++)
    {
// MAYBE NEED PROCESSOR PACK
//        prefetchpointer=block[comp+1];
//        __asm
//        {
//          mov eax,[prefetchpointer]
//          prefetcht0 [eax]
//        };
//
        SSE2MMX_IDCT(block[comp]);
    };
    SSE2MMX_IDCT(block[comp]);
  }
  else
  {
    for (comp=0; comp<Mpeg_MacroBlk_Array_Limit; comp++)
    {
      idct_decoder(block[comp]);
//    prefetchpointer=block[comp+1];
//       __asm
//      {
//          mov eax,prefetchpointer
//          prefetch0 [eax]
//      };
//      idct_decoder(block[comp]);
    }
  }
*/


  switch (MParse.iDCT_Flag)
  {
    case IDCT_SSE2:
      for (comp=0; comp < Mpeg_MacroBlk_Array_Limit; comp++)
        SSE2MMX_IDCT(block[comp]);
      break;
    case IDCT_SSEMMX:
      for (comp=0; comp < Mpeg_MacroBlk_Array_Limit; comp++)
        SSEMMX_IDCT(block[comp]);
      break;

    case IDCT_MMX:
      for (comp=0; comp < Mpeg_MacroBlk_Array_Limit; comp++)
        MMX_IDCT(block[comp]);
      break;

    case IDCT_FPU:
      for (comp=0; comp < Mpeg_MacroBlk_Array_Limit; comp++)
        FPU_IDCT(block[comp]);
      break;

    case IDCT_REF:
      for (comp=0; comp<Mpeg_MacroBlk_Array_Limit; comp++)
        REF_IDCT(block[comp]);

      break;

  }

  Add_Block(Mpeg_MacroBlk_Array_Limit, bx, by, dct_type,
                    (macroblock_type & MACBLK_INTRA)==0);

}



//-----------------------------------------------------------
/* ISO/IEC 13818-2 section 7.6.6 */

static void skipped_macroblock(int dc_dct_pred[3], int PMV[2][2][2],
                               int *motion_type,
                 int motion_vert_field_sel[2][2],
                 int *macroblock_type)
{
  Clear_Block(Mpeg_MacroBlk_Array_Limit);

  /* reset intra_dc predictors */
  /* ISO/IEC 13818-2 section 7.2.1: DC coefficients in intra blocks */

  dc_dct_pred[0] = 0; dc_dct_pred[1] = 0;  dc_dct_pred[2] = 0; 

  /* reset motion vector predictors */
  /* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */

  if (MPEG_Pic_Type == P_TYPE)
  {
    PMV[0][0][0] = 0; PMV[0][0][1] = 0; PMV[1][0][0] = 0; PMV[1][0][1] = 0;
  }

  /* derive motion_type */

  if (MPEG_Pic_Structure == FULL_FRAME_PIC)
     *motion_type = MC_FRAME;
  else
  {
    *motion_type = MC_FIELD;
    motion_vert_field_sel[0][0]
            = motion_vert_field_sel[0][1]
            = (MPEG_Pic_Structure == BOTTOM_FIELD);
  }

  /* clear MACBLK_INTRA */

  *macroblock_type &= ~MACBLK_INTRA;
}


//---------------------------------------------------------------
/* return==-1 means go to next picture */
/* the expression "start of slice" is used throughout the normative
   body of the MPEG specification */

static int Pic_Decode_Slice_START(int *MBA, int *MBAinc,
              int dc_dct_pred[3], int PMV[2][2][2])
{
  unsigned int code;
  int slice_vert_pos_ext;

  code = GetB_Show_Next_Start_Code(0);

  /*
#ifdef DBG_RJ
  if (DBGflag)
  {
       sprintf(szBuffer,"   Slice=%d", code);
       DBGout(szBuffer);
  }
#endif
  */

  if (code < SLICE_START_CODE_MIN  ||  code > SLICE_START_CODE_MAX)
  {
    // only slice headers are allowed in Picture data
//    MParse.Fault_Flag = 11;    // Invalid start code within  Picture data
//    MParse.Fault_More = code;
    if ( MParse.SystemStream_Flag > 0 // Allow for broken packet structure
    &&   code == PACK_START_CODE
    &&   MParse.Fault_Flag < CRITICAL_ERROR_LEVEL
    && ! MParse.Stop_Flag)
    {
        Get_Next_Packet();
        code = GetB_Show_Next_Start_Code(0);
    }

  }

  code = Get_Bits(32);
  if (code < SLICE_START_CODE_MIN  ||  code > SLICE_START_CODE_MAX)
  {
    // only slice headers are allowed in Picture data
    if (MParse.Fault_Flag < CRITICAL_ERROR_LEVEL && ! MParse.Stop_Flag )
    {
        MParse.Fault_Prev = MParse.Fault_Flag;
        MParse.Fault_Flag = 11; // Invalid start code within  Picture data
        MParse.Fault_More = code;
    }
    return -1;
  }

  /* decode slice header (may change quantizer_scale) */

  slice_vert_pos_ext = slice_header();

  /* decode macroblock address increment */

  *MBAinc = Get_macroblock_address_increment();
  if (MParse.Fault_Flag)
    return -1;

  /* set current location */
  /* NOTE: the arithmetic used to derive macroblock_address below is
     equivalent to ISO/IEC 13818-2 section 6.3.17: Macroblock */

  *MBA = ((slice_vert_pos_ext << 7) + (code & 255) - 1) * mb_width
                                    + *MBAinc - 1;
  //if (*MBA > Coded_Pic_Height) // Trying to stop table overflow
  //    *MBA = Coded_Pic_Height; //  but needs more analysis...

  *MBAinc = 1;  // first macroblock in slice: not skipped

  /* reset all DC coefficient and motion vector predictors */
  /* ISO/IEC 13818-2 section 7.2.1: DC coefficients in intra blocks */

  dc_dct_pred[0] = dc_dct_pred[1] = dc_dct_pred[2] = 0;

  /* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */

  PMV[0][0][0] = PMV[0][0][1] = PMV[1][0][0] = PMV[1][0][1] = 0;
  PMV[0][1][0] = PMV[0][1][1] = PMV[1][1][0] = PMV[1][1][1] = 0;

  /* successfull: trigger decode macroblocks in slice */
  return 1;
}



//----------------------------------------------------------------------
/* ISO/IEC 13818-2 sections 7.2 through 7.5 */

static int decode_macroblock(int *macroblock_type,
            int *motion_type, int *dct_type,
            int PMV[2][2][2], int dc_dct_pred[3],
            int motion_vert_field_sel[2][2], int dmvector[2])
{

  int quantizer_scale_code, comp, motion_vector_count, mv_format;
  int dmv, mvscale, coded_block_pattern;

  /* ISO/IEC 13818-2 section 6.3.17.1: Macroblock modes */

  macroblock_modes(macroblock_type, motion_type,
                   &motion_vector_count, &mv_format,
                   &dmv, &mvscale, dct_type);

  if (MParse.Fault_Flag) return 0; // trigger: go to next slice

  if (*macroblock_type & MACBLK_QUANT)
  {
    quantizer_scale_code = Get_Bits(5);

    /* ISO/IEC 13818-2 section 7.4.2.2: Quantizer scale factor */
    quantizer_scale = MPEG_Pic_q_scale_type ?
                      Non_Linear_quantizer_scale[quantizer_scale_code]
                                            : (quantizer_scale_code << 1);
  }

  /* ISO/IEC 13818-2 section 6.3.17.2: Motion vectors */
  /* decode forward motion vectors */

  if ((*macroblock_type & MACBLK_MOTION_FWD)
    || ((*macroblock_type & MACBLK_INTRA)
          && MPEG_Pic_concealment_motion_vectors))

    motion_vectors(PMV, dmvector, motion_vert_field_sel, 0,
                        motion_vector_count, mv_format,
                        MPEG_Pic_f_code[0][0] - 1,
                        MPEG_Pic_f_code[0][1] - 1,
                        dmv, mvscale);

  if (MParse.Fault_Flag)
    return 0; // trigger: go to next slice

  /* decode backward motion vectors */

  if (*macroblock_type & MACBLK_MOTION_BWD)
    motion_vectors(PMV, dmvector, motion_vert_field_sel, 1,
                        motion_vector_count,mv_format,
                        MPEG_Pic_f_code[1][0] - 1,
                        MPEG_Pic_f_code[1][1] - 1,
                        0, mvscale);

  if (MParse.Fault_Flag)
    return 0;  // trigger: go to next slice

  if ((*macroblock_type & MACBLK_INTRA)
          && MPEG_Pic_concealment_motion_vectors)
     InputBuffer_Flush(1);  // marker bit

  /* macroblock_pattern */
  /* ISO/IEC 13818-2 section 6.3.17.4: Coded block pattern */

  if (*macroblock_type & MACBLK_PATTERN)
  {
    coded_block_pattern = Get_coded_block_pattern();

    if (MPEG_Seq_chroma_format == CHROMA422)
             coded_block_pattern = (coded_block_pattern<<2) | Get_Bits(2);
    else
    if (MPEG_Seq_chroma_format == CHROMA444)
             coded_block_pattern = (coded_block_pattern<<6) | Get_Bits(6);
  }
  else
             coded_block_pattern = (*macroblock_type & MACBLK_INTRA) ?
                                                  (1<<Mpeg_MacroBlk_Array_Limit)-1 : 0;

  if (MParse.Fault_Flag)
    return 0; // trigger: go to next slice

  Clear_Block(Mpeg_MacroBlk_Array_Limit);

  /* decode blocks */

  for (comp = 0; comp < Mpeg_MacroBlk_Array_Limit; comp++)
  {
    if (coded_block_pattern & (1<<(Mpeg_MacroBlk_Array_Limit-1-comp)))
    {
      if (*macroblock_type & MACBLK_INTRA)
        Decode_MPEG2_Intra_Block(comp, dc_dct_pred);
      else
        Decode_MPEG2_Non_Intra_Block(comp);

      if (MParse.Fault_Flag)
        return 0; // trigger: go to next slice
    }
  }

  /* reset intra_dc predictors */
  /* ISO/IEC 13818-2 section 7.2.1: DC coefficients in intra blocks */

  if (!(*macroblock_type & MACBLK_INTRA))
    dc_dct_pred[0] = dc_dct_pred[1] = dc_dct_pred[2] = 0;

  /* reset motion vector predictors */

  if ((*macroblock_type & MACBLK_INTRA)
    && !MPEG_Pic_concealment_motion_vectors)
  {
    /* intra mb without concealment motion vectors */
    /* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */
    PMV[0][0][0] = PMV[0][0][1] = PMV[1][0][0] = PMV[1][0][1] = 0;
    PMV[0][1][0] = PMV[0][1][1] = PMV[1][1][0] = PMV[1][1][1] = 0;
  }

  /* special "No_MC" macroblock_type case */
  /* ISO/IEC 13818-2 section 7.6.3.5: Prediction in P pictures */

  if (MPEG_Pic_Type == P_TYPE
    && !(*macroblock_type & (MACBLK_MOTION_FWD|MACBLK_INTRA)))
  {
    /* non-intra mb without forward mv in a P picture */
    /* ISO/IEC 13818-2 section 7.6.3.4: Resetting motion vector predictors */

    PMV[0][0][0] = PMV[0][0][1] = PMV[1][0][0] = PMV[1][0][1] = 0;

    /* derive motion_type */
    /* ISO/IEC 13818-2 section 6.3.17.1: Macroblock modes, frame_motion_type */

    if (MPEG_Pic_Structure == FULL_FRAME_PIC)
      *motion_type = MC_FRAME;
    else
    {
      *motion_type = MC_FIELD;
      motion_vert_field_sel[0][0]
                   = (MPEG_Pic_Structure == BOTTOM_FIELD);
    }
  }

  /* successfully decoded macroblock */
  return 1 ;
}



//------------------------------------------------------------
/* decode one intra coded MPEG-2 block */

static void Decode_MPEG2_Intra_Block(int iP_comp, int dc_dct_pred[])
{

  int val = 0 ;            // Default value to allow for bad data
  int i, j, sign, *qmat, iW_comp;
  unsigned int code;
  DCTtab *tab;
  short *bp;

  if      (iP_comp > 15) iW_comp = 15 ;  // RJ TRAP BAD count PARM
  else if (iP_comp < 00) iW_comp = 00  ;
  else                   iW_comp = iP_comp  ;


  bp = block[iW_comp];
  qmat = (iW_comp<4 || MPEG_Seq_chroma_format < CHROMA422) // == CHROMA420)
        ? MPEG_Seq_intra_quantizer_matrix : chroma_intra_quantizer_matrix;


  /* ISO/IEC 13818-2 section 7.2.1: decode DC coefficients */

  switch ( cc_table[iW_comp] )
  {
    case 0:
      val = (dc_dct_pred[0]+= Get_Luma_DC_dct_diff());
      break;

    case 1:
      val = (dc_dct_pred[1]+= Get_Chroma_DC_dct_diff());
      break;

    case 2:
      val = (dc_dct_pred[2]+= Get_Chroma_DC_dct_diff());
      break;


    default:
      val = 0 ; //  RJ Allow for invalid data

  }

  bp[0] = (short)(val << (3-MPEG_Pic_intra_dc_precision)) ;  //RJ CAREFUL OF CAST

  /* decode AC coefficients */

  for (i=1; ; i++)
  {
    code = Show_Bits(16);

    if (code>=16384 && !MPEG_Pic_intra_vlc_format)
      tab = &DCTtabnext[(code>>12)-4];
    else
    if (code>=1024)
    {
      if (MPEG_Pic_intra_vlc_format)
        tab = &DCTtab0a[(code>>8)-4];
      else
        tab = &DCTtab0[(code>>8)-4];
    }
    else
    if (code>=512)
    {
      if (MPEG_Pic_intra_vlc_format)
        tab = &DCTtab1a[(code>>6)-8];
      else
        tab = &DCTtab1[(code>>6)-8];
    }
    else
    if (code>=256)
      tab = &DCTtab2[(code>>4)-16];
    else
    if (code>=128)
      tab = &DCTtab3[(code>>3)-16];
    else
    if (code>=64)
      tab = &DCTtab4[(code>>2)-16];
    else
    if (code>=32)
      tab = &DCTtab5[(code>>1)-16];
    else
    if (code>=16)
      tab = &DCTtab6[code-16];
    else
    {
      if (MParse.Fault_Flag < CRITICAL_ERROR_LEVEL && ! MParse.Stop_Flag )
      {
          MParse.Fault_Prev = MParse.Fault_Flag;
          MParse.Fault_Flag = 1;  // Invalid DCT table code
          MParse.Fault_More = code;
      }
      return;
    }

    InputBuffer_Flush(tab->len);

    if (tab->run < 64)
    {
      i+= tab->run;
      val = tab->level;
      sign = Get_Bits(1);
    }
    else
    if (tab->run == 64) /* end_of_block */
      return;
    else // escape
    {
      i+= Get_Bits(6);
      if (Mpeg_SEQ_Version != 2) // Mpeg-1 Run Level Escape - different amounts of FLC
      {
        val = Get_Bits(8);
        if (val==0)
            val = Get_Bits(8);
        else
        if (val==128)
            val = Get_Bits(8) - 256;
        else
        if (val>128)
            val -= 256;

        if((sign = (val<0))) // assignment within conditional expression
           val = -val;
      }
      else
      {
        val = Get_Bits(12);
        sign = (val>=2048) ;
        if (sign)
          val = 4096 - val;
      }

    }
    if (i > 63) // Allow for bad data
        i = 63;
    j = scan[MPEG_Pic_alternate_scan][i];

    val = (val * quantizer_scale * qmat[j]) >> 4;
    bp[j] = (short)(sign ? -val : val);          //RJ CAREFUL WITH CAST
  }
}


//-------------------------------------------------------------------
/* decode one non-intra coded MPEG-2 block */

static void Decode_MPEG2_Non_Intra_Block(int comp)
{
  int val, i, j, sign, *qmat;
  unsigned int code;
  DCTtab *tab;
  short *bp;

  bp = block[comp];
  qmat = (comp < 4 || MPEG_Seq_chroma_format < CHROMA422) // == CHROMA420)
    ? MPEG_Seq_non_intra_quantizer_matrix
    : chroma_non_intra_quantizer_matrix;

  /* decode AC coefficients */

  for (i=0; ; i++)
  {
    code = Show_Bits(16);

    if (code>=16384)
    {
      if (i==0)
        tab = &DCTtabfirst[(code>>12)-4];
      else
        tab = &DCTtabnext[(code>>12)-4];
    }
    else
    if (code>=1024)
      tab = &DCTtab0[(code>>8)-4];
    else
    if (code>=512)
      tab = &DCTtab1[(code>>6)-8];
    else
    if (code>=256)
      tab = &DCTtab2[(code>>4)-16];
    else
    if (code>=128)
      tab = &DCTtab3[(code>>3)-16];
    else
    if (code>=64)
      tab = &DCTtab4[(code>>2)-16];
    else
    if (code>=32)
      tab = &DCTtab5[(code>>1)-16];
    else
    if (code>=16)
      tab = &DCTtab6[code-16];
    else
    {
      if (MParse.Fault_Flag < CRITICAL_ERROR_LEVEL && ! MParse.Stop_Flag )
      {
          MParse.Fault_Prev = MParse.Fault_Flag;
          MParse.Fault_Flag = 1;    // Invalid DCT table code
          MParse.Fault_More = code;
      }
      return;
    }

    InputBuffer_Flush(tab->len);

    if (tab->run < 64)
    {
      i+= tab->run;
      val = tab->level;
      sign = Get_Bits(1);
    }
    else
    if (tab->run == 64) /* end_of_block */
      return;
    else           /* escape */
    {
      i+= Get_Bits(6);
      if (Mpeg_SEQ_Version != 2) // Mpeg-1 Run Level Escape - different amounts of FLC
      {
        val = Get_Bits(8);
        if (val==0)
            val = Get_Bits(8);
        else
        if (val==128)
            val = Get_Bits(8) - 256;
        else
        if (val>128)
            val -= 256;

        if((sign = (val<0)))  //assignment within conditional expression
           val = -val;
      }
      else
      {
        val = Get_Bits(12);
        sign = (val>=2048) ;
        if (sign)
          val = 4096 - val;
      }

    }
    if (i > 63) // Allow for bad data
        i = 63;
    j = scan[MPEG_Pic_alternate_scan][i];

    val = (((val<<1)+1) * quantizer_scale * qmat[j]) >> 5;
    bp[j] = (short)(sign ? -val : val);         //RJ CAREFUL WITH CAST
  }
}

//-------------------------------------------------------------------

static int Get_macroblock_type()
{
  int macroblock_type = 0;      // RJ Default value to allow for Bad Data

  switch (MPEG_Pic_Type)
  {
    case I_TYPE:
      macroblock_type = Get_I_macroblock_type();
      break;

    case P_TYPE:
      macroblock_type = Get_P_macroblock_type();
      break;

    case B_TYPE:
      macroblock_type = Get_B_macroblock_type();
      break;

    default:                             // ALLOW FOR BAD DATA
      sprintf(szMsgTxt, "GetPic-BAD Pic Type: %d",
                      MPEG_Pic_Type) ;
      macroblock_type = 0 ;
      MParse.Fault_Flag = 42 ;  // Unknown Pic type - not I,P,B
  }

  return macroblock_type;
}


//---------------------------------------------------------------

static int Get_I_macroblock_type()
{
  if (Get_Bits(1))
    return 1;

  if (!Get_Bits(1))
    MParse.Fault_Flag = 2;   // Invalid I Macro Block type

  return 17;
}


//--------------------------------------------------------------

static int Get_P_macroblock_type()
{
  int code;

  if ((code = Show_Bits(6))>=8)
  {
    code >>= 3;
    InputBuffer_Flush(PMBtab0[code].len);

    return PMBtab0[code].val;
  }

  if (code==0)
  {
    MParse.Fault_Flag = 2;   // Invalid P Macro Block type
    return 0;
  }

  InputBuffer_Flush(PMBtab1[code].len);

  return PMBtab1[code].val;
}


//-----------------------------------------------------------------


static int Get_B_macroblock_type()
{
  int code;

  if ((code = Show_Bits(6))>=8)
  {
    code >>= 2;
    InputBuffer_Flush(BMBtab0[code].len);

    return BMBtab0[code].val;
  }

  if (code==0)
  {
    MParse.Fault_Flag = 2;   // Invalid B Macro Block type
    return 0;
  }

  InputBuffer_Flush(BMBtab1[code].len);

  return BMBtab1[code].val;
}


//---------------------------------------------------------------


static int Get_coded_block_pattern()
{
  int code;

  if ((code = Show_Bits(9))>=128)
  {
    code >>= 4;
    InputBuffer_Flush(CBPtab0[code].len);

    return CBPtab0[code].val;
  }

  if (code>=8)
  {
    code >>= 1;
    InputBuffer_Flush(CBPtab1[code].len);

    return CBPtab1[code].val;
  }

  if (code<1)
  {
    MParse.Fault_Flag = 3;   // Invalid Coded Block Pattern
    return 0;
  }

  InputBuffer_Flush(CBPtab2[code].len);

  return CBPtab2[code].val;
}



//------------------------------------------------------------------

static int Get_macroblock_address_increment()
{
  int code, val;

  val = 0;

  while ( (code = Show_Bits(11)) < 24)
  {
    if (code!=15) /* if not macroblock_stuffing */
    {
      if (code==8) /* if macroblock_escape */
        val+= 33;
      else
      {
        MParse.Fault_Flag = 4;    //Invalid macroblock_address_increment
        return 1;
      }
    }
    InputBuffer_Flush(11);
  }

  /* macroblock_address_increment == 1 */
  /* ('1' is in the MSB position of the lookahead) */

  if (code >= 1024)
  {
    InputBuffer_Flush(1);
    return val + 1;
  }

  /* codes 00010 ... 011xx */
  if (code >= 128)
  {
    /* remove leading zeros */
    code >>= 6;
    InputBuffer_Flush(MBAtab1[code].len);

    return val + MBAtab1[code].val;
  }

  /* codes 00000011000 ... 0000111xxxx */
  code -= 24; /* remove common base */
  InputBuffer_Flush(MBAtab2[code].len);

  return val + MBAtab2[code].val;
}


//-------------------------------------------------------------------
/*
   parse VLC and perform dct_diff arithmetic.
   MPEG-2:  ISO/IEC 13818-2 section 7.2.1

   Note: the arithmetic here is presented more elegantly than
   the spec, yet the results, dct_diff, are the same.
*/

static int Get_Luma_DC_dct_diff()
{

  int code, size, dct_diff;

  /* decode length */
  code = Show_Bits(5);

  if (code<31)
  {
    size = DClumtab0[code].val;
    InputBuffer_Flush(DClumtab0[code].len);
  }
  else
  {
    code = Show_Bits(9) - 0x1f0;
    size = DClumtab1[code].val;
    InputBuffer_Flush(DClumtab1[code].len);
  }

  if (size==0)
    dct_diff = 0;
  else
  {
    dct_diff = Get_Bits(size);

    if ((dct_diff & (1<<(size-1)))==0)
      dct_diff-= (1<<size) - 1;
  }

  return dct_diff;
}




//---------------------------------------------------------
static int Get_Chroma_DC_dct_diff()
{
  int code, size, dct_diff;

  /* decode length */
  code = Show_Bits(5);

  if (code<31)
  {
    size = DCchromtab0[code].val;
    InputBuffer_Flush(DCchromtab0[code].len);
  }
  else
  {
    code = Show_Bits(10) - 0x3e0;
    size = DCchromtab1[code].val;
    InputBuffer_Flush(DCchromtab1[code].len);
  }

  if (size==0)
    dct_diff = 0;
  else
  {
    dct_diff = Get_Bits(size);

    if ((dct_diff & (1<<(size-1)))==0)
      dct_diff-= (1<<size) - 1;
  }

  return dct_diff;
}




//-------------------------------------------------------------------
static void form_predictions(int bx, int by, int macroblock_type,
                             int motion_type,
           int PMV[2][2][2], int motion_vert_field_sel[2][2],
           int dmvector[2])
{

  static int ibCurr_Field_Id;
  static unsigned char **predframe;
  static int DMV[2][2];
  static int stw;

  stw = 0;

  if ((macroblock_type & MACBLK_MOTION_FWD)
              || MPEG_Pic_Type==P_TYPE)
  {
    if (MPEG_Pic_Structure == FULL_FRAME_PIC)
    {
      if (motion_type == MC_FRAME
            || !(macroblock_type & MACBLK_MOTION_FWD))
      {
        /* frame-based prediction (broken into top and bottom halves
           for spatial scalability prediction purposes) */
        form_prediction(fwd_ref_frame, 0,
                        curr_frame,  0,
                    Coded_Pic_Width,
                    DOUBLE_WIDTH,
                    16, 8, bx, by,
                    PMV[0][0][0],
                    PMV[0][0][1], stw);

        form_prediction(fwd_ref_frame, 1,
                    curr_frame, 1,
                    Coded_Pic_Width,
                    DOUBLE_WIDTH,
                    16, 8, bx, by,
                    PMV[0][0][0],
                    PMV[0][0][1], stw);
      }
      else if (motion_type == MC_FIELD) /* field-based prediction */
      {
        /* top field prediction */
        form_prediction(fwd_ref_frame, motion_vert_field_sel[0][0],
                        curr_frame,    0,
                        DOUBLE_WIDTH,
                        DOUBLE_WIDTH,
                        16, 8, bx, by>>1,
                        PMV[0][0][0],
                        PMV[0][0][1]>>1, stw);

        /* bottom field prediction */
        form_prediction(fwd_ref_frame, motion_vert_field_sel[1][0],
                        curr_frame,    1,
                        DOUBLE_WIDTH,
                        DOUBLE_WIDTH,
                        16, 8, bx, by>>1,
                        PMV[1][0][0],
                        PMV[1][0][1]>>1, stw);
      }
      else if (motion_type==MC_DMV) /* dual prime prediction */
      {
        /* calculate derived motion vectors */
        Dual_Prime_Arithmetic(DMV, dmvector,
                              PMV[0][0][0],
                              PMV[0][0][1]>>1);

        /* predict top field from top field */

        form_prediction(fwd_ref_frame, 0,
                        curr_frame,    0,
                        DOUBLE_WIDTH,
                        DOUBLE_WIDTH,
                        16, 8, bx, by>>1,
                        PMV[0][0][0],
                        PMV[0][0][1]>>1, 0);

        /* predict and add to top field from bottom field */
        form_prediction(fwd_ref_frame, 1,
                        curr_frame, 0,
                        DOUBLE_WIDTH, DOUBLE_WIDTH,
                        16, 8, bx, by>>1,
                        DMV[0][0], DMV[0][1], 1);

        /* predict bottom field from bottom field */
        form_prediction(fwd_ref_frame, 1,
                        curr_frame,    1,
                        DOUBLE_WIDTH, DOUBLE_WIDTH,
                        16, 8, bx, by>>1,
                        PMV[0][0][0], PMV[0][0][1]>>1, 0);

        /* predict and add to bottom field from top field */
        form_prediction(fwd_ref_frame, 0,
                        curr_frame,    1,
                        DOUBLE_WIDTH, DOUBLE_WIDTH,
                        16, 8, bx, by>>1,
                        DMV[1][0], DMV[1][1], 1);
      }
      else
        MParse.Fault_Flag = 5;    // Invalid motion_type
    }


    else
    {
      /* field picture */
      ibCurr_Field_Id = (MPEG_Pic_Structure == BOTTOM_FIELD);

      /* determine which frame to use for prediction */
      if (MPEG_Pic_Type == P_TYPE
          && Second_Field
          && ibCurr_Field_Id!=motion_vert_field_sel[0][0])
        predframe = bwd_ref_frame;
      else
        predframe = fwd_ref_frame;


      if (motion_type==MC_FIELD
          || !(macroblock_type & MACBLK_MOTION_FWD))
      {
        form_prediction(predframe,  motion_vert_field_sel[0][0],
                    curr_frame, 0,
                    DOUBLE_WIDTH, DOUBLE_WIDTH,
                    16, 16, bx, by,
                    PMV[0][0][0], PMV[0][0][1], stw);
      }
      else if (motion_type==MC_16X8)
      {
        form_prediction(predframe,  motion_vert_field_sel[0][0],
                        curr_frame, 0,
                        DOUBLE_WIDTH, DOUBLE_WIDTH,
                        16, 8, bx, by,
                        PMV[0][0][0],
                        PMV[0][0][1], stw);

        if (MPEG_Pic_Type == P_TYPE
            && Second_Field
            && ibCurr_Field_Id!=motion_vert_field_sel[1][0])
          predframe = bwd_ref_frame;
        else
          predframe = fwd_ref_frame;

        form_prediction(predframe,  motion_vert_field_sel[1][0],
                        curr_frame, 0,
                        DOUBLE_WIDTH, DOUBLE_WIDTH,
                        16, 8, bx, by+8,
                        PMV[1][0][0],
                        PMV[1][0][1], stw);
      }
      else if (motion_type==MC_DMV)
      {
        if (Second_Field)
          predframe = bwd_ref_frame;
        else
          predframe = fwd_ref_frame;

        /* calculate derived motion vectors */
        Dual_Prime_Arithmetic(DMV, dmvector, PMV[0][0][0],
                                             PMV[0][0][1]);

        /* predict from field of same parity */
        form_prediction(fwd_ref_frame, ibCurr_Field_Id,
                        curr_frame,    0,
                        DOUBLE_WIDTH, DOUBLE_WIDTH,
                        16, 16, bx, by,
                        PMV[0][0][0],
                        PMV[0][0][1], 0);

        /* predict from field of opposite parity */
        form_prediction(predframe,  !ibCurr_Field_Id,
                        curr_frame, 0,
                        DOUBLE_WIDTH, DOUBLE_WIDTH,
                        16, 16, bx, by,
                        DMV[0][0], DMV[0][1], 1);
      }
      else
        MParse.Fault_Flag = 5;    // Invalid motion_type
    }

    stw = 1;
  }

  if (macroblock_type & MACBLK_MOTION_BWD)
  {
    if (MPEG_Pic_Structure==FULL_FRAME_PIC)
    {
      if (motion_type==MC_FRAME)
      {
        /* frame-based prediction */
        form_prediction(bwd_ref_frame, 0, curr_frame, 0,
          Coded_Pic_Width, DOUBLE_WIDTH, 16, 8, bx, by,
          PMV[0][1][0], PMV[0][1][1], stw);

        form_prediction(bwd_ref_frame, 1, curr_frame, 1,
          Coded_Pic_Width, DOUBLE_WIDTH, 16, 8, bx, by,
          PMV[0][1][0], PMV[0][1][1], stw);
      }
      else /* field-based prediction */
      {
        /* top field prediction */
        form_prediction(bwd_ref_frame,
          motion_vert_field_sel[0][1],
          curr_frame, 0,
          DOUBLE_WIDTH, DOUBLE_WIDTH, 16, 8,
          bx, by>>1, PMV[0][1][0], PMV[0][1][1]>>1, stw);

        /* bottom field prediction */
        form_prediction(bwd_ref_frame,
                    motion_vert_field_sel[1][1],
                    curr_frame, 1,
                    DOUBLE_WIDTH,
                    DOUBLE_WIDTH, 16, 8, bx, by>>1,
                    PMV[1][1][0], PMV[1][1][1]>>1, stw);
      }
    }
    else
    {
      /* field picture */
      if (motion_type==MC_FIELD)
      {
        /* field-based prediction */
        form_prediction(bwd_ref_frame,
                        motion_vert_field_sel[0][1],
                        curr_frame, 0,
                        DOUBLE_WIDTH, DOUBLE_WIDTH,
                        16, 16, bx, by,
                        PMV[0][1][0], PMV[0][1][1], stw);
      }
      else if (motion_type==MC_16X8)
      {
        form_prediction(bwd_ref_frame,
                        motion_vert_field_sel[0][1],
                        curr_frame, 0,
                        DOUBLE_WIDTH,
                        DOUBLE_WIDTH, 16, 8, bx, by,
                        PMV[0][1][0], PMV[0][1][1], stw);

        form_prediction(bwd_ref_frame,
                        motion_vert_field_sel[1][1],
                        curr_frame, 0,
                        DOUBLE_WIDTH,
                        DOUBLE_WIDTH, 16, 8,
                        bx, by+8, PMV[1][1][0], PMV[1][1][1], stw);
      }
      else
        MParse.Fault_Flag = 5;    // Invalid motion_type
    }
  }

  __asm emms;
}


//-------------------------------------------------------

static void form_prediction(
              unsigned char *src[], int sfield,
              unsigned char *dst[], int dfield,
              int lx, int lx2,
              int w, int h,
              int x, int y,
              int dx, int dy,
              int average_flag)
{

// START EXPERIMENT  5121
// unsigned char *Src_Offset, *Dst_Offset;

  //  Trap boundary errors
  // (There could be a better place to do this, such as where PMV is set)

  if (dx > Coded_Pic_Width)
      dx = Coded_Pic_Width;
  else
  if (dx < -Coded_Pic_Width)
      dx =  Coded_Pic_Width;

  if (dy > Coded_Pic_Height*2)
      dy = Coded_Pic_Height*2;
  else
  if (dy < -Coded_Pic_Height*2)
      dy =  Coded_Pic_Height*2;





// END EXPERIMENT

  form_component_prediction(src[0]+(sfield?lx2>>1:0),
                            dst[0]+(dfield?lx2>>1:0),
                            lx, lx2, w, h, x, y, dx, dy,
                            average_flag);

  if (MPEG_Seq_chroma_format != CHROMA444)
  {
    lx>>=1; lx2>>=1; w>>=1; x>>=1; dx/=2;
  }

  if (MPEG_Seq_chroma_format  < CHROMA422) // == CHROMA420)
  {
    h>>=1; y>>=1; dy/=2;
  }

  /* Cb */
  form_component_prediction(src[1]+(sfield?lx2>>1:0),
                            dst[1]+(dfield?lx2>>1:0),
                            lx, lx2, w, h, x, y, dx, dy, average_flag);

  /* Cr */
  form_component_prediction(src[2]+(sfield?lx2>>1:0),
                            dst[2]+(dfield?lx2>>1:0),
                            lx, lx2, w, h, x, y, dx, dy, average_flag);
}


//--------------------------------------------------------------

/* ISO/IEC 13818-2 section 7.6.4: Forming predictions */

static void form_component_prediction(unsigned char *src,
                                      unsigned char *dst,
                      int lx, int lx2,
                      int  w, int  h,
                      int  x, int  y,
                      int dx, int dy, int average_flag)
{
  static const __int64 mmmask_0001 = 0x0001000100010001;
  static const __int64 mmmask_0002 = 0x0002000200020002;
  static const __int64 mmmask_0003 = 0x0003000300030003;
  static const __int64 mmmask_0006 = 0x0006000600060006;

  unsigned char *s = src + lx * (y + (dy>>1)) + x + (dx>>1);
  unsigned char *d = dst + lx * y + x;
  int flag = (average_flag<<2) + ((dx & 1)<<1) + (dy & 1);

  switch (flag)
  {
    case 0:
      // d[i] = s[i];
      __asm
      {
        mov     eax, [s]
        mov     ebx, [d]
        mov     esi, 0x00
        mov     edi, [h]

        // RJ  This area is subject to crashes
        // probably needs more validation of mpeg data
        // before executing the macro block
mc0:
        movq    mm1, [eax+esi]   // <=== CRASH POINT
        movq    [ebx+esi], mm1
        add     esi, 0x08
        cmp     esi, [w]
        jl      mc0
/*
        add     esi, 0x08
        cmp     esi, [w]   // Why is the CMP separated from the "jl mc0" ?
        movq    [ebx+esi-8], mm1
        jl      mc0
*/
        add     eax, [lx2]
        add     ebx, [lx2]
        mov     esi, 0x00
        dec     edi
//      cmp     edi, 0x00  // MOVED "dec edi" DOWN HERE so don't need CMP 5106
        jg      mc0

      }
      break;

    case 1:
      // d[i] = (s[i]+s[i+lx]+1)>>1;
      __asm
      {
        pxor    mm0, mm0
        movq    mm7, [mmmask_0001]
        mov     eax, [s]
        mov     ebx, [d]
        mov     ecx, eax
        add     ecx, [lx]
        mov     esi, 0x00
        mov     edi, [h]
mc1:
        movq    mm1, [eax+esi]
        movq    mm2, [ecx+esi]

        movq    mm3, mm1
        movq    mm4, mm2

        punpcklbw mm1, mm0
        punpckhbw mm3, mm0
        punpcklbw mm2, mm0
        punpckhbw mm4, mm0

        paddsw    mm1, mm2
        paddsw    mm3, mm4

        paddsw    mm1, mm7
        paddsw    mm3, mm7

        psrlw   mm1, 1
        psrlw   mm3, 1

        packuswb  mm1, mm0
        packuswb  mm3, mm0

        psllq   mm3, 32
        por     mm1, mm3

        movq    [ebx+esi], mm1
        add     esi, 0x08
        cmp     esi, [w]
        jl      mc1

/*
        add     esi, 0x08
        cmp     esi, [w]
        movq    [ebx+esi-8], mm1
        jl      mc1
*/
        add     eax, [lx2]
        add     ebx, [lx2]
        add     ecx, [lx2]
        mov     esi, 0x00
        dec     edi
//        cmp     edi, 0x00
        jg      mc1
      }
      break;

    case 2:
      // d[i] = (s[i]+s[i+1]+1)>>1;
      __asm
      {
        pxor    mm0, mm0
        movq    mm7, [mmmask_0001]
        mov     eax, [s]
        mov     ebx, [d]
        mov     esi, 0x00
        mov     edi, [h]
mc2:
        movq    mm1, [eax+esi]
        movq    mm2, [eax+esi+1]

        movq    mm3, mm1
        movq    mm4, mm2

        punpcklbw mm1, mm0
        punpckhbw mm3, mm0

        punpcklbw mm2, mm0
        punpckhbw mm4, mm0

        paddsw    mm1, mm2
        paddsw    mm3, mm4

        paddsw    mm1, mm7
        paddsw    mm3, mm7

        psrlw   mm1, 1
        psrlw   mm3, 1

        packuswb  mm1, mm0
        packuswb  mm3, mm0

        psllq   mm3, 32
        por     mm1, mm3

        movq    [ebx+esi], mm1
        add     esi, 0x08
        cmp     esi, [w]
        jl      mc2
/*
        add     esi, 0x08
        cmp     esi, [w]
        movq    [ebx+esi-8], mm1
        jl      mc2
*/

        add     eax, [lx2]
        add     ebx, [lx2]
        mov     esi, 0x00
        dec     edi
//        cmp     edi, 0x00
        jg      mc2
      }
      break;

    case 3:
      // d[i] = (s[i]+s[i+1]+s[i+lx]+s[i+lx+1]+2)>>2;
      __asm
      {
        pxor    mm0, mm0
        movq    mm7, [mmmask_0002]
        mov     eax, [s]
        mov     ebx, [d]
        mov     ecx, eax
        add     ecx, [lx]
        mov     esi, 0x00
        mov     edi, [h]
mc3:
        movq    mm1, [eax+esi]
        movq    mm2, [eax+esi+1]
        movq    mm3, mm1
        movq    mm4, mm2

        punpcklbw mm1, mm0
        punpckhbw mm3, mm0

        punpcklbw mm2, mm0
        punpckhbw mm4, mm0

        paddsw    mm1, mm2
        paddsw    mm3, mm4

        movq    mm5, [ecx+esi]
        paddsw    mm1, mm7

        movq    mm6, [ecx+esi+1]
        paddsw    mm3, mm7

        movq    mm2, mm5
        movq    mm4, mm6

        punpcklbw mm2, mm0
        punpckhbw mm5, mm0

        punpcklbw mm4, mm0
        punpckhbw mm6, mm0

        paddsw    mm2, mm4
        paddsw    mm5, mm6

        paddsw    mm1, mm2
        paddsw    mm3, mm5

        psrlw   mm1, 2
        psrlw   mm3, 2

        packuswb  mm1, mm0
        packuswb  mm3, mm0

        psllq   mm3, 32
        por     mm1, mm3

        movq    [ebx+esi], mm1
        add     esi, 0x08
        cmp     esi, [w]
        jl      mc3
/*
        add     esi, 0x08
        cmp     esi, [w]
        movq    [ebx+esi-8], mm1
        jl      mc3
*/

        add     eax, [lx2]
        add     ebx, [lx2]
        add     ecx, [lx2]
        mov     esi, 0x00
        dec     edi
//        cmp     edi, 0x00
        jg      mc3
      }
      break;

    case 4:
      // d[i] = (s[i]+d[i]+1)>>1;
      __asm
      {
        pxor    mm0, mm0
        movq    mm7, [mmmask_0001]
        mov     eax, [s]
        mov     ebx, [d]
        mov     esi, 0x00
        mov     edi, [h]
mc4:
        movq    mm1, [eax+esi]
        movq    mm2, [ebx+esi]
        movq    mm3, mm1
        movq    mm4, mm2

        punpcklbw mm1, mm0
        punpckhbw mm3, mm0

        punpcklbw mm2, mm0
        punpckhbw mm4, mm0

        paddsw    mm1, mm2
        paddsw    mm3, mm4

        paddsw    mm1, mm7
        paddsw    mm3, mm7

        psrlw   mm1, 1
        psrlw   mm3, 1

        packuswb  mm1, mm0
        packuswb  mm3, mm0

        psllq   mm3, 32
        por     mm1, mm3

        movq    [ebx+esi], mm1
        add     esi, 0x08
        cmp     esi, [w]
        jl      mc4
/*
        add     esi, 0x08
        cmp     esi, [w]
        movq    [ebx+esi-8], mm1
        jl      mc4
*/

        add     eax, [lx2]
        add     ebx, [lx2]
        mov     esi, 0x00
        dec     edi
//        cmp     edi, 0x00
        jg      mc4
      }
      break;

    case 5:
      // d[i] = ((d[i]<<1) + s[i]+s[i+lx] + 3)>>2;
      __asm
      {
        pxor    mm0, mm0
        movq    mm7, [mmmask_0003]
        mov     eax, [s]
        mov     ebx, [d]
        mov     ecx, eax
        add     ecx, [lx]
        mov     esi, 0x00
        mov     edi, [h]
mc5:                             // RJ - Some crashes happen here
        movq    mm1, [eax+esi]
        movq    mm2, [ecx+esi]
        movq    mm3, mm1
        movq    mm4, mm2

        punpcklbw mm1, mm0
        punpckhbw mm3, mm0

        punpcklbw mm2, mm0
        punpckhbw mm4, mm0

        paddsw    mm1, mm2
        paddsw    mm3, mm4

        movq    mm5, [ebx+esi]

        paddsw    mm1, mm7
        paddsw    mm3, mm7

        movq    mm6, mm5
        punpcklbw mm5, mm0
        punpckhbw mm6, mm0

        psllw   mm5, 1
        psllw   mm6, 1

        paddsw    mm1, mm5
        paddsw    mm3, mm6

        psrlw   mm1, 2
        psrlw   mm3, 2

        packuswb  mm1, mm0
        packuswb  mm3, mm0

        psllq   mm3, 32
        por     mm1, mm3

        movq    [ebx+esi], mm1
        add     esi, 0x08
        cmp     esi, [w]
        jl      mc5
/*
        add     esi, 0x08
        cmp     esi, [w]
        movq    [ebx+esi-8], mm1
        jl      mc5
*/

        add     eax, [lx2]
        add     ebx, [lx2]
        add     ecx, [lx2]
        mov     esi, 0x00
        dec     edi
//        cmp     edi, 0x00
        jg      mc5
      }
      break;

    case 6:
      // d[i] = ((d[i]<<1) + s[i]+s[i+1] + 3) >> 2;
      __asm
      {
        pxor    mm0, mm0
        movq    mm7, [mmmask_0003]
        mov     eax, [s]
        mov     ebx, [d]
        mov     esi, 0x00
        mov     edi, [h]
mc6:
        movq    mm1, [eax+esi]
        movq    mm2, [eax+esi+1]
        movq    mm3, mm1
        movq    mm4, mm2

        punpcklbw mm1, mm0
        punpckhbw mm3, mm0

        punpcklbw mm2, mm0
        punpckhbw mm4, mm0

        paddsw    mm1, mm2
        paddsw    mm3, mm4

        movq    mm5, [ebx+esi]

        paddsw    mm1, mm7
        paddsw    mm3, mm7

        movq    mm6, mm5
        punpcklbw mm5, mm0
        punpckhbw mm6, mm0

        psllw   mm5, 1
        psllw   mm6, 1

        paddsw    mm1, mm5
        paddsw    mm3, mm6

        psrlw   mm1, 2
        psrlw   mm3, 2

        packuswb  mm1, mm0
        packuswb  mm3, mm0

        psllq   mm3, 32
        por     mm1, mm3

        movq    [ebx+esi], mm1
        add     esi, 0x08
        cmp     esi, [w]
        jl      mc6
/*
        add     esi, 0x08
        cmp     esi, [w]
        movq    [ebx+esi-8], mm1
        jl      mc6
*/

        add     eax, [lx2]
        add     ebx, [lx2]
        mov     esi, 0x00
        dec     edi
//        cmp     edi, 0x00
        jg      mc6
      }
      break;

    case 7:
      // d[i] = ((d[i]<<2) + s[i]+s[i+1]+s[i+lx]+s[i+lx+1] + 6)>>3;
      __asm
      {
        pxor    mm0, mm0
        movq    mm7, [mmmask_0006]
        mov     eax, [s]
        mov     ebx, [d]
        mov     ecx, eax
        add     ecx, [lx]
        mov     esi, 0x00
        mov     edi, [h]
mc7:
        movq    mm1, [eax+esi]
        movq    mm2, [eax+esi+1]
        movq    mm3, mm1
        movq    mm4, mm2

        punpcklbw mm1, mm0
        punpckhbw mm3, mm0

        punpcklbw mm2, mm0
        punpckhbw mm4, mm0

        paddsw    mm1, mm2
        paddsw    mm3, mm4

        movq    mm5, [ecx+esi]
        paddsw    mm1, mm7

        movq    mm6, [ecx+esi+1]
        paddsw    mm3, mm7

        movq    mm2, mm5
        movq    mm4, mm6

        punpcklbw mm2, mm0
        punpckhbw mm5, mm0

        punpcklbw mm4, mm0
        punpckhbw mm6, mm0

        paddsw    mm2, mm4
        paddsw    mm5, mm6

        paddsw    mm1, mm2
        paddsw    mm3, mm5

        movq    mm6, [ebx+esi]

        movq    mm4, mm6
        punpcklbw mm4, mm0
        punpckhbw mm6, mm0

        psllw   mm4, 2
        psllw   mm6, 2

        paddsw    mm1, mm4
        paddsw    mm3, mm6

        psrlw   mm1, 3
        psrlw   mm3, 3

        packuswb  mm1, mm0
        packuswb  mm3, mm0

        psllq   mm3, 32
        por     mm1, mm3

        movq    [ebx+esi], mm1
        add     esi, 0x08
        cmp     esi, [w]
        jl      mc7
/*
        add     esi, 0x08
        cmp     esi, [w]
        movq    [ebx+esi-8], mm1
        jl      mc7
*/

        add     eax, [lx2]
        add     ebx, [lx2]
        add     ecx, [lx2]
        mov     esi, 0x00
        dec     edi
//        cmp     edi, 0x00
        jg      mc7
      }
      break;
  }
}


void SwitchIDCT()
{
/*
  switch (MParse.iDCT_Flag)
  {
    case IDCT_MMX:
      idct_decoder = MMX_IDCT;
      break;

    case IDCT_SSEMMX:
      idct_decoder = SSEMMX_IDCT;
      break;

    case IDCT_FPU:
      idct_decoder = FPU_IDCT;
      break;

    case IDCT_REF:
      idct_decoder = REF_IDCT;
      break;

    case IDCT_SSE2:
      idct_decoder = SSE2MMX_IDCT;
  }
*/

}

