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
#include "klist_io.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

static uint getuint(char **_p, char *end, int *err)
{
	uint a = 0, c;
	char *p = *_p;
	while (p < end && isspace(*p))
		p++;
	if (p == end)
		return *err = 1, 0;
	for (;;) {
		c = (*p)-'0';
		if (c > 9)
			break;
		a = a*10 + c;
		p++;
		if (p == end)
			break;
	}
	*_p = p;
	return a;
}

static double getfloat(char **_p, char *end, int *err)
{
	double a, mul;
	uint c;
	char *p = *_p;
	while (p < end && isspace(*p))
		p++;
	if (p == end)
		return *err = 1, 0;
	a = 0;
	for (;;) {
		c = (*p)-'0';
		if (c > 9)
			break;
		a = a*10.0 + c;
		p++;
		if (p == end)
			break;
	}
	if (*p == '.' && ++p != end) {
		mul = 0.1;
		for (;;) {
			c = (*p)-'0';
			if (c > 9)
				break;
			a += mul*c;
			mul *= .1;
			p++;
			if (p == end)
				break;
		}
	}
	*_p = p;
	return a;
}


int klist_parse(KList *kl, char **p, char *end)
{
	int err;
	uint ki, si;
	uint sofs, pofs;

	err = 0;
	sofs = pofs = 0;
	/* read sizes */
	kl->kanjic = getuint(p, end, &err);
	kl->strokec = getuint(p, end, &err);
	kl->pointc = getuint(p, end, &err);
	if (err)
		goto err;
	/* realloc if needed */
	if (kl->kanjic > kl->kanjim) {
		kl->kanjim = kl->kanjic;
		kl->kanjiv = realloc(kl->kanjiv, kl->kanjim * sizeof *kl->kanjiv);
		if (!kl->kanjiv)
			goto err;
	}
	if (kl->strokec > kl->strokem) {
		kl->strokem = kl->strokec;
		kl->strokev = realloc(kl->strokev, kl->strokem * sizeof *kl->strokev);
		if (!kl->strokev)
			goto err;
	}
	if (kl->pointc > kl->pointm) {
		kl->pointm = kl->pointc;
		kl->pointv = realloc(kl->pointv, kl->pointm * sizeof *kl->pointv);
		if (!kl->pointv)
			goto err;
	}
	sofs = pofs = 0;
	rep (ki, kl->kanjic) {
		/* read kanji */
		Kanji *const k = kl->kanjiv + ki;
		DEBUG("READ KANJI %u/%u\n", ki, kl->kanjic);
		k->code = getuint(p, end, &err);
		k->n = getuint(p, end, &err);
		if (err)
			goto err;
		k->p = kl->strokev + sofs;
		if ((sofs += k->n) > kl->strokec)
			goto err; /* more strokes than promised */
		rep (si, k->n) {
			DEBUG("READ STROKE %u/%u\n", si, k->n);
			/* read stroke */
			Stroke *const s = k->p + si;
			s->n = 0;
			s->p = kl->pointv + pofs;
			for (;;) {
				DEBUG("READ POINT %u\n", s->n);
				s->p[s->n].x = getfloat(p, end, &err);
				s->p[s->n].y = getfloat(p, end, &err);
				if (err)
					goto err;
				s->n++;
				if ((pofs += 1) > kl->pointc)
					goto err;
				if (*p == end)
					goto err;
				if (*(*p)++ == '\n')
					break;
			}
		}
	}
	if (sofs != kl->strokec || pofs != kl->pointc)
		goto err; /* less strokes/points than promised */
	if (err)
		goto err;
	return 0;
err:
	/* yes, we do not free the buffers: klist_fini does that */
	return 1;
}

void klist_init(KList *kl)
{
	memset(kl, 0, sizeof *kl);
}

void klist_fini(KList *kl)
{
	free(kl->kanjiv); /* free(NULL) is fine */
	free(kl->strokev);
	free(kl->pointv);
}

int klist_fread(KList *kl, FILE *f)
{
	char *buf, *p, *end;
	size_t n, m;
	size_t r;

	m = 1024*1024;
	n = 0;
	if (!(buf = malloc(m)))
		return 1;
	for (;;) {
		r = fread(buf+n, 1, m-n, f);
		if ((n += r) == m) {
			if (!(buf = realloc(buf, m *= 2)))
				return 1;
		} else
			break;
	}
	if (ferror(f))
		return 1;
	p = buf;
	end = buf + n;
	r = klist_parse(kl, &p, end);
	free(buf);
	if (p != end) /* trailing data */
		return 1;
	return r;
}

void klist_fwrite(const KList *kl, FILE *f)
{
	int ki, si, pi;
	fprintf(f, "%u %u %u\n", kl->kanjic, kl->strokec, kl->pointc);
	for (ki = 0; ki < kl->kanjic; ki++) {
		const Kanji *k = kl->kanjiv + ki;
		fprintf(f, "%"PRIu32" %u\n", k->code, k->n);
		for (si = 0; si < k->n; si++) {
			Stroke *s = k->p + si;
			assert(s->n);
			for (pi = 0; pi < s->n-1; pi++)
				printf("%lf %lf ", s->p[pi].x, s->p[pi].y);
			printf("%lf %lf\n", s->p[pi].x, s->p[pi].y);
		}
	}
}
