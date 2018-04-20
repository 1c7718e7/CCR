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
#include "klist_io.h"
#include <inttypes.h>
#include <math.h>
#include <ctype.h>

/* Convert klist to a binary format suitable for lookup_strokecnt.c */
/* NB: changes to the format must also be mirrored in lookup_strokecnt.c */

static void put_varlen(uint n)
{
	while (n >= 0x80)
		putchar((n&0x7F)|0x80), n >>= 7;
	putchar(n);
}

static void put_f16(double f)
{
	assert(0 <= f && f <= 1);
	uint i = round(f*((1<<16)-1));
	assert((i&0xFFFF) == i);
	putchar(i&0xFF);
	putchar(i>>8&0xFF);
}

static void put_kanji(Kanji *k)
{
	uint si, pi;
	put_varlen(k->code);
	put_varlen(k->n);
	rep (si, k->n) {
		Stroke *const s = k->p + si;
		put_varlen(s->n);
		rep (pi, s->n) {
			put_f16(s->p[pi].x);
			put_f16(s->p[pi].y);
		}
	}
}

static int cmp_kanji(const void *a, const void *b)
{
	return ((const Kanji*)a)->n - ((const Kanji*)b)->n;
}

static uint sort_kanji(KList *kl)
{
	uint i, bucketc, lastc;
	qsort(kl->kanjiv, kl->kanjic, sizeof *kl->kanjiv, cmp_kanji);
	bucketc = 0;
	lastc = (uint)-1;
	rep (i, kl->kanjic) {
		if (lastc != kl->kanjiv[i].n) {
			lastc = kl->kanjiv[i].n;
			bucketc++;
		}
	}
	fprintf(stderr, "%u buckets\n", bucketc);
	return bucketc;
}

static void write_sc(const KList *kl, uint bucketc)
{
	uint i;
	fputs("CCR0SC00", stdout);
	put_varlen(kl->kanjic);
	put_varlen(kl->strokec);
	put_varlen(kl->pointc);
	put_varlen(bucketc);
	rep (i, kl->kanjic)
		put_kanji(kl->kanjiv + i);
}

int main(int argc, char **argv)
{
	KList kl;
	uint bucketc;

	klist_init(&kl);
	if (klist_fread(&kl, stdin)) {
		if (ferror(stdin))
			exit_perror("stdin");
		else
			exit_error("invalid input");
	}
	bucketc = sort_kanji(&kl);
	write_sc(&kl, bucketc);
	klist_fini(&kl);
	return 0;
}
