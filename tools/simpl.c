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
#define _POSIX_C_SOURCE 200809L
#include "std.h"
#include <math.h>
#include "ccr.h"
#include <stdio.h>
#include "klist_io.h"
#include "svg.h"

#define MAXSAMPLES 512000

static struct {
	uint ofs, n;
	Vec2 p[MAXSAMPLES];
} smp;

static void new_kanji()
{
	smp.n = 0;
	smp.ofs = 0;
}

static Vec2 *new_stroke()
{
	smp.n = 0;
	return smp.p + smp.ofs;
}

static uint end_stroke()
{
	return smp.n;
}

static void new_point(Vec2 v)
{
	if (smp.ofs+1 >= MAXSAMPLES)
		exit_error("too many points\n");
	smp.p[smp.ofs].x = v.x;
	smp.p[smp.ofs].y = v.y;
	smp.ofs++;
	smp.n++;
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
		new_point(B);
	} else {
		m = (a + b)/2;
		M = bezier(p, m);
		do_curve_aux(p, A, M, a, m);
		do_curve_aux(p, M, B, m, b);
	}
}

static void do_curve(Vec2 p[4])
{
	new_point(p[0]);
	do_curve_aux(p, p[0], p[3], 0, 1);
}

static void svg_callback(void *_, double *p, unsigned n)
{
	if (n > 4)
		exit_error("cubic splines not supported");
	Vec2 v[4];
	uint i;

	rep (i, n) {
		v[i].x = p[2*i+0]*(1.0/109);
		v[i].y = p[2*i+1]*(1.0/109);
	}
	if (n == 4)
		do_curve(v);
	else if (n == 2) {
		new_point(v[0]);
		new_point(v[1]);
	}
}

int main(int argc, char *argv[])
{
	int kanji_cnt;
	struct klist klist;
	SvgState svg;
	Kanji kbuf;
	Stroke sbuf[KANJI_MAXSTROKES];
	int kanji_i, stroke_i, point_i;
	char *linebuf;
	size_t linebuf_size;

	if (argc != 1)
		exit_error("usage: simpl < <svg_data> > <klist>");
	linebuf_size = 0;
	linebuf = NULL;
	kbuf.p = sbuf;
	if (1 != scanf("%i", &kanji_cnt))
		exit_error("number expected");
	klist_init(&klist);
	for (kanji_i = 0; kanji_i < kanji_cnt; kanji_i++) {
		if (2 != scanf("%"SCNu32"%u ", &kbuf.code, &kbuf.n))
			exit_error("kanji expected");
		point_i = 0;
		new_kanji();
		for (stroke_i = 0; stroke_i < kbuf.n; stroke_i++) {
			Stroke *s = kbuf.p + stroke_i;
			s->p = new_stroke();
			svg_start(&svg, svg_callback, s);
			ssize_t len = getline(&linebuf, &linebuf_size, stdin);
			if (len < 0)
				exit_error("stroke expected");
			svg_feeds(&svg, linebuf, len);
			s->n = end_stroke();
		}
		kanji_normalize(&kbuf);
		klist_append_copy(&klist, &kbuf);
	}
	klist_fwrite(&klist, stdout);
	klist_fini(&klist);
	return 0;
}
