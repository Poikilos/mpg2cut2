

// Snapshot of current frame 

#include "global.h"
#define true 1
#define false 0
#include "Mpg2Cut2_API.h"
#include <vfw.h>
#include "PIC_BUF.h"
#include "Buttons.h"

char SnapSuffix ;
unsigned char *lpBMPpic;
int iRawSize;
int iRGB_Width, iRGB_Height;

int iBMP_Pic_Width, iBMP_Pic_Height, iBMP_Aspect_Style, iBMP_Stride;
int iBMP_Crop43_Flag, iBMP_Crop_Margin;

int iBMP_Margin_Ht,      iBMP_Margin_Wd;
int iBMP_Margin_Ht_Area, iBMP_Margin_Wd_Area;
BYTE cMarginColour;

void Snap_ToFile(), Snap_ToClipBoard();;


// BMP File Header STRUCTURE
const unsigned char BMPHdr_C1[2] = {'B', 'M'} ;
int BMPHdr_Size ;
const unsigned char BMPHdr_C2[12] =
  {
                                      0x00, 0x00,
  0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00
  } ;
int BMPHdr_Width ;
int BMPHdr_Height ;
const unsigned char BMPHdr_C3[28] =
  {
              0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x0b,
  0x00, 0x00, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  } ;




//----------------------------------------------

void SNAP_Save(UINT P_Snap_Type, ThumbRec* lpThumbRec)
{

int iRC, iTmp1, iTmp2; //, iTmpLen;
//int iRGBLumFlag;
int iBMPLumFlag, iThumbFlag, iDeintFlag;



  if (DBGflag)
  {
      sprintf(szDBGln, "\n\n\n*SNAP*\n");
      DBGout(szDBGln);
  }

  

  if (!lpThumbRec)
     cMarginColour = 0x80;
  else
     cMarginColour=lpThumbRec->MarginColour;

  iBMP_Aspect_Style = iCtl_BMP_Aspect;
  iBMPLumFlag = iLumEnable_Flag[1];

  //   Default scan mode to match main display
  iDeintFlag = Deint_VIEW & Deint_SNAP;
  iThumbFlag = 0;
  iBMP_Crop43_Flag = iView_Centre_Crop; 

  if (P_Snap_Type == IDM_BMP_THUMB
  ||  P_Snap_Type == RJPM_THUMB2CLIPBOARD
  ||  P_Snap_Type == RJPM_THUMB2MEM)
  {
      iThumbFlag    = 1;
      //if (iBMP_Aspect_Style != BMP_ASPECT_BICUBIC)  
            iBMP_Aspect_Style = BMP_ASPECT_SUBSAMPLE; // Always use fast sub-Sample for Thumbs

      if ((!lpThumbRec) || (lpThumbRec->iLum==RJPC_AUTO)) // v20503
         iBMPLumFlag = 1;
      else
         iBMPLumFlag = lpThumbRec->iLum;
                
  }
  else
  {

    // Acknowledge the BMP request by flashing the BMP button

    if (iViewToolBar & 256)
    {
      EnableWindow(hBmpButton, MF_GRAYED);
      MoveWindow(hBmpButton,   400, 300,
                 iTool_Wd, iTool_Ht, true);
    }
    else
    {
      BmpButton_Create();
      UpdateWindow(hWnd_MAIN);
    }



  }

  /*
  if (ClipResize_Flag)
  {
      BMPHdr_Width  -= Clip_Left+Clip_Right;
      BMPHdr_Height -= Clip_Top+Clip_Bottom;
  }
  */


  iRGB_Width  = Coded_Pic_Width;
  iRGB_Height = Coded_Pic_Height;

  // 2 different ways to snap the buffer :-
  //   Full Frame snaps both fields 
  //         Normally OK with PAL Telecine material,
  //         Yucky on other interlaced material
  //   Interlace - Separates the two fields
  //         Avoids interlacing artefacts, but lower resolution

  //   SHIFT key reverses it
  if (P_Snap_Type == IDM_BMP_SHIFT)
      iDeintFlag = 1 - Deint_VIEW;
  else
  if (P_Snap_Type == RJPM_BMP_DEINT)
      iDeintFlag = 1;

  if (iThumbFlag)
  {
    SnapSuffix = 'T' ;
    iDeintFlag = 0;
  }
  else
  if (iDeintFlag)
  {

    SnapSuffix = 'J' ;
    if (iBMP_Aspect_Style != BMP_ASPECT_BICUBIC) // BiCubic
        SnapSuffix = 'i' ;

    iRGB_Width    = Coded_Pic_Width  * 2;
    iRGB_Height   = Coded_Pic_Height / 2;
  }
  else
  {
    SnapSuffix = 'F' ;
    if (iAspectOut  == iAspectIn)
        iBMP_Aspect_Style  = BMP_ASPECT_RAW; // Take ASIS - No-Sample - is already good.

    if (iBMP_Aspect_Style != BMP_ASPECT_BICUBIC) // BiCubic
        SnapSuffix = 'Q' ;
  }



  iBMP_Pic_Height = iRGB_Height;
  iBMP_Pic_Width  = iRGB_Width;


  if ( ! iRGB_Width ||  ! iRGB_Height)
  {
    strcpy(szMsgTxt,  "Pic Dimension=0");
    DSP1_Main_MSG(0,0);
    return;
  }


  if (iBMP_Aspect_Style != BMP_ASPECT_RAW) 
  {
    // Decide whether stretching Horizontally or Vertically
    if ((iRGB_Width * 2000 / iRGB_Height) < iAspectOut)
        iBMP_Pic_Width = (iBMP_Pic_Height * iAspectOut + 1001) / 2000 ;
    else
    {
        iBMP_Pic_Height  = (iBMP_Pic_Width  * 2000 + (iAspectOut/2) + 1) 
                                                    / iAspectOut ;
        if (iDeintFlag)
        {
          iBMP_Pic_Height  = (iBMP_Pic_Height + 1) / 2;
          iBMP_Pic_Width   = (iBMP_Pic_Width  + 1) / 2;
        }
    }
  }


  iBMP_Margin_Ht      = 0;    iBMP_Margin_Ht_Area = 0;
  iBMP_Margin_Wd      = 0;    iBMP_Margin_Wd_Area = 0;

  if (iThumbFlag)
  {

     if (lpThumbRec) // Pick up size from passed parm if available
     {
         BMPHdr_Width=lpThumbRec->nImageWidth;
         BMPHdr_Height=lpThumbRec->nImageHeight;
     }      
     else
     {
        BMPHdr_Height =  90; 
        BMPHdr_Width  = 160; 
     }


     // Because thumbnail area is pre-specified,
     // the aspect ratio may not match the mpeg picture.
     // So we need to calculate what margins to add
     // either horizontally or vertically to pad it out

     // Compare aspect ratios 
     // To avoid floating point, use cross-multiply rather than divide
     iTmp1 = BMPHdr_Height * iBMP_Pic_Width;
     iTmp2 = BMPHdr_Width  * iBMP_Pic_Height;
     if (iTmp1 < iTmp2 ) // Narrower than the allotted aspect ratio
     {
        iBMP_Pic_Width  = BMPHdr_Height * iBMP_Pic_Width / iBMP_Pic_Height;
        iBMP_Pic_Height = BMPHdr_Height;
     }
     else
     if (iTmp1 > iTmp2 ) // Wider than the allotted aspect ratio
     {
        iBMP_Pic_Height = BMPHdr_Width * iBMP_Pic_Height / iBMP_Pic_Width;
        iBMP_Pic_Width  = BMPHdr_Width;
     }
     else                // Same aspect ratio
     {
        iBMP_Pic_Height = BMPHdr_Height;
        iBMP_Pic_Width  = BMPHdr_Width;
     }

     iBMP_Stride = ((BMPHdr_Width*3) + 3) & 0xFFFFFFFC; // Number of BMP bytes allocated per line

     iBMP_Margin_Ht      = (BMPHdr_Height - iBMP_Pic_Height) / 2;
     iBMP_Margin_Ht_Area = iBMP_Margin_Ht * iBMP_Stride;
     iBMP_Margin_Wd      = (BMPHdr_Width  - iBMP_Pic_Width ) / 2;
     iBMP_Margin_Wd_Area = iBMP_Margin_Wd * 3;
  }
  else
  {
      BMPHdr_Height = iBMP_Pic_Height;
      BMPHdr_Width  = iBMP_Pic_Width;
      iBMP_Stride = ((BMPHdr_Width*3) + 3) & 0xFFFFFFFC; // Number of BMP bytes allocted per line
  }

  iBMP_Crop_Margin = (BMPHdr_Width / 6 - 4) & 0xFFFFFFFC;



  iRawSize    = iBMP_Stride * BMPHdr_Height ;
  BMPHdr_Size = iRawSize + 54 ;

  if (DBGflag)
  {
      sprintf(szDBGln, "HDR  V=%d W=%d S=%d\nPic  V=%d W=%d\nMarg V=%d, W=%d",
                        BMPHdr_Height, BMPHdr_Width, iBMP_Stride,
                        iBMP_Pic_Height, iBMP_Pic_Width,
                        iBMP_Margin_Ht, iBMP_Margin_Wd);
      DBGout(szDBGln);
  }


  // Need the RGB24 buffer to be populated as basis for BMP
  if (!rgb24)
  {
    strcpy(szMsgTxt,  "Frame PTR Lost");
    DSP1_Main_MSG(0,0);
  }


  //iTmp4 = MParse.iColorMode;
  if(MParse.iColorMode != STORE_RGB24) 
  {
    //iRGBLumFlag    = Luminance_Flag;
    //Luminance_Flag = iBMPLumFlag;
    Store_RGB24(curr_frame, 1); // bwd_ref_frame); // , Frame_Number-1);  
    //Luminance_Flag = iRGBLumFlag;
  }


  // picture may be direct copy from the display buffer
  // OR a rebuild using a separate buffer
  lpBMPpic = rgb24;

  if (P_Snap_Type == RJPM_THUMB2MEM)
      lpBMPpic = (BYTE*)lpThumbRec->lpRGB24;
  else
  if (iBMP_Aspect_Style != BMP_ASPECT_RAW)  
  {
     if (iBMP_BufSize < BMPHdr_Size)  // Only reallocate if really need to
     {
       iRC = SNAP_Buffer_Alloc(BMPHdr_Size * 5);  // Allow for bug - potential overrun of buffer
       if (!iRC)
            return;
     } 
     lpBMPpic     = lpBMP_Buffer;
  }


  if (lpBMPpic != rgb24) // Output area is separate from input area
  {
     SNAP_Resample_RGB(iBMP_Aspect_Style, 
                       lpBMPpic);
                       // iBMP_Pic_Width, iBMP_Pic_Height );
  }



  if (P_Snap_Type != RJPM_THUMB2MEM)
  {
     if ((P_Snap_Type == IDM_BMP_CLIPBOARD) 
     ||  (P_Snap_Type == RJPM_THUMB2CLIPBOARD))
          Snap_ToClipBoard();
     else
          Snap_ToFile();
  }


  // Restore the BMP button, now that we are finished

  if (P_Snap_Type != IDM_BMP_THUMB)
  {
    // ShowWindow(hWnd, wCmdShow);
    Sleep(100);
    if (iViewToolBar & 256)
    {
        MoveWindow(hBmpButton,   0, 0,
                   iTool_Wd, iTool_Ht, true);
        EnableWindow(hBmpButton, true);
    }
    else
        DestroyWindow(hBmpButton);

  }

  //MainPaint() ;
}




//--------------------------------------


void Snap_ToFile()
{
        
  FILE *BMPFile;
  char *ext;
  char *lsSlash, *lsTemp, *lsName;
  int iRC, i;
  char szBMPname[_MAX_PATH*2];

  //SYSTEMTIME st;
  //GetLocalTime(&st);

  process.iUnique++;

  lsName = File_Name[File_Ctr];
  lsSlash = lpLastSlash(lsName); // &szInput[0]);
  if (!lsSlash)
       lsSlash = lsName;         // &szInput[0]);


  // GENERIC VOB NAME (VTS_)

  // - On removable volume - get Disk Volume Label
  // - On non-removable - ASK (or ALWAYS ASK)

  /*
  //if (*(DWORD*)(lsName) == *(DWORD*)(&'VTS_')
  if (!strnicmp((lsName[1]),"\VIDEO_TS\VTS_", 14)
  {
     //if (lsName[1] ==':')
     {
        strcpy(szTmp3, "D:\\");
        szTmp3[0] = *lsName[0];
        iRC = GetVolumeInformation(&szTmp3[0], 
                                   File_name[MAX_FILE_NUMBER], _MAX_PATH, 
                                   NULL, 0, NULL, &iTmp1, &iTmp2,
                                   &szTmp32, 32);
        if (iRC)
        {
        }

  }
  */

  if (iBMP_Folder_Active && szCtl_BMP_Folder[0])
  {
      strcpy (szTemp, szCtl_BMP_Folder);
      strcat (szTemp, lsSlash);
      lsTemp = &szTemp[0];
  }
  else
      lsTemp = lsName; // &szInput[0];

  if (iCtl_Out_DeBlank || iCtl_Out_MixedCase)
      FileNameTidy(&szBMPname[0], lsTemp);
  else
      strcpy(szBMPname, lsTemp);
  

  ext = strrchr(szBMPname, '.');
  if (ext)
    *ext = 0x00 ;

  if (CurrTC.RunFrameNum)
  {
     if (iView_TC_Format == 4 || iView_TC_Format == 7)
     {
         RelativeTC_SET();
         Relative_TOD();
     }
     else
         memcpy(&ShowTC, &CurrTC,     sizeof(ShowTC));
      
     sprintf(szTemp, "_%02d%02d-%02d%02d%c",
        // st.wDay, st.wHour, st.wMinute, st.wSecond,
                ShowTC.hour, ShowTC.minute, ShowTC.sec, ShowTC.frameNum,
                SnapSuffix);
     strcat(szBMPname, szTemp);
   }

  sprintf(szTemp, "_%03d.bmp", process.iUnique);
  strcat(szBMPname, szTemp) ;
  BMPFile = fopen(szBMPname, "wb");

  // If cannot write to same path, try elsewhere
  if (!BMPFile)
  {
      strcpy (szTemp, szBMPname);
      lsSlash = lpLastSlash(&szTemp[0]);
      if (!lsSlash)
           lsSlash = &szTemp[0];
      strcpy (szBMPname, szCtl_BMP_Folder);
      strcat (szBMPname, lsSlash);
      iRC = X800_PopFileDlg(szBMPname, hWnd_MAIN, SAVE_BMP, -1, &"BMP Folder");
      if (! iRC)
          MessageBeep(MB_OK);
      else
          BMPFile = fopen(szBMPname, "wb");
          if (BMPFile)
          {
              iBMP_Folder_Active = 1;
              strcpy(szCtl_BMP_Folder, szBMPname);
              lsSlash = lpLastSlash(szCtl_BMP_Folder);
              if (! lsSlash)
                    lsSlash = &szCtl_BMP_Folder[0];
             *lsSlash = 0x00;
          }
  }

  if (!BMPFile)
  {
      strcpy (szTemp, szBMPname);
      lsSlash = lpLastSlash(&szTemp[0]);
      if (lsSlash)
      {
          strcpy (szBMPname, "C:\\TEMP");
          strcat (szBMPname, lsSlash);
          BMPFile = fopen(szBMPname, "wb");
      }
  }

  if (!BMPFile)
  {
      strcpy (szBMPname, "C:\\TEMP\\Mpg2Cut");
      strcat (szBMPname,   szTemp) ;
      BMPFile = fopen(szBMPname, "wb");
  }

  if (!BMPFile)
  {
      MessageBox(hWnd_MAIN, "Cannot Write BMP to ",
                             szBMPname, MB_ICONSTOP | MB_OK);
  }
  else
  {

    // BMPHdr - This should really use a single structure
    //          but I cannot figure out the syntax


    i =  fwrite(&BMPHdr_C1,     1, sizeof(BMPHdr_C1),   BMPFile);
    i =  fwrite(&BMPHdr_Size,   1, sizeof(int),         BMPFile);
    i =  fwrite(&BMPHdr_C2,     1, sizeof(BMPHdr_C2),   BMPFile);
    i =  fwrite(&BMPHdr_Width,  1, sizeof(int),         BMPFile);
    i =  fwrite(&BMPHdr_Height, 1, sizeof(int),         BMPFile);
    i =  fwrite(&BMPHdr_C3,     1, sizeof(BMPHdr_C3),   BMPFile);

    i += fwrite( lpBMPpic, 1, iRawSize, BMPFile);

    /*
    j = i & 3;

    while (j>0)
    {
      i += fputc(0, BMPFile);
      j--;
    }
    fseek(BMPFile, 2, SEEK_SET);
    fwrite(&i, sizeof(int), 1, BMPFile);
    fseek(BMPFile, 18, SEEK_SET);
    fwrite(&width,  sizeof(int), 1, BMPFile);
    fwrite(&height, sizeof(int), 1, BMPFile);
    */

  }

  fclose(BMPFile);
}





//---------- Code adapted from http://eatworms.swmed.edu/~boris/B_Player/VideoWnd.cpp -------

void Snap_ToClipBoard()
{
  HGLOBAL hGlobal;
  BYTE *lpGlobal;
  BYTE *lpOUT;


  HANDLE hRC;
  int iRC;

  BITMAPINFOHEADER ClpHdr;

  DWORD dwClpSize;
  
  ClpHdr.biSize             = sizeof(ClpHdr);
  ClpHdr.biWidth            = BMPHdr_Width;
  ClpHdr.biHeight           = BMPHdr_Height; 
  ClpHdr.biPlanes           = 1; 
  ClpHdr.biBitCount         = 24;
  ClpHdr.biCompression      = BI_RGB; 
  ClpHdr.biSizeImage        = iRawSize * 4 / 3; 
  ClpHdr.biXPelsPerMeter    = 2315; 
  ClpHdr.biYPelsPerMeter    = 2315; 
  ClpHdr.biClrUsed          = 0; //BMPHdr_Width * BMPHdr_Height; 
  ClpHdr.biClrImportant     = 0; 

  dwClpSize = ClpHdr.biSize + ClpHdr.biSizeImage 
            + (int)ClpHdr.biClrUsed * sizeof(RGBQUAD);
  
  hGlobal = GlobalAlloc (GHND | GMEM_SHARE, (dwClpSize));
  if (! hGlobal)
    MessageBox(hWnd_MAIN, "GAlloc Failed", szAppName, MB_ICONSTOP | MB_OK);
  else
  {
    lpGlobal =  (BYTE *) GlobalLock (hGlobal);
    if (!lpGlobal)
        MessageBox(hWnd_MAIN, "GLock Failed", szAppName, MB_ICONSTOP | MB_OK);
    else
    {
       
       CopyMemory( lpGlobal, &ClpHdr, sizeof(ClpHdr)) ;
       lpOUT = lpGlobal + sizeof(ClpHdr);
       CopyMemory( lpOUT, lpBMPpic, iRawSize) ;

       GlobalUnlock( hGlobal) ;

       iRC = OpenClipboard (hWnd_MAIN) ;
       if (!iRC)
       {
           MessageBox(hWnd_MAIN, "Opn Clipboard Failed", szAppName, MB_ICONSTOP | MB_OK);
       }
       else
       {
          EmptyClipboard ();
          hRC = SetClipboardData (CF_DIB, hGlobal) ;
          if (hRC == NULL)
              MessageBox(hWnd_MAIN, "Set Clipboard Failed", szAppName, MB_ICONSTOP | MB_OK);
          CloseClipboard ();
       }
    }
  }

  //CoTaskMemFree(pFrame);
  //pFrame = 0;

  return;
}






//---------------------------------------

int SNAP_Buffer_Alloc(int P_Size)
{

  if (DBGflag)
  {
      sprintf(szDBGln, "BMP ALLOC=%d,  PreAlloc=%d", P_Size, iBMP_BufSize);
      DBGout(szDBGln);
  }


  if (iBMP_BufSize < P_Size)  // Only reallocate if really need to
  {
        if (iBMP_BufSize)
          free(lpBMP_Buffer);

        lpBMP_Buffer =  (unsigned  char*)malloc(P_Size);

        if (lpBMP_Buffer == NULL)
        {
            Err_Malloc(&"BMPBuf");
            iBMP_BufSize = 0;
        }
        else
        {
          iBMP_BufSize = P_Size;
        }
  }

  return iBMP_BufSize;

}



// 
// * POS BiCubic resampler.   
//
//  original code from LIBIMAGE on Koders.com, 
//
//  Here adapted to RGB24, optimized structure a bit
//  and converted from floating point to integer

//  Since "C" does not support Cobol style scaled ntegers
//  I have done the scaling manually using a base of 1024
//  to approximate  "PIC S9(6)V999 COMP".

__forceinline  static 
int iBiCubicConv( int x, int y, 
         unsigned char *lpRGBpel,
         unsigned char *lpBMPpel,
         int iWidth, int iHeight) //, int linestride )
{
  unsigned char *lpRGBcurr;
  //int x, y;
  int iTmp1, iRC;

  int output[3];
  int dx, dx2, hx;
  int dy, dy2, hy;
  int hxhy, curpixel;


  int i, j, iBase_X, iBase_Y;
  int iRef_X, iRef_Y ;

  // x = P_x * 1024.0;
  // y = P_y * 1024.0;

  iBase_X =  x / 1024;  //floor( x ); // Base Source column
  iBase_Y =  y / 1024;  //floor( y ); // Base Source row

  output[0] = 0;
  output[1] = 0;
  output[2] = 0;

  iRC = 0;

  for( i = 0; i < 4; i++ ) 
  {
     iRef_Y = iBase_Y + i - 1;  // Nearby Reference Row

     for( j = 0; j < 4; j++ ) 
     {
        iRef_X = iBase_X + j - 1;  // Nearby Reference Column


        if( iRef_X  <  0     || iRef_Y  <  0 
        ||  iRef_X >= iWidth || iRef_Y >= iHeight ) 
        {
            continue;  // Escape  (Opposite of Cobol CONTINUE)
        }


        // Calculate delta x and delta y. 

        //dx = fabs( x - (double) iRef_X );
        dx =  x - (iRef_X * 1024) ; 
        if (dx < 0) dx = -dx;

        dx2 = dx * dx / 1024;

        //dy = fabs( y - (double) iRef_Y );
        dy =  y -  (iRef_Y * 1024) ; 
        if (dy < 0) dy = -dy;

        dy2 = dy * dy / 1024;


        // Calculate the x contribution. 
        if( dx < 1024 ) 
        {
            hx = 1024 - ( ( 2048 - dx) * dx2 / 1024 ) ;
        } 
        else 
        {
            hx = 4096 - ( 8 * dx ) + ( ( 5120 - dx) * dx2 / 1024);
        }

        /* Calculate the y contribution. */
        if( dy < 1024 ) 
        {
            hy = 1024 - ( ( 2048 - dy) * dy2 / 1024 );
        } 
        else 
        {
            hy = 4096 - ( 8 * dy ) + ( ( 5120 - dy) * dy2 / 1024);
        }

        hxhy = hx * hy / 1024; // Intermediate uses about 28 bits, but is brought back to 18 by dividing by 1024

        // Add in the weighted contribution from this pixel. 
        lpRGBcurr  = lpRGBpel + ( ( (iRef_Y * iWidth) + iRef_X) * 3) ;

        // Do Blue, Green, Red channels
        curpixel   =  (*lpRGBcurr++);
        output[0] += curpixel * ( hxhy );
        curpixel   =  (*lpRGBcurr++);
        output[1] += curpixel * ( hxhy );
        curpixel   =  (*lpRGBcurr++);
        output[2] += curpixel * ( hxhy );
     }
  }

  // BLUE
  iTmp1 =  output[0] / 1024;      // * 255.0);
  if (iTmp1 < 0)   
      iTmp1 = 0; 
  else
  if (iTmp1 > 32) // Above Blackish ?
  {
      iRC = 1;
      if (iTmp1 > 255) 
          iTmp1 = 255; 
  }
  *lpBMPpel++ =  (unsigned char)iTmp1;

  // RED
  iTmp1 =  output[1] / 1024;     
  if (iTmp1 < 0)
      iTmp1 = 0; 
  else
  if (iTmp1 > 32) // Above Blackish ?
  {
      iRC = 1;
      if (iTmp1 > 255) 
          iTmp1 = 255; 
  }
  *lpBMPpel++ =  (unsigned char)iTmp1;

  // GREEN
  iTmp1 =  output[2] / 1024; 
  if (iTmp1 < 0)
      iTmp1 = 0; 
  else
  if (iTmp1 > 32) // Above Blackish ?
  {
      iRC = 1;
      if (iTmp1 > 255) 
          iTmp1 = 255; 
  }
  *lpBMPpel   =  (unsigned char)iTmp1; 


  return iRC;   // Indicate whether or not the pixel was blackish
}



 /* OLD FLOATING POINT VERSION

// 
//  Based on POS BiCubic resampler.   
//
//  original code from LIBIMAGE on Koders.com, 
//  Here adapted to RGB24 and optimized structure a bit

__forceinline  static 
int iBiCubicConv( double x, double y, 
         unsigned char *lpRGBpel,
         unsigned char *lpBMPpel,
         int iWidth, int iHeight) //, int linestride )
{
  unsigned char *lpRGBcurr;
  int iTmp1, iRC;

  double output[3];
  double dx, dx2, hx;
  double dy, dy2, hy;
  double hxhy, curpixel;


  int i, j, iBase_X, iBase_Y;
  int iRef_X, iRef_Y ;

  iBase_X = (int) x;  //floor( x ); // Base Source column
  iBase_Y = (int) y;  //floor( y ); // Base Source row

  output[0] = 0.0;
  output[1] = 0.0;
  output[2] = 0.0;

  for( i = 0; i < 4; i++ ) 
  {
     iRef_Y = iBase_Y + i - 1;  // Nearby Reference Row

     for( j = 0; j < 4; j++ ) 
     {
        iRef_X = iBase_X + j - 1;  // Nearby Reference Column


        if( iRef_X  <  0     || iRef_Y  <  0 
        ||  iRef_X >= iWidth || iRef_Y >= iHeight ) 
        {
            continue;  // Escape  (Opposite of Cobol CONTINUE)
        }


        // Calculate delta x and delta y. 

        //dx = fabs( x - (double) iRef_X );
        dx =  x - (double) iRef_X ; 
        if (dx < 0) dx = -dx;

        dx2 = dx * dx;

        //dy = fabs( y - (double) iRef_Y );
        dy =  y - (double) iRef_Y ; 
        if (dy < 0) dy = -dy;

        dy2 = dy * dy;


        // Calculate the x contribution. 
        if( dx < 1.0 ) 
        {
            hx = 1.0 - ( ( 2.0 - dx) * dx2  ) ;
        } 
        else 
        {
            hx = 4.0 - ( 8.0 * dx ) + ( ( 5.0 - dx) * dx2 );
        }

        // Calculate the y contribution. 
        if( dy < 1.0 ) 
        {
            hy = 1.0 - ( ( 2.0 - dy) * dy2  );
        } 
        else 
        {
            hy = 4.0 - ( 8.0 * dy ) + ( ( 5.0 - dy) * dy2 );
        }

        hxhy = hx * hy; 

        // Add in the contribution from this pixel. 
        lpRGBcurr  = lpRGBpel + (((iRef_Y * iWidth ) + iRef_X)*3) ;

        // Do Blue, Green, Red channels
        curpixel   = (double) (*lpRGBcurr++);
        output[0] += curpixel * ( hxhy );
        curpixel   = (double) (*lpRGBcurr++);
        output[1] += curpixel * ( hxhy );
        curpixel   = (double) (*lpRGBcurr++);
        output[2] += curpixel * ( hxhy );
     }
  }

  // BLUE
  iTmp1 =  (int)(output[0]);      // * 255.0);
  if (iTmp1 < 0)
      iTmp1 = 0; 
  else
  if (iTmp1 > 32) // Above Blackish ?
  {
      iRC = 1;
      if (iTmp1 > 255) 
          iTmp1 = 255; 
  }
  *lpBMPpel++ =  (unsigned char)iTmp1;

  // RED
  iTmp1 =  (int)(output[1]);      //  * 255.0);
  if (iTmp1 < 0)
      iTmp1 = 0; 
  else
  if (iTmp1 > 32) // Above Blackish ?
  {
      iRC = 1;
      if (iTmp1 > 255) 
          iTmp1 = 255; 
  }
  *lpBMPpel++ =  (unsigned char)iTmp1;

  // RED
  iTmp1 =  (int)(output[2]);      //  * 255.0);
  if (iTmp1 < 0)
      iTmp1 = 0; 
  else
  if (iTmp1 > 32) // Above Blackish ?
  {
      iRC = 1;
      if (iTmp1 > 255) 
          iTmp1 = 255; 
  }
  *lpBMPpel   =  (unsigned char)iTmp1; 

  return iRC;
}

*/




//----------------------

void SNAP_Resample_RGB(int P_Bmp_Aspect,  // SubSample=1  Bicubic=4
                  unsigned char *lpBMP_Frame)
                  //int iBMP_Pic_Width, int iBMP_Pic_Height )
{

  int iBMP_Vert, iBMP_Horiz; //, iBMP_Stride;
  int iRGB_Vert, iRGB_Horiz; //iRGBstride;
  int iBiCubicWanted, iBreatheCtr, iRC;

  //double fConv_Width, fConv_Height, fRGB_pct_H, fRGB_pct_V;
  int      iConv_Width, iConv_Height, fRGB_pct_H, fRGB_pct_V;

  unsigned char *lpBMP_Curr, *lpRGB_Curr, *lpRGB_Line;

  if (P_Bmp_Aspect == BMP_ASPECT_BICUBIC)
    iBiCubicWanted = 1;
  else
    iBiCubicWanted = 0;

  //iBMP_Stride = ((iBMP_Pic_Width*3) + 3) & 0xFFFFFFFC; // Number of BMP bytes allocted per line

//fConv_Height = (float)iBMP_Pic_Height / (float)iRGB_Height; 
//fConv_Width  = (float)iBMP_Pic_Width  / (float)Coded_Pic_Width; 
  iConv_Height =        iBMP_Pic_Height * 1024 / iRGB_Height; 
  iConv_Width  =        iBMP_Pic_Width  * 1024 / Coded_Pic_Width; 

  // Grey out the Top Margin

  if (iBMP_Margin_Ht_Area > 0)
  {
    memset(lpBMP_Frame, cMarginColour, iBMP_Margin_Ht_Area);
    lpBMP_Frame += iBMP_Margin_Ht_Area;
  }

  if (DBGflag)
  {
      sprintf(szDBGln, "H=%d W=%d S=%d  Need=%d", 
                         iBMP_Pic_Height, iBMP_Pic_Width, iBMP_Stride,
                         (iBMP_Pic_Height * iBMP_Stride * 3));
      DBGout(szDBGln);
  }

  // Build the Output Pixels

  iBreatheCtr = 0;
  
  //SetPriorityClass(hMain_GUI, IDLE_PRIORITY_CLASS);

  for( iBMP_Vert = 0; iBMP_Vert < iBMP_Pic_Height; iBMP_Vert++ ) 
  {

    lpBMP_Curr = lpBMP_Frame + (iBMP_Vert * iBMP_Stride);

    iRGB_Vert  = iBMP_Vert   *  iRGB_Height / iBMP_Pic_Height;
    lpRGB_Line = rgb24       + (iRGB_Vert   * iRGB_Width * 3);
    
    // Grey out the Left Margin

    if (iBMP_Margin_Wd_Area > 0)
    {
        memset(lpBMP_Curr, cMarginColour, iBMP_Margin_Wd_Area);
        lpBMP_Curr += iBMP_Margin_Wd_Area;
    }

    if (iBiCubicWanted)  
    {  
      //fRGB_pct_V = float(iBMP_Vert)          / fConv_Height;
        fRGB_pct_V =       iBMP_Vert * 1048576 / iConv_Height;
    }

    if (DBGflag && iRGB_Vert > 570)
    {
       sprintf(szDBGln, "Vert=%d  Line=%d  W=%d=%d", 
                         iRGB_Vert, lpRGB_Line, Coded_Pic_Width, iBMP_Pic_Width);
       DBGout(szDBGln);
    }

    for( iBMP_Horiz = 0; iBMP_Horiz < iBMP_Pic_Width; iBMP_Horiz++ ) 
    {

      iRGB_Horiz = iBMP_Horiz * Coded_Pic_Width / iBMP_Pic_Width;
      lpRGB_Curr = lpRGB_Line + (iRGB_Horiz * 3);


      if (iBMP_Horiz == 10 && DBGflag)
      {
         sprintf(szDBGln, " Vert=%d     Curr=%d", iBMP_Vert, lpRGB_Curr);
         DBGout(szDBGln);
      }

      if (! iBiCubicWanted) 
      {
         // Crude sub-sample - good enough for Thumbnails

         // Check for 4:3 inside widescreen
         if (iBMP_Horiz < iBMP_Crop_Margin)
         {
           if (*lpRGB_Curr > 32)
                iBMP_Crop43_Flag = 0;
           *lpBMP_Curr++ = *lpRGB_Curr++; // Blue

           if (*lpRGB_Curr > 32)
                iBMP_Crop43_Flag = 0;
           *lpBMP_Curr++ = *lpRGB_Curr++; // Green

           if (*lpRGB_Curr > 32)
                iBMP_Crop43_Flag = 0;
           *lpBMP_Curr++ = *lpRGB_Curr;   // Red
         }
         else
         {
            *lpBMP_Curr++ = *lpRGB_Curr++; // Blue
            *lpBMP_Curr++ = *lpRGB_Curr++; // Green
            *lpBMP_Curr++ = *lpRGB_Curr;   // Red
         }
      }
      else
      {
        // BiCubic  
      //fRGB_pct_H = (float)(iBMP_Horiz)          / fConv_Width;
        fRGB_pct_H =         iBMP_Horiz * 1048576 / iConv_Width;

        iRC = iBiCubicConv( 
                  fRGB_pct_H, fRGB_pct_V,
                //fRGB_pct_H, fRGB_pct_V,
                  rgb24, lpBMP_Curr,
                  iRGB_Width, iRGB_Height);

        if (iBMP_Horiz < iBMP_Crop_Margin
        &&  iRC)
        {
          iBMP_Crop43_Flag = 0;
        }

        lpBMP_Curr += 3;
        
      }

    } // END FOR WIDTH

    // Grey out the Right Margin

    if (iBMP_Margin_Wd_Area > 0)
    {
        memset(lpBMP_Curr, cMarginColour, iBMP_Margin_Wd_Area);
        lpBMP_Curr += iBMP_Margin_Wd_Area;  // Not the most effecient place for this
    }
    
    if (iBreatheCtr >= 10)
    {
        iBreatheCtr = 0;
        Sleep(1);
    }
    else
        iBreatheCtr++;
    

  } // END FOR HEIGHT

  // Grey out the Bottom Margin

  if (iBMP_Margin_Ht_Area > 0)
  {
    memset(lpRGB_Curr, cMarginColour, iBMP_Margin_Ht_Area);
    //lpRGB_Curr += iBMP_Margin_Ht_Area); // Not needed in this context
  }

  //SetPriorityClass(hMain_GUI, iCtl_Priority[1]);

}



//---------------------

/*

// Reasmples a single colour channel array

void resample_chroma_420_to_444( double *c420, double *c444,
                                 int width, int height )
{
    int i, j;

    for( i = 0; i < height; i++ ) 
    {
        for( j = 0; j < width; j++ ) 
        {
            c444[ 0 ] = *c420;
            c444[ 1 ] = *c420;
            c444[ (width*2)   ] = *c420;
            c444[ (width*2)+1 ] = *c420;
            c444 += 2;
            c420++;
        }
        c444 += width*2;
    }
}

*/





