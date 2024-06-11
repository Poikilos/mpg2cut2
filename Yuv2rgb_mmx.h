
    // Convert 4 pels of YUV variables from registers
    // outputting into rgb buffer

    // Note there are 2 lines where it reloads U,V from dUtmp, dVtmp.


  __asm
  {

    movq    mm7, [mmmask_0128] //             Move Quad x80
    psubw   mm3, mm7           // MM3=U-x80   Packed Subtract Word
    psubw   mm5, mm7           // MM5=U-x80   Packed Subtract Word

    psubw   mm1, [RGB_Offset]  // MM1=Y-OFFSET Packed Subtract Word

    movq    mm2, mm1           // MM2=Y-OFFSET  Move Quad
    movq    mm7, [mmmask_0001] //               Move Quad
    punpcklwd mm1, mm7       // MM1=Y-OFFSET LO  Unpack Low Packed Word-Dbl 
    punpckhwd mm2, mm7       // MM2=Y-OFFSET HI  Unpack Low Packed Word-Dbl

    movq    mm7, [RGB_Scale] // Move Quad
    pmaddwd   mm1, mm7       // Y LO     Packed Multiply and Add Word
    pmaddwd   mm2, mm7       // Y HI     Packed Multiply and Add Word

    movq    mm4, mm3         // U       Move Quad
    punpcklwd mm3, mm0       // U LO    Unpack Low Packed Word-Dbl 
    punpckhwd mm4, mm0       // U HI    Unpack HIGH Packed Word-Dbl 

    movq    mm7, [RGB_CBU]   // Move Quad
    pmaddwd   mm3, mm7       // U LO          Packed Multiply and Add Word
    pmaddwd   mm4, mm7       // U HI          Packed Multiply and Add Word

    paddd   mm3, mm1         // U LO   Y LO   Add packed dwords
    paddd   mm4, mm2         // U HI   Y HI   Add packed dwords
    psrld   mm3, 13          // U LO          Packed Shift Right Logical 
    psrld   mm4, 13          // U HI          Packed Shift Right Logical 
    packuswb  mm3, mm0       // U LO          Pack with Unsigned Saturation
    packuswb  mm4, mm0       // U HI          Pack with Unsigned Saturation

    movq    mm6, mm5         // U       Move Quad
    punpcklwd mm5, mm0       // U LO    Unpack Low Packed Word-Dbl 
    punpckhwd mm6, mm0       // U HI    Unpack HIGH Packed Word-Dbl 
    movq    mm7, [RGB_CRV]   //         Move Quad
    pmaddwd   mm5, mm7       // U LO     Packed Multiply and Add Word
    pmaddwd   mm6, mm7       // U HI     Packed Multiply and Add Word
    paddd   mm5, mm1         // U LO     Add packed dwords
    paddd   mm6, mm2         // U HI     Add packed dwords

    psrld   mm5, 13          // U LO      Packed Shift Right Logical 
    psrld   mm6, 13          // U HI      Packed Shift Right Logical 
    packuswb  mm5, mm0       // U LO      Pack with Unsigned Saturation
    packuswb  mm6, mm0       // U HI      Pack with Unsigned Saturation

    punpcklbw mm3, mm5       //  Unpack Low Packed Word-Dbl 
    punpcklbw mm4, mm6       //  Unpack HIGH Packed Word-Dbl 
    movq    mm5, mm3         //  Move Quad
    movq    mm6, mm4         //  Move Quad
    psrlq   mm5, 16          //  Packed Shift Right Logical 
    psrlq   mm6, 16          //  Packed Shift Right Logical 
    por     mm3, mm5         //  OR qword from .. to mm
    por     mm4, mm6         //  OR qword from .. to mm

    //movd    mm5, [ebx+esi]   //  U  Move doubleword  
    //movd    mm6, [ecx+esi]   //  V  Move doubleword  
    movd    mm5, [dUtmp]   //  U  Move doubleword  
    movd    mm6, [dVtmp]   //  V  Move doubleword  

    punpcklbw mm5, mm0       //  U  Unpack Low Packed Word-Dbl
    punpcklbw mm6, mm0       //  V  Unpack Low Packed Word-Dbl

    movq    mm7, [mmmask_0128]  //   Move Quad
    psubw   mm5, mm7            // U  Packed Subtract
    psubw   mm6, mm7            // V  Packed Subtract

    movq    mm7, mm6         // Move Quad
    punpcklwd mm6, mm5       // Unpack Low Packed Word-Dbl 
    punpckhwd mm7, mm5       // Unpack Low Packed Word-Dbl 

    movq    mm5, [RGB_CGX]   // Move Quad
    pmaddwd   mm6, mm5       // Packed Multiply and Add
    pmaddwd   mm7, mm5       // Packed Multiply and Add
    paddd   mm6, mm1         // Add packed dwords
    paddd   mm7, mm2         // Add packed dwords

    psrld   mm6, 13          // Packed Shift Right Logical 
    psrld   mm7, 13          // Packed Shift Right Logical 
    packuswb  mm6, mm0       // Pack with Unsigned Saturation
    packuswb  mm7, mm0       // Pack with Unsigned Saturation

    punpcklbw mm3, mm6       // Unpack Low Packed Word-Dbl
    punpcklbw mm4, mm7       // Unpack Low Packed Word-Dbl

    movq    mm1, mm3         // Move Quad
    movq    mm5, mm4         // Move Quad
    movq    mm6, mm4         // Move Quad

    psrlq   mm1, 32          // Packed Shift Right Logical 
    psllq   mm1, 24          // Packed Shift Right Logical 
    por     mm1, mm3

    psrlq   mm3, 40          // Packed Shift Right Logical 
    psllq   mm6, 16          // Packed Shift Right Logical 
    por     mm3, mm6
    movd    [edx], mm1

    psrld   mm4, 16          // Packed Shift Right Logical 
    psrlq   mm5, 24          // Packed Shift Right Logical 
    por     mm5, mm4         // OR qword from .. to mm
    movd    [edx+4], mm3     // Move doubleword 
  }
