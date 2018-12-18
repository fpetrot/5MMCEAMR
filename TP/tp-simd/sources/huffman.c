/*******************************************************************************
 * vim:set ts=3:
 * File   : huffman.c, file for JPEG-JFIF sequential decoder    
 *
 * Copyright (C) 2007 TIMA Laboratory
 * Author(s) : 	  Pierre-Henri HORREIN pierre-henri.horrein@imag.fr
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
#include "huffman.h"
#include "define_common.h"


/*
 * Tree version of huffman tables management
 * Enjoy !
 * expansion is unnecessary, but is still present just in case
 */

void free_huffman_tables(huff_table_t *root) {
	if (root->left != NULL) {
		free_huffman_tables(root->left) ;
	}
	if (root->right != NULL) {
		free_huffman_tables(root->right) ;
	}
	free(root) ;
}

// we need to find the first location at depth depth in the table
huff_table_t *find_location(huff_table_t *root, uint16_t depth) {
	huff_table_t *return_ptr ;
	if (root->is_elt == 0) {
		if (depth == 0) {
			return root ;
		} 
		if (root->left == NULL) {
			root->left = (huff_table_t *)malloc(sizeof(huff_table_t)) ;
			root->left->value = 0 ;	
			root->left->code = 0 ;	
			root->left->is_elt = 0 ;	
			root->left->left = NULL ;
			root->left->right = NULL ;
			root->left->parent = root ;	
		}
		return_ptr = find_location(root->left, depth-1) ;
	   if (return_ptr != NULL) {
			return return_ptr;
		}
		if (root->right == NULL) {
			root->right = (huff_table_t *)malloc(sizeof(huff_table_t)) ;
			root->right->value = 0 ;	
			root->right->code = 0 ;	
			root->right->is_elt = 0 ;	
			root->right->left = NULL ;
			root->right->right = NULL ;
			root->right->parent = root ;	
		}
		return_ptr = find_location(root->right, depth-1) ;
	   if (return_ptr != NULL) {
			return return_ptr;
		}
	} else {
		return NULL ;
	}
	return NULL ;
}

void expand_huff_tree(huff_table_t *root) {
	//parcours de l'arbre
	if (root->left != NULL) {
	  root->left->code = (uint16_t)((root->code * 2) & 0xfffe) & 0xffff	;
	  if (root->left->is_elt == 1) {
	  } else {
		  expand_huff_tree(root->left) ;
	  }
	}
	if (root->right != NULL) {
	  root->right->code = (uint16_t)((root->code * 2) | 0x0001) & 0xffff	;
	  if (root->right->is_elt == 1) {
	  } else {
		  expand_huff_tree(root->right) ;
	  }
	}
}



int load_huffman_table(FILE * movie,
		uint16_t DHT_section_length,
		uint8_t DHT_section_info,
		huff_table_t * ht)
{
	uint8_t buffer = 0;
	int32_t size = 0, i = 0, j = 0;
	uint8_t num_values[16] ;
	huff_table_t *current_ptr;
	// ht est la racine de l'arbre
	// rien d'autre n'est alloué !!!
	
	// récupération des longueurs :
	ht->code = 0 ;
	ht->value = 0 ;
	ht->is_elt = 0 ;
	ht->parent = NULL ;
	ht->left = NULL ;
	ht->right = NULL ;

	for (i=0 ; i < 16 ; i++) {
		NEXT_TOKEN(buffer) ;
		num_values[i] = buffer ;
	}
	size = 16 ;

	for (i = 0 ; i < 16 ; i++) {
		for (j = 0 ; j < num_values[i] ; j++) {
			current_ptr = find_location(ht, i+1) ;
			if(current_ptr == NULL) {
				printf("Ooops ! No location found, exiting...\n") ;
				return -1 ;
			}
			size++;
			NEXT_TOKEN(buffer);
			current_ptr->value = buffer ;
			current_ptr->is_elt = 1 ;
		}
	}
	// arbre construit : expansion

//	expand_huff_tree(ht) ;

	return (size) ;
}

