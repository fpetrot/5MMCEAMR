/*******************************************************************************
 * vim:set ts=3:
 * File   : unpack_block.c, file for JPEG-JFIF sequential decoder    
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
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "unpack_block.h"
#include "define_common.h"
#include "utils.h"

/** Tree version of unpack block
 * Once more, enjoy !
 * Finding the right value according to the code is easy :
 * 	_ when you find a '1', you search right child of the current node of the
 * 	tree
 * 	_ when you find a '0', you search left child
 * 	_ once you find an element (ie. is_elt = 1), you stop
 */


uint32_t get_bits(FILE * movie, scan_desc_t * scan_desc,
                                uint8_t number)
{
   int32_t i = 0, newbit = 0;
   uint32_t result = 0;
   uint8_t wwindow = 0, aux = 0;

   if (number == 0)
      return 0;

   for (i = 0; i < number; i++) {
      if (scan_desc->bit_count == 0) {
         NEXT_TOKEN(wwindow);
         scan_desc->bit_count = 8;
         if (wwindow == 0xFF)
            NEXT_TOKEN(aux);
      } else
         wwindow = scan_desc->window;

      newbit = (wwindow >> 7) & 1;
      scan_desc->window = wwindow << 1;
      scan_desc->bit_count -= 1;
      result = (result << 1) | newbit;
   }

   return result;
}

uint8_t get_symbol(FILE * movie, scan_desc_t * scan_desc,
                                 uint32_t acdc, uint32_t component)
{
   uint8_t temp = 0;
   uint32_t length = 0;
   huff_table_t *HT = scan_desc->table[acdc][component];

   for (length = 0; length < 16; length++) {
      temp = get_bits(movie, scan_desc, 1);
 //     code = (2 * code) | (temp & 0x1);
		if ((temp & 0x1) == 0x0) {
			HT = HT->left ;
		} else {
			HT = HT->right ;
		}
		if (HT == NULL) {
			fprintf(stderr, "Found unrecognized code word, application will segfault...\n") ;
			return 0 ;
		}
		if (HT->is_elt == 1) {
//			if (HT->code == code) {
				return HT->value ;
//			} else {
//				break ;
//			}
		}

	}

   return 0;
}

void unpack_block(FILE *movie, scan_desc_t *scan_desc,
                                uint32_t index, int32_t T[64]) 
{
   uint32_t temp = 0, i = 0, run = 0, cat = 0;
   int32_t value = 0;
   uint8_t symbol = 0;

   memset((void *) T, 0, 64 * sizeof(int32_t));
   symbol = get_symbol(movie, scan_desc, HUFF_DC, index);
   temp = get_bits(movie, scan_desc, symbol);

   value = reformat(temp, symbol);
   value += scan_desc->pred[index];
   scan_desc->pred[index] = value;

   T[0] = value;

   for (i = 1; i < 64; i++) {
      symbol = get_symbol(movie, scan_desc, HUFF_AC, index);

      if (symbol == HUFF_EOB) {
         break;
      }
      if (symbol == HUFF_ZRL) {
			i += 15;
         continue;
      }

      cat = symbol & 0xf;
      run = (symbol >> 4) & 0xf;
      i += run;
      temp = get_bits(movie, scan_desc, cat);
      value = reformat(temp, cat);
      T[i] = value;
   }
}

