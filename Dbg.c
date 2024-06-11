
#include "global.h"
#define true 1
#define false 0

//----------------------------------------------------------

unsigned uPrev_DBGTime_ms, uNew_DBGTime_ms, uDiff_DBGTime_ms;

// Output a string to the Debug trace file
int DBGout(char P_Rec[256])
{
	int iRC;
	char szTemp1[256], szTime[16];

  // Has their been a significant delay between trace entries ?

  uNew_DBGTime_ms  = timeGetTime();
  uDiff_DBGTime_ms = uNew_DBGTime_ms - uPrev_DBGTime_ms;

  if (uDiff_DBGTime_ms > 100)
  {
      sprintf(szTime, "\n+%ums\n", uDiff_DBGTime_ms);
      fputs(szTime, DBGfile);
  }
  else
    szTime[0] = 0;

  uPrev_DBGTime_ms = uNew_DBGTime_ms;

  // DbgView

  if (bDBGStr) OutputDebugString(P_Rec);    // For DbgView

  // Output a string to the Debug trace file

  iRC = fputs(P_Rec, DBGfile);
	if (iRC == EOF)
	{	
		strcpy(szTemp1, "DBG PUT FAILED: ") ;
		strcat(szTemp1, P_Rec) ;
		strcpy(szAppName, szTemp1) ;
	}
  else
    fputs("\n", DBGfile);


	return iRC;
}


//-------------------------
void DBGln2(char P_fmt[256], __int64 P_big1, __int64 P_big2)

{
	int rc ;
  int iTmp1, iTmp2;


	if (DBGflag)
	{
		DBGctr++;
			iTmp1 = (int)P_big1 ;
			iTmp2 = (int)P_big2 ;

			strcpy(szDBGln, P_fmt);
			rc = sprintf(szDBGln, P_fmt, iTmp1, iTmp2);
			if (rc == EOF)
			{
				strcpy(szMsgTxt, "DBG FMT PROB: ") ;
				strcat(szMsgTxt, P_fmt) ;
				strcat(szMsgTxt, "---") ;
  			rc = DBGout(szMsgTxt);
			}
			rc = DBGout(szDBGln);
			fflush(DBGfile);
	}

}



//-------------------------
void DBGln4(char P_fmt[256], __int64 P_big1, __int64 P_big2,
														 __int64 P_big3, __int64 P_big4)

{
	int rc ;
  int iTmp1, iTmp2, iTmp3, iTmp4;

	if (DBGflag)
	{
		DBGctr++;

			iTmp1 = (int)(P_big1) ;
			iTmp2 = (int)(P_big2) ;
			iTmp3 = (int)(P_big3) ;
			iTmp4 = (int)(P_big4) ;

			strcpy(szDBGln, P_fmt);
			rc = sprintf(szDBGln, P_fmt, iTmp1, iTmp2, iTmp3, iTmp4);
			if (rc == EOF)
			{
					strcpy(szMsgTxt, "DBG FAILED: ") ;
					strcat(szMsgTxt, P_fmt) ;
			}
			rc = DBGout(szDBGln);
	}

}



//-------------------------
void DBGln4a(char P_fmt[256], void *P_big1, void *P_big2,
														  void *P_big3, void *P_big4)

{
	int rc ;
  //int iTmp1, iTmp2, iTmp3, iTmp4;

	if (DBGflag)
	{
		  DBGctr++;

			strcpy(szDBGln, P_fmt);
			rc = sprintf(szDBGln, P_fmt, P_big1, P_big2, P_big3, P_big4);
			if (rc == EOF)
			{
					strcpy(szMsgTxt, "DBG FAILED: ") ;
					strcat(szMsgTxt, P_fmt) ;
			}
			rc = DBGout(szDBGln);
	}

}



//------------------------------
void DBGctl()
{
  uPrev_DBGTime_ms = timeGetTime();
	if (DBGflag)
	{
		DBGflag = false;
    Sleep(10);
		fclose(DBGfile) ;
		CheckMenuItem(hMenu, IDM_DBG, MF_UNCHECKED);
	}
	else
	{
    iDBGsuffix++;
		sprintf(szDBGln, "C:\\TEMP\\Mpg2Cut2-%03d.DBG", iDBGsuffix);
		DBGfile = fopen(szDBGln, "wt") ;
		if (!DBGfile)
		{
				ERRMSG_File("DBGfile", 'o', 0, szDBGln, 0, 9930) ;
				CheckMenuItem(hMenu, IDM_DBG, MF_UNCHECKED);
		    DBGflag = false;
		}
		else
		{
				CheckMenuItem(hMenu, IDM_DBG, MF_CHECKED);
				DBGflag = true;
		}
    if (!iBusy)
        DSP2_Main_SEL_INFO(0); 
	}
}


void DBGAud(void *P_Text)
{
  TextOut(hDC, 0, iMsgPosY, (char*)P_Text, 4);
  UpdateWindow(hWnd_MAIN);
  Sleep(10);
}