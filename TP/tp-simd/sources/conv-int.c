/*******************************************************************************
 * vim:set ts=3:
 * File   : conv-int.c, file for JPEG-JFIF sequential decoder    
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
 * Conversion YCrCb vers RGB en utilisant maintenant uniquement des entiers.
 * J'ai fait quelques essais pour trouver des valeurs qui donnent des résultats corrects du
 * point du vue du rapport signal à bruit en comparant aux résultats utilisant les flottants.
 */
void YCrCb_to_ARGB(uint8_t *YCrCb_MCU[3], uint32_t *RGB_MCU, uint32_t nb_MCU_H, uint32_t nb_MCU_V)
{
   uint8_t *MCU_Y, *MCU_Cr, *MCU_Cb;
   int16_t R, G, B;
   uint32_t ARGB;
   uint8_t index, i, j;

   MCU_Y = YCrCb_MCU[0];
   MCU_Cr = YCrCb_MCU[2];
   MCU_Cb = YCrCb_MCU[1];
   for (i = 0; i < 8 * nb_MCU_V; i++) {
      for (j = 0; j < 8 * nb_MCU_H; j++) {
         index = i * (8 * nb_MCU_H)  + j;
         /* Conversion des pixels */
         R = ((MCU_Y[index] << 8) + (MCU_Cr[index] - 128) * 359) >> 8;
         G = ((MCU_Y[index] << 8) - (MCU_Cr[index] - 128) * 183 - (MCU_Cb[index] - 128) * 88) >> 8;
         B = ((MCU_Y[index] << 8) + (MCU_Cb[index] - 128) * 455) >> 8;

         /* Saturation */
         if (R > 255) R = 255;
         if (R < 0)   R = 0;
         if (G > 255) G = 255;
         if (G < 0)   G = 0;
         if (B > 255) B = 255;
         if (B < 0)   B = 0;
         ARGB = (R << 16) | (G << 8) | (B << 0);
         /* Mise à jour du macro-bloc de sortie */
         RGB_MCU[index] = ARGB;
      }
   }
}
