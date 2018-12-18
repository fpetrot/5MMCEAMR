/*******************************************************************************
 * vim:set ts=3:
 * File   : idct.c, file for JPEG-JFIF sequential decoder    
 *
 * Copyright (C) 2007 TIMA Laboratory
 * Author(s) :      Patrice, GERIN patrice.gerin@imag.fr
 * Bug Fixer(s) :   Xavier, GUERIN xavier.guerin@imag.fr
 * 					  Pierre-Henri HORREIN pierre-henri.horrein@imag.fr
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
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "idct.h"
#if 1


/*
 * Minimum and maximum values a `signed int' can hold.  
 */

#define  IDCT_INT_MIN   (- IDCT_INT_MAX - 1)
#define  IDCT_INT_MAX   2147483647

/*
 * Useful constants: 
 */

/*
 * ck = cos(k*pi/16) = s8-k = sin((8-k)*pi/16) times 1 << C_BITS and
 * rounded 
 */
#define c0_1  16384
#define c0_s2 23170
#define c1_1  16069
#define c1_s2 22725
#define c2_1  15137
#define c2_s2 21407
#define c3_1  13623
#define c3_s2 19266
#define c4_1  11585
#define c4_s2 16384
#define c5_1  9102
#define c5_s2 12873
#define c6_1  6270
#define c6_s2 8867
#define c7_1  3196
#define c7_s2 4520
#define c8_1  0
#define c8_s2 0
#define sqrt2 c0_s2

#define Y(i,j)          Y[(i << 3) + j]
#define X(i,j)          (output[(i << 3) + j])

/*
 * The number of bits of accuracy in all (signed) integer operations: May
 * lie between 1 and 32 (bounds inclusive). 
 */

#define ARITH_BITS      16

/*
 * The minimum signed integer value that fits in ARITH_BITS: 
 */

#define ARITH_MIN       (-1 << (ARITH_BITS-1))

/*
 * The maximum signed integer value that fits in ARITH_BITS: 
 */

#define ARITH_MAX       (~ARITH_MIN)

/*
 * The number of bits coefficients are scaled up before 2-D IDCT: 
 */

#define S_BITS           3

/*
 * The number of bits in the fractional part of a fixed point constant: 
 */

#define C_BITS          14

#define SCALE(x,n)      ((x) << (n))

/*
 * Butterfly: but(a,b,x,y) = rot(sqrt(2),4,a,b,x,y) 
 */

#define but(a,b,x,y)    { x = SUB(a,b); y = ADD(a,b); }

/*
 * This version is vital in passing overall mean error test. 
 */

static inline int32_t DESCALE (int32_t x, int32_t n)
{
   return (x + (1 << (n - 1)) - (x < 0)) >> n;
}

/*
 * Maximum and minimum intermediate int values: 
 */

static inline int32_t ADD(int32_t x, int32_t y)
{
   int32_t r = x + y;

   return r;         /* in effect: & 0x0000FFFF */
}

static inline int32_t SUB(int32_t x, int32_t y)
{
   int32_t r = x - y;

   return r;         /* in effect: & 0x0000FFFF */
}

static inline int32_t CMUL(int32_t c, int32_t x)
{
   int32_t r = c * x;
   
   /*
    * Less accurate rounding here also works fine: 
    */
   
   r = (r + (1 << (C_BITS - 1))) >> C_BITS;
   return r;
}

/*
 * Rotate (x,y) over angle k*pi/16 (counter-clockwise) and scale with f. 
 */

static inline void rot(int32_t f, int32_t k, int32_t x, int32_t y, int32_t *rx, int32_t *ry) {
   int32_t COS[2][8] = {
      {c0_1, c1_1, c2_1, c3_1, c4_1, c5_1, c6_1, c7_1},
      {c0_s2, c1_s2, c2_s2, c3_s2, c4_s2, c5_s2, c6_s2, c7_s2}
   };

   *rx = SUB(CMUL(COS[f][k], x), CMUL(COS[f][8 - k], y));
   *ry = ADD(CMUL(COS[f][8 - k], x), CMUL(COS[f][k], y));
}

/*
 * Inverse 1-D Discrete Cosine Transform. Result Y is scaled up by factor
 * sqrt(8). Original Loeffler algorithm. 
 */

static inline void idct_1d(int32_t *Y) {
   int32_t z1[8], z2[8], z3[8];

   /*
    * Stage 1: 
    */

   but(Y[0], Y[4], z1[1], z1[0]);
   rot(1, 6, Y[2], Y[6], &z1[2], &z1[3]);
   but(Y[1], Y[7], z1[4], z1[7]);
   z1[5] = CMUL(sqrt2, Y[3]);
   z1[6] = CMUL(sqrt2, Y[5]);

   /*
    * Stage 2: 
    */
   but(z1[0], z1[3], z2[3], z2[0]);
   but(z1[1], z1[2], z2[2], z2[1]);
   but(z1[4], z1[6], z2[6], z2[4]);
   but(z1[7], z1[5], z2[5], z2[7]);

   /*
    * Stage 3: 
    */
   z3[0] = z2[0];
   z3[1] = z2[1];
   z3[2] = z2[2];
   z3[3] = z2[3];
   rot(0, 3, z2[4], z2[7], &z3[4], &z3[7]);
   rot(0, 1, z2[5], z2[6], &z3[5], &z3[6]);

   /*
    * Final stage 4: 
    */
   but(z3[0], z3[7], Y[7], Y[0]);
   but(z3[1], z3[6], Y[6], Y[1]);
   but(z3[2], z3[5], Y[5], Y[2]);
   but(z3[3], z3[4], Y[4], Y[3]);
}

/*
 * Inverse 2-D Discrete Cosine Transform. 
 */

void IDCT(int32_t *input, uint8_t *output) {
   int32_t Y[64];
   int32_t k, l;

   for (k = 0; k < 8; k++) {  
      for (l = 0; l < 8; l++) Y(k, l) = SCALE(input[(k << 3) + l], S_BITS);
      idct_1d(&Y(k, 0));
   }

   for (l = 0; l < 8; l++) {
      int32_t Yc[8];

      for (k = 0; k < 8; k++) Yc[k] = Y(k, l);
      
      idct_1d(Yc);
      
      for (k = 0; k < 8; k++) {
         int32_t r = 128 + DESCALE(Yc[k], S_BITS + 3);
				 r = r > 0 ? (r < 255 ? r : 255) : 0;
         X(k, l) = r;
      }
   }
}
#else
#if 0
/**
 * Formule de la transformée en cosinus discrete inverse pour les
 * macroblocs de taille $n\times n$ :
 * pixel(x, y) = \frac{1}{\sqrt{2n}}
 * \sum_{u = 0}^{n-1}\sum_{v = 0}^{n-1}
 * C(u) C(v) cos(\frac{2x+1}u\pi}{2n}) cos(\frac{2y+1}v\pi}{2n})
 * frequence(u,v)
 *
 * C(k) = 1/sqrt(2) si k = 0, 1 sinon
 *
 * x et y sont les coordonnées des pixels dans le domaine spatiale et
 * u et v sont les coordonnées des frequences dans le domaine
 * fréquentiel
 */
#include <math.h>
#define M_PI_16	3.14159265358979323846/16	/* pi/16 */
#define M_SQRT2_8	1.41421356237309504880/8	/* sqrt(2)/8 */

void IDCT(int32_t *input, uint8_t *output) {
   int32_t x, y, u, v;
   double  d;

   for (y = 0; y < 8; y++) {
      for (x = 0; x < 8; x++) {
         u = v = 0;
         d = *(input+0+8*0)
             *cos((2*x+1)*u*M_PI_16)
             *cos((2*y+1)*v*M_PI_16)/8;
         v = 0;
         for (u = 1; u < 8; u++) {
            d +=*(input+v+8*u)
                *cos((2*x+1)*u*M_PI_16)
                *cos((2*y+1)*v*M_PI_16)*M_SQRT2_8;
         }
         u = 0;
         for (v = 1; v < 8; v++) {
            d +=*(input+v+8*u)
                *cos((2*x+1)*u*M_PI_16)
                *cos((2*y+1)*v*M_PI_16)*M_SQRT2_8;
         }
         for (v = 1; v < 8; v++) {
            for (u = 1; u < 8; u++) {
               d +=*(input+v+8*u)
                   *cos((2*x+1)*u*M_PI_16)
                   *cos((2*y+1)*v*M_PI_16)/4;
            }
         }
         d += 128.0;
         *(output+y+x*8) = d < 0 ? (uint8_t)0
                                 : d > 255.0 ? (uint8_t)255
                                             : (uint8_t)d;
      }
   }
}
#else

/*
 * Calcul d'une IDCT 2D en 2 IDCT 1D sur les lignes et les colonnes.
 * (Ici en flottant simple précision, 12 multiplications par IDCT 1D)
 * Factorisation utilisée ici (Normalisée avec c4=1): 
 *
 *   // Partie gauche
 *   f0 = (x0+x4);
 *   f1 = (x0-x4);
 *   f2 = c6*(x2+x6) + (c2-c6)*x2;
 *   f3 = c6*(x2+x6) + (-c2-c6)*x6;
 *
 *   // Partie droite
 *   e0 = c3*(x1+x3+x5+x7) + ( c5 - c3)*(x1+x5) + ( c1+c3-c5-c7)*x1 + ( c7 - c3)*(x1+x7);
 *   e1 = c3*(x1+x3+x5+x7) + (-c5 - c3)*(x3+x7) + ( c1+c3+c5-c7)*x3 + (-c1 - c3)*(x3+x5);
 *   e2 = c3*(x1+x3+x5+x7) + ( c5 - c3)*(x1+x5) + ( c1+c3-c5+c7)*x5 + (-c1 - c3)*(x3+x5);
 *   e3 = c3*(x1+x3+x5+x7) + (-c5 - c3)*(x3+x7) + (-c1+c3+c5-c7)*x7 + ( c7 - c3)*(x1+x7); 
 *
 *   out[0] = f0 + f2 + e0;
 *   out[1] = f1 + f3 + e1;
 *   out[2] = f1 - f3 + e2;
 *   out[3] = f0 - f2 + e3;
 *   out[4] = f0 - f2 - e3;
 *   out[5] = f1 - f3 - e2;
 *   out[6] = f1 + f3 - e1;
 *   out[7] = f0 + f2 - e0;
 */
 
#define CLAMP(i) if (i & 0xFF00) i = (((~i) >> 15) & 0xFF);
 
// Normalised to c4=1
#define c3   1.175875602f
#define c6   0.541196100f
 
#define k1   0.765366865f   //  c2-c6
#define k2  -1.847759065f   // -c2-c6
#define k3  -0.390180644f   //  c5-c3
#define k4  -1.961570561f   // -c5-c3
#define k5   1.501321110f   //  c1+c3-c5-c7
#define k6   2.053119869f   //  c1+c3-c5+c7
#define k7   3.072711027f   //  c1+c3+c5-c7
#define k8   0.298631336f   // -c1+c3+c5-c7
#define k9  -0.899976223f   //  c7-c3
#define k10 -2.562915448f   // -c1-c3
 
void IDCT(int32_t *input, uint8_t *output) {
{
  float x0,x1,x2,x3,x4,x5,x6,x7;
  float e0,e1,e2,e3;
  float f0,f1,f2,f3;
  float x26,x1357,x15,x37,x17,x35;
  float out[64];
  short v;
 
  // Lignes
  for(int i=0;i<8;i++) {
 
    x0 = (float)block[0+i*8];    x1 = (float)block[1+i*8];
    x2 = (float)block[2+i*8];    x3 = (float)block[3+i*8];
    x4 = (float)block[4+i*8];    x5 = (float)block[5+i*8];
    x6 = (float)block[6+i*8];    x7 = (float)block[7+i*8];
 
    // Partie gauche
    f0 = (x0+x4);
    f1 = (x0-x4);
    x26 = c6*(x2+x6);
    f2 = x26 + k1*x2;
    f3 = x26 + k2*x6;
 
    // Partie droite
    x1357 = c3*(x1+x3+x5+x7);
    x15 = k3*(x1+x5);
    x37 = k4*(x3+x7);
    x17 = k9*(x1+x7);
    x35 = k10*(x3+x5);
 
    e0 = x1357 + x15 + k5*x1 + x17;
    e1 = x1357 + x37 + k7*x3 + x35;
    e2 = x1357 + x15 + k6*x5 + x35;
    e3 = x1357 + x37 + k8*x7 + x17;
 
    out[0+i*8] = f0 + f2 + e0;
    out[1+i*8] = f1 + f3 + e1;
    out[2+i*8] = f1 - f3 + e2;
    out[3+i*8] = f0 - f2 + e3;
    out[7+i*8] = f0 + f2 - e0;
    out[6+i*8] = f1 + f3 - e1;
    out[5+i*8] = f1 - f3 - e2;
    out[4+i*8] = f0 - f2 - e3;
 
  }
 
  // Colonnes
  for(int i=0;i<8;i++) {
 
    x0 = out[i+8*0];    x1 = out[i+8*1];
    x2 = out[i+8*2];    x3 = out[i+8*3];
    x4 = out[i+8*4];    x5 = out[i+8*5];
    x6 = out[i+8*6];    x7 = out[i+8*7];
 
    // Partie gauche
    f0 = (x0+x4);
    f1 = (x0-x4);
    x26 = c6*(x2+x6);
    f2 = x26 + k1*x2;
    f3 = x26 + k2*x6;
 
    // Partie droite
    x1357 = c3*(x1+x3+x5+x7);
    x15 = k3*(x1+x5);
    x37 = k4*(x3+x7);
    x17 = k9*(x1+x7);
    x35 = k10*(x3+x5);
 
    e0 = x1357 + x15 + k5*x1 + x17;
    e1 = x1357 + x37 + k7*x3 + x35;
    e2 = x1357 + x15 + k6*x5 + x35;
    e3 = x1357 + x37 + k8*x7 + x17;
 
    // Tronque dans l'intervalle [0,255]  
    // Note: La normalisation avec c4=1 implique une division par 8
    // Le +1028 sert à recentrer et à arrondir la valeur du pixel
    // v = (short)((.+.+.)/8.0 + 128.5f);
 
    v = (short)((f0 + f2 + e0) + 1028.0f) >> 3;
    CLAMP(v); dest[i+8*0] = (uchar)v;
    v = (short)((f1 + f3 + e1) + 1028.0f) >> 3;
    CLAMP(v); dest[i+8*1] = (uchar)v;
    v = (short)((f1 - f3 + e2) + 1028.0f) >> 3;
    CLAMP(v); dest[i+8*2] = (uchar)v;
    v = (short)((f0 - f2 + e3) + 1028.0f) >> 3;
    CLAMP(v); dest[i+8*3] = (uchar)v;
    v = (short)((f0 - f2 - e3) + 1028.0f) >> 3;
    CLAMP(v); dest[i+8*4] = (uchar)v;
    v = (short)((f1 - f3 - e2) + 1028.0f) >> 3;
    CLAMP(v); dest[i+8*5] = (uchar)v;
    v = (short)((f1 + f3 - e1) + 1028.0f) >> 3;
    CLAMP(v); dest[i+8*6] = (uchar)v;
    v = (short)((f0 + f2 - e0) + 1028.0f) >> 3;
    CLAMP(v); dest[i+8*7] = (uchar)v;
 
  }
}

#endif
#endif
