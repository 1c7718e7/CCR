/* This file is part of CCR.
 * Copyright (C) 2018  Martin Shirokov
 * 
 * CCR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * CCR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with CCR.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef GFX_H
#define GFX_H

#include <SDL.h>

/* The following functions do the same as in with SDL_gfx, */
/* EXCEPT THAT the behaviour is undefined if some of the input
   coordinates are beyond the surface, i.e. the input must be clipped. */

void lineColor(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 c);
void rectangleColor(SDL_Surface *s, int x1, int y1, int x2, int y2, Uint32 c);

#endif /* end of include guard: GFX_H */

