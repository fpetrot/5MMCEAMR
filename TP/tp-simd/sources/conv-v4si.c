/*******************************************************************************
 * vim:set ts=3:
 * File   : conv.c, file for JPEG-JFIF sequential decoder    
 *
 * Copyright (C) 2007-2018 TIMA Laboratory
 * Author: Frédéric Pétrot <Frederic.Petrot@imag.fr>
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
#include "stdio.h"
#include "utils.h"

/*
 * Type vecteur supporté par gcc (cf info gcc) que l'on utilise ici
 * v4si : 4 entiers signés par vecteur 
 */
typedef int32_t  v4si __attribute__ ((vector_size (16)));

void YCrCb_to_ARGB(uint8_t *YCrCb_MCU[3], uint32_t *RGB_MCU, uint32_t nb_MCU_H, uint32_t nb_MCU_V)
{
   uint8_t *MCU_Y, *MCU_Cr, *MCU_Cb;
   uint8_t index, i, j, k;
   v4si    ARGB, R, G, B;
   v4si    Y, Cr, Cb;

   MCU_Y  = YCrCb_MCU[0];
   MCU_Cb = YCrCb_MCU[1];
   MCU_Cr = YCrCb_MCU[2];

   for (i = 0; i < 8 * nb_MCU_V; i++) {
      for (j = 0; j < 8 * nb_MCU_H; j+=4) {
         index = i * (8 * nb_MCU_H) + j;

         /* On travaille à présent sur des vecteurs de 4 éléments d'un coup */
         Y  = (v4si){MCU_Y [index + 0], MCU_Y [index + 1], MCU_Y [index + 2], MCU_Y [index + 3]};
         Cb = (v4si){MCU_Cb[index + 0], MCU_Cb[index + 1], MCU_Cb[index + 2], MCU_Cb[index + 3]};
         Cr = (v4si){MCU_Cr[index + 0], MCU_Cr[index + 1], MCU_Cr[index + 2], MCU_Cr[index + 3]};
         
         R = ((Y << 8) + (Cr - 128) * 359) >> 8;
         G = ((Y << 8) - (Cr - 128) * 183 - (Cb - 128) * 88) >> 8;
         B = ((Y << 8) + (Cb - 128) * 455) >> 8;

         /* Saturation qu'on ne sait pas faire en C car cela nécessite un if par élément du vecteur */
         for (k = 0; k < 4; k++) {
            if (R[k] > 255) R[k] = 255;
            if (R[k] < 0)   R[k] = 0;
            if (G[k] > 255) G[k] = 255;
            if (G[k] < 0)   G[k] = 0;
            if (B[k] > 255) B[k] = 255;
            if (B[k] < 0)   B[k] = 0;
         }

         ARGB = (R << 16) | (G << 8) | B;
         *(v4si *)&RGB_MCU[index] = ARGB;
      }
   }
}
