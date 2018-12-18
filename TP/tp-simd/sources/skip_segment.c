/*******************************************************************************
 * vim:set ts=3:
 * File   : skip_segment.c, file for JPEG-JFIF sequential decoder    
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
#include "skip_segment.h"
#include "define_common.h"

void skip_segment(FILE * movie)
{
   union {
      uint16_t segment_size;
      uint8_t size[2];
   } u;

   NEXT_TOKEN(u.size[0]);
   NEXT_TOKEN(u.size[1]);
   CPU_DATA_IS_BIGENDIAN(16, u.segment_size);

   IPRINTF("Skip segment (%d data)\r\n", (unsigned int) u.segment_size);
   SKIP(u.segment_size - 2);
}

