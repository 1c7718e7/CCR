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
#include "std.h"
#include <math.h>
#include "ccr.h"
#include "svg.h"

/* read svg path data and output simplified strokes */

#define KANJIVG_SCALE (1.0/109)

#define MAXSAMPLES 512000
#define MAXSTROKES 320000

static struct {
	uint n, o;
	Vec2 p[MAXSAMPLES];
} smp;

static Stroke stroke_buf[MAXSTROKES];
static Kanji stk = {0, 0xFFFF, stroke_buf};

static void output_stroke(Stroke *s)
{
	uint i;

#if 1
	printf("%.5lf %.5lf", s->p[0].x, s->p[0].y);
	ran (i, 1, s->n)
		printf(" %.5lf %.5lf", s->p[i].x, s->p[i].y);
	putchar('\n');
#else
	printf("M%lf %lf", s->p[0].x*109, s->p[0].y*109);
	ran (i, 1, s->n)
		printf("L%.5lf %.5lf", s->p[i].x*109, s->p[i].y*109);
	putchar('\n');
#endif
}

static void push_kanji(u32 code)
{
	uint i;
	kanji_normalize(&stk);

	printf("%u %u\n", code, stk.n);
	rep (i, stk.n)
		output_stroke(stk.p + i);
}

static void push_stroke()
{
	const uint n = smp.n - smp.o;
	if (n < 1)
		return;
	if (stk.n == MAXSTROKES)
		exit_error("too many strokes\n");
	stk.p[stk.n] = (Stroke) { n, smp.p + smp.o };
	stroke_simplify(stk.p + stk.n);
	stk.n++;
	smp.o = smp.n;
}

static void push_point(Vec2 p)
{
	if (smp.n == MAXSAMPLES)
		exit_error("too many points\n");

	smp.p[smp.n++] = p;
}

static Vec2 bezier(Vec2 p[4], double t)
{
	Vec2 a;
	const double bt = 1.0 - t;

	a.x = bt*bt*bt*p[0].x
	    + 3*bt*bt*t*p[1].x
	    + 3*bt*t*t*p[2].x
	    + t*t*t*p[3].x;
	a.y = bt*bt*bt*p[0].y
	    + 3*bt*bt*t*p[1].y
	    + 3*bt*t*t*p[2].y
	    + t*t*t*p[3].y;
	return a;
}

static void do_curve_aux(Vec2 p[4], Vec2 A, Vec2 B, double a, double b)
{
	double m, tmp;
	Vec2 M;

	tmp = B.x - A.x;
	m = tmp*tmp;
	tmp = B.y - A.y;
	m += tmp*tmp;
	DEBUG("DELTA² %lf\n", m);
	if (m < 1./256) {
		push_point(B);
	} else {
		m = (a + b)/2;
		M = bezier(p, m);
		do_curve_aux(p, A, M, a, m);
		do_curve_aux(p, M, B, m, b);
	}
}

static void do_curve(Vec2 p[4])
{
	push_point(p[0]);
	do_curve_aux(p, p[0], p[3], 0, 1);
}

static void svg_callback(void *_, double *p, unsigned n)
{
	Vec2 v[4];
	uint i;

	rep (i, n) {
		v[i].x = p[2*i+0]*KANJIVG_SCALE;
		v[i].y = p[2*i+1]*KANJIVG_SCALE;
	}
	if (n == 4)
		do_curve(v);
	else if (n == 2) {
		push_point(v[0]);
		push_point(v[1]);
	}
}

int main(int argc, char *argv[])
{
	SvgState svg;
	int i;
	u32 c;

	if (argc != 2) {
		exit_error("usage: simpl <code_point> < <svg_data>");
	}
	c = strtol(argv[1], 0, 0);
reset:
	svg_start(&svg, svg_callback, NULL);
	while ((i = getchar()) != EOF) {
		if (svg_feedc(&svg, i))
			exit_error("error on %c\n", (char)i);
		if (i == '\n') {
			push_stroke();
			goto reset;
		}
	}
	push_kanji(c);
	if (ferror(stdin))
		exit_perror("getchar");
	return 0;
}

