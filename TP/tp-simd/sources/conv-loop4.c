/*******************************************************************************
 * vim:set ts=3:
 * File   : conv.c, file for JPEG-JFIF sequential decoder    
 *
 * Copyright (C) 2009-2018 TIMA Laboratory
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
#include "stdlib.h"
#include "stdio.h"
#include "utils.h"

void YCrCb_to_ARGB(uint8_t * YCrCb_MCU[3], uint32_t * RGB_MCU, uint32_t nb_MCU_H, uint32_t nb_MCU_V)
{
   uint8_t *MCU_Y, *MCU_Cr, *MCU_Cb;
   uint8_t index, i, j, k;
   int32_t R[4], G[4], B[4];
   uint8_t Y[4], Cr[4], Cb[4];
   int32_t ARGB[4];
   MCU_Y = YCrCb_MCU[0];
   MCU_Cr = YCrCb_MCU[2];
   MCU_Cb = YCrCb_MCU[1];
   for (i = 0; i < 8 * nb_MCU_V; i++) {
      for (j = 0; j < 8 * nb_MCU_H; j += 4) {
         for (k = 0; k < 4; k++) {
            index = i * (8 * nb_MCU_H) + j + k;
            Y[k] = MCU_Y[index];
            Cb[k] = MCU_Cb[index];
            Cr[k] = MCU_Cr[index];
         }
         for (k = 0; k < 4; k++) {
            R[k] = ((Y[k] << 8) + (Cr[k] - 128) * 359) >> 8;
            G[k] = ((Y[k] << 8) - (Cr[k] - 128) * 183 - (Cb[k] - 128) * 88) >> 8;
            B[k] = ((Y[k] << 8) + (Cb[k] - 128) * 455) >> 8;
            // Saturate 
            if (R[k] > 255) R[k] = 255;
            if (R[k] < 0)   R[k] = 0;
            if (G[k] > 255) G[k] = 255;
            if (G[k] < 0)   G[k] = 0;
            if (B[k] > 255) B[k] = 255;
            if (B[k] < 0)   B[k] = 0;
            ARGB[k] = ((R[k]) << 16) | ((G[k]) << 8) | (B[k]);
         }
         for (k = 0; k < 4; k++) {
            RGB_MCU[index - 3 + k] = ARGB[k];
         }
      }
   }
}
