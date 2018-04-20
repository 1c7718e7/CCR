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

/* A linear search through all the kanji */

struct kanji_db {
	KanjiDist *d;
	Vec2 *pv;
	Stroke *sv;
	Kanji *kv;
	uint n;
};

static uint getuint(char **_p, char *end)
{
	uint a = 0, c;
	char *p = *_p;
	while (p < end && isspace(*p))
		p++;
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

static double getfloat(char **_p, char *end)
{
	double a, mul;
	uint c;
	char *p = *_p;
	while (p < end && isspace(*p))
		p++;
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

KanjiDB *kanji_db_open(char *data, size_t n)
{
	KanjiDB *db;
	uint nk, ns, np, si, ki;
	char *end;
	Vec2 *pm;
	Stroke *sm;

	end = data+n;
	nk = getuint(&data, end);
	ns = getuint(&data, end);
	np = getuint(&data, end);
	if (!(db = malloc(sizeof *db)))
		return NULL;
	db->pv = NULL;
	db->sv = NULL;
	db->kv = NULL;
	if (!(db->d = kanji_dist_new()))
		goto err_n_free;
	if (!(db->pv = malloc(np * sizeof *db->pv)))
		goto err_n_free;
	if (!(db->sv = malloc(ns * sizeof *db->sv)))
		goto err_n_free;
	if (!(db->kv = malloc(nk * sizeof *db->kv)))
		goto err_n_free;
	db->n = nk;
	pm = db->pv;
	sm = db->sv;
	rep (ki, nk) {
		Kanji *const k = &db->kv[ki];
		k->code = getuint(&data, end);
		k->n = getuint(&data, end);
		k->p = sm;
		rep (si, k->n) {
			if (!ns)
				goto err_n_free; /* more strokes than promised */
			sm->p = pm;
			sm->n = 0;
			do {
				if (!np)
					goto err_n_free; /* more points than promised */
				pm->x = getfloat(&data, end);
				pm->y = getfloat(&data, end);
				sm->n++;
				pm++;
				np--;
			} while (data < end && (*data++) != '\n');
			ns--;
			sm++;
		}
	}
	if (data != end)
		goto err_n_free;
	return db;
err_n_free:
	if (db->pv)
		free(db->pv);
	if (db->sv)
		free(db->sv);
	if (db->kv)
		free(db->kv);
	free(db);
	return NULL;
}

void kanji_db_close(KanjiDB *db)
{
	free(db->pv);
	free(db->sv);
	free(db->kv);
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

uint kanji_db_lookup(KanjiDB *db, const Kanji *k, KanjiMatch *res, uint n)
{
	uint i;

	rep (i, n) {
		res[i].score = HUGE_VAL;
		res[i].code = 0xFFFF;
	}
	rep (i, db->n) {
		//fprintf(stderr, "%i/%i\n", i, db->n);
		double d = kanji_dist(db->d, k, db->kv + i);
		heap_insert(res, n, d, db->kv[i].code);
	}
	heap_finish(res, n);
	while (n && res[n-1].code == 0xFFFF)
		n--;
	return n;
}
