//
// Separately compiled sub-routine
// 

#include "windows.h"
#include "global.h"
#include <commctrl.h>

#include "Audio.h"
#include "mpalib.h"
#include "mpalib_more.h"
#include "AC3Dec\A53_interface.h"

#define true 1
#define false 0


  int Old_NTSC_Purity;

struct OLDSTATS
{
  int Old_ScanMode_code, Old_Pic_Structure;
  int Old_Pack_Min_Size, Old_Pack_Max_Size, Old_Pack_Avg_Size; 
  int iMuxRate;
  int Old_aspect_ratio_code, Old_Nom_kBitRate;
  float Old_Frame_Rate;
  char szFrameRate[16];
  int chroma_format,  Profile, Level;
  int horizontal_size, vertical_size;
  char szAudioBoost[32];
} OldStats;

PACKET_CTRS OldPkts;

LRESULT CALLBACK Statistics(HWND, UINT, WPARAM, LPARAM);


//---------------------------------
void  Stats_FPS() //int P_Force)
{
  int iFrames_Diff, iTrue_Pct, iTmp1, iTmp2;
  unsigned uNew_Time_ms, uTime_DIFF_adj, uTmp1;
  char cTmp1;

  PktStats.iChk_AnyPackets = 0;   PktStats.iChk_AudioPackets = 0;

  if (iViewToolBar & 1)
      T100_Upd_Posn_Info(0);

  if (iGOPrelative > iGOPtot)
    iTmp1 = iGOPrelative;
  else
    iTmp1 = iGOPtot;

  sprintf(szBuffer, "%03d  %dF", process.iGOP_Ctr, iTmp1);
  SetDlgItemText(hStats, IDC_GOP_CTR, szBuffer);

  if (process.Action == ACTION_RIP)
  {
     uNew_Time_ms = timeGetTime();
     iTmp1 = (int)(uNew_Time_ms  - PlayCtl.uPrev_Time_ms[1] + 1);
     PlayCtl.iVideoTime_DIFF_ms = (int)(uNew_Time_ms  - PlayCtl.uPrev_Time_ms[0] + 1);
     //PlayCtl.uTime_DIFF_s = PlayCtl.iVideoTime_DIFF_ms / 1000;
     if (iTmp1 > 1000)
     {
         iFrames_Diff = Frame_Number - PlayCtl.uPrev_Frames;

         PlayCtl.iDecoded_hFPS = iFrames_Diff * 100000 / PlayCtl.iVideoTime_DIFF_ms;
         PlayCtl.iFPS_Dec_Pct  = PlayCtl.iDecoded_hFPS / iFrame_Rate_int;

         iTrue_Pct = PlayCtl.iFPS_Dec_Pct;

         if (MParse.FastPlay_Flag >= CUE_SLOW)  // CUE mode
         {
            if (MParse.FastPlay_Flag >= (CUE_SLOW+2))  // SUPER-CUE mode
            {
                iTmp1 = MParse.FastPlay_Flag - (CUE_SLOW+2);
                iTmp2 = 24 * iJumpSecs[iTmp1];
            }
            else
              iTmp2 = 12;
            
            iTrue_Pct = iTrue_Pct * iTmp2;
         }

         if (MParse.SlowPlay_Flag > 0)
         {
             if (MParse.SlowPlay_Flag == 1)
                 iTmp2 = 5;
             else
                 iTmp2 = MParse.SlowPlay_Flag * 4;

             PlayCtl.iFPS_Dec_Pct = (PlayCtl.iFPS_Dec_Pct * iTmp2) / 4;
         }
         else
         if (MParse.FastPlay_Flag)
         {
           //if (MParse.FastPlay_Flag >= 2) // Very Fast ?
           switch (MParse.FastPlay_Flag)
           {
           case 0: break;
           case 1: PlayCtl.iFPS_Dec_Pct = PlayCtl.iFPS_Dec_Pct * 2 / 3; break;
           case 2: PlayCtl.iFPS_Dec_Pct = PlayCtl.iFPS_Dec_Pct / 2; break;
           case 3: PlayCtl.iFPS_Dec_Pct = PlayCtl.iFPS_Dec_Pct / 3; break;
           case 4: PlayCtl.iFPS_Dec_Pct = PlayCtl.iFPS_Dec_Pct / 4; break;
           default: 
             PlayCtl.iFPS_Dec_Pct = PlayCtl.iFPS_Dec_Pct  
                                  * (MParse.FastPlay_Flag + 1)
                                  / (MParse.FastPlay_Flag * 3); // * 2 / 6;
           }
         }


         // Trap poor performance
         if (iFrames_Diff > 2 
         // && PlayCtl.iDrop_Behind 
         // &&  PlayCtl.iFPS_Dec_Pct < 70
         )
             Timing_DropMore();

         iVideoBitRate_Avg = (iVideoBitRate_Bytes<<3) / PlayCtl.iVideoTime_DIFF_ms;
                             // * iFrame_Rate_int / iFrames_Diff) >>7 ;// fFrame_Rate_Orig / iFrames_Diff) >>7 ;

         if (MParse.ShowStats_Flag)
         {
            sprintf(szBuffer,   "%d",      iVideoBitRate_Avg) ;
            SetDlgItemText(hStats,     IDC_BITRATE_AVG, szBuffer);

            if (iMuxChunkRate) //(iNom_kBitRate)
                iTmp1 = iMuxChunkRate * 2 / 5; // iNom_kBitRate;
            else
            if (iNom_kBitRate)
                iTmp1 = iNom_kBitRate;
            else
            if (Coded_Pic_Width <= 400)
               iTmp1 =  2000;
            else
            if (Coded_Pic_Width < 769)
               iTmp1 = 10000;
            else
               iTmp1 = 40000;

            iTmp1 = iVideoBitRate_Avg * 100 / iTmp1; // Calculate percantage 
            if (iTmp1 > 200)
                iTmp1 = 200;
            SendDlgItemMessage(hStats, IDC_BITRATE_BAR, PBM_SETPOS, 
                                            iTmp1,  0); 

            sprintf(szBuffer, "%.1f", (float)(100000 * PlayCtl.iShown_Frames / PlayCtl.iVideoTime_DIFF_ms) / 100.0);
            SetDlgItemText(hStats, IDC_FPS_SHOWN, szBuffer); 

            sprintf(szBuffer, "%.1f", (float)(100000 * PlayCtl.iDropped_Frames / PlayCtl.iVideoTime_DIFF_ms) / 100.0);
            SetDlgItemText(hStats, IDC_FPS_DROPPED, szBuffer);

            sprintf(szBuffer, "%.1f", (float)(PlayCtl.iDecoded_hFPS) / 100.0);
            SetDlgItemText(hStats, IDC_FPS_DECODED, szBuffer);

            if (iTrue_Pct < 400)
                sprintf(szBuffer, "%d%%", (iTrue_Pct));
            else
                sprintf(szBuffer, "x%d ", ((iTrue_Pct+50)/100));
            SetDlgItemText(hStats, STATS_DEC_PCT, szBuffer);
            
            if (PlayCtl.iDrop_Behind)
              cTmp1 = 'D';
            else
              cTmp1 = 'd';

            sprintf(szBuffer, "D%d", PlayCtl.iDrop_B_Frames_Flag);
            SetDlgItemText(hStats, STATS_DROP_MODE, szBuffer);
         }

        PlayCtl.uPrev_Time_ms[1] = uNew_Time_ms;

        // Average up to 2 seconds
        if (PlayCtl.iVideoTime_DIFF_ms > 2000)
        {

           PlayCtl.uPrev_Time_ms[0] = uNew_Time_ms;
           PlayCtl.uPrev_Frames     =  Frame_Number;

           iVideoBitRate_Bytes     = 0;
           PlayCtl.iShown_Frames   = 0; 
           PlayCtl.iDropped_Frames = 0;
        }
      }
  }
  else 
  {
    //if (CurrTC.VideoDTS)
    //    PlayCtl.iVideoTime_DIFF_ms = CurrTC.VideoDTS - PrevTC.VideoDTS;
    //else
          PlayCtl.iVideoTime_DIFF_ms = CurrTC.VideoPTS - PrevTC.VideoPTS;

    // Allow for Nebula Card under stress = Audio with no Video
    PlayCtl.iAudioTime_DIFF_ms = CurrTC.AudioPTS - PrevTC.AudioPTS;
    if (PlayCtl.iVideoTime_DIFF_ms < PlayCtl.iAudioTime_DIFF_ms)
        PlayCtl.iVideoTime_DIFF_ms = PlayCtl.iAudioTime_DIFF_ms;

    if (iFrame_Rate_int
    &  (PlayCtl.iVideoTime_DIFF_ms <= 0 || PlayCtl.iVideoTime_DIFF_ms > 90000))
    {
        PlayCtl.iVideoTime_DIFF_ms = Frame_Number * 45000 / iFrame_Rate_int;
        if (MParse.SlowPlay_Flag > 0)
        {
            if (MParse.SlowPlay_Flag == 1)
                iTmp2 = 5;
            else
                iTmp2 = MParse.SlowPlay_Flag * 4;

            PlayCtl.iVideoTime_DIFF_ms = (PlayCtl.iVideoTime_DIFF_ms * iTmp2) / 4;
        }
        else
        if (MParse.FastPlay_Flag)
        {
            PlayCtl.iVideoTime_DIFF_ms /= 2;
        }

        if (PlayCtl.iVideoTime_DIFF_ms <= 0)
            PlayCtl.iVideoTime_DIFF_ms = 1800; 
    }
      
    //PlayCtl.uTime_DIFF_s = PlayCtl.iVideoTime_DIFF_ms / 1000;

    if (PlayCtl.iVideoTime_DIFF_ms
    &&  MPEG_Pic_Type == I_TYPE
    && (process.Action == ACTION_FWD_GOP || process.Action == ACTION_INIT))
    {
       iVideoBitRate_Avg = (iVideoBitRate_Bytes * 360 ) / PlayCtl.iVideoTime_DIFF_ms; //  360=8*45000/1k

       if (MParse.ShowStats_Flag)
       {
         sprintf(szBuffer,   "%d",    iVideoBitRate_Avg) ;
         SetDlgItemText(hStats,     IDC_BITRATE_AVG, szBuffer);

            if (iMuxChunkRate) //(iNom_kBitRate)
                iTmp1 = iMuxChunkRate * 2 / 5; // iNom_kBitRate;
            else
            if (iNom_kBitRate)
                iTmp1 = iNom_kBitRate;
            else
            if (Coded_Pic_Width <= 400)
               iTmp1 =  2000;
            else
            if (Coded_Pic_Width < 769)
               iTmp1 = 10000;
            else
               iTmp1 = 40000;

            iTmp1 = iVideoBitRate_Avg * 100 / iTmp1; // Calculate percantage 
            if (iTmp1 > 200)
                iTmp1 = 200;
            SendDlgItemMessage(hStats, IDC_BITRATE_BAR, PBM_SETPOS, 
                                            iTmp1,  0); 
       }
    }
  }



  if (MParse.ShowStats_Flag 
  && (MPEG_Pic_Type  == I_TYPE)//  ||  P_Force)
     )
  {
          if (MParse.SystemStream_Flag < 0)
          {
            sprintf(szBuffer, "TS Pkts %d", PktStats.iTS_Packets);
            SetDlgItemText(hStats, IDC_PS2_STATUS, szBuffer);
          }

      if (PktStats.iTS_ReSyncs)
      {
         sprintf(szTmp32, "%d R#%d", 
                                  (PktStats.iTS_BadBytes
                                  /PktStats.iTS_ReSyncs),
                                   PktStats.iTS_ReSyncs);
         SetDlgItemText(hStats, STATS_PACK_LENS, szTmp32);

         //sprintf(szMsgTxt, "TS %s", szTmp32);
         //DSP1_Main_MSG(0,0);
      }

  }


  // Update Packet stats 
  if (MParse.ShowStats_Flag 
  &&  process.Action == ACTION_RIP 
  && (MPEG_Pic_Type  == I_TYPE)//  ||  P_Force)
     )
  {
    uTime_DIFF_adj = (uNew_Time_ms - PlayCtl.uPrev10_Time_ms) / 1000;

    if (uTime_DIFF_adj < 1)
        uTime_DIFF_adj = 1;

    if (uTime_DIFF_adj > 3 || Frame_Number < 33)
    {
          //PlayCtl.uPrev10_Time_ms = uNew_Time_ms;

      uTmp1 = PktStats.iVid_Packets/uTime_DIFF_adj;
      if (OldPkts.iVid_Packets !=  uTmp1)
      {
          sprintf(szBuffer, "%u", uTmp1);
          SetDlgItemText(hStats, IDC_VPK_CTR, szBuffer);
          OldPkts.iVid_Packets =  uTmp1;
      }

      uTmp1 = PktStats.iMPA_Packets/uTime_DIFF_adj;
      if (OldPkts.iMPA_Packets !=  uTmp1)
      {
          sprintf(szBuffer, "%u", uTmp1);
          SetDlgItemText(hStats, IDC_MPA_CTR, szBuffer);
          OldPkts.iMPA_Packets =  uTmp1;
       }

       uTmp1 = PktStats.iPS1_Packets/uTime_DIFF_adj;
       if (OldPkts.iPS1_Packets !=  uTmp1)
       {
          sprintf(szBuffer, "%u", uTmp1);
          SetDlgItemText(hStats, IDC_PS1_CTR, szBuffer);
          OldPkts.iPS1_Packets =  uTmp1;
       }

       uTmp1 = PktStats.iPS2_Packets/uTime_DIFF_adj;
       if (OldPkts.iPS2_Packets !=  uTmp1)
       {
          sprintf(szBuffer, "%u", uTmp1);
          SetDlgItemText(hStats, IDC_PS2_CTR, szBuffer);
          OldPkts.iPS2_Packets =  uTmp1;
       }

       if (MParse.SystemStream_Flag < 0)
           uTmp1 = PktStats.iTS_Packets;
       else
           uTmp1 = PktStats.iSubTit_Packets;

       uTmp1 = uTmp1 / uTime_DIFF_adj;
       if (OldPkts.iTS_Packets != uTmp1)
       {
          sprintf(szBuffer, "%u", uTmp1);
          SetDlgItemText(hStats, IDC_SUBT_CTR, szBuffer);
          OldPkts.iTS_Packets =  uTmp1;
       }

       uTmp1 = PktStats.iPad_Packets/uTime_DIFF_adj;
       if (OldPkts.iPad_Packets !=  uTmp1)
       {
          sprintf(szBuffer, "%u", uTmp1);
          SetDlgItemText(hStats, IDC_PAD_CTR, szBuffer);
          OldPkts.iPad_Packets =  uTmp1;
       }

       uTmp1 = PktStats.iUnk_Packets/uTime_DIFF_adj;
       if (OldPkts.iUnk_Packets !=  uTmp1)
       {
          sprintf(szBuffer, "%u x%02X", uTmp1, getbit_Unk_Stream_Id);
          SetDlgItemText(hStats, IDC_UNK_CTR, szBuffer);
          OldPkts.iUnk_Packets =  uTmp1;
       }

    }

    if (uTime_DIFF_adj > 3) // Refresh every few secs
    {
          ZeroMemory(&PktStats, sizeof(PktStats));
          PlayCtl.uPrev10_Time_ms = uNew_Time_ms;
    }
  }

    
  if (iMsgLife 
  &&  process.Action  ==  ACTION_RIP)
  {
      iMsgLife--;
      if (  !  iMsgLife) //  && iPreview_Clip_Ctr >= iEDL_ctr)
      {
           DSP_Msg_Clear();
           if (iViewToolBar > 1 && iPreview_Clip_Ctr < 900 ) 
              DSP2_Main_SEL_INFO(1);
      }
   }


  return;

}




//--------------------------------------------------------------------
// Show stats that only change at sequence header or are relatively stable 
void S100_Stats_Hdr_Main(int P_Refresh)
{
  int  iTmp1, iTmp2, iTmp3;

  char szTmp16[16], szTmp04[4];

  const char TEXTURE_CAT[5][8] = { " ", "Chunky", "Rough", "Freq A", "Fine AV"};
  char *lpTexture_Cat, *lpSkel;

  const char FieldMode_Name[4][10] = {"? Pic Fmt", "Top First", "Bot First", "NON-Field"};




  if (OldStats.Old_ScanMode_code != ScanMode_code)
  {
     sprintf(szBuffer, "%s", ScanMode_Name[ScanMode_code]);
     SetDlgItemText(hStats, IDC_SCAN_MODE1, szBuffer);
     OldStats.Old_ScanMode_code  = ScanMode_code;
  }
    
  if (OldStats.Old_Pic_Structure != MPEG_Pic_Structure)
  {
      sprintf(szBuffer, "%s", FieldMode_Name[MPEG_Pic_Structure]);
      SetDlgItemText(hStats, IDC_SCAN_MODE2, szBuffer);
      OldStats.Old_Pic_Structure = MPEG_Pic_Structure;
  }


  if (OldStats.Old_Frame_Rate != fFrame_Rate_Orig)
  {
    if (fFrame_Rate_Orig == (float)(iFrame_Rate_int))
        sprintf(OldStats.szFrameRate, "%d fps",   iFrame_Rate_int);
    else
        sprintf(OldStats.szFrameRate, "%.3f fps", fFrame_Rate_Orig);

    SetDlgItemText(hStats, IDC_FRAME_RATE, OldStats.szFrameRate);
    OldStats.Old_Frame_Rate  = fFrame_Rate_Orig;
  }


  if (!FILM_Purity)
  {
     if (frame_rate > 28 && frame_rate < 31)
         sprintf(szTmp32, "NTSC");
     else
     if (frame_rate > 58 && frame_rate < 62)
         sprintf(szTmp32, "NTSC+ ");
     else
     if (frame_rate == 25)
     {
       if (Coded_Pic_Height >= 576 || Coded_Pic_Height == 288 || Coded_Pic_Height == 144)
         sprintf(szTmp32, "PAL");
       else
         sprintf(szTmp32, "PAL ?");
     }
     else
     if (frame_rate == 50)
     {
         sprintf(szTmp32, "PAL+ ");
     }
     else
     if (Coded_Pic_Height == 576 || Coded_Pic_Height == 288 || Coded_Pic_Height == 144)
         sprintf(szTmp32, "PAL- ");
     else
         sprintf(szTmp32, "?");
  }
  else
  if (NTSC_Purity || FILM_Purity)
  {
      if (!NTSC_Purity)
        sprintf(szTmp32, "FILM");
      else 
      if (NTSC_Purity > Old_NTSC_Purity)
        sprintf(szTmp32, "ntsc%2d %%", 100 - (FILM_Purity*100)/(FILM_Purity+NTSC_Purity));
      else
        sprintf(szTmp32, "film%2d %%", (FILM_Purity*100)/(FILM_Purity+NTSC_Purity));

      Old_NTSC_Purity = NTSC_Purity;
  }

  //sprintf(szBuffer, "%s %s", szTmp32, OldStats.szFrameRate);
  SetDlgItemText(hStats, IDC_VIDEO_STANDARD,  szTmp32); //szBuffer);


  // Describe the video stream format
  if ( Mpeg_SEQ_Version  != Mpeg_PES_Version)
    sprintf(szTmp16, "+%d", Mpeg_PES_Version);
  else
    szTmp16[0] = 0;

  sprintf(szTmp32, "mpeg-%d%s", Mpeg_SEQ_Version, szTmp16);

  if (MParse.SystemStream_Flag > 0)
  {
    if (getbit_VOB_ID  < 999990)
      sprintf(szBuffer, "Vob%d Cell%d", getbit_VOB_ID, getbit_CELL_ID);
    else
      sprintf(szBuffer, "%s PS", szTmp32);
  }
  else
  if (MParse.SystemStream_Flag < 0)
  {
    if (uVid_PID < STREAM_AUTO)
      iTmp1 = uVid_PID;
    else
      iTmp1 = 000;

    sprintf(szBuffer, "TS PID %d", iTmp1);
  }
  else
      sprintf(szBuffer, "%s ES", szTmp32);

  if (strcmp(StatsPrev.VobTxt, szBuffer)
  || P_Refresh)
  {
     strcpy(StatsPrev.VobTxt, szBuffer);
     SetDlgItemText(hStats, IDC_VOB_ID, szBuffer);
  }


  if (iMuxChunkRate != OldStats.iMuxRate)
  {
      if (process.iFixedRate)
          strcpy(szTmp04,"F ");
      else
          szTmp04[0] = 0;
    sprintf(szBuffer, "%s%d", szTmp04, (iMuxChunkRate*2/5));  // 50*8/1000
    SetDlgItemText(hStats, IDC_MUX_RATE, szBuffer);
    OldStats.iMuxRate = iMuxChunkRate;
  }


  if (OldStats.chroma_format != MPEG_Seq_chroma_format
  ||  OldStats.Profile       != MPEG_Profile
  ||  OldStats.Level         != MPEG_Level)
  {
    //if (MPEG_Seq_chroma_format == 1) // 4:2:0 is very, very common
    //   iTmp1 = 4;                    // so don't report it
    //else
       iTmp1 = MPEG_Seq_chroma_format;

    sprintf(szBuffer, "%s%c%s %s", MPEG_Profile_Desc[MPEG_Profile], 
                                   MPEG_ProfLvl_Escape,
                                   MPEG_Level_Desc[MPEG_Level],
                                   MPEG_Seq_chroma_Desc[iTmp1]);

    SetDlgItemText(hStats, IDC_CHROMA_FMT, szBuffer);

    OldStats.chroma_format = MPEG_Seq_chroma_format;
    OldStats.Profile       = MPEG_Profile;
    OldStats.Level         = MPEG_Level;
  }

  if (MParse.iColorMode == STORE_YUY2)
        strcpy(szTmp32, "YUY2");
  else
        strcpy(szTmp32, "RGB");

  SetDlgItemText(hStats, IDC_OVL_MODE, szTmp32);


  S300_Stats_Audio_Desc();

  if (P_Refresh || WAV_Fmt_Flag == 1)
  {
      SetDlgItemText(hStats, STATS_AUDIO_WAV, WAV_Fmt_Brief);
      WAV_Fmt_Flag = 0;
  }


  /*
  strcpy(szBuffer, "tidw");
  if (mpa_Ctl[0].rip)                szBuffer[0] = 'T';
  if (iMPAdec_Init)                  szBuffer[1] = 'I';
  if (byMPALib_OK)                   szBuffer[2] = 'D';
  if (iWAV_Init)                     szBuffer[3] = 'W';
  //if (iPlayAudio)                   szBuffer[4] = 'P';
  //if (MParse.Rip_Flag)               szBuffer[5] = 'R';
  SetDlgItemText(hStats, STATS_AUDIO_FLAGS, szBuffer);
  */

  if (process.PES_Audio_CRC_Flag)
  {
     strcpy(szBuffer, "CRC");
  }
  else
  {
    szBuffer[0] = 0;
  }

  SetDlgItemText(hStats, STATS_AUDIO_FLAGS, szBuffer);
                          
  if (OldStats.Old_Pack_Min_Size != process.Pack_Min_Size
  ||  OldStats.Old_Pack_Max_Size != process.Pack_Max_Size
  ||  OldStats.Old_Pack_Avg_Size != process.Pack_Avg_Size)
  {
    if (process.Pack_Max_Size > 0)
    {
       if (!process.Pack_Avg_Size)
       {
         process.Pack_Avg_Size = (process.Pack_Min_Size 
                                + process.Pack_Max_Size + 1024) / 2;
       }

       iTmp1 = process.Pack_Avg_Size/1024;
       iTmp2 = process.Pack_Min_Size/1024;
       iTmp3 = process.Pack_Max_Size/1024;

       if (iTmp1 == iTmp2 && iTmp1 == iTmp3)
          lpSkel = &"%dk packs";
       else
          lpSkel = &"%dk [%dk..%dk]";

       sprintf(szBuffer, lpSkel, iTmp1, iTmp2, iTmp3);

       SetDlgItemText(hStats, STATS_PACK_LENS, szBuffer);
       OldStats.Old_Pack_Min_Size  = process.Pack_Min_Size;
       OldStats.Old_Pack_Avg_Size  = process.Pack_Avg_Size;
       OldStats.Old_Pack_Max_Size  = process.Pack_Max_Size;
    }
  }
  //process.Pack_Min_Size = 9999999;
  //process.Pack_Max_Size = 0;

  sprintf(szBuffer, " i %d", process.iAudio_Interleave); // Interleave gap: VideoFrames between Audio Packets
  SetDlgItemText(hStats, STATS_AUDIO_INTERLEAVE, szBuffer);

  //lpTexture_Cat = (char *)(&TEXTURE_CAT[0]);
  if (process.PIC_Loc > 512000 
  &&  process.Action >= ACTION_FWD_GOP
  &&  process.Action <  ACTION_FWD_GOP2)
  {
      if (  ! process.Got_PTS_Flag)       // SEQ hdr pkt missing PTS ?
        lpTexture_Cat = (char *)(&"NO PTS");
     else
     if ( process.iAudio_Interleave < 5
     && StatsPrev.iAudio_Interleave < 5)
     {
       if ( process.iVid_PTS_Resolution == 1 
       && StatsPrev.iVid_PTS_Resolution == 1)
         lpTexture_Cat = (char *)(&TEXTURE_CAT[4][0]);
       else
         lpTexture_Cat = (char *)(&TEXTURE_CAT[3][0]);
     }
     else
       if  (process.iVid_PTS_Resolution < 5 
       && StatsPrev.iVid_PTS_Resolution < 5)
         lpTexture_Cat = (char *)(&TEXTURE_CAT[2][0]);
       else
         lpTexture_Cat = (char *)(&TEXTURE_CAT[1][0]);

     strcpy(szBuffer,lpTexture_Cat);
     SetDlgItemText(hStats, STATS_TEXTURE, lpTexture_Cat);

     StatsPrev.iAudio_Interleave   = process.iAudio_Interleave;
     StatsPrev.iVid_PTS_Resolution = process.iVid_PTS_Resolution;
  }

  Stats_Volume_Boost();
}




//------------- Sequence Hdr Info only -----------
void S120_Stats_Hdr_Seq()
{
  char *lpszAspect;

  if (OldStats.Old_aspect_ratio_code != MPEG_Seq_aspect_ratio_code
  ||  OldStats.horizontal_size   != MPEG_Seq_horizontal_size
  ||  OldStats.vertical_size     != MPEG_Seq_vertical_size)
  {
      if (process.Mpeg2_Flag || Mpeg_PES_Version == 2)
        lpszAspect = Mpeg2_Aspect_Ratio_Name[MPEG_Seq_aspect_ratio_code];
      else
        lpszAspect = Mpeg1_Aspect_Ratio_Name[MPEG_Seq_aspect_ratio_code];

      sprintf(szBuffer, "%d.%d %s", 
              MPEG_Seq_horizontal_size, MPEG_Seq_vertical_size,
              lpszAspect);
      SetDlgItemText(hStats, IDC_ASPECT_RATIO, szBuffer);
      OldStats.Old_aspect_ratio_code  = MPEG_Seq_aspect_ratio_code;
      OldStats.horizontal_size        = MPEG_Seq_horizontal_size;
      OldStats.vertical_size          = MPEG_Seq_vertical_size;
  }

  if (OldStats.Old_Nom_kBitRate != iNom_kBitRate)
  {
      sprintf(szBuffer, "%d", iNom_kBitRate) ;
      SetDlgItemText(hStats, IDC_BITRATE_NOM, szBuffer);
      OldStats.Old_Nom_kBitRate  = iNom_kBitRate;
  }

}


//----------
// Show stats that can change with each picture

void S200_Stats_Pic_Main(int P_Force)
{

  char *lpszFMT;
  //__int64 i64Tmp1;
  int iTmp1; // pts;
  //sprintf(szBuffer, "%d:%02d:%02d", RelativeTC.hour, RelativeTC.minute,
  //                                  RelativeTC.sec,  RelativeTC.frameNum);

  if (MPEG_Pic_Type == I_TYPE || process.Action != ACTION_RIP  || P_Force)
  {
     if ( gopTC.hour)
          sprintf(szBuffer, "%d:%02d:%02d.%02d", gopTC.hour, gopTC.minute,
                                                 gopTC.sec,  gopTC.frameNum);
     else
         sprintf(szBuffer, "%02d:%02d.%02d", gopTC.minute,
                                             gopTC.sec, gopTC.frameNum);

     SetDlgItemText(hStats, IDC_GOP_TC, szBuffer);
/*
     if (OrgTC.hour)
         sprintf(szBuffer, "%d:%02d:%02d.%02d", OrgTC.hour, OrgTC.minute,
                                                OrgTC.sec, OrgTC.frameNum);
     else
         sprintf(szBuffer, "%02d:%02d.%02d", OrgTC.minute,
                                             OrgTC.sec, OrgTC.frameNum);
     SetDlgItemText(hStats, IDC_ORG_TC, szBuffer);
*/
     iTmp1 = process.iVid_PTS_Diff;  // process.iVid_PTS_Seq_Diff;
     if (process.Action >= ACTION_FWD_GOP 
     &&  process.Action <  ACTION_FWD_GOP2)
     {
         if (iTmp1 > 9999)
         {
           iTmp1 = (iTmp1 + 250) / 1000; // Round up (a bit) to integer
           lpszFMT = &"@ %d %ds";  // /gop";
         }
         else
           lpszFMT = &"@ %d %dms";  // /gop";
     }
     else
     {
         lpszFMT = &"@ %d";
     }

     sprintf(szBuffer, lpszFMT, process.iVid_PTS_Resolution, // Frames per unique PTS 
                                iTmp1);

     SetDlgItemText(hStats, STATS_VPTS_RES, szBuffer);

     sprintf(szBuffer, "%02d %c", MPEG_Pic_Temporal_Ref,
                             Coded_Pic_Abbr[MPEG_Pic_Type]);
     SetDlgItemText(hStats, STATS_PIC_TREF, szBuffer);


     if (process.Action == ACTION_RIP)
     {
      if (process.op)
      { 
       process.ed        = timeGetTime();
       process.elapsed = (process.ed-process.op)/1000;
       process.percent = (float)(100 * ( process.run-process.startrunloc
                                       + MParse.NextLoc
                                       )
                                 / (process.endrunloc-process.startrunloc));

       process.remain = (int)(   (process.ed - process.op)
                                     * (       100.0 - process.percent)
                                     / process.percent
                                    ) / 1000;
      
       sprintf(szBuffer, "%d:%02d:%02d", process.elapsed/3600,
                                        (process.elapsed%3600)/60,
                                         process.elapsed%60);
       SetDlgItemText(hStats, IDC_ELAPSED, szBuffer);

       sprintf(szBuffer, "%d:%02d:%02d", process.remain/3600,
                                       (process.remain%3600)/60,
                                        process.remain%60);
       SetDlgItemText(hStats, IDC_REMAIN, szBuffer);
      }
     }

      /*
      else if (GetDlgItemText(hStats, IDC_ELAPSED, szBuffer, 9))
      {
      SetDlgItemText(hStats, IDC_VIDEO_STANDARD, "");
      SetDlgItemText(hStats, IDC_SCAN_MODE1, "");
      SetDlgItemText(hStats, IDC_CODED_NUMBER, "");
      SetDlgItemText(hStats, IDC_PLAYBACK_NUMBER, "");
      SendDlgItemMessage(hStats, IDC_BITRATE_BAR, PBM_SETPOS, 0, 0);
      SetDlgItemText(hStats, IDC_FILE, "");
      SetDlgItemText(hStats, IDC_FILE_SIZE, "");
      SetDlgItemText(hStats, IDC_ELAPSED, "");
      SetDlgItemText(hStats, IDC_REMAIN, "");
      SetDlgItemText(hStats, IDC_FPS_DECODED, "");
      }
      */

  PTS_2Field( process.VideoPTS, IDC_VID_PTS);
  //pts = process.VideoPTS/45000; //90000 ;
  //iTmp1 = (int)((process.VideoPTS - (pts * 90000)) / frame_rate) ;
  //sprintf(szBuffer, "%d:%02d:%02d.%04d", 
  //                   pts/3600, (pts%3600)/60, pts%60, iTmp1);
  //SetDlgItemText(hStats, IDC_VID_PTS, szBuffer);
  

  //sprintf(szBuffer, "%d:%02d:%02d.%02d", CurrTC.hour, CurrTC.minute,
  //                                  CurrTC.sec,  CurrTC.frameNum);
  //SetDlgItemText(hStats, IDC_PIC_TC, szBuffer);



  //if (MParse.SystemStream_Flag && process.Action != ACTION_INIT
                       // && process.AudioPTS != 0
  //   )
  //{  
      if (process.AudioPTS != PTS_NOT_FOUND)
          PTS_2Field( process.AudioPTS, IDC_AUDIO_PTS);
      else
          SetDlgItemText(hStats, IDC_AUDIO_PTS, "");

    //pts = process.AudioPTS/90000;
      //iTmp1 = (int)((process.AudioPTS - (pts * 90000)) / frame_rate) ;
    //sprintf(szBuffer, "%d:%02d:%02d.%04d", 
    //            pts/3600, (pts%3600)/60, pts%60, iTmp1);

  //}
  //else
  //  strcpy(szBuffer, " ");

  //SetDlgItemText(hStats, IDC_AUDIO_PTS, szBuffer);

   PTS_2Field(process.DelayPTS, IDC_AUDIO_DELAY);
   SetDlgItemText(hStats, STATS_DELAY_SIGN, process.Delay_Sign);

   if (process.VideoDTS != PTS_NOT_FOUND)
       PTS_2Field(process.VideoDTS, IDC_VID_DTS);

   if (process.uViewSCR != PTS_NOT_FOUND)
       PTS_2Field( process.uViewSCR, IDC_SCR);


   // NEED process.SSCRM-> i64Tmp1->PTS_2Field-> IDC_SCR
   if (process.Suspect_SCR_Flag)
       SetDlgItemText(hStats, IDC_SCR_TXT, "WEIRD");
   else
       SetDlgItemText(hStats, IDC_SCR_TXT, "");
       

 } // END-IF  Time for a major stats update

}


int iTrk_External, iTrk_Srch;


//------------------------------------------------


void S333_Trk_Audio_Desc(HANDLE hDial, unsigned int *TXT_FLD)
{
  int iFormat;

  // First - The Mpeg Audio info

  iTrk_External = 0;
  iTrk_Srch = 0;
  while (iTrk_Srch < 8)
  {
     if (mpa_Ctl[iTrk_Srch].uStream)
     {
        iTrk_External = iTrk_Srch + 1;
        uAudio_Track_Stream[iTrk_External] = mpa_Ctl[iTrk_Srch].uStream;
        S370_AudioTrackDesc(FORMAT_MPA, iTrk_Srch, iTrk_External);
        SetDlgItemText(hDial, TXT_FLD[iTrk_External], szBuffer);
     }
     iTrk_Srch++;
  };
  
  // Now the Private Substreams (Typically AC3 or LPCM)

  // Allow for mixed Mpeg & AC3 audio tracks with same relative track number
  //if (iTrk_External > 0 && iTrk_External < 5)
  //    iTrk_External = 5;

  for (iFormat = 0; iFormat < 5; iFormat++)
  {
    iTrk_Srch = 0;
    while (iTrk_External < 8 && iTrk_Srch < 8)
    {
      if (SubStream_CTL[iFormat][iTrk_Srch].uStream)
      {
          iTrk_External++;          
          uAudio_Track_Stream[iTrk_External] = SubStream_CTL[iFormat][iTrk_Srch].uStream;
          S370_AudioTrackDesc(iFormat, iTrk_Srch, iTrk_External);
          SetDlgItemText(hDial, TXT_FLD[iTrk_External], szBuffer);
      }

      iTrk_Srch++;

    } // endwhile TRK
  }; // endfor FORMAT

}



unsigned int IDC_AUDIO_FIELD[9] 
  = {0, IDC_AUDIO_TYPE_0, IDC_AUDIO_TYPE_1, IDC_AUDIO_TYPE_2, IDC_AUDIO_TYPE_3, 
        IDC_AUDIO_TYPE_4, IDC_AUDIO_TYPE_5, IDC_AUDIO_TYPE_6, IDC_AUDIO_TYPE_7};





void S300_Stats_Audio_Desc()
{
  //int iTmp1;

  //const char AUDIO_FORMAT_ABBR[9][5] = { "any", "MPa", "PCM", "AC3", "DTS", "DD+", "F-6", "PS1", "PS2"};




  //if (iWant_Aud_Format == FORMAT_AUTO) strcpy(szBuffer, "Auto") ;
  //else                           
  //     sprintf(szBuffer, "%d", iWant_Aud_Format) ;
  SetDlgItemText(hStats, IDC_FORMAT_FLAG, FORMAT_ABBR[iWant_Aud_Format]); // szBuffer);

  /*
  switch (iAudio_SEL_Track)
  {
    case TRACK_AUTO :
      strcpy(szBuffer, "A") ;
      break ;
    case TRACK_NONE :
      strcpy(szBuffer, "N") ; //*(short*)(&szBuffer[0]) = (short)("N") ;  
      break ;
    default :
      iTmp1 = iAudio_SEL_Track + 1 ;
      sprintf(szBuffer, "%d", iTmp1) ;
      break ;
  }

  SetDlgItemText(hStats, IDC_TRK_SEL_NUM, szBuffer);
  */

  
  if (iAudio_SEL_Track != TRACK_NONE)                    //RJTRK MEMO
  {
    S370_AudioTrackDesc(iAudio_SEL_Format, iAudio_SEL_Track,  
                                          (iAudio_SEL_Track +1));
  }
  else 
    strcpy(szBuffer,  "OFF");

  SetDlgItemText(hStats, IDC_AUDIO_TYPE, szBuffer );

//  sprintf(szBuffer,"s%d", process.iAudio_SelStatus);
//  SetDlgItemText(hStats, IDC_AUDIO_PS2, szBuffer);

  S333_Trk_Audio_Desc(hStats, &IDC_AUDIO_FIELD[0]);
}
 



// --------------------------------------------
void Stats_Kill()
{
  if (hStats)
      DestroyWindow(hStats);
  hStats = NULL;
  MParse.ShowStats_Flag = false;
  ZeroMemory(&StatsPrev, sizeof(StatsPrev));
}


void Stats_Show(int refresh, int P_Visibility)
{
  int iTmp1;

  if (MParse.ShowStats_Flag)
  {
      GetWindowRect(hStats, &srect);
      if (refresh)
      {
        DestroyWindow(hStats); 
        ZeroMemory(&StatsPrev, sizeof(StatsPrev));
      }
  }

  memset(&OldStats, 0xCCCCCCCC, sizeof(OldStats));
  if (refresh  ||  ! MParse.ShowStats_Flag)
  {
      hStats = CreateDialog(hInst, (LPCTSTR)IDD_STATISTICS, hWnd_MAIN,
                                   (DLGPROC)Statistics);
      if ( ! MParse.ShowStats_Flag)
      {
         GetWindowRect(hStats, &srect);
         GetWindowRect(hWnd_MAIN, &wrect);
         srect.right = wrect.right+srect.right-srect.left;
         srect.left  = wrect.right;
         srect.bottom = wrect.top+srect.bottom-srect.top;
         srect.top    = wrect.top+12;

         // The person really wants to read the panel
         if (P_Visibility > 0)
         {
            // trap stats panel too far right
            iTmp1 = srect.right - VGA_Width - 5;
            if (iTmp1 > 0)
            {
                srect.right = VGA_Width;
                srect.left  = srect.left - iTmp1;
            }
            // trap stats panel too far down
            iTmp1 = srect.bottom - VGA_Height + 20;
            if (iTmp1 > 0)
            {
                srect.bottom = VGA_Height;
                srect.top    = srect.top - iTmp1;
            }
         }
      }

      // trap stats panel title bar off screen
      if (srect.top < -10)
      {
         srect.bottom -= srect.top;
         srect.top    = -10;
      }

      // trap stats panel title bar off screen
      iTmp1 = srect.left - VGA_Width + 12;
      if (iTmp1 > 0)
      {
         srect.left  -= iTmp1;
         srect.right -= iTmp1;
      }

      MoveWindow(hStats, srect.left, srect.top, //Edge_Height-Edge_Width/2,
                         srect.right-srect.left, srect.bottom-srect.top, true);

      MParse.ShowStats_Flag = true;
      S100_Stats_Hdr_Main(1);
      S120_Stats_Hdr_Seq();
      S200_Stats_Pic_Main(1);

      if (MParse.SlowPlay_Flag > 0)
      {
          sprintf(szMsgTxt, "SLOW-%d", MParse.SlowPlay_Flag);
          SetDlgItemText(hStats, IDC_MPAdec_NAME, szMsgTxt); // "SLOW-MO");
      }
      else
      if (MParse.FastPlay_Flag)
      {
          sprintf(szBuffer,"FAST-%d", MParse.FastPlay_Flag);
          SetDlgItemText(hStats, IDC_MPAdec_NAME, szBuffer);
      }
      else
      if (byAC3_Init) // && iWant_Aud_Format > FORMAT_MPA) // AC3, etc
          SetDlgItemText(hStats, IDC_MPAdec_NAME, "A52");
      else
          SetDlgItemText(hStats, IDC_MPAdec_NAME, szMPAdec_NAME);
  }

  MParse.ShowStats_Flag = true;

  ShowWindow(hStats, SW_SHOW);
  SetFocus(hWnd_MAIN);

}



void Stats_Volume_Boost()
{
  char cOvfl;

  if (  ( ( !strcmp(szAudio_Status, "Loaded") || szAudio_Status[0] <= ' ')
          && iCtl_Volume_Boost)
  ||  ! MParse.ShowStats_Flag)
  {
        if (iVolume_Boost  <= 0)
        {
            sprintf(szBuffer, "NO boost");  //szAudio_Status);
        }
        else
        {
           if (PlayCtl.iAudioFloatingOvfl)
               cOvfl = '<';
           else
               cOvfl = ' ';

           sprintf(szBuffer, "Boost%c%d:%d", cOvfl, 
                              (iVolume_Boost-K_BOOST_DENOM), 
                              (iVolume_BOLD-K_BOOST_DENOM));
        }
  }
  else
      sprintf(szBuffer, szAudio_Status); 

  szBuffer[31] = 0;
  if (strcmp(OldStats.szAudioBoost, szBuffer))
  {
      if (MParse.ShowStats_Flag)
          SetDlgItemText(hStats, STATS_MPAdec_STATUS, szBuffer);
      else
      {
          strcpy(szMsgTxt, szBuffer);
          DSP1_Main_MSG(0,0);
      }
      strcpy(OldStats.szAudioBoost, szBuffer);
  }

}




//-------------------------------------------



char *lsABBR, szChannels[4];
int iTrack, iFormat;
unsigned uMode, uBitRate, ukHz;



//-------------------------------------------------------------------------
void S370_AudioTrackDesc(int P_Track_Format, 
                         int P_Internal_Trk,
                         int P_External_Trk) 
{

  iTrack  = P_Internal_Trk;
  iFormat = P_Track_Format;

  if (iFormat >= 11)
      iFormat = 11;
  lsABBR = FORMAT_ABBR[iFormat];

  if (iTrack >= CHANNELS_MAX)
  {
     if (iTrack == TRACK_AUTO)
         strcpy(szTmp32, "Auto");
     else
     if (iTrack == TRACK_NONE)
         strcpy(szTmp32, "Off");
     else
         sprintf(szTmp32, "#%d", iTrack);
  }
  else
  if (iFormat == 0)    // unused
      szTmp32[0] = 0;
  else
  if (iFormat == FORMAT_MPA)    //  MPEG Audio
      strcpy(szTmp32, mpa_Ctl[iTrack].desc);
  else
  if (iFormat == FORMAT_LPCM)
  {
      uMode = SubStream_CTL[FORMAT_LPCM][iTrack].mode;
      if (uMode == 0)  strcpy(szChannels, "M"); 
      else
      if (uMode == 1)  strcpy(szChannels, "S"); 
      else
         sprintf(szChannels, "%d", uMode); 

      uBitRate = SubStream_CTL[FORMAT_LPCM][iTrack].uBitRate;
      ukHz     = SubStream_CTL[FORMAT_LPCM][iTrack].uSampleRate / 1000;

      sprintf(szTmp32, "%s %s %d %dkHz",  
          lsABBR, szChannels, uBitRate, ukHz);
  }
  else
  if (iFormat >= FORMAT_AC3 && iFormat <= FORMAT_DDPLUS)
  {
      uMode = SubStream_CTL[iFormat][iTrack].mode, 
      uBitRate = SubStream_CTL[iFormat][iTrack].uBitRate;

      sprintf(szTmp32, "%s %s %dk",   lsABBR, 
                                     AC3Mode[uMode], 
                                     AC3Rate[uBitRate]);
  }
  else
      sprintf(szTmp32, "? %X %02X", P_Track_Format, P_Internal_Trk) ;

  sprintf(szBuffer, "%d. %s", P_External_Trk, szTmp32);
}



//-------------------------------------------------------------
LRESULT CALLBACK Statistics(HWND hStatisticsDlg, UINT message,
                                          WPARAM wParam, LPARAM lParam)
{
  //int iMax;
  //if (Coded_Pic_Width < 769)
  //    iMax = 10000;
  //else
  //    iMax = 40000;

   switch (message)
   {
      case WM_INITDIALOG:
         SendDlgItemMessage(hStatisticsDlg, IDC_BITRATE_BAR, PBM_SETRANGE, 0,
                                                   MAKELPARAM(0, 200)); //iMax));
         return true;

      case WM_COMMAND:
         if (LOWORD(wParam) == IDCANCEL)
         {
            DestroyWindow(hStatisticsDlg);
            MParse.ShowStats_Flag = false;
            ZeroMemory(&StatsPrev, sizeof(StatsPrev));
            return true;
         }
   }
   return false;
}


