
#include "global.h"


//--------------------------------------------
// Convert display format number to binary integer
//                      NON-NUMERIC returns  -9999
int iDeEdit(char *P_Src, int P_Len)
{
  int iVal, iNext;

  iVal = 0;
  
  while (P_Len)
  {
    iNext = *P_Src++ - '0';
    if (iNext < 0 || iNext > 9)
    {
      return iVal;
    }
    iVal = (iVal * 10) + iNext;
  }

  return iVal;
}





//--------------------------------------------
// Point to delimiting slash between  path and file
// or colon if no slash found
char *lpDOT(char *P_szPathFile)
{
  char *w_lpDOT;

   w_lpDOT = strrchr(P_szPathFile, '.');
   if ( ! w_lpDOT)
   {
       w_lpDOT = P_szPathFile + strlen(P_szPathFile);
   }

   return w_lpDOT;

}



//--------------------------------------------
// Point to delimiting slash between  path and file
// or colon if no slash found
// Returns ZERO if no slash AND no Colon found
char *lpLastSlash(char *P_szPathFile)
{
  char *w_lpSlash;

   w_lpSlash = strrchr(P_szPathFile, '\\');
   if ( ! w_lpSlash)
   {
       w_lpSlash = strrchr(P_szPathFile, ':');
       //if ( ! w_lpSlash)
       //{
       //       w_lpSlash = &P_szPathFile[0]-1;
       //}
   }

   return w_lpSlash;

}




//--------------------------------------------
// Tidy up the file name
void FileNameTidy(char *P_Dest, char *P_Src)
{
  char cTmp;
  char *lpEndPath;
  int iLower;

  iLower = 0;
  lpEndPath = lpLastSlash(P_Src);
  if (!lpEndPath)
       lpEndPath = P_Src;
  
  cTmp = 'x';
  while (cTmp)
  {
    cTmp = *P_Src++;
    if (P_Src > lpEndPath)
    {
       if (cTmp >= 'A' && cTmp <= 'Z')
       {
          if (iLower && iCtl_Out_MixedCase)
              cTmp  = (char)(cTmp | 0x20); // Convert to lower-case
          else
              iLower = 1;
       }
       else
       if (cTmp >= 'a' && cTmp <= 'z') 
       {
          if (! iLower && iCtl_Out_MixedCase)
          {
              cTmp  = (char)(cTmp & 0xBF);  // Convert to UPPER-case
              iLower = 1;
          }
       }
       else
       {
           iLower = 0;
           if (cTmp == ' ' && iCtl_Out_DeBlank)
           {
               cTmp  = '_';
           }
       }
    }  

    *P_Dest++ = cTmp;
  }

  return;
}





//--------------------------------------------

// Mimic a Borland Function 
// String Copy returning pointer to terminating null
char *stpcpy0 (char *P_Dest, char *P_Src)
{
  register char cTmp1;
next:
    cTmp1 = *P_Src++;
    *P_Dest = cTmp1;
    if (cTmp1)
    {
      P_Dest++;
      goto next;
    }

  return P_Dest;
}



//--------------------------------------------

// Mimic a Borland Function 
// String Copy returning pointer to end
// adjusted to point PAST the terminating NULL !
char *stpcpy1 (char *P_Dest, char *P_Src)
{
  char cTmp1;
  cTmp1 = 'X';
  while (cTmp1)
  {
    cTmp1 = *P_Src++;
    *P_Dest++ = cTmp1;
  }
  return P_Dest;
}



// String Word Copy returning pointer after terminating input delimiter 
// Multiple Words can be combined by enclosing in quotes
// Otherwise Space or Equals Sign will delimit a word
unsigned char *stpToken (unsigned char *P_Dest, 
                         unsigned char *P_Src, 
                                    int P_Numeric)
{
  unsigned char cTmp1, cDelim1, cDelim2, cLow, cHigh;  

  if (P_Numeric)
  {
    cLow  = '0';
    cHigh = '9';
  }
  else
  {
    cLow  = 1;
    cHigh = 255;
  }
  
  cTmp1 = *P_Src;

  // Do not go past end of string
  if (cTmp1)
  {
    // Skip leading spaces
    while (cTmp1 == ' ')
    {
       cTmp1 = *(++P_Src);
    }

    // Choose a delimiter
    if (cTmp1 == '"' || cTmp1 == 0x27) // Double or single quotes
    {
       cDelim1 = cTmp1;
       cDelim2 = 0;
       cTmp1 = *++P_Src;
    }
    else 
    {
       cDelim1 = ' ';  // Normal end of word
       cDelim2 = '=';  // End of KeyWord
    }

    // Copy up to delimiter or end
    while (cTmp1 && cTmp1 != cDelim1 && cTmp1 != cDelim2
                 && cTmp1 >= cLow    && cTmp1 <= cHigh)
    {
       *P_Dest++ = cTmp1;
       cTmp1 = *++P_Src;
    }
  }


  cTK_Delim = cTmp1;

  // Skip the delimiter, except for end of string
  if (cTK_Delim)
  {
       P_Src++;
  }

  // Terminate the output string
  *P_Dest = 0;

  return P_Src;
}

