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

/* Deprecated code to create databases for lookup_vptree.c */
/* VP-trees did not turn out to be significantly faster than linear search */

#define PROGRESS(x, i, n) fprintf(stderr,\
	"\x1b[1F\x1b[2K\x1b[1G"\
	"%s... %u/%u\n", (x), (uint)(i), (uint)(n))
#define PROGRESS_DONE(x) fprintf(stderr,\
	"\x1b[1F\x1b[2K\x1b[1G"\
	"%s done\n\n", (x))

typedef struct vp VP;
typedef struct kanji_db KanjiDB;

struct kanji_db {
	KanjiDist *d;
	Vec2 *pv;
	Stroke *sv;
	VP *nodev;
	VP *root;
	uint n, np, ns;
};

struct vp {
	Kanji kanji;
	double level;
	uint insize;
	VP *in, *out;
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

static uint partition_vp(VP **v, double *d, uint m, uint n, double *level)
{
	uint l, r;
	VP *vt;
	double dt;

	
	if (n == 0) {
		*level = 0.0; /* value doesn't matter */
		return 0;
	}
	if (n == 1) {
		*level = HUGE_VAL;
		return n;
	}

	ran (l, 1, n) if (d[0] != d[l])
		goto nondegerate;
	/* welp, all distances are the same */
	*level = HUGE_VAL;
	if (d[0] == HUGE_VAL)
		*level = 999999999999.0;
	return 0;
nondegerate:
	if (fabs(d[0]) == HUGE_VAL) {
		dt = d[0]; d[0] = d[l]; d[l] = dt;
		vt = v[0]; v[0] = v[l]; v[l] = vt;
	}

	l = 1;
	r = n;
	/* quickmedian, using v[0] as pivot */
	do {
		while (l < r && d[l] > d[0]) {
			r--;
			dt = d[r]; d[r] = d[l]; d[l] = dt;
			vt = v[r]; v[r] = v[l]; v[l] = vt;
		}
		l++;
	} while (l < r);
	if (r == m) { /* pivot is the median */
		*level = .5*(d[0] + (r == n ? 999999999999.0 : d[r]));
		return r;
	} else if (r == 0) { /* pivot is the unique minimum */
		return 1+partition_vp(v+1, d+1, m-1, n-1, level);
	} else if (r == n) { /* pivot is the unique maximum */
		return partition_vp(v+1, d+1, m, n-1, level);
	} if (m < r) {
		return partition_vp(v, d, m, r, level);
	} else /* if (m > r) */ {
		return r + partition_vp(v+r, d+r, m-r, n-r, level);
	}
}

static VP **v_start;
static uint v_total;
static VP *build_vp_range(KanjiDist *kd, VP **v, double *d, uint n)
{
	uint m, i, iter, p;
	VP *pivot;

	if (n == 0)
		return NULL;
	p = rand()%n;
	pivot = v[p];
	v[p] = v[0];
	v++;
	n--;

	rep (i, n)
		d[i] = kanji_dist(kd, &pivot->kanji, &v[i]->kanji);

	m = partition_vp(v, d, n/2, n, &pivot->level);
	pivot->in = build_vp_range(kd, v, d, m);
	pivot->out = build_vp_range(kd, v+m, d, n-m);
	PROGRESS("build tree", (v+n)-v_start, v_total);
	return pivot;
}

static void dumpdepth(VP *v, uint d)
{
	if (!v)
		return;
	d++;
	fprintf(stderr, "%u\n", d);
	dumpdepth(v->in, d);
	dumpdepth(v->out, d);
}

static uint calc_sizes(VP *v)
{
	if (!v)
		return 0;
	v->insize = calc_sizes(v->in);
	return v->insize + calc_sizes(v->out) + 1;
}

static VP *build_vp(KanjiDist *kd, VP *v, uint n)
{
	uint i;
	VP *r;
	double *d;
	VP **p;

	PROGRESS("build tree", 0, n);
	p = malloc(n * sizeof *p);
	d = malloc(n * sizeof *d);
	rep (i, n)
		p[i] = v+i;
	v_start = p;
	v_total = n;
	r = build_vp_range(kd, p, d, n);
	PROGRESS_DONE("build tree");
	calc_sizes(r);
	//dumpdepth(r, 0);
	free(p);
	return r;
}

KanjiDB *kanji_db_open(char *data, size_t n)
{
	KanjiDB *db;
	uint nk, ns, np, si, ki;
	char *end;
	Vec2 *pm;
	Stroke *sm;

	PROGRESS("parse database", 0, 1);
	end = data+n;
	nk = getuint(&data, end);
	ns = getuint(&data, end);
	np = getuint(&data, end);
	if (!(db = malloc(sizeof *db)))
		return NULL;
	db->pv = NULL;
	db->sv = NULL;
	db->nodev = NULL;
	if (!(db->d = kanji_dist_new()))
		goto err_n_free;
	if (!(db->pv = malloc(np * sizeof *db->pv)))
		goto err_n_free;
	if (!(db->sv = malloc(ns * sizeof *db->sv)))
		goto err_n_free;
	if (!(db->nodev = malloc(nk * sizeof *db->nodev)))
		goto err_n_free;
	db->n = nk;
	db->np = np;
	db->ns = ns;
	pm = db->pv;
	sm = db->sv;
	rep (ki, nk) {
		Kanji *const k = &db->nodev[ki].kanji;
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
	PROGRESS_DONE("parse database");
	db->root = build_vp(db->d, db->nodev, db->n);
	return db;
err_n_free:
	if (db->pv)
		free(db->pv);
	if (db->sv)
		free(db->sv);
	if (db->nodev)
		free(db->nodev);
	free(db);
	return NULL;
}

void kanji_db_close(KanjiDB *db)
{
	free(db->pv);
	free(db->sv);
	free(db->nodev);
	kanji_dist_free(db->d);
	free(db);
}

static KanjiDB *open_db_file(const char *name)
{
	FILE *f;
	char *data;
	size_t n, m;
	KanjiDB *db;

	n = 0;
	m = 8*1024*1024;
	data = malloc(m);

	PROGRESS("read database", 0, 1);
	f = cfopen(name, "rb");
	for (;;) {
		n += fread(data, 1, m-n, f);
		if (ferror(f))
			exit_perror(name);
		if (n == m) {
			data = realloc(data, m *= 2);
		} else
			break;
	}
	PROGRESS_DONE("read database");
	fclose(f);
	if (!(db = kanji_db_open(data, n)))
		exit_error("kanji_db_open failed");
	free(data);
	return db;
}

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

static void put_f32(double f)
{
	f = 1./(1+f*.25);
	assert(0 <= f && f <= 1);
	ulong i = round(f*((1UL<<32)-1));
	assert((i&0xFFFFFFFF) == i);
	putchar(i&0xFF);
	putchar(i>>8&0xFF);
	putchar(i>>16&0xFF);
	putchar(i>>24&0xFF);
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

static void put_vp(VP *v, uint size)
{
	put_kanji(&v->kanji);
	put_f32(v->level);
	put_varlen(v->insize);
	if (!v->in)
		assert(v->insize == 0);
	else
		put_vp(v->in, v->insize);
	if (!v->out)
		assert(v->insize == size - 1);
	else
		put_vp(v->out, size - v->insize - 1);
}

static void write_vptree(KanjiDB *db)
{
	fputs("CCR0VP00", stdout);
	put_varlen(db->n);
	put_varlen(db->ns);
	put_varlen(db->np);
	put_vp(db->root, db->n);
}

int main(int argc, char **argv)
{
	KanjiDB *db = open_db_file(argc > 1 ? argv[1] : "kanji.list" );
	write_vptree(db);
	kanji_db_close(db);
}
