/*******************************************************************************
 * vim:set ts=3:
 * File   : conv-sse.c, file for JPEG-JFIF sequential decoder    
 *
 * Copyright (C) 2018 TIMA Laboratory
 * Author: Frédéric Pétrot <frederic.petrot@univ-grenoble-alpes.fr>
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
 */
void YCrCb_to_ARGB(uint8_t *YCrCb_MCU[3], uint32_t *RGB_MCU, uint32_t nb_MCU_H, uint32_t nb_MCU_V)
{
   uint8_t *MCU_Y, *MCU_Cr, *MCU_Cb;
   uint8_t index, i;
   __m128  Rf[4], Gf[4], Bf[4], Y[4], Cr[4], Cb[4];
   __m128i ARGB[4], R[4], G[4], B[4];
   __m128i Rs[2], Gs[2], Bs[2];
   __m128i Rc, Gc, Bc;

   /* On définit les tableaux de constantes dont on va avoir besoin */
   /*
    * Instructions à utiliser
    *     _mm_set... 
    */
   const __m128  v128f      = _mm_set_ps1(128.0f);
   const __m128  v1_402f    = _mm_set_ps1(1.402f);
   const __m128  v1_7772f   = _mm_set_ps1(1.7772f);
   const __m128  v0_381834f = _mm_set_ps1(0.381834f);
   const __m128  v0_71414f  = _mm_set_ps1(0.71414f);
   const __m128i v0         = _mm_set1_epi32(0);

   MCU_Y  = YCrCb_MCU[0];
   MCU_Cb = YCrCb_MCU[1];
   MCU_Cr = YCrCb_MCU[2];

   for (i = 0; i < 64 * (nb_MCU_V * nb_MCU_H); i += 16) {
         /* On travaille à présent sur des vecteurs de 4 éléments d'un coup */
         index = i;

         /*
          * Chargement des macro-blocs:
          * instruction à utiliser
          *     _mm_set...
          */
         Y[0]  = _mm_setr_ps(MCU_Y [index + 0], MCU_Y [index + 1], MCU_Y [index + 2], MCU_Y [index + 3]);
         Cb[0] = _mm_setr_ps(MCU_Cb[index + 0], MCU_Cb[index + 1], MCU_Cb[index + 2], MCU_Cb[index + 3]);
         Cr[0] = _mm_setr_ps(MCU_Cr[index + 0], MCU_Cr[index + 1], MCU_Cr[index + 2], MCU_Cr[index + 3]);
         Y[1]  = _mm_setr_ps(MCU_Y [index + 4], MCU_Y [index + 5], MCU_Y [index + 6], MCU_Y [index + 7]);
         Cb[1] = _mm_setr_ps(MCU_Cb[index + 4], MCU_Cb[index + 5], MCU_Cb[index + 6], MCU_Cb[index + 7]);
         Cr[1] = _mm_setr_ps(MCU_Cr[index + 4], MCU_Cr[index + 5], MCU_Cr[index + 6], MCU_Cr[index + 7]);
         Y[2]  = _mm_setr_ps(MCU_Y [index + 8], MCU_Y [index + 9], MCU_Y [index +10], MCU_Y [index +11]);
         Cb[2] = _mm_setr_ps(MCU_Cb[index + 8], MCU_Cb[index + 9], MCU_Cb[index +10], MCU_Cb[index +11]);
         Cr[2] = _mm_setr_ps(MCU_Cr[index + 8], MCU_Cr[index + 9], MCU_Cr[index +10], MCU_Cr[index +11]);
         Y[3]  = _mm_setr_ps(MCU_Y [index +12], MCU_Y [index +13], MCU_Y [index +14], MCU_Y [index +15]);
         Cb[3] = _mm_setr_ps(MCU_Cb[index +12], MCU_Cb[index +13], MCU_Cb[index +14], MCU_Cb[index +15]);
         Cr[3] = _mm_setr_ps(MCU_Cr[index +12], MCU_Cr[index +13], MCU_Cr[index +14], MCU_Cr[index +15]);

         /*
          * Calcul de la conversion YCrCb vers RGB:
          *    aucune instruction sse explicite
          */
         Rf[0] = (Cr[0] - v128f) * v1_402f + Y[0];
         Bf[0] = (Cb[0] - v128f) * v1_7772f + Y[0];
         Gf[0] = Y[0] - (Cb[0] - v128f) * v0_381834f - (Cr[0] - v128f) * v0_71414f;

         Rf[1] = (Cr[1] - v128f) * v1_402f + Y[1];
         Bf[1] = (Cb[1] - v128f) * v1_7772f + Y[1];
         Gf[1] = Y[1] - (Cb[1] - v128f) * v0_381834f - (Cr[1] - v128f) * v0_71414f;

         Rf[2] = (Cr[2] - v128f) * v1_402f + Y[2];
         Bf[2] = (Cb[2] - v128f) * v1_7772f + Y[2];
         Gf[2] = Y[2] - (Cb[2] - v128f) * v0_381834f - (Cr[2] - v128f) * v0_71414f;

         Rf[3] = (Cr[3] - v128f) * v1_402f + Y[3];
         Bf[3] = (Cb[3] - v128f) * v1_7772f + Y[3];
         Gf[3] = Y[3] - (Cb[3] - v128f) * v0_381834f - (Cr[3] - v128f) * v0_71414f;

         /*
          * Conversion en entier avec calcul de la saturation:
          * trois étapes :
          *    conversion vecteur flottants 32 bits vers vecteur entiers 32 bits
          *    conversion vecteur entiers 32 bits vers vecteur entiers 16 bits non signé en saturant
          *    conversion vecteur entiers 16 bits vers vecteur entiers 8 bits non signé en saturant
          * instructions à utiliser
          *    _mm_cvt...
          *    _mm_pack...
          */
         R[0] = _mm_cvtps_epi32(Rf[0]);
         G[0] = _mm_cvtps_epi32(Gf[0]);
         B[0] = _mm_cvtps_epi32(Bf[0]);
         R[1] = _mm_cvtps_epi32(Rf[1]);
         G[1] = _mm_cvtps_epi32(Gf[1]);
         B[1] = _mm_cvtps_epi32(Bf[1]);
         R[2] = _mm_cvtps_epi32(Rf[2]);
         G[2] = _mm_cvtps_epi32(Gf[2]);
         B[2] = _mm_cvtps_epi32(Bf[2]);
         R[3] = _mm_cvtps_epi32(Rf[3]);
         G[3] = _mm_cvtps_epi32(Gf[3]);
         B[3] = _mm_cvtps_epi32(Bf[3]);

	 Rs[0] = _mm_packus_epi32(R[0], R[1]);
         Gs[0] = _mm_packus_epi32(G[0], G[1]);
         Bs[0] = _mm_packus_epi32(B[0], B[1]);
	 Rs[1] = _mm_packus_epi32(R[2], R[3]);
         Gs[1] = _mm_packus_epi32(G[2], G[3]);
         Bs[1] = _mm_packus_epi32(B[2], B[3]);

         Rc = _mm_packus_epi16(Rs[0], Rs[1]);
         Gc = _mm_packus_epi16(Gs[0], Gs[1]);
         Bc = _mm_packus_epi16(Bs[0], Bs[1]);

         /*
          * Transposition de la matrice d'octets, brillant !
          * instructions à utiliser
          *    _mm_unpack...
          */
         /* R3 | R2 | R1 | R0 */
         /* ARGB3 | ARGB2 | ARGB1 | ARGB0 */ 
         __m128i AR[2], GB[2];
         AR[0]   = _mm_unpacklo_epi8(Rc, v0);
         GB[0]   = _mm_unpacklo_epi8(Bc, Gc);
         AR[1]   = _mm_unpackhi_epi8(Rc, v0);
         GB[1]   = _mm_unpackhi_epi8(Bc, Gc);
         ARGB[0] = _mm_unpacklo_epi16(GB[0], AR[0]);
         ARGB[1] = _mm_unpackhi_epi16(GB[0], AR[0]);
         ARGB[2] = _mm_unpacklo_epi16(GB[1], AR[1]);
         ARGB[3] = _mm_unpackhi_epi16(GB[1], AR[1]);

         /*
          * Écriture du résultat en mémoire
          * instruction à utiliser
          * _mm_store...
          */
         _mm_store_si128((__m128i*)&RGB_MCU[index], ARGB[0]);
         _mm_store_si128((__m128i*)&RGB_MCU[index+4], ARGB[1]);
         _mm_store_si128((__m128i*)&RGB_MCU[index+8], ARGB[2]);
         _mm_store_si128((__m128i*)&RGB_MCU[index+12], ARGB[3]);
   }
}
