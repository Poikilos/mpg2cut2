
// Mpeg Audio Header 

// Work Fields
/*
  unsigned MPA_SLOT_BYTES [4]
#ifdef GLOBAL
    = {4, 1, 1, 1} // (Mp1 slots=4 bytes others = 1byte)
#endif
;

  unsigned MPA_SLOT_MULTIPLIER [4]
#ifdef GLOBAL
    = {12, 144, 144, 144} // (Mp1 slots=4 bytes others = 1byte)
#endif
;
*/

unsigned MPA_SAMPLE_HZ[4][4]
#ifdef GLOBAL
    = {
    {44100, 48000, 32000, 0}, // Original Std
    {22050, 24000, 16000, 0}, // Mpeg 2.5
    {22050, 24000, 16000, 0}, // LSF Extension
    {11025, 12000,  8000, 0}  // Mpeg 2.5 + LSF
} 
#endif
;
//#ifdef GLOBAL
//    = {
//    {11025, 12000,  8000, 48000},  // , 0},
//    {44100, 48000, 32000, 48000},  // , 0},
//    {22050, 24000, 16000, 48000},
//    {    0,     0,     0,     0}} // , 0},
//#endif
//;

unsigned MPA_KBIT_RATE[2][3][16]
#ifdef GLOBAL
    ={
/* MPEG-1 */
  /* Layer I   */
  {{0,32,64,96,  128,160,192,224,    256,288,320,352,   384,416,448,666},
  /* Layer II  */
   {0,32,48,56,   64, 80, 96,112,    128,160,192,224,   256,320,384,666},
  /* Layer III */
   {0,32,40,48,   56, 64, 80, 96,    112,128,160,192,   224,256,320,666}},
/* MPEG-2 LSF - MPEG-2 Low Sampling Frequencies Extension */
  /* Layer I   */
  {{ 0,32,48,56,  64, 80, 96,112,    128,144,160,176,   192,224,256,666},
  /* II & III  */
   { 0, 8,16,24,  32, 40, 48, 56,     64, 80, 96,112,   128,144,160,666},
   { 0, 8,16,24,  32, 40, 48, 56,     64, 80, 96,112,   128,144,160,666}}
}
#endif
;



char MPA_EXTN_ABBR[4][4]  // L=LSF=LowSamplingFreq  .5= Mpeg 2.5 unofficial extension
#ifdef GLOBAL
    = {"", "L", ".5", ".5L"}
#endif
;
 

char MPG_CH_MODE_ABBR[4][4]
#ifdef GLOBAL
    = {"S", "js", "x2", "M"} 
#endif
;
  unsigned uMPA_Mpeg_Ver;   // 1=Mpeg1  2=Mpeg2 3=RESERVED 4=mpeg2.5
  unsigned uMPA_25_LSF, uMPA_Mpeg25, uMPA_LSF;  // Flags -  Mpeg2.5  and LowSamplingFreqExtension
  unsigned uMPA_SampFreq_Ix, uMPA_Channel_ix;
  unsigned uMPA_Padding;
  unsigned uMPA_kBitRate_Ix;
  unsigned uMPA_kBitRate;
  unsigned uMPA_Layer, uMPA_Layer_Ix; //,uMPA_Layer_Code;

  unsigned uMPA_Samples, uSlotBytes, uSlotMultiplier, uFrame_ms; 
  int      iMPA_FrameLen;
  int      iMPA_FrameOK;

  DWORD uMPA_Sample_Hz, uMPA_Channels;

/*
  // 2.4.2
                      |    #Samples/frame   |
  LAYER  | Dependency | mpeg1 mpeg2 mpeg2.5 | Slot-size | Multiply 
  -------+------------+---------------------+-----------+--------
    mp1  ! Intracoded |   384   =      =     |  4 bytes  |   12
    mp2  ! Intracoded |  1152   =      =     |  1 byte   |  144
    mp3  ! Back Refs  |  1152  576    576    |  1 byte   |  144

  iMPA_FrameLen  = ( (int( uSlotMultiplier
                           * uMPA_kBitRate * 1000
                           / MPA_SAMPLE_HZ[uMPA_25_LSF][Freq_ix]  //  uFrequency[Freq_ix]
                         )
                       + uMPA_Padding
                     )
                     * uSlotBytes
                   );
  
  iMPA_FrameLen  = ( (int(uSlotMultiplier 
                          * uMPA_kBitRate * 1000 
                          / uMPA_Sample_Hz
                         )
                       + uMPA_Padding
                      )
                      * uSlotBytes
                   );


  EXAMPLES:    mp2 48000kHz  Frame=784 (or 785 if padded)
                             Time=24ms (48000/1152)




*/
