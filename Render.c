
#define DBG_RJ

 
#include "global.h"
#include <math.h>
#include <sys/timeb.h>
#include "DDRAW_CTL.h"

#include "Audio.h"
#include "wave_out.h"

#include <vfw.h>
#include "PIC_BUF.h"
#include "Mpg2Cut2_API.h"

void View_Visible_Calc(int, int);


BITMAPINFOHEADER birgb;
LPBITMAPINFOHEADER lpbirgb = &birgb;

BITMAPINFOHEADER biyuv;
LPBITMAPINFOHEADER lpbiyuv = &biyuv;

unsigned char Lum_Tbl_OVL[256];
unsigned char Lum_Tbl_RGB[256];
unsigned char Sat_Tbl_U[256];
unsigned char Sat_Tbl_V[256];

static int iLUM_AREA;


int iPrim_Height, iSrcLineBytes, iSrcLineBytes2;


//-------------------------------------

__forceinline void R250_SIGNAL_Overlay()
{
   int iRC;
   if (iCtl_VistaOVL_mod == 3)
   {
       iRC = PostThreadMessage(threadId_MAIN, RJPM_SIGNAL_OVL, 0, 0);
       Sleep(1);
   }
   else
     D200_UPD_Overlay();

}


//---------------------------------------------------------------

void RenderRGB24()
{
  int iK_Width, iK_Width2, iK_Height, iK_Get_Ht, iK_xFrom, iK_yFrom;
  int iChunk_Height, iChunk_Bytes, iChunk_DIB, iK_Zoom, iK_ZoomBytes;
  int iClient_PicHeight;
  int iVertInc_ADJ;
  int iK_Inter_Flag, iK_Inter_Src, iK_Inter_Offset, iK_Inter_Inc;
  int iK_Line;
  int iLine, iLine_Max, iRC;
  char cMsgMode;

  int nVertErrTot, iDDO_LineCtr;
  unsigned char  *src, *src2, *src3, *srcY_eol, *dst2;

  //_ftime( &ts2 );
  //dwTime=(ts2.time-ts1.time)*1000+(ts2.millitm-ts1.millitm);
  //if (dwTime<(DWORD)(1000/fFrame_Rate_Orig)-1)
  //     Sleep((DWORD)(1000/fFrame_Rate_Orig)-1-dwTime);
  //_ftime( &ts1 );

  // TIMING CHECK
  //if (iDDO_Frame_Ready)
  //    R250_SIGNAL_Overlay(); // D200_UPD_Overlay();

  iSync_Diff_ms = 0;

  if (!rgb24)
  {
    strcpy(szMsgTxt,  "Frame PTR Lost");
    DSP1_Main_MSG(0,0);
  }


  //if (MPEG_Pic_Type != I_TYPE)
  if (!MParse.Stop_Flag && !MParse.Pause_Flag 
  &&   process.Action ==  ACTION_RIP
  &&  !MParse.Tulebox_SingleStep_flag)
    if (Store_Timing_Chk(0)) // Maybe drop frame
        return;

  PlayCtl.iShown_Frames++;

  cMsgMode = 'b';

  // Cannot handle some zoom settings yet
  if (iZoom < 1)
      iK_Zoom = 1;
  else
      iK_Zoom = iZoom;


  iK_ZoomBytes =  iK_Zoom * 3;

  // How much space do we have for viewing ?
  View_Visible_Calc( iK_Zoom, 3);


  // What is the size of the copy Chunk ?
  if (Deint_VIEW || iView_Invert)
  {
      iChunk_Height = 1;
  }
  else
  if (iVertInc == 0)
  {
      iChunk_Height = iGet_Height;
  }
  else
  {
      iChunk_Height = 2000 / iVertInc;
      if (iChunk_Height < 0)
          iChunk_Height = 0 - iChunk_Height;
      iChunk_Height++;
  }

  iClient_PicHeight = Client_Height - iTopMargin - 1;
  if (iChunk_Height > iClient_PicHeight)
      iChunk_Height = iClient_PicHeight;

  if (! iChunk_Height)
        iChunk_Height = 1;

  src = rgb24 + iGet_xFromBytes
              + ((Clip_Height - 1) * iSrcLineBytes)
              - ((iGet_yFrom + iChunk_Height) * iSrcLineBytes) ;

  if (iView_Invert)
  {
      src -= ((iGet_Height -1) * iSrcLineBytes);
      iSrcLineBytes  = -iSrcLineBytes;
      iSrcLineBytes2 = -iSrcLineBytes2;
  }

  iChunk_Bytes = (iChunk_Height * iSrcLineBytes) ;


  // Removed code below due to inconsistencies between files
  //if (Deint_VIEW)
  //  src += iSrcLineBytes;


  // Kludge controls to action part of view adjustments

  iK_Width  = Clip_Width;
  iK_Width2 = ((iK_Width + 3) / 4) * 8;
  iK_Height = Clip_Height;
  iK_xFrom  = iView_xFrom;
  iK_yFrom  = iK_Height - iView_yFrom;
  iK_Get_Ht = iGet_Height;

  if (iK_Zoom > 1)
  {
      iK_Width  = iK_Width2;
      iChunk_Height = 1;
      iChunk_Bytes  = iSrcLineBytes2;
      iK_Height = iK_Height / 2;
      iK_yFrom  = iK_yFrom  / 2;
      iK_Get_Ht = iK_Get_Ht / 2;
  }

  iK_Inter_Offset = iSrcLineBytes;
  iK_Inter_Flag = 1;
  iChunk_DIB = iChunk_Height;

  iVertInc_ADJ = iVertInc;
  if (Deint_VIEW)
  {
      iK_Inter_Offset = iSrcLineBytes2;
      if (iVertInc == 0)
      {
          iChunk_DIB = iChunk_DIB / 2;
          iVertInc_ADJ = -2000;
      }

      if (iK_Zoom <= 1) // != 2)
      {
         iK_Width  = iK_Width2;
         iChunk_Bytes *= 2;
         iK_Height    /=  2;
         iK_yFrom     /=  2;
         iK_Get_Ht    /=  2;
         if (iK_Zoom > 2)
             iK_Inter_Flag = 0;
      }
  }
  else
  {
      if (iView_Aspect_Mode)
          iK_Inter_Offset = (int)((iChunk_Bytes / iSrcLineBytes) / 2)
                                                * iSrcLineBytes;
      if (iK_Zoom > 1)
          iK_Inter_Flag = 0;

  }

  if (! iChunk_DIB)
        iChunk_DIB = 1;

  if (iK_Zoom > 1)
      iK_Inter_Inc = iK_ZoomBytes;
  else
      iK_Inter_Inc = 4;

  iView_Width = Clip_Width / iK_Zoom;
  if (iView_Width > iPhysView_Width)
      iView_Width = iPhysView_Width;

  if ( !iK_Width)  iK_Width  = iK_Width2       * iK_Zoom;  // Trap compiler error
  if ( !iK_Height) iK_Height = Clip_Height / 2 / iK_Zoom;  // Trap compiler error

  if (iChunk_Height > 1)
  {
    birgb.biWidth  = iK_Width;
    birgb.biHeight = iK_Height;
  }
  //birgb.biHeight = iK_Height;
  birgb.biSizeImage = birgb.biWidth * birgb.biHeight * 3;


  // copy from the internal RGB buffer to the Windows RGB surface

  iLine     = iGet_yFrom;
  iLine_Max = iGet_Bot - iChunk_Height+1;
  if (iCtl_CropTop)
      iLine_Max+=2;

  iK_Line = 0;
  iDDO_LineCtr = iTopMargin; // + iPrim_Height ;
  nVertErrTot = 0;

#ifdef DBG_RJ
  if (DBGflag &&  process.Action != ACTION_RIP)
  {
        sprintf(szMsgTxt, "Z=%d %d %d\n  K=%d.%d CH=%d\n  G=%d.%d %d +%d..%d.%d \n  V=%d\nDIB=%d.%d",
                iK_Zoom, iK_ZoomBytes, iK_Inter_Inc,
                iK_Width, iK_Height,
                                // iK_xFrom, iK_yFrom, iK_Line, 
                            iChunk_Height,
                                     iGet_Width, iGet_Height, (iGet_WidthBytes/3),
                                           iGet_yFrom, iLine_Max, iGet_Bot,
                                           iView_Width,
                                            birgb.biWidth, birgb.biHeight);
        DBGout(szMsgTxt);
        DSP1_Main_MSG(0,0);
  }
#endif


  while ( iLine < iLine_Max)
  {
    switch (iK_Zoom)
    {

      case 1:  // NO ZOOM - Copy ASIS
           dst2 = src;
      break;


      //   ZOOM code could be rewritten in Assembler
      default:  // ZOOM OUT by any factor

        srcY_eol = src + iGet_WidthBytes;
        dst2    = rgbZoomLine;
        for (src2 = src ;
             src2 < srcY_eol ;
             src2 = src2 + iK_ZoomBytes) // jump n pels
        {
            // Quick and dirty sub-sample
            // Copy RGB, then skip
            *(UNALIGNED DWORD *)(dst2) = *(UNALIGNED DWORD *)(src2);// copy 4 bytes at once
             dst2 =  dst2+3;   // next work area pel
         } //endfor

         dst2    = rgbZoomLine;
      break;

    } // END-SWITCH

/*
#ifdef DBG_RJ
    if (DBGflag &&  process.Action != ACTION_RIP)
    {
        sprintf(szBuffer, "DIB Line=%03d.%03d/%03d Width=%d  Chunk=%d",
                           iDDO_LineCtr, iLine, iLine_Max,
                           iView_Width,  iChunk_Height);
        DBGout(szBuffer);
    }
#endif
*/



    //if ( ((iDDO_LineCtr + iChunk_DIB) < Client_Height)
    //||   iChunk_Height > 100  ||  iDDO_LineCtr < 0)
    {
       iRC=SetDIBitsToDevice(hDC,
            0, iDDO_LineCtr,  // coordinates of upper-left corner of dest. rect.
            iView_Width, iChunk_DIB, // size of source rect.
            0, 0,             // coordinate of upper-left corner of source rect
            0, iChunk_DIB,    // start, number of array scan lines
            dst2,             // rgb or zoom buffer
           (LPBITMAPINFO)lpbirgb, DIB_RGB_COLORS);
    }
    //else
    //   iRC = 0x6969;

    /*
    if (!iRC)
    {
        // iMsgDone = 1;  MParse.Stop_Flag = 1;
        //iLine = iLine_Max;
        Msg_LastError("SetDIB1: ", iRC, cMsgMode);
        cMsgMode = ' ';
    }
    */

    iDDO_LineCtr += iChunk_Height;


    // Anamorphic scaling as per Aspect Ratio correction factor
    // The maths here is quick & dirty, but it will do.
    if (iView_Aspect_Mode)
        nVertErrTot+= (iVertInc_ADJ * iChunk_Height) ;

    // STRETCH Vertical by interpolating selected lines when required

    iK_Inter_Src = 1;

    if (iView_Aspect_Mode)
    {
      while (nVertErrTot <= -2000)
      {
         src3    = src - iK_Inter_Offset;

         if (iK_Inter_Flag)
         {
             srcY_eol = src + iGet_WidthBytes;
             dst2    = rgbZoomLine;
             for (src2 = src ;
                  src2 < srcY_eol ;
                  src2 = src2 + iK_Inter_Inc) // jump some pels
             {
                // merge 4 bytes at once then skip

                if (iK_Inter_Inc == 4)
                {
                  *(int *)(dst2) =
                            ((*(int *)(src2) >>1) & 0x7F7F7F7F)
                          + ((*(int *)(src3) >>1) & 0x7F7F7F7F);
                   dst2 = dst2+4; // next work area int
                   src3 = src3+4;
                }
                else
                if (iK_Inter_Src)
                {
                  *(UNALIGNED DWORD *)(dst2) =
                            ((*(UNALIGNED DWORD *)(src2) >>1) & 0x7F7F7F7F)
                          + ((*(UNALIGNED DWORD *)(src3) >>1) & 0x7F7F7F7F);
                   dst2 = dst2+3; // next work area pel
                   src3 = src3+iK_ZoomBytes;
                }
                else
                {
                  *(UNALIGNED DWORD *)(dst2) =
                            ((*(UNALIGNED DWORD *)(src2) >>1) & 0x7F7F7F7F)
                          + ((*(UNALIGNED DWORD *)(src3) >>1) & 0x7F7F7F7F);
                   dst2 = dst2+3;   // next work area pel
                   src3 = src3+3;
                }

             } //endfor

             dst2 = rgbZoomLine;
             src3 = dst2;
             iK_Inter_Src = 0;
         }
         else
         {
            dst2 = src3;
         }

/*
#ifdef DBG_RJ
         if (DBGflag &&  process.Action != ACTION_RIP)
         {
            sprintf(szBuffer, "DIB Line=%03d.%03d/%03d Width=%d  Chunk=%d",
                           iDDO_LineCtr, iLine, iLine_Max,
                           iView_Width,  1);
            DBGout(szBuffer);
         }
#endif
*/

         iRC=SetDIBitsToDevice(hDC,
               0, iDDO_LineCtr, // coordinates of upper-left corner of dest. rect.
               iView_Width, 1,  // size of source rect.
               0, 0,          // coordinate of upper-left corner of source rect
               0, 1,          // start, number of array scan lines
               dst2,          // rgb or zoom buffer
             (LPBITMAPINFO)lpbirgb, DIB_RGB_COLORS);
         /*
         if (!iRC)
         {
             //iMsgDone = 1;  MParse.Stop_Flag = 1;
             //iLine = iLine_Max;
             Msg_LastError("SetDIB2: ", iRC, cMsgMode);
             cMsgMode = ' ';
         }
         */

        iDDO_LineCtr++ ;
        nVertErrTot += 2000 ;
      } // END-WHILE
    } // END-IF

    src          -= iChunk_Bytes ;
    iLine        += iChunk_Height;
    iK_Line      += iChunk_Height;


    //if (Deint_VIEW)
    //{
          //src   -= iChunk_Bytes;
    //    iLine += iChunk_Height;
    //}

    // SQUEEZE Vertical by skipping selected lines
    if (iView_Aspect_Mode)
    {
      while (nVertErrTot >= 2000)
      {
        nVertErrTot = nVertErrTot - 2000 + iVertInc;
        if (Deint_VIEW)
        {
           src -= iSrcLineBytes2;
           iLine+=2 ;
        }
        else
        {
           src -= iSrcLineBytes ;
           iLine++ ;
        }  // end-ELSE

        iK_Line++;

      } //end-while
    } // end-IF
    else
    if (Deint_VIEW)
    {
        iLine  += iChunk_Height;
    }

  } //endfor iLine (source line index)

  //StretchDIBits(hDC, 0, iTopMargin],  // coordinates of upper-left corner of dest. rect.
  //  iView_Width, iView_Height, // size of destination rectangle
  //  iK_xFrom, iK_yFrom,        // coordinate of upper-left corner of source rect
   //  iGet_Width, iK_Get_Ht,   // size of source rectangle
  //  rgb24,                    // address of bitmap bits
  //  (LPBITMAPINFO)lpbirgb, DIB_RGB_COLORS,
   //  ????? );   // raster operation code    <=== What the smeg is this ?

}



//-------------------------------------------------------------

// Update the display output using YUY2 overlay
//
// 3 Modes: -
//    Mode  0 - Grey out the image, to indicate EOF reached
//    Mode  1 - Normal output of current frame
//    Mode -1 - PLANNED- Backward a frame by toggling YUV ovelay buffers.

void RenderYUY2(int P_Mode)
{
  int iLine, iUse_Height;
  int nVertErrTot, iError;
  unsigned char  *src, *dst;
  unsigned char  *src2, *srcY_eol, *dst2, *dst_MAX;

  // TIMING CHECK
  if (iDDO_Frame_Ready)
  {
      R250_SIGNAL_Overlay(); // D200_UPD_Overlay();
  }

  iSync_Diff_ms = 0;

  if (!yuy2)
  {
     strcpy(szMsgTxt,  "Frame PTR Lost");
     DSP1_Main_MSG(0,0);
     return;
  }

  //if (P_Mode > 0 ) // && MPEG_Pic_Type != I_TYPE)
  if (!MParse.Stop_Flag && !MParse.Pause_Flag 
  &&   process.Action ==  ACTION_RIP
  &&  !MParse.Tulebox_SingleStep_flag)
    if (Store_Timing_Chk(1)) // Maybe drop frame
         return;

  PlayCtl.iShown_Frames++;

  iError = 0;


#ifdef DBG_RJ
  if (DBGflag)
  {
    sprintf(szDBGln, "RenderY F=%d\n", iView_Fast_YUV);
    DBGout(szDBGln);
  }
#endif


  // Optionally use EXPERIMENTAL fast view 420
  if (iView_Fast_YUV)
  {
    RenderF420(P_Mode);
    return;
  }

  // CALCULATE POINTS TO COPY

  // Analyse Canvas Sizes
  View_Visible_Calc(iZoom, 2);



  src = yuy2;

  src = src + iGet_xFromBytes;// iView_xFrom2 ;

  if (iView_yFrom > 0)
      src = src + (iView_yFrom * iSrcLineBytes);

   // Optionally swap UV to allow for weird captures
  src = src + iView_SwapUV;

  nVertErrTot = iDDO_LineCtr = 0 ;

  //  COPY THE IMAGE INFORMATION INTO Direct Draw Overlay buffer

  // Check for occasional loss of Overlay pointer - Don't know why it gets lost
  if ( ! lpOverlay)
  {
      DD_PtrLost_Box(&"Overlay");
      MParse.Fault_Flag = CRITICAL_ERROR_LEVEL;
      D300_FREE_Overlay();
      return;
  }

#ifdef DBG_RJ
  if (DBGflag)
  {
    sprintf(szBuffer, "yFrom=%d GBot=%d, GWB=%d",
                       iGet_yFrom, iGet_Bot, iGet_WidthBytes);
    DBGout(szBuffer);
  }
#endif

  if (P_Mode > -1)   // Skip build for backward a frame
  {
   if (IDirectDrawSurface_Lock(lpOverlay, NULL, &ddsd, 0, NULL) != DD_OK)
      iError++;
   else
   {
    if (! ddsd.lpSurface)
    { 
      if (! iError)
      {
          DD_PtrLost_Box(&"Surface");
      }
      iError++;
    }
    else
    {
      dst = (unsigned char *)ddsd.lpSurface;
      dst_MAX = dst + DDraw_Surface_Size - iGet_WidthBytes;

      // copy from the internal yuy2 buffer to the Direct Draw yuy2 Overlay

      for ( iLine = iGet_yFrom; 
           (iLine < iGet_Bot && dst < dst_MAX); 
            iLine++)
      {
        if (P_Mode)
        {
          switch (iZoom)
          {

          //   ZOOM code could be rewritten in Assembler
          case 2:  // ZOOM OUT by factor of 2
            srcY_eol =   src + iGet_WidthBytes;
            dst2    =   dst;
            for (src2 = src ;
                 src2 < srcY_eol ;
                 src2 = src2 + 8) // jump 4 pels
            {
              // Quick and dirty sub-sample
              // Copy YUxV, then replace "x" with following Y, then skip

              *(UNALIGNED DWORD *)(dst2) = *(UNALIGNED DWORD *)(src2);  // copy 4 bytes at once
               dst2 =  dst2+2;   // iView_Point to output "x"
              *dst2 = *(src2+4); // copy 1 byte
               dst2 =  dst2+2;   // iView_Point to next output pel

            //FUTURE: Could make the output a bit less ugly..
            // IF NOT PLAYING
            //    COMPUTE dstY1=(byte)( ( (word)srcY1U + (word)srcY2V )/2)
            //    COMPUTE dstY2=(byte)( ( (word)srcY3U + (word)srcY4V )/2)
            // ELSE
            //    quick and dirty byte repair as currently
            } //endfor
            break;

          case 3:  // ZOOM OUT by factor of 3 - Easy !
            srcY_eol =   src + iGet_WidthBytes;
            dst2    =   dst;
            for (src2 = src ;
               src2 < srcY_eol ;
               src2 = src2 + 6) // jump 3 pels
            {
            // Quick and dirty sub-sample
            // Copy Yx, then skip
            *(UNALIGNED WORD *)(dst2) = *(UNALIGNED WORD *)(src2);  // copy 2 bytes at once
             dst2 =  dst2+2; // iView_Point to next output pel
            } // endfor
            break;

          default:  // NO ZOOM - Just a direct copy of a line segment
            memcpy(dst, src, iGet_WidthBytes);
            break;

          } // end-switch
        }
        else
        {
          //if (iDDO_LineCtr++ & 1)
              ZeroMemory(dst, iGet_WidthBytes);  // effectively wash-out the image
          dst += ddsd.lPitch;
          iLine++;
          iDDO_LineCtr++ ;
          nVertErrTot-=iVertInc ;
        } // end-if

        src += iSrcLineBytes;
        dst += ddsd.lPitch;

        // Anamorphic scaling as per Aspect Ratio correction factor
        // The maths here is quick & dirty, but it will do for now.
        iDDO_LineCtr++ ;
        if (P_Mode)
        {
           nVertErrTot+=iVertInc ;

           if (Deint_VIEW)
           {
             src += iSrcLineBytes ;
             iLine++ ;
           }

           // Squeeze Vertical by skipping selected lines
           while (nVertErrTot >= 2000)
           {
            nVertErrTot = nVertErrTot - 2000 + iVertInc;
            if (Deint_VIEW)
            {
              src += iSrcLineBytes2 ;
              iLine+=2 ;
            }
            else
            {
              src += iSrcLineBytes ;
              iLine++ ;
            } // end-if Deint
           } //end-while
          /*
          // Stretch Vertical by doubling selected lines
          // NOW DONE VIA Direct Draw for better performance

          if (nVertErrTot <= -2000)
          {
            memcpy(dst, (dst - ddsd.lPitch), ddsd.lPitch); //DOUBLE_CLIP_WIDTH);
            dst += ddsd.lPitch;
            iDDO_LineCtr++ ;
            nVertErrTot += 2000 ;
          } 
          */
        } //end switch P_Mode

      } //endfor iLine (source line index)

      // We may not have filled up the available height;
      // so calculate back from what was built
      iUse_Height = (iLine - iGet_yFrom + 1) * 2048 / iAspVert
                    + prect.top;
      if (prect.bottom > iUse_Height)
          prect.bottom = iUse_Height;


      //if (DBGflag)
      //   sprintf(szMsgTxt, "vH=%d", iDDO_LineCtr);
    } // endif pointer OK

    if (IDirectDrawSurface_Unlock(lpOverlay, NULL) !=DD_OK )
    {
        iError++;
    }

   } // endif Surface_Lock succeeded
  } // endif P_Mode not backward


  // SHOW
  if (! iError  &&  iPrim_Width > 8  &&  iDDO_LineCtr > 8 )
  {
    // Populate overlay internal rect corners

    /*
    if (iPut_Width   > VGA_Width)
        iPut_Width   = VGA_Width;
    if (iDDO_LineCtr > VGA_Height)
        iDDO_LineCtr = VGA_Height;
    */

    SetRect(&orect, 0, 0, iPut_Width, iDDO_LineCtr);

    if (iAudio_Lock && iSync_Diff_ms >= 0) //iFrame_Period_ms)
        iDDO_Frame_Ready = 1;
    else
    {
        // Map internal Overlay data into view rectangle of client area
      R250_SIGNAL_Overlay(); // D200_UPD_Overlay();
    }

    strcpy(RecoveryReason,"?  ");
      //} // was: endif Surface_Unlock
  } // was: endif Surface_Lock

}



//-------------------------------------------------------------

// Using the separate 4:2:0 component buffers
// Update the display output YUY2 overlay
//
// 3 Modes: -
//    Mode  0 - Grey out the image, to indicate EOF reached
//    Mode  1 - Normal output of current frame
//    Mode -1 - PLANNED - Backward a frame by toggling YUV ovelay buffers.

void RenderF420(int P_Mode)  // Fast420 
{
  int iLine, iUse_Height;
  int iChromaWidthAdj, iChromaStride, iChromaStride2;
  int iChromaStage, iChromaRipple;
  int nVertErrTot, iError, iRetry, iOffset_Y, iOffset_UV;

  unsigned char  *srcY,  *srcU,  *srcV;
  unsigned char  *src2Y, *src2U, *src2V; // , *lpTmp1;
  unsigned char  *srcY_eol, *srcU_eol, *srcV_eol;
  unsigned char  *dst; // ,   *dstU,  *dstV;
  unsigned char  *dst2,  *dst2U, *dst2V, *dst_MAX;
  HRESULT hRC;
  DWORD register dwPUT;

  iError = 0;

  // Check for usability of frame pointer
  // Maybe something somewhere is freeing the buffers ?

  if (y444 !=  bwd_ref_frame[0]
  &&  y444 !=  fwd_ref_frame[0]
  &&  y444 !=  aux_frame    [0]
  &&  y444 !=  LumAdjBuf)
  {
        strcpy(szMsgTxt,  "Frame PTR Lost");
        DSP1_Main_MSG(0,0);

        /*
        if (DBGflag)
        {
           sprintf(szBuffer, "Frame PTRs y444=x%08X bwd=x%08X fwd==x%08X aux=x%08X  cur==x%08X",
                       y444, bwd_ref_frame[0], fwd_ref_frame[0], 
                             aux_frame    [0], curr_frame   [0] );
           DBGout(szBuffer);
           if (PlayCtl.iShown_Frames < 5)
           {
              fflush(DBGfile); // DBGctl();mDBGctl();
           }
        }
        */

        return;
  }
  // CALCULATE POINTS TO COPY

  srcY = y444;

  // Optionally swap UV to allow for weird captures
  if (iView_SwapUV)
  {
    srcU = currV;
    srcV = currU;
  }
  else
  {
    srcU = currU;
    srcV = currV;
  }

  // Analyse Canvas Sizes

  View_Visible_Calc(iZoom, 1);
  // iViewWidth2 = iAspect_Width2;
  if ((iGet_xFromBytes + iGet_WidthBytes) > Coded_Pic_Width)
  {
     iGet_WidthBytes = Coded_Pic_Width - iGet_xFromBytes;
  }

  // Interlaced 420 chroma is weird.  Others are easy.
  iChromaWidthAdj   = Chroma_Width;
  iChromaStride2    = iChromaWidthAdj;

  iChromaRipple = 0;

  if (MPEG_Seq_chroma_format >= CHROMA422)  // 4:2:2  or 4:4:4
      iChromaRipple = -1;
  else
  if (Deint_VIEW && !ScanMode_code)  // deinterlace an interlaced source
  {
      iChromaRipple = 2;
      iChromaWidthAdj = Chroma_Width * 2;
      // KLUGE
      if (Coded_Pic_Width > 1440) // 720)
      { 
         if (iZoom != 3) 
             iChromaStride2  = iChromaWidthAdj;
      }
  }


  // Adjust inital start positions for current view 
  iOffset_Y = (iView_yFrom * iSrcLineBytes);

  if (iView_Invert)
  {
      iOffset_Y += (iGet_Height - 1) * iSrcLineBytes;
      iSrcLineBytes  = -iSrcLineBytes;
      iSrcLineBytes2 = -iSrcLineBytes2;
      iChromaStride  = -iChromaWidthAdj;
      iChromaStride2 = -iChromaStride2;
  }
  else
  {
      iChromaStride = iChromaWidthAdj;
  }

  iOffset_UV = iOffset_Y; 
  if ( MPEG_Seq_chroma_format  < CHROMA444) // Is Chroma Height sub-sampled ?
       iOffset_UV = (iOffset_UV>>2)<<1;

  iOffset_Y  +=  iGet_xFromBytes;
  iOffset_UV +=  iGet_xFromBytes;

  if ( MPEG_Seq_chroma_format  < CHROMA422) // Is Chroma Width sub-sampled ?
  {
       iOffset_UV = (iOffset_UV>>2)<<1;
       // KLUGE.  TODO: FIGURE OUT RELIABLE RULE FOR THIS
       if (iView_Invert && Coded_Pic_Width <= 720 && iZoom < 2) 
           iOffset_UV += (Chroma_Width / 2);
  }


  srcY += iOffset_Y;

  srcU += iOffset_UV;
  if (iConverge_Blue_H)
  {
    if (Coded_Pic_Width > 400)
        srcU += 4;
    else
        srcU += 2;
  }
  if (iConverge_Blue_V)
        srcU += Chroma_Width;


  srcV += iOffset_UV;

  if (iConverge_Red_H)
  {
    if (Coded_Pic_Width > 400)
        srcV += 4;
    else
        srcV += 2;
  }
  if (iConverge_Red_V)
        srcV += Chroma_Width;





  

#ifdef DBG_RJ
  if (DBGflag)
  {
    sprintf(szBuffer, "srcY=x%08X  yFrom=%d GBot=%d B=%d\nY444=x%08X  xFrom=%d GWB=%d W=%d=%d   DD=%d",
                       srcY, 
                       iGet_yFrom, iGet_Bot, iOffset_UV,
                       y444,
                       iGet_xFromBytes, iGet_WidthBytes, 
                       Coded_Pic_Width, iSrcLineBytes,
                       DDraw_Surface_Size);
    DBGout(szBuffer);
    if (PlayCtl.iShown_Frames < 5)
    {
       fflush(DBGfile); // DBGctl(); DBGctl();
    }
  }
#endif


  nVertErrTot = 0; iDDO_LineCtr = 0 ;

  //  COPY THE IMAGE INFORMATION INTO Direct Draw Overlay buffer

  // Check for occasional loss of Overlay pointer - Don't know why it gets lost
  if ( ! lpOverlay)
  {
    strcpy(szMsgTxt,  "DDOvrlayPtr LOST");
    DSP1_Main_MSG(0,0);

    // Try to get it back         //IDirectDrawSurface_Restore(lpOverlay);

    Calc_PhysView_Size();                     // from RenderF420
    if ( MParse.SeqHdr_Found_Flag)
         Mpeg_Aspect_Resize();

    D100_CHECK_Overlay();
    if (DDOverlay_Flag)
    {
        R250_SIGNAL_Overlay(); // D200_UPD_Overlay();
    }
  
    if ( ! lpOverlay)
    {
        DD_PtrLost_Box(&"Overlay");
        D300_FREE_Overlay();
        MParse.Fault_Flag = CRITICAL_ERROR_LEVEL;
        return;
    }
  }

  if (P_Mode > -1)   // Skip build for backward a frame
  {
    hRC = -1;
    for (iRetry=30; iRetry--;)   // Allow for resource busy
    {
      hRC = IDirectDrawSurface_Lock(lpOverlay, NULL, &ddsd, 0, NULL) ;
      if ( hRC == DD_OK  ||  iRetry <= 0)
        break;
      else

      if (hRC == DDERR_WASSTILLDRAWING
      ||  hRC == DDERR_VERTICALBLANKINPROGRESS)
      {
         Sleep(3);
      }
      else
         Sleep(1); //  break;
    }

    if (DBGflag && iRetry)
    {
        sprintf(szBuffer, "Retry %d*2ms, Aud=%d %03dms", (5-iRetry), iWAVEOUT_Scheduled_Blocks, iWavQue_ms);
        DBGout(szBuffer);
    }

    if (hRC != DD_OK)  // DRAT - Cannot get the surface
    {
      iError++;    //Msg_LastError("DDSurfLoc", hRC, 0);
      if ((PlayCtl.iErrMsg < 5 && iMsgLife < 1) || DBGflag)
      {
         PlayCtl.iErrMsg++;
         if (hRC == DDERR_WASSTILLDRAWING)
            strcpy(szTmp32,"WasStillDrawing");
         else
           sprintf(szTmp32, "x%X ", hRC);

         sprintf(szMsgTxt, "DDSurfLoc ERR %s", szTmp32);
         DSP1_Main_MSG(1,0);
      }
    }
    else
    {
      if (! ddsd.lpSurface)
      { 
        if (! iError)
           DD_PtrLost_Box(&"Surface");
        iError++;
      }
      else
      {
         iChromaStage = 0;

         dst = (unsigned char *)ddsd.lpSurface;
         dst_MAX = dst  +  DDraw_Surface_Size - iGet_WidthBytes;

         // YV12 pointers - EXPERIMENTAL
         dst2U   = dst   +  DDraw_Canvas_Size;           // YV12 pointers
         dst2V   = dst2U + (DDraw_Canvas_Size / 4);      // YV12 pointers

         // copy from the internal yuv buffers to the Direct Draw yuy2 Overlay

         for ( iLine = iGet_yFrom; 
              (iLine < iGet_Bot 
                  && dst < dst_MAX 
                  && dst >= (unsigned char *)ddsd.lpSurface);
               iLine++)
         {
           if (P_Mode)
           {
             dst2     =   dst;
             src2U    =   srcU;
             src2V    =   srcV;
             srcY_eol =   srcY  + iGet_WidthBytes;
             srcU_eol =   src2U + iChromaWidthAdj;
             srcV_eol =   src2V + iChromaWidthAdj;

             switch (iZoom)
             {

              //   ZOOM code should be rewritten in Assembler
              case 2:  // ZOOM OUT by factor of 2
                  // Optionally allow for different Video Card component ordering
                if (ddPixelFormat.dwFourCC == 'YVYU') // UYVY
                {
                     for (src2Y = srcY    ;
                          src2Y < srcY_eol ;)
                     {
                        *dst2++  =  Sat_Tbl_U[*src2U]; //   do 1 byte
                         src2U   =  src2U+2; // skip 2 byte
                       *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; // next 1 byte  
                         src2Y   =  src2Y+2; // skip 2 bytes
                        *dst2++  =  Sat_Tbl_V[*src2V]; //   do 1 byte
                         src2V   =  src2V+2; // skip 2 byte
                       *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; // next 1 byte  
                         src2Y   =  src2Y+2; // skip 2 bytes
                      } //endfor
                }
                else
                if (iLumEnable_Flag[0] || iSatAdj_Flag)
                {
                  if (iCtl_Ovl_DWord)
                  {
                     // Build a DWORD before accessing BUS
                     for (src2Y = srcY    ;
                          src2Y < srcY_eol ;)
                     {
                         dwPUT  = Lum_Tbl_OVL[*(src2Y)]; // next 1 byte  
                         src2Y  =  src2Y+2; // skip 2 bytes
                       //dwPUT +=           (*src2U)<<8;  // copy 1 byte
                         dwPUT +=  Sat_Tbl_U[*src2U]<<8; //   do 1 byte
                         src2U  =  src2U+2; // skip 2 byte
                         dwPUT += (Lum_Tbl_OVL[*(src2Y)])<<16; // next 1 byte  
                         src2Y  =  src2Y+2; // skip 2 bytes
                       //dwPUT +=           (*src2V)<<24;  // copy 1 byte
                         dwPUT +=  Sat_Tbl_V[*src2V]<<8; //   do 1 byte
                         src2V  =  src2V+2; // skip 2 byte
                        *(DWORD*)(dst2) = dwPUT; // Write to BUS
                         dst2  +=4;
                      } //endfor
                  }
                  else
                  {
                     for (src2Y = srcY    ;
                          src2Y < srcY_eol ;)
                     {
                       *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; // next 1 byte  
                         src2Y   =  src2Y+2; // skip 2 bytes
                      //*dst2++  =            *src2U;   // copy 1 byte
                        *dst2++  =  Sat_Tbl_U[*src2U]; //   do 1 byte
                         src2U   =  src2U+2; // skip 2 byte
                       *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; // next 1 byte  
                         src2Y   =  src2Y+2; // skip 2 bytes
                      //*dst2++  = *src2V;   // copy 1 byte
                        *dst2++  =  Sat_Tbl_V[*src2V]; //   do 1 byte
                         src2V   =  src2V+2; // skip 2 byte
                      } //endfor
                  }
                }

                else
                {
                   for (src2Y = srcY    ;
                        src2Y < srcY_eol ;)
                   {
                     // Copy YxYz, then replace "x" and "z"  U and V
  
                     *(UNALIGNED DWORD *)(dst2) = *(UNALIGNED DWORD *)(src2Y);  // copy 4 bytes at once
                      src2Y  =  src2Y+4; // skip 4 bytes
                      dst2++;            // skip 1 byte
                     *dst2   = *src2U;   // copy 1 byte
                      dst2   =  dst2 +2; // skip 2 byte
                      src2U  =  src2U+2; // skip 2 byte
                     *dst2++ = *src2V;   // copy 1 byte
                      src2V  =  src2V+2; // skip 2 byte
                   } //endfor
                }

                break;

              case 3:  // ZOOM OUT by factor of 3 - Easy !

                if (ddPixelFormat.dwFourCC == 'YVYU') // UYVY
                {
                   for (src2Y = srcY    ;
                        src2Y < srcY_eol ;)
                   {
                     *dst2++ =  Sat_Tbl_U[*src2U]; //   do 1 U byte
                      src2U  =  src2U+3;           // skip 2 bytes
                    *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; // do 1 Y byte  
                      src2Y  =  src2Y+3;               // skip 2 bytes
                     *dst2++ =  Sat_Tbl_V[*src2V]; //   do 1 V byte
                      src2V  =  src2V+3;           // skip 2 bytes
                    *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; // do 1 Y byte  
                      src2Y  =  src2Y+3;               // skip 2 bytes
                   } //endfor
                }
                else
                //if (iLumEnable_Flag[0] || iSatAdj_Flag)
                {
                   for (src2Y = srcY    ;
                        src2Y < srcY_eol ;)
                   {
                    *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; //   do 1 Y byte  
                      src2Y  =  src2Y+3;               // skip 2 Y bytes
                     *dst2++ =  Sat_Tbl_U[*src2U];    //   do 1 U byte
                      src2U  =  src2U+3;              // skip 2 U bytes
                    *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; //   do 1 Y byte  
                      src2Y  =  src2Y+3;               // skip 2 Y bytes
                     *dst2++ =  Sat_Tbl_V[*src2V];   //   do 1 V  byte
                      src2V  =  src2V+3;             // skip 2 V bytes
                   } //endfor
                }
                /*
                else
                {  // NO ADJUST
                   for (src2Y = srcY    ;
                        src2Y < srcY_eol ;)
                   {
                     *dst2++ = *src2Y;   // copy 1 byte
                      src2Y  =  src2Y+3; // skip 2 bytes
                     *dst2++ = *src2U;   // copy 1 byte
                      src2U  =  src2U+3; // skip 2 bytes
                     *dst2++ = *src2Y;   // copy 1 byte
                      src2Y  =  src2Y+3; // skip 2 bytes
                     *dst2++ = *src2V;   // copy 1 byte
                      src2V  =  src2V+3; // skip 2 bytes
                   } //endfor
                }
                */
                break;


              case 4:  // ZOOM OUT by factor of 4

                if (ddPixelFormat.dwFourCC == 'YVYU') // UYVY
                {
                   for (src2Y = srcY    ;
                        src2Y < srcY_eol ;)
                   {
                     *dst2++ =  Sat_Tbl_U[*src2U]; //   do 1 U byte
                      src2U  =  src2U+4;           // skip 3 bytes
                    *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; // do 1 Y byte  
                      src2Y  =  src2Y+4;               // skip 3 bytes
                     *dst2++ =  Sat_Tbl_V[*src2V]; //   do 1 V byte
                      src2V  =  src2V+4;           // skip 3 bytes
                    *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; // do 1 Y byte  
                      src2Y  =  src2Y+4;               // skip 3 bytes
                   } //endfor
                }
                else
                {
                   for (src2Y = srcY    ;
                        src2Y < srcY_eol ;)
                   {
                    *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; //   do 1 Y byte  
                      src2Y  =  src2Y+4;               // skip 3 Y bytes
                     *dst2++ =  Sat_Tbl_U[*src2U];    //   do 1 U byte
                      src2U  =  src2U+4;              // skip 3 U bytes
                    *(dst2++) = Lum_Tbl_OVL[*(src2Y)]; //   do 1 Y byte  
                      src2Y  =  src2Y+4;               // skip 3 Y bytes
                     *dst2++ =  Sat_Tbl_V[*src2V];   //   do 1 V  byte
                      src2V  =  src2V+4;             // skip 3 V bytes
                   } //endfor
                }
                break;

             default:  // NO ZOOM - Just copy byte for byte in a line segment

                // Optionally allow for different Video Card compoent ordering
                // copy from the internal yuv buffers to Direct Draw yuv Overlay
                if (ddPixelFormat.dwFourCC == '21VY') // YV12 - PLANAR
                {
                     for (src2Y = srcY    ;
                          src2Y < srcY_eol ;)
                     {
                        *(DWORD*)(dst2) = *(DWORD*)(src2Y); // Write to BUS
                         dst2  +=4;
                         src2Y +=4;
                     }

                     /*
                     src2V = srcV;
                     for (src2U = srcU    ;
                          src2U < srcU_eol ;)
                     {
                        *dst2U++ = *src2U++; 
                        *dst2U++ = *src2V++; 
                     }
                     */       

                     
                     for (src2U = srcU    ;
                          src2U < srcU_eol ;)
                     {
                        *(DWORD*)(dst2U) = 0x80808080; // 0x8F8F8F8F8; // *(DWORD*)(src2U); // Write to BUS
                         dst2U  +=4;
                         src2U  +=4;
                     }
                            

                     
                     for (src2V = srcV    ;
                          src2V < srcV_eol ;)
                     {
                        *(DWORD*)(dst2V) = 0x80808080;// *(DWORD*)(src2V); // Write to BUS
                         dst2V  +=4;
                         src2V  +=4;
                     }
                            


                }
                else
                if (ddPixelFormat.dwFourCC == 'YVYU') // UYVY
                {
                     for (src2Y = srcY    ;
                          src2Y < srcY_eol ;)
                     {
                       *dst2++  =    Sat_Tbl_U[*(src2U++)]; //   do 1 byte
                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                       *dst2++  =    Sat_Tbl_V[*(src2V++)]; //   do 1 byte
                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                       *dst2++  =    Sat_Tbl_U[*(src2U++)]; //   do 1 byte
                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                       *dst2++  =    Sat_Tbl_V[*(src2V++)]; //   do 1 byte
                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                     }
                }
                else                                       // YUY2
                if (iLumEnable_Flag[0] || iSatAdj_Flag)
                {
                  if (iSatAdj_Flag)
                  {
                    if (iSatGain[0] == 100 && !iSat_VHS && !iSat_Sine)
                    {
                     for (src2Y = srcY    ;
                          src2Y < srcY_eol ;)  // THERE BE DRAGONS HERE
                     {

                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // Y  next 1 byte  
                       *dst2++  = *(src2U++)+iSatAdd_U[0];     // U  do 1 byte
                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // Y  next 1 byte  
                       *dst2++  = *(src2V++)+iSatAdd_V[0];     // V  do 1 byte
                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // Y  next 1 byte  
                       *dst2++  = *(src2U++)+iSatAdd_U[0];     // U  do 1 byte
                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // Y  next 1 byte  
                       *dst2++  = *(src2V++)+iSatAdd_V[0];     // V  do 1 byte
                     } //endfor
                    } // endif
                    else
                    {
                     for (src2Y = srcY    ;
                          src2Y < srcY_eol ;)  // THERE BE DRAGONS HERE
                     {

                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                       *dst2++  =    Sat_Tbl_U[*(src2U++)]; //   do 1 byte
                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                       *dst2++  =    Sat_Tbl_V[*(src2V++)]; //   do 1 byte
                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                       *dst2++  =    Sat_Tbl_U[*(src2U++)]; //   do 1 byte
                      *(dst2++) =  Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                       *dst2++  =    Sat_Tbl_V[*(src2V++)]; //   do 1 byte
                      } //endfor
                    } // endelse                     
                  }
                  else
                  {  // NO SAT ADJUST
                    if (iCtl_Ovl_DWord)
                    {
                      for (src2Y = srcY    ;
                           src2Y < srcY_eol ;)  // THERE BE DRAGONS HERE
                      {
                         // Build a DWORD before accessing BUS
                         dwPUT  = Lum_Tbl_OVL[*(src2Y++)];
                         dwPUT += (*src2U++)<<8;
                         dwPUT += Lum_Tbl_OVL[*(src2Y++)]<<16;
                         dwPUT += (*src2V++)<<24;
                        *(DWORD*)(dst2) = dwPUT; // Write 4 bytes at once to BUS
                         dst2  +=4;

                         // Build a DWORD before accessing BUS
                         dwPUT  = Lum_Tbl_OVL[*(src2Y++)];
                         dwPUT += (*src2U++)<<8;
                         dwPUT += Lum_Tbl_OVL[*(src2Y++)]<<16;
                         dwPUT += (*src2V++)<<24;
                        *(DWORD*)(dst2) = dwPUT; // Write to BUS
                         dst2  +=4;

                      }//endfor
                    }
                    else
                    {
                      for (src2Y = srcY    ;
                           src2Y < srcY_eol ;)  // THERE BE DRAGONS HERE
                      {
                        *(dst2++) = Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                        *dst2++ = *src2U++; // copy 1 byte
                        *(dst2++) = Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                        *dst2++ = *src2V++; // copy 1 byte

                        *(dst2++) = Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                        *dst2++ = *src2U++; // copy 1 byte
                        *(dst2++) = Lum_Tbl_OVL[*(src2Y++)]; // next 1 byte  
                        *dst2++ = *src2V++; // copy 1 byte
                       
                      }//endfor

                    } // endelse DWORD access
                  } // endelse  LUMSAT
                } // endelse LUM_OVL
                else
                {  // NO ADJUST
                   for (src2Y = srcY    ;
                        src2Y < srcY_eol ;)
                   {
                     *dst2++ = *src2Y++; // next 1 byte  THERE BE DRAGONS HERE
                     *dst2++ = *src2U++; // copy 1 byte
                     *dst2++ = *src2Y++; // next 1 byte
                     *dst2++ = *src2V++; // copy 1 byte
                     *dst2++ = *src2Y++; // next 1 byte
                     *dst2++ = *src2U++; // next 1 byte
                     *dst2++ = *src2Y++; // next 1 byte
                     *dst2++ = *src2V++; // next 1 byte
                   } //endfor
                }

                break;
             } // end-switch ZOOM
           }
           else
           { // END OF FILE - SPECIAL DISPLAY
             //if (iDDO_LineCtr++ & 1)
                  ZeroMemory(dst, iGet_WidthBytes);  // effectively wash-out the image
             dst += ddsd.lPitch;
             iLine+=3;
             iDDO_LineCtr++ ;
             nVertErrTot-=iVertInc ;
           } // end-if


           srcY += iSrcLineBytes;
           dst  += ddsd.lPitch;

           if (iChromaStage > iChromaRipple)
           {
               iChromaStage = 0;
               if (iView_Invert)
               {
                  srcU += iChromaStride;
                  srcV += iChromaStride;
               }
               else
               {
                  srcU = srcU_eol; // += iChromaStride;
                  srcV = srcV_eol; // += iChromaStride;
               }
           }
           else
               iChromaStage++;

           // Anamorphic scaling as per Aspect Ratio correction factor
           // The maths here is quick & dirty, but it will do for now.
           iDDO_LineCtr++ ;

           if (P_Mode)
           {
              nVertErrTot += iVertInc ;

              if (Deint_VIEW)
              {
                 srcY += iSrcLineBytes ;
                 iLine++ ;

                 if (iChromaStage > iChromaRipple)
                 {
                    iChromaStage = 0;
                    srcU += iChromaStride;
                    srcV += iChromaStride;

                 }
                 else
                    iChromaStage++;
              } // ENDIF Deinterlace

              // Squeeze Vertical by skipping selected lines
              while (nVertErrTot >= 2000)
              {
                nVertErrTot = nVertErrTot - 2000 + iVertInc;

                if (Deint_VIEW)
                {
                   srcY += iSrcLineBytes2 ;
                   iLine+=2 ;
                   srcU += iChromaStride2; 
                   srcV += iChromaStride2; 
                   // KLUGE
                   //if (Coded_Pic_Width > 720 && iZoom == 3) 
                   //    iChromaStage = 0;

                }
                else
                {
                   srcY += iSrcLineBytes ;
                   iLine++ ;
                   if (iChromaStage > iChromaRipple)
                   {
                       iChromaStage = 0;
                       srcU += iChromaStride2; 
                       srcV += iChromaStride2; 
                   }
                   else
                       iChromaStage++;
                } // end-if Deint
              } //end-while nVertErr
/*
              // Stretch Vertical by doubling selected lines
              // *** NOW DONE VIA Direct Draw Overlay for better performance
 
              if (nVertErrTot <= -2000)
              {
                 memcpy(dst, (dst - ddsd.lPitch), ddsd.lPitch); //DOUBLE_CLIP_WIDTH);
                 dst += ddsd.lPitch;
                 iDDO_LineCtr++ ;
                 nVertErrTot += 2000 ;
              }
*/   
           } //end switch P_Mode

         } //endfor iLine (source line index)

         // We may not have filled up the available height;
         // so calculate back from what was built
         iUse_Height = (iLine - iGet_yFrom + 1) * 2048 / iAspVert
                     + prect.top;
         if (prect.bottom > iUse_Height)
             prect.bottom = iUse_Height;

      } // ENDIF Pointer OK

      //if (DBGflag)
      //   sprintf(szMsgTxt, "vH=%d", iDDO_LineCtr);
      hRC = IDirectDrawSurface_Unlock(lpOverlay, NULL);
      if (hRC !=DD_OK )
      {
          iError++;
          sprintf(szBuffer,
               "DDraw Surface Unlock FAILED.\n\n RC=%d",
                                        hRC);
          MessageBox ( NULL, szBuffer, "Mpg2Cut2 - BUG !",
                       MB_OK | MB_ICONEXCLAMATION
                             | MB_SETFOREGROUND | MB_TOPMOST);
      }
    } // endif Surface_Lock OK
  } // endif P_Mode not backward


  // SHOW
  if (! iError  &&  iPrim_Width > 8  &&  iDDO_LineCtr > 8 )
  {
    // Populate overlay internal rect corners
    if (iPut_Width   > iVGA_Avail_Width)
        iPut_Width   = iVGA_Avail_Width;
    if (iDDO_LineCtr > iVGA_Avail_Height)
        iDDO_LineCtr = iVGA_Avail_Height;
    if (iPut_Width   > (int)(ddsd.dwWidth)) // DDraw_dwWidth)
        iPut_Width   = (int)(ddsd.dwWidth); // DDraw_dwWidth; 
    if (iDDO_LineCtr > (int)ddsd.dwHeight)  
        iDDO_LineCtr = (int)ddsd.dwHeight;

    SetRect(&orect, 0, 0, iPut_Width, iDDO_LineCtr);

    if (iAudio_Lock && iSync_Diff_ms > 0)   // iFrame_Period_ms)
        iDDO_Frame_Ready = 1;
    else
    {
      // Map internal Overlay data into view rectangle of client area
      R250_SIGNAL_Overlay(); // D200_UPD_Overlay();
    }

    strcpy(RecoveryReason,"?  ");

  } // was: endif Surface_Lock

}


/*
void View_FindMasking()
{
  //int iLine, iLimit, iJump1, iJump2, iJump3;
  //unsigned char *lpcTst;


    // Scan Luminance area for possible widescreen mask at top
    iLine = 0;
    iLimit = (Coded_Pic_Height / 3) + 10 ; // Letterbox mask in 4:3 frame
    iJump1 = Coded_Pic_Width / 12;
    iJump2 = iJump1 * 3;
    iJump3 = Coded_Pic_Width - iJump2; // Allow for rounding error 
    lpcTst = y444 + iJump2;            // Skip the left hand pillarbox's margin

    for (;;)
    {
      if (*lpcTst > 19)   // Leftish
        break;

      lpcTst += iJump1;

      if (*lpcTst > 19)   // 
        break;

      lpcTst += iJump1;

      if (*lpcTst > 19)   //
        break;

      lpcTst += iJump1;

      if (*lpcTst > 19)   // Middle
        break;

      lpcTst += iJump1;

      if (*lpcTst > 19)   // 
        break;

      lpcTst += iJump1;

      if (*lpcTst > 19)   //
        break;

      lpcTst += iJump1;

      if (*lpcTst > 19)   // Rightish
        break;

      lpcTst += iJump3;   // Next line
      iLine++;
    }


    //if (iLine == 0 && process.iView_Extra_Crop) // Force Zoom in ?
    //    iLine = Coded_Pic_Height / 3;

    iLimit -= 5; // Check we found an edge, not just a dark frame.
    if (iLine < iLimit && iLine > 10)
    {
      process.iView_TopMask = iLine;
      //if (iView_Centre_Crop)
      {
         iView_yFrom   = (iLine - 4) & 0xFFFFFFFE;
         if (iView_yFrom < 0)
             iView_yFrom = 0;
      }
    }
    else
      process.iView_TopMask = 0;

  }
}
*/  

POINT  OLD_Point;

//---------------------------
void View_Visible_Calc(int P_Zoom, int P_Pixel_Width)
{
  int iMax, iW_Zoom, iRC;
  int iExtraZoom;

  iExtraZoom = 0;

  if (P_Zoom < 1)
  {
     iW_Zoom = 1;
     if (iView_Centre_Crop && process.iView_Extra_Crop)
     {
        iExtraZoom = 1; 
     }
  }
  else
     iW_Zoom = P_Zoom;

  if ( ! Clip_Width)    
         Clip_Width = 32;
  if ( ! P_Pixel_Width) 
         P_Pixel_Width = 1;

  iSrcLineBytes  = Clip_Width  * P_Pixel_Width;
  iSrcLineBytes2 = iSrcLineBytes * 2;

  //VGA_Width  = GetSystemMetrics(SM_CXSCREEN) ;
  //VGA_Height = GetSystemMetrics(SM_CYSCREEN) ;

  /*
  if (process.iView_TopMask < 0  &&  y444   // First time in new mode ?
  // && Frame_Number
  &&  Coded_Pic_Height      > 288)  
  {
       View_FindMasking();
  */
  
  iGet_xFromBytes = iView_xFrom * P_Pixel_Width;
  iGet_yFrom      = iView_yFrom;

  if (IsIconic(hWnd_MAIN))
  {
      iView_Point.x = OLD_Point.x;
      iView_Point.y = OLD_Point.y;
  }
  else
  {
      iView_Point.x = 0;
      iView_Point.y = iTopMargin;  // 0;
      iRC = ClientToScreen(hWnd_MAIN, &iView_Point); // Convert relative pos to absolute pos
      if (!iRC  // Error ?
      ||  iView_Point.x < -2048      // insane ??
      ||  iView_Point.y < -2048  )   // insane ??
      {
        iView_Point.x = OLD_Point.x;
        iView_Point.y = OLD_Point.y;
      }
      else
      {
         OLD_Point.x = iView_Point.x;
         OLD_Point.y = iView_Point.y;
      }
  }

  prect.left   = iView_Point.x;
  prect.top    = iView_Point.y;  // Absolute screen co-ord of TopMargin 
  prect.right  = prect.left + iAspect_Width;
  prect.bottom = prect.top  + iAspect_Height;


  // Trim rectangles to allow for window partially off-screen

  if (prect.left < 0)
  {
      if (prect.left < -2048)  // sanity check
          prect.left = 0;
      //prect.right  +=  prect.left;
      iGet_xFromBytes  -= (prect.left * P_Pixel_Width);
      prect.left    = 0;
  }

  if (prect.top < 0)
  {
    //prect.bottom -= prect.top;
    iGet_yFrom   -= prect.top;
    prect.top     = 0;
  }

  iGet_yFrom = iGet_yFrom & 0xFFFFFE;

  if (prect.right  > VGA_Width)
      prect.right  = VGA_Width;

  if (prect.bottom > VGA_Height)
      prect.bottom = VGA_Height;

  prect.right--;   // Convert to Origin 0
  prect.bottom--;  // Convert to Origin 0

  // Align the image horizontally
  if (iGet_xFromBytes < 0 || iGet_xFromBytes > iSrcLineBytes)
  {
      sprintf(szMsgTxt, "*BUG* View xFrom= %d;%d  Max= %d\n\nWidth=%d Height=%d",
                          iView_xFrom, 
                          iGet_xFromBytes, iSrcLineBytes,
                          MPEG_Seq_horizontal_size,
                          MPEG_Seq_vertical_size);
      iGet_xFromBytes = 0;
  }

  // Align the image vertically
  iMax = Coded_Pic_Height - 1;

  if (iGet_yFrom > iMax || iGet_yFrom < 0)
  {
      sprintf(szMsgTxt, "BUG-yFrom=%d Max=%d", iGet_yFrom, iMax);
      iGet_yFrom = 0;
  }

  iPrim_Height = prect.bottom - prect.top; // Available Height on screen
  iPrim_Width  = prect.right  - prect.left ; // How much space to fill on screen ?
  if (iMainWin_State == 0)
      iPrim_Width -=1;
  else
      iPrim_Width +=1;


  // How many lines do we need to get in order to fill this view ?

  if (MParse.iColorMode != STORE_RGB24)
  {
      iGet_Height = iPrim_Height * iAspVert / 2048;

      if (iExtraZoom)
          iGet_Height = iGet_Height * 3 / 4;

  }
  else
  {
      iGet_Height = Coded_Pic_Width * 2048 / iAspVert * iW_Zoom;
  }

  if (iGet_Height > Clip_Height)
  {
      iGet_Height = Clip_Height;
      iView_yFrom = 0;
  }

  //if (Deint_VIEW)
  //    iGet_Height = iGet_Height * 2;
  iGet_Bot = iGet_Height  + iView_yFrom;
  if (iGet_Bot > Clip_Height)
  {
      iGet_Bot = Clip_Height;
      iGet_Height = iGet_Bot - iView_yFrom;
  }

  if (iCtl_CropTop )
  {
      iGet_yFrom  +=2;
      iMax        -=4;
      iGet_Height -=4;
      iGet_Bot    -=4;
  }


  // COLUMNS  - How many columns ... ?

/*
  iMax = Coded_Pic_Width - (iGet_xFromBytes / P_Pixel_Width); // How Many Available ?
  if (iPrim_Width > iMax)
      iPut_Width = iMax;
  else
      iPut_Width = iPrim_Width;

  iGet_Width = iPut_Width * 2048 / iAspHoriz * iW_Zoom;
  iGet_WidthBytes = iGet_Width * P_Pixel_Width;

*/
  if (MParse.iColorMode != STORE_RGB24)
  {
      iGet_Width = iPrim_Width * Coded_Pic_Width / iAspect_Width;

      if (iExtraZoom || iCentre_Cropped) //  &&  process.iView_Extra_Crop))
          iGet_Width = iGet_Width * 4 / 5;
  }
  else 
      iGet_Width = Coded_Pic_Width;

  iMax = Coded_Pic_Width - (iGet_xFromBytes / P_Pixel_Width); // How Many Available ?
  if (iGet_Width > iMax)
      iGet_Width = iMax;

  iGet_WidthBytes = iGet_Width * P_Pixel_Width;
  iPut_Width = iGet_Width / iW_Zoom;


  if (DBGflag &&  process.Action != ACTION_RIP)
  {
      sprintf(szBuffer, "Rect=%d-%d=%d %d-%d=%d  Pic=%d.%d\nGetW=%d=%d Max=%d PrimW=%d PhysW=%d PutW=%d AspW=%d AspOut=%d\nMax=%d FromW=%d~%d PxW=%d Z=%d\nGetH=%d PrimH=%d PhysH=%d BotH=%d AspV=%d TopH=%d\n\n",
              prect.left, prect.right, (prect.right-prect.left+1),
              prect.top, prect.bottom, (prect.bottom-prect.top+1),
              Coded_Pic_Width, Coded_Pic_Height,
              iGet_Width, iGet_WidthBytes, iMax,
              iPrim_Width, iPhysView_Width, 
              iPut_Width, iAspect_Width, iAspectOut,
              iMax, iView_xFrom, iGet_xFromBytes, P_Pixel_Width, iW_Zoom,
              iGet_Height, iPrim_Height, iPhysView_Height, 
              iGet_Bot, iAspVert, iGet_yFrom);
      DBGout(szBuffer);
  }

}


 


//--------------------------------------

void  Render_Init()
{
  unsigned char  *lpTmp, *lpEnd;

  //int i;
  //i = Coded_Pic_Width ;
  //i = Clip_Height;

  iLUM_AREA      = Coded_Pic_Width * Coded_Pic_Height;

  // RGB 
    ZeroMemory(&birgb, sizeof(BITMAPINFOHEADER));
    birgb.biSize = sizeof(BITMAPINFOHEADER);
    birgb.biWidth  = Clip_Width;
    birgb.biHeight = Clip_Height;
    birgb.biPlanes = 1;
    birgb.biBitCount = 24;
    birgb.biCompression = BI_RGB;
    birgb.biSizeImage = birgb.biWidth * birgb.biHeight * 3;

  // YUY2
    ZeroMemory(&biyuv, sizeof(BITMAPINFOHEADER));
    biyuv = birgb;
    biyuv.biBitCount = 16;
    biyuv.biCompression = mmioFOURCC('Y','U','Y','2');
    biyuv.biSizeImage = Clip_Width * Clip_Height * 2;


    //if (process.Action == ACTION_RIP
    //&&  MParse.FO_Flag == FO_SWAP
    //&&  Pic_Started)
    {
      // mask
      if (MParse.iColorMode==STORE_YUY2)
      {
         //for (i = 0; i < (Coded_Pic_Width * Coded_Pic_Height * 2);
            //                 /* i += 2 */   i += 4 )
         lpEnd = yuy2 + (Coded_Pic_Width * Coded_Pic_Height * 2);
         for (lpTmp = yuy2;
                 lpTmp < lpEnd;
                 lpTmp +=4)
         {
           // yuy2[i] = 0;
           // yuy2[i+1] = 128;
           //*(UNALIGNED DWORD *)(yuy2[i]) = 0x7F007F00;
           *(UNALIGNED DWORD *)(lpTmp) = (UNALIGNED DWORD)(0x7F007F00); // copy 4 bytes at once

         }
      }
      else
      {
         ZeroMemory(rgb24, Coded_Pic_Width * Clip_Height * 3);
      }
    }

}








//--------------------------------------------------------------
// NEW VERSION FOR GAMMA - FROM DVD2AVI 1.77.3

void Lum_Filter_OVL(unsigned char *src, unsigned char *dst)
{
  int i;

  src += CLIP_AREA;  // Offset for clipping
  dst += CLIP_AREA;  // Offset for clipping

  for (i=0; i<iLUM_AREA; i++)
    *(dst++) = Lum_Tbl_OVL[*(src++)];
}



void Lum_Filter_RGB(unsigned char *src, unsigned char *dst)
{
  int i;

  src += CLIP_AREA;  // Offset for clipping
  dst += CLIP_AREA;  // Offset for clipping

  for (i=0; i<iLUM_AREA; i++)
    *(dst++) = Lum_Tbl_RGB[*(src++)];
}


//----------------------------------------------------------

/* OLD LUMINANCE FILTER DID NOT HANDLE GAMMA

static void Lum_Filter_OVL(unsigned char *src, unsigned char *dst)
{
  src += CLIP_AREA;
  dst += CLIP_AREA;

  __asm
  {
    mov     eax, [src]
    mov     ebx, [dst]
    mov     esi, 0x00
    mov     edi, [iLUM_AREA]
    pxor    mm0, mm0

    movq    mm5, [iLumOffsetMask]
    movq    mm6, [iLumGainMask]
    movq    mm7, [mmmask_0064]

LumAdjBufconv:
    movq    mm1, [eax+esi]
    movq    mm2, mm1

    punpcklbw mm1, mm0
    punpckhbw mm2, mm0

    pmullw    mm1, mm6
    pmullw    mm2, mm6

    paddw   mm1, mm7
    paddw   mm2, mm7

    psrlw   mm1, 7
    psrlw   mm2, 7

    paddw   mm1, mm5
    paddw   mm2, mm5

    packuswb  mm1, mm0
    packuswb  mm2, mm0

    add     esi, 0x08
    cmp     esi, edi
    movq    [ebx+esi-8], mm1
    movq    [ebx+esi-4], mm2
    jl      LumAdjBufconv

  }
}
*/


//----------------------------------------------------------
void Lum_Filter_CSpace(unsigned char *P_Lum_Tbl, int P_ColorSpace) 
{
  int i; // j;
  int  iResult, iSigned, iLumEnabled; //, iCtl_SinThreshold;
  double fResult, fPow2iLumGamma, 
                  fiLumGain, 
                  fiLumOffset; //, fLow, fHigh;

  iLumEnabled = iLumEnable_Flag[P_ColorSpace];
  // iCtl_SinThreshold = iCtl_VHS_Threshold * 2;

  fPow2iLumGamma = pow(2.0, -(iLumGamma[P_ColorSpace] - 100)/100.0);
  fiLumGain      = iLumGain[P_ColorSpace]   /128.0;
  fiLumOffset    = iLumOffset[P_ColorSpace] +127.5;

  for (i = 0; i < 256; i++)
  {
    /*
    if (MParse.PC_Range_Flag)
    {
       j = i;
    }
    else
    {
       j = i - 16;
       if (j < 0)
           j = 0;
    }
    */

    if (iLumEnabled && !iLum_Deselected)
    {
       fResult = ( 255.0  * pow( i / 255.0, fPow2iLumGamma)
               -127.0) * fiLumGain   // RJ - REINSTATED GAIN - I LIKE CONTRAST
               + fiLumOffset;

       iResult = (int)fResult;
    }
    else
       iResult = i;
    
    if (iView_Negative)
        iResult = 255 - iResult;

    // Clamp to one-byte value range
    if (iResult < 0)
        iResult = 0;
    else 
    if (iResult > 255)
        iResult = 255;
    
    P_Lum_Tbl[i] = (unsigned char)iResult;



    // SATURATION - NOT QUITE RIGHT YET

    // Convert for byte-signed format
    iSigned = i - 128;

    // Blue Diff
    if (iSatAdj_Flag)
    {
        iResult = (iSigned * iSatGain[0]) / 100 + iSatAdd_U[0] + 128;
    }
    else
       iResult = i;

    if (iResult  < 0)
        iResult  = 0;
    else
    if (iResult > 255)
        iResult = 255;

    Sat_Tbl_U[i] = (unsigned char)(iResult);


    // Red Diff 

//#define VHS_THRESHOLD 32

    if (iSatAdj_Flag)
    {
       if (iSat_VHS && iSigned > iCtl_VHS_Threshold)
       {
          iResult = (             (iCtl_VHS_Threshold  * iSatGain[0]) 
                       + ((iSigned-iCtl_VHS_Threshold) * iSatGain[0] / 3)
                    ) 
                  / 100 
                  + iSatAdd_V[0] + 128;
       }
       else
       if (iSat_Sine && iSigned > iCtl_SinThreshold)
       {
          iResult = (             (iCtl_SinThreshold  * iSatGain[0]) 
                       + ((iSigned-iCtl_SinThreshold) * iSatGain[0] * 2)
                    ) 
                  / 100 
                  + iSatAdd_V[0] + 128;
       }
       else
           iResult = (iSigned * iSatGain[0]) / 100  + iSatAdd_V[0] + 128;
    }
    else
       iResult = i;

    if (iResult  < 0)
        iResult  = 0;
    else
    if (iResult > 255)
        iResult = 255;

    Sat_Tbl_V[i] = (unsigned char)(iResult);
 
  }
}



void Lum_Filter_Init(int P_ColorSpace)
{
  if (P_ColorSpace != 0)
     Lum_Filter_CSpace(&Lum_Tbl_RGB[0], 1);

  if (P_ColorSpace != 1)
     Lum_Filter_CSpace(&Lum_Tbl_OVL[0], 0);

}



