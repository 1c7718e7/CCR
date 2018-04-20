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

/* A distance function for Strokes. Based on the Frechet distance */
/* Here we compute it with the obvious O(nm) DP */
struct stroke_dist {
	double *buffer; /* will be resized as needed */
	uint size;
} dummy;

StrokeDist *stroke_dist_new()
{
	StrokeDist *d = malloc(sizeof *d);
	d->size = 64;
	d->buffer = malloc(d->size * sizeof *d->buffer);
	return d;
}

static double *get_buffer(StrokeDist *d, uint n)
{
	uint size = d->size;
	if (n > size) {
		do size *= 2;
		while (n > size);
		d->buffer = realloc(d->buffer, (d->size = size) * sizeof *d->buffer);
	}
	return d->buffer;
}

void stroke_dist_free(StrokeDist *d)
{
	free(d->buffer);
	free(d);
}

static inline double vec2_dist(Vec2 x, Vec2 y)
{
	x.x -= y.x;
	x.y -= y.y;
	// taking the square root because we want it to be metric
	return sqrt(x.x*x.x + x.y*x.y);
}

double stroke_dist(StrokeDist *d, const Stroke *a, const Stroke *b)
{
	double *dp, t;
	uint i, j;
	#define DP(_i, _j) dp[(_i) + (_j)*a->n]

	dp = get_buffer(d, a->n*b->n);
	rep (i, a->n*b->n)
		dp[i] = HUGE_VAL;

	/* when pos along one curve is zero */
	rep (i, a->n)
		DP(i, 0) = vec2_dist(a->p[i], b->p[0]);
	ran (j, 1, b->n)
		DP(0, j) = vec2_dist(a->p[0], b->p[j]);
	
	/* inductive case */
	rep (i, a->n-1)
	rep (j, b->n-1) {
		t = DP(i, j) + vec2_dist(a->p[i+1], b->p[j]);
		if (DP(i+1, j+0) > t)
			DP(i+1, j+0) = t;
		t = DP(i, j) + vec2_dist(a->p[i], b->p[j+1]);
		if (DP(i+0, j+1) > t)
			DP(i+0, j+1) = t;
		t = DP(i, j) + vec2_dist(a->p[i+1], b->p[j+1]);
		if (DP(i+1, j+1) > t)
			DP(i+1, j+1) = t;
	}
	return DP(a->n-1, b->n-1);
}

