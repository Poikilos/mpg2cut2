
//--------------------------------------------------------------

static void conv422to444(unsigned char *src, unsigned char *dst)
{
  src += HALF_CLIP_AREA;
  dst += CLIP_AREA;

  __asm
  {
    mov     eax, [src]
    mov     ebx, [dst]
    mov     edi, [Clip_Height]

    movq    mm1, [mmmask_0001]
    pxor    mm0, mm0

convyuv444init:
    movq    mm7, [eax]
    mov     esi, 0x00

convyuv444:
    movq    mm2, mm7
    movq    mm7, [eax+esi+8]
    movq    mm3, mm2
    movq    mm4, mm7

    psrlq   mm3, 8
    psllq   mm4, 56
    por     mm3, mm4

    movq    mm4, mm2
    movq    mm5, mm3

    punpcklbw mm4, mm0
    punpcklbw mm5, mm0

    movq    mm6, mm4
    paddusw   mm4, mm1
    paddusw   mm4, mm5
    psrlw   mm4, 1
    psllq   mm4, 8
    por     mm4, mm6

    punpckhbw mm2, mm0
    punpckhbw mm3, mm0

    movq    mm6, mm2
    paddusw   mm2, mm1
    paddusw   mm2, mm3

    movq    [ebx+esi*2], mm4

    psrlw   mm2, 1
    psllq   mm2, 8
    por     mm2, mm6

    add     esi, 0x08
    cmp     esi, [HALF_WIDTH_D8]
    movq    [ebx+esi*2-8], mm2
    jl      convyuv444

    movq    mm2, mm7
    punpcklbw mm2, mm0
    movq    mm3, mm2

    psllq   mm2, 8
    por     mm2, mm3

    movq    [ebx+esi*2], mm2

    punpckhbw mm7, mm0
    movq    mm6, mm7

    psllq   mm6, 8
    por     mm6, mm7

    movq    [ebx+esi*2+8], mm6

    add     eax, [HALF_WIDTH]
    add     ebx, [Coded_Pic_Width]
    dec     edi
    cmp     edi, 0x00
    jg      convyuv444init
  }
}



//--------------------------------------------------------------

static void conv420to422(unsigned char *src, unsigned char *dst)
{
  if (ScanMode_code)
  {
    __asm
    {
      mov     eax, [src]
      mov     ebx, [dst]
      mov     ecx, ebx
      add     ecx, [HALF_WIDTH]
      mov     esi, 0x00
      movq    mm3, [mmmask_0003]
      pxor    mm0, mm0
      movq    mm4, [mmmask_0002]

      mov     edx, eax
      add     edx, [HALF_WIDTH]
convyuv422topp:
      movd    mm1, [eax+esi]
      movd    mm2, [edx+esi]
      movd    [ebx+esi], mm1
      punpcklbw mm1, mm0
      pmullw    mm1, mm3
      paddusw   mm1, mm4
      punpcklbw mm2, mm0
      paddusw   mm2, mm1
      psrlw   mm2, 0x02
      packuswb  mm2, mm0

      add     esi, 0x04
      cmp     esi, [HALF_WIDTH]
      movd    [ecx+esi-4], mm2
      jl      convyuv422topp

      add     eax, [HALF_WIDTH]
      add     ebx, [Coded_Pic_Width]
      add     ecx, [Coded_Pic_Width]
      mov     esi, 0x00

      mov     edi, [PROGRESSIVE_HEIGHT]
convyuv422p:
      movd    mm1, [eax+esi]

      punpcklbw mm1, mm0
      mov     edx, eax

      pmullw    mm1, mm3
      sub     edx, [HALF_WIDTH]

      movd    mm5, [edx+esi]
      movd    mm2, [edx+esi]

      punpcklbw mm5, mm0
      punpcklbw mm2, mm0
      paddusw   mm5, mm1
      paddusw   mm2, mm1
      paddusw   mm5, mm4
      paddusw   mm2, mm4
      psrlw   mm5, 0x02
      psrlw   mm2, 0x02
      packuswb  mm5, mm0
      packuswb  mm2, mm0

      mov     edx, eax
      add     edx, [HALF_WIDTH]
      add     esi, 0x04
      cmp     esi, [HALF_WIDTH]
      movd    [ebx+esi-4], mm5
      movd    [ecx+esi-4], mm2

      jl      convyuv422p

      add     eax, [HALF_WIDTH]
      add     ebx, [Coded_Pic_Width]
      add     ecx, [Coded_Pic_Width]
      mov     esi, 0x00
      dec     edi
      cmp     edi, 0x00
      jg      convyuv422p

      mov     edx, eax
      sub     edx, [HALF_WIDTH]
convyuv422bottomp:
      movd    mm1, [eax+esi]
      movd    mm5, [edx+esi]
      punpcklbw mm5, mm0
      movd    [ecx+esi], mm1

      punpcklbw mm1, mm0
      pmullw    mm1, mm3
      paddusw   mm5, mm1
      paddusw   mm5, mm4
      psrlw   mm5, 0x02
      packuswb  mm5, mm0

      add     esi, 0x04
      cmp     esi, [HALF_WIDTH]
      movd    [ebx+esi-4], mm5
      jl      convyuv422bottomp
    }
  }
  else
  {
    __asm
    {
      mov     eax, [src]
      mov     ecx, [dst]
      mov     esi, 0x00
      pxor    mm0, mm0
      movq    mm3, [mmmask_0003]
      movq    mm4, [mmmask_0004]
      movq    mm5, [mmmask_0005]

convyuv422topi:
      movd    mm1, [eax+esi]
      mov     ebx, eax
      add     ebx, [HALF_WIDTH]
      movd    mm2, [ebx+esi]
      movd    [ecx+esi], mm1
      punpcklbw mm1, mm0
      movq    mm6, mm1
      pmullw    mm1, mm3

      punpcklbw mm2, mm0
      movq    mm7, mm2
      pmullw    mm2, mm5
      paddusw   mm2, mm1
      paddusw   mm2, mm4
      psrlw   mm2, 0x03
      packuswb  mm2, mm0

      mov     edx, ecx
      add     edx, [HALF_WIDTH]
      pmullw    mm6, mm5
      movd    [edx+esi], mm2

      add     ebx, [HALF_WIDTH]
      movd    mm2, [ebx+esi]
      punpcklbw mm2, mm0
      pmullw    mm2, mm3
      paddusw   mm2, mm6
      paddusw   mm2, mm4
      psrlw   mm2, 0x03
      packuswb  mm2, mm0

      add     edx, [HALF_WIDTH]
      add     ebx, [HALF_WIDTH]
      pmullw    mm7, [mmmask_0007]
      movd    [edx+esi], mm2

      movd    mm2, [ebx+esi]
      punpcklbw mm2, mm0
      paddusw   mm2, mm7
      paddusw   mm2, mm4
      psrlw   mm2, 0x03
      packuswb  mm2, mm0

      add     edx, [HALF_WIDTH]
      add     esi, 0x04
      cmp     esi, [HALF_WIDTH]
      movd    [edx+esi-4], mm2

      jl      convyuv422topi

      add     eax, [Coded_Pic_Width]
      add     ecx, [DOUBLE_WIDTH]
      mov     esi, 0x00

      mov     edi, [INTERLACED_HEIGHT]
convyuv422i:
      movd    mm1, [eax+esi]
      punpcklbw mm1, mm0
      movq    mm6, mm1
      mov     ebx, eax
      sub     ebx, [Coded_Pic_Width]
      movd    mm3, [ebx+esi]
      pmullw    mm1, [mmmask_0007]
      punpcklbw mm3, mm0
      paddusw   mm3, mm1
      paddusw   mm3, mm4
      psrlw   mm3, 0x03
      packuswb  mm3, mm0

      add     ebx, [HALF_WIDTH]
      movq    mm1, [ebx+esi]
      add     ebx, [Coded_Pic_Width]
      movd    [ecx+esi], mm3

      movq    mm3, [mmmask_0003]
      movd    mm2, [ebx+esi]

      punpcklbw mm1, mm0
      pmullw    mm1, mm3
      punpcklbw mm2, mm0
      movq    mm7, mm2
      pmullw    mm2, mm5
      paddusw   mm2, mm1
      paddusw   mm2, mm4
      psrlw   mm2, 0x03
      packuswb  mm2, mm0

      pmullw    mm6, mm5
      mov     edx, ecx
      add     edx, [HALF_WIDTH]
      movd    [edx+esi], mm2

      add     ebx, [HALF_WIDTH]
      movd    mm2, [ebx+esi]
      punpcklbw mm2, mm0
      pmullw    mm2, mm3
      paddusw   mm2, mm6
      paddusw   mm2, mm4
      psrlw   mm2, 0x03
      packuswb  mm2, mm0

      pmullw    mm7, [mmmask_0007]
      add     edx, [HALF_WIDTH]
      add     ebx, [HALF_WIDTH]
      movd    [edx+esi], mm2

      movd    mm2, [ebx+esi]
      punpcklbw mm2, mm0
      paddusw   mm2, mm7
      paddusw   mm2, mm4
      psrlw   mm2, 0x03
      packuswb  mm2, mm0

      add     edx, [HALF_WIDTH]
      add     esi, 0x04
      cmp     esi, [HALF_WIDTH]
      movd    [edx+esi-4], mm2

      jl      convyuv422i
      add     eax, [Coded_Pic_Width]
      add     ecx, [DOUBLE_WIDTH]
      mov     esi, 0x00
      dec     edi
      cmp     edi, 0x00
      jg      convyuv422i

convyuv422bottomi:
      movd    mm1, [eax+esi]
      movq    mm6, mm1
      punpcklbw mm1, mm0
      mov     ebx, eax
      sub     ebx, [Coded_Pic_Width]
      movd    mm3, [ebx+esi]
      punpcklbw mm3, mm0
      pmullw    mm1, [mmmask_0007]
      paddusw   mm3, mm1
      paddusw   mm3, mm4
      psrlw   mm3, 0x03
      packuswb  mm3, mm0

      add     ebx, [HALF_WIDTH]
      movq    mm1, [ebx+esi]
      punpcklbw mm1, mm0
      movd    [ecx+esi], mm3

      pmullw    mm1, [mmmask_0003]
      add     ebx, [Coded_Pic_Width]
      movd    mm2, [ebx+esi]
      punpcklbw mm2, mm0
      movq    mm7, mm2
      pmullw    mm2, mm5
      paddusw   mm2, mm1
      paddusw   mm2, mm4
      psrlw   mm2, 0x03
      packuswb  mm2, mm0

      mov     edx, ecx
      add     edx, [HALF_WIDTH]
      pmullw    mm7, [mmmask_0007]
      movd    [edx+esi], mm2

      add     edx, [HALF_WIDTH]
      movd    [edx+esi], mm6

      punpcklbw mm6, mm0
      paddusw   mm6, mm7
      paddusw   mm6, mm4
      psrlw   mm6, 0x03
      packuswb  mm6, mm0

      add     edx, [HALF_WIDTH]
      add     esi, 0x04
      cmp     esi, [HALF_WIDTH]
      movd    [edx+esi-4], mm6

      jl      convyuv422bottomi
    }
  }
}



//-------------------------------------------------------------------
/*
static void conv422toyuy2odd(unsigned char *py, unsigned char *pu,
                    unsigned char *pv, unsigned char *dst)
{
  py += CLIP_STEP;
  pu += CLIP_HALF_STEP;
  pv += CLIP_HALF_STEP;

  Top_Field_Built = 1;

  __asm
  {
    mov     eax, [py]
    mov     ebx, [pu]
    mov     ecx, [pv]
    mov     edx, [dst]
    mov     esi, 0x00
    mov     edi, [Clip_Height]

yuy2conv:
    movd    mm2, [ebx+esi]
    movd    mm3, [ecx+esi]
    punpcklbw mm2, mm3
    movq    mm1, [eax+esi*2]
    movq    mm4, mm1
    punpcklbw mm1, mm2
    punpckhbw mm4, mm2

    add     esi, 0x04
    cmp     esi, [HALF_CLIP_WIDTH]
    movq    [edx+esi*4-16], mm1
    movq    [edx+esi*4-8], mm4
    jl      yuy2conv

    add     eax, [DOUBLE_WIDTH]
    add     ebx, [Coded_Pic_Width]
    add     ecx, [Coded_Pic_Width]
    add     edx, [QUAD_CLIP_WIDTH]
    sub     edi, 0x02
    mov     esi, 0x00
    cmp     edi, 0x00
    jg      yuy2conv

    emms
  }
}

*/

