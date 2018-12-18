/*******************************************************************************
 * vim:set ts=3:
 * File   : conv.c, file for JPEG-JFIF sequential decoder    
 *
 * Copyright (C) 2007-2018 TIMA Laboratory
 * Author(s) :      Frédéric Pétrot <Frederic.Petrot@imag.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *************************************************************************************/

#include "conv.h"
#include "stdlib.h"
#include "stdio.h"
#include "utils.h"

/* SIMD optimized integer version with 4 iterations unrolled */
void YCrCb_to_ARGB(uint8_t *YCrCb_MCU[3], uint32_t *RGB_MCU, uint32_t nb_MCU_H, uint32_t nb_MCU_V)
{
   uint8_t *MCU_Y, *MCU_Cr, *MCU_Cb;
   int8_t index, i, j;
   static int initialized = 0;

   if (!initialized) {
      __asm__("emms");
      initialized = 1;
   }

   MCU_Y = YCrCb_MCU[0];
   MCU_Cr = YCrCb_MCU[2];
   MCU_Cb = YCrCb_MCU[1];
   for (i = 0; i < 8 * nb_MCU_V; i++) {
      for (j = 0; j < 8 * nb_MCU_H; j += 4) {
         index = i * (8 * nb_MCU_H)  + j;
         /* Set mm0 to full zero */
         __asm__("pxor      %mm0, %mm0");
         /* Load 4 bytes of MCU_Y memory into mm0 and at the same time
          * expand them to an intel "word", i.e. 16 bits, putting the read value on the most
          * significant byte.
          * This is powerfull (but hard to understand, poor compiler), and corresponds exactly
          * to what we need:
          *     xmm0 = (MCU_Y[index] << 8)
          */
         __asm__("punpcklbw %0,  %%mm0"::"m"(MCU_Y[index]));
#ifdef YCrCb_DEBUG
         /* Checking the intermediate results */
         uint16_t Y[4]; 
         __asm__("movq %%mm0, %0":"=m"(Y[0]));
         /* OK! */
#endif
         /* Cr Cb Computation */
         /*     xmm1 = (MCU_Cr[index] - 128) */
         uint64_t v80 = 0x0080008000800080;
         /*     xmm7 contains the operand to be subtracted */
         __asm__("movq %0,  %%mm7" :: "r"(v80));
         /* Load MCU_Cr into mm1 and expand it to 16 bits, and sub 128 */
         __asm__("pxor      %mm1, %mm1");
         __asm__("punpcklbw %0,  %%mm1"::"m"(MCU_Cr[index]));
         __asm__("psrlq     $8, %mm1");
         __asm__("psubw     %mm7, %mm1");
         /* Load MCU_Cb into mm7 and expand it to 16 bits, and sub 128 */
         /*     xmm2 = (MCU_Cb[index] - 128) */
         __asm__("pxor      %mm2, %mm2");
         __asm__("punpcklbw %0,  %%mm2"::"m"(MCU_Cb[index]));
         __asm__("psrlq     $8, %mm2");
         __asm__("psubw     %mm7, %mm2");
#ifdef YCrCb_DEBUG
         /* Checking the intermediate results */
         int8_t  Cr[4], Cb[4];
         __asm__("movq %%mm1, %0":"=m"(Cr[0]));
         __asm__("movq %%mm2, %0":"=m"(Cb[0]));
         /* OK! */
#endif
         /* Here we have the Y_i in mm0, Cr_i in mm1 and Cb_i in mm2.
          * Since these values are used for R, G and B computations,
          * these registers should not be the target of any instruction */
         /* Load 359 (0x0167) into mm3 */
         uint64_t v167 = 0x0167016701670167;

         /* Now compute R by mult and add */
         /* R = ((MCU_Y[index] << 8) + (MCU_Cr[index] - 128) * 359) >> 8
          *      (xmm0               +  xmm1                 * xmm3) >> 8 */
         __asm__("movq      %0, %%mm3" ::"r"(v167));
         __asm__("pmullw    %mm1, %mm3");
#if 0
         __asm__("movq      %0, %%mm7" ::"r"(v167));
         __asm__("pmulhw    %mm1, %mm7");
         /* Make a copy of mm3 */
         __asm__("movq      %mm3, %mm4");
         /* Build two 32 bit results */
         __asm__("punpcklwd %mm7, %mm3");
         __asm__("punpckhwd %mm7, %mm4");
         /* Set mm7 to full 0 */
         __asm__("pxor      %mm7, %mm7");
         /* Extend xmm0 value on 32 bits */
         /* Make a copy of mm0 */
         __asm__("movq      %mm0, %mm5");
         __asm__("punpcklwd %mm7, %mm5");
         /* Make a copy of mm0 */
         __asm__("movq      %mm0, %mm6");
         __asm__("punpckhwd %mm7, %mm6");
         /* Do the addition */
         __asm__("paddd     %mm5, %mm3");
         __asm__("paddd     %mm6, %mm4");
         /* Divide by 256 */
         __asm__("packssdw  %mm7, %mm3");
         __asm__("packssdw  %mm7, %mm4");
         __asm__("psrlw     $8, %mm3");
         __asm__("psrlw     $8, %mm4");
         __asm__("packuswb  %mm4, %mm3");
#else
         __asm__("paddsw    %mm0, %mm3");
         __asm__("psrlw     $8, %mm3");
         __asm__("packuswb  %mm3, %mm3");
#endif
#ifdef RGB_DEBUG
         uint8_t R[4];
         __asm__("movd %%mm3, %0":"=m"(R[0]));
#endif
         /* Load 183 (0xB7) into mm4 and 88 (0x58) into mm5 */
         uint64_t vB7 = 0x00B700B700B700B7;
         uint64_t v58 = 0x0058005800580058;
         /* Now compute G by mult, sub, add and saturate */
         /* G = ((MCU_Y[index] << 8) - (MCU_Cr[index] - 128) * 183 - (MCU_Cb[index] - 128) * 88) >> 8 */
         __asm__("movq      %0, %%mm4" ::"r"(vB7));
         __asm__("pmullw    %mm1, %mm4");
         __asm__("movq      %0, %%mm5" ::"r"(v58));
         __asm__("pmullw    %mm2, %mm5");

         __asm__("paddsw    %mm4, %mm5");
         /* Copy Y_i as we will corrupt it */
         __asm__("movq      %mm0, %mm4");
         __asm__("psubw     %mm5, %mm4");
         __asm__("psrlw     $8, %mm4");
         __asm__("packuswb  %mm4, %mm4");
#ifdef RGB_DEBUG
         uint8_t G[4];
         __asm__("movd %%mm4, %0":"=m"(G[0]));
#endif

         /* Load 455 (0x01C7) into mm5 */
         uint64_t v1C7 = 0x01C701C701C701C7;
         __asm__("movq      %0, %%mm5" ::"r"(v1C7));
         /* Now compute B by mult, add and saturate */
         __asm__("pmulhw    %mm1, %mm5");
         __asm__("paddsw    %mm0, %mm5");
         __asm__("psrlw     $8, %mm5");
         __asm__("packuswb  %mm5, %mm5");
#ifdef RGB_DEBUG
         uint8_t B[4];
         __asm__("movd      %%mm5, %0":"=m"(B[0]));
#endif
         /* Now the 4 lowest bytes of mm3, mm4 and mm5 contain the 
          * 4 R, G and B values  of the iteration.
          * We do some kind of matrix transpose to build the 2 times
          * 64 words containing the appropriate values:
          * First, produce a 0/R word, then a G/B word, and finally
          * mix them to produce two 0/R/G/B doubles */
         __asm__("pxor      %mm7, %mm7");
         __asm__("punpcklbw %mm7, %mm3");
         __asm__("punpcklbw %mm4, %mm5");
         __asm__("movq      %mm5, %mm0");
         __asm__("punpcklwd %mm3, %mm5");
         __asm__("punpckhwd %mm3, %mm0");
         __asm__("movq      %%mm5, %0":"=m"(RGB_MCU[index]));
         __asm__("movq      %%mm0, %0":"=m"(RGB_MCU[index+2]));
      }
   }
}
