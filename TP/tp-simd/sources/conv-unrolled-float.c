/*******************************************************************************
 * vim:set ts=3:
 * File   : conv-unrolled-float.c, file for JPEG-JFIF sequential decoder    
 *
 * Copyright (C) 2018 TIMA Laboratory
 * Author(s) :   Frédéric Pétrot <Frederic.Petrot@imag.fr>
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

void YCrCb_to_ARGB(uint8_t *YCrCb_MCU[3], uint32_t *RGB_MCU, uint32_t nb_MCU_H, uint32_t nb_MCU_V)
{
   uint8_t *MCU_Y, *MCU_Cr, *MCU_Cb;
   int32_t R[4], G[4], B[4];
   uint32_t ARGB[4];
   uint8_t index, i, j;

   MCU_Y = YCrCb_MCU[0];
   MCU_Cr = YCrCb_MCU[2];
   MCU_Cb = YCrCb_MCU[1];
   for (i = 0; i < 8 * nb_MCU_V; i++) {
      for (j = 0; j < 8 * nb_MCU_H; j += 4) {
         index = i * (8 * nb_MCU_H)  + j;

         R[0] = (MCU_Cr[index + 0] - 128) * 1.402f + MCU_Y[index + 0];
         R[1] = (MCU_Cr[index + 1] - 128) * 1.402f + MCU_Y[index + 1];
         R[2] = (MCU_Cr[index + 2] - 128) * 1.402f + MCU_Y[index + 2];
         R[3] = (MCU_Cr[index + 3] - 128) * 1.402f + MCU_Y[index + 3];

         B[0] = (MCU_Cb[index + 0] - 128) * 1.7772f + MCU_Y[index + 0];
         B[1] = (MCU_Cb[index + 1] - 128) * 1.7772f + MCU_Y[index + 1];
         B[2] = (MCU_Cb[index + 2] - 128) * 1.7772f + MCU_Y[index + 2];
         B[3] = (MCU_Cb[index + 3] - 128) * 1.7772f + MCU_Y[index + 3];

         G[0] = MCU_Y[index + 0] - (MCU_Cb[index + 0] - 128) * 0.381834f - (MCU_Cr[index + 0] - 128) * 0.71414f;
         G[1] = MCU_Y[index + 1] - (MCU_Cb[index + 1] - 128) * 0.381834f - (MCU_Cr[index + 1] - 128) * 0.71414f;
         G[2] = MCU_Y[index + 2] - (MCU_Cb[index + 2] - 128) * 0.381834f - (MCU_Cr[index + 2] - 128) * 0.71414f;
         G[3] = MCU_Y[index + 3] - (MCU_Cb[index + 3] - 128) * 0.381834f - (MCU_Cr[index + 3] - 128) * 0.71414f;

         /* Saturate */
         if (R[0] > 255) R[0] = 255;
         if (R[0] < 0)   R[0] = 0;
         if (G[0] > 255) G[0] = 255;
         if (G[0] < 0)   G[0] = 0;
         if (B[0] > 255) B[0] = 255;
         if (B[0] < 0)   B[0] = 0;

         if (R[1] > 255) R[1] = 255;
         if (R[1] < 0)   R[1] = 0;
         if (G[1] > 255) G[1] = 255;
         if (G[1] < 0)   G[1] = 0;
         if (B[1] > 255) B[1] = 255;
         if (B[1] < 0)   B[1] = 0;

         if (R[2] > 255) R[2] = 255;
         if (R[2] < 0)   R[2] = 0;
         if (G[2] > 255) G[2] = 255;
         if (G[2] < 0)   G[2] = 0;
         if (B[2] > 255) B[2] = 255;
         if (B[2] < 0)   B[2] = 0;

         if (R[3] > 255) R[3] = 255;
         if (R[3] < 0)   R[3] = 0;
         if (G[3] > 255) G[3] = 255;
         if (G[3] < 0)   G[3] = 0;
         if (B[3] > 255) B[3] = 255;
         if (B[3] < 0)   B[3] = 0;

         ARGB[0] = (R[0] << 16) | (G[0] << 8) | (B[0]);
         ARGB[1] = (R[1] << 16) | (G[1] << 8) | (B[1]);
         ARGB[2] = (R[2] << 16) | (G[2] << 8) | (B[2]);
         ARGB[3] = (R[3] << 16) | (G[3] << 8) | (B[3]);

         RGB_MCU[index + 0] = ARGB[0];
         RGB_MCU[index + 1] = ARGB[1];
         RGB_MCU[index + 2] = ARGB[2];
         RGB_MCU[index + 3] = ARGB[3];
      }
   }
}
