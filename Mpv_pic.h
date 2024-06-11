

int  ChromaFormat[4]
#ifdef GLOBAL
=  { 6,  6,  8,  12}
#endif
;

int slice_header(void);
int quantizer_scale;

short *block[16], *p_block[16];   //RJ WAS [8] Expanded for 4:2:2 and maybe 4:4:4
void *fTempArray, *p_fTempArray;


/* default intra quantization matrix */
unsigned char default_intra_quantizer_matrix[64]
#ifdef GLOBAL
=
{
  8, 16, 19, 22, 26, 27, 29, 34,
  16, 16, 22, 24, 27, 29, 34, 37,
  19, 22, 26, 27, 29, 34, 34, 38,
  22, 22, 26, 27, 29, 34, 37, 40,
  22, 26, 27, 29, 32, 35, 40, 48,
  26, 27, 29, 32, 35, 40, 48, 58,
  26, 27, 29, 34, 38, 46, 56, 69,
  27, 29, 35, 38, 46, 56, 69, 83
}
#endif
;

/* zig-zag and alternate scan patterns */
unsigned char scan[2][64]
#ifdef GLOBAL
=
{
  { // Zig-Zag scan pattern
    0,  1,  8, 16,  9,  2,  3, 10,
     17, 24, 32, 25, 18, 11,  4,  5,
     12, 19, 26, 33, 40, 48, 41, 34,
     27, 20, 13,  6,  7, 14, 21, 28,
     35, 42, 49, 56, 57, 50, 43, 36,
     29, 22, 15, 23, 30, 37, 44, 51,
     58, 59, 52, 45, 38, 31, 39, 46,
     53, 60, 61, 54, 47, 55, 62, 63
  }
  ,
  { // Alternate scan pattern
    0,  8, 16, 24,  1,  9,  2, 10,
     17, 25, 32, 40, 48, 56, 57, 49,
     41, 33, 26, 18,  3, 11, 4,  12,
     19, 27, 34, 42, 50, 58, 35, 43,
     51, 59, 20, 28,  5, 13,  6, 14,
     21, 29, 36, 44, 52, 60, 37, 45,
     53, 61, 22, 30,  7, 15, 23, 31,
     38, 46, 54, 62, 39, 47, 55, 63
  }
}
#endif
;

/* non-linear quantization coefficient table */
unsigned char Non_Linear_quantizer_scale[32]
#ifdef GLOBAL
=
{
  0, 1, 2, 3, 4, 5, 6, 7,
  8, 10, 12, 14, 16, 18, 20, 22,
  24, 28, 32, 36, 40, 44, 48, 52,
  56, 64, 72, 80, 88, 96, 104, 112
}
#endif
;

#define ERROR_VALUE (-1)

typedef struct {
  char run, level, len;
} DCTtab;

typedef struct {
  char val, len;
} VLCtab;

/* Table B-10, motion_code, codes 0001 ... 01xx */
VLCtab MVtab0[8]
#ifdef GLOBAL
=
{
  {ERROR_VALUE,0}, {3,3}, {2,2}, {2,2}, {1,1}, {1,1}, {1,1}, {1,1}
}
#endif
;

/* Table B-10, motion_code, codes 0000011 ... 000011x */
VLCtab MVtab1[8]
#ifdef GLOBAL
=
{
  {ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0}, {7,6}, {6,6}, {5,6}, {4,5}, {4,5}
}
#endif
;

/* Table B-10, motion_code, codes 0000001100 ... 000001011x */
VLCtab MVtab2[12]
#ifdef GLOBAL
=
{
  {16,9}, {15,9}, {14,9}, {13,9},
  {12,9}, {11,9}, {10,8}, {10,8},
  {9,8},  {9,8},  {8,8},  {8,8}
}
#endif
;

/* Table B-9, coded_block_pattern, codes 01000 ... 111xx */
VLCtab CBPtab0[32]
#ifdef GLOBAL
=
{
  {ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0},
  {ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0},
  {62,5}, {2,5},  {61,5}, {1,5},  {56,5}, {52,5}, {44,5}, {28,5},
  {40,5}, {20,5}, {48,5}, {12,5}, {32,4}, {32,4}, {16,4}, {16,4},
  {8,4},  {8,4},  {4,4},  {4,4},  {60,3}, {60,3}, {60,3}, {60,3}
}
#endif
;

/* Table B-9, coded_block_pattern, codes 00000100 ... 001111xx */
VLCtab CBPtab1[64]
#ifdef GLOBAL
=
{
  {ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0}, {ERROR_VALUE,0},
  {58,8}, {54,8}, {46,8}, {30,8},
  {57,8}, {53,8}, {45,8}, {29,8}, {38,8}, {26,8}, {37,8}, {25,8},
  {43,8}, {23,8}, {51,8}, {15,8}, {42,8}, {22,8}, {50,8}, {14,8},
  {41,8}, {21,8}, {49,8}, {13,8}, {35,8}, {19,8}, {11,8}, {7,8},
  {34,7}, {34,7}, {18,7}, {18,7}, {10,7}, {10,7}, {6,7},  {6,7},
  {33,7}, {33,7}, {17,7}, {17,7}, {9,7},  {9,7},  {5,7},  {5,7},
  {63,6}, {63,6}, {63,6}, {63,6}, {3,6},  {3,6},  {3,6},  {3,6},
  {36,6}, {36,6}, {36,6}, {36,6}, {24,6}, {24,6}, {24,6}, {24,6}
}
#endif
;

/* Table B-9, coded_block_pattern, codes 000000001 ... 000000111 */
VLCtab CBPtab2[8]
#ifdef GLOBAL
=
{
  {ERROR_VALUE,0}, {0,9}, {39,9}, {27,9}, {59,9}, {55,9}, {47,9}, {31,9}
}
#endif
;

/* Table B-1, macroblock_address_increment, codes 00010 ... 011xx */
VLCtab MBAtab1[16]
#ifdef GLOBAL
=
{
  {ERROR_VALUE,0}, {ERROR_VALUE,0}, {7,5}, {6,5}, {5,4}, {5,4}, {4,4},
  {4,4}, {3,3}, {3,3}, {3,3}, {3,3}, {2,3}, {2,3}, {2,3}, {2,3}
}
#endif
;

/* Table B-1, macroblock_address_increment, codes 00000011000 ... 0000111xxxx */
VLCtab MBAtab2[104]
#ifdef GLOBAL
=
{
  {33,11}, {32,11}, {31,11}, {30,11}, {29,11}, {28,11}, {27,11}, {26,11},
  {25,11}, {24,11}, {23,11}, {22,11}, {21,10}, {21,10}, {20,10}, {20,10},
  {19,10}, {19,10}, {18,10}, {18,10}, {17,10}, {17,10}, {16,10}, {16,10},
  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},  {15,8},
  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},  {14,8},
  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},  {13,8},
  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},  {12,8},
  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},  {11,8},
  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},  {10,8},
  {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},
  {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},   {9,7},
  {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},
  {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7},   {8,7}
}
#endif
;

/* Table B-12, dct_dc_size_luminance, codes 00xxx ... 11110 */
VLCtab DClumtab0[32]
#ifdef GLOBAL
=
{
  {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
  {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
  {0, 3}, {0, 3}, {0, 3}, {0, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3},
  {4, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {6, 5}, {ERROR_VALUE, 0}
}
#endif
;

/* Table B-12, dct_dc_size_luminance, codes 111110xxx ... 111111111 */
VLCtab DClumtab1[16]
#ifdef GLOBAL
=
{
  {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6},
  {8, 7}, {8, 7}, {8, 7}, {8, 7}, {9, 8}, {9, 8}, {10,9}, {11,9}
}
#endif
;

/* Table B-13, dct_dc_size_chrominance, codes 00xxx ... 11110 */
VLCtab DCchromtab0[32]
#ifdef GLOBAL
=
{
  {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2},
  {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
  {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
  {3, 3}, {3, 3}, {3, 3}, {3, 3}, {4, 4}, {4, 4}, {5, 5}, {ERROR_VALUE, 0}
}
#endif
;

/* Table B-13, dct_dc_size_chrominance, codes 111110xxxx ... 1111111111 */
VLCtab DCchromtab1[32]
#ifdef GLOBAL
=
{
  {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
  {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
  {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7},
  {8, 8}, {8, 8}, {8, 8}, {8, 8}, {9, 9}, {9, 9}, {10,10}, {11,10}
}
#endif
;

/* Table B-14, DCT coefficients table zero,
 * codes 0100 ... 1xxx (used for first (DC) coefficient)
 */
DCTtab DCTtabfirst[12]
#ifdef GLOBAL
=
{
  {0,2,4}, {2,1,4}, {1,1,3}, {1,1,3},
  {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1},
  {0,1,1}, {0,1,1}, {0,1,1}, {0,1,1}
}
#endif
;

/* Table B-14, DCT coefficients table zero,
 * codes 0100 ... 1xxx (used for all other coefficients)
 */
DCTtab DCTtabnext[12]
#ifdef GLOBAL
=
{
  {0,2,4},  {2,1,4},  {1,1,3},  {1,1,3},
  {64,0,2}, {64,0,2}, {64,0,2}, {64,0,2}, /* EOB */
  {0,1,2},  {0,1,2},  {0,1,2},  {0,1,2}
}
#endif
;

/* Table B-14, DCT coefficients table zero,
 * codes 000001xx ... 00111xxx
 */
DCTtab DCTtab0[60]
#ifdef GLOBAL
=
{
  {65,0,6}, {65,0,6}, {65,0,6}, {65,0,6}, /* Escape */
  {2,2,7}, {2,2,7}, {9,1,7}, {9,1,7},
  {0,4,7}, {0,4,7}, {8,1,7}, {8,1,7},
  {7,1,6}, {7,1,6}, {7,1,6}, {7,1,6},
  {6,1,6}, {6,1,6}, {6,1,6}, {6,1,6},
  {1,2,6}, {1,2,6}, {1,2,6}, {1,2,6},
  {5,1,6}, {5,1,6}, {5,1,6}, {5,1,6},
  {13,1,8}, {0,6,8}, {12,1,8}, {11,1,8},
  {3,2,8}, {1,3,8}, {0,5,8}, {10,1,8},
  {0,3,5}, {0,3,5}, {0,3,5}, {0,3,5},
  {0,3,5}, {0,3,5}, {0,3,5}, {0,3,5},
  {4,1,5}, {4,1,5}, {4,1,5}, {4,1,5},
  {4,1,5}, {4,1,5}, {4,1,5}, {4,1,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5}
}
#endif
;

/* Table B-15, DCT coefficients table one,
 * codes 000001xx ... 11111111
*/
DCTtab DCTtab0a[252]
#ifdef GLOBAL
=
{
  {65,0,6}, {65,0,6}, {65,0,6}, {65,0,6}, /* Escape */
  {7,1,7}, {7,1,7}, {8,1,7}, {8,1,7},
  {6,1,7}, {6,1,7}, {2,2,7}, {2,2,7},
  {0,7,6}, {0,7,6}, {0,7,6}, {0,7,6},
  {0,6,6}, {0,6,6}, {0,6,6}, {0,6,6},
  {4,1,6}, {4,1,6}, {4,1,6}, {4,1,6},
  {5,1,6}, {5,1,6}, {5,1,6}, {5,1,6},
  {1,5,8}, {11,1,8}, {0,11,8}, {0,10,8},
  {13,1,8}, {12,1,8}, {3,2,8}, {1,4,8},
  {2,1,5}, {2,1,5}, {2,1,5}, {2,1,5},
  {2,1,5}, {2,1,5}, {2,1,5}, {2,1,5},
  {1,2,5}, {1,2,5}, {1,2,5}, {1,2,5},
  {1,2,5}, {1,2,5}, {1,2,5}, {1,2,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
  {3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {1,1,3}, {1,1,3}, {1,1,3}, {1,1,3},
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4}, /* EOB */
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
  {64,0,4}, {64,0,4}, {64,0,4}, {64,0,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,3,4}, {0,3,4}, {0,3,4}, {0,3,4},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,1,2}, {0,1,2}, {0,1,2}, {0,1,2},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,2,3}, {0,2,3}, {0,2,3}, {0,2,3},
  {0,4,5}, {0,4,5}, {0,4,5}, {0,4,5},
  {0,4,5}, {0,4,5}, {0,4,5}, {0,4,5},
  {0,5,5}, {0,5,5}, {0,5,5}, {0,5,5},
  {0,5,5}, {0,5,5}, {0,5,5}, {0,5,5},
  {9,1,7}, {9,1,7}, {1,3,7}, {1,3,7},
  {10,1,7}, {10,1,7}, {0,8,7}, {0,8,7},
  {0,9,7}, {0,9,7}, {0,12,8}, {0,13,8},
  {2,3,8}, {4,2,8}, {0,14,8}, {0,15,8}
}
#endif
;

/* Table B-14, DCT coefficients table zero,
 * codes 0000001000 ... 0000001111
 */
DCTtab DCTtab1[8]
#ifdef GLOBAL
=
{
  {16,1,10}, {5,2,10}, {0,7,10}, {2,3,10},
  {1,4,10}, {15,1,10}, {14,1,10}, {4,2,10}
}
#endif
;

/* Table B-15, DCT coefficients table one,
 * codes 000000100x ... 000000111x
 */
DCTtab DCTtab1a[8]
#ifdef GLOBAL
=
{
  {5,2,9}, {5,2,9}, {14,1,9}, {14,1,9},
  {2,4,10}, {16,1,10}, {15,1,9}, {15,1,9}
}
#endif
;

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 000000010000 ... 000000011111
 */
DCTtab DCTtab2[16]
#ifdef GLOBAL
=
{
  {0,11,12}, {8,2,12}, {4,3,12}, {0,10,12},
  {2,4,12}, {7,2,12}, {21,1,12}, {20,1,12},
  {0,9,12}, {19,1,12}, {18,1,12}, {1,5,12},
  {3,3,12}, {0,8,12}, {6,2,12}, {17,1,12}
}
#endif
;

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 0000000010000 ... 0000000011111
 */
DCTtab DCTtab3[16]
#ifdef GLOBAL
=
{
  {10,2,13}, {9,2,13}, {5,3,13}, {3,4,13},
  {2,5,13}, {1,7,13}, {1,6,13}, {0,15,13},
  {0,14,13}, {0,13,13}, {0,12,13}, {26,1,13},
  {25,1,13}, {24,1,13}, {23,1,13}, {22,1,13}
}
#endif
;

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 00000000010000 ... 00000000011111
 */
DCTtab DCTtab4[16]
#ifdef GLOBAL
=
{
  {0,31,14}, {0,30,14}, {0,29,14}, {0,28,14},
  {0,27,14}, {0,26,14}, {0,25,14}, {0,24,14},
  {0,23,14}, {0,22,14}, {0,21,14}, {0,20,14},
  {0,19,14}, {0,18,14}, {0,17,14}, {0,16,14}
}
#endif
;

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 000000000010000 ... 000000000011111
 */
DCTtab DCTtab5[16]
#ifdef GLOBAL
=
{
  {0,40,15}, {0,39,15}, {0,38,15}, {0,37,15},
  {0,36,15}, {0,35,15}, {0,34,15}, {0,33,15},
  {0,32,15}, {1,14,15}, {1,13,15}, {1,12,15},
  {1,11,15}, {1,10,15}, {1,9,15}, {1,8,15}
}
#endif
;

/* Table B-14/15, DCT coefficients table zero / one,
 * codes 0000000000010000 ... 0000000000011111
 */
DCTtab DCTtab6[16]
#ifdef GLOBAL
=
{
  {1,18,16}, {1,17,16}, {1,16,16}, {1,15,16},
  {6,3,16}, {16,2,16}, {15,2,16}, {14,2,16},
  {13,2,16}, {12,2,16}, {11,2,16}, {31,1,16},
  {30,1,16}, {29,1,16}, {28,1,16}, {27,1,16}
}
#endif
;

/* Table B-3, macroblock_type in P-pictures, codes 001..1xx */
VLCtab PMBtab0[8]
#ifdef GLOBAL
=
{
  {ERROR_VALUE,0},
  {MACBLK_MOTION_FWD,3},
  {MACBLK_PATTERN,2}, {MACBLK_PATTERN,2},
  {MACBLK_MOTION_FWD|MACBLK_PATTERN,1},
  {MACBLK_MOTION_FWD|MACBLK_PATTERN,1},
  {MACBLK_MOTION_FWD|MACBLK_PATTERN,1},
  {MACBLK_MOTION_FWD|MACBLK_PATTERN,1}
}
#endif
;

/* Table B-3, macroblock_type in P-pictures, codes 000001..00011x */
VLCtab PMBtab1[8]
#ifdef GLOBAL
=
{
  {ERROR_VALUE,0},
  {MACBLK_QUANT|MACBLK_INTRA,6},
  {MACBLK_QUANT|MACBLK_PATTERN,5}, {MACBLK_QUANT|MACBLK_PATTERN,5},
  {MACBLK_QUANT|MACBLK_MOTION_FWD|MACBLK_PATTERN,5}, {MACBLK_QUANT|MACBLK_MOTION_FWD|MACBLK_PATTERN,5},
  {MACBLK_INTRA,5}, {MACBLK_INTRA,5}
}
#endif
;

/* Table B-4, macroblock_type in B-pictures, codes 0010..11xx */
VLCtab BMBtab0[16]
#ifdef GLOBAL
=
{
  {ERROR_VALUE,0},
  {ERROR_VALUE,0},
  {MACBLK_MOTION_FWD,4},
  {MACBLK_MOTION_FWD|MACBLK_PATTERN,4},
  {MACBLK_MOTION_BWD,3},
  {MACBLK_MOTION_BWD,3},
  {MACBLK_MOTION_BWD|MACBLK_PATTERN,3},
  {MACBLK_MOTION_BWD|MACBLK_PATTERN,3},
  {MACBLK_MOTION_FWD|MACBLK_MOTION_BWD,2},
  {MACBLK_MOTION_FWD|MACBLK_MOTION_BWD,2},
  {MACBLK_MOTION_FWD|MACBLK_MOTION_BWD,2},
  {MACBLK_MOTION_FWD|MACBLK_MOTION_BWD,2},
  {MACBLK_MOTION_FWD|MACBLK_MOTION_BWD|MACBLK_PATTERN,2},
  {MACBLK_MOTION_FWD|MACBLK_MOTION_BWD|MACBLK_PATTERN,2},
  {MACBLK_MOTION_FWD|MACBLK_MOTION_BWD|MACBLK_PATTERN,2},
  {MACBLK_MOTION_FWD|MACBLK_MOTION_BWD|MACBLK_PATTERN,2}
}
#endif
;

/* Table B-4, macroblock_type in B-pictures, codes 000001..00011x */
VLCtab BMBtab1[8]
#ifdef GLOBAL
=
{
  {ERROR_VALUE,0},
  {MACBLK_QUANT|MACBLK_INTRA,6},
  {MACBLK_QUANT|MACBLK_MOTION_BWD|MACBLK_PATTERN,6},
  {MACBLK_QUANT|MACBLK_MOTION_FWD|MACBLK_PATTERN,6},
  {MACBLK_QUANT|MACBLK_MOTION_FWD|MACBLK_MOTION_BWD|MACBLK_PATTERN,5},
  {MACBLK_QUANT|MACBLK_MOTION_FWD|MACBLK_MOTION_BWD|MACBLK_PATTERN,5},
  {MACBLK_INTRA,5},
  {MACBLK_INTRA,5}
}
#endif
;

/* motion.c */
void motion_vectors(int PMV[2][2][2], int dmvector[2], int motion_vertical_field_select[2][2],
  int s, int motion_vector_count, int mv_format, int h_r_size, int v_r_size, int dmv, int mvscale);
void Dual_Prime_Arithmetic(int DMV[][2], int *dmvector, int mvx, int mvy);


