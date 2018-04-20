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
#include "ccr.h"
#include <math.h>

/* A distance function for Strokes. Relies heavily on direction. */
static struct stroke_dist { char c; } dummy;
StrokeDist *stroke_dist_new() { return &dummy; }
void stroke_dist_free(StrokeDist *d) {}

double stroke_dist(StrokeDist *d, const Stroke *a, const Stroke *b)
{
	Vec2 A, B, C;
	float al, bl, cl;
	float k;
	float err;
	uint i, j;
	static const double len_factor = 1.f;

	if (a->n == 0 || b->n == 0)
		return 1./0.;
	k = 1;
	C.x = a->p[0].x - b->p[0].x;
	C.y = a->p[0].y - b->p[0].y;
	err = sqrt(C.x*C.x + C.y*C.y)*2;
	i = j = 1;
	al = bl = 0.f;
	while (i != a->n && j != b->n) {
		if (al == 0.f) {
			A.x = a->p[i-1].x - a->p[i].x;
			A.y = a->p[i-1].y - a->p[i].y;
			al = sqrt(A.x*A.x + A.y*A.y);
			i++;
		}
		if (bl == 0.f) {
			B.x = b->p[j-1].x - b->p[j].x;
			B.y = b->p[j-1].y - b->p[j].y;
			bl = sqrt(B.x*B.x + B.y*B.y);
			bl *= k;
			j++;
		}

		cl = min(al, bl);
		C.x = A.x - B.x;
		C.y = A.y - B.y;
		err += sqrt(cl*cl*(C.x*C.x + C.y*C.y));
		al -= cl;
		bl -= cl;
	}

	err += len_factor*(al + bl);
	while (i != a->n) {
		A.x = a->p[i-1].x - a->p[i].x;
		A.y = a->p[i-1].y - a->p[i].y;
		err += len_factor*sqrt(A.x*A.x + A.y*A.y);
		i++;
	}
	while (j != b->n) {
		B.x = b->p[j-1].x - b->p[j].x;
		B.y = b->p[j-1].y - b->p[j].y;
		err += len_factor*sqrt(B.x*B.x + B.y*B.y);
		j++;
	}
	return err;
}

