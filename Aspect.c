
#include "global.h"

//-----------------------------------------------------------------------
//Calculate the amount of Aspect ratio correction required
//
// To avoid floating point overhead,
// scaled integers are used.
// I could not find any native support for this in the "C" language,
// so I am doing it manually, by multiplying everything by 2000.
// Give me COBOL !

  int iUnCroppedWidth, iUnCroppedHeight;

void Mpeg_Aspect_Before()
{
  int iAspect_Code, iZoom, iCView_Aspect, iCView_Height;
  int iTmp; //, iTmp2;

  //if ( ! MParse.SeqHdr_Found_Flag)
  //  return;

  // Auto setting of default Zoom (mainly for HDTV on low/mid-res VGA)
  if (iCtl_Zoom_Wanted >= 0)
      iZoom = iCtl_Zoom_Wanted; // Manual Setting
  else
  {
    iTmp = (Coded_Pic_Width*100) / VGA_Width ;
    if (iTmp < 170) // 220)
    {
      if (iTmp < 50) // Resolution less than half the screen ?
          iZoom = 0;
      else
          iZoom = 1;
    }
    else
    {
      if (iTmp >  280)
          iZoom = 3;
      else
          iZoom = 2;
    }

    Set_Zoom_Menu(iZoom);
  }


  //if (iZoom < 2
//  || ((iMainWin_State > 0) 
//        && Coded_Pic_Width  < iViewMax_Width
//        && Coded_Pic_Height < iViewMax_Height)
   //  )
   //   iZoom  = 1;


  if (DBGflag)
  {
      sprintf(szBuffer, "ASP Mode=%d   Zoom=%d [%d]   %d.%d", 
                        iView_Aspect_Mode, iZoom, iCtl_Zoom_Wanted,
                        Coded_Pic_Width, Coded_Pic_Height);
      DBGout(szBuffer);
  }


  iSrcUse = Coded_Pic_Height;// / iZoom;
  if (Deint_VIEW)
      iSrcUse = iSrcUse / 2;


  if ((Coded_Pic_Width != 0) && (Coded_Pic_Height != 0))
  {
     iAspectIn = 2000 * Coded_Pic_Width
                      / Coded_Pic_Height ;
  }
  else
     iAspectIn = 2000;


  if (iView_Aspect_Mode != 4)
      iAspect_Code = iView_Aspect_Mode + 1;
  else
      iAspect_Code = MPEG_Seq_aspect_ratio_code;


  if (Mpeg_SEQ_Version == 2 // process.Mpeg2_Flag || Mpeg_PES_Version == 2
  || iView_Aspect_Mode != 4)
  {
     switch (iAspect_Code) // MPEG-2 = Frame Aspect Ratio
     {
       case  1: iAspectOut = iAspectIn       ; break;  // Square pixels(VGA) ??
       case  2: iAspectOut = 2000 *  4 / 3 +2; break;  // TRAD TV & FILM (Academy 1.333)
       case  3: iAspectOut = 2000 * 16 / 9 +2; break;  // DTV Wide Screen (1.777)
       case  4: iAspectOut = 2    * 2210     ; break;  // 70mm

       // Unusual settings
       case 10: iAspectOut = 1600 *  4 / 3 +2; break;  // CROPPED - used in some very crappy old captures  1.066666
       case 11: iAspectOut = 2    * 1190     ; break;  // MovieTone Talkie
       case 12: iAspectOut = 2    * 1370     ; break;  // Academy 1932 OR Std-16mm
       case 13: iAspectOut = 2    * 1500     ; break;  // Some Satellite feeds

       case 14: iAspectOut = 2    * 1667     ; break;  // European Cinema WideScreen OR Super-16mm OR Japanese Hi-Vision
       case 15: iAspectOut = 2    * 1850     ; break;  // U.S. Cinema WideScreen

       case 16: iAspectOut = 2    * 2350     ; break;  // CinemaScope - Original 2.35
       case 17: iAspectOut = 2    * 2390     ; break;  // CinemaScope - MODERN 2.39
       case 18: iAspectOut = 2    * 2550     ; break;  // CinemaScope 55 - Camera Neg 2.55

       case 19: iAspectOut = 2    * 2590     ; break;  // Cinerama - 2.59
       case 20: iAspectOut = 2    * 2760     ; break;  // MGM Camera 65 - 2.76
       case 21: iAspectOut = 2    * 4000     ; break;  // PolyVision 4:1
                                               
       default:
       {
        sprintf(szBuffer, "Unknown Aspect Ratio Code=%d",
                           iAspect_Code) ;
        iAspectOut = iAspectIn ;
       }
     }
  }
  else
  {
    if (iCtl_View_Aspect_Mpeg1_Force == 1) // Guess = Infer Aspect
    {
      if (iAspect_Code ==  3 
      ||  iAspect_Code ==  6)
          iAspectOut = 2000 * 16 / 9 +2;  // Probably should be 16:9
      else
      if (iAspect_Code ==  1)
          iAspectOut = iAspectIn;         // Should be VGA
      else
          iAspectOut = 2000 *  4 / 3 +2;  // Probably should be 4:3
    }
    else
    if (iCtl_View_Aspect_Mpeg1_Force
    ||  iAspectIn == (2 * 1222)) // Don't know what standard this is, but it is very common
        iAspectOut = 2000 *  4 / 3 +2;  // Probably should be 4:3
    else
     switch (iAspect_Code) // Mpeg-1 = PEL aspect code
     {
       case  1: iAspectOut = iAspectIn       ; break;  // Square pixels(VGA) ??
       case  2: iAspectOut = iAspectIn * 10000 /  6735 +2; break;  
       case  3: iAspectOut = iAspectIn * 10000 /  7031 +2; break;  // 16:9, 625line	
       case  4: iAspectOut = iAspectIn * 10000 /  7615 +2; break;  //
       case  5: iAspectOut = iAspectIn * 10000 /  8055 +2; break;  //
       case  6: iAspectOut = iAspectIn * 10000 /  8437 +2; break;  // 16:9, 525line	
       case  7: iAspectOut = iAspectIn * 10000 /  8935 +2; break;  //
       case  8: iAspectOut = iAspectIn * 10000 /  9375 +2; break;  // CCIR601, 625line	
       case  9: iAspectOut = iAspectIn * 10000 /  9815 +2; break;  //
       case 10: iAspectOut = iAspectIn * 10000 / 10255 +2; break;  //
       case 11: iAspectOut = iAspectIn * 10000 / 10695 +2; break;  //
       case 12: iAspectOut = iAspectIn * 10000 / 11250 +2; break;  // CCIR601, 525line	
       case 13: iAspectOut = iAspectIn * 10000 / 11575 +2; break;  //
       case 14: iAspectOut = iAspectIn * 10000 / 12015 +2; break;  //
       default:
       {
           sprintf(szBuffer, "Unknown Aspect Ratio Code=%d",
                              iAspect_Code) ;
           iAspectOut = iAspectIn ;
       }
     }	
  }

//  if (Deint_VIEW)
//      iAspectIn = iAspectIn * 2;
//  if (iZoom) // (iView_Aspect_Mode)
//      iAspectIn = iAspectIn / iZoom ;

  // Compensate for running 16:10 monitor on 4:3 adapter card setting
  if (iCtl_AspMismatch)
  {
      if (VGA_Width  <= 0
      ||  VGA_Height <= 0)
      {
          VGA_Width  = 800;
          VGA_Height = 600;
      }

      iAspectOut = iAspectOut * VGA_Width / VGA_Height * 1050 / 1680;
  }

  // Manual adjustment for Monitor Aspect Ratio error
  iAspectOut = iAspectOut * iCtl_View_Aspect_Adjust / 100;

  iVertInc = 0;
  iAspVert = 2048;
  iAspect_Width = Coded_Pic_Height * iAspectOut / 2000;

  if (iAspect_Width < Coded_Pic_Width)
      iAspect_Width_Max = Coded_Pic_Width;
  else
      iAspect_Width_Max = iAspect_Width;


  iCentre_Cropped = 0;
  if (iZoom > 0)
  {
    if (process.iView_Extra_Crop)
      iView_Centre_Crop = 1;
    else
    if (iAspectOut > 2700)
      iView_Centre_Crop = iCtl_View_Centre_Crop;
    else
      iView_Centre_Crop = 0;
    ToggleMenu('=', &iView_Centre_Crop, IDM_VIEW_CTR);
  }



  if (iZoom < 1  && iAspect_Width > Coded_Pic_Width
  &&  MParse.iColorMode != STORE_RGB24)
  {
       iAspect_Width  = iAspect_Width_Max;
       iAspect_Height = Coded_Pic_Height; 
       iAspHoriz      = Coded_Pic_Width  * 2048  / iAspect_Width;

       //if (Deint_VIEW)
       //   iVertInc = 2000;
  }
  else
  {
    if (iZoom < 2)
        iZoom  = 1;

    //iAspect_Width_Unzoom = Coded_Pic_Width;
    iAspect_Height    = Coded_Pic_Height / iZoom ;
    iAspect_Width     = Coded_Pic_Width  / iZoom;
    iAspHoriz         = 2048;

    if (Coded_Pic_Width  &&  iView_Aspect_Mode  &&  iAspectOut)
    {
       if ((iMainWin_State > 0) // Maximized Window ?
       //&&  iZoom == 1 
       &&  MParse.iColorMode == STORE_YUY2 // Stretching width requires Overlay
       &&  (    iAspect_Width  < iViewMax_Width  // Need to Stretch ?
            ||  iAspect_Height < iViewMax_Height // Need to Stretch ?
            || (iView_Centre_Crop && (iAspect_Height < (iViewMax_Height)
                                      || process.iView_Extra_Crop)
               ) 
           )
          )
       {

         // The code for Centre Crop When Maximized is still a but buggy.

         // Roughly calc Client Video area Aspect Ratio,
         // allowing for controls at top of window)
         iCView_Height = iViewMax_Height; // iVGA_Avail_Height;
         //if (iViewToolBar)
         //    iCView_Height -= iTopMargin;
         iCView_Aspect = iViewMax_Width * 2000 / iCView_Height;

         //if (iViewToolBar)
         //  iCView_Aspect = 3454;
         //else
         //  iCView_Aspect = 2711;

         if (iAspectOut >= iCView_Aspect  // Wider aspect than Client Video area ? 
         && (!iView_Centre_Crop 
                || (iAspectOut < 2700 && process.iView_Extra_Crop)
             )
            )
         {
           iAspect_Width  =  iVGA_Avail_Width; // VGA_Width;
           if (process.iView_Extra_Crop > 1)
           {
               iCentre_Cropped = 1;
               iUnCroppedWidth = iAspect_Width; // - 16; // Allow for off-centre 4:3 sub-frame
               iAspect_Width = iAspect_Width * 8 / 7;
           }

           iAspect_Height = (iAspect_Width  * 2000 / iAspectOut);
         }
         else
         {
           iAspect_Height =  iViewMax_Height;
           if (process.iView_Extra_Crop)
           {
               //iCentre_Cropped = 1;
               //iUnCroppedWidth = iAspect_Width; // - 16; // Allow for off-centre 4:3 sub-frame
               iAspect_Height = iAspect_Height * 8 / 7;
           }

           iAspect_Width  = (iAspect_Height * iAspectOut / 2000);
         }

         iAspHoriz      =  Coded_Pic_Width  * 2048  / iAspect_Width;
         iAspVert       =  Coded_Pic_Height * 2048  / iAspect_Height;
         
         // Use internal squeeze when NO DeInterlace
         // and Client Video height smaller than actual frame lines
         if (iSrcUse > iAspect_Height) 
             iVertInc   = (2000 - (iAspect_Height * 2000 / iSrcUse)) ;

         //if (MParse.iColorMode!=STORE_YUY2)
         //    iVertInc       = (2000 - (iAspect_Height * 2000 / iSrcUse)) ;

       } // END-IF  - MAXMIMIZED AND STRETCHING
       else
       {
           iAspect_Height = (iAspect_Width    * 2000  / iAspectOut);
           iAspVert       =  Coded_Pic_Height * 2048  / iAspect_Height;
           iVertInc       = (2000 - (iAspect_Height * 2000 / iSrcUse)) ;

           //if (iView_Centre_Crop)
           //{
           //    iCentre_Cropped = 1;
           //    iUnCroppedWidth = iAspect_Width; // - 16; // Allow for off-centre 4:3 sub-frame
           //    iAspect_Width  = (iAspect_Width * 2 / 3 + 15) & 0xFFFFFFFE ;
           //}
       }
    }
  }


  iAspect_Width2 = iAspect_Width<<1;



  // Optionally align the image Horizontally if too wide to show
  if (! iCentre_Cropped)
  {
    iUnCroppedWidth = iAspect_Width;

    if (process.iView_Extra_Crop)
    {
       iUnCroppedWidth = iUnCroppedWidth * 3 / 4;
    }
  }
}



//--------------------------------------------------



void Mpeg_Aspect_After()
{
  
  iOverload_Width = ( (iUnCroppedWidth - iPhysView_Width - 4)
                      * Coded_Pic_Width / iAspect_Width)
                    & 0xFFFFFFFE ;

  if (iOverload_Width  > 2)
  {
    //iViewWidth2  = (iAspect_Width2 - iOverload_Width - iOverload_Width);
  }
  else
  {
      iOverload_Width = 0;
  }


  //if (iCentre_Cropped)
  //    iAspect_Width = iAspect_Width * 2 / 3;

  if (iCentre_Cropped  &&  process.iView_Extra_Crop)
  {
    iUnCroppedHeight = iAspect_Height * 3 / 4;
  }
  else
    iUnCroppedHeight = iAspect_Height;


  // Optionally align the image vertically if too tall to show
  iOverload_Height = (  (iUnCroppedHeight
                       - iViewMax_Height 
                         )
                       * iAspVert / 2048
                       )
                 &  0xFFFFFFFC ;

    if (iOverload_Height < 0)
        iOverload_Height = 0;


   if (iAspect_Width != iPred_Prev_Width)
   {
       iPred_Prev_Width =  iAspect_Width;

       iView_xFrom      =  (iOverload_Width  / 2) &  0xFFFFFFFC; // default to centre
       if (iView_xFrom > Coded_Pic_Width)
           iView_xFrom = 0;

       iView_yFrom      = ((iOverload_Height / 3) &  0xFFFFFFFC); // default to above middle
       if (iOverload_Height > 48)
           iView_yFrom +=16;
       if (iView_yFrom > Coded_Pic_Height)
           iView_yFrom = 0;

       if  (DBGflag)
       {
           sprintf(szBuffer,  "\nSET xFrom=%d x=%d  yFrom=%d x=%d",
                            iView_xFrom, iOverload_Width,
                            iView_yFrom, iOverload_Height);
           DBGout(szBuffer);
       }
   }

   
  if  (DBGflag)
  {
    sprintf(szBuffer,  "Aspect#%d  %d/%d=%d  Out=%d  Corr=%d  V=%d  H=%d\n  ==%d.%d  Max=%d Load=%d AspW=%d PhysW=%d\n",
                          MPEG_Seq_aspect_ratio_code,
                                  Coded_Pic_Width,  Coded_Pic_Height,
                                        iAspectIn,  iAspectOut,  iVertInc,
                                                      iAspVert,  iAspHoriz,
                          iAspect_Width,  iAspect_Height, iMainWin_State, 
                            iOverload_Width, iAspect_Width, iPhysView_Width);
    DBGout(szBuffer);
  }


  return;

}


void Mpeg_Aspect_Calc()
{
  Mpeg_Aspect_Before();
  Mpeg_Aspect_After();
}


void Mpeg_Aspect_Resize()
{
  if (DBGflag)
      DBGout("ASP RESIZE");

  Mpeg_Aspect_Before();

  if ((   iAspect_Width  != Prev_Coded_Width 
       || iAspect_Height != Prev_Coded_Height)
  && ! MParse.iMultiRqst
  && iShowVideo_Flag)
  {
      D500_ResizeMainWindow(iAspect_Width, iAspect_Height, 1);
      Prev_Coded_Width  = iAspect_Width;
      Prev_Coded_Height = iAspect_Height;
  }
  else
  if (DBGflag)
  {
     sprintf(szBuffer,
       "    SKIPPED Multi=%d Show=%d\nNew=%d.%d\nold=%d.%d  ",
            MParse.iMultiRqst, iShowVideo_Flag,
            iAspect_Width, iAspect_Height,
            Prev_Coded_Width, Prev_Coded_Height);
     DBGout(szBuffer);
  }


  Mpeg_Aspect_After();
}





