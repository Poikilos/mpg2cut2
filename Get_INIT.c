

#define DBG_RJ
 
// Initalize Mpeg2 Video Decoder.
// Allocates and sub-allocates picture work area buffers

#include  "global.h"
#include "MPV_PIC.h"
#include "PIC_BUF.h"
#include "Audio.h"

//------------------

int Old_Pic_Width=720,   Old_Pic_Height=576,  Old_Pic_Size = 720*576;
int Old_Pic_Reset=0;
int Old_Seq_Width=2048,  Old_Seq_Height=2048;
int iWarned2k=0;


void Derived_Sizes()
{
  mb_width  = (MPEG_Seq_horizontal_size+15)/16;

  // The specs say to use MPEG_SeqXtn_progressive_sequence
  // but it works better if we take Picture ScanMode into account.

  mb_height = (MPEG_Pic_Structure || MPEG_SeqXtn_progressive_sequence)
            ?     (MPEG_Seq_vertical_size+15)/16     // Progressive
            :  2*((MPEG_Seq_vertical_size+31)/32)  ; // interlaced


  //  Trap BAD canvas size leading to huge mallocs or crashes, etc

  if  (  !  mb_width)   mb_width  =  1;
  if  (  !  mb_height)  mb_height =  1;

  Coded_Pic_Width    =  16  *  mb_width;
  Coded_Pic_Height   =  16  *  mb_height;

  DOUBLE_WIDTH  =  Coded_Pic_Width<<1;
  HALF_WIDTH    =  Coded_Pic_Width>>1;


  Coded_Pic_Size =  Coded_Pic_Width  *  Coded_Pic_Height  ;


  Chroma_Width   =  (MPEG_Seq_chroma_format  ==  CHROMA444)
                        ?  Coded_Pic_Width  :  Coded_Pic_Width>>1;

  Chroma_Height  =  (MPEG_Seq_chroma_format    >  CHROMA420)  //  !=  CHROMA420)
                        ?  Coded_Pic_Height  :  Coded_Pic_Height>>1;

  Chroma_Size    =  Chroma_Width  *  Chroma_Height  ;

  Clip_Width     =  Coded_Pic_Width;
  Clip_Height    =  Coded_Pic_Height;


  Mpeg_MacroBlk_Array_Limit  =  ChromaFormat[MPEG_Seq_chroma_format];

}




void  Decoder_INIT()
{
  //int Max_Height, Max_Width;
  int iNewBuffers, iSize, iChannel;
  int iRC, iTmp1;

  if (DBGflag)
      DBGout("Decoder Init");


  Old_Pic_Reset = 0;

  //  Trap BAD canvas size leading to huge mallocs or crashes, etc
  {
      if (MPEG_Seq_horizontal_size != Old_Seq_Width
      ||  MPEG_Seq_vertical_size   != Old_Seq_Height)
      {
          if ((MPEG_Seq_horizontal_size &3)
          ||  (MPEG_Seq_vertical_size   &3)
          ||   MPEG_Seq_horizontal_size > 1920
          ||   MPEG_Seq_vertical_size   > 1088)
          {
              sprintf(szMsgTxt, "ODD/MIXED CANVAS SIZE %d.%d **",  
                                 MPEG_Seq_horizontal_size,
                                MPEG_Seq_vertical_size);
              DSP1_Main_MSG(0,0);
              MessageBeep(MB_OK);

              if (MParse.SeqHdr_Found_Flag)
              {
                 MPEG_Seq_horizontal_size = Old_Seq_Width;
                 MPEG_Seq_vertical_size   = Old_Seq_Height;
                 Old_Pic_Reset = 1;
              }
          }
      }
  }
  

  if (MPEG_Seq_horizontal_size > 2048
  ||  MPEG_Seq_vertical_size   > 2048)
  {
      sprintf(szMsgTxt, "Pic dimension > 2048\n\nSize = %d.%d\n\nCould crash on some systems !\n\nForce 2k Limit ?",
                              MPEG_Seq_horizontal_size,
                              MPEG_Seq_vertical_size);

      if (iView_Limit2k > 0 && !MParse.SeqHdr_Found_Flag)
      { 
           iRC =  Warning_Box(&szMsgTxt[0], 0, &iTmp1, IDM_VIEW_HUGE, MB_OKCANCEL);
           if (iRC == IDCANCEL)
               iView_Limit2k = 0;
           else
               iView_Limit2k = 1;

           if (iTmp1)
           {
               iCtl_View_Limit2k = iView_Limit2k;
               ToggleMenu('=', &iCtl_View_Limit2k, IDM_VIEW_HUGE);
           }
      }
      else
      {
           DSP1_Main_MSG(0,0);
           MessageBeep(MB_OK);
           if (!iWarned2k)
           {
             iWarned2k = 1;
             Sleep(1000);
           }
      }


      if (iView_Limit2k)
      {
          if (MPEG_Seq_horizontal_size > 2048)
              MPEG_Seq_horizontal_size = 2048;
          if (MPEG_Seq_vertical_size   > 2048)
              MPEG_Seq_vertical_size   = 2048;

          Old_Pic_Reset = 1;
      }
  }



  //  Don't  let  ridiculous  data  crash  DirectDraw
  if  (MPEG_Seq_horizontal_size  >  8192  || MPEG_Seq_horizontal_size < 8)
  {
      sprintf(szMsgTxt, "WIDTH %d BAD **",  MPEG_Seq_horizontal_size);
      DSP1_Main_MSG(0,0);
      MessageBeep(MB_OK);
      Old_Pic_Reset = 1;
  }


  if  (MPEG_Seq_vertical_size  >  4096  || MPEG_Seq_vertical_size < 8)
  {
      sprintf(szMsgTxt, "HEIGHT %d BAD **",  MPEG_Seq_vertical_size);
      DSP1_Main_MSG(0,0);
      MessageBeep(MB_OK);
      Old_Pic_Reset = 1;
  }

  Derived_Sizes();

  iNewBuffers = 0;

  if (Coded_Pic_Size != PicBuffer_Canvas_Size) 
  {
     if (Coded_Pic_Size > PicBuffer_Canvas_Size) 
     {
         PicBuffer_Alloc();
         iNewBuffers = 1;
     }
     else
     {
         iView_Fast_YUV = 0;
         iView_Fast_RGB = 0;
     }
  }


  if (Old_Pic_Reset)
  {
      MPEG_Seq_horizontal_size = Old_Seq_Width; // Max_Width;
      MPEG_Seq_vertical_size   = Old_Seq_Height; // Max_Height;

      Derived_Sizes();

      if (DBGflag)
      {
          sprintf(szDBGln, "RESET %d.%d", 
                            MPEG_Seq_horizontal_size,
                            MPEG_Seq_vertical_size);
         DBGout(szDBGln);
      }

  }
  else
  {
     if (Old_Seq_Width  != MPEG_Seq_horizontal_size
     ||  Old_Seq_Height != MPEG_Seq_vertical_size)
     {
         if (DBGflag)
         {
            sprintf(szBuffer, "NEW SIZE=%d.%d\nold=%d.%d  ",
                          MPEG_Seq_horizontal_size, MPEG_Seq_vertical_size,
                    Old_Seq_Width, Old_Seq_Height);
            DBGout(szBuffer);
         }

         if (DDOverlay_Flag) //  && !MParse.SizeCommitted)
             D300_FREE_Overlay();
         Old_Seq_Width  = MPEG_Seq_horizontal_size;
         Old_Seq_Height = MPEG_Seq_vertical_size;

         if (!iCtl_Zoom_Retain)
              iCtl_Zoom_Wanted = -1;  // Reset Zoom default to auto setting optional
         iPhysView_Width  = VGA_Width;
         //iPhysView_Height = VGA_Height;
     }

      // If reusing old buffers,
      // clear out picture when new process begun
 
      if (!Frame_Number && ! iNewBuffers)
      {
        for  (iChannel = 0;  
             iChannel < 1; // 3;  // Only do Y.  Don't really care about U,V 
             iChannel++) 
        {
           if  (iChannel==0)
                iSize  =  PicBuffer_Canvas_Size;
           else
                iSize  =  Chroma_Size;

           memset(bwd_ref_frame[iChannel], 0x00, iSize);
           memset(fwd_ref_frame[iChannel], 0x00, iSize);
           memset(aux_frame    [iChannel], 0x00, iSize);
        }

        //memset(LumAdjBuf,          0x00,  PicBuffer_Canvas_Size);
        //memset(yuy2,         0x00, (PicBuffer_Canvas_Size*2));
        //memset(rgb24,        0x00, (PicBuffer_Canvas_Size*3));

      } // end-if clearing buffers

  } // end-if new size valid


  if (Coded_Pic_Width)
      Mpeg_Aspect_Resize();
  else
  {
      sprintf(szMsgTxt,  "Width=%d",  Coded_Pic_Width);
      iVertInc  =  0  ;
      iAspVert  =  2048;  iAspHoriz  =  2048;
      iAspect_Height  =  Coded_Pic_Height;
  }

  if (!DDOverlay_Flag   && !iCtl_View_RGB_Always
  &&  MParse.iColorMode != STORE_RGB24)
      D100_CHECK_Overlay(); 
  
  MParse.ReInit = 1;  MParse.SizeCommitted = 1;

  if (DBGflag)
      DBGout("Decoder Rdy");
}


//-------------------
void Mpeg_Drop_Init()
{
     if (MParse.FastPlay_Flag 
      || winVer.dwMajorVersion >= 6 || iCtl_VistaOVL_mod)
         PlayCtl.iDrop_Behind   = 257;
     else
         PlayCtl.iDrop_Behind    =  iCtl_Drop_Behind;

     PlayCtl.iDrop_PTS_Flag  =  iCtl_Drop_PTS;

     if  (iAudio_SEL_Track  !=  TRACK_NONE 
     &&  ( !  iOverride_FrameRate_Code
          || (iCtl_PALTelecide
              && iOverride_FrameRate_Code == 2   // Want 24FPS
              && MPEG_Seq_frame_rate_code == 3   //  was 25FPS
              && !iAudio_Force44K)
         ))
          iAudio_Lock  =  iCtl_Play_AudLock;

     if (PlayCtl.iDrop_Behind && MPEG_Pic_Structure == 3  // Not supported on full interlace
     &&  MParse.SlowPlay_Flag <= 0)
     {
         if (PlayCtl.iDrop_Behind > 1            // Drop allowed on HD ?
         && (Coded_Pic_Height > 576 || iFrame_Rate_int > 33)) 
         {
             if (cpu.sse2 || cpu._3dnow)  // Pentium 4 or better 
                 PlayCtl.iDrop_B_Frames_Flag = 0;
             else
             if (cpu.ssemmx)  // Pentium 3 or better
                 PlayCtl.iDrop_B_Frames_Flag = 1;
             else
                 PlayCtl.iDrop_B_Frames_Flag = 2;
         }
         else
         {
             if (cpu.ssemmx)  // Pentium 3 or better
                 PlayCtl.iDrop_B_Frames_Flag = 0;
             else
                PlayCtl.iDrop_B_Frames_Flag = 1;
         }
     }

     if ( ! iCtl_ShowVideo_Flag)
     {
         PlayCtl.iDrop_Behind        = 257;
         PlayCtl.iDrop_B_Frames_Flag = 3;
     }
}

//---------------------------------------------------------
//  allocate  mem  area  as  internal  sub-pool,  mimics  "malloc"
unsigned  char  *  suballoc(int  P_Size)
{
unsigned  char  *  iPtr;

  iPtr        =  (unsigned  char  *)((int)(subpool_ptr    +  64)  &  0xFFFFFFC0);
  subpool_ptr  =  iPtr  +  P_Size;      //  ((iPtr  +  P_Size  +  64)  &  0xFFFFFFC0)  ;

return  iPtr;
}



//----------------------
void  PicBuffer_Alloc()
{
  int  i,  size,  SafeTotal, iGenerous;
  int  SafeWidth,  SafeHeight,    SafeSize  ;
  int  SafeCWidth,  SafeCHeight,  SafeCSize  ;

  if (DBGflag)
  {
    DBGout("BUFF");
  }

  if  (PicBuffer_Canvas_Size  >  0)
       PicBuffer_Free();

  //  RJ
  //  Changed  buffer  allocation  strategy  for  fault  tolerance
  //  Single  GetMain  for  combined  buffer  area,  with  padding,
  //  which  I  then  suballocate,
  //  with  the  safest  buffers  at  each  end,
  //  and  the  risky  buffers  in  the  middle
  
  //  First,  add  some  padding  to  all  the  dimensions

  SafeWidth    =  Coded_Pic_Width   +  8;
  SafeHeight   =  Coded_Pic_Height  +  8;

  SafeSize     =  SafeWidth * SafeHeight;

  if (cpu.sse2)
     iGenerous = 1928*1088;
  else
     iGenerous = 1024*1024;
  
  if (SafeSize < iGenerous)
      SafeSize = iGenerous;

  PicBuffer_Canvas_Size = SafeSize;

  SafeCWidth    =  Chroma_Width  +  8;
  SafeCHeight   =  Chroma_Height +  8;
  SafeCSize     =  SafeCWidth * SafeCHeight;

  iGenerous = iGenerous/2;
  if (SafeCSize < iGenerous)
      SafeCSize = iGenerous;

  //  Calculate  total  size  of  buffers  to  be  pooled
  //      RJ  -  WOW  -  It's  a  big  whack  -  may  be  able  to  trim  this  down  one  day  ?
  SafeTotal    =  ((SafeSize+64) *  12) +  ((SafeCSize+64) * 6) + 8192
                  +  8  *  (sizeof(short)*64  +    64)
                  +         sizeof(float)*128 +  8192;

  //  Allow  for  experimentation
  if  (Mpeg_PES_Version != 2  &&  SafeSize < (768*576))
       SafeTotal = SafeTotal * 2;

  frame_pool  =  (unsigned  char*)malloc(SafeTotal);
  if (frame_pool == NULL)
      Err_Malloc(&"PicBuf");


  //  Now  we  sub  allocate  into  various  frame  work  areas
  subpool_ptr  =  (unsigned char *)(frame_pool + 4096);

  //  Allow  for  experimentation
  if  (Mpeg_PES_Version  !=  2  &&  SafeSize  <  (768  *  576))
      subpool_ptr  +=  SafeSize;

  //  Safe  buffers  (less  likely  to  cause  crash)
  u444  =  (unsigned  char*)suballoc(SafeSize);
  v444  =  (unsigned  char*)suballoc(SafeSize);
    //  RJ  Moved  here  from  GUI  initialization
  //  Allocate  memory  for  macroblock  tiles

  for  (i=0;  i<16;  i++)
  {
    p_block[i]  =  (short  *)suballoc(sizeof(short)*64  +  64);
    block[i]    =  (short  *)((long)p_block[i]  +  64
                           -  (long)p_block[i]%64);
  }

  p_fTempArray  =  (void  *)malloc(sizeof(float)*128  +  64);
  if (p_fTempArray == NULL)
      Err_Malloc(&"TempArray");

  fTempArray    =  (void  *)((long)p_fTempArray  +  64
                          -  (long)p_fTempArray%64);


  for  (i=0;  i<3;  i++)
  {
    if  (i==0)
      size  =  SafeSize  ;
    else
      size  =  SafeCSize ;

    bwd_ref_frame   [i] = (unsigned char*)suballoc(size);
  //end_bwd_ref     [i] = bwd_ref_frame[i] + size - 64 ;
    curr_frame      [i] = bwd_ref_frame[i];
    curr_full_frame [i] = curr_frame   [i];

    fwd_ref_frame   [i] = (unsigned char*)suballoc(size);
  //end_fwd_ref     [i] = fwd_ref_frame[i] + size - 64;

    aux_frame[i]     = (unsigned char*)suballoc(size);
    //end_aux[i]       = aux_frame[i] + size - 64 ;
  }


  // DTV is mostly 4:2:0, for both SD and HD
  // therefore this is a high risk area, 
  // so stick it in the middle of the buffer area
  u422  = (unsigned char*)suballoc(SafeSize/2);
  v422  = (unsigned char*)suballoc(SafeSize/2);

  yuy2  = (unsigned char*)suballoc(SafeSize*2);
  LumAdjBuf   = (unsigned char*)suballoc(SafeSize);
  //sat   = (unsigned char*)suballoc(SafeSize);


// Allocate rgb24 here because not used when overlay available
// Allocated last in case it proceeds backwards too far
  RGB24_size = SafeSize*3;
  rgb24 = (unsigned char*)suballoc(RGB24_size);

// rgb24 Zoom line work area
  rgbZoomLine = (unsigned char*)suballoc(8192);


}


//---------------------
void PicBuffer_Free()
{
  int i;

  if (PicBuffer_Canvas_Size < 1)
     return;

  PicBuffer_Canvas_Size = 0;

  // Free the big frame pool

  free(frame_pool);

  if (DBGflag)
  {
      DBGout("Frame Pool Freed");
  }


  for (i=0; i<3; i++)
  {
      // RJ *** No longer have individual frame mallocs to free ***
      bwd_ref_frame[i] = 0;      //    free(bwd_ref_frame[i]);
      fwd_ref_frame[i] = 0;      //    free(fwd_ref_frame[i]);
      aux_frame    [i] = 0;      //    free(aux_frame[i]);
      curr_frame   [i] = 0;
  }
      // RJ *** No longer have individual frame mallocs to free ***
  //free(u422);    free(v422);    free(u444);    free(v444);
  //free(rgb24);   free(yuy2);   
  //free(LumAdjBuf);

  // ** USED TO CRASH HERE WHEN COMPILED WITH DEBUG - DUNNO WHY
  return;
}




// Reset Decoder file analysis state

void MPEG_File_Reset()
{
          process.NAV_Loc   =  -1;    // ptr  to  MOST  RECENT  VOB  NAV  pack  header
          process.PAT_Loc   =  -1;    // ptr to 1st TS PAT header

          uPID_Map[0]  = STREAM_AUTO;  uPID_Map[1]  = STREAM_AUTO;
          uPID_Map[2]  = STREAM_AUTO;  uPID_Map[3]  = STREAM_AUTO;
          uPID_Map[4]  = STREAM_AUTO;  uPID_Map[5]  = STREAM_AUTO;
          uPID_Map[6]  = STREAM_AUTO;  uPID_Map[7]  = STREAM_AUTO;
          uPID_Map[8]  = STREAM_AUTO;  uPID_Map[9]  = STREAM_AUTO;
          uPID_Map[10] = STREAM_AUTO;  uPID_Map[11] = STREAM_AUTO;
          uPID_Map[12] = STREAM_AUTO;  uPID_Map[13] = STREAM_AUTO;
          uPID_Map[14] = STREAM_AUTO;  uPID_Map[15] = STREAM_AUTO;
          uPID_Map_Used = 0;

          if (iCtl_Track_Memo)
          {
              iAudio_SEL_Track  = TRACK_AUTO;
              iAudio_SEL_Format = FORMAT_AUTO;
          }

          process.Suspect_SCR_Flag = 0;
          MParse.EDL_AutoSave = iCtl_EDL_AutoSave;

          // Get hints about format from the extension
          if (!stricmp(szInExt, "TS"))
          {
             MParse.SystemStream_Flag = -1;
             PktChk_Audio = 500; PktChk_Any = 12000;
          }
          else
          if (!stricmp(szInExt, "PVA"))
          {
             MParse.SystemStream_Flag = -2;
             PktChk_Audio = PKTCHK_AUDIO_TS; PktChk_Any = PKTCHK_ANY_TS;
          }
          else
            MParse.SystemStream_Flag = 0;

          // Default decoder controls
          uCtl_Vid_PID = STREAM_AUTO;
          if (uAud_PID_All)
              uCtl_Aud_PID = STREAM_AUTO;
          uGot_PID = uCtl_Vid_PID;
          uCtl_Video_Stream = STREAM_AUTO;

          // Default decode values to Mpeg-1 ES implied settings
          Mpeg_PES_Version     = 1;  Mpeg_SEQ_Version = 1;
          process.Mpeg2_Flag   = 0;
          //iPES_Mpeg_Any = 0;
          Mpeg_Version_Alerted = 0;

          MPEG_Pic_intra_dc_precision = 0;
          MPEG_Pic_top_field_first        = 0;
          MPEG_Pic_pred_frame_dct         = 1;
          MPEG_Pic_concealment_motion_vectors = 0;
          MPEG_Pic_q_scale_type           = 0; // ?????
          MPEG_Pic_intra_vlc_format       = 0;
          MPEG_Pic_alternate_scan         = 0; // ???
          MPEG_Pic_repeat_first_field     = 0;
          MPEG_Pic_chroma_420_type        = 1;
          MPEG_Pic_Origin_progressive     = 1; 
          MPEG_Pic_composite_display_flag = 0;

          MPEG_Seq_chroma_format = 0;
          MPEG_Seq_load_intra_quantizer_matrix = 0;
          MPEG_Seq_load_chroma_intra_quantizer_matrix = 0;

          MPEG_iFrame_rate_extension_n = 0;
          MPEG_iFrame_rate_extension_d = 0;
          MPEG_Pic_f_code[0][0] = 15;  MPEG_Pic_f_code[0][1] = 15;
          MPEG_Pic_f_code[1][0] = 15;  MPEG_Pic_f_code[1][1] = 15;
          MPEG_Profile        =  0;
          MPEG_Level          =  0;
          MPEG_ProfLvl_Escape = ' ';

          MPEG_Pic_Structure  = FULL_FRAME_PIC;
          PicOrig_ScanMode_code = 1; 

          d2v_curr.Progressive_Format = 1;
          d2v_fwd.Progressive_Format = 1;
          d2v_bwd.Progressive_Format = 1;
          //d2v.Fld1_Top_Rpt           = 0;

          ZeroMemory(&SubStream_CTL,  sizeof(SubStream_CTL));
          ZeroMemory(&mpa_Ctl,        sizeof(mpa_Ctl));
          ZeroMemory(&iAudio_Trk_FMT, sizeof(iAudio_Trk_FMT));
}



