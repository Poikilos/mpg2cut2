
#include "global.h"
#include "out.h"
#include "Audio.h"

#define true 1
#define false 0

const int TRK_FLD[9] =  {0, TRKS_MPG_0, TRKS_MPG_1, TRKS_MPG_2, TRKS_MPG_3, 
                            TRKS_MPG_4, TRKS_MPG_5, TRKS_MPG_6, TRKS_MPG_7};

unsigned int TRK_TXT[9] = {0, TRK_TXT_0, TRK_TXT_1, TRK_TXT_2, TRK_TXT_3, 
                              TRK_TXT_4, TRK_TXT_5, TRK_TXT_6, TRK_TXT_7};

int iTrk, iTrkSel[8];
HANDLE hDialog;


void Radio_Show()
{
  
  unsigned uField1, uField2;
  //unsigned uAct, uChk;

  if (iOut_Audio_All)
  {
      uField1 = IDU_UNMUX_ALL;
      uField2 = IDU_UNMUX_SELECTIVE_AUDIO_ONLY;
  }
  else
  {
      uField1 = IDU_UNMUX_SELECTIVE_AUDIO_ONLY;
      uField2 = IDU_UNMUX_ALL;
  }

  SendDlgItemMessage(hDialog, uField1, BM_SETCHECK, BST_CHECKED, 0);
  SendDlgItemMessage(hDialog, uField2, BM_SETCHECK, BST_UNCHECKED, 0);

    
  
  uField1 = 0;

  if (iCtl_Out_SplitSegments == 0)
     uField1 = PARTS_SPLIT_NONE;
  else
  if (iCtl_Out_SplitSegments == -1)
      uField1 = PARTS_SPLIT_CLIPS;
  else
  if (iCtl_Out_SplitSegments == 50)
      uField1 = PARTS_SPLIT_50MB;
  else
  if (iCtl_Out_SplitSegments == 100)
      uField1 = PARTS_SPLIT_100MB;
  else
  if (iCtl_Out_SplitSegments == 700)
      uField1 = PARTS_SPLIT_700MB;
  else
  if (iCtl_Out_SplitSegments == 1024)
      uField1 = PARTS_SPLIT_1GB;
  else
  if (iCtl_Out_SplitSegments == 2048)
      uField1 = PARTS_SPLIT_2GB;
  else
  if (iCtl_Out_SplitSegments == 4096)
      uField1 = PARTS_SPLIT_4GB;          

  if (uField1)
      SendDlgItemMessage(hDialog,uField1, BM_SETCHECK,
                                         BST_CHECKED, 0);


}

  
void TRK_Show()
{
  unsigned uChk; // uField, uAct;

  Radio_Show();
          

  //if (iOut_Audio_All)
  //    uAct = BN_DISABLE;
  //else
  //    uAct = BN_ENABLE;
  
  for (iTrk = 0; iTrk < 8; iTrk++)
  {
    if (iOut_Audio_TrkSel[iTrk])
        uChk = BST_CHECKED;
    else
        uChk = BST_UNCHECKED;

    SendDlgItemMessage(hDialog, TRK_FLD[iTrk], BM_SETCHECK, uChk, 0);   
    // SendDlgItemMessage(hDialog, TRK_FLD[iTrk], uAct, uAct, 0);
  }

}


LRESULT CALLBACK Out_Part_Dialog(HWND hDialog_P, UINT message,
                                WPARAM wParam, LPARAM lParam)
{

  int i, iTmp1;

  unsigned uField, uAct; //, uChk;

  int iEnough, iOut_DoIt;

  hDialog = hDialog_P;

  iEnough = 0;

  iOut_DoIt = 0;

  switch (message)
  {
     case WM_INITDIALOG:

          TRK_Show();
          S333_Trk_Audio_Desc(hDialog, &TRK_TXT[0]);

          ShowWindow(hDialog, SW_SHOW);

          return true;


     case WM_COMMAND:
        uAct = 0;
        uField = LOWORD(wParam); 
        switch (uField)
        {
          case IDOK:
               iOut_DoIt = 1;

               // calculate whch streams wanted
               ZeroMemory(&cOut_SubStreamWanted,  sizeof(cOut_SubStreamWanted));
               for (i=0;i<8;i++)
               {
                 if (iOut_Audio_TrkSel[i])
                 {
                     iTmp1 = uAudio_Track_Stream[i];
                     cOut_SubStreamWanted[iTmp1] = 1;
                 }
               }

               iEnough = 2;
               break;


          case IDCANCEL:
               iOut_DoIt = 0;


               iEnough = 2;
               break;


          case PARTS_SPLIT_NONE:
               iCtl_Out_SplitSegments =    0; uAct = BM_SETCHECK; 
               break;
          case PARTS_SPLIT_CLIPS:
               iCtl_Out_SplitSegments =   -1; uAct = BM_SETCHECK; 
               break;
          case PARTS_SPLIT_50MB:
               iCtl_Out_SplitSegments =   50; uAct = BM_SETCHECK; 
               break;
          case PARTS_SPLIT_100MB:
               iCtl_Out_SplitSegments =  100; uAct = BM_SETCHECK; 
               break;
          case PARTS_SPLIT_700MB:
               iCtl_Out_SplitSegments =  700; uAct = BM_SETCHECK; 
               break;
          case PARTS_SPLIT_1GB:
               iCtl_Out_SplitSegments = 1024; uAct = BM_SETCHECK; 
               break;
          case PARTS_SPLIT_2GB:
               iCtl_Out_SplitSegments = 2048; uAct = BM_SETCHECK; 
               break;
          case PARTS_SPLIT_4GB:
               iCtl_Out_SplitSegments = 4096; uAct = BM_SETCHECK; 
               break;


          case IDU_UNMUX_ALL:
               iOut_Audio_All = 1; uAct = BM_SETCHECK; 
               TRK_Show();
               break;

          case IDU_UNMUX_SELECTIVE_AUDIO_ONLY:
               iOut_Audio_All = 0; uAct = BM_SETCHECK; 
               TRK_Show();
               break;

          case TRKS_MPG_0:
               iOut_Audio_All = 0; uAct = 0xFFFFFF;
               iOut_Audio_TrkSel[1] = (SendDlgItemMessage(hDialog,
                        TRKS_MPG_0, BM_GETCHECK, 1, 0) == BST_CHECKED);
               break;
          case TRKS_MPG_1:
               iOut_Audio_All = 0; uAct = 0xFFFFFF;
               iOut_Audio_TrkSel[2] = (SendDlgItemMessage(hDialog,
                        TRKS_MPG_1, BM_GETCHECK, 1, 0) == BST_CHECKED);
               break;
          case TRKS_MPG_2:
               iOut_Audio_All = 0; uAct = 0xFFFFFF;
               iOut_Audio_TrkSel[3] = (SendDlgItemMessage(hDialog,
                        TRKS_MPG_2, BM_GETCHECK, 1, 0) == BST_CHECKED);
               break;
          case TRKS_MPG_3:
               iOut_Audio_All = 0; uAct = 0xFFFFFF;
               iOut_Audio_TrkSel[4] = (SendDlgItemMessage(hDialog,
                        TRKS_MPG_3, BM_GETCHECK, 1, 0) == BST_CHECKED);
               break;
          case TRKS_MPG_4:
               iOut_Audio_All = 0; uAct = 0xFFFFFF;
               iOut_Audio_TrkSel[5] = (SendDlgItemMessage(hDialog,
                        TRKS_MPG_4, BM_GETCHECK, 1, 0) == BST_CHECKED);
               break;
          case TRKS_MPG_5:
               iOut_Audio_All = 0; uAct = 0xFFFFFF;
               iOut_Audio_TrkSel[6] = (SendDlgItemMessage(hDialog,
                        TRKS_MPG_5, BM_GETCHECK, 1, 0) == BST_CHECKED);
               break;
          case TRKS_MPG_6:
               iOut_Audio_All = 0; uAct = 0xFFFFFF;
               iOut_Audio_TrkSel[7] = (SendDlgItemMessage(hDialog,
                        TRKS_MPG_6, BM_GETCHECK, 1, 0) == BST_CHECKED);
               break;
          case TRKS_MPG_7:
               iOut_Audio_All = 0; uAct = 0xFFFFFF;
               iOut_Audio_TrkSel[8] = (SendDlgItemMessage(hDialog,
                        TRKS_MPG_7, BM_GETCHECK, 1, 0) == BST_CHECKED);
               break;
         }

         Radio_Show();

         /*
         if (uAct == BM_SETCHECK) 
         { 
            SendDlgItemMessage(hDialog, PARTS_SPLIT_NONE, 
                                          BM_SETCHECK,  BST_UNCHECKED, 0);
            SendDlgItemMessage(hDialog, PARTS_SPLIT_CLIPS, 
                                          BM_SETCHECK,  BST_UNCHECKED, 0);
            SendDlgItemMessage(hDialog, PARTS_SPLIT_50MB, 
                                          BM_SETCHECK,  BST_UNCHECKED, 0);
            SendDlgItemMessage(hDialog, PARTS_SPLIT_100MB, 
                                          BM_SETCHECK,  BST_UNCHECKED, 0);
            SendDlgItemMessage(hDialog, PARTS_SPLIT_700MB, 
                                          BM_SETCHECK,  BST_UNCHECKED, 0);
            SendDlgItemMessage(hDialog, PARTS_SPLIT_1GB, 
                                          BM_SETCHECK,  BST_UNCHECKED, 0);


            SendDlgItemMessage(hDialog, uField, uAct,
                                                        BST_CHECKED, 0);
         }
         */

         break;
   }


  // KILL ?
  if (iEnough > 1)
  {
      //DestroyWindow(hDialog);
      EndDialog(hDialog, iOut_DoIt);
      //hPostDlg = NULL;
      iEnough = 1;
  }

  return iEnough;

}

