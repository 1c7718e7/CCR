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

static void simplify_aux(Stroke *s, uint a, uint b)
{
	Vec2 A, B, C;
	uint m, i;
	double sum, d2, il2, max;

	if (b - a <= 1) { /* it's simple already */
		s->p[s->n++] = s->p[b];
		return;
	}

	max = 0;
	m = a+1;
	sum = 0;
	A = s->p[a];
	B = s->p[b];
	B.x -= A.x; B.y -= A.y;
	il2 = 1./(B.x*B.x + B.y*B.y);
	DEBUG("SIMPLIFY %u -- %u\n", a, b);
	DEBUG("DISTANCES:\n");
	ran (i, a+1, b-1) {
		C = s->p[i];
		C.x -= A.x; C.y -= A.y;
		d2 = (C.x*B.x + C.y*B.y)*il2; /* projection */
		/* Also we should check for d2 < 0 and d2 > 1,
		   but it works well anyway. */
		C.x -= B.x*d2; C.y -= B.y*d2;
		d2 = C.x*C.x + C.y*C.y;
		sum += d2;
		DEBUG("\t%u %lf\n", i+1, d2);
		if (d2 > max) {
			max = d2;
			m = i;
		}
	}
	sum /= b - a; /* perhaps use real length? */
	DEBUG("MSE %lf\n", sum);
	if (sum < 0.0001) { /* straigthen the segment */
		s->p[s->n++] = s->p[b];
	} else { /* subdivide the segment */
		simplify_aux(s, a, m);
		simplify_aux(s, m, b);
	}
}

void stroke_simplify(Stroke *s)
{
	uint tmp = s->n-1;
	if (!s->n)
		return;
	s->n = 1;
	simplify_aux(s, 0, tmp);
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
}
