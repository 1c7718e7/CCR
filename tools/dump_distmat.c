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

/* This program reads a klist and computes all pairwise distances.
   Mainly for use with test_metric.
   load in gnuplot with: plot 'mat' matrix skip 1
   Output format:
   <number of points>
   d1,1 d1,2 ... d1,n
   d2,1 d2,2 ... d2,n
   .... .... ... ....
   dn,1 dn,2 ... d1,n
*/

struct kanji_db {
	KanjiDist *d;
	Vec2 *pv;
	Stroke *sv;
	Kanji *kv;
	uint n;
};
typedef struct kanji_db KanjiDB;

/* TODO switch to klist_io */
KanjiDB *readdb(FILE *f)
{
	KanjiDB *db;
	uint nk, ns, np, si, ki;
	Vec2 *pm;
	Stroke *sm;

	if (3 != fscanf(f, "%u%u%u", &nk, &ns, &np))
		return NULL;
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
		if (2 != fscanf(f, "%" SCNu32 "%u", &k->code, &k->n))
			goto err_n_free;
		k->p = sm;
		rep (si, k->n) {
			if (!ns)
				goto err_n_free; /* more strokes than promised */
			sm->p = pm;
			sm->n = 0;
			do {
				if (!np)
					goto err_n_free; /* more points than promised */
				if (2 != fscanf(f, "%lf%lf", &pm->x, &pm->y))
					goto err_n_free;
				sm->n++;
				pm++;
				np--;
			} while (fgetc(f) != '\n');
			ns--;
			sm++;
		}
	}
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

static double mat[12000][12000];
static KanjiDB *db;

static void calc_mat()
{
	uint i, j, prog;
	FILE *f;

	fprintf(stderr, "calculating distances     ");
	prog = 0;
	rep (i, db->n) {
		rep (j, db->n)
			mat[i][j] = kanji_dist(db->d, &db->kv[i], &db->kv[j]);
		while (i*70 > prog*db->n) {
			prog++;
			fprintf(stderr, "\x1b[5D. %3i%%", prog*100/70);
		}
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "dumping...\n");
	f = stdout;
	fprintf(f, "%u\n", db->n);
	rep (i, db->n) {
		rep (j, db->n-1)
			fprintf(f, "%lf ", mat[i][j]);
		fprintf(f, "%lf\n", mat[i][j]);
	}
}

int main(int argc, char **argv)
{
	FILE *f;

	f = cfopen(argc == 1 ? "kanji.list" : argv[1], "rb");
	fprintf(stderr, "opening...\n");
	db = readdb(f);
	calc_mat();
	kanji_db_close(db);
	fclose(f);
	return 0;
}
