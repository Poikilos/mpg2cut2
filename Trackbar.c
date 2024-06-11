//
// 
#define DBG_RJ 1

#include "global.h"
#include <commctrl.h>

#define true 1
#define false 0

     int    iStartFile;
  __int64 i64StartLoc;

#include "GetBlk.h"
  //------------------------------------------------
// Update the controls for the current input position
void T100_Upd_Posn_Info(int P_Upd_CurrLoc) 
{
  int iRC;

  if (RdAHD_Flag && iRdAHD_CurrIx >= 0)
  {
      iStartFile  =   iRdAHD_TellBefore_File[iRdAHD_CurrIx];
      i64StartLoc = i64RdAHD_TellBefore_Loc [iRdAHD_CurrIx];
  }
  else
  {
      iStartFile   = File_Ctr;
      i64StartLoc  = _telli64(FileDCB[File_Ctr]);
  }

  //if (process.Action >= ACTION_NEW_RUNLOC 
  // && process.Action <= ACTION_PLAY )
  //{
       process.startFile = iStartFile;
       process.startLoc  = i64StartLoc;
  //}

  T110_Upd_Posn_TrackBar();
   
    
  d2v_curr.file = iStartFile;
  d2v_curr.lba  = i64StartLoc / MPEG_SEARCH_BUFSZ - 2 ;

  if (d2v_curr.lba < 0)
  {
      if (d2v_curr.file > 0)
      {
        d2v_curr.file --;
        d2v_curr.lba = process.length[d2v_curr.file] / MPEG_SEARCH_BUFSZ - 1;
      }
      else
        d2v_curr.lba = 0;
  }
 
  if (P_Upd_CurrLoc)
  {
      iRC = Get_Hdr_Loc(&process.CurrLoc,  &process.CurrFile);

      //process.CurrFile = d2v_curr.file;
      //process.CurrBlk  = d2v_curr.lba;
      //process.CurrLoc  = process.PACK_Loc ;  //MAYBE NEED RIPPLE ?
      //process.CurrFile = process.PACK_File ;
      //process.CurrLoc  =  Calc_Loc(&process.CurrFile, -4, 0) ;
  }
}



void T110_Upd_Posn_TrackBar()
{
  int iRC;
  int iScrollPos;

  iScrollPos = (int)((process.origin[iStartFile] + i64StartLoc)
                      * TRACK_PITCH
                      / process.total);

#ifdef DBG_RJ
  if (DBGflag)
  {
        sprintf(szDBGln, "       Show ScrollPos=%d, Loc=x%08X", 
                                          iScrollPos, i64StartLoc); 
        DBGout(szDBGln);
  }
#endif

    
  if (process.Action != ACTION_NEW_RUNLOC)
  {
     process.trackPrev = iScrollPos ;
     if (iViewToolBar & 1)
         iRC = SendMessage(hTrack, TBM_SETPOS, (WPARAM) TRUE, iScrollPos);
  }

}





//-----------------------------------
void T580_Trackbar_CLIP()
{
  process.trackleft = (int)(
                      (process.origin[process.startFile] + process.startLoc)
                         //(process.run + process.FromBlk * MPEG_SEARCH_BUFSZ)
                                    * TRACK_PITCH / process.total);

  process.trackright = (int)(
                       (process.origin[process.endFile] + process.endLoc)
                           // (process.run + (__int64)process.ToPadBlk*MPEG_SEARCH_BUFSZ)
                                    * TRACK_PITCH / process.total);

  if (process.trackright > TRACK_PITCH)
      process.trackright = TRACK_PITCH;


#ifdef DBG_RJ
    if (DBGflag)
    {
        sprintf(szDBGln, "       ScrollPosCLIP From=%d To=%d", 
                                          process.trackleft, process.trackright); 
        DBGout(szDBGln);
    }
#endif

  SendMessage(hTrack, TBM_SETSEL, (WPARAM) true,
              (LPARAM) MAKELONG(process.trackleft, process.trackright));


}


//-----------------------------------
void T590_Trackbar_SEL()
{
  if (process.total)
  {
    process.trackleft = (int)(
                      (process.origin[process.FromFile] + process.FromLoc)
                         //(process.run + process.FromBlk * MPEG_SEARCH_BUFSZ)
                                    * TRACK_PITCH / process.total);

    process.trackright = (int)(
                       (process.origin[process.ToPadFile] + process.ToPadLoc)
                           // (process.run + (__int64)process.ToPadBlk*MPEG_SEARCH_BUFSZ)
                                    * TRACK_PITCH / process.total);
  }
  else
  {
    process.trackleft  = 0;
    process.trackright = 0;
  }

  if (process.trackright > TRACK_PITCH)
      process.trackright = TRACK_PITCH;


#ifdef DBG_RJ
    if (DBGflag)
    {
        sprintf(szDBGln, "       ScrollPosSEL From=%d To=%d", 
                                          process.trackleft, process.trackright); 
        DBGout(szDBGln);
    }
#endif

  SendMessage(hTrack, TBM_SETSEL, (WPARAM) true,
              (LPARAM) MAKELONG(process.trackleft, process.trackright));

  UpdateWindow(hTrack);
}



//-----------------------------------
void T599_Trackbar_END()
{

  SendMessage(hTrack,  TBM_SETPOS,  (WPARAM)  TRUE,
                  (int)(TRACK_PITCH));

}

//------------------------------------------------
void T600_Msg_HSCROLL(UINT P_wmId) //DWORD P_ScrollCode, DWORD P_Pos)
{
   int iTrackPos, iTrackDiff;
   DWORD dwRC;

   SetFocus(hWnd_MAIN);
   iTrackPos=SendMessage(hTrack, TBM_GETPOS, 0, 0);

   iTrackDiff = iTrackPos - process.trackPrev;

   if ( iTrackDiff > 5 || iTrackDiff < 5 )
   {
      if (P_wmId == 3 && process.Action == ACTION_RIP)
         MParse.Stop_Flag = 2;  // Allow scroll while playing

      Sleep(20); 
      /*
      dwRC = WaitForSingleObject(hThread_MPEG, 0);

#ifdef DBG_RJ
      if (DBGflag)
      {
          sprintf(szDBGln, "SCROLL GETPOS=%d Prev=%d  MpegRC=x%04X", iTrackPos,  process.trackPrev, dwRC);
          DBGout(szDBGln);
      }
#endif

     if (dwRC == WAIT_OBJECT_0)
     {
     */
         iShowVideo_Flag = iCtl_ShowVideo_Flag;

         szMsgTxt[0] = 0;

         process.trackPrev = iTrackPos;
         process.startrunloc = process.total * iTrackPos / TRACK_PITCH;

#ifdef DBG_RJ
         if (DBGflag)
         {
             sprintf(szDBGln, "       HScrollPos=%d, Loc=x%08X", 
                                         iTrackPos, process.startrunloc); 
             DBGout(szDBGln);
         }
#endif


         iKick.Action = process.Action = ACTION_NEW_RUNLOC;
         if (DBGflag)
             DBGout("TBAR KICK");

         //Sleep(10);

         dwRC = MPEG_processKick();
         //  EnableWindow(hTrack, false); <=== THIS LINE CAUSED WEIRD CRASHES !
         //hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec, 0, 0, &threadId_MPEG);
      //}
   
      //else if (dwRC != STATUS_TIMEOUT)     // 0x0102
         //{
        //  sprintf(szMsgTxt, "rc=x%04x", dwRC);
        //}
   }

   return;

}


//------------------------------------
//  TBM_SETTIC , etc
void Tick_Ctl(int P_Act, int P_File, __int64 P_Loc) 
{
 int iTickPos;
 iTickPos =  (int)(process.origin[P_File]
              + P_Loc
              * TRACK_PITCH / process.total);

 if (iTickPos > TRACK_PITCH)
     iTickPos = TRACK_PITCH;

 SendMessage(hTrack, P_Act, (WPARAM) 0, iTickPos);

}



//------------------------------------
void Tick_CLEAR()
{

 SendMessage(hTrack, TBM_CLEARTICS, (WPARAM) 0, 0);

}


//--------------------

