/*******************************************************************************
 * vim:set ts=3:
 * File   : conv-sse.c, file for JPEG-JFIF sequential decoder    
 *
 * Copyright (C) 2018 TIMA Laboratory
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
#include <x86intrin.h>

#include <stdalign.h> 

/* Fonction d'affichage de variables de __m128x sous divers formats */
void p128_x(__m128 fl, __m128i in, __m128i sh, __m128i ch)
{
    alignas(16) float   t[4];
    alignas(16) int32_t u[4];
    alignas(16) int16_t v[4];
    alignas(16) int8_t  w[4];
     
    _mm_store_ps   (           t, fl);
    _mm_store_si128((__m128i *)u, in);
    _mm_store_si128((__m128i *)v, sh);
    _mm_store_si128((__m128i *)w, ch);

#if 1
    printf("{%f, %f, %f, %f} ", t[0], t[1], t[2], t[3]);
    printf("{%d, %d, %d, %d} ", u[0], u[1], u[2], u[3]);
    printf("{%hd, %hd, %hd, %hd} ", v[0], v[1], v[2], v[3]);
    printf("{%hhu, %hhu, %hhu, %hhu}\n", w[0], w[1], w[2], w[3]);
#endif
}

/*
 * Conversion en utilisant maintenant les extensions SSE des processeurs intel.
 * la doc est là : https://software.intel.com/sites/landingpage/IntrinsicsGuide/#techs=SSE,SSE2,SSE3,SSSE3,SSE4_1,SSE4_2
 * Une bonne journée de travail pour arriver à ce résultat finalement relativement simple
 */
void YCrCb_to_ARGB(uint8_t *YCrCb_MCU[3], uint32_t *RGB_MCU, uint32_t nb_MCU_H, uint32_t nb_MCU_V)
{
   uint8_t *MCU_Y, *MCU_Cr, *MCU_Cb;
   uint8_t index, i, j;
   __m128  Rf, Gf, Bf, Y, Cr, Cb;
   __m128i ARGB, R, G, B;
   __m128i Rs, Gs, Bs;
   __m128i Rc, Gc, Bc;

   /* On définit les tableaux de constantes dont on va avoir besoin */
#if 0
   const __m128  v128f      = _mm_set_ps1(128.0f);
   const __m128  v1_402f    = _mm_set_ps1(1.402f);
   const __m128  v1_7772f   = _mm_set_ps1(1.7772f);
   const __m128  v0_381834f = _mm_set_ps1(0.381834f);
   const __m128  v0_71414f  = _mm_set_ps1(0.71414f);
#endif
   const __m128i v0         = _mm_set1_epi32(0);

   MCU_Y  = YCrCb_MCU[0];
   MCU_Cb = YCrCb_MCU[1];
   MCU_Cr = YCrCb_MCU[2];

   for (i = 0; i < 8 * nb_MCU_V; i++) {
      for (j = 0; j < 8 * nb_MCU_H; j+=4) {
         /* On travaille à présent sur des vecteurs de 4 éléments d'un coup */
         index = i * (8 * nb_MCU_H) + j;

         Y  = _mm_setr_ps(MCU_Y [index + 0], MCU_Y [index + 1], MCU_Y [index + 2], MCU_Y [index + 3]);
         Cb = _mm_setr_ps(MCU_Cb[index + 0], MCU_Cb[index + 1], MCU_Cb[index + 2], MCU_Cb[index + 3]);
         Cr = _mm_setr_ps(MCU_Cr[index + 0], MCU_Cr[index + 1], MCU_Cr[index + 2], MCU_Cr[index + 3]);

#if 0
         Rf = Y + (Cr - v128f) * v1_402f;
         Bf = Y + (Cb - v128f) * v1_7772f;
         Gf = Y - (Cb - v128f) * v0_381834f - (Cr - v128f) * v0_71414f;
#else
         Rf = Y + (Cr - 128.0f) * 1.402f;
         Bf = Y + (Cb - 128.0f) * 1.7772f;
         Gf = Y - (Cb - 128.0f) * 0.381834f - (Cr - 128.0f) * 0.71414f;
#endif

         /* Conversion en entier avec calcul de la saturation */
         R = _mm_cvtps_epi32(Rf);
         G = _mm_cvtps_epi32(Gf);
         B = _mm_cvtps_epi32(Bf);

         Rs = _mm_packus_epi32(R, v0);
         Gs = _mm_packus_epi32(G, v0);
         Bs = _mm_packus_epi32(B, v0);

         Rc = _mm_packus_epi16(Rs, v0);
         Gc = _mm_packus_epi16(Gs, v0);
         Bc = _mm_packus_epi16(Bs, v0);

         /* Transposition de la matrice d'octets, brillant ! */
         __m128i tmp0, tmp1;
         tmp0 = _mm_unpacklo_epi8(Rc, v0);
         tmp1 = _mm_unpacklo_epi8(Bc, Gc);
         ARGB = _mm_unpacklo_epi16(tmp1, tmp0);

         _mm_store_si128((__m128i *)&RGB_MCU[index], ARGB);
      }
   }
}
