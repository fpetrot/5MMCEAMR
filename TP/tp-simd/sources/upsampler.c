/*******************************************************************************
 * vim:set ts=3:
 * File   : upsampler.c, file for JPEG-JFIF sequential decoder    
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
#include "upsampler.h"
#include "define_common.h"


/**
 * prend les données sous échantillonnées de MCU_ds, et fait le formatage
 * nécessaiire pour pouvoir afficher le résultat
 * Le résultat est stocké dans MCU_us
 * 	*h_factor : facteur de sous échantillonage horizontal pour le bloc (si 1 :
 * 	pas de sous échantillonage, si 2, 1valeur pour 2 pixels adjacents, etc...)
 * 	*v_factor : facteur de sous échantiollonage vertical
 * 	*nb_MCU_H : nombre de blocs 8x8 en largeur (en sortie !)
 * 	*nb_MCU_V : nombre de blocs 8x8 en hauteur (en sortie toujours)
 */
void upsampler(uint8_t *MCU_ds, uint8_t *MCU_us, uint8_t h_factor, uint8_t v_factor, uint16_t nb_MCU_H, uint16_t nb_MCU_V) {

	int us_index = 0, ds_index = 0 , index = 0;
	int base_index = 0 ;

	// 	_ h_factor > 1, v_factor = 1 : 422 classique
	// 	_ h_factor = 1, v_factor > 1 : non testé
	// 	_ h_factor > 1, v_factor > 1 : 420 
	// 	_ h_factor = 1, v_factor = 1 : 444, ou Y
	if (h_factor > 1) {
		// Il faut étendre les données
		// On parcourt le buffer d'entrée, et on double les données en sorti
		// la condition d'arrêt concerne la sortie (l'entrée n'est pas remplie)
		// us_index : indice pour le buffer de sortie
		// ds_index : indice pour le buffer d'entrée
		// index : indice générique pour la duplication
		int stop_cond = 64 * nb_MCU_H * nb_MCU_H ;
		ds_index = 0 ;
		us_index = 0 ;
		// tant qu'on n'a pas rempli le buffer de sorti
		while (us_index < stop_cond) {
			// on boucle sur le facteur de sous-échantillonage, pour dupliquer
			// h_factor fois
			// A chaque fois, on incrémente us_index, ds_index est constant
			for (index = 0 ; index < h_factor ; index++) {
				MCU_us[us_index] = MCU_ds[ds_index] ;
				us_index++ ;
			}
			// les données sont dupliquées, on passe à la donnée suivante
			ds_index++ ;
		}
		// Une fois le résultat obtenu, si il faut dupliquer en vertical, on
		// recopie le résultat dans MCU_ds
		if (v_factor > 1) {
			memcpy(MCU_ds, MCU_us, nb_MCU_V * nb_MCU_H * 64) ;
		}
	}
	if (v_factor > 1) {
		// le principe est le même sauf qu'on duplique les lignes plutot que les
		// colonnes
		int stop_cond = 64 * nb_MCU_H * nb_MCU_H ;
		ds_index = 0 ;
		us_index = 0 ;
		// on parcourt le buffer de sortie
		while (us_index < stop_cond) {
			//pour chaque ligne lue, on la duplique v_factor fois
			// A ce moment la, une ligne a forcément comme longueur MCU_sx *
			// nb_MCU_H
			for (index = 0 ; index < v_factor ; index++) {
				memcpy(MCU_us + us_index, MCU_ds + ds_index , 8 * nb_MCU_H) ;
				us_index += 8 * nb_MCU_H ;
			}
			// on incrémente l'indice d'entrée
			ds_index += 8 * nb_MCU_H ;
		}
	} 
	if ((v_factor == 1) && (h_factor == 1)) {
		// on ne reformate que si aucun sous échantillonage a été testé ( ca doit
		// pouvoir poser problème dans le cas d'échantillonage bizarre...)
		// le reformatage est nécessaire pour la suite : le bloc obtenu après
		// upsampler correspond à une MCU de taille 8*nb_MCU_Hx8*nb_MCU_V
		// la récupération des MCUs, par contre, nous donne des blocs indépendants
		// (le premier bloc prend les premières adresses, etc). On doit donc
		// reformater pour que les premières adresses correspondent à la première
		// ligne du bloc final
		int stop_cond = 64 * nb_MCU_H * nb_MCU_H ;
		ds_index = 0 ;
		us_index = 0 ;
		// on parcourt le bloc de sortie, le fonctionnement reste identique aux
		// cas précédents
		while (us_index < stop_cond) {
			//
			int increment = ((64 * nb_MCU_H) - 8) ;
			for (index = 0 ; index < nb_MCU_H ; index ++) {
				memcpy(MCU_us+us_index, MCU_ds+base_index, 8) ;
				us_index += 8 ;
				base_index += 64 ;
			}
			base_index -= increment	;
			if (base_index == 64) {
				base_index = 64 * nb_MCU_H ;
			}
		}
	} 
}

