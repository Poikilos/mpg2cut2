
// These routines are compiled with optimization turned off,
// mainly to help communcation between threads.


#include "global.h"
#include "Gui_Comm.h"

#include "Audio.h"
#include "wave_out.h"
#include "AC3Dec\A53_interface.h"
#include "PLUG.H" 
#include "MPV_PIC.H"
#include "Getbit.h"
#include "GetBlk.h"
#include "DDRAW_CTL.h"

#define false 0



int iMpeg_Recursion_Test=0;

//----------------------------------------------------------------------------------------------------------------------------
// Kick the MPEG decoder thread into action
// Eventually this will be the common interface for all MPEG ACTIONS

DWORD MPEG_processKick()   //HANDLE hThread_MPEG)
{
  DWORD dRC, dTst;
  int iRetry;

  if (!File_Limit) // Need at least 1 input file to
  {
     MessageBeep(MB_OK);
     return 0;
  }


  //if (!MParse.Stop_Flag) // threadId_MPEG)
  //  Sleep(20);

  if (iMpeg_Recursion_Test)
  {
    sprintf(szBuffer, "Recursive Call to MpegDec\n\nRqst=%d Prev=%d",
                                            iKick.Action, process.Action);
    MessageBox(hWnd_MAIN, szBuffer, szAppName, MB_OK);

  }

  //if (!MParse.Stop_Flag) // threadId_MPEG)
  //  return;

  dTst = WAIT_OBJECT_0;
  dRC  = WAIT_TIMEOUT;
  iRetry = 0;

  //for(;;)
  {
    dRC = WaitForSingleObject(hThread_MPEG, 0);
    /*
    if (dRC != WAIT_TIMEOUT || iRetry > 30)  // 0x0102-Timeout
      break;
    // Allow for audio to complete
    Sleep(50);
    iRetry++;
    */
  } 


  if (dRC  != WAIT_OBJECT_0
  &&  dRC  != 0xFFFFFFFF )
  {
    if ((dRC  != WAIT_TIMEOUT &&  dRC  != WAIT_ABANDONED)
    || DBGflag)
    {
      sprintf(szMsgTxt, "Wait RC=x%04X",  dRC);
      if (DBGflag)
        DBGout(szMsgTxt);

      DSP1_Main_MSG(0,0);
    }
  }
  else
  {
     // Now that we have waited long enough for the decoder to end
     // we can copy from the separate iKick structure  
     // that was used to avoid conflict with any decode in progress

     if (iKick.Action != ACTION_NOTHING)
     {
         iShowVideo_Flag = iCtl_ShowVideo_Flag;

         process.Action = iKick.Action;
         
         if (iKick.Action == ACTION_NEW_CURRLOC)
         {
             process.CurrFile = iKick.File ;
             process.CurrLoc  = iKick.Loc  ;
         }
         

         iMpeg_Recursion_Test = 1;
         hThread_MPEG = CreateThread(NULL, 0, MPEG2Dec,
                                                    0, 0, &threadId_MPEG);
         iMpeg_Recursion_Test = 0;
     }
  }

  return dRC;
}



/*
char cStopping;
void Stopping_Info()
{
  TextOut(hDC, 0, iMsgPosY, cStopping,  1);
}
*/


void Mpeg_Stop_Rqst()
{

  int iRetry, iTmp1;


  if (DBGflag)
      DBGout("STOP RQST");

  // Let Current decode know that it is time to start cleaning up

  iPlayAudio = 0;
  MParse.SlowPlay_Flag = 0;
  MParse.FastPlay_Flag = 0;
  PlayCtl.iStopNextFrame = 1;

  MParse.Tulebox_SingleStep_flag = 0;
  iKick.Action = 1;

  if (MParse.Pause_Flag)
  {
      MParse.Pause_Flag = 0;
      ResumeThread(hThread_MPEG);
  }


  Sleep(75);
      
  //MParse.Stop_Flag++;
  //Sleep(10); // Allow other task to stop gently

  // PLAY mode takes a while to stop
  if (process.iWavQue_Len    > 0  
  ||  ! MParse.Stop_Flag) 
  // (process.Action == ACTION_RIP )
  {
      Sleep(25);

      if (process.iWavQue_Len > 0  ||  ! MParse.Stop_Flag)
      {
         //MParse.Stop_Flag  = 3;

         if (DBGflag)
             DBGout("STOP PLAY");

         Sleep(25);

         // Give Audio time to flush
         for (iRetry=0; 
                iRetry < 10 
             && process.iWavQue_Len    > 0 
             && process.iWavBytesPerMs > 0;
                iRetry++)
         {
               iTmp1 = process.iWavQue_Len / process.iWavBytesPerMs;
               if (DBGflag)
               {
                   sprintf(szDBGln, "WAIT AUDIO %d ms", iTmp1);
                   DBGout(szDBGln);
               }

               if (iTmp1 > 50)
                   iTmp1 = 50;
               else
               if (iTmp1 < 10)
                   iTmp1 = 10;

               Sleep(iTmp1); 

               if (DBGflag)
                   DBGout("WAITED AUDIO");
               /*
               else
               {
                 cStopping = '-';
                 Stopping_Info();
               }
               */

         }

         MParse.Stop_Flag++;
         Sleep(10); // Allow other task to stop
         process.Action = 0;
               /*
               {
                 cStopping = ' ';
                 Stopping_Info();
               }
               */
      }
  }


  MParse.Stop_Flag++;
  Sleep(20);

  SetFocus(hWnd_MAIN);

  //Menu_Main_Enable();

}


// SingleStep HACK by Tulebox

/*
void GUI_KillThread()
{
 if(MParse.Pause_Flag)
 {
    GUI_ResumeThread();
    ThreadKill();
 }
}


void GUI_ResumeThread()
{
 MParse.Pause_Flag = 0;
 ResumeThread(hThread_MPEG);
}


void GUI_SuspendThread()
{
 MParse.Pause_Flag = 1;
 SuspendThread(hThread_MPEG);
}
*/

// END Tulebox




//--------------------------------------------------------
int  Y100_FIN()
{
  int i; //, iRC;

  // Tell the Player to stop
  Mpeg_Stop_Rqst();

  MParse.Stop_Flag  = 3;
  MParse.Fault_Flag = 97;
  iPlayAudio = 0;

  Sleep(50);

  iFin_Done = 1;
  INI_SAVE();

  Sleep(50);

  if (iWAV_Init)  // Wait for MPEG to stop playing
  {
    if (iCtl_AudioThread  &&  dwSPEAKER_ThreadId)
    {
        PostThreadMessage(dwSPEAKER_ThreadId,
                          WAVEOUT_CLOSE, 0, 0);
        Sleep(100);  
    }
    
    if (iWAV_Init)
    {
      if (hThread_MPEG)
          WaitForSingleObject(hThread_MPEG, 0); // == WAIT_OBJECT_0)
      else
         Sleep(500);  
    }

    if (iWAV_Init)
    {
         Sleep(500);  
         if (iWAV_Init)
         {
             Sleep(500);  
             if (iWAV_Init)
                 WAV_WIN_Spkr_close();
         }
    }
    Sleep(50);  // Crashes hear if window closed while playing audio

  }

  if (byAC3_Init || iMPAdec_Init || iWAV_Init)
  {
      byAC3_Init = 0; iMPAdec_Init = 0;
  }


  // SUPPRESSED following code until I have time to finish it.
  //if (iEDL_Chg_Flag)
  //{
  //   iRC = MessageBox(hWnd, "Save Cut LIST ?", "Mpg2Cut2 - CONFIRM", MB_YESNOCANCEL);
  //   if (iRC == IDYES)
  //   {
  //    iRC = C800_Clip_FILE(SAVE_EDL, 0, 'o');
  //   }
  //}

  // Make sure all files are closed
  Sleep(100);
  F900_Close_Release('c');


  if (iBusy)
  {
    Out_CanFlag = 'C';
    Sleep(100);
    SetPriorityClass(hThread_OUT, IDLE_PRIORITY_CLASS);
    SetThreadPriority(hThread_OUT, THREAD_PRIORITY_IDLE);
  }
  // Release allocated memory

  // Release Direct Draw and various buffers before we terminate
  //ProcessReset("DES");


  //for (i=0; i<8; i++)
  //    free(p_block[i]);


  if (DBGflag)
  {
      fclose(DBGfile) ;
      DBGflag = false;
  }

  if (iBMP_BufSize)
  {
     free(lpBMP_Buffer);
     iBMP_BufSize = 0;
  }


  if (PlugFileRename.hDll)
      Plug89_Free_DLL(&PlugFileRename.hDll);
  if (PlugFileList.hDll)
      Plug89_Free_DLL(&PlugFileList.hDll);

  free(p_fTempArray);

  free(RdAHD_malloc);
  free(lpTmp16K);

  Sleep(100);

  for (i=0; i<=MAX_FILE_NUMBER; i++)
      free(File_Name[i]);

/* if (hLibrary)
       FreeLibrary(hLibrary);
*/
  if (hFont1)
  {
    DeleteObject(hFont1);
    hFont1 = 0;
  }


  if (iBusy)
  {
      Out_CanFlag = 'C';
      Sleep(1000);
      if (iBusy)
      {
         TerminateProcess(hThread_OUT, 666);    
      }
  }

  ReleaseDC(hWnd_MAIN, hDC);
  DeleteObject(hBrush_MASK);  DeleteObject(hBrush_MSG_BG);


  return 1;
}

