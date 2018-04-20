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

/* The kanji are split into bins based on stroke count.
   On lookup look through only the bins with similar stroke count.
   Works quickly and even when one input stroke is missing
 */

typedef struct vp VP;

struct kanji_db {
	KanjiDist *d;
	Vec2 *pointv;
	Stroke *strokev;
	Kanji *kanjiv;
	struct bucket {
		uint strokec;
		uint kanjic;
		Kanji *kanjiv;
	} *bucketv;
	uint kanjic, strokec, pointc, bucketc;
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

KanjiDB *kanji_db_open(char *_data, size_t n)
{
	KanjiDB *db;
	int err;
	uint ki, si, pi;
	struct bucket *bp;
	Kanji *kp;
	Stroke *sp;
	Vec2 *pp;
	uchar *data = (uchar*)_data;
	uchar *end = data + n;

	err = 0;
	db = NULL;
	if (memcmp(data, "CCR0SC00", 8))
		goto err_n_free;
	data += 8;
	err = 0;
	if (!(db = malloc(sizeof *db)))
		goto err_n_free;
	if (!(db->d = kanji_dist_new()))
		goto err_n_free;
	db->kanjiv = NULL;
	db->strokev = NULL;
	db->pointv = NULL;
	db->bucketv = NULL;

	/* read sizes */
	db->kanjic = get_varlen(&data, end, &err);
	db->strokec = get_varlen(&data, end, &err);
	db->pointc = get_varlen(&data, end, &err);
	db->bucketc = get_varlen(&data, end, &err);
	if (err)
		goto err_n_free;

	/* alloc arrays */
	if (!(db->kanjiv = malloc(db->kanjic * sizeof *db->kanjiv)))
		goto err_n_free;
	if (!(db->strokev = malloc(db->strokec * sizeof *db->strokev)))
		goto err_n_free;
	if (!(db->pointv = malloc(db->pointc * sizeof *db->pointv)))
		goto err_n_free;
	if (!(db->bucketv = malloc(db->bucketc * sizeof *db->bucketv)))
		goto err_n_free;

	kp = db->kanjiv;
	sp = db->strokev;
	pp = db->pointv;
	bp = NULL;
	rep (ki, db->kanjic) {
		/* read kanji */
		///DEBUG("read kanji %u\n", ki);
		kp->code = get_varlen(&data, end, &err);
		kp->n = get_varlen(&data, end, &err);
		if (err)
			goto err_n_free;
		if (!bp || bp->strokec != kp->n) {
			//DEBUG("bucket %u\n", (uint)(bp-db->bucketv));
			/* start of new bucket */
			if (bp) {
				if (bp->strokec > kp->n)
					goto err_n_free;
				else
					bp++;
			} else
				bp = db->bucketv;
			if (bp >= db->bucketv + db->bucketc)
				goto err_n_free;
			bp->strokec = kp->n;
			bp->kanjic = 0;
			bp->kanjiv = kp;
		}
		bp->kanjic++;
		kp->p = sp;
		rep (si, kp->n) {
			//DEBUG("read stroke\n");
			/* read stroke */
			if (sp == db->strokev + db->strokec)
				goto err_n_free;
			sp->n = get_varlen(&data, end, &err);
			if (err)
				goto err_n_free;
			sp->p = pp;
			rep (pi, sp->n) {
				//DEBUG("read point\n");
				if (pp == db->pointv + db->pointc)
					goto err_n_free;
				pp->x = get_f16(&data, end, &err);
				pp->y = get_f16(&data, end, &err);
				pp++;
			}
			if (err)
				goto err_n_free;
			sp++;
		}
		kp++;
	}
	if (sp != db->strokev + db->strokec ||
	    pp != db->pointv  + db->pointc  ||
	    bp != db->bucketv + db->bucketc - 1)
		goto err_n_free;
	if (err)
		goto err_n_free;
	return db;
err_n_free:
	if (db) {
		free(db->kanjiv);
		free(db->strokev);
		free(db->pointv);
		free(db->bucketv);
		if (db->d)
			kanji_dist_free(db->d);
	}
	return NULL;
}

void kanji_db_close(KanjiDB *db)
{
	free(db->kanjiv);
	free(db->strokev);
	free(db->pointv);
	free(db->bucketv);
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

static void heap_insert(KanjiMatch *h, uint n, double score, u32 code, void *cookie)
{
	if (h[0].score <= score)
		return;
	h[0].score = score;
	h[0].code = code;
	h[0].cookie = cookie;
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

static void bucket_lookup(struct bucket *b, KanjiDB *db, const Kanji *k,
                          KanjiMatch *res, uint n)
{
	uint i;
	double s;
	rep (i, b->kanjic) {
		Kanji *const k2 =  b->kanjiv + i;
		s = kanji_dist(db->d, k, k2);
		DEBUG("%u/%u ~ %u/%u = %lf\n", k->code, k->n, k2->code, k2->n, s);
		heap_insert(res, n, s, k2->code, k2);
		DEBUG("WORST %lf\n", res[0].score);
	}
}

uint kanji_db_lookup(KanjiDB *db, const Kanji *k, KanjiMatch *res, uint n)
{
	uint i;
	uint bc;

	rep (i, n) {
		res[i].score = HUGE_VAL;
		res[i].code = 0xFFFF;
		res[i].cookie = NULL;
	}
	bc = 0;
	rep (i, db->bucketc) if (db->bucketv[i].strokec >= k->n && bc++ < 2)
		bucket_lookup(db->bucketv + i, db, k, res, n);
	heap_finish(res, n);
	while (n && res[n-1].code == 0xFFFF)
		n--;
	return n;
}

const Kanji *kanji_db_data(KanjiDB *db, void *cookie)
{
	return cookie;
}

void kanji_db_data_release(KanjiDB *db, void *cookie, const Kanji *k) {}
