/*******************************************************************************
 * vim:set ts=3:
 * File   : screen.c, file for JPEG-JFIF sequential decoder    
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
#include "stdio.h"
#include "stdlib.h"
#include "SDL/SDL.h"
#include "define_common.h"

static SDL_Surface *screen;
static SDL_Surface *image;
// Frame rate management, stores previous counter value
static int old_time ;
// According to when the quit event is detected, the player can get stuck
// the "initialized" parameter prevents this
static int initialized ;


void screen_init(uint32_t width, uint32_t height)
{

	int width_int = width , height_int = height ;
   /* Initialize defaults, Video and Audio */
   if ((SDL_Init(SDL_INIT_VIDEO) == -1)) { 
      printf("Could not initialize SDL: %s.\n", SDL_GetError());
      exit(-1);
   }

   /* Initialize the SDL library */
   if (SDL_Init(SDL_INIT_VIDEO) < 0)  {
      fprintf(stderr,
            "Couldn't initialize SDL: %s\n", SDL_GetError());
      exit(1);
   }

   /* Clean up on exit */
   atexit(SDL_Quit);

   screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
   if (screen == NULL) {
      fprintf(stderr, "Couldn't set %dx%dx32 video mode for screen: %s\n", width,height,
            SDL_GetError());
      exit(1);
   }
	if ((width % 8) != 0) {
		width_int = ((width / 8) + 1) * 8 ;
	}
	if ((height % 8) != 0) {
		height_int = ((height / 8) + 1) * 8 ;
	}

		IPRINTF("initializing image buffer with w=%d, h=%d\n", width_int, height_int) ;
		image = SDL_SetVideoMode(width_int, height_int, 32, SDL_SWSURFACE);
   if (image == NULL) {
      fprintf(stderr, "Couldn't set %dx%dx32 video mode for image: %s\n", width,height,
            SDL_GetError());
      exit(1);
   }
	old_time = SDL_GetTicks() ;
	initialized	= 1 ;
}

int screen_exit()
{
   /* Shutdown all subsystems */
	SDL_Event event ;
	while(initialized) {
		SDL_PollEvent(&event) ;
		if ((event.type == SDL_QUIT )) {
			SDL_Quit();
			return 1;
		}
	}
	return 0 ;
}

void screen_cpyrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, void *ptr)
{
   void *dest_ptr;
   void *src_ptr;
   uint32_t line;
   
   SDL_LockSurface(image);

   for(line = 0; line < h ; line++)
   {
      dest_ptr = (void*)((uintptr_t)image->pixels + (((x+line)*image->w + y) << 2));
      src_ptr = (void*)((uintptr_t)ptr + ((line * w) << 2));
      memcpy(dest_ptr,src_ptr,w << 2);
   }

   SDL_UnlockSurface(image);
}

int screen_refresh() 
{
	SDL_Rect offset ;
	uint64_t new_time, finish_time ;
	SDL_Event event ;
	offset.x = 0 ;
	offset.y = 0 ;
	SDL_BlitSurface (image, &offset, screen, &offset) ;
	new_time = SDL_GetTicks() ;
	finish_time = SDL_GetTicks() ;
	while (new_time - old_time < 1000 / 25) {
		new_time = SDL_GetTicks() ;
	}
	if (SDL_Flip(screen) == -1) {
		printf("Could not refresh screen: %s\n.", SDL_GetError() );
	}
	IPRINTF("[screen]: instantaneous fps is %0.2f\n", 1000.00f / (SDL_GetTicks() - old_time)) ;
	printf("[screen] : framerate is %0.2ffps, computed one image in %0.2fms\n", 1000.00f / (SDL_GetTicks() - old_time), (finish_time - old_time) * 1.00f) ;
	old_time = SDL_GetTicks() ;
	// In this case, SDL is 
	if(SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			initialized = 0 ;
			SDL_Quit();
			return 1 ;
		}
	}
	return 0 ;
}



