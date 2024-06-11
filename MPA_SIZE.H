
// Mpeg Audio - Calculate Frame Size
//              using fields previously extracted from header.
//              ZERO LENGTH => BAD

// VALIDATE SOME MORE
__forceinline static void MPA_FrameLen()
{
  if (uMPA_SampFreq_Ix > 2   // invalid MPA sampling frequncy code
  // ||  uMPA_Mpeg_Ver   == 3  // Invalid Mpeg Version ?
     )
  {
      iMPA_FrameLen = 0;  
  }
  else

  // Layer II - restricted combinations of bitrate and mode
  if (uMPA_Layer == 2
  &&  uMPA_Channel_ix == 3 // Mono
  &&  uMPA_kBitRate > 192)
  {
      iMPA_FrameLen = 0;  // Not a valid MPA header 
  }
  else
  if (uMPA_Layer == 2
  &&  uMPA_Channel_ix != 3 // NOT Mono
  &&  (   uMPA_kBitRate == 32 || uMPA_kBitRate == 48
       || uMPA_kBitRate == 56 || uMPA_kBitRate == 80
      )
     )
  {
      iMPA_FrameLen = 0;  // Not a valid MPA header 
  }
  else
  {
      if (uMPA_Layer_Ix)
      {
          uSlotBytes = 1;
          uSlotMultiplier = 144;
          if (uMPA_Layer == 3)   // mp3
          {
            if (uMPA_Mpeg_Ver > 2)  // seen conflicting info on this rule.
                uMPA_Samples = 576;
            else
                uMPA_Samples = 1152;
          }
      }
      else
      { 
          uSlotBytes = 4;
          uMPA_Samples = 384;
          uSlotMultiplier = 12;
      }

      iMPA_FrameLen  = uSlotMultiplier 
                     * uMPA_kBitRate * 1000
                     / MPA_SAMPLE_HZ[uMPA_25_LSF][uMPA_SampFreq_Ix];  //  uFrequency[Freq_ix]
      iMPA_FrameLen  = (iMPA_FrameLen  + uMPA_Padding) * uSlotBytes;
  }
}



/*
frame - Layer I   - 384 samples
      - Layer II  - 1152 samples. 
	    - Layer III - mpeg1=1152 samples mpeg2=576 mpeg2.5=576
   
  uSlotBytes = MPA_SLOT_BYTES[uMPA_Layer_Ix];
  iMPA_FrameLen  = uMPA_kBitRate * 1000 
                 * uSlotBytes
                 / uMPA_Sample_Hz;
  if (uMPA_Padding)
     iMPA_FrameLen += uSlotBytes;

frame consists of an integer number of slots 
 (four bytes in Layer I, one byte in Layer II or III).

The position of consecutive syncwords can usually be calculated 
from the information provided by the seven bits just after the syncword :
 the bitstream is subdivided in slots. 
 The distance between the start of two consecutive syncwords is constant
 and equals "N" slots. 

The value of "N" depends on the Layer. 
For Layer I the following equation is valid: 
	N = 12  * bit_rate / sampling_frequency.
For Layers II and III the equation becomes: 
	N = 144 * bit_rate / sampling_frequency.

If this calculation does not give an integer number 
the result is truncated and 'padding' is required. 
In this case the number of slots in a frame will vary between N and N+1. 
The padding bit is set to '0' if the number of slots equals N, 
and to '1' otherwise. 

If nominal bitrate is zero/variable then cannot calculate frame length.

This knowledge of the position of consecutive syncwords 
greatly facilitates synchronization.

*/
