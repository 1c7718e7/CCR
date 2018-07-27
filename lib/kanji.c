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
#include <string.h>

#define ABS_TOLERANCE 0.001
#define REL_TOLERANCE 0.001

/* li = index of the first vertex in our interval */
/* ri = index of the vertex right after our interval */
static void simplify_aux(Stroke *stroke, uint li, uint ri, double tol)
{
	Vec2 L, M, R, Dir, M_proj;
	uint pivot, mi;
	double pivot_dist, len, dist, lr_len;

	if (li == ri) {
		stroke->p[stroke->n++] = stroke->p[ri];
		DEBUG("PUT %i\n", ri);
		return;
	}

	L = stroke->p[li-1];
	R = stroke->p[ri];
	Dir.x = R.x - L.x;
	Dir.y = R.y - L.y;
	lr_len = sqrt(Dir.x*Dir.x + Dir.y*Dir.y);
	Dir.x /= lr_len;
	Dir.y /= lr_len;

	/* find the furthest point from line segment LR */
	pivot = -1;
	pivot_dist = 0.0;
	for (mi = li; mi < ri; mi++) {
		M = stroke->p[mi];

		/* project M onto LR */
		len = (M.x - L.x)*Dir.x + (M.y - L.y)*Dir.y;
		if (len < 0.0)
			len = 0.0;
		if (len > lr_len)
			len = lr_len;
		M_proj.x = L.x + len*Dir.x;
		M_proj.y = L.y + len*Dir.y;

		/* the line segment M'M */
		M_proj.x -= M.x;
		M_proj.y -= M.y;

		dist = M_proj.x*M_proj.x + M_proj.y*M_proj.y;
		if (dist > pivot_dist) {
			pivot = mi;
			pivot_dist = dist;
		}
	}

	DEBUG("SIMPL %i..%i: PIVOT %i DIST %lf\n", li, ri, pivot, pivot_dist);

	if (pivot_dist < tol) {
		stroke->p[stroke->n++] = stroke->p[ri];
		DEBUG("PUT %i\n", ri);
	} else {
		simplify_aux(stroke, li, pivot, tol);
		simplify_aux(stroke, pivot+1, ri, tol);
	}
}

void stroke_simplify(Stroke *s)
{
	Vec2 D;
	double g_len, tol;
	uint i, n;

	n = s->n;
	if (n <= 1)
		return;

	g_len = 0.0;
	for (i = 0; i < n-1; i++) {
		D.x = s->p[i].x - s->p[i+1].x;
		D.y = s->p[i].y - s->p[i+1].y;
		g_len += sqrt(D.x*D.x + D.y*D.y);
	}
	s->n = 1;
	tol = g_len * REL_TOLERANCE;
	if (tol < ABS_TOLERANCE)
		tol = ABS_TOLERANCE;
	simplify_aux(s, 1, n-1, tol);
	#if 0 /* bad idea */
	g_len = 0.0;
	for (i = 0; i < n-1; i++) {
		D.x = s->p[i].x - s->p[i+1].x;
		D.y = s->p[i].y - s->p[i+1].y;
		g_len += sqrt(D.x*D.x + D.y*D.y);
	}
	if (s->n >= 3) {
		/* clean up short tails at the end of a stroke */
		D.x = s->p[s->n-2].x - s->p[s->n-1].x;
		D.y = s->p[s->n-2].y - s->p[s->n-1].y;
		len = sqrt(D.x*D.x + D.y*D.y);
		if (len*20 < g_len)
			s->n--;
	}
	#endif
}

void kanji_normalize(Kanji *_k)
{
	Vec2 min, max;
	Vec2 shift, scale;
	uint j, k, n;
	Stroke *s;

	s = _k->p;
	n = _k->n;
	min.x = min.y = HUGE_VAL;
	max.x = max.y = -HUGE_VAL;
	rep (j, n)
	rep (k, s[j].n) {
		min.x = min(min.x, s[j].p[k].x);
		min.y = min(min.y, s[j].p[k].y);
		max.x = max(max.x, s[j].p[k].x);
		max.y = max(max.y, s[j].p[k].y);
	}

#define CALC(D)\
	if (max.D - min.D < .1) {\
		double m = (max.D + min.D)*.5;\
		max.D = m+.05;\
		min.D = m-.05;\
	}\
	shift.D = -min.D;\
	scale.D = 1/(max.D - min.D);
	CALC(x);
	CALC(y);
	rep (j, n)
	rep (k, s[j].n) {
		s[j].p[k].x = scale.x * (s[j].p[k].x + shift.x);
		s[j].p[k].y = scale.y * (s[j].p[k].y + shift.y);
	}
	rep (j, n)
		stroke_simplify(&s[j]);
}
