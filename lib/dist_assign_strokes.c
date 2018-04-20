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
#include "wmcbm.h"
#include <math.h>

/* Implement kanji_dist by finding a assignment between strokes of two kanji
 * so that the total stroke_dist is minimized. */

struct kanji_dist {
	StrokeDist *d;
	void *wmcbm;
	double cost[KANJI_MAXSTROKES][KANJI_MAXSTROKES];
};

KanjiDist *kanji_dist_new()
{
	KanjiDist *d = malloc(sizeof *d);
	if (!d)
		return d;
	if (!(d->d = stroke_dist_new())) {
		free(d);
		return NULL;
	}
	if (!(d->wmcbm = malloc(wmcbm_mem_needed(KANJI_MAXSTROKES, KANJI_MAXSTROKES)))) {
		free(d->d);
		free(d);
		return NULL;
	}
	return d;
}

void kanji_dist_free(KanjiDist *d)
{
	stroke_dist_free(d->d);
	free(d->wmcbm);
	free(d);
}

static double stroke_len(Stroke *s)
{
	uint i;
	double l;
	Vec2 last;

	last = s->p[0];
	for (l = 0, i = 1; i < s->n; i++) {
		last.x -= s->p[i].x;
		last.y -= s->p[i].y;
		l += sqrt(last.x*last.x + last.y*last.y);
		last = s->p[i];
	}
	return l;
}

// non-infinity result will ruin the metric property
static double dst_ignore_cost(Stroke *s)
{
	return 8*stroke_len(s);
}

static double src_ignore_cost(Stroke *s)
{
	return HUGE_VAL; //16*stroke_len(s); /* larger penalty */
}

double kanji_dist(KanjiDist *d, const Kanji *a, const Kanji *b)
{
	uint i, j;

	if (a->n != b->n && a->n+1 != b->n)
		return HUGE_VAL;
	rep (i, a->n)
	rep (j, b->n)
		d->cost[i][j] = stroke_dist(d->d, a->p + i, b->p + j);
	ran (i, a->n, b->n)
	rep (j, b->n)
		d->cost[i][j] = dst_ignore_cost(b->p + j);
	rep (i, a->n)
	ran (j, b->n, a->n)
		d->cost[i][j] = src_ignore_cost(a->p + i);
	i = max(a->n, b->n);
	return wmcbm_solve(d->wmcbm, (double*)d->cost, i, i, KANJI_MAXSTROKES, NULL)/i;
}
