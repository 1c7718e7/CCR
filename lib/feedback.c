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

/* Like dist_assign_strokes.c but also returns the assignment. */

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


static double dst_ignore_cost(Stroke *s)
{
	return 8*stroke_len(s);
}

static double src_ignore_cost(Stroke *s)
{
	return HUGE_VAL;
}

void kanji_feedback(const Kanji *user, const Kanji *model, int *ui, int *mi)
{
	double *cost;
	void *wmcbm;
	int size, i, j;
	StrokeDist *d;
	unsigned *map;

	size = max(user->n, model->n);
	wmcbm = malloc(wmcbm_mem_needed(size, size));
	cost = malloc(size*size * sizeof *cost);
	map = malloc(size * sizeof *map);
	d = stroke_dist_new();

	rep (i, user->n)
	rep (j, model->n)
		cost[i + size*j] = stroke_dist(d, user->p + i, model->p + j);
	ran (i, user->n, model->n)
	rep (j, model->n)
		cost[i + size*j] = dst_ignore_cost(model->p + j);
	rep (i, user->n)
	ran (j, model->n, user->n)
		cost[i + size*j] = src_ignore_cost(user->p + i);

	wmcbm_solve(wmcbm, cost, size, size, size, map);

	rep (j, model->n)
		mi[j] = -1;
	rep (i, user->n) {
		if (map[i] < model->n) {
			ui[i] = map[i];
			mi[map[i]] = i;
		} else {
			ui[i] = -1;
		}
	}

	stroke_dist_free(d);
	free(wmcbm);
	free(cost);
	free(map);
}
