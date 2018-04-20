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
#include <inttypes.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

/* This code is deprecated */

/* Lookups are accelerated by a Vantage Point Tree
   For this to work kanji_dist() must satisfy the following rules:
   kanji_dist(a, b) == 0, iff a == b
   kanji_dist(a, b) == kanji_dist(b, a)
   kanji_dist(a, c) <= kanji_dist(a, b) < kanji_dist(b, c)
   The third rule is difficult to satisfy, and for our problem VP-tree
   rarely brings over 20% speed gains.
 */


typedef struct vp VP;

struct kanji_db {
	KanjiDist *d;
	Vec2 *pointv;
	Stroke *strokev;
	VP *nodev;
	uint nodec, strokec, pointc;
};

struct vp {
	Kanji kanji;
	double level;
	VP *in, *out;
};

static uint get_varlen(uchar **p, uchar *end, int *err)
{
	unsigned char c;
	uint a = 0, s = 0;

	do {
		if (*p >= end) {
			*err = 1;
			return a;
		}
		c = *((*p)++);
		a |= (c&0x7F)<<s;
		s += 7;
	} while (c&0x80);

	return a;
}

static double get_f16(uchar **p, uchar *end, int *err)
{
	uint i;
	if (*p+2 > end)
		return *err = 1, 0.0;
	i = (uint)p[0][0]
	  + 256*(uint)p[0][1];
	*p += 2;
	return i*(1.0/65355.0);
}

static double get_f32(uchar **p, uchar *end, int *err)
{
	uint i;
	double f;
	if (*p+4 > end)
		return *err = 1, 0.0;
	i = (uint)p[0][0]
	  + 0x100*(uint)p[0][1]
	  + 0x10000*(uint)p[0][2]
	  + 0x1000000*(uint)p[0][3];
	*p += 4;
	f = i*(1.0/4294967295.0);
	f = 4.0*(1.0/f - 1);
	assert(f >= 0.0);
	return f;
}

static void get_vptree(KanjiDB *db, uint limit, uint *ni, uint *si, uint *pi,
                       uchar **p, uchar *end, int *err)
{
	VP *vp;
	uint ksi, spi, insize;

	/* alloc one node */
	if (*ni+1 > limit)
		goto err;
	vp = db->nodev + *ni;
	*ni += 1;

	/* read the kanji */
	vp->kanji.code = get_varlen(p, end, err);
	vp->kanji.n = get_varlen(p, end, err);
	if (*err)
		return;
	/* alloc strokes */
	vp->kanji.p = db->strokev + *si;
	*si += vp->kanji.n;
	if (*si > db->strokec)
		goto err;
	rep (ksi, vp->kanji.n) {
		/* read a stroke */
		Stroke *const s = vp->kanji.p + ksi;
		s->n = get_varlen(p, end, err);
		if (*err)
			return;
		/* alloc points for a stroke */
		s->p = db->pointv + *pi;
		*pi += s->n;
		if (*pi > db->pointc)
			goto err;
		/* read points */
		rep (spi, s->n) {
			s->p[spi].x = get_f16(p, end, err);
			s->p[spi].y = get_f16(p, end, err);
		}
	}
	/* read vp tree info */
	vp->level = get_f32(p, end, err); /* DBG was already set??? */
	insize = get_varlen(p, end, err);
	if (*err)
		return;
	if (*ni+insize > limit)
		goto err;
	
	vp->in = NULL;
	if (insize > 0) {
		vp->in = db->nodev + *ni;
		get_vptree(db, *ni + insize, ni, si, pi, p, end, err);
	}
	if (*err)
		return;
	vp->out = NULL;
	if (*ni < limit) {
		vp->out = db->nodev + *ni;
		get_vptree(db, limit, ni, si, pi, p, end, err);
	}
	return;
err:
	*err = 1;
	return;
}

KanjiDB *kanji_db_open(char *_data, size_t n)
{
	KanjiDB *db;
	int err;
	uint ni, si, pi;
	uchar *data = _data;
	uchar *end = data + n;

	db = NULL;
	if (memcmp(data, "CCR0VP00", 8))
		goto err_n_free;
	data += 8;
	err = 0;
	if (!(db = malloc(sizeof *db)))
		goto err_n_free;
	if (!(db->d = kanji_dist_new()))
		goto err_n_free;
	db->nodev = NULL;
	db->strokev = NULL;
	db->pointv = NULL;

	db->nodec = get_varlen(&data, end, &err);
	db->strokec = get_varlen(&data, end, &err);
	db->pointc = get_varlen(&data, end, &err);
	if (err)
		goto err_n_free;

	if (!(db->nodev = malloc(db->nodec * sizeof *db->nodev)))
		goto err_n_free;
	if (!(db->strokev = malloc(db->strokec * sizeof *db->strokev)))
		goto err_n_free;
	if (!(db->pointv = malloc(db->pointc * sizeof *db->pointv)))
		goto err_n_free;

	ni = si = pi = 0;
	get_vptree(db, db->nodec, &ni, &si, &pi, &data, end, &err);
	if (ni != db->nodec ||
	    si != db->strokec ||
	    pi != db->pointc)
		goto err_n_free;
	if (err)
		goto err_n_free;
	return db;
err_n_free:
	if (db) {
		free(db->nodev);
		free(db->strokev);
		free(db->pointv);
		if (db->d)
			kanji_dist_free(db->d);
	}
	return NULL;
}

void kanji_db_close(KanjiDB *db)
{
	free(db->nodev);
	free(db->strokev);
	free(db->pointv);
	kanji_dist_free(db->d);
	free(db);
}

static void heap_lower(KanjiMatch *h, uint n, uint i)
{
	uint j, k;
	KanjiMatch t;

	for (;;) {
		k = i;
		j = 2*i + 1;;
		if (j < n && h[j].score > h[k].score) {
			k = j;
		}
		j = 2*i + 2;;
		if (j < n && h[j].score > h[k].score) {
			k = j;
		}
		if (k == i)
			break;
		t = h[i];
		h[i] = h[k];
		h[k] = t;
		i = k;
	}
}

static void heap_insert(KanjiMatch *h, uint n, double score, u32 code)
{
	if (h[0].score <= score)
		return;
	h[0].score = score;
	h[0].code = code;
	heap_lower(h, n, 0);
}

static void heap_finish(KanjiMatch *h, uint n)
{
	KanjiMatch t;

	while (n) {
		t = h[0];
		h[0] = h[n-1];
		h[n-1] = t;
		n--;
		heap_lower(h, n, 0);
	}
}

static uint vp_lookup(KanjiDist *kd, VP *vp, Kanji *k, KanjiMatch *res, uint n, double mind)
{
	if (!vp)
		return 0;
	if (mind > res[0].score)
		return 0;
	double d = kanji_dist(kd, k, &vp->kanji);
	heap_insert(res, n, d, vp->kanji.code);
	if (mind > d)
		fprintf(stderr, "%lf > %lf: PRUNING IS FUCKING WRONG!!!\n", mind, d);
	double m;
	uint cost;
	VP *f, *s;
	if (d <= vp->level) {
		f = vp->in;
		s = vp->out;
		m = vp->level - d;
	} else {
		f = vp->out;
		s = vp->in;
		m = d - vp->level;
	}
	if (m == HUGE_VAL)
		m = 0;
	cost = 1;
	cost += vp_lookup(kd, f, k, res, n, mind);
	cost += vp_lookup(kd, s, k, res, n, max(mind, m));
	return cost;
}

uint kanji_db_lookup(KanjiDB *db, const Kanji *k, KanjiMatch *res, uint n)
{
	uint i;

	rep (i, n) {
		res[i].score = HUGE_VAL;
		res[i].code = 0xFFFF;
	}
	i = vp_lookup(db->d, db->nodev, k, res, n, 0);
	fprintf(stderr, "%i/%i -> %.2lf speedup\n", i, db->nodec, (double)db->nodec/i);
	heap_finish(res, n);
	while (n && res[n-1].code == 0xFFFF)
		n--;
	return n;
}
