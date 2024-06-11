
// GetBit Modules needed inline for speed 

__forceinline static unsigned int Show_Bits(unsigned int N);
__forceinline static void InputBuffer_Flush(unsigned int N);
__forceinline static unsigned int Get_Byte(void);
__forceinline static unsigned int Get_Short(void);
__forceinline static unsigned int GetB_Show_Next_Start_Code(int);


//--------------------------------------------
static unsigned int Show_Bits(unsigned int N)
{
  if (N <= BitsLeft)
    return (CurrentBfr << (32 - BitsLeft)) >> (32 - N);
  else
  {
    N -= BitsLeft;
    return (
            ( ( CurrentBfr << (32 - BitsLeft)
              )            >> (32 - BitsLeft)
            ) << N
           )
         + (NextBfr >> (32 - N)
           );
  }
}




//-----------------------------------------------------------------------

__forceinline static unsigned int Get_Bits(unsigned int N)
{
  register unsigned int uValue, N2;

  uValue = CurrentBfr << (32 - BitsLeft);
  if (N < BitsLeft)
  {
      uValue = uValue >> (32 - N);
      BitsLeft -= N;
  }
  else
  {
     uValue = uValue >> (32 - BitsLeft);

     N2 = N - BitsLeft;
     if (N2)
        uValue = (uValue << N2) + (NextBfr >> (32 - N2));

     CurrentBfr = NextBfr;
     BitsLeft = 32 - N2;
     InputBuffer_NEXT_fill(0);

  }

  return uValue;
}





// Fiddle about clearing bit buffers, etc, etc, etc
// to search for an Mpeg start code
//----------------------------------------------
static unsigned int GetB_Show_Next_Start_Code(int P_Major)
{

unsigned int uCode, uFirstTime;
int iRC;

  uCode = SYSTEM_END_CODE; // Dummy default in case of EOF or other problem 

  if (MParse.SeqHdr_Found_Flag || MParse.SystemStream_Flag)
    uFirstTime = 0;
  else
    uFirstTime = 1;

  //  PERFORMANCE ??
  if (iCtl_Byte_Sync && P_Major 
      && !(CurrentBfr & 0xFFFFFE00) && !(CurrentBfr & 0x00FFFFFE))
  {
      BitsLeft = 0;
      InputBuffer_NEXT_fill(1);
/*
    int iSlowdown;
    while ( MParse.Fault_Flag < CRITICAL_ERROR_LEVEL &&  ! MParse.Stop_Flag)
    {

      if ( !(NextBfr & 0xFF00FF00))
      {
        iSlowdown = 1;
        if (  !(NextBfr & 0xFFFE0000) && !(CurrentBfr & 0x000000FF)
           || !(NextBfr & 0xFE000000) && !(CurrentBfr & 0x0000FFFF) )
        {
           break;
        }
        else
        if (  !(NextBfr & 0xFFFFFE00)
           || !(NextBfr & 0x00FFFFFE)
           || !(NextBfr & 0x0000FFFF) )
        {
           break;
        }
        else
        if ( (NextBfr & 0xFF00FF00) && (NextBfr & 0x00FF00FF))
           iSlowdown = 0;
      }

      CurrentBfr = NextBfr;
      BitsLeft = 32;
      InputBuffer_NEXT_fill(iSlowdown);

    } // ENDWHILE 
*/
    
  } // ENDIF Acceleration
  else
    InputBuffer_Flush(BitsLeft & 7);

//while (Show_Bits(24) != 1)
//      InputBuffer_Flush(8);

 while ( MParse.Fault_Flag < CRITICAL_ERROR_LEVEL &&  ! MParse.Stop_Flag)
 {
    uCode = Show_Bits(24);
    if (uCode == 1)
       break;

    if (uFirstTime)
    {
      if (     *(char*)(&uCode)    == 0x47 
           || *((char*)(&uCode)+2) == 0x47 ) // Mpeg-2 Transport Stream 
      {
         // Chg2RGB24(0); 
         if ( !MParse.SystemStream_Flag)
         {
            iRC = F594_TS_Warn_Msg();
            //if (iRC == IDOK)
            //    MParse.Stop_Flag = 1;
         }

        MParse.SystemStream_Flag = -1;
        PktChk_Audio = PKTCHK_AUDIO_TS; PktChk_Any = PKTCHK_ANY_TS;
        Mpeg_PES_Version = 2;  process.Mpeg2_Flag = 4;
        break;
      }
      else
      if (      *(short*)(&uCode)    == 0x4156) // 'AV' = PVA  Stream 
      {
         // Chg2RGB24(0);
         if ( !MParse.SystemStream_Flag)
         {
            iRC = F591_Ask_Trojan(1, &"PVA Stream", 
                                     &iCtl_WarnTS, IDM_WARN_FMT_TS);
            //if (iRC == IDOK)
            //    MParse.Stop_Flag = 1;
         }

        MParse.SystemStream_Flag = -2;
        PktChk_Audio = PKTCHK_AUDIO_TS; PktChk_Any = PKTCHK_ANY_TS;
        Mpeg_PES_Version = 2;   process.Mpeg2_Flag = 4;
        break;
      }
    }

    if (uCode)
      uFirstTime = 0;

    InputBuffer_Flush(8);
 }

 return uCode;

}

//-------------------------------------
/*
void InputBuffer_FLUSH_ALL(unsigned int N)
{
  CurrentBfr = NextBfr;
  BitsLeft = BitsLeft + 32 - N;
  InputBuffer_NEXT_fill(0);
}
*/



//----------------------------------------------

static void InputBuffer_Flush(unsigned int N)
{
  if (N < BitsLeft)
    BitsLeft -= N;
  else
  {
    // InputBuffer_FLUSH_ALL(N);
    CurrentBfr = NextBfr;
    BitsLeft = BitsLeft + 32 - N;
    InputBuffer_NEXT_fill(0);
  }
}


//------------------------------------------
static unsigned int Get_Byte()
{
  // Have we reached the end of the current buffer ?
  while (RdPTR >= RdEOB)   //RdBFR+MPEG_SEARCH_BUFSZ)
  {
    Mpeg_READ_Adjust();
  }


  if (RdPTR < RdBFR) // LOWER bounds bug check - realign to start of buffer
  {
      Mpeg_READ_Bug_Adj();
  }

  return *RdPTR++;
}




//------------------------------------------------
static unsigned int Get_Short()
{
  unsigned int i = Get_Byte();
  return (i<<8) | Get_Byte();
}
