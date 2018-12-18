/*******************************************************************************
 * vim:set ts=3:
 * File   : conv-float.c, file for JPEG-JFIF sequential decoder    
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

/*
 * Conversion YCrCb vers RGB en utilisant les formules de Wikipedia :
 *    https://en.wikipedia.org/wiki/YCbCr#JPEG_conversion
 */
void YCrCb_to_ARGB(uint8_t *YCrCb_MCU[3], uint32_t *RGB_MCU, uint32_t nb_MCU_H, uint32_t nb_MCU_V)
{
   uint8_t *MCU_Y, *MCU_Cr, *MCU_Cb;
   int32_t R, G, B;
   uint32_t ARGB;
   uint8_t index, i, j;

   /* Récupération des pointeurs sur les macro-bloc 8x8 de lumimance, chrominance rouge, chrominance bleue */
   MCU_Y  = YCrCb_MCU[0];
   MCU_Cr = YCrCb_MCU[2];
   MCU_Cb = YCrCb_MCU[1];

   for (i = 0; i < 8 * nb_MCU_V; i++) {
      for (j = 0; j < 8 * nb_MCU_H; j++) {
         index = i * (8 * nb_MCU_H)  + j;

         /* Calcul de la conversion pixel par pixel */
         R = (MCU_Cr[index] - 128) * 1.402f + MCU_Y[index];
         B = (MCU_Cb[index] - 128) * 1.7772f + MCU_Y[index];
         G = MCU_Y[index] - (MCU_Cb[index] - 128) * 0.381834f - (MCU_Cr[index] - 128) * 0.71414f;

         /* Saturation des valeurs pour se ramener à 32 bits pour Alpha, Red, Green, Blue */
         if (R > 255) R = 255;
         if (R < 0)   R = 0;
         if (G > 255) G = 255;
         if (G < 0)   G = 0;
         if (B > 255) B = 255;
         if (B < 0)   B = 0;
         ARGB = ((R & 0xFF) << 16) | ((G & 0xFF) << 8) | (B & 0xFF);
         /* Écriture du pixel dans le macro-bloc de sortie */
         RGB_MCU[(i * (8 * nb_MCU_H) + j)] = ARGB;
      }
   }
}
