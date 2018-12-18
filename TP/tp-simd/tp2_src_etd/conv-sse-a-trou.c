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
 */
void YCrCb_to_ARGB(uint8_t *YCrCb_MCU[3], uint32_t *RGB_MCU, uint32_t nb_MCU_H, uint32_t nb_MCU_V)
{
   uint8_t *MCU_Y, *MCU_Cr, *MCU_Cb;
   uint8_t index, i, j;
   __m128  Rf, Gf, Bf, Y, Cr, Cb;
   __m128i ARGB, R, G, B;
   __m128i Rs, Gs, Bs;
   __m128i Rc, Gc, Bc;

   /* On définit le tableau de zéro dont on va avoir besoin */
   /*
    * Instructions à utiliser
    *     _mm_set... 
    */
   const __m128i v0         = _mm_...(0);

   MCU_Y  = YCrCb_MCU[0];
   MCU_Cb = YCrCb_MCU[1];
   MCU_Cr = YCrCb_MCU[2];

   for (i = 0; i < 8 * nb_MCU_V; i++) {
      for (j = 0; j < 8 * nb_MCU_H; /* TODO: a ajuster, ... */) {
         /* On travaille à présent sur des vecteurs de 4 éléments d'un coup */
         index = i * (8 * nb_MCU_H) + ...;

         /*
          * Chargement des macro-blocs:
          * instruction à utiliser
          *     _mm_set...
          */
         Y  = _mm_...(MCU_Y [index + ...], ..., ..., ...);
         Cb = _mm_...(MCU_Cb[index + ...], ..., ..., ...);
         Cr = _mm_...(MCU_Cr[index + ...], ..., ..., ...);

         /*
          * Calcul de la conversion YCrCb vers RGB:
          *    aucune instruction sse explicite
          */
         Rf = ...;
         Bf = ...;
         Gf = ...;

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
         R = ...
         G = ...
         B = ...

         Rs = ...
         Gs = ...
         Bs = ...

         Rc = ...
         Gc = ...
         Bc = ...

         /*
          * Transposition de la matrice d'octets, brillant !
          * instructions à utiliser
          *    _mm_unpack...
          */
         __m128i tmp0, tmp1;
         tmp0 = ...
         tmp1 = ...
         ARGB = ...

         /*
          * Écriture du résultat en mémoire
          * instruction à utiliser
          * _mm_store...
          */
         _mm_store...
      }
   }
}
