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
      for (j = 0; j < 8 * nb_MCU_H; /* TODO */) {
         index = i * (8 * nb_MCU_H)  + j;
         /* Set mm0 to full zero */
         __asm__("pxor      %mm0, %mm0");
         /* Load 4 bytes of MCU_Y memory into mm0 and at the same time
          * expand them to an intel "word", i.e. 16 bits. */
         /* TODO */
#ifdef YCrCb_DEBUG
         /* Checking the intermediate results */
         uint16_t Y[4]; 
         __asm__("movq %%mm0, %0":"=m"(Y[0]));
         /* OK! */
#endif
         /* TODO: Cr Cb Computation */

         /* TODO: Load MCU_Cr into mm2 and expand it to 16 bits, and sub 128 */

         /* TODO: Load MCU_Cr into mm2 and expand it to 16 bits, and sub 128 */

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
         /* TODO: Load 359 (0x0167) into mm3 */

         /* TODO: Now compute R by mult and add */

         /* TODO: R is in mm3, finish computation */

#ifdef RGB_DEBUG
         uint8_t R[4];
         __asm__("movd %%mm3, %0":"=m"(R[0]));
#endif
         /* TODO: Load 183 (0xB7) into mm4 and 88 (0x58) into mm5 */

         /* TODO: Now compute G by mult, sub, add and saturate */

         /* TODO: Copy Y_i as we will corrupt it */

         /* TODO: G is in mm4, finish computation */
#ifdef RGB_DEBUG
         uint8_t G[4];
         __asm__("movd %%mm4, %0":"=m"(G[0]));
#endif

         /* TODO: Load 455 (0x01C7) into mm5 */

         /* TODO: Now compute B by mult, add and saturate */

         /* TODO: B is in mm5, finish computation */

#ifdef RGB_DEBUG
         uint8_t B[4];
         __asm__("movd      %%mm5, %0":"=m"(B[0]));
#endif
         /* TODO: Now the 4 lowest bytes of mm3, mm4 and mm5 contain the 
          * 4 R, G and B values  of the iteration.
          * We do some kind of matrix transpose to build the 2 times
          * 64 words containing the appropriate values:
          * First, produce a 0/R word, then a G/B word, and finally
          * mix them to produce two 0/R/G/B quads */
         __asm__("pxor      %mm7, %mm7");
         /* TODO: finish ... */
      }
   }
}
