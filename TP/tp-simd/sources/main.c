/*******************************************************************************
 * vim:set ts=3:
 * File   : main.c, file for JPEG-JFIF sequential decoder    
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
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>

#include "jpeg.h"
#include "utils.h"
#include "screen.h"
#include "errno.h"
#include "define_common.h"
#include "iqzz.h"
#include "unpack_block.h"
#include "idct.h"
#include "conv.h"
#include "upsampler.h"
#include "huffman.h"
#include "skip_segment.h"



int main(int argc, char *argv[])
{
   uint8_t marker[2];
   uint8_t HT_type = 0;
   uint8_t HT_index = 0;
   uint8_t DQT_table[4][64];
   uint8_t QT_index = 0;
   uint16_t nb_MCU = 0, nb_MCU_X = 0, nb_MCU_Y = 0;
	uint16_t max_ss_h = 0 , max_ss_v = 0 ;
   uint8_t index = 0, index_X, index_Y;
   int32_t MCU[64];
   int32_t unZZ_MCU[64];
   uint8_t *YCrCb_MCU[3];
   uint8_t *YCrCb_MCU_ds[3];
   uint32_t *RGB_MCU = NULL;
   uint32_t YH = 0, YV = 0;
#ifdef UPS_DEBUG
   uint32_t CrH = 0, CrV = 0, CbH = 0, CbV = 0 ;
#endif
   uint32_t screen_init_needed;
   bool no_screen = false;
   uint32_t end_of_file;
	int chroma_ss ;
   uint32_t nb_frames = UINT32_MAX;

   FILE *movie;
   jfif_header_t jfif_header;
   DQT_section_t DQT_section;
   SOF_section_t SOF_section;
   SOF_component_t SOF_component[3];
   DHT_section_t DHT_section;
   SOS_section_t SOS_section;
   SOS_component_t SOS_component[3];
   scan_desc_t scan_desc = { 0, 0, {}, {} };
   huff_table_t *tables[2][4];

   screen_init_needed = 1;

   if (argc != 2 && argc != 3 && argc != 4) {
      fprintf(stderr, "Usage : %s <mjpeg file> [nb of frames [no screen]]\n", argv[0]);
      exit(-1);
   }

   if ((movie = fopen(argv[1], "r")) == NULL) {
      perror(strerror(errno));
      exit(-1);
   }

   if (argc == 3 || argc == 4)
      nb_frames = strtoul(argv[2], NULL, 0);

   if (argc == 4)
      no_screen = true;


   for (HT_index = 0; HT_index < 4; HT_index++) {
      tables[HUFF_DC][HT_index] = (huff_table_t *) calloc(1, sizeof(huff_table_t));
      tables[HUFF_AC][HT_index] = (huff_table_t *) calloc(1, sizeof(huff_table_t));
   }

        /*---- Actual computation ----*/
   end_of_file = 0;
   while (!end_of_file) {
      COPY_SECTION(&marker, 2);

      if (marker[0] == M_SMS) {
         switch (marker[1]) {
         case M_SOF0:{
               if (nb_frames == 0)
                  exit(0);
               else
                  nb_frames--;

               IPRINTF("SOF0 marker found for frame %u\r\n", nb_frames);

               COPY_SECTION(&SOF_section, sizeof(SOF_section));
               CPU_DATA_IS_BIGENDIAN(16, SOF_section.length);
               CPU_DATA_IS_BIGENDIAN(16, SOF_section.height);
               CPU_DATA_IS_BIGENDIAN(16, SOF_section.width);

               VPRINTF("Data precision = %d\r\n",
                       SOF_section.data_precision);
               VPRINTF("Image height = %d\r\n", SOF_section.height);
               VPRINTF("Image width = %d\r\n", SOF_section.width);
               VPRINTF("%d component%c\r\n",
                       SOF_section.n, (SOF_section.n > 1) ? 's' : ' ');

               COPY_SECTION(&SOF_component,
                            sizeof(SOF_component_t) * SOF_section.n);

					for (index = 0 ; index < SOF_section.n ; index++) {
						if ((SOF_component[0].HV & 0x0f) > max_ss_v) {
							max_ss_v = SOF_component[0].HV & 0x0f ;
						}
						if (((SOF_component[0].HV >> 4)& 0x0f) > max_ss_h) {
							max_ss_h = (SOF_component[0].HV >> 4) & 0x0f ;
						}
					}
               YV = SOF_component[0].HV & 0x0f;
               YH = (SOF_component[0].HV >> 4) & 0x0f;
#ifdef UPS_DEBUG
               CrV = SOF_component[1].HV & 0x0f;
               CrH = (SOF_component[1].HV >> 4) & 0x0f;
               CbV = SOF_component[2].HV & 0x0f;
               CbH = (SOF_component[2].HV >> 4) & 0x0f;

               IPRINTF("Subsampling factor = %ux%u, %ux%u, %ux%u\r\n", YH, YV, CrH, CrV, CbH, CbV);
#endif

               if ((YH != 1) || (YV != 1)) {
                  fprintf(stderr, "Subsampling not supported ...\n");
                 // exit(0);
               }

               if (screen_init_needed == 1) {
                  screen_init_needed = 0;
                  if (!no_screen) {
                     screen_init(SOF_section.width, SOF_section.height);
                  }

                  nb_MCU_X = intceil(SOF_section.height, MCU_sx * max_ss_v);
                  nb_MCU_Y = intceil(SOF_section.width, MCU_sy * max_ss_h);
                  nb_MCU = nb_MCU_X * nb_MCU_Y;
						for (index = 0 ; index < SOF_section.n ; index++) {
							YCrCb_MCU[index] = malloc(MCU_sx * MCU_sy * max_ss_h * max_ss_v) ;
							YCrCb_MCU_ds[index] = malloc(MCU_sx * MCU_sy * max_ss_h * max_ss_v) ;
						}
						RGB_MCU = malloc (MCU_sx * MCU_sy * max_ss_h * max_ss_v * sizeof(int32_t)) ;
               }

               break;
            }

         case M_DHT:{
					// Depending on how the JPEG is encoded, DHT marker may not be
					// repeated for each DHT. We need to take care of the general
					// state where all the tables are stored sequentially
					// DHT size represent the currently read data... it starts as a
					// zero value
					int DHT_size = 0 ;
               IPRINTF("DHT marker found\r\n");

               COPY_SECTION(&DHT_section.length, 2);
               CPU_DATA_IS_BIGENDIAN(16, DHT_section.length);
					// We've read the size : DHT_size is now 2
					DHT_size += 2 ;

					//We loop while we've not read all the data of DHT section
					while (DHT_size < DHT_section.length) {

						// read huffman info, DHT size ++
						NEXT_TOKEN(DHT_section.huff_info) ;
						DHT_size++ ;

						// load the current huffman table
						HT_index = DHT_section.huff_info & 0x0f;
						HT_type = (DHT_section.huff_info >> 4) & 0x01;

						IPRINTF("Huffman table index is %d\r\n", HT_index);
						IPRINTF("Huffman table type is %s\r\n",
								HT_type ? "AC" : "DC");

						VPRINTF("Loading Huffman table\r\n");
						DHT_size += load_huffman_table(movie, DHT_section.length, DHT_section.huff_info,	tables[HT_type] [HT_index]);
						IPRINTF("Huffman table length is %d, read %d\r\n", DHT_section.length, DHT_size);
					}

               break;
            }

         case M_SOI:{
               IPRINTF("SOI marker found\r\n");
               break;
            }

         case M_EOI:{
               IPRINTF("EOI marker found\r\n");
					for (index = 0 ; index < SOF_section.n ; index ++) {
						free(YCrCb_MCU[index]) ;
						free(YCrCb_MCU_ds[index]) ;
					}
					free(RGB_MCU) ;
					for (HT_index = 0; HT_index < 4; HT_index++) {
						free_huffman_tables(tables[HUFF_DC][HT_index]) ;
						free_huffman_tables(tables[HUFF_AC][HT_index]) ;
					}
               end_of_file = 1;
               break;
            }

         case M_SOS:{
               IPRINTF("SOS marker found\r\n");

               COPY_SECTION(&SOS_section, sizeof(SOS_section));
               CPU_DATA_IS_BIGENDIAN(16, SOS_section.length);

               COPY_SECTION(&SOS_component,
                            sizeof(SOS_component_t) * SOS_section.n);
               IPRINTF("Scan with %d components\r\n", SOS_section.n);
					
					// bug ?
               //SKIP(SOS_section.n);
					SKIP(3) ;

               scan_desc.bit_count = 0;
               for (index = 0; index < SOS_section.n; index++) {
                  scan_desc.pred[index] = 0;
                  scan_desc.table[HUFF_AC][index] =
                      tables[HUFF_AC][(SOS_component[index].
                                        acdc >> 4) & 0x0f];
                  scan_desc.table[HUFF_DC][index] =
                      tables[HUFF_DC][SOS_component[index].acdc & 0x0f];
               }

               for (index_X = 0; index_X < nb_MCU_X; index_X++) {
                  for (index_Y = 0; index_Y < nb_MCU_Y; index_Y++) {
                     for (index = 0; index < SOF_section.n; index++) {
								//avoiding unneeded computation
								nb_MCU = ((SOF_component[index].HV>> 4) & 0xf) * (SOF_component[index].HV & 0x0f) ; 
								for (chroma_ss = 0; chroma_ss < nb_MCU; chroma_ss++) {
									
									unpack_block(movie, &scan_desc,
											index, MCU) ;
									iqzz_block(MCU, unZZ_MCU,
											DQT_table[SOF_component[index].
											q_table]);
									IDCT(unZZ_MCU, YCrCb_MCU_ds[index] + (64 * chroma_ss));
								}
								upsampler(YCrCb_MCU_ds[index], YCrCb_MCU[index],  
										max_ss_h / ((SOF_component[index].HV >> 4) & 0xf), 
										max_ss_v / ((SOF_component[index].HV) & 0xf),
										max_ss_h,
										max_ss_v) ;
                     }

                     YCrCb_to_ARGB(YCrCb_MCU, RGB_MCU, max_ss_h, max_ss_v);

                     if (!no_screen) {
                        screen_cpyrect
                            (index_X * MCU_sx * max_ss_v,
                             index_Y * MCU_sy * max_ss_h, 
                             MCU_sy * max_ss_h, 
                             MCU_sx * max_ss_v, 
                             RGB_MCU);
                     }
                  }
               }
					
               if (!no_screen) {
                  if (screen_refresh() == 1) {
                     end_of_file = 1 ;
                  }
               }

               COPY_SECTION(&marker, 2);

               break;
            }

         case M_DQT:{
					int DQT_size = 0;
               IPRINTF("DQT marker found\r\n");

               COPY_SECTION(&DQT_section.length, 2);
               CPU_DATA_IS_BIGENDIAN(16, DQT_section.length);
					DQT_size += 2 ;

					while (DQT_size < DQT_section.length) {

						NEXT_TOKEN(DQT_section.pre_quant) ;
						DQT_size += 1 ;
						IPRINTF
							("Quantization precision is %s\r\n",
							 ((DQT_section.
								pre_quant >> 4) & 0x0f) ? "16 bits" : "8 bits");

						QT_index = DQT_section.pre_quant & 0x0f;
						IPRINTF("Quantization table index is %d\r\n", QT_index);

						IPRINTF("Reading quantization table\r\n");
						COPY_SECTION(DQT_table[QT_index], 64);
						DQT_size += 64 ;

					}

               break;
            }

         case M_APP0:{
               IPRINTF("APP0 marker found\r\n");

               COPY_SECTION(&jfif_header, sizeof(jfif_header));
               CPU_DATA_IS_BIGENDIAN(16, jfif_header.length);
               CPU_DATA_IS_BIGENDIAN(16, jfif_header.xdensity);
               CPU_DATA_IS_BIGENDIAN(16, jfif_header.ydensity);

               if (jfif_header.identifier[0] != 'J'
                   || jfif_header.identifier[1] != 'F'
                   || jfif_header.identifier[2] != 'I'
                   || jfif_header.identifier[3] != 'F') {
                  VPRINTF("Not a JFIF file\r\n");
               }

               break;
            }

			case M_COM:{
					IPRINTF("COM marker found\r\n") ;
					uint16_t length ;
					char * comment ;
					COPY_SECTION(&length, 2) ;
					CPU_DATA_IS_BIGENDIAN(16, length) ;
					comment = (char *) malloc (length - 2) ;
					COPY_SECTION(comment, length - 2) ;
					IPRINTF("Comment found : %s\r\n", comment) ;
					free(comment) ;
						  }

					break ;




			default:{
               IPRINTF("Unsupported token: 0x%x\r\n", marker[1]);
               skip_segment(movie);
               break;
            }
         }
      } else
			VPRINTF("Invalid marker, expected SMS (0xff), got 0x%lx (second byte is 0x%lx) \n", marker[0], marker[1]);
   }

   if (!no_screen) {
      screen_exit() ;
   }
   return 0;
}
